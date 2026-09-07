#pragma once

#include "CoreTypes.h"
#include "Containers/ArrayView.h"

/** Native asset class paths and editable naming defaults for the UE5 rule set. */
struct FUE5AssetType
{
	const TCHAR* ClassPath;
	const TCHAR* Prefix;
};

/** Does not load or enable any of the optional plugins represented in the catalog. */
UE5LINTER_API TConstArrayView<FUE5AssetType> GetUE5AssetTypes();
