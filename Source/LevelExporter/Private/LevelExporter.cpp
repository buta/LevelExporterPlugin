
#include "LevelExporter.h"
#include "LevelExporterStyle.h"
#include "LevelExporterCommands.h"
#include "ExportSettings.h"
#include "ExportData.h"

#include "AssetExportTask.h"
#include "ToolMenus.h"
#include "EngineUtils.h"
#include "Exporters/Exporter.h"
#include "JsonObjectConverter.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Misc/ScopedSlowTask.h"
#include "Misc/FeedbackContext.h"
#include "Misc/MessageDialog.h"
#include "Widgets/SWindow.h"
#include "Widgets/Input/SButton.h"

#define LOCTEXT_NAMESPACE "FLevelExporterModule"

void FLevelExporterModule::StartupModule()
{
	FLevelExporterStyle::Initialize();
	FLevelExporterStyle::ReloadTextures();
	FLevelExporterCommands::Register();

	PluginCommands = MakeShareable(new FUICommandList);

	PluginCommands->MapAction(
		FLevelExporterCommands::Get().SettingsAction,
		FExecuteAction::CreateRaw(this, &FLevelExporterModule::PluginSettingsButtonClicked),
		FCanExecuteAction());

	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FLevelExporterModule::RegisterMenus));
}

void FLevelExporterModule::ShutdownModule()
{
	UToolMenus::UnRegisterStartupCallback(this);
	UToolMenus::UnregisterOwner(this);
	FLevelExporterStyle::Shutdown();
	FLevelExporterCommands::Unregister();
}

void FLevelExporterModule::PluginSettingsButtonClicked()
{
	FPropertyEditorModule& propertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
	FDetailsViewArgs detailsViewArgs;
	detailsViewArgs.bAllowSearch = false;
	detailsViewArgs.NameAreaSettings = FDetailsViewArgs::HideNameArea;
	TSharedPtr<class IDetailsView> detailsView = propertyEditorModule.CreateDetailView(detailsViewArgs);
	if (!Settings)
	{
		Settings = NewObject<UExportSettings>();
	}
	detailsView->SetObject(Settings);
	TSharedPtr<SBox> inspectorBox;
	TSharedRef<SWindow> window = SNew(SWindow)
		.Title(LOCTEXT("ExportSettingsTitle", "Level exporter"))
		.SupportsMinimize(false)
		.SupportsMaximize(false)
		.ClientSize(FVector2D(450.0f, 300.0f))
		.SizingRule(ESizingRule::UserSized)
		[
			SNew(SVerticalBox)
			+SVerticalBox::Slot()
			.FillHeight(1.0f)
			.Padding(2)
			[
				SAssignNew(inspectorBox, SBox)
			]
			+SVerticalBox::Slot()
			.AutoHeight()
			.Padding(2)
			.VAlign(EVerticalAlignment::VAlign_Bottom)
			[
				//cba with details panel customisation, so let's just put the browse button below the panel :/ 
				SNew(SHorizontalBox)
				+SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SButton)
					.Text(LOCTEXT("FolderPickerButton", "Browse"))
					.OnClicked_Lambda([this]()->FReply { Settings->ShowFolderPicker(); return FReply::Handled(); })
				]
				+SHorizontalBox::Slot()
				.FillWidth(1.0f)
				[
				SNew(SButton)
				.Text(LOCTEXT("StartExportingButton", "Export"))
				.HAlign(EHorizontalAlignment::HAlign_Center)
				.OnClicked_Lambda([this]()->FReply { ExportLevelData(); return FReply::Handled(); })
				]
			]
		];

	window->GetOnWindowClosedEvent().AddLambda([this](const TSharedRef<SWindow>& w) { Settings->SaveConfig(); });
	inspectorBox->SetContent(detailsView->AsShared());
	GEditor->EditorAddModalWindow(window);
}

void FLevelExporterModule::RegisterMenus()
{
	// Owner will be used for cleanup in call to UToolMenus::UnregisterOwner
	FToolMenuOwnerScoped OwnerScoped(this);
	{
		UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar");
		{
			FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("Content");
			{
				FToolMenuEntry& Entry = Section.AddEntry(FToolMenuEntry::InitToolBarButton(FLevelExporterCommands::Get().SettingsAction, LOCTEXT("ExportLevel_Label", "Export Level")));
				Entry.SetCommandList(PluginCommands);
			}
		}
	}
}

void FLevelExporterModule::ExportLevelData()
{
	if (!Settings)
	{
		Settings = NewObject<UExportSettings>();
	}
	bool success = false;
	{
		FScopedSlowTask slowTask(1.0f, LOCTEXT("ExportingLevel", "Exporting Level..."));
		slowTask.MakeDialog(false, true);
		FExportData data;
		//gather everything
		if (GEditor)
		{
			UWorld* world = GEditor->GetEditorWorldContext().World();
			for (TActorIterator<AActor> actorIt(world); actorIt; ++actorIt)
			{
				if (AActor* actor = *actorIt)
				{
					TArray<UStaticMeshComponent*> staticMeshes;
					actor->GetComponents<UStaticMeshComponent>(staticMeshes);
					for (UStaticMeshComponent* meshComponent : staticMeshes)
					{
						if (meshComponent)
						{
							if (UStaticMesh* sm = meshComponent->GetStaticMesh())
							{
								FExportMesh& meshData = data[sm];
								if (UInstancedStaticMeshComponent* instanced = Cast< UInstancedStaticMeshComponent>(meshComponent))
								{
									for (int i = 0; i < instanced->GetInstanceCount(); ++i)
									{
										FTransform t;
										instanced->GetInstanceTransform(i, t, true);
										meshData.InstanceTransforms.Add(FTransformWithEuler(t, Settings->Scale));
									}
								}
								else
								{
									meshData.Transforms.Add(FTransformWithEuler(meshComponent->GetComponentTransform(), Settings->Scale));
								}
								
								for (UMaterialInterface* material : meshComponent->GetMaterials())
								{
									if (material)
									{
										TArray <UTexture*> usedTextures;
										material->GetUsedTextures(usedTextures, EMaterialQualityLevel::Num, true, ERHIFeatureLevel::Num, true);
										for (UTexture* texture : usedTextures)
										{
											if (texture)
											{
												data.AddTexture(texture);
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
		//export
		FString jsonString;
		data.PrepareForExport();
		FJsonObjectConverter::UStructToJsonObjectString(data, jsonString, false, false, true);
		FFileHelper::SaveStringToFile(jsonString, *FPaths::Combine(Settings->DestinationFolder, TEXT("scene.json")));

		UExporter* modelExporter = nullptr;
		UExporter* textureExporter = nullptr;

		FString modelFileFormat = UExportSettings::EnumToString(TEXT("EModelFormat"), (uint8)Settings->ModelFormat);
		FString textureFileFormat = UExportSettings::EnumToString(TEXT("ETextureFormat"), (uint8)Settings->TextureFormat);

		TArray<UAssetExportTask*> tasks;
		for (const TPair<UObject*, const FString>& pair : data.GetModels())
		{
			if (!modelExporter)
			{
				modelExporter = UExporter::FindExporter(pair.Key, *modelFileFormat);
				check(modelExporter);
				modelExporter->SetBatchMode(true);
			}
			FString path = FPaths::Combine(Settings->DestinationFolder, FPaths::SetExtension(pair.Value, modelFileFormat));
			tasks.Add(NewTask(pair.Key, modelExporter, path));
		}
		for (const TPair<UObject*, const FString>& pair : data.GetTextures())
		{
			if (!textureExporter)
			{
				textureExporter = UExporter::FindExporter(pair.Key, *textureFileFormat);
				check(textureExporter);
				textureExporter->SetBatchMode(true);
			}
			FString path = FPaths::Combine(Settings->DestinationFolder, FPaths::SetExtension(pair.Value, textureFileFormat));
			tasks.Add(NewTask(pair.Key, textureExporter, path));
		}
		success = UExporter::RunAssetExportTasks(tasks);
	}
	FText dialogText = success ? LOCTEXT("ExportSuccess", "Export success.") : LOCTEXT("ExportFail", "Export failed.");
	FMessageDialog::Open(EAppMsgType::Ok, dialogText);
}

UAssetExportTask* FLevelExporterModule::NewTask(UObject* ObjectToExport, UExporter* Exporter, const FString& Path)
{
	if (ObjectToExport && Exporter && Settings && !Path.IsEmpty())
	{
		UAssetExportTask* assetExportTask = NewObject<UAssetExportTask>();
		assetExportTask->Object = ObjectToExport;
		assetExportTask->Exporter = Exporter;
		assetExportTask->Filename = Path;
		assetExportTask->bAutomated = true;
		assetExportTask->bPrompt = false;
		assetExportTask->bReplaceIdentical = true;
		return assetExportTask;
	}
	return nullptr;
}

void FLevelExporterModule::AddReferencedObjects(FReferenceCollector& Collector)
{
	if (Settings)
	{
		Collector.AddReferencedObject(Settings);
	}
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FLevelExporterModule, LevelExporter)