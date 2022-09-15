#include <module/module.test.h>
#include <module/module.h>
#include <CUnit/Basic.h>

void module_add_test() 
{
	CU_ASSERT_EQUAL(module_add(2, 14), 16);
	CU_ASSERT_EQUAL(module_add(0, 0), 0);
	CU_ASSERT_EQUAL(module_add(-1 ,6), 5);
}
