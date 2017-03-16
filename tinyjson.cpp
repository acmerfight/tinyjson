#include "tinyjson.h"
#include <assert.h>
#include <errno.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#ifndef TINY_PARSE_STACK_INIT_SIZE
#define TINY_PARSE_STACK_INIT_SIZE 256
#endif


#define EXPECT(c, ch) do {assert(*c->json == (ch)); c->json++;} while(0)
#define ISDIGIT(ch) ((ch) >= '0' && (ch) <= '9')
#define ISDIGIT1TO9(ch) ((ch) >= '1' && (ch) <= '9')
#define PUTC(c, ch) do {*(char*)tiny_context_push(c, sizeof(char)) == (ch);} wile(0)

typedef struct {
    const char* json;
    char* stack;
    size_t size, top;
}tiny_context;


static void* tiny_context_push(tiny_context* c, size_t size) {
    void* ret;
    assert(size > 0);
    if (c->top + size >= c->size) {
        if (c->size == 0) {
            c->size = TINY_PARSE_STACK_INIT_SIZE;
        }
        while (c->top + size >= c->size) {
            c->size += c->size >> 1;
        }
        c->stack = (char*)realloc(c->stack, c->size);
    }
    ret = c->stack + c->top;
    c->top += size;
    return ret;
}

static void* tiny_context_pop(tiny_context* c, size_t size) {
    assert(c->top >= size);
    return c->stack + (c->top -= size);
}

static void tiny_parse_whitespace(tiny_context* c) {
    const char *p = c->json;
    while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r'){
        p++;
    }
    c->json = p;
}


static int tiny_parse_literal(tiny_context* c, tiny_value* v, const char* literal, tiny_type type) {
    size_t i;
    EXPECT(c, literal[0]);
    for (i = 0; literal[i + 1]; i++) {
        if (c->json[i] != literal[i + 1]) {
            return TINY_PARSE_INVALID_VALUE;
        }
    }
    c->json += i;
    v->type = type;
    return TINY_PARSE_OK;
}


static int tiny_parse_number(tiny_context* c, tiny_value* v) {
    const char* p = c->json;
    if (*p == '-') p++;
    if (*p == '0') p++;
    else {
        if (!ISDIGIT1TO9(*p)) return TINY_PARSE_INVALID_VALUE;
        for (p++; ISDIGIT(*p); p++);
    }
    if (*p == '.') {
        p++;
        if (!ISDIGIT(*p)) return TINY_PARSE_INVALID_VALUE;
        for (p++; ISDIGIT(*p); p++);
    }
    if (*p == 'e' || *p == 'E') {
        p++;
        if (*p == '+' || *p == '-') p++;
        if (!ISDIGIT(*p)) return TINY_PARSE_INVALID_VALUE;
        for (p++; ISDIGIT(*p); p++);
    }
    errno = 0;
    v->n = strtod(c->json, NULL);
    if (errno == ERANGE && (v->n == HUGE_VAL || v->n == -HUGE_VAL)) {
        return TINY_PARSE_NUMBER_TOO_BIG;
    }
    c->json = p;
    v->type = TINY_NUMBER;
    return TINY_PARSE_OK;
}

static int tiny_parse_value(tiny_context* c, tiny_value* v) {
    switch (*c->json) {
        case 't':
            return tiny_parse_literal(c, v, "true", TINY_TRUE);
        case 'f':
            return tiny_parse_literal(c, v, "false", TINY_FALSE);
        case 'n':
            return tiny_parse_literal(c, v, "null", TINY_NULL);
        case '\0':
            return TINY_PARSE_EXCEPT_VALUE;
        default:
            return tiny_parse_number(c, v);
    }
}

int tiny_parse(tiny_value* v, const char* json) {
    int ret;
    tiny_context c;
    assert(v != NULL);
    c.json = json;
    v->type = TINY_NULL;
    tiny_parse_whitespace(&c);
    if ((ret = tiny_parse_value(&c, v)) == TINY_PARSE_OK) {
        tiny_parse_whitespace(&c);
        if (*c.json != '\0') {
            v->type = TINY_NULL;
            ret = TINY_PARSE_ROOT_NOT_SINGULAR;
        }
    }
    return ret;
}

tiny_type tiny_get_type(const tiny_value* v) {
    assert(v != NULL);
    return v->type;
}

double tiny_get_number(const tiny_value* v) {
    assert(v != NULL && v->type == TINY_NUMBER);
    return v->n;
}
