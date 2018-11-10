/*******************************************************************************

 WINBINDER - The native Windows binding for PHP for PHP

 Copyright © Hypervisual - see LICENSE.TXT for details
 Author: Rubem Pechansky (http://winbinder.org/contact.php)

 ZEND wrapper for common dialog boxes

*******************************************************************************/

//----------------------------------------------------------------- DEPENDENCIES

#include "phpwb.h"
//#include <shlobj.h>
//#include <stdio.h>

//----------------------------------------------------------- EXPORTED FUNCTIONS

ZEND_FUNCTION(wbtemp_sys_dlg_open)
{
	zend_long pwboParent = (long)NULL;
	char *title = "", *filter = "", *path = "";
	int title_len = 0, filter_len = 0, path_len = 0, fileCount = 0;
	zend_long style;
	TCHAR szFile[MAX_PATH_BUFFER] = { 0 };
	TCHAR szDir[MAX_PATH] = { 0 };

	char *file = 0;
	int file_len = 0;

	TCHAR *szTitle = 0;
	TCHAR *szFilter = 0;
	TCHAR *szPath = 0;
	TCHAR thisOne[MAX_PATH], fullPath[MAX_PATH * 2];				

    if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,
	  "l|sssl", &pwboParent, &title, &title_len, &filter, &filter_len, &path, &path_len, &style) == FAILURE)
		return;

	if(pwboParent && !wbIsWBObj((void *)pwboParent, TRUE))
		RETURN_NULL()

	szTitle = Utf82WideChar(title, title_len);
	szFilter = Utf82WideChar(filter, filter_len);
	szPath = Utf82WideChar(path, path_len);

	wbSysDlgOpen((PWBOBJ)pwboParent, szTitle, szFilter, szPath, szFile, style, MAX_PATH_BUFFER);
	
	if(*szFile) 
	{
		if (!BITTEST(style, WBC_MULTISELECT))
		{
				file = WideChar2Utf8(szFile, &file_len);				
				RETURN_STRING(file);
		}
		array_init(return_value);

		TCHAR* ptr = szFile;
		wcscpy(szDir, ptr);
		ptr += wcslen(szDir) +1;
		while (*ptr)
		{
			if (fileCount == 0) wcscat(szDir, L"\\");
			wcscpy(fullPath, szDir);
			wcscat(fullPath, ptr);
			ptr += (wcslen(ptr) + 1);
			file = WideChar2Utf8(fullPath, &file_len);
			add_next_index_string(return_value, file);
			fileCount++;
		}
		if (fileCount == 0)
		{
			file = WideChar2Utf8(szDir, &file_len);
			add_next_index_string(return_value, file);
		}
	} else
		RETURN_STRING("")
}

ZEND_FUNCTION(wbtemp_sys_dlg_save)
{
	zend_long pwboParent = (long)NULL;
	char *title = "", *filter = "", *path = "", *file = "", *defext = "";
	int title_len = 0, filter_len = 0, path_len = 0, file_len = 0, defext_len = 0;
	TCHAR szFile[MAX_PATH] = TEXT("");
	TCHAR* szDefExt = 0;

	TCHAR *szTitle = 0;
	TCHAR *szFilter = 0;
	TCHAR *szPath = 0;

    if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,
	  "l|sssss", &pwboParent, &title, &title_len, &filter, &filter_len, &path, &path_len, &file, &file_len, &defext, &defext_len) == FAILURE)
		return;

	if(pwboParent && !wbIsWBObj((void *)pwboParent, TRUE))
		RETURN_NULL()

	if(*file)
//		strcpy(szFile, file);
		Utf82WideCharCopy(file, file_len, szFile, file_len);
	if(*defext)
//		strcpy(szDefExt, defext);
		szDefExt = Utf82WideChar(defext, defext_len);

	szTitle = Utf82WideChar(title, title_len);
	szFilter = Utf82WideChar(filter, filter_len);
	szPath = Utf82WideChar(path, path_len);

	wbSysDlgSave((PWBOBJ)pwboParent, szTitle, szFilter, szPath, szFile, szDefExt);

	if(*szFile) {
		file = WideChar2Utf8(szFile, &file_len);
		RETURN_STRING(file)
	} else
		RETURN_STRING("")
}

ZEND_FUNCTION(wb_sys_dlg_path)
{
	zend_long pwboParent = (long)NULL;
	char *title = "", *path = "";
	int title_len = 0, path_len = 0;
	TCHAR szSelPath[MAX_PATH] = TEXT("");

	TCHAR *szTitle = 0;
	TCHAR *szPath = 0;
	char *selPath = 0;
	int sel_len = 0;

    if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,
	  "l|ss", &pwboParent, &title, &title_len, &path, &path_len) == FAILURE)
		return;

	if(pwboParent && !wbIsWBObj((void *)pwboParent, TRUE))
		RETURN_NULL()

	szTitle = Utf82WideChar(title, title_len);
	szPath = Utf82WideChar(path, path_len);

	wbSysDlgPath((PWBOBJ)pwboParent, szTitle, szPath, szSelPath);

	if(*szSelPath) {
		selPath = WideChar2Utf8(szSelPath, &sel_len);
		RETURN_STRING(selPath)
	}
	else
		RETURN_STRING("")
}

ZEND_FUNCTION(wb_sys_dlg_color)
{
	zend_long pwboParent = (long)NULL;
	zend_long color = NOCOLOR;
	char *title = "";
	int title_len = 0;

	TCHAR *szTitle = 0;

    if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,
	  "l|sl", &pwboParent, &title, &title_len, &color) == FAILURE)
		return;

	if(pwboParent && !wbIsWBObj((void *)pwboParent, TRUE))
		RETURN_NULL()

	szTitle = Utf82WideChar(title, title_len);
	RETURN_LONG(wbSysDlgColor((PWBOBJ)pwboParent, szTitle, (COLORREF)color));
}

/*
ZEND_FUNCTION(wb_sys_dlg_font)
{
    long pwbparent = (long)NULL;
	char *title = "";
	char *name = "";
	int height = 0, color = 0, flags = 0;
	int title_len = 0, name_len = 0;
	FONT font;

    if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,
	  "|lsslll", &pwbparent, &title, &title_len, &name, &name_len, &height, &color, &flags) == FAILURE)
		return;

	if(pwbo && !wbIsWBObj((void *)pwbo, TRUE))
		RETURN_NULL()

	font.pszName = name;
	font.nHeight = height;
	font.color = color;
	font.dwFlags = flags;
	RETURN_LONG(wbSysDlgFont((PWBOBJ)pwbparent, title, &font));
}
*/

//------------------------------------------------------------------ END OF FILE
