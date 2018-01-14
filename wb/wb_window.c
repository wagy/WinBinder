/*******************************************************************************

 WINBINDER - The native Windows binding for PHP

 Copyright © Hypervisual - see LICENSE.TXT for details
 Author: Rubem Pechansky (http://winbinder.org/contact.php)

 Window creation and manipulation functions

*******************************************************************************/

//----------------------------------------------------------------- DEPENDENCIES

#include "wb.h"
#include <string.h>				// For stricmp()
#include <stdlib.h>				// For atol()
#include <shellapi.h>			// For Shell_NotifyIcon()
#include <mmsystem.h>			// For timeSetEvent() and timeKillEvent

//-------------------------------------------------------------------- CONSTANTS

#define MAXWINNAME				256
#define WBDEFCLASSSTYLE			(CS_DBLCLKS | CS_PARENTDC)
#define DEFAULT_WIN_STYLE		(WS_POPUP|WS_MINIMIZEBOX|WS_VISIBLE|WS_CLIPSIBLINGS|WS_CLIPCHILDREN|WS_MAXIMIZEBOX|WS_CAPTION|WS_BORDER|WS_SYSMENU|WS_THICKFRAME)
#define CUSTOM_MESSAGE_NAME		"@WB_win32_%d_%s"

//----------------------------------------------------------------------- MACROS

#define CALL_CALLBACK(id,lp1,lp2,lp3)	/* Call user function */ \
	{if(pwbobj && pwbobj->parent && pwbobj->parent->pszCallBackFn && *pwbobj->parent->pszCallBackFn) { \
		wbCallUserFunction(pwbobj->parent->pszCallBackFn, pwbobj->parent->pszCallBackObj, pwbobj->parent, pwbobj, id, lp1, lp2, lp3); }}

#define SEND_MESSAGE			/* Send additional notification message */ \
	((pwbobj->parent->uClass == TabControl) ? (pwbobj->parent->parent->style & WBC_NOTIFY) : (pwbobj->parent->style & WBC_NOTIFY))

#define TEST_FLAG(f)			/* Test notification message flag */ \
	((pwbobj->parent->uClass == TabControl) ? (pwbobj->parent->parent->lparam & (f)) : (pwbobj->parent->lparam & (f)))

//------------------------------------------------------------------- PROTOTYPES

// Global

BOOL SetTaskBarIcon(HWND hwnd, BOOL bModify);

// External

extern PWBOBJ AssignHandlerToTabs(HWND hwndParent, LPDWORD pszObj, LPCTSTR pszHandler);
extern DWORD GetCalendarTime(PWBOBJ pwbo);
extern LRESULT CALLBACK BrowserWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
extern BOOL RegisterImageButtonClass(void);
HWND CreateToolTip(PWBOBJ pwbo, LPCTSTR pszTooltip);

// Static

static HICON GetWindowIcon(HWND hwnd);
static BOOL DeleteTaskBarIcon(HWND hwnd);
static void UpdateLVlParams(HWND hwnd);
static int CALLBACK CompareLVItemsAscending(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
static int CALLBACK CompareLVItemsDescending(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
static void CALLBACK TimeProc(UINT uID, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2);
static DWORD CenterWindow(HWND hwndMovable, HWND hwndFixed);
static BOOL CALLBACK EnumWindowsProc(HWND hWnd, LPARAM lParam);
static DWORD GetUniqueStringId(LPCTSTR szStr);

// Procedures for WinBinder classes

static LRESULT CALLBACK DefaultWBProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
static LRESULT CALLBACK MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
static LRESULT CALLBACK OwnerDrawnWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
static LRESULT CALLBACK OwnerDrawnNakedWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
static LRESULT CALLBACK NakedWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
static LRESULT CALLBACK ModelessWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
static LRESULT CALLBACK ModalWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
static LRESULT CALLBACK TabPageProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

//----------------------------------------------------------------------- TYPES

// Used in EnumWindowsProc() for wbGetRequestedAppWindow()

typedef struct {
	HWND hwndFound;
	DWORD idRequested;
} APPW_DATA;

//-------------------------------------------------------------------- VARIABLES

// Global

HINSTANCE hAppInstance = NULL;
HBRUSH hbrTabs = NULL;
COLORREF clrTabs = NOCOLOR;

// Static

static HWND hToolBar = NULL;
static HWND hStatusBar = NULL;
static HWND hwndListView = NULL;
PWBOBJ pwndMain = NULL;
LISTVIEWCOLOR test;

//------------------------------------------------------------- PUBLIC FUNCTIONS

PWBOBJ wbCreateWindow(PWBOBJ pwboParent, UINT uWinBinderClass, LPCTSTR pszCaption, LPCTSTR pszTooltip,
	int xPos, int yPos, int nWidth, int nHeight, UINT id, DWORD dwWBStyle, long lParam)
{
	TCHAR szWindowName[MAXWINNAME] = TEXT("");
	LPTSTR pszClass;
	DWORD dwStyle = 0;
	DWORD dwExStyle = 0;
	PWBOBJ pwbo;

	// Initialize WinBinder object

	pwbo = wbMalloc(sizeof(WBOBJ));
	if(!pwbo)
		return NULL;
	pwbo->hwnd = NULL;
	pwbo->id = 0;
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

//	No need to create the buffer here because a WM_SIZE message is always sent when a window is created
//	pwbo->pbuffer = BITTEST(dwWBStyle, WBC_OWNERDRAWN) ? wbCreateBitmap(nWidth, nHeight, NULL, NULL) : NULL;

	switch(uWinBinderClass) {

		case AppWindow:				// Fixed size main window
			pszClass = 	(BITTEST(dwWBStyle, WBC_CUSTOMDRAW) ? OWNERDRAWN_WINDOW_CLASS : MAIN_WINDOW_CLASS);
			dwStyle = dwStyle ? dwStyle : WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_VISIBLE | WS_CLIPCHILDREN | WS_MINIMIZEBOX | CS_DBLCLKS;
			break;

		case ResizableWindow:		// Resizable main window
			pszClass = 	(BITTEST(dwWBStyle, WBC_CUSTOMDRAW) ? OWNERDRAWN_WINDOW_CLASS : MAIN_WINDOW_CLASS);
			dwStyle = dwStyle ? dwStyle : WS_OVERLAPPEDWINDOW | WS_CAPTION | WS_SYSMENU | WS_VISIBLE | WS_CLIPCHILDREN | WS_SIZEBOX | CS_DBLCLKS;
			break;

		case PopupWindow:			// Fixed size main window with no minimize/maximize button
			pszClass = 	(BITTEST(dwWBStyle, WBC_CUSTOMDRAW) ? OWNERDRAWN_WINDOW_CLASS : MAIN_WINDOW_CLASS);
			dwStyle = dwStyle ? dwStyle : WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_VISIBLE | WS_CLIPCHILDREN | CS_DBLCLKS;
			break;

		case NakedWindow:			// Fixed size borderless window
			pszClass = 	(BITTEST(dwWBStyle, WBC_CUSTOMDRAW) ? OWNERDRAWN_NAKED_CLASS: NAKED_WINDOW_CLASS);
			dwStyle = dwStyle ? dwStyle : WS_POPUP | WS_SYSMENU | WS_MINIMIZEBOX | WS_VISIBLE | WS_CLIPCHILDREN | CS_DBLCLKS;
			if(BITTEST(dwWBStyle, WBC_BORDER))
				dwExStyle |= WS_EX_DLGMODALFRAME;
			//pwbo->style |= WBC_CUSTOMDRAW;				// All naked windows are owner-drawn
//			dwExStyle = dwExStyle ? dwExStyle : 0;
//			pwbo->rcTitle = (LPARAM)wbMalloc(sizeof(RECT));
//			SetRect((LPRECT)pwbo->rcTitle, -1, -1, 0, 0);
			break;

		case ModalDialog:			// Modal dialog box
			pszClass = MODAL_WINDOW_CLASS;
			dwStyle = dwStyle ? dwStyle : WS_POPUP | WS_SYSMENU | WS_CAPTION | WS_VISIBLE | WS_CLIPCHILDREN | DS_MODALFRAME | CS_DBLCLKS;
			dwExStyle = dwExStyle ? dwExStyle : WS_EX_DLGMODALFRAME;
			break;

		case ModelessDialog:		// Modeless dialog box
			pszClass = MODELESS_WINDOW_CLASS;
			dwStyle = dwStyle ? dwStyle : WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION | WS_VISIBLE | WS_CLIPCHILDREN | CS_DBLCLKS;
			break;

		case ToolDialog:			// Modeless tool dialog
			pszClass = MODELESS_WINDOW_CLASS;
			dwStyle = dwStyle ? dwStyle : WS_POPUP | WS_SYSMENU | WS_CAPTION | WS_VISIBLE | WS_CLIPCHILDREN | CS_DBLCLKS;
			dwExStyle = dwExStyle ? dwExStyle : WS_EX_TOOLWINDOW;
			break;

		default:
			return NULL;
	}

	// Create the window

	if(BITTEST(dwWBStyle, WBC_INVISIBLE))
		dwStyle = dwStyle & (~WS_VISIBLE);

	wcsncpy(szWindowName, pszCaption, MAXWINNAME - 1);
	pwbo->hwnd = CreateWindowEx(
		dwExStyle,
		pszClass,
		szWindowName,
		dwStyle ? dwStyle : DEFAULT_WIN_STYLE,
		xPos == WBC_CENTER ? xPos : (xPos < 0 ? CW_USEDEFAULT : xPos),
		yPos == WBC_CENTER ? yPos : (yPos < 0 ? CW_USEDEFAULT : yPos),
		nWidth <= 0 ? CW_USEDEFAULT : nWidth,
		nHeight <= 0 ? CW_USEDEFAULT : nHeight,
		pwboParent ? pwboParent->hwnd : NULL,
		NULL, hAppInstance, NULL);

	if(!pwbo->hwnd)
		return NULL;

	// Assigns pwndMain and the window ID

	if(pwndMain == NULL) {
		pwndMain = pwbo;
		pwbo->id = GetUniqueStringId(pszCaption);		// Assigns a unique ID to a parent window
	}

	// Create a tooltip for this window if required

	CreateToolTip(pwbo, pszTooltip);

	SetWindowLong(pwbo->hwnd, GWL_USERDATA, (LONG)pwbo);

	// Is it a modal dialog?

	if(uWinBinderClass == ModalDialog)
		SendMessage(pwbo->hwnd, WM_SETFOCUS, (WPARAM)GetWindow(pwbo->hwnd, GW_CHILD), 0);

//	wbSetCursor(pwbo, NULL);		// Assumes class cursor

	// Should it be always on top?

	if(BITTEST(dwWBStyle, WBC_TOP))
		SetWindowPos(pwbo->hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

	return pwbo;
}

/* Used to detect existing application instances */

HWND wbGetRequestedAppWindow(LPCTSTR pszCaption, BOOL bBringToFront)
{
	APPW_DATA appw;

	appw.hwndFound = NULL;
	appw.idRequested = GetUniqueStringId(pszCaption);

	EnumWindows(EnumWindowsProc, (LPARAM)&appw);

	if(appw.hwndFound) {				// Found it: bring the window to the front
		if(bBringToFront) {
			if(IsIconic(appw.hwndFound))
				ShowWindow(appw.hwndFound, SW_RESTORE);
			SetForegroundWindow(appw.hwndFound);
			BringWindowToTop(appw.hwndFound);
		}
		return appw.hwndFound;
	} else {
		return NULL;
	}
}

// Assign a handler to a window and to its children tab controls, if any

BOOL wbSetWindowHandler(PWBOBJ pwbo, LPDWORD pszObj, LPCTSTR pszHandler)
{
	if(!pszHandler || !*pszHandler)
		return FALSE;

	if(!pwbo || !pwbo->hwnd || !IsWindow(pwbo->hwnd))
		return FALSE;

	// An object name was passed

	if(pszObj != NULL) {
		pwbo->pszCallBackObj = pszObj;
	} else
		pwbo->pszCallBackObj = NULL;

	pwbo->pszCallBackFn = (LPTSTR)pszHandler;

	if (pwbo->parent == NULL) //Only main window
	{
		AssignHandlerToTabs(pwbo->hwnd, pszObj, pszHandler);
	}
	

	return TRUE;
}


/* Closes the window referred by pwbo. If the window is not the main window,
  "eats" WM_CLOSE messages to prevent closing the application */

BOOL wbDestroyWindow(PWBOBJ pwbo)
{
	BOOL bRet, bIsMainWindow;
	MSG msg;

	if(!pwbo || !pwbo->hwnd || !IsWindow(pwbo->hwnd))
		return FALSE;

	bIsMainWindow = (pwbo == pwndMain);

	// Reset window handler

	pwbo->pszCallBackObj = NULL;
	pwbo->pszCallBackFn = NULL;
	AssignHandlerToTabs(pwbo->hwnd, NULL, NULL);

	bRet = DestroyWindow(pwbo->hwnd);

	if(!GetFocus())
		SetFocus(GetForegroundWindow());

	pwbo->hwnd = NULL;

	wbFree(pwbo);

	if(bIsMainWindow) {

		pwndMain = NULL;
		if(M_nMMTimerId)
			timeKillEvent(M_nMMTimerId);

	} else {

		// "Eat" message WM_CLOSE

		do {
			if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		} while(msg.message == WM_CLOSE);
	}

	return bRet;
}

/*
	TODO: Center horizontally only or vertically only according to xPos, yPos
*/

BOOL wbSetWindowPosition(PWBOBJ pwbo, int xPos, int yPos, PWBOBJ pwboParent)
{
	if(!pwbo || !pwbo->hwnd || !IsWindow(pwbo->hwnd))
		return FALSE;

	if(xPos == WBC_CENTER || yPos == WBC_CENTER) {
		if(!pwboParent || !pwboParent->hwnd || !IsWindow(pwboParent->hwnd))
			CenterWindow(pwbo->hwnd, NULL);
		else
			CenterWindow(pwbo->hwnd, pwboParent->hwnd);
		return TRUE;
	} else {
		// pwboParent is ignored here
		return SetWindowPos(pwbo->hwnd, 0, xPos, yPos, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
	}
}

/*
	TODO: Use pwboParent as the reference
*/

DWORD wbGetWindowPosition(PWBOBJ pwbo, PWBOBJ pwboParent, BOOL bClientRect)
{
	RECT rc;
	BOOL bRet;

	if(!pwbo || !pwbo->hwnd || !IsWindow(pwbo->hwnd))
		return FALSE;

	// pwboParent is ignored here

	if(bClientRect) {
		RECT rcParent;

		bRet = GetWindowRect(pwbo->hwnd, &rc);
		GetWindowRect(pwbo->parent->hwnd, &rcParent);

		rc.left -= rcParent.left;
		rc.top -= rcParent.top;
	} else
		bRet = GetWindowRect(pwbo->hwnd, &rc);

	if(!bRet)
		return (DWORD)MAKELONG(WBC_CENTER, WBC_CENTER);
	else
		return (DWORD)MAKELONG(rc.left, rc.top);
}

BOOL wbSetWindowSize(PWBOBJ pwbo, int nWidth, int nHeight, int nShowMode)
{
	if(!pwbo || !pwbo->hwnd || !IsWindow(pwbo->hwnd))
		return FALSE;

	if(nShowMode < 0) {
		nWidth = MAX(0, nWidth);
		nHeight = MAX(0, nHeight);

		return SetWindowPos(pwbo->hwnd, 0, 0, 0, nWidth, nHeight, SWP_NOMOVE | SWP_NOZORDER);

	} else {

		switch(nShowMode) {

			case WBC_MINIMIZED:
				return ShowWindow(pwbo->hwnd, SW_MINIMIZE);

			case WBC_MAXIMIZED:
				return ShowWindow(pwbo->hwnd, SW_MAXIMIZE);

			case WBC_NORMAL:
				return ShowWindow(pwbo->hwnd, SW_RESTORE);

			default:
				return FALSE;
		}
	}
}

DWORD wbGetWindowSize(PWBOBJ pwbo, BOOL bClientRect)
{
	RECT rc;
	BOOL bRet;

	if(!pwbo || !pwbo->hwnd || !IsWindow(pwbo->hwnd))
		return FALSE;

	if(IsIconic(pwbo->hwnd))
		return (DWORD)MAKELONG(WBC_CENTER, WBC_MINIMIZED);
//	else if(IsZoomed(pwbo->hwnd))
//		return (DWORD)MAKELONG(WBC_CENTER, WBC_MAXIMIZED);

	if(bClientRect)
		bRet = GetClientRect(pwbo->hwnd, &rc);
	else
		bRet = GetWindowRect(pwbo->hwnd, &rc);
	if(!bRet)
		return (DWORD)MAKELONG(WBC_CENTER, WBC_CENTER);
	else
		return (DWORD)MAKELONG(rc.right - rc.left, rc.bottom - rc.top);
}

BOOL wbSetWindowArea(PWBOBJ pwbo, UINT type, int xPos, int yPos, int nWidth, int nHeight)
{
	if(!pwbo || !pwbo->hwnd || !IsWindow(pwbo->hwnd))
		return FALSE;

	switch(type) {

		case WBC_TITLE:
			SetRect(&pwbo->rcTitle, xPos, yPos, nWidth, nHeight);
			return TRUE;

		case WBC_MINSIZE:
			pwbo->arMin.width = nWidth;
			pwbo->arMin.height = nHeight;
			return TRUE;

		case WBC_MAXSIZE:
			pwbo->arMax.width = nWidth;
			pwbo->arMax.height = nHeight;
			return TRUE;

		default:
			return FALSE;
	}
}

/* Fills an array with all controls and returns the number of controls found. If pwboList is NULL,
	just returns the number of controls. */

UINT wbGetControlList(PWBOBJ pwboParent, PWBOBJ pwboList[], UINT nMaxControls)
{
	HWND hwnd;
	UINT i = 0;

	if(!wbIsWBObj((void *)pwboParent, TRUE))
		return 0;

	if(!nMaxControls)
		return 0;

	hwnd = GetWindow(pwboParent->hwnd, GW_CHILD);
	if(hwnd) {
		if(pwboList)
			pwboList[i] = wbGetWBObj(hwnd);
	} else
		return 0;
	for(i = 1; i < nMaxControls; i++) {
		hwnd = GetWindow(hwnd, GW_HWNDNEXT);
		if(hwnd) {
			if(pwboList)
				pwboList[i] = wbGetWBObj(hwnd);
		} else
			break;
	}
	return i;
}

/* Sort a list view column */

BOOL wbSortLVColumn(PWBOBJ pwbo, int nSubItem, BOOL bAscending)
{
	hwndListView = pwbo->hwnd;		// For CompareLVItems()

	SendMessage(pwbo->hwnd, LVM_SORTITEMS, nSubItem, (bAscending ?
		(LPARAM)(PFNLVCOMPARE)CompareLVItemsAscending :
		(LPARAM)(PFNLVCOMPARE)CompareLVItemsDescending));
	UpdateLVlParams(hwndListView);
	return TRUE;
}

/* Make uPeriod equal to zero to kill the timer. id < 0 means a hi-res timer */

BOOL wbSetTimer(PWBOBJ pwbo, int id, UINT uPeriod)
{
	if(!pwbo || !pwbo->hwnd || !IsWindow(pwbo->hwnd))
		return FALSE;

	if(id > 0) {
		if(!uPeriod)
			return KillTimer(pwbo->hwnd, id);

		if(SetTimer(pwbo->hwnd, id, uPeriod, NULL))
			return TRUE;
		else
			return FALSE;

	} else {

		MMRESULT mmId;

		if(!uPeriod)
			return (timeKillEvent(M_nMMTimerId) == TIMERR_NOERROR);

		mmId = timeSetEvent(uPeriod, 0, TimeProc, (DWORD_PTR)pwbo, TIME_PERIODIC | TIME_CALLBACK_FUNCTION);
		if(mmId != (MMRESULT)NULL) {
			M_nTimerId = id;
			M_nMMTimerId = mmId;
			return TRUE;
		} else {
			return FALSE;
		}
	}
}

//------------------------------------------- FUNCTIONS PUBLIC TO WINBINDER ONLY

// ***** LIMITATION:
// ***** This implementation assumes there is only one window with one toolbar

void SetToolBarHandle(HWND hCtrl)
{
	hToolBar = hCtrl;
}

// ***** LIMITATION:
// ***** This implementation assumes there is only one window with a status bar

void SetStatusBarHandle(HWND hCtrl)
{
	hStatusBar = hCtrl;
}

/* Register all window classes */

BOOL RegisterClasses(void)
{
	WNDCLASS wc;

	// Main window class

	memset(&wc, 0, sizeof(WNDCLASS));
	wc.style = WBDEFCLASSSTYLE;
	wc.lpfnWndProc = (WNDPROC)MainWndProc;
	wc.hInstance = hAppInstance;
	wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
	wc.lpszClassName = MAIN_WINDOW_CLASS;
	wc.lpszMenuName = NULL;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.cbWndExtra = 0;

	if(!RegisterClass(&wc))
		return FALSE;

	// Owner-drawn window class

	memset(&wc, 0, sizeof(WNDCLASS));
	wc.style = WBDEFCLASSSTYLE | CS_HREDRAW | CS_VREDRAW; // For owner-drawn windows
	wc.lpfnWndProc = (WNDPROC)OwnerDrawnWndProc;
	wc.hInstance = hAppInstance;
	wc.hbrBackground = NULL;	// Background must be NULL here
	wc.lpszClassName = OWNERDRAWN_WINDOW_CLASS;
	wc.lpszMenuName = NULL;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.cbWndExtra = 0;

	if(!RegisterClass(&wc))
		return FALSE;

	// Naked window class

	memset(&wc, 0, sizeof(WNDCLASS));
	wc.style = WBDEFCLASSSTYLE;
	wc.lpfnWndProc = (WNDPROC)NakedWndProc;
	wc.hInstance = hAppInstance;
	wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
	wc.lpszClassName = NAKED_WINDOW_CLASS;
	wc.lpszMenuName = NULL;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.cbWndExtra = 0;

	if(!RegisterClass(&wc))
		return FALSE;

	// Owner-drawn naked window class

	memset(&wc, 0, sizeof(WNDCLASS));
	wc.style = WBDEFCLASSSTYLE | CS_HREDRAW | CS_VREDRAW; // For owner-drawn windows
	wc.lpfnWndProc = (WNDPROC)OwnerDrawnNakedWndProc;
	wc.hInstance = hAppInstance;
	wc.hbrBackground = NULL;	// Background must be NULL here
	wc.lpszClassName = OWNERDRAWN_NAKED_CLASS;
	wc.lpszMenuName = NULL;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.cbWndExtra = 0;

	if(!RegisterClass(&wc))
		return FALSE;

	// Modal dialogs

	memset(&wc, 0, sizeof(WNDCLASS));
	wc.style = WBDEFCLASSSTYLE;
	wc.lpfnWndProc = (WNDPROC)ModalWndProc;
	wc.hInstance = hAppInstance;
	wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
	wc.lpszClassName = MODAL_WINDOW_CLASS;
	wc.lpszMenuName = NULL;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hIcon = NULL;
	wc.cbWndExtra = DLGWINDOWEXTRA;

	if(!RegisterClass(&wc))
		return FALSE;

	// Modeless dialogs

	memset(&wc, 0, sizeof(WNDCLASS));
	wc.style = WBDEFCLASSSTYLE;
	wc.lpfnWndProc = (WNDPROC)ModelessWndProc;
	wc.hInstance = hAppInstance;
	wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
	wc.lpszClassName = MODELESS_WINDOW_CLASS;
	wc.lpszMenuName = NULL;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hIcon = NULL;
	wc.cbWndExtra = DLGWINDOWEXTRA;

	if(!RegisterClass(&wc))
		return FALSE;

	// Tab pages

	memset(&wc, 0, sizeof(WNDCLASS));
	wc.style = WBDEFCLASSSTYLE;
	wc.lpfnWndProc = (WNDPROC)TabPageProc;
	wc.hInstance = hAppInstance;
	wc.hbrBackground = hbrTabs ? hbrTabs : (HBRUSH)(COLOR_BTNFACE + 1);	// Not for under Windows XP
	wc.lpszClassName = TAB_PAGE_CLASS;
	wc.lpszMenuName = NULL;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hIcon = NULL;
	wc.cbWndExtra = DLGWINDOWEXTRA;

	if(!RegisterClass(&wc))
		return FALSE;

	// Browser window class

	memset(&wc, 0, sizeof(WNDCLASS));
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = (WNDPROC)BrowserWndProc;
	wc.hInstance = hAppInstance;
	wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
	wc.lpszClassName = BROWSER_WINDOW_CLASS;
	wc.lpszMenuName = NULL;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hIcon = NULL;
	wc.cbWndExtra = DLGWINDOWEXTRA;

	if(!RegisterClass(&wc))
		return FALSE;

	// Register custom controls

	if(!RegisterImageButtonClass())
		return FALSE;

	return TRUE;
}

//-------------------------------------------------- WINDOW PROCESSING FUNCTIONS

/*
	The all-important common processing routine for all parent windows

	TODO: Filter user commands. Try: (LPNMHDR)lParam)->code == NM_CLICK, etc.
	TODO: Use the return value as a flag to know whether the calling function should continue default processing or not

*/

static LRESULT CALLBACK DefaultWBProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static HWND hTBWnd = NULL;			// Handle of toolbar window

	switch(msg) {

		//------------------------------- Notification messages

		case WBWM_KEYDOWN:			// Custom WinBinder message
			{
				PWBOBJ pwbobj;
				HWND hCtrl = (HWND)wParam;

				pwbobj = wbGetWBObj(hCtrl);

				if(!pwbobj || !pwbobj->parent)
					break;

				if(SEND_MESSAGE && TEST_FLAG(WBC_KEYDOWN))
					CALL_CALLBACK(pwbobj->id, WBC_KEYDOWN, lParam, 0);
			}
			break;

		case WBWM_KEYUP:			// Custom WinBinder message
			{
				PWBOBJ pwbobj;
				HWND hCtrl = (HWND)wParam;

				pwbobj = wbGetWBObj(hCtrl);

				if(!pwbobj || !pwbobj->parent)
					break;

				if(SEND_MESSAGE && TEST_FLAG(WBC_KEYUP))
					CALL_CALLBACK(pwbobj->id, WBC_KEYUP, lParam, 0);
			}
			break;

		case WM_NOTIFY:
			{
				TCHAR szClass[256];
				HWND hCtrl = ((LPNMHDR)lParam)->hwndFrom;
				PWBOBJ pwbobj;

				if(!IsWindow(hCtrl))
					break;

				// The classes below don't call the callback function

				GetClassName(hCtrl, szClass, 255);

				if(!_wcsicmp(szClass, TOOLBARCLASSNAME)) {			// Toolbar

					if(!hTBWnd)
						hTBWnd = ((LPNMHDR)lParam)->hwndFrom;
					break;

				} else if(!_wcsicmp(szClass, TOOLTIPS_CLASS)) {		// Tooltip

					if(((LPNMHDR)lParam)->code == (UINT)TTN_NEEDTEXT) {
						if(hTBWnd) {
							TBBUTTON tbb;
							int index;

							index = SendMessage(hTBWnd, TB_COMMANDTOINDEX,
								((LPTOOLTIPTEXT)lParam)->hdr.idFrom, 0);
							SendMessage(hTBWnd, TB_GETBUTTON, index, (LPARAM)(LPTBBUTTON)&tbb);
							((LPTOOLTIPTEXT)lParam)->lpszText = (LPTSTR)tbb.dwData;
						}
					}
					break;
				}

				// Verify if window is a WinBinder object

				pwbobj = wbGetWBObj(hCtrl);

				if(!pwbobj || !pwbobj->parent)
					break;

				if(!pwbobj->parent->pszCallBackFn)
					break;

				// Call callback function according to WinBinder class

				switch(pwbobj->uClass) {

					case Spinner:

						if(((LPNMHDR)lParam)->code == (UINT)UDN_DELTAPOS) {
							CALL_CALLBACK(((LPNMHDR)lParam)->idFrom, 0, 0, 0);
						}
						break;

					case TreeView:

						switch(((LPNMHDR)lParam)->code) {

							case NM_DBLCLK:
								if(SEND_MESSAGE && TEST_FLAG(WBC_DBLCLICK))
									CALL_CALLBACK(((LPNMHDR)lParam)->idFrom, WBC_DBLCLICK, 0, 0);
								break;

							case (UINT)TVN_SELCHANGED:
								CALL_CALLBACK(((LPNMHDR)lParam)->idFrom, 0, 0, 0);
								break;
						}
						break;

					case TabControl:

						if(((LPNMHDR)lParam)->code == (UINT)TCN_SELCHANGE) {

							HWND hTab = ((LPNMHDR)lParam)->hwndFrom;
							int nSelTab = TabCtrl_GetCurSel(hTab);

							wbSelectTab(wbGetWBObj(hCtrl), nSelTab);
							if(SEND_MESSAGE && TEST_FLAG(WBC_HEADERSEL))
								CALL_CALLBACK(((LPNMHDR)lParam)->idFrom, WBC_HEADERSEL, nSelTab, 0);
//							CALL_CALLBACK(((LPNMHDR)lParam)->idFrom, nSelTab, 0);
							return 0;
						}
						break;

					case Calendar:
						{
							PWBOBJ pwbobj;

							pwbobj = wbGetWBObj(hCtrl);
							if(!pwbobj)
								break;

							switch(((LPNMHDR)lParam)->code) {
								case MCN_SELCHANGE:
									CALL_CALLBACK(((LPNMHDR)lParam)->idFrom, GetCalendarTime(pwbobj), 0, 0);
									break;
							}
						}
						break;

					case ListView:
					{
						//UINT c = ((LPNMHDR)lParam)->code;
						switch (((LPNMHDR)lParam)->code) {
						//case 0xffffff4f:
						case NM_CUSTOMDRAW:
							if (pwbobj->pszCallBackFn != NULL) //has color handler
							{
								LPNMLVCUSTOMDRAW lplvcd = (LPNMLVCUSTOMDRAW)lParam;
								
								switch (lplvcd->nmcd.dwDrawStage)
								{
								case CDDS_PREPAINT:
									return CDRF_NOTIFYITEMDRAW;
								case CDDS_ITEMPREPAINT:
									{
										LISTVIEWCOLOR lvc = { 0 };
										UINT ret = wbCallUserFunction(pwbobj->pszCallBackFn, pwbobj->pszCallBackObj, pwbobj->parent, pwbobj, ((LPNMHDR)lParam)->idFrom, lplvcd->nmcd.lItemlParam, -1, &lvc);

										if (ret > 0)
										{
											if (ret == 2) return CDRF_NOTIFYSUBITEMDRAW;
											switch (lvc.nMode)
											{
											case 1:
												lplvcd->clrText = lvc.dwForeground; break;
											case 2:
												lplvcd->clrTextBk = lvc.dwBackground; break;
											case 3:
												lplvcd->clrText = lvc.dwForeground;
												lplvcd->clrTextBk = lvc.dwBackground; break;
											default: return CDRF_DODEFAULT;
											}
											return CDRF_NEWFONT;
										}
										return CDRF_DODEFAULT;
									}
									break;
								case CDDS_SUBITEM | CDDS_ITEMPREPAINT:
									{
										LISTVIEWCOLOR lvc = { 0 };
										UINT ret = wbCallUserFunction(pwbobj->pszCallBackFn, pwbobj->pszCallBackObj, pwbobj->parent, pwbobj, ((LPNMHDR)lParam)->idFrom, lplvcd->nmcd.lItemlParam, lplvcd->iSubItem, &lvc);
										if (ret > 0)
										{
											switch (lvc.nMode)
											{
												case 1:
													lplvcd->clrText = lvc.dwForeground; break;
												case 2:
													lplvcd->clrTextBk = lvc.dwBackground; break;
												case 3:
													lplvcd->clrText = lvc.dwForeground;
													lplvcd->clrTextBk = lvc.dwBackground; break;
												default: return CDRF_DODEFAULT;
											}
											return CDRF_NEWFONT;
										}
										return CDRF_DODEFAULT;
									}
									break;
								}
							}
							break;
						case NM_DBLCLK:

							if (SEND_MESSAGE && TEST_FLAG(WBC_DBLCLICK))
								CALL_CALLBACK(((LPNMHDR)lParam)->idFrom, WBC_DBLCLICK, 0, 0);
							break;


							/*case NM_CLICK:
								if(SEND_MESSAGE && TEST_FLAG(WBC_LBUTTON))
									CALL_CALLBACK(((LPNMHDR)lParam)->idFrom, WBC_LBUTTON ,0,0);
								break;
*/
						case NM_RCLICK:

							if (SEND_MESSAGE && TEST_FLAG(WBC_RBUTTON))
								CALL_CALLBACK(((LPNMHDR)lParam)->idFrom, WBC_RBUTTON, 0, 0);
							break;


						case LVN_ITEMCHANGED:

							if (((LPNM_LISTVIEW)lParam)->uChanged & (LVIF_STATE | LVIS_CHECKED)) {
								CALL_CALLBACK(((LPNMHDR)lParam)->idFrom, 0, 0, 0);
							}
							break;

						case LVN_COLUMNCLICK:

							hwndListView = pwbobj->hwnd;		// For CompareLVItems()
							SendMessage(pwbobj->hwnd, LVM_SORTITEMS,
								((NM_LISTVIEW FAR *)lParam)->iSubItem, (LPARAM)(PFNLVCOMPARE)CompareLVItemsAscending);
							UpdateLVlParams(hwndListView);
							if (SEND_MESSAGE && TEST_FLAG(WBC_HEADERSEL))
								CALL_CALLBACK(((LPNMHDR)lParam)->idFrom, WBC_HEADERSEL, ((NM_LISTVIEW FAR *)lParam)->iSubItem, 0);
							break;
						}
					}
						break;

				} // switch(pwbobj->uClass)]

			} // ~WM_NOTIFY
			break;

		// 2011_11_24 - Stefan Loewe: added this case to allow tabbing thru "normal" window
		case WM_ACTIVATE:
			hCurrentDlg = hwnd;	// Used in IsDialogMessage() -- main loop
			break;

		case WM_HSCROLL:		// Scroll bars, sliders
		case WM_VSCROLL:

			{
				PWBOBJ pwbobj;
				HWND hCtrl;
				int nMin, nMax, nPos;

				hCtrl = (HWND)lParam;
				pwbobj = wbGetWBObj(hCtrl);

				if(!pwbobj)
					break;

				nPos = SendMessage(hCtrl, SBM_GETPOS, 0, 0);

				switch(LOWORD(wParam)) {
					case SB_LINEUP:
						SendMessage(hCtrl, SBM_GETRANGE,
							(WPARAM)(LPINT)&nMin, (WPARAM)(LPINT)&nMax);
						nPos -= (nMax - nMin) / 100;
						nPos = MAX(nPos, 0);
						break;

					case SB_LINEDOWN:
						SendMessage(hCtrl, SBM_GETRANGE,
							(WPARAM)(LPINT)&nMin, (WPARAM)(LPINT)&nMax);
						nPos += (nMax - nMin) / 100;
						nPos = MAX(nPos, 0);
						break;

					case SB_PAGEUP:
						SendMessage(hCtrl, SBM_GETRANGE,
							(WPARAM)(LPINT)&nMin, (WPARAM)(LPINT)&nMax);
						nPos -= (nMax - nMin) / 10;
						nPos = MAX(nPos, 0);
						break;

					case SB_PAGEDOWN:
						SendMessage(hCtrl, SBM_GETRANGE,
							(WPARAM)(LPINT)&nMin, (WPARAM)(LPINT)&nMax);
						nPos += (nMax - nMin) / 10;
						nPos = MAX(nPos, 0);
						break;

					case SB_THUMBPOSITION:
					case SB_THUMBTRACK:						// Same value as TB_THUMBTRACK
						nPos = (short)HIWORD(wParam);
						break;
				}

				SendMessage(hCtrl, SBM_SETPOS, nPos, TRUE);
				CALL_CALLBACK(pwbobj->id, 0, 0, 0);
			}
			break;

		case WM_COMMAND:
			{
				HWND hCtrl;
				PWBOBJ pwbobj;

				if((HWND)lParam == 0 && (HIWORD(wParam) == 0 || HIWORD(wParam) == 1)) { // Is the message from a menu or accelerator?

					pwbobj = wbGetWBObj(hwnd);
					// Must not use the macro CALL_CALLBACK here
					if(pwbobj && pwbobj->pszCallBackFn && *pwbobj->pszCallBackFn)
						wbCallUserFunction(pwbobj->pszCallBackFn, pwbobj->pszCallBackObj, pwbobj, pwbobj, LOWORD(wParam), 0, HIWORD(wParam), 0);
					return 0;
				}

				hCtrl = (HWND)lParam;
				pwbobj = wbGetWBObj(hCtrl);

				if(!pwbobj || !pwbobj->parent || !pwbobj->parent->pszCallBackFn)
					break;

				switch(pwbobj->uClass) {	// Classes that receive their notifications via WM_COMMAND

					case EditBox:
					case RTFEditBox:
						if((HIWORD(wParam) == EN_CHANGE)) {
							CALL_CALLBACK(LOWORD(wParam), 0, HIWORD(wParam), 0);
						} else if(HIWORD(wParam) == EN_SETFOCUS) {
							if(SEND_MESSAGE && TEST_FLAG(WBC_GETFOCUS))
								CALL_CALLBACK(LOWORD(wParam), WBC_GETFOCUS, HIWORD(wParam), 0);
						}
						break;

					case ComboBox:

						if((HIWORD(wParam) == CBN_EDITCHANGE)) {

							// Store selected index for future use

							pwbobj->lparam = -1;
							CALL_CALLBACK(LOWORD(wParam), 0, HIWORD(wParam), 0);

						} else if((HIWORD(wParam) == CBN_SELCHANGE)) {

							// The code block below makes wb_get_text() return the correct contents
							// of a non-read-only combo box right after a selection is made

							if(!(pwbobj->style & WBC_READONLY)) {

								int nIndex, nTextLen;
								LPTSTR szText = TEXT("");

								nIndex = SendMessage(pwbobj->hwnd, CB_GETCURSEL, 0, 0);
								if(nIndex != CB_ERR) {
									nTextLen = SendMessage(pwbobj->hwnd, CB_GETLBTEXTLEN, nIndex, 0);
									if(nIndex != CB_ERR) {
										if(nTextLen) {
											szText = wbMalloc(nTextLen * sizeof(TCHAR));
											SendMessage(pwbobj->hwnd, CB_GETLBTEXT, nIndex, (LPARAM)szText);
											SendMessage(pwbobj->hwnd, WM_SETTEXT, 0, (LPARAM)szText);
											wbFree(szText);
										} else
											SendMessage(pwbobj->hwnd, WM_SETTEXT, 0, (LPARAM)TEXT(""));
									}
								}
							}

							// Store selected index for future use

							pwbobj->lparam = SendMessage(pwbobj->hwnd, CB_GETCURSEL, 0, 0);
							CALL_CALLBACK(LOWORD(wParam), 0, HIWORD(wParam), 0);
						} else if(HIWORD(wParam) == CBN_SETFOCUS) {
							if(SEND_MESSAGE && TEST_FLAG(WBC_GETFOCUS))
								CALL_CALLBACK(LOWORD(wParam), WBC_GETFOCUS, HIWORD(wParam), 0);
						}
						break;

					case ListBox:
						if(HIWORD(wParam) == LBN_DBLCLK) {
							if(SEND_MESSAGE && TEST_FLAG(WBC_DBLCLICK))
								CALL_CALLBACK(LOWORD(wParam), WBC_DBLCLICK, HIWORD(wParam), 0);
						} else if (HIWORD(wParam) == LBN_SELCHANGE) {
							CALL_CALLBACK(LOWORD(wParam), 0, HIWORD(wParam), 0);
						} else if(HIWORD(wParam) == LBN_SETFOCUS) {
							if(SEND_MESSAGE && TEST_FLAG(WBC_GETFOCUS))
								CALL_CALLBACK(LOWORD(wParam), WBC_GETFOCUS, HIWORD(wParam), 0);
						}
						break;

					case InvisibleArea:
						CALL_CALLBACK(LOWORD(wParam), 0, HIWORD(wParam), 0);
						break;

					case HyperLink:
						if(HIWORD(wParam) == STN_CLICKED)
							CALL_CALLBACK(LOWORD(wParam), WBC_GETFOCUS, HIWORD(wParam), 0);
						break;

					case PushButton:
					case CheckBox:
					case RadioButton:
					case ToolBar:

						if(HIWORD(wParam) == BN_CLICKED) {
							CALL_CALLBACK(LOWORD(wParam), 0, HIWORD(wParam), 0);
						} else if(HIWORD(wParam) == BN_SETFOCUS) {
							if(SEND_MESSAGE && TEST_FLAG(WBC_GETFOCUS))
								CALL_CALLBACK(LOWORD(wParam), WBC_GETFOCUS, HIWORD(wParam), 0);
						}
						break;

//					default:				// TODO: Maybe delete these lines,
//						bProcess = TRUE;	// but must be sure first
//						CALL_CALLBACK(LOWORD(wParam), 0, 0);
				}

				return 0;
			}
			break;

		//------------------------------- Mouse messages

		case WM_MOUSEMOVE:
			{
				PWBOBJ pwbobj = wbGetWBObj(hwnd);

				if(!pwbobj)
					break;

				if(BITTEST(pwbobj->lparam, WBC_MOUSEMOVE)) {

					DWORD dwAlt = (GetKeyState(VK_MENU) < 0) ? WBC_ALT : 0;

					if(pwbobj && pwbobj->pszCallBackFn && *pwbobj->pszCallBackFn)
						wbCallUserFunction(pwbobj->pszCallBackFn, pwbobj->pszCallBackObj, pwbobj, pwbobj, 0,
						WBC_MOUSEMOVE | wParam | dwAlt, lParam, 0);
				}
			}
			break;

		// Alt key checking: http://msdn.microsoft.com/library/en-us/winui/winui/windowsuserinterface/userinput/mouseinput/mouseinputreference/mouseinputmessages/wm_lbuttondown.asp
		// Also see "System and Nonsystem Keystrokes" in API help

		case WM_LBUTTONDOWN:
		case WM_MBUTTONDOWN:
		case WM_RBUTTONDOWN:
			{
				PWBOBJ pwbobj = wbGetWBObj(hwnd);

				if(!pwbobj)
					break;

				if(BITTEST(pwbobj->lparam, WBC_MOUSEDOWN)) {

					DWORD dwAlt = (GetKeyState(VK_MENU) < 0) ? WBC_ALT : 0;

					if(pwbobj && pwbobj->pszCallBackFn && *pwbobj->pszCallBackFn)
						wbCallUserFunction(pwbobj->pszCallBackFn, pwbobj->pszCallBackObj, pwbobj, pwbobj, 0,
						WBC_MOUSEDOWN | wParam | dwAlt, lParam, 0);
				}
			}
			break;

		case WM_LBUTTONUP:
			wParam |= MK_LBUTTON;		// Why isn't this flag included?
			goto MOUSE1;
		case WM_MBUTTONUP:
			wParam |= MK_MBUTTON;		// Why isn't this flag included?
			goto MOUSE1;
		case WM_RBUTTONUP:
			wParam |= MK_RBUTTON;		// Why isn't this flag included?
			goto MOUSE1;
MOUSE1:
			{
				PWBOBJ pwbobj = wbGetWBObj(hwnd);

				if(!pwbobj)
					break;

				if(BITTEST(pwbobj->lparam, WBC_MOUSEUP)) {

					DWORD dwAlt = (GetKeyState(VK_MENU) < 0) ? WBC_ALT : 0;
//printf("[%d]\n", WBC_MOUSEUP | wParam | dwAlt);
					if(pwbobj && pwbobj->pszCallBackFn && *pwbobj->pszCallBackFn)
						wbCallUserFunction(pwbobj->pszCallBackFn, pwbobj->pszCallBackObj, pwbobj, pwbobj, 0,
						WBC_MOUSEUP | wParam | dwAlt, lParam, 0);
				}
			}
			break;

		case WM_LBUTTONDBLCLK:
		case WM_MBUTTONDBLCLK:
		case WM_RBUTTONDBLCLK:
			{
				PWBOBJ pwbobj = wbGetWBObj(hwnd);

				if(!pwbobj)
					break;

				if(BITTEST(pwbobj->lparam, WBC_DBLCLICK)) {
					PWBOBJ pwbobj = wbGetWBObj(hwnd);
					DWORD dwAlt = (GetKeyState(VK_MENU) < 0) ? WBC_ALT : 0;

					if(pwbobj && pwbobj->pszCallBackFn && *pwbobj->pszCallBackFn)
						wbCallUserFunction(pwbobj->pszCallBackFn, pwbobj->pszCallBackObj, pwbobj, pwbobj, 0,
						WBC_DBLCLICK | dwAlt | wParam, lParam, 0);
				}
			}
			break;

		//------------------------------- Other messages

		case WM_CTLCOLORSTATIC:			// For static controls and others
		case WM_CTLCOLORBTN:			// For pushbuttons

			if(hbrTabs) {				// Not for versions under Windows XP
				HWND hCtrl;
				PWBOBJ pwbobj;

				hCtrl = (HWND)lParam;
				pwbobj = wbGetWBObj(hCtrl);

				if(!pwbobj || !pwbobj->parent)
					break;

				if(pwbobj->uClass == EditBox)				// Not for edit controls
					break;

				SetBkColor((HDC)wParam, clrTabs);			// Static controls need this
				if(pwbobj->parent->uClass == TabControl)	// Only for controls on tabs
					return (LRESULT)hbrTabs;				// Paint the background with the tab page color
			}
			break;

		case WM_TIMER:

			{
				PWBOBJ pwbobj;

				pwbobj = wbGetWBObj(hwnd);

				if(!pwbobj || !pwbobj->pszCallBackFn)
					break;

				if(pwbobj->pszCallBackFn)
//					wbCallUserFunction(pwbobj->pszCallBackFn, pwbobj, pwbobj, wParam, 0, 0, 0);
					wbCallUserFunction(pwbobj->pszCallBackFn, pwbobj->pszCallBackObj, pwbobj, pwbobj, wParam, 0, 0, 0);
				return 0;
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

		case WBWM_IDAPP:			// Custom WinBinder message
			{
				PWBOBJ pwbobj;

				pwbobj = wbGetWBObj(hwnd);
				return pwbobj ? pwbobj->id : 0;	// Returns the window ID
			}
			break;

		case WM_SETFOCUS:
			{
				PWBOBJ pwbobj = wbGetWBObj(hwnd);

				if(!pwbobj)
					break;

				if(BITTEST(pwbobj->lparam, WBC_GETFOCUS)) {

					DWORD dwAlt = (GetKeyState(VK_MENU) < 0) ? WBC_ALT : 0;

					if(pwbobj && pwbobj->pszCallBackFn && *pwbobj->pszCallBackFn)
						wbCallUserFunction(pwbobj->pszCallBackFn, pwbobj->pszCallBackObj, pwbobj, pwbobj, 0,
						WBC_GETFOCUS, 0, 0);
				}
			}
			break;
				
		case WM_HOTKEY:
			{
				PWBOBJ pwbobj;

				pwbobj = wbGetWBObj(hwnd);

				if(!pwbobj || !pwbobj->pszCallBackFn)
					break;

				if(pwbobj->pszCallBackFn)
//					wbCallUserFunction(pwbobj->pszCallBackFn, pwbobj, pwbobj, wParam, 0, 0, 0);
					wbCallUserFunction(pwbobj->pszCallBackFn, pwbobj->pszCallBackObj, pwbobj, pwbobj, wParam, lParam, 0, 0);
				return 1;

			break;
			}			

	}

	return DefWindowProc(hwnd, msg, wParam, lParam);
}


/* Main window class processing */

static LRESULT CALLBACK MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg) {

		//------------------------- Custom messages

		case WBWM_NOTIFYICON:
			{
				PWBOBJ pwbobj = wbGetWBObj(hwnd);

				if(!pwbobj)
					break;

				if(pwbobj->style & WBC_TASKBAR) {

					switch (lParam){
						case WM_LBUTTONDBLCLK:
						case WM_LBUTTONUP:
							ShowWindow(hwnd, SW_SHOWMINNOACTIVE);
							ShowWindow(hwnd, SW_NORMAL);
							SetForegroundWindow(hwnd);
							BringWindowToTop(hwnd);
//							SetFocus(hwnd);
							DeleteTaskBarIcon(hwnd);
							return TRUE;
					}
				}
			}
			break;

		//------------------------- Conventional Windows messages

		case WM_CREATE:
			if(((LPCREATESTRUCT)lParam)->x == WBC_CENTER && ((LPCREATESTRUCT)lParam)->y == WBC_CENTER)
				CenterWindow(hwnd, ((LPCREATESTRUCT)lParam)->hwndParent);
			break;

		// 2011_11_24 - Stefan Loewe: added this case to allow tabbing thru "normal" window
		case WM_ACTIVATE:
			hCurrentDlg = hwnd;	// Used in IsDialogMessage() -- main loop
			break;

		case WM_SIZING:				// Used to set window size limits
			{
				PWBOBJ pwbo = wbGetWBObj(hwnd);

				if(!pwbo)
					break;

				if(pwbo->arMin.width || pwbo->arMin.height) {

					LPRECT lprc = (LPRECT)lParam;	// lParam sets the window size

					if(pwbo->arMin.width && lprc->right - lprc->left < pwbo->arMin.width)
						lprc->right = lprc->left + pwbo->arMin.width;
					if(pwbo->arMax.width && lprc->right - lprc->left > pwbo->arMax.width)
						lprc->right = lprc->left + pwbo->arMax.width;
					if(pwbo->arMin.height && lprc->bottom - lprc->top < pwbo->arMin.height)
						lprc->bottom = lprc->top + pwbo->arMin.height;
					if(pwbo->arMax.height && lprc->bottom - lprc->top > pwbo->arMax.height)
						lprc->bottom = lprc->top + pwbo->arMax.height;
				}
			}
			break;

		case WM_SIZE:
			{
				PWBOBJ pwbobj = wbGetWBObj(hwnd);

				if(!pwbobj)
					break;

				if(pwbobj->style & WBC_TASKBAR) {
					if(wParam == SIZE_MINIMIZED) {
						SetTaskBarIcon(hwnd, FALSE);
						ShowWindow(hwnd, SW_HIDE);
					} else if(wParam == SIZE_RESTORED) {
						DeleteTaskBarIcon(hwnd);
					}
				}

				SendMessage(hToolBar, msg, wParam, lParam);
				SendMessage(hStatusBar, msg, wParam, lParam);

				// Use the callback function to let the user resize a control or draw something on screen

//				if(pwbobj->style & WBC_NOTIFY && wParam == SIZE_RESTORED && lParam == 0) {
				if(pwbobj->style & WBC_NOTIFY && ((wParam == SIZE_RESTORED) || (wParam == SIZE_MAXIMIZED))) {
					if((pwbobj->lparam & WBC_RESIZE) && pwbobj->pszCallBackFn && *pwbobj->pszCallBackFn)
						wbCallUserFunction(pwbobj->pszCallBackFn, pwbobj->pszCallBackObj, pwbobj, pwbobj, IDDEFAULT, WBC_RESIZE,
						(LPARAM)pwbobj->pbuffer, lParam);
//printf("%d %d\n", wParam, lParam);
					if((pwbobj->lparam & WBC_REDRAW) && pwbobj->pszCallBackFn && *pwbobj->pszCallBackFn)
						wbCallUserFunction(pwbobj->pszCallBackFn, pwbobj->pszCallBackObj, pwbobj, pwbobj, IDDEFAULT, WBC_REDRAW,
						(LPARAM)pwbobj->pbuffer, 0);
				}

			}
			break;

		case WM_CLOSE:
			{
				PWBOBJ pwbobj = wbGetWBObj(hwnd);

				if(pwbobj && pwbobj->pszCallBackFn && *pwbobj->pszCallBackFn) {

					// This message is only sent to windows that have callback functions

					SendMessage(hwnd, WM_COMMAND, MAKELONG(IDCLOSE, 0), (LPARAM)NULL);
					return 0;
				} else {
		 			break;
				}
 			}
			break;

		case WM_DESTROY:
			{
				PWBOBJ pwbobj = wbGetWBObj(hwnd);
				

				if(!pwbobj)
					break;

				if(pwbobj->style & WBC_TASKBAR)
					DeleteTaskBarIcon(hwnd);
				
				if(pwbobj == pwndMain) {
				
					PostQuitMessage(0);
				}
			}
			//PostQuitMessage(0);
			break;
		
		// added by AF
		case WBWM_MIDI:		
			{	
				PWBOBJ pwbobj;
				DWORD *ptr;
				ptr = lParam;
				//MessageBox(NULL, "midi", 0,0);
				pwbobj = wbGetWBObj(hwnd);

				if (!pwbobj || !pwbobj->pszCallBackFn)
					break;

				if (pwbobj->pszCallBackFn)
					//					wbCallUserFunction(pwbobj->pszCallBackFn, pwbobj, pwbobj, wParam, 0, 0, 0);

				{
					wbCallUserFunction(pwbobj->pszCallBackFn, pwbobj->pszCallBackObj, ptr[3], wParam, ptr[0], ptr[1], ptr[2], 0);

					return TRUE;
				}
				//	return DefaultWBProc(hwnd, msg, wParam, lParam);
			}
			break;
		// added by AF
		case WBWM_ENUM:
			{
				PWBOBJ pwbobj;								

				pwbobj = wbGetWBObj(hwnd);

				if(!pwbobj || !pwbobj->pszCallBackFn)
					break;

				if(pwbobj->pszCallBackFn)
				{	wbCallUserFunction( pwbobj->pszCallBackFn, pwbobj->pszCallBackObj, pwbobj, WBWM_ENUM, lParam, wParam, 0, 0);
					
					return TRUE;
				}
			}
			break;
		// added by AF
		case WBWM_HOOK:
			{
				PWBOBJ pwbobj;								
				DWORD *ptr;
				ptr = lParam;
				
				pwbobj = wbGetWBObj(hwnd);

				if(!pwbobj || !pwbobj->pszCallBackFn)
					break;

				if(pwbobj->pszCallBackFn)
				{	wbCallUserFunction( pwbobj->pszCallBackFn, pwbobj->pszCallBackObj, pwbobj, WBWM_HOOK, wParam, ptr[0],  ptr[1],0);
					
					return TRUE;
				}
			}
			break;

// ******* OOPS -- Will DefWindowProc() (below) ever get called?
// ******* And should it be DefWindowProc() or DefaultWBProc()???

		default:
			return DefaultWBProc(hwnd, msg, wParam, lParam);
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

LRESULT ProcessCustomDraw(LPARAM lParam)
{
	LPNMLVCUSTOMDRAW lplvcd = (LPNMLVCUSTOMDRAW)lParam;

	switch (lplvcd->nmcd.dwDrawStage)
	{
		//!(cd->nmcd.dwDrawStage & CDDS_ITEM))
		case CDDS_PREPAINT: //Before the paint cycle begins
							//request notifications for individual listview items
			//return CDRF_NOTIFYITEMDRAW;
			return CDRF_NOTIFYSUBITEMDRAW;

		case CDDS_ITEMPREPAINT: //Before an item is drawn
		{
			return CDRF_NOTIFYSUBITEMDRAW;
		}
		break;

		//Before a subitem is drawn
		case CDDS_SUBITEM | CDDS_ITEMPREPAINT:
		{
			switch (lplvcd->iSubItem)
			{
			case 0:
			{
				lplvcd->clrText = RGB(255, 255, 255);
				lplvcd->clrTextBk = RGB(240, 55, 23);
				return CDRF_NEWFONT;
			}
			break;

			case 1:
			{
				lplvcd->clrText = RGB(255, 255, 0);
				lplvcd->clrTextBk = RGB(0, 0, 0);
				return CDRF_NEWFONT;
			}
			break;

			case 2:
			{
				lplvcd->clrText = RGB(20, 26, 158);
				lplvcd->clrTextBk = RGB(200, 200, 10);
				return CDRF_NEWFONT;
			}
			break;

			case 3:
			{
				lplvcd->clrText = RGB(12, 15, 46);
				lplvcd->clrTextBk = RGB(200, 200, 200);
				return CDRF_NEWFONT;
			}
			break;

			case 4:
			{
				lplvcd->clrText = RGB(120, 0, 128);
				lplvcd->clrTextBk = RGB(20, 200, 200);
				return CDRF_NEWFONT;
			}
			break;

			case 5:
			{
				lplvcd->clrText = RGB(255, 255, 255);
				lplvcd->clrTextBk = RGB(0, 0, 150);
				return CDRF_NEWFONT;
			}
			break;

			}
		}
	}
	return CDRF_DODEFAULT;
}

// Owner-drawn window class: subclasses MainWndProc

static LRESULT CALLBACK OwnerDrawnWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg) {

		case WM_ACTIVATE:
			if(LOWORD(wParam) != WA_INACTIVE && !(HIWORD(wParam)))
				SendMessage(hwnd, WM_SIZE, SIZE_RESTORED, 0);
			break;

		case WM_SIZE:				// Create screen buffer and draw on it
			{
				PWBOBJ pwbobj;
				RECT rc;
				LPRECT prcOld;

				pwbobj = wbGetWBObj(hwnd);

				if(wParam == SIZE_MINIMIZED)
					break;

				if(!pwbobj)
					break;

				GetClientRect(pwbobj->hwnd, &rc);

				if(!pwbobj->lparams[1]) {
					prcOld = wbMalloc(sizeof(RECT));
					SetRect(prcOld, 0, 0, 0, 0);
					// Store previous buffer size in the window parameters
					pwbobj->lparams[1] = (LONG)prcOld;
				} else
					prcOld = (LPRECT)pwbobj->lparams[1];

				// Is the window is larger than before, enlarge the buffer;
				// Otherwise do nothing

				if((rc.right - rc.left > prcOld->right - prcOld->left) ||
				   (rc.bottom - rc.top > prcOld->bottom - prcOld->top)) {

					// Yes, create a larger buffer

					if(pwbobj->pbuffer)
						DeleteObject(pwbobj->pbuffer);
					pwbobj->pbuffer = wbCreateBitmap(rc.right, rc.bottom, NULL, NULL);

					// Paint it with the default window color

					wbDrawRect(pwbobj->pbuffer, 0, 0, rc.right - rc.left, rc.bottom - rc.top,
					  GetSysColor(COLOR_BTNFACE), TRUE, 0, 0);

//					memcpy(prcOld, &rc, sizeof(RECT));
					*prcOld = rc;
				}
			}
			break;

		case WM_PAINT:				// Paint whatever is in the screen buffer
			{
				PAINTSTRUCT ps;
				HDC hdc, hdcBuf;
				PWBOBJ pwbobj;

				pwbobj = wbGetWBObj(hwnd);

				if(!pwbobj || !pwbobj->pbuffer)
					break;

				hdc = BeginPaint(pwbobj->hwnd, &ps);
				hdcBuf = CreateCompatibleDC(hdc);
				SelectObject(hdcBuf, pwbobj->pbuffer);
				BitBlt(hdc, 0, 0, ps.rcPaint.right, ps.rcPaint.bottom, hdcBuf, 0, 0, SRCCOPY);
				DeleteDC(hdcBuf);
				EndPaint(pwbobj->hwnd, &ps);
			}
			return 0;

//		case WM_CLOSE:
		case WM_DESTROY:			// Release screen buffer from memory
			{
				PWBOBJ pwbobj;
				pwbobj = wbGetWBObj(hwnd);

				if(pwbobj->pbuffer) {
					DeleteObject(pwbobj->pbuffer);
					pwbobj->pbuffer = NULL;
				}
				// Zero the parameter used to store buffer size
				pwbobj->lparams[1] = 0;
			}
			break;

//		default:
//			return MainWndProc(hwnd, msg, wParam, lParam);
	}
	return MainWndProc(hwnd, msg, wParam, lParam);
}

// Naked window class: subclasses MainWndProc

static LRESULT CALLBACK NakedWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg) {

		case WM_NCHITTEST:
			{
				PWBOBJ pwbobj = wbGetWBObj(hwnd);

				if(!pwbobj)
					break;

				// pwbo->rcTitle stores a rectangle that emulates a title bar

				if(pwbobj->rcTitle.left < 0 || pwbobj->rcTitle.top < 0)
					return HTCAPTION;					// Special case: full window area
				else {

					POINT pt;
					RECT rcWin;

					GetWindowRect(hwnd, &rcWin);
					pt.x = LOWORD(lParam) - rcWin.left;
					pt.y = HIWORD(lParam) - rcWin.top;
//printf("%d %d %d %d\n", pt.x, pt.y, pwbobj->rcTitle.left, pwbobj->rcTitle.right);
					if(PtInRect(&pwbobj->rcTitle, pt))
						return HTCAPTION;
				}
			}
			break;

//		default:
//			return MainWndProc(hwnd, msg, wParam, lParam);
	}
	return MainWndProc(hwnd, msg, wParam, lParam);
}

// Owner-drawn naked window class: subclasses NakedWndProc, OwnerDrawnWndProc

static LRESULT CALLBACK OwnerDrawnNakedWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg) {

		case WM_NCHITTEST:
			return NakedWndProc(hwnd, msg, wParam, lParam);

		default:
			return OwnerDrawnWndProc(hwnd, msg, wParam, lParam);
	}
}

// The word "modal" here is not quite true: these are not "real" modal dialog boxes

static LRESULT CALLBACK ModalWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg) {

		case WM_CREATE:
			if(((LPCREATESTRUCT)lParam)->hwndParent) {
				if(((LPCREATESTRUCT)lParam)->x == WBC_CENTER && ((LPCREATESTRUCT)lParam)->y == WBC_CENTER)
					CenterWindow(hwnd, ((LPCREATESTRUCT)lParam)->hwndParent);
				EnableWindow(GetParent(hwnd), FALSE);
			}
			break;

		case WM_ACTIVATE:
			hCurrentDlg = hwnd;	// Used in IsDialogMessage() -- main loop
			break;

		case WM_CLOSE:
            {
				PWBOBJ pwbobj = wbGetWBObj(hwnd);

				if(pwbobj && pwbobj->pszCallBackFn && *pwbobj->pszCallBackFn) {

					// This message is only sent to windows that have callback functions

//					SendMessage(hwnd, WM_DESTROY, 0, 0);		// I will be sent below anyway
					SendMessage(hwnd, WM_COMMAND, MAKELONG(IDCLOSE, 0), (LPARAM)NULL);

	//				return DefaultWBProc(hwnd, msg, wParam, lParam);
				}
			}
            break;

		case WM_DESTROY:
			{
				HWND hParent;

				hParent = GetParent(hwnd);
				if(hParent) {
					EnableWindow(hParent, TRUE);
					SetForegroundWindow(hParent);
					BringWindowToTop(hParent);
//					SetFocus(hParent);
				} else {
					PostQuitMessage(0);
				}
			}
			// 2011_11_24 - Stefan Loewe: do not set it to NULL, because when closing, e.g. a modal dialog, tabbing through
			// controls no longer works for the opening window (or any other)
			//hCurrentDlg = NULL;
			break;

// ******* OOPS -- Will DefDlgProc() (below) ever get called?
// ******* And should it be DefDlgProc() or DefaultWBProc()???

		default:
			return DefaultWBProc(hwnd, msg, wParam, lParam);
	}
	return DefDlgProc(hwnd, msg, wParam, lParam);
}

static LRESULT CALLBACK ModelessWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg) {

		case WM_CREATE:
			if(((LPCREATESTRUCT)lParam)->hwndParent) {
				if(((LPCREATESTRUCT)lParam)->x == WBC_CENTER && ((LPCREATESTRUCT)lParam)->y == WBC_CENTER)
					CenterWindow(hwnd, ((LPCREATESTRUCT)lParam)->hwndParent);
			}
			break;

		case WM_ACTIVATE:
		case WM_CLOSE:
		//case WM_DESTROY:
			return ModalWndProc(hwnd, msg, wParam, lParam);		
			
		
		case WM_DESTROY:
			//return;

// ******* OOPS -- Will DefDlgProc() (below) ever get called?
// ******* And should it be DefDlgProc() or DefaultWBProc()???

		default:
			return DefaultWBProc(hwnd, msg, wParam, lParam);
	}
	return DefDlgProc(hwnd, msg, wParam, lParam);
}

// Processes messages for tab pages

static LRESULT CALLBACK TabPageProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg) {

		case WM_CLOSE:		// Prevents that an ESC from inside an edit box closes this page
			return 0;

		case WM_SHOWWINDOW:
			if(wParam) {
//				HWND hwndNext;

				hCurrentDlg = hwnd;	// Used in IsDialogMessage() -- main loop

// The lines below were causing a bug with the menu (bug #399)

//				hwndNext = GetNextDlgTabItem(hwnd, NULL, FALSE);
//				if(hwndNext)
//					SetFocus(hwndNext);
			} else
				hCurrentDlg = NULL;
			break;
	}
	return DefaultWBProc(hwnd, msg, wParam, lParam);
}

//------------------------------------------------------------ PRIVATE FUNCTIONS

/* Callback function for multimedia timer. Merely sends a WM_TIMER message to the window.
  Attempts to call wbCallUserFunction() directly failed miserably: the app crashes when
  zend_is_callable() is called. */

static void CALLBACK TimeProc(UINT uID, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2)
{
	PWBOBJ pwbo;

	pwbo = (PWBOBJ)dwUser;

	if(!pwbo || !(pwbo->pszCallBackFn))
		return;

	SendMessage(pwbo->hwnd, WM_TIMER, M_nTimerId, 0);
}

/* Try several methods to retrieve the icon from an application window
 Adapted from from http://groups.google.com/groups?hl=en&lr=&selm=38BC4F60.11F62F%40thematic.com
 Peter Forbes / David Lowndes
*/

static HICON GetWindowIcon(HWND hwnd)
{
	HICON hIcon;

	hIcon = (HICON)SendMessage(hwnd, WM_GETICON, ICON_SMALL, 0);
	if(hIcon)
		return hIcon;

	hIcon = (HICON)SendMessage(hwnd, WM_GETICON, ICON_BIG, 0);
	if(hIcon)
		return hIcon;

	hIcon = (HICON)SendMessage(hwnd, WM_QUERYDRAGICON, 0, 0);
	if(hIcon)
		return hIcon;

	hIcon = (HICON)GetClassLong(hwnd, GCL_HICONSM);
	if(hIcon)
		return hIcon;

	hIcon = (HICON)GetClassLong(hwnd, GCL_HICON);
	if(hIcon)
		return hIcon;

	return LoadIcon(NULL, IDI_APPLICATION);
}

// Taskbar icon functions: from Win32 programmer's reference, "Adding and Deleting Taskbar Icons" and
// http://msdn.microsoft.com/library/default.asp?url=/library/en-us/shellcc/platform/shell/programmersguide/shell_int/shell_int_programming/taskbar.asp

BOOL SetTaskBarIcon(HWND hwnd, BOOL bModify)
{
	BOOL bRes;
	NOTIFYICONDATA tnid;

	if(!hwnd)
		return FALSE;

	tnid.cbSize = sizeof(NOTIFYICONDATA);
	tnid.hWnd = hwnd;
	tnid.uID = 0;
	tnid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
	tnid.uCallbackMessage = WBWM_NOTIFYICON;
	tnid.hIcon = CopyIcon(GetWindowIcon(hwnd));
	GetWindowText(hwnd, tnid.szTip, sizeof(tnid.szTip));
	bRes = Shell_NotifyIcon(bModify ? NIM_MODIFY : NIM_ADD, &tnid);
	if(tnid.hIcon)
		DestroyIcon(tnid.hIcon);
	return bRes;
}

static BOOL DeleteTaskBarIcon(HWND hwnd)
{
	BOOL bRes;
	NOTIFYICONDATA tnid;

	if(!hwnd)
		return FALSE;
	tnid.cbSize = sizeof(NOTIFYICONDATA);
	tnid.hWnd = hwnd;
	tnid.uID = 0;
	bRes = Shell_NotifyIcon(NIM_DELETE, &tnid);
	DestroyIcon(tnid.hIcon);
	return bRes;
}

/*
	Ported from Iczelion's Win32 Assembly Homepage: http://spiff.tripnet.se/~iczelion/tut31.html

	Quote: "Note that when the items are rearranged, their indexes also change. So when the sorting is done
	in my example, I need to update the values in lParam to reflect the new indexes."
*/

static void UpdateLVlParams(HWND hwnd)
{
	LV_ITEM lvi;
	int count;

	count = SendMessage(hwnd, LVM_GETITEMCOUNT, 0, 0);
	lvi.mask = LVIF_PARAM;
	lvi.iSubItem = 0;
	for(lvi.iItem = 0; lvi.iItem < count; lvi.iItem++) {
		lvi.lParam = lvi.iItem;
		SendMessage(hwnd, LVM_SETITEM, 0, (LPARAM)&lvi);
	}
}

/*

 This is the callback function that compares two ListView items.

 - lParam1 && lParam2 contain the item indices
 - lParamSort is the sub-item index
 - 64 characters is more than enough for string comparison

*/

static BOOL IsDecimalNumber(LPTSTR szStr)
{
	int i;
	int nLen = MIN(wcslen(szStr), 63);

	for(i = 0; i < nLen; i++)
		if((*(szStr + i) < '0' || *(szStr + i) > '9') && *(szStr + i) != ' ')
			return FALSE;
	return TRUE;
}

static int CALLBACK CompareLVItemsAscending(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	TCHAR szStr1[64];
	TCHAR szStr2[64];

	ListView_GetItemText(hwndListView, lParam1, lParamSort, szStr1, 63);
	ListView_GetItemText(hwndListView, lParam2, lParamSort, szStr2, 63);

	if(!*szStr1 && !*szStr2)				// Two empty strings
		return 0;
	else if(!*szStr2)						// Second string is empty
		return 1;
	else if(!*szStr1)						// First string is empty
		return -1;
	else if(IsDecimalNumber(szStr1) && IsDecimalNumber(szStr2))		// Two integers
		return(_wtol(szStr1) == _wtol(szStr2) ? 0 : ((_wtol(szStr1) > _wtol(szStr2)) ? 1 : -1));
	else
		return lstrcmp(szStr1, szStr2);		// According to locale, not case-sensitive
}

static int CALLBACK CompareLVItemsDescending(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	TCHAR szStr1[64];
	TCHAR szStr2[64];

	ListView_GetItemText(hwndListView, lParam1, lParamSort, szStr1, 63);
	ListView_GetItemText(hwndListView, lParam2, lParamSort, szStr2, 63);

	if(!*szStr1 && !*szStr2)				// Two empty strings
		return 0;
	else if(!*szStr2)						// Second string is empty
		return -1;
	else if(!*szStr1)						// First string is empty
		return 1;
	else if(IsDecimalNumber(szStr1) && IsDecimalNumber(szStr2))		// Two integers
		return(_wtol(szStr1) == _wtol(szStr2) ? 0 : ((_wtol(szStr1) > _wtol(szStr2)) ? -1 : 1));
	else
		return lstrcmp(szStr2, szStr1);		// According to locale, not case-sensitive
}

/* Moves the window hwndMovable to the center of the window hwndFixed. If hwndFixed is NULL, moves
	hwndMovable to the center of the system desktop. hwndMovable never gets out of the system desktop
	limits. Returns a DWORD containing the new x and y coordinates of hwndMovable in the low and
	high words respectively. */

static DWORD CenterWindow(HWND hwndMovable, HWND hwndFixed)
{
	RECT rcMovable, rcFixed;
	LONG xPos, yPos;

	// Get window size

	if(!hwndFixed) {
		RECT rcDesktop;

		GetWindowRect(GetDesktopWindow(), &rcDesktop);
		rcFixed = rcDesktop;
	} else
		GetWindowRect(hwndFixed, &rcFixed);
	GetWindowRect(hwndMovable, &rcMovable);

	// Move window

	xPos = MIN(MAX(((rcFixed.right + rcFixed.left) - (rcMovable.right - rcMovable.left)) / 2, 0),
		rcFixed.right - (rcMovable.right - rcMovable.left)),
	yPos = MIN(MAX(((rcFixed.bottom + rcFixed.top) - (rcMovable.bottom - rcMovable.top)) / 2, 0),
		rcFixed.bottom - (rcMovable.bottom - rcMovable.top)),
	SetWindowPos(hwndMovable, 0, xPos, yPos, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOZORDER);

	return (DWORD)MAKELONG(xPos, yPos);
}

/* EnumWindows() callback function for wbGetRequestedAppWindow().
   TRUE continues the search, FALSE ends it */

static BOOL CALLBACK EnumWindowsProc(HWND hWnd, LPARAM lParam)
{
	DWORD result;
	LRESULT lres;
	TCHAR szAux[256];
	APPW_DATA *pappw;

	// Discards windows classes not from WinBinder

	GetClassName(hWnd, szAux, 255);
	if(szAux[0] == 'w' && szAux[1] == 'b')
		GetWindowText(hWnd, szAux, 255);
	else
		return TRUE;

	// Checks the custom message result which is the id of the top-level window

	pappw = (APPW_DATA *)lParam;
	pappw->hwndFound = NULL;
	lres = SendMessageTimeout(hWnd, WBWM_IDAPP, 0, 0, SMTO_BLOCK | SMTO_ABORTIFHUNG, 200, &result);

	if(!lres)
		return TRUE;

	if(pappw->idRequested == result) {			// Found it!
		pappw->hwndFound = hWnd;
		return FALSE;
	} else
		return TRUE;
}

// Calculate a simple checksum for the given string

static DWORD GetUniqueStringId(LPCTSTR szStr)
{
	int i, nEnd;
	DWORD dwSum;
	TCHAR szBuffer[1029];

	memset(szBuffer, 1028 * sizeof(TCHAR), 0);
	wcsncpy(szBuffer, szStr, 1023);

	nEnd = ((wcslen(szBuffer) + 3) & ~3);

	for(i = 0, dwSum = 0; i < nEnd; i += sizeof(DWORD)) {
		dwSum += szBuffer[i + 3] |
				 (szBuffer[i + 2] << 8) |
				 (szBuffer[i + 1] << 16) |
				 (szBuffer[i + 0] << 24);
	}
	return dwSum;
}

//------------------------------------------------------------------ END OF FILE
