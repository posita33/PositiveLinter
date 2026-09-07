#include "Tests/UE5LinterAutomationTypes.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Engine/DataAsset.h"
#include "Engine/Texture2D.h"
#include "LinterNamingConvention.h"
#include "Misc/AutomationTest.h"
#include "Modules/ModuleManager.h"
#include "NiagaraSystem.h"
#include "Rules/LintRule_UE5_IsNamedCorrectly.h"
#include "UE5AssetTypes.h"
#include "UE5NamingConvention.h"

namespace UE5LinterTests
{
	template <typename T>
	T* NewNamedObject(const TCHAR* Name)
	{
		return NewObject<T>(GetTransientPackage(), MakeUniqueObjectName(GetTransientPackage(), T::StaticClass(), FName(Name)));
	}

	UUE5LinterAutomationRuleSet* NewRuleSet()
	{
		UUE5LinterAutomationRuleSet* Rules = NewObject<UUE5LinterAutomationRuleSet>();
		Rules->LegacyRuleSet.Reset();
		return Rules;
	}

	FLintRuleList NamingRules()
	{
		FLintRuleList Rules;
		Rules.LintRules.Add(ULintRule_UE5_IsNamedCorrectly::StaticClass());
		return Rules;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FUE5LinterCatalogTest, "PositiveLinter.UE5.Catalog.SoftReferences", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FUE5LinterCatalogTest::RunTest(const FString& Parameters)
{
	const TConstArrayView<FUE5AssetType> Catalog = GetUE5AssetTypes();
	TestEqual(TEXT("The initial UE5 catalog contains 18 asset types"), Catalog.Num(), 18);

	TMap<FName, bool> LoadedModules;
	TSet<FSoftObjectPath> ClassPaths;
	for (const FUE5AssetType& AssetType : Catalog)
	{
		const FSoftObjectPath ClassPath(AssetType.ClassPath);
		TestTrue(FString::Printf(TEXT("Valid class path: %s"), AssetType.ClassPath), ClassPath.IsValid());
		TestFalse(FString::Printf(TEXT("Unique class path: %s"), AssetType.ClassPath), ClassPaths.Contains(ClassPath));
		ClassPaths.Add(ClassPath);
		FString ModuleName = ClassPath.GetLongPackageName();
		ModuleName.RemoveFromStart(TEXT("/Script/"));
		const FName Module(*ModuleName);
		LoadedModules.FindOrAdd(Module) = FModuleManager::Get().IsModuleLoaded(Module);
	}

	const UUE5NamingConvention* Convention = NewObject<UUE5NamingConvention>();
	for (const FUE5AssetType& AssetType : Catalog)
	{
		const FSoftObjectPath ClassPath(AssetType.ClassPath);
		const FLinterNamingConventionInfo* Entry = Convention->ClassNamingConventions.FindByPredicate(
			[&ClassPath](const FLinterNamingConventionInfo& Info)
			{
				return Info.SoftClassPtr.ToSoftObjectPath() == ClassPath && Info.Variant.IsNone();
			});
		if (TestNotNull(FString::Printf(TEXT("Naming entry exists: %s"), AssetType.ClassPath), Entry))
		{
			TestEqual(TEXT("Catalog prefix is reflected in naming defaults"), Entry->Prefix, FString(AssetType.Prefix));
		}
	}
	for (const TPair<FName, bool>& Module : LoadedModules)
	{
		TestEqual(FString::Printf(TEXT("Constructing naming defaults does not load %s"), *Module.Key.ToString()), FModuleManager::Get().IsModuleLoaded(Module.Key), Module.Value);
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FUE5LinterNiagaraNamingTest, "PositiveLinter.UE5.Naming.NiagaraPrefixes", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FUE5LinterNiagaraNamingTest::RunTest(const FString& Parameters)
{
	const ULintRule_UE5_IsNamedCorrectly* Rule = GetDefault<ULintRule_UE5_IsNamedCorrectly>();
	const UUE5LinterAutomationRuleSet* Rules = UE5LinterTests::NewRuleSet();
	UNiagaraSystem* Correct = UE5LinterTests::NewNamedObject<UNiagaraSystem>(TEXT("FXS_TestSystem"));
	UNiagaraSystem* Wrong = UE5LinterTests::NewNamedObject<UNiagaraSystem>(TEXT("PS_TestSystem"));
	TArray<FLintRuleViolation> Violations;

	TestTrue(TEXT("Niagara system accepts FXS_"), Rule->PassesRule(Correct, Rules, Violations));
	TestTrue(TEXT("A correct name has no violations"), Violations.IsEmpty());
	TestFalse(TEXT("Niagara system rejects legacy PS_"), Rule->PassesRule(Wrong, Rules, Violations));
	if (TestEqual(TEXT("An incorrect name creates one violation"), Violations.Num(), 1))
	{
		TestTrue(TEXT("Violation identifies the original asset"), Violations[0].Violator.Get() == Wrong);
		TestTrue(TEXT("Violation suggests the UE5 prefix"), Violations[0].RecommendedAction.ToString().Contains(TEXT("FXS_")));
		TestTrue(TEXT("Naming violations have Warning severity"), Violations[0].ViolatedRule.GetDefaultObject()->RuleSeverity == ELintRuleSeverity::Warning);
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FUE5LinterNativeBlueprintNamingTest, "PositiveLinter.UE5.Naming.NativeBlueprintClass", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FUE5LinterNativeBlueprintNamingTest::RunTest(const FString& Parameters)
{
	UUE5LinterAutomationRuleSet* Rules = UE5LinterTests::NewRuleSet();
	UUE5NamingConvention* Convention = NewObject<UUE5NamingConvention>();
	Convention->ClassNamingConventions.Reset();
	Convention->ClassNamingConventions.Emplace(TSoftClassPtr<UObject>(UUE5LinterAutomationBlueprint::StaticClass()), TEXT("CR_"));
	Convention->ClassNamingConventions.Emplace(TSoftClassPtr<UObject>(UObject::StaticClass()), TEXT("BP_"));
	Rules->SetNamingConvention(Convention);
	UUE5LinterAutomationBlueprint* Correct = UE5LinterTests::NewNamedObject<UUE5LinterAutomationBlueprint>(TEXT("CR_TestRig"));
	UUE5LinterAutomationBlueprint* Wrong = UE5LinterTests::NewNamedObject<UUE5LinterAutomationBlueprint>(TEXT("BP_TestRig"));
	Correct->ParentClass = UObject::StaticClass();
	Wrong->ParentClass = UObject::StaticClass();
	const ULintRule_UE5_IsNamedCorrectly* Rule = GetDefault<ULintRule_UE5_IsNamedCorrectly>();
	TArray<FLintRuleViolation> Violations;

	TestTrue(TEXT("The native Blueprint asset class prefix wins over ParentClass"), Rule->PassesRule(Correct, Rules, Violations));
	TestFalse(TEXT("The Blueprint ParentClass prefix does not hide the native asset convention"), Rule->PassesRule(Wrong, Rules, Violations));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FUE5LinterSubclassRulesTest, "PositiveLinter.UE5.Rules.SubclassLookup", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FUE5LinterSubclassRulesTest::RunTest(const FString& Parameters)
{
	UUE5LinterAutomationRuleSet* Rules = UE5LinterTests::NewRuleSet();
	Rules->UE5ClassLintRulesMap.Reset();
	Rules->UE5ClassLintRulesMap.Add(TSoftClassPtr<UObject>(UDataAsset::StaticClass()), UE5LinterTests::NamingRules());
	const FLintRuleList Resolved = Rules->GetResolvedLintRulesForClass(UPrimaryDataAsset::StaticClass());
	if (TestEqual(TEXT("A derived asset resolves its base-class rules"), Resolved.LintRules.Num(), 1))
	{
		TestTrue(TEXT("The inherited rule is the configured naming rule"), Resolved.LintRules[0] == ULintRule_UE5_IsNamedCorrectly::StaticClass());
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FUE5LinterEmptyOverrideTest, "PositiveLinter.UE5.Rules.EmptyOverride", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FUE5LinterEmptyOverrideTest::RunTest(const FString& Parameters)
{
	UUE5LinterAutomationRuleSet* Rules = UE5LinterTests::NewRuleSet();
	TestFalse(TEXT("Niagara has preset rules before an override"), Rules->GetResolvedLintRulesForClass(UNiagaraSystem::StaticClass()).LintRules.IsEmpty());
	Rules->SetProjectClassRules(UNiagaraSystem::StaticClass(), FLintRuleList());
	TestTrue(TEXT("An explicit empty project entry disables preset rules"), Rules->GetResolvedLintRulesForClass(UNiagaraSystem::StaticClass()).LintRules.IsEmpty());

	UUE5LinterAutomationRuleSet* SoftRules = UE5LinterTests::NewRuleSet();
	SoftRules->UE5ClassLintRulesMap.Add(TSoftClassPtr<UObject>(UNiagaraSystem::StaticClass()), FLintRuleList());
	TestTrue(TEXT("An empty soft-class entry also disables preset rules"), SoftRules->GetResolvedLintRulesForClass(UNiagaraSystem::StaticClass()).LintRules.IsEmpty());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FUE5LinterOwnedRuleListTest, "PositiveLinter.UE5.Rules.OwnedSnapshot", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FUE5LinterOwnedRuleListTest::RunTest(const FString& Parameters)
{
	UUE5LinterAutomationRuleSet* Rules = UE5LinterTests::NewRuleSet();
	const TSoftClassPtr<UObject> Key(UDataAsset::StaticClass());
	Rules->UE5ClassLintRulesMap.Reset();
	Rules->UE5ClassLintRulesMap.Add(Key, UE5LinterTests::NamingRules());
	const FLintRuleList Snapshot = Rules->GetResolvedLintRulesForClass(UPrimaryDataAsset::StaticClass());
	Rules->UE5ClassLintRulesMap.FindChecked(Key).LintRules.Reset();
	TestTrue(TEXT("A new lookup observes the edited rule map"), Rules->GetResolvedLintRulesForClass(UPrimaryDataAsset::StaticClass()).LintRules.IsEmpty());
	if (TestEqual(TEXT("A previously resolved list retains its owned rules"), Snapshot.LintRules.Num(), 1))
	{
		TestTrue(TEXT("The original rule remains available"), Snapshot.LintRules[0] == ULintRule_UE5_IsNamedCorrectly::StaticClass());
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FUE5LinterLegacyFallbackTest, "PositiveLinter.UE5.Rules.GamemakinFallback", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FUE5LinterLegacyFallbackTest::RunTest(const FString& Parameters)
{
	UUE5LintRuleSet* Rules = NewObject<UUE5LintRuleSet>();
	ULintRuleSet* Legacy = Rules->LegacyRuleSet.LoadSynchronous();
	if (!TestNotNull(TEXT("The bundled Gamemakin rule-set asset loads"), Legacy))
	{
		return false;
	}
	const FLintRuleList Expected = Legacy->GetResolvedLintRulesForClass(UTexture2D::StaticClass());
	const FLintRuleList Actual = Rules->GetResolvedLintRulesForClass(UTexture2D::StaticClass());
	TestFalse(TEXT("The legacy texture rule list is populated"), Expected.LintRules.IsEmpty());
	TestTrue(TEXT("UE5 fallback preserves the original Gamemakin texture rules and order"), Actual.LintRules == Expected.LintRules);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
