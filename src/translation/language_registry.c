#include "translation/language_registry.h"

#include <stddef.h>  /* NULL */

/* Forward declarations for per-language translation loaders.
 * These are intentionally NOT declared in translation.h — they are
 * implementation details consumed exclusively by this registry. */
void translation_czech(const translation_string **strings, int *num_strings);
void translation_english(const translation_string **strings, int *num_strings);
void translation_french(const translation_string **strings, int *num_strings);
void translation_german(const translation_string **strings, int *num_strings);
void translation_greek(const translation_string **strings, int *num_strings);
void translation_italian(const translation_string **strings, int *num_strings);
void translation_japanese(const translation_string **strings, int *num_strings);
void translation_korean(const translation_string **strings, int *num_strings);
void translation_polish(const translation_string **strings, int *num_strings);
void translation_portuguese(const translation_string **strings, int *num_strings);
void translation_russian(const translation_string **strings, int *num_strings);
void translation_simplified_chinese(const translation_string **strings, int *num_strings);
void translation_spanish(const translation_string **strings, int *num_strings);
void translation_swedish(const translation_string **strings, int *num_strings);
void translation_traditional_chinese(const translation_string **strings, int *num_strings);
void translation_ukrainian(const translation_string **strings, int *num_strings);

/* "New Game" strings in each language's .eng file encoding.
 * Previously defined in core/locale.c; consolidated here so that a single
 * table row captures everything required to support a language. */
static const uint8_t NEW_GAME_ENGLISH[]            = { 0x4e, 0x65, 0x77, 0x20, 0x47, 0x61, 0x6d, 0x65, 0 };
static const uint8_t NEW_GAME_FRENCH[]             = { 0x4e, 0x6f, 0x75, 0x76, 0x65, 0x6c, 0x6c, 0x65, 0x20, 0x70, 0x61, 0x72, 0x74, 0x69, 0x65, 0 };
static const uint8_t NEW_GAME_GERMAN[]             = { 0x4e, 0x65, 0x75, 0x65, 0x73, 0x20, 0x53, 0x70, 0x69, 0x65, 0x6c, 0 };
static const uint8_t NEW_GAME_GREEK[]              = { 0xcd, 0xdd, 0xef, 0x20, 0xd0, 0xe1, 0xe9, 0xf7, 0xed, 0xdf, 0xe4, 0xe9, 0 };
static const uint8_t NEW_GAME_ITALIAN[]            = { 0x4e, 0x75, 0x6f, 0x76, 0x61, 0x20, 0x70, 0x61, 0x72, 0x74, 0x69, 0x74, 0x61, 0 };
static const uint8_t NEW_GAME_SPANISH[]            = { 0x4e, 0x75, 0x65, 0x76, 0x61, 0x20, 0x70, 0x61, 0x72, 0x74, 0x69, 0x64, 0x61, 0 };
static const uint8_t NEW_GAME_PORTUGUESE[]         = { 0x4e, 0x6f, 0x76, 0x6f, 0x20, 0x6a, 0x6f, 0x67, 0x6f, 0 };
static const uint8_t NEW_GAME_POLISH[]             = { 0x4e, 0x6f, 0x77, 0x61, 0x20, 0x67, 0x72, 0x61, 0 };
static const uint8_t NEW_GAME_RUSSIAN[]            = { 0xcd, 0xee, 0xe2, 0xe0, 0xff, 0x20, 0xe8, 0xe3, 0xf0, 0xe0, 0 };
static const uint8_t NEW_GAME_SWEDISH[]            = { 0x4e, 0x79, 0x74, 0x74, 0x20, 0x73, 0x70, 0x65, 0x6c, 0 };
static const uint8_t NEW_GAME_TRADITIONAL_CHINESE[] = { 0x83, 0x80, 0x20, 0x84, 0x80, 0x20, 0x85, 0x80, 0 };
static const uint8_t NEW_GAME_SIMPLIFIED_CHINESE[] = { 0x82, 0x80, 0x20, 0x83, 0x80, 0x20, 0x84, 0x80, 0 };
static const uint8_t NEW_GAME_KOREAN[]             = { 0xbb, 0xf5, 0x20, 0xb0, 0xd4, 0xc0, 0xd3, 0 };
static const uint8_t NEW_GAME_JAPANESE[]           = { 0x83, 0x6a, 0x83, 0x85, 0x81, 0x5b, 0x83, 0x51, 0x81, 0x5b, 0x83, 0x80, 0 };
static const uint8_t NEW_GAME_CZECH[]              = { 0x4e, 0x6f, 0x76, 0xe1, 0x20, 0x68, 0x72, 0x61, 0 };
static const uint8_t NEW_GAME_UKRAINIAN[]          = { 0xcd, 0xee, 0xe2, 0xe0, 0x20, 0xe3, 0xf0, 0xe0, 0 };

/* Content-patch hooks for specific languages.
 * These replace known erroneous strings in the original game data files
 * with correct translations from the Augustus translation system. */

static const uint8_t *german_message_text_override(int32_t offset)
{
    // The German .mm file has a corrupt entry at offset 289 ("city retaken").
    if (offset == 289) {
        return translation_for(TR_FIX_GERMAN_CITY_RETAKEN);
    }
    return NULL;
}

static const uint8_t *korean_string_override(int group, int index)
{
    // The Korean .eng file uses the word for "Funds" as the Doctors Clinic name.
    if (group == 28 && index == 46) {
        return translation_for(TR_FIX_KOREAN_BUILDING_DOCTORS_CLINIC);
    }
    return NULL;
}

/* Central language registry.
 * Columns: {type, name, new_game_bytes, encoding, loader,
 *           year_before_ad, translate_money_dn, paragraph_indent, translate_rank_autosaves,
 *           message_text_override, string_override}
 * Ordered to match the LANGUAGE_* enum in core/locale.h for clarity. */
static const language_info REGISTRY[] = {
  /*                                                                                          yr  dn  indent  autosaves  msg_override             str_override */
    { LANGUAGE_ENGLISH,             "English",            NEW_GAME_ENGLISH,            ENCODING_WESTERN_EUROPE,     translation_english,             0,  1,  50,  1,  NULL,                         NULL                  },
    { LANGUAGE_FRENCH,              "French",             NEW_GAME_FRENCH,             ENCODING_WESTERN_EUROPE,     translation_french,              1,  1,  50,  1,  NULL,                         NULL                  },
    { LANGUAGE_GERMAN,              "German",             NEW_GAME_GERMAN,             ENCODING_WESTERN_EUROPE,     translation_german,              1,  1,  50,  1,  german_message_text_override,  NULL                  },
    { LANGUAGE_ITALIAN,             "Italian",            NEW_GAME_ITALIAN,            ENCODING_WESTERN_EUROPE,     translation_italian,             1,  1,  50,  1,  NULL,                         NULL                  },
    { LANGUAGE_SPANISH,             "Spanish",            NEW_GAME_SPANISH,            ENCODING_WESTERN_EUROPE,     translation_spanish,             1,  1,  50,  1,  NULL,                         NULL                  },
    { LANGUAGE_JAPANESE,            "Japanese",           NEW_GAME_JAPANESE,           ENCODING_JAPANESE,           translation_japanese,            1,  1,  17,  0,  NULL,                         NULL                  },
    { LANGUAGE_KOREAN,              "Korean",             NEW_GAME_KOREAN,             ENCODING_KOREAN,             translation_korean,              1,  0,  50,  0,  NULL,                         korean_string_override },
    { LANGUAGE_POLISH,              "Polish",             NEW_GAME_POLISH,             ENCODING_EASTERN_EUROPE,     translation_polish,              1,  1,  50,  1,  NULL,                         NULL                  },
    { LANGUAGE_PORTUGUESE,          "Portuguese",         NEW_GAME_PORTUGUESE,         ENCODING_WESTERN_EUROPE,     translation_portuguese,          1,  1,  50,  1,  NULL,                         NULL                  },
    { LANGUAGE_RUSSIAN,             "Russian",            NEW_GAME_RUSSIAN,            ENCODING_CYRILLIC,           translation_russian,             1,  1,  50,  1,  NULL,                         NULL                  },
    { LANGUAGE_SWEDISH,             "Swedish",            NEW_GAME_SWEDISH,            ENCODING_WESTERN_EUROPE,     translation_swedish,             1,  1,  50,  1,  NULL,                         NULL                  },
    { LANGUAGE_SIMPLIFIED_CHINESE,  "Simplified Chinese", NEW_GAME_SIMPLIFIED_CHINESE, ENCODING_SIMPLIFIED_CHINESE, translation_simplified_chinese,  1,  1,  50,  0,  NULL,                         NULL                  },
    { LANGUAGE_TRADITIONAL_CHINESE, "Traditional Chinese", NEW_GAME_TRADITIONAL_CHINESE, ENCODING_TRADITIONAL_CHINESE, translation_traditional_chinese, 1,  1,  50,  0,  NULL,                         NULL                  },
    { LANGUAGE_CZECH,               "Czech",              NEW_GAME_CZECH,              ENCODING_CZECH,              translation_czech,               1,  1,  50,  1,  NULL,                         NULL                  },
    { LANGUAGE_GREEK,               "Greek",              NEW_GAME_GREEK,              ENCODING_GREEK,              translation_greek,               1,  1,  50,  1,  NULL,                         NULL                  },
    { LANGUAGE_UKRAINIAN,           "Ukrainian",          NEW_GAME_UKRAINIAN,          ENCODING_CYRILLIC,           translation_ukrainian,           1,  1,  50,  1,  NULL,                         NULL                  },
};

#define REGISTRY_COUNT (int)(sizeof(REGISTRY) / sizeof(language_info))

const language_info *language_registry_get(language_type language)
{
    for (int i = 0; i < REGISTRY_COUNT; i++) {
        if (REGISTRY[i].type == language) {
            return &REGISTRY[i];
        }
    }
    return NULL;
}

const language_info *language_registry_all(int *count)
{
    *count = REGISTRY_COUNT;
    return REGISTRY;
}

const language_info *language_registry_get_default(void)
{
    for (int i = 0; i < REGISTRY_COUNT; i++) {
        if (REGISTRY[i].type == LANGUAGE_ENGLISH) {
            return &REGISTRY[i];
        }
    }
    return REGISTRY_COUNT ? &REGISTRY[0] : NULL;
}
