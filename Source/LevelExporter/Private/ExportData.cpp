#include "ExportData.h"
#include "Internationalization/Regex.h"

FTransformWithEuler::FTransformWithEuler(const FTransform& Transform, const EScale& IScale)
{
	float scale = IScale == EScale::M ? 0.01f : 1.0f;
	Location = Transform.GetLocation() * scale;
	Rotation = Transform.Rotator().Euler();
	Scale = Transform.GetScale3D() * scale;
}

FString UniqueNamer::GenerateUniqueName(const UObject* Object) const
{
	check(Object);
	//SM_ and T_ is unreal's naming scheme, do we need it outside?
	static const FRegexPattern RemovePrefix(TEXT("(?:^SM_)?(?:^T_)?(.+?)$"));
	FRegexMatcher match(RemovePrefix, Object->GetFName().ToString());
	check(match.FindNext());
	FString name = match.GetCaptureGroup(1);
	if (NameLookup.Contains(name))
	{
		for (int i = 1; ; ++i)
		{
			FString newName = FString::Printf(TEXT("%s_%02d"), *name, i);
			if (!NameLookup.Contains(newName))
			{
				return newName;
			}
		}
	}
	return name;
}

void UniqueNamer::Add(UObject* Object)
{
	if (!NameMap.Contains(Object))
	{
		const FString name = GenerateUniqueName(Object);
		NameMap.Add(Object, name);
		NameLookup.Add(name);
	}
}

const FString* UniqueNamer::Get(const UObject* Object) const
{
	const FString* ret = NameMap.Find(Object);
	check(ret);
	return ret;
}

const TMap<UObject*, const FString>& UniqueNamer::GetMap() const
{
	return NameMap;
}

FExportMesh& FExportData::operator[](UStaticMesh* Key)
{
	FExportMesh* data = MeshDataMap.Find(Key);
	if (!data)
	{
		MeshDataMap.Add(Key);
		data = &MeshDataMap[Key];
		Models.Add(Key);
		data->Name = *Models.Get(Key);
	}
	return *data;
}

void FExportData::AddTexture(UObject* Texture)
{
	Textures.Add(Texture);
}

const TMap<UObject*, const FString>& FExportData::GetModels() const
{
	return Models.GetMap();
}

const TMap<UObject*, const FString>& FExportData::GetTextures() const
{
	return Textures.GetMap();
}

void FExportData::PrepareForExport()
{
	Meshes.Empty();
	MeshDataMap.GenerateValueArray(Meshes);
}
