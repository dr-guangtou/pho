# Pho - Agent Development Guide

## Overview

**Pho** is a lightweight, keyboard-driven image viewer written in C using GTK2. It was created by Akkana Peck and is designed for fast browsing of image collections with minimal UI overhead.

## Purpose

- Fast image viewing with keyboard navigation
- EXIF-aware rotation handling
- Keyword tagging and caption management
- Slideshow mode
- Presentation mode for projectors
- Works as a tool for photo workflows (culling, sorting, tagging)

## Architecture

### File Organization

| File | Purpose |
|------|---------|
| `pho.h` | Main header with data structures, constants, and globals |
| `pho.c` | Core image manipulation (loading, scaling, rotation, navigation) |
| `gmain.c` | GTK main loop, argument parsing, key event handling |
| `gwin.c` | Window management, display modes, drawing |
| `phoimglist.c` | Doubly-linked circular list for image management |
| `imagenote.c` | Caption/comment handling, notes/keywords system |
| `gdialogs.c` | Info dialog, prompts, file chooser dialog |
| `keydialog.c` | Keywords dialog for tagging images |
| `dialogs.h` | Dialog function declarations |
| `exif/` | EXIF parsing library (based on jhead) |

### Core Data Structures

```c
// Image node in circular doubly-linked list
typedef struct PhoImage_s {
    char* filename;           // Image file path
    int trueWidth, trueHeight; // Original dimensions
    int curWidth, curHeight;   // Current display dimensions
    int curRot;               // Current rotation (0, 90, 180, 270)
    int exifRot;              // EXIF-specified rotation
    unsigned long noteFlags;  // Bitmask for keywords (max 63 flags)
    unsigned int deleted;     // Marked for deletion
    struct PhoImage_s* prev;  // Previous image in list
    struct PhoImage_s* next;  // Next image in list
    char* comment;            // User comment
    char* caption;            // Image caption
} PhoImage;
```

### Key Global Variables

- `gFirstImage`: Head of circular image list
- `gCurImage`: Currently displayed image
- `gImage`: Current GdkPixbuf for display
- `gScaleMode`: Scaling behavior (NORMAL, FULLSCREEN, FULLSIZE, etc.)
- `gDisplayMode`: Display mode (NORMAL, PRESENTATION, KEYWORDS)
- `gWin`: Main GTK window widget

### Scaling Modes

| Mode | Description |
|------|-------------|
| `PHO_SCALE_NORMAL` | Fit to screen if larger |
| `PHO_SCALE_FULLSCREEN` | Scale to fill screen (up or down) |
| `PHO_SCALE_FULLSIZE` | 1:1 pixel display |
| `PHO_SCALE_IMG_RATIO` | Scale by fixed ratio of original |
| `PHO_SCALE_SCREEN_RATIO` | Scale by ratio of screen size |
| `PHO_SCALE_FIXED` | Fixed max dimension |

## Main Functions and Features

### Image Navigation

- `NextImage()`: Navigate forward, skip unloadable images
- `PrevImage()`: Navigate backward
- `ThisImage()`: Reload and display current image
- `ShowImage()`: Display current image with scaling/rotation
- `ShuffleImages()`: Randomize image order (Fisher-Yates shuffle)

### Image Manipulation

- `ScaleAndRotate()`: Main routine for scaling and rotating
- `LoadImageFromFile()`: Load image via gdk-pixbuf, read EXIF
- `RotateImage()`: Pixel-level rotation (90°, 180°, 270°)
- `ScaleToFit()`: Calculate scaled dimensions

### Display Management

- `PrepareWindow()`: Create or resize window appropriately
- `DrawImage()`: Render image to drawable
- `SetViewModes()`: Change display/scale modes
- `NewWindow()`: Create new GTK window with appropriate setup

### Keyboard Handling

Main handler: `HandleGlobalKeys()` in `gmain.c`

| Key | Action |
|-----|--------|
| Space/PageDown | Next image |
| Backspace/PageUp | Previous image |
| Home/End | First/Last image |
| f | Toggle full-size mode |
| F | Toggle fullscreen mode |
| p | Toggle presentation mode |
| k | Keywords mode |
| 0-9 | Toggle note flags |
| r/Right | Rotate 90° CW |
| R/l/Left | Rotate 90° CCW |
| +/- | Scale up/down |
| d | Delete image |
| g | Run GIMP (or $PHO_CMD) |
| q/Esc | Quit |

### EXIF Support

Located in `exif/` directory, provides:
- Camera make/model
- Date/time
- Orientation (auto-rotation)
- Exposure info (aperture, shutter, ISO)
- Flash, focal length, etc.

Uses modified jhead library for EXIF parsing.

### Dialogs

1. **Info Dialog** (`gdialogs.c`): Shows image metadata, EXIF, allows comments
2. **Keywords Dialog** (`keydialog.c`): Dedicated UI for tagging images
3. **Prompt Dialog**: Generic yes/no/cancel prompts

## Build System

Uses traditional Makefile:
```bash
make          # Build pho binary
make install  # Install to /usr/local
```

Dependencies:
- GTK+ 2.0
- GDK Pixbuf
- pkg-config

## Potential Issues and Technical Debt

### 1. GTK2 Deprecation

**Critical**: GTK2 is obsolete and no longer maintained. The codebase uses many deprecated GTK2 APIs:
- `gtk_signal_connect` (should use `g_signal_connect`)
- `GTK_OBJECT`, `GTK_SIGNAL_FUNC` macros
- Direct widget struct access (e.g., `widget->window`)
- `gdk_pixbuf_render_to_drawable` (deprecated, no direct GTK3 equivalent)

### 2. Thread Safety

- No threading used, but GTK requires all UI operations on main thread
- Signal handlers run on main thread (OK for this design)

### 3. Memory Management

- Uses `calloc()`/`malloc()` without consistent NULL checks
- Some potential memory leaks in error paths
- GLib's memory management mixed with raw C allocation
- Static buffers used in some places (not thread-safe)

### 4. Buffer Overflow Risks

- `sprintf()` used without bounds checking in several places
- Fixed-size buffers (e.g., `char buf[512]`, `char buf[BUFSIZ]`)
- String concatenation without length checks

### 5. Integer Overflow

- Image dimensions stored as `int`, could overflow with very large images
- Multiplication of width*height not checked for overflow

### 6. Error Handling

- Inconsistent error handling patterns
- Some functions return error codes that are ignored
- Silent failures in some cases

### 7. Global State

- Heavy reliance on global variables makes testing difficult
- Circular list structure requires careful manipulation
- No encapsulation of state

### 8. Rotation Algorithm

- Pixel-by-pixel rotation is slow for large images
- Could benefit from hardware acceleration or optimized libraries

### 9. Shuffle Implementation

Uses `rand()` which:
- Has poor randomness quality
- Not seeded securely (just `srand(time(NULL))`)
- Modulo bias in `rand() / (RAND_MAX / (len - i) + 1)`

### 10. File Path Handling

- No Unicode/UTF-8 handling
- Path separator assumptions may not work on all platforms

## Code Review: Modern C Best Practices

### Issues by Category

#### Security

1. **Buffer overflows**: Replace `sprintf` with `snprintf`
   ```c
   // Current (risky):
   sprintf(title, "pho: %s info", gCurImage->filename);
   
   // Better:
   snprintf(title, sizeof(title), "pho: %s info", gCurImage->filename);
   ```

2. **Format string vulnerabilities**: User input could reach format functions

3. **Integer overflow**: Check dimensions before multiplication

#### Memory Safety

1. **NULL pointer checks**: Inconsistent checking
   ```c
   // Many places assume malloc succeeds
   newimg->filename = fnam;  // no copy, we don't own the memory
   ```

2. **Use after free**: In `DeleteItem()`, careful with pointer updates

3. **Memory leaks**: Error paths may not free allocated memory

#### Type Safety

1. **Implicit int**: Old C style declarations
2. **Magic numbers**: Many bare numbers without constants
3. **Enum conversions**: Implicit casts between enums and ints

#### Modern C (C11/C17) Improvements

1. **Use `bool` from `<stdbool.h>`** instead of int for boolean values
2. **Use `const` correctness** more rigorously
3. **Anonymous structs/unions** could simplify some code
4. **Static assertions** for compile-time checks

#### Style Issues

1. **Mixed naming conventions**: `camelCase`, `snake_case`, `ALL_CAPS`
2. **Inconsistent brace style**: Sometimes K&R, sometimes Allman
3. **Comment style**: Mixed `/* */` and `//` (though codebase is mostly consistent)
4. **Indentation**: Uses spaces, but inconsistent 4-space alignment

#### GTK-Specific Issues

1. **Deprecated APIs**: Entire codebase needs GTK3/4 migration
2. **Reference counting**: Some `g_object_unref` calls could be safer
3. **Event handling**: Uses old GTK1.x signal connection style

## Recommendations for Improvement

### High Priority

1. **Security audit**: Fix all sprintf/snprintf issues
2. **GTK3 migration**: This is critical for long-term viability
3. **Add NULL checks** after all allocations
4. **Bounds checking** on all array accesses

### Medium Priority

1. **Unit tests**: Currently no test suite
2. **Static analysis**: Run clang-analyzer, cppcheck
3. **Memory debugging**: Run with valgrind/ASan
4. **Refactor globals**: Encapsulate state in a context struct

### Low Priority

1. **Code formatting**: Standardize on one style
2. **Documentation**: Add function-level documentation
3. **Error messages**: Make them more user-friendly
4. **Logging**: Replace printf debugging with proper logging

## Development Rules

### Mandatory Maintenance of docs/plan.md and docs/test-review.md

**CRITICAL**: These files are **hard-copy memory** for the project. You MUST:

**For docs/plan.md:**

1. **Read it first**: Before starting any work, check plan.md for known issues
2. **Update it constantly**: After discovering ANY new bug or issue, add it immediately
3. **Move fixed items**: When fixing an issue, move it to the "Recently Fixed" section
4. **Update maintenance log**: Record date, action, and notes for every change
5. **Prioritize**: Use the priority levels (Critical > High > Medium > Low)

This file persists across sessions and prevents context loss. It is the single source of truth for what needs to be done.

**For docs/test-review.md:**
1. **Review before testing**: Check this file for testing standards and requirements
2. **Update when adding tests**: Document new test frameworks, fixtures, or patterns
3. **Track coverage**: Update coverage metrics as tests are added
4. **Note blockers**: If testing reveals architectural issues, document them

### Mandatory Testing After Changes

**CRITICAL**: After ANY bug fix or improvement, you MUST:

1. **Verify compilation**: Run `make clean && make` to ensure clean build
2. **Test basic functionality**: Run `./pho <test-image>` to verify the binary works
3. **Check for regressions**: Test the specific area that was modified
4. **Document results**: Update `docs/todo.md` with test results

**Why this matters**: The object files (`.o`) and static library (`exif/libphoexif.a`) must be rebuilt after changes. Simply running `make` may fail if dependencies aren't tracked correctly.

Example workflow:
```bash
# After making changes:
make clean
make
./pho test-img/*.jpg  # Basic smoke test
```

### Adding a New Feature

1. Check if it's UI-related (goes in `gdialogs.c` or new file)
2. Image manipulation goes in `pho.c`
3. Add key binding in `HandleGlobalKeys()` in `gmain.c`
4. Update help text in `Usage()` and `VerboseHelp()`

### Common Pitfalls

1. **List operations**: The image list is circular - be careful with termination
2. **Window management**: GTK window state is complex; test on multiple WMs
3. **EXIF handling**: Must call `ExifReadInfo()` before using EXIF data
4. **Rotation**: Current rotation is relative, stored in `curRot`

### Testing Checklist

- Test with various image formats (JPEG, PNG, GIF, etc.)
- Test EXIF rotation handling
- Test on different window managers
- Test with very large images
- Test slideshow mode
- Test keyboard navigation thoroughly
