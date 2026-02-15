/* Unit tests for pho.c */
#include "../unity/unity.h"
#include "../../pho.h"
#include <stdlib.h>

static PhoImage* test_img = NULL;

void setUp(void) { test_img = NULL; }
void tearDown(void) { if (test_img) { free(test_img); test_img = NULL; } }

void test_new_pho_image_allocates_memory(void) {
    test_img = NewPhoImage("test.jpg");
    TEST_ASSERT_NOT_NULL(test_img);
}

void test_new_pho_image_sets_filename(void) {
    test_img = NewPhoImage("test.jpg");
    TEST_ASSERT_EQUAL_STRING("test.jpg", test_img->filename);
}

void test_scale_to_fit_no_scaling_needed(void) {
    int width = 400, height = 300;
    ScaleToFit(&width, &height, 800, 600, PHO_SCALE_SCREEN_RATIO, 1.0);
    TEST_ASSERT_EQUAL_INT(400, width);
    TEST_ASSERT_EQUAL_INT(300, height);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_new_pho_image_allocates_memory);
    RUN_TEST(test_new_pho_image_sets_filename);
    RUN_TEST(test_scale_to_fit_no_scaling_needed);
    return UNITY_END();
}
