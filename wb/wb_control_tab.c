/*******************************************************************************

 WINBINDER - The native Windows binding for PHP

 Copyright © Hypervisual - see LICENSE.TXT for details
 Author: Rubem Pechansky (http://winbinder.org/contact.php)

 Tab control

*******************************************************************************/

//----------------------------------------------------------------- DEPENDENCIES

#include "wb.h"

//------------------------------------------------------------- PUBLIC FUNCTIONS

/* Create tabs according to the tab control title */

BOOL wbSetTabControlText(PWBOBJ pwboTab, LPCTSTR pszText)
{
	TCHAR *szTitle = _wcsdup(pszText);
	TCHAR *ptr = NULL;
	TCHAR *next = NULL;

	if(!pwboTab || !pwboTab->hwnd || !IsWindow(pwboTab->hwnd))
		return FALSE;

	if(!pszText || !*pszText)
		return FALSE;

	SendMessage(pwboTab->hwnd, TCM_DELETEALLITEMS, 0, 0);
	ptr = wcstok(szTitle, TEXT("\r\n,"), &next);
	while(ptr) {
		wbCreateTabItem(pwboTab, ptr);
		ptr = wcstok(NULL, TEXT("\r\n,"), &next);
	}
	return TRUE;
}

/* Create a tab in a tab control */

BOOL wbCreateTabItem(PWBOBJ pwboTab, LPCTSTR pszItem)
{
	TC_ITEM tcitem;
	int nItems;
	PTABDATA pTabData;
	UINT i;
	RECT rc;

	if(!pwboTab || !pwboTab->hwnd || !IsWindow(pwboTab->hwnd))
		return FALSE;

	if(!pszItem || !*pszItem)
		return FALSE;

	tcitem.mask = TCIF_TEXT;
	tcitem.cchTextMax = 0;
	tcitem.iImage = 0;
	tcitem.lParam = 0;
	tcitem.pszText = (LPTSTR)pszItem;

	// Create the tab

	nItems = SendMessage(pwboTab->hwnd, TCM_GETITEMCOUNT, 0, 0);
	SendMessage(pwboTab->hwnd, TCM_INSERTITEM, nItems, (LPARAM) &tcitem);

	// Create the container page

	pTabData = (PTABDATA)pwboTab->lparam;

	pTabData->page[pTabData->nPages].hwnd = CreateWindow(
		TAB_PAGE_CLASS,
		TEXT(""),
//			WS_CHILD | WS_TABSTOP | DS_3DLOOK | DS_SETFONT | DS_CONTROL,
		WS_CHILD,
		0, 0, CW_USEDEFAULT, CW_USEDEFAULT,
		pwboTab->hwnd,
		NULL,
		hAppInstance,
		NULL
	);

	GetClientRect(pwboTab->hwnd, &rc);
	TabCtrl_AdjustRect(pwboTab->hwnd, FALSE, &rc);

	// Adjust page size and visibility

	SetWindowPos(pTabData->page[pTabData->nPages].hwnd, NULL,
	  rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top,
	  SWP_NOACTIVATE| SWP_NOCOPYBITS| SWP_NOOWNERZORDER| SWP_NOREDRAW);

	ShowWindow(pTabData->page[pTabData->nPages].hwnd, pTabData->nPages == 0 ? SW_SHOW : SW_HIDE);

	// Requests all existing child controls that belong to this tab / page

	for(i = 0; i < pTabData->nCtrls; i++) {
		if(pTabData->ctrl[i].nTab == pTabData->nPages) {
			SetParent(pTabData->ctrl[i].hwnd, pTabData->page[pTabData->nPages].hwnd);
		}
	}

//	if(!pTabData->nPages)
//		SetFocus(pTabData->page[0].hwnd);

	pTabData->nPages++;

	return TRUE;
}

/* Selects a tab, showing or displaying the appropriate page and controls */

BOOL wbSelectTab(PWBOBJ pwboTab, int nItem)
{
	int nSelTab = 0;
	UINT i;
	PTABDATA pTabData;

	if(!pwboTab || !pwboTab->hwnd || !IsWindow(pwboTab->hwnd))
		return FALSE;

	nSelTab = TabCtrl_GetCurSel(pwboTab->hwnd);

	if(nSelTab != nItem)
		TabCtrl_SetCurSel(pwboTab->hwnd, nItem);

	nSelTab = TabCtrl_GetCurSel(pwboTab->hwnd);	// New tab index

	// Show / hide pages

	pTabData = (PTABDATA)pwboTab->lparam;
	if(!pTabData)
		return FALSE;
	for(i = 0; i < pTabData->nPages; i++)
		ShowWindow(pTabData->page[i].hwnd, (int)i == nSelTab ? SW_SHOW : SW_HIDE);

	return TRUE;
}

/*LRESULT CALLBACK TabProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg) {

		case WM_COMMAND:		// Passes commands to parent window
		case WM_NOTIFY:
		case WM_HSCROLL:		// For scroll bars
		case WM_VSCROLL:
			{
				HWND hwndParent = GetParent(hwnd);

				if(hwndParent)
					SendMessage(hwndParent, msg, wParam, lParam);
			}
			break;
	}
	return CallWindowProc(lpfnTabProcOld, hwnd, msg, wParam, lParam);
}*/

//------------------------------------------- FUNCTIONS PUBLIC TO WINBINDER ONLY

// Insert the control data in its parent's TABDATA structure

BOOL RegisterControlInTab(PWBOBJ pwboParent, PWBOBJ pwbo, UINT id, UINT nTab)
{
	PWBOBJ pwboTab;
	PTABDATA pTabData;

	if(pwboParent == pwbo)
		return FALSE;

	pwboTab = (PWBOBJ)GetWindowLong(pwboParent->hwnd, GWL_USERDATA);
	if(!pwboTab)
		return FALSE;

	if(pwboTab == pwbo)
		return FALSE;

	pTabData = (PTABDATA)pwboTab->lparam;
	if(!pTabData)
		return FALSE;

	pTabData->ctrl[pTabData->nCtrls].hwnd = pwbo->hwnd;
	pTabData->ctrl[pTabData->nCtrls].id = id;
	pTabData->ctrl[pTabData->nCtrls].nTab = MIN(MIN((UINT)nTab, pTabData->nPages - 1), MAX_TABS);
	pTabData->nCtrls++;

	return TRUE;
}

// Look for children tab controls and assign the handler to them

PWBOBJ AssignHandlerToTabs(HWND hwndParent, LPDWORD pszObj, LPCTSTR pszHandler)
{
	HWND hChild = NULL;
	PWBOBJ poChild = NULL;
	HWND hTabPage;

	// Find the first tab control, if any

	while((hChild = FindWindowEx(hwndParent, hChild, WC_TABCONTROL, NULL)) != NULL) {

		// Found a tab control: assign the handler to it

		poChild = wbGetWBObj(hChild);
		if(!poChild)
			break;

		// An object name was passed

		if(pszObj != NULL) {
			poChild->pszCallBackObj = pszObj;
		} else
			poChild->pszCallBackObj = NULL;

		poChild->pszCallBackFn = (LPTSTR)pszHandler;

//		printf("> %08X\n", poChild);

		// Now, find its child window (must be a TAB_PAGE_CLASS)

		hTabPage = FindWindowEx(poChild->hwnd, NULL, TAB_PAGE_CLASS, NULL);
		if(!hTabPage)
			return NULL;

		// ...and call this function recursively

		AssignHandlerToTabs(hTabPage, pszObj, pszHandler);
	}
	return poChild;
}


//------------------------------------------------------------------ END OF FILE
