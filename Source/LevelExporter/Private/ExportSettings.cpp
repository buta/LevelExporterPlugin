#include "ExportSettings.h"

#include "Developer/DesktopPlatform/Public/IDesktopPlatform.h"
#include "Developer/DesktopPlatform/Public/DesktopPlatformModule.h"

UExportSettings::UExportSettings()
{
	//set default folder
}

void UExportSettings::ShowFolderPicker()
{
	IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
	if (DesktopPlatform)
	{
		DesktopPlatform->OpenDirectoryDialog(nullptr, TEXT("Select a folder"), DestinationFolder, DestinationFolder);
	}
}

const FString UExportSettings::EnumToString(const TCHAR* Enum, uint8 EnumValue)
{
	const UEnum* EnumPtr = FindObject<UEnum>(ANY_PACKAGE, Enum, true);
	if (!EnumPtr)
	{
		return NSLOCTEXT("Enum not found", "Enum not found", "Enum not found").ToString();
	}
	return EnumPtr->GetNameStringByIndex((int32)EnumValue);
}
