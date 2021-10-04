#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "LevelExporterStyle.h"

class FLevelExporterCommands : public TCommands<FLevelExporterCommands>
{
public:
	FLevelExporterCommands()
		: TCommands<FLevelExporterCommands>(TEXT("LevelExporter"), NSLOCTEXT("Contexts", "LevelExporter", "LevelExporter Plugin"), NAME_None, FLevelExporterStyle::GetStyleSetName())
	{
	}
	// TCommands<> interface
	virtual void RegisterCommands() override;
public:
	TSharedPtr< FUICommandInfo > SettingsAction;
};
