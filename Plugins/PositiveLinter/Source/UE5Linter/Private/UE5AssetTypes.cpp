#include "UE5AssetTypes.h"

TConstArrayView<FUE5AssetType> GetUE5AssetTypes()
{
	// Class paths were checked against UE 5.7 source. Prefixes are naming defaults,
	// not engine requirements, and can be customized in a naming convention asset.
	static constexpr FUE5AssetType AssetTypes[] =
	{
		{ TEXT("/Script/Niagara.NiagaraSystem"), TEXT("FXS_") },
		{ TEXT("/Script/Niagara.NiagaraEmitter"), TEXT("FXE_") },
		{ TEXT("/Script/MetasoundEngine.MetaSoundSource"), TEXT("MSS_") },
		{ TEXT("/Script/MetasoundEngine.MetaSoundPatch"), TEXT("MSP_") },
		{ TEXT("/Script/EnhancedInput.InputAction"), TEXT("IA_") },
		{ TEXT("/Script/EnhancedInput.InputMappingContext"), TEXT("IMC_") },
		{ TEXT("/Script/ControlRigDeveloper.ControlRigBlueprint"), TEXT("CR_") },
		{ TEXT("/Script/IKRig.IKRigDefinition"), TEXT("IKR_") },
		{ TEXT("/Script/IKRig.IKRetargeter"), TEXT("RTG_") },
		{ TEXT("/Script/PCG.PCGGraph"), TEXT("PCG_") },
		{ TEXT("/Script/PCG.PCGGraphInstance"), TEXT("PCGI_") },
		{ TEXT("/Script/PCG.PCGDataAsset"), TEXT("PCGD_") },
		{ TEXT("/Script/StateTreeModule.StateTree"), TEXT("ST_") },
		{ TEXT("/Script/Engine.DataLayerAsset"), TEXT("DLA_") },
		{ TEXT("/Script/LevelSequence.LevelSequence"), TEXT("LS_") },
		{ TEXT("/Script/GeometryCollectionEngine.GeometryCollection"), TEXT("GC_") },
		{ TEXT("/Script/PoseSearch.PoseSearchDatabase"), TEXT("PSD_") },
		{ TEXT("/Script/PoseSearch.PoseSearchSchema"), TEXT("PSS_") },
	};

	return MakeArrayView(AssetTypes);
}
