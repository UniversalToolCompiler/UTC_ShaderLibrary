/** Copyright 2023, Universal Tool Compiler */

#include "UI/GraphEditor/MMGGraphNode.h"
#include "UI/GraphEditor/MMGEdGraphNode.h"

#include "GraphEditorSettings.h"
#include "SLevelOfDetailBranchNode.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Layout/SSpacer.h"

void SMMGGraphNode::Construct(const FArguments& InArgs, UMMGEdGraphNode* InNode)
{
	GraphNode = InNode;
	MMGNode = InNode;
	UpdateGraphNode();
}

void SMMGGraphNode::UpdateGraphNode()
{
	RightNodeBox.Reset();
	LeftNodeBox.Reset();

	InputPins.Reset();
	OutputPins.Reset();

	TSharedPtr<SNodeTitle> NodeTitle = SNew(SNodeTitle, GraphNode);

	/** The gloss is drawn twice for a stronger title color */
	TSharedRef<SOverlay> DefaultTitleAreaWidget =
	SNew(SOverlay)
	+SOverlay::Slot()
	[
		SNew(SImage)
		.Image(FAppStyle::GetBrush("Graph.Node.TitleGloss"))
		.ColorAndOpacity(MMGNode->TitleAreaColor)
	]
	+SOverlay::Slot()
	[
		SNew(SImage)
		.Image(FAppStyle::GetBrush("Graph.Node.TitleGloss"))
		.ColorAndOpacity(MMGNode->TitleAreaColor)
	]
	+SOverlay::Slot()
	.HAlign(HAlign_Left)
	.VAlign(VAlign_Center)
	[
		SNew(SBorder)
		.BorderImage(nullptr)
		.Padding(FMargin(10, 5, 30, 3))
		.BorderBackgroundColor(MMGNode->TitleAreaColor)
		[
			SNew(SHorizontalBox)
			+SHorizontalBox::Slot()
			[
				SNew(SVerticalBox)
				+SVerticalBox::Slot()
				.AutoHeight()
				[
					CreateTitleWidget(NodeTitle)
				]
				+SVerticalBox::Slot()
				.AutoHeight()
				[
					NodeTitle.ToSharedRef()
				]
			]
		]
	]
	+SOverlay::Slot()
	.VAlign(VAlign_Top)
	[
		SNew(SBorder)
		.Visibility(EVisibility::HitTestInvisible)
		.BorderImage(FAppStyle::GetBrush("Graph.Node.TitleHighlight"))
		.BorderBackgroundColor(this, &SGraphNode::GetNodeTitleIconColor)
		[
			SNew(SSpacer)
			.Size(FVector2D(20, 20))
		]
	];

	TSharedRef<SWidget> TitleAreaWidget =
	SNew(SLevelOfDetailBranchNode)
	.HighDetail()
	[
		DefaultTitleAreaWidget
	];

	TSharedRef<SVerticalBox> InnerVerticalBox = SNew(SVerticalBox)
	+SVerticalBox::Slot()
	.AutoHeight()
	.HAlign(HAlign_Fill)
	.VAlign(VAlign_Top)
	.Padding(Settings->GetNonPinNodeBodyPadding())
	[
		TitleAreaWidget
	]
	+SVerticalBox::Slot()
	.AutoHeight()
	.HAlign(HAlign_Fill)
	.VAlign(VAlign_Top)
	[
		CreateNodeContentArea()
	];

	LeftNodeBox->SetRenderOpacity(2);
	RightNodeBox->SetRenderOpacity(2);

	this->GetOrAddSlot(ENodeZone::Center)
	.HAlign(HAlign_Center)
	.VAlign(VAlign_Center)
	[
		SNew(SVerticalBox)
		+SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SOverlay)
			+SOverlay::Slot()
			.Padding(Settings->GetNonPinNodeBodyPadding())
			[
				SNew(SImage)
				.Image(GetNodeBodyBrush())
				.ColorAndOpacity(this, &SGraphNode::GetNodeBodyColor)
			]
			+SOverlay::Slot()
			[
				InnerVerticalBox
			]
		]
	];

	CreatePinWidgets();
}

TSharedRef<SWidget> SMMGGraphNode::CreateNodeContentArea()
{
	return SNew(SBorder)
	.BorderImage(FAppStyle::GetBrush("NoBorder"))
	.HAlign(HAlign_Fill)
	.VAlign(VAlign_Fill)
	.Padding(FMargin(0, 3))
	[
		SNew(SHorizontalBox)
		+SHorizontalBox::Slot()
		.FillWidth(1.0f)
		.HAlign(HAlign_Left)
		[
			SAssignNew(LeftNodeBox, SVerticalBox)
		]
		+SHorizontalBox::Slot()
		.AutoWidth()
		.HAlign(HAlign_Right)
		[
			SAssignNew(RightNodeBox, SVerticalBox)
		]
	];
}
