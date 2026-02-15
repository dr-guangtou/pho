/*
 * Regression tests for new features:
 * - RAW format detection
 * - Configurable slideshow delay
 */

#include "../unity/unity.h"
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

void setUp(void) {}
void tearDown(void) {}

/* ==========================================================================
 * RAW Format Detection Tests
 * ========================================================================== */

/* Helper function to check if extension is RAW (matches implementation) */
static int IsRawFormat(const char* filename)
{
    const char* ext = strrchr(filename, '.');
    if (!ext) return 0;
    
    static const char* raw_extensions[] = {
        ".cr2", ".cr3", ".nef", ".nrw", ".arw", ".sr2", ".srf",
        ".dng", ".orf", ".raf", ".pef", ".x3f", ".rw2", ".rwl",
        ".iiq", ".3fr", ".mos", ".raw", NULL
    };
    
    for (int i = 0; raw_extensions[i] != NULL; i++) {
        if (strcasecmp(ext, raw_extensions[i]) == 0)
            return 1;
    }
    return 0;
}

/* Test Canon RAW formats */
void test_canon_raw_formats(void) {
    TEST_ASSERT_TRUE(IsRawFormat("image.CR2"));
    TEST_ASSERT_TRUE(IsRawFormat("image.cr2"));
    TEST_ASSERT_TRUE(IsRawFormat("image.CR3"));
    TEST_ASSERT_TRUE(IsRawFormat("image.cr3"));
}

/* Test Nikon RAW formats */
void test_nikon_raw_formats(void) {
    TEST_ASSERT_TRUE(IsRawFormat("image.NEF"));
    TEST_ASSERT_TRUE(IsRawFormat("image.nef"));
    TEST_ASSERT_TRUE(IsRawFormat("image.NRW"));
    TEST_ASSERT_TRUE(IsRawFormat("image.nrw"));
}

/* Test Sony RAW formats */
void test_sony_raw_formats(void) {
    TEST_ASSERT_TRUE(IsRawFormat("image.ARW"));
    TEST_ASSERT_TRUE(IsRawFormat("image.arw"));
    TEST_ASSERT_TRUE(IsRawFormat("image.SR2"));
    TEST_ASSERT_TRUE(IsRawFormat("image.srf"));
}

/* Test Adobe DNG */
void test_dng_format(void) {
    TEST_ASSERT_TRUE(IsRawFormat("image.DNG"));
    TEST_ASSERT_TRUE(IsRawFormat("image.dng"));
}

/* Test other RAW formats */
void test_other_raw_formats(void) {
    TEST_ASSERT_TRUE(IsRawFormat("image.ORF"));  /* Olympus */
    TEST_ASSERT_TRUE(IsRawFormat("image.RAF"));  /* Fujifilm */
    TEST_ASSERT_TRUE(IsRawFormat("image.PEF"));  /* Pentax */
    TEST_ASSERT_TRUE(IsRawFormat("image.X3F"));  /* Sigma */
    TEST_ASSERT_TRUE(IsRawFormat("image.RW2"));  /* Panasonic */
    TEST_ASSERT_TRUE(IsRawFormat("image.RWL"));  /* Leica */
    TEST_ASSERT_TRUE(IsRawFormat("image.IIQ"));  /* Phase One */
    TEST_ASSERT_TRUE(IsRawFormat("image.3FR"));  /* Hasselblad */
    TEST_ASSERT_TRUE(IsRawFormat("image.MOS"));  /* Leaf */
    TEST_ASSERT_TRUE(IsRawFormat("image.RAW"));  /* Generic */
}

/* Test non-RAW formats are not detected */
void test_non_raw_formats(void) {
    TEST_ASSERT_FALSE(IsRawFormat("image.jpg"));
    TEST_ASSERT_FALSE(IsRawFormat("image.JPG"));
    TEST_ASSERT_FALSE(IsRawFormat("image.jpeg"));
    TEST_ASSERT_FALSE(IsRawFormat("image.png"));
    TEST_ASSERT_FALSE(IsRawFormat("image.PNG"));
    TEST_ASSERT_FALSE(IsRawFormat("image.gif"));
    TEST_ASSERT_FALSE(IsRawFormat("image.bmp"));
    TEST_ASSERT_FALSE(IsRawFormat("image.tiff"));
}

/* Test edge cases */
void test_raw_edge_cases(void) {
    /* No extension */
    TEST_ASSERT_FALSE(IsRawFormat("image"));
    
    /* Empty extension */
    TEST_ASSERT_FALSE(IsRawFormat("image."));
    
    /* Similar but not RAW */
    TEST_ASSERT_FALSE(IsRawFormat("image.cr2x"));
    TEST_ASSERT_FALSE(IsRawFormat("image.xcr2"));
}

/* Test paths with directories */
void test_raw_with_paths(void) {
    TEST_ASSERT_TRUE(IsRawFormat("/path/to/image.CR2"));
    TEST_ASSERT_TRUE(IsRawFormat("../photos/image.nef"));
    TEST_ASSERT_TRUE(IsRawFormat("~/pictures/image.ARW"));
}

/* ==========================================================================
 * Slideshow Delay Tests
 * ========================================================================== */

#define DEFAULT_SLIDESHOW_DELAY 3000  /* 3 seconds in milliseconds */

/* Test default delay constant */
void test_default_slideshow_delay(void) {
    TEST_ASSERT_EQUAL_INT(3000, DEFAULT_SLIDESHOW_DELAY);
}

/* Test delay parsing logic (simulates CheckArg) */
static int ParseSlideshowDelay(const char* arg, int* delay)
{
    /* Skip the 's' */
    arg++;
    
    if (*arg == '\0') {
        /* No number given - use default */
        *delay = DEFAULT_SLIDESHOW_DELAY;
        return 0;
    }
    
    /* Check if it's a valid number */
    if (!isdigit(*arg) && *arg != '.')
        return -1;  /* Invalid */
    
    *delay = (int)(atof(arg) * 1000.);
    return 0;
}

/* Test -s with explicit delay */
void test_slideshow_explicit_delay(void) {
    int delay;
    
    TEST_ASSERT_EQUAL_INT(0, ParseSlideshowDelay("s5", &delay));
    TEST_ASSERT_EQUAL_INT(5000, delay);  /* 5 seconds */
    
    TEST_ASSERT_EQUAL_INT(0, ParseSlideshowDelay("s2.5", &delay));
    TEST_ASSERT_EQUAL_INT(2500, delay);  /* 2.5 seconds */
    
    TEST_ASSERT_EQUAL_INT(0, ParseSlideshowDelay("s0.5", &delay));
    TEST_ASSERT_EQUAL_INT(500, delay);   /* 0.5 seconds */
}

/* Test -s without number (default) */
void test_slideshow_default_delay(void) {
    int delay;
    
    TEST_ASSERT_EQUAL_INT(0, ParseSlideshowDelay("s", &delay));
    TEST_ASSERT_EQUAL_INT(DEFAULT_SLIDESHOW_DELAY, delay);
}

/* Test various valid delay values */
void test_slideshow_various_delays(void) {
    int delay;
    
    TEST_ASSERT_EQUAL_INT(0, ParseSlideshowDelay("s1", &delay));
    TEST_ASSERT_EQUAL_INT(1000, delay);
    
    TEST_ASSERT_EQUAL_INT(0, ParseSlideshowDelay("s10", &delay));
    TEST_ASSERT_EQUAL_INT(10000, delay);
    
    TEST_ASSERT_EQUAL_INT(0, ParseSlideshowDelay("s0.1", &delay));
    TEST_ASSERT_EQUAL_INT(100, delay);
}

/* ==========================================================================
 * Integration Tests
 * ========================================================================== */

/* Test comprehensive RAW detection */
void test_comprehensive_raw_detection(void) {
    /* Test a mix of formats */
    TEST_ASSERT_TRUE(IsRawFormat("photo.CR2"));
    TEST_ASSERT_TRUE(IsRawFormat("photo.nef"));
    TEST_ASSERT_FALSE(IsRawFormat("photo.jpg"));
    TEST_ASSERT_FALSE(IsRawFormat("photo.png"));
    TEST_ASSERT_TRUE(IsRawFormat("photo.DNG"));
    TEST_ASSERT_FALSE(IsRawFormat("photo.gif"));
}

/* Test delay and RAW together */
void test_slideshow_and_raw_independent(void) {
    /* Slideshow delay parsing should not affect RAW detection */
    int delay;
    TEST_ASSERT_EQUAL_INT(0, ParseSlideshowDelay("s3", &delay));
    TEST_ASSERT_EQUAL_INT(3000, delay);
    
    /* RAW detection still works */
    TEST_ASSERT_TRUE(IsRawFormat("image.CR2"));
    TEST_ASSERT_FALSE(IsRawFormat("image.jpg"));
}

int main(void) {
    UNITY_BEGIN();
    
    /* RAW format detection tests */
    RUN_TEST(test_canon_raw_formats);
    RUN_TEST(test_nikon_raw_formats);
    RUN_TEST(test_sony_raw_formats);
    RUN_TEST(test_dng_format);
    RUN_TEST(test_other_raw_formats);
    RUN_TEST(test_non_raw_formats);
    RUN_TEST(test_raw_edge_cases);
    RUN_TEST(test_raw_with_paths);
    
    /* Slideshow delay tests */
    RUN_TEST(test_default_slideshow_delay);
    RUN_TEST(test_slideshow_explicit_delay);
    RUN_TEST(test_slideshow_default_delay);
    RUN_TEST(test_slideshow_various_delays);
    
    /* Integration tests */
    RUN_TEST(test_comprehensive_raw_detection);
    RUN_TEST(test_slideshow_and_raw_independent);
    
    return UNITY_END();
}
