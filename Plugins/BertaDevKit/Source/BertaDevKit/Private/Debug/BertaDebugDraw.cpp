#include "Debug/BertaDebugDraw.h"

#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SphereComponent.h"
#include "Debug/BertaDebugLog.h"
#include "DrawDebugHelpers.h"
#include "Settings/BertaDevKitSettings.h"

// ─── Private helpers ────────────────────────────────────────────────────────

const UWorld* UBertaDebugDraw::ResolveWorld(const UObject* WorldContextObject,
                                            const TCHAR* CallerName)
{
	if (!ensureMsgf(IsValid(WorldContextObject),
	                TEXT("[BertaDebugDraw::%s] WorldContextObject is null or invalid."),
	                CallerName))
	{
		return nullptr;
	}

	const UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject,
	                                                         EGetWorldErrorMode::LogAndReturnNull);

	if (!IsValid(World))
	{
		UE_LOG(LogBertaDebug,
		       Warning,
		       TEXT("[BertaDebugDraw::%s] Could not retrieve a valid UWorld."),
		       CallerName);
		return nullptr;
	}

	return World;
}

float UBertaDebugDraw::ResolveLifeTime(const float Duration)
{
	// DrawDebug* uses -1.0f as the sentinel for "draw forever".
	// We expose Duration <= 0.0f as the public contract for persistence
	// so callers never need to know about the -1.0f convention.
	return (Duration <= 0.0f)
		       ? -1.0f
		       : Duration;
}

FColor UBertaDebugDraw::ToFColor(const FLinearColor& LinearColor)
{
	// Quantise from linear float [0,1] to sRGB uint8 [0,255].
	// DrawDebug* predates FLinearColor — FColor is what the engine expects here.
	return LinearColor.ToFColor(true);
}

// ─── Public API ─────────────────────────────────────────────────────────────

void UBertaDebugDraw::DrawSphere(const UObject* WorldContextObject,
                                 const bool bEnabled,
                                 const FVector Center,
                                 const float Radius,
                                 const FLinearColor Color,
                                 const float Duration,
                                 const int32 Segments,
                                 const float Thickness)
{
	if (!bEnabled)
	{
		return;
	}

	if (!UBertaDevKitSettings::Get()->bDebugDrawEnabled)
	{
		return;
	}

	const UWorld* World = ResolveWorld(WorldContextObject,
	                                   TEXT("DrawSphere"));
	if (!World)
	{
		return;
	}

	const float ResolvedLifeTime = ResolveLifeTime(Duration);
	const bool bPersistent = (ResolvedLifeTime < 0.0f);

	DrawDebugSphere(World,
	                Center,
	                Radius,
	                Segments,
	                ToFColor(Color),
	                bPersistent,
	                ResolvedLifeTime,
	                /*DepthPriority=*/
	                0,
	                Thickness);
}

void UBertaDebugDraw::DrawSphereFromComponent(const USphereComponent* SphereComponent,
                                              const bool bEnabled,
                                              const FLinearColor Color,
                                              const float Duration,
                                              const int32 Segments,
                                              const float Thickness)
{
	if (!bEnabled)
	{
		return;
	}

	if (!UBertaDevKitSettings::Get()->bDebugDrawEnabled)
	{
		return;
	}

	const UWorld* World = ResolveWorld(SphereComponent,
	                                   TEXT("DrawSphereFromComponent"));
	if (!World)
	{
		return;
	}

	const float ResolvedLifeTime = ResolveLifeTime(Duration);
	const bool bPersistent = (ResolvedLifeTime < 0.0f);

	// Extract geometry directly from the component so the caller never needs
	// to query these values manually. GetScaledSphereRadius accounts for any
	// non-uniform scale applied to the component or its owner.
	DrawDebugSphere(World,
	                SphereComponent->GetComponentLocation(),
	                SphereComponent->GetScaledSphereRadius(),
	                Segments,
	                ToFColor(Color),
	                bPersistent,
	                ResolvedLifeTime,
	                /*DepthPriority=*/
	                0,
	                Thickness);
}

void UBertaDebugDraw::DrawLine(const UObject* WorldContextObject,
                               const bool bEnabled,
                               const FVector Start,
                               const FVector End,
                               const FLinearColor Color,
                               const float Duration,
                               const float Thickness)
{
	if (!bEnabled)
	{
		return;
	}

	if (!UBertaDevKitSettings::Get()->bDebugDrawEnabled)
	{
		return;
	}

	const UWorld* World = ResolveWorld(WorldContextObject,
	                                   TEXT("DrawLine"));
	if (!World)
	{
		return;
	}

	const float ResolvedLifeTime = ResolveLifeTime(Duration);
	const bool bPersistent = (ResolvedLifeTime < 0.0f);

	DrawDebugLine(World,
	              Start,
	              End,
	              ToFColor(Color),
	              bPersistent,
	              ResolvedLifeTime,
	              /*DepthPriority=*/
	              0,
	              Thickness);
}

void UBertaDebugDraw::DrawBox(const UObject* WorldContextObject,
                              const bool bEnabled,
                              const FVector Center,
                              const FVector Extent,
                              const FLinearColor Color,
                              const float Duration,
                              const float Thickness)
{
	if (!bEnabled)
	{
		return;
	}

	if (!UBertaDevKitSettings::Get()->bDebugDrawEnabled)
	{
		return;
	}

	const UWorld* World = ResolveWorld(WorldContextObject,
	                                   TEXT("DrawBox"));
	if (!World)
	{
		return;
	}

	const float ResolvedLifeTime = ResolveLifeTime(Duration);
	const bool bPersistent = (ResolvedLifeTime < 0.0f);

	// This overload draws an axis-aligned box (no rotation).
	// Use DrawBoxFromComponent if you need an oriented box.
	DrawDebugBox(World,
	             Center,
	             Extent,
	             ToFColor(Color),
	             bPersistent,
	             ResolvedLifeTime,
	             /*DepthPriority=*/
	             0,
	             Thickness);
}

void UBertaDebugDraw::DrawBoxFromComponent(const UBoxComponent* BoxComponent,
                                           const bool bEnabled,
                                           const FLinearColor Color,
                                           const float Duration,
                                           const float Thickness)
{
	if (!bEnabled)
	{
		return;
	}

	if (!UBertaDevKitSettings::Get()->bDebugDrawEnabled)
	{
		return;
	}

	const UWorld* World = ResolveWorld(BoxComponent,
	                                   TEXT("DrawBoxFromComponent"));
	if (!World)
	{
		return;
	}

	const float ResolvedLifeTime = ResolveLifeTime(Duration);
	const bool bPersistent = (ResolvedLifeTime < 0.0f);

	// GetScaledBoxExtent returns half-extents already multiplied by the component's
	// world scale — exactly what DrawDebugBox expects for its Extent parameter.
	// GetComponentQuat preserves any rotation applied to the box, ensuring the
	// debug shape matches the actual collision volume in the world.
	DrawDebugBox(World,
	             BoxComponent->GetComponentLocation(),
	             BoxComponent->GetScaledBoxExtent(),
	             BoxComponent->GetComponentQuat(),
	             ToFColor(Color),
	             bPersistent,
	             ResolvedLifeTime,
	             /*DepthPriority=*/
	             0,
	             Thickness);
}

void UBertaDebugDraw::DrawCapsule(const UObject* WorldContextObject,
                                  const bool bEnabled,
                                  const FVector Center,
                                  const float HalfHeight,
                                  const float Radius,
                                  const FLinearColor Color,
                                  const float Duration,
                                  const float Thickness)
{
	if (!bEnabled)
	{
		return;
	}

	if (!UBertaDevKitSettings::Get()->bDebugDrawEnabled)
	{
		return;
	}

	const UWorld* World = ResolveWorld(WorldContextObject,
	                                   TEXT("DrawCapsule"));
	if (!World)
	{
		return;
	}

	const float ResolvedLifeTime = ResolveLifeTime(Duration);
	const bool bPersistent = (ResolvedLifeTime < 0.0f);

	// FQuat::Identity orients the capsule along the Z axis, matching
	// UCapsuleComponent's default orientation — the most common use case.
	DrawDebugCapsule(World,
	                 Center,
	                 HalfHeight,
	                 Radius,
	                 FQuat::Identity,
	                 ToFColor(Color),
	                 bPersistent,
	                 ResolvedLifeTime,
	                 /*DepthPriority=*/
	                 0,
	                 Thickness);
}

void UBertaDebugDraw::DrawCapsuleFromComponent(const UCapsuleComponent* CapsuleComponent,
                                               const bool bEnabled,
                                               const FLinearColor Color,
                                               const float Duration,
                                               const float Thickness)
{
	if (!bEnabled)
	{
		return;
	}

	if (!UBertaDevKitSettings::Get()->bDebugDrawEnabled)
	{
		return;
	}

	const UWorld* World = ResolveWorld(CapsuleComponent,
	                                   TEXT("DrawCapsuleFromComponent"));
	if (!World)
	{
		return;
	}

	const float ResolvedLifeTime = ResolveLifeTime(Duration);
	const bool bPersistent = (ResolvedLifeTime < 0.0f);

	// Extract geometry and transform directly from the component so the caller
	// never needs to query these values manually before calling this function.
	DrawDebugCapsule(World,
	                 CapsuleComponent->GetComponentLocation(),
	                 CapsuleComponent->GetScaledCapsuleHalfHeight(),
	                 CapsuleComponent->GetScaledCapsuleRadius(),
	                 CapsuleComponent->GetComponentQuat(),
	                 ToFColor(Color),
	                 bPersistent,
	                 ResolvedLifeTime,
	                 /*DepthPriority=*/
	                 0,
	                 Thickness);
}

void UBertaDebugDraw::DrawPoint(const UObject* WorldContextObject,
                                const bool bEnabled,
                                const FVector Location,
                                const FLinearColor Color,
                                const float Duration,
                                const float Size)
{
	if (!bEnabled)
	{
		return;
	}

	if (!UBertaDevKitSettings::Get()->bDebugDrawEnabled)
	{
		return;
	}

	const UWorld* World = ResolveWorld(WorldContextObject,
	                                   TEXT("DrawPoint"));
	if (!World)
	{
		return;
	}

	const float ResolvedLifeTime = ResolveLifeTime(Duration);
	const bool bPersistent = (ResolvedLifeTime < 0.0f);

	DrawDebugPoint(World,
	               Location,
	               Size,
	               ToFColor(Color),
	               bPersistent,
	               ResolvedLifeTime);
}

void UBertaDebugDraw::DrawArrow(const UObject* WorldContextObject,
                                const bool bEnabled,
                                const FVector Start,
                                const FVector End,
                                const float ArrowSize,
                                const FLinearColor Color,
                                const float Duration,
                                const float Thickness)
{
	if (!bEnabled)
	{
		return;
	}

	if (!UBertaDevKitSettings::Get()->bDebugDrawEnabled)
	{
		return;
	}

	const UWorld* World = ResolveWorld(WorldContextObject,
	                                   TEXT("DrawArrow"));
	if (!World)
	{
		return;
	}

	const float ResolvedLifeTime = ResolveLifeTime(Duration);
	const bool bPersistent = (ResolvedLifeTime < 0.0f);

	DrawDebugDirectionalArrow(World,
	                          Start,
	                          End,
	                          ArrowSize,
	                          ToFColor(Color),
	                          bPersistent,
	                          ResolvedLifeTime,
	                          /*DepthPriority=*/
	                          0,
	                          Thickness);
}

void UBertaDebugDraw::DrawString(const UObject* WorldContextObject,
                                 const bool bEnabled,
                                 const FVector Location,
                                 const FString& Text,
                                 const FLinearColor Color,
                                 const float Duration,
                                 const float FontScale)
{
	if (!bEnabled)
	{
		return;
	}

	if (!UBertaDevKitSettings::Get()->bDebugDrawEnabled)
	{
		return;
	}

	const UWorld* World = ResolveWorld(WorldContextObject,
	                                   TEXT("DrawString"));
	if (!World)
	{
		return;
	}

	const float ResolvedLifeTime = ResolveLifeTime(Duration);

	// DrawDebugString does not take a bPersistent flag — it uses LifeTime directly
	// with -1.0f as the persistent sentinel, consistent with ResolveLifeTime's output.
	DrawDebugString(World,
	                Location,
	                Text,
	                /*TestBaseActor=*/
	                nullptr,
	                ToFColor(Color),
	                ResolvedLifeTime,
	                /*bDrawShadow=*/
	                false,
	                FontScale);
}

void UBertaDebugDraw::DrawCoordinateSystem(const UObject* WorldContextObject,
                                           const bool bEnabled,
                                           const FVector Location,
                                           const FRotator Rotation,
                                           const float AxisLength,
                                           const float Duration,
                                           const float Thickness)
{
	if (!bEnabled)
	{
		return;
	}

	if (!UBertaDevKitSettings::Get()->bDebugDrawEnabled)
	{
		return;
	}

	const UWorld* World = ResolveWorld(WorldContextObject,
	                                   TEXT("DrawCoordinateSystem"));
	if (!World)
	{
		return;
	}

	const float ResolvedLifeTime = ResolveLifeTime(Duration);

	// DrawDebugCoordinateSystem has a known engine bug: when bPersistentLines = true
	// it ignores LifeTime entirely and the lines never clear, even with
	// FlushPersistentDebugLines. Passing false and relying on LifeTime = -1.0f
	// for persistence is the correct workaround.
	// Axis colors are hardcoded by the engine: red = X, green = Y, blue = Z.
	DrawDebugCoordinateSystem(World,
	                          Location,
	                          Rotation,
	                          AxisLength,
	                          /*bPersistentLines=*/
	                          false,
	                          ResolvedLifeTime,
	                          /*DepthPriority=*/
	                          0,
	                          Thickness);
}

void UBertaDebugDraw::FlushPersistentShapes(const UObject* WorldContextObject)
{
	const UWorld* World = ResolveWorld(WorldContextObject,
	                                   TEXT("FlushPersistentShapes"));
	if (!World)
	{
		return;
	}

	// FlushPersistentDebugLines clears all persistent line-based shapes
	// (spheres, lines, boxes, capsules, arrows, points, coordinate systems).
	// FlushDebugStrings clears persistent DrawDebugString text separately —
	// the two systems use different internal buffers and must be flushed independently.
	FlushPersistentDebugLines(World);
	FlushDebugStrings(World);
}
