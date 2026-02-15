/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * gmain.c: gtk main routines for pho, an image viewer.
 *
 * Copyright 2004 by Akkana Peck.
 * You are free to use or modify this code under the Gnu Public License.
 */

#include "pho.h"
#include "dialogs.h"
#include "exif/phoexif.h"

#include <gdk/gdk.h>
#include <gdk/gdkkeysyms.h>

#include <stdlib.h>       /* for getenv() */
#include <unistd.h>
#include <string.h>
#include <ctype.h>

char * gCapFileFormat = "Captions";

/* randomize order in which images will be shown? */
int gRandomOrder = 0;

/* Toggle a variable between two modes, preferring the first.
 * If it's anything but mode1 it will end up as mode1.
 */
#define ToggleBetween(val,mode1,mode2) (val != mode1 ? mode1 : mode2)

/* If we're switching into scaling mode because the user pressed - or +/=,
 * which scaling mode should we switch into?
 */
static int ModeForScaling(int oldmode)
{
    switch (oldmode)
    {
      case PHO_SCALE_IMG_RATIO:
      case PHO_SCALE_FULLSIZE:
      case PHO_SCALE_NORMAL:
        return PHO_SCALE_IMG_RATIO;

      case PHO_SCALE_FIXED:
          return PHO_SCALE_FIXED;

      case PHO_SCALE_FULLSCREEN:
      default:
        return PHO_SCALE_SCREEN_RATIO;
    }
}

static void RunPhoCommand()
{
    int i;
    char* cmd;
    
    /* Check for NULL current image to prevent crash */
    if (!gCurImage) {
        fprintf(stderr, "No current image selected\n");
        return;
    }
    
    cmd = getenv("PHO_CMD");
    if (!cmd) cmd = "gimp";
    else if (! *cmd) {
        if (gDebug)
            printf("PHO_CMD set to NULL, not running anything\n");
        return;
    }
    else if (gDebug)
        printf("Calling PHO_CMD %s\n", cmd);

    gint gargc;
    gchar** gargv;
    gchar** new_argv = 0;
    GError *gerror = 0;
    if (! g_shell_parse_argv (cmd, &gargc, &gargv, &gerror)) {
        fprintf(stderr, "Couldn't parse PHO_CMD %s\nError was %s",
                cmd, gerror->message);
        return;
    }

    /* PHO_CMD command can have a %s in it; if it does, substitute filename. */
    int added_arg = 0;
    for (i=1; i<gargc; ++i) {
        if (gargv[i][0] == '%' && gargv[i][1] == 's') {
            gargv[i] = gCurImage->filename;
            added_arg = 1;
        }
    }
    /* If it didn't have a %s, then we have to allocate a whole new argv array
     * so we can add the filename argument at the end.
     * Note that glib expects the argv to be zero terminated --
     * it provides an argc from g_shell_parse_args() but doesn't
     * take one in g_spawn_async().
     */
    if (! added_arg) {
        int new_argc = gargc + 2;
        gchar** new_argv = malloc(sizeof(gchar *) * new_argc);
        if (!new_argv) {
            fprintf(stderr, "Out of memory!\n");
            return;
        }
        for (i=0; i<gargc; ++i)
            new_argv[i] = gargv[i];
        new_argv[gargc] = gCurImage->filename;
        new_argv[gargc+1] = 0;
        gargv = new_argv;
    }

    if (! g_spawn_async(NULL, gargv,
                        NULL, G_SPAWN_SEARCH_PATH, NULL, NULL, NULL, &gerror))
        fprintf(stderr, "Couldn't spawn %s: \"%s\"\n", cmd, gerror->message);

    if (new_argv)
        free(new_argv);
}

void TryScale(float times)
{
    /* Save the view modes, in case this fails */
    int saveScaleMode = gScaleMode;
    double saveScaleRatio = gScaleRatio;
    int saveDisplayMode = gDisplayMode;

    if (SetViewModes(gDisplayMode, ModeForScaling(gScaleMode),
                     gScaleRatio * times) == 0)
        return;

    /* Oops! It didn't work. Try to reset back to previous settings */
    SetViewModes(saveDisplayMode, saveScaleMode, saveScaleRatio);
}

gint HandleGlobalKeys(GtkWidget* widget, GdkEventKey* event)
{
    if (gDebug) {
        printf("\nKey event: ");
        printf("state:%d, ", event->state);
        printf("keyval: %x\n", event->keyval);
    }
    /* In some desktops, high modifier keys like MOD3_MASK mysteriously
     * get added to key events where the user didn't press any modifier
     * keys. So only test for ctrl and alt; ignore all other modifiers.
     */
    if (event->state &&
        (event->state & (GDK_CONTROL_MASK | GDK_MOD1_MASK))) {
        switch (event->keyval)
        {
            case GDK_KEY_f:
                /* Don't respond to ctrl-F -- that might be an attempt
                 * to edit in a text field in the keywords dialog.
                 * But we don't do anything on f with any modifier key either.
                 */
                return FALSE;
            case GDK_KEY_0:
            case GDK_KEY_1:
            case GDK_KEY_2:
            case GDK_KEY_3:
            case GDK_KEY_4:
            case GDK_KEY_5:
            case GDK_KEY_6:
            case GDK_KEY_7:
            case GDK_KEY_8:
            case GDK_KEY_9:
                if (event->state & GDK_MOD1_MASK) { /* alt-num: add 10 to num */
                    ToggleNoteFlag(gCurImage, event->keyval - GDK_KEY_0 + 10);
                    return TRUE;
                }
                return FALSE;
            case GDK_KEY_equal:
                if (event->state & GDK_CONTROL_MASK) {
                    TryScale(1.25);
                    return TRUE;
                }
                return FALSE;
            case GDK_KEY_KP_Subtract:
                if (event->state & GDK_CONTROL_MASK) {
                    TryScale(.8);
                    return TRUE;
                }
                return FALSE;
            default:
                return FALSE;
        }
    }

    /* Now we know no modifier keys were down. */
    switch (event->keyval)
    {
      case GDK_KEY_d:
          if (gCurImage)
              DeleteImage(gCurImage);
          break;
      case GDK_KEY_space:
      case GDK_KEY_Page_Down:
      case GDK_KEY_KP_Page_Down:
          /* If we're in slideshow mode, cancel the slideshow */
          if (gDelayMillis > 0) {
              gDelayMillis = 0;
          }
          else if (NextImage() != 0) {
              if (Prompt("Quit pho?", "Quit", "Continue", "qx \n", "cn") != 0)
                  EndSession();
          }
          return TRUE;
      case GDK_KEY_BackSpace:
      case GDK_KEY_Page_Up:
      case GDK_KEY_KP_Page_Up:
          PrevImage();
          return TRUE;
      case GDK_KEY_Home:
          gCurImage = 0;
          NextImage();
          return TRUE;
      case GDK_KEY_End:
          gCurImage = gFirstImage->prev;
          ThisImage();
          return TRUE;
      case GDK_KEY_n:   /* Get out of any weird display modes */
          SetViewModes(PHO_DISPLAY_NORMAL, PHO_SCALE_NORMAL, 1.);
          ShowImage();
          return TRUE;
      case GDK_KEY_f:   /* Full size mode: show image bit-for-bit */
          SetViewModes(gDisplayMode,
                       ToggleBetween(gScaleMode,
                                     PHO_SCALE_FULLSIZE, PHO_SCALE_NORMAL),
                       1.);
          return TRUE;
      case GDK_KEY_F:   /* Full screen mode: as big as possible on screen */
          SetViewModes(gDisplayMode,
                       ToggleBetween(gScaleMode,
                                     PHO_SCALE_FULLSCREEN, PHO_SCALE_NORMAL),
                       1.);
          return TRUE;
      case GDK_KEY_p:
          SetViewModes((gDisplayMode == PHO_DISPLAY_PRESENTATION)
                       ? PHO_DISPLAY_NORMAL
                       : PHO_DISPLAY_PRESENTATION,
                       gScaleMode, gScaleRatio);
          return TRUE;
      case GDK_KEY_0:
      case GDK_KEY_1:
      case GDK_KEY_2:
      case GDK_KEY_3:
      case GDK_KEY_4:
      case GDK_KEY_5:
      case GDK_KEY_6:
      case GDK_KEY_7:
      case GDK_KEY_8:
      case GDK_KEY_9:
          ToggleNoteFlag(gCurImage, event->keyval - GDK_KEY_0);
          return TRUE;
      case GDK_KEY_t:   /* make life easier for xv switchers */
      case GDK_KEY_r:
      case GDK_KEY_Right:
      case GDK_KEY_KP_Right:
          ScaleAndRotate(gCurImage, 90);
          return TRUE;
      case GDK_KEY_T:   /* make life easier for xv users */
      case GDK_KEY_R:
      case GDK_KEY_l:
      case GDK_KEY_L:
      case GDK_KEY_Left:
      case GDK_KEY_KP_Left:
          ScaleAndRotate(gCurImage, 270);
          return TRUE;
      case GDK_KEY_Up:
      case GDK_KEY_Down:
          ScaleAndRotate(gCurImage, 180);
          return TRUE;
      case GDK_KEY_plus:
      case GDK_KEY_KP_Add:
      case GDK_KEY_equal:
          TryScale(2.);
          return TRUE;
      case GDK_KEY_minus:
      case GDK_KEY_slash:
      case GDK_KEY_KP_Subtract:
          TryScale(.5);
          return TRUE;
      case GDK_KEY_g:  /* start gimp, or some other app */
          RunPhoCommand();
          return TRUE;
      case GDK_KEY_i:
          ToggleInfo();
          return TRUE;
      case GDK_KEY_k:
          ToggleKeywordsMode();
          return TRUE;
      case GDK_KEY_o:
          ChangeWorkingFileSet();
          return TRUE;
      case GDK_KEY_Escape:
      case GDK_KEY_q:
          EndSession();
          return TRUE;
      default:
          if (gDebug)
              printf("Don't know key 0x%lu\n", (unsigned long)(event->keyval));
          return FALSE;
    }
    /* Keep gcc 2.95 happy: */
    return FALSE;
}

PhoImage* AddImage(char* filename)
{
    PhoImage* img = NewPhoImage(filename);
    if (gDebug)
        printf("Adding image %s\n", filename);
    if (!img) {
        fprintf(stderr, "Out of memory!\n");
        exit(1);
    }
    /* Make img the new last image in the list */
    AppendItem(img);
    return img;
}

static void parseGeom(char* geom, int* width, int* height)
{
    int num;
    if (!isdigit(*geom)) {
        printf("Geometry '%s' must start with a digit\n", geom);
        Usage();
    }
    num = atoi(geom);
    if (num > 0)
        *width = num;

    /* Now see if there's a height */
    while (*(++geom)) {
        if (*geom == 'x') {
            ++geom;
            if (!isdigit(*geom)) {
                printf("Number after 'x' in geometry must start with a digit, not %s\n", geom);
                Usage();
            }
            num = atoi(geom);
            if (num > 0)
                *height = num;
            return;
        }
    }
}

/* CheckArg takes a string, like -Pvg, and sets all the relevant flags. */
static void CheckArg(char* arg)
{
    for ( ; *arg != 0; ++arg)
    {
        if (*arg == '-')
            ;
        else if (*arg == 'd')
            gDebug = 1;
        else if (*arg == 'h')
            Usage();
        else if (*arg == 'v')
            VerboseHelp();
        else if (*arg == 'm') {
            if (isdigit(arg[1]) || arg[1] == '.')
                gUseMonitor = (int)(atof(arg+1));
            else Usage();
            if (gDebug)
                printf("Slideshow delay %d milliseconds\n", gDelayMillis);
        }
        else if (*arg == 'n')
            gMakeNewWindows = 1;
        else if (*arg == 'p') {
            gDisplayMode = PHO_DISPLAY_PRESENTATION;
            if (arg[1] == '\0') {
                /* Normal presentation mode -- center everything
                 * and size to the current display.
                 */
                gPresentationWidth = 0;
                gPresentationHeight = 0;
            } else {
                char* geom = arg+1;
                if (*geom == 'p') ++geom;
                gPresentationWidth = 0;
                gPresentationHeight = 0;
                parseGeom(geom, &gPresentationWidth, &gPresentationHeight);

                /* Check for width without height, in case the user
                 * ran something like pho -p1024, which is almost
                 * certainly a mistake.
                 */
                if (gPresentationWidth && !gPresentationHeight) {
                    fprintf(stderr,
                            "Presentation width %d given with no height.\n\n",
                            gPresentationWidth);
                    Usage();
                }
                /* Note there's no check for the opposite, height without width.
                 * For that, the user would have had to specify something like
                 * 0x768; in the unlikely event the user does that,
                 * maybe it means they really want it for some strange reason.
                 */
            }
        } else if (*arg == 'P')
            gDisplayMode = PHO_DISPLAY_NORMAL;
        else if (*arg == 'k') {
            gDisplayMode = PHO_DISPLAY_KEYWORDS;
            gScaleMode = PHO_SCALE_FIXED;
            gScaleRatio = 0.0;
        } else if (*arg == 's') {
            /* find the slideshow delay time, from e.g. pho -s2 */
            /* If no number given, use default delay */
            if (isdigit(arg[1]) || arg[1] == '.')
                gDelayMillis = (int)(atof(arg+1) * 1000.);
            else if (arg[1] == '\0')
                gDelayMillis = DEFAULT_SLIDESHOW_DELAY;
            else Usage();
            if (gDebug)
                printf("Slideshow delay %d milliseconds\n", gDelayMillis);
        } else if (*arg == 'r') {
            gRepeat = 1;
        } else if (*arg == 'c') {
            gCapFileFormat = strdup(arg+1);
            if (gDebug)
                printf("Format set to '%s'\n", gCapFileFormat);

            /* Can't follow this with any other letters -- they're all
             * part of the filename -- so return.
             */
            return;
        } else if (*arg == 'R') {
            gRandomOrder = 1;
        }
    }
}

int main(int argc, char** argv)
{
    /* Initialize some defaults from environment variables,
     * before reading cmdline args.
     */
    int options = 1;

    char* env = getenv("PHO_ARGS");
    if (env && *env)
        CheckArg(env);

    while (argc > 1)
    {
        if (argv[1][0] == '-' && options) {
            if (strcmp(argv[1], "--"))
                CheckArg(argv[1]);
            else
                options = 0;
        }
        else {
            AddImage(argv[1]);
        }
        --argc;
        ++argv;
    }

    if (gFirstImage == 0)
        Usage();

    if (gRandomOrder)
        ShuffleImages();

    /* Initialize some variables associated with the notes flags */
    InitNotes();

    /* See http://www.gtk.org/tutorial */
    gtk_init(&argc, &argv);

    /* gdk_rgb_init() removed in GTK3 - RGB handling is automatic now */

    /* GTK3: Use monitor API instead of deprecated gdk_screen_width/height */
    GdkDisplay *display = gdk_display_get_default();
    if (!display) {
        fprintf(stderr, "Error: Cannot get default display\n");
        exit(1);
    }
    GdkMonitor *monitor = gdk_display_get_primary_monitor(display);
    if (!monitor) {
        /* Fallback: try to get the first monitor */
        monitor = gdk_display_get_monitor(display, 0);
    }
    if (!monitor) {
        fprintf(stderr, "Error: Cannot get monitor information\n");
        exit(1);
    }
    GdkRectangle geometry;
    gdk_monitor_get_geometry(monitor, &geometry);
    gPhysMonitorWidth = gMonitorWidth = geometry.width;
    gPhysMonitorHeight = gMonitorHeight = geometry.height;

    /* Load the first image */
    if (NextImage() != 0)
        exit(1);

    gtk_main();
    return 0;
}

