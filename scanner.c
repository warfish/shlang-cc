#include "scanner.h"
#include "string.h"
#include "support.h"
#include "test.h"

#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <errno.h>
#include <assert.h>

#define SHL_IDENTIFIER_LIMIT 63

static const char* g_keywords[] = {
    "auto",
    "break",
    "case", "char", "const", "continue",
    "default", "do", "double",
    "else", "enum", "extern",
    "float", "for",
    "goto",
    "if", "inline", "int",
    "long",
    "register", "restrict", "return",
    "short", "signed", "sizeof", "static", "struct", "switch",
    "typedef",
    "union", "unsigned",
    "void", "volatile",
    "while",
    "_Alignas", "_Alignof", "_Atomic",
    "_Bool",
    "_Complex",
    "_Generic",
    "_Imaginary",
    "_Noreturn",
    "_Static_assert",
    "_Thread_local",
};

static const char* g_operators[] = {
    "+", "++", "+=",
    "-", "--", "-=",
    "*", "*=",
    "/", "/=",
    "%", "%=",
    "=", "==",
    "!", "!=",
    "<", "<=",
    ">", ">=",
    "<<", "<<=",
    ">>", ">>=",
    "&", "&&", "&=",
    "|", "||", "|=",
    "^", "^=",
    "~", "~=",
};

// Is end of word
static bool iseow(char c)
{
    // Now i see why older compilers required a new line at the end of file
    return (isspace(c) || (c == EOF));
}

static bool make_token(token_t* token, token_type_t type, const char* value, integer_literal_type_t inttype)
{
    token->type = type;
    token->value = string(value);
    token->inttype = inttype;

    return true;
}

typedef struct dfa_rule {
    char symbol;
    size_t total;
    const struct dfa_rule* next;
} dfa_rule_t;

static bool dfa_match(const dfa_rule_t* dfa, size_t total, input_buffer_t* ib, char* buf, size_t bufsize)
{
    assert(ib);
    assert(buf);

    char c = buffer_getchar(ib);
    if (iseow(c)) {
        return true;
    }

    if (bufsize == 0) {
        return false;
    }

    for (size_t i = 0; i < total; ++i) {
        if (dfa[i].symbol == c) {
            *buf = c;
            return dfa_match(dfa[i].next, dfa[i].total, ib, ++buf, --bufsize);
        }
    }

    return false;
}

static bool match_full_word(const char* word, size_t offset, input_buffer_t* in, token_t* token)
{
    assert(word);
    assert(in);
    assert(token);

    const char* str = word + offset;
    while (*str != '\0') {
        char c = buffer_getchar(in);
        if (*str != c) {
            return false;
        }

        ++str;
    }

    // We should see end of word now or match fails
    if (!iseow(buffer_getchar(in))) {
        return false;
    }

    token->value = string(word);
    return true;
}

static bool match_keyword(input_buffer_t* in, token_t* token)
{
    assert(in);
    assert(token);

#if 0
    dfa_rule_t l3_1[]  = { { '=', 0, NULL } };

    dfa_rule_t l2_1[]  = { { '=', 0, NULL }, { '+', 0, NULL } }; // '+'
    dfa_rule_t l2_2[]  = { { '=', 0, NULL }, { '-', 0, NULL } }; // '-'
    dfa_rule_t l2_3[]  = { { '=', 0, NULL } }; // '*'
    dfa_rule_t l2_4[]  = { { '=', 0, NULL } }; // '/'
    dfa_rule_t l2_5[]  = { { '=', 0, NULL } }; // '='
    dfa_rule_t l2_6[]  = { { '=', 0, NULL }, { '<', countof(l3_1), l3_1 } }; // '<'
    dfa_rule_t l2_7[]  = { { '=', 0, NULL }, { '>', countof(l3_1), l3_1 } }; // '>'
    dfa_rule_t l2_8[]  = { { '=', 0, NULL } }; // '!'
    dfa_rule_t l2_9[]  = { { '=', 0, NULL } }; // '%'
    dfa_rule_t l2_10[] = { { '=', 0, NULL }, { '&', 0, NULL } }; // '&'
    dfa_rule_t l2_11[] = { { '=', 0, NULL }, { '|', 0, NULL } }; // '|'
    dfa_rule_t l2_12[] = { { '=', 0, NULL } }; // '^'
    dfa_rule_t l2_13[] = { { '=', 0, NULL } }; // '~'

    dfa_rule_t dfa_c_o_n [] = {
        { 's', 1, dfa_match_word("const", 3) },
        { 't', 1, dfa_match_word("continue", 3) },
    };

    dfa_rule_t dfa_c_o[] = {
        { 'n', countof(dfa_c_o_n), dfa_c_o_n },
    };

    dfa_rule_t dfa_c[] = {
        { 'a', 1, dfa_match_word("case", 2) },
        { 'h', 1, dfa_match_word("char", 2) },
        { 'o', countof(dfa_c_o), dfa_c_o },
    };

    dfa_rule_t dfa[] = {
        { 'a', 1, dfa_match_word("auto", 1) },
        { 'b', 1, dfa_match_word("break", 1) },
        { 'c', countof(dfa_c), dfa_c },
        { 'd', countof(dfa_d), dfa_d },
        { 'e', countof(l2_5), l2_5 },
        { 'f', countof(l2_6), l2_6 },
        { 'g', countof(l2_7), l2_7 },
        { 'i', countof(l2_8), l2_8 },
        { 'l', countof(l2_9), l2_9 },
        { 'r', countof(l2_10), l2_10 },
        { 's', countof(l2_11), l2_11 },
        { 't', countof(l2_12), l2_12 },
        { 'u', countof(l2_13), l2_13 },
        { 'v', countof(l2_13), l2_13 },
        { 'w', countof(l2_13), l2_13 },
        { '_', countof(l2_13), l2_13 },
    };
#endif // 0

    token->type = kTokenKeyword;

    char c;
    switch(c = buffer_getchar(in)) {

    /* auto */
    case 'a':   return match_full_word("auto", 1, in, token);

    /* break */
    case 'b':   return match_full_word("break", 1, in, token);

    /* const | char | continue | case */
    case 'c':
        switch (c = buffer_getchar(in)) {
        case 'a':   return match_full_word("case", 2, in, token);
        case 'h':   return match_full_word("char", 2, in, token);
        case 'o':
            switch (c = buffer_getchar(in)) {
            case 'n':
                switch (c = buffer_getchar(in)) {
                case 's':   return match_full_word("const", 4, in, token);
                case 't':   return match_full_word("continue", 4, in, token);
                default:    return false;
                };
            default:    return false;
            };
        default: return false;
        };

    /* double | do | default */
    case 'd':
        switch (c = buffer_getchar(in)) {
        case 'e':   return match_full_word("default", 2, in, token);
        case 'o':
            switch (c = buffer_getchar(in)) {
            case 'u':   return match_full_word("double", 3, in, token);
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
        case 'x':   return match_full_word("extern", 2, in, token);
        case 'l':   return match_full_word("else", 2, in, token);
        case 'n':   return match_full_word("enum", 2, in, token);
        default:    return false;
        };

    /* float | for */
    case 'f':
        switch (c = buffer_getchar(in)) {
        case 'l':   return match_full_word("float", 2, in, token);
        case 'o':   return match_full_word("for", 2, in, token);
        default:    return false;
        };

    /* goto */
    case 'g':   return match_full_word("goto", 1, in, token);

    /* int | if | inline */
    case 'i':
        switch (c = buffer_getchar(in)) {
        case 'f':   return match_full_word("if", 2, in, token);
        case 'n':
            switch (c = buffer_getchar(in)) {
            case 't':   return match_full_word("int", 3, in, token);
            case 'l':   return match_full_word("inline", 3, in, token);
            default:    return false;
            };
        default:    return false;
        };

    /* long */
    case 'l':   return match_full_word("long", 1, in, token);

    /* return | register | restrict */
    case 'r':
        switch (c = buffer_getchar(in)) {
        case 'e':
            switch (c = buffer_getchar(in)) {
            case 'g':   return match_full_word("register", 3, in, token);
            case 's':   return match_full_word("restrict", 3, in, token);
            case 't':   return match_full_word("return", 3, in, token);
            default:    return false;
            };
        default:    return false;
        };

    /* static | short | signed | sizeof | struct | switch */
    case 's':
        switch(c = buffer_getchar(in)) {
        case 'h':   return match_full_word("short", 2, in, token);
        case 't':
            switch(c = buffer_getchar(in)) {
            case 'a':   return match_full_word("static", 3, in, token);
            case 'r':   return match_full_word("struct", 3, in, token);
            default:    return false;
            };
        case 'i':
            switch (c = buffer_getchar(in)) {
            case 'g':   return match_full_word("signed", 3, in, token);
            case 'z':   return match_full_word("sizeof", 3, in, token);
            default:    return false;
            };
        case 'w':   return match_full_word("switch", 2, in, token);
        default:    return false;
        };

    /* typedef */
    case 't':   return match_full_word("typedef", 1, in, token);

    /* unsigned | union */
    case 'u':
        switch (c = buffer_getchar(in)) {
        case 'n':
            switch (c = buffer_getchar(in)) {
            case 's':   return match_full_word("unsigned", 3, in, token);
            case 'i':   return match_full_word("union", 3, in, token);
            default:    return false;
            };
        default:    return false;
        };

    /* volatile | void */
    case 'v':
        switch (c = buffer_getchar(in)) {
        case 'o':
            switch (c = buffer_getchar(in)) {
            case 'i':   return match_full_word("void", 3, in, token);
            case 'l':   return match_full_word("volatile", 3, in, token);
            default:    return false;
            };
        default:    return false;
        };

    /* while */
    case 'w':   return match_full_word("while", 1, in, token);

    /* underscore keywords deserve a separate section */
    case '_':
        switch (c = buffer_getchar(in)) {

        /* _Alignas, _Alignof, _Atomic */
        case 'A':
            switch (c = buffer_getchar(in)) {
            case 't':   return match_full_word("_Atomic", 3, in, token);
            case 'l':
                switch (c = buffer_getchar(in)) {
                case 'i':
                    switch (c = buffer_getchar(in)) {
                    case 'g':
                        switch (c = buffer_getchar(in)) {
                        case 'n':
                            switch (c = buffer_getchar(in)) {
                            case 'a':   return match_full_word("_Alignas", 7, in, token);
                            case 'o':   return match_full_word("_Alignof", 7, in, token);
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
        case 'B':   return match_full_word("_Bool", 2, in, token);

        /* _Complex */
        case 'C':   return match_full_word("_Complex", 2, in, token);

        /* _Generic */
        case 'G':   return match_full_word("_Generic", 2, in, token);

        /* _Imaginary */
        case 'I':   return match_full_word("_Imaginary", 2, in, token);

        /* _Noreturn */
        case 'N':   return match_full_word("_Noreturn", 2, in, token);

        /* _Static_assert */
        case 'S':   return match_full_word("_Static_assert", 2, in, token);

        /* _Thread_local */
        case 'T':   return match_full_word("_Thread_local", 2, in, token);
        };

    default:    return false;
    };
}

static void test_keyword_matcher(void)
{
    strings_init();

    token_t token;
    input_buffer_t* ib;

    const char* invalid[] = {
        "class", "namespace", "template", "typename", "virtual", "final", "throw", "catch", "try",
        "bool", "true", "false", "offsetof", "alignof", "containerof", "", " "
    };

    for (size_t i = 0; i < countof(g_keywords); ++i) {
        const char* str = g_keywords[i];

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

static bool match_operator(input_buffer_t* in, token_t* token)
{
    assert(in);
    assert(token);

    // TODO: Generate dfa rules?
    dfa_rule_t l3_1[]  = { { '=', 0, NULL } };

    dfa_rule_t l2_1[]  = { { '=', 0, NULL }, { '+', 0, NULL } }; // '+'
    dfa_rule_t l2_2[]  = { { '=', 0, NULL }, { '-', 0, NULL } }; // '-'
    dfa_rule_t l2_3[]  = { { '=', 0, NULL } }; // '*'
    dfa_rule_t l2_4[]  = { { '=', 0, NULL } }; // '/'
    dfa_rule_t l2_5[]  = { { '=', 0, NULL } }; // '='
    dfa_rule_t l2_6[]  = { { '=', 0, NULL }, { '<', countof(l3_1), l3_1 } }; // '<'
    dfa_rule_t l2_7[]  = { { '=', 0, NULL }, { '>', countof(l3_1), l3_1 } }; // '>'
    dfa_rule_t l2_8[]  = { { '=', 0, NULL } }; // '!'
    dfa_rule_t l2_9[]  = { { '=', 0, NULL } }; // '%'
    dfa_rule_t l2_10[] = { { '=', 0, NULL }, { '&', 0, NULL } }; // '&'
    dfa_rule_t l2_11[] = { { '=', 0, NULL }, { '|', 0, NULL } }; // '|'
    dfa_rule_t l2_12[] = { { '=', 0, NULL } }; // '^'
    dfa_rule_t l2_13[] = { { '=', 0, NULL } }; // '~'

    dfa_rule_t dfa[] = {
        { '+', countof(l2_1), l2_1 },
        { '-', countof(l2_2), l2_2 },
        { '*', countof(l2_3), l2_3 },
        { '/', countof(l2_4), l2_4 },
        { '=', countof(l2_5), l2_5 },
        { '<', countof(l2_6), l2_6 },
        { '>', countof(l2_7), l2_7 },
        { '!', countof(l2_8), l2_8 },
        { '%', countof(l2_9), l2_9 },
        { '&', countof(l2_10), l2_10 },
        { '|', countof(l2_11), l2_11 },
        { '^', countof(l2_12), l2_12 },
        { '~', countof(l2_13), l2_13 },
    };

    char buf[SHL_IDENTIFIER_LIMIT + 1] = {0};
    if (dfa_match(dfa, countof(dfa), in, buf, SHL_IDENTIFIER_LIMIT)) {
        return make_token(token, kTokenOperator, buf, 0);
    }

    return false;
}

static void test_operator_matcher(void)
{
    strings_init();

    for (size_t i = 0; i < countof(g_operators); ++i)
    {
        input_buffer_t* ib = buffer_mem((void*)g_operators[i], strlen(g_operators[i]));
        CU_ASSERT(ib != NULL);

        token_t token;
        if(!match_operator(ib, &token)) {
            printf("%s\n", g_operators[i]);
            CU_ASSERT(0);
        }
        CU_ASSERT_TRUE(token.type == kTokenOperator);
        CU_ASSERT_TRUE(0 == strcmp(_S(token.value), g_operators[i]));

        buffer_close(ib);
    }
}
TEST_ADD(test_operator_matcher);


static bool match_identifier(input_buffer_t* in, token_t* token)
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
        if (iseow(c)) {
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


static bool match_integer_constant(input_buffer_t* in, token_t* token)
{
    assert(in);
    assert(token);

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

    char c = buffer_getchar(in);
    if (!isdigit(c)) {
        return false;
    }

    buf[i++] = c;

    if (c == '0') {
        /* Hex or oct number or just 0 */
        c = buffer_getchar(in);
        if (iseow(c)) {
            return make_token(token, kTokenIntConstant, buf, kIntegerDefaultType);
        } else if (c == 'x' || c == 'X') {
            hex = true;
            buf[i++] = c;
        } else if (c >= '1' && c <= '8') {
            oct = true;
            buf[i++] = c;
        } else {
            goto parse_suffix;
        }
    }

    bool dec = !hex && !oct;

    /* Parse remaining (x)digits */
    for (; i < SHL_IDENTIFIER_LIMIT; ++i) {
        c = buffer_getchar(in);
        if (iseow(c)) {
            return make_token(token, kTokenIntConstant, buf, kIntegerDefaultType);
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

parse_suffix:
    switch (c) {
    case 'u':
    case 'U':
        if (iseow(c = buffer_getchar(in))) {
            return make_token(token, kTokenIntConstant, buf, kIntegerTypeUnsigned);
        }

        switch (c) {
        case 'l':
        case 'L':
            if (iseow(c = buffer_getchar(in))) {
                return make_token(token, kTokenIntConstant, buf, kIntegerTypeUnsignedLong);
            }

            switch (c) {
            case 'l':
            case 'L':
                if (iseow(c = buffer_getchar(in))) {
                    return make_token(token, kTokenIntConstant, buf, kIntegerTypeUnsignedLongLong);
                }

            default:
                return false;
            };

        default:
            return false;
        };

    case 'l':
    case 'L':
        if (iseow(c = buffer_getchar(in))) {
            return make_token(token, kTokenIntConstant, buf, kIntegerTypeLong);
        }

        switch (c) {
        case 'l':
        case 'L':
            if (iseow(c = buffer_getchar(in))) {
                return make_token(token, kTokenIntConstant, buf, kIntegerTypeLongLong);
            }

        default:
            return false;
        };

    default:
        return false;
    };
}

static void test_integer_constant_matcher(void)
{
    strings_init();

    const char* bases[] = {
        "0",
        "12",
        "0xdeadf00d",
        "0Xbaba17ba",
        "012345678",
    };

    const char* suffixes[] = {
        "",     // kIntegerTypeInt
        "l",    // kIntegerTypeLong
        "LL",   // kIntegerTypeLongLong
        "u",    // kIntegerTypeUnsigned
        "ul",   // kIntegerTypeUnsignedLong
        "ULL",  // kIntegerTypeUnsignedLongLong
    };

    for (size_t i = 0; i < countof(bases); ++i)
    {
        for (size_t j = 0; j < countof(suffixes); ++j) {
            const char* base = bases[i];
            const char* suffix = suffixes[j];

            char str[32] = {0};
            strcat(str, base);
            strcat(str, suffix);

            input_buffer_t* ib = buffer_mem(str, strlen(str));
            CU_ASSERT(ib != NULL);

            token_t token;
            CU_ASSERT_TRUE(match_integer_constant(ib, &token));
            CU_ASSERT_TRUE(token.type == kTokenIntConstant);
            CU_ASSERT_TRUE(token.inttype == (integer_literal_type_t)j);
            CU_ASSERT_TRUE(!strcmp(_S(token.value), base));

            buffer_close(ib);
        }
    }

    const char* invalid[] = {
        "-42",
        "deadf00d",
        "0deaff00d",
        "00",
        "",
        "-a",
        "10ulll",
    };

    for (size_t i = 0; i < countof(invalid); ++i)
    {
        const char* str = invalid[i];

        input_buffer_t* ib = buffer_mem((void*)str, strlen(str));
        CU_ASSERT(ib != NULL);

        token_t token;
        CU_ASSERT_FALSE(match_integer_constant(ib, &token));

        buffer_close(ib);
    }
}
TEST_ADD(test_integer_constant_matcher);

/////////////////////////////////////////////////////////////////////////////////

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
    // Add all keywords and operators to string table for faster comparison
    for (size_t i = 0; i < countof(g_keywords); ++i) {
        string_t str = string(g_keywords[i]);
        if (!_S(str)) {
            return ENOMEM;
        }
    }

    for (size_t i = 0; i < countof(g_operators); ++i) {
        string_t str = string(g_operators[i]);
        if (!_S(str)) {
            return ENOMEM;
        }
    }

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

static void keyword_matcher_test(void)
{
}
TEST_ADD(keyword_matcher_test);

/////////////////////////////////////////////////////////////////////////////////
