#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Misc/DataValidation.h"
#include "UE5ValidationTestAsset.generated.h"

/** In-memory fixture for native data-validation automation tests. */
UCLASS(Transient, HideDropdown, NotBlueprintable)
class UUE5ValidationTestAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	EDataValidationResult ValidationResult = EDataValidationResult::NotValidated;
	TArray<FText> ValidationErrors;
	TArray<FText> ValidationWarnings;

	virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override
	{
		for (const FText& Error : ValidationErrors)
		{
			Context.AddError(Error);
		}
		for (const FText& Warning : ValidationWarnings)
		{
			Context.AddWarning(Warning);
		}
		return ValidationResult;
	}
};
