# Pho Project Status

**Last Updated**: 2026-02-15  
**Status**: GTK3 Migration Phase 1 - Analysis Complete

---

## Current State

### 🔄 In Progress: GTK3 Migration Phase 1

**Branch**: `gtk3-migration-phase1`  
**Summary**: Build system updated, compilation attempted, 47 errors identified

---

## GTK3 Migration Analysis Results

### Phase 1 Complete ✅

1. **Build System Updated**
   - ✅ Makefile changed from `gtk+-2.0` to `gtk+-3.0`
   - ✅ pkg-config calls updated
   - ✅ pho.h header simplified (removed GTK2 deprecation suppression)
   - ✅ GTK3 installed via Homebrew on macOS

2. **Compilation Analysis Complete**
   - **Total Errors**: 47 compilation errors
   - **Total Files Affected**: 6 source files
   - **Error Categories**:
     - Key symbol constants (12 errors)
     - Direct widget struct access (14 errors) 
     - Deprecated signal system (6 errors)
     - Removed functions (10 errors)
     - X11-specific code (2 errors)
     - Type changes (3 errors)

---

## Detailed Error Breakdown

### gmain.c (14 errors)
| Line | Error | GTK2 API | GTK3 Replacement |
|------|-------|----------|------------------|
| 149 | Key symbol | `GDK_f` | `GDK_KEY_f` |
| 155-164 | Key symbols | `GDK_0` - `GDK_9` | `GDK_KEY_0` - `GDK_KEY_9` |
| 166 | Key symbol | `GDK_0` | `GDK_KEY_0` |
| 170 | Key symbol | `GDK_equal` | `GDK_KEY_equal` |
| 176 | Key symbol | `GDK_KP_Subtract` | `GDK_KEY_KP_Subtract` |

### gwin.c (16 errors)
| Line | Error | GTK2 API | GTK3 Replacement |
|------|-------|----------|------------------|
| 66,70 | Removed type | `GdkBitmap` | Cairo surfaces |
| 70 | Removed function | `gdk_bitmap_create_from_data` | Cairo API |
| 73 | Removed function | `gdk_cursor_new_from_pixmap` | `gdk_cursor_new_from_surface` |
| 77,78,88 | Struct access | `widget->window` | `gtk_widget_get_window(widget)` |
| 105,156,247,309 | Macro removed | `GTK_WIDGET_MAPPED` | `gtk_widget_get_mapped` |
| 111 | Removed function | `gdk_drawable_get_size` | `gtk_widget_get_allocated_width/height` |
| 313 | Removed function | `gdk_window_clear` | Cairo paint |
| 422 | Removed function | `gdk_pixbuf_render_to_drawable` | Cairo `gdk_cairo_set_source_pixbuf` |

### gdialogs.c (12 errors)
| Line | Error | GTK2 API | GTK3 Replacement |
|------|-------|----------|------------------|
| 63,83,100,232 | Struct access | `widget->window` | `gtk_widget_get_window(widget)` |
| 63,83,234 | Macro removed | `GTK_WIDGET_FLAGS` / `GTK_VISIBLE` | `gtk_widget_get_visible` |
| 174,178 | Function removed | `gtk_entry_set_editable` | `gtk_editable_set_editable` |
| 185 | Function removed | `gtk_signal_handler_block_by_func` | `g_signal_handler_block` |
| 185 | Macro removed | `GTK_OBJECT` | `G_OBJECT` |
| 186 | Type removed | `GtkSignalFunc` | `GCallback` |
| 205,206,207 | Key symbols | `GDK_Escape`, `GDK_Return`, `GDK_KP_Enter` | `GDK_KEY_*` |

### keydialog.c (9 errors)
| Line | Error | GTK2 API | GTK3 Replacement |
|------|-------|----------|------------------|
| 88 | Struct access | `widget->window` | `gtk_widget_get_window(widget)` |
| 88 | Macro removed | `GTK_WIDGET_FLAGS` / `GTK_VISIBLE` | `gtk_widget_get_visible` |
| 132 | Key symbol | `GDK_Escape` | `GDK_KEY_Escape` |
| 152-158 | Key symbols | `GDK_a`, `GDK_e`, `GDK_u`, `GDK_h`, `GDK_w`, `GDK_k`, `GDK_d` | `GDK_KEY_*` |
| 178 | Deprecated | `gtk_hbox_new` | `gtk_box_new(GTK_ORIENTATION_HORIZONTAL, ...)` |
| 193 | Function removed | `gtk_signal_connect` | `g_signal_connect` |
| 193 | Macro removed | `GTK_OBJECT` | `G_OBJECT` |

### winman.c (2 errors)
| Line | Error | GTK2 API | GTK3 Replacement |
|------|-------|----------|------------------|
| 13 | Missing header | `gdk/gdkx.h` | Platform-specific or remove |

### focustest.c (2 errors)
| Line | Error | GTK2 API | GTK3 Replacement |
|------|-------|----------|------------------|
| 43 | Missing header | `gdk/gdkx.h` | Platform-specific or remove |

---

## Migration Priority by Phase

### Phase 2: Signal & Key Symbol Updates (Next)
- [ ] Fix all `GDK_` key symbols → `GDK_KEY_` (gmain.c, gdialogs.c, keydialog.c)
- [ ] Replace `gtk_signal_connect` → `g_signal_connect` (gdialogs.c, keydialog.c)
- [ ] Replace `GTK_OBJECT` → `G_OBJECT`
- [ ] Replace `GtkSignalFunc` → `GCallback`

### Phase 3: Widget Accessors
- [ ] Fix `widget->window` → `gtk_widget_get_window(widget)`
- [ ] Fix `GTK_WIDGET_MAPPED` → `gtk_widget_get_mapped()`
- [ ] Fix `GTK_WIDGET_FLAGS` / `GTK_VISIBLE` → `gtk_widget_get_visible()`

### Phase 4: Drawing System (Critical)
- [ ] Replace `gdk_pixbuf_render_to_drawable` with Cairo
- [ ] Replace `gdk_window_clear` with Cairo paint
- [ ] Implement `draw` signal instead of `expose_event`
- [ ] Handle `GdkBitmap` / cursor creation with Cairo

### Phase 5: Removed Functions
- [ ] `gdk_drawable_get_size` → `gtk_widget_get_allocated_width/height`
- [ ] `gtk_entry_set_editable` → `gtk_editable_set_editable`
- [ ] `gtk_hbox_new` → `gtk_box_new`
- [ ] `gtk_signal_handler_block_by_func` → `g_signal_handler_block`

### Phase 6: Platform-Specific
- [ ] Handle `gdk/gdkx.h` for X11 vs macOS Quartz

---

## Quick Commands

```bash
# Check compilation errors
export PKG_CONFIG_PATH="/opt/homebrew/lib/pkgconfig:$PKG_CONFIG_PATH"
make clean && make 2>&1 | grep error:

# Test GTK3 installation
pkg-config --cflags --libs gtk+-3.0

# Run tests (after fixes)
cd tests && make test

# Check branch status
git status
git log --oneline -5
```

---

## File Locations

| Document | Purpose |
|----------|---------|
| `docs/plan.md` | Future roadmap and feature planning |
| `docs/gtk3-migration-plan.md` | Detailed GTK3 migration guide |
| `docs/completed-fixes.md` | Historical record of all fixes |
| `docs/test-review.md` | Testing strategy |
| `AGENTS.md` | Coding standards and guidelines |

---

*GTK3 Migration Phase 1 complete - 47 errors identified, ready for Phase 2 fixes*
