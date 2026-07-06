// DataAsset_InputConfig.cpp
// Implements tag-based lookup for input actions with proper asset identity tracing.

#include "DataAssets/Input/DataAsset_InputConfig.h"
#include "Utils/WarriorRPGLogCategories.h"

UInputAction* UDataAsset_InputConfig::FindNativeInputActionByTag(const FGameplayTag& InInputTag) const
{
	for (const FWarriorInputActionConfig& InputActionConfig : NativeInputActions)
	{
		// Defensive: verify the designer assigned an InputAction to this entry.
		// Added *GetName() to tell the designer EXACTLY which DataAsset is misconfigured.
		if (!ensureMsgf(InputActionConfig.InputAction,
		                TEXT("%s: Entry with tag [%s] has a null InputAction. " "Assign it in the DataAsset editor."),
		                *GetName(),
		                *InputActionConfig.InputTag.ToString()))
		{
			continue;
		}

		if (InputActionConfig.InputTag == InInputTag)
		{
			return InputActionConfig.InputAction;
		}
	}

	UE_LOG(LogWarriorRPG,
	       Warning,
	       TEXT("%s: No InputAction found for tag [%s]. " "Ensure it is configured in the NativeInputActions array."),
	       *GetName(),
	       *InInputTag.ToString());

	return nullptr;
}
