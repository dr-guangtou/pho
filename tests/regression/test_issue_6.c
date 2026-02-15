/*
 * Regression test for Issue #6: Logic Error in Slideshow Timeout
 */

#include "../unity/unity.h"
#include "../../pho.h"

extern PhoImage* gFirstImage;
extern PhoImage* gCurImage;
extern int gDelayMillis;
extern int gPendingTimeout;

static int should_add_timeout(void) {
    return (gDelayMillis > 0 && gPendingTimeout == 0
        && (gCurImage->next != 0 && gCurImage->next != gFirstImage));
}

void setUp(void) {
    gFirstImage = NULL; gCurImage = NULL;
    gDelayMillis = 1000; gPendingTimeout = 0;
}

void test_slideshow_not_added_at_end(void) {
    PhoImage img;
    gFirstImage = &img; gCurImage = &img;
    img.next = &img; img.prev = &img;
    TEST_ASSERT_EQUAL_INT(0, should_add_timeout());
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_slideshow_not_added_at_end);
    return UNITY_END();
}
