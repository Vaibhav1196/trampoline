/*Instance 1 of task t8*/

#include "embUnit.h"
#include "tpl_os.h"

DeclareMessage(rm_newiswithin);

/*test case:test the reaction of the system called with 
 an activation of a task*/
static void test_t8_instance1(void)
{
	StatusType result_inst_1, result_inst_2;
	StatusType received_char;
	
	result_inst_1 = ReceiveMessage(rm_newiswithin, &received_char);
	TEST_ASSERT_EQUAL_INT(E_OK, result_inst_1);
	TEST_ASSERT_EQUAL_INT(7, (int)received_char);
	
	result_inst_2 = TerminateTask();
	TEST_ASSERT_EQUAL_INT(E_OK, result_inst_2);
}

/*create the test suite with all the test cases*/
TestRef COMInternalTest_seq6_t8_instance1(void)
{
	EMB_UNIT_TESTFIXTURES(fixtures) {
		new_TestFixture("test_t8_instance1",test_t8_instance1)
	};
	EMB_UNIT_TESTCALLER(COMInternalTest,"COMInternalTest_sequence6",NULL,NULL,fixtures);
	
	return (TestRef)&COMInternalTest;
}