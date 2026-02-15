/* Unit tests for phoimglist.c */
#include "../unity/unity.h"
#include "../../pho.h"
#include <stdlib.h>

extern PhoImage* gFirstImage;
extern PhoImage* gCurImage;

void setUp(void) { gFirstImage = NULL; gCurImage = NULL; }
void tearDown(void) { if (gFirstImage) ClearImageList(); }

void test_append_item_to_empty_list(void) {
    PhoImage* img = NewPhoImage("test.jpg");
    AppendItem(img);
    TEST_ASSERT_EQUAL_PTR(img, gFirstImage);
}

void test_delete_only_item(void) {
    PhoImage* img = NewPhoImage("test.jpg");
    AppendItem(img);
    DeleteItem(img);
    TEST_ASSERT_NULL(gFirstImage);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_append_item_to_empty_list);
    RUN_TEST(test_delete_only_item);
    return UNITY_END();
}
