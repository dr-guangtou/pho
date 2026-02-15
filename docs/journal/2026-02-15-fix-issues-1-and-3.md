# Bug Fix Journal - Issues #1 and #3

**Date**: 2026-02-15  
**Issues**: #1 (Title buffer overflow) and #3 (strncpy null termination)

---

## Issue #1: Buffer Overflow in Title Construction

**File**: `gwin.c:376-407`  
**Type**: Security - Buffer overflow

### Problem
Multiple `strcat()` calls appended to `title[BUFSIZ]` without bounds checking:
1. After `snprintf()` creates the base title
2. EXIF date appended with `strcat()`
3. Scale mode info appended with `strcat()`

With a long filename and EXIF date present, the buffer could overflow.

### Solution
Replaced all `strcat()` with `strncat()` using calculated remaining space:

```c
/* Before - dangerous: */
strcat(title, " (");
strcat(title, date);
strcat(title, ")");

/* After - safe: */
size_t remaining = sizeof(title) - strlen(title) - 1;
if (remaining >= strlen(date) + 3) {
    strncat(title, " (", remaining);
    strncat(title, date, remaining - 2);
    strncat(title, ")", remaining - 2 - strlen(date));
}
```

### Key Changes
- Calculate remaining buffer space before each append
- Use `strncat()` with the remaining size
- Check if there's enough room before appending
- All `strcat()` calls converted to `strncat()`

### Testing
All 5 regression tests pass:
- `test_strncat_prevents_overflow`
- `test_title_construction_with_long_filename`
- `test_safe_concat_pattern`
- `test_truncation_when_no_room`
- `test_multiple_safe_concats`

---

## Issue #3: Non-Terminating strncpy() in EXIF Parser

**File**: `exif/exif.c:429-466`  
**Type**: Security - Buffer overflow

### Problem
`strncpy()` doesn't guarantee null-termination when source string is >= max length:

```c
/* Dangerous - may not be null-terminated: */
strncpy(ImageInfo.CameraMake, (char*)ValuePtr, 31);
```

If `ValuePtr` is 31+ characters, `CameraMake` won't have a null terminator.

### Solution
Added explicit null termination after each `strncpy()`:

```c
/* Safe - explicitly null-terminated: */
strncpy(ImageInfo.CameraMake, (char*)ValuePtr, 31);
ImageInfo.CameraMake[31] = '\0';
```

### Locations Fixed
1. `TAG_MAKE` - `CameraMake[32]` (set `[31]`)
2. `TAG_MODEL` - `CameraModel[40]` (set `[39]`)
3. `TAG_DATETIME_ORIGINAL` - `DateTime[20]` (set `[19]`)
4. `TAG_USERCOMMENT` (ASCII path) - `Comments[2000]` (set `[199]`)
5. `TAG_USERCOMMENT` (non-ASCII path) - `Comments[2000]` (set `[199]`)

### Testing
All 5 regression tests pass:
- `test_camera_make_null_terminated`
- `test_camera_model_null_terminated`
- `test_datetime_null_terminated`
- `test_comments_null_terminated`
- `test_strncpy_without_null_term_is_dangerous`

---

## Summary

| Issue | Severity | Lines Changed | Tests Added | Status |
|-------|----------|---------------|-------------|--------|
| #1 | High | ~30 | 5 | ✅ Fixed |
| #3 | High | ~5 | 5 | ✅ Fixed |

Both fixes follow secure coding practices and are verified by regression tests.
