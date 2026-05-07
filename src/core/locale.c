#include "locale.h"

#include "core/log.h"
#include "core/string.h"
#include "core/lang.h"
#include "translation/language_registry.h"

static struct {
    language_type last_determined_language;
} data;

static language_type determine_language(void)
{
    // Dirty way to check the language, but there's not really another way:
    // Check if the string for "New game" is in one of the supported languages
    const uint8_t *new_game_string = lang_get_string(1, 1);
    int count;
    const language_info *all = language_registry_all(&count);
    for (int i = 0; i < count; i++) {
        if (string_equals(all[i].new_game_bytes, new_game_string)) {
            return all[i].type;
        }
    }
    return LANGUAGE_UNKNOWN;
}

static void log_language(void)
{
    const language_info *info = language_registry_get(data.last_determined_language);
    const char *desc = info ? info->name : "Unknown";
    log_info("Detected language:", desc, 0);
}

language_type locale_determine_language(void)
{
    data.last_determined_language = determine_language();
    log_language();
    return data.last_determined_language;
}

language_type locale_last_determined_language(void)
{
    if (!data.last_determined_language) {
        return LANGUAGE_UNKNOWN;
    } else {
        return data.last_determined_language;
    }
}


int locale_year_before_ad(void)
{
    // In all languages it's "200 AD" except for English
    return data.last_determined_language != LANGUAGE_ENGLISH;
}

int locale_translate_money_dn(void)
{
    // In Korean, 'Dn' translate to 'Funds', which makes no sense for
    // constructions costs and other places where Dn is used for money.
    return data.last_determined_language != LANGUAGE_KOREAN;
}

int locale_paragraph_indent(void)
{
    return data.last_determined_language == LANGUAGE_JAPANESE ? 17 : 50;
}

int locale_translate_rank_autosaves(void)
{
    switch (data.last_determined_language) {
        case LANGUAGE_ENGLISH:
        case LANGUAGE_FRENCH:
        case LANGUAGE_GERMAN:
        case LANGUAGE_ITALIAN:
        case LANGUAGE_POLISH:
        case LANGUAGE_PORTUGUESE:
        case LANGUAGE_SPANISH:
        case LANGUAGE_SWEDISH:
        case LANGUAGE_RUSSIAN:
        case LANGUAGE_CZECH:
        case LANGUAGE_UKRAINIAN:
            return 1;

        case LANGUAGE_JAPANESE:
        case LANGUAGE_KOREAN:
        case LANGUAGE_TRADITIONAL_CHINESE: // original adds 01_ prefixes
        case LANGUAGE_SIMPLIFIED_CHINESE:
        default:
            return 0;
    }
}


