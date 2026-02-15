# Lessons Learned - Pho Project

## Security Best Practices

### Buffer Safety
- **Always use `snprintf` instead of `sprintf`** - Even if buffer seems "big enough", long filenames or edge cases can overflow
- **Calculate remaining buffer size** when appending to strings (e.g., `sizeof(title) - (titleEnd - title)`)
- **Never assume input size** - Filenames can be up to PATH_MAX bytes

### Memory Safety
- **Always check malloc/calloc return values** - Out-of-memory conditions can happen
- **Fail gracefully** - Print error message and return rather than crashing
- **Be consistent** - Some parts of the codebase check, others don't - establish a standard

### Randomization
- **Avoid modulo bias** - Using `rand() % n` or scaling formulas introduces bias
- **Use rejection sampling** - Discard values that would create bias
- **Fisher-Yates is the standard** - Iterate backwards, swap with random earlier element

## Code Patterns in This Codebase

### Error Handling
```c
// Pattern found: Sometimes checks, sometimes doesn't
PhoImage* newimg = calloc(1, sizeof(PhoImage));
if (newimg == 0) return 0;  // Good

// vs

char** new_argv = malloc(sizeof(gchar *) * new_argc);
// No check! - Fixed in this session
```

### String Building
```c
// Old pattern (unsafe):
char buf[512];
sprintf(buf, "Delete file %s?", filename);

// New pattern (safe):
char buf[512];
snprintf(buf, sizeof(buf), "Delete file %s?", filename);
```

## Build System Notes

- Uses traditional Makefile (no autotools/cmake)
- GTK+2 is deprecated but still available via Homebrew
- Warnings about `-Wstrict-prototypes` are expected for GTK2 code
- Compiles cleanly on macOS ARM64 with clang

## Test Writing Lessons

### Always Verify Test Expectations

When writing tests, the expected values must be based on the actual function behavior, not what you think it should do.

**Example**: The ScaleToFit function applies `scaleRatio` twice in certain modes:
1. First to calculate intermediate size
2. Again at the final assignment

This resulted in 400x300 * 2.0 * 2.0 = 1600x1200, not the originally expected 800x600.

**Rule**: When a test fails, verify whether the code is wrong or the test expectation is wrong before "fixing" the code.

## Bug Fix Lessons

### Issue #5: Uninitialized Variable

**The Bug**: Variables used without initialization in edge case

**The Fix**: 
```c
// Before - undefined behavior when no scaling needed:
if (condition) {
    new_w = ...;  // initialized
    new_h = ...;  // initialized
}
// new_w, new_h may be uninitialized here!

// After - always initialized:
if (condition) {
    new_w = ...;
    new_h = ...;
} else {
    new_w = *width;   // preserve original
    new_h = *height;  // preserve original
}
```

**Key Insight**: Every code path must initialize variables before use

### Issue #6: Logic Error with || vs &&

**The Bug**: Used `||` instead of `&&`, creating always-true condition

**The Fix**:
```c
// Before - ALWAYS TRUE:
if (ptr != 0 || ptr != gFirstImage)  // A pointer can't be both!

// After - correctly checks for valid next:
if (ptr != 0 && ptr != gFirstImage)  // Must be non-null AND not wrapping
```

**Key Insight**: 
- `||` (OR) is true if EITHER condition is true
- `&&` (AND) is true only if BOTH conditions are true
- Boolean algebra mistakes create subtle bugs

**Testing Value**: Unit tests verified slideshow behavior at list boundaries

## Testing Strategy

### Critical Rule
**Always run `make clean && make` after changes** - Never assume incremental builds work correctly. The Makefile dependencies may not be perfect, and object files may become stale.

### For All Changes
1. Run `make clean && make` (not just `make`)
2. Verify binary is created: `ls -la pho`
3. Test basic functionality: `./pho test-img/*.jpg`
4. Test the specific feature that was modified
5. Verify no regressions in related areas

### For Security Fixes
1. Verify compilation succeeds
2. Test with edge cases (long filenames, many files)
3. Test error conditions (low memory if possible)
4. Verify no functional regressions

## File Organization

- Core logic: `pho.c`
- UI/GTK: `gmain.c`, `gwin.c`, `gdialogs.c`, `keydialog.c`
- Data structures: `phoimglist.c`
- Features: `imagenote.c` (captions/notes)
- EXIF: `exif/` directory
