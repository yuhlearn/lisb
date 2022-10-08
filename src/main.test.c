#include <CUnit/Basic.h>
#include <scanner/scanner.test.h>
#include <parser/parser.test.h>

#define TEST_SIZE(arr) (sizeof(arr) / sizeof(TestPair))

typedef struct
{
	char *label;
	void (*test)(void);
} TestPair;

typedef struct
{
	char *label;
	TestPair *suit;
	int count;
} SuitPair;

int main()
{
	// Initialize the CUnit test registry
	if (CUE_SUCCESS != CU_initialize_registry())
		return CU_get_error();

	/* SCANNER TESTS */

	TestPair scanner_tests[] = {
		{"scanner_scan_token_test", scanner_scan_token_test},
	};

	TestPair parser_tests[] = {
		{"parser_parse_empty_test", parser_parse_empty_test},
		{"parser_parse_constant_test_1", parser_parse_constant_test_1},
		{"parser_parse_constant_test_2", parser_parse_constant_test_2},
		{"parser_parse_constant_test_3", parser_parse_constant_test_3},
		{"parser_parse_constant_test_4", parser_parse_constant_test_4},
		{"parser_parse_symbol_test_1", parser_parse_symbol_test_1},
		{"parser_parse_symbol_test_2", parser_parse_symbol_test_2},
		{"parser_parse_quote_test_1", parser_parse_quote_test_1},
		{"parser_parse_quote_test_2", parser_parse_quote_test_2},
		{"parser_parse_quote_test_3", parser_parse_quote_test_3},
		{"parser_parse_quote_list_test_1", parser_parse_quote_list_test_1},
		{"parser_parse_quote_list_test_2", parser_parse_quote_list_test_2},
		{"parser_parse_quote_list_test_3", parser_parse_quote_list_test_3},
		{"parser_parse_quote_list_test_4", parser_parse_quote_list_test_4},
		{"parser_parse_quote_list_test_5", parser_parse_quote_list_test_5},
		{"parser_parse_lambda_test", parser_parse_lambda_test},
		{"parser_parse_lambda_formals_test_1", parser_parse_lambda_formals_test_1},
		{"parser_parse_lambda_formals_test_2", parser_parse_lambda_formals_test_2},
		{"parser_parse_lambda_formals_test_3", parser_parse_lambda_formals_test_3},
		{"parser_parse_lambda_formals_test_4", parser_parse_lambda_formals_test_4},
		{"parser_parse_lambda_body_test_1", parser_parse_lambda_body_test_1},
		{"parser_parse_lambda_body_test_2", parser_parse_lambda_body_test_2},
		{"parser_parse_lambda_body_test_3", parser_parse_lambda_body_test_3},
		{"parser_parse_lambda_fail_test_1", parser_parse_lambda_fail_test_1},
		{"parser_parse_lambda_fail_test_2", parser_parse_lambda_fail_test_2},
		{"parser_parse_lambda_fail_test_3", parser_parse_lambda_fail_test_3},
		{"parser_parse_lambda_fail_test_4", parser_parse_lambda_fail_test_4},
		{"parser_parse_let_test_1", parser_parse_let_test_1},
		{"parser_parse_let_test_2", parser_parse_let_test_2},
		{"parser_parse_let_test_3", parser_parse_let_test_3},
		{"parser_parse_begin_test_1", parser_parse_begin_test_1},
		{"parser_parse_begin_test_2", parser_parse_begin_test_2},
		{"parser_parse_begin_fail_test_1", parser_parse_begin_fail_test_1},
		{"parser_parse_begin_fail_test_2", parser_parse_begin_fail_test_2},
		{"parser_parse_if_test", parser_parse_if_test},
		{"parser_parse_set_test_1", parser_parse_set_test_1},
		{"parser_parse_set_test_2", parser_parse_set_test_2},
		{"parser_parse_set_test_3", parser_parse_set_test_3},
		{"parser_parse_call_cc_test_1", parser_parse_call_cc_test_1},
		{"parser_parse_call_cc_test_2", parser_parse_call_cc_test_2},
		{"parser_parse_call_cc_test_3", parser_parse_call_cc_test_3},
		{"parser_parse_call_cc_test_4", parser_parse_call_cc_test_4},
		{"parser_parse_application_test_1", parser_parse_application_test_1},
		{"parser_parse_application_test_2", parser_parse_application_test_2},
		{"parser_parse_application_test_3", parser_parse_application_test_3},
		{"parser_parse_application_test_4", parser_parse_application_test_4},
	};

	SuitPair tests[] = {
		{"scanner_tests", scanner_tests, TEST_SIZE(scanner_tests)},
		{"parser_tests", parser_tests, TEST_SIZE(parser_tests)},
	};

	for (int i = 0; i < sizeof(tests) / sizeof(SuitPair); i++)
	{
		CU_pSuite p_suite = CU_add_suite(tests[i].label, 0, 0);

		if (NULL == p_suite)
		{
			CU_cleanup_registry();
			return CU_get_error();
		}

		for (int j = 0; j < tests[i].count; j++)
		{
			if (NULL == CU_add_test(p_suite, tests[i].suit[j].label, tests[i].suit[j].test))
			{
				CU_cleanup_registry();
				return CU_get_error();
			}
		}
	}

	// choices are: CU_BRM_SILENT, CU_BRM_NORMAL and CU_BRM_VERBOSE
	CU_basic_set_mode(CU_BRM_NORMAL);
	CU_basic_run_tests();

	return CU_get_error();
}
