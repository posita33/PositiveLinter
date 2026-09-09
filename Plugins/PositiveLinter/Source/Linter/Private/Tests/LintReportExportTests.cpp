// Copyright PositiveLinter contributors. All Rights Reserved.
#include "LintReportExport.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Dom/JsonObject.h"
#include "Dom/JsonValue.h"
#include "Misc/AutomationTest.h"
#include "Serialization/JsonSerializer.h"

namespace LintReportExportTests
{
	TSharedPtr<FJsonValue> MakeRule(int32 Severity, const FString& Action, const FString& Description = TEXT("Shared rule description"))
	{
		const TSharedRef<FJsonObject> Rule = MakeShared<FJsonObject>();
		Rule->SetStringField(TEXT("RuleGroup"), TEXT("UE5"));
		Rule->SetStringField(TEXT("RuleTitle"), TEXT("Check asset"));
		Rule->SetStringField(TEXT("RuleDesc"), Description);
		Rule->SetStringField(TEXT("RuleURL"), TEXT("https://example.com/rules?a=1&b=2"));
		Rule->SetNumberField(TEXT("RuleSeverity"), Severity);
		Rule->SetStringField(TEXT("RuleRecommendedAction"), Action);
		return MakeShared<FJsonValueObject>(Rule);
	}

	TSharedPtr<FJsonValue> MakeAsset(const FString& Name, TArray<TSharedPtr<FJsonValue>> Issues)
	{
		const TSharedRef<FJsonObject> Asset = MakeShared<FJsonObject>();
		const FString Path = FString::Printf(TEXT("/Game/Tests/%s.%s"), *Name, *Name);
		Asset->SetStringField(TEXT("ViolatorAssetName"), Name);
		Asset->SetStringField(TEXT("ViolatorAssetPath"), Path);
		Asset->SetStringField(TEXT("ViolatorFullName"), TEXT("/Script/Engine.Blueprint ") + Path);
		Asset->SetArrayField(TEXT("Violations"), Issues);
		return MakeShared<FJsonValueObject>(Asset);
	}

	TSharedPtr<FJsonObject> Parse(const FString& Json)
	{
		TSharedPtr<FJsonObject> Object;
		FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(Json), Object);
		return Object;
	}

	FString StringAt(const FJsonObject& Report, const TSharedPtr<FJsonValue>& Index)
	{
		return Report.GetArrayField(TEXT("strings"))[static_cast<int32>(Index->AsNumber())]->AsString();
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLintReportRoundTripTest, "PositiveLinter.Report.CompactRoundTrip", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FLintReportRoundTripTest::RunTest(const FString& Parameters)
{
	using namespace LintReportExportTests;
	const TSharedRef<FJsonObject> Legacy = MakeShared<FJsonObject>();
	const TArray<TSharedPtr<FJsonValue>> LegacyAssets{
		MakeAsset(TEXT("BP_検証"), {MakeRule(0, TEXT("Fix A")), MakeRule(1, TEXT("Fix B"))}),
		MakeAsset(TEXT("BP_Second"), {MakeRule(0, TEXT("Fix A")), MakeRule(2, TEXT("Advice")), MakeRule(0, TEXT("Fix A"), TEXT("Different guidance"))})};
	Legacy->SetArrayField(TEXT("Violators"), LegacyAssets);
	FLintReportMetadata Metadata;
	Metadata.Project = TEXT("日本語プロジェクト");
	Metadata.GeneratedAt = TEXT("2026-09-07T12:01:06Z");
	Metadata.RuleSet = TEXT("UE5 Style Guide");
	Metadata.Paths = {TEXT("/Game/Tests"), TEXT("/Plugin/Assets")};
	const TSharedPtr<FJsonObject> Compact = Parse(FLintReportExport::BuildCompactJson(Legacy, Metadata));
	if (!TestTrue(TEXT("Compact report is valid JSON"), Compact.IsValid()))
	{
		return false;
	}
	TestEqual(TEXT("Schema version"), Compact->GetIntegerField(TEXT("version")), 2);
	TestEqual(TEXT("Project preserved"), Compact->GetStringField(TEXT("project")), Metadata.Project);
	TestEqual(TEXT("Timestamp preserved"), Compact->GetStringField(TEXT("generatedAt")), Metadata.GeneratedAt);
	TestEqual(TEXT("Rule-set preserved"), Compact->GetStringField(TEXT("ruleSet")), Metadata.RuleSet);
	TestEqual(TEXT("Both scanned paths preserved"), Compact->GetArrayField(TEXT("paths")).Num(), Metadata.Paths.Num());
	TestEqual(TEXT("Second scanned path preserved"), Compact->GetArrayField(TEXT("paths"))[1]->AsString(), Metadata.Paths[1]);
	const TArray<TSharedPtr<FJsonValue>>& Assets = Compact->GetArrayField(TEXT("assets"));
	const TArray<TSharedPtr<FJsonValue>>& Rules = Compact->GetArrayField(TEXT("rules"));
	TestEqual(TEXT("All violators preserved"), Assets.Num(), LegacyAssets.Num());
	TestEqual(TEXT("Duplicate rules interned, severity and description differences remain distinct"), Rules.Num(), 4);
	TSet<FString> UniqueStrings;
	for (const TSharedPtr<FJsonValue>& Value : Compact->GetArrayField(TEXT("strings")))
	{
		UniqueStrings.Add(Value->AsString());
	}
	TestEqual(TEXT("No duplicate strings in dictionary"), UniqueStrings.Num(), Compact->GetArrayField(TEXT("strings")).Num());
	for (int32 AssetIndex = 0; AssetIndex < Assets.Num(); ++AssetIndex)
	{
		const TArray<TSharedPtr<FJsonValue>>& Asset = Assets[AssetIndex]->AsArray();
		const TSharedPtr<FJsonObject> Original = LegacyAssets[AssetIndex]->AsObject();
		TestEqual(TEXT("Asset name round-trips"), StringAt(*Compact, Asset[0]), Original->GetStringField(TEXT("ViolatorAssetName")));
		TestEqual(TEXT("Object path round-trips"), StringAt(*Compact, Asset[1]), Original->GetStringField(TEXT("ViolatorAssetPath")));
		TestEqual(TEXT("Full native class path retained"), StringAt(*Compact, Asset[2]), FString(TEXT("/Script/Engine.Blueprint")));
		const TArray<TSharedPtr<FJsonValue>>& Issues = Asset[3]->AsArray();
		const TArray<TSharedPtr<FJsonValue>>& OriginalIssues = Original->GetArrayField(TEXT("Violations"));
		TestEqual(TEXT("All violations retained, including repeated rule occurrences"), Issues.Num(), OriginalIssues.Num());
		for (int32 IssueIndex = 0; IssueIndex < Issues.Num(); ++IssueIndex)
		{
			const TArray<TSharedPtr<FJsonValue>>& Issue = Issues[IssueIndex]->AsArray();
			const TArray<TSharedPtr<FJsonValue>>& Rule = Rules[static_cast<int32>(Issue[0]->AsNumber())]->AsArray();
			const TSharedPtr<FJsonObject> OriginalRule = OriginalIssues[IssueIndex]->AsObject();
			const TCHAR* Fields[] = {TEXT("RuleGroup"), TEXT("RuleTitle"), TEXT("RuleDesc"), TEXT("RuleURL")};
			for (int32 Field = 0; Field < UE_ARRAY_COUNT(Fields); ++Field)
			{
				TestEqual(Fields[Field], StringAt(*Compact, Rule[Field]), OriginalRule->GetStringField(Fields[Field]));
			}
			TestEqual(TEXT("Severity preserved as Error, Warning or Info"), static_cast<int32>(Rule[4]->AsNumber()), OriginalRule->GetIntegerField(TEXT("RuleSeverity")));
			TestEqual(TEXT("Per-violation action round-trips"), StringAt(*Compact, Issue[1]), OriginalRule->GetStringField(TEXT("RuleRecommendedAction")));
		}
	}
	TestEqual(TEXT("Standalone JSON data retains its original shape"), Legacy->Values.Num(), 1);
	TestTrue(TEXT("Standalone JSON violators retained"), Legacy->HasField(TEXT("Violators")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLintReportEscapingTest, "PositiveLinter.Report.HtmlEscaping", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FLintReportEscapingTest::RunTest(const FString& Parameters)
{
	using namespace LintReportExportTests;
	const FString Dangerous = TEXT("</script><script>alert('x')</script>&\u2028\u2029 {% TITLE %} {% LINT_REPORT %} 日本語");
	const TSharedRef<FJsonObject> Legacy = MakeShared<FJsonObject>();
	Legacy->SetArrayField(TEXT("Violators"), {MakeAsset(TEXT("BP_Test"), {MakeRule(1, Dangerous)})});
	FLintReportMetadata Metadata;
	Metadata.Project = TEXT("<Report> & \"quoted\" 'title' {% LINT_REPORT %}");
	const FString Template = TEXT("<title>{% TITLE %}</title><script type=\"application/json\" id=\"report-data\">{% LINT_REPORT %}</script>");
	const FString Html = FLintReportExport::RenderHtml(Template, Legacy, Metadata);
	TestTrue(TEXT("Title is HTML escaped"), Html.StartsWith(TEXT("<title>&lt;Report&gt; &amp; &quot;quoted&quot; &#39;title&#39; {% LINT_REPORT %}</title>")));
	const FString Marker = TEXT("id=\"report-data\">");
	const int32 PayloadStart = Html.Find(Marker) + Marker.Len();
	const FString Payload = Html.Mid(PayloadStart, Html.Len() - PayloadStart - FString(TEXT("</script>")).Len());
	TestFalse(TEXT("No literal less-than in script data"), Payload.Contains(TEXT("<")));
	TestFalse(TEXT("No literal greater-than in script data"), Payload.Contains(TEXT(">")));
	TestFalse(TEXT("No literal ampersand in script data"), Payload.Contains(TEXT("&")));
	TestFalse(TEXT("No literal line separator"), Payload.Contains(TEXT("\u2028")));
	TestFalse(TEXT("No literal paragraph separator"), Payload.Contains(TEXT("\u2029")));
	const TSharedPtr<FJsonObject> Parsed = Parse(Payload);
	if (!TestTrue(TEXT("Escaped data remains valid JSON"), Parsed.IsValid()))
	{
		return false;
	}
	TestEqual(TEXT("Project preserved despite title escaping"), Parsed->GetStringField(TEXT("project")), Metadata.Project);
	const TArray<TSharedPtr<FJsonValue>>& Asset = Parsed->GetArrayField(TEXT("assets"))[0]->AsArray();
	const TArray<TSharedPtr<FJsonValue>>& Issue = Asset[3]->AsArray()[0]->AsArray();
	TestEqual(TEXT("Dangerous content and template tokens round-trip literally"), StringAt(*Parsed, Issue[1]), Dangerous);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLintReportEmptyTest, "PositiveLinter.Report.EmptyReport", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FLintReportEmptyTest::RunTest(const FString& Parameters)
{
	const TSharedRef<FJsonObject> Legacy = MakeShared<FJsonObject>();
	Legacy->SetArrayField(TEXT("Violators"), {});
	FLintReportMetadata Metadata;
	Metadata.Project = TEXT("Clean project");
	const TSharedPtr<FJsonObject> Compact = LintReportExportTests::Parse(FLintReportExport::BuildCompactJson(Legacy, Metadata));
	if (!TestTrue(TEXT("Empty report is valid JSON"), Compact.IsValid()))
	{
		return false;
	}
	TestTrue(TEXT("No assets for zero findings"), Compact->GetArrayField(TEXT("assets")).IsEmpty());
	TestTrue(TEXT("No rules for zero findings"), Compact->GetArrayField(TEXT("rules")).IsEmpty());
	TestTrue(TEXT("No unused dictionary strings"), Compact->GetArrayField(TEXT("strings")).IsEmpty());
	TestTrue(TEXT("No invented scan paths"), Compact->GetArrayField(TEXT("paths")).IsEmpty());
	return true;
}

#endif
