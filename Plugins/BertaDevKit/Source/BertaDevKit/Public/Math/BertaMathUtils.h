// BertaMathUtils.h
#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "BertaMathUtils.generated.h"

/**
 * Pure math utilities that complement UKismetMathLibrary.
 *
 * All functions are stateless and side-effect-free. They operate exclusively
 * on numeric types, vectors, and rotators — no world context required.
 *
 * @note Functions marked BlueprintThreadSafe can be called from Animation
 *       Blueprints and multi-threaded contexts safely.
 */
UCLASS()
class BERTADEVKIT_API UBertaMathUtils : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// -------------------------------------------------------------------------
	// Remapping & Normalization
	// -------------------------------------------------------------------------

	/**
	 * Normalizes Value from [RangeMin, RangeMax] to [0, 1].
	 * Returns 0 if RangeMin == RangeMax to avoid division by zero.
	 *
	 * @param Value      The value to normalize.
	 * @param RangeMin   The lower bound of the input range.
	 * @param RangeMax   The upper bound of the input range.
	 * @return           Normalized value in [0, 1], clamped.
	 */
	UFUNCTION(BlueprintCallable,
		BlueprintPure,
		Category = "Berta|Math|Remap",
		meta = (BlueprintThreadSafe))
	static float NormalizeToRange(float Value,
	                              float RangeMin,
	                              float RangeMax);

	/**
	 * Remaps Value from [InMin, InMax] to [OutMin, OutMax], clamped to the output range.
	 * The clamp is applied by NormalizeToRange, which constrains the alpha to [0, 1]
	 * before the Lerp — not by an explicit FMath::Clamp on the output.
	 *
	 * @param Value    The value to remap.
	 * @param InMin    Lower bound of the input range.
	 * @param InMax    Upper bound of the input range.
	 * @param OutMin   Lower bound of the output range.
	 * @param OutMax   Upper bound of the output range.
	 * @return         Remapped value, clamped to [OutMin, OutMax].
	 */
	UFUNCTION(BlueprintCallable,
		BlueprintPure,
		Category = "Berta|Math|Remap",
		meta = (BlueprintThreadSafe))
	static float RemapClamped(float Value,
	                          float InMin,
	                          float InMax,
	                          float OutMin,
	                          float OutMax);

	/**
	 * Remaps Value from [InMin, InMax] to [OutMin, OutMax] without clamping.
	 * Values outside the input range produce extrapolated output values.
	 *
	 * @param Value    The value to remap.
	 * @param InMin    Lower bound of the input range.
	 * @param InMax    Upper bound of the input range.
	 * @param OutMin   Lower bound of the output range.
	 * @param OutMax   Upper bound of the output range.
	 * @return         Remapped value, not clamped.
	 */
	UFUNCTION(BlueprintCallable,
		BlueprintPure,
		Category = "Berta|Math|Remap",
		meta = (BlueprintThreadSafe))
	static float RemapUnclamped(float Value,
	                            float InMin,
	                            float InMax,
	                            float OutMin,
	                            float OutMax);

	// -------------------------------------------------------------------------
	// Easing Functions
	// All easing functions expect Alpha in [0, 1] and return a value in [0, 1].
	// -------------------------------------------------------------------------

	/**
	 * Quadratic ease-in: slow start, fast end.
	 *
	 * @param Alpha   Normalized input in [0, 1].
	 * @return        Eased value in [0, 1].
	 */
	UFUNCTION(BlueprintCallable,
		BlueprintPure,
		Category = "Berta|Math|Easing",
		meta = (BlueprintThreadSafe))
	static float EaseInQuad(float Alpha);

	/**
	 * Quadratic ease-out: fast start, slow end.
	 *
	 * @param Alpha   Normalized input in [0, 1].
	 * @return        Eased value in [0, 1].
	 */
	UFUNCTION(BlueprintCallable,
		BlueprintPure,
		Category = "Berta|Math|Easing",
		meta = (BlueprintThreadSafe))
	static float EaseOutQuad(float Alpha);

	/**
	 * Quadratic ease-in-out: slow start, fast middle, slow end.
	 *
	 * @param Alpha   Normalized input in [0, 1].
	 * @return        Eased value in [0, 1].
	 */
	UFUNCTION(BlueprintCallable,
		BlueprintPure,
		Category = "Berta|Math|Easing",
		meta = (BlueprintThreadSafe))
	static float EaseInOutQuad(float Alpha);

	/**
	 * Cubic ease-in: stronger acceleration than quadratic.
	 *
	 * @param Alpha   Normalized input in [0, 1].
	 * @return        Eased value in [0, 1].
	 */
	UFUNCTION(BlueprintCallable,
		BlueprintPure,
		Category = "Berta|Math|Easing",
		meta = (BlueprintThreadSafe))
	static float EaseInCubic(float Alpha);

	/**
	 * Cubic ease-out: stronger deceleration than quadratic.
	 *
	 * @param Alpha   Normalized input in [0, 1].
	 * @return        Eased value in [0, 1].
	 */
	UFUNCTION(BlueprintCallable,
		BlueprintPure,
		Category = "Berta|Math|Easing",
		meta = (BlueprintThreadSafe))
	static float EaseOutCubic(float Alpha);

	/**
	 * Cubic ease-in-out: stronger acceleration and deceleration than quadratic.
	 *
	 * @param Alpha   Normalized input in [0, 1].
	 * @return        Eased value in [0, 1].
	 */
	UFUNCTION(BlueprintCallable,
		BlueprintPure,
		Category = "Berta|Math|Easing",
		meta = (BlueprintThreadSafe))
	static float EaseInOutCubic(float Alpha);

	/**
	 * SmoothStep: cubic Hermite interpolation — zero first derivative at both endpoints.
	 * Identical to the GLSL/HLSL smoothstep when Edge0=0 and Edge1=1.
	 *
	 * @param Alpha   Normalized input in [0, 1].
	 * @return        Smoothed value in [0, 1].
	 */
	UFUNCTION(BlueprintCallable,
		BlueprintPure,
		Category = "Berta|Math|Easing",
		meta = (BlueprintThreadSafe))
	static float SmoothStep(float Alpha);

	// -------------------------------------------------------------------------
	// Angular Operations
	// -------------------------------------------------------------------------

	/**
	 * Returns the shortest signed delta (in degrees) from AngleFrom to AngleTo.
	 * Result is always in [-180, 180].
	 *
	 * Example: ShortestAngleDelta(350, 10) == 20, not -340.
	 *
	 * @param AngleFrom   Starting angle in degrees.
	 * @param AngleTo     Target angle in degrees.
	 * @return            Shortest signed delta in [-180, 180].
	 */
	UFUNCTION(BlueprintCallable,
		BlueprintPure,
		Category = "Berta|Math|Angular",
		meta = (BlueprintThreadSafe))
	static float ShortestAngleDelta(float AngleFrom,
	                                float AngleTo);

	/**
	 * Normalizes an angle in degrees to the range [-180, 180].
	 *
	 * @param Angle   Angle in degrees, any value.
	 * @return        Equivalent angle in [-180, 180].
	 */
	UFUNCTION(BlueprintCallable,
		BlueprintPure,
		Category = "Berta|Math|Angular",
		meta = (BlueprintThreadSafe))
	static float NormalizeAngle180(float Angle);

	/**
	 * Returns true if TestAngle lies within the arc centered at CenterAngle
	 * with the given half-width (all values in degrees).
	 *
	 * Handles wrap-around correctly (e.g., arc centered at 350 with HalfArcDeg=20
	 * covers [330, 370] mod 360, so 5 degrees is inside).
	 *
	 * @param TestAngle      The angle to test, in degrees.
	 * @param CenterAngle    Center of the arc, in degrees.
	 * @param HalfArcDeg     Half the arc width, in degrees. Must be in [0, 180].
	 * @return               True if TestAngle is within the arc.
	 */
	UFUNCTION(BlueprintCallable,
		BlueprintPure,
		Category = "Berta|Math|Angular",
		meta = (BlueprintThreadSafe))
	static bool IsAngleInArc(float TestAngle,
	                         float CenterAngle,
	                         float HalfArcDeg);

	// -------------------------------------------------------------------------
	// Snap & Distribution
	// -------------------------------------------------------------------------

	/**
	 * Snaps Value to the nearest multiple of GridSize.
	 * Returns Value unchanged if GridSize <= 0.
	 *
	 * @param Value      The value to snap.
	 * @param GridSize   The grid interval. Must be > 0.
	 * @return           Value snapped to the nearest multiple of GridSize.
	 */
	UFUNCTION(BlueprintCallable,
		BlueprintPure,
		Category = "Berta|Math|Snap",
		meta = (BlueprintThreadSafe))
	static float SnapToMultiple(float Value,
	                            float GridSize);

	/**
	 * Rounds Value to the nearest multiple of Step.
	 * Alias for SnapToMultiple with identical behavior — provided for naming clarity
	 * in contexts where "round" is more intuitive than "snap".
	 *
	 * @param Value   The value to round.
	 * @param Step    The rounding interval. Must be > 0.
	 * @return        Value rounded to the nearest multiple of Step.
	 */
	UFUNCTION(BlueprintCallable,
		BlueprintPure,
		Category = "Berta|Math|Snap",
		meta = (BlueprintThreadSafe))
	static float RoundToMultiple(float Value,
	                             float Step);

	/**
	 * Distributes Count points evenly along the segment [Start, End].
	 * If Count == 1, returns a single point at the midpoint.
	 * If Count <= 0, returns an empty array.
	 *
	 * @param Start   Start of the segment.
	 * @param End     End of the segment.
	 * @param Count   Number of points to distribute.
	 * @return        Array of Count evenly-spaced world positions.
	 */
	UFUNCTION(BlueprintCallable,
		BlueprintPure,
		Category = "Berta|Math|Snap",
		meta = (BlueprintThreadSafe))
	static TArray<FVector> DistributePointsOnLine(FVector Start,
	                                              FVector End,
	                                              int32 Count);

	// -------------------------------------------------------------------------
	// Simple Prediction
	// -------------------------------------------------------------------------

	/**
	 * Predicts the world position of a moving object after DeltaTime seconds,
	 * assuming constant velocity (no acceleration, no gravity).
	 *
	 * Useful for AI targeting, debug overlays, and lead-shot calculations
	 * where a full physics simulation is not required.
	 *
	 * @param CurrentPosition   Current world position of the object.
	 * @param Velocity          Current velocity vector (units per second).
	 * @param DeltaTime         Time ahead to predict, in seconds.
	 * @return                  Predicted world position.
	 */
	UFUNCTION(BlueprintCallable,
		BlueprintPure,
		Category = "Berta|Math|Prediction",
		meta = (BlueprintThreadSafe))
	static FVector PredictFuturePosition(FVector CurrentPosition,
	                                     FVector Velocity,
	                                     float DeltaTime);

	/**
	 * Computes the world position where a projectile launched from Origin
	 * in Direction at LaunchSpeed (units/s) will hit a horizontal plane at PlaneZ,
	 * accounting for gravity (units/s²). Returns false if the projectile never
	 * reaches the plane (e.g., launched upward away from it).
	 *
	 * @param Origin             Launch world position.
	 * @param Direction          Launch direction (does not need to be normalized).
	 * @param LaunchSpeed        Scalar speed at launch, in units per second.
	 * @param PlaneZ             World Z of the target horizontal plane.
	 * @param GravityZ           Gravity acceleration in units/s². Must be negative for
	 *                           downward gravity (e.g., -980 for UE default).
	 *                           A positive value simulates upward gravity.
	 * @param OutImpactPoint     World position of the impact if the function returns true.
	 * @return                   True if the projectile reaches the plane; false otherwise.
	 */
	UFUNCTION(BlueprintCallable,
		BlueprintPure,
		Category = "Berta|Math|Prediction",
		meta = (BlueprintThreadSafe))
	static bool ProjectileImpactPoint(FVector Origin,
	                                  FVector Direction,
	                                  float LaunchSpeed,
	                                  float PlaneZ,
	                                  float GravityZ,
	                                  FVector& OutImpactPoint);
};
