# Pho Project Status

**Last Updated**: 2026-02-15  
**Status**: ✅ GTK3 MIGRATION COMPLETE

---

## ✅ Completed: GTK3 Migration (2026-02-15)

### Migration Summary

**Branch**: `gtk3-migration-phase1`  
**Build Status**: ✅ SUCCESS  
**Test Status**: ✅ **50/50 PASSING**  
**Runtime Status**: ✅ VERIFIED  
**GTK Version**: 3.24.51  
**Binary**: `pho` (130KB, Mach-O 64-bit ARM64)

---

### All Phases Complete ✅

| Phase | Description | Status |
|-------|-------------|--------|
| 1 | Build System | ✅ Makefile, pkg-config, headers |
| 2 | Signals & Keys | ✅ 52 key symbols, signal connections |
| 3 | Widget Accessors | ✅ All struct access updated |
| 4 | Cairo Drawing | ✅ Full GDK→Cairo migration |
| 5 | Monitor API | ✅ Multi-monitor support |
| 6 | Final Cleanup | ✅ 0 GTK deprecation warnings |

### Key Changes

- **Drawing**: `gdk_pixbuf_render_to_drawable` → Cairo
- **Events**: `expose_event` → `draw` signal with `cairo_t*`
- **Accessors**: `widget->window` → `gtk_widget_get_window()`
- **Monitor**: `gdk_screen_*` → `GdkDisplay`/`GdkMonitor`
- **Input**: `gdk_pointer_grab` → `gdk_device_grab`

### Critical Bug Fix

**Segfault on image open**: Fixed by using cairo context from draw signal handler instead of creating nested `GdkDrawingContext`.

---

## Test Results

```
========================================
Running Unit Tests
========================================
5 Tests 0 Failures 0 Ignored OK

========================================
Running Regression Tests
========================================
45 Tests 0 Failures 0 Ignored OK

========================================
All tests passed!
========================================
```

---

## Files Modified

| File | Changes |
|------|---------|
| `Makefile` | GTK3 pkg-config |
| `pho.h` | Header cleanup, DrawImage signature |
| `dialogs.h` | IsVisible macro |
| `gmain.c` | Monitor API, key symbols |
| `gwin.c` | Cairo drawing, widgets, monitor API, NULL checks |
| `gdialogs.c` | Signals, containers, accessors |
| `keydialog.c` | Signals, containers |
| `pho.c` | NULL checks in ScaleAndRotate |
| `tests/Makefile` | GTK3 pkg-config for tests |
| `README.md` | Updated for GTK3 |

---

## Verification

```bash
# Build
$ make clean && make
# 37 warnings (C prototype style, non-GTK)
# 0 GTK deprecation warnings
# Binary: pho created successfully

# Tests
$ make test
# All 50 tests passed!

# Runtime test
$ ./pho test-img/*.jpg
# Images display correctly

# Check GTK3 linking
$ otool -L pho | grep gtk
# /opt/homebrew/opt/gtk+3/lib/libgtk-3.0.dylib
```

---

## Post-Migration Checklist

- [x] All compilation errors fixed
- [x] All GTK deprecation warnings resolved
- [x] All tests passing
- [x] Runtime tested with actual images
- [x] Documentation updated (README.md, plan.md, todo.md)
- [x] Merge to master branch ✅
- [x] Window focus feature (bring to front on launch) ✅
- [ ] Create release tag
- [ ] Linux platform verification (📌 Pinned - not required for current use)

### Recent Feature: Window Focus on Launch

**Branch**: `fix/macos-window-focus` (merged to master)  
**Status**: ✅ **Window comes to front**  
**Known Limitation**: Keyboard focus requires manual click (macOS Terminal limitation)

**Implementation**: Uses `gtk_window_set_keep_above()` hack with `g_idle_add()` deferred execution to force window to front when launching from terminal on macOS.

**Technical Details**:
- Deferred execution via `g_idle_add()` ensures window is realized before raising
- Temporary `keep_above` forces window stacking order
- `gtk_window_present()` and `gtk_widget_grab_focus()` attempted for keyboard focus
- Keyboard focus limitation appears to be macOS security policy for terminal-launched apps

---

## Next Phase: Enhanced Features

With GTK3 migration complete, the project is ready for new features:

1. **Enhanced Image Formats**: WebP, HEIF/HEIC, SVG support
2. **Performance Improvements**: Thumbnail caching, async loading
3. **UI/UX Enhancements**: Dark mode, configurable shortcuts
4. **Export Features**: Batch resize, contact sheets

See `docs/plan.md` for detailed feature roadmap.

---

*GTK2→GTK3 Migration COMPLETE and VERIFIED.*
