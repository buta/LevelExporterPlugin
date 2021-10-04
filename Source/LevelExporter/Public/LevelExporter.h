// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

#include "ExportSettings.h"

class UAssetExportTask;
class UExporter;
struct FExportData;

class FLevelExporterModule : public FGCObject, public IModuleInterface
{
public:
	/** FGCObject implementation */
	virtual void AddReferencedObjects(FReferenceCollector& Collector) override;
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	
private:
	void PluginSettingsButtonClicked();
	void RegisterMenus();
	void ExportLevelData();
	UAssetExportTask* NewTask(UObject* ObjectToExport, UExporter* Exporter, const FString& Path);

	UExportSettings* Settings;
	TSharedPtr<class FUICommandList> PluginCommands;
};
