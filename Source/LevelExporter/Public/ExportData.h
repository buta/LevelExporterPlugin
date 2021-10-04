#pragma once

#include "CoreMinimal.h"
#include "ExportSettings.h"
#include "ExportData.generated.h"

class UStaticMesh;

USTRUCT()
struct FTransformWithEuler 
{
	GENERATED_BODY()
	FTransformWithEuler() {};
	FTransformWithEuler(const FTransform& Transform, const EScale& IScale);
	UPROPERTY()
	FVector Location;
	UPROPERTY()
	FVector Rotation;
	UPROPERTY()
	FVector Scale;
};

USTRUCT()
struct FExportMesh
{
	GENERATED_BODY()
	UPROPERTY()
	FString Name;
	UPROPERTY()
	TArray <FTransformWithEuler> Transforms;
	UPROPERTY()
	TArray<FTransformWithEuler> InstanceTransforms;
};

class UniqueNamer
{
	TMap<UObject*, const FString> NameMap;
	TSet<FString> NameLookup;
	FString GenerateUniqueName(const UObject* Object) const;
public:
	void Add(UObject* Object);
	const FString* Get(const UObject* Object) const;
	const TMap<UObject*, const FString>& GetMap() const;
};

USTRUCT()
struct FExportData
{
	GENERATED_BODY()
private:
	TMap<UStaticMesh*, FExportMesh> MeshDataMap;
	
	/*
	Assets with the same name could be found in different folders, so it's important
	we don't export two different meshes with the same name
	*/
	UniqueNamer Models;
	UniqueNamer Textures;
public:
	UPROPERTY()
	TArray<FExportMesh> Meshes;

	FExportMesh& operator[](UStaticMesh* Key);
	void AddTexture(UObject* Texture);
	const TMap<UObject*, const FString>& GetModels() const;
	const TMap<UObject*, const FString>& GetTextures() const;
	/*
	Put the export data from the map into the array to be exported into json
	*/
	void PrepareForExport();
};
