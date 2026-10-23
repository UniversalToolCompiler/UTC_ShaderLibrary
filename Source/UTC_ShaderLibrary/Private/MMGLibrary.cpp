/** Copyright 2023, Universal Tool Compiler */

#include "MMGLibrary.h"
#include "UI/MMGTreeView.h"
#include "UTC_Utils.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "ComponentReregisterContext.h"
#include "EditorAssetLibrary.h"
#include "Factories/MaterialFactoryNew.h"
#include "Factories/MaterialInstanceConstantFactoryNew.h"
#include "GraphEditor.h"
#include "IMaterialEditor.h"
#include "MaterialGraph/MaterialGraph.h"
#include "Materials/Material.h"
#include "Materials/MaterialExpressionAppendVector.h"
#include "Materials/MaterialExpressionBlendMaterialAttributes.h"
#include "Materials/MaterialExpressionComment.h"
#include "Materials/MaterialExpressionComponentMask.h"
#include "Materials/MaterialExpressionConstant.h"
#include "Materials/MaterialExpressionConstant2Vector.h"
#include "Materials/MaterialExpressionConstant3Vector.h"
#include "Materials/MaterialExpressionConstant4Vector.h"
#include "Materials/MaterialExpressionMakeMaterialAttributes.h"
#include "Materials/MaterialExpressionNamedReroute.h"
#include "Materials/MaterialExpressionScalarParameter.h"
#include "Materials/MaterialExpressionLinearInterpolate.h"
#include "Materials/MaterialExpressionStaticBool.h"
#include "Materials/MaterialExpressionSubstrate.h"
#include "Materials/MaterialExpressionStaticBoolParameter.h"
#include "Materials/MaterialExpressionTextureObject.h"
#include "Materials/MaterialExpressionTextureObjectParameter.h"
#include "Materials/MaterialExpressionTextureSample.h"
#include "Materials/MaterialExpressionTextureSampleParameter2D.h"
#include "Materials/MaterialExpressionVectorParameter.h"
#include "Materials/MaterialFunction.h"
#include "Materials/MaterialInstanceConstant.h"
#include "Misc/ScopedSlowTask.h"
#include "Subsystems/AssetEditorSubsystem.h"

#define LOCTEXT_NAMESPACE "MMGLibrary"

DEFINE_LOG_CATEGORY_STATIC(LogMMGLibrary, Log, All);

void FMMGLibrary::GenerateMaterial(const FMMGGenerateMaterialStruct& MainSettings, const TArray<FMMGTreeViewPtr>& TreeViewParameters, const FMMGMaterialSettingsStruct& MaterialSettings)
{
	bAddToMaterial = false;
	bSubstrate = MaterialSettings.UseSubstrate();

	TreeViewList = TreeViewParameters;
	Settings = MainSettings;
	PackageName = GetDestinationFolder() + Settings.MaterialName;

	if (!ErrorDetector())
	{
		return;
	}

	UPackage* Package = CreatePackage(*PackageName);

	/** Generate the main material */
	UMaterialFactoryNew* MaterialFactory = NewObject<UMaterialFactoryNew>();
	UTCMaterial = Cast<UMaterial>(MaterialFactory->FactoryCreateNew(UMaterial::StaticClass(), Package, *Settings.MaterialName, RF_Standalone | RF_Public, nullptr, GWarn));
	FAssetRegistryModule::AssetCreated(UTCMaterial);

	Package->FullyLoad();
	Package->SetDirtyFlag(true);

	/**Material Settings: Substrate reads its front material from the root pin, so it cannot go through material attributes */
	UTCMaterial->bUseMaterialAttributes = bSubstrate ? 0 : 1;
	UTCMaterial->MaterialDomain = MaterialSettings.MaterialDomain;
	UTCMaterial->BlendMode = MaterialSettings.BlendMode;
	UTCMaterial->SetShadingModel(MaterialSettings.ShadingModel);
	UTCMaterial->TwoSided = MaterialSettings.TwoSided;

	InitNodesGen();

	UTCMaterial->PreEditChange(nullptr);
	UTCMaterial->PostEditChange();

	FGlobalComponentReregisterContext RecreateComponents;

	if (Settings.bGenerateMaterialInstance)
	{
		GenerateMaterialInstance();
	}

	ReinitVar();
}

FString FMMGLibrary::GetDestinationFolder() const
{
	return Settings.MaterialDestination.Path.IsEmpty() ? FString(TEXT("/Game/")) : Settings.MaterialDestination.Path + TEXT("/");
}

/** MM_Name gets MI_Name */
void FMMGLibrary::GenerateMaterialInstance()
{
	FString InstanceName = Settings.MaterialName;
	InstanceName.RemoveFromStart(TEXT("MM_"));
	InstanceName = TEXT("MI_") + InstanceName;

	PackageName = GetDestinationFolder() + InstanceName;
	UPackage* InstancePackage = CreatePackage(*PackageName);

	UMaterialInstanceConstantFactoryNew* InstanceFactory = NewObject<UMaterialInstanceConstantFactoryNew>();
	UMaterialInstanceConstant* UTCInstance = Cast<UMaterialInstanceConstant>(InstanceFactory->FactoryCreateNew(UMaterialInstanceConstant::StaticClass(), InstancePackage, *InstanceName, RF_Standalone | RF_Public, nullptr, GWarn));
	FAssetRegistryModule::AssetCreated(UTCInstance);

	InstancePackage->FullyLoad();
	InstancePackage->SetDirtyFlag(true);

	UTCInstance->Parent = UTCMaterial;

	UTCInstance->PreEditChange(nullptr);
	UTCInstance->PostEditChange();
}

void FMMGLibrary::AddToMaterial(const FMMGAddToMaterialStruct& MainSettings, const TArray<FMMGTreeViewPtr>& TreeViewParameters, const FMMGMaterialSettingsStruct& MaterialSettings)
{
	bAddToMaterial = true;
	bSubstrate = MaterialSettings.UseSubstrate();

	TreeViewList = TreeViewParameters;
	UTCMaterial = MainSettings.TargetMaterial;

	if (!ErrorDetector())
	{
		bAddToMaterial = false;
		return;
	}

	UPackage* Package = UTCMaterial->GetPackage();
	Package->FullyLoad();
	Package->SetDirtyFlag(true);

	/** The nodes go through the material editor, which edits its own copy of the material */
	SGraphEditor* GraphWidget = FindGraphEditorByMaterial(UTCMaterial);
	if (!GraphWidget)
	{
		FUTC_Utils::SpawnNotification(LOCTEXT("GraphNotFound", "Graph not found, try to undock the target material editor"));
		return;
	}

	EdGraph = GraphWidget->GetCurrentGraph();
	MatGraph = static_cast<UMaterialGraph*>(EdGraph);

	IAssetEditorInstance* AssetEditorInstance = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->FindEditorForAsset(UTCMaterial, false);
	MaterialEd = static_cast<IMaterialEditor*>(AssetEditorInstance);

	/** Start below the existing nodes */
	int32 MaxNodeY = MaterialParameterPosition.Y;
	for (const UEdGraphNode* Node : EdGraph->Nodes)
	{
		MaxNodeY = FMath::Max(MaxNodeY, Node->NodePosY);
	}
	MaxNodeY = FMath::Max(MaxNodeY, UTCMaterial->EditorY);
	MaterialParameterPosition.Y = MaxNodeY + 750;

	PackedTextureParamPtr = nullptr;
	InitNodesGen();

	MatGraph->RebuildGraph();

	if (IsValid(JumpToThisExpression))
	{
		MaterialEd->JumpToExpression(JumpToThisExpression);
	}

	UTCMaterial->PreEditChange(nullptr);
	UTCMaterial->PostEditChange();

	ReinitVar();
}

template <typename ExpressionType>
ExpressionType* FMMGLibrary::NewExpression(bool bAutoAssignResource)
{
	if (bAddToMaterial)
	{
		return Cast<ExpressionType>(MaterialEd->CreateNewMaterialExpression(ExpressionType::StaticClass(), FVector2D(0, 0), false, bAutoAssignResource, EdGraph));
	}

	ExpressionType* Expression = NewObject<ExpressionType>(UTCMaterial);
	UTCMaterial->GetEditorOnlyData()->ExpressionCollection.Expressions.Add(Expression);
	return Expression;
}

UMaterialExpressionComment* FMMGLibrary::NewComment()
{
	if (bAddToMaterial)
	{
		return MaterialEd->CreateNewMaterialExpressionComment(FVector2D(0, 0), EdGraph);
	}
	return NewExpression<UMaterialExpressionComment>();
}

void FMMGLibrary::InitNodesGen()
{
	FScopedSlowTask AddToMaterialTask(TreeViewList.Num(), LOCTEXT("AddToMaterialNewNodes", "Add to Material new nodes"));
	AddToMaterialTask.MakeDialog();

	for (const FMMGTreeViewPtr& TreeViewParent : TreeViewList)
	{
		AddToMaterialTask.EnterProgressFrame();
		AddToMaterialTask.FrameMessage = FText::AsCultureInvariant(FString(TEXT("Add to Material new nodes: ")) + *TreeViewParent->FunctionName);

		TreeViewParentPtr = TreeViewParent;

		if (*TreeViewParentPtr->FunctionType == MMGNames::MaterialAttribute)
		{
			GenerateMaterialAttributeFunction();
		}
		if (*TreeViewParentPtr->FunctionType == MMGNames::Mask)
		{
			GenerateMaskFunction();
		}
	}
	ConnectRerouteOutput();
}

bool FMMGLibrary::HasUVs() const
{
	return TreeViewParentPtr->CurrentUVsComboItem.IsValid() && *TreeViewParentPtr->CurrentUVsComboItem != MMGNames::NoUV;
}

/** Make Material Attributes (or Substrate Slab) fed by one material function per child, behind a reroute named after the function */
void FMMGLibrary::GenerateMaterialAttributeFunction()
{
	if (bSubstrate)
	{
		GenerateSubstrateSlab();
		FunctionRootPtr = SlabPtr;
		MaterialAttributePtr = nullptr;
	}
	else
	{
		MaterialAttributePtr = NewExpression<UMaterialExpressionMakeMaterialAttributes>();
		FunctionRootPtr = MaterialAttributePtr;
	}
	if (bAddToMaterial && TreeViewList[0] == TreeViewParentPtr)
	{
		JumpToThisExpression = FunctionRootPtr;
	}

	MANamedRerouteDeclarationPtr = NewExpression<UMaterialExpressionNamedRerouteDeclaration>();
	MANamedRerouteDeclarationPtr->Name = FName(*TreeViewParentPtr->FunctionName);
	MANamedRerouteDeclarationPtr->Input.Expression = FunctionRootPtr;

	RerouteDeclarationMap.Add(*TreeViewParentPtr->FunctionName, MANamedRerouteDeclarationPtr);

	if (HasUVs() && HasToGenerateUVsFunction())
	{
		GenerateUVsFunction();
	}

	/** Generate Material Outputs */
	for (const FMMGTreeViewPtr& TreeViewChild : TreeViewParentPtr->ChildFunctions)
	{
		TreeViewChildPtr = TreeViewChild;

		/** Custom Packed: one material function per used channel, sharing one texture */
		if (!TreeViewChildPtr->ChildFunctions.IsEmpty() && TreeViewChildPtr->ChildFunctions[0]->IsCustomPack)
		{
			ProceedCustomPacked = true;
			CustomPackedInputIndex = 0;
			CustomPackedName = GetCustomPackedName();

			for (const TPair<FString, FString>& Pair : TreeViewChildPtr->ChildFunctions[0]->CustomPackedSelectionSave)
			{
				if (FUTC_Utils::IsPackedChannelSet(Pair.Value))
				{
					CurrentPackedOutput = Pair.Key;
					GenerateMaterialOutputs(FName(FUTC_Utils::TextToPascal(Pair.Value)));
				}
			}
			DoOnceCustomPacked = true;
		}
		/** Classic */
		else
		{
			ProceedCustomPacked = false;
			GenerateMaterialOutputs(FName(FUTC_Utils::TextToPascal(*TreeViewChildPtr->CurrentChildComboItem)));
		}
	}

	SetNodePosition(FunctionRootPtr, EMaterialAttribute);
	SetNodePosition(MANamedRerouteDeclarationPtr, ERerouteDeclarationMA);

	/** Generate UVs parameters, or drop the UVs function if no texture uses it */
	if (HasUVs() && HasToGenerateUVsFunction())
	{
		if (isUVsFunctionConnected)
		{
			FunctionDataStruct = MMGAssets::GetUVs()->FindRow<FMMGMaterialFunctionStruct>(FName(FUTC_Utils::TextToPascal(*TreeViewParentPtr->CurrentUVsComboItem)), TEXT(""));

			UVsParameterPosition.Y = FunctionRootPtr->MaterialExpressionEditorY;
			GenerateUVsParameters();

			SetNodePosition(UVsFunctionPtr, EUVsFunction);

			ParamExpressionList.Empty();
		}
		else
		{
			UTCMaterial->GetEditorOnlyData()->ExpressionCollection.Expressions.Remove(UVsFunctionPtr);
		}
	}

	/** Reorder reroute usage to fit with MA */
	for (EMAttributes Attribute : TEnumRange<EMAttributes>())
	{
		if (UMaterialExpression* const* RerouteUsage = RerouteUsageMap.Find(Attribute))
		{
			NamedRerouteUsageList.Add(*RerouteUsage);
		}
	}

	for (UMaterialExpression* RerouteNode : NamedRerouteUsageList)
	{
		SetNodePosition(RerouteNode, ERerouteUsageFunction);
	}

	if (bSubstrate)
	{
		SetSubstrateSlabPositions();
	}

	/** Main comment node */
	MainCommentPtr = NewComment();
	MainCommentPtr->CommentColor = FLinearColor(0, 0, 0, 1);
	MainCommentPtr->Text = *TreeViewParentPtr->FunctionName;
	SetNodePosition(MainCommentPtr, EMainComment);

	RerouteUsageMap.Empty();
	NamedRerouteUsageList.Empty();
	MFExpressionList.Empty();
	CommentYPositionDoOnce = true;
	isUVsFunctionConnected = false;
	PackedTextureParamPtr = nullptr;

	MaterialParameterPosition.Y += 200;
}

/** One child attribute: its material function, a parameter per input, reroutes and comment */
void FMMGLibrary::GenerateMaterialOutputs(FName RowName)
{
	FunctionDataStruct = MMGAssets::GetMaterialOutputs()->FindRow<FMMGMaterialFunctionStruct>(RowName, TEXT(""));

	GenerateMaterialFunctionNode(RowName.ToString());
	ConnectFunctionToMaterialAttribute();

	/** Refresh, so the function call lists the inputs of its function */
	UTCMaterial->PreEditChange(nullptr);
	UTCMaterial->PostEditChange();

	const FString RowLabel = FUTC_Utils::PascalToText(RowName.ToString());
	InputIndex = 0;
	const TArray<FFunctionExpressionInput> InputsList = MaterialFunctionPtr->FunctionInputs;
	for (const FFunctionExpressionInput& Input : InputsList)
	{
		const FString InputName = Input.ExpressionInput->InputName.ToString();
		if (ProceedCustomPacked && !InputName.StartsWith(TEXT("Texture")))
		{
			CustomPackedInputIndex++;
		}

		/** Parameter names carry the function name when the material holds several functions */
		if (TreeViewList.Num() > 1 || bAddToMaterial)
		{
			ParameterName = FName(*TreeViewParentPtr->FunctionName + TEXT(" - ") + RowLabel + TEXT(" - ") + InputName);
		}
		else
		{
			ParameterName = FName(RowLabel + TEXT(" - ") + InputName);
		}

		GenerateParameterByMFInputType(MaterialFunctionPtr, Input);
		++InputIndex;
	}

	MaterialParameterPosition.Y += 100;

	SetNodePosition(MaterialFunctionPtr, EMaterialFunction);
	if (ProceedCustomPacked)
	{
		SetNodePosition(PackedTextureParamPtr, EPackedTexture);
		SetNodePosition(PackedNamedRerouteDeclarationPtr, EPackedRerouteDeclaration);
	}
	SetNodePosition(FunctionNamedRerouteDeclarationPtr, ERerouteDeclarationFunction);
	SetNodePosition(FunctionNamedRerouteUsagePtr, ERerouteUsageFunction);

	/** Comment node */
	CommentPtr = NewComment();
	CommentPtr->CommentColor = FLinearColor(0.019f, 0.076f, 0.13f, 1.f);
	CommentPtr->Text = RowName.ToString();
	SetNodePosition(CommentPtr, EComment);

	ParamExpressionList.Empty();
}

/** Blend Material Attributes (or Substrate Horizontal Blend) whose alpha is the mask function, behind a reroute named after the function */
void FMMGLibrary::GenerateMaskFunction()
{
	if (bSubstrate)
	{
		GenerateSubstrateHorizontalBlend();
		FunctionRootPtr = HorizontalBlendPtr;
		BlendAttributesPtr = nullptr;
	}
	else
	{
		BlendAttributesPtr = NewExpression<UMaterialExpressionBlendMaterialAttributes>();
		BlendAttributeMap.Add(*TreeViewParentPtr->FunctionName, BlendAttributesPtr);
		FunctionRootPtr = BlendAttributesPtr;
	}
	if (bAddToMaterial && TreeViewList[0] == TreeViewParentPtr)
	{
		JumpToThisExpression = FunctionRootPtr;
	}

	MANamedRerouteDeclarationPtr = NewExpression<UMaterialExpressionNamedRerouteDeclaration>();
	MANamedRerouteDeclarationPtr->Name = FName(*TreeViewParentPtr->FunctionName);
	MANamedRerouteDeclarationPtr->Input.Expression = FunctionRootPtr;

	RerouteDeclarationMap.Add(*TreeViewParentPtr->FunctionName, MANamedRerouteDeclarationPtr);

	if (HasUVs() && HasToGenerateUVsFunction())
	{
		GenerateUVsFunction();
	}

	for (const FMMGTreeViewPtr& TreeViewChild : TreeViewParentPtr->ChildFunctions)
	{
		TreeViewChildPtr = TreeViewChild;

		const FName RowName = FName(FUTC_Utils::TextToPascal(*TreeViewChildPtr->CurrentChildComboItem));
		FunctionDataStruct = MMGAssets::GetMasks()->FindRow<FMMGMaterialFunctionStruct>(RowName, TEXT(""));

		GenerateMaterialFunctionNode(RowName.ToString());
		if (bSubstrate)
		{
			HorizontalBlendPtr->Mix.Expression = FunctionNamedRerouteUsagePtr;
			MaskAlphaDeclarations.Add(*TreeViewParentPtr->FunctionName, FunctionNamedRerouteDeclarationPtr);
		}
		else
		{
			BlendAttributesPtr->Alpha.Expression = FunctionNamedRerouteUsagePtr;
		}

		/** Refresh, so the function call lists the inputs of its function */
		UTCMaterial->PreEditChange(nullptr);
		UTCMaterial->PostEditChange();

		InputIndex = 0;
		const TArray<FFunctionExpressionInput> InputsList = MaterialFunctionPtr->FunctionInputs;
		for (const FFunctionExpressionInput& Input : InputsList)
		{
			ParameterName = FName(*TreeViewParentPtr->FunctionName + TEXT(" - ") + Input.ExpressionInput->InputName.ToString());
			GenerateParameterByMFInputType(MaterialFunctionPtr, Input);
			++InputIndex;
		}

		MaterialParameterPosition.Y += 100;

		SetNodePosition(MaterialFunctionPtr, EMaterialFunction);
		SetNodePosition(FunctionNamedRerouteDeclarationPtr, ERerouteDeclarationFunction);
		SetNodePosition(FunctionNamedRerouteUsagePtr, ERerouteUsageFunction);

		/** Comment node */
		CommentPtr = NewComment();
		CommentPtr->CommentColor = FLinearColor(0.13f, 0.08f, 0.025f, 1.f);
		CommentPtr->Text = RowName.ToString();
		SetNodePosition(CommentPtr, EComment);

		ParamExpressionList.Empty();
	}

	SetNodePosition(FunctionRootPtr, EMaterialAttribute);
	SetNodePosition(MANamedRerouteDeclarationPtr, ERerouteDeclarationMA);

	/** Generate UVs parameters, or drop the UVs function if no texture uses it */
	if (HasUVs() && HasToGenerateUVsFunction())
	{
		if (isUVsFunctionConnected)
		{
			FunctionDataStruct = MMGAssets::GetUVs()->FindRow<FMMGMaterialFunctionStruct>(FName(FUTC_Utils::TextToPascal(*TreeViewParentPtr->CurrentUVsComboItem)), TEXT(""));

			UVsParameterPosition.Y = FunctionRootPtr->MaterialExpressionEditorY;
			GenerateUVsParameters();

			SetNodePosition(UVsFunctionPtr, EUVsFunction);

			ParamExpressionList.Empty();
		}
		else
		{
			UTCMaterial->GetEditorOnlyData()->ExpressionCollection.Expressions.Remove(UVsFunctionPtr);
		}
	}

	for (UMaterialExpression* RerouteNode : NamedRerouteUsageList)
	{
		SetNodePosition(RerouteNode, ERerouteUsageFunction);
	}

	if (bSubstrate)
	{
		SetSubstrateBlendPositions();
	}

	/** Main comment node */
	MainCommentPtr = NewComment();
	MainCommentPtr->CommentColor = FLinearColor(0, 0, 0, 1);
	MainCommentPtr->Text = *TreeViewParentPtr->FunctionName;
	SetNodePosition(MainCommentPtr, EMainComment);

	NamedRerouteUsageList.Empty();
	MFExpressionList.Empty();
	CommentYPositionDoOnce = true;
	isUVsFunctionConnected = false;
	MaterialParameterPosition.Y += 200;
}

void FMMGLibrary::GenerateUVsFunction()
{
	FunctionDataStruct = MMGAssets::GetUVs()->FindRow<FMMGMaterialFunctionStruct>(FName(FUTC_Utils::TextToPascal(*TreeViewParentPtr->CurrentUVsComboItem)), TEXT(""));

	UVsFunctionPtr = NewExpression<UMaterialExpressionMaterialFunctionCall>();
	UVsFunctionPtr->MaterialFunction = FunctionDataStruct->MaterialFunction;
	if (bAddToMaterial)
	{
		UVsFunctionPtr->UpdateFromFunctionResource(false);
		MatGraph->RebuildGraph();
	}

	/** Refresh */
	UTCMaterial->PreEditChange(nullptr);
	UTCMaterial->PostEditChange();
}

void FMMGLibrary::GenerateUVsParameters()
{
	InputIndex = 0;
	const TArray<FFunctionExpressionInput> InputList = UVsFunctionPtr->FunctionInputs;
	for (const FFunctionExpressionInput& Input : InputList)
	{
		ParameterName = FName(*TreeViewParentPtr->FunctionName + TEXT(" - ") + Input.ExpressionInput->InputName.ToString());
		GenerateParameterByMFInputType(UVsFunctionPtr, Input, true);
		++InputIndex;
	}
}

bool FMMGLibrary::HasToGenerateUVsFunction() const
{
	return TreeViewParentPtr->ChildFunctions.ContainsByPredicate([](const FMMGTreeViewPtr& Child) { return Child->AffectedByUVs; });
}

/** Material function call with its reroute declaration and usage */
void FMMGLibrary::GenerateMaterialFunctionNode(const FString& FunctionName)
{
	MaterialFunctionPtr = NewExpression<UMaterialExpressionMaterialFunctionCall>();
	MaterialFunctionPtr->MaterialFunction = FunctionDataStruct->MaterialFunction;
	if (bAddToMaterial)
	{
		MaterialFunctionPtr->UpdateFromFunctionResource(false);
		MatGraph->RebuildGraph();
	}
	MFExpressionList.Add(MaterialFunctionPtr);

	FunctionNamedRerouteDeclarationPtr = NewExpression<UMaterialExpressionNamedRerouteDeclaration>();
	FunctionNamedRerouteDeclarationPtr->Name = FName(*TreeViewParentPtr->FunctionName + TEXT(" - ") + FunctionName);

	FunctionNamedRerouteUsagePtr = NewExpression<UMaterialExpressionNamedRerouteUsage>();
	FunctionNamedRerouteUsagePtr->Declaration = FunctionNamedRerouteDeclarationPtr;

	FunctionNamedRerouteDeclarationPtr->Input.Expression = MaterialFunctionPtr;

	if (*TreeViewParentPtr->FunctionType == MMGNames::MaterialAttribute)
	{
		RerouteUsageMap.Add(FunctionDataStruct->MaterialAttributeType.GetValue(), FunctionNamedRerouteUsagePtr);
	}
	if (*TreeViewParentPtr->FunctionType == MMGNames::Mask)
	{
		NamedRerouteUsageList.Add(FunctionNamedRerouteUsagePtr);
	}
}

/** Parameter matching the type of a function input, in a group per function and child */
void FMMGLibrary::GenerateParameterByMFInputType(UMaterialExpressionMaterialFunctionCall* MaterialFunction, const FFunctionExpressionInput& CurrentInput, bool isUVsFunction)
{
	const EFunctionInputType InputType = CurrentInput.ExpressionInput->InputType.GetValue();

	/** Preview default value */
	UMaterialExpression* PreviewExpression = MaterialFunction->FunctionInputs[InputIndex].ExpressionInput->Preview.Expression;

	const bool bMask = (*TreeViewParentPtr->FunctionType == MMGNames::Mask);
	const bool bMaterialAttribute = (*TreeViewParentPtr->FunctionType == MMGNames::MaterialAttribute);
	const FString& FunctionName = *TreeViewParentPtr->FunctionName;
	const FString ChildNumber = FString::FromInt(TreeViewParentPtr->ChildFunctions.Find(TreeViewChildPtr) + 1);

	FString GroupName;
	if (isUVsFunction)
	{
		if (bMask)
		{
			GroupName = TEXT("~ ") + FunctionName + TEXT(" 00 - ") + FunctionDataStruct->MaterialGroup + TEXT(" ~");
		}
		else if (bMaterialAttribute)
		{
			GroupName = TEXT("- ") + FunctionName + TEXT(" 00 - ") + FunctionDataStruct->MaterialGroup + TEXT(" -");
		}
	}
	else if (bMask)
	{
		GroupName = TEXT("~ ") + FunctionName + TEXT(" 01 - ") + FunctionDataStruct->MaterialGroup + TEXT(" ~");
		SortPriority = InputIndex;
	}
	else if (bMaterialAttribute && !ProceedCustomPacked)
	{
		GroupName = TEXT("- ") + FunctionName + TEXT(" 0") + ChildNumber + TEXT(" - ") + FunctionDataStruct->MaterialGroup + TEXT(" -");
		SortPriority = InputIndex;
	}
	else if (bMaterialAttribute && ProceedCustomPacked)
	{
		GroupName = TEXT("- ") + FunctionName + TEXT(" 0") + ChildNumber + TEXT(" - ") + CustomPackedName + TEXT(" -");
		SortPriority = CustomPackedInputIndex;
	}

	switch (InputType)
	{
	case FunctionInput_Scalar:
	case FunctionInput_Vector2:
	case FunctionInput_Vector3:
	case FunctionInput_Vector4:
		/** Vector inputs named "Texture..." take a texture sample */
		if (MaterialFunction->FunctionInputs[InputIndex].ExpressionInput->InputName.ToString().StartsWith(TEXT("Texture")))
		{
			if (!ProceedCustomPacked)
			{
				GenerateTextureSampleParameter(MaterialFunction, PreviewExpression, CurrentInput, InputType, GroupName, isUVsFunction);
			}
			/** The first channel of a packed texture creates the texture, the others read it */
			else if (DoOnceCustomPacked)
			{
				PackedTextureParamPtr = nullptr;
				CustomPackedMFList.Empty();

				GenerateTextureSampleParameter(MaterialFunction, PreviewExpression, CurrentInput, InputType, GroupName, isUVsFunction);
				SetCustomPackedRerouteUsage(MaterialFunction);

				CustomPackedMFList.Add(MaterialFunction);
				DoOnceCustomPacked = false;
			}
			else if (IsValid(PackedTextureParamPtr))
			{
				SetCustomPackedRerouteUsage(MaterialFunction);
				CustomPackedMFList.Add(MaterialFunction);
			}
		}
		else if (InputType == FunctionInput_Scalar)
		{
			GenerateScalarParameter(MaterialFunction, PreviewExpression, CurrentInput, GroupName, isUVsFunction);
		}
		else if (InputType == FunctionInput_Vector4)
		{
			GenerateVector4Parameter(MaterialFunction, PreviewExpression, CurrentInput, GroupName, isUVsFunction);
		}
		else
		{
			GenerateVector2n3Parameter(MaterialFunction, PreviewExpression, CurrentInput, InputType, GroupName, isUVsFunction);
		}
		break;

	case FunctionInput_Texture2D:
		GenerateTexture2DParameter(MaterialFunction, PreviewExpression, CurrentInput, GroupName, isUVsFunction);
		break;

	case FunctionInput_StaticBool:
		GenerateBoolParameter(MaterialFunction, PreviewExpression, CurrentInput, GroupName, isUVsFunction);
		break;

	default:
		UE_LOG(LogMMGLibrary, Warning, TEXT("Input '%s': material function input type %d is not supported"), *CurrentInput.ExpressionInput->InputName.ToString(), static_cast<int32>(InputType));
		break;
	}
}

void FMMGLibrary::GenerateTextureSampleParameter(UMaterialExpressionMaterialFunctionCall* MaterialFunction, UMaterialExpression* PreviewExpression, const FFunctionExpressionInput& CurrentInput, EFunctionInputType InputType, const FString& GroupName, const bool isUVsFunction)
{
	UMaterialExpressionTextureSampleParameter2D* TextureParam = NewExpression<UMaterialExpressionTextureSampleParameter2D>();

	/** Default value from the input preview */
	if (const UMaterialExpressionTextureSample* PreviewValue = Cast<UMaterialExpressionTextureSample>(PreviewExpression))
	{
		TextureParam->Texture = PreviewValue->Texture;
		TextureParam->SamplerType = PreviewValue->SamplerType;
	}

	/** Connect UVs */
	if (TreeViewChildPtr->AffectedByUVs && HasUVs())
	{
		TextureParam->Coordinates.Expression = UVsFunctionPtr;
		isUVsFunctionConnected = true;
	}

	/** Vector 4 inputs take the RGBA output */
	const int32 OutputIndex = (InputType == FunctionInput_Vector4) ? 5 : 0;

	if (isUVsFunction)
	{
		SetNodePosition(TextureParam, EUVsParameter);
	}
	else if (!ProceedCustomPacked)
	{
		SetNodePosition(TextureParam, EParameter);
	}

	TextureParam->Desc = CurrentInput.ExpressionInput->Description;
	TextureParam->Group = FName(GroupName);

	if (ProceedCustomPacked)
	{
		/** The packed texture goes through a reroute that each channel reads */
		PackedNamedRerouteDeclarationPtr = NewExpression<UMaterialExpressionNamedRerouteDeclaration>();

		TextureParam->SortPriority = 0;
		TextureParam->ParameterName = FName(*TreeViewParentPtr->FunctionName + TEXT(" - ") + CustomPackedName + TEXT(" - Texture"));

		PackedNamedRerouteDeclarationPtr->Name = FName(*TreeViewParentPtr->FunctionName + TEXT(" - ") + CustomPackedName);
		PackedTextureParamPtr = TextureParam;
		PackedNamedRerouteDeclarationPtr->Input.Connect(5, PackedTextureParamPtr);
	}
	else
	{
		TextureParam->SortPriority = SortPriority;
		TextureParam->ParameterName = ParameterName;
		MaterialFunction->FunctionInputs[InputIndex].Input.Connect(OutputIndex, TextureParam);
	}

	ParamExpressionList.Add(TextureParam);
}

void FMMGLibrary::GenerateScalarParameter(UMaterialExpressionMaterialFunctionCall* MaterialFunction, UMaterialExpression* PreviewExpression, const FFunctionExpressionInput& CurrentInput, const FString& GroupName, const bool isUVsFunction)
{
	UMaterialExpressionScalarParameter* ScalarParam = NewExpression<UMaterialExpressionScalarParameter>();

	const UMaterialExpressionConstant* PreviewValue = Cast<UMaterialExpressionConstant>(PreviewExpression);
	ScalarParam->DefaultValue = PreviewValue ? PreviewValue->R : 1.f;

	ScalarParam->Desc = CurrentInput.ExpressionInput->Description;
	ScalarParam->ParameterName = ParameterName;
	ScalarParam->SortPriority = SortPriority;
	ScalarParam->Group = FName(GroupName);

	SetNodePosition(ScalarParam, isUVsFunction ? EUVsParameter : EParameter, true);

	MaterialFunction->FunctionInputs[InputIndex].Input.Expression = ScalarParam;
	ParamExpressionList.Add(ScalarParam);
}

void FMMGLibrary::GenerateVector2n3Parameter(UMaterialExpressionMaterialFunctionCall* MaterialFunction, UMaterialExpression* PreviewExpression, const FFunctionExpressionInput& CurrentInput, EFunctionInputType InputType, const FString& GroupName, const bool isUVsFunction)
{
	UMaterialExpressionVectorParameter* VectorParam = NewExpression<UMaterialExpressionVectorParameter>();

	if (InputType == FunctionInput_Vector2)
	{
		if (const UMaterialExpressionConstant2Vector* PreviewValue = Cast<UMaterialExpressionConstant2Vector>(PreviewExpression))
		{
			VectorParam->DefaultValue.R = PreviewValue->R;
			VectorParam->DefaultValue.G = PreviewValue->G;
		}
	}
	else if (InputType == FunctionInput_Vector3)
	{
		if (const UMaterialExpressionConstant3Vector* PreviewValue = Cast<UMaterialExpressionConstant3Vector>(PreviewExpression))
		{
			VectorParam->DefaultValue = PreviewValue->Constant;
		}
	}

	VectorParam->Desc = CurrentInput.ExpressionInput->Description;
	VectorParam->ParameterName = ParameterName;
	VectorParam->Group = FName(GroupName);
	VectorParam->SortPriority = SortPriority;

	SetNodePosition(VectorParam, isUVsFunction ? EUVsParameter : EParameter);

	MaterialFunction->FunctionInputs[InputIndex].Input.Connect(0, VectorParam);
	ParamExpressionList.Add(VectorParam);
}

/** Vector parameters have no RGBA output: append their RGB and A outputs */
void FMMGLibrary::GenerateVector4Parameter(UMaterialExpressionMaterialFunctionCall* MaterialFunction, UMaterialExpression* PreviewExpression, const FFunctionExpressionInput& CurrentInput, const FString& GroupName, const bool isUVsFunction)
{
	UMaterialExpressionVectorParameter* VectorParam = NewExpression<UMaterialExpressionVectorParameter>();
	UMaterialExpressionAppendVector* AppendParam = NewExpression<UMaterialExpressionAppendVector>();

	if (const UMaterialExpressionConstant4Vector* PreviewValue = Cast<UMaterialExpressionConstant4Vector>(PreviewExpression))
	{
		VectorParam->DefaultValue = PreviewValue->Constant;
	}

	VectorParam->Desc = CurrentInput.ExpressionInput->Description;
	VectorParam->ParameterName = ParameterName;
	VectorParam->Group = FName(GroupName);
	VectorParam->SortPriority = SortPriority;

	SetNodePosition(VectorParam, isUVsFunction ? EUVsParameter : EParameter);

	MaterialFunction->FunctionInputs[InputIndex].Input.Connect(0, AppendParam);
	AppendParam->A.Connect(0, VectorParam);
	AppendParam->B.Connect(4, VectorParam);

	AppendParam->MaterialExpressionEditorX = VectorParam->MaterialExpressionEditorX + AppendParam->GetWidth() * 2;
	AppendParam->MaterialExpressionEditorY = VectorParam->MaterialExpressionEditorY + AppendParam->GetHeight() / 2;

	ParamExpressionList.Add(VectorParam);
}

void FMMGLibrary::GenerateTexture2DParameter(UMaterialExpressionMaterialFunctionCall* MaterialFunction, UMaterialExpression* PreviewExpression, const FFunctionExpressionInput& CurrentInput, const FString& GroupName, const bool isUVsFunction)
{
	UMaterialExpressionTextureObjectParameter* TextureObjectParam = NewExpression<UMaterialExpressionTextureObjectParameter>();

	if (const UMaterialExpressionTextureObject* PreviewValue = Cast<UMaterialExpressionTextureObject>(PreviewExpression))
	{
		TextureObjectParam->Texture = PreviewValue->Texture;
		TextureObjectParam->SamplerType = PreviewValue->SamplerType;
	}

	TextureObjectParam->Desc = CurrentInput.ExpressionInput->Description;
	TextureObjectParam->ParameterName = ParameterName;
	TextureObjectParam->Group = FName(GroupName);
	TextureObjectParam->SortPriority = SortPriority;

	SetNodePosition(TextureObjectParam, isUVsFunction ? EUVsParameter : EParameter);

	MaterialFunction->FunctionInputs[InputIndex].Input.Connect(0, TextureObjectParam);
	ParamExpressionList.Add(TextureObjectParam);
}

void FMMGLibrary::GenerateBoolParameter(UMaterialExpressionMaterialFunctionCall* MaterialFunction, UMaterialExpression* PreviewExpression, const FFunctionExpressionInput& CurrentInput, const FString& GroupName, const bool isUVsFunction)
{
	UMaterialExpressionStaticBoolParameter* BoolParam = NewExpression<UMaterialExpressionStaticBoolParameter>(true);

	if (const UMaterialExpressionStaticBool* PreviewValue = Cast<UMaterialExpressionStaticBool>(PreviewExpression))
	{
		BoolParam->DefaultValue = PreviewValue->Value;
	}

	BoolParam->Desc = CurrentInput.ExpressionInput->Description;
	BoolParam->ParameterName = ParameterName;
	BoolParam->Group = FName(GroupName);
	BoolParam->SortPriority = SortPriority;

	SetNodePosition(BoolParam, isUVsFunction ? EUVsParameter : EParameter, true);

	MaterialFunction->FunctionInputs[InputIndex].Input.Expression = BoolParam;
	ParamExpressionList.Add(BoolParam);
}

/** A channel of the packed texture: reroute usage, then a component mask keeping that channel */
void FMMGLibrary::SetCustomPackedRerouteUsage(UMaterialExpressionMaterialFunctionCall* MaterialFunction)
{
	PackedNamedRerouteUsagePtr = NewExpression<UMaterialExpressionNamedRerouteUsage>();
	UMaterialExpressionComponentMask* PackedMaskExpression = NewExpression<UMaterialExpressionComponentMask>();

	PackedNamedRerouteUsagePtr->Declaration = PackedNamedRerouteDeclarationPtr;

	PackedMaskExpression->R = CurrentPackedOutput == MMGPack::ChannelKeys[0];
	PackedMaskExpression->G = CurrentPackedOutput == MMGPack::ChannelKeys[1];
	PackedMaskExpression->B = CurrentPackedOutput == MMGPack::ChannelKeys[2];
	PackedMaskExpression->A = CurrentPackedOutput == MMGPack::ChannelKeys[3];

	PackedMaskExpression->Input.Connect(0, PackedNamedRerouteUsagePtr);
	MaterialFunction->FunctionInputs[InputIndex].Input.Connect(0, PackedMaskExpression);

	SetNodePosition(PackedNamedRerouteUsagePtr, EParameter, true);
	SetNodePosition(PackedMaskExpression, EPackedCompMask);
}

/** Initials of the packed channels, "Detail " skipped: Roughness, Metallic, AO -> "RMA" */
FString FMMGLibrary::GetCustomPackedName() const
{
	FString PackedName;
	for (const TPair<FString, FString>& Pair : TreeViewChildPtr->ChildFunctions[0]->CustomPackedSelectionSave)
	{
		if (FUTC_Utils::IsPackedChannelSet(Pair.Value))
		{
			FString ChannelName = Pair.Value;
			ChannelName.RemoveFromStart(TEXT("Detail "));
			PackedName += ChannelName.Left(1);
		}
	}
	return PackedName;
}

void FMMGLibrary::ConnectFunctionToMaterialAttribute()
{
	if (bSubstrate)
	{
		if (!ConnectFunctionToSubstrate())
		{
			KeepForSubstrateRoot();
		}
		return;
	}

	FExpressionInput* AttributeInput = nullptr;
	switch (FunctionDataStruct->MaterialAttributeType.GetValue())
	{
	case EMAttributes::BaseColor:			AttributeInput = &MaterialAttributePtr->BaseColor; break;
	case EMAttributes::Roughness:			AttributeInput = &MaterialAttributePtr->Roughness; break;
	case EMAttributes::Metallic:			AttributeInput = &MaterialAttributePtr->Metallic; break;
	case EMAttributes::Specular:			AttributeInput = &MaterialAttributePtr->Specular; break;
	case EMAttributes::EmissiveColor:		AttributeInput = &MaterialAttributePtr->EmissiveColor; break;
	case EMAttributes::Opacity:				AttributeInput = &MaterialAttributePtr->Opacity; break;
	case EMAttributes::OpacityMask:			AttributeInput = &MaterialAttributePtr->OpacityMask; break;
	case EMAttributes::Normal:				AttributeInput = &MaterialAttributePtr->Normal; break;
	case EMAttributes::WorldPositionOffset:	AttributeInput = &MaterialAttributePtr->WorldPositionOffset; break;
	case EMAttributes::SubsurfaceColor:		AttributeInput = &MaterialAttributePtr->SubsurfaceColor; break;
	case EMAttributes::AmbientOcclusion:	AttributeInput = &MaterialAttributePtr->AmbientOcclusion; break;
	case EMAttributes::PixelDepthOffset:	AttributeInput = &MaterialAttributePtr->PixelDepthOffset; break;
	default: break;
	}

	if (AttributeInput)
	{
		AttributeInput->Expression = FunctionNamedRerouteUsagePtr;
	}
}

/** Feeds each blend with its A and B functions; the function no mask uses drives the material */
void FMMGLibrary::ConnectRerouteOutput()
{
	RerouteDeclarationRest = RerouteDeclarationMap;
	for (const FMMGTreeViewPtr& TreeViewParent : TreeViewList)
	{
		if (*TreeViewParent->FunctionType != MMGNames::Mask)
		{
			continue;
		}

		/** A and B: the blend inputs, or the background and foreground of the Substrate blend */
		FExpressionInput* BlendInputs[2];
		EExpressionType UsagePositions[2];
		if (bSubstrate)
		{
			CurrentHorizontalBlend = HorizontalBlendMap.FindRef(*TreeViewParent->FunctionName);
			BlendInputs[0] = &CurrentHorizontalBlend->Background;
			BlendInputs[1] = &CurrentHorizontalBlend->Foreground;
			UsagePositions[0] = ERerouteUsageSubstrateBlendA;
			UsagePositions[1] = ERerouteUsageSubstrateBlendB;
		}
		else
		{
			CurrentBlendAttribute = BlendAttributeMap.FindRef(*TreeViewParent->FunctionName);
			BlendInputs[0] = &CurrentBlendAttribute->A;
			BlendInputs[1] = &CurrentBlendAttribute->B;
			UsagePositions[0] = ERerouteUsageBlendA;
			UsagePositions[1] = ERerouteUsageBlendB;
		}

		const TSharedPtr<FString> MaskItems[] = {TreeViewParent->ChildFunctions[0]->CurrentAComboItem, TreeViewParent->ChildFunctions[0]->CurrentBComboItem};
		for (int32 Input = 0; Input < UE_ARRAY_COUNT(MaskItems); ++Input)
		{
			if (!MaskItems[Input].IsValid())
			{
				continue;
			}

			const FString& ItemName = *MaskItems[Input];
			UMaterialExpressionNamedRerouteUsage* NamedRerouteUsage = NewExpression<UMaterialExpressionNamedRerouteUsage>();
			NamedRerouteUsage->Declaration = RerouteDeclarationMap.FindRef(ItemName);
			BlendInputs[Input]->Expression = NamedRerouteUsage;
			SetNodePosition(NamedRerouteUsage, UsagePositions[Input]);

			RerouteDeclarationRest.Remove(ItemName);
		}
	}

	TArray<UMaterialExpressionNamedRerouteDeclaration*> Rest;
	RerouteDeclarationRest.GenerateValueArray(Rest);
	if (Rest.IsEmpty())
	{
		return;
	}

	for (int32 Index = 1; Index < Rest.Num(); ++Index)
	{
		FUTC_Utils::SpawnNotification(FText::Format(LOCTEXT("NoConnection", "Warning: {0} has no connection."), FText::FromString(*RerouteDeclarationRest.FindKey(Rest[Index]))), true);
	}

	/** Rest usage */
	if (bAddToMaterial)
	{
		return;
	}

	UTCMaterial->EditorX = 2400;
	if (bSubstrate)
	{
		ConnectRootToSubstrate(Rest[0], *RerouteDeclarationRest.FindKey(Rest[0]));
		return;
	}

	UMaterialExpressionNamedRerouteUsage* RestNamedRerouteUsage = NewExpression<UMaterialExpressionNamedRerouteUsage>();
	RestNamedRerouteUsage->Declaration = Rest[0];
	UTCMaterial->GetEditorOnlyData()->MaterialAttributes.Expression = RestNamedRerouteUsage;
	RestNamedRerouteUsage->MaterialExpressionEditorX = UTCMaterial->EditorX - RestNamedRerouteUsage->GetWidth() * 2;
}

//--------------------------------------------------------------------------------------------------------------
/** Substrate */

/** The function's Slab; the helpers converting the legacy attributes are added when an attribute needs them */
void FMMGLibrary::GenerateSubstrateSlab()
{
	SlabPtr = NewExpression<UMaterialExpressionSubstrateSlabBSDF>();
	/** New slabs default to diffusion SSS; the legacy default lit has none */
	SlabPtr->SubSurfaceType = MSS_None;
	MetalnessPtr = nullptr;
	SubsurfaceMFPPtr = nullptr;
	CoveragePtr = nullptr;
	SubstrateChannelMasks.Reset();
}

/** Parameter blending mixes the slab inputs like Blend Material Attributes does, and keeps a single closure */
void FMMGLibrary::GenerateSubstrateHorizontalBlend()
{
	HorizontalBlendPtr = NewExpression<UMaterialExpressionSubstrateHorizontalMixing>();
	HorizontalBlendPtr->bUseParameterBlending = true;
	HorizontalBlendMap.Add(*TreeViewParentPtr->FunctionName, HorizontalBlendPtr);
}

/** Root pins Substrate keeps: the function's value goes to the root when the material is built, blended through the masks */
void FMMGLibrary::KeepForSubstrateRoot()
{
	static const TPair<EMAttributes, EMaterialProperty> RootPins[] = {
		{EMAttributes::OpacityMask, MP_OpacityMask},
		{EMAttributes::WorldPositionOffset, MP_WorldPositionOffset},
		{EMAttributes::AmbientOcclusion, MP_AmbientOcclusion},
		{EMAttributes::PixelDepthOffset, MP_PixelDepthOffset}};

	for (const TPair<EMAttributes, EMaterialProperty>& Pin : RootPins)
	{
		if (FunctionDataStruct->MaterialAttributeType.GetValue() == Pin.Key)
		{
			SubstrateRootProperties.AddUnique(Pin.Value);
			SubstrateRootValues.FindOrAdd(*TreeViewParentPtr->FunctionName).Add(Pin.Value, FunctionNamedRerouteUsagePtr);
		}
	}
}

/** Plugs the attribute into the Slab like the engine's legacy conversion does. False for the attributes Substrate leaves on the material root */
bool FMMGLibrary::ConnectFunctionToSubstrate()
{
	UMaterialExpression* Value = FunctionNamedRerouteUsagePtr;
	const EMAttributes Attribute = FunctionDataStruct->MaterialAttributeType.GetValue();

	switch (Attribute)
	{
	case EMAttributes::BaseColor:
	case EMAttributes::Metallic:
	case EMAttributes::Specular:
		if (!MetalnessPtr)
		{
			MetalnessPtr = NewExpression<UMaterialExpressionSubstrateMetalnessToDiffuseAlbedoF0>();
			SlabPtr->DiffuseAlbedo.Connect(0, MetalnessPtr);
			SlabPtr->F0.Connect(1, MetalnessPtr);
		}
		if (Attribute == EMAttributes::BaseColor)
		{
			MetalnessPtr->BaseColor.Expression = Value;
		}
		else
		{
			/** The legacy pins keep the first channel of a vector, the converter would use all three */
			UMaterialExpressionComponentMask* FirstChannel = NewExpression<UMaterialExpressionComponentMask>();
			FirstChannel->R = 1;
			FirstChannel->Input.Connect(0, Value);
			SubstrateChannelMasks.Add(FirstChannel, Value);
			(Attribute == EMAttributes::Metallic ? MetalnessPtr->Metallic : MetalnessPtr->Specular).Connect(0, FirstChannel);
		}
		return true;

	case EMAttributes::Roughness:
		SlabPtr->Roughness.Expression = Value;
		return true;

	case EMAttributes::Normal:
		SlabPtr->Normal.Expression = Value;
		return true;

	case EMAttributes::EmissiveColor:
		SlabPtr->EmissiveColor.Expression = Value;
		return true;

	/** Legacy subsurface: the color is a transmittance, lit with wrap lighting */
	case EMAttributes::SubsurfaceColor:
		if (!SubsurfaceMFPPtr)
		{
			SubsurfaceMFPPtr = NewExpression<UMaterialExpressionSubstrateTransmittanceToMFP>();
			SlabPtr->SSSMFP.Connect(0, SubsurfaceMFPPtr);
			SlabPtr->SubSurfaceType = MSS_Wrap;
		}
		SubsurfaceMFPPtr->TransmittanceColor.Expression = Value;
		return true;

	/** Substrate ignores the root opacity: it becomes the coverage of the slab */
	case EMAttributes::Opacity:
		if (!CoveragePtr)
		{
			CoveragePtr = NewExpression<UMaterialExpressionSubstrateWeight>();
			CoveragePtr->A.Connect(0, SlabPtr);
			MANamedRerouteDeclarationPtr->Input.Connect(0, CoveragePtr);
		}
		CoveragePtr->Weight.Expression = Value;
		return true;

	default:
		return false;
	}
}

/** The last function on the Front Material pin, and the root pins Substrate keeps from the functions that write them */
void FMMGLibrary::ConnectRootToSubstrate(UMaterialExpressionNamedRerouteDeclaration* RootDeclaration, const FString& RootFunction)
{
	UMaterialExpressionNamedRerouteUsage* FrontUsage = NewExpression<UMaterialExpressionNamedRerouteUsage>();
	FrontUsage->Declaration = RootDeclaration;
	FrontUsage->MaterialExpressionEditorX = UTCMaterial->EditorX - FrontUsage->GetWidth() * 2;
	UTCMaterial->GetEditorOnlyData()->FrontMaterial.Connect(0, FrontUsage);

	/** Root pins below, their blends through the masks on the left */
	RootLayoutY = 200;
	RootLayoutBounds = FBox2D(ForceInit);
	for (const EMaterialProperty Property : SubstrateRootProperties)
	{
		if (UMaterialExpression* Value = GetSubstrateRootValue(RootFunction, Property, 0))
		{
			UTCMaterial->GetExpressionInputForProperty(Property)->Connect(0, Value);
		}
	}

	if (RootLayoutBounds.bIsValid)
	{
		UMaterialExpressionComment* Comment = NewComment();
		Comment->Text = TEXT("Root Attributes");
		Comment->CommentColor = FLinearColor(0, 0, 0, 1);
		Comment->MaterialExpressionEditorX = RootLayoutBounds.Min.X - 50;
		Comment->MaterialExpressionEditorY = RootLayoutBounds.Min.Y - 75;
		Comment->SizeX = RootLayoutBounds.GetSize().X + 350;
		Comment->SizeY = RootLayoutBounds.GetSize().Y + 150;
	}
}

/**
 * Value of a root pin for a function: its own attribute, or for a mask the lerp of its two inputs like Blend Material Attributes did.
 * Null when no function down that branch writes the pin, which leaves the engine default.
 */
UMaterialExpression* FMMGLibrary::GetSubstrateRootValue(const FString& FunctionName, EMaterialProperty Property, int32 Depth)
{
	/** Masks naming each other in a loop */
	if (Depth > TreeViewList.Num())
	{
		return nullptr;
	}

	const int32 LeafX = UTCMaterial->EditorX - 650 - 350 * Depth;
	const TMap<EMaterialProperty, UMaterialExpressionNamedRerouteUsage*>* FunctionValues = SubstrateRootValues.Find(FunctionName);
	if (UMaterialExpressionNamedRerouteUsage* Usage = FunctionValues ? FunctionValues->FindRef(Property) : nullptr)
	{
		/** The function's own usage moves here the first time, then each use reads the declaration again */
		if (UsedSubstrateRootValues.Contains(Usage))
		{
			UMaterialExpressionNamedRerouteUsage* NewUsage = NewExpression<UMaterialExpressionNamedRerouteUsage>();
			NewUsage->Declaration = Usage->Declaration;
			Usage = NewUsage;
		}
		UsedSubstrateRootValues.Add(Usage);
		PlaceRootNode(Usage, LeafX);
		return Usage;
	}

	const FMMGTreeViewPtr* Mask = TreeViewList.FindByPredicate([&FunctionName](const FMMGTreeViewPtr& Item)
	{
		return *Item->FunctionName == FunctionName && *Item->FunctionType == MMGNames::Mask && !Item->ChildFunctions.IsEmpty();
	});
	if (!Mask)
	{
		return nullptr;
	}

	const FString CacheKey = FunctionName + TEXT("|") + FString::FromInt(Property);
	if (UMaterialExpression* const* Cached = SubstrateRootBlends.Find(CacheKey))
	{
		return *Cached;
	}

	const FMMGTreeViewPtr& MaskChild = (*Mask)->ChildFunctions[0];
	const int32 FirstChildY = RootLayoutY;
	UMaterialExpression* A = MaskChild->CurrentAComboItem.IsValid() ? GetSubstrateRootValue(*MaskChild->CurrentAComboItem, Property, Depth + 1) : nullptr;
	UMaterialExpression* B = MaskChild->CurrentBComboItem.IsValid() ? GetSubstrateRootValue(*MaskChild->CurrentBComboItem, Property, Depth + 1) : nullptr;
	if (!A && !B)
	{
		return nullptr;
	}

	const int32 ChildX = LeafX - 350;
	UMaterialExpressionLinearInterpolate* Blend = NewExpression<UMaterialExpressionLinearInterpolate>();
	Blend->A.Connect(0, A ? A : NewSubstrateRootDefault(Property, ChildX));
	Blend->B.Connect(0, B ? B : NewSubstrateRootDefault(Property, ChildX));

	UMaterialExpressionNamedRerouteUsage* Alpha = NewExpression<UMaterialExpressionNamedRerouteUsage>();
	Alpha->Declaration = MaskAlphaDeclarations.FindRef(FunctionName);
	PlaceRootNode(Alpha, ChildX);
	Blend->Alpha.Connect(0, Alpha);

	Blend->MaterialExpressionEditorX = LeafX;
	Blend->MaterialExpressionEditorY = FirstChildY;
	RootLayoutBounds += FVector2D(LeafX, FirstChildY);

	SubstrateRootBlends.Add(CacheKey, Blend);
	return Blend;
}

/** What the root pin reads when nothing is plugged: 1 for opacity mask and AO, 0 for the offsets */
UMaterialExpression* FMMGLibrary::NewSubstrateRootDefault(EMaterialProperty Property, int32 X)
{
	UMaterialExpression* Default = nullptr;
	if (Property == MP_WorldPositionOffset)
	{
		Default = NewExpression<UMaterialExpressionConstant3Vector>();
	}
	else
	{
		UMaterialExpressionConstant* Constant = NewExpression<UMaterialExpressionConstant>();
		Constant->R = (Property == MP_OpacityMask || Property == MP_AmbientOcclusion) ? 1.f : 0.f;
		Default = Constant;
	}
	PlaceRootNode(Default, X);
	return Default;
}

/** Next row of the root attributes area */
void FMMGLibrary::PlaceRootNode(UMaterialExpression* Expression, int32 X)
{
	Expression->MaterialExpressionEditorX = X;
	Expression->MaterialExpressionEditorY = RootLayoutY;
	RootLayoutBounds += FVector2D(X, RootLayoutY);
	RootLayoutY += 70;
}

/** Rough node height from its pin count: the editor only knows the real size once the node is drawn */
int32 FMMGLibrary::EstimateNodeHeight(UMaterialExpression* Expression)
{
	int32 NumInputs = 0;
	for (FExpressionInputIterator It{Expression}; It; ++It)
	{
		++NumInputs;
	}
	return 60 + 25 * FMath::Max(NumInputs, Expression->GetOutputs().Num());
}

/** Channel masks, helpers, Slab, coverage and the function's reroute, left to right from where the slab was placed */
void FMMGLibrary::SetSubstrateSlabPositions()
{
	constexpr int32 ColumnWidth = 400;
	const int32 BaseX = SlabPtr->MaterialExpressionEditorX;
	const int32 TopY = SlabPtr->MaterialExpressionEditorY;

	/** Next to the reroute usage they read */
	for (const TPair<UMaterialExpression*, UMaterialExpression*>& Mask : SubstrateChannelMasks)
	{
		Mask.Key->MaterialExpressionEditorX = BaseX;
		Mask.Key->MaterialExpressionEditorY = Mask.Value->MaterialExpressionEditorY;
	}

	const int32 HelperX = SubstrateChannelMasks.IsEmpty() ? BaseX : BaseX + 200;
	int32 HelperY = TopY;
	for (UMaterialExpression* Helper : {static_cast<UMaterialExpression*>(MetalnessPtr), static_cast<UMaterialExpression*>(SubsurfaceMFPPtr)})
	{
		if (Helper)
		{
			Helper->MaterialExpressionEditorX = HelperX;
			Helper->MaterialExpressionEditorY = HelperY;
			HelperY += EstimateNodeHeight(Helper) + 40;
		}
	}

	SlabPtr->MaterialExpressionEditorX = HelperX + ColumnWidth;
	SlabPtr->MaterialExpressionEditorY = TopY;

	UMaterialExpression* SubstrateOutput = SlabPtr;
	if (CoveragePtr)
	{
		CoveragePtr->MaterialExpressionEditorX = SlabPtr->MaterialExpressionEditorX + ColumnWidth;
		CoveragePtr->MaterialExpressionEditorY = TopY;
		SubstrateOutput = CoveragePtr;
	}
	MANamedRerouteDeclarationPtr->MaterialExpressionEditorX = SubstrateOutput->MaterialExpressionEditorX + ColumnWidth;
	MANamedRerouteDeclarationPtr->MaterialExpressionEditorY = TopY;

	/** The next function starts below these nodes */
	MaterialParameterPosition.Y = FMath::Max(MaterialParameterPosition.Y, static_cast<double>(TopY + EstimateNodeHeight(SlabPtr) + 100));
}

/** The horizontal blend is wider than the Blend Material Attributes it replaces; its mask reads from facing the Mix pin */
void FMMGLibrary::SetSubstrateBlendPositions()
{
	MANamedRerouteDeclarationPtr->MaterialExpressionEditorX = HorizontalBlendPtr->MaterialExpressionEditorX + 400;
	MANamedRerouteDeclarationPtr->MaterialExpressionEditorY = HorizontalBlendPtr->MaterialExpressionEditorY;

	for (UMaterialExpression* MaskUsage : NamedRerouteUsageList)
	{
		MaskUsage->MaterialExpressionEditorX = HorizontalBlendPtr->MaterialExpressionEditorX - 220;
		MaskUsage->MaterialExpressionEditorY = HorizontalBlendPtr->MaterialExpressionEditorY + 90;
	}

	MaterialParameterPosition.Y = FMath::Max(MaterialParameterPosition.Y, static_cast<double>(HorizontalBlendPtr->MaterialExpressionEditorY + EstimateNodeHeight(HorizontalBlendPtr) + 100));
}

void FMMGLibrary::SetNodePosition(UMaterialExpression* Expression, EExpressionType ExpressionType, bool ReduceNodeSize)
{
	const bool bMaterialAttribute = (*TreeViewParentPtr->FunctionType == MMGNames::MaterialAttribute);
	const bool bMask = (*TreeViewParentPtr->FunctionType == MMGNames::Mask);

	switch (ExpressionType)
	{
	case EMaterialAttribute:
		{
			const int32 MinY = MFExpressionList[0]->MaterialExpressionEditorY;
			const int32 MaxY = MFExpressionList.Last()->MaterialExpressionEditorY;
			Expression->MaterialExpressionEditorX = 1550;
			Expression->MaterialExpressionEditorY = ((MinY + MaxY) / 2) - Expression->GetHeight();
			break;
		}

	case EMaterialFunction:
		{
			const int32 MinY = ParamExpressionList[0]->MaterialExpressionEditorY;
			const int32 MaxY = ParamExpressionList.Last()->MaterialExpressionEditorY;
			Expression->MaterialExpressionEditorX = MaterialParameterPosition.X + ParamExpressionList.Last()->GetWidth() * 4 + 100;
			Expression->MaterialExpressionEditorY = (MinY + MaxY) / 2;
			break;
		}

	case EUVsFunction:
		{
			const int32 MinY = ParamExpressionList[0]->MaterialExpressionEditorY;
			const int32 MaxY = ParamExpressionList.Last()->MaterialExpressionEditorY;
			Expression->MaterialExpressionEditorX = ParamExpressionList.Last()->MaterialExpressionEditorX + ParamExpressionList.Last()->GetWidth() * 3.5;
			Expression->MaterialExpressionEditorY = (MinY + MaxY) / 2;
			break;
		}

	case ERerouteDeclarationFunction:
		Expression->MaterialExpressionEditorX = MaterialFunctionPtr->MaterialExpressionEditorX + MaterialFunctionPtr->GetWidth() * 2.5;
		Expression->MaterialExpressionEditorY = MaterialFunctionPtr->MaterialExpressionEditorY;
		break;

	case ERerouteUsageFunction:
		if (bMaterialAttribute)
		{
			const int32 RerouteIndex = NamedRerouteUsageList.Find(Expression);
			Expression->MaterialExpressionEditorX = FunctionRootPtr->MaterialExpressionEditorX - Expression->GetWidth() * 2;
			Expression->MaterialExpressionEditorY = FunctionRootPtr->MaterialExpressionEditorY + Expression->GetHeight() / 2 * RerouteIndex;
		}
		else if (bMask)
		{
			Expression->MaterialExpressionEditorX = FunctionRootPtr->MaterialExpressionEditorX - Expression->GetWidth() * 3;
			Expression->MaterialExpressionEditorY = FunctionRootPtr->MaterialExpressionEditorY + Expression->GetHeight();
		}
		break;

	case ERerouteDeclarationMA:
		Expression->MaterialExpressionEditorX = FunctionRootPtr->MaterialExpressionEditorX + FunctionRootPtr->GetWidth() + 100;
		Expression->MaterialExpressionEditorY = FunctionRootPtr->MaterialExpressionEditorY;
		break;

	case ERerouteUsageBlendA:
		Expression->MaterialExpressionEditorX = CurrentBlendAttribute->MaterialExpressionEditorX - Expression->GetWidth() * 1.5;
		Expression->MaterialExpressionEditorY = CurrentBlendAttribute->MaterialExpressionEditorY - Expression->GetHeight() / 2;
		break;

	case ERerouteUsageBlendB:
		Expression->MaterialExpressionEditorX = CurrentBlendAttribute->MaterialExpressionEditorX - Expression->GetWidth() * 1.5;
		Expression->MaterialExpressionEditorY = CurrentBlendAttribute->MaterialExpressionEditorY + Expression->GetHeight() / 4;
		break;

	case EParameter:
		Expression->MaterialExpressionEditorX = MaterialParameterPosition.X;
		Expression->MaterialExpressionEditorY = MaterialParameterPosition.Y;
		MaterialParameterPosition.Y += ReduceNodeSize ? (Expression->GetHeight() / 2) * 1.5 : Expression->GetHeight() * 1.5;
		break;

	case EUVsParameter:
		if (bMaterialAttribute)
		{
			Expression->MaterialExpressionEditorX = FunctionRootPtr->MaterialExpressionEditorX * -1 / 1.25 - 250;
		}
		else if (bMask)
		{
			Expression->MaterialExpressionEditorX = FunctionRootPtr->MaterialExpressionEditorX * -1 / 1.25;
		}

		UVsParameterPosition.X = Expression->MaterialExpressionEditorX;
		Expression->MaterialExpressionEditorY = UVsParameterPosition.Y;
		UVsParameterPosition.Y += ReduceNodeSize ? (Expression->GetHeight() / 2) * 1.5 : Expression->GetHeight() * 1.5;
		break;

	case EPackedTexture:
		{
			const int32 MinY = CustomPackedMFList[0]->MaterialExpressionEditorY;
			const int32 MaxY = CustomPackedMFList.Last()->MaterialExpressionEditorY;
			Expression->MaterialExpressionEditorX = MaterialParameterPosition.X + Expression->GetWidth() * -6;
			Expression->MaterialExpressionEditorY = (MinY + MaxY) / 2;
			break;
		}

	case EPackedRerouteDeclaration:
		Expression->MaterialExpressionEditorX = PackedTextureParamPtr->MaterialExpressionEditorX + PackedTextureParamPtr->GetWidth() * 3;
		Expression->MaterialExpressionEditorY = PackedTextureParamPtr->MaterialExpressionEditorY + PackedTextureParamPtr->GetHeight();
		break;

	case EPackedCompMask:
		Expression->MaterialExpressionEditorX = PackedNamedRerouteUsagePtr->MaterialExpressionEditorX + PackedNamedRerouteUsagePtr->GetWidth() * 2;
		Expression->MaterialExpressionEditorY = PackedNamedRerouteUsagePtr->MaterialExpressionEditorY;
		break;

	case EComment:
		{
			/** Frames the parameters of the function up to its reroute declaration */
			UMaterialExpression* FirstParameter = ParamExpressionList[0];
			UMaterialExpression* LastParameter = ParamExpressionList.Last();
			Expression->MaterialExpressionEditorX = FirstParameter->MaterialExpressionEditorX - 50;
			Expression->MaterialExpressionEditorY = FirstParameter->MaterialExpressionEditorY - 50;

			const bool bSmallLastParameter = Cast<UMaterialExpressionScalarParameter>(LastParameter) || Cast<UMaterialExpressionStaticBoolParameter>(LastParameter);
			CommentPtr->SizeX = FunctionNamedRerouteDeclarationPtr->MaterialExpressionEditorX - LastParameter->MaterialExpressionEditorX + FunctionNamedRerouteDeclarationPtr->GetWidth() * 3;
			CommentPtr->SizeY = LastParameter->MaterialExpressionEditorY - FirstParameter->MaterialExpressionEditorY + LastParameter->GetHeight() * (bSmallLastParameter ? 1 : 2);

			if (CommentYPositionDoOnce)
			{
				FirstCommentYPosition = CommentPtr->MaterialExpressionEditorY;
				CommentYPositionDoOnce = false;
			}
			break;
		}

	case EMainComment:
		/** Frames the whole function, from its UVs or packed texture to its reroute declaration */
		if (HasUVs() && HasToGenerateUVsFunction() && isUVsFunctionConnected)
		{
			Expression->MaterialExpressionEditorX = UVsParameterPosition.X - 50;
		}
		else if (IsValid(PackedTextureParamPtr))
		{
			Expression->MaterialExpressionEditorX = PackedTextureParamPtr->MaterialExpressionEditorX - 50;
		}
		else
		{
			Expression->MaterialExpressionEditorX = MaterialParameterPosition.X - 100;
		}

		Expression->MaterialExpressionEditorY = FirstCommentYPosition - 75;

		{
			/** A Substrate slab or blend is taller than its function's parameters can be */
			int32 BottomY = CommentPtr->MaterialExpressionEditorY + CommentPtr->SizeY;
			if (bSubstrate)
			{
				BottomY = FMath::Max(BottomY, FunctionRootPtr->MaterialExpressionEditorY + EstimateNodeHeight(FunctionRootPtr));
			}

			MainCommentPtr->SizeX = FMath::Abs(MANamedRerouteDeclarationPtr->MaterialExpressionEditorX - Expression->MaterialExpressionEditorX + MANamedRerouteDeclarationPtr->GetWidth() * 2.5);
			MainCommentPtr->SizeY = FMath::Abs(BottomY - Expression->MaterialExpressionEditorY) + 25;
		}
		break;

	/** Facing the Background and Foreground pins; the mask usage faces the Mix pin below them */
	case ERerouteUsageSubstrateBlendA:
		Expression->MaterialExpressionEditorX = CurrentHorizontalBlend->MaterialExpressionEditorX - 220;
		Expression->MaterialExpressionEditorY = CurrentHorizontalBlend->MaterialExpressionEditorY - 30;
		break;

	case ERerouteUsageSubstrateBlendB:
		Expression->MaterialExpressionEditorX = CurrentHorizontalBlend->MaterialExpressionEditorX - 220;
		Expression->MaterialExpressionEditorY = CurrentHorizontalBlend->MaterialExpressionEditorY + 30;
		break;
	}
}

/** Opens the material editor and returns its graph editor, found in the window titled after the material */
SGraphEditor* FMMGLibrary::FindGraphEditorByMaterial(UMaterial* TargetMaterial)
{
	GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->OpenEditorForAsset(TargetMaterial);

	TArray<TSharedRef<SWindow>> Windows;
	FSlateApplication::Get().GetAllVisibleWindowsOrdered(Windows);

	TArray<SWidget*> Stack;
	for (const TSharedRef<SWindow>& Window : Windows)
	{
		if (Window->ToString().Contains(TargetMaterial->GetName()))
		{
			Stack.Push(&Window.Get());
		}
	}

	while (Stack.Num() > 0)
	{
		SWidget* Widget = Stack.Pop();
		FChildren* Children = Widget->GetChildren();
		for (int32 ChildIndex = 0; ChildIndex < Children->Num(); ChildIndex++)
		{
			SWidget& ChildWidget = Children->GetChildAt(ChildIndex).Get();
			if (ChildWidget.GetType() == TEXT("SGraphEditor"))
			{
				return static_cast<SGraphEditor*>(&ChildWidget);
			}
			Stack.Push(&ChildWidget);
		}
	}
	return nullptr;
}

void FMMGLibrary::ReinitVar()
{
	MaterialParameterPosition = FVector2D(0, 0);
	UVsParameterPosition = FVector2D(0, 0);

	ParamExpressionList.Empty();
	MFExpressionList.Empty();
	NamedRerouteUsageList.Empty();
	RerouteDeclarationRest.Empty();

	RerouteDeclarationMap.Empty();
	BlendAttributeMap.Empty();
	RerouteUsageMap.Empty();

	bAddToMaterial = false;

	CustomPackedMFList.Empty();

	bSubstrate = false;
	HorizontalBlendMap.Empty();
	MaskAlphaDeclarations.Empty();
	SubstrateRootProperties.Empty();
	SubstrateRootValues.Empty();
	UsedSubstrateRootValues.Empty();
	SubstrateRootBlends.Empty();
}

bool FMMGLibrary::ErrorDetector()
{
	auto Fail = [](const FText& Message)
	{
		FUTC_Utils::SpawnNotification(Message);
		return false;
	};

	if (bAddToMaterial)
	{
		if (!IsValid(UTCMaterial))
		{
			return Fail(LOCTEXT("NoTarget", "Need to select a Target Material"));
		}
	}
	else
	{
		if (Settings.MaterialName.IsEmpty())
		{
			return Fail(LOCTEXT("NoName", "Set a material name."));
		}
		if (UEditorAssetLibrary::DoesAssetExist(PackageName))
		{
			return Fail(FText::Format(LOCTEXT("AlreadyExists", "{0} already exist."), FText::FromString(Settings.MaterialName)));
		}
	}

	if (TreeViewList.IsEmpty())
	{
		return Fail(LOCTEXT("NoFunction", "No valid function"));
	}

	TArray<FString> FunctionNameListCheck;
	for (const FMMGTreeViewPtr& TreeViewParent : TreeViewList)
	{
		const FText FunctionName = FText::FromString(*TreeViewParent->FunctionName);

		if (FunctionNameListCheck.Contains(*TreeViewParent->FunctionName))
		{
			return Fail(FText::Format(LOCTEXT("DuplicateName", "Function name '{0}', is assigned several times."), FunctionName));
		}
		FunctionNameListCheck.Add(*TreeViewParent->FunctionName);

		if (TreeViewParent->ChildFunctions.IsEmpty())
		{
			return Fail(FText::Format(LOCTEXT("EmptyFunction", "Function: {0}, is empty."), FunctionName));
		}

		/** A material attribute can only be fed once per function */
		TArray<FString> FunctionChildListCheck;
		auto CheckAttributeInput = [this, &FunctionChildListCheck](const FString& ChildLabel)
		{
			const FMMGMaterialFunctionStruct* DTStruct = MMGAssets::GetMaterialOutputs()->FindRow<FMMGMaterialFunctionStruct>(FName(FUTC_Utils::TextToPascal(ChildLabel)), TEXT(""));
			const FString AttributeInput = UEnum::GetValueAsString(DTStruct->MaterialAttributeType);
			if (FunctionChildListCheck.Contains(AttributeInput))
			{
				return AttributeInput;
			}
			FunctionChildListCheck.Add(AttributeInput);
			return FString();
		};

		for (const FMMGTreeViewPtr& TreeViewChild : TreeViewParent->ChildFunctions)
		{
			if (!TreeViewChild->CurrentChildComboItem.IsValid())
			{
				return Fail(FText::Format(LOCTEXT("UnassignedChild", "Function: {0}, has an unassigned child."), FunctionName));
			}

			if (*TreeViewChild->CurrentChildComboItem != MMGNames::PackedTexture && *TreeViewParent->FunctionType == MMGNames::MaterialAttribute)
			{
				const FString UsedAttribute = CheckAttributeInput(*TreeViewChild->CurrentChildComboItem);
				if (!UsedAttribute.IsEmpty())
				{
					return Fail(FText::Format(LOCTEXT("SameAttribute", "Function: {0}, use several times the same attribute input '{1}'."), FunctionName, FText::FromString(UsedAttribute)));
				}
			}

			/** New materials need both mask inputs, an existing material may already hold them */
			if (*TreeViewParent->FunctionType == MMGNames::Mask && !bAddToMaterial)
			{
				if (!TreeViewChild->CurrentAComboItem.IsValid() || !TreeViewChild->CurrentBComboItem.IsValid())
				{
					return Fail(FText::Format(LOCTEXT("UnassignedInput", "Function: {0}, has an unassigned input."), FunctionName));
				}
			}

			if (!TreeViewChild->ChildFunctions.IsEmpty() && TreeViewChild->ChildFunctions[0]->IsCustomPack)
			{
				bool isEmpty = true;
				for (const TPair<FString, FString>& Pair : TreeViewChild->ChildFunctions[0]->CustomPackedSelectionSave)
				{
					if (FUTC_Utils::IsPackedChannelSet(Pair.Value))
					{
						const FString UsedAttribute = CheckAttributeInput(Pair.Value);
						if (!UsedAttribute.IsEmpty())
						{
							return Fail(FText::Format(LOCTEXT("PackedSameAttribute", "Custom Packed Function in: {0}, use several times the same attribute input '{1}'."), FunctionName, FText::FromString(UsedAttribute)));
						}
						isEmpty = false;
					}
				}

				if (isEmpty)
				{
					return Fail(FText::Format(LOCTEXT("EmptyPack", "Custom packed texture function in {0} is empty."), FunctionName));
				}
			}
		}
	}
	return true;
}

#undef LOCTEXT_NAMESPACE
