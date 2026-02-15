/*
 * Regression test for Issue #3: Non-Terminating strncpy() in EXIF Parser
 */

#include "../unity/unity.h"
#include "../../exif/jhead.h"
#include <string.h>

void setUp(void) { memset(&ImageInfo, 0, sizeof(ImageInfo)); }
void tearDown(void) {}

void test_camera_make_null_terminated(void) {
    char long_make[40];
    memset(long_make, 'A', sizeof(long_make) - 1);
    long_make[sizeof(long_make) - 1] = '\0';
    strncpy(ImageInfo.CameraMake, long_make, 31);
    ImageInfo.CameraMake[31] = '\0';
    TEST_ASSERT_EQUAL_INT('\0', ImageInfo.CameraMake[31]);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_camera_make_null_terminated);
    return UNITY_END();
}
