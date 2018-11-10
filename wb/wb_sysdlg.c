/*******************************************************************************

 WINBINDER - The native Windows binding for PHP

 Copyright © Hypervisual - see LICENSE.TXT for details
 Author: Rubem Pechansky (http://winbinder.org/contact.php)

 Functions for common dialog boxes

*******************************************************************************/

//----------------------------------------------------------------- DEPENDENCIES

#include "wb.h"
#include <shlobj.h>
#include <stdio.h>
#include <commdlg.h>

//-------------------------------------------------------------------- CONSTANTS

#define MAXFILTER		1024
#define MAXEXTENSION	256

//----------------------------------------------------------------------- MACROS

//---------------------------------------------------------- FUNCTION PROTOTYPES

// Private

static int CALLBACK BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData);
static LPTSTR DeleteChars(LPTSTR pszMain, UINT nPos, UINT nLength);
static LPTSTR StripPath(LPTSTR pszFileName);

// External

extern LPTSTR MakeWinPath(LPTSTR pszPath);

//-------------------------------------------------------------------- VARIABLES

static TCHAR szSearchPath[MAX_PATH];

//----------------------------------------------------------- EXPORTED FUNCTIONS

BOOL wbSysDlgOpen(PWBOBJ pwboParent, LPCTSTR pszTitle, LPCTSTR pszFilter, LPCTSTR pszPath, LPTSTR pszFileName, DWORD dwWBStyle, DWORD bufSize)
{
	OPENFILENAME ofn = { 0 };
	BOOL bRet;
	TCHAR *pszCopy;

	if(pszPath && *pszPath)
		pszCopy = MakeWinPath(_wcsdup(pszPath));
	else
		pszCopy = NULL;

	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = pwboParent ? (HWND)pwboParent->hwnd : NULL;
	ofn.hInstance = NULL;
	ofn.lpstrFilter = pszFilter && *pszFilter ? pszFilter : TEXT("All files (*.*)\0*.*\0\0");
	ofn.lpstrCustomFilter = NULL;
	ofn.nMaxCustFilter = 0;
	ofn.nFilterIndex = 0;
	ofn.lpstrFile = pszFileName;
	ofn.nMaxFile = bufSize -1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = pszCopy;
	ofn.lpstrTitle = (pszTitle && * pszTitle) ? pszTitle : NULL;
	ofn.Flags = OFN_HIDEREADONLY | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
	ofn.nFileOffset = 0;
	ofn.nFileExtension = 0;
	ofn.lpstrDefExt = NULL;
	ofn.lCustData = 0;
	ofn.lpfnHook = NULL;
	ofn.lpTemplateName = NULL;

	ofn.Flags |= BITTEST(dwWBStyle, WBC_MULTISELECT) ? OFN_ALLOWMULTISELECT | OFN_EXPLORER : 0;

	bRet = GetOpenFileName(&ofn);
	if(!bRet && pszFileName)
		*pszFileName = '\0';

	if(pszCopy)
		free(pszCopy);
	return bRet;
}

BOOL wbSysDlgSave(PWBOBJ pwboParent, LPCTSTR pszTitle, LPCTSTR pszFilter, LPCTSTR pszPath, LPTSTR pszFileName, LPCTSTR lpstrDefExt)
{
	OPENFILENAME ofn;
	BOOL bRet;
	TCHAR *pszCopy;

	if(!pszFileName)
		return FALSE;

	if(pszPath && *pszPath)
		pszCopy = MakeWinPath(_wcsdup(pszPath));
	else
		pszCopy = NULL;

	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = pwboParent ? (HWND)pwboParent->hwnd : NULL;
	ofn.hInstance = NULL;
	ofn.lpstrFilter = pszFilter && *pszFilter ? pszFilter : TEXT("All files (*.*)\0*.*\0\0");
	ofn.lpstrCustomFilter = NULL;
	ofn.nMaxCustFilter = 0;
	ofn.nFilterIndex = 0;
	ofn.lpstrFile = StripPath(MakeWinPath(pszFileName));
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = MAX_PATH;
	ofn.lpstrInitialDir = pszCopy;
	ofn.lpstrTitle = (pszTitle && * pszTitle) ? pszTitle : NULL;
	ofn.Flags = OFN_HIDEREADONLY | OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;
	ofn.nFileOffset = 0;
	ofn.nFileExtension = 0;

	// The lpstrDefExt member (below) does not seem to behave like described
	// in the Windows API documentation. If rather behaves like this:
	// If lpstrFilter is NULL, lpstrDefExt indeed sets the default extension.
	// If lpstrFilter is not NULL, however, the default extension will always
	// be set to the first extension contained in lpstrFilter (which is the
	// desired behavior in most cases), regardless of the contents of the buffer
	// pointed by lpstrDefExt. As expected, if lpstrDefExt is NULL, then a
	// default extension is not appended to the file name.

	ofn.lpstrDefExt = (lpstrDefExt && *lpstrDefExt) ? lpstrDefExt : NULL;

	ofn.lCustData = 0;
	ofn.lpfnHook = NULL;
	ofn.lpTemplateName = NULL;

	bRet = GetSaveFileName(&ofn);
	if(!bRet && pszFileName)
		*pszFileName = '\0';

	if(pszCopy)
		free(pszCopy);
	return bRet;
}

/* TODO: pszTitle does not change the dialog title, but a label instead */

BOOL wbSysDlgPath(PWBOBJ pwboParent, LPCTSTR pszTitle, LPCTSTR pszPath, LPTSTR pszSelPath)
{
	BROWSEINFO bi;
	LPITEMIDLIST pidl;
	LPMALLOC pMalloc;
	BOOL bRet;

	if(SHGetMalloc(&pMalloc) != NOERROR)
		return FALSE;

	// SearchPath is used on the BrowseCallbackProc() function below

	if(!*pszPath)
		*szSearchPath = '\0';
	else
		wcscpy(szSearchPath, pszPath);

	bi.hwndOwner = pwboParent ? (HWND)pwboParent->hwnd : NULL;
	bi.pidlRoot = 0;
	bi.pszDisplayName = pszSelPath;
	bi.lpszTitle = (pszTitle && * pszTitle) ? pszTitle : NULL;
	bi.ulFlags = BIF_RETURNONLYFSDIRS;
	bi.lpfn = BrowseCallbackProc;
    bi.lParam = 0;
    bi.iImage = 0;
	if(!(pidl = SHBrowseForFolder(&bi))) {
		pMalloc->lpVtbl->Free(pMalloc, pidl);
		pMalloc->lpVtbl->Release(pMalloc);
		return FALSE;
	}
	bRet = SHGetPathFromIDList(pidl, pszSelPath);
	if(!bRet) {
		pMalloc->lpVtbl->Free(pMalloc, pidl);
		pMalloc->lpVtbl->Release(pMalloc);
		return FALSE;
	}
	pMalloc->lpVtbl->Free(pMalloc, pidl);
	pMalloc->lpVtbl->Release(pMalloc);
	return TRUE;
}

/* TODO: pszTitle is not used */

COLORREF wbSysDlgColor(PWBOBJ pwboParent, LPCTSTR pszTitle, COLORREF color)
{
	static COLORREF clrCustomColors[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	CHOOSECOLOR cc;
	BOOL bRet;

	clrCustomColors[0] = color;
	cc.lStructSize = sizeof(cc);
	cc.hwndOwner = pwboParent ? (HWND)pwboParent->hwnd : NULL;
	cc.hInstance = NULL;
	cc.rgbResult = color;
	cc.lpCustColors = clrCustomColors;
	cc.Flags = CC_SHOWHELP | CC_RGBINIT | CC_FULLOPEN;
	cc.lCustData = 0;
	cc.lpfnHook = 0;
	cc.lpTemplateName = 0;

	bRet = ChooseColor(&cc);
	if(bRet)
		return cc.rgbResult;
	else
		return NOCOLOR;
}

/* Select an installed font
  TODO: pszTitle is not being used
  TODO: not implemented
*/
/*
PFONT wbSysDlgFont(PWBOBJ pwboParent, LPCTSTR pszTitle, PFONT pfont)
{
	CHOOSEFONT cf;
	LOGFONT lf;

	ZeroMemory(&cf, sizeof(CHOOSEFONT));
	ZeroMemory(&lf, sizeof(LOGFONT));
	cf.lStructSize = sizeof (CHOOSEFONT);
	cf.hwndOwner = (HWND)pwboParent->hwnd;
	cf.lpLogFont = &lf;

	if(pfont) {				// A default font was specified
		cf.Flags = CF_TTONLY | CF_EFFECTS | CF_INITTOLOGFONTSTRUCT;
		cf.rgbColors = pfont->color;
		lf.lfHeight = pfont->nHeight;
		lf.lfWeight = (pfont->dwFlags & FTA_BOLD) ? FW_BOLD : FW_NORMAL;
		lf.lfWeight = (pfont->dwFlags & FTA_ITALIC) ? TRUE : FALSE;
		lf.lfUnderline = (pfont->dwFlags & FTA_UNDERLINE) ? TRUE : FALSE;
		lf.lfCharSet = DEFAULT_CHARSET;
		strncpy(lf.lfFaceName, pfont->pszName, LF_FACESIZE);
	} else
		cf.Flags = CF_TTONLY;

	if(!ChooseFont(&cf))
		return NULL;

	return NULL;
}
*/

//------------------------------------------------------------ PRIVATE FUNCTIONS

/* Callback function for the dialog box Browse For Folder */

static int CALLBACK BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{
	switch(uMsg) {
		case BFFM_INITIALIZED:
			SendMessage(hwnd, BFFM_SETSELECTION, TRUE, (LPARAM)szSearchPath);
			break;
	}
	return 0;
}

/* Remove nLength characters from pszMain, at position nPos. Return pszMain. */

static LPTSTR DeleteChars(LPTSTR pszMain, UINT nPos, UINT nLength)
{
	LPTSTR pszInitialPos, pszPos;

	if(pszMain) {
		if(nPos < wcslen(pszMain)) {
			for(pszInitialPos = pszPos = pszMain + nPos; *pszPos && nLength; ++pszPos, --nLength)
				;
			memmove(pszInitialPos, pszPos, sizeof(TCHAR) * (wcslen(pszPos) + 1));
		}
	}
	return pszMain;
}

/* Remove the path from pszFileName, leaving only the file name plus extension. Return pszFileName. */

static LPTSTR StripPath(LPTSTR pszFileName)
{
	LPTSTR szPos;

	if((szPos = wcschr(pszFileName, '\\')) != NULL)
		DeleteChars(pszFileName, 0, szPos - pszFileName + 1);
	return pszFileName;
}

//------------------------------------------------------------------ END OF FILE
