#include "Debug/BertaScreenStats.h"

#include "Log/BertaDevKitLog.h"
#include "Settings/BertaDevKitSettings.h"
#include "Engine/Engine.h"
#include "Misc/CoreDelegates.h"

// --------------------------------------------------------------------
// Static Storage
// --------------------------------------------------------------------

TMap<FName, FBertaStatEntry>& UBertaScreenStats::GetEntries()
{
	// Static local — constructed on first call, destroyed on program exit.
	// Avoids the static initialization order fiasco that affects static members
	// defined at namespace/class scope across translation units.
	static TMap<FName, FBertaStatEntry> Entries;
	return Entries;
}

// --------------------------------------------------------------------
// Registration
// --------------------------------------------------------------------

void UBertaScreenStats::EnsureRegistered()
{
	// Static local flag — initialized to false on first call, then never again.
	// This is safer than a class-level static member: initialization is guaranteed
	// to occur on first call, avoiding the static initialization order fiasco.
	// No locking needed — the editor and gameplay code that calls Set* is single-threaded.
	static bool bIsRegistered = false;

	if (bIsRegistered)
	{
		return;
	}

	// Bind to OnBeginFrame so RenderStats fires once per frame automatically.
	// No need to unregister manually — this delegate fires for the lifetime
	// of the Engine and the static function pointer is always valid.
	FCoreDelegates::OnBeginFrame.AddStatic(&UBertaScreenStats::RenderStats);

	bIsRegistered = true;

	UE_LOG(LogBertaDevKit,
	       Log,
	       TEXT("[BertaScreenStats] Registered on-screen stats renderer."));
}

// --------------------------------------------------------------------
// Rendering
// --------------------------------------------------------------------

void UBertaScreenStats::RenderStats()
{
	// Early out if Engine is not ready — can happen during very early startup
	// or in headless (dedicated server) builds with no rendering context.
	if (!GEngine)
	{
		return;
	}

	const UBertaDevKitSettings* Settings = UBertaDevKitSettings::Get();
	if (!Settings || !Settings->bScreenStatsEnabled)
	{
		return;
	}

	const TMap<FName, FBertaStatEntry>& Entries = GetEntries();

	if (Entries.IsEmpty())
	{
		return;
	}

	// Each entry gets a unique stable key derived from its FName hash.
	// AddOnScreenDebugMessage uses this key to overwrite the same line
	// every frame instead of appending a new one.
	for (const TPair<FName, FBertaStatEntry>& Pair : Entries)
	{
		const FName& EntryName = Pair.Key;
		const FBertaStatEntry& Entry = Pair.Value;

		// Build the display string: "Name: Value"
		const FString DisplayText = FString::Printf(TEXT("%s: %s"),
		                                            *Entry.DisplayName,
		                                            *Entry.Value);

		// GetTypeHash on FName returns a stable uint32 — cast to int32 for the
		// message key. Collisions are theoretically possible but negligible in
		// practice for a small debug panel with named entries.
		const int32 MessageKey = static_cast<int32>(GetTypeHash(EntryName));

		GEngine->AddOnScreenDebugMessage(MessageKey,
		                                 0.0f,
		                                 // Duration 0 = display for exactly one frame; re-added next frame.
		                                 Entry.Color.ToFColor(true),
		                                 DisplayText);
	}
}

// --------------------------------------------------------------------
// Internal Setter
// --------------------------------------------------------------------

void UBertaScreenStats::SetEntry(const FName Name,
                                 const FString& FormattedValue,
                                 const FLinearColor Color)
{
	// Lazy registration — only pay the delegate cost if the system is actually used.
	EnsureRegistered();

	FBertaStatEntry& Entry = GetEntries().FindOrAdd(Name);

	// DisplayName is set from the FName string on first add and stays stable —
	// avoids converting FName to FString on every frame update.
	if (Entry.DisplayName.IsEmpty())
	{
		Entry.DisplayName = Name.ToString();
	}

	Entry.Value = FormattedValue;
	Entry.Color = Color;
}

// --------------------------------------------------------------------
// Public Interface — Set*
// --------------------------------------------------------------------

void UBertaScreenStats::SetFloat(const FName Name,
                                 const float Value,
                                 const int32 DecimalPlaces,
                                 const FLinearColor Color)
{
	// FString::SanitizeFloat strips trailing zeros — not what we want here.
	// Instead, use snprintf via FCString for runtime-dynamic precision,
	// which bypasses the compile-time format string checker entirely.
	const int32 ClampedDecimals = FMath::Max(0,
	                                         DecimalPlaces);

	TCHAR Buffer[64];
	FCString::Snprintf(Buffer,
	                   UE_ARRAY_COUNT(Buffer),
	                   TEXT("%.*f"),
	                   ClampedDecimals,
	                   Value);

	SetEntry(Name,
	         FString(Buffer),
	         Color);
}

void UBertaScreenStats::SetInt(const FName Name,
                               const int32 Value,
                               const FLinearColor Color)
{
	SetEntry(Name,
	         FString::FromInt(Value),
	         Color);
}

void UBertaScreenStats::SetBool(const FName Name,
                                const bool Value,
                                const bool bAutoColor,
                                const FLinearColor Color)
{
	// Auto-color gives immediate visual feedback: green = healthy, red = problem.
	// The pattern is common enough in debug UIs that it should be the default.
	const FLinearColor ResolvedColor = bAutoColor
		                                   ? (Value
			                                      ? FLinearColor::Green
			                                      : FLinearColor::Red)
		                                   : Color;

	const FString Formatted = Value
		                          ? TEXT("true")
		                          : TEXT("false");

	SetEntry(Name,
	         Formatted,
	         ResolvedColor);
}

void UBertaScreenStats::SetString(const FName Name,
                                  const FString& Value,
                                  const FLinearColor Color)
{
	SetEntry(Name,
	         Value,
	         Color);
}

void UBertaScreenStats::SetVector(const FName Name,
                                  const FVector Value,
                                  const int32 DecimalPlaces,
                                  const FLinearColor Color)
{
	const int32 ClampedDecimals = FMath::Max(0,
	                                         DecimalPlaces);

	TCHAR BufX[64];
	TCHAR BufY[64];
	TCHAR BufZ[64];

	FCString::Snprintf(BufX,
	                   UE_ARRAY_COUNT(BufX),
	                   TEXT("%.*f"),
	                   ClampedDecimals,
	                   Value.X);
	FCString::Snprintf(BufY,
	                   UE_ARRAY_COUNT(BufY),
	                   TEXT("%.*f"),
	                   ClampedDecimals,
	                   Value.Y);
	FCString::Snprintf(BufZ,
	                   UE_ARRAY_COUNT(BufZ),
	                   TEXT("%.*f"),
	                   ClampedDecimals,
	                   Value.Z);

	const FString Formatted = FString::Printf(TEXT("X=%s Y=%s Z=%s"),
	                                          BufX,
	                                          BufY,
	                                          BufZ);

	SetEntry(Name,
	         Formatted,
	         Color);
}

// --------------------------------------------------------------------
// Panel Control
// --------------------------------------------------------------------

void UBertaScreenStats::Remove(const FName Name)
{
	GetEntries().Remove(Name);
}

void UBertaScreenStats::Clear()
{
	GetEntries().Reset();
}
