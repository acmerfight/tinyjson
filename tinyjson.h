#ifndef TINYJSON_LIBRARY_H
#define TINYJSON_LIBRARY_H

#include <stddef.h>

typedef enum {
    TINY_NULL,
    TINY_FALSE,
    TINY_TRUE,
    TINY_NUMBER,
    TINY_STRING,
    TINY_ARRAY,
    TINY_OBJECT
} tiny_type;

typedef struct {
    union {
        struct {
            char* s;
            size_t len;
        } s;
        double n;
    } u;
    tiny_type type;
} tiny_value;

enum {
    TINY_PARSE_OK = 0,
    TINY_PARSE_EXCEPT_VALUE,
    TINY_PARSE_INVALID_VALUE,
    TINY_PARSE_ROOT_NOT_SINGULAR,
    TINY_PARSE_NUMBER_TOO_BIG,
    TINY_PARSE_MISS_QUOTATION_MARK,
    TINY_PARSE_INVALID_STRING_ESCAPE,
    TINY_PARSE_INVALID_STRING_CHAR
};

#define tiny_init(v) do {(v) -> type = TINY_NULL;} while(0)

int tiny_parse(tiny_value* v, const char* json);

void tiny_free(tiny_value* v);

tiny_type tiny_get_type(const tiny_value* v);

#define tiny_set_null(v) tiny_free(v)

int tiny_get_boolean(const tiny_value* v);
void tiny_set_boolean(tiny_value* v, int b);

double tiny_get_number(const tiny_value* v);
void tiny_set_number(tiny_value* v, double n);

const char* tiny_get_string(const tiny_value* v);
size_t tiny_get_string_length(const tiny_value* v);
void tiny_set_string(tiny_value* v, const char* s, size_t len);

#endif