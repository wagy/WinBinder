/*******************************************************************************

 WINBINDER - The native Windows binding for PHP

 Copyright © Hypervisual - see LICENSE.TXT for details
 Author: Rubem Pechansky (http://winbinder.org/contact.php)

 Treeview control

*******************************************************************************/

//----------------------------------------------------------------- DEPENDENCIES

#include "wb.h"

//----------------------------------------------------------- EXPORTED FUNCTIONS

/* Return the handle of the selected node in a treeview */

HTREEITEM wbGetTreeViewItemSelected(PWBOBJ pwbo)
{
	TV_ITEM tvi;

	if(!wbIsWBObj(pwbo, TRUE))					// Is it a valid control?
		return 0;

	tvi.hItem = TreeView_GetSelection(pwbo->hwnd);
	if(!tvi.hItem)
		return 0;
	else {
		tvi.mask = TVIF_HANDLE;
		if(TreeView_GetItem(pwbo->hwnd, &tvi))
			return tvi.hItem;
		else
			return 0;
	}
}

/* Select a node in a treeview. If hItem is 0, select the root node. */

BOOL wbSetTreeViewItemSelected(PWBOBJ pwbo, HTREEITEM hItem)
{
	if(!wbIsWBObj(pwbo, TRUE))					// Is it a valid control?
		return 0;

	if((int)hItem < 0)
		return TreeView_SelectItem(pwbo->hwnd, NULL);
	else if(hItem)
		return TreeView_SelectItem(pwbo->hwnd, hItem);
	else
		return TreeView_SelectItem(pwbo->hwnd, TreeView_GetNextItem(pwbo->hwnd, 0, TVGN_ROOT));
}

/* Create an ImageList for a Treeview */

BOOL wbCreateTreeViewImageList(PWBOBJ pwbo, HBITMAP hbmImage, int nImages, COLORREF clTransparent)
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
			ImageList_Add(hi, hbmImage, hbmMask);
			TreeView_SetImageList(pwbo->hwnd, hi, TVSIL_NORMAL);
			DeleteObject(hbmMask);
		} else {
			hi = ImageList_Create(bm.bmWidth / nImages, bm.bmHeight, ILC_COLORDDB, nImages, 0);
			ImageList_Add(hi, hbmImage, NULL);
			TreeView_SetImageList(pwbo->hwnd, hi, TVSIL_NORMAL);
		}
	} else {
		TreeView_SetImageList(pwbo->hwnd, NULL, TVSIL_NORMAL);
		ImageList_Destroy(hi);
	}
	return TRUE;
}

/* Change the caption of the specified node */

BOOL wbSetTreeViewItemText(PWBOBJ pwbo, HTREEITEM hItem, LPTSTR lpszItem)
{
	TV_ITEM tvi;

	if(!pwbo || !pwbo->hwnd || !IsWindow(pwbo->hwnd))
		return FALSE;

	if(!hItem)
		return FALSE;

	// Change the item

	tvi.hItem = hItem;
	tvi.mask = TVIF_HANDLE | TVIF_TEXT;

	if(lpszItem && *lpszItem) {
		tvi.pszText = lpszItem;
		tvi.cchTextMax = wcslen(lpszItem);
	} else {
		tvi.pszText = TEXT("");
		tvi.cchTextMax = 0;
	}

	// UPDATED IN API: Returns a non-zero value if successful, or zero otherwise

	return TreeView_SetItem(pwbo->hwnd, &tvi) != 0 ? TRUE : FALSE;
}

/* Return the caption of the specified node */

BOOL wbGetTreeViewItemText(PWBOBJ pwbo, HTREEITEM hItem, LPTSTR pszText, int nChar)
{
	TV_ITEM tvi;

	if(!pwbo || !pwbo->hwnd || !IsWindow(pwbo->hwnd))
		return FALSE;

	if(!pszText || !nChar)
		return FALSE;

	if(!hItem)
		return FALSE;

	// Change the item

	tvi.hItem = hItem;
	tvi.mask = TVIF_HANDLE | TVIF_TEXT;
	tvi.pszText = pszText;
	tvi.cchTextMax = nChar;

	// UPDATED IN API: Returns a non-zero value if successful, or zero otherwise

	return TreeView_GetItem(pwbo->hwnd, &tvi);
}

/* Return the value of the selected node on a treeview */

LPARAM wbGetTreeViewItemValue(PWBOBJ pwbo, HTREEITEM hItem)
{
	TV_ITEM tvi = { 0 };


	if (!wbIsWBObj(pwbo, TRUE))					// Is it a valid control?
		return 0;

	if (!hItem)
		return 0;

	tvi.hItem = hItem;
	tvi.mask = TVIF_PARAM;
	if (TreeView_GetItem(pwbo->hwnd, &tvi))
		return tvi.lParam;
	else
		return 0;
}

/* Change the value (lParam) of the specified node */

BOOL wbSetTreeViewItemValue(PWBOBJ pwbo, HTREEITEM hItem, int lParam)
{
	TV_ITEM tvi;

	if(!pwbo || !pwbo->hwnd || !IsWindow(pwbo->hwnd))
		return FALSE;

	if(!hItem)
		return FALSE;

	// Change the item

	tvi.hItem = hItem;
	tvi.mask = TVIF_HANDLE | TVIF_PARAM;
	tvi.lParam = lParam;

	// UPDATED IN API: Returns a non-zero value if successful, or zero otherwise

	return TreeView_SetItem(pwbo->hwnd, &tvi) != 0 ? TRUE : FALSE;
}

/* Change the collapsed/expanded state of the specified node */

BOOL wbSetTreeViewItemState(PWBOBJ pwbo, HTREEITEM hItem, BOOL bExpand)
{
	if(!pwbo || !pwbo->hwnd || !IsWindow(pwbo->hwnd))
		return FALSE;

	if(!hItem)
		return FALSE;

//	return TreeView_Expand(pwbo->hwnd, hItem, TVE_TOGGLE);//bExpand ? TVE_EXPAND : TVE_COLLAPSE);
	return TreeView_Expand(pwbo->hwnd, hItem, bExpand ? TVE_EXPAND : TVE_COLLAPSE);
}

/* Return the collapsed/expanded state of the specified node */

BOOL wbGetTreeViewItemState(PWBOBJ pwbo, HTREEITEM hItem)
{
	TV_ITEM tvi;

	if(!pwbo || !pwbo->hwnd || !IsWindow(pwbo->hwnd))
		return FALSE;

	if(!hItem)
		return FALSE;

	// Get item data

	tvi.hItem = hItem;
	tvi.mask = TVIF_HANDLE | TVIF_STATE;
//	tvi.state = TVIS_EXPANDED;
	tvi.stateMask = TVIS_EXPANDED;

	TreeView_GetItem(pwbo->hwnd, &tvi);
	return BITTEST(tvi.state, TVIS_EXPANDED);
}

/* Return a NULL-terminated array with all the children of a specified node */
/*
HTREEITEM *wbGetTreeViewItemChildren(PWBOBJ pwbo, HTREEITEM hItem)
{
	HTREEITEM hLast;
	HTREEITEM **hChildren;

	if(!pwbo || !pwbo->hwnd || !IsWindow(pwbo->hwnd))
		return FALSE;

	if(!hItem)
		return FALSE;

	// Get the first child node

	hLast = TreeView_GetChild(pwbo->hwnd, hItem);
	if(hLast) {

		// Alloc space for the node array

		hChildren = wbMalloc(sizeof(HTREEITEM) * SendMessage(pwbo->hwnd, TVM_GETCOUNT, 0, 0));

		// Get the next children nodes

		while(hLast) {
			hLast = TreeView_GetNextSibling(pwbo->hwnd, hLast);
			if(hLast)
				hChildren = wbRealloc(hChildren, sizeof(HTREEITEM));
		}
	}

	return NULL;
}
*/

/* Return the level of a specified node */

UINT wbGetTreeViewItemLevel(PWBOBJ pwbo, HTREEITEM hItem)
{
	UINT nLevel;

	if(!pwbo || !pwbo->hwnd || !IsWindow(pwbo->hwnd))
		return FALSE;

	if(!hItem)
		return FALSE;

	nLevel = 0;
	while(hItem) {
		hItem = TreeView_GetParent(pwbo->hwnd, hItem);
		nLevel++;
	}

	return nLevel;
}

/* Return the parent of a specified node */

HTREEITEM wbGetTreeViewItemParent(PWBOBJ pwbo, HTREEITEM hItem)
{
	if(!pwbo || !pwbo->hwnd || !IsWindow(pwbo->hwnd))
		return FALSE;

	if(!hItem)
		return FALSE;

	return TreeView_GetParent(pwbo->hwnd, hItem);
}

/* Delete the specified node from the treeview */
/*
BOOL wbDeleteTreeViewItem(PWBOBJ pwbo, HTREEITEM hItem)
{
	if(!pwbo || !pwbo->hwnd || !IsWindow(pwbo->hwnd))
		return FALSE;

	if(hItem)
		return SendMessage(pwbo->hwnd, TVM_DELETEITEM, 0, (LPARAM)(HTREEITEM)hItem);
	else
		return FALSE;
}*/

/* Change the two images of the specified node */

BOOL wbSetTreeViewItemImages(PWBOBJ pwbo, HTREEITEM hItem, int nImageIndex, int nSelectedImageIndex)
{
	TV_ITEM tvi;

	if(!pwbo || !pwbo->hwnd || !IsWindow(pwbo->hwnd))
		return FALSE;

	if(!hItem)
		return FALSE;

	tvi.mask = TVIF_HANDLE;
	if(nImageIndex >= 0)
		tvi.mask |= TVIF_IMAGE;
	if(nSelectedImageIndex >= 0)
		tvi.mask |= TVIF_SELECTEDIMAGE;

	tvi.hItem = hItem;
	tvi.iImage = nImageIndex;
	tvi.iSelectedImage = nSelectedImageIndex;

	return TreeView_SetItem(pwbo->hwnd, &tvi);
}

/* Create a new sibling of hItem */

HTREEITEM wbAddTreeViewItemSibling(PWBOBJ pwbo, HTREEITEM hItem, LPTSTR lpszItem, int lParam, BOOL bSetlParam, int nImageIndex, int nSelectedImageIndex)
{
	TV_ITEM tvi;
	TV_INSERTSTRUCT tvins;
	PTREEDATA ptrdt	= (PTREEDATA)pwbo->lparam;

	if(!pwbo || !pwbo->hwnd || !IsWindow(pwbo->hwnd))
		return FALSE;

	if((!lpszItem || !*lpszItem))
		return NULL;

	// Add the item

	tvi.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | (bSetlParam ? TVIF_PARAM : 0);
	tvi.pszText = lpszItem;
	tvi.cchTextMax = wcslen(lpszItem);
	tvi.lParam = lParam;
	tvi.iImage = (nImageIndex >= 0 ? nImageIndex : 4);	// Default image indices: 4, 5
	tvi.iSelectedImage = (nSelectedImageIndex >= 0 ? nSelectedImageIndex : 5);

	if(!hItem)
		tvins.hParent = TVI_ROOT;
	else
		tvins.hParent = TreeView_GetParent(pwbo->hwnd, hItem);

	tvins.hInsertAfter = hItem;
	tvins.item = tvi;

	// Insert the item

	ptrdt->hLast = TreeView_InsertItem(pwbo->hwnd, &tvins);
	return ptrdt->hLast;
}

/* Create a new child of hItem */

HTREEITEM wbAddTreeViewItemChild(PWBOBJ pwbo, HTREEITEM hItem, LPTSTR lpszItem, int lParam, BOOL bSetlParam, int nImageIndex, int nSelectedImageIndex)
{
	TV_ITEM tvi;
	TV_INSERTSTRUCT tvins;
	PTREEDATA ptrdt	= (PTREEDATA)pwbo->lparam;

	if(!pwbo || !pwbo->hwnd || !IsWindow(pwbo->hwnd))
		return FALSE;

	if((!lpszItem || !*lpszItem))
		return NULL;

	// Add the item

	tvi.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | (bSetlParam ? TVIF_PARAM : 0);
	tvi.pszText = lpszItem;
	tvi.cchTextMax = wcslen(lpszItem);
	tvi.lParam = lParam;
	tvi.iImage = (nImageIndex >= 0 ? nImageIndex : 4);	// Default image indices: 4, 5
	tvi.iSelectedImage = (nSelectedImageIndex >= 0 ? nSelectedImageIndex : 5);

	if(!hItem)
		tvins.hParent = TVI_ROOT;
	else
		tvins.hParent = hItem;

	tvins.hInsertAfter = hItem;
	tvins.item = tvi;

	// Insert the item

	ptrdt->hLast = TreeView_InsertItem(pwbo->hwnd, &tvins);
	return ptrdt->hLast;
}

/* Create a new item in the treeview in the specified level */

HTREEITEM wbAddTreeViewItemLevel(PWBOBJ pwbo, int nLevel, LPTSTR lpszItem, int lParam, BOOL bSetlParam, int nImageIndex, int nSelectedImageIndex)
{
	TV_ITEM tvi;
	TV_INSERTSTRUCT tvins;
	PTREEDATA ptrdt	= (PTREEDATA)pwbo->lparam;

	if(!pwbo || !pwbo->hwnd || !IsWindow(pwbo->hwnd))
		return FALSE;

	if((!lpszItem || !*lpszItem))
		return NULL;

	// Add the item

	tvi.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | (bSetlParam ? TVIF_PARAM : 0);
	tvi.pszText = lpszItem;
	tvi.cchTextMax = wcslen(lpszItem);
	tvi.lParam = lParam;
	tvi.iImage = (nImageIndex >= 0 ? nImageIndex : 4);	// Default image indices: 4, 5
	tvi.iSelectedImage = (nSelectedImageIndex >= 0 ? nSelectedImageIndex : 5);

	// Where to insert the new item?

	nLevel = MAX(0, nLevel);	// Negative levels don't make sense

	if(nLevel == 0) {							//----------- At root level

		tvins.hParent = TVI_ROOT;
		tvins.hInsertAfter = TVI_LAST;
		ptrdt->hParent[nLevel] = tvins.hParent;

	} else if(nLevel > ptrdt->nLastLevel) {		//----------- As a child item

		nLevel = ptrdt->nLastLevel + 1;	// Can't skip levels
		tvins.hParent = ptrdt->hLast;
		tvins.hInsertAfter = TVI_LAST;
		ptrdt->hParent[nLevel] = tvins.hParent;

		if(tvins.hParent && !ptrdt->bCustomImages[ptrdt->nLastLevel]) {

			TV_ITEM tviParent;

			// Change images of parent item to default "folders": applies them
			// only to parent items that are not flagged as custom

			tviParent.mask = TVIF_HANDLE | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
			tviParent.hItem = tvins.hParent;
			tviParent.iImage = 0;	// Default image indices
			tviParent.iSelectedImage = 1;
			TreeView_SetItem(pwbo->hwnd, &tviParent);
		}

	} else if(nLevel == ptrdt->nLastLevel) {	//----------- As a sibling

		tvins.hParent = ptrdt->hParent[nLevel];
		tvins.hInsertAfter = TVI_LAST;
		ptrdt->hParent[nLevel] = tvins.hParent;

	} else {									//----------- Back up 1 or more levels

		tvins.hParent = ptrdt->hParent[(ptrdt->nLastLevel > nLevel)];
		tvins.hInsertAfter = TVI_LAST;
		ptrdt->hParent[nLevel] = tvins.hParent;

	}

	tvins.item = tvi;
	ptrdt->nLastLevel = nLevel;
	ptrdt->bCustomImages[nLevel] = (nImageIndex >= 0);	// Flags custom images

	// Insert the item
	ptrdt->hLast = TreeView_InsertItem(pwbo->hwnd, &tvins);
	return ptrdt->hLast;
}

//------------------------------------------------------------------ END OF FILE
