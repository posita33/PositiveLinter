#pragma once

#include "CoreMinimal.h"
#include "AssetRegistry/AssetData.h"
#include "LintRule.h"
#include "LintRule_NiagaraSystem_Compiles.generated.h"

/** Checks Niagara system validity after pending VM and GPU compilation completes. */
UCLASS(BlueprintType, Blueprintable)
class UE5LINTER_API ULintRule_NiagaraSystem_Compiles : public ULintRule
{
	GENERATED_BODY()

public:
	ULintRule_NiagaraSystem_Compiles(const FObjectInitializer& ObjectInitializer);

protected:
	virtual bool PassesRule_Internal_Implementation(UObject* ObjectToLint, const ULintRuleSet* ParentRuleSet, TArray<FLintRuleViolation>& OutRuleViolations) const override;
};
