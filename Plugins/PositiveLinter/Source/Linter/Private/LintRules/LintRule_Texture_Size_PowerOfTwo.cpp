// Copyright 2019-2020 Gamemakin LLC. All Rights Reserved.
#include "LintRules/LintRule_Texture_Size_PowerOfTwo.h"
#include "LintRuleSet.h"
#include "LinterNamingConvention.h"
#include "HAL/FileManager.h"
#include "TextureCompiler.h"

ULintRule_Texture_Size_PowerOfTwo::ULintRule_Texture_Size_PowerOfTwo(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bRequiresGameThread = true;
	IgnoreTexturesInTheseGroups.Add(TextureGroup::TEXTUREGROUP_UI);
}

bool ULintRule_Texture_Size_PowerOfTwo::PassesRule(UObject* ObjectToLint, const ULintRuleSet* ParentRuleSet, TArray<FLintRuleViolation>& OutRuleViolations) const
{
	// If we aren't a texture, abort
	UTexture2D* Texture = Cast<UTexture2D>(ObjectToLint);
	if (Texture == nullptr)
	{
		// @TODO: Bubble up some sort of configuration error?
		return true;
	}

	// Texture platform data can be replaced while asynchronous compilation is in
	// progress. This rule runs on the game thread, so wait for the data that we
	// are about to inspect to be ready.
#if WITH_EDITOR
	TArray<UTexture*> TexturesToFinish;
	TexturesToFinish.Add(Texture);
	FTextureCompilingManager::Get().FinishCompilation(TexturesToFinish);
#endif

	if (Texture->GetPlatformData() == nullptr)
	{
		return true;
	}

	// If we're to ignore this texture LOD group, abort
	if (IgnoreTexturesInTheseGroups.Contains(Texture->LODGroup))
	{
		return true;
	}

	return Super::PassesRule(ObjectToLint, ParentRuleSet, OutRuleViolations);
}

bool ULintRule_Texture_Size_PowerOfTwo::PassesRule_Internal_Implementation(UObject* ObjectToLint, const ULintRuleSet* ParentRuleSet, TArray<FLintRuleViolation>& OutRuleViolations) const
{
	const UTexture2D* Texture = Cast<UTexture2D>(ObjectToLint);
	if (Texture == nullptr || Texture->GetPlatformData() == nullptr)
	{
		return true;
	}

	const int32 TexSizeX = Texture->GetSizeX();
	const int32 TexSizeY = Texture->GetSizeY();

	const bool bXFail = TexSizeX <= 0 || !FMath::IsPowerOfTwo(TexSizeX);
	const bool bYFail = TexSizeY <= 0 || !FMath::IsPowerOfTwo(TexSizeY);

	UEnum* TextureGroupEnum = StaticEnum<TextureGroup>();
	FString IgnoredLODGroupNames;

	if (TextureGroupEnum != nullptr)
	{
		for (TEnumAsByte<TextureGroup> LODGroup : IgnoreTexturesInTheseGroups)
		{
			IgnoredLODGroupNames += TextureGroupEnum->GetMetaData(TEXT("DisplayName"), LODGroup) + TEXT(", ");
		}
	}
	IgnoredLODGroupNames.RemoveFromEnd(TEXT(", "));

	FText IgnoredLODGroupTip = IgnoredLODGroupNames.Len() > 0 ? FText::FormatOrdered(NSLOCTEXT("Linter", "LintRule_Texture_Size_PowerOfTwo_AllowedLODGroups", ". Alternatively, assign this texture to one of these LOD Groups: [{0}]"), FText::FromString(IgnoredLODGroupNames)) : FText::GetEmpty();

	if (bXFail || bYFail)
	{
		FText RecommendedAction;
		if (bXFail && bYFail)
		{
			RecommendedAction = FText::FormatOrdered(NSLOCTEXT("Linter", "LintRule_Texture_Size_PowerOfTwo_Fail_XY", "Please fix the width and height of this texture, currently {0} by {1}{2}"), TexSizeX, TexSizeY, IgnoredLODGroupTip);
		}
		else if (bXFail)
		{
			RecommendedAction = FText::FormatOrdered(NSLOCTEXT("Linter", "LintRule_Texture_Size_PowerOfTwo_Fail_X", "Please fix the width of this texture, currently {0}{1}"), TexSizeX, IgnoredLODGroupTip);
		}
		else if (bYFail)
		{
			RecommendedAction = FText::FormatOrdered(NSLOCTEXT("Linter", "LintRule_Texture_Size_PowerOfTwo_Fail_Y", "Please fix the height of this texture, currently {0}{1}"), TexSizeY, IgnoredLODGroupTip);
		}

		OutRuleViolations.Push(FLintRuleViolation(ObjectToLint, GetClass(), RecommendedAction));
		return false;
	}
	
	return true;
}
