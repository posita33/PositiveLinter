#pragma once

#include "LintRuleSet.h"
#include "UE5LintRuleSet.generated.h"

/** UE5 asset rules with the original UE4 style rules as a fallback. */
UCLASS(BlueprintType, Blueprintable)
class UE5LINTER_API UUE5LintRuleSet : public ULintRuleSet
{
	GENERATED_BODY()

public:
	UUE5LintRuleSet(const FObjectInitializer& ObjectInitializer);

	virtual FLintRuleList GetResolvedLintRulesForClass(UClass* Class) const override;
	virtual ULinterNamingConvention* GetNamingConvention() const override;

	/** Rules for optional plugin classes. Only the loaded asset's hierarchy is inspected. */
	UPROPERTY(EditDefaultsOnly, Category = "Rules|UE5", meta = (AllowAbstract = "true"))
	TMap<TSoftClassPtr<UObject>, FLintRuleList> UE5ClassLintRulesMap;

	/** Used for classes not listed above. A UE5LintRuleSet cannot itself be a legacy fallback. */
	UPROPERTY(EditDefaultsOnly, Category = "Rules|UE5")
	TSoftObjectPtr<ULintRuleSet> LegacyRuleSet;
};
