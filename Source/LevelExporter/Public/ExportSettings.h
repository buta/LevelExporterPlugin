#pragma once

#include "CoreMinimal.h"
#include "ExportSettings.generated.h"

UENUM(BlueprintType)
enum class ETextureFormat : uint8
{
	TGA		,
	BMP		,
	Num		UMETA(Hidden)
};
UENUM(BlueprintType)
enum class EModelFormat : uint8
{
	FBX		,
	OBJ		,
	Num		UMETA(Hidden)
};
UENUM(BlueprintType)
enum class EScale : uint8
{
	CM		UMETA(Centimeters),
	M		UMETA(Meters),
	Num		UMETA(Hidden)
};
//
UCLASS(config = EditorPerProjectUserSettings, MinimalAPI, BlueprintType)
class UExportSettings : public UObject
{
	GENERATED_BODY()
public:
	UExportSettings();
	//Unreal uses cm
	UPROPERTY(config, EditAnywhere, Category = "Export settings")
	EScale Scale = EScale::M;
	UPROPERTY(config, EditAnywhere, Category = "Export settings")
	ETextureFormat TextureFormat = ETextureFormat::TGA;
	UPROPERTY(config, EditAnywhere, Category = "Export settings")
	EModelFormat ModelFormat = EModelFormat::FBX;
	UPROPERTY(config, EditAnywhere, Category = "Export settings")
	FString DestinationFolder;

	void ShowFolderPicker();
	static const FString EnumToString(const TCHAR* Enum, uint8 EnumValue);
};
