#ifndef TRANSLATION_LANGUAGE_REGISTRY_H
#define TRANSLATION_LANGUAGE_REGISTRY_H

#include "core/encoding.h"
#include "core/locale.h"
#include "translation/translation.h"

#include <stdint.h>

/**
 * Describes all per-language data needed by the encoding, locale-detection,
 * and translation subsystems.  Adding a new language only requires:
 *   1. A new LANGUAGE_* value in core/locale.h
 *   2. A new translation_<lang>.c file
 *   3. One new row in the REGISTRY table in language_registry.c
 */
typedef struct {
    language_type   type;
    const char     *name;           /**< Human-readable name used for logging */
    const uint8_t  *new_game_bytes; /**< "New Game" string in the .eng file encoding */
    encoding_type   encoding;       /**< Character encoding used by this language */
    void (*load)(const translation_string **strings, int *num_strings);
} language_info;

/**
 * Returns language info for a specific language, or NULL if not registered.
 */
const language_info *language_registry_get(language_type language);

/**
 * Returns a pointer to the full registry array and sets *count to its length.
 */
const language_info *language_registry_all(int *count);

#endif // TRANSLATION_LANGUAGE_REGISTRY_H
