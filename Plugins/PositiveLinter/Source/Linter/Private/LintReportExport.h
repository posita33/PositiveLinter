// Copyright PositiveLinter contributors. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"

class FJsonObject;

struct FLintReportMetadata
{
	FString Project;
	FString GeneratedAt = FDateTime::UtcNow().ToIso8601();
	FString RuleSet;
	TArray<FString> Paths;
};

/** Shared HTML export for the editor and commandlet. The standalone JSON format is unchanged. */
class FLintReportExport
{
public:
	static FString BuildCompactJson(const TSharedRef<FJsonObject>& LegacyReport, const FLintReportMetadata& Metadata);
	static FString RenderHtml(const FString& HtmlTemplate, const TSharedRef<FJsonObject>& LegacyReport, const FLintReportMetadata& Metadata);
	static bool CreateHtmlReport(const TSharedRef<FJsonObject>& LegacyReport, const FLintReportMetadata& Metadata, FString& OutHtml, FString& OutError);
};
