// Copyright 2019-2020 Gamemakin LLC. All Rights Reserved.

using UnrealBuildTool;

public class Linter : ModuleRules
{
    public Linter(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;		
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"AssetRegistry",
				"PropertyEditor",
			}
		);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
            {
                "ContentBrowser",
                "Slate",
                "SlateCore",
				"AppFramework",
				"InputCore",
                "UnrealEd",
                "GraphEditor",
                "AssetTools",
                "BlueprintGraph",
                "LauncherPlatform",
                "Projects",
				"DesktopPlatform",
				"Json",
                "UATHelper"
				// ... add private dependencies that you statically link with here ...	
			}
		);

        PublicIncludePathModuleNames.Add("Launch");

    }
}
