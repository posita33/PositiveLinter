#pragma once

#include "Engine/Blueprint.h"
#include "LinterNamingConvention.h"
#include "UE5LintRuleSet.h"
#include "UE5LinterAutomationTypes.generated.h"

/** Private fixtures for exercising configurable rule-set behavior. */
UCLASS(Transient, HideDropdown, NotBlueprintable)
class UUE5LinterAutomationRuleSet : public UUE5LintRuleSet
{
	GENERATED_BODY()

public:
	UUE5LinterAutomationRuleSet(const FObjectInitializer& ObjectInitializer)
		: Super(ObjectInitializer)
	{
	}

	void SetNamingConvention(ULinterNamingConvention* Convention)
	{
		NamingConvention = Convention;
	}

	void SetProjectClassRules(UClass* Class, const FLintRuleList& Rules)
	{
		ClassLintRulesMap.Add(Class, Rules);
	}
};

/** Simulates a native Blueprint asset type such as Control Rig without its plugin. */
UCLASS(Transient, HideDropdown, NotBlueprintable)
class UUE5LinterAutomationBlueprint : public UBlueprint
{
	GENERATED_BODY()

public:
	UUE5LinterAutomationBlueprint(const FObjectInitializer& ObjectInitializer)
		: Super(ObjectInitializer)
	{
	}
};
