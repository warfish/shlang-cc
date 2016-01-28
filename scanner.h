/*
 * Scanner interface
 */

#pragma once

#include "strings.h"
#include "buffer.h"

typedef enum
{
    kTokenKeyword,
    kTokenIdentifier,
    kTokenIntConstant,
    kTokenStrConstant,

    kTokenTotal // Always last
} token_type_t;

typedef struct token
{
    token_type_t type;
    string_t value;
} token_t;

typedef enum
{
    kIntegerTypeInt,
    kIntegerTypeLong,
    kIntegerTypeLongLong,

    kIntegerTypeUnsigned,
    kIntegerTypeUnsignedLong,
    kIntegerTypeUnsignedLongLong,

    kIntegerDefaultType = kIntegerTypeInt
} integer_literal_type_t;

/**
 * Init scanner.
 * Only needs to be called once.
 */
int init_scanner(void);

/**
 * Advance input buffer and parse next incoming token
 */
int parse_next_token(input_buffer_t* in, token_t* out_token);
