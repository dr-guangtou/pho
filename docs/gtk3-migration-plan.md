# GTK2 to GTK3 Migration Plan

**Document Version**: 1.0  
**Last Updated**: 2026-02-15  
**Priority**: High  
**Estimated Effort**: 2-3 weeks  
**Risk Level**: High (complex API changes)

---

## Executive Summary

Pho currently uses GTK+ 2.0, which is deprecated and no longer maintained. This migration to GTK3 is essential for:
- Long-term viability
- Modern OS compatibility
- Security updates
- HiDPI display support
- Future Wayland compatibility

---

## Pre-Migration Analysis

### Current GTK2 Usage Statistics
- **Total GTK/GDK function calls**: ~400+ across codebase
- **Files affected**: `pho.c`, `gmain.c`, `gwin.c`, `gdialogs.c`, `keydialog.c`, `winman.c`, `focustest.c`
- **Key areas**:
  - Main window management (`gwin.c` - ~200 GTK calls)
  - Dialog boxes (`gdialogs.c`, `keydialog.c`)
  - Image display and scaling (`pho.c`)

### Critical API Changes Required

| GTK2 API | GTK3 Replacement | Files Affected | Difficulty |
|----------|------------------|----------------|------------|
| `gtk_signal_connect` | `g_signal_connect` | gwin.c, gdialogs.c, keydialog.c | Easy |
| `GTK_OBJECT` | `G_OBJECT` | gwin.c, gdialogs.c, keydialog.c | Easy |
| `gtk_object_destroy` | `gtk_widget_destroy` | gwin.c | Easy |
| `widget->window` | `gtk_widget_get_window(widget)` | gwin.c, winman.c, focustest.c | Medium |
| `gtk_drawing_area_size` | `gtk_widget_set_size_request` | gwin.c, winman.c, focustest.c | Easy |
| `gtk_widget_modify_bg` | CSS styling via GtkStyleProvider | gwin.c | Medium |
| `gdk_pixbuf_render_to_drawable` | Cairo rendering | gwin.c | **Hard** |
| `gdk_window_clear` | Cairo paint | gwin.c | Medium |
| `gdk_drawable_get_size` | `gtk_widget_get_allocated_width/height` | gwin.c, winman.c, focustest.c | Medium |
| `gdk_screen_get_n_monitors` | `gdk_display_get_n_monitors` + `gdk_monitor_get_geometry` | gwin.c, gmain.c | Medium |
| `gdk_screen_width/height` | `gdk_monitor_get_geometry` | gmain.c, winman.c, focustest.c | Medium |

---

## Migration Strategy

### Phase 1: Preparation & Build System (Day 1-2)

#### 1.1 Update Build Configuration
```makefile
# Makefile changes
# Change from:
GTKFLAGS := $(shell pkg-config --cflags gtk+-2.0 gdk-2.0)
GLIBS := $(shell pkg-config --libs gtk+-2.0 gdk-2.0)

# To:
GTKFLAGS := $(shell pkg-config --cflags gtk+-3.0)
GLIBS := $(shell pkg-config --libs gtk+-3.0)
```

#### 1.2 Update Header Includes
```c
// pho.h and all .c files
// Change from:
#include <gtk/gtk.h>
#include <gdk/gdk.h>

// To (GTK3 unifies headers):
#include <gtk/gtk.h>
```

#### 1.3 Fix Deprecation Warnings
Add to Makefile temporarily:
```makefile
CFLAGS += -DGDK_DISABLE_DEPRECATED -DGTK_DISABLE_DEPRECATED
```

**Testing**: Verify compilation fails with clear error messages identifying deprecated APIs.

---

### Phase 2: Signal Connection Updates (Day 2-3)

#### 2.1 Replace gtk_signal_connect with g_signal_connect

**Pattern**:
```c
// GTK2 (current):
gtk_signal_connect(GTK_OBJECT(gWin), "delete_event",
                   GTK_SIGNAL_FUNC(HandleDelete), NULL);

// GTK3:
g_signal_connect(G_OBJECT(gWin), "delete-event",
                 G_CALLBACK(HandleDelete), NULL);
```

#### 2.2 Files to Modify
- `gwin.c`: ~8 signal connections (lines 671-719)
- `gdialogs.c`: ~4 signal connections (lines 185-468)
- `keydialog.c`: ~3 signal connections (lines 193-249)

**Testing**: Run `make` and verify no signal-related warnings.

---

### Phase 3: Widget & Window Accessors (Day 3-4)

#### 3.1 Replace Direct Struct Access

**Pattern**:
```c
// GTK2 (current):
gdk_drawable_get_size(gWin->window, &width, &height);
gdk_window_clear(sDrawingArea->window);

// GTK3:
gdk_drawable_get_size(gtk_widget_get_window(gWin), &width, &height);
GdkWindow *window = gtk_widget_get_window(sDrawingArea);
gdk_window_clear(window);
```

#### 3.2 Critical Locations
- `gwin.c:111, 313, 422, 471, 537, 784, 785, 825`
- `winman.c:95, 170`
- `focustest.c:55`

**Testing**: Verify window sizing and display modes work correctly.

---

### Phase 4: Drawing System Migration (Day 5-10) - **CRITICAL**

#### 4.1 Replace gdk_pixbuf_render_to_drawable with Cairo

**Current GTK2 code** (`gwin.c:422`):
```c
gdk_pixbuf_render_to_drawable(gImage, sDrawingArea->window,
                              sDrawingArea->style->fg_gc[GTK_STATE_NORMAL],
                              0, 0, x, y, width, height,
                              GDK_RGB_DITHER_NONE, 0, 0);
```

**GTK3 Replacement**:
```c
// Use cairo surface for drawing
cairo_t *cr = gdk_cairo_create(gtk_widget_get_window(sDrawingArea));
gdk_cairo_set_source_pixbuf(cr, gImage, x, y);
cairo_paint(cr);
cairo_destroy(cr);
```

#### 4.2 Implement Draw Callback

Replace `expose_event` with `draw` signal:

```c
// GTK2:
g_signal_connect(G_OBJECT(sDrawingArea), "expose_event",
                 G_CALLBACK(HandleExpose), NULL);

// GTK3:
g_signal_connect(G_OBJECT(sDrawingArea), "draw",
                 G_CALLBACK(HandleDraw), NULL);
```

#### 4.3 Handle Draw Callback Signature Change

```c
// GTK2:
static gint HandleExpose(GtkWidget* widget, GdkEventExpose* event)
{
    // Use event->area for clip region
}

// GTK3:
static gboolean HandleDraw(GtkWidget* widget, cairo_t *cr)
{
    // Use cr directly for drawing
    // Cairo handles clip region
}
```

#### 4.4 Background Clearing

```c
// GTK2:
gdk_window_clear(sDrawingArea->window);

// GTK3:
cairo_t *cr = gdk_cairo_create(gtk_widget_get_window(sDrawingArea));
cairo_set_source_rgb(cr, 0, 0, 0);  // Black background
cairo_paint(cr);
cairo_destroy(cr);
```

**Testing**: 
- Test all display modes (normal, fullscreen, presentation)
- Verify image scaling and rotation display correctly
- Check performance on large images

---

### Phase 5: Monitor & Screen API Updates (Day 10-12)

#### 5.1 Replace gdk_screen_* functions

**Multi-monitor support** (`gwin.c:629-647`):

```c
// GTK2:
GdkScreen* screen = gdk_drawable_get_screen(gWin->window);
gint nMonitors = gdk_screen_get_n_monitors(screen);
gdk_screen_get_monitor_geometry(screen, i, &rect);

// GTK3:
GdkDisplay *display = gtk_widget_get_display(gWin);
GdkMonitor *monitor = gdk_display_get_monitor(display, i);
gdk_monitor_get_geometry(monitor, &rect);
int nMonitors = gdk_display_get_n_monitors(display);
```

#### 5.2 Replace gdk_screen_width/height

```c
// GTK2:
gMonitorWidth = gdk_screen_width();
gMonitorHeight = gdk_screen_height();

// GTK3:
GdkDisplay *display = gdk_display_get_default();
GdkMonitor *monitor = gdk_display_get_primary_monitor(display);
GdkRectangle geometry;
gdk_monitor_get_geometry(monitor, &geometry);
gMonitorWidth = geometry.width;
gMonitorHeight = geometry.height;
```

**Testing**: Test on multi-monitor setups, verify fullscreen on correct monitor.

---

### Phase 6: Styling & Theming (Day 12-14)

#### 6.1 Replace gtk_widget_modify_bg

**Current** (`gwin.c:693`):
```c
gtk_widget_modify_bg(sDrawingArea, GTK_STATE_NORMAL, &sBlack);
```

**GTK3 Options**:

Option 1 - CSS (recommended):
```c
GtkCssProvider *provider = gtk_css_provider_new();
gtk_css_provider_load_from_data(provider,
    "drawingarea { background-color: black; }", -1, NULL);
GtkStyleContext *context = gtk_widget_get_style_context(sDrawingArea);
gtk_style_context_add_provider(context,
    GTK_STYLE_PROVIDER(provider),
    GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
```

Option 2 - Override draw handler:
```c
// In draw handler, before rendering image:
cairo_set_source_rgb(cr, 0, 0, 0);
cairo_paint(cr);
```

**Testing**: Verify black background in presentation mode.

---

### Phase 7: Final Cleanup & Testing (Day 14-18)

#### 7.1 Fix Remaining Deprecations
- `gtk_drawing_area_size` → `gtk_widget_set_size_request`
- `gtk_object_destroy` → `gtk_widget_destroy`
- Update any remaining GTK2-specific types

#### 7.2 Comprehensive Testing Checklist

| Feature | Test Command | Expected Result |
|---------|--------------|-----------------|
| Basic display | `./pho test-img/*.jpg` | Images display |
| Fullscreen | Press `F` key | Fullscreen mode |
| Presentation | `./pho -p test-img/*.jpg` | Fullscreen centered |
| Slideshow | `./pho -s2 test-img/*.jpg` | Auto-advance |
| Multi-monitor | Move window, test `-m1` | Correct monitor |
| Rotation | Press `r` key | Image rotates |
| Scaling | Press `+/-` keys | Image scales |
| Delete | Press `d` key | File deleted |
| Keywords | Press `k` key | Keywords dialog |
| Info | Press `i` key | Info dialog |

#### 7.3 Platform Testing
- [ ] macOS ARM64 (primary)
- [ ] macOS Intel
- [ ] Linux (Ubuntu/Debian)
- [ ] Linux (Fedora)

---

## Risk Mitigation

### High-Risk Areas

1. **Cairo Drawing Performance**
   - **Risk**: Cairo may be slower than GDK direct rendering
   - **Mitigation**: Profile on large images, consider surface caching

2. **Multi-monitor on macOS**
   - **Risk**: macOS multi-monitor handling changed in GTK3
   - **Mitigation**: Extensive testing with `-m` flag

3. **Window Manager Compatibility**
   - **Risk**: Different WMs handle GTK3 windows differently
   - **Mitigation**: Test on GNOME, KDE, XFCE, and macOS

### Rollback Plan

Keep `gtk2-branch` available:
```bash
git checkout -b gtk2-legacy-backup master
git checkout master
# Proceed with GTK3 migration
```

---

## File-by-File Migration Order

### Priority 1: Core UI (Week 1)
1. `gwin.c` - Main window (most complex)
2. `gdialogs.c` - Info dialog
3. `keydialog.c` - Keywords dialog

### Priority 2: Main & Support (Week 2)
4. `gmain.c` - Initialization
5. `pho.c` - Image handling (mostly GDK pixbuf, less changes)
6. `winman.c` - Window management
7. `focustest.c` - Testing utility

### Priority 3: Build & Polish (Week 3)
8. `Makefile` - Build system
9. `pho.h` - Headers
10. `tests/` - Test compilation

---

## Resources

### GTK3 Documentation
- [GTK3 Reference Manual](https://docs.gtk.org/gtk3/)
- [GDK3 Reference](https://docs.gtk.org/gdk3/)
- [Cairo Tutorial](https://www.cairographics.org/tutorial/)

### Migration Guides
- [GNOME GTK3 Migration Guide](https://developer.gnome.org/gtk3/stable/gtk-migrating-2-to-3.html)
- [PyGObject GTK2→3 Guide](https://pygobject.readthedocs.io/en/latest/guide/porting.html) (concepts apply to C)

### Key Differences Summary
- No more `GdkDrawable` - use Cairo
- No more `expose_event` - use `draw`
- No direct widget struct access - use accessors
- CSS-based theming instead of direct style modification

---

## Success Criteria

- [ ] Compiles without GTK2 deprecation warnings
- [ ] All 64 existing tests pass
- [ ] Manual testing checklist complete
- [ ] Multi-monitor support verified
- [ ] Performance comparable to GTK2 version
- [ ] Documentation updated

---

## Handover Notes

### For Next Agent

**Start Here**: Phase 1 - Update Makefile and verify compilation errors

**Expected Challenges**:
1. `gwin.c` drawing code (most complex changes)
2. Multi-monitor API changes
3. Testing across platforms

**Key Files to Understand First**:
1. `gwin.c:400-450` - Image drawing (cairo migration needed)
2. `gwin.c:620-670` - Monitor detection
3. `gmain.c:465-485` - Initialization

**Testing Strategy**:
- Make incremental changes
- Test after each file modification
- Use `make clean && make` frequently
- Keep console open for GTK warnings

---

*Created: 2026-02-15*  
*Next Review: After Phase 1 completion*
