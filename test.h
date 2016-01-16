/*
 * All test fptrs are put into a separate elf section and then iterated over 
 */

#pragma once

#include <CUnit/Basic.h>

extern CU_pSuite g_suite;

typedef void(*test_func_ptr)(void);

#if defined(__GNUC__)
#	define TEST_SECTION  	".test"
#	define TEST_START 		_test_start
#	define TEST_END 		_test_end

	// Declares a CUnit constructor function that registers a new test and puts ctor pointer into special ELF section
#	define TEST_DECL_CTOR(func) 						\
		static void func ##_ctor (void) { 				\
			(void) CU_add_test(g_suite, #func, func);	\
		}												\
														\
		static test_func_ptr func ##_ctor_fptr __attribute__((__section__(TEST_SECTION))) __attribute__((used)) = func ##_ctor

	extern test_func_ptr TEST_START;
	extern test_func_ptr TEST_END;
#else
#	error Unsupported toolchain
#endif

#define TEST_ADD(func) TEST_DECL_CTOR(func)
