#include "Rules/LintRule_UE5_DataValid.h"

#include "Misc/DataValidation.h"

#define LOCTEXT_NAMESPACE "UE5Linter"

ULintRule_UE5_DataValid::ULintRule_UE5_DataValid(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	RuleGroup = TEXT("UE5.DataValidation");
	RuleTitle = LOCTEXT("NativeDataValidationTitle", "Assets must pass native data validation");
	RuleDescription = LOCTEXT("NativeDataValidationDescription", "Reports errors returned by the asset class's UObject::IsDataValid implementation. Assets without a native validator are skipped. Registered Editor Validator assets are not executed. MetaSound validates document identity and page settings here; this is not a MetaSound graph compilation check.");
	RuleSeverity = ELintRuleSeverity::Error;
	bRequiresGameThread = true;
}

bool ULintRule_UE5_DataValid::PassesRule_Internal_Implementation(UObject* ObjectToLint, const ULintRuleSet* ParentRuleSet, TArray<FLintRuleViolation>& OutRuleViolations) const
{
	if (ObjectToLint == nullptr)
	{
		return true;
	}

	if (!IsInGameThread())
	{
		OutRuleViolations.Emplace(ObjectToLint, GetClass(), LOCTEXT("NativeDataValidationWrongThread", "Run native asset validation on the game thread. Enable Requires Game Thread on the rule."));
		return false;
	}

	const EDataValidationUsecase Usecase = IsRunningCommandlet() ? EDataValidationUsecase::Commandlet : EDataValidationUsecase::Manual;
	FDataValidationContext Context(false, Usecase, TConstArrayView<FAssetData>());
	const EDataValidationResult Result = static_cast<const UObject*>(ObjectToLint)->IsDataValid(Context);

	TArray<FText> Warnings;
	TArray<FText> Errors;
	Context.SplitIssues(Warnings, Errors);
	for (const FText& Error : Errors)
	{
		OutRuleViolations.Emplace(ObjectToLint, GetClass(), Error);
	}

	if (bFailOnWarnings)
	{
		for (const FText& Warning : Warnings)
		{
			OutRuleViolations.Emplace(ObjectToLint, GetClass(), Warning);
		}
	}

	// Some validators return Invalid without emitting a diagnostic. Preserve that
	// failure, while NotValidated with no errors remains a deliberate skip.
	if (Result == EDataValidationResult::Invalid && OutRuleViolations.IsEmpty())
	{
		OutRuleViolations.Emplace(ObjectToLint, GetClass(), LOCTEXT("NativeDataValidationFailed", "This asset's native data validator reported invalid data without a diagnostic. Open the asset and inspect its validation settings."));
	}

	return OutRuleViolations.IsEmpty();
}

#undef LOCTEXT_NAMESPACE
