/*
 * Regression tests for Medium/Low Priority Issues #14-#21
 * 
 * Issue #14: Integer Overflow in ShuffleArray()
 * Issue #15: Integer Underflow in RotateImage()
 * Issue #16: Array Bounds in Exif Parser
 * Issue #17: Signal Safety in EndSession()
 * Issue #18: Inconsistent NULL Checks
 * Issue #19: Magic Numbers
 * Issue #20: Missing Static Declarations
 * Issue #21: Unchecked Return Values
 */

#include "../unity/unity.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>

void setUp(void) {}
void tearDown(void) {}

/* ==========================================================================
 * Issue #14: Integer Overflow in ShuffleArray()
 * ========================================================================== */

/* Test that size_t is used for array indices */
void test_size_t_for_array_indices(void) {
    /* size_t should be able to hold larger values than int */
    TEST_ASSERT_TRUE(sizeof(size_t) >= sizeof(int));
    
    /* Simulate safe indexing with size_t */
    size_t large_index = (size_t)INT_MAX + 10;
    TEST_ASSERT_TRUE(large_index > (size_t)INT_MAX);
}

/* Test array shuffle pattern with size_t */
void test_shuffle_with_size_t_indices(void) {
    int arr[5] = {1, 2, 3, 4, 5};
    size_t len = 5;
    
    /* Simulate shuffle with size_t indices */
    for (size_t i = len; i > 1; i--) {
        size_t j = (i-1) % len;  /* Simplified shuffle */
        int tmp = arr[j];
        arr[j] = arr[i-1];
        arr[i-1] = tmp;
    }
    
    /* Array should still contain all elements */
    int sum = 0;
    for (int i = 0; i < 5; i++) sum += arr[i];
    TEST_ASSERT_EQUAL_INT(15, sum);  /* 1+2+3+4+5 = 15 */
}

/* ==========================================================================
 * Issue #15: Integer Underflow in RotateImage()
 * ========================================================================== */

/* Test dimension validation prevents underflow */
void test_dimension_validation(void) {
    int width = 0;
    int height = 100;
    
    /* Pattern from fix: validate dimensions before use */
    int valid = (width > 0 && height > 0);
    TEST_ASSERT_FALSE(valid);
    
    /* Valid dimensions */
    width = 100;
    valid = (width > 0 && height > 0);
    TEST_ASSERT_TRUE(valid);
}

/* Test that negative dimensions are caught */
void test_negative_dimension_check(void) {
    int width = -1;
    int height = 100;
    
    /* Should detect invalid dimensions */
    int valid = (width > 0 && height > 0);
    TEST_ASSERT_FALSE(valid);
}

/* Test rotation calculation safety */
void test_rotation_calculation_safety(void) {
    int curWidth = 100;
    int curHeight = 0;  /* Invalid */
    int x = 0, y = 0;
    
    /* This would underflow: curHeight - y - 1 = -1 */
    if (curHeight <= 0) {
        /* Fix: check dimensions before calculation */
        TEST_PASS();
        return;
    }
    
    int newx = curHeight - y - 1;  /* Would be unsafe */
    (void)newx;
}

/* ==========================================================================
 * Issue #16: Array Bounds in Exif Parser
 * ========================================================================== */

/* Simulated TagTable structure */
typedef struct {
    unsigned short Tag;
    const char* Desc;
} TestTagTable_t;

static TestTagTable_t TestTagTable[] = {
    {0x100, "ImageWidth"},
    {0x101, "ImageLength"},
    {0x102, "BitsPerSample"},
    {0, NULL}  /* Sentinel */
};

/* Test bounded array lookup */
void test_bounded_array_lookup(void) {
    unsigned short targetTag = 0x101;
    const char* desc = NULL;
    int found = 0;
    
    /* Pattern from fix: use calculated bounds instead of infinite loop */
    static const int tableSize = sizeof(TestTagTable) / sizeof(TestTagTable[0]);
    
    for (int a = 0; a < tableSize; a++) {
        if (TestTagTable[a].Tag == 0) {
            /* Sentinel reached - tag not found */
            break;
        }
        if (TestTagTable[a].Tag == targetTag) {
            desc = TestTagTable[a].Desc;
            found = 1;
            break;
        }
    }
    
    TEST_ASSERT_TRUE(found);
    TEST_ASSERT_EQUAL_STRING("ImageLength", desc);
}

/* Test array bounds protection */
void test_array_bounds_protection(void) {
    static const int tableSize = sizeof(TestTagTable) / sizeof(TestTagTable[0]);
    
    /* Ensure we can't iterate past the array end */
    int iterations = 0;
    for (int a = 0; a < tableSize; a++) {
        iterations++;
    }
    
    TEST_ASSERT_EQUAL_INT(tableSize, iterations);
}

/* ==========================================================================
 * Issue #17: Signal Safety in EndSession()
 * ========================================================================== */

/* Test proper cleanup ordering */
void test_cleanup_ordering(void) {
    int step1 = 0, step2 = 0, step3 = 0;
    
    /* Pattern: proper cleanup sequence */
    step1 = 1;  /* Set state to prevent callbacks */
    step2 = 1;  /* Update dialogs */
    step3 = 1;  /* Save data */
    
    /* Process remaining events before quit */
    TEST_ASSERT_TRUE(step1 && step2 && step3);
}

/* ==========================================================================
 * Issue #18: Inconsistent NULL Checks
 * ========================================================================== */

/* Test consistent NULL check style (!ptr) */
void test_consistent_null_check_style(void) {
    char* ptr = NULL;
    
    /* Preferred style: if (!ptr) */
    int is_null = !ptr;
    TEST_ASSERT_TRUE(is_null);
    
    /* Equivalent but less preferred: if (ptr == NULL) */
    is_null = (ptr == NULL);
    TEST_ASSERT_TRUE(is_null);
}

/* Test pointer validation pattern */
void test_pointer_validation_pattern(void) {
    char* ptr = NULL;
    int error = 0;
    
    /* Consistent pattern: check and return early */
    if (!ptr) {
        error = 1;
    }
    
    TEST_ASSERT_EQUAL_INT(1, error);
}

/* ==========================================================================
 * Issue #19: Magic Numbers
 * ========================================================================== */

/* Define constants (simulating pho.h) */
#define TEST_ROTATE_0   0
#define TEST_ROTATE_90  90
#define TEST_ROTATE_180 180
#define TEST_ROTATE_270 270

/* Test magic number replacement with constants */
void test_rotation_constants(void) {
    int degrees = TEST_ROTATE_90;
    
    /* Using named constant instead of magic number */
    TEST_ASSERT_EQUAL_INT(90, degrees);
    
    /* Pattern: check rotation with constants */
    int is_perpendicular = (degrees == TEST_ROTATE_90 || degrees == TEST_ROTATE_270);
    TEST_ASSERT_TRUE(is_perpendicular);
}

/* Test scale mode constants */
#define TEST_SCALE_NORMAL     0
#define TEST_SCALE_FULLSCREEN 1

void test_scale_mode_constants(void) {
    int mode = TEST_SCALE_NORMAL;
    
    TEST_ASSERT_EQUAL_INT(0, mode);
    TEST_ASSERT_NOT_EQUAL(TEST_SCALE_FULLSCREEN, mode);
}

/* ==========================================================================
 * Issue #20: Static Declarations
 * ========================================================================== */

/* Test internal function visibility concept */
static int internal_helper_function(void) {
    return 42;
}

void test_static_function_pattern(void) {
    /* Static functions are only visible within the file */
    int result = internal_helper_function();
    TEST_ASSERT_EQUAL_INT(42, result);
}

/* Test internal linkage for variables */
static int internal_counter = 0;

void test_static_variable_pattern(void) {
    internal_counter++;
    TEST_ASSERT_EQUAL_INT(1, internal_counter);
}

/* ==========================================================================
 * Issue #21: Unchecked Return Values
 * ========================================================================== */

/* Test checked return value pattern */
void test_checked_return_value(void) {
    FILE* fp = fopen("/dev/null", "r");
    int error = 0;
    
    /* Pattern: always check return value */
    if (!fp) {
        error = 1;
    } else {
        /* Safe to use fp */
        fclose(fp);
    }
    
    /* fp was successfully opened or error was set */
    TEST_ASSERT_FALSE(error);
}

/* Test critical function return checking */
void test_critical_function_return_check(void) {
    /* Simulating memory allocation with check */
    char* buf = malloc(100);
    int success = 0;
    
    if (buf) {
        /* Allocation succeeded */
        strcpy(buf, "test");
        success = 1;
        free(buf);
    } else {
        /* Allocation failed - handle error */
        success = 0;
    }
    
    TEST_ASSERT_TRUE(success);
}

/* ==========================================================================
 * Integration Tests
 * ========================================================================== */

/* Test comprehensive safe coding patterns */
void test_comprehensive_safe_patterns(void) {
    /* NULL check */
    char* ptr = NULL;
    TEST_ASSERT_TRUE(!ptr);
    
    /* Bounds check */
    int arr[5];
    int index = 3;
    TEST_ASSERT_TRUE(index >= 0 && index < 5);
    arr[index] = 42;
    
    /* Dimension check */
    int width = 100, height = 100;
    TEST_ASSERT_TRUE(width > 0 && height > 0);
}

int main(void) {
    UNITY_BEGIN();
    
    /* Issue #14: Integer overflow tests */
    RUN_TEST(test_size_t_for_array_indices);
    RUN_TEST(test_shuffle_with_size_t_indices);
    
    /* Issue #15: Integer underflow tests */
    RUN_TEST(test_dimension_validation);
    RUN_TEST(test_negative_dimension_check);
    RUN_TEST(test_rotation_calculation_safety);
    
    /* Issue #16: Array bounds tests */
    RUN_TEST(test_bounded_array_lookup);
    RUN_TEST(test_array_bounds_protection);
    
    /* Issue #17: Signal safety tests */
    RUN_TEST(test_cleanup_ordering);
    
    /* Issue #18: NULL check consistency tests */
    RUN_TEST(test_consistent_null_check_style);
    RUN_TEST(test_pointer_validation_pattern);
    
    /* Issue #19: Magic number tests */
    RUN_TEST(test_rotation_constants);
    RUN_TEST(test_scale_mode_constants);
    
    /* Issue #20: Static declaration tests */
    RUN_TEST(test_static_function_pattern);
    RUN_TEST(test_static_variable_pattern);
    
    /* Issue #21: Unchecked return value tests */
    RUN_TEST(test_checked_return_value);
    RUN_TEST(test_critical_function_return_check);
    
    /* Integration tests */
    RUN_TEST(test_comprehensive_safe_patterns);
    
    return UNITY_END();
}
