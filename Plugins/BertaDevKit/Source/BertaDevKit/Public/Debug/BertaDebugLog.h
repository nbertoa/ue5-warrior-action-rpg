#pragma once

#include "Logging/LogMacros.h"

/**
 * Log category for BertaDebugUtils and BertaDebugDraw.
 * Use this category to identify debug messages emitted via BertaDebugUtils
 * across all projects that use the BertaDevKit plugin.
 *
 * Usage:
 *      UE_LOG(LogBertaDebug, Log,     TEXT("Message"));
 *      UE_LOG(LogBertaDebug, Warning, TEXT("Message"));
 *      UE_LOG(LogBertaDebug, Error,   TEXT("Message"));
 */
DECLARE_LOG_CATEGORY_EXTERN(LogBertaDebug,
                            Log,
                            All);
