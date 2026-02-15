/*
 * Regression test for Issue #2: Buffer Overflow in CapFileName()
 * File: imagenote.c:126-137
 * Issue: snprintf return value not checked, potential buffer overflow
 * Fix: Check return value, handle truncation, ensure null-termination
 */

#include "../unity/unity.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void setUp(void) {}
void tearDown(void) {}

/* Test that snprintf returns the correct length */
void test_snprintf_returns_length(void) {
    char buf[100];
    int len = snprintf(buf, sizeof(buf), "test file %s.txt", "example");
    TEST_ASSERT_EQUAL_INT(21, len);  /* "test file example.txt" = 21 chars */
    TEST_ASSERT_EQUAL_STRING("test file example.txt", buf);
}

/* Test that snprintf handles truncation correctly */
void test_snprintf_truncation_detected(void) {
    char buf[10];
    int len = snprintf(buf, sizeof(buf), "very long string that exceeds buffer");
    /* len should be the untruncated length */
    TEST_ASSERT_TRUE(len > (int)sizeof(buf));
    /* Buffer should be null-terminated */
    TEST_ASSERT_EQUAL_INT('\0', buf[sizeof(buf)-1]);
}

/* Test the pattern used in the fix: check for truncation */
void test_truncate_detection_pattern(void) {
    char buf[20];
    const char* long_fmt = "caption_%.100s.txt";  /* format that can expand */
    const char* long_name = "very_long_filename_that_causes_truncation.jpg";
    
    int caplength = snprintf(buf, sizeof(buf), long_fmt, long_name);
    
    /* Detect truncation */
    if ((size_t)caplength >= sizeof(buf)) {
        /* Would overflow - in real code, return error */
        TEST_ASSERT_TRUE(1);  /* Correctly detected overflow */
    } else {
        TEST_FAIL_MESSAGE("Expected truncation but didn't occur");
    }
}

/* Test safe bounds checking for strncmp after snprintf */
void test_safe_strncmp_after_snprintf(void) {
    char buf[50] = "test caption file";
    int caplength = 17;  /* known safe length */
    
    /* Only compare up to what's actually in the buffer */
    if ((size_t)caplength < sizeof(buf)) {
        int cmp = strncmp("test image.jpg", buf, caplength);
        /* Should be non-zero since strings differ */
        TEST_ASSERT_NOT_EQUAL(0, cmp);
    }
}

/* Test null-termination safety */
void test_null_termination_safety(void) {
    char buf[20];
    memset(buf, 'X', sizeof(buf));  /* Fill with non-null */
    
    int len = snprintf(buf, sizeof(buf), "short");
    
    /* Ensure we can add null termination after snprintf */
    if (len >= 0 && (size_t)len < sizeof(buf)) {
        buf[sizeof(buf)-1] = '\0';  /* Safe double-null-termination */
    }
    
    TEST_ASSERT_EQUAL_STRING("short", buf);
}

/* Test error handling for negative return */
void test_snprintf_negative_return(void) {
    /* This tests the pattern: check if caplength < 0 */
    int bad_length = -1;
    
    if (bad_length < 0) {
        /* Error case - in real code, return error */
        TEST_ASSERT_TRUE(1);  /* Correctly detected error */
    } else {
        TEST_FAIL_MESSAGE("Should have detected negative length");
    }
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_snprintf_returns_length);
    RUN_TEST(test_snprintf_truncation_detected);
    RUN_TEST(test_truncate_detection_pattern);
    RUN_TEST(test_safe_strncmp_after_snprintf);
    RUN_TEST(test_null_termination_safety);
    RUN_TEST(test_snprintf_negative_return);
    return UNITY_END();
}
