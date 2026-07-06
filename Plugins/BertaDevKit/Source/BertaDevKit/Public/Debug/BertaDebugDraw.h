#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "BertaDebugDraw.generated.h"

class UCapsuleComponent;
class USphereComponent;
class UBoxComponent;

/**
 * Blueprint and C++ utility library for in-world debug drawing.
 *
 * All functions are stripped from Shipping builds via the DevelopmentOnly meta tag —
 * zero runtime overhead in production. Shapes are drawn using Unreal's built-in
 * DrawDebug* family of functions.
 *
 * Duration behaviour:
 *   Duration <= 0.0f  →  shape persists until manually cleared or until PIE ends.
 *   Duration >  0.0f  →  shape disappears after that many seconds.
 *
 * @warning Persistent shapes accumulate every call. Avoid Duration = 0.0f inside
 *          Tick or any high-frequency event — the screen will fill up quickly.
 *          Use a positive Duration or call FlushPersistentShapes to clear them.
 */
UCLASS()
class BERTADEVKIT_API UBertaDebugDraw : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Draws a debug sphere in the world.
	 *
	 * Useful for visualising ability ranges, overlap radii, and area-of-effect boundaries.
	 *
	 * @param WorldContextObject  Any UObject with a valid world context (e.g. an Actor).
	 * @param bEnabled            If false, the function returns immediately without drawing anything.
	 *                            Connect your debug boolean variable here to gate the output.
	 * @param Center              World-space centre of the sphere.
	 * @param Radius              Radius in Unreal units.
	 * @param Color               Draw color.
	 * @param Duration            How long the sphere stays visible in seconds.
	 *                            Pass 0.0f (default) to make it persistent.
	 * @param Segments            Number of longitude segments. Higher = smoother, costlier.
	 * @param Thickness           Line thickness in pixels.
	 * @warning Calling this every Tick with Duration = 0.0f accumulates persistent shapes.
	 */
	UFUNCTION(BlueprintCallable,
		Category = "Berta|Debug|Draw",
		meta = (WorldContext = "WorldContextObject", DevelopmentOnly, DisplayName = "Draw Debug Sphere", AdvancedDisplay
			= "Segments, Thickness"))
	static void DrawSphere(const UObject* WorldContextObject,
	                       bool bEnabled = true,
	                       FVector Center = FVector::ZeroVector,
	                       float Radius = 50.0f,
	                       FLinearColor Color = FLinearColor::Red,
	                       float Duration = 0.0f,
	                       int32 Segments = 12,
	                       float Thickness = 1.0f);

	/**
	 * Draws a debug sphere matching the dimensions and world transform of a USphereComponent.
	 *
	 * Eliminates the boilerplate of extracting Center and Radius from the component
	 * before calling DrawSphere. The shape is drawn at the component's current world location.
	 *
	 * @param SphereComponent  The sphere component to visualise. Must be valid.
	 * @param bEnabled         If false, the function returns immediately without drawing anything.
	 *                         Connect your debug boolean variable here to gate the output.
	 * @param Color            Draw color.
	 * @param Duration         How long the sphere stays visible in seconds.
	 *                         Pass 0.0f (default) to make it persistent.
	 * @param Segments         Number of longitude segments. Higher = smoother, costlier.
	 * @param Thickness        Line thickness in pixels.
	 * @warning Calling this every Tick with Duration = 0.0f accumulates persistent shapes.
	 */
	UFUNCTION(BlueprintCallable,
		Category = "Berta|Debug|Draw",
		meta = (DevelopmentOnly, DisplayName = "Draw Debug Sphere From Component", AdvancedDisplay =
			"Segments, Thickness"))
	static void DrawSphereFromComponent(const USphereComponent* SphereComponent,
	                                    bool bEnabled = true,
	                                    FLinearColor Color = FLinearColor::Red,
	                                    float Duration = 0.0f,
	                                    int32 Segments = 12,
	                                    float Thickness = 1.0f);

	/**
	 * Draws a debug line between two world-space points.
	 *
	 * Useful for visualising traces, aim directions, and movement vectors.
	 *
	 * @param WorldContextObject  Any UObject with a valid world context (e.g. an Actor).
	 * @param bEnabled            If false, the function returns immediately without drawing anything.
	 *                            Connect your debug boolean variable here to gate the output.
	 * @param Start               World-space start point.
	 * @param End                 World-space end point.
	 * @param Color               Draw color.
	 * @param Duration            How long the line stays visible in seconds.
	 *                            Pass 0.0f (default) to make it persistent.
	 * @param Thickness           Line thickness in pixels.
	 * @warning Calling this every Tick with Duration = 0.0f accumulates persistent shapes.
	 */
	UFUNCTION(BlueprintCallable,
		Category = "Berta|Debug|Draw",
		meta = (WorldContext = "WorldContextObject", DevelopmentOnly, DisplayName = "Draw Debug Line", AdvancedDisplay =
			"Thickness"))
	static void DrawLine(const UObject* WorldContextObject,
	                     bool bEnabled = true,
	                     FVector Start = FVector::ZeroVector,
	                     FVector End = FVector::ZeroVector,
	                     FLinearColor Color = FLinearColor::Red,
	                     float Duration = 0.0f,
	                     float Thickness = 1.0f);

	/**
	 * Draws an axis-aligned debug box centred at a world-space location.
	 *
	 * Useful for visualising hitboxes, collision volumes, and detection zones.
	 *
	 * @note This function draws an axis-aligned bounding box (AABB). If you need
	 *       a rotated box, use DrawBoxFromComponent which reads the component's orientation.
	 *
	 * @param WorldContextObject  Any UObject with a valid world context (e.g. an Actor).
	 * @param bEnabled            If false, the function returns immediately without drawing anything.
	 *                            Connect your debug boolean variable here to gate the output.
	 * @param Center              World-space centre of the box.
	 * @param Extent              Half-extents along each axis (X, Y, Z).
	 * @param Color               Draw color.
	 * @param Duration            How long the box stays visible in seconds.
	 *                            Pass 0.0f (default) to make it persistent.
	 * @param Thickness           Line thickness in pixels.
	 * @warning Calling this every Tick with Duration = 0.0f accumulates persistent shapes.
	 */
	UFUNCTION(BlueprintCallable,
		Category = "Berta|Debug|Draw",
		meta = (WorldContext = "WorldContextObject", DevelopmentOnly, DisplayName = "Draw Debug Box", AdvancedDisplay =
			"Thickness"))
	static void DrawBox(const UObject* WorldContextObject,
	                    bool bEnabled = true,
	                    FVector Center = FVector::ZeroVector,
	                    FVector Extent = FVector(50.0f,
	                                             50.0f,
	                                             50.0f),
	                    FLinearColor Color = FLinearColor::Red,
	                    float Duration = 0.0f,
	                    float Thickness = 1.0f);

	/**
	 * Draws a debug box matching the dimensions and world transform of a UBoxComponent.
	 *
	 * Eliminates the boilerplate of extracting Center, Extent, and rotation from the
	 * component before calling DrawBox. The shape is drawn at the component's current
	 * world location and orientation.
	 *
	 * @param BoxComponent  The box component to visualise. Must be valid.
	 * @param bEnabled      If false, the function returns immediately without drawing anything.
	 *                      Connect your debug boolean variable here to gate the output.
	 * @param Color         Draw color.
	 * @param Duration      How long the box stays visible in seconds.
	 *                      Pass 0.0f (default) to make it persistent.
	 * @param Thickness     Line thickness in pixels.
	 * @warning Calling this every Tick with Duration = 0.0f accumulates persistent shapes.
	 */
	UFUNCTION(BlueprintCallable,
		Category = "Berta|Debug|Draw",
		meta = (DevelopmentOnly, DisplayName = "Draw Debug Box From Component", AdvancedDisplay = "Thickness"))
	static void DrawBoxFromComponent(const UBoxComponent* BoxComponent,
	                                 bool bEnabled = true,
	                                 FLinearColor Color = FLinearColor::Red,
	                                 float Duration = 0.0f,
	                                 float Thickness = 1.0f);

	/**
	 * Draws a debug capsule centred at a world-space location.
	 *
	 * Useful for visualising character collision bounds and overlap checks in GAS abilities.
	 *
	 * @param WorldContextObject  Any UObject with a valid world context (e.g. an Actor).
	 * @param bEnabled            If false, the function returns immediately without drawing anything.
	 *                            Connect your debug boolean variable here to gate the output.
	 * @param Center              World-space centre of the capsule.
	 * @param HalfHeight          Half the total height of the capsule along its up axis.
	 * @param Radius              Radius of the capsule's hemispherical caps.
	 * @param Color               Draw color.
	 * @param Duration            How long the capsule stays visible in seconds.
	 *                            Pass 0.0f (default) to make it persistent.
	 * @param Thickness           Line thickness in pixels.
	 * @warning Calling this every Tick with Duration = 0.0f accumulates persistent shapes.
	 */
	UFUNCTION(BlueprintCallable,
		Category = "Berta|Debug|Draw",
		meta = (WorldContext = "WorldContextObject", DevelopmentOnly, DisplayName = "Draw Debug Capsule",
			AdvancedDisplay = "Thickness"))
	static void DrawCapsule(const UObject* WorldContextObject,
	                        bool bEnabled = true,
	                        FVector Center = FVector::ZeroVector,
	                        float HalfHeight = 88.0f,
	                        float Radius = 34.0f,
	                        FLinearColor Color = FLinearColor::Red,
	                        float Duration = 0.0f,
	                        float Thickness = 1.0f);

	/**
	 * Draws a debug capsule matching the dimensions and world transform of a UCapsuleComponent.
	 *
	 * Eliminates the boilerplate of extracting Center, HalfHeight, and Radius from the
	 * component before calling DrawCapsule. The shape is drawn at the component's current
	 * world location and orientation.
	 *
	 * @param CapsuleComponent  The capsule component to visualise. Must be valid.
	 * @param bEnabled          If false, the function returns immediately without drawing anything.
	 *                          Connect your debug boolean variable here to gate the output.
	 * @param Color             Draw color.
	 * @param Duration          How long the capsule stays visible in seconds.
	 *                          Pass 0.0f (default) to make it persistent.
	 * @param Thickness         Line thickness in pixels.
	 * @warning Calling this every Tick with Duration = 0.0f accumulates persistent shapes.
	 */
	UFUNCTION(BlueprintCallable,
		Category = "Berta|Debug|Draw",
		meta = (DevelopmentOnly, DisplayName = "Draw Debug Capsule From Component", AdvancedDisplay = "Thickness"))
	static void DrawCapsuleFromComponent(const UCapsuleComponent* CapsuleComponent,
	                                     bool bEnabled = true,
	                                     FLinearColor Color = FLinearColor::Red,
	                                     float Duration = 0.0f,
	                                     float Thickness = 1.0f);

	/**
	 * Draws a debug point at a world-space location.
	 *
	 * Useful for marking hit locations, impact points, and spawn positions.
	 *
	 * @param WorldContextObject  Any UObject with a valid world context (e.g. an Actor).
	 * @param bEnabled            If false, the function returns immediately without drawing anything.
	 *                            Connect your debug boolean variable here to gate the output.
	 * @param Location            World-space position of the point.
	 * @param Color               Draw color.
	 * @param Duration            How long the point stays visible in seconds.
	 *                            Pass 0.0f (default) to make it persistent.
	 * @param Size                Visual size of the point in pixels.
	 * @warning Calling this every Tick with Duration = 0.0f accumulates persistent shapes.
	 */
	UFUNCTION(BlueprintCallable,
		Category = "Berta|Debug|Draw",
		meta = (WorldContext = "WorldContextObject", DevelopmentOnly, DisplayName = "Draw Debug Point", AdvancedDisplay
			= "Size"))
	static void DrawPoint(const UObject* WorldContextObject,
	                      bool bEnabled = true,
	                      FVector Location = FVector::ZeroVector,
	                      FLinearColor Color = FLinearColor::Red,
	                      float Duration = 0.0f,
	                      float Size = 10.0f);

	/**
	 * Draws a debug arrow between two world-space points.
	 *
	 * Useful for visualising trace directions, aim vectors, knockback directions,
	 * and any other quantity that has both magnitude and orientation.
	 *
	 * @param WorldContextObject  Any UObject with a valid world context (e.g. an Actor).
	 * @param bEnabled            If false, the function returns immediately without drawing anything.
	 *                            Connect your debug boolean variable here to gate the output.
	 * @param Start               World-space start point (tail of the arrow).
	 * @param End                 World-space end point (tip of the arrow).
	 * @param ArrowSize           Size of the arrowhead in Unreal units.
	 * @param Color               Draw color.
	 * @param Duration            How long the arrow stays visible in seconds.
	 *                            Pass 0.0f (default) to make it persistent.
	 * @param Thickness           Line thickness in pixels.
	 * @warning Calling this every Tick with Duration = 0.0f accumulates persistent shapes.
	 */
	UFUNCTION(BlueprintCallable,
		Category = "Berta|Debug|Draw",
		meta = (WorldContext = "WorldContextObject", DevelopmentOnly, DisplayName = "Draw Debug Arrow", AdvancedDisplay
			= "Thickness"))
	static void DrawArrow(const UObject* WorldContextObject,
	                      bool bEnabled = true,
	                      FVector Start = FVector::ZeroVector,
	                      FVector End = FVector::ZeroVector,
	                      float ArrowSize = 50.0f,
	                      FLinearColor Color = FLinearColor::Red,
	                      float Duration = 0.0f,
	                      float Thickness = 1.0f);

	/**
	 * Draws a debug string at a world-space location.
	 *
	 * Useful for annotating shapes with labels, values, or state information
	 * directly in the viewport — without losing spatial reference.
	 *
	 * @param WorldContextObject  Any UObject with a valid world context (e.g. an Actor).
	 * @param bEnabled            If false, the function returns immediately without drawing anything.
	 *                            Connect your debug boolean variable here to gate the output.
	 * @param Location            World-space position where the text is anchored.
	 * @param Text                The string to display.
	 * @param Color               Text color.
	 * @param Duration            How long the text stays visible in seconds.
	 *                            Pass 0.0f (default) to make it persistent.
	 * @param FontScale           Scale multiplier applied to the default font size.
	 * @warning Calling this every Tick with Duration = 0.0f accumulates persistent strings.
	 *          Call FlushPersistentShapes to clear them.
	 */
	UFUNCTION(BlueprintCallable,
		Category = "Berta|Debug|Draw",
		meta = (WorldContext = "WorldContextObject", DevelopmentOnly, DisplayName = "Draw Debug String", AdvancedDisplay
			= "FontScale"))
	static void DrawString(const UObject* WorldContextObject,
	                       bool bEnabled = true,
	                       FVector Location = FVector::ZeroVector,
	                       const FString& Text = TEXT(""),
	                       FLinearColor Color = FLinearColor::White,
	                       float Duration = 0.0f,
	                       float FontScale = 1.0f);

	/**
	 * Draws three orthogonal axes (X, Y, Z) at a world-space transform.
	 *
	 * Useful for visualising object orientation, socket transforms, bone transforms,
	 * and the result of any rotation or coordinate-space calculation.
	 * Axes are color-coded by the engine: red = X, green = Y, blue = Z.
	 *
	 * @param WorldContextObject  Any UObject with a valid world context (e.g. an Actor).
	 * @param bEnabled            If false, the function returns immediately without drawing anything.
	 *                            Connect your debug boolean variable here to gate the output.
	 * @param Location            World-space origin of the coordinate system.
	 * @param Rotation            Orientation of the axes in world space.
	 * @param AxisLength          Length of each axis line in Unreal units.
	 * @param Duration            How long the axes stay visible in seconds.
	 *                            Pass 0.0f (default) to make it persistent.
	 * @param Thickness           Line thickness in pixels.
	 * @warning Calling this every Tick with Duration = 0.0f accumulates persistent shapes.
	 */
	UFUNCTION(BlueprintCallable,
		Category = "Berta|Debug|Draw",
		meta = (WorldContext = "WorldContextObject", DevelopmentOnly, DisplayName = "Draw Debug Coordinate System",
			AdvancedDisplay = "Thickness"))
	static void DrawCoordinateSystem(const UObject* WorldContextObject,
	                                 bool bEnabled = true,
	                                 FVector Location = FVector::ZeroVector,
	                                 FRotator Rotation = FRotator::ZeroRotator,
	                                 float AxisLength = 50.0f,
	                                 float Duration = 0.0f,
	                                 float Thickness = 1.0f);

	/**
	 * Clears all persistent debug shapes and strings from the world.
	 *
	 * Wraps FlushPersistentDebugLines and FlushDebugStrings in a single Blueprint-callable
	 * function. Call this to reset the viewport after drawing persistent shapes.
	 *
	 * @param WorldContextObject  Any UObject with a valid world context (e.g. an Actor).
	 */
	UFUNCTION(BlueprintCallable,
		Category = "Berta|Debug|Draw",
		meta = (WorldContext = "WorldContextObject", DevelopmentOnly, DisplayName = "Flush Persistent Debug Shapes"))
	static void FlushPersistentShapes(const UObject* WorldContextObject);

private:
	/**
	 * Validates WorldContextObject and retrieves the UWorld from it.
	 *
	 * Centralises the two-step guard that every public function needs:
	 * first checking the context object, then resolving the world from it.
	 * If either step fails, the function logs the failure and returns nullptr
	 * so the caller can do a single null check instead of eight lines of guards.
	 *
	 * @param WorldContextObject  The UObject to resolve the world from.
	 * @param CallerName          Name of the calling function, used in log and ensure messages.
	 * @return                    The resolved UWorld, or nullptr if validation failed.
	 */
	static const UWorld* ResolveWorld(const UObject* WorldContextObject,
	                                  const TCHAR* CallerName);

	/**
	 * Resolves Duration to the float expected by DrawDebug* functions.
	 *
	 * DrawDebug* uses -1.0f for persistent and a positive value for timed shapes.
	 * Centralising this avoids repeating the conditional in every public function.
	 *
	 * @param Duration  The duration value passed by the caller.
	 * @return          -1.0f if Duration <= 0.0f, otherwise Duration unchanged.
	 */
	static float ResolveLifeTime(float Duration);

	/**
	 * Converts an FLinearColor to the FColor expected by DrawDebug* functions.
	 *
	 * DrawDebug* predates FLinearColor — it takes FColor (8-bit sRGB).
	 * Accepting FLinearColor in the public API gives Blueprint callers access
	 * to the colour picker widget instead of a plain integer field.
	 *
	 * @param LinearColor  Linear-space colour from the caller.
	 * @return             Quantised sRGB FColor.
	 */
	static FColor ToFColor(const FLinearColor& LinearColor);
};
