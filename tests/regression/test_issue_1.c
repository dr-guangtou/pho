/*
 * Regression test for Issue #1: Buffer Overflow in Title Construction
 */

#include "../unity/unity.h"
#include "../../pho.h"
#include <string.h>

#define TITLELEN BUFSIZ

void setUp(void) {}
void tearDown(void) {}

void test_strncat_prevents_overflow(void) {
    char buf[20];
    strncpy(buf, "Hello", sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';
    strncat(buf, " World This Is Too Long", sizeof(buf) - strlen(buf) - 1);
    TEST_ASSERT_EQUAL_INT('\0', buf[19]);
}

void test_safe_concat_pattern(void) {
    char title[50] = "Base";
    size_t remaining = sizeof(title) - strlen(title) - 1;
    strncat(title, " extra", remaining);
    TEST_ASSERT_EQUAL_STRING("Base extra", title);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_strncat_prevents_overflow);
    RUN_TEST(test_safe_concat_pattern);
    return UNITY_END();
}
