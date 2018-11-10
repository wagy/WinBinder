/*******************************************************************************

 WINBINDER - The native Windows binding for PHP

 Copyright © Hypervisual - see LICENSE.TXT for details
 Author: Rubem Pechansky (http://winbinder.org/contact.php)

 ListView control

*******************************************************************************/

//----------------------------------------------------------------- DEPENDENCIES

#include "wb.h"

//---------------------------------------------------------- FUNCTION PROTOTYPES

// Private

static BOOL wbSetListViewColumnWidth(PWBOBJ pwbo, int nCol, int nWidth);

//----------------------------------------------------------- EXPORTED FUNCTIONS

int wbCreateListViewItem(PWBOBJ pwbo, int nItem, int nImage, LPCTSTR pszText)
{
	int count;
	LV_ITEM lvi;

	if(!pwbo || !pwbo->hwnd || !IsWindow(pwbo->hwnd))
		return FALSE;

	count = (int)SendMessage(pwbo->hwnd, LVM_GETITEMCOUNT, 0, 0);
	if(nItem < 0)
		nItem = count;

//	lvi.mask = (nImage >= 0 ? (LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM) : (LVIF_TEXT | LVIF_PARAM));
	lvi.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;
	lvi.iItem = nItem;
	lvi.iSubItem = 0;
	lvi.state = 0;
	lvi.stateMask = 0;
	lvi.pszText = (LPTSTR)((pszText && *pszText) ? pszText : NULL);
	lvi.cchTextMax = 0;
	lvi.iImage = ((nImage >= 0) ? nImage : -1);
	lvi.lParam = nItem;			// For LVM_SORTITEMS / CompareLVItems()

	return ListView_InsertItem(pwbo->hwnd, &lvi);
}

/*BOOL wbDeleteListViewItem(PWBOBJ pwbo, int nItem)
{
	if(!pwbo || !pwbo->hwnd || !IsWindow(pwbo->hwnd))
		return FALSE;

	return ListView_DeleteItem(pwbo->hwnd, nItem);
}*/

/* Create or destroy an ImageList for the ListView */

BOOL wbCreateListViewImageList(PWBOBJ pwbo, HBITMAP hbmImage, int nImages, COLORREF clTransparent)
{
	HBITMAP hbmMask;
	static HIMAGELIST hi;
	BITMAP bm;

	if(!pwbo || !pwbo->hwnd || !IsWindow(pwbo->hwnd))
		return FALSE;

	if(hbmImage && nImages) {

		GetObject(hbmImage, sizeof(BITMAP), (LPSTR) &bm);

		if((bm.bmWidth == 0) | (bm.bmHeight == 0))
			return FALSE;

		nImages = MAX(1, MIN(nImages, MIN(bm.bmWidth, MAX_IMAGELIST_IMAGES)));

		if(clTransparent != NOCOLOR) {
			hbmMask = wbCreateMask(hbmImage, clTransparent);

			hi = ImageList_Create(bm.bmWidth / nImages, bm.bmHeight, ILC_COLORDDB | ILC_MASK, nImages, 0);
			if(hi) {
				ImageList_Add(hi, hbmImage, hbmMask);
				ListView_SetImageList(pwbo->hwnd, hi, LVSIL_SMALL);
			}
			DeleteObject(hbmMask);
		} else {
			hi = ImageList_Create(bm.bmWidth / nImages, bm.bmHeight, ILC_COLORDDB, nImages, 0);
			ImageList_Add(hi, hbmImage, NULL);
			ListView_SetImageList(pwbo->hwnd, hi, LVSIL_SMALL);
		}
	} else {
		ListView_SetImageList(pwbo->hwnd, NULL, LVSIL_SMALL);
		ImageList_Destroy(hi);
	}
	return TRUE;
}

BOOL wbGetListViewItemChecked(PWBOBJ pwbo, int nItem)
{
	if(!pwbo || !pwbo->hwnd || !IsWindow(pwbo->hwnd))
		return FALSE;

	return (ListView_GetItemState(pwbo->hwnd, nItem, LVIS_STATEIMAGEMASK) & LVIS_CHECKED);
}

BOOL wbSetListViewItemChecked(PWBOBJ pwbo, int nItem, BOOL bChecked)
{
	LV_ITEM lvi;

	if(!pwbo || !pwbo->hwnd || !IsWindow(pwbo->hwnd))
		return FALSE;

	lvi.stateMask = LVIS_STATEIMAGEMASK;
	lvi.state = bChecked ? LVIS_CHECKED : 0x1000;

	return SendMessage(pwbo->hwnd, LVM_SETITEMSTATE, (WPARAM)nItem, (LPARAM)&lvi);
}

// Returns the number of checked items. Optionally fills the array pbItems
// with the checked items indices.

int wbGetListViewCheckedItems(PWBOBJ pwbo, int *pbItems)
{
	int i, nCount, nChecked;

	if(!pwbo || !pwbo->hwnd || !IsWindow(pwbo->hwnd))
		return FALSE;

	nCount = SendMessage(pwbo->hwnd, LVM_GETITEMCOUNT, 0, 0);
	if(!nCount)
		return 0;

	if(pbItems) {
		for(i = 0, nChecked = 0; i < nCount; i++) {
			if(ListView_GetItemState(pwbo->hwnd, i, LVIS_CHECKED))
				pbItems[nChecked++] = i;
		}
	} else {
		for(i = 0, nChecked = 0; i < nCount; i++) {
			if(ListView_GetItemState(pwbo->hwnd, i, LVIS_CHECKED))
				nChecked++;
		}
	}
	return nChecked;
}

/* Returns the number of selected items. Optionally fills the array pbItems
   with the selected item indices. */

int wbGetListViewSelectedItems(PWBOBJ pwbo, int *pbItems)
{
	int i, nCount, nSel;

	if(!pwbo || !pwbo->hwnd || !IsWindow(pwbo->hwnd))
		return FALSE;

	nCount = SendMessage(pwbo->hwnd, LVM_GETITEMCOUNT, 0, 0);
	if(!nCount)
		return 0;

	if(pbItems) {
		for(i = 0, nSel = 0; i < nCount; i++) {
			if(ListView_GetItemState(pwbo->hwnd, i, LVIS_SELECTED))
				pbItems[nSel++] = i;
		}
	} else {
		nSel = SendMessage(pwbo->hwnd, LVM_GETSELECTEDCOUNT, 0, 0);
	}
	return nSel;
}

/* Get the number of columns in the pwbo control, optionally filling up an
  array of integers with column widths */

int wbGetListViewColumnWidths(PWBOBJ pwbo, int *pwidths)
{
 	LV_COLUMN lvct;
	int i;

	if(!pwbo || !pwbo->hwnd || !IsWindow(pwbo->hwnd))
		return FALSE;

	for(i = 0; i < MAX_LISTVIEWCOLS; i++) {
		lvct.iSubItem = i - 0;
		lvct.mask = LVCF_SUBITEM;
		if(pwidths) {
			*(pwidths + i) = ListView_GetColumnWidth(pwbo->hwnd, i);
		}
		if(!ListView_GetColumn(pwbo->hwnd, i, &lvct))
			break;
	}
	return i;
}

/* Set the width on all columnns. */

BOOL wbSetListViewColumnWidths(PWBOBJ pwbo, int *pwidths)
{
 	LV_COLUMN lvct;
	int i;

	if(!pwbo || !pwbo->hwnd || !IsWindow(pwbo->hwnd))
		return FALSE;

	if(!pwidths)
		return FALSE;

	for(i = 0; i < MAX_LISTVIEWCOLS; i++) {
		lvct.iSubItem = i;
		lvct.mask = LVCF_SUBITEM;
		if(!ListView_GetColumn(pwbo->hwnd, i, &lvct))
			break;
		if(!wbSetListViewColumnWidth(pwbo, i, pwidths[i]))
			return FALSE;
	}
	return TRUE;
}

int wbGetListViewItemCount(PWBOBJ pwbo)
{
	if(!pwbo || !pwbo->hwnd || !IsWindow(pwbo->hwnd))
		return FALSE;

	return SendMessage(pwbo->hwnd, LVM_GETITEMCOUNT, 0, 0);
}

/* Sets or clears a listview cell image. Use nImage = -1 to clear the image. */

BOOL wbSetListViewItemImage(PWBOBJ pwbo, int nItem, int nCol, int nImage)
{
	LV_ITEM lvi;

	if(!pwbo || !pwbo->hwnd || !IsWindow(pwbo->hwnd))
		return FALSE;

	lvi.mask = LVIF_IMAGE;
	lvi.iItem = nItem;
	lvi.iSubItem = nCol;
	lvi.iImage = nImage;

	return SendMessage(pwbo->hwnd, LVM_SETITEM, 0, (LPARAM)&lvi);
}

BOOL wbClearListViewColumns(PWBOBJ pwbo)
{
	int i, nCols;

	if(!pwbo || !pwbo->hwnd || !IsWindow(pwbo->hwnd))
		return FALSE;

	nCols = wbGetListViewColumnWidths(pwbo, NULL) - 1;

	// Must delete in reverse, otherwise it won't work

	for(i = nCols; i >= 0; i--)
		ListView_DeleteColumn(pwbo->hwnd, i);
	return TRUE;
}

/* If nWidth is negative, calculate width automatically */

BOOL wbCreateListViewColumn(PWBOBJ pwbo, int nCol, LPTSTR lpszItem, int nWidth, int nAlign)
{
	// Adapted from Dave's Book of Top Ten Lists for Great Windows Programming,
	// By Dave Edson, p. 521

	LV_COLUMN lvc;

	if(!pwbo || !pwbo->hwnd || !IsWindow(pwbo->hwnd))
		return FALSE;

	if(nWidth < 0) {
		SIZE siz;

		if(wbGetTextSize(&siz, lpszItem, 0))
			nWidth = siz.cx + 20;	// This number is empirical and arbitrary
		else
			nWidth = 20;
	}

	// Set up text, width and alignment

	lvc.mask = LVCF_FMT | LVCF_TEXT | LVCF_SUBITEM | LVCF_WIDTH;
	switch(nAlign) {
		case WBC_LEFT:
		default:
			lvc.fmt = LVCFMT_LEFT;
			break;
		case WBC_RIGHT:
			lvc.fmt = LVCFMT_RIGHT;
			break;
		case WBC_CENTER:
			lvc.fmt = LVCFMT_CENTER;
			break;
	}
	lvc.cx = nWidth;

	nCol = wbGetListViewColumnWidths(pwbo, NULL);
	lvc.iSubItem = nCol;
	lvc.pszText  = lpszItem;
	return (ListView_InsertColumn(pwbo->hwnd, nCol, &lvc) != -1);


//	bRet = ListView_InsertColumn(pwbo->hwnd, nCol, &lvc);
//	ListView_SetColumnWidth(pwbo->hwnd, nCol, LVSCW_AUTOSIZE_USEHEADER); // Doesn't work
//	return (bRet != -1);
}

BOOL wbSelectListViewItem(PWBOBJ pwbo, int nItem, BOOL bState)
{
	LV_ITEM lvi;

	if(!pwbo || !pwbo->hwnd || !IsWindow(pwbo->hwnd))
		return FALSE;

	lvi.mask = LVIF_STATE;
	lvi.iItem = nItem;
	lvi.iSubItem = 0;
	lvi.state = bState ? LVIS_SELECTED : 0;
	lvi.stateMask = LVIS_SELECTED;
	lvi.pszText = NULL;
	lvi.cchTextMax = 0;
	lvi.iImage = 0;
	lvi.lParam = 0;

	return SendMessage(pwbo->hwnd, LVM_SETITEMSTATE, nItem, (LPARAM)&lvi);
}

/* Selects / deselects all items */

BOOL wbSelectAllListViewItems(PWBOBJ pwbo, BOOL bState)
{
	int i, count;
	LV_ITEM lvi;

	if(!pwbo || !pwbo->hwnd || !IsWindow(pwbo->hwnd))
		return FALSE;

	lvi.mask = LVIF_STATE;
	lvi.iSubItem = 0;
	lvi.state = bState ? LVIS_SELECTED : 0;
	lvi.stateMask = LVIS_SELECTED;
	lvi.pszText = NULL;
	lvi.cchTextMax = 0;
	lvi.iImage = 0;
	lvi.lParam = 0;

	count = ListView_GetItemCount(pwbo->hwnd);

	for(i = 0; i < count; i++) {
		lvi.iItem = i;
		if(!SendMessage(pwbo->hwnd, LVM_SETITEMSTATE, i, (LPARAM)&lvi))
			return FALSE;
	}
	return TRUE;
}

BOOL wbSetListViewItemText(PWBOBJ pwbo, int nItem, int nCol, LPCTSTR pszText)
{
	LV_ITEM lvi;

	if(!pwbo || !pwbo->hwnd || !IsWindow(pwbo->hwnd))
		return FALSE;

	lvi.mask = (LVIF_TEXT | LVIF_PARAM);
	lvi.iSubItem = nCol;
	lvi.pszText = (LPTSTR)pszText;
//	lvi.lParam = (LPARAM)pszText;

	return SendMessage(pwbo->hwnd, LVM_SETITEMTEXT, nItem, (LPARAM)&lvi);
}

/* Return the text of a single cell */

BOOL wbGetListViewItemText(PWBOBJ pwbo, int nItem, int nCol, LPTSTR pszText, int nChar)
{
	if(!pwbo || !pwbo->hwnd || !IsWindow(pwbo->hwnd))
		return FALSE;

	if(!pszText || !nChar)
		return FALSE;

	if((nItem + 1 > SendMessage(pwbo->hwnd, LVM_GETITEMCOUNT, 0, 0)) ||
	   (nCol + 1 > wbGetListViewColumnWidths(pwbo, NULL))) {		// Does the column exist?
		nChar = 0;
		*pszText = '\0';
		return FALSE;
	}

	ListView_GetItemText(pwbo->hwnd, nItem, nCol, pszText, nChar);

	return (*pszText != '\0');
}

//------------------------------------------------------------ PRIVATE FUNCTIONS

/* Set the width on one columnn. If nWidth is negative, calculate width automatically */

static BOOL wbSetListViewColumnWidth(PWBOBJ pwbo, int nCol, int nWidth)
{
	if(!pwbo || !pwbo->hwnd || !IsWindow(pwbo->hwnd))
		return FALSE;

	if(nWidth < 0)					// Auto size
		nWidth = LVSCW_AUTOSIZE;
	else if(nWidth == 65535)		// Ignore
		return TRUE;

	ListView_SetColumnWidth(pwbo->hwnd, nCol, nWidth);

	return TRUE;
}

//------------------------------------------------------------------ END OF FILE
