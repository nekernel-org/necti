#include <xpcom.h>

/**
@brief MsgBox sample app.
*/

INT32 ENTRY_POINT(XHANDLE POBJ) /* POBJ = Program Object */
{
    XMessageBox(X_CURRENT_DESKTOP, X_ABOUT_DLG, NULL, NULL);
    return 0;
}
