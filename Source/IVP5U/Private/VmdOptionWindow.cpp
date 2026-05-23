// Copyright 2015-2026 IVP5U contributors

#include "VmdOptionWindow.h"

#include "CoreMinimal.h"
#include "Editor/ContentBrowser/Public/ContentBrowserModule.h"
#include "IDocumentation.h"
#include "PropertyEditorModule.h"
#include "IDetailsView.h"

#define LOCTEXT_NAMESPACE "VMDOption"

#define myFAppStyle FAppStyle::Get()

void SVmdOptionWindow::Construct(const FArguments& InArgs)
{
	ImportUI = InArgs._ImportUI;
	WidgetWindow = InArgs._WidgetWindow;

	check(ImportUI);

	TSharedPtr<SBox> InspectorBox;
	this->ChildSlot
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(2)
			[
				SNew(SBorder)
				.Padding(FMargin(3))
				.BorderImage(myFAppStyle.GetBrush("ToolPanel.GroupBorder"))
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.AutoWidth()
					[
						SNew(STextBlock)
						.Font(myFAppStyle.GetFontStyle("CurveEd.LabelFont"))
						.Text(LOCTEXT("Import_CurrentFileTitle", "Current File: "))
					]
					+ SHorizontalBox::Slot()
						.Padding(5, 0, 0, 0)
						.AutoWidth()
						.VAlign(VAlign_Center)
						[
							SNew(STextBlock)
							.Font(myFAppStyle.GetFontStyle("CurveEd.InfoFont"))
							.Text(InArgs._FullPath)
						]
				]
			]
			+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(2)
				[
					SAssignNew(InspectorBox, SBox)
					.MaxDesiredHeight(650.0f)
					.WidthOverride(400.0f)
				]
			+ SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(HAlign_Right)
				.Padding(2)
				[
					SNew(SUniformGridPanel)
					.SlotPadding(2)
					+ SUniformGridPanel::Slot(0, 0)
						[
							SNew(SButton)
							.HAlign(HAlign_Center)
							.Text(LOCTEXT("MMDOptionWindow_ImportAll", "Import All"))
							.ToolTipText(LOCTEXT("MMDOptionWindow_ImportAll_ToolTip", "Import all files with these same settings"))
							.OnClicked(this, &SVmdOptionWindow::OnImportAll)
						]
					+ SUniformGridPanel::Slot(1, 0)
						[
							SAssignNew(ImportButton, SButton)
							.HAlign(HAlign_Center)
							.Text(LOCTEXT("MMDOptionWindow_Import", "Import"))
							.OnClicked(this, &SVmdOptionWindow::OnImport)
						]
					+ SUniformGridPanel::Slot(2, 0)
						[
							SNew(SButton)
							.HAlign(HAlign_Center)
							.Text(LOCTEXT("MMDOptionWindow_Cancel", "Cancel"))
							.ToolTipText(LOCTEXT("MMDOptionWindow_Cancel_ToolTip", "Cancels importing this MMD file"))
							.OnClicked(this, &SVmdOptionWindow::OnCancel)
						]
				]
		];

	FPropertyEditorModule& PropertyEditorModule 
		= FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
	FDetailsViewArgs DetailsViewArgs;
	DetailsViewArgs.bAllowSearch = false;
	DetailsViewArgs.NameAreaSettings = FDetailsViewArgs::HideNameArea;
	TSharedPtr<IDetailsView> DetailsView
		= PropertyEditorModule.CreateDetailView(DetailsViewArgs);

	InspectorBox->SetContent(DetailsView->AsShared());
	DetailsView->SetObject(ImportUI);
}


#undef LOCTEXT_NAMESPACE
