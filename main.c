#include <stdio.h>
#include <stdlib.h>

#include "arena.h"
#include "test.h"
#include "strings.h"

#if defined(TEST)

CU_pSuite g_suite = NULL;

static int RunUnitTests()
{
    /* Init cunit */
    if (CUE_SUCCESS != CU_initialize_registry()) {
        return CU_get_error();
    }

    g_suite = CU_add_suite("test suite", NULL, NULL);
    if (NULL == g_suite) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    /* run test constructors */
    test_func_ptr* t = &TEST_START;
    while (t != &TEST_END) {
        (*t++)();
    }

    /* run tests */
    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_basic_run_tests();
    CU_cleanup_registry();
    g_suite = NULL;
    return CU_get_error();
}

#else
static int RunUnitTests() { return 0; }
#endif

int main(void) 
{
    if (CUE_SUCCESS != RunUnitTests()) {
        return EXIT_FAILURE;
    }

    return 0;
}
