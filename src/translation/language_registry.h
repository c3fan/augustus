#ifndef TRANSLATION_LANGUAGE_REGISTRY_H
#define TRANSLATION_LANGUAGE_REGISTRY_H

#include "core/encoding.h"
#include "core/locale.h"
#include "translation/translation.h"

#include <stdint.h>

/**
 * Describes all per-language data needed by the encoding, locale-detection,
 * translation, and locale-behaviour subsystems.  Adding a new language only
 * requires:
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

    /** Locale behaviour flags **/

    /**
     * Non-zero if the year number comes before the "AD" suffix
     * (e.g. "200 AD").  Zero if "AD" comes first (e.g. "AD 200").
     * English is the only language that uses "AD 200" order.
     */
    int year_before_ad;

    /**
     * Non-zero if the "Dn" currency abbreviation should be taken from the
     * funds-menu translation.  Zero (Korean) uses the fixed string "Dn"
     * because the translated word means "Funds", not a currency symbol.
     */
    int translate_money_dn;

    /**
     * Width in pixels of a paragraph indent in rich text.
     * Most languages use 50; Japanese uses 17.
     */
    int paragraph_indent;

    /**
     * Non-zero if rank-based autosave filenames should be translated
     * (e.g. "Citizen.sav" -> localised).  Zero for CJK languages where
     * the original game used numeric 01_ prefixes instead.
     */
    int translate_rank_autosaves;

    /** Content-patch hooks (may be NULL) **/

    /**
     * Optional hook to override a raw text entry from the .mm message file.
     * @param offset  The raw byte offset that would normally be looked up.
     * @return A replacement string, or NULL to use the original data.
     *
     * Use this to work around known errors in the original game's message data
     * for a specific language (e.g. the German "city retaken" message).
     */
    const uint8_t *(*message_text_override)(int32_t offset);

    /**
     * Optional hook to override a specific (group, index) string from the
     * .eng text file.
     * @param group  String group number.
     * @param index  String index within the group.
     * @return A replacement string, or NULL to use the original data.
     *
     * Use this to work around known errors in the original game's text file
     * for a specific language (e.g. the Korean "Doctors Clinic" name).
     */
    const uint8_t *(*string_override)(int group, int index);
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
