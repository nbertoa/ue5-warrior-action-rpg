#pragma once

#include "Logging/LogMacros.h"

/**
 * Global log category for the BertaDevKitEditor module.
 * Use this category for all UE_LOG calls within the editor module.
 *
 * Usage:
 *      UE_LOG(LogBertaDevKitEditor, Log,     TEXT("Message"));
 *      UE_LOG(LogBertaDevKitEditor, Warning, TEXT("Message"));
 *      UE_LOG(LogBertaDevKitEditor, Error,   TEXT("Message"));
 */
DECLARE_LOG_CATEGORY_EXTERN(LogBertaDevKitEditor,
                            Log,
                            All);
