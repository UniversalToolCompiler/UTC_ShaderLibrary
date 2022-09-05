/** Copyright 2022, Universal Tool Compiler */
#include "MMGLibrary.h"

#include "AssetViewUtils.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Factories/MaterialFactoryNew.h"
#include "MaterialExpressionIO.h"
#include "MaterialGraph/MaterialGraph.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "EditorAssetLibrary.h"
#include "MaterialEditingLibrary.h"
#include "ShaderCompiler.h"
#include "Engine/AssetManager.h"
#include "Subsystems/AssetEditorSubsystem.h"
#include "Materials/MaterialInstanceConstant.h"

#include "Factories/MaterialInstanceConstantFactoryNew.h"
#include "Framework/Notifications/NotificationManager.h"
#include "MaterialGraph/MaterialGraphNode_Composite.h"
#include "Materials/MaterialExpressionAppendVector.h"
#include "Materials/MaterialExpressionConstant.h"
#include "Materials/MaterialExpressionConstant2Vector.h"
#include "Materials/MaterialExpressionConstant3Vector.h"
#include "Materials/MaterialExpressionConstant4Vector.h"
#include "Materials/MaterialExpressionVectorParameter.h"
#include "Materials/MaterialExpressionScalarParameter.h"
#include "Materials/MaterialExpressionTextureSample.h"
#include "Materials/MaterialExpressionTextureSampleParameter.h"
#include "Materials/MaterialExpressionTextureSampleParameter2D.h"
#include "Materials/MaterialExpressionComment.h"
#include "Materials/MaterialExpressionMakeMaterialAttributes.h"
#include "Materials/MaterialExpressionStaticBool.h"
#include "Materials/MaterialExpressionStaticBoolParameter.h"
#include "Materials/MaterialExpressionTextureObject.h"
#include "Materials/MaterialExpressionTextureObjectParameter.h"
#include "Misc/ScopedSlowTask.h"

#define LOCTEXT_NAMESPACE "MMGLibrary"

void FMMGLibrary::GenerateMaterial(const FMMGGenerateMaterialStruct MainSettings, const TArray<FMMGTreeViewPtr> TreeViewParameters, const FMMGMaterialSettingsStruct MaterialSettings)
{
	bAddToMaterial = false;
	
	TreeViewList = TreeViewParameters;
	Settings = MainSettings;
	
	/** Get material infos */
	PackageName = Settings.materialDestination.Path;
	if(PackageName.IsEmpty())
		PackageName = "/Game/";
	else
		PackageName += "/";
	
	PackageName += Settings.materialName;
	
	if(!ErrorDetector())
		return;
	
	UPackage* Package = CreatePackage(*PackageName);
	
	/** Generate the main material */
	auto MaterialFactory = NewObject<UMaterialFactoryNew>();
	UTCMaterial = (UMaterial*)MaterialFactory->FactoryCreateNew(UMaterial::StaticClass(), Package, *Settings.materialName, RF_Standalone | RF_Public, NULL, GWarn);
	FAssetRegistryModule::AssetCreated(UTCMaterial);
	
	Package->FullyLoad();
	Package->SetDirtyFlag(true);

	/**Material Settings*/ 
	UTCMaterial->bUseMaterialAttributes = 1;
	UTCMaterial->MaterialDomain = MaterialSettings.MaterialDomain;
	UTCMaterial->BlendMode = MaterialSettings.BlendMode;
	UTCMaterial->SetShadingModel(MaterialSettings.ShadingModel);
	UTCMaterial->TwoSided = MaterialSettings.TwoSided;
	
	InitNodesGen();
	
	UTCMaterial->PreEditChange(NULL);
	UTCMaterial->PostEditChange();
	
	FGlobalComponentReregisterContext RecreateComponents;

	/** Generate material instance */
	if (Settings.GenMaterialInst)
	{
		GenerateMaterialInstance();
	}

	ReinitVar();
}

void FMMGLibrary::GenerateMaterialInstance()
{
	/** Get material infos */
	PackageName = Settings.materialDestination.Path;
	if(PackageName.IsEmpty())
		PackageName = "/Game/";
	else
		PackageName += "/";

	FString InstanceName = Settings.materialName;
	FString InstancePrefix = "MI_";
		
	if(Settings.materialName.StartsWith("MM_"))
	{
		InstanceName.RemoveFromStart("MM_");
		InstanceName = InstancePrefix += InstanceName;
	}
	else
		InstanceName = InstancePrefix += InstanceName;
			
	PackageName += InstanceName;
	UPackage* InstancePackage = CreatePackage(*PackageName);
	
	auto InstanceFactory = NewObject<UMaterialInstanceConstantFactoryNew>();
	UMaterialInstanceConstant* UTCInstance = (UMaterialInstanceConstant*)InstanceFactory->FactoryCreateNew(UMaterialInstanceConstant::StaticClass(),InstancePackage, *InstanceName,RF_Standalone | RF_Public, NULL, GWarn );
	FAssetRegistryModule::AssetCreated(UTCInstance);
	
	InstancePackage->FullyLoad();
	InstancePackage->SetDirtyFlag(true);
	
	UTCInstance->Parent = UTCMaterial;

	UTCInstance->PreEditChange(NULL);
	UTCInstance->PostEditChange();
}

void FMMGLibrary::AddToMaterial(const FMMGAddToMaterialStruct MainSettings, const TArray<FMMGTreeViewPtr> TreeViewParameters)
{
	bAddToMaterial = true;
	
	TreeViewList = TreeViewParameters;
	UTCMaterial = MainSettings.TargetMaterial;
	
	if(!ErrorDetector())
	{
		bAddToMaterial = false;
		return;
	}
	
	UPackage* Package = UTCMaterial->GetPackage();

	Package->FullyLoad();
	Package->SetDirtyFlag(true);
	
	/**Get Material Editor */ 
	SGraphEditor* GraphWidget = FindGraphEditorByMaterial(UTCMaterial);
	EdGraph = GraphWidget->GetCurrentGraph();
	MatGraph = static_cast<UMaterialGraph*>(EdGraph);
	
	IAssetEditorInstance* AssetEditorInstance = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->FindEditorForAsset(UTCMaterial, false);
	MaterialEd = static_cast<IMaterialEditor*>(AssetEditorInstance);

	/**Get Start Position*/ 
	int32 MaxNodeY = MaterialParameterPosition.Y;
	for (auto Node : EdGraph->Nodes)
	{
		if(Node->NodePosY > MaxNodeY)
			MaxNodeY = Node->NodePosY;
	}
	
	if(UTCMaterial->EditorY >= MaxNodeY)
		MaxNodeY = UTCMaterial->EditorY;
	
	MaxNodeY += 750;
	MaterialParameterPosition.Y = MaxNodeY;

	/**Start Nodes Generation */ 
	InitNodesGen();
	
	MatGraph->RebuildGraph();
	//MaterialEd->UpdateMaterialAfterGraphChange();
	//UTCMaterial->UpdateCachedExpressionData();
	//UTCMaterial->UpdateMaterialShaderCacheAndTextureReferences();

	if(IsValid(JumpToThisExpression))
		MaterialEd->JumpToExpression(JumpToThisExpression);
	
	UTCMaterial->PreEditChange(NULL);
	UTCMaterial->PostEditChange();
		
	
	ReinitVar();
}

void FMMGLibrary::InitNodesGen()
{
	FScopedSlowTask AddToMaterialTask(TreeViewList.Num(), LOCTEXT("AddToMaterialNewNodes", "Add to Material new nodes"));
	AddToMaterialTask.MakeDialog();
	
	for (FMMGTreeViewPtr TreeViewParent : TreeViewList)
	{

		/** Loading */
		FText CurrentFunction = FText::AsCultureInvariant(FString("Add to Material new nodes: ") + *TreeViewParent->FunctionName);
		AddToMaterialTask.EnterProgressFrame();
		AddToMaterialTask.FrameMessage = CurrentFunction ;
		
		TreeViewParentPtr = TreeViewParent;
		
		if(*TreeViewParentPtr->FunctionType == TreeViewWidget.MAType)
			GenerateMaterialAttributeFunction();

		if(*TreeViewParentPtr->FunctionType == TreeViewWidget.MaskType)
			GenerateMaskFunction();
	}
	ConnectRerouteOutput();
}

void FMMGLibrary::GenerateMaterialAttributeFunction()
{
	if(bAddToMaterial)
	{
		/** Material attribute */
		UMaterialExpression* MaterialAttributeExpression = MaterialEd->CreateNewMaterialExpression(UMaterialExpressionMakeMaterialAttributes::StaticClass(), FVector2D(0,0), false, false, EdGraph);
		MaterialAttributePtr = Cast<UMaterialExpressionMakeMaterialAttributes>(MaterialAttributeExpression);

		if(TreeViewList[0] == TreeViewParentPtr)
			JumpToThisExpression = MaterialAttributeExpression;
		
		/** Material attribute reroute named node
		 * Declaration
		 */
		UMaterialExpression* NRDExpression = MaterialEd->CreateNewMaterialExpression(UMaterialExpressionNamedRerouteDeclaration::StaticClass(), FVector2D(0,0), false, false, EdGraph);
		MANamedRerouteDeclarationPtr = Cast<UMaterialExpressionNamedRerouteDeclaration>(NRDExpression);
	}
	else
	{
		/** Material attribute */
		MaterialAttributePtr = NewObject<UMaterialExpressionMakeMaterialAttributes>(UTCMaterial);
		UTCMaterial->Expressions.Add(MaterialAttributePtr);

		/** Material attribute reroute named node
		 * Declaration
		 */
		MANamedRerouteDeclarationPtr = NewObject<UMaterialExpressionNamedRerouteDeclaration>(UTCMaterial);
		UTCMaterial->Expressions.Add(MANamedRerouteDeclarationPtr);
	}

	MANamedRerouteDeclarationPtr->Name = FName(*TreeViewParentPtr->FunctionName);
	MANamedRerouteDeclarationPtr->Input.Expression = MaterialAttributePtr;
	
	RerouteDeclarationMap.Add(*TreeViewParentPtr->FunctionName, MANamedRerouteDeclarationPtr);
	
	/** Generate UVs function */
	if(TreeViewParentPtr->CurrentUVsComboItem.IsValid() && *TreeViewParentPtr->CurrentUVsComboItem != TreeViewWidget.NoUV && HasToGenerateUVsFunction())
		GenerateUVsFunction();

	FunctionIndex = 0;
	for(TSharedPtr<FMMGTreeView> TreeViewChild : TreeViewParentPtr->ChildrenFunction)
	{
		TreeViewChildPtr = TreeViewChild;

		/** Get material function by ComboBox selection */
		FName RowName = FName(Utils->TextToPascal(*TreeViewChildPtr->CurrentChildComboItem));
		FunctionDataStruct = MaterialOutputDT->FindRow<FMMGMaterialFunctionStruct>(RowName, "");

		GenerateMaterialFunctionNode(RowName.ToString());
		ConnectFunctionToMaterialAttribute();
			
		/** Refresh */
		UTCMaterial->PreEditChange(NULL);
		UTCMaterial->PostEditChange();
		
		/** Get inputs' type */
		InputIndex = 0;
		TArray<FFunctionExpressionInput> InputsList = MaterialFunctionPtr->FunctionInputs;
		for (FFunctionExpressionInput input : InputsList)
		{
			/** Parameter name */
			if(TreeViewList.Num() > 1 || bAddToMaterial)
				ParameterName = FName (*TreeViewParentPtr->FunctionName + FString(" - ") + RowName.ToString() + FString(" - ") + input.ExpressionInput->InputName.ToString());
			else
				ParameterName = FName (RowName.ToString() + FString(" - ") + input.ExpressionInput->InputName.ToString());

			/** Generate parameter */
			GenerateParameterByMFInputType(MaterialFunctionPtr, input);
			++InputIndex;
		}

		MaterialParameterPosition.Y += 100;
		
		/** function location */
		SetNodePosition(MaterialFunctionPtr, EMaterialFunction);
		/** Function reroute declaration location */
		SetNodePosition(FunctionNamedRerouteDeclarationPtr, ERerouteDeclarationFunction);
		/** Function reroute usage location */
		SetNodePosition(FunctionNamedRerouteUsagePtr, ERerouteUsageFunction);

		/** Comment node */
		if(bAddToMaterial)
		{
			CommentPtr = MaterialEd->CreateNewMaterialExpressionComment(FVector2D(0,0), EdGraph);
		}
		else
		{
			CommentPtr = NewObject<UMaterialExpressionComment>(UTCMaterial);
			UTCMaterial->Expressions.Add(CommentPtr);
		}

		CommentPtr->CommentColor = FLinearColor(0.019,0.076,0.13,1);
		CommentPtr->Text = RowName.ToString();

		/** Comment location */
		SetNodePosition(CommentPtr, EComment);
		
		ParamExpressionList.Empty();
		++FunctionIndex;
	}
	
	/** MA location */
	SetNodePosition(MaterialAttributePtr, EMaterialAttribute);
	/** MA reroute declaration location */
	SetNodePosition(MANamedRerouteDeclarationPtr, ERerouteDeclarationMA);

	/** Generate UVs parameters */
	if(TreeViewParentPtr->CurrentUVsComboItem.IsValid() && *TreeViewParentPtr->CurrentUVsComboItem != TreeViewWidget.NoUV && HasToGenerateUVsFunction())
	{
		if(isUVsFunctionConnected)
		{
			FName RowName = FName(Utils->TextToPascal(*TreeViewParentPtr->CurrentUVsComboItem));
			FunctionDataStruct = UVsDT->FindRow<FMMGMaterialFunctionStruct>(RowName, "");

			UVsParameterPosition.Y = MaterialAttributePtr->MaterialExpressionEditorY;
			GenerateUVsParameters();
			
			SetNodePosition(UVsFunctionPtr, EUVsFunction);
			
			ParamExpressionList.Empty();
		}
		else
			UTCMaterial->Expressions.Remove(UVsFunctionPtr);
	}
	
	/** Reorder reroute usafe to fit with MA */
	for (EMAttributes Enum : TEnumRange<EMAttributes>())
	{
		if(RerouteUsageMap.Contains(Enum))
			NamedRerouteUsageList.Add(*RerouteUsageMap.Find(Enum)); 
	}
	
	/** Reroute usage location */
	for (UMaterialExpression* RerouteNode : NamedRerouteUsageList)
	{
		SetNodePosition(RerouteNode, ERerouteUsageFunction);
	}
	
	/** Main comment node */
	if(bAddToMaterial)
	{
		MainCommentPtr = MaterialEd->CreateNewMaterialExpressionComment(FVector2D(0,0), EdGraph);
	}
	else
	{
		MainCommentPtr = NewObject<UMaterialExpressionComment>(UTCMaterial);
		UTCMaterial->Expressions.Add(MainCommentPtr);
	}
	
	MainCommentPtr->CommentColor = FLinearColor(0,0,0,1);
	MainCommentPtr->Text = *TreeViewParentPtr->FunctionName;
		
	/** Comment location */
	SetNodePosition(MainCommentPtr, EMainComment);
	
	RerouteUsageMap.Empty();
	NamedRerouteUsageList.Empty();
	MFExpressionList.Empty();
	CommentYPositionDoOnce = true;
	isUVsFunctionConnected = false;
	
	MaterialParameterPosition.Y += 200;
}

void FMMGLibrary::GenerateMaskFunction()
{
	if(bAddToMaterial)
	{
		/** Blend attribute */
		UMaterialExpression* BlendAttributeExpression = MaterialEd->CreateNewMaterialExpression(UMaterialExpressionBlendMaterialAttributes::StaticClass(), FVector2D(0,0), false, false, EdGraph);
		BlendAttributesPtr = Cast<UMaterialExpressionBlendMaterialAttributes>(BlendAttributeExpression);

		if(TreeViewList[0] == TreeViewParentPtr)
			JumpToThisExpression = BlendAttributeExpression;

		/** Blend attribute reroute named node
		 * Declaration
		 */
		UMaterialExpression* NRDExpression = MaterialEd->CreateNewMaterialExpression(UMaterialExpressionNamedRerouteDeclaration::StaticClass(), FVector2D(0,0), false, false, EdGraph);
		MANamedRerouteDeclarationPtr = Cast<UMaterialExpressionNamedRerouteDeclaration>(NRDExpression);
	}
	else
	{
		/** Blend attribute */
		BlendAttributesPtr = NewObject<UMaterialExpressionBlendMaterialAttributes>(UTCMaterial);
		UTCMaterial->Expressions.Add(BlendAttributesPtr);

		/** Blend attribute reroute named node
		* Declaration
		*/
		MANamedRerouteDeclarationPtr = NewObject<UMaterialExpressionNamedRerouteDeclaration>(UTCMaterial);
		UTCMaterial->Expressions.Add(MANamedRerouteDeclarationPtr);
	}

	MANamedRerouteDeclarationPtr->Name = FName(*TreeViewParentPtr->FunctionName);
	MANamedRerouteDeclarationPtr->Input.Expression = BlendAttributesPtr;
	
	BlendAttributeMap.Add(*TreeViewParentPtr->FunctionName, BlendAttributesPtr);
	RerouteDeclarationMap.Add(*TreeViewParentPtr->FunctionName, MANamedRerouteDeclarationPtr);
	
	if(TreeViewParentPtr->CurrentUVsComboItem.IsValid() && *TreeViewParentPtr->CurrentUVsComboItem != TreeViewWidget.NoUV && HasToGenerateUVsFunction())
		GenerateUVsFunction();
	
	for(TSharedPtr<FMMGTreeView> TreeViewChild : TreeViewParentPtr->ChildrenFunction)
	{
		TreeViewChildPtr = TreeViewChild;
		
		/** Get material function by combobox selection */
		FName RowName = FName(Utils->TextToPascal(*TreeViewChildPtr->CurrentChildComboItem));
		FunctionDataStruct = MasksDT->FindRow<FMMGMaterialFunctionStruct>(RowName, "");
		
		GenerateMaterialFunctionNode(RowName.ToString());

		BlendAttributesPtr->Alpha.Expression = FunctionNamedRerouteUsagePtr;

		/** Refresh */
		UTCMaterial->PreEditChange(NULL);
		UTCMaterial->PostEditChange();

		/** Get inputs' type */
		InputIndex = 0;
		TArray<FFunctionExpressionInput> InputsList = MaterialFunctionPtr->FunctionInputs;
		for (FFunctionExpressionInput input : InputsList)
		{
			ParameterName = FName (*TreeViewParentPtr->FunctionName + FString(" - ") + input.ExpressionInput->InputName.ToString());

			/** Generate parameter */
			GenerateParameterByMFInputType(MaterialFunctionPtr, input);
			++InputIndex;
		}

		MaterialParameterPosition.Y += 100;

		/** function location */
		SetNodePosition(MaterialFunctionPtr, EMaterialFunction);
		/** Function reroute declaration location */
		SetNodePosition(FunctionNamedRerouteDeclarationPtr, ERerouteDeclarationFunction);
		/** Function reroute usage location */
		SetNodePosition(FunctionNamedRerouteUsagePtr, ERerouteUsageFunction);

		/** Comment node */
		if(bAddToMaterial)
		{
			CommentPtr = MaterialEd->CreateNewMaterialExpressionComment(FVector2D(0,0), EdGraph);
		}
		else
		{
			CommentPtr = NewObject<UMaterialExpressionComment>(UTCMaterial);
			UTCMaterial->Expressions.Add(CommentPtr);
		}
		
		CommentPtr->CommentColor = FLinearColor(0.13,0.08,0.025,1);
		CommentPtr->Text = RowName.ToString();

		/** Comment location */
		SetNodePosition(CommentPtr, EComment);
		
		ParamExpressionList.Empty();
	}
	
	/** Blend location */
	SetNodePosition(BlendAttributesPtr, EMaterialAttribute);
	/** Blend reroute declaration location */
	SetNodePosition(MANamedRerouteDeclarationPtr, ERerouteDeclarationMA);

	/** Generate UVs parameters */
	if(TreeViewParentPtr->CurrentUVsComboItem.IsValid() && *TreeViewParentPtr->CurrentUVsComboItem != TreeViewWidget.NoUV && HasToGenerateUVsFunction())
	{
		if(isUVsFunctionConnected)
		{
			FName RowName = FName(Utils->TextToPascal(*TreeViewParentPtr->CurrentUVsComboItem));
			FunctionDataStruct = UVsDT->FindRow<FMMGMaterialFunctionStruct>(RowName, "");

			UVsParameterPosition.Y = BlendAttributesPtr->MaterialExpressionEditorY;
			GenerateUVsParameters();
			
			SetNodePosition(UVsFunctionPtr, EUVsFunction);

			ParamExpressionList.Empty();
		}
		else
			UTCMaterial->Expressions.Remove(UVsFunctionPtr);
	}
	
	/** Reroute usage location */
	for (UMaterialExpression* RerouteNode : NamedRerouteUsageList)
	{
		SetNodePosition(RerouteNode, ERerouteUsageFunction);
	}
	
	/** Main comment node */
	if(bAddToMaterial)
	{
		MainCommentPtr = MaterialEd->CreateNewMaterialExpressionComment(FVector2D(0,0), EdGraph);
	}
	else
	{
		MainCommentPtr = NewObject<UMaterialExpressionComment>(UTCMaterial);
		UTCMaterial->Expressions.Add(MainCommentPtr);
	}
	
	MainCommentPtr->CommentColor = FLinearColor(0,0,0,1);
	MainCommentPtr->Text = *TreeViewParentPtr->FunctionName;
		
	/** Comment location */
	SetNodePosition(MainCommentPtr, EMainComment);
	
	NamedRerouteUsageList.Empty();
	MFExpressionList.Empty();
	CommentYPositionDoOnce = true;
	isUVsFunctionConnected = false;
	MaterialParameterPosition.Y += 200;
}

void FMMGLibrary::GenerateUVsFunction()
{
	/** Get UVs function by ComboBox selection */
	FName RowName = FName(Utils->TextToPascal(*TreeViewParentPtr->CurrentUVsComboItem));
	FunctionDataStruct = UVsDT->FindRow<FMMGMaterialFunctionStruct>(RowName, "");
	
	/** Generate the material function */
	if(bAddToMaterial)
	{
		UMaterialExpression* UVsFunctionExpression = MaterialEd->CreateNewMaterialExpression(UMaterialExpressionMaterialFunctionCall::StaticClass(), FVector2D(0,0), false, false, EdGraph);
		UVsFunctionPtr = Cast<UMaterialExpressionMaterialFunctionCall>(UVsFunctionExpression);
		UVsFunctionPtr->MaterialFunction = FunctionDataStruct->MaterialFunction;
		UVsFunctionPtr->UpdateFromFunctionResource(false);
		MatGraph->RebuildGraph();
	}
	else
	{
		UVsFunctionPtr = NewObject<UMaterialExpressionMaterialFunctionCall>(UTCMaterial);
		UTCMaterial->Expressions.Add(UVsFunctionPtr);
		UVsFunctionPtr->MaterialFunction = FunctionDataStruct->MaterialFunction;
	}
	
	/** Refresh */
	UTCMaterial->PreEditChange(NULL);
	UTCMaterial->PostEditChange();
	
}

void FMMGLibrary::GenerateUVsParameters()
{
	InputIndex = 0;
	TArray<FFunctionExpressionInput> InputList = UVsFunctionPtr->FunctionInputs;
	/** Get inputs' type */
	for (FFunctionExpressionInput input : InputList)
	{
		ParameterName = FName (*TreeViewParentPtr->FunctionName + FString(" - ") + input.ExpressionInput->InputName.ToString());

		/** Generate parameters */
		GenerateParameterByMFInputType(UVsFunctionPtr, input, true);
		++InputIndex;
	}
}

bool FMMGLibrary::HasToGenerateUVsFunction() const
{
	for(auto Child : TreeViewParentPtr->ChildrenFunction)
	{
		if(Child->AffectedByUVs)
			return true;
	}
	return false;
}

void FMMGLibrary::GenerateMaterialFunctionNode(const FString FunctionName)
{
	if(bAddToMaterial)
	{
		/** Generate the material function */
		UMaterialExpression* MaterialFunctionExpression = MaterialEd->CreateNewMaterialExpression(UMaterialExpressionMaterialFunctionCall::StaticClass(), FVector2D(0,0), false, false, EdGraph);
		MaterialFunctionPtr = Cast<UMaterialExpressionMaterialFunctionCall>(MaterialFunctionExpression);
		MaterialFunctionPtr->MaterialFunction = FunctionDataStruct->MaterialFunction;
		MaterialFunctionPtr->UpdateFromFunctionResource(false);
		MatGraph->RebuildGraph();
		
		MFExpressionList.Add(MaterialFunctionPtr);

		/** Function reroute named node
		 * Declaration
		 */
		UMaterialExpression* NRDExpression = MaterialEd->CreateNewMaterialExpression(UMaterialExpressionNamedRerouteDeclaration::StaticClass(), FVector2D(0,0), false, false, EdGraph);
		FunctionNamedRerouteDeclarationPtr = Cast<UMaterialExpressionNamedRerouteDeclaration>(NRDExpression);
		FunctionNamedRerouteDeclarationPtr->Name = FName(*TreeViewParentPtr->FunctionName + FString(" - ") + FunctionName );
		
		/** Usage */
		UMaterialExpression* NRUExpression = MaterialEd->CreateNewMaterialExpression(UMaterialExpressionNamedRerouteUsage::StaticClass(), FVector2D(0,0), false, false, EdGraph);
		FunctionNamedRerouteUsagePtr = Cast<UMaterialExpressionNamedRerouteUsage>(NRUExpression);
		FunctionNamedRerouteUsagePtr->Declaration = FunctionNamedRerouteDeclarationPtr;
		
		FunctionNamedRerouteDeclarationPtr->Input.Expression = MaterialFunctionPtr;	
	}
	else
	{
		/** Generate the material function */
		MaterialFunctionPtr = NewObject<UMaterialExpressionMaterialFunctionCall>(UTCMaterial);
		MaterialFunctionPtr->MaterialFunction = FunctionDataStruct->MaterialFunction;
		UTCMaterial->Expressions.Add(MaterialFunctionPtr);
		MFExpressionList.Add(MaterialFunctionPtr);
	
		/** Function reroute named node
		 * Declaration
		 */
		FunctionNamedRerouteDeclarationPtr = NewObject<UMaterialExpressionNamedRerouteDeclaration>(UTCMaterial);
		FunctionNamedRerouteDeclarationPtr->Name = FName(*TreeViewParentPtr->FunctionName + FString(" - ") + FunctionName );
		UTCMaterial->Expressions.Add(FunctionNamedRerouteDeclarationPtr);

		/** Usage */
		FunctionNamedRerouteUsagePtr = NewObject<UMaterialExpressionNamedRerouteUsage>(UTCMaterial);;
		FunctionNamedRerouteUsagePtr->Declaration = FunctionNamedRerouteDeclarationPtr;
		UTCMaterial->Expressions.Add(FunctionNamedRerouteUsagePtr);
		
		FunctionNamedRerouteDeclarationPtr->Input.Expression = MaterialFunctionPtr;	
	}

	if(*TreeViewParentPtr->FunctionType == TreeViewWidget.MAType)
		RerouteUsageMap.Add(FunctionDataStruct->MaterialAttributeType.GetValue(), FunctionNamedRerouteUsagePtr);
	if(*TreeViewParentPtr->FunctionType == TreeViewWidget.MaskType)
		NamedRerouteUsageList.Add(FunctionNamedRerouteUsagePtr);
}

void FMMGLibrary::GenerateParameterByMFInputType(UMaterialExpressionMaterialFunctionCall* MaterialFunction, FFunctionExpressionInput CurrentInput, bool isUVsFunction)
{
	EFunctionInputType InputType = CurrentInput.ExpressionInput->InputType.GetValue();
	
	/** Preview default value */
	UMaterialExpression* PreviewExpression = MaterialFunction->FunctionInputs[InputIndex].ExpressionInput->Preview.Expression;

	FString GroupName;
	if(isUVsFunction)
	{
		if(*TreeViewParentPtr->FunctionType == TreeViewWidget.MaskType)
			GroupName = "~ " + *TreeViewParentPtr->FunctionName + " 00" + " - " + FunctionDataStruct->MaterialGroup + " ~";
		if(*TreeViewParentPtr->FunctionType == TreeViewWidget.MAType)
			GroupName = "- " + *TreeViewParentPtr->FunctionName + " 00" + " - " + FunctionDataStruct->MaterialGroup + " -";
	}
	else
	{
		if(*TreeViewParentPtr->FunctionType == TreeViewWidget.MaskType)
			GroupName = "~ " + *TreeViewParentPtr->FunctionName + " 01 - " +  FunctionDataStruct->MaterialGroup + " ~";
		if (*TreeViewParentPtr->FunctionType == TreeViewWidget.MAType)
			GroupName = "- " + *TreeViewParentPtr->FunctionName + " 0" + FString::FromInt(FunctionIndex + 1 ) + " - " + FunctionDataStruct->MaterialGroup + " -";
	}
		
		
//--------------------------------------------------------------------------------------------------------------
	if(InputType == EFunctionInputType(FunctionInput_Scalar) || InputType == EFunctionInputType(FunctionInput_Vector2) || InputType == EFunctionInputType(FunctionInput_Vector3) || InputType == EFunctionInputType(FunctionInput_Vector4))
	{
		/** If Texture as vector parameter */
		if(MaterialFunction->FunctionInputs[InputIndex].ExpressionInput->InputName.ToString().StartsWith("Texture"))
		{
			GenerateTextureSampleParameter(MaterialFunction, PreviewExpression, CurrentInput, InputType, GroupName, isUVsFunction);
			return;
		}
		/** If vector paramter */
		else
		{
			/** Scalar */
			if(InputType == EFunctionInputType(FunctionInput_Scalar))
			{
				GenerateScalarParameter(MaterialFunction, PreviewExpression, CurrentInput, InputType, GroupName, isUVsFunction);
				return;
			}

			/** Vector 2/3 */
			else if(InputType == EFunctionInputType(FunctionInput_Vector2) || InputType == EFunctionInputType(FunctionInput_Vector3) )
			{
				GenerateVector2n3Parameter(MaterialFunction, PreviewExpression, CurrentInput, InputType, GroupName, isUVsFunction);
				return;
			}

			/** Vector 4 */
			else if( InputType == EFunctionInputType(FunctionInput_Vector4))
			{
				GenerateVector4Parameter(MaterialFunction, PreviewExpression, CurrentInput, InputType, GroupName, isUVsFunction);
				return;
			}
		}
	}
//--------------------------------------------------------------------------------------------------------------
	else if(InputType == EFunctionInputType(FunctionInput_Texture2D))
	{
		GenerateTexture2DParameter(MaterialFunction, PreviewExpression, CurrentInput, InputType, GroupName, isUVsFunction);
		return;
	}
//--------------------------------------------------------------------------------------------------------------
	else if(InputType == EFunctionInputType(FunctionInput_TextureCube))
	{
		UE_LOG(LogTemp, Warning, TEXT("Output: TextureCube Not Implemented"));
	}
//--------------------------------------------------------------------------------------------------------------
	else if(InputType == EFunctionInputType(FunctionInput_Texture2DArray))
	{
		UE_LOG(LogTemp, Warning, TEXT("Output: Texture2DArray Not Implemented"));
	}
//--------------------------------------------------------------------------------------------------------------
	else if(InputType == EFunctionInputType(FunctionInput_VolumeTexture))
	{
		UE_LOG(LogTemp, Warning, TEXT("Output: VolumeTexture Not Implemented"));
	}
//--------------------------------------------------------------------------------------------------------------
	else if(InputType == EFunctionInputType(FunctionInput_StaticBool))
	{
		GenerateBoolParameter(MaterialFunction, PreviewExpression, CurrentInput, InputType, GroupName, isUVsFunction);
		return;
	}
//--------------------------------------------------------------------------------------------------------------
	else if(InputType == EFunctionInputType(FunctionInput_MaterialAttributes))
	{
		UE_LOG(LogTemp, Warning, TEXT("Output: MaterialAttributes Not Implemented"));
	}
//--------------------------------------------------------------------------------------------------------------
	else if(InputType == EFunctionInputType(FunctionInput_TextureExternal))
	{
		UE_LOG(LogTemp, Warning, TEXT("Output: TextureExternal Not Implemented"));
	}
//--------------------------------------------------------------------------------------------------------------
	else if(InputType == EFunctionInputType(FunctionInput_MAX))
	{
		UE_LOG(LogTemp, Warning, TEXT("Output: MAX Not Implemented"));
	}
}

void FMMGLibrary::GenerateTextureSampleParameter(UMaterialExpressionMaterialFunctionCall* MaterialFunction, UMaterialExpression* PreviewExpression, FFunctionExpressionInput CurrentInput, EFunctionInputType InputType, FString GroupName, const bool isUVsFunction)
{
	UMaterialExpressionTextureSampleParameter2D* TextureParam;
	if(bAddToMaterial)
	{
		UMaterialExpression* TextureExpression = MaterialEd->CreateNewMaterialExpression(UMaterialExpressionTextureSampleParameter2D::StaticClass(), FVector2D(0,0), false, false, EdGraph);
		TextureParam = Cast<UMaterialExpressionTextureSampleParameter2D>(TextureExpression);
	}
	else
	{
		TextureParam = NewObject<UMaterialExpressionTextureSampleParameter2D>(UTCMaterial);
		UTCMaterial->Expressions.Add(TextureParam);
	}

	/** Cast to preview value & set default */
	UMaterialExpressionTextureSample* PreviewValue = Cast<UMaterialExpressionTextureSample>(PreviewExpression);
	if(PreviewValue)
	{
		TextureParam->Texture = PreviewValue->Texture;
		TextureParam->SamplerType = PreviewValue->SamplerType;
	}

	/** Connect UVs */
	if(TreeViewChildPtr->AffectedByUVs && TreeViewParentPtr->CurrentUVsComboItem.IsValid() && *TreeViewParentPtr->CurrentUVsComboItem != TreeViewWidget.NoUV)
	{
		TextureParam->Coordinates.Expression = UVsFunctionPtr;
		isUVsFunctionConnected = true;
	}
			
	/** Parameter infos */
	TextureParam->Desc = CurrentInput.ExpressionInput->Description;
	TextureParam->ParameterName = ParameterName;
	TextureParam->Group = FName( GroupName);
	TextureParam->SortPriority = InputIndex;

	int32 OutputIndex = 0;
	if (InputType == EFunctionInputType(FunctionInput_Vector4))
		OutputIndex = 5;

	/** Increment node location */
	if(isUVsFunction)
	{
		SetNodePosition(TextureParam, EUVsParameter);
	}
	else
		SetNodePosition(TextureParam, EParameter);
			
	/** Connect */
	MaterialFunction->FunctionInputs[InputIndex].Input.Connect(OutputIndex, TextureParam);

	ParamExpressionList.Add(TextureParam);
}

void FMMGLibrary::GenerateScalarParameter(UMaterialExpressionMaterialFunctionCall* MaterialFunction, UMaterialExpression* PreviewExpression, FFunctionExpressionInput CurrentInput, EFunctionInputType InputType, FString GroupName, const bool isUVsFunction)
{
	UMaterialExpressionScalarParameter* ScalarParam;
	if(bAddToMaterial)
	{
		UMaterialExpression* ScalarExpression = MaterialEd->CreateNewMaterialExpression(UMaterialExpressionScalarParameter::StaticClass(), FVector2D(0,0), false, false, EdGraph);
		ScalarParam = Cast<UMaterialExpressionScalarParameter>(ScalarExpression);
	}
	else
	{
		ScalarParam = NewObject<UMaterialExpressionScalarParameter>(UTCMaterial);
		UTCMaterial->Expressions.Add(ScalarParam);
	}
	
	/** Cast to preview value & set default */
	UMaterialExpressionConstant* PreviewValue = Cast<UMaterialExpressionConstant>(PreviewExpression);
	if(PreviewValue)
	{
		ScalarParam->DefaultValue = PreviewValue->R;
	}
	else
		ScalarParam->DefaultValue = 1;

	/** Parameter infos */
	ScalarParam->Desc = CurrentInput.ExpressionInput->Description;
	ScalarParam->ParameterName = ParameterName;
	ScalarParam->SortPriority = InputIndex;
	ScalarParam->Group = FName(GroupName);

	/** Increment node location */
	if(isUVsFunction)
	{
		SetNodePosition(ScalarParam, EUVsParameter, true);
	}
	else
		SetNodePosition(ScalarParam, EParameter, true);
				
	/** Connect */
	MaterialFunction->FunctionInputs[InputIndex].Input.Expression = ScalarParam;
				
	ParamExpressionList.Add(ScalarParam);
}

void FMMGLibrary::GenerateVector2n3Parameter(UMaterialExpressionMaterialFunctionCall* MaterialFunction, UMaterialExpression* PreviewExpression, FFunctionExpressionInput CurrentInput, EFunctionInputType InputType, FString GroupName, const bool isUVsFunction)
{
	UMaterialExpressionVectorParameter* VectorParam;
	if(bAddToMaterial)
	{
		UMaterialExpression* VectorExpression = MaterialEd->CreateNewMaterialExpression(UMaterialExpressionVectorParameter::StaticClass(), FVector2D(0,0), false, false, EdGraph);
		VectorParam = Cast<UMaterialExpressionVectorParameter>(VectorExpression);
	}
	else
	{
		VectorParam = NewObject<UMaterialExpressionVectorParameter>(UTCMaterial);
		UTCMaterial->Expressions.Add(VectorParam);
	}
	
				
	if(InputType == EFunctionInputType(FunctionInput_Vector2))
	{
		/** Cast to preview value & set default */
		UMaterialExpressionConstant2Vector* PreviewValue = Cast<UMaterialExpressionConstant2Vector>(PreviewExpression);
		if(PreviewValue)
		{
			VectorParam->DefaultValue.R = PreviewValue->R;
			VectorParam->DefaultValue.G = PreviewValue->G;
		}
	}
	else if (InputType == EFunctionInputType(FunctionInput_Vector3))
	{
		UMaterialExpressionConstant3Vector* PreviewValue = Cast<UMaterialExpressionConstant3Vector>(PreviewExpression);
		if(PreviewValue)
		{
			VectorParam->DefaultValue = PreviewValue->Constant;
		}
	}
				
	/** Parameter infos */
	VectorParam->Desc = CurrentInput.ExpressionInput->Description;
	VectorParam->ParameterName = ParameterName;
	VectorParam->Group = FName( GroupName);
	VectorParam->SortPriority = InputIndex;

	/** Increment node location */
	if(isUVsFunction)
	{
		SetNodePosition(VectorParam, EUVsParameter);
	}
	else
		SetNodePosition(VectorParam, EParameter);
				
	/** Connect */
	MaterialFunction->FunctionInputs[InputIndex].Input.Connect(0, VectorParam);

	ParamExpressionList.Add(VectorParam);
}

void FMMGLibrary::GenerateVector4Parameter(UMaterialExpressionMaterialFunctionCall* MaterialFunction, UMaterialExpression* PreviewExpression, FFunctionExpressionInput CurrentInput, EFunctionInputType InputType, FString GroupName, const bool isUVsFunction)
{
	UMaterialExpressionVectorParameter* VectorParam;
	UMaterialExpressionAppendVector* AppendParam;
	if(bAddToMaterial)
	{
		UMaterialExpression* VectorExpression = MaterialEd->CreateNewMaterialExpression(UMaterialExpressionVectorParameter::StaticClass(), FVector2D(0,0), false, false, EdGraph);
		VectorParam = Cast<UMaterialExpressionVectorParameter>(VectorExpression);

		UMaterialExpression* AppendExpression = MaterialEd->CreateNewMaterialExpression(UMaterialExpressionAppendVector::StaticClass(), FVector2D(0,0), false, false, EdGraph);
		AppendParam = Cast<UMaterialExpressionAppendVector>(AppendExpression);
	}
	else
	{
		VectorParam = NewObject<UMaterialExpressionVectorParameter>(UTCMaterial);
		UTCMaterial->Expressions.Add(VectorParam);
		
		AppendParam = NewObject<UMaterialExpressionAppendVector>(UTCMaterial);
		UTCMaterial->Expressions.Add(AppendParam);
	}
	
	/** Cast to preview value & set default */
	UMaterialExpressionConstant4Vector* PreviewValue = Cast<UMaterialExpressionConstant4Vector>(PreviewExpression);
	if(PreviewValue)
	{
		VectorParam->DefaultValue = PreviewValue->Constant;
	}
	
	/** Parameter infos */
	VectorParam->Desc = CurrentInput.ExpressionInput->Description;
	VectorParam->ParameterName = ParameterName;
	VectorParam->Group = FName( GroupName);
	VectorParam->SortPriority = InputIndex;

	/** Increment node location */
	if(isUVsFunction)
	{
		SetNodePosition(VectorParam, EUVsParameter);
	}
	else
		SetNodePosition(VectorParam, EParameter);
				
	/** Connect */
	MaterialFunction->FunctionInputs[InputIndex].Input.Connect(0, AppendParam);
	AppendParam->A.Connect(0, VectorParam);
	AppendParam->B.Connect(4, VectorParam);
				
	/** Append param add position */
	AppendParam->MaterialExpressionEditorX = VectorParam->MaterialExpressionEditorX;
	AppendParam->MaterialExpressionEditorY = VectorParam->MaterialExpressionEditorY;
	AppendParam->MaterialExpressionEditorX += AppendParam->GetWidth() *2;
	AppendParam->MaterialExpressionEditorY += AppendParam->GetHeight()/2;
	
	ParamExpressionList.Add(VectorParam);
}

void FMMGLibrary::GenerateTexture2DParameter(UMaterialExpressionMaterialFunctionCall* MaterialFunction, UMaterialExpression* PreviewExpression, FFunctionExpressionInput CurrentInput, EFunctionInputType InputType, FString GroupName, const bool isUVsFunction)
{
	UMaterialExpressionTextureObjectParameter* TextureObjectParam;
	if(bAddToMaterial)
	{
		UMaterialExpression* TextureObjExpression = MaterialEd->CreateNewMaterialExpression(UMaterialExpressionTextureObjectParameter::StaticClass(), FVector2D(0,0), false, false, EdGraph);
		TextureObjectParam = Cast<UMaterialExpressionTextureObjectParameter>(TextureObjExpression);
	}
	else
	{
		TextureObjectParam = NewObject<UMaterialExpressionTextureObjectParameter>(UTCMaterial);
		UTCMaterial->Expressions.Add(TextureObjectParam);
	}
	/** Cast to preview value & set default */
	UMaterialExpressionTextureObject* PreviewValue = Cast<UMaterialExpressionTextureObject>(PreviewExpression);
	if(PreviewValue)
	{
		TextureObjectParam->Texture = PreviewValue->Texture;
		TextureObjectParam->SamplerType = PreviewValue->SamplerType;
	}
			
	/** Parameter infos */
	TextureObjectParam->Desc = CurrentInput.ExpressionInput->Description;
	TextureObjectParam->ParameterName = ParameterName;
	TextureObjectParam->Group = FName( GroupName);
	TextureObjectParam->SortPriority = InputIndex;
		
	/** Increment node location */
	if(isUVsFunction)
	{
		SetNodePosition(TextureObjectParam, EUVsParameter);
	}
	else
		SetNodePosition(TextureObjectParam, EParameter);
			
	/** Connect */
	MaterialFunction->FunctionInputs[InputIndex].Input.Connect(0, TextureObjectParam);

	ParamExpressionList.Add(TextureObjectParam);
}

void FMMGLibrary::GenerateBoolParameter(UMaterialExpressionMaterialFunctionCall* MaterialFunction, UMaterialExpression* PreviewExpression, FFunctionExpressionInput CurrentInput, EFunctionInputType InputType, FString GroupName, const bool isUVsFunction)
{
	UMaterialExpressionStaticBoolParameter* BoolParam;
	if(bAddToMaterial)
	{
		UMaterialExpression* BoolExpression = MaterialEd->CreateNewMaterialExpression(UMaterialExpressionStaticBoolParameter::StaticClass(), FVector2D(0,0), false, true, EdGraph);
		BoolParam = Cast<UMaterialExpressionStaticBoolParameter>(BoolExpression);
	}
	else
	{
		BoolParam = NewObject<UMaterialExpressionStaticBoolParameter>(UTCMaterial);
		UTCMaterial->Expressions.Add(BoolParam);
	}
	
	/** Cast to preview value & set default */
	UMaterialExpressionStaticBool* PreviewValue = Cast<UMaterialExpressionStaticBool>(PreviewExpression);
	if(PreviewValue)
	{
		BoolParam->DefaultValue = PreviewValue->Value;
	}
		
	/** Parameter infos */
	BoolParam->Desc = CurrentInput.ExpressionInput->Description;
	BoolParam->ParameterName = ParameterName;
	BoolParam->Group = FName( GroupName);
	BoolParam->SortPriority = InputIndex;
	
	if(isUVsFunction)
	{
		SetNodePosition(BoolParam, EUVsParameter, true);
	}
	else
		SetNodePosition(BoolParam, EParameter, true);
		
	/** Connect */
	MaterialFunction->FunctionInputs[InputIndex].Input.Expression = BoolParam;

	ParamExpressionList.Add(BoolParam);
}

void FMMGLibrary::ConnectFunctionToMaterialAttribute()
{
	if (FunctionDataStruct->MaterialAttributeType.GetValue() == EMAttributes::BaseColor)
	{
		MaterialAttributePtr->BaseColor.Expression = FunctionNamedRerouteUsagePtr;
	}
	else if (FunctionDataStruct->MaterialAttributeType.GetValue() == EMAttributes::Roughness)
	{
		MaterialAttributePtr->Roughness.Expression = FunctionNamedRerouteUsagePtr;
	}
	else if (FunctionDataStruct->MaterialAttributeType.GetValue() == EMAttributes::Metallic)
	{
		MaterialAttributePtr->Metallic.Expression = FunctionNamedRerouteUsagePtr;
	}
	else if (FunctionDataStruct->MaterialAttributeType.GetValue() == EMAttributes::Specular)
	{
		MaterialAttributePtr->Specular.Expression = FunctionNamedRerouteUsagePtr;
	}
	else if (FunctionDataStruct->MaterialAttributeType.GetValue() == EMAttributes::EmissiveColor)
	{
		MaterialAttributePtr->EmissiveColor.Expression = FunctionNamedRerouteUsagePtr;
	}
	else if (FunctionDataStruct->MaterialAttributeType.GetValue() == EMAttributes::Opacity)
	{
		MaterialAttributePtr->Opacity.Expression = FunctionNamedRerouteUsagePtr;
	}
	else if (FunctionDataStruct->MaterialAttributeType.GetValue() == EMAttributes::OpacityMask)
	{
		MaterialAttributePtr->OpacityMask.Expression = FunctionNamedRerouteUsagePtr;
	}
	else if (FunctionDataStruct->MaterialAttributeType.GetValue() == EMAttributes::Normal)
	{
		MaterialAttributePtr->Normal.Expression = FunctionNamedRerouteUsagePtr;
	}
	else if (FunctionDataStruct->MaterialAttributeType.GetValue() == EMAttributes::WorldPositionOffset)
	{
		MaterialAttributePtr->WorldPositionOffset.Expression = FunctionNamedRerouteUsagePtr;
	}
	else if (FunctionDataStruct->MaterialAttributeType.GetValue() == EMAttributes::SubsurfaceColor)
	{
		MaterialAttributePtr->SubsurfaceColor.Expression = FunctionNamedRerouteUsagePtr;
	}
	else if (FunctionDataStruct->MaterialAttributeType.GetValue() == EMAttributes::AmbientOcclusion)
	{
		MaterialAttributePtr->AmbientOcclusion.Expression = FunctionNamedRerouteUsagePtr;
	}
	else if (FunctionDataStruct->MaterialAttributeType.GetValue() == EMAttributes::PixelDepthOffset)
	{
		MaterialAttributePtr->PixelDepthOffset.Expression = FunctionNamedRerouteUsagePtr;
	}
}

void FMMGLibrary::ConnectRerouteOutput()
{
	RerouteDeclarationRest = RerouteDeclarationMap;
	for (FMMGTreeViewPtr TreeViewParent : TreeViewList)
	{
		if(*TreeViewParent->FunctionType == TreeViewWidget.MaskType)
		{
			bool bAValid = TreeViewParent->ChildrenFunction[0]->CurrentAComboItem.IsValid();
			bool bBValid = TreeViewParent->ChildrenFunction[0]->CurrentBComboItem.IsValid();
			
			UMaterialExpressionNamedRerouteDeclaration* AMaskRerouteItem = nullptr;
			FString AItem;
			if(bAValid)
			{
				AItem = *TreeViewParent->ChildrenFunction[0]->CurrentAComboItem;
				AMaskRerouteItem = *RerouteDeclarationMap.Find(AItem);
			}
			
			UMaterialExpressionNamedRerouteDeclaration* BMaskRerouteItem = nullptr;
			FString BItem;
			if(bBValid)
			{
				BItem = *TreeViewParent->ChildrenFunction[0]->CurrentBComboItem;
				BMaskRerouteItem = *RerouteDeclarationMap.Find(BItem);
			}
			
			CurrentBlendAttribute = *BlendAttributeMap.Find(*TreeViewParent->FunctionName);

			UMaterialExpressionNamedRerouteUsage* ANamedRerouteUsage = nullptr;
			UMaterialExpressionNamedRerouteUsage* BNamedRerouteUsage = nullptr;
			if(bAddToMaterial)
			{
				if(bAValid)
				{
					UMaterialExpression* ANRUExpression = MaterialEd->CreateNewMaterialExpression(UMaterialExpressionNamedRerouteUsage::StaticClass(), FVector2D(0,0), false, false, EdGraph);
					ANamedRerouteUsage = Cast<UMaterialExpressionNamedRerouteUsage>(ANRUExpression);
				}

				if(bBValid)
				{
					UMaterialExpression* BNRUExpression = MaterialEd->CreateNewMaterialExpression(UMaterialExpressionNamedRerouteUsage::StaticClass(), FVector2D(0,0), false, false, EdGraph);
					BNamedRerouteUsage = Cast<UMaterialExpressionNamedRerouteUsage>(BNRUExpression);
				}

			}
			else
			{
				/** A usage */
				ANamedRerouteUsage = NewObject<UMaterialExpressionNamedRerouteUsage>(UTCMaterial);
				UTCMaterial->Expressions.Add(ANamedRerouteUsage);

				/** B usage */
				BNamedRerouteUsage = NewObject<UMaterialExpressionNamedRerouteUsage>(UTCMaterial);
				UTCMaterial->Expressions.Add(BNamedRerouteUsage);
			}

			if(bAValid)
			{
				/**Assign Declaration*/
				ANamedRerouteUsage->Declaration = AMaskRerouteItem;
				
				/** Connect usage to blend */
				CurrentBlendAttribute->A.Expression = ANamedRerouteUsage;

				/** Set usage position */
				SetNodePosition(ANamedRerouteUsage, ERerouteUsageBlendA);

				RerouteDeclarationRest.Remove(AItem);
			}
				
			if(bBValid)
			{
				/**Assign Declaration*/
				BNamedRerouteUsage->Declaration = BMaskRerouteItem;

				/** Connect usage to blend */
				CurrentBlendAttribute->B.Expression = BNamedRerouteUsage;

				/** Set usage position */
				SetNodePosition(BNamedRerouteUsage, ERerouteUsageBlendB);

				RerouteDeclarationRest.Remove(BItem);
			}
		}
	}
	TArray <UMaterialExpressionNamedRerouteDeclaration*> Rest;
	RerouteDeclarationRest.GenerateValueArray(Rest);

	if(Rest.Num() > 1)
	{
		for (auto item : Rest)
		{
			if(item != Rest[0])
			{
				FText InNotif = FText::FromString("Warning: " + *RerouteDeclarationRest.FindKey(item) + " has no connection." );
				SpawnWarningNotif(InNotif);
			}
		}
	}
	
	/** Rest usage */
	if(!bAddToMaterial)
	{
		UMaterialExpressionNamedRerouteUsage* RestNamedRerouteUsage = NewObject<UMaterialExpressionNamedRerouteUsage>(UTCMaterial);
		RestNamedRerouteUsage->Declaration = Rest[0];
		UTCMaterial->Expressions.Add(RestNamedRerouteUsage);
		UTCMaterial->MaterialAttributes.Expression = RestNamedRerouteUsage;

		UTCMaterial->EditorX = 2400;
		RestNamedRerouteUsage->MaterialExpressionEditorX = UTCMaterial->EditorX - RestNamedRerouteUsage->GetWidth()*2;
	}
}

void FMMGLibrary::SetNodePosition(UMaterialExpression* Expression, EExpressionType ExpressionType, bool ReduceNodeSize)
{
	if(ExpressionType == EMaterialAttribute)
	{
		int32 MinY = MFExpressionList[0]->MaterialExpressionEditorY;
		int32 MaxY = MFExpressionList.Last()->MaterialExpressionEditorY;
		
		Expression->MaterialExpressionEditorX = 1550;
		Expression->MaterialExpressionEditorY = ((MinY + MaxY)/2) - Expression->GetHeight();
	}

	else if(ExpressionType == EMaterialFunction)
	{
		int32 MinY = ParamExpressionList[0]->MaterialExpressionEditorY;
		int32 MaxY = ParamExpressionList.Last()->MaterialExpressionEditorY;
		
		Expression->MaterialExpressionEditorX = MaterialParameterPosition.X + ParamExpressionList.Last()->GetWidth() *4 + 100;
		Expression->MaterialExpressionEditorY = (MinY + MaxY)/2;
	}
	
	else if(ExpressionType == EUVsFunction)
	{
		int32 MinY = ParamExpressionList[0]->MaterialExpressionEditorY;
		int32 MaxY = ParamExpressionList.Last()->MaterialExpressionEditorY;

		Expression->MaterialExpressionEditorX = ParamExpressionList.Last()->MaterialExpressionEditorX + ParamExpressionList.Last()->GetWidth() *3.5;
		Expression->MaterialExpressionEditorY = (MinY + MaxY)/2;
	}
	

	else if(ExpressionType == ERerouteDeclarationFunction)
	{
		Expression->MaterialExpressionEditorX = MaterialFunctionPtr->MaterialExpressionEditorX + MaterialFunctionPtr->GetWidth() * 2.5;
		Expression->MaterialExpressionEditorY = MaterialFunctionPtr->MaterialExpressionEditorY;
	}

	else if(ExpressionType == ERerouteUsageFunction)
	{
		if(*TreeViewParentPtr->FunctionType == TreeViewWidget.MAType)
		{
			int32 RerouteIndex = NamedRerouteUsageList.Find(Expression);
			Expression->MaterialExpressionEditorX = MaterialAttributePtr->MaterialExpressionEditorX - Expression->GetWidth() * 2 ;
			Expression->MaterialExpressionEditorY = MaterialAttributePtr->MaterialExpressionEditorY + Expression->GetHeight()/2 * RerouteIndex;
		}
		else if(*TreeViewParentPtr->FunctionType == TreeViewWidget.MaskType)
		{
			Expression->MaterialExpressionEditorX = BlendAttributesPtr->MaterialExpressionEditorX - Expression->GetWidth() * 3 ;
			Expression->MaterialExpressionEditorY = BlendAttributesPtr->MaterialExpressionEditorY + Expression->GetHeight() ;
		}

	}

	else if(ExpressionType == ERerouteDeclarationMA)
	{
		if(*TreeViewParentPtr->FunctionType == TreeViewWidget.MAType)
		{
			Expression->MaterialExpressionEditorX = MaterialAttributePtr->MaterialExpressionEditorX + MaterialAttributePtr->GetWidth() + 100;
			Expression->MaterialExpressionEditorY = MaterialAttributePtr->MaterialExpressionEditorY;
		}
		else if(*TreeViewParentPtr->FunctionType == TreeViewWidget.MaskType)
		{
			Expression->MaterialExpressionEditorX = BlendAttributesPtr->MaterialExpressionEditorX + BlendAttributesPtr->GetWidth() + 100;
			Expression->MaterialExpressionEditorY = BlendAttributesPtr->MaterialExpressionEditorY;
		}
	}

	else if(ExpressionType == ERerouteUsageBlendA)
	{
		Expression->MaterialExpressionEditorX = CurrentBlendAttribute->MaterialExpressionEditorX - Expression->GetWidth() *1.5;
		Expression->MaterialExpressionEditorY = CurrentBlendAttribute->MaterialExpressionEditorY - Expression->GetHeight()/2;
	}

	else if(ExpressionType == ERerouteUsageBlendB)
	{
		Expression->MaterialExpressionEditorX = CurrentBlendAttribute->MaterialExpressionEditorX - Expression->GetWidth() *1.5;
		Expression->MaterialExpressionEditorY = CurrentBlendAttribute->MaterialExpressionEditorY + Expression->GetHeight()/4;
	}

	else if(ExpressionType == EParameter)
	{
		Expression->MaterialExpressionEditorX = MaterialParameterPosition.X;
		Expression->MaterialExpressionEditorY = MaterialParameterPosition.Y;
		if(ReduceNodeSize)
			MaterialParameterPosition.Y += (Expression->GetHeight()/2)*1.5;
		else
			MaterialParameterPosition.Y += Expression->GetHeight()*1.5;
	}

	else if(ExpressionType == EUVsParameter)
	{
		if(*TreeViewParentPtr->FunctionType == TreeViewWidget.MAType)
			Expression->MaterialExpressionEditorX = MaterialAttributePtr->MaterialExpressionEditorX * -1 / 1.25 ;
		
		else if(*TreeViewParentPtr->FunctionType == TreeViewWidget.MaskType)
			Expression->MaterialExpressionEditorX = BlendAttributesPtr->MaterialExpressionEditorX * -1 / 1.25;

		UVsParameterPosition.X = Expression->MaterialExpressionEditorX;
		Expression->MaterialExpressionEditorY = UVsParameterPosition.Y;
		
		if(ReduceNodeSize)
			UVsParameterPosition.Y += (Expression->GetHeight()/2)*1.5;
		else
			UVsParameterPosition.Y += Expression->GetHeight()*1.5;
	}

	else if(ExpressionType == EComment)
	{
		Expression->MaterialExpressionEditorX = ParamExpressionList[0]->MaterialExpressionEditorX - 50;
		Expression->MaterialExpressionEditorY = ParamExpressionList[0]->MaterialExpressionEditorY - 50;

		int32 CommentSizeX = FunctionNamedRerouteDeclarationPtr->MaterialExpressionEditorX - ParamExpressionList.Last()->MaterialExpressionEditorX + FunctionNamedRerouteDeclarationPtr->GetWidth() * 3;
		int32 CommentSizeY = ParamExpressionList.Last()->MaterialExpressionEditorY - ParamExpressionList[0]->MaterialExpressionEditorY;
		if(Cast<UMaterialExpressionScalarParameter>( ParamExpressionList.Last()) || Cast<UMaterialExpressionStaticBoolParameter>( ParamExpressionList.Last()))
			CommentSizeY = CommentSizeY + ParamExpressionList.Last()->GetHeight();
		else
			CommentSizeY = CommentSizeY + ParamExpressionList.Last()->GetHeight() * 2;
		
		CommentPtr->SizeX = CommentSizeX;
		CommentPtr->SizeY = CommentSizeY;

		if(CommentYPositionDoOnce)
		{
			FirstCommentYPosition = CommentPtr->MaterialExpressionEditorY;
			CommentYPositionDoOnce = false;
		}
	}

	else if(ExpressionType == EMainComment)
	{
		if(TreeViewParentPtr->CurrentUVsComboItem.IsValid() && *TreeViewParentPtr->CurrentUVsComboItem != TreeViewWidget.NoUV && HasToGenerateUVsFunction() && isUVsFunctionConnected)
			Expression->MaterialExpressionEditorX = UVsParameterPosition.X - 50;
		else
			Expression->MaterialExpressionEditorX = MaterialParameterPosition.X - 100;
		
		Expression->MaterialExpressionEditorY = FirstCommentYPosition - 75;

		MainCommentPtr->SizeX = abs(MANamedRerouteDeclarationPtr->MaterialExpressionEditorX - Expression->MaterialExpressionEditorX + MANamedRerouteDeclarationPtr->GetWidth() * 2.5);
		MainCommentPtr->SizeY = abs(CommentPtr->MaterialExpressionEditorY + CommentPtr->SizeY - Expression->MaterialExpressionEditorY) + 25 ;
	}
}

SGraphEditor* FMMGLibrary::FindGraphEditorByMaterial(UMaterial* TargetMaterial)
{
	GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->OpenEditorForAsset(TargetMaterial);
	
	TArray<TSharedRef<SWindow>> Windows;
	FSlateApplication::Get().GetAllVisibleWindowsOrdered(Windows);
	
	TArray<SWidget*> Stack;
	
	for (const TSharedRef<SWindow>& Window : Windows)
	{
		FString WindowName = Window.Get().ToString();
		
		if(WindowName.Contains(TargetMaterial->GetName()))
			Stack.Push(&Window.Get());
	}
	
	while (Stack.Num() > 0)
	{
		SWidget* Widget = Stack.Pop();
		const int32 NumChildren = Widget->GetChildren()->Num();
		for (int32 ChildIndex = 0; ChildIndex < NumChildren; ChildIndex++)
		{
			SWidget& ChildWidget = Widget->GetChildren()->GetChildAt(ChildIndex).Get();
			if(ChildWidget.GetType().ToString() == "SGraphEditor")
			{
				SGraphEditor& Graph = static_cast<SGraphEditor&>(ChildWidget);
				return &Graph;
			}
			
			Stack.Push(&ChildWidget);
		}
	}
	return NULL;
}

void FMMGLibrary::ReinitVar()
{
	MaterialParameterPosition = FVector2d(0, 0);
	UVsParameterPosition = FVector2d(0, 0);
	
	ParamExpressionList.Empty();
	MFExpressionList.Empty();
	NamedRerouteUsageList.Empty();
	RerouteDeclarationRest.Empty();
	
	RerouteDeclarationMap.Empty();
	BlendAttributeMap.Empty();
	RerouteUsageMap.Empty();

	bAddToMaterial = false;
}

bool FMMGLibrary::ErrorDetector()
{
	if(bAddToMaterial)
	{
		if(!IsValid(UTCMaterial))
		{
			FText InNotif = FText::FromString("Need to select a Target Material");
			SpawnErrorNotif(InNotif);
			return false;
		}
	}
	else
	{
		if(Settings.materialName.IsEmpty())
		{
			FText InNotif = FText::FromString("Set a material name.");
			SpawnErrorNotif(InNotif);
			return false;
		}
	
		/** Check if MM already exist */
		if(UEditorAssetLibrary::DoesAssetExist(PackageName))
		{
			FText InNotif = FText::FromString(Settings.materialName + " already exist.");
			SpawnErrorNotif(InNotif);
			return false;
		}
	}
	
	/** No function */
	if(TreeViewList.IsEmpty())
	{
		FText InNotif = FText::FromString("No valid function");
		SpawnErrorNotif(InNotif);
		return false;
	}
	
	TArray<FString> FunctionNameListCheck;
	for (auto TreeViewParent : TreeViewList)
	{
		/** Check function name */
		if(!FunctionNameListCheck.Contains(*TreeViewParent->FunctionName))
			FunctionNameListCheck.Add(*TreeViewParent->FunctionName);
		else
		{
			FText InNotif = FText::FromString("Function name '" + *TreeViewParent->FunctionName + "', is assigned several times.");
			SpawnErrorNotif(InNotif);
			return false;
		}
		
		
		/** Empty function */
		if(TreeViewParent->ChildrenFunction.IsEmpty())
		{
			FText InNotif = FText::FromString("Function: " + *TreeViewParent->FunctionName + ", is empty.");
			SpawnErrorNotif(InNotif);
			return false;
		}
		
		TArray<FString> FunctionChildListCheck;
		for (auto TreeViewChild : TreeViewParent->ChildrenFunction)
		{
			/** Unassigned child function */
			if(!TreeViewChild->CurrentChildComboItem.IsValid())
			{
				FText InNotif = FText::FromString("Function: " + *TreeViewParent->FunctionName + ", has an unassigned child.");
				SpawnErrorNotif(InNotif);
				return false;
			}
			
			/**  Check Function Child */
			if(!FunctionChildListCheck.Contains(*TreeViewChild->CurrentChildComboItem))
				FunctionChildListCheck.Add(*TreeViewChild->CurrentChildComboItem);
			else
			{
				FText InNotif = FText::FromString("Function: " + *TreeViewParent->FunctionName + ", has several times the same child " + "'" + *TreeViewChild->CurrentChildComboItem + "'.");
				SpawnErrorNotif(InNotif);
				return false;
			}

			/** Check mask inputs */
			if(*TreeViewParent->FunctionType == TreeViewWidget.MaskType && !bAddToMaterial)
			{
				if(!TreeViewChild->CurrentAComboItem.IsValid() || !TreeViewChild->CurrentBComboItem.IsValid())
				{
					FText InNotif = FText::FromString("Function: " + *TreeViewParent->FunctionName + ", has an unassigned input.");
					SpawnErrorNotif(InNotif);
					return false;
				}
			}
			
		}
	}
	return true;
}

void FMMGLibrary::SpawnErrorNotif(const FText NotifContent)
{
	FNotificationInfo Info(NotifContent);
	Info.ExpireDuration = 10.f;
	Info.bUseSuccessFailIcons = false;
	Info.Image =FCoreStyle::Get().GetBrush("Icons.Error");
	Info.WidthOverride = 500.0f;
	Info.bUseLargeFont = true;
	
	FSlateNotificationManager::Get().AddNotification(Info);
}

void FMMGLibrary::SpawnWarningNotif(const FText NotifContent)
{
	FNotificationInfo Info(NotifContent);
	Info.ExpireDuration = 10.f;
	Info.bUseSuccessFailIcons = false;
	Info.Image = FCoreStyle::Get().GetBrush("Icons.Warning");
	Info.WidthOverride = 500.0f;
	Info.bUseLargeFont = true;
	
	FSlateNotificationManager::Get().AddNotification(Info);
}
#undef LOCTEXT_NAMESPACE
