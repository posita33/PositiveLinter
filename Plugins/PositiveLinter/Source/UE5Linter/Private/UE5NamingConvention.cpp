#include "UE5NamingConvention.h"

#include "UE5AssetTypes.h"

UUE5NamingConvention::UUE5NamingConvention(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	for (const FUE5AssetType& AssetType : GetUE5AssetTypes())
	{
		ClassNamingConventions.Emplace(
			TSoftClassPtr<UObject>(FSoftObjectPath(AssetType.ClassPath)),
			AssetType.Prefix);
	}

	SortConventions();
}
