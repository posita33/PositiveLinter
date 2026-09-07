#include "UE5LintRuleSet.h"

#include "AnyObject_LinterDummyClass.h"
#include "UE5AssetTypes.h"
#include "UE5NamingConvention.h"
#include "Rules/LintRule_UE5_IsNamedCorrectly.h"
#include "Rules/LintRule_UE5_DataValid.h"
#include "Rules/LintRule_NiagaraSystem_Compiles.h"

DEFINE_LOG_CATEGORY_STATIC(LogUE5Linter, Log, All);

UUE5LintRuleSet::UUE5LintRuleSet(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	RuleSetDescription = NSLOCTEXT("UE5Linter", "RuleSetDescription", "PositiveLinter UE5 Style Guide (UE 5.7.4)");
	NameForCommandlet = TEXT("ue5.style");
	LegacyRuleSet = TSoftObjectPtr<ULintRuleSet>(FSoftObjectPath(TEXT("/PositiveLinter/GamemakinLinter/GamemakinLinterRuleSet.GamemakinLinterRuleSet")));

	for (const FUE5AssetType& AssetType : GetUE5AssetTypes())
	{
		FLintRuleList& Rules = UE5ClassLintRulesMap.Add(TSoftClassPtr<UObject>(FSoftObjectPath(AssetType.ClassPath)));
		Rules.LintRules.Add(ULintRule_UE5_IsNamedCorrectly::StaticClass());
		Rules.LintRules.Add(ULintRule_UE5_DataValid::StaticClass());
		if (FCString::Strcmp(AssetType.ClassPath, TEXT("/Script/Niagara.NiagaraSystem")) == 0)
		{
			Rules.LintRules.Add(ULintRule_NiagaraSystem_Compiles::StaticClass());
		}
	}
}

ULinterNamingConvention* UUE5LintRuleSet::GetNamingConvention() const
{
	ULinterNamingConvention* CustomConvention = Super::GetNamingConvention();
	return CustomConvention != nullptr ? CustomConvention : GetMutableDefault<UUE5NamingConvention>();
}

FLintRuleList UUE5LintRuleSet::GetResolvedLintRulesForClass(UClass* Class) const
{
	// Called on the game thread when the runner is constructed. Prepare the
	// native naming defaults before any legacy worker can read them.
	GetNamingConvention();
	for (UClass* SearchClass = Class; SearchClass != nullptr; SearchClass = SearchClass->GetSuperClass())
	{
		// Explicit project entries take precedence over the same-class preset.
		if (const FLintRuleList* Rules = ClassLintRulesMap.Find(SearchClass))
		{
			return *Rules;
		}
		const TSoftClassPtr<UObject> ClassPath{FSoftObjectPath(SearchClass)};
		if (const FLintRuleList* Rules = UE5ClassLintRulesMap.Find(ClassPath))
		{
			return *Rules;
		}
	}

	if (const FLintRuleList* Fallback = ClassLintRulesMap.Find(UAnyObject_LinterDummyClass::StaticClass()))
	{
		return *Fallback;
	}

	if (ULintRuleSet* Legacy = LegacyRuleSet.LoadSynchronous())
	{
		// Prevent self references and cycles through other UE5 presets.
		if (!Legacy->IsA<UUE5LintRuleSet>())
		{
			return Legacy->GetResolvedLintRulesForClass(Class);
		}
		UE_LOG(LogUE5Linter, Warning, TEXT("LegacyRuleSet must not reference another UE5LintRuleSet (%s)."), *Legacy->GetPathName());
	}
	return Super::GetResolvedLintRulesForClass(Class);
}
