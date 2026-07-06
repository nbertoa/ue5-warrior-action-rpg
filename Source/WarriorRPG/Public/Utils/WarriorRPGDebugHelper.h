// WarriorRPGDebugHelper.h
// Static utility class for on-screen debug messages paired with log output.
// Provides a single call that displays a message on the viewport AND writes it to the log.
// Intended for development only — all output is stripped from shipping builds automatically.

#pragma once

#include "CoreMinimal.h"

/**
 * Static debug helper for WarriorRPG.
 * Combines on-screen messages (AddOnScreenDebugMessage) with UE_LOG output
 * so every visual debug message is also captured in the log file.
 */
class DebugHelper
{
public:
	/**
	 * Displays a string message on screen and writes it to the log.
	 *
	 * @param Msg       The message to display.
	 * @param Color     On-screen text color. Defaults to a random color for easy visual distinction.
	 * @param InKey     Message key for on-screen display. Use -1 for non-replacing messages,
	 *                  or a positive integer to update an existing message in-place.
	 */
	static void Print(const FString& Msg,
	                  const FColor& Color = FColor::MakeRandomColor(),
	                  int32 InKey = -1);

	/**
	 * Displays a labeled float value on screen and writes it to the log.
	 *
	 * @param FloatTitle        Label prefix shown before the value.
	 * @param FloatValueToPrint The float value to display.
	 * @param InKey             Message key. Use -1 to always add a new line.
	 * @param Color             On-screen text color.
	 */
	static void Print(const FString& FloatTitle,
	                  float FloatValueToPrint,
	                  int32 InKey = -1,
	                  const FColor& Color = FColor::MakeRandomColor());

	/**
	 * Displays a labeled int32 value on screen and writes it to the log.
	 * Separate overload from the float version to avoid silent implicit conversions
	 * when passing integer combo counts and similar discrete values.
	 *
	 * @param IntTitle          Label prefix shown before the value.
	 * @param IntValueToPrint   The integer value to display.
	 * @param InKey             Message key. Use -1 to always add a new line.
	 * @param Color             On-screen text color.
	 */
	static void Print(const FString& IntTitle,
	                  int32 IntValueToPrint,
	                  int32 InKey = -1,
	                  const FColor& Color = FColor::MakeRandomColor());
};
