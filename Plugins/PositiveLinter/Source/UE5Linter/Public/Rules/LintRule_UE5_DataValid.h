#pragma once

#include "CoreMinimal.h"
#include "AssetRegistry/AssetData.h"
#include "LintRule.h"
#include "LintRule_UE5_DataValid.generated.h"

/** Runs the asset class's native validation; it does not run registered editor validators. */
UCLASS(BlueprintType, Blueprintable)
class UE5LINTER_API ULintRule_UE5_DataValid : public ULintRule
{
	GENERATED_BODY()

public:
	ULintRule_UE5_DataValid(const FObjectInitializer& ObjectInitializer);

	/** Treat native validation warnings as violations as well as errors. */
	UPROPERTY(EditDefaultsOnly, Category = "Settings")
	bool bFailOnWarnings = false;

protected:
	virtual bool PassesRule_Internal_Implementation(UObject* ObjectToLint, const ULintRuleSet* ParentRuleSet, TArray<FLintRuleViolation>& OutRuleViolations) const override;
};
