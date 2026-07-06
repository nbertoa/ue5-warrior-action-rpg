// BertaMathUtils.cpp

#include "Math/BertaMathUtils.h"

#include "Math/UnrealMathUtility.h"

// -----------------------------------------------------------------------------
// Remapping & Normalization
// -----------------------------------------------------------------------------

float UBertaMathUtils::NormalizeToRange(const float Value,
                                        const float RangeMin,
                                        const float RangeMax)
{
	// Guard against degenerate range to avoid division by zero.
	// A zero-width range has no meaningful normalization — return 0 as a safe default.
	if (FMath::IsNearlyEqual(RangeMin,
	                         RangeMax))
	{
		return 0.0f;
	}

	const float Normalized = (Value - RangeMin) / (RangeMax - RangeMin);
	return FMath::Clamp(Normalized,
	                    0.0f,
	                    1.0f);
}

float UBertaMathUtils::RemapClamped(const float Value,
                                    const float InMin,
                                    const float InMax,
                                    const float OutMin,
                                    const float OutMax)
{
	// Normalize to [0,1] first, then scale to the output range.
	// NormalizeToRange clamps the alpha to [0,1] — that clamp is what prevents
	// the output from exceeding [OutMin, OutMax]. There is no separate clamp on
	// the output; the constraint comes entirely from the alpha being bounded.
	const float Alpha = NormalizeToRange(Value,
	                                     InMin,
	                                     InMax);
	return FMath::Lerp(OutMin,
	                   OutMax,
	                   Alpha);
}

float UBertaMathUtils::RemapUnclamped(const float Value,
                                      const float InMin,
                                      const float InMax,
                                      const float OutMin,
                                      const float OutMax)
{
	// Same as RemapClamped but intentionally no clamping on the alpha —
	// values outside [InMin, InMax] produce extrapolated results beyond [OutMin, OutMax].
	if (FMath::IsNearlyEqual(InMin,
	                         InMax))
	{
		return OutMin;
	}

	const float Alpha = (Value - InMin) / (InMax - InMin);
	return FMath::Lerp(OutMin,
	                   OutMax,
	                   Alpha);
}

// -----------------------------------------------------------------------------
// Easing Functions
// -----------------------------------------------------------------------------

float UBertaMathUtils::EaseInQuad(const float Alpha)
{
	// f(t) = t² — acceleration from zero velocity.
	return Alpha * Alpha;
}

float UBertaMathUtils::EaseOutQuad(const float Alpha)
{
	// f(t) = 1 - (1-t)² — deceleration to zero velocity.
	// Derived by mirroring EaseInQuad around the center point.
	const float OneMinusAlpha = 1.0f - Alpha;
	return 1.0f - (OneMinusAlpha * OneMinusAlpha);
}

float UBertaMathUtils::EaseInOutQuad(const float Alpha)
{
	// Blend EaseInQuad for the first half and EaseOutQuad for the second half.
	// The branch at 0.5 ensures C1 continuity (matching first derivatives).
	if (Alpha < 0.5f)
	{
		// First half: apply ease-in on doubled alpha, then halve the result.
		return 2.0f * Alpha * Alpha;
	}

	// Second half: mirror of ease-in, shifted and scaled to [0.5, 1].
	const float ShiftedAlpha = Alpha - 1.0f;
	return 1.0f - 2.0f * ShiftedAlpha * ShiftedAlpha;
}

float UBertaMathUtils::EaseInCubic(const float Alpha)
{
	// f(t) = t³ — stronger acceleration than quadratic.
	return Alpha * Alpha * Alpha;
}

float UBertaMathUtils::EaseOutCubic(const float Alpha)
{
	// f(t) = 1 - (1-t)³ — stronger deceleration than quadratic.
	const float OneMinusAlpha = 1.0f - Alpha;
	return 1.0f - (OneMinusAlpha * OneMinusAlpha * OneMinusAlpha);
}

float UBertaMathUtils::EaseInOutCubic(const float Alpha)
{
	// Same blend strategy as EaseInOutQuad but with cubic exponents.
	if (Alpha < 0.5f)
	{
		return 4.0f * Alpha * Alpha * Alpha;
	}

	const float ShiftedAlpha = Alpha - 1.0f;
	return 1.0f + 4.0f * ShiftedAlpha * ShiftedAlpha * ShiftedAlpha;
}

float UBertaMathUtils::SmoothStep(const float Alpha)
{
	// Cubic Hermite: f(t) = t²(3 - 2t)
	// Guarantees f(0)=0, f(1)=1, f'(0)=0, f'(1)=0.
	// Zero first derivative at both ends means no visible velocity discontinuity
	// when chaining animations — this is why it's preferred over linear Lerp for UI.
	const float ClampedAlpha = FMath::Clamp(Alpha,
	                                        0.0f,
	                                        1.0f);
	return ClampedAlpha * ClampedAlpha * (3.0f - 2.0f * ClampedAlpha);
}

// -----------------------------------------------------------------------------
// Angular Operations
// -----------------------------------------------------------------------------

float UBertaMathUtils::ShortestAngleDelta(const float AngleFrom,
                                          const float AngleTo)
{
	// Raw delta can land anywhere in (-360, 360).
	// Wrapping it to (-180, 180] gives us the shortest signed arc.
	const float RawDelta = AngleTo - AngleFrom;
	return FMath::UnwindDegrees(RawDelta);
}

float UBertaMathUtils::NormalizeAngle180(const float Angle)
{
	// UnwindDegrees maps any angle to (-180, 180] — exactly the range we want.
	return FMath::UnwindDegrees(Angle);
}

bool UBertaMathUtils::IsAngleInArc(const float TestAngle,
                                   const float CenterAngle,
                                   const float HalfArcDeg)
{
	// Convert the problem to a 1D distance check on the circle.
	// ShortestAngleDelta gives us the shortest path from center to test,
	// so comparing its absolute value to the half-arc handles wrap-around correctly.
	const float DeltaFromCenter = FMath::Abs(ShortestAngleDelta(CenterAngle,
	                                                            TestAngle));
	return DeltaFromCenter <= HalfArcDeg;
}

// -----------------------------------------------------------------------------
// Snap & Distribution
// -----------------------------------------------------------------------------

float UBertaMathUtils::SnapToMultiple(const float Value,
                                      const float GridSize)
{
	// A zero or negative grid size is a caller error — return unchanged to avoid NaN.
	if (GridSize <= 0.0f)
	{
		return Value;
	}

	// RoundToFloat gives us the nearest integer multiple of GridSize.
	return FMath::RoundToFloat(Value / GridSize) * GridSize;
}

float UBertaMathUtils::RoundToMultiple(const float Value,
                                       const float Step)
{
	// Identical algorithm to SnapToMultiple.
	// The separate function exists purely for naming clarity at the call site.
	return SnapToMultiple(Value,
	                      Step);
}

TArray<FVector> UBertaMathUtils::DistributePointsOnLine(const FVector Start,
                                                        const FVector End,
                                                        const int32 Count)
{
	TArray<FVector> Points;

	if (Count <= 0)
	{
		return Points;
	}

	Points.Reserve(Count);

	if (Count == 1)
	{
		// A single point has no "distribution" — place it at the midpoint.
		Points.Add(FMath::Lerp(Start,
		                       End,
		                       0.5f));
		return Points;
	}

	// Distribute Count points so the first lands exactly on Start
	// and the last exactly on End.
	const float StepAlpha = 1.0f / static_cast<float>(Count - 1);

	for (int32 Index = 0; Index < Count; ++Index)
	{
		const float Alpha = StepAlpha * static_cast<float>(Index);
		Points.Add(FMath::Lerp(Start,
		                       End,
		                       Alpha));
	}

	return Points;
}

// -----------------------------------------------------------------------------
// Simple Prediction
// -----------------------------------------------------------------------------

FVector UBertaMathUtils::PredictFuturePosition(const FVector CurrentPosition,
                                               const FVector Velocity,
                                               const float DeltaTime)
{
	// Kinematic equation: p(t) = p0 + v*t
	// No acceleration term — caller is responsible for using this only where
	// constant-velocity is a valid approximation (short time horizons, non-physics objects).
	return CurrentPosition + Velocity * DeltaTime;
}

bool UBertaMathUtils::ProjectileImpactPoint(const FVector Origin,
                                            const FVector Direction,
                                            const float LaunchSpeed,
                                            const float PlaneZ,
                                            const float GravityZ,
                                            FVector& OutImpactPoint)
{
	// Decompose launch into a velocity vector.
	// SafeNormal returns zero vector if Direction is degenerate — guard below.
	const FVector LaunchVelocity = Direction.GetSafeNormal() * LaunchSpeed;

	// Vertical component of launch velocity.
	const float Vz = LaunchVelocity.Z;

	// Vertical displacement to cover: PlaneZ - Origin.Z
	const float DeltaZ = PlaneZ - Origin.Z;

	// Solve the quadratic: 0.5*GravityZ*t² + Vz*t - DeltaZ = 0
	// Coefficients: A = 0.5*GravityZ, B = Vz, C = -DeltaZ
	const float A = 0.5f * GravityZ;
	const float B = Vz;
	const float C = -DeltaZ;

	// If GravityZ is zero, the path is linear — solve directly.
	if (FMath::IsNearlyZero(A))
	{
		// Linear case: Vz*t = DeltaZ → t = DeltaZ / Vz
		if (FMath::IsNearlyZero(B))
		{
			// No vertical motion and no gravity — projectile never reaches the plane.
			return false;
		}

		const float TimeOfImpact = DeltaZ / B;
		if (TimeOfImpact < 0.0f)
		{
			return false;
		}

		OutImpactPoint = Origin + LaunchVelocity * TimeOfImpact;
		return true;
	}

	// Discriminant of the quadratic.
	const float Discriminant = B * B - 4.0f * A * C;

	if (Discriminant < 0.0f)
	{
		// No real solution — the projectile never reaches the target plane.
		return false;
	}

	const float SqrtDiscriminant = FMath::Sqrt(Discriminant);

	// Two candidate times — take the smallest positive one (first crossing).
	const float T1 = (-B - SqrtDiscriminant) / (2.0f * A);
	const float T2 = (-B + SqrtDiscriminant) / (2.0f * A);

	float TimeOfImpact = -1.0f;

	if (T1 >= 0.0f)
	{
		TimeOfImpact = T1;
	}
	else if (T2 >= 0.0f)
	{
		TimeOfImpact = T2;
	}

	if (TimeOfImpact < 0.0f)
	{
		// Both solutions are in the past — projectile was launched away from the plane.
		return false;
	}

	OutImpactPoint = Origin + LaunchVelocity * TimeOfImpact;
	return true;
}
