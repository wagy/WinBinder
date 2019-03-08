/*******************************************************************************

 WINBINDER - The native Windows binding for PHP

 Copyright � Hypervisual - see LICENSE.TXT for details
 Author: Rubem Pechansky (http://winbinder.org/contact.php)

 Common header for WinBinder: all exported functions and constants

*******************************************************************************/

#include <stdio.h>

#ifndef _WB_H
#define _WB_H


//----------------------------------------------------------- COMPILER CONSTANTS

// General defines

#define NO_REGEX_EXTRA_H
//#define ZEND_WIN32
//#define PHP_WIN32
#define UNICODE
#define _UNICODE

#ifdef __LCC__
#	define _WINGDI_
#	ifndef _off_t
#		include <sys/types.h>
#	endif
#	ifndef UDM_SETRANGE32
#		define UDM_SETRANGE32       (WM_USER+111)	// Missing in lcc-win32
#	endif
#	define __FUNCTION__ __func__	// Lcc uses __func__, not __FUNCTION__
#else
#	define WIN32_LEAN_AND_MEAN		// For faster compilation
#endif

// Define constants for recent Windows controls. For WINVER constants, See
// http://msdn.microsoft.com/library/default.asp?url=/library/en-us/winprog/winprog/using_the_windows_headers.asp

#ifdef __GNUC__						// For GCC
#	define WINVER 			0x0501	// Internet Explorer 5.01
#	define _WIN32_IE 		WINVER
#endif

#ifdef _MSC_VER						// For MSVC (Rick)
#	if _MSC_VER < 1300
#		define __FUNCTION__ "(?)"
#	endif
#endif

#ifdef __POCC__						// For Pelles C
#	ifndef WIN32
#		define WIN32
#	endif
#	define _MAX_PATH				260		// From stdlib.h
#	define _MAX_FNAME				256		// From stdlib.h
#	define __POCC__OLDNAMES					// For access, itoa, stricmp, etc.
#endif

//----------------------------------------------------------------- DEPENDENCIES

// Common dependencies

#include "wb_version.h"
#include <windows.h>
#include <commctrl.h>
#pragma comment(lib,"kernel32.lib")
#pragma comment(lib,"user32.lib")
#pragma comment(lib,"gdi32.lib")
#pragma comment(lib,"winspool.lib")
#pragma comment(lib,"comdlg32.lib")
#pragma comment(lib,"advapi32.lib")
#pragma comment(lib,"shell32.lib")
#pragma comment(lib,"ole32.lib")
#pragma comment(lib,"oleaut32.lib")
#pragma comment(lib,"uuid.lib")
#pragma comment(lib,"odbc32.lib")
#pragma comment(lib,"odbccp32.lib")
#pragma comment(lib,"comctl32.lib")
#pragma comment(lib,"winmm.lib")
#pragma comment(lib,"ws2_32.lib")
#pragma comment(lib,"Dnsapi.lib")

//-------------------------------------------------------------------- CONSTANTS

// May be missing in some headers

#ifndef LVIS_CHECKED
#	define LVIS_CHECKED			0x2000
#endif

// Other constants

#define MAX_TABS				16		// Maximum number of tabs in a TabControl
#define MAX_TABCONTROLS			512		// Maximum number of controls in a TabControl
#define MAX_LISTVIEWCOLS		256		// Maximum columns in a ListView
#define MAX_ACCELS				256		// Maximum number of keyboard accelerators
#define MAX_ERR_MESSAGE			1024	// Maximum size of error message
#define MAX_ITEM_STRING			1024	// Maximum string in a ListView / TreeView
#define MAX_TREEVIEW_LEVELS		25		// Maximum treeview levels
#define MAX_IMAGELIST_IMAGES	128		// Maximum images inside a ImageList
#define REPEAT_TIMER			901		// Timer identifier for ImageButtons
#define NOCOLOR					CLR_INVALID
#define MAX_PATH_BUFFER			1024	// Maximum size for selected files

// Strings

#define RICHEDITCONTROL			TEXT("RichEdit20A")
#define TAB_PAGE_CLASS			TEXT("wbTabPage")
#define WB_EXTENSION			TEXT("phpw")

// WinBinder window class names (must begin with "wb", lowercase)

#define MAIN_WINDOW_CLASS		TEXT("wbMainWnd")
#define OWNERDRAWN_WINDOW_CLASS	TEXT("wbOwnerdrawnWnd")
#define OWNERDRAWN_NAKED_CLASS	TEXT("wbOwnerdrawnNakedWnd")
#define NAKED_WINDOW_CLASS		TEXT("wbNakedWnd")
#define MODELESS_WINDOW_CLASS	TEXT("wbModelessDlg")
#define MODAL_WINDOW_CLASS		TEXT("wbModalDlg")
#define BROWSER_WINDOW_CLASS	TEXT("wbHTMLWnd")
#define IMAGE_BUTTON_CLASS		TEXT("wbImageButton")

// Custom WinBinder messages

#define WBWM_NOTIFYICON			(WM_APP + 2)
#define WBWM_KEYDOWN			(WM_APP + 3)
#define WBWM_KEYUP				(WM_APP + 4)
#define WBWM_IDAPP				(WM_APP + 5)
#define WBWM_MIDI				(WM_APP + 6)
#define WBWM_ENUM				(WM_APP + 7)
#define WBWM_HOOK				(WM_APP + 8)

//----------------------------------------------------------- EXPORTED CONSTANTS

// WinBinder window and control classes

enum {

	// Window classes

	AppWindow = 1,
	ModalDialog,
	ModelessDialog,
	NakedWindow,
	PopupWindow,
	ResizableWindow,
	ToolDialog,

	// WinBinder control classes,

	Accel,
	Calendar,
	CheckBox,
	ComboBox,
	EditBox,
	Frame,
	Gauge,
	HTMLControl,
	HyperLink,
	ImageButton,
	InvisibleArea,
	Label,
	ListBox,
	ListView,
	Menu,
	PushButton,
	RTFEditBox,
	RadioButton,
	ScrollBar,
	Slider,
	Spinner,
	StatusBar,
	TabControl,
	ToolBar,
	TreeView,
};

#define NUMCLASSES				TreeView	// Must be the last class

// Style flags (parameter style of wb_create_window)

#define WBC_VISIBLE				0x00000000
#define WBC_LEFT				0x00000000
#define WBC_MIDDLE				0x00000000
#define WBC_ENABLED				0x00000000
#define WBC_INVISIBLE			0x00000001
#define WBC_DISABLED			0x00000002	// Equivalent to WS_DISABLED
#define WBC_IMAGE				0x00000004	// Control has an image
#define WBC_BORDER				0x00000008	// Control has a border
#define WBC_NOTIFY				0x00000010	// lparam contains additional notification messages
#define WBC_RIGHT				0x00000020
#define WBC_READONLY			0x00000040
#define WBC_LINES				0x00000080	// Grid lines, tab ticks
#define WBC_MULTILINE			0x00000080	// *** SAME VALUE AS WBC_LINES (may change in the future) ***
#define WBC_MASKED				0x00000100	// Password in edit fields
#define WBC_TASKBAR				0x00000200	// Minimize to the taskbar status area
#define WBC_AUTOREPEAT			0x00000200	// Auto-repeat (used for ImageButtons)
#define WBC_NUMBER				0x00000400	// Numeric edit fields
#define WBC_CENTER				0x00000800	// Used also in wbSetWindowPosition(), must be a low #
#define WBC_TOP					0x00001000
#define WBC_BOTTOM				0x00002000
#define WBC_CHECKBOXES			0x00010000
#define WBC_ELLIPSIS			0x00020000
#define WBC_SORT				0x00040000
#define WBC_GROUP				0x00080000	// For radio button groups
#define WBC_SINGLE				0x00100000	// Single selection for ListViews
#define WBC_CUSTOMDRAW			0x10000000	// For main windows
#define WBC_NOHEADER			0x10000000	// for list views
#define WBC_TRANSPARENT			0x20000000	// For ImageButtons
#define WBC_MULTISELECT			0x40000000	// For ListViews
#define WBC_DEFAULT				0x00000008	// Control has a border
#define WBC_DEFAULTPOS			(signed)CW_USEDEFAULT	// 0x80000000

// Notification message flags (parameter param of wb_create_window)

#define WBC_DBLCLICK			0x00000040
#define WBC_MOUSEMOVE			0x00000080
#define WBC_MOUSEDOWN			0x00000100
#define WBC_MOUSEUP				0x00000200

#define WBC_KEYDOWN				0x00000400	// Used in wbCheckInput()
#define WBC_KEYUP				0x00000800

#define WBC_GETFOCUS			0x00001000
#define WBC_RESIZE				0x00002000
#define WBC_REDRAW				0x00004000
#define WBC_HEADERSEL			0x00008000

// Additional notification message flags

#define WBC_ALT					0x00000020
#define WBC_CONTROL				0x00000008	// MK_CONTROL
#define WBC_SHIFT				0x00000004	// MK_SHIFT
#define WBC_LBUTTON				0x00000001	// MK_LBUTTON
#define WBC_MBUTTON				0x00000010	// MK_MBUTTON
#define WBC_RBUTTON				0x00000002	// MK_RBUTTON

// Message box and beep types

#define WBC_BEEP				0xFFFFFFFF	// See MessageBeep() doc

// Window states

#define WBC_NORMAL				0
#define WBC_MINIMIZED			1
#define WBC_MAXIMIZED			2

// Area types for wbSetWindowArea()

#define WBC_TITLE				1
#define WBC_MINSIZE				2
#define WBC_MAXSIZE				3


// Listview custom color state
#define WBC_LV_NONE				0
#define WBC_LV_FORE				1
#define WBC_LV_BACK				2

#define WBC_LV_DEFAULT			0
#define WBC_LV_DRAW				1
#define WBC_LV_COLUMNS			2

// RTFEditBox
#define WBC_RTF_TEXT			1





// Windows standard identifiers

#define IDDEFAULT				0			// Used with WBC_NOTIFY, meaning the parent window

// Font attributes

#define FTA_NORMAL				0x00000000
#define FTA_REGULAR				FTA_NORMAL
#define FTA_BOLD				0x00000001
#define	FTA_ITALIC				0x00000002
#define FTA_UNDERLINE			0x00000004
#define FTA_STRIKEOUT			0x00000008

//----------------------------------------------------------------------- MACROS

// General-purpose

#define BITTEST(a, b)		(((a) & (b)) == (b))
#define NUMITEMS(array)		(sizeof(array) / sizeof(*array))
#define EXISTFILE(file)		(_waccess(file, 0) == 0)

#ifndef MIN
#define MIN(a,b)			(((a) < (b)) ? (a) : (b))
#endif
#ifndef MAX
#define MAX(a,b)			(((a) > (b)) ? (a) : (b))
#endif

#define NUL					'\0'

// Structure member aliases

#define M_nMouseCursor		(pwbo->lparams[0])
#define M_bRepeatOn			(pwbo->lparams[1])
#define M_nImageIndex		(pwbo->lparams[2])
#define M_hiImageList		(pwbo->lparams[3])
#define M_nTimerId			(pwbo->lparams[4])
#define M_nMMTimerId		(pwbo->lparams[5])
#define M_ToolTipWnd		(pwbo->lparams[6])

//------------------------------------------------------------------- STRUCTURES

typedef struct {				// ar
	LONG width;
	LONG height;
} AREA;

// WinBinder object (window, control, or menu), stored with SetWindowLong() / GWL_USERDATA

typedef struct _wbo {			// wbo
	HWND hwnd;					// Control or window handle
	UINT id;					// Control identifier
	UINT uClass;				// Object class
	int item;					// Item index
	int subitem;				// Sub-item index
	DWORD style;				// WinBinder style
	struct _wbo *parent;		// Parent window
	LPTSTR pszCallBackFn;		// Callback function
	LPDWORD pszCallBackObj;		// Object for callback method
	LPARAM lparam;				// User-defined parameter
	LONG lparams[8];			// General-purpose parameter array
	RECT rcTitle;				// Title area
	AREA arMin;					// Minimum window area
	AREA arMax;					// Maximum window area
	HBITMAP pbuffer;			// Screen buffer for container windows
} WBOBJ, *PWBOBJ;

// Item description

typedef struct {				// wbi
	UINT id;
	UINT index;
	LPCTSTR pszCaption;
	LPCTSTR pszHint;
	LPCTSTR pszImage;
	LPARAM lparam;				// Additional information
} WBITEM, *PWBITEM;

// Tab control data

typedef struct {				// td
	UINT nCtrls;
	UINT nPages;
	struct {
		HWND hwnd;
	} page[MAX_TABS];
	struct {
		HWND hwnd;
		UINT id;
		BYTE nTab;
	} ctrl[MAX_TABCONTROLS];
} TABDATA, *PTABDATA;

// Treeview data

typedef struct {				// trdt
	HTREEITEM hLast;
	HTREEITEM hParent[MAX_TREEVIEW_LEVELS];
	BOOL bCustomImages[MAX_TREEVIEW_LEVELS];
	int nLastLevel;
} TREEDATA, *PTREEDATA;

// Simplified font structure

typedef struct {				// ft
	LPTSTR pszName;
	int nHeight;
	COLORREF color;
	DWORD dwFlags;
	HFONT hFont;
} FONT, *PFONT;

// Sounds

typedef struct {			// media
	void *pvMedia;			// Data pointer or file name
	DWORD dwLength;			// Size in bytes
	WORD wFormat;			// Sound type
} MEDIA, *PMEDIA;

// Media file formats

typedef enum {				// snf
	SNF_UNKNOWN,
	SNF_WAV,
	SNF_MIDI,
	SNF_AVI,
} MEDIAFILEFORMAT;

typedef struct {
	int   nMode;
	DWORD dwForeground;
	DWORD dwBackground;
} LISTVIEWCOLOR;

//------------------------------------------------- GLOBAL VARIABLE DECLARATIONS

extern HINSTANCE	hAppInstance;		// Instance handle
extern HWND			hCurrentDlg;		// Current dialog box
extern HFONT		hIconFont;			// Icon font
extern WNDPROC		lpfnTabProcOld;		// Original tab control procedure
extern COLORREF		clrTabs;			// Color for tab control backgrounds
extern HBRUSH		hbrTabs;			// Brush for tab control backgrounds

//------------------------------------------------------------ PUBLIC PROTOTYPES

// WB_BITMAP.C

BOOL		wbSaveBitmap(HBITMAP hbm, LPCTSTR pszFileName);
DWORD		wbGetImageDimensions(HBITMAP hbm);
HBITMAP 	wbReplaceColors(HDC cvTarget, HBITMAP hbm, COLORREF clTransparent, COLORREF clNew);
HBITMAP		wbCreateBitmap(int nWidth, int nHeight, BITMAPINFO *hbmpData, void *lpDIBBits);
HBITMAP 	wbCreateMask(HBITMAP hbm, COLORREF clTransparent);
BOOL		wbDestroyBitmap(HBITMAP hbm);
HDC			wbCreateBitmapDC(HBITMAP hbm);
BOOL		wbCopyBits(HDC hdc, HBITMAP hBitmap, int xTarget, int yTarget);
BOOL		wbCopyPartialBits(HDC hdc, HBITMAP hBitmap, int xTarget, int yTarget, int nWidth, int nHeight, int xSource, int ySource);
BOOL		wbMaskPartialBits(HDC hdc, HBITMAP hbmImage, HBITMAP hbmMask, int xTarget, int yTarget, int nWidth, int nHeight, int xSource, int ySource);
//HANDLE		wbLoadImage(LPCTSTR pszImageFile, UINT nIndex);
HANDLE      wbLoadImage(LPCTSTR pszImageFile, UINT nIndex, LPARAM lParam);
DWORD		wbGetBitmapBits(HBITMAP hbm, BYTE **lpBits, BOOL bCompress4to3);
COLORREF	wbGetPixelDirect(unsigned char *pixdata, int xPos, int yPos, BOOL bCompress4to3);

// WB_CONTROL.C

PWBOBJ		wbCreateControl(PWBOBJ pwboParent, UINT uWinBinderClass, LPCTSTR pszCaption, LPCTSTR pszTooltip, int xPos, int yPos, int nWidth, int nHeight, UINT id, DWORD dwWinBinderStyle, long lParam, int nTab);
BOOL		wbDestroyControl(PWBOBJ pwbo);
PWBOBJ		wbGetControl(PWBOBJ pwboParent, int id);
BOOL		wbCreateItem(PWBOBJ pwbo, LPCTSTR pszItemText);
DWORD		wbGetItemCount(PWBOBJ pwbo);
BOOL		wbSetImage(PWBOBJ pwbo, HANDLE hImage, COLORREF clTransp, LPARAM lParam);
PWBOBJ		wbGetFocus(void);
BOOL		wbSetFocus(PWBOBJ pwbo);
DWORD		wbGetValue(PWBOBJ pwbo);
BOOL		wbSetValue(PWBOBJ pwbo, DWORD dwValue);
BOOL		wbSetRange(PWBOBJ pwbo, LONG dwMin, LONG dwMax);
BOOL		wbGetVisible(PWBOBJ pwbo);
BOOL		wbSetVisible(PWBOBJ pwbo, BOOL bState);
BOOL		wbSetStyle(PWBOBJ pwbo, DWORD dwWBStyle, BOOL bSet);
BOOL		wbIsValidClass(UINT uClass);
BOOL		wbGetEnabled(PWBOBJ pwbo);
BOOL		wbSetEnabled(PWBOBJ pwbo, BOOL bState);
BOOL		wbSetText(PWBOBJ pwbo, LPCTSTR pszText, int nItem, BOOL bTooltip);
BOOL		wbGetText(PWBOBJ pwbo, LPTSTR pszText, UINT nMaxChars, int nIndex);
BOOL		wbGetRtfText(PWBOBJ pwbo, char **unc);
UINT		wbGetTextLength(PWBOBJ pwbo, int nIndex);
BOOL		wbDeleteItems(PWBOBJ pwbo, BOOL bClearAll);
BOOL		wbRefreshControl(PWBOBJ pwbo, int xpos, int ypos, int nWidth, int nHeight, BOOL bNow);
DWORD		wbGetSelected(PWBOBJ pwbo);
BOOL		wbSetStatusBarParts(PWBOBJ pwbo, int nParts, int *aWidths);

// WB_CONTROL_COMBO.C

BOOL		wbCreateComboItem(PWBOBJ pwbol, LPCTSTR pszItem, BOOL bCombo);

// WB_CONTROL_LISTVIEW.C

int			wbCreateListViewItem(PWBOBJ pwbo, int nItem, int nImage, LPCTSTR pszText);
//BOOL		wbDeleteListViewItem(PWBOBJ pwbo, int nItem);
BOOL		wbCreateListViewImageList(PWBOBJ pwbo, HBITMAP hbmImage, int nImages, COLORREF clTransparent);
BOOL		wbCreateListViewColumn(PWBOBJ pwbo, int nCol, LPTSTR lpszItem, int nWidth, int nAlign);
BOOL		wbSetListViewItemText(PWBOBJ pwbo, int nItem, int nSubItem, LPCTSTR pszText);
BOOL		wbSetListViewItemChecked(PWBOBJ pwbo, int nItem, BOOL bChecked);
int			wbGetListViewItemCount(PWBOBJ pwbo);
BOOL		wbSetListViewItemImage(PWBOBJ pwbo, int nItem, int nSubItem, int nImage);
BOOL		wbGetListViewItemChecked(PWBOBJ pwbo, int nItem);
BOOL		wbGetListViewItemText(PWBOBJ pwbo, int nItem, int nSubItem, LPTSTR pszText, int nChar);
BOOL		wbClearListViewColumns(PWBOBJ pwbo);
BOOL		wbSelectListViewItem(PWBOBJ pwbo, int nItem, BOOL bState);
BOOL		wbSelectAllListViewItems(PWBOBJ pwbo, BOOL bState);
int			wbGetListViewCheckedItems(PWBOBJ pwbo, int *pbItems);
int			wbGetListViewSelectedItems(PWBOBJ pwbo, int *pbItems);
int			wbGetListViewColumnWidths(PWBOBJ pwbo, int *pwidths);
BOOL		wbSetListViewColumnWidths(PWBOBJ pwbo, int *pwidths);

// WB_CONTROL_MENU.C

PWBOBJ		wbCreateMenu(PWBOBJ pwboParent, PWBITEM pitem[], int nItems);
BOOL		wbGetMenuItemChecked(PWBOBJ pwbo);
BOOL		wbSetMenuItemChecked(PWBOBJ pwbo, BOOL bState);
BOOL		wbSetMenuItemSelected(PWBOBJ pwbo);
BOOL		wbSetMenuItemImage(PWBOBJ pwbo, HANDLE hImage);

// WB_CONTROL_TOOLBAR.C

PWBOBJ		wbCreateToolbar(PWBOBJ pwboParent, PWBITEM pitem[], int nItems, int nBtnWidth, int nBtnHeight, HBITMAP hbm);

// WB_CONTROL_TREEVIEW.C

BOOL		wbCreateTreeViewImageList(PWBOBJ pwbo, HBITMAP hbmImage, int nImages, COLORREF clTransparent);
//BOOL		wbDeleteTreeViewItem(PWBOBJ pwbo, HTREEITEM hItem);
BOOL		wbSetTreeViewItemImages(PWBOBJ pwbo, HTREEITEM hItem, int nImageIndex, int nSelectedImageIndex);
BOOL		wbSetTreeViewItemState(PWBOBJ pwbo, HTREEITEM hItem, BOOL bExpand);
BOOL		wbSetTreeViewItemText(PWBOBJ pwbo, HTREEITEM hItem, LPTSTR lpszItem);
BOOL		wbSetTreeViewItemValue(PWBOBJ pwbo, HTREEITEM hItem, int lParam);
BOOL		wbSetTreeViewItemSelected(PWBOBJ pwbo, HTREEITEM hItem);
BOOL		wbGetTreeViewItemState(PWBOBJ pwbo, HTREEITEM hItem);
BOOL		wbGetTreeViewItemText(PWBOBJ pwbo, HTREEITEM hItem, LPTSTR pszText, int nChar);
LPARAM		wbGetTreeViewItemValue(PWBOBJ pwbo, HTREEITEM hItem);
HTREEITEM	wbAddTreeViewItemChild(PWBOBJ pwbo, HTREEITEM hItem, LPTSTR lpszItem, int lParam, BOOL bSetlParam, int nImageIndex, int nSelectedImageIndex);
HTREEITEM	wbAddTreeViewItemLevel(PWBOBJ pwbo, int nLevel, LPTSTR lpszItem, int lParam, BOOL bSetlParam, int nImageIndex, int nSelectedImageIndex);
HTREEITEM	wbAddTreeViewItemSibling(PWBOBJ pwbo, HTREEITEM hItem, LPTSTR lpszItem, int lParam, BOOL bSetlParam, int nImageIndex, int nSelectedImageIndex);
HTREEITEM	wbGetTreeViewItemParent(PWBOBJ pwbo, HTREEITEM hItem);
HTREEITEM	wbGetTreeViewItemSelected(PWBOBJ pwbo);
HTREEITEM	*wbGetTreeViewItemChildren(PWBOBJ pwbo, HTREEITEM hItem);

// WB_CONTROL_TAB.C

BOOL		wbSetTabControlText(PWBOBJ pwboTab, LPCTSTR pszText);
BOOL		wbCreateTabItem(PWBOBJ pwbo, LPCTSTR pszItem);
BOOL		wbSelectTab(PWBOBJ pwboTab, int nItem);

// WB_DRAW.C

COLORREF	wbGetPixel(HANDLE hImage, int xPos, int yPos);
BOOL		wbSetPixel(HANDLE hImage, int xPos, int yPos, COLORREF color);
BOOL		wbDrawLine(HANDLE handle, int x0, int y0, int x1, int y1, COLORREF cl, UINT nLineWidth, UINT nLineStyle);
BOOL		wbDrawRect(HANDLE handle, int xPos, int yPos, int nWidth, int nHeight, COLORREF cl, BOOL bFilled, UINT nLineWidth, UINT nLineStyle);
BOOL		wbDrawEllipse(HANDLE handle, int xPos, int yPos, int nWidth, int nHeight, COLORREF cl, BOOL bFilled, UINT nLineWidth, UINT nLineStyle);
BOOL		wbDrawText(HANDLE handle, LPCTSTR pszString, int xPos, int yPos, int nWidth, int nHeight, int nFont, DWORD dwFlags);
BOOL		wbDrawBitmap(HANDLE handle, HBITMAP hbmBits, int xPos, int yPos, int nWidth, int nHeight, int xOffset, int yOffset, COLORREF clTransparent);
BOOL		wbGetTextSize(PSIZE psizeText, LPCTSTR pszString, int nFont);


// WB_LOWLEVEL.C

LPARAM		wbSendMessage(PWBOBJ pwbo, UINT uMsg, WPARAM wParam, LPARAM lParam);
HMODULE		wbLoadLibrary(LPCTSTR pszLibName);
FARPROC		wbGetLibraryFunction(HMODULE hLib, LPCSTR pszFunction);
BOOL		wbReleaseLibrary(HMODULE hLib);

// WB_FONTS.C

int			wbCreateFont(LPCTSTR pszName, int nHeight, COLORREF color, DWORD dwFlags);
BOOL		wbSetControlFont(PWBOBJ pwbo, int nFont, BOOL bRedraw);
BOOL		wbDestroyFont(int nFont);
PFONT		wbGetFont(int nFont);

// WB_GENERIC.C

BOOL		wbIsWBObj(void *pwbo, BOOL bShowErrors);
PWBOBJ		wbGetWBObj(HANDLE hwnd);
BOOL		wbConvertLineBreaks(TCHAR **ppszTarget, const TCHAR *pszSource);
LPTSTR		wbStriStr(LPCTSTR pszString, LPCTSTR pszPattern);


// WB_SYSDLG.C

BOOL		wbSysDlgOpen(PWBOBJ pwboParent, LPCTSTR pszTitle, LPCTSTR pszFilter, LPCTSTR pszPath, LPTSTR pszFileName, DWORD style, DWORD bufSize);
BOOL		wbSysDlgSave(PWBOBJ pwboParent, LPCTSTR pszTitle, LPCTSTR pszFilter, LPCTSTR pszPath, LPTSTR pszFileName, LPCTSTR lpstrDefExt);
BOOL		wbSysDlgPath(PWBOBJ pwboParent, LPCTSTR pszTitle, LPCTSTR pszPath, LPTSTR pszSelPath);
COLORREF	wbSysDlgColor(PWBOBJ pwboParent, LPCTSTR pszTitle, COLORREF color);
PFONT		wbSysDlgFont(PWBOBJ pwboParent, LPCTSTR pszTitle, PFONT pfont);

// WB_WINDOW.C

PWBOBJ		wbCreateWindow(PWBOBJ pwboParent, UINT uWinBinderClass, LPCTSTR pszCaption, LPCTSTR pszTooltip, int xPos, int yPos, int nWidth, int nHeight, UINT id, DWORD dwWBStyle, long lParam);
BOOL		wbDestroyWindow(PWBOBJ pwbo);
BOOL		wbSetWindowHandler(PWBOBJ pwbo, LPDWORD pszObj,  LPCTSTR pszHandler);
BOOL		wbSetWindowSize(PWBOBJ pwbo, int nWidth, int nHeight, int nShowMode);
DWORD		wbGetWindowSize(PWBOBJ pwbo, BOOL bClientRect);
DWORD		wbGetWindowPosition(PWBOBJ pwbo, PWBOBJ pwboParent, BOOL bClientRect);
BOOL		wbSetWindowPosition(PWBOBJ pwbo, int xPos, int yPos, PWBOBJ pwboParent);
void *		wbGetAppInfo(LPCTSTR pszInfo, BOOL *pbString, LPTSTR pszString, UINT uLen);
BOOL		wbSetAppInfo(LPCTSTR pszInfo, BOOL *pbString, DWORD dwValue);
BOOL		wbSetTimer(PWBOBJ pwbo, int id, UINT uPeriod);
BOOL		wbSortLVColumn(PWBOBJ pwbo, int nSubItem, BOOL bAscending);
UINT		wbGetControlList(PWBOBJ pwboParent, PWBOBJ pwboList[], UINT nMaxControls);
HWND		wbGetRequestedAppWindow(LPCTSTR pszCaption, BOOL bBringToFront);
BOOL		wbSetWindowArea(PWBOBJ pwbo, UINT type, int xPos, int yPos, int nWidth, int nHeight);

// WB_WINSYS.C

BOOL		wbEnd(void);
BOOL		wbInit(void);
BOOL		wbSetAccel(HACCEL hAccTable);
WPARAM		wbMainLoop(void);
PMEDIA		wbOpenMedia(LPCTSTR pszFileName, DWORD dwFlags);
BOOL		wbControlMedia(PMEDIA pMd, LPCTSTR pszCommand, DWORD dwFlags);
BOOL		wbCloseMedia(PMEDIA pMd);
BOOL		wbPlaySystemSound(int nStyle);
BOOL		wbPlaySound(LPCTSTR pszFileName, LPCTSTR pszCommand);
BOOL 		wbStopSound(LPCTSTR pszCommand);
BOOL		wbShowLastError(LPCTSTR pszCaption, BOOL bMessageBox);
int			wbMessageBox(PWBOBJ pwboParent, LPCTSTR pszText, LPCTSTR pszCaption, UINT nStyle);
BOOL		wbExec(LPCTSTR pszPgm, LPCTSTR pszParm, BOOL bShowWindow);
BOOL		wbFindFile(LPTSTR pszFileName, UINT uLen);
BOOL		wbReadRegistryKey(LPCTSTR pszKey, LPTSTR pszSubKey, LPTSTR pszEntry, LPTSTR pszValue, int *pnBufLen);
BOOL		wbWriteRegistryKey(LPCTSTR pszKey, LPTSTR pszSubKey, LPTSTR pszEntry, LPCTSTR pszValue, DWORD dwValue, BOOL bString);
LONG		wbGetSystemInfo(LPCTSTR pszInfo, BOOL *pbString, LPTSTR pszString, UINT uLen);
BOOL		wbSetAccelerators(PWBOBJ pwbo, LPACCEL paccels, int nCount);
DWORD		wbMakeAccelFromString(const char *pszAccel);
UINT		wbCheckInput(PWBOBJ pwbo, DWORD dwFlags, DWORD dwTimeout);
BOOL		wbSetCursor(PWBOBJ pwbo, LPCTSTR pszCursor, HANDLE handle);

// Library-dependent functions

UINT 		wbCallUserFunction(LPCTSTR pszFunctionName, LPDWORD pszObject, PWBOBJ pwboParent, PWBOBJ pctrl, UINT id, LPARAM lParam1, LPARAM lParam2, LPARAM lParam3);
BOOL		wbError(LPCTSTR szFunction, int nType, LPCTSTR pszFmt, ...);
void *		wbMalloc(size_t size);
void *		wbCalloc(size_t nmemb, size_t size);
BOOL		wbFree(void *ptr);
void *		wbRealloc(void *ptr, size_t size);
char *		wbStrDup(const char *string);
char *		wbStrnDup(const char *string, size_t size);


//------------------------------------------------------------------ END OF FILE

#endif // _WB_H
