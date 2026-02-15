/*
 * Test stub implementations for functions defined in gmain.c
 * This file provides the missing symbols needed for testing
 */

#include "../pho.h"
#include "../dialogs.h"

/* Globals from gmain.c */
char* gCapFileFormat = "Captions";
int gRandomOrder = 0;

/* Function stubs */
PhoImage* AddImage(char* filename) {
    PhoImage* img = NewPhoImage(filename);
    if (img) {
        AppendItem(img);
    }
    return img;
}

gint HandleGlobalKeys(GtkWidget* widget, GdkEventKey* event) {
    /* Stub - just return FALSE to indicate not handled */
    (void)widget;
    (void)event;
    return FALSE;
}

/* Note: ChangeWorkingFileSet and ToggleKeywordsMode are defined in gdialogs.c and keydialog.c */
