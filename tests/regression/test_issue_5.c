/*
 * Regression test for Issue #5: Uninitialized Variable in ScaleToFit()
 */

#include "../unity/unity.h"
#include "../../pho.h"

void setUp(void) { gScaleMode = PHO_SCALE_NORMAL; gScaleRatio = 1.0; }
void tearDown(void) {}

void test_scale_to_fit_no_scaling_needed_screen_ratio(void) {
    int width = 400, height = 300;
    ScaleToFit(&width, &height, 800, 600, PHO_SCALE_SCREEN_RATIO, 1.0);
    TEST_ASSERT_EQUAL_INT(400, width);
    TEST_ASSERT_EQUAL_INT(300, height);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_scale_to_fit_no_scaling_needed_screen_ratio);
    return UNITY_END();
}
