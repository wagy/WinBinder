/*******************************************************************************

 WINBINDER - The native Windows binding for PHP

 Copyright © Hypervisual - see LICENSE.TXT for details
 Author: Rubem Pechansky (http://winbinder.org/contact.php)

 Functions for Windows controls

*******************************************************************************/

//----------------------------------------------------------------- DEPENDENCIES

#include "wb.h"
#include <time.h>				// For time_t
#include <stdlib.h>				// For itoa(), atoi()
#include <shellapi.h>			// For Shell_NotifyIcon()

#ifdef _MSC_VER
#	include <richedit.h>		// For EM
#endif

//-------------------------------------------------------------------- CONSTANTS

//-------------------------------------------------------------------- VARIABLES

// Global

WNDPROC lpfnFrameProcOld = NULL;
WNDPROC lpfnEditProcOld = NULL;
WNDPROC lpfnHyperLinkProcOld = NULL;
WNDPROC lpfnInvisibleProcOld = NULL;

// External

extern BOOL SetTaskBarIcon(HWND hwnd, BOOL bModify);
extern BOOL wbSetMenuItemText(PWBOBJ pwbo, LPCTSTR pszText);
extern DWORD GetCalendarTime(PWBOBJ pwbo);
extern BOOL SetCalendarTime(PWBOBJ pwbo, time_t UnixTime);

extern BOOL DisplayHTMLString(PWBOBJ pwbo, LPCTSTR string);
extern BOOL EmbedBrowserObject(PWBOBJ pwbo);

//---------------------------------------------------------- FUNCTION PROTOTYPES

// Global to the WinBinder API layer only

BOOL IsBitmap(HANDLE handle);
BOOL IsIcon(HANDLE handle);
BOOL RegisterImageButtonClass(void);
HWND CreateToolTip(PWBOBJ pwbo, LPCTSTR pszTooltip);

// Private

static BOOL SetTransparentBitmap(HWND hwnd, HBITMAP hbmBits, BOOL bStatic, COLORREF clTransp);

static LRESULT CALLBACK FrameProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
static LRESULT CALLBACK EditBoxProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
static LRESULT CALLBACK InvisibleProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
static LRESULT CALLBACK ImageButtonProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

// External

extern void SetStatusBarHandle(HWND hCtrl);
extern BOOL RegisterControlInTab(PWBOBJ pwboParent, PWBOBJ pwbo, UINT id, UINT nTab);
extern LRESULT CALLBACK HyperLinkProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

//----------------------------------------------------------- EXPORTED FUNCTIONS

/*
	TODO: Call wbCreateToolbar() if wbclass is a toolbar
	TODO: Call wbCreateMenu() if wbclass is a menu
*/

PWBOBJ wbCreateControl(PWBOBJ pwboParent, UINT uWinBinderClass, LPCTSTR pszSourceCaption, LPCTSTR pszSourceTooltip,
  int xPos, int yPos, int nWidth, int nHeight, UINT id, DWORD dwWBStyle, long lParam, int nTab)
{
	PWBOBJ pwbo;
	LPTSTR pszClass;
	int nVisible;
	DWORD dwStyle, dwExStyle;
	HWND hTab = NULL, hwndParent = NULL;
	TCHAR *pszCaption;
	TCHAR *pszTooltip;

	if(!pwboParent || !pwboParent->hwnd || !IsWindow(pwboParent->hwnd)) {
		wbError(TEXT(__FUNCTION__), MB_ICONWARNING, TEXT("Cannot create control if parent window is NULL"));
		return NULL;
	}
	pwbo = wbMalloc(sizeof(WBOBJ));
	if(!pwbo)
		return NULL;

	wbConvertLineBreaks(&pszCaption, pszSourceCaption);
	wbConvertLineBreaks(&pszTooltip, pszSourceTooltip);

	pwbo->hwnd = NULL;
	pwbo->id = id;
	pwbo->uClass = uWinBinderClass;
	pwbo->item = -1;
	pwbo->subitem = -1;
	pwbo->style = dwWBStyle;
	pwbo->parent = pwboParent;
	pwbo->pszCallBackFn = NULL;
	pwbo->pszCallBackObj = NULL;
	pwbo->lparam = lParam;
	ZeroMemory(pwbo->lparams, sizeof(LONG) * 8);
	ZeroMemory(&pwbo->rcTitle, sizeof(RECT) + 2 * sizeof(AREA));
	pwbo->pbuffer = NULL;

	// If parent is a tab control, change the parent to a page handle

	if(pwboParent->uClass == TabControl) {

		PTABDATA pTabData;

		pTabData = (PTABDATA)pwboParent->lparam;

		// nTab is the page index

		nTab = MIN(MIN((UINT)nTab, pTabData->nPages - 1), MAX_TABS);
		hTab = pTabData->page[nTab].hwnd ?
		  pTabData->page[nTab].hwnd : pwboParent->hwnd;

	}

	// Set visible / enabled flags

	nVisible = BITTEST(dwWBStyle, WBC_INVISIBLE) ? 0 : WS_VISIBLE;
	nVisible |= BITTEST(dwWBStyle, WBC_DISABLED) ? WS_DISABLED : 0;

	if(BITTEST(dwWBStyle, WBC_BORDER))
		dwExStyle = WS_EX_STATICEDGE;
	else
		dwExStyle = 0;

	// Set Windows class and styles according to WinBinder class

	switch(uWinBinderClass) {

		//-------------------------------- Regular child controls

		case HyperLink:

			pszClass = TEXT("STATIC");
			dwStyle = WS_CHILD | SS_NOTIFY | SS_LEFT | nVisible;
			dwExStyle = WS_EX_TRANSPARENT;
			break;

		case Label:

			pszClass = TEXT("STATIC");
			if(BITTEST(dwWBStyle, WBC_ELLIPSIS))
				dwStyle = WS_CHILD | SS_WORDELLIPSIS | nVisible;
			else
				dwStyle = WS_CHILD | nVisible;

			if(BITTEST(dwWBStyle, WBC_CENTER))
				dwStyle |= SS_CENTER;
			else if(BITTEST(dwWBStyle, WBC_RIGHT))
				dwStyle |= SS_RIGHT;
			else {
				dwStyle |= SS_LEFT;
				if(!BITTEST(dwWBStyle, WBC_MULTILINE))
					dwStyle |= SS_LEFTNOWORDWRAP;
			}
			break;

		case Frame:
			if((nWidth > nHeight) && nHeight <= 8) {			// Horizontal line
				pszClass = TEXT("STATIC");
				dwStyle = WS_CHILD | SS_ETCHEDHORZ | nVisible;
			} else if((nWidth < nHeight) && nWidth <= 8) {		// Vertical line
				pszClass = TEXT("STATIC");
				dwStyle = WS_CHILD | SS_ETCHEDVERT | nVisible;
			} else if(BITTEST(dwWBStyle, WBC_IMAGE)) {			// Image
				pszClass = TEXT("STATIC");
				dwStyle = WS_CHILD | SS_WORDELLIPSIS | SS_CENTERIMAGE | nVisible;
				if(BITTEST(dwWBStyle, WBC_CENTER))
					dwStyle |= SS_CENTER;
				else if(BITTEST(dwWBStyle, WBC_RIGHT))
					dwStyle |= SS_RIGHT;
				else
					dwStyle |= SS_LEFT;
			} else {											// Group box
				pszClass = TEXT("BUTTON");
				dwStyle = WS_CHILD | WS_TABSTOP | BS_GROUPBOX | nVisible;
				dwExStyle |= WS_EX_TRANSPARENT;
			}
			break;

		case Calendar:
			pszClass = MONTHCAL_CLASS;
			dwStyle = (BITTEST(dwWBStyle, WBC_BORDER) ? WS_BORDER : 0) | WS_CHILD | WS_VISIBLE | MCS_DAYSTATE | nVisible;
			break;

		case PushButton:
			pszClass = TEXT("BUTTON");
			dwStyle =  (BITTEST(dwWBStyle, WBC_BORDER) ? BS_DEFPUSHBUTTON : 0) | WS_CHILD | WS_TABSTOP | BS_PUSHBUTTON | BS_NOTIFY | nVisible;
			break;

		case InvisibleArea:
			pszClass = TEXT("STATIC");
			dwStyle = WS_CHILD | SS_NOTIFY | SS_BITMAP | nVisible;
			dwExStyle = WS_EX_TRANSPARENT;
			break;

		case ImageButton:
			pszClass = IMAGE_BUTTON_CLASS;		// This is a custom control class
			dwStyle = WS_CHILD | WS_TABSTOP | nVisible;
			if(pwbo->style & WBC_TRANSPARENT)
				dwExStyle = WS_EX_TRANSPARENT;
			break;

		case CheckBox:
			pszClass = TEXT("BUTTON");
			dwStyle = WS_CHILD | WS_TABSTOP | BS_AUTOCHECKBOX | BS_NOTIFY | nVisible;
			break;

		case RadioButton:
			pszClass = TEXT("BUTTON");
			dwStyle = WS_CHILD | WS_TABSTOP | BS_AUTORADIOBUTTON | BS_NOTIFY | nVisible;
			if(BITTEST(dwWBStyle, WBC_GROUP))
				dwStyle |= WS_GROUP;
			break;

		case EditBox:
			pszClass = TEXT("EDIT");

			dwStyle = WS_CHILD | WS_TABSTOP | nVisible;
			if(BITTEST(dwWBStyle, WBC_READONLY))
				dwStyle |= ES_READONLY;
			if(BITTEST(dwWBStyle, WBC_MASKED))
				dwStyle |= ES_PASSWORD;
			if(BITTEST(dwWBStyle, WBC_NUMBER))
				dwStyle |= ES_NUMBER;
			if(BITTEST(dwWBStyle, WBC_MULTILINE)) {
				if(BITTEST(dwWBStyle, WBC_READONLY))
					dwStyle |= WS_VSCROLL | ES_MULTILINE;
				else
					dwStyle |= WS_VSCROLL | ES_MULTILINE | ES_WANTRETURN;
			} else {
				dwStyle |= ES_AUTOHSCROLL;
				if(BITTEST(dwWBStyle, WBC_CENTER))
					dwStyle |= ES_CENTER;
				else if(BITTEST(dwWBStyle, WBC_RIGHT))
					dwStyle |= ES_RIGHT;
				else
					dwStyle |= ES_LEFT;
			}
			dwExStyle |= WS_EX_CLIENTEDGE;
			break;

		case RTFEditBox:
			pszClass = RICHEDITCONTROL;
			dwStyle = WS_CHILD | WS_TABSTOP | nVisible;
			if(BITTEST(dwWBStyle, WBC_READONLY))
				dwStyle |= ES_READONLY | WS_VSCROLL | ES_MULTILINE;
			else
				dwStyle |= WS_VSCROLL | ES_MULTILINE | ES_WANTRETURN;
			if(BITTEST(dwWBStyle, WBC_CENTER))
				dwStyle |= ES_CENTER;
			else if(BITTEST(dwWBStyle, WBC_RIGHT))
				dwStyle |= ES_RIGHT;
			else
				dwStyle |= ES_LEFT;
			dwExStyle |= WS_EX_CLIENTEDGE;
			break;

		case ListBox:
			pszClass = TEXT("LISTBOX");
			dwStyle = WS_CHILD | WS_TABSTOP | WS_VSCROLL | LBS_NOTIFY | LBS_NOINTEGRALHEIGHT | nVisible;
			if(BITTEST(dwWBStyle, WBC_SORT))
				dwStyle |= LBS_SORT;
			if(BITTEST(dwWBStyle, WBC_MULTISELECT))
				dwStyle |= LBS_EXTENDEDSEL;
			dwExStyle |= WS_EX_CLIENTEDGE;
			break;

		case ComboBox:
			pszClass = TEXT("COMBOBOX");
			dwStyle = WS_CHILD | WS_TABSTOP | WS_VSCROLL | nVisible;
			if(BITTEST(dwWBStyle, WBC_READONLY))
				dwStyle |= CBS_DROPDOWNLIST;
			else
				dwStyle |= CBS_DROPDOWN;
			if(BITTEST(dwWBStyle, WBC_SORT))
				dwStyle |= CBS_SORT;
			break;

		case ScrollBar:
			pszClass = TEXT("SCROLLBAR");
			if(nHeight >= nWidth)
				dwStyle = WS_CHILD | WS_TABSTOP | SBS_VERT | nVisible;
			else
				dwStyle = WS_CHILD | WS_TABSTOP | nVisible;
			break;

		case ListView:
			pszClass = WC_LISTVIEW;
			dwStyle = WS_CHILD | WS_TABSTOP | LVS_REPORT | LVS_SHOWSELALWAYS | LVS_NOLABELWRAP | nVisible;
			if(!BITTEST(dwWBStyle, WBC_SORT))
				dwStyle |= LVS_NOSORTHEADER;
			if(BITTEST(dwWBStyle, WBC_SINGLE))
				dwStyle |= LVS_SINGLESEL;
			if(BITTEST(dwWBStyle, WBC_NOHEADER))
				dwStyle |= LVS_NOCOLUMNHEADER;
			dwExStyle |= WS_EX_CLIENTEDGE;
			break;

		case Spinner:
			pszClass = UPDOWN_CLASS;
			if(BITTEST(dwWBStyle, WBC_GROUP))
				dwStyle = WS_CHILD | WS_TABSTOP | UDS_ARROWKEYS | UDS_NOTHOUSANDS | UDS_AUTOBUDDY | UDS_SETBUDDYINT | nVisible;
			else
				dwStyle = WS_CHILD | WS_TABSTOP | UDS_ARROWKEYS | UDS_NOTHOUSANDS | nVisible;
			break;

		case Gauge:
			pszClass = PROGRESS_CLASS;
			dwStyle = WS_CHILD | nVisible;
			break;

		case Slider:
			pszClass = TRACKBAR_CLASS;
			if(BITTEST(dwWBStyle, WBC_LINES))
				dwStyle = WS_CHILD | WS_TABSTOP | TBS_AUTOTICKS | nVisible;
			else
				dwStyle = WS_CHILD | WS_TABSTOP | TBS_NOTICKS | nVisible;
			break;

		case TreeView:
			{
				PTREEDATA ptrdt;
				int i;

				pszClass = WC_TREEVIEW;
				dwStyle = WS_CHILD | WS_TABSTOP | TVS_HASBUTTONS | TVS_LINESATROOT | TVS_SHOWSELALWAYS | nVisible;
				dwExStyle |= WS_EX_CLIENTEDGE;

				// Attach a structure to the treeview control and initialize it

				ptrdt = wbMalloc(sizeof(TREEDATA));
//				memset(ptrdt, 0, sizeof(TREEDATA));

				ptrdt->hLast = TVI_ROOT;
				for(i = 0; i < MAX_TREEVIEW_LEVELS; i++) {
					ptrdt->hParent[i] = TVI_ROOT;
					ptrdt->bCustomImages[i] = FALSE;
				}
				ptrdt->nLastLevel = 0;
				pwbo->lparam = (LPARAM)ptrdt;
			}
			break;

		case StatusBar:
			pszClass =  STATUSCLASSNAME;
			if(BITTEST(GetWindowLong(pwboParent->hwnd, GWL_STYLE), WS_THICKFRAME))
				dwStyle = WS_CHILD | CCS_BOTTOM | CCS_NOMOVEY | SBARS_SIZEGRIP | nVisible;
			else
				dwStyle = WS_CHILD | CCS_BOTTOM | CCS_NOMOVEY | nVisible;
			break;

		case TabControl:
			pszClass = WC_TABCONTROL;
			dwStyle = WS_CHILD | WS_TABSTOP | WS_CLIPCHILDREN | nVisible;
			{
				PTABDATA pTabData;

				// Alloc memory for tab data

				pTabData = wbMalloc(sizeof(TABDATA));
				memset(pTabData, 0, sizeof(TABDATA));
				pwbo->lparam = (LPARAM)pTabData;
				pwbo->uClass = TabControl;

			}
			break;

		case HTMLControl:
			pszClass = BROWSER_WINDOW_CLASS;
			dwStyle = WS_CHILD | nVisible;
			break;

		default:
			wbError(TEXT(__FUNCTION__), MB_ICONWARNING, TEXT("Unknown control class %d"), uWinBinderClass);
			return NULL;

	} // end switch

	// Create the control

	hwndParent = (pwboParent->uClass == TabControl) ? hTab : (pwboParent ? pwboParent->hwnd : NULL);
	pwbo->hwnd = CreateWindowEx(
		dwExStyle,
		pszClass,
		pszCaption && *pszCaption ? pszCaption : TEXT(""),
		dwStyle,
		xPos, yPos, nWidth, nHeight,
		hwndParent,
		(HMENU)id,
		hAppInstance,
		NULL
	);

	if(!pwbo->hwnd) {
		wbError(TEXT(__FUNCTION__), MB_ICONWARNING, TEXT("Error creating control of class %d ('%s')"),
		  uWinBinderClass, pszClass);
		return NULL;
	}

	// Further processing after control creation

	switch(pwbo->uClass) {

		case HTMLControl:
			EmbedBrowserObject(pwbo);
			break;

		case RadioButton:
		case CheckBox:
		case PushButton:


			SendMessage(pwbo->hwnd, WM_SETFONT, (WPARAM)hIconFont, 0);
			if(lParam)		// Check button
				SendMessage(pwbo->hwnd, BM_SETCHECK, TRUE, 0);
			CreateToolTip(pwbo, pszTooltip);
			break;

		case EditBox:			// Subclasses edit box to process keyboard messages

			SendMessage(pwbo->hwnd, WM_SETFONT, (WPARAM)hIconFont, 0);
			lpfnEditProcOld = (WNDPROC)SetWindowLong(pwbo->hwnd, GWL_WNDPROC, (LONG)EditBoxProc);
			CreateToolTip(pwbo, pszTooltip);
			break;

		case TabControl:

			SendMessage(pwbo->hwnd, WM_SETFONT, (WPARAM)hIconFont, 0);
//			// Subclasses tab control to process WM_COMMAND
//			lpfnTabProcOld = (WNDPROC)SetWindowLong(pwbo->hwnd, GWL_WNDPROC, (LONG)TabProc);
			CreateToolTip(pwbo, pszTooltip);
			wbSetTabControlText(pwbo, pszCaption);

			// Assign window handler, if any

//			if(pwbo->parent && pwbo->parent->pszCallBackFn && *pwbo->parent->pszCallBackFn)
//				pwbo->pszCallBackFn = pwbo->parent->pszCallBackFn;
			break;

		case Frame:				// Subclasses Frame control to process WM_COMMAND

			SendMessage(pwbo->hwnd, WM_SETFONT, (WPARAM)hIconFont, 0);
			if(!wcsicmp(pszClass, TEXT("BUTTON")))		// Only for group boxes!
				lpfnFrameProcOld = (WNDPROC)SetWindowLong(pwbo->hwnd, GWL_WNDPROC, (LONG)FrameProc);
			break;

		case InvisibleArea:		// Subclasses InvisibleArea to process WM_MOUSEMOVE
			CreateToolTip(pwbo, pszTooltip);
			lpfnInvisibleProcOld = (WNDPROC)SetWindowLong(pwbo->hwnd, GWL_WNDPROC, (LONG)InvisibleProc);
			wbSetCursor(pwbo, NULL, 0);		// Assumes class cursor
			break;

		case ImageButton:
			CreateToolTip(pwbo, pszTooltip);
			wbSetCursor(pwbo, NULL, 0);		// Assumes class cursor

			if(!(pwbo->style & WBC_TRANSPARENT)) {
				HIMAGELIST hi;
				static int nImages = 4;

				hi = ImageList_Create(nWidth, nHeight, ILC_COLORDDB, nImages, 0);
				M_hiImageList = (LPARAM)hi;	// Store ImageList in control
			}
			break;

		case Spinner:

			SendMessage(pwbo->hwnd, WM_SETFONT, (WPARAM)hIconFont, 0);
			SendMessage(pwbo->hwnd, UDM_SETRANGE, 0, MAKELONG(100, 0));
			SendMessage(pwbo->hwnd, UDM_SETPOS, 0, MAKELONG((short)0, 0));
			CreateToolTip(pwbo, pszTooltip);
			break;

		case ScrollBar:
			SendMessage(pwbo->hwnd, SBM_SETRANGE, 0, 100);
			CreateToolTip(pwbo, pszTooltip);
			break;

		case StatusBar:
			SendMessage(pwbo->hwnd, WM_SETFONT, (WPARAM)hIconFont, 0);
			SetStatusBarHandle(pwbo->hwnd);
			CreateToolTip(pwbo, pszTooltip);
			break;

		case ListView:

			SendMessage(pwbo->hwnd, WM_SETFONT, (WPARAM)hIconFont, 0);
			SendMessage(pwbo->hwnd, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT | LVS_EX_HEADERDRAGDROP | LVS_EX_LABELTIP | LVS_EX_SUBITEMIMAGES);
			break;

		case RTFEditBox:

			SendMessage(pwbo->hwnd, WM_SETFONT, (WPARAM)hIconFont, 0);
			SendMessage(pwbo->hwnd, EM_FMTLINES, TRUE, 0);
			SendMessage(pwbo->hwnd, EM_SETEVENTMASK, 0, ENM_CHANGE);	// Enables EN_CHANGE messages. Should be EN_UPDATE instead?
			SetWindowText(pwbo->hwnd, pszCaption);
			CreateToolTip(pwbo, pszTooltip);
			break;

		case HyperLink:
			SendMessage(pwbo->hwnd, WM_SETFONT, (WPARAM)hIconFont, 0);
			// Subclasses static control
			lpfnHyperLinkProcOld = (WNDPROC)SetWindowLong(pwbo->hwnd, GWL_WNDPROC, (LONG)HyperLinkProc);
			wbSetCursor(pwbo, NULL, 0);		// Assumes class cursor
			CreateToolTip(pwbo, pszTooltip);
			break;

		case Calendar:
		case ListBox:
		case ComboBox:
		case Gauge:
		case Slider:
		case TreeView:

			SendMessage(pwbo->hwnd, WM_SETFONT, (WPARAM)hIconFont, 0);
			CreateToolTip(pwbo, pszTooltip);
			break;

		default:

			SendMessage(pwbo->hwnd, WM_SETFONT, (WPARAM)hIconFont, 0);
			break;
	}

	// Add any additional styles that may be present

	wbSetStyle(pwbo, dwWBStyle, TRUE);

	// Is the parent window a tab control?

	if(pwboParent->uClass == TabControl)
		RegisterControlInTab(pwboParent, pwbo, id, nTab);

	SetWindowLong(pwbo->hwnd, GWL_USERDATA, (LONG)pwbo);

	return pwbo;
}


/* Destroy a control created by wbCreateControl(). */

BOOL wbDestroyControl(PWBOBJ pwbo)
{
	if(!pwbo || !pwbo->hwnd || !IsWindow(pwbo->hwnd))
		return FALSE;

	switch(pwbo->uClass) {
		case TreeView:
		case TabControl:
			wbFree((void *)pwbo->lparam);
			break;
	}
	return DestroyWindow(pwbo->hwnd);
}


/*
	TODO: Process treeview items?
	TODO: Must validate toolbar item first
*/

PWBOBJ wbGetControl(PWBOBJ pwboParent, int id)
{
	PWBOBJ pwbo = NULL;

	if(!wbIsWBObj(pwboParent, TRUE)) {				// Is it a valid control?
		wbError(TEXT(__FUNCTION__), MB_ICONWARNING, TEXT("First parameter is not a valid WinBinder control"));
		return NULL;
	}

	if(pwboParent->uClass == Menu) {

		if(IsMenu((HMENU)pwboParent->hwnd)) {		// Is it a menu?

			MENUITEMINFO mi;

			mi.cbSize = sizeof(MENUITEMINFO);
			mi.fMask = MIIM_ID;
			if(GetMenuItemInfo((HMENU)pwboParent->hwnd, id, FALSE, &mi)) {

				pwbo = wbMalloc(sizeof(WBOBJ));
				memcpy(pwbo, pwboParent, sizeof(WBOBJ));
				pwbo->id = id;

			} else {

				wbError(TEXT(__FUNCTION__), MB_ICONWARNING, TEXT("Menu item #%d not found in menu #%d"), id, (int)pwboParent->hwnd);
				return NULL;
			}
		} else {
			wbError(TEXT(__FUNCTION__), MB_ICONWARNING, TEXT("Menu object does not have a valid menu handle"));
			return NULL;
		}

	} else if(!IsWindow(pwboParent->hwnd)) {			// Is it a window?

		wbError(TEXT(__FUNCTION__), MB_ICONWARNING, TEXT("Object does not have a valid window handle"));
		return NULL;

	} else {											// Yes, it's a window

		switch(pwboParent->uClass) {

			case ToolBar:								// It's a toolbar

				pwbo = wbMalloc(sizeof(WBOBJ));
				pwbo->hwnd = pwboParent->hwnd;
				pwbo->id = id;
				pwbo->uClass = pwboParent->uClass;
				pwbo->item = -1;
				pwbo->subitem = -1;
				pwbo->style = 0;
				pwbo->parent = pwboParent;
				pwbo->pszCallBackFn = NULL;
				pwbo->pszCallBackObj = NULL;
				pwbo->lparam = 0;
				pwbo->pbuffer = NULL;
				break;

			default:									// Regular window classes
				{
					HWND hCtrl;

					hCtrl = GetDlgItem(pwboParent->hwnd, id);

					if(hCtrl) {

						// It's a regular control

						if(IsWindow(hCtrl)) {
							pwbo = (PWBOBJ)GetWindowLong(hCtrl, GWL_USERDATA);
						} else {
							pwbo = wbMalloc(sizeof(WBOBJ));
							pwbo->hwnd = hCtrl;
							pwbo->id = id;
							pwbo->uClass = pwboParent->uClass;
							pwbo->item = -1;
							pwbo->subitem = -1;
							pwbo->style = 0;
							pwbo->parent = pwboParent;
							pwbo->pszCallBackFn = NULL;
							pwbo->pszCallBackObj = NULL;
							pwbo->lparam = 0;
							pwbo->pbuffer = NULL;
						}

					} else {

						// Is its parent a tab control?

						if(pwboParent->uClass == TabControl) {

							// Yes, let's try to find it

							PTABDATA pTabData;
							UINT i;

							pTabData = (PTABDATA)pwboParent->lparam;

							for(i = 0; i < pTabData->nCtrls; i++) {
								if(pTabData->ctrl[i].id == (UINT)id) {

									hCtrl = pTabData->ctrl[i].hwnd;

									if(IsWindow(hCtrl)) {
										pwbo = (PWBOBJ)GetWindowLong(hCtrl, GWL_USERDATA);
									} else {
										pwbo = NULL;
									}
									break;
								}
							}

						} else {

							// No, control does not exist

							TCHAR szText[256];

							wbGetText(pwboParent, szText, 255, -1);
							wbError(TEXT(__FUNCTION__), MB_ICONWARNING, TEXT("Control #%d not found in window '%s'"), id, szText);
							return NULL;
						}
					}
				}
		} // switch
	} // else

	return pwbo;
}

/* Create status bar parts */

BOOL wbSetStatusBarParts(PWBOBJ pwbo, int nParts, int *aWidths)
{
	if(!wbIsWBObj(pwbo, TRUE))		// Is it a valid control?
		return FALSE;

	if(pwbo->uClass != StatusBar)
		return FALSE;

	nParts = min(max(0, nParts), 255);

	return SendMessage(pwbo->hwnd, SB_SETPARTS, nParts, (LPARAM)aWidths);
}

BOOL wbSetText(PWBOBJ pwbo, LPCTSTR pszSourceText, int nItem, BOOL bTooltip)
{
	TCHAR *pszText;

	if(!wbIsWBObj(pwbo, TRUE))					// Is it a valid control?
		return FALSE;

	wbConvertLineBreaks(&pszText, pszSourceText);

	if(bTooltip) {
		HWND hwndTT;
		TOOLINFO ti;

		hwndTT = (HWND)M_ToolTipWnd;
		if(!hwndTT)
			return FALSE;

		ti.cbSize = sizeof(TOOLINFO);
		ti.uFlags = 0;
		ti.hwnd	= pwbo->hwnd;
		ti.uId	= 0;
		ti.hinst = NULL;
		ti.lpszText	= (LPTSTR)pszText;

		SendMessage(hwndTT, TTM_UPDATETIPTEXT, 0, (LPARAM)&ti);
		return TRUE;

	} else {

		switch(pwbo->uClass) {

			case Spinner:

				// Change text of "buddy" control. The buddy is the edit box attached to the Up-down control
				{
					HWND hBuddy = (HWND)SendMessage(pwbo->hwnd, UDM_GETBUDDY, 0, 0);

					if(hBuddy)
						return SetWindowText(hBuddy, pszText);
					else
						return FALSE;
				}
				break;

			case TabControl:

				return wbSetTabControlText(pwbo, pszText);

			case Menu:

				pwbo->id = nItem;
				return wbSetMenuItemText(pwbo, pszText);

			case HTMLControl:
	//			if(nItem)
	//				return DisplayHTMLPage(pwbo, pszText);
	//			else
					return DisplayHTMLString(pwbo, pszText);

			case StatusBar:
				if(SendMessage(pwbo->hwnd, SB_GETPARTS, 0, 0) > 1)
					return SendMessage(pwbo->hwnd, SB_SETTEXT, nItem | 0, (LPARAM)(pszText ? pszText : TEXT("")));
				else
					return SetWindowText(pwbo->hwnd, pszText);

			default:

				return SetWindowText(pwbo->hwnd, pszText);
		}
	}
}

/*
	Limitation: Treeview items <= 1024 characters
*/

UINT wbGetTextLength(PWBOBJ pwbo, int nIndex)
{
	if(!wbIsWBObj(pwbo, TRUE))					// Is it a valid control?
		return FALSE;

	switch(pwbo->uClass) {

		case Spinner:
			{
				// Measure length of text of "buddy" control. The buddy is the edit box attached to the Up-down control

				HWND hBuddy;

				hBuddy = (HWND)SendMessage(pwbo->hwnd, UDM_GETBUDDY, 0, 0);
				return GetWindowTextLength(hBuddy);
			}

		case ListBox:
			{
				if(nIndex < 0) {
					if(pwbo->item < 0) {
						nIndex = SendMessage(pwbo->hwnd, LB_GETCURSEL, 0, 0);
						if(nIndex == LB_ERR)
							return 0;
					} else {
						nIndex = pwbo->item;
					}
				}
				return SendMessage(pwbo->hwnd, LB_GETTEXTLEN, nIndex, 0);
			}

		case ComboBox:
			{
				if(nIndex < 0) {
					if(!(pwbo->style & WBC_READONLY))
						return SendMessage(pwbo->hwnd, WM_GETTEXTLENGTH, 0, 0);
					else {
						if(pwbo->item < 0) {
							nIndex = SendMessage(pwbo->hwnd, CB_GETCURSEL, 0, 0);
							if(nIndex == CB_ERR)
								return 0;
						} else {
							nIndex = pwbo->item;
						}
					}
				}
				return SendMessage(pwbo->hwnd, CB_GETLBTEXTLEN, nIndex, 0);
			}

		case TreeView:
			{
				TV_ITEM tvi;
				TCHAR szText[MAX_ITEM_STRING];

				tvi.hItem = TreeView_GetSelection(pwbo->hwnd);
				if(!tvi.hItem)
					return 0;

				tvi.pszText = szText;
				tvi.cchTextMax = MAX_ITEM_STRING;
				tvi.mask = TVIF_TEXT;
				TreeView_GetItem(pwbo->hwnd, &tvi);
				if(tvi.pszText && *tvi.pszText)
					return wcslen(tvi.pszText);
				else
					return 0;
			}

		default:
			/*
				TODO: Trying to fix control text length

				According to WinHelp for GetWindowTextLength, "To obtain the exact length of
				the text, use (...) the GetWindowText function." This makes no practical difference
				when allocating memory for strings, but may return incorrect results in wb_get_size().
			{
				char *pszText;
				int nLen;

				pszText = wbMalloc(32767);
				nLen = GetWindowText(pwbo->hwnd, pszText, 32767);
				wbFree(pszText);
				return nLen;
			}*/
			return GetWindowTextLength(pwbo->hwnd);
	}
}

DWORD CALLBACK EditStreamOutCallback(DWORD_PTR dwCookie, LPBYTE pbBuff, LONG cb, LONG *pcb)
{
	char **rtf = (char**)dwCookie;
	*rtf = wbMalloc(cb+1);
	//strcpy_s(*rtf, cb, pbBuff);
	memset(*rtf, 0, cb+1);
	memcpy(*rtf, pbBuff, cb);
	*pcb = cb;
	return 0;
}

BOOL wbGetRtfText(PWBOBJ pwbo, char **unc)
{
	if (!wbIsWBObj(pwbo, TRUE))					// Is it a valid control?
		return FALSE;

	switch (pwbo->uClass) {
		case RTFEditBox: 
		{
			EDITSTREAM es = { 0 }; 
			es.dwCookie = (DWORD_PTR)unc;
			es.pfnCallback = &EditStreamOutCallback;
			SendMessage(pwbo->hwnd, EM_STREAMOUT, (WPARAM)(SF_RTF), (LPARAM)&es); 
			//use SF_TEXT|SF_UNICODE  then you need 
			return TRUE;
		}
	}
	return FALSE;
}


BOOL wbGetText(PWBOBJ pwbo, LPTSTR pszText, UINT nMaxChars, int nIndex)
{
	if(!wbIsWBObj(pwbo, TRUE))					// Is it a valid control?
		return FALSE;

	if(!nMaxChars)
		return FALSE;

	*pszText = '\0';

	switch(pwbo->uClass) {

		case Spinner:
			{
				// Reads text of "buddy" control. The buddy is the edit box attached to the Up-down control
				HWND hBuddy;

				hBuddy = (HWND)SendMessage(pwbo->hwnd, UDM_GETBUDDY, 0, 0);
				GetWindowText(hBuddy, pszText, nMaxChars);
				return TRUE;
			}

		case ListBox:
			{
				BOOL bRet;

				if(nIndex < 0) {
					if(pwbo->item < 0) {
						nIndex = SendMessage(pwbo->hwnd, LB_GETCURSEL, 0, 0);
						if(nIndex == LB_ERR)
							return FALSE;
					} else {
						nIndex = pwbo->item;
					}
				}

				bRet = SendMessage(pwbo->hwnd, LB_GETTEXT, nIndex, (LPARAM)(LPTSTR)pszText);
				return (bRet != LB_ERR);
			}

		case ComboBox:
			{
				BOOL bRet;

				if(nIndex < 0) {
					if(!(pwbo->style & WBC_READONLY)) {
						SendMessage(pwbo->hwnd, WM_GETTEXT, nMaxChars, (LPARAM)pszText);
						return TRUE;
					} else {
						if(pwbo->item < 0) {
							nIndex = SendMessage(pwbo->hwnd, CB_GETCURSEL, 0, 0);
							if(nIndex == CB_ERR) {
								return FALSE;
							}
						} else {
							nIndex = pwbo->item;
						}
					}
				}

				bRet = SendMessage(pwbo->hwnd, CB_GETLBTEXT, nIndex, (LPARAM)(LPTSTR)pszText);
				return (bRet != CB_ERR);
			}

		case TreeView:
			{
				TV_ITEM tvi;

				tvi.hItem = TreeView_GetSelection(pwbo->hwnd);
				if(!tvi.hItem)
					return FALSE;
				tvi.pszText = pszText;
				tvi.cchTextMax = nMaxChars;
				tvi.mask = TVIF_TEXT;
				TreeView_GetItem(pwbo->hwnd, &tvi);
				return (tvi.pszText && *tvi.pszText);
			}

		default:

			GetWindowText(pwbo->hwnd, pszText, nMaxChars);
			return TRUE;
	}
}

BOOL wbSetEnabled(PWBOBJ pwbo, BOOL bState)
{
	BOOL bRet;

	if(!wbIsWBObj(pwbo, TRUE))					// Is it a valid control?
		return FALSE;

	// Change status flag

	if(pwbo->uClass == Menu && IsMenu((HMENU)pwbo->hwnd)) {		// Is it a menu item?

		MENUITEMINFO mi;

		mi.cbSize = sizeof(MENUITEMINFO);
		mi.fMask = MIIM_STATE;
		mi.fState = bState ? MFS_ENABLED : (MFS_DISABLED | MFS_GRAYED);

		bRet = SetMenuItemInfo((HMENU)pwbo->hwnd, pwbo->id, FALSE, &mi);
		if(!bRet)
			wbError(TEXT(__FUNCTION__), MB_ICONWARNING, TEXT("Could not set state of menu item #%d"), pwbo->id);

	} else if(IsWindow(pwbo->hwnd)) {		// Is it a window?

		switch(pwbo->uClass) {

			case ToolBar:
				bRet = SendMessage((HWND)pwbo->hwnd, TB_ENABLEBUTTON, pwbo->id, MAKELONG(bState, 0));
				break;

			default:
				EnableWindow(pwbo->hwnd, bState ? 1 : 0);
				// Should NOT use the return value from EnableWindow()
				bRet = TRUE;
				break;
		}

		// Force draw custom-drawn controls when they change state

		if(bRet && (pwbo->uClass == ImageButton))
			InvalidateRect(pwbo->hwnd, NULL, FALSE);

	} else
		bRet = FALSE;

	// Change style according to bState

	if(bRet && bState)
		pwbo->style &= ~WBC_DISABLED;
	else
		pwbo->style |= WBC_DISABLED;

	return bRet;
}

BOOL wbGetEnabled(PWBOBJ pwbo)
{
	if(!wbIsWBObj(pwbo, TRUE))					// Is it a valid control?
		return FALSE;

	if(pwbo->uClass == Menu && IsMenu((HMENU)pwbo->hwnd)) {		// Is it a menu item?

		MENUITEMINFO mi;

		mi.cbSize = sizeof(MENUITEMINFO);
		mi.fMask = MIIM_STATE;

		if(!GetMenuItemInfo((HMENU)pwbo->hwnd, pwbo->id, FALSE, &mi)) {
			wbError(TEXT(__FUNCTION__), MB_ICONWARNING, TEXT("Could not get state of menu item #%d."), pwbo->id);
			return FALSE;
		}
		return (mi.fState & (MFS_DISABLED | MFS_GRAYED) ? FALSE : TRUE);

	} else if(IsWindow(pwbo->hwnd)) {		// Is it a window?

		switch(pwbo->uClass) {
			case ToolBar:								// Toolbar
				return SendMessage(pwbo->hwnd, TB_ISBUTTONENABLED, pwbo->id, 0);
			default:
				return IsWindowEnabled(pwbo->hwnd);	// The window
		}
	} else
		return FALSE;
}

DWORD wbGetSelected(PWBOBJ pwbo)
{
	if(!wbIsWBObj(pwbo, TRUE))					// Is it a valid control?
		return FALSE;

	switch(pwbo->uClass) {

		case ComboBox:

			// lparam < 0 means the child edit control was modified

			if(pwbo->lparam < 0)
				return -1;
			else
				return SendMessage(pwbo->hwnd, CB_GETCURSEL, 0, 0);

		case ListBox:

			return SendMessage(pwbo->hwnd, LB_GETCURSEL, 0, 0);

		case TabControl:

			return SendMessage(pwbo->hwnd, TCM_GETCURSEL, 0, 0);

		case CheckBox:
		case RadioButton:

			// 0 means unchecked, 1 means checked.

			return (SendMessage(pwbo->hwnd, BM_GETCHECK, 0, 0) == BST_CHECKED);

		case EditBox:
		case HyperLink:
		case ImageButton:
		case Label:
		case Menu:
		case PushButton:
		case RTFEditBox:
		case Slider:
		case Spinner:
			return 0;

		default:
			return 0;
	}
}

DWORD wbGetItemCount(PWBOBJ pwbo)
{
	if(!wbIsWBObj(pwbo, TRUE))					// Is it a valid control?
		return FALSE;

	switch(pwbo->uClass) {

		case ListView:
			return SendMessage(pwbo->hwnd, LVM_GETITEMCOUNT, 0, 0);

		case ListBox:
			return SendMessage(pwbo->hwnd, LB_GETCOUNT, 0, 0);

		case ComboBox:
			return SendMessage(pwbo->hwnd, CB_GETCOUNT, 0, 0);

		default:
			return 0;
	}
}

BOOL wbDeleteItems(PWBOBJ pwbo, BOOL bClearAll)
{
	if(!wbIsWBObj(pwbo, TRUE))					// Is it a valid control?
		return FALSE;

	switch(pwbo->uClass) {

		case ListBox:
			if(!bClearAll)
				SendMessage(pwbo->hwnd, LB_DELETESTRING, pwbo->item, 0);
			else
				SendMessage(pwbo->hwnd, LB_RESETCONTENT, 0, 0);
			break;

		case ComboBox:
			if(!bClearAll)
				SendMessage(pwbo->hwnd, CB_DELETESTRING, pwbo->item, 0);
			else
				SendMessage(pwbo->hwnd, CB_RESETCONTENT, 0, 0);
			break;

		case ListView:
			if(!bClearAll)
				return ListView_DeleteItem(pwbo->hwnd, pwbo->item);
			else
				return SendMessage(pwbo->hwnd, LVM_DELETEALLITEMS, 0, 0);
			break;

		case TreeView:

			if(!bClearAll) {
				if(pwbo->item)
					return SendMessage(pwbo->hwnd, TVM_DELETEITEM, 0, (LPARAM)(HTREEITEM)pwbo->item);
			} else {
				PTREEDATA ptrdt	= (PTREEDATA)pwbo->lparam;
				int i;

				// Clear the treeview data

				ptrdt = wbMalloc(sizeof(TREEDATA));

				ptrdt->hLast = TVI_ROOT;
				for(i = 0; i < MAX_TREEVIEW_LEVELS; i++) {
					ptrdt->hParent[i] = TVI_ROOT;
					ptrdt->bCustomImages[i] = FALSE;
				}
				ptrdt->nLastLevel = 0;
				pwbo->lparam = (LPARAM)ptrdt;
			}
			return TreeView_DeleteAllItems(pwbo->hwnd);
	}
	return TRUE;
}

BOOL wbGetVisible(PWBOBJ pwbo)
{
	if(!wbIsWBObj(pwbo, TRUE))					// Is it a valid control?
		return FALSE;

	return IsWindowVisible(pwbo->hwnd);
}

BOOL wbSetVisible(PWBOBJ pwbo, BOOL bState)
{
	if(!wbIsWBObj(pwbo, TRUE))					// Is it a valid control?
		return FALSE;

	SetWindowPos(pwbo->hwnd, 0, 0, 0, 0, 0, (bState ? SWP_SHOWWINDOW : SWP_HIDEWINDOW) | SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER);
	return TRUE;
}

PWBOBJ wbGetFocus(void)
{
	return wbGetWBObj(GetFocus());
}

BOOL wbSetFocus(PWBOBJ pwbo)
{
	if(!wbIsWBObj(pwbo, TRUE))					// Is it a valid control?
		return FALSE;

	switch(pwbo->uClass) {

		case EditBox:
			SendMessage(pwbo->hwnd, EM_SETSEL, 0, -1);	// Select all
			break;
	}

	return (SetFocus(pwbo->hwnd) != NULL);
}

// Sets the styles of some controls

BOOL wbSetStyle(PWBOBJ pwbo, DWORD dwWBStyle, BOOL bSet)
{
	switch(pwbo->uClass) {

		case AppWindow:
		case ResizableWindow:
		case PopupWindow:
		case NakedWindow:

			if(bSet) {
				if(BITTEST(dwWBStyle, WBC_TOP))
					SetWindowPos(pwbo->hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
			} else {
				if(BITTEST(dwWBStyle, WBC_TOP))
					SetWindowPos(pwbo->hwnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
			}
			break;

		case ListView:

			if(bSet) {
				if(BITTEST(dwWBStyle, WBC_LINES))
					SendMessage(pwbo->hwnd, LVM_SETEXTENDEDLISTVIEWSTYLE, LVS_EX_GRIDLINES, LVS_EX_GRIDLINES);
				if(BITTEST(dwWBStyle, WBC_CHECKBOXES))
					SendMessage(pwbo->hwnd, LVM_SETEXTENDEDLISTVIEWSTYLE, LVS_EX_CHECKBOXES, LVS_EX_CHECKBOXES);

			} else {
				if(BITTEST(dwWBStyle, WBC_LINES))
					SendMessage(pwbo->hwnd, LVM_SETEXTENDEDLISTVIEWSTYLE, LVS_EX_GRIDLINES, 0);
				if(BITTEST(dwWBStyle, WBC_CHECKBOXES))
					SendMessage(pwbo->hwnd, LVM_SETEXTENDEDLISTVIEWSTYLE, LVS_EX_CHECKBOXES, 0);
			}
			break;

		case TreeView:

			if(bSet) {
				if(BITTEST(dwWBStyle, WBC_LINES))
					SetWindowLong(pwbo->hwnd, GWL_STYLE, GetWindowLong(pwbo->hwnd, GWL_STYLE) | (TVS_HASLINES));
			} else {
				if(BITTEST(dwWBStyle, WBC_LINES)) {
					SetWindowLong(pwbo->hwnd, GWL_STYLE, GetWindowLong(pwbo->hwnd, GWL_STYLE) & ~(TVS_HASLINES));
				}
			}
			break;

		case Slider:

			if(bSet) {
				if(BITTEST(dwWBStyle, WBC_LINES))
					SendMessage(pwbo->hwnd, TBM_SETTICFREQ, 10, 1);
			} else {
				if(BITTEST(dwWBStyle, WBC_LINES)) {
					SendMessage(pwbo->hwnd, TBM_CLEARTICS, 1, 0);
				}
			}
			break;

		default:
			return FALSE;
	}
	return TRUE;
}

BOOL wbSetValue(PWBOBJ pwbo, DWORD dwValue)
{
	if(!wbIsWBObj(pwbo, TRUE))					// Is it a valid control?
		return FALSE;

	switch(pwbo->uClass) {

		case ListBox:
			if(pwbo->item < 0)
				pwbo->item = SendMessage(pwbo->hwnd, LB_GETCURSEL, 0, 0);
			return SendMessage(pwbo->hwnd, LB_SETITEMDATA, pwbo->item, dwValue);

		case ComboBox:
			if(pwbo->item < 0)
				pwbo->item = SendMessage(pwbo->hwnd, LB_GETCURSEL, 0, 0);
			return SendMessage(pwbo->hwnd, CB_SETITEMDATA, pwbo->item, dwValue);

		case Slider:
			return SendMessage(pwbo->hwnd, TBM_SETPOS, TRUE, dwValue);

		case Gauge:
			return SendMessage(pwbo->hwnd, PBM_SETPOS, (WPARAM)dwValue, 0);

		case Spinner:
			return SendMessage(pwbo->hwnd, UDM_SETPOS, 0, MAKELONG((short)dwValue, 0));

		case ScrollBar:
			return SendMessage(pwbo->hwnd, SBM_SETPOS, (WPARAM)dwValue, TRUE);

		case Calendar:
			return SetCalendarTime(pwbo, dwValue);

		// Set value as text

		case EditBox:
		case RTFEditBox:
		case Label:
			{
				TCHAR szText[32];

				_itow(dwValue, szText, 10);
				return SetWindowText(pwbo->hwnd, szText);
			}

		// Check / uncheck button

		case PushButton:
		case CheckBox:
		case RadioButton:
			return SendMessage(pwbo->hwnd, BM_SETCHECK, dwValue, 0);

		// Check/uncheck menu item

		case Menu:
			return wbSetMenuItemChecked(pwbo, dwValue);
	}

	return TRUE;
}

BOOL wbSetRange(PWBOBJ pwbo, LONG lMin, LONG lMax)
{
	if(!wbIsWBObj(pwbo, TRUE))					// Is it a valid control?
		return FALSE;

	switch(pwbo->uClass) {

		// Set range

		case Slider:
			SendMessage(pwbo->hwnd, TBM_SETRANGEMIN, FALSE, lMin);
			SendMessage(pwbo->hwnd, TBM_SETRANGEMAX, TRUE, lMax);
			break;

		case Gauge:
			SendMessage(pwbo->hwnd, PBM_SETRANGE32, lMin, lMax);
			break;

		case Spinner:		// Inverted so that the control increases upwards
			SendMessage(pwbo->hwnd, UDM_SETRANGE32, lMin, lMax);
			break;

		case ScrollBar:
			SendMessage(pwbo->hwnd, SBM_SETRANGE, lMin, lMax);
			break;
	}

	return TRUE;
}

DWORD wbGetValue(PWBOBJ pwbo)
{
	if(!wbIsWBObj(pwbo, TRUE))					// Is it a valid control?
		return 0;

	switch(pwbo->uClass) {

		case Calendar:
			return GetCalendarTime(pwbo);

		case ListBox:
			if(pwbo->item < 0)
				pwbo->item = SendMessage(pwbo->hwnd, LB_GETCURSEL, 0, 0);
			return SendMessage(pwbo->hwnd, LB_GETITEMDATA, pwbo->item, 0);

		case ComboBox:
			if(pwbo->item < 0)
				pwbo->item = SendMessage(pwbo->hwnd, CB_GETCURSEL, 0, 0);

			// lparam < 0 means the child edit control was modified
			if(pwbo->lparam < 0)
				return -1;
			else
				return SendMessage(pwbo->hwnd, CB_GETITEMDATA, pwbo->item, 0);

		case EditBox:
		case Label:
		case PushButton:
		case RTFEditBox:

			{
				TCHAR szText[65];

				wbGetText(pwbo, szText, 64, -1);
				return _wtoi(szText);
			}

		case Slider:

			return SendMessage(pwbo->hwnd, TBM_GETPOS, 0, 0);

		case Spinner:

			return LOWORD(SendMessage(pwbo->hwnd, UDM_GETPOS, 0, 0));

		case ScrollBar:

			return SendMessage(pwbo->hwnd, SBM_GETPOS, 0, 0);

		case CheckBox:
		case RadioButton:

			// 0 means unchecked, 1 means checked.

			return (SendMessage(pwbo->hwnd, BM_GETCHECK, 0, 0) == BST_CHECKED);

		case TreeView:

			{
	    		TV_ITEM tvi;

				tvi.hItem = TreeView_GetSelection(pwbo->hwnd);
				return wbGetTreeViewItemValue(pwbo, tvi.hItem);
			}

		case ListView:
			return wbGetListViewItemChecked(pwbo, pwbo->item);

		case Menu:
			return wbGetMenuItemChecked(pwbo);

		default:
			return 0;
	}
}

BOOL wbSetImage(PWBOBJ pwbo, HANDLE hImage, COLORREF clTransp, LPARAM lParam)
{
	BOOL bRet = TRUE;
	LONG lStyle;

	if(!wbIsWBObj(pwbo, TRUE))					// Is it a valid control?
		return FALSE;

	if(!hImage)
		return FALSE;

	switch(pwbo->uClass) {

		case Menu:
			wbSetMenuItemImage(pwbo, hImage);
			break;

		case PushButton:

			lStyle = GetWindowLong(pwbo->hwnd, GWL_STYLE) & ~(BS_ICON | BS_BITMAP);
			bRet = TRUE;
			if(IsIcon(hImage)) {
				SetWindowLong(pwbo->hwnd, GWL_STYLE, lStyle | BS_ICON);
				SendMessage(pwbo->hwnd, BM_SETIMAGE, IMAGE_ICON, (LPARAM)hImage);
				break;
			} else if(IsBitmap(hImage)) {
				SetWindowLong(pwbo->hwnd, GWL_STYLE, lStyle | BS_BITMAP);
				bRet = SetTransparentBitmap(pwbo->hwnd, (HBITMAP)hImage, FALSE, clTransp);
				break;
			} else
				bRet = FALSE;
			break;

		case ImageButton:

			if(IsBitmap(hImage)) {

				HIMAGELIST hi;

				hi = (HIMAGELIST)M_hiImageList;		// Retrieve ImageList from control
				if(clTransp == NOCOLOR)
					ImageList_Add(hi, hImage, NULL);
				else
					ImageList_AddMasked(hi, hImage, clTransp);
				break;
			} else
				bRet = FALSE;

			break;

		case Frame:

			lStyle = GetWindowLong(pwbo->hwnd, GWL_STYLE) & ~(SS_ICON | SS_BITMAP);
			bRet = TRUE;
			if(IsIcon(hImage)) {
				SetWindowLong(pwbo->hwnd, GWL_STYLE, lStyle | SS_ICON);
				SendMessage(pwbo->hwnd, STM_SETIMAGE, IMAGE_ICON, (LPARAM)hImage);
				InvalidateRect(pwbo->hwnd, NULL, FALSE);
				break;
			} else if(IsBitmap(hImage)) {
				SetWindowLong(pwbo->hwnd, GWL_STYLE, lStyle | SS_BITMAP);
				SetTransparentBitmap(pwbo->hwnd, (HBITMAP)hImage, TRUE, clTransp);
				InvalidateRect(pwbo->hwnd, NULL, FALSE);
				break;
			} else
				bRet = FALSE;
			break;

		case ListView:

			lParam = MAX(1, lParam);	// Image count
			bRet = wbCreateListViewImageList(pwbo, hImage, lParam, clTransp);
			break;

		case TreeView:

			lParam = MAX(1, lParam);	// Image count
			bRet = wbCreateTreeViewImageList(pwbo, hImage, lParam, clTransp);
			break;

		default:

			// Set the window icon

			SendMessage(pwbo->hwnd, WM_SETICON, ICON_BIG, (WPARAM)hImage);
			bRet = TRUE;

			// Change the icon in the taskbar if it is the case

			if((pwbo->style & WBC_TASKBAR) && IsIconic(pwbo->hwnd))
				bRet = SetTaskBarIcon(pwbo->hwnd, TRUE);

	} // end switch

	return bRet;
}

BOOL wbCreateItem(PWBOBJ pwbo, LPCTSTR pszItemText)
{
	LRESULT lRet;

	if(!wbIsWBObj(pwbo, TRUE))					// Is it a valid control?
		return FALSE;

	if(!pszItemText || !*pszItemText)
		return FALSE;

	switch(pwbo->uClass) {

		case ListBox:
			lRet = SendMessage(pwbo->hwnd, LB_ADDSTRING, 0, (LPARAM)pszItemText);
			return ((lRet != LB_ERR) && (lRet != LB_ERRSPACE));

		case ComboBox:

			lRet = SendMessage(pwbo->hwnd, CB_ADDSTRING, 0, (LPARAM)pszItemText);
			if(lRet == CB_ERR || lRet == CB_ERRSPACE)
				return FALSE;
			lRet = SendMessage(pwbo->hwnd, CB_SETCURSEL, 0, 0);
			return (lRet != CB_ERR);

		case TabControl:
			return wbCreateTabItem(pwbo, pszItemText);

		default:
			return FALSE;
	}
}

/* Refresh a window or control. */

BOOL wbRefreshControl(PWBOBJ pwbo, int xpos, int ypos, int nWidth, int nHeight, BOOL bNow)
{
	BOOL bRet;
	RECT rc;

	if(!wbIsWBObj(pwbo, TRUE))					// Is it a valid control?
		return FALSE;

	if(!IsWindow(pwbo->hwnd))
		return FALSE;

	if(!((pwbo->style & WBC_NOTIFY) && (pwbo->lparam & WBC_REDRAW) && pwbo->pszCallBackFn && *pwbo->pszCallBackFn)) {

		// Not custom drawn

		if(nWidth <= 0 || nHeight <= 0) {
			bRet = InvalidateRect(pwbo->hwnd, NULL, TRUE);	// Whole control
		} else {
			rc.left = xpos;
			rc.top = ypos;
			rc.right = xpos + nWidth;
			rc.bottom = ypos + nHeight;
			bRet = InvalidateRect(pwbo->hwnd, &rc, TRUE);
		}

		// Force an immediate update

		if(bNow) {
			bRet = UpdateWindow(pwbo->hwnd);
		}

	} else {

		// Custom drawn

		if(nWidth <= 0 || nHeight <= 0) {

			// We use the callback function to let the user do the drawing

			// *** Should probably use pwbo->parent for child controls, but the
			// *** use of parameter pwboParent in wbCallUserFunction() is not clear
			wbCallUserFunction(pwbo->pszCallBackFn, pwbo->pszCallBackObj, pwbo, pwbo,
				IDDEFAULT, WBC_REDRAW, (LPARAM)pwbo->pbuffer, 0);

			bRet = InvalidateRect(pwbo->hwnd, NULL, TRUE);

			// Force an immediate update

			if(bNow) {
				bRet = UpdateWindow(pwbo->hwnd);
			}

		} else {

			rc.left = xpos;
			rc.top = ypos;
			rc.right = xpos + nWidth;
			rc.bottom = ypos + nHeight;

			// We use the callback function to let the user do the drawing

			// *** Should probably use pwbo->parent for child controls, but the
			// *** use of parameter pwboParent in wbCallUserFunction() is not clear
			wbCallUserFunction(pwbo->pszCallBackFn, pwbo->pszCallBackObj, pwbo, pwbo,
				IDDEFAULT, WBC_REDRAW, (LPARAM)pwbo->pbuffer,
				(LPARAM)&rc);

			bRet = InvalidateRect(pwbo->hwnd, &rc, TRUE);

			// Force an immediate update

			if(bNow) {
				bRet = UpdateWindow(pwbo->hwnd);
			}
		}
	}
	return bRet;
}

//------------------------------------------- FUNCTIONS PUBLIC TO WINBINDER ONLY

BOOL IsIcon(HANDLE handle)
{
	ICONINFO ii;

	return GetIconInfo((HICON)handle, &ii);
}

BOOL IsBitmap(HANDLE handle)
{
	SIZE siz;

	return GetBitmapDimensionEx(handle, &siz);
}

/* Create ImageButton class */

BOOL RegisterImageButtonClass(void)
{
	WNDCLASS wc;

	memset(&wc, 0, sizeof(WNDCLASS));
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = (WNDPROC)ImageButtonProc;
	wc.hInstance = hAppInstance;
	wc.hbrBackground = NULL;
	wc.lpszClassName = IMAGE_BUTTON_CLASS;
	wc.lpszMenuName = NULL;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hIcon = NULL;
	wc.cbWndExtra = DLGWINDOWEXTRA;

	if(!RegisterClass(&wc))
		return FALSE;

	wbSetCursor((PWBOBJ)ImageButton, NULL, 0);

	return TRUE;
}

/* Associate the tooltip text pszTooltip to control pwbo. If pszTooltip is NULL, create an empty tooltip */

HWND CreateToolTip(PWBOBJ pwbo, LPCTSTR pszTooltip)
{
	HWND hwndTT;
	TOOLINFO ti;

	// Create the ToolTip control.

	hwndTT = CreateWindow(TOOLTIPS_CLASS, NULL,
		TTS_ALWAYSTIP,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
		pwbo->hwnd,
		NULL, NULL, NULL);

	// Create a tool that contains the desired control (pwbo->hwnd)

	ti.cbSize = sizeof(TOOLINFO);
	ti.uFlags = TTF_SUBCLASS;
	ti.hwnd	= pwbo->hwnd;
	ti.uId	= 0;
	ti.hinst = NULL;

	// Get the window text

//	ti.lpszText	= wbMalloc(256);

	if(!pszTooltip || !*pszTooltip)
		ti.lpszText = TEXT("");//'\0';//GetWindowText(pwbo->hwnd, ti.lpszText, 255);
	else
		ti.lpszText = (LPTSTR)pszTooltip;

	// It's not completely clear on the doc: Must have the line below, otherwise the tooltip won't appear.
	// The MSDN doc, in the "Messaging and Notification" topic, gets close:
	// http://msdn.microsoft.com/library/default.asp?url=/library/en-us/shellcc/platform/commctls/tooltip/tooltip.asp
	// It says "The tool is a control or is defined as a rectangle in the tool's TOOLINFO structure."
	// I add: "For controls, the rect member of the TOOLINFO structure must be set to the size of the client area."

	GetClientRect(pwbo->hwnd, &ti.rect);

	// This does not actually set the control width, it rather instructs it
	// to recognize line breaks

	SendMessage(hwndTT, TTM_SETMAXTIPWIDTH, 0, 640);

	// Add the tool to the control

	if(!SendMessage(hwndTT, TTM_ADDTOOL, 0, (LPARAM)&ti))
		return NULL;

	M_ToolTipWnd = (LONG)hwndTT;
	return hwndTT;
}

//------------------------------------------------------------ PRIVATE FUNCTIONS

/*

For opaque bitmaps, set clTransp to NOCOLOR

*/

static BOOL SetTransparentBitmap(HWND hwnd, HBITMAP hbmBits, BOOL bStatic, COLORREF clTransp)
{
//	static HBITMAP hbm = NULL, hbmMask;
	HBITMAP hbm, hbmMask = NULL, hbmOld;
	HDC hdc;
	RECT rc;
	HBRUSH hbr;
	BITMAP bm;

//	if(clTransp == NOCOLOR) {
//		SendMessage(hwnd, bStatic ? STM_SETIMAGE : BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hbmBits);
//		return TRUE;
//	}

	// Create an empty bitmap the same size as hwnd

	GetClientRect(hwnd, &rc);

	hbm = wbCreateBitmap(rc.right - rc.left, rc.bottom - rc.top, NULL, NULL);
	hdc = wbCreateBitmapDC(hbm);

	// Paint hbmBits with a mask

	if(clTransp != NOCOLOR)
		hbmMask = wbCreateMask(hbmBits, clTransp);
	hbr = CreateSolidBrush(GetSysColor(COLOR_BTNFACE));
	SelectObject(hdc, hbr);
	SelectObject(hdc, GetStockObject(WHITE_PEN));
	Rectangle(hdc, 0, 0, rc.right - rc.left, rc.bottom - rc.top);
	GetObject(hbmBits, sizeof(BITMAP), (LPVOID)&bm);
	if(clTransp == NOCOLOR) {
		wbCopyPartialBits(hdc, hbmBits,
			((rc.right - rc.left) - bm.bmWidth) / 2,
			((rc.bottom - rc.top) - bm.bmHeight) / 2,
			rc.right - rc.left, rc.bottom - rc.top, 0, 0);
	} else {
		wbMaskPartialBits(hdc, hbmBits, hbmMask,
			((rc.right - rc.left) - bm.bmWidth) / 2,
			((rc.bottom - rc.top) - bm.bmHeight) / 2,
			rc.right - rc.left, rc.bottom - rc.top, 0, 0);
	}

	// Deletes old handle, if any

	hbmOld = (HBITMAP)SendMessage(hwnd, bStatic ? STM_SETIMAGE : BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hbm);
	if(hbmOld)
		DeleteObject(hbmOld);

	// Cleanup

	DeleteObject(hbr);
//	if(hbmMask && (clTransp != NOCOLOR))
	if(hbmMask)
		DeleteObject(hbmMask);
	DeleteDC(hdc);

	return TRUE;
}

// Processing routine for Frame controls

static LRESULT CALLBACK FrameProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg) {

		case WM_SETFOCUS:
		case WM_HSCROLL:		// For scroll bars
		case WM_VSCROLL:
		case WM_COMMAND:		// Passes commands to parent window
		case WM_NOTIFY:
			{
				HWND hwndParent = GetParent(hwnd);

				if(hwndParent)
					SendMessage(hwndParent, msg, wParam, lParam);

				hCurrentDlg = hwnd;
			}
			break;
	}
	return CallWindowProc(lpfnFrameProcOld, hwnd, msg, wParam, lParam);
}

// Processing routine for EditBoxes

static LRESULT CALLBACK EditBoxProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg) {		

		case WM_SETFOCUS:
			{
				HWND hwndParent = GetParent(hwnd);
				if(hwndParent)
					SendMessage(hwndParent, WM_SETFOCUS, (WPARAM)hwnd, (LPARAM)wParam);
			}
			break;

		case WM_KEYDOWN:
		case WM_SYSKEYDOWN:
			{
				HWND hwndParent = GetParent(hwnd);

				if(hwndParent)
					SendMessage(hwndParent, WBWM_KEYDOWN, (WPARAM)hwnd, (LPARAM)wParam);
			}
			break;

		case WM_KEYUP:
		case WM_SYSKEYUP:
			{
				HWND hwndParent = GetParent(hwnd);

				if(hwndParent)
					SendMessage(hwndParent, WBWM_KEYUP, (WPARAM)hwnd, (LPARAM)wParam);
			}
			break;

		case WM_SETCURSOR:
			{
				PWBOBJ pwbo = wbGetWBObj(hwnd);
				if(!pwbo)
					break;

				if(M_nMouseCursor != 0) {
					SetCursor(M_nMouseCursor == -1 ? 0 : (HCURSOR)M_nMouseCursor);
					return TRUE;			// Must return here, not break
				} else {
					break;					// Normal behavior
				}
			}
			break;
	}
	return CallWindowProc(lpfnEditProcOld, hwnd, msg, wParam, lParam);
}

// Processing routine for InvisibleAreas

static LRESULT CALLBACK InvisibleProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg) {

		case WM_MOUSEMOVE:
			{
				PWBOBJ pwbobj = wbGetWBObj(hwnd);

				if(!pwbobj)
					break;

				if(BITTEST(pwbobj->style, WBC_NOTIFY) &&
					BITTEST(pwbobj->lparam, WBC_MOUSEMOVE)) {

					DWORD dwAlt = (GetKeyState(VK_MENU) < 0) ? WBC_ALT : 0;

					if(pwbobj && pwbobj->parent && pwbobj->parent->pszCallBackFn && *pwbobj->parent->pszCallBackFn) {
//printf("%08X %s\n", pwbobj->lparam, pwbobj->parent->pszCallBackFn);
						wbCallUserFunction(pwbobj->parent->pszCallBackFn, pwbobj->pszCallBackObj, pwbobj->parent, pwbobj, pwbobj->id,
						WBC_MOUSEMOVE | wParam | dwAlt, lParam, 0);
					}
				}
			}
			break;

		case WM_SETCURSOR:
			{
				PWBOBJ pwbo = wbGetWBObj(hwnd);
				if(!pwbo)
					break;

				if(M_nMouseCursor != 0) {
					SetCursor(M_nMouseCursor == -1 ? 0 : (HCURSOR)M_nMouseCursor);
					return TRUE;			// Must return here, not break
				} else {
					break;					// Normal behavior
				}
			}
			break;
	}
	return CallWindowProc(lpfnInvisibleProcOld, hwnd, msg, wParam, lParam);
}

// Processing routine for ImageButton

static LRESULT CALLBACK ImageButtonProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	PWBOBJ pwbo;

	switch(msg) {

		case WM_MOUSEMOVE:

			pwbo = wbGetWBObj(hwnd);
			{
				if(!(wParam & MK_LBUTTON)) {

					RECT rc;
					POINT pt;

					GetWindowRect(hwnd, &rc);
					pt.x = LOWORD(lParam) + rc.left;
					pt.y = HIWORD(lParam) + rc.top;

					if(PtInRect(&rc, pt)) {
						SetCapture(hwnd);
						InvalidateRect(hwnd, NULL, FALSE);
					} else {
						InvalidateRect(hwnd, NULL, FALSE);
						ReleaseCapture();
					}
				}

				if(BITTEST(pwbo->style, WBC_NOTIFY) &&
					BITTEST(pwbo->lparam, WBC_MOUSEMOVE)) {

					DWORD dwAlt = (GetKeyState(VK_MENU) < 0) ? WBC_ALT : 0;

					if(pwbo && pwbo->parent && pwbo->parent->pszCallBackFn && *pwbo->parent->pszCallBackFn) {
						wbCallUserFunction(pwbo->parent->pszCallBackFn, pwbo->pszCallBackObj, pwbo->parent, pwbo, pwbo->id,
						WBC_MOUSEMOVE | wParam | dwAlt, lParam, 0);
					}
				}
			}
			break;

		case WM_LBUTTONDOWN:
			pwbo = wbGetWBObj(hwnd);
			M_nImageIndex = 2;					// Pressed
			InvalidateRect(hwnd, NULL, FALSE);
			UpdateWindow(hwnd);						// Force an update
			SetCapture(hwnd);

			// Starts the auto-repeat feature

			if(pwbo->style & WBC_AUTOREPEAT) {

				// Sets the timer for the first delay

				if(pwbo && pwbo->parent && pwbo->parent->pszCallBackFn && *pwbo->parent->pszCallBackFn) {
					wbCallUserFunction(pwbo->parent->pszCallBackFn, pwbo->pszCallBackObj, pwbo->parent, pwbo, pwbo->id,
					wParam, lParam, 0);
				}
				M_bRepeatOn = FALSE;
				SetTimer(hwnd, REPEAT_TIMER, 400, NULL);
			}
			break;

		case WM_LBUTTONUP:
			pwbo = wbGetWBObj(hwnd);
			M_nImageIndex = 0;					// Released
			InvalidateRect(hwnd, NULL, FALSE);
			ReleaseCapture();

			if(pwbo->style & WBC_AUTOREPEAT) {

				// Kills the timer

				KillTimer(hwnd, REPEAT_TIMER);
				M_bRepeatOn = FALSE;
			} else if(!(pwbo->style & WBC_DISABLED)) {
				if(pwbo && pwbo->parent && pwbo->parent->pszCallBackFn && *pwbo->parent->pszCallBackFn) {
					wbCallUserFunction(pwbo->parent->pszCallBackFn, pwbo->pszCallBackObj, pwbo->parent, pwbo, pwbo->id,
					wParam, lParam, 0);
				}
			}
			break;

		case WM_PAINT:
			{
				HDC hdc;
				PAINTSTRUCT ps;
				HIMAGELIST hi;
				RECT rc;

				pwbo = wbGetWBObj(hwnd);
				if(!pwbo)
					break;

				if(pwbo->style & WBC_TRANSPARENT)
					break;

				// ImageList was already stored in control

				hi = (HIMAGELIST)M_hiImageList;
				if(!hi)
					break;

				hdc = BeginPaint(hwnd, &ps);

				// Select image based on mouse position
				// Image index is stored in control

				GetWindowRect(hwnd, &rc);

				if(pwbo->style & WBC_DISABLED) {
					M_nImageIndex = 3;				// Disabled
				} else {

					POINT pt;

					GetCursorPos(&pt);

					if(PtInRect(&rc, pt)) {
						if(M_nImageIndex != 2)
							M_nImageIndex = 1;
					} else {
						if(M_bRepeatOn && (pwbo->style & WBC_AUTOREPEAT))
							M_nImageIndex = 2;
						else
							M_nImageIndex = 0;
					}
				}
				ImageList_Draw(hi, M_nImageIndex, hdc, 0, 0, ILD_NORMAL);

				EndPaint(hwnd, &ps);
			}
			return 0;

		case WM_TIMER:

			pwbo = wbGetWBObj(hwnd);
			if((wParam == REPEAT_TIMER) && (pwbo->style & WBC_AUTOREPEAT)) {
				if(pwbo->style & WBC_DISABLED) {

					// Kills the timer

					KillTimer(hwnd, REPEAT_TIMER);

				} else {

					if(pwbo && pwbo->parent && pwbo->parent->pszCallBackFn && *pwbo->parent->pszCallBackFn) {
						wbCallUserFunction(pwbo->parent->pszCallBackFn, pwbo->pszCallBackObj, pwbo->parent, pwbo, pwbo->id,
						wParam, lParam, 0);
					}

					if(!M_bRepeatOn) {

						// Starts repeating commands

						SetTimer(hwnd, REPEAT_TIMER, 150, NULL);
						M_bRepeatOn = TRUE;
					}
				}
			}
			break;

		case WM_SETCURSOR:
			{
				pwbo = wbGetWBObj(hwnd);
				if(!pwbo)
					break;

				if(M_nMouseCursor != 0) {
					SetCursor(M_nMouseCursor == -1 ? 0 : (HCURSOR)M_nMouseCursor);
					return TRUE;			// Must return here, not break
				} else {
					break;					// Normal behavior
				}
			}
			break;

	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

//------------------------------------------------------------------ END OF FILE
