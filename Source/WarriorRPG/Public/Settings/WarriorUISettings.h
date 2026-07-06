// WarriorUISettings.h
// Developer settings asset for the WarriorRPG UI system.
// Maps Gameplay Tags to activatable widget classes, allowing designers to
// configure which widget Blueprint corresponds to each tag from Project Settings
// without touching C++ code.
//
// Accessible at runtime via GetDefault<UWarriorUISettings>().

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "GameplayTagContainer.h"
#include "WarriorUISettings.generated.h"

class UWarriorActivatableWidget;

/**
 * UWarriorUISettings
 *
 * Project Settings entry (Config = Game) for the WarriorRPG UI system.
 * Holds a map from Gameplay Tags to widget Blueprint classes.
 * Use UWarriorUIFunctionLibrary::GetWidgetClassByTag to query this at runtime.
 *
 * Editable under Project Settings → Game → Warrior UI Settings.
 * Changes here do not require recompilation — only a cook or PIE restart.
 */
UCLASS(Config = Game,
	DefaultConfig,
	meta = (DisplayName = "Warrior UI Settings"))
class WARRIORRPG_API UWarriorUISettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	/**
	 * Maps a Gameplay Tag (UI.Widget hierarchy) to the widget Blueprint class
	 * that should be instantiated when that tag is requested.
	 * ForceInlineRow displays the class picker inline in the Details panel
	 * instead of collapsing it behind a dropdown, improving editor usability.
	 */
	UPROPERTY(Config,
		EditAnywhere,
		Category = "Widget Reference",
		meta = (ForceInlineRow, Categories = "WarriorRPGTags.UI.Widget"))
	TMap<FGameplayTag, TSubclassOf<UWarriorActivatableWidget>> WidgetClassByTag;
};
