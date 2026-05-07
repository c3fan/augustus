#include "translation.h"

#include "core/encoding.h"
#include "core/log.h"
#include "core/string.h"
#include "translation/language_registry.h"

#include <string.h>

#define BUFFER_SIZE 100000

static struct {
    uint8_t *strings[TRANSLATION_MAX_KEY];
    uint8_t buffer[BUFFER_SIZE];
    int buf_index;
} data;

static void set_strings(const translation_string *strings, int num_strings, int is_default)
{
    for (int i = 0; i < num_strings; i++) {
        const translation_string *string = &strings[i];
        if (data.strings[string->key]) {
            continue;
        }
        if (is_default) {
            log_info("Translation key not found:", string->string, string->key);
        }
        int length_left = BUFFER_SIZE - data.buf_index;
        encoding_from_utf8(string->string, &data.buffer[data.buf_index], length_left);
        data.strings[string->key] = &data.buffer[data.buf_index];
        data.buf_index += 1 + string_length(&data.buffer[data.buf_index]);
    }
}

void translation_load(language_type language)
{
    const translation_string *strings = NULL;
    int num_strings = 0;
    const translation_string *default_strings = NULL;
    int num_default_strings = 0;

    const language_info *default_info = language_registry_get_default();
    if (default_info) {
        default_info->load(&default_strings, &num_default_strings);
    }

    const language_info *info = language_registry_get(language);
    if (info) {
        info->load(&strings, &num_strings);
    } else {
        log_error("Invalid translation selected", 0, 0);
    }

    memset(data.strings, 0, sizeof(data.strings));
    data.buf_index = 0;
    set_strings(strings, num_strings, 0);
    set_strings(default_strings, num_default_strings, 1);
}

const uint8_t *translation_for(translation_key key)
{
    return data.strings[key];
}
