#include <scanner/scanner.test.h>
#include <CUnit/Basic.h>

int main()
{
	CU_pSuite pSuite = NULL;

	// Initialize the CUnit test registry
	if (CUE_SUCCESS != CU_initialize_registry())
		return CU_get_error();

	// Add suits
	pSuite = CU_add_suite("scannerScanTokenTest", 0, 0);
	if (NULL == pSuite) // Always check if add was successful
	{
		CU_cleanup_registry();
		return CU_get_error();
	}

	// Add tests to the suite
	if (NULL == CU_add_test(pSuite, "scannerScanTokenTest", scannerScanTokenTest))
	{
		CU_cleanup_registry();
		return CU_get_error();
	}

	// Other choices are: CU_BRM_SILENT and CU_BRM_NORMAL
	CU_basic_set_mode(CU_BRM_VERBOSE);
	CU_basic_run_tests();

	return CU_get_error();
}
