/* Runs the unit tests in the code and records their results and reports any
 * problems. At the end, also reports the total outcome of all the tests. */
#ifndef RF_TESTER_H
#define RF_TESTER_H 1

#include <stdbool.h>

/* A function type that is a unit test. Runs the test when called, and returns
 * whether the test passed. */
typedef bool (*unit_test_t)(void);

/* Convenience macro that calls `rf_test_unit` with the name of the given
 * function. */
#define RF_TEST_UNIT(tested_unit) rf_test_unit(tested_unit, #tested_unit)

// =================================================
//    }-{   P U B L I C   F U N C T I O N S   }-{
// =================================================

/* Runs the given unit test and reports the given name if the test fails or
 * takes too long to run. */
void rf_test_unit(unit_test_t tested_unit, char const* reported_name);
/* Starts the timer for all the tests and resets counters for passed and failed
 * unit tests. */
void rf_tester_start(void);
/* Reports all the tests results and returns whether all the tests passed. */
bool rf_tester_report(void);

// =====================================================
//    ***   Q U A L I T Y   A S S E S S M E N T   ***
// =====================================================

/* Run all the tests for the tester module. */
void rf_tester_test(void);

#endif // RF_TESTER_H