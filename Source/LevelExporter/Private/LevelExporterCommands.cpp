
#include "LevelExporterCommands.h"

#define LOCTEXT_NAMESPACE "FLevelExporterModule"

void FLevelExporterCommands::RegisterCommands()
{
	UI_COMMAND(SettingsAction, "LevelExporterSettings", "Execute LevelExporterSettings action", EUserInterfaceActionType::Button, FInputGesture());
}

#undef LOCTEXT_NAMESPACE
