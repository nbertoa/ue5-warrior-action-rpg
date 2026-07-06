#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "BertaDebugUtils.generated.h"

/**
 * Verbosity level for BertaDebugUtils log functions.
 * Controls both the on-screen color and the UE_LOG verbosity level.
 */
UENUM(BlueprintType)
enum class EBertaLogVerbosity : uint8
{
	Log UMETA(DisplayName = "Log"),
	Warning UMETA(DisplayName = "Warning"),
	Error UMETA(DisplayName = "Error")
};

/**
 * Output target for BertaDebugUtils log functions.
 * Controls whether the message goes to the screen, the Output Log, or both.
 */
UENUM(BlueprintType)
enum class EBertaLogOutput : uint8
{
	PrintAndLog UMETA(DisplayName = "Print And Log"),
	PrintOnly UMETA(DisplayName = "Print Only"),
	LogOnly UMETA(DisplayName = "Log Only")
};

/**
 * Utility library for unified debug logging.
 *
 * Every call prints to the on-screen debug display AND to the Output Log simultaneously
 * unless a specific EBertaLogOutput value is provided.
 * All functions are stripped from Shipping builds via the DevelopmentOnly meta specifier.
 * Available from both C++ and Blueprint graphs.
 *
 * @see EBertaLogVerbosity
 * @see EBertaLogOutput
 */
UCLASS()
class BERTADEVKIT_API UBertaDebugUtils : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Prints a message to the screen, the Output Log, or both.
	 *
	 * @param Message    The text to display.
	 * @param bEnabled   If false, the function returns immediately without logging anything.
	 *                   Connect your debug boolean variable here to gate the output.
	 * @param Verbosity  Controls the on-screen color and the log verbosity level.
	 * @param Output     Where to send the message: screen, log, or both.
	 * @param Duration   How many seconds the message stays on screen.
	 * @param Key        Unique key — same key overwrites the previous message instead of stacking.
	 *                   Pass -1 to always add a new line.
	 */
	UFUNCTION(BlueprintCallable,
		Category = "BertaDevKit|Debug",
		meta = (DevelopmentOnly, DisplayName = "Log", AdvancedDisplay = "Duration, Key"))
	static void PrintLog(const FString& Message,
	                     bool bEnabled = true,
	                     EBertaLogVerbosity Verbosity = EBertaLogVerbosity::Log,
	                     EBertaLogOutput Output = EBertaLogOutput::PrintAndLog,
	                     float Duration = 5.0f,
	                     int32 Key = -1);

	/**
	 * Prints the name of the calling object alongside a message.
	 * Useful for tracking which Actor or Component is emitting a log.
	 *
	 * @param WorldContext  Automatically bound to the calling object in Blueprint.
	 * @param Message       The text to display.
	 * @param bEnabled      If false, the function returns immediately without logging anything.
	 *                      Connect your debug boolean variable here to gate the output.
	 * @param Verbosity     Controls the on-screen color and the log verbosity level.
	 * @param Output        Where to send the message: screen, log, or both.
	 * @param Duration      Seconds on screen.
	 */
	UFUNCTION(BlueprintCallable,
		Category = "BertaDevKit|Debug",
		meta = (DevelopmentOnly, DisplayName = "Log With Context", DefaultToSelf = "WorldContext", HidePin =
			"WorldContext", AdvancedDisplay = "Duration"))
	static void PrintLogWithContext(const UObject* WorldContext,
	                                const FString& Message,
	                                bool bEnabled = true,
	                                EBertaLogVerbosity Verbosity = EBertaLogVerbosity::Log,
	                                EBertaLogOutput Output = EBertaLogOutput::PrintAndLog,
	                                float Duration = 5.0f);

	/**
	 * Prints a message to a named log category.
	 * Available from Blueprint.
	 *
	 * If the category exists in the project (declared with DEFINE_LOG_CATEGORY),
	 * the message is logged there correctly.
	 * If the category does not exist, UE creates it with default settings —
	 * behavior in that case is not guaranteed.
	 * On-screen display always works regardless of category validity.
	 *
	 * @param CategoryName  The exact name of an existing log category (e.g., "LogCombat").
	 * @param Message       The text to display.
	 * @param bEnabled      If false, the function returns immediately without logging anything.
	 *                      Connect your debug boolean variable here to gate the output.
	 * @param Verbosity     Controls the on-screen color and the log verbosity level.
	 * @param Output        Where to send the message: screen, log, or both.
	 * @param Duration      Seconds on screen.
	 * @param Key           Unique key — same key overwrites the previous message instead of stacking.
	 *                      Pass -1 to always add a new line.
	 */
	UFUNCTION(BlueprintCallable,
		Category = "BertaDevKit|Debug",
		meta = (DevelopmentOnly, DisplayName = "Log To Category", AdvancedDisplay = "Duration, Key"))
	static void PrintLogToNamedCategory(const FString& CategoryName,
	                                    const FString& Message,
	                                    bool bEnabled = true,
	                                    EBertaLogVerbosity Verbosity = EBertaLogVerbosity::Log,
	                                    EBertaLogOutput Output = EBertaLogOutput::PrintAndLog,
	                                    float Duration = 5.0f,
	                                    int32 Key = -1);

	/**
	 * Prints the name of the calling object alongside a message to a named log category.
	 * Combines the context prefix of Log With Context with the named category of Log To Category.
	 *
	 * If the category exists in the project (declared with DEFINE_LOG_CATEGORY),
	 * the message is logged there correctly.
	 * If the category does not exist, UE creates it with default settings —
	 * behavior in that case is not guaranteed.
	 * On-screen display always works regardless of category validity.
	 *
	 * @param WorldContext  Automatically bound to the calling object in Blueprint.
	 * @param CategoryName  The exact name of an existing log category (e.g., "LogCombat").
	 * @param Message       The text to display.
	 * @param bEnabled      If false, the function returns immediately without logging anything.
	 *                      Connect your debug boolean variable here to gate the output.
	 * @param Verbosity     Controls the on-screen color and the log verbosity level.
	 * @param Output        Where to send the message: screen, log, or both.
	 * @param Duration      Seconds on screen.
	 */
	UFUNCTION(BlueprintCallable,
		Category = "BertaDevKit|Debug",
		meta = (DevelopmentOnly, DisplayName = "Log To Category With Context", DefaultToSelf = "WorldContext", HidePin =
			"WorldContext", AdvancedDisplay = "Duration"))
	static void PrintLogToNamedCategoryWithContext(const UObject* WorldContext,
	                                               const FString& CategoryName,
	                                               const FString& Message,
	                                               bool bEnabled = true,
	                                               EBertaLogVerbosity Verbosity = EBertaLogVerbosity::Log,
	                                               EBertaLogOutput Output = EBertaLogOutput::PrintAndLog,
	                                               float Duration = 5.0f);

private:
	/**
	 * Builds a context-prefixed message from a UObject caller.
	 * If WorldContext is invalid, the prefix falls back to "NULL" to keep the
	 * message visible rather than silently dropping caller information.
	 *
	 * @param WorldContext  The calling object. May be null.
	 * @param Message       The original message to prefix.
	 * @return              A new FString in the format "[ObjectName] Message".
	 */
	static FString BuildContextMessage(const UObject* WorldContext,
	                                   const FString& Message);

	/**
	 * Maps a verbosity level to its corresponding on-screen display color.
	 *
	 * @param Verbosity  The verbosity level to map.
	 * @return           The FColor to use for on-screen display.
	 */
	static FColor GetColorForVerbosity(EBertaLogVerbosity Verbosity);

	/**
	 * Handles the actual print-to-screen logic.
	 * Extracted to avoid duplication across all public functions.
	 *
	 * @param Message    The text to display.
	 * @param Verbosity  Controls the on-screen color.
	 * @param Duration   Seconds on screen.
	 * @param Key        Unique key for overwrite behavior.
	 */
	static void PrintToScreen(const FString& Message,
	                          EBertaLogVerbosity Verbosity,
	                          float Duration,
	                          int32 Key);

	/**
	 * Handles the actual log-to-output logic.
	 * Extracted to avoid duplication across all public functions.
	 *
	 * @param CategoryName  The log category name to log into.
	 * @param Message       The text to log.
	 * @param Verbosity     Controls the log verbosity level.
	 */
	static void LogToOutput(const FName& CategoryName,
	                        const FString& Message,
	                        EBertaLogVerbosity Verbosity);
};
