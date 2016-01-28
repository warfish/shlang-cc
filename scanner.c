#include "scanner.h"
#include "string.h"
#include "support.h"
#include "test.h"

#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <errno.h>
#include <assert.h>

typedef struct matcher
{
    token_type_t type;  // Token type this matcher matches for
    bool(*match)(struct matcher* matcher, input_buffer_t* in, token_t* token);
} matcher_t;

static bool match_word(const char* word, size_t offset, input_buffer_t* in, token_t* token)
{
    assert(word);
    assert(in);
    assert(token);

    // TODO: read on grep optimizations
    const char* str = word + offset;
    do {
        char c = buffer_getchar(in);
        if (*str == '\0' && (isspace(c) || buffer_iseof(in))) {
            token->value = string(word);
            return true;
        } else if (*str != c) {
            return false;
        }

        ++str;
    } while (!buffer_iseof(in));

    return false;
}

bool match_keyword(input_buffer_t* in, token_t* token)
{
    assert(in);
    assert(token);

    token->type = kTokenKeyword;

    char c;
    switch(c = buffer_getchar(in)) {

    /* auto */
    case 'a':   return match_word("auto", 1, in, token);

    /* break */
    case 'b':   return match_word("break", 1, in, token);

    /* const | char | continue | case */
    case 'c':
        switch (c = buffer_getchar(in)) {
        case 'a':   return match_word("case", 2, in, token);
        case 'h':   return match_word("char", 2, in, token);
        case 'o':
            switch (c = buffer_getchar(in)) {
            case 'n':
                switch (c = buffer_getchar(in)) {
                case 's':   return match_word("const", 4, in, token);
                case 't':   return match_word("continue", 4, in, token);
                default:    return false;
                };
            default:    return false;
            };
        default: return false;
        };

    /* double | do | default */
    case 'd':
        switch (c = buffer_getchar(in)) {
        case 'e':   return match_word("default", 2, in, token);
        case 'o':
            switch (c = buffer_getchar(in)) {
            case 'u':   return match_word("double", 3, in, token);
            default:
                if (isspace(c) || buffer_iseof(in)) {
                    token->value = string("do");
                    return true;
                } else {
                    return false;
                }
            };
        default:    return false;
        };

    /* extern | else | enum */
    case 'e':
        switch (c = buffer_getchar(in)) {
        case 'x':   return match_word("extern", 2, in, token);
        case 'l':   return match_word("else", 2, in, token);
        case 'n':   return match_word("enum", 2, in, token);
        default:    return false;
        };

    /* float | for */
    case 'f':
        switch (c = buffer_getchar(in)) {
        case 'l':   return match_word("float", 2, in, token);
        case 'o':   return match_word("for", 2, in, token);
        default:    return false;
        };

    /* goto */
    case 'g':   return match_word("goto", 1, in, token);

    /* int | if | inline */
    case 'i':
        switch (c = buffer_getchar(in)) {
        case 'f':   return match_word("if", 2, in, token);
        case 'n':
            switch (c = buffer_getchar(in)) {
            case 't':   return match_word("int", 3, in, token);
            case 'l':   return match_word("inline", 3, in, token);
            default:    return false;
            };
        default:    return false;
        };

    /* long */
    case 'l':   return match_word("long", 1, in, token);

    /* return | register | restrict */
    case 'r':
        switch (c = buffer_getchar(in)) {
        case 'e':
            switch (c = buffer_getchar(in)) {
            case 'g':   return match_word("register", 3, in, token);
            case 's':   return match_word("restrict", 3, in, token);
            case 't':   return match_word("return", 3, in, token);
            default:    return false;
            };
        default:    return false;
        };

    /* static | short | signed | sizeof | struct | switch */
    case 's':
        switch(c = buffer_getchar(in)) {
        case 'h':   return match_word("short", 2, in, token);
        case 't':
            switch(c = buffer_getchar(in)) {
            case 'a':   return match_word("static", 3, in, token);
            case 'r':   return match_word("struct", 3, in, token);
            default:    return false;
            };
        case 'i':
            switch (c = buffer_getchar(in)) {
            case 'g':   return match_word("signed", 3, in, token);
            case 'z':   return match_word("sizeof", 3, in, token);
            default:    return false;
            };
        case 'w':   return match_word("switch", 2, in, token);
        default:    return false;
        };

    /* typedef */
    case 't':   return match_word("typedef", 1, in, token);

    /* unsigned | union */
    case 'u':
        switch (c = buffer_getchar(in)) {
        case 'n':
            switch (c = buffer_getchar(in)) {
            case 's':   return match_word("unsigned", 3, in, token);
            case 'i':   return match_word("union", 3, in, token);
            default:    return false;
            };
        default:    return false;
        };

    /* volatile | void */
    case 'v':
        switch (c = buffer_getchar(in)) {
        case 'o':
            switch (c = buffer_getchar(in)) {
            case 'i':   return match_word("void", 3, in, token);
            case 'l':   return match_word("volatile", 3, in, token);
            default:    return false;
            };
        default:    return false;
        };

    /* while */
    case 'w':   return match_word("while", 1, in, token);

    /* underscore keywords deserve a separate section */
    case '_':
        switch (c = buffer_getchar(in)) {

        /* _Alignas, _Alignof, _Atomic */
        case 'A':
            switch (c = buffer_getchar(in)) {
            case 't':   return match_word("_Atomic", 3, in, token);
            case 'l':
                switch (c = buffer_getchar(in)) {
                case 'i':
                    switch (c = buffer_getchar(in)) {
                    case 'g':
                        switch (c = buffer_getchar(in)) {
                        case 'n':
                            switch (c = buffer_getchar(in)) {
                            case 'a':   return match_word("_Alignas", 7, in, token);
                            case 'o':   return match_word("_Alignof", 7, in, token);
                            default:    return false;
                            };
                        default:    return false;
                        };
                    default:    return false;
                    };
                default:    return false;
                };
            default:    return false;
            };

        /* _Bool */
        case 'B':   return match_word("_Bool", 2, in, token);

        /* _Complex */
        case 'C':   return match_word("_Complex", 2, in, token);

        /* _Generic */
        case 'G':   return match_word("_Generic", 2, in, token);

        /* _Imaginary */
        case 'I':   return match_word("_Imaginary", 2, in, token);

        /* _Noreturn */
        case 'N':   return match_word("_Noreturn", 2, in, token);

        /* _Static_assert */
        case 'S':   return match_word("_Static_assert", 2, in, token);

        /* _Thread_local */
        case 'T':   return match_word("_Thread_local", 2, in, token);
        };

    default:    return false;
    };
}

#if defined(TEST)
static void test_keyword_matcher(void)
{
    strings_init();

    token_t token;
    input_buffer_t* ib;

    const char* valid[] = {
        "auto", "break", "case", "char", "const", "continue", "default", "do", "double", "else", "enum", "extern",
        "float", "for", "goto", "if", "inline", "int", "long", "register", "restrict", "return", "short",
        "signed", "sizeof", "static", "struct", "switch", "typedef", "union", "unsigned", "void", "volatile", "while",
        "_Alignas", "_Alignof", "_Atomic", "_Bool", "_Complex", "_Generic", "_Imaginary", "_Noreturn", "_Static_assert", "_Thread_local"
    };

    const char* invalid[] = {
        "class", "namespace", "template", "typename", "virtual", "final", "throw", "catch", "try",
        "bool", "true", "false", "offsetof", "alignof", "containerof", "", " "
    };

    for (size_t i = 0; i < countof(valid); ++i) {
        const char* str = valid[i];

        ib = buffer_mem((void*)str, strlen(str));
        CU_ASSERT(ib != NULL);
        CU_ASSERT_TRUE(match_keyword(ib, &token));
        CU_ASSERT_TRUE(token.type == kTokenKeyword);
        if (0 != strcmp(_S(token.value), str)) {
            printf("%s\n", str);
            CU_ASSERT_TRUE(0);
        }

        buffer_close(ib);
    }

    for (size_t i = 0; i < countof(invalid); ++i) {
        const char* str = invalid[i];
        ib = buffer_mem((void*)str, strlen(str));
        CU_ASSERT(ib != NULL);
        if(match_keyword(ib, &token)) {
            printf("%s\n", str);
            CU_ASSERT_TRUE(0);
        }
        buffer_close(ib);
    }
}
TEST_ADD(test_keyword_matcher);
#endif

#define SHL_IDENTIFIER_LIMIT 63

bool match_identifier(input_buffer_t* in, token_t* token)
{
    assert(in);
    assert(token);

    token->type = kTokenIdentifier;

    char buf[SHL_IDENTIFIER_LIMIT + 1] = {0};
    char* p = buf;

    // Identifier is a word that started with an ASCII alpha or underscore and followed any number by ASCII alpha, underscore or number symbols
    *p = buffer_getchar(in);
    if (!isalpha(*p) && (*p != '_')) {
        return false;
    }
    ++p;

    for (int i = 1; i < SHL_IDENTIFIER_LIMIT; ++i) {
        char c = buffer_getchar(in);
        if (isspace(c) || buffer_iseof(in)) {
            token->value = string(buf);
            return true;
        } else if (!isalnum(c) && (c != '_')) {
            return false;
        }

        buf[i] = c;
    }

    // Identifier too big
    return false;
}

#if defined(TEST)
static void test_identifier_matcher(void)
{
    strings_init();

    token_t token;
    input_buffer_t* ib;

    const char* valid[] = {
        "_good",
        "good",
        "_123good",
        "_123",
        "_",
    };

    const char* invalid[] = {
        "0xdeadfood",
        "",
        "\nbad",
        "\tbad",
        " bad",
        "-bad",
        "bad-bad",
    };

    for (size_t i = 0; i < countof(valid); ++i) {
        const char* str = valid[i];
        ib = buffer_mem((void*)str, strlen(str));
        CU_ASSERT(ib != NULL);
        CU_ASSERT_TRUE(match_identifier(ib, &token));
        CU_ASSERT_TRUE(token.type == kTokenIdentifier);
        CU_ASSERT_TRUE(0 == strcmp(_S(token.value), str));
        buffer_close(ib);
    }

    for (size_t i = 0; i < countof(invalid); ++i) {
        const char* str = invalid[i];
        ib = buffer_mem((void*)str, strlen(str));
        CU_ASSERT(ib != NULL);
        CU_ASSERT_FALSE(match_identifier(ib, &token));
        buffer_close(ib);
    }
}
TEST_ADD(test_identifier_matcher);
#endif

static bool match_integer_constant(input_buffer_t* in, token_t* token)
{
    assert(in);
    assert(token);

    token->type = kTokenIntConstant;

    // Integer constant is one of those:
    // - 0x|0X followed by any alnum (hex)
    // - 0 follwed by any num (oct)
    // - any num (dec)
    //
    // All of those can have suffixes: u, ul, l, ll, ull in any case combination

    char buf[SHL_IDENTIFIER_LIMIT + 1] = {0};
    size_t i = 0;
    bool hex = false;
    bool oct = false;
    integer_literal_type_t type = kIntegerDefaultType;

    char c = buffer_getchar(in);
    if (isspace(c) || buffer_iseof(in) || !isdigit(c)) {
        return false;
    }

    buf[i++] = c;

    if (c == '0') {
        /* Hex or oct number or just 0 */
        c = buffer_getchar(in);
        if (isspace(c) || buffer_iseof(in)) {
            token->value = string(buf);
            return true;
        } else if (c == 'x' || c == 'X') {
            hex = true;
            buf[i++] = c;
        } else if (c >= '1' && c <= '8') {
            oct = true;
            buf[i++] = c;
        } else {
            return false;
        }
    }

    bool dec = !hex && !oct;

    /* Parse remaining (x)digits */
    for (; i < SHL_IDENTIFIER_LIMIT; ++i) {
        c = buffer_getchar(in);
        if (isspace(c) || buffer_iseof(in)) {
            token->value = string(buf);
            return true;
        }

        if (hex && !isxdigit(c)) {
            break;
        } else if (oct && !(c >= '1' && c <= '8')) {
            break;
        } else if (dec && !isdigit(c)) {
            break;
        } else {
            buf[i] = c;
        }
    }

    /* Parse suffix */
    switch (c) {
    case 'u':
    case 'U':
        c = buffer_getchar(in);
        if (isspace(c) || buffer_iseof(in)) {
            type = kIntegerTypeUnsigned;
            return true;
        }

        switch (c) {
        case 'l':
        case 'L':
            c = buffer_getchar(in);
            if (isspace(c) || buffer_iseof(in)) {
                type = kIntegerTypeUnsignedLong;
                return true;
            }

            switch (c) {
            case 'l':
            case 'L':
                c = buffer_getchar(in);
                if (isspace(c) || buffer_iseof(in)) {
                    type = kIntegerTypeUnsignedLongLong;
                    return true;
                } else {
                    return false;
                }
            default:    return false;
            };
        default:    return false;
        };

    case 'l':
    case 'L':
        c = buffer_getchar(in);
        if (isspace(c) || buffer_iseof(in)) {
            type = kIntegerTypeLong;
            return true;
        }

        switch (c) {
        case 'l':
        case 'L':
            c = buffer_getchar(in);
            if (isspace(c) || buffer_iseof(in)) {
                type = kIntegerTypeLongLong;
                return true;
            } else {
                return false;
            }
            default:    return false;
        };
    default:    return false;
    };
}

#if defined(TEST)
static void test_integer_constant_matcher(void)
{
    strings_init();

    token_t token;
    input_buffer_t* ib;

    const char* valid[] = {
        "0",
        "12",
        "10u",
        "10UL",
        "10ull",
        "0xdeadf00d",
        "-0xdeadf00d",
        "0Xbaba17ba",
        "0xcafebeefu",
        "0xcafebeefUL",
        "0xcafebeefull",
        "012345678",
        "042u",
        "042UL",
        "042ull",
    };

    const char* invalid[] = {
        "-42",
        "deadf00d",
        "0deaff00d",
        "00",
        "",
        "-a",
        "10ulll",
    };

    for (size_t i = 0; i < countof(valid); ++i) {
        const char* str = valid[i];
        ib = buffer_mem((void*)str, strlen(str));
        CU_ASSERT(ib != NULL);
        CU_ASSERT_TRUE(match_integer_constant(ib, &token));
        CU_ASSERT_TRUE(token.type == kTokenIntConstant);
        CU_ASSERT_TRUE(0 == strcmp(_S(token.value), str));
        buffer_close(ib);
    }

    for (size_t i = 0; i < countof(invalid); ++i) {
        const char* str = invalid[i];
        ib = buffer_mem((void*)str, strlen(str));
        CU_ASSERT(ib != NULL);
        CU_ASSERT_FALSE(match_integer_constant(ib, &token));
        buffer_close(ib);
    }
}
TEST_ADD(test_integer_constant_matcher);
#endif

// Order follows matcher priority
static bool(*g_matchers[])(input_buffer_t* ib, token_t* token) = {
    &match_keyword,
    &match_identifier,
    &match_integer_constant,
};

//static regex_t g_keyword_re = {

//    NULL, NULL
//};
//
//static regex_t* g_all_regex[] = {
//    &g_keyword_re
//};

int init_scanner(void)
{
//    for (size_t i = 0; i < countof(g_all_regex); ++i)
//    {
//        regex_t* re = g_all_regex[i];
//
//        const char* error_str = NULL;
//        int error_offset = 0;
//        re->re = pcre_compile(re->spec, PCRE_CASELESS, &error_str, &error_offset, NULL);
//        if (!re->re) {
//            return -1;
//        }
//
//        // It's ok if study turns out to be NULL
//        re->study = pcre_study(re->re, 0, &error_str);
//        if (error_str) {
//            pcre_free(re->re);
//            return -1;
//        }
//    }

    return 0;
}

int parse_next_token(input_buffer_t* in, token_t* out_token)
{
    if (!in) {
        return EINVAL;
    }

    if (!out_token) {
        return EINVAL;
    }

    while (!buffer_iseof(in)) {
        for (int i = 0; i < countof(g_matchers); ++i) {
            off_t offset = buffer_get_offset(in);
            if (g_matchers[i](in, out_token)) {
                return 0;
            }

            buffer_set_offset(in, offset);
        }
    }

    return -1;
}

/////////////////////////////////////////////////////////////////////////////////

#if defined(TEST)
#include "test.h"

static void keyword_matcher_test(void)
{
    strings_init();

    input_buffer_t* ib = buffer_open("scanner.inc");
    CU_ASSERT(ib != NULL);

    printf("\n");
    token_t token;
    while (parse_next_token(ib, &token) == 0) {
        printf("%02d: \'%s\'\n", token.type, _S(token.value));
    }

    //const char* line = NULL;
    //int i = 0;
    //while (NULL != (line = buffer_getline(ib))) {
        //printf("%d: %s\n", i++, line);
        //free((void*)line);
    //}

    buffer_close(ib);
}
//TEST_ADD(keyword_matcher_test);

#endif

/////////////////////////////////////////////////////////////////////////////////
