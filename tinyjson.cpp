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
#define PUTC(c, ch) do {*(char*)tiny_context_push(c, sizeof(char)) = (ch); } while(0)

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
    v->u.n = strtod(c->json, NULL);
    if (errno == ERANGE && (v->u.n == HUGE_VAL || v->u.n == -HUGE_VAL)) {
        return TINY_PARSE_NUMBER_TOO_BIG;
    }
    c->json = p;
    v->type = TINY_NUMBER;
    return TINY_PARSE_OK;
}


static int tiny_parse_string(tiny_context* c, tiny_value* v) {
    size_t head = c->top, len;
    const char *p;
    EXPECT(c, '\"');
    p = c->json;
    for (;;) {
        char ch = *p++;
        switch (ch) {
            case '\"':
                len = c->top - head;
                tiny_set_string(v, (const char *)tiny_context_pop(c, len), len);
                c->json = p;
                return TINY_PARSE_OK;
            case '\0':
                c->top = head;
                return TINY_PARSE_MISS_QUOTATION_MARK;
            default:
                PUTC(c, ch);
        }
    }
}

static int tiny_parse_value(tiny_context* c, tiny_value* v) {
    switch (*c->json) {
        case 't':
            return tiny_parse_literal(c, v, "true", TINY_TRUE);
        case 'f':
            return tiny_parse_literal(c, v, "false", TINY_FALSE);
        case 'n':
            return tiny_parse_literal(c, v, "null", TINY_NULL);
        default:
            return tiny_parse_number(c, v);
        case '"':
            return tiny_parse_string(c, v);
        case '\0':
            return TINY_PARSE_EXCEPT_VALUE;
    }
}


int tiny_parse(tiny_value* v, const char* json) {
    int ret;
    tiny_context c;
    assert(v != NULL);
    c.json = json;
    c.stack = NULL;
    c.size = c.top = 0;
    tiny_init(v);
    tiny_parse_whitespace(&c);
    if ((ret = tiny_parse_value(&c, v)) == TINY_PARSE_OK) {
        tiny_parse_whitespace(&c);
        if (*c.json != '\0') {
            v->type = TINY_NULL;
            ret = TINY_PARSE_ROOT_NOT_SINGULAR;
        }
    }
    assert(c.top == 0);
    free(c.stack);
    return ret;
}

tiny_type tiny_get_type(const tiny_value* v) {
    assert(v != NULL);
    return v->type;
}

double tiny_get_number(const tiny_value* v) {
    assert(v != NULL && v->type == TINY_NUMBER);
    return v->u.n;
}

void tiny_set_number(const tiny_value* v) {
}


void tiny_free(tiny_value *v) {
    assert(v != NULL);
    if (v->type == TINY_STRING) {
        free(v->u.s.s);
    }
    v->type = TINY_NULL;
}

int tiny_get_boolean(const tiny_value* v) {
    return 0;
}

void tiny_set_boolean(const tiny_value* v, int b) {
}

const char* tiny_get_string(const tiny_value* v) {
    assert(v != NULL && v->type == TINY_STRING);
    return v->u.s.s;
}

size_t tiny_get_string_length(const tiny_value* v) {
    assert(v != NULL && v->type == TINY_STRING);
    return v->u.s.len;
}


void tiny_set_string(tiny_value* v, const char* s, size_t len) {
    assert(v != NULL && (s != NULL || len == 0));
    tiny_free(v);
    v->u.s.s = (char *)malloc(len + 1);
    memcpy(v->u.s.s, s, len);
    v->u.s.s[len] = '\0';
    v->u.s.len = len;
    v->type = TINY_STRING;
}
