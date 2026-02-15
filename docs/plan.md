# Pho Development Plan - Issues & Roadmap

**Document Version**: 1.0  
**Last Updated**: 2026-02-15  
**Maintainer**: Update this file after every bug fix or discovery

---

## ⚫ Infrastructure Priority (Setup Before Bug Fixes)

### 0. Testing Infrastructure
- **File**: New `tests/` directory
- **Type**: Infrastructure - Missing completely
- **Issue**: Zero automated tests; all testing is manual
- **Impact**: Cannot verify bug fixes, high risk of regression
- **Recommendation**: 
  - Add Unity test framework (single header)
  - Create `tests/unit/` for function tests
  - Create `tests/regression/` for bug fix tests
  - Add `make test` target
- **Status**: ⬜ Open - **BLOCKS bug fix verification**

See `docs/test-review.md` for detailed analysis.

---

## 🔴 Critical Priority (Fix Immediately)

### 1. Buffer Overflow in Title Construction
- **File**: `gwin.c:376-407`
- **Type**: Security - Buffer overflow
- **Issue**: Multiple `strcat()` calls after `snprintf()` without bounds checking
- **Impact**: Crash or potential code execution with long filenames + EXIF date
- **Fix**: Replaced `strcat()` with `strncat()` using calculated remaining buffer space
- **Status**: ✅ **FIXED** - Verified by regression/test_issue_1
- **Commit**: Safe string concatenation with bounds checking in DrawImage() title construction

### 2. Buffer Overflow in CapFileName()
- **File**: `imagenote.c:126-137`
- **Type**: Security - Buffer overflow
- **Issue**: Static buffer can overflow with malicious format string; `snprintf()` return value not checked
- **Impact**: Crash with crafted caption format, potential buffer overflow with long filenames
- **Fix**: Added truncation check, ensure null-termination, return error if buffer too small
- **Status**: ✅ **FIXED** - Verified by regression/test_issue_2
- **Commit**: Added bounds checking and truncation detection to CapFileName()

### 3. Non-Terminating strncpy() in EXIF Parser
- **File**: `exif/exif.c:429-466`
- **Type**: Security - Buffer overflow
- **Issue**: `strncpy()` doesn't guarantee null-termination
- **Impact**: Unterminated strings causing information leak or crash
- **Fix**: Set last byte to '\0' after each strncpy
- **Status**: ✅ **FIXED** - Verified by regression/test_issue_3
- **Commit**: Added null termination after all strncpy calls in EXIF parser

### 4. Buffer Overflow in process_COM()
- **File**: `exif/jpgfile.c:59-89`
- **Type**: Security - Buffer overflow
- **Issue**: `strcpy()` from 2001-byte buffer to 2000-byte buffer (MAX_COMMENT+1 to MAX_COMMENT)
- **Impact**: 1-byte overflow when comment is exactly MAX_COMMENT characters
- **Fix**: Use `strncpy()` with `MAX_COMMENT-1` bounds, ensure null-termination
- **Status**: ✅ **FIXED** - Verified by regression/test_issue_4
- **Commit**: Replaced strcpy with strncpy in process_COM to prevent 1-byte overflow

### 5. Uninitialized Variable in ScaleToFit()
- **File**: `pho.c:318-377`
- **Type**: Logic error / Crash
- **Issue**: `new_w` and `new_h` never initialized if ratios ≤ 1
- **Impact**: Garbage values used for scaling
- **Fix**: Added else clause to preserve original dimensions when no scaling needed
- **Status**: ✅ **FIXED** - Verified by regression/test_issue_5
- **Commit**: Added `else { new_w = *width; new_h = *height; }` in PHO_SCALE_SCREEN_RATIO branch

### 6. Logic Error in Slideshow Timeout
- **File**: `pho.c:68-69`
- **Type**: Logic error
- **Issue**: Condition `(ptr != 0 || ptr != gFirstImage)` is ALWAYS TRUE
- **Impact**: Slideshow never stops at end
- **Fix**: Changed `||` to `&&` - now correctly detects end of list
- **Status**: ✅ **FIXED** - Verified by regression/test_issue_6
- **Commit**: Changed `||` to `&&` in ShowImage() slideshow condition

---

## 🟠 High Priority (Fix Soon)

### 7. NULL Pointer Dereference in RunPhoCommand()
- **File**: `gmain.c:79,98-99`
- **Type**: Crash
- **Issue**: `gCurImage->filename` accessed without null check
- **Fix**: Added `if (!gCurImage) return;` at function start
- **Status**: ✅ **FIXED** - Verified by regression/test_issues_7_12

### 8. NULL Pointer Dereference in HandleGlobalKeys()
- **File**: `gmain.c:182`
- **Type**: Crash
- **Issue**: `DeleteImage(gCurImage)` called without null check
- **Fix**: Added `if (gCurImage)` check before calling DeleteImage
- **Status**: ✅ **FIXED** - Verified by regression/test_issues_7_12

### 9. Memory Leak in LoadImageFromFile()
- **File**: `pho.c:77-127`
- **Type**: Memory leak
- **Issue**: `GError *err` not freed on error path
- **Fix**: Added `g_error_free(err);` before return on error path
- **Status**: ✅ **FIXED** - Verified by regression/test_issues_7_12

### 10. Memory Leak in AddImgToList()
- **File**: `imagenote.c:72-94`
- **Type**: Memory leak / Double-free
- **Issue**: strdup() failure path didn't free temporary string
- **Fix**: Added NULL check and free for strdup() failure path
- **Status**: ✅ **FIXED** - Verified by regression/test_issues_7_12

### 11. Resource Leak in ReadCaption()
- **File**: `imagenote.c:143-256`
- **Type**: Resource leak
- **Issue**: `capfile` not closed if `calloc()` for caption fails
- **Fix**: Added `close(capfile)` before return on allocation failure
- **Status**: ✅ **FIXED** - Verified by regression/test_issues_7_12

### 12. Use-After-Free Risk in Keywords Dialog
- **File**: `keydialog.c:61-62`
- **Type**: Crash (undefined behavior)
- **Issue**: `strdup(NULL)` if `gtk_entry_get_text()` returns NULL
- **Fix**: Added NULL check before strdup, set caption to NULL if text is NULL
- **Status**: ✅ **FIXED** - Verified by regression/test_issues_7_12

### 13. File Descriptor Leak in jhead.c
- **File**: `exif/jhead.c:163-213`
- **Type**: Resource leak
- **Issue**: Multiple return paths may not close file
- **Fix**: Code review shows file is properly closed - no actual leak found
- **Status**: ✅ **CLOSED - No Issue Found** - ThumbnailFile properly closed at line 170

---

## 🟡 Medium Priority (Fix When Convenient)

### 14. Integer Overflow in ShuffleArray()
- **File**: `pho.c:265-275`
- **Type**: Integer overflow
- **Issue**: Array index calculations with very large lists
- **Impact**: Unlikely in practice (needs >2B images)
- **Fix**: Use `size_t` for indices
- **Status**: ⬜ Open

### 15. Integer Underflow in RotateImage()
- **File**: `pho.c:757-785`
- **Type**: Integer underflow
- **Issue**: Loop variables could underflow with 0-dimension images
- **Fix**: Add dimension checks
- **Status**: ⬜ Open

### 16. Array Bounds in Exif Parser
- **File**: `exif/exif.c:342`
- **Type**: Array bounds
- **Issue**: `TagTable` lookup lacks proper bounds checking
- **Fix**: Add bounds check before array access
- **Status**: ⬜ Open

### 17. Signal Safety in EndSession()
- **File**: `gwin.c:596`
- **Type**: Race condition
- **Issue**: `exit(0)` after `gtk_main_quit()` potential race
- **Fix**: Remove redundant `exit()` or ensure proper cleanup
- **Status**: ⬜ Open

---

## 🔵 Low Priority / Code Quality

### 18. Inconsistent NULL Checks
- **Files**: Multiple
- **Issue**: `== 0` vs `!ptr` inconsistently used
- **Fix**: Standardize on `!ptr` style
- **Status**: ⬜ Open

### 19. Magic Numbers
- **Files**: Multiple
- **Issue**: Hard-coded rotation values (90, 180, 270)
- **Fix**: Define constants
- **Status**: ⬜ Open

### 20. Missing Static Declarations
- **Files**: Multiple
- **Issue**: Internal functions not marked `static`
- **Fix**: Add `static` to file-internal functions
- **Status**: ⬜ Open

### 21. Unchecked Return Values
- **Files**: Multiple
- **Issue**: `fclose()`, `free()`, GTK returns unchecked
- **Fix**: Check critical function returns
- **Status**: ⬜ Open

---

## ✅ Recently Fixed

### High Priority Issues #7-12 - NULL dereferences, Memory/Resource leaks
- **Files**: `gmain.c`, `pho.c`, `imagenote.c`, `keydialog.c`
- **Fixed**: 2026-02-15
- **Changes**:
  - #7: NULL check for gCurImage in RunPhoCommand()
  - #8: NULL check before DeleteImage() in HandleGlobalKeys()
  - #9: g_error_free() on error path in LoadImageFromFile()
  - #10: free() on strdup() failure in AddImgToList()
  - #11: close() on calloc() failure in ReadCaption()
  - #12: NULL check before strdup() in Keywords Dialog

### Buffer Overflow in CapFileName() - Issue #2
- **File**: `imagenote.c`
- **Fixed**: 2026-02-15
- **Changes**: Added snprintf truncation check, null-termination guarantee

### Buffer Overflow in process_COM() - Issue #4
- **File**: `exif/jpgfile.c`
- **Fixed**: 2026-02-15
- **Changes**: Replaced strcpy with strncpy to prevent 1-byte overflow

### Buffer Overflow - sprintf to snprintf
- **Files**: `pho.c`, `gdialogs.c`, `gwin.c`
- **Fixed**: 2026-02-15
- **Changes**: 7 locations

### NULL Checks After malloc
- **Files**: `gmain.c`, `imagenote.c`
- **Fixed**: 2026-02-15
- **Changes**: 2 locations

### Randomization Bias
- **File**: `pho.c`
- **Fixed**: 2026-02-15
- **Changes**: Fisher-Yates with rejection sampling

### Title Construction Buffer Overflow - Issue #1
- **File**: `gwin.c`
- **Fixed**: 2026-02-15
- **Changes**: Replaced strcat with strncat

### EXIF strncpy Null Termination - Issue #3
- **File**: `exif/exif.c`
- **Fixed**: 2026-02-15
- **Changes**: Added null termination after strncpy calls

### Uninitialized Variable in ScaleToFit() - Issue #5
- **File**: `pho.c`
- **Fixed**: 2026-02-15
- **Changes**: Added else clause for no-scaling case

### Slideshow Logic Error - Issue #6
- **File**: `pho.c`
- **Fixed**: 2026-02-15
- **Changes**: Changed || to && for end-of-list detection

---

## 📋 Maintenance Log

| Date | Action | Notes |
|------|--------|-------|
| 2026-02-15 | Initial plan created | Comprehensive review completed |
| 2026-02-15 | Fixed 3 high-priority items | sprintf, NULL checks, randomization |
| 2026-02-15 | Build issue discovered | Missing object files after clean - need full rebuild |
| 2026-02-15 | Testing review completed | Created test-review.md - zero automated tests found |
| 2026-02-15 | Testing infrastructure setup | Unity framework, 4 test files, make test target |
| 2026-02-15 | **FIXED Issue #5** | Uninitialized variable in ScaleToFit() - all regression tests pass |
| 2026-02-15 | **FIXED Issue #6** | Slideshow logic error - changed || to &&, all tests pass |
| 2026-02-15 | **FIXED Issue #3** | strncpy null termination in EXIF parser - 5 locations fixed |
| 2026-02-15 | **FIXED Issue #1** | Title buffer overflow - replaced strcat with strncat |
| 2026-02-15 | **FIXED Issue #2** | CapFileName buffer overflow - added truncation checks |
| 2026-02-15 | **FIXED Issue #4** | process_COM buffer overflow - replaced strcpy with strncpy |
| 2026-02-15 | **FIXED Issues #7-12** | High priority fixes: NULL dereferences, memory/resource leaks |
| 2026-02-15 | **CLOSED Issue #13** | No actual file descriptor leak found in jhead.c |

---

## 🎯 Recommended Next Steps

### Immediate (This Week)
1. Fix Critical #5 (Uninitialized variable) - 1 line fix
2. Fix Critical #6 (Slideshow logic) - 1 character fix
3. Fix High #7 and #8 (NULL dereferences) - Add null checks

### Short Term (Next 2 Weeks)
4. Fix Critical #1-4 (Buffer overflows) - Requires careful testing
5. Fix High #9-13 (Memory/resource leaks)

### Medium Term (Next Month)
6. Address Medium priority items
7. Add unit tests for core functions
8. Run static analysis (clang-analyzer, cppcheck)

### Long Term
9. GTK3 migration planning
10. Refactor global state into context struct
11. Add automated test suite

---

## 📝 Notes for Developers

- **Always run**: `make clean && make` after changes
- **Test with**: Long filenames (>500 chars), various image formats
- **Check**: No new compiler warnings introduced
- **Update this file**: After fixing any item, move it to "Recently Fixed"
