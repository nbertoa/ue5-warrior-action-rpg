#pragma once

#include "Logging/LogMacros.h"

/**
 * Global log category for the BertaDevKit plugin.
 * Use this category for all UE_LOG calls within the plugin.
 *
 * Usage:
 *      UE_LOG(LogBertaDevKit, Log,     TEXT("Message"));
 *      UE_LOG(LogBertaDevKit, Warning, TEXT("Message"));
 *      UE_LOG(LogBertaDevKit, Error,   TEXT("Message"));
 */
DECLARE_LOG_CATEGORY_EXTERN(LogBertaDevKit,
                            Log,
                            All);
