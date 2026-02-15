/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * gwin.c: gtk window management routines for pho, an image viewer.
 *
 * Copyright 2004,2009 by Akkana Peck.
 * You are free to use or modify this code under the Gnu Public License.
 */

#include "pho.h"
#include "dialogs.h"
#include "exif/phoexif.h"

#include <stdio.h>
#include <stdlib.h>       /* for exit() */
#include <string.h>
#include <unistd.h>

#include <gdk/gdk.h>
#include <gdk/gdkscreen.h>

/* Some window managers don't deal well with windows that resize,
 * or don't retain focus if a resized window no longer contains
 * the mouse pointer.
 * Offer an option to make new windows instead.
 */
int gMakeNewWindows = 0;

/* Which monitor to use. If negative, don't specify,
 * let the window manager decide.
 */
int gUseMonitor = -1;

/* The size our window frame adds on the top left of the image */
gint sFrameWidth = 10;
gint sFrameHeight = 28;
int sHaveFrameSize = 0;

gint gPhysMonitorWidth = 0;
gint gPhysMonitorHeight = 0;

int sDragOffsetX = 0;
int sDragOffsetY = 0;
int sDragStartX = 0;
int sDragStartY = 0;

/* gtk related window attributes */
GtkWidget *gWin = 0;
static GtkWidget *sDrawingArea = 0;

/* This is so gross. GTK has no way to tell if a window has been exposed.
 * GTK_WIDGET_MAPPED and GTK_WIDGET_VISIBLE are both true before the
 * main loop has run or an expose event has happened.
 */
static int sExposed = 0;

/* forward definitions */
static void NewWindow();
static void MoveWin2Monitor(int whichmon, int x, int y);

static void hide_cursor(GtkWidget* w)
{
    static GdkCursor* cursor = 0;

    if (cursor == 0) {
        /* GTK3: Create invisible cursor using Cairo surface */
        cairo_surface_t *surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 1, 1);
        cairo_t *cr = cairo_create(surface);
        cairo_set_source_rgba(cr, 0, 0, 0, 0);  /* Transparent */
        cairo_paint(cr);
        cursor = gdk_cursor_new_from_surface(gdk_display_get_default(), surface, 0, 0);
        cairo_destroy(cr);
        cairo_surface_destroy(surface);
    }

    GdkWindow *window = gtk_widget_get_window(w);
    if (window != NULL)
        gdk_window_set_cursor(window, cursor);
}

static void show_cursor(GtkWidget* w)
{
    gdk_window_set_cursor(gtk_widget_get_window(w), NULL);
}

/*
 * Adjust gMonitorWidth/Height and sFrameWidth/Height
 * according to the display mode and current frames,
 * then resize the image accordingly.
 * Call this when the image size changes --
 * zoom, shift in/out of presentation mode, etc.
 */
static void AdjustScreenSize()
{
    if (gDisplayMode == PHO_DISPLAY_PRESENTATION) {
        gMonitorWidth = gPhysMonitorWidth;
        gMonitorHeight = gPhysMonitorHeight;
    }
    else {
        if (!sHaveFrameSize && gtk_widget_get_mapped(gWin)) {
            gint width, height;
            GdkRectangle rect;

            if (gDebug)
                printf("AdjustScreenSize: Requesting frame size\n");
            width = gtk_widget_get_allocated_width(gWin);
            height = gtk_widget_get_allocated_height(gWin);
            gdk_window_get_frame_extents(gtk_widget_get_window(gWin), &rect);
            sFrameWidth = rect.width - width;
            sFrameHeight = rect.height - height;
            sHaveFrameSize = 1;
        }

        if (gDebug)
            printf("AdjustScreenSize: Frame size: %d, %d\n",
                   sFrameWidth, sFrameHeight);

        gMonitorWidth = gPhysMonitorWidth - sFrameWidth;
        gMonitorHeight = gPhysMonitorHeight - sFrameHeight;
    }

    ScaleAndRotate(gCurImage, 0);
}

static void CenterWindow(GtkWidget* win)
{
    gint w, h;
    if (gDebug) printf("CenterWindow\n");
    gtk_window_get_size(GTK_WINDOW(gWin), &w, &h);
    gtk_window_set_gravity(GTK_WINDOW(win), GDK_GRAVITY_CENTER);
    gtk_window_move(GTK_WINDOW(win),
                    (gMonitorWidth - gCurImage->curWidth)/2,
                    (gMonitorHeight - gCurImage->curHeight)/2);
    gtk_window_set_gravity(GTK_WINDOW(win), GDK_GRAVITY_NORTH_WEST);
}

/* If we  resized, see if we might move off the screen.
 * Make sure we don't do that, assuming the image is still
 * small enough to fit. Then request focus from the window manager.
 * Try to move the window as little as possible.
 */
static void MaybeMove()
{
    gint x, y, w, h, nx, ny;

    if (gDebug) printf("MaybeMove\n");
    if (gCurImage->curWidth > gPhysMonitorWidth
        || gCurImage->curHeight > gPhysMonitorHeight)
        return;

    /* If we don't have a window yet, don't move it */
    if (!gWin || !gtk_widget_get_mapped(gWin))
        return;

    /* If we're in presentation or keywords mode, never move the window */
    if (gDisplayMode == PHO_DISPLAY_PRESENTATION
        || gDisplayMode == PHO_DISPLAY_KEYWORDS)
        return;

    gtk_window_get_position(GTK_WINDOW(gWin), &x, &y);
    nx = x;  ny = y;
    gtk_window_get_size(GTK_WINDOW(gWin), &w, &h);
    /* printf("Currently (%d, %d) %d x %d\n", x, y, w, h); */

    /* See if it would overflow off the screen */
    if (x + gCurImage->curWidth > gMonitorWidth)
        nx = gMonitorWidth - gCurImage->curWidth;
    if (nx < 0)
        nx = 0;
    if (y + gCurImage->curHeight > gMonitorHeight)
        ny = gMonitorHeight - gCurImage->curHeight;
    if (ny < 0)
        ny = 0;

    if (x != nx || y != ny) {
        gtk_window_move(GTK_WINDOW(gWin), nx, ny);
    }

    /* Request focus from the window manager.
     * This is pretty much a no-op, but what the heck.
     * However, it also messes with NewWindow's ability
     * to get the frame size because it maps the window before
     * we call gtk_widget_show().
    gtk_window_present(GTK_WINDOW(gWin));
     */
}

double FracOfScreenSize() {
    if (gMonitorWidth == 0 || gMonitorHeight == 0)
        return 0.0;    /* Signal to re-call this function later */

    if (gMonitorWidth > gMonitorHeight)
        return gMonitorHeight * .95;
    return gMonitorWidth * .95;
}

/* Change the view (display and scale) modes,
 * and re-display the image if necessary.
 */
/* XXX Pho really needs a state table defining which commands
 * take which state to which new state! What a mess.
 */
int SetViewModes(int dispmode, int scalemode, double scalefactor)
{
    sDragOffsetX = sDragOffsetY = 0;

    if (dispmode == gDisplayMode && scalemode == gScaleMode
        && scalefactor == gScaleRatio)
        return 0;
    if (gDebug)
        printf("SetViewModes(%d, %d, %f (was %d, %d, %f)\n",
               dispmode, scalemode, scalefactor,
               gDisplayMode, gScaleMode, gScaleRatio);

    if (dispmode == PHO_DISPLAY_KEYWORDS) {
        if (gDisplayMode != PHO_DISPLAY_KEYWORDS) {
            /* switching to keyword mode from some other mode */
            gScaleMode = PHO_SCALE_FIXED;
            gScaleRatio = FracOfScreenSize();
            /*
            if (gDebug)
                printf("Showing keywords dialog from SetViewModes\n");
            ShowKeywordsDialog();
             */
        }
        else {
            /* staying in keywords mode but changing some other factor:
             * don't force anything.
             */
            gScaleMode = scalemode;
            gScaleRatio = scalefactor;
        }
    }
    else {
        if (gDisplayMode == PHO_DISPLAY_KEYWORDS)  /* leaving keywords mode */
            HideKeywordsDialog();
        gScaleMode = scalemode;
        gScaleRatio = scalefactor;
    }

    if (dispmode != gDisplayMode) {
        gDisplayMode = dispmode;
        if (gWin && sDrawingArea && gtk_widget_get_mapped(gWin))
            /* Changing an existing window */
        {
            if (dispmode == PHO_DISPLAY_PRESENTATION) {
                if (gDebug)
                    printf("\nSetViewModes: changing to presentation mode\n");
                hide_cursor(sDrawingArea);
                gtk_window_fullscreen(GTK_WINDOW(gWin));
                gtk_window_move(GTK_WINDOW(gWin), 0, 0);
                /* Is it still necessary to move a fullscreen window? */

                AdjustScreenSize();
            }
            else {
                if (gDebug)
                    printf("\nSetViewModes: changing to normal mode\n");
                show_cursor(sDrawingArea);
                gtk_window_unfullscreen(GTK_WINDOW(gWin));
                AdjustScreenSize();
                if (gDebug)
                    printf("Monitor size after AdjustScreenSize: %dx%d\n",
                           gMonitorWidth, gMonitorHeight);
                CenterWindow(gWin);
            }
        }
    }
    else {
        /* If we're not changing the display mode, then we're changing
         * the scale mode or scale factor, and may need to move to
         * keep the window under the mouse so as not to lose focus
         * or move off the screen.
         * XXX unfortunately this doesn't work when changing from keywords
         * to fullscreen/normal.
         */
        int ret = ScaleAndRotate(gCurImage, 0);
        if (ret != 0) return ret;
        MaybeMove();
    }

    if (gDisplayMode == PHO_DISPLAY_KEYWORDS) {
        ShowKeywordsDialog();
    }

    return 0;
}

/* DrawImage is called from the expose callback.
 * It assumes we already have the image in gImage.
 */
void DrawImage()
{
    int dstX = 0, dstY = 0;
    char title[BUFSIZ];
#   define TITLELEN ((sizeof title) / (sizeof *title))

    if (gDebug) {
        printf("DrawImage %s, %dx%d\n", gCurImage->filename,
               gCurImage->curWidth, gCurImage->curHeight);
    }

    if (gImage == 0 || gWin == 0 || sDrawingArea == 0) return;
    if (!sExposed) return;
    if (!gtk_widget_get_mapped(gWin)) return;

    if (gDisplayMode == PHO_DISPLAY_PRESENTATION) {
        gint width, height;
        /* GTK3: Use cairo to clear window background (via drawing frame API) */
        GdkWindow *window = gtk_widget_get_window(sDrawingArea);
        GdkDrawingContext *context = gdk_window_begin_draw_frame(window, NULL);
        cairo_t *cr = gdk_drawing_context_get_cairo_context(context);
        cairo_set_source_rgb(cr, 0, 0, 0);
        cairo_paint(cr);
        gdk_window_end_draw_frame(window, context);

        /* Center the image. This has to be done according to
         * the current window size, not the phys monitor size,
         * because in the xinerama case, gtk_window_fullscreen()
         * only fullscreens the current monitor, not all of them.
         */
        gtk_window_get_size(GTK_WINDOW(gWin), &width, &height);

        /* If we have a presentation screen size set (e.g. for a projector
         * that has a different resolution from our native monitor),
         * Fudge the screen size and center based on a virtual screen
         * starting in the upper left corner of our current screen.
         * That way, it will center on the projector or other device.
         */
        if (gPresentationWidth > 0)
            width = gPresentationWidth;
        if (gPresentationHeight > 0)
            height = gPresentationHeight;

        dstX = (width - gCurImage->curWidth) / 2 + sDragOffsetX;
        dstY = (height - gCurImage->curHeight) / 2 + sDragOffsetY;

        /* But we probably shouldn't allow dragging the image
         * completely off the screen -- just drag to the point where
         * a corner is visible.
         */
        /* Left edge */
        if (gCurImage->curWidth > gMonitorWidth
            && dstX < gMonitorWidth - gCurImage->curWidth)
            dstX = gMonitorWidth - gCurImage->curWidth;
        else if (gCurImage->curWidth <= gMonitorWidth && dstX <= 0)
            dstX = 0;

        /* Top edge */
        if (gCurImage->curHeight > gMonitorHeight
            && dstY < gMonitorHeight - gCurImage->curHeight)
            dstY = gMonitorHeight - gCurImage->curHeight;
        else if (gCurImage->curHeight <= gMonitorHeight && dstY <= 0)
            dstY = 0;

        /* Right edge */
        if (gCurImage->curWidth < gMonitorWidth
            && dstX > gMonitorWidth - gCurImage->curWidth)
            dstX = gMonitorWidth - gCurImage->curWidth;
        else if (gCurImage->curWidth >= gMonitorWidth
                 && dstX > 0)
            dstX = 0;

        /* Bottom edge */
        if (gCurImage->curHeight < gMonitorHeight
            && dstY > gMonitorHeight - gCurImage->curHeight)
            dstY = gMonitorHeight - gCurImage->curHeight;
        else if (gCurImage->curHeight >= gMonitorHeight
                 && dstY > 0)
            dstY = 0;

        /* XXX Would be good to reset sDragOffsetX and sDragOffsetY
         * in these cases so they don't get crazily out of kilter.
         */
    }
    else {
        /* Update the titlebar */
        snprintf(title, sizeof(title), "pho: %s (%d x %d)", gCurImage->filename,
                gCurImage->trueWidth, gCurImage->trueHeight);
        if (HasExif())
        {
            const char* date = ExifGetString(ExifDate);
            if (date && date[0]) {
                /* Safely append date if there's room */
                size_t current_len = strlen(title);
                size_t remaining = sizeof(title) - current_len - 1;
                if (remaining >= strlen(date) + 3) {
                    strncat(title, " (", remaining);
                    strncat(title, date, remaining - 2);
                    strncat(title, ")", remaining - 2 - strlen(date));
                }
            }
        }
        /* Safely append scale mode info */
        size_t title_remaining = sizeof(title) - strlen(title) - 1;
        if (gScaleMode == PHO_SCALE_FULLSIZE)
            strncat(title, " (fullsize)", title_remaining);
        else if (gScaleMode == PHO_SCALE_FULLSCREEN)
            strncat(title, " (fullscreen)", title_remaining);
        else if (gScaleMode == PHO_SCALE_IMG_RATIO ||
                 gScaleMode == PHO_SCALE_SCREEN_RATIO) {
            char* titleEnd = title + strlen(title);
            size_t remaining = sizeof(title) - (titleEnd - title) - 1;
            if (gScaleRatio < 1)
                snprintf(titleEnd, remaining + 1, " [%s/ %d]",
                        (gScaleMode == PHO_SCALE_IMG_RATIO ? "fullsize " : ""),
                        (int)(1. / gScaleRatio));
            else
                snprintf(titleEnd, remaining + 1, " [%s* %d]",
                        (gScaleMode == PHO_SCALE_IMG_RATIO ? "fullsize " : ""),
                        (int)gScaleRatio);
        }
        else if (gScaleMode == PHO_SCALE_FIXED)
            strncat(title, " (fixed)", title_remaining);
        gtk_window_set_title(GTK_WINDOW(gWin), title);

        if (gDisplayMode == PHO_DISPLAY_KEYWORDS) {
            if (gDebug)
                printf("Showing keywords dialog from DrawImage\n");
            ShowKeywordsDialog(gCurImage);
        }
    }

    /* GTK3: Use Cairo for drawing (via drawing frame API) */
    GdkWindow *draw_window = gtk_widget_get_window(sDrawingArea);
    GdkDrawingContext *draw_context = gdk_window_begin_draw_frame(draw_window, NULL);
    cairo_t *cr = gdk_drawing_context_get_cairo_context(draw_context);
    gdk_cairo_set_source_pixbuf(cr, gImage, dstX, dstY);
    cairo_paint(cr);
    gdk_window_end_draw_frame(draw_window, draw_context);

    UpdateInfoDialog(gCurImage);
}

static gboolean
HandlePress(GtkWidget *widget, GdkEventButton *event)
{
    if (event->button != 2 )
        return TRUE;

    /*  grab with owner_events == TRUE so the popup's widgets can
     *  receive events. we filter away events outside this toplevel
     *  away in button_press()
     *  GTK3: Use gdk_device_grab instead of gdk_pointer_grab
     */
    GdkDevice *device = gdk_event_get_device((GdkEvent*)event);
    if (gdk_device_grab (device, gtk_widget_get_window(widget), GDK_OWNERSHIP_WINDOW, TRUE,
                          GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK
                          | GDK_POINTER_MOTION_MASK
                          | GDK_POINTER_MOTION_HINT_MASK,
                          NULL, GDK_CURRENT_TIME) != GDK_GRAB_SUCCESS)
        printf("Couldn't grab!\n");
    sDragStartX = event->x;
    sDragStartY = event->y;

    return TRUE;
}

static gboolean
HandleRelease(GtkWidget *widget, GdkEventButton *event)
{
    if (event->button != 2 )
        return TRUE;

    /* Ungrab, stop listening for motion */
    /* GTK3: Use gdk_device_ungrab instead of gdk_display_pointer_ungrab */
    GdkDevice *device = gdk_event_get_device((GdkEvent*)event);
    gdk_device_ungrab(device, GDK_CURRENT_TIME);
    return TRUE;
}

static gboolean
HandleMotionNotify(GtkWidget *widget, GdkEventMotion *event)
{
#define DRAGRESTART 3
    int x, y;
    GdkModifierType state;
    /* GTK3: Use gdk_window_get_device_position instead */
    GdkDevice *device = gdk_event_get_device((GdkEvent*)event);
    gdk_window_get_device_position(gtk_widget_get_window(widget), device, &x, &y, &state);

    if (state & GDK_BUTTON2_MASK) {
        sDragOffsetX += x - sDragStartX;
        sDragOffsetY += y - sDragStartY;
        /* Drag offsets will get sanity checked when we show the image,
         * to disallow dragging the image entirely off the screen.
         */

        /* We've handled this drag, so the next motion event
         * should start from here.
         */
        sDragStartX = x;
        sDragStartY = y;

        /* Assuming we're definitely in presentation mode,
         * the user probably can't see the pointer, and it's confusing
         * and frustrating when the mouse hits the edge of the screen
         * and it's not obvious what to do about it.
         * So when that happens, warp it to the opposite side of the screen.
         */
        if (gDisplayMode == PHO_DISPLAY_PRESENTATION) {
            int warpToX = -1;
            int warpToY = -1;
            if (x >= gMonitorWidth-1) {
                warpToX = DRAGRESTART;
                warpToY = y;
            }
            else if (x <= 1) {
                warpToX = gMonitorWidth-DRAGRESTART;
                warpToY = y;
            }
            if (y >= gMonitorHeight-1) {
                warpToX = x;
                warpToY = DRAGRESTART;
            }
            else if (y <= 1) {
                warpToX = x;
                warpToY = gMonitorHeight-DRAGRESTART;
            }
            /* If we've hit an edge of the screen, warp to the opposite edge */
            if (warpToX >= 0 && warpToY >= 0) {
                /* GTK3: Use gdk_device_warp instead of gdk_display_warp_pointer */
                GdkDevice *pointer_device = gdk_event_get_device((GdkEvent*)event);
                gdk_device_warp(pointer_device, gtk_widget_get_screen(sDrawingArea),
                                warpToX, warpToY);
                sDragStartX = warpToX;
                sDragStartY = warpToY;
            }
        }

        /* This flickers: would be nice to collapse events more */
        DrawImage();
    }

    return TRUE;
}

/* GTK3: "draw" signal handler replaces "expose_event"
 * The draw signal provides a cairo_t for rendering.
 */
static gboolean HandleExpose(GtkWidget* widget, cairo_t *cr)
{
    gint width, height;

    sExposed = 1;
    width = gtk_widget_get_allocated_width(widget);
    height = gtk_widget_get_allocated_height(widget);
    if (gDebug) {
        cairo_rectangle_int_t clip_rect;
        gdk_cairo_get_clip_rectangle(cr, (GdkRectangle*)&clip_rect);
        printf("HandleExpose: clip %dx%d +%d+%d in window %dx%d\n",
               clip_rect.width, clip_rect.height,
               clip_rect.x, clip_rect.y,
               width, height);
    }

    /* If a specific monitor was specified, move the window now.
     * Unfortunately that means it briefly appears on the default monitor,
     * then moves to the specified one. I haven't found any way to
     * get monitor information before the first window is exposed;
     * the map and configure events are both too early
     * (configure works for presentation mode but not normal mode).
     */
    if (gUseMonitor >= 0) {
        gint root_x, root_y;
        gdk_window_get_position(gtk_widget_get_window(gWin), &root_x, &root_y);
        /*
        printf("HandleExpose: moving to monitor %d\n", gUseMonitor);
        MoveWin2Monitor(gUseMonitor, root_x, root_y);
        */
    }

    /* Make sure the window can resize smaller */
    if (!gMakeNewWindows)
        gtk_widget_set_size_request(GTK_WIDGET(gWin), 1, 1);

    if (!sHaveFrameSize && gDisplayMode != PHO_DISPLAY_PRESENTATION) {
        int oldw = sFrameWidth;
        int oldh = sFrameHeight;
        AdjustScreenSize();
        if (sFrameWidth != oldw || sFrameHeight != oldh) {
            gtk_window_resize(GTK_WINDOW(gWin),
                              gCurImage->curWidth, gCurImage->curHeight);
            /* Since we resized, we might no longer be over the cursor
             * and may have lost focus:
             */
            MaybeMove();
        }
    }

    DrawImage();

    return TRUE;
}

void EndSession()
{
    gCurImage = 0;
    UpdateInfoDialog();
    RememberKeywords();
    PrintNotes();
    /* Ensure all pending GTK events are processed before quitting */
    while (gtk_events_pending())
        gtk_main_iteration();
    gtk_main_quit();
    /* Exit is a fallback in case gtk_main_quit() doesn't terminate
     * (can happen with some window managers). Most cleanup is done above.
     */
    exit(0);
}

static gint HandleDelete(GtkWidget* widget, GdkEventKey* event, gpointer data)
{
    /* If you return FALSE in the "delete_event" signal handler,
     * GTK will emit the "destroy" signal. Returning TRUE means
     * you don't want the window to be destroyed.
     * This is useful for popping up 'are you sure you want to quit?'
     * type dialogs. */
    EndSession();
    return TRUE; /* Never called -- just here to keep gcc happy. */
}

/* Move to a specific monitor */
void MoveWin2Monitor(int whichmon, int x, int y)
{
    if (!gtk_widget_get_realized(gWin)) {
        if (gDebug)
            printf("MoveWin2Monitor: no window yet, can't set monitor\n");
        return;
    }

    /* GTK3: Use GdkDisplay and GdkMonitor instead of GdkScreen */
    GdkDisplay *display = gtk_widget_get_display(gWin);
    gint nMonitors = gdk_display_get_n_monitors(display);
    GdkRectangle rect;
    if (gDebug) {
        printf("Found %d monitors:\n", nMonitors);
        for (gint i=0; i < nMonitors; ++i) {
            GdkMonitor *monitor = gdk_display_get_monitor(display, i);
            gdk_monitor_get_geometry(monitor, &rect);
            printf("  %d x %d (%d, %d)\n",
                   rect.width, rect.height, rect.x, rect.y);
        }
    }
    if (whichmon >= nMonitors) {
        gtk_window_move(GTK_WINDOW(gWin), x, y);
        return;
    }
    GdkMonitor *target_monitor = gdk_display_get_monitor(display, whichmon);
    gdk_monitor_get_geometry(target_monitor, &rect);
    if (gDebug)
        printf("Moving to monitor %d\n", nMonitors);
    gtk_window_move(GTK_WINDOW(gWin), rect.x + x, rect.y + y);
}

/* Make a new window, destroying the old one. */
static void NewWindow()
{
    gint root_x = 0;
    gint root_y = 0;

    sExposed = 0;    /* reset the exposed flag */

    if (gDebug)
        printf("NewWindow()\n");

    if (gWin) {
        gdk_window_get_position(gtk_widget_get_window(gWin), &root_x, &root_y);
        gtk_widget_destroy(gWin);
    }

    gWin = gtk_window_new(GTK_WINDOW_TOPLEVEL);

    /* GTK3: gtk_window_set_wmclass is deprecated with no replacement.
     * The WM_CLASS is set automatically from the application name.
     */

    /* Window manager delete */
    g_signal_connect(G_OBJECT(gWin), "delete-event",
                       G_CALLBACK(HandleDelete), 0);

    /* This event occurs when we call gtk_widget_destroy() on the window,
     * or if we return FALSE in the "delete_event" callback.
    gtk_signal_connect(GTK_OBJECT(gWin), "destroy",
                       (GtkSignalFunc)HandleDestroy, 0);
     */

    /* KeyPress events on the drawing area don't come through --
     * they have to be on the window.
     */
    g_signal_connect(G_OBJECT(gWin), "key-press-event",
                       G_CALLBACK(HandleGlobalKeys), 0);

    sDrawingArea = gtk_drawing_area_new();
    gtk_container_add(GTK_CONTAINER(gWin), sDrawingArea);
    gtk_widget_show(sDrawingArea);

    /* GTK3: Use CSS instead of gtk_widget_modify_bg */
    GtkCssProvider *css_provider = gtk_css_provider_new();
    gtk_css_provider_load_from_data(css_provider,
        "drawingarea { background-color: #000000; }", -1, NULL);
    gtk_style_context_add_provider(
        gtk_widget_get_style_context(sDrawingArea),
        GTK_STYLE_PROVIDER(css_provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

    /* We hope this has already been done: LoadImageAndRotate
     * should have called ScaleAndRotate when the image was loaded.
    AdjustScreenSize();
     */
    if (gDisplayMode == PHO_DISPLAY_PRESENTATION) {
        gtk_widget_set_size_request(sDrawingArea,
                              gPhysMonitorWidth, gPhysMonitorHeight);
        gtk_window_fullscreen(GTK_WINDOW(gWin));

        /* Listen for middle clicks to drag position: */
        gtk_widget_set_events(sDrawingArea, GDK_BUTTON_PRESS_MASK);
        g_signal_connect(G_OBJECT(sDrawingArea), "button-press-event",
                           G_CALLBACK(HandlePress), 0);
        g_signal_connect(G_OBJECT(sDrawingArea), "button-release-event",
                           G_CALLBACK(HandleRelease), 0);
        g_signal_connect(G_OBJECT(sDrawingArea), "motion-notify-event",
                           G_CALLBACK(HandleMotionNotify), 0);
    }
    else {
        gtk_widget_set_size_request(sDrawingArea,
                              gCurImage->curWidth, gCurImage->curHeight);
        gtk_window_unfullscreen(GTK_WINDOW(gWin));
    }

    g_signal_connect(G_OBJECT(sDrawingArea), "draw",
                       G_CALLBACK(HandleExpose), 0);
    /* To track in/out of fullscreen mode, use configure_event
     * or window_state_event.
     */

    gtk_widget_show(gWin);

    /* Must come after show(), hide_cursor needs a window */
    if (gDisplayMode == PHO_DISPLAY_PRESENTATION)
        hide_cursor(sDrawingArea);

    /* Hopefully gWin->window exists by now, so it's safe
     * to place it on the intended monitor.
     */
    if (gUseMonitor >= 0) {
        MoveWin2Monitor(gUseMonitor, root_x, root_y);
        // printf("NewWindow: moving to monitor %d\n", gUseMonitor);
    }
}

/**
 * gdk_window_focus:
 * @window: a #GdkWindow
 * @timestamp: timestamp of the event triggering the window focus
 *
 * Sets keyboard focus to @window. If @window is not onscreen this
 * will not work. In most cases, gtk_window_present() should be used on
 * a #GtkWindow, rather than calling this function.
 *
 * For Pho: this is a replacement for gdk_window_focus
 * due to the issue in http://bugzilla.gnome.org/show_bug.cgi?id=150668
 *
 **/

/* PrepareWindow is responsible for making the window the right
 * size and position, so the user doesn't see flickering.
 * It may actually make a new window, or it may just resize and/or
 * reposition the existing window.
 */
void PrepareWindow()
{
    if (gMakeNewWindows || gWin == 0) {
        NewWindow();
        return;
    }

    sDragOffsetX = sDragOffsetY = 0;

    /* If the window is new but hasn't been mapped yet,
     * there's nothing we can do from here.
     */
    if (!gtk_widget_get_mapped(gWin))
        return;

    /* Otherwise, resize and reposition the current window. */

    if (gDisplayMode == PHO_DISPLAY_PRESENTATION) {
        /* XXX shouldn't have to do this every time */
        gtk_widget_set_size_request(sDrawingArea,
                              gPhysMonitorWidth, gPhysMonitorHeight);
    }
    else {
        gint winwidth, winheight;

        winwidth = gtk_widget_get_allocated_width(gWin);
        winheight = gtk_widget_get_allocated_height(gWin);
        winwidth = gtk_widget_get_allocated_width(sDrawingArea);
        winheight = gtk_widget_get_allocated_height(sDrawingArea);

        /* We need to size the actual window, not just the drawing area.
         * Resizing the drawing area will resize the window for many
         * window managers, but not for metacity.
         *
         * Worse, metacity maximizes a window if the initial size is
         * bigger in either dimension than the screen size.
         * Since we can't be sure about the size of the wm decorations,
         * we will probably hit this and get unintentionally maximized,
         * after which metacity refuses to resize the window any smaller.
         * (Mac OS X apparently does this too.)
         * So force non-maximal mode.  (Users who want a maximized
         * window will probably prefer fullscreen mode anyway.)
         */
        gtk_window_unfullscreen(GTK_WINDOW(gWin));
        gtk_window_unmaximize(GTK_WINDOW(gWin));

        /* XXX Without the next line, we may get no expose events!
         * Likewise, if the next line doesn't actually resize anything
         * we may not get an expose event.
         */
        if (gCurImage->curWidth != winwidth
            || gCurImage->curHeight != winheight) {
            gtk_window_resize(GTK_WINDOW(gWin),
                              gCurImage->curWidth, gCurImage->curHeight);

            /* Unfortunately, on OS X this resize may not work,
             * if it puts part ofthe window off-screen; in which case
             * we won't get an Expose event. So if that happened,
             * force a redraw:
             */
            /* Just as unfortunately, get_size probably isn't reliable
             * on Linux either, until after the window manager has had
             * a chance to act on the resize request
             * (at which time we'll get another Configure notify).
             * So the dilemma is: we need to call DrawImage now in the
             * case where there will be no further events. But if there
             * are further events, we want to wait for them and not
             */
            winwidth = gtk_widget_get_allocated_width(sDrawingArea);
            winheight = gtk_widget_get_allocated_height(sDrawingArea);
            if (gCurImage->curWidth != winwidth
                || gCurImage->curHeight != winheight) {
                if (gDebug)
                    printf("Resize didn't work! Forcing redraw\n");
                DrawImage();
            }
        }

        /* If we didn't resize the window, then we won't get an expose
         * event, and hence DrawImage won't be called. So call it explicitly
         * -- but not if we haven't displayed a window yet.
         * If we do that, we get duplicate calls to DrawImage
         * plus in keywords mode, the keywords dialog gets set as
         * transient too early. (I hate window management.)
         */
        else if (gtk_widget_get_visible(gWin)) {
            DrawImage();
        }

        /* Try to ensure that the window will be over the cursor
         * (so it will still have focus -- some window managers will
         * lose focus otherwise). But not in Keywords mode, where the
         * mouse should be over the Keywords dialog, not necessarily
         * the image window.
         */
        MaybeMove();
    }

    /* Want to request the focus here, but
     * neither gtk_window_present nor gdk_window_focus seem to work.
     */
}

