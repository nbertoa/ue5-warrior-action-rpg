#include "WorldValidation/BertaWorldValidation.h"

#include "EngineUtils.h"
#include "Components/LightComponentBase.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Log/BertaDevKitEditorLog.h"
#include "Settings/BertaDevKitSettings.h"
#include "Widgets/Notifications/SNotificationList.h"

// ─────────────────────────────────────────────────────────────────────────────
// Helpers
// ─────────────────────────────────────────────────────────────────────────────

namespace
{
	/** Returns a human-readable string for EComponentMobility values used in log messages. */
	FString MobilityToString(const EComponentMobility::Type Mobility)
	{
		switch (Mobility)
		{
		case EComponentMobility::Static: return TEXT("Static");
		case EComponentMobility::Stationary: return TEXT("Stationary");
		case EComponentMobility::Movable: return TEXT("Movable");
		default: return TEXT("Unknown");
		}
	}
}

// ─────────────────────────────────────────────────────────────────────────────
// Public entry point
// ─────────────────────────────────────────────────────────────────────────────

void UBertaWorldValidation::RunValidation()
{
	const UBertaDevKitSettings* Settings = UBertaDevKitSettings::Get();

	// Master switch — bail out early if the system is globally disabled.
	if (!Settings->bWorldValidationEnabled)
	{
		UE_LOG(LogBertaDevKitEditor,
		       Log,
		       TEXT("[BertaWorldValidation] World Validation is disabled in Project Settings."));
		return;
	}

	// GEditor is always valid in an Editor-only module, but guard defensively.
	if (!GEditor)
	{
		UE_LOG(LogBertaDevKitEditor,
		       Warning,
		       TEXT("[BertaWorldValidation] GEditor is not available. Validation aborted."));
		return;
	}

	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World)
	{
		UE_LOG(LogBertaDevKitEditor,
		       Warning,
		       TEXT("[BertaWorldValidation] No valid world found. Make sure a level is open."));
		return;
	}

	UE_LOG(LogBertaDevKitEditor,
	       Log,
	       TEXT("[BertaWorldValidation] Starting world validation on level: %s"),
	       *World->GetMapName());

	// Read configurable values once before iterating — avoids redundant GetDefault<> calls per actor.
	const float BoundsThreshold = Settings->WorldBoundsThreshold;
	const EComponentMobility::Type ExpectedMobility = Settings->ExpectedLightMobility;

	int32 ViolationCount = 0;
	int32 ActorCount = 0;

	for (TActorIterator<AActor> It(World); It; ++It)
	{
		const AActor* Actor = *It;
		if (!IsValid(Actor))
		{
			continue;
		}

		++ActorCount;

		// Each check returns true if a violation was found.
		// The results are OR-combined so ViolationCount increments once per actor,
		// regardless of how many checks failed on it.
		bool bActorHasViolation = false;

		if (Settings->bValidateStaticMeshComponents)
		{
			bActorHasViolation |= ValidateStaticMeshComponents(Actor);
		}

		if (Settings->bValidateWorldBounds)
		{
			bActorHasViolation |= ValidateWorldBounds(Actor,
			                                          BoundsThreshold);
		}

		if (Settings->bValidateLightMobility)
		{
			bActorHasViolation |= ValidateLightMobility(Actor,
			                                            ExpectedMobility);
		}

		if (Settings->bValidateActorScale)
		{
			bActorHasViolation |= ValidateActorScale(Actor);
		}

		if (bActorHasViolation)
		{
			++ViolationCount;
		}
	}

	// ── Summary log ──────────────────────────────────────────────────────────

	UE_LOG(LogBertaDevKitEditor,
	       Log,
	       TEXT("[BertaWorldValidation] Validation complete. Actors checked: %d | Actors with violations: %d"),
	       ActorCount,
	       ViolationCount);

	// ── Editor notification ───────────────────────────────────────────────────

	const FText NotificationText = ViolationCount > 0
		                               ? FText::Format(NSLOCTEXT("BertaWorldValidation",
		                                                         "ViolationsFound",
		                                                         "World Validation: {0} actor(s) with violations. See Output Log."),
		                                               FText::AsNumber(ViolationCount))
		                               : NSLOCTEXT("BertaWorldValidation",
		                                           "AllPassed",
		                                           "World Validation: All checks passed.");

	FNotificationInfo Info(NotificationText);
	Info.ExpireDuration = 5.0f;
	Info.bUseLargeFont = false;

	TSharedPtr<SNotificationItem> Notification = FSlateNotificationManager::Get().AddNotification(Info);
	if (Notification.IsValid())
	{
		Notification->SetCompletionState(ViolationCount > 0
			                                 ? SNotificationItem::CS_Fail
			                                 : SNotificationItem::CS_Success);
	}
}

// ─────────────────────────────────────────────────────────────────────────────
// Private checks
// ─────────────────────────────────────────────────────────────────────────────

bool UBertaWorldValidation::ValidateStaticMeshComponents(const AActor* Actor)
{
	bool bFoundViolation = false;

	TArray<UStaticMeshComponent*> MeshComponents;
	Actor->GetComponents<UStaticMeshComponent>(MeshComponents);

	for (const UStaticMeshComponent* MeshComponent : MeshComponents)
	{
		if (!IsValid(MeshComponent))
		{
			continue;
		}

		// A valid component with no mesh assigned is a misconfiguration — flag it.
		if (MeshComponent->GetStaticMesh() != nullptr)
		{
			continue;
		}

		UE_LOG(LogBertaDevKitEditor,
		       Warning,
		       TEXT("[BertaWorldValidation] Missing static mesh — Actor: '%s', Component: '%s'"),
		       *Actor->GetActorLabel(),
		       *MeshComponent->GetName());

		bFoundViolation = true;
	}

	return bFoundViolation;
}

bool UBertaWorldValidation::ValidateWorldBounds(const AActor* Actor,
                                                const float BoundsThreshold)
{
	const FVector Location = Actor->GetActorLocation();

	// Check each axis independently so that a violation on any one axis is flagged.
	const bool bOutOfBounds = FMath::Abs(Location.X) > BoundsThreshold || FMath::Abs(Location.Y) > BoundsThreshold ||
			FMath::Abs(Location.Z) > BoundsThreshold;

	if (!bOutOfBounds)
	{
		return false;
	}

	UE_LOG(LogBertaDevKitEditor,
	       Warning,
	       TEXT("[BertaWorldValidation] Actor out of world bounds — Actor: '%s', Location: %s, Threshold: %.1f cm"),
	       *Actor->GetActorLabel(),
	       *Location.ToString(),
	       BoundsThreshold);

	return true;
}

bool UBertaWorldValidation::ValidateLightMobility(const AActor* Actor,
                                                  const EComponentMobility::Type ExpectedMobility)
{
	// Only evaluate actors that own at least one light component.
	const ULightComponentBase* LightComponent = Actor->FindComponentByClass<ULightComponentBase>();
	if (!IsValid(LightComponent))
	{
		return false;
	}

	const EComponentMobility::Type ActualMobility = LightComponent->Mobility;
	if (ActualMobility == ExpectedMobility)
	{
		return false;
	}

	UE_LOG(LogBertaDevKitEditor,
	       Warning,
	       TEXT("[BertaWorldValidation] Light mobility mismatch — Actor: '%s', Expected: %s, Found: %s"),
	       *Actor->GetActorLabel(),
	       *MobilityToString(ExpectedMobility),
	       *MobilityToString(ActualMobility));

	return true;
}

bool UBertaWorldValidation::ValidateActorScale(const AActor* Actor)
{
	const FVector Scale = Actor->GetActorScale3D();

	// Zero or negative scale breaks physics and rendering silently.
	const bool bInvalidScale = Scale.X <= 0.0f || Scale.Y <= 0.0f || Scale.Z <= 0.0f;

	if (!bInvalidScale)
	{
		return false;
	}

	UE_LOG(LogBertaDevKitEditor,
	       Warning,
	       TEXT("[BertaWorldValidation] Invalid scale — Actor: '%s', Scale: %s"),
	       *Actor->GetActorLabel(),
	       *Scale.ToString());

	return true;
}
