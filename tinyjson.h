#ifndef TINYJSON_LIBRARY_H
#define TINYJSON_LIBRARY_H

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
    tiny_type type;
} tiny_value;

enum {
    TINY_PARSE_OK = 0,
    TINY_PARSE_EXCEPT_VALUE,
    TINY_PARSE_INVALID_VALUE,
    TINY_PARSE_ROOT_NOT_SINGULAR
};

int tiny_parse(tiny_value* v, const char* json);
tiny_type tiny_get_type(const tiny_value* v);

#endif