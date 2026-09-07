#include "Rules/LintRule_UE5_IsNamedCorrectly.h"

#include "LintRuleSet.h"
#include "LinterNamingConvention.h"

ULintRule_UE5_IsNamedCorrectly::ULintRule_UE5_IsNamedCorrectly(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bRequiresGameThread = true;
	RuleGroup = TEXT("UE5.Naming");
	RuleTitle = NSLOCTEXT("UE5Linter", "NamingTitle", "UE5 asset naming convention");
	RuleDescription = NSLOCTEXT("UE5Linter", "NamingDescription", "Use the configured prefix and suffix for this asset class. Prefixes are project conventions, not engine requirements.");
	RuleSeverity = ELintRuleSeverity::Warning;
}

bool ULintRule_UE5_IsNamedCorrectly::PassesRule_Internal_Implementation(UObject* ObjectToLint, const ULintRuleSet* ParentRuleSet, TArray<FLintRuleViolation>& OutRuleViolations) const
{
	const ULinterNamingConvention* Convention = ParentRuleSet->GetNamingConvention();
	if (Convention == nullptr)
	{
		return true;
	}

	// Inspect the native asset class before considering a Blueprint's ParentClass.
	// This prevents Control Rig from inheriting BP_ and MetaSound Source from A_.
	const TArray<FLinterNamingConventionInfo> Names = Convention->GetNamingConventionsForClassVariant(ObjectToLint->GetClass(), GetRuleBasedObjectVariantName(ObjectToLint));
	if (Names.IsEmpty())
	{
		return Super::PassesRule_Internal_Implementation(ObjectToLint, ParentRuleSet, OutRuleViolations);
	}

	const FString AssetName = ObjectToLint->GetName();
	for (const FLinterNamingConventionInfo& Name : Names)
	{
		const bool bPrefixMatches = Name.Prefix.IsEmpty() || AssetName.StartsWith(Name.Prefix, ESearchCase::CaseSensitive);
		const bool bSuffixMatches = Name.Suffix.IsEmpty() || AssetName.EndsWith(Name.Suffix, ESearchCase::CaseSensitive);
		if (bPrefixMatches && bSuffixMatches)
		{
			return true;
		}
	}

	const FString SuggestedName = BuildSuggestedName(AssetName, Names[0].Prefix, Names[0].Suffix);
	OutRuleViolations.Emplace(ObjectToLint, GetClass(), FText::FormatOrdered(
		NSLOCTEXT("UE5Linter", "NamingAction", "Recommended name: [{0}]. Edit the naming convention if your project uses a different prefix."),
		FText::FromString(SuggestedName)));
	return false;
}
