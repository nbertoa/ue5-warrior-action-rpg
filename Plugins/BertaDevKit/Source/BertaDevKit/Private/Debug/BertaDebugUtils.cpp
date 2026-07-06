#include "Debug/BertaDebugUtils.h"

#include "Debug/BertaDebugLog.h"
#include "Engine/Engine.h"
#include "Settings/BertaDevKitSettings.h"

void UBertaDebugUtils::PrintLog(const FString& Message,
                                const bool bEnabled,
                                const EBertaLogVerbosity Verbosity,
                                const EBertaLogOutput Output,
                                const float Duration,
                                const int32 Key)
{
	if (!bEnabled)
	{
		return;
	}

	// Check the global master switch from Project Settings.
	// UBertaDevKitSettings::Get() calls GetDefault<> which is always valid — no null check needed.
	if (!UBertaDevKitSettings::Get()->bDebugLogEnabled)
	{
		return;
	}

	if (Output == EBertaLogOutput::PrintAndLog || Output == EBertaLogOutput::PrintOnly)
	{
		PrintToScreen(Message,
		              Verbosity,
		              Duration,
		              Key);
	}

	if (Output == EBertaLogOutput::PrintAndLog || Output == EBertaLogOutput::LogOnly)
	{
		LogToOutput(LogBertaDebug.GetCategoryName(),
		            Message,
		            Verbosity);
	}
}

void UBertaDebugUtils::PrintLogWithContext(const UObject* WorldContext,
                                           const FString& Message,
                                           const bool bEnabled,
                                           const EBertaLogVerbosity Verbosity,
                                           const EBertaLogOutput Output,
                                           const float Duration)
{
	if (!bEnabled)
	{
		return;
	}

	// Check the global master switch from Project Settings.
	// UBertaDevKitSettings::Get() calls GetDefault<> which is always valid — no null check needed.
	if (!UBertaDevKitSettings::Get()->bDebugLogEnabled)
	{
		return;
	}

	// Delegate to PrintLog so all output routing logic stays centralized.
	// bEnabled is already confirmed true — pass true directly to avoid
	// a redundant check in the delegated call.
	PrintLog(BuildContextMessage(WorldContext,
	                             Message),
	         true,
	         Verbosity,
	         Output,
	         Duration,
	         /*Key=*/
	         -1);
}

void UBertaDebugUtils::PrintLogToNamedCategory(const FString& CategoryName,
                                               const FString& Message,
                                               const bool bEnabled,
                                               const EBertaLogVerbosity Verbosity,
                                               const EBertaLogOutput Output,
                                               const float Duration,
                                               const int32 Key)
{
	if (!bEnabled)
	{
		return;
	}

	// Check the global master switch from Project Settings.
	// UBertaDevKitSettings::Get() calls GetDefault<> which is always valid — no null check needed.
	if (!UBertaDevKitSettings::Get()->bDebugLogEnabled)
	{
		return;
	}

	if (Output == EBertaLogOutput::PrintAndLog || Output == EBertaLogOutput::PrintOnly)
	{
		PrintToScreen(Message,
		              Verbosity,
		              Duration,
		              Key);
	}

	if (Output == EBertaLogOutput::PrintAndLog || Output == EBertaLogOutput::LogOnly)
	{
		LogToOutput(FName(*CategoryName),
		            Message,
		            Verbosity);
	}
}

void UBertaDebugUtils::PrintLogToNamedCategoryWithContext(const UObject* WorldContext,
                                                          const FString& CategoryName,
                                                          const FString& Message,
                                                          const bool bEnabled,
                                                          const EBertaLogVerbosity Verbosity,
                                                          const EBertaLogOutput Output,
                                                          const float Duration)
{
	if (!bEnabled)
	{
		return;
	}

	// Check the global master switch from Project Settings.
	// UBertaDevKitSettings::Get() calls GetDefault<> which is always valid — no null check needed.
	if (!UBertaDevKitSettings::Get()->bDebugLogEnabled)
	{
		return;
	}

	// Delegate to PrintLogToNamedCategory so all named category routing logic
	// stays centralized in one place.
	// bEnabled is already confirmed true — pass true directly to avoid
	// a redundant check in the delegated call.
	PrintLogToNamedCategory(CategoryName,
	                        BuildContextMessage(WorldContext,
	                                            Message),
	                        true,
	                        Verbosity,
	                        Output,
	                        Duration,
	                        /*Key=*/
	                        -1);
}

FString UBertaDebugUtils::BuildContextMessage(const UObject* WorldContext,
                                              const FString& Message)
{
	// If WorldContext is invalid, fall back to a visible "NULL" label rather than
	// silently dropping caller information — a visible NULL is easier to debug.
	const FString ContextName = IsValid(WorldContext)
		                            ? WorldContext->GetName()
		                            : TEXT("NULL");

	return FString::Printf(TEXT("[%s] %s"),
	                       *ContextName,
	                       *Message);
}

FColor UBertaDebugUtils::GetColorForVerbosity(const EBertaLogVerbosity Verbosity)
{
	switch (Verbosity)
	{
	case EBertaLogVerbosity::Warning: return FColor::Yellow;
	case EBertaLogVerbosity::Error: return FColor::Red;
	default: return FColor::Cyan;
	}
}

void UBertaDebugUtils::PrintToScreen(const FString& Message,
                                     const EBertaLogVerbosity Verbosity,
                                     const float Duration,
                                     const int32 Key)
{
	// GEngine can be null during early engine init or in server builds
	// that have no rendering context — always guard before calling.
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(Key,
		                                 Duration,
		                                 GetColorForVerbosity(Verbosity),
		                                 Message);
	}
}

void UBertaDebugUtils::LogToOutput(const FName& CategoryName,
                                   const FString& Message,
                                   const EBertaLogVerbosity Verbosity)
{
	switch (Verbosity)
	{
	case EBertaLogVerbosity::Log:
		FMsg::Logf(__FILE__,
		           __LINE__,
		           CategoryName,
		           ELogVerbosity::Log,
		           TEXT("%s"),
		           *Message);
		break;

	case EBertaLogVerbosity::Warning:
		FMsg::Logf(__FILE__,
		           __LINE__,
		           CategoryName,
		           ELogVerbosity::Warning,
		           TEXT("%s"),
		           *Message);
		break;

	case EBertaLogVerbosity::Error:
		FMsg::Logf(__FILE__,
		           __LINE__,
		           CategoryName,
		           ELogVerbosity::Error,
		           TEXT("%s"),
		           *Message);
		break;

	default:
		// A new EBertaLogVerbosity value was added without updating this switch.
		// Fail loudly in development so the gap is caught immediately.
		ensureMsgf(false,
		           TEXT("LogToOutput: unhandled EBertaLogVerbosity value %d — add a case for it."),
		           static_cast<int32>(Verbosity));
		break;
	}
}
