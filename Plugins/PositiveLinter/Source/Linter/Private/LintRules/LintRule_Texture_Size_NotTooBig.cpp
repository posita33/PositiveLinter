// Copyright 2019-2020 Gamemakin LLC. All Rights Reserved.
#include "LintRules/LintRule_Texture_Size_NotTooBig.h"
#include "LintRuleSet.h"
#include "LinterNamingConvention.h"
#include "HAL/FileManager.h"
#include "TextureCompiler.h"

ULintRule_Texture_Size_NotTooBig::ULintRule_Texture_Size_NotTooBig(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bRequiresGameThread = true;
}

bool ULintRule_Texture_Size_NotTooBig::PassesRule(UObject* ObjectToLint, const ULintRuleSet* ParentRuleSet, TArray<FLintRuleViolation>& OutRuleViolations) const
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

	return Super::PassesRule(ObjectToLint, ParentRuleSet, OutRuleViolations);
}

bool ULintRule_Texture_Size_NotTooBig::PassesRule_Internal_Implementation(UObject* ObjectToLint, const ULintRuleSet* ParentRuleSet, TArray<FLintRuleViolation>& OutRuleViolations) const
{
	const UTexture2D* Texture = Cast<UTexture2D>(ObjectToLint);
	if (Texture == nullptr || Texture->GetPlatformData() == nullptr)
	{
		return true;
	}

	const int32 TexSizeX = Texture->GetSizeX();
	const int32 TexSizeY = Texture->GetSizeY();

	// Check to see if textures are too big
	if (TexSizeX > MaxTextureSizeX || TexSizeY > MaxTextureSizeY)
	{
		FText RecommendedAction = NSLOCTEXT("Linter", "LintRule_Texture_Size_NotTooBig_TooBig", "Please shrink your textures dimensions so that they fit within {0}x{1} pixels.");
		OutRuleViolations.Push(FLintRuleViolation(ObjectToLint, GetClass(), FText::FormatOrdered(RecommendedAction, MaxTextureSizeX, MaxTextureSizeY)));
		return false;
	}

	return true;
}
