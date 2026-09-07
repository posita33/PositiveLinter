// Copyright 2019-2020 Gamemakin LLC. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "HAL/ThreadSafeCounter.h"
#include "HAL/Runnable.h"
#include "AssetRegistry/AssetData.h"
#include "Linter.h"
#include "LintRuleSet.h"

class FLintRunner : public FRunnable
{

public:

	FLintRunner(UObject* InLoadedObject, const ULintRuleSet* LintRuleSet, TArray<FLintRuleViolation>* InpOutRuleViolations, FScopedSlowTask* InParentScopedSlowTask);

	virtual bool RequiresGamethread();

	virtual bool Init() override;
	virtual uint32 Run() override;
	virtual void Stop() override;
	virtual void Exit() override;

protected:
	UObject* LoadedObject = nullptr;
	const ULintRuleSet* RuleSet = nullptr;
	TArray<FLintRuleViolation>* pOutRuleViolations;

	// Own the resolved list: derived rule sets may compose it dynamically.
	FLintRuleList LoadedRuleList;
	static FCriticalSection LintDataUpdateLock;

	FScopedSlowTask* ParentScopedSlowTask;
};

