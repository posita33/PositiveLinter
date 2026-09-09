// Copyright PositiveLinter contributors. All Rights Reserved.
#include "LintReportExport.h"

#include "Dom/JsonObject.h"
#include "Dom/JsonValue.h"
#include "Interfaces/IPluginManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Policies/CondensedJsonPrintPolicy.h"
#include "Serialization/JsonSerializer.h"

namespace
{
	FString ReadString(const FJsonObject& Object, const TCHAR* Field)
	{
		FString Result;
		Object.TryGetStringField(Field, Result);
		return Result;
	}

	FString EscapeHtml(FString Value)
	{
		Value.ReplaceInline(TEXT("&"), TEXT("&amp;"));
		Value.ReplaceInline(TEXT("<"), TEXT("&lt;"));
		Value.ReplaceInline(TEXT(">"), TEXT("&gt;"));
		Value.ReplaceInline(TEXT("\""), TEXT("&quot;"));
		Value.ReplaceInline(TEXT("'"), TEXT("&#39;"));
		return Value;
	}

	FString EscapeScriptData(FString Json)
	{
		// application/json script elements still terminate at a literal </script> in HTML.
		Json.ReplaceInline(TEXT("<"), TEXT("\\u003c"));
		Json.ReplaceInline(TEXT(">"), TEXT("\\u003e"));
		Json.ReplaceInline(TEXT("&"), TEXT("\\u0026"));
		Json.ReplaceInline(TEXT("\u2028"), TEXT("\\u2028"));
		Json.ReplaceInline(TEXT("\u2029"), TEXT("\\u2029"));
		return Json;
	}

	TSharedPtr<FJsonValue> Number(int32 Value)
	{
		return MakeShared<FJsonValueNumber>(Value);
	}
}

FString FLintReportExport::BuildCompactJson(const TSharedRef<FJsonObject>& LegacyReport, const FLintReportMetadata& Metadata)
{
	TArray<TSharedPtr<FJsonValue>> Strings;
	TMap<FString, int32> StringIndices;
	const auto Intern = [&Strings, &StringIndices](const FString& Value) -> int32
	{
		if (const int32* Existing = StringIndices.Find(Value))
		{
			return *Existing;
		}
		const int32 Index = Strings.Add(MakeShared<FJsonValueString>(Value));
		StringIndices.Add(Value, Index);
		return Index;
	};

	TArray<TSharedPtr<FJsonValue>> Rules;
	TArray<TSharedPtr<FJsonValue>> Assets;
	TMap<FString, int32> RuleIndices;
	const TArray<TSharedPtr<FJsonValue>>* Violators = nullptr;
	if (LegacyReport->TryGetArrayField(TEXT("Violators"), Violators))
	{
		for (const TSharedPtr<FJsonValue>& ViolatorValue : *Violators)
		{
			const TSharedPtr<FJsonObject>* Asset = nullptr;
			if (!ViolatorValue.IsValid() || !ViolatorValue->TryGetObject(Asset) || !Asset->IsValid())
			{
				continue;
			}

			const FJsonObject& AssetObject = **Asset;
			const int32 NameIndex = Intern(ReadString(AssetObject, TEXT("ViolatorAssetName")));
			const int32 PathIndex = Intern(ReadString(AssetObject, TEXT("ViolatorAssetPath")));
			FString ClassName = ReadString(AssetObject, TEXT("ViolatorFullName"));
			int32 Separator = INDEX_NONE;
			if (ClassName.FindChar(TEXT(' '), Separator))
			{
				ClassName.LeftInline(Separator);
			}
			const int32 ClassIndex = Intern(ClassName);
			TArray<TSharedPtr<FJsonValue>> Issues;
			const TArray<TSharedPtr<FJsonValue>>* Violations = nullptr;
			if (AssetObject.TryGetArrayField(TEXT("Violations"), Violations))
			{
				for (const TSharedPtr<FJsonValue>& ViolationValue : *Violations)
				{
					const TSharedPtr<FJsonObject>* Rule = nullptr;
					if (!ViolationValue.IsValid() || !ViolationValue->TryGetObject(Rule) || !Rule->IsValid())
					{
						continue;
					}
					const FJsonObject& RuleObject = **Rule;
					const int32 Group = Intern(ReadString(RuleObject, TEXT("RuleGroup")));
					const int32 Title = Intern(ReadString(RuleObject, TEXT("RuleTitle")));
					const int32 Description = Intern(ReadString(RuleObject, TEXT("RuleDesc")));
					const int32 Url = Intern(ReadString(RuleObject, TEXT("RuleURL")));
					int32 Severity = 0;
					RuleObject.TryGetNumberField(TEXT("RuleSeverity"), Severity);
					// All five fields form the identity: similarly titled rules may differ in severity or guidance.
					const FString RuleKey = FString::Printf(TEXT("%d,%d,%d,%d,%d"), Group, Title, Description, Url, Severity);
					int32 RuleIndex;
					if (const int32* Existing = RuleIndices.Find(RuleKey))
					{
						RuleIndex = *Existing;
					}
					else
					{
						RuleIndex = Rules.Add(MakeShared<FJsonValueArray>(TArray<TSharedPtr<FJsonValue>>{
							Number(Group), Number(Title), Number(Description), Number(Url), Number(Severity)}));
						RuleIndices.Add(RuleKey, RuleIndex);
					}
					const int32 Action = Intern(ReadString(RuleObject, TEXT("RuleRecommendedAction")));
					Issues.Add(MakeShared<FJsonValueArray>(TArray<TSharedPtr<FJsonValue>>{Number(RuleIndex), Number(Action)}));
				}
			}
			Assets.Add(MakeShared<FJsonValueArray>(TArray<TSharedPtr<FJsonValue>>{
				Number(NameIndex), Number(PathIndex), Number(ClassIndex), MakeShared<FJsonValueArray>(MoveTemp(Issues))}));
		}
	}

	TArray<TSharedPtr<FJsonValue>> Paths;
	for (const FString& Path : Metadata.Paths)
	{
		Paths.Add(MakeShared<FJsonValueString>(Path));
	}
	const TSharedRef<FJsonObject> CompactReport = MakeShared<FJsonObject>();
	CompactReport->SetNumberField(TEXT("version"), 2);
	CompactReport->SetStringField(TEXT("project"), Metadata.Project);
	CompactReport->SetStringField(TEXT("generatedAt"), Metadata.GeneratedAt);
	CompactReport->SetStringField(TEXT("ruleSet"), Metadata.RuleSet);
	CompactReport->SetArrayField(TEXT("paths"), Paths);
	CompactReport->SetArrayField(TEXT("strings"), Strings);
	CompactReport->SetArrayField(TEXT("rules"), Rules);
	CompactReport->SetArrayField(TEXT("assets"), Assets);
	FString Json;
	const TSharedRef<TJsonWriter<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>> Writer =
		TJsonWriterFactory<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>::Create(&Json);
	FJsonSerializer::Serialize(CompactReport, Writer);
	return Json;
}

FString FLintReportExport::RenderHtml(const FString& HtmlTemplate, const TSharedRef<FJsonObject>& LegacyReport, const FLintReportMetadata& Metadata)
{
	const FString PayloadToken = TEXT("{% LINT_REPORT %}");
	const int32 PayloadPosition = HtmlTemplate.Find(PayloadToken, ESearchCase::CaseSensitive);
	const FString Title = EscapeHtml(Metadata.Project);
	// Replace placeholders in the template only; literal placeholder text in project names or data stays intact.
	FString Prefix = PayloadPosition == INDEX_NONE ? HtmlTemplate : HtmlTemplate.Left(PayloadPosition);
	Prefix.ReplaceInline(TEXT("{% TITLE %}"), *Title, ESearchCase::CaseSensitive);
	if (PayloadPosition == INDEX_NONE)
	{
		return Prefix;
	}
	FString Suffix = HtmlTemplate.Mid(PayloadPosition + PayloadToken.Len());
	Suffix.ReplaceInline(TEXT("{% TITLE %}"), *Title, ESearchCase::CaseSensitive);
	return Prefix + EscapeScriptData(BuildCompactJson(LegacyReport, Metadata)) + Suffix;
}

bool FLintReportExport::CreateHtmlReport(const TSharedRef<FJsonObject>& LegacyReport, const FLintReportMetadata& Metadata, FString& OutHtml, FString& OutError)
{
	OutHtml.Reset();
	OutError.Reset();
	const TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(TEXT("PositiveLinter"));
	if (!Plugin.IsValid())
	{
		OutError = TEXT("PositiveLinter plugin could not be found.");
		return false;
	}
	const FString TemplatePath = FPaths::Combine(Plugin->GetBaseDir(), TEXT("Resources/LintReportTemplate.html"));
	FString HtmlTemplate;
	if (!FFileHelper::LoadFileToString(HtmlTemplate, *TemplatePath))
	{
		OutError = FString::Printf(TEXT("Could not load HTML report template: %s"), *TemplatePath);
		return false;
	}
	if (!HtmlTemplate.Contains(TEXT("{% LINT_REPORT %}"), ESearchCase::CaseSensitive))
	{
		OutError = TEXT("The HTML report template is missing its data placeholder.");
		return false;
	}
	OutHtml = RenderHtml(HtmlTemplate, LegacyReport, Metadata);
	return true;
}
