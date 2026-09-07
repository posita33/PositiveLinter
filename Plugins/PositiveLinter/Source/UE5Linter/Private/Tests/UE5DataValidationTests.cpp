#include "Tests/UE5ValidationTestAsset.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "AssetRegistry/AssetData.h"
#include "LintRuleSet.h"
#include "Misc/AutomationTest.h"
#include "Rules/LintRule_NiagaraSystem_Compiles.h"
#include "Rules/LintRule_UE5_DataValid.h"

namespace UE5DataValidationTests
{
	struct FFixture
	{
		ULintRule_UE5_DataValid* Rule = NewObject<ULintRule_UE5_DataValid>();
		ULintRuleSet* RuleSet = NewObject<ULintRuleSet>();
		UUE5ValidationTestAsset* Asset = NewObject<UUE5ValidationTestAsset>();
		TArray<FLintRuleViolation> Violations;

		bool Validate()
		{
			return Rule->PassesRule(Asset, RuleSet, Violations);
		}
	};
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FUE5NativeValidationValidTest, "PositiveLinter.UE5.Validation.Valid", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FUE5NativeValidationValidTest::RunTest(const FString& Parameters)
{
	UE5DataValidationTests::FFixture Fixture;
	Fixture.Asset->ValidationResult = EDataValidationResult::Valid;
	TestTrue(TEXT("A valid native result passes"), Fixture.Validate());
	TestEqual(TEXT("A valid result creates no violations"), Fixture.Violations.Num(), 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FUE5NativeValidationInvalidTest, "PositiveLinter.UE5.Validation.InvalidWithDiagnostic", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FUE5NativeValidationInvalidTest::RunTest(const FString& Parameters)
{
	UE5DataValidationTests::FFixture Fixture;
	Fixture.Asset->ValidationResult = EDataValidationResult::Invalid;
	Fixture.Asset->ValidationErrors.Add(FText::FromString(TEXT("Required reference is missing.")));
	TestFalse(TEXT("An invalid native result fails"), Fixture.Validate());
	TestEqual(TEXT("One native error creates one violation"), Fixture.Violations.Num(), 1);
	if (Fixture.Violations.Num() == 1)
	{
		TestEqual(TEXT("The native diagnostic is preserved"), Fixture.Violations[0].RecommendedAction.ToString(), FString(TEXT("Required reference is missing.")));
		TestTrue(TEXT("The violation identifies the asset"), Fixture.Violations[0].Violator.Get() == Fixture.Asset);
		TestTrue(TEXT("The violation identifies the rule"), Fixture.Violations[0].ViolatedRule == Fixture.Rule->GetClass());
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FUE5NativeValidationFallbackTest, "PositiveLinter.UE5.Validation.InvalidWithoutDiagnostic", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FUE5NativeValidationFallbackTest::RunTest(const FString& Parameters)
{
	UE5DataValidationTests::FFixture Fixture;
	Fixture.Asset->ValidationResult = EDataValidationResult::Invalid;
	TestFalse(TEXT("An invalid result without a message still fails"), Fixture.Validate());
	TestEqual(TEXT("A fallback violation is generated"), Fixture.Violations.Num(), 1);
	if (Fixture.Violations.Num() == 1)
	{
		TestFalse(TEXT("The fallback diagnostic is nonempty"), Fixture.Violations[0].RecommendedAction.IsEmpty());
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FUE5NativeValidationSkipTest, "PositiveLinter.UE5.Validation.NotValidated", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FUE5NativeValidationSkipTest::RunTest(const FString& Parameters)
{
	UE5DataValidationTests::FFixture Fixture;
	Fixture.Asset->ValidationResult = EDataValidationResult::NotValidated;
	TestTrue(TEXT("An asset with no native validator is skipped"), Fixture.Validate());
	TestEqual(TEXT("Skipped validation creates no violations"), Fixture.Violations.Num(), 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FUE5NativeValidationWarningsTest, "PositiveLinter.UE5.Validation.WarningPolicy", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FUE5NativeValidationWarningsTest::RunTest(const FString& Parameters)
{
	UE5DataValidationTests::FFixture Fixture;
	Fixture.Asset->ValidationResult = EDataValidationResult::Valid;
	Fixture.Asset->ValidationWarnings.Add(FText::FromString(TEXT("Optional value is not configured.")));
	TestTrue(TEXT("Native warnings pass with the default policy"), Fixture.Validate());
	TestEqual(TEXT("Ignored warnings create no violations"), Fixture.Violations.Num(), 0);

	Fixture.Rule->bFailOnWarnings = true;
	TestFalse(TEXT("Native warnings fail when requested"), Fixture.Validate());
	TestEqual(TEXT("One warning creates one violation"), Fixture.Violations.Num(), 1);
	if (Fixture.Violations.Num() == 1)
	{
		TestEqual(TEXT("The warning diagnostic is preserved"), Fixture.Violations[0].RecommendedAction.ToString(), FString(TEXT("Optional value is not configured.")));
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FUE5NiagaraWrongAssetTest, "PositiveLinter.UE5.Validation.NiagaraSkipsOtherAssets", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FUE5NiagaraWrongAssetTest::RunTest(const FString& Parameters)
{
	ULintRule_NiagaraSystem_Compiles* Rule = NewObject<ULintRule_NiagaraSystem_Compiles>();
	ULintRuleSet* RuleSet = NewObject<ULintRuleSet>();
	UUE5ValidationTestAsset* Asset = NewObject<UUE5ValidationTestAsset>();
	TArray<FLintRuleViolation> Violations;
	TestTrue(TEXT("A non-Niagara asset is skipped without compilation"), Rule->PassesRule(Asset, RuleSet, Violations));
	TestEqual(TEXT("The wrong asset type creates no violations"), Violations.Num(), 0);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
