/*******************************************************************************

 WINBINDER - The native Windows binding for PHP for PHP

 Copyright © Hypervisual - see LICENSE.TXT for details
 Author: Rubem Pechansky (http://winbinder.org/contact.php)

 Library of ZEND-specific functions for the WinBinder extension

*******************************************************************************/

//----------------------------------------------------------------- DEPENDENCIES

#include "phpwb.h"

//-------------------------------------------------------------------- CONSTANTS

#define CALLBACK_ARGS	6	// Number of arguments of the callback function

//----------------------------------------------------------- EXPORTED FUNCTIONS

/*
	TODO: optionally use a message box to display errors and/or log errors to a file
*/

BOOL wbError(LPCTSTR szFunction, int nType, LPCTSTR pszFmt, ...)
{
	TCHAR szString[MAX_ERR_MESSAGE];
	TCHAR szAux[MAX_ERR_MESSAGE];
	char str[MAX_ERR_MESSAGE];

	// Build the string

	va_list args;
	va_start(args, pszFmt);
	vswprintf(szAux, MAX_ERR_MESSAGE, pszFmt, args);
	va_end(args);

	// Prepend the function name

	wcscpy(szString, szFunction);
	wcscat(szString, TEXT(": "));
	wcscat(szString, szAux);

	switch(nType) {
		case MB_OK:
		case MB_ICONINFORMATION:
			nType = E_NOTICE;
			break;

		case MB_ICONWARNING:
			nType = E_WARNING;
			break;

		case MB_ICONSTOP:
		case MB_ICONQUESTION:
		default:
			nType = E_ERROR;
			break;
	}
	WideCharCopy(szString, str, MAX_ERR_MESSAGE);
	zend_error(nType, str);
	return FALSE;
}

// *** The use of parameter pwboParent in wbCallUserFunction() is not clear

UINT wbCallUserFunction(LPCTSTR pszFunctionName, LPDWORD pszObject, PWBOBJ pwboParent, PWBOBJ pctrl, UINT id, LPARAM lParam1, LPARAM lParam2, LPARAM lParam3)
{
	zval fname = { 0 };
	zval return_value = { 0 };
	zval parms[CALLBACK_ARGS];
	BOOL bRet;
	UINT ret = 0;
	char *pszFName;
	char *pszOName;
	zend_string *funName;
	int name_len = 0;

	TSRMLS_FETCH();

	if (pszObject == NULL)
	{
		name_len = 0;
	}

	// Is there a callback function assigned to the window?

	pszFName = WideChar2Utf8(pszFunctionName, &name_len);
	if(!pszFName || !*pszFName) {
		TCHAR szTitle[256];
		char title[256];

		if(GetWindowText(pwboParent->hwnd, szTitle, 256)) {
			WideCharCopy(szTitle, title, 256);
			zend_error(E_WARNING, "%s(): No callback function assigned to window '%s'",
			  get_active_function_name(TSRMLS_C), title);
		} else
			zend_error(E_WARNING, "%s(): No callback function assigned to window #%ld",
			  get_active_function_name(TSRMLS_C), (LONG)pwboParent);
		return FALSE;
	}

	
	ZVAL_STRING(&fname, pszFName);

	/* why we test again ??? GYW
	// Error checking is VERY POOR for user methods (i.e. when pszObjectName is not NULL)
	if(pszObject == NULL && !zend_is_callable(&fname, 0, &funName)) {
		zend_error(E_WARNING, "%s(): '%s' is not a function or cannot be called",
		  get_active_function_name(TSRMLS_C), funName);
		if (funName) efree(funName);				// These two lines prevent a leakage
		return FALSE;
	}
	*/

	// In case of an object
	//if(pszObjectName && *pszObjectName) {
	//	ZVAL_STRING(&oname, pszObjectName);
	//} 


	// PWBOBJ pointer
	ZVAL_LONG(&parms[0], (LONG)pwboParent);
	
	// id
	ZVAL_LONG(&parms[1], (LONG)id);
	
	// control handle
	ZVAL_LONG(&parms[2], (LONG)pctrl);
	
	// lparam1
	ZVAL_LONG(&parms[3], (LONG)lParam1);

	// lparam2
	ZVAL_LONG(&parms[4], (LONG)lParam2);

	// lparam3
	ZVAL_LONG(&parms[5], (LONG)lParam3);

	// Call the user function
	bRet = call_user_function_ex(
		CG(function_table),			// Hash value for the function table
		&pszObject,						// Pointer to an object (may be NULL)
		&fname,						// Function name
		&return_value,				// Return value
		CALLBACK_ARGS,				// Parameter count
		parms,						// Parameter array
		0,							// No separation flag (always 0)
		NULL TSRMLS_CC
		);

	if(bRet != SUCCESS) {
    	zend_error(E_WARNING, "%s(): User function call failed",
    	  get_active_function_name(TSRMLS_C));
	}

	// Free everything we can
	//if (funName) efree(funName);
	switch (Z_TYPE(return_value))
	{
		case IS_LONG:				
		case IS_TRUE:
		case IS_FALSE: ret = Z_LVAL(return_value); break;
	}
	return ret;
}

// Memory-allocation functions

void *wbMalloc(size_t size)
{
	return emalloc(size);
}

void *wbCalloc(size_t nmemb, size_t size)
{
	return ecalloc(nmemb, size);
}

void *wbRealloc(void *ptr, size_t size)
{
	return erealloc(ptr, size);
}

char *wbStrDup(const char *string)
{
	return estrdup(string);
}

char *wbStrnDup(const char *string, size_t size)
{
	return estrndup(string, size);
}

BOOL wbFree(void *ptr)
{
	if(ptr)
		efree(ptr);
	return TRUE;
}

//------------------------------------------------------------------ END OF FILE
