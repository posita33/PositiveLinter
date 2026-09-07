using UnrealBuildTool;

public class UE5Linter : ModuleRules
{
    public UE5Linter(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new[]
        {
            "Core", "CoreUObject", "Engine", "Linter", "GamemakinLinter"
        });

        PrivateDependencyModuleNames.AddRange(new[]
        {
            "Niagara", "RenderCore"
        });
    }
}
