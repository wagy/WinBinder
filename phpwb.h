/*******************************************************************************

 WINBINDER - The native Windows binding for PHP for PHP

 Copyright � Hypervisual - see LICENSE.TXT for details
 Author: Rubem Pechansky (http://winbinder.org/contact.php)

 Common header for WinBinder for PHP

*******************************************************************************/

#ifndef _PHP_WB_H
#define _PHP_WB_H


//-------------------------------------------------------------------- CONSTANTS

// WINBINDER constants

#define APPNAME			"WinBinder-PHP"
#define MODULENAME		"winbinder"		// For phpinfo() and extension_loaded()
#define LIBVERSION		WINBINDER_VERSION

// ZEND constants

#if defined _MSC_VER		// Rick
#	if defined(ZEND_EXPORTS)
#		undef ZEND_EXPORTS
#	endif
#	undef ZEND_DEBUG
#endif

#define ZEND_DEBUG	FALSE	// Must be FALSE

#ifndef ZTS
#	define ZTS					// Must define ZTS (Zend Thread Safety)
#endif

#define PHP_REGEX_H			// Skip php_regex.h: '_off_t' not defined anywhere

//----------------------------------------------------------------- DEPENDENCIES

#include "wb/wb.h"
#include <php.h>
#include <windows.h>
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



extern zend_module_entry winbinder_module_entry;
#define phpext_winbinder_ptr &winbinder_module_entry

	//---------------------------------------------------------- AUXILIARY FUNCTIONS

int parse_array(zval *array, const char *fmt, ...);
zval *process_array(zval *zitems);


// String encode converting function

TCHAR * Utf82WideChar(const char *str, int len);
void Utf82WideCharCopy(const char *str, int str_len, TCHAR *wcs, int wcs_len);
char *WideChar2Utf8(LPCTSTR wcs, int *len);
void WideCharCopy(LPCTSTR wcs, char *s, int len);
void dumptcs(TCHAR *str);
char *ConvertUTF16ToUTF8(LPCWSTR pszTextUTF16, int *plen);

//------------------------------------------------------------------ END OF FILE

#endif // _PHP_WB_H
