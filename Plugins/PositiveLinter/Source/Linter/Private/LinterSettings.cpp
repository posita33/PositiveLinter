// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "LinterSettings.h"
#include "LintRuleSet.h"

namespace
{
	const TCHAR* DefaultUE5RuleSetPath = TEXT("/PositiveLinter/UE5Linter/UE5LintRuleSet.UE5LintRuleSet");
}

ULinterSettings::ULinterSettings(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	if (DefaultLintRuleSet.IsNull())
	{
		// Keep this as a soft reference: Linter starts before the UE5Linter module,
		// while the rule-set asset is loaded later when the wizard or commandlet runs.
		DefaultLintRuleSet = TSoftObjectPtr<ULintRuleSet>(FSoftObjectPath(DefaultUE5RuleSetPath));
	}
}
