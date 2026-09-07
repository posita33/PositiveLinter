#pragma once

#include "CoreMinimal.h"
#include "GamemakinNamingConvention.h"
#include "UE5NamingConvention.generated.h"

/** Gamemakin naming conventions extended with modern Unreal Engine asset types. */
UCLASS(BlueprintType)
class UE5LINTER_API UUE5NamingConvention : public UGamemakinNamingConvention
{
	GENERATED_BODY()

public:
	UUE5NamingConvention(const FObjectInitializer& ObjectInitializer);
};
