/*******************************************************************************

 WINBINDER - The native Windows binding for PHP

 Copyright © Hypervisual - see LICENSE.TXT for details
 Author: Rubem Pechansky (http://winbinder.org/contact.php)

 Bitmap functions

*******************************************************************************/

//----------------------------------------------------------------- DEPENDENCIES

#include "wb.h"
#include <shellapi.h>		// For ExtractIcon()

//-------------------------------------------------------------------- CONSTANTS

#define MAXWRITE	32767

//----------------------------------------------------------------------- MACROS

#define DIBHDRSIZE(pDIB)	(((BITMAPINFOHEADER *)(pDIB))->biSize)
#define DIBWIDTH(pDIB)		((WORD)((BITMAPINFOHEADER *)(pDIB))->biWidth)
#define DIBHEIGHT(pDIB)		((WORD)((BITMAPINFOHEADER *)(pDIB))->biHeight)
#define MAKEEVEN(n)			(ISODD(n) ? (n) + 1 : (n))				// Useful lo line up INTs
#define ISEVEN(n)			(((n) & 0x01) == 0x00)					// TRUE if n is even
#define ISODD(n)			(((n) & 0x01) == 0x01)					// TRUE if n is odd

//---------------------------------------------------------------- PRIVATE TYPES

typedef BYTE *HBMP;

//---------------------------------------------------------- FUNCTION PROTOTYPES

// Private

static HBMP			ReadBitmap(LPCTSTR szBMPFileName);
static DWORD		DIBGetColorCount(HBMP lpDIB);
static void			*DIBGetAddress(HBMP lpDIB);
static void			*SetBitmap(BITMAPINFO *hbmpData, void *lpDIBBits, int nWidth, int nHeight);
static PBITMAPINFO	CreateBitmapInfoStruct(HBITMAP hBmp);
static BOOL			CreateBMPFile(LPCTSTR pszFile, PBITMAPINFO pbi, HBITMAP hBMP, HDC hDC);

// External

extern BOOL IsIcon(HANDLE handle);
extern BOOL IsBitmap(HANDLE handle);
extern char *WideChar2Utf8(LPCTSTR wcs, int *len);

//-------------------------------------------------------------------- VARIABLES

static int pix_cx, pix_cy;				// For low-level bitmap functions

//----------------------------------------------------------- EXPORTED FUNCTIONS

/* Create a blank bitmap. nWidth and nHeight are the dimensions. Return a handle to
  the new bitmap or NULL if failed. Must be followed by a call to wbDestroyBitmap().
*/

HBITMAP wbCreateBitmap(int nWidth, int nHeight, BITMAPINFO *hbmpData, void *lpDIBBits)
{
	HBITMAP hbm;
	HDC hdc;
	BITMAP bm;

	hdc = CreateDC(TEXT("DISPLAY"),NULL,NULL,NULL);
//	hdc = CreateCompatibleDC(NULL);
	if(!hdc) {
		return NULL;
	}

	if(nWidth && nHeight) {

		if(hbmpData && lpDIBBits) {	// In case hbmpData and lpDIBBits are supplied
			hbm = SetBitmap(hbmpData, lpDIBBits, nWidth, nHeight);
			DeleteDC(hdc);
			return hbm;
		}

		hbm = CreateCompatibleBitmap(hdc, nWidth, nHeight);
		if(!hbm) {
			DeleteDC(hdc);
			return NULL;
		}
		bm.bmWidth = nWidth;
		bm.bmHeight = nHeight;
		bm.bmBitsPixel = 24;		// ***** Only 24 bpp bitmaps
		DeleteDC(hdc);
		return hbm;
	} else {
		DeleteDC(hdc);
		return NULL;
	}
}

/* Create a transparent mask from a bitmap, pbm. clTransparent is the transparent color. Return a
  handle to the new mask.
  Must be followed by a call to wbDestroyBitmap().

  // TODO: perform a non-destructive mask creation

*/

HBITMAP wbCreateMask(HBITMAP hbm, COLORREF clTransparent)
{
	HBITMAP hbmMask;
	HDC hdc, cvBits, cvMask;
	BITMAP bm;

	// Create the DCs

	if(!hbm)
		return NULL;
	hdc = GetDC(NULL);
	if(!hdc)
		return NULL;
	cvBits = CreateCompatibleDC(hdc);
	if(!cvBits)
		return NULL;
	cvMask = CreateCompatibleDC(hdc);
	if(!cvMask) {
		DeleteDC(cvBits);
		return NULL;
	}
	ReleaseDC(NULL, hdc);
	GetObject(hbm, sizeof(BITMAP), (LPSTR) &bm);

	// Create the mask

	hbm = SelectObject(cvBits, hbm);
	if(!hbm) {
		DeleteDC(cvMask);
		DeleteDC(cvBits);
	}
	hbmMask = CreateBitmap(bm.bmWidth, bm.bmHeight, 1, 1, NULL);
	hbmMask = SelectObject(cvMask, hbmMask);
	SetBkColor(cvBits, clTransparent);
	BitBlt(cvMask, 0, 0, bm.bmWidth, bm.bmHeight, cvBits, 0, 0, SRCCOPY);
	SetBkColor(cvBits, 0x000000);
	SetTextColor(cvBits, 0xFFFFFF);
	BitBlt(cvBits, 0, 0, bm.bmWidth, bm.bmHeight, cvMask, 0, 0, SRCAND);
	hbmMask = SelectObject(cvMask, hbmMask);
	hbm = SelectObject(cvBits, hbm);

	// Cleanup

	DeleteDC(cvMask);
	DeleteDC(cvBits);
	return hbmMask;
}

/* Destroy the bitmap hbm. */

BOOL wbDestroyBitmap(HBITMAP hbm)
{
	if(!hbm)
		return FALSE;
	return DeleteObject(hbm);
}

/* Copy the entire bitmap hBitmap to hdc at the coordinates xTarget, yTarget. */

BOOL wbCopyBits(HDC hdc, HBITMAP hBitmap, int xTarget, int yTarget)
{
	HDC hdcMem;
	BITMAP bm;
	BOOL bRet;

	if(!hdc || !hBitmap)
		return FALSE;

	GetObject(hBitmap, sizeof(BITMAP), (LPSTR)&bm);

	hdcMem = CreateCompatibleDC(hdc);
	SelectObject(hdcMem, hBitmap);
	bRet = BitBlt(hdc, xTarget, yTarget, bm.bmWidth, bm.bmHeight, hdcMem, 0, 0, SRCCOPY);
	DeleteDC(hdcMem);
	return bRet;
}

/* Copy a rectangle from the bitmap hBitmap to hdc at the coordinates xTarget, yTarget.
  nWidth and nHeight are the dimensions of the rectangle. xSource and ySource are the coordinates of
  the source rectangle. */

BOOL wbCopyPartialBits(HDC hdc, HBITMAP hBitmap, int xTarget, int yTarget,
  int nWidth, int nHeight, int xSource, int ySource)
{
	BOOL bRet;
	HDC hdcMem;

	if(!hdc || !hBitmap)
		return FALSE;

	hdcMem = CreateCompatibleDC(hdc);
	SelectObject(hdcMem, hBitmap);
	bRet = BitBlt(hdc, xTarget, yTarget, nWidth, nHeight, hdcMem, xSource, ySource, SRCCOPY);
	DeleteDC(hdcMem);
	return bRet;
}

// TODO: Use MaskBlt() instead -- see wbDrawBitmap()

BOOL wbMaskPartialBits(HDC hdc, HBITMAP hbmImage, HBITMAP hbmMask, int xTarget, int yTarget,
  int nWidth, int nHeight, int xSource, int ySource)
{
	BOOL bRet;
	HDC hdcImage, hdcMask;

	if(!hdc || !hbmImage)
		return FALSE;

	if(!hbmMask)
		return wbCopyPartialBits(hdc, hbmImage, xTarget, yTarget, nWidth, nHeight, xSource, ySource);

	hdcImage = CreateCompatibleDC(hdc);
	SelectObject(hdcImage, hbmImage);
	hdcMask = CreateCompatibleDC(hdc);
	SelectObject(hdcMask, hbmMask);
	bRet = BitBlt(hdc, xTarget, yTarget, nWidth, nHeight, hdcMask, xSource, ySource, SRCAND);
	if(bRet)
		bRet = BitBlt(hdc, xTarget, yTarget, nWidth, nHeight, hdcImage, xSource, ySource, SRCPAINT);
	DeleteDC(hdcMask);
	DeleteDC(hdcImage);
	return bRet;
}

/* Return a DC for the bitmap hbm. Must be followed by a call to DeleteDC(). */

HDC wbCreateBitmapDC(HBITMAP hbm)
{
	HDC hdc, hdcBmp;

	hdc = GetDC(NULL);
	hdcBmp = CreateCompatibleDC(hdc);
	SelectObject(hdcBmp, hbm);
	ReleaseDC(NULL, hdc);
	return hdcBmp;
}

/* Return the handle to the bitmap or icon from file pszImageFile. */

HANDLE wbLoadImage(LPCTSTR pszImageFile, UINT nIndex, LPARAM lParam)
{
	TCHAR szFile[MAX_PATH];

	if(!pszImageFile || !*pszImageFile)
		return NULL;

	wcsncpy(szFile, pszImageFile, MAX_PATH - 1);

	if(!wbFindFile(szFile, MAX_PATH)) {
		return NULL;
	}

	if(wcsstr(szFile, TEXT(".bmp"))) {								// Load bitmap

		int cxDib, cyDib;
		HBMP hbmpData;
		HBMP lpDIBBits;
		HBITMAP hbm;

		hbmpData = ReadBitmap(szFile);
		if(!hbmpData) {
			wbError(TEXT(__FUNCTION__), MB_ICONWARNING, TEXT("Invalid bitmap format in file %s"), szFile);
			return NULL;
		}

		// Reject old Windows bitmap format

		if(DIBHDRSIZE(hbmpData) == sizeof(BITMAPCOREHEADER))
			return NULL;

		lpDIBBits = DIBGetAddress(hbmpData);
		if(lpDIBBits) {
			cxDib = DIBWIDTH(hbmpData);
			cyDib = DIBHEIGHT(hbmpData);
			hbm = SetBitmap((BITMAPINFO *)hbmpData, lpDIBBits, cxDib, cyDib);
			return hbm;
		} else
			return NULL;

	} else if(wcsstr(szFile, TEXT(".ico")) || wcsstr(szFile, TEXT(".icl")) ||
	  wcsstr(szFile, TEXT(".dll")) || wcsstr(szFile, TEXT(".exe"))) {	 // Load icon

		HICON hIconBuf[1];
		HICON hIcon;

		if(lParam == 0) {
			ExtractIconEx(szFile, nIndex, hIconBuf, NULL, 1);	// Returns a large icon
			hIcon = hIconBuf[0];
		} else if(lParam == 1) {
			ExtractIconEx(szFile, nIndex, NULL, hIconBuf, 1);	// Returns a small icon
			hIcon = hIconBuf[0];
		} else if(lParam == -1) {								// Returns the default icon
			hIcon = ExtractIcon(hAppInstance, szFile, nIndex);
		}

		// Is it a valid ICO, DLL or EXE?

		if(hIcon == (HICON)1) {
			wbError(TEXT(__FUNCTION__), MB_ICONWARNING, TEXT("File %s does not contain any icons."), szFile);
			return NULL;
		} else if(hIcon == NULL) {
			if(nIndex == 0)
				wbError(TEXT(__FUNCTION__), MB_ICONWARNING, TEXT("No icons found in file %s"), szFile);
			else
				wbError(TEXT(__FUNCTION__), MB_ICONWARNING, TEXT("Icon index #%d not found in file %s"), nIndex, szFile);
			return NULL;
		} else {
			return hIcon;
		}

	} else if(wcsstr(szFile, TEXT(".cur")) || wcsstr(szFile, TEXT(".ani"))) {	 // Load cursor

		return LoadCursorFromFile(szFile);

	} else {
		wbError(TEXT(__FUNCTION__), MB_ICONWARNING, TEXT("Unrecognized image format in file %s"), szFile);
		return NULL;
	}
}

DWORD wbGetImageDimensions(HBITMAP hbm)
{
	DWORD dwDim;

	if(!hbm)
		return (DWORD)MAKELONG(-1, -1);

	if(IsIcon(hbm)) {		// Retrieves the icon's bitmap
		HICON hicon;
		ICONINFO ii;

		hicon = (HICON)hbm;
		GetIconInfo(hicon, &ii);
		hbm = ii.hbmColor;
	}

	if(IsBitmap(hbm)) {

		PBITMAPINFO pbmi;

		if(!(pbmi = CreateBitmapInfoStruct(hbm)))
			return FALSE;

		dwDim = (DWORD)MAKELONG(pbmi->bmiHeader.biWidth, pbmi->bmiHeader.biHeight);
		wbFree(pbmi);

	} else {
		return 0;
	}

	return dwDim;
}

BOOL wbSaveBitmap(HBITMAP hbm, LPCTSTR pszFileName)
{
	PBITMAPINFO pbmi;
	HDC hdc;

	hdc = CreateCompatibleDC(NULL);

	if(!(pbmi = CreateBitmapInfoStruct(hbm)))
		return FALSE;

	if(!CreateBMPFile(pszFileName, pbmi, hbm, hdc))
		return FALSE;

	wbFree(pbmi);
	DeleteDC(hdc);
	return TRUE;
}

//------------------------------------------- FUNCTIONS FOR DIRECT BITMAP ACCESS

// Fill up a memory area with the RGB bitmap bit data

DWORD wbGetBitmapBits(HBITMAP hbm, BYTE **lpBits, BOOL bCompress4to3)
{
	HDC hdc;
	PBITMAPINFO pbmi;

	if(!hbm)
		return 0;

	if(!(pbmi = CreateBitmapInfoStruct(hbm)))
		return 0;

    if(!(*lpBits = (LPBYTE)wbMalloc(pbmi->bmiHeader.biSizeImage)))
		return 0;

	if(!(hdc = CreateDC(TEXT("DISPLAY"),NULL,NULL,NULL)))
		return 0;

	if(!GetDIBits(hdc, hbm, 0, (WORD)pbmi->bmiHeader.biHeight, *lpBits, pbmi, DIB_RGB_COLORS)) {
		*lpBits = NULL;
		DeleteDC(hdc);
		return 0;
	}

	pix_cx = pbmi->bmiHeader.biWidth;
	pix_cy = pbmi->bmiHeader.biHeight;

	DeleteDC(hdc);

	// Some applications need RGB (24-bit) data instead of RGBQUAD (32-bit) data

	if(bCompress4to3) {

		int i, x, nLen = pbmi->bmiHeader.biSizeImage;

		// Remove every fourth byte from the original RGBQUAD data

		for(i = 0, x = 0; i < nLen; i += 4, x += 3) {
			*((*lpBits) + x    ) = *((*lpBits) + i    );
			*((*lpBits) + x + 1) = *((*lpBits) + i + 1);
			*((*lpBits) + x + 2) = *((*lpBits) + i + 2);
		}
		return (nLen / 4) * 3;
	} else
		return pbmi->bmiHeader.biSizeImage;
}

COLORREF wbGetPixelDirect(unsigned char *pixdata, int xPos, int yPos, BOOL bCompress4to3)
{
	int i = ((pix_cy - yPos - 1) * pix_cx + xPos) * (bCompress4to3 ? 3 : 4);

	return (COLORREF)(
		((pixdata[i    ]) << 16) +
		((pixdata[i + 1]) << 8) +
		( pixdata[i + 2])
	);
}

//------------------------------------------------------------ PRIVATE FUNCTIONS

/* Use the Windows function SetDIBBits to create a bitmap. nWidth and nHeight are the
  bitmap dimensions. hbmpData points to a BMPINFO structure that contains information
  about the DIB. lpDIBBits points to the DIB color data. Return a HBITMAP handle to
  the new bitmap. */

static void *SetBitmap(BITMAPINFO *hbmpData, void *lpDIBBits, int nWidth, int nHeight)
{
	HBITMAP hbm;
	HDC cvMain;

	cvMain = GetDC(NULL);
	hbm = CreateCompatibleBitmap(cvMain, nWidth, nHeight);
	SetDIBits(cvMain, hbm, 0, nHeight, lpDIBBits, hbmpData, DIB_RGB_COLORS);
	ReleaseDC(NULL, cvMain);
	return hbm;
}

/* Create a handle to the .BMP file, szBMPFileName */

static HBMP ReadBitmap(LPCTSTR szBMPFileName)
{
	BITMAPFILEHEADER bmfh;
	HBMP lpDIB;
	DWORD dwDIBSize, dwOffset, dwHeaderSize ;
	int hFile;
	WORD wDibRead;

	char *str = 0;
	int str_len = 0;

	str = WideChar2Utf8(szBMPFileName, &str_len);

	if(-1 == (hFile = _lopen (str, OF_READ | OF_SHARE_DENY_WRITE))) {
		wbFree(str);
		return NULL;
	}
	wbFree(str);

	if(_lread (hFile, (LPSTR) &bmfh, sizeof (BITMAPFILEHEADER)) != sizeof (BITMAPFILEHEADER)) {
		_lclose (hFile);
		return NULL;
	}

	if(bmfh.bfType != * (WORD *) "BM") {
		_lclose (hFile);
		return NULL;
	}

	dwDIBSize = bmfh.bfSize - sizeof (BITMAPFILEHEADER);
	lpDIB = (HBMP)wbMalloc(dwDIBSize);
	if(!lpDIB) {
		_lclose (hFile);
        return NULL;
	}

	dwOffset = 0 ;
	while(dwDIBSize > 0) {
		wDibRead = (WORD) min(32768, dwDIBSize);
		if(wDibRead != _lread (hFile, (LPSTR) (lpDIB + dwOffset), wDibRead)) {
			_lclose (hFile);
			wbFree(lpDIB);
            return NULL ;
		}
		dwDIBSize -= wDibRead;
		dwOffset += wDibRead;
	}
	_lclose(hFile);
	dwHeaderSize = DIBHDRSIZE(lpDIB);
	if(dwHeaderSize < 12 || (dwHeaderSize > 12 && dwHeaderSize < 16)) {
		wbFree(lpDIB);
        return NULL;
	}
	return lpDIB;
}

/* Return a pointer to the bitmap start */

static void *DIBGetAddress(HBMP lpDIB)
{
	DWORD dwColorTableSize;

	dwColorTableSize = DIBGetColorCount(lpDIB) * sizeof(RGBQUAD);
	return lpDIB + DIBHDRSIZE(lpDIB) + dwColorTableSize;
}

/* Return the number of colors of the DIB */

static DWORD DIBGetColorCount(HBMP lpDIB)
{
	DWORD dwColors;
	WORD wBitCount;

	wBitCount = ((BITMAPINFOHEADER *) lpDIB)->biBitCount;

	if(DIBHDRSIZE (lpDIB) >= 36)
		dwColors = ((BITMAPINFOHEADER *) lpDIB)->biClrUsed;
	else
		dwColors = 0;
	if(dwColors == 0) {

		if(wBitCount != 24)
			dwColors = 1L << wBitCount;
		else
			dwColors = 0;
	}
	return dwColors;
}

/* From Windows Help, "Storing an Image". pbmi (the returned pointer) must be freed after use */

static PBITMAPINFO CreateBitmapInfoStruct(HBITMAP hBmp)
{
	BITMAP bmp;
	PBITMAPINFO pbmi;
	WORD cClrBits;

	// Retrieve the bitmap's color format, width, and height

	if(!GetObject(hBmp, sizeof(BITMAP), (LPSTR)&bmp))
		return NULL;

	// Convert the color format to a count of bits

	cClrBits = (WORD)(bmp.bmPlanes * bmp.bmBitsPixel);

	if(cClrBits == 1)
		cClrBits = 1;
	else if (cClrBits <= 4)
		cClrBits = 4;
	else if (cClrBits <= 8)
		cClrBits = 8;
	else if (cClrBits <= 16)
		cClrBits = 16;
	else if (cClrBits <= 24)
		cClrBits = 24;
	else
		cClrBits = 32;

	// Allocate memory for the BITMAPINFO structure. This structure
	// contains a BITMAPINFOHEADER structure and an array of RGBQUAD data
	// structures

	if(cClrBits != 24) {
		 pbmi = (PBITMAPINFO)wbMalloc(
		   sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * (2 ^ cClrBits));
	} else {
		// There is no RGBQUAD array for the 24-bit-per-pixel format
		pbmi = (PBITMAPINFO) wbMalloc(sizeof(BITMAPINFOHEADER));
	}

	// Initialize the fields in the BITMAPINFO structure

	pbmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	pbmi->bmiHeader.biWidth = bmp.bmWidth;
	pbmi->bmiHeader.biHeight = bmp.bmHeight;
	pbmi->bmiHeader.biPlanes = bmp.bmPlanes;
	pbmi->bmiHeader.biBitCount = bmp.bmBitsPixel;
	if(cClrBits < 24)
		pbmi->bmiHeader.biClrUsed = 2 ^ cClrBits;

	// If the bitmap is not compressed, set the BI_RGB flag

	pbmi->bmiHeader.biCompression = BI_RGB;

	// Compute the number of bytes in the array of color
	// indices and store the result in biSizeImage

	pbmi->bmiHeader.biSizeImage = (pbmi->bmiHeader.biWidth + 7) / 8
	  * pbmi->bmiHeader.biHeight * cClrBits;

	// Set biClrImportant to 0, indicating that all of the
	// device colors are important

	pbmi->bmiHeader.biClrImportant = 0;

	return pbmi;
}

/* From Windows Help, "Storing an Image" */

static BOOL CreateBMPFile(LPCTSTR pszFile, PBITMAPINFO pbi, HBITMAP hBMP, HDC hDC)
{

	HANDLE hf;				  /* file handle */
	BITMAPFILEHEADER hdr;	   /* bitmap file-header */
	PBITMAPINFOHEADER pbih;	 /* bitmap info-header */
	LPBYTE lpBits;			  /* memory pointer */
	DWORD cb;				   /* incremental count of bytes */
	BYTE *hp;				   /* byte pointer */
	DWORD dwTmp;

	pbih = (PBITMAPINFOHEADER) pbi;
	lpBits = (LPBYTE) wbMalloc(pbih->biSizeImage);
	if(!lpBits)
		 return FALSE;

	// Retrieve the color table (RGBQUAD array) and the bits
	// (array of palette indices) from the DIB.

	if (!GetDIBits(hDC, hBMP, 0, (WORD) pbih->biHeight, lpBits, pbi, DIB_RGB_COLORS))
		return FALSE;

	// Create the .BMP file

	hf = CreateFile(pszFile, GENERIC_READ | GENERIC_WRITE, (DWORD) 0,
	  (LPSECURITY_ATTRIBUTES) NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, (HANDLE) NULL);

	if(hf == INVALID_HANDLE_VALUE)
		return FALSE;

	hdr.bfType = 0x4d42;		// 0x42 = "B" 0x4d = "M"

	// Compute size of entire file

	hdr.bfSize = (DWORD)(sizeof(BITMAPFILEHEADER) +
	  pbih->biSize + pbih->biClrUsed * sizeof(RGBQUAD) + pbih->biSizeImage);

	hdr.bfReserved1 = 0;
	hdr.bfReserved2 = 0;

	// Compute the offset to the array of color indices

	hdr.bfOffBits = (DWORD) sizeof(BITMAPFILEHEADER) +
	  pbih->biSize + pbih->biClrUsed * sizeof (RGBQUAD);

	// Copy the BITMAPFILEHEADER into the .BMP file

	if(!WriteFile(hf, (LPVOID) &hdr, sizeof(BITMAPFILEHEADER), (LPDWORD) &dwTmp,
	  (LPOVERLAPPED) NULL))
		return FALSE;

	// Copy the BITMAPINFOHEADER and RGBQUAD array into the file

	if(!WriteFile(hf, (LPVOID) pbih, sizeof(BITMAPINFOHEADER) +
	  pbih->biClrUsed * sizeof (RGBQUAD), (LPDWORD) &dwTmp, (LPOVERLAPPED) NULL))
		return FALSE;

	// Copy the array of color indices into the .BMP file

	cb = pbih->biSizeImage;
	hp = lpBits;
	while(cb > MAXWRITE)  {
		if(!WriteFile(hf, (LPSTR) hp, (int) MAXWRITE, (LPDWORD) &dwTmp, (LPOVERLAPPED) NULL))
			return FALSE;
		cb -= MAXWRITE;
		hp += MAXWRITE;
	}

	if(!WriteFile(hf, (LPSTR) hp, (int) cb, (LPDWORD) &dwTmp, (LPOVERLAPPED) NULL))
		return FALSE;

	// Close the .BMP file

	if(!CloseHandle(hf))
		return FALSE;

	// Free memory


	wbFree((HGLOBAL)lpBits);
	return TRUE;
}

//------------------------------------------------------------------ END OF FILE
