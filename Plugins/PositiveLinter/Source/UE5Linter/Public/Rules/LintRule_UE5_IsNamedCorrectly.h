#pragma once

#include "LintRules/LintRule_IsNamedCorrectly_Base.h"
#include "LintRule_UE5_IsNamedCorrectly.generated.h"

/** Checks the asset class first, including Blueprint-based assets such as Control Rig. */
UCLASS(BlueprintType, Blueprintable)
class UE5LINTER_API ULintRule_UE5_IsNamedCorrectly : public ULintRule_IsNamedCorrectly_Base
{
	GENERATED_BODY()

public:
	ULintRule_UE5_IsNamedCorrectly(const FObjectInitializer& ObjectInitializer);

protected:
	virtual bool PassesRule_Internal_Implementation(UObject* ObjectToLint, const ULintRuleSet* ParentRuleSet, TArray<FLintRuleViolation>& OutRuleViolations) const override;
};
