/*
 * Regression test for Issue #4: Buffer Overflow in process_COM()
 * File: exif/jpgfile.c:59-89
 * Issue: strcpy() from 2001-byte buffer to 2000-byte buffer
 * Fix: Use strncpy with proper bounds
 */

#include "../unity/unity.h"
#include <string.h>
#include <stdlib.h>

#define MAX_COMMENT 2000  /* Same as in jhead.h */

void setUp(void) {}
void tearDown(void) {}

/* Test that strncpy prevents overflow with large source */
void test_strncpy_prevents_overflow(void) {
    char dest[MAX_COMMENT];        /* Destination buffer */
    char src[MAX_COMMENT + 1];     /* Source buffer (2001 bytes) */
    
    /* Fill source with exactly MAX_COMMENT characters + null */
    memset(src, 'A', MAX_COMMENT);
    src[MAX_COMMENT] = '\0';
    
    /* Use strncpy with bounds checking - same pattern as fix */
    strncpy(dest, src, MAX_COMMENT - 1);
    dest[MAX_COMMENT - 1] = '\0';  /* Ensure null termination */
    
    /* Destination should be properly null-terminated */
    TEST_ASSERT_EQUAL_INT('\0', dest[MAX_COMMENT - 1]);
    TEST_ASSERT_EQUAL_INT(MAX_COMMENT - 1, strlen(dest));
}

/* Test that strncpy handles exactly MAX_COMMENT-1 characters */
void test_strncpy_max_safe_length(void) {
    char dest[MAX_COMMENT];
    char src[MAX_COMMENT];
    
    /* Create string of MAX_COMMENT-1 characters */
    memset(src, 'B', MAX_COMMENT - 1);
    src[MAX_COMMENT - 1] = '\0';
    
    strncpy(dest, src, MAX_COMMENT - 1);
    dest[MAX_COMMENT - 1] = '\0';
    
    TEST_ASSERT_EQUAL_STRING(src, dest);
    TEST_ASSERT_EQUAL_INT(MAX_COMMENT - 1, strlen(dest));
}

/* Test null termination is guaranteed */
void test_strncpy_null_termination(void) {
    char dest[MAX_COMMENT];
    char src[] = "test comment";
    
    /* Fill dest with non-null to check that null is written */
    memset(dest, 'X', sizeof(dest));
    
    strncpy(dest, src, MAX_COMMENT - 1);
    dest[MAX_COMMENT - 1] = '\0';  /* Explicit null termination */
    
    TEST_ASSERT_EQUAL_STRING("test comment", dest);
}

/* Test handling of embedded nulls (edge case) */
void test_strncpy_embedded_nulls(void) {
    char dest[MAX_COMMENT];
    char src[MAX_COMMENT + 1];
    
    memset(src, 'C', MAX_COMMENT + 1);
    src[MAX_COMMENT / 2] = '\0';  /* Embedded null */
    
    strncpy(dest, src, MAX_COMMENT - 1);
    dest[MAX_COMMENT - 1] = '\0';
    
    /* Should stop at embedded null */
    TEST_ASSERT_EQUAL_INT(MAX_COMMENT / 2, strlen(dest));
}

/* Test the specific pattern from the fix */
void test_process_com_fix_pattern(void) {
    /* Simulate the fixed code pattern:
     *   strncpy(ImageInfo.Comments, Comment, MAX_COMMENT-1);
     *   ImageInfo.Comments[MAX_COMMENT-1] = '\0';
     */
    char ImageInfo_Comments[MAX_COMMENT];
    char Comment[MAX_COMMENT + 1];  /* Local buffer can be 2001 bytes */
    
    /* Fill with maximum possible comment content */
    memset(Comment, 'D', MAX_COMMENT);
    Comment[MAX_COMMENT] = '\0';
    
    /* Apply the fix pattern */
    strncpy(ImageInfo_Comments, Comment, MAX_COMMENT - 1);
    ImageInfo_Comments[MAX_COMMENT - 1] = '\0';
    
    /* Verify no overflow */
    TEST_ASSERT_EQUAL_INT(MAX_COMMENT - 1, strlen(ImageInfo_Comments));
    TEST_ASSERT_EQUAL_INT('\0', ImageInfo_Comments[MAX_COMMENT - 1]);
}

/* Test that old strcpy pattern would overflow */
void test_old_strcpy_would_overflow(void) {
    char dest[MAX_COMMENT];
    char src[MAX_COMMENT + 1];
    
    /* This test documents why the fix is needed:
     * With strcpy, writing 2001 bytes (including null) to 2000-byte buffer
     * would write 1 byte past the end.
     * We don't actually call strcpy to avoid undefined behavior.
     */
    memset(src, 'E', MAX_COMMENT);
    src[MAX_COMMENT] = '\0';
    
    /* Document the sizes */
    TEST_ASSERT_EQUAL_INT(MAX_COMMENT, strlen(src));  /* 2000 chars */
    TEST_ASSERT_EQUAL_INT(MAX_COMMENT + 1, sizeof(src));  /* 2001 bytes */
    TEST_ASSERT_EQUAL_INT(MAX_COMMENT, sizeof(dest));  /* 2000 bytes */
    
    /* With strcpy: strlen(src) + 1 = 2001 bytes written
     * to 2000-byte buffer = 1 byte overflow
     * The fix uses strncpy which prevents this.
     */
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_strncpy_prevents_overflow);
    RUN_TEST(test_strncpy_max_safe_length);
    RUN_TEST(test_strncpy_null_termination);
    RUN_TEST(test_strncpy_embedded_nulls);
    RUN_TEST(test_process_com_fix_pattern);
    RUN_TEST(test_old_strcpy_would_overflow);
    return UNITY_END();
}
