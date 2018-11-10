/*******************************************************************************

 WINBINDER - The native Windows binding for PHP for PHP

 Copyright © Hypervisual - see LICENSE.TXT for details
 Author: Rubem Pechansky (http://winbinder.org/contact.php)

 General-purpose functions (not exported to the ZEND engine)

*******************************************************************************/

//----------------------------------------------------------------- DEPENDENCIES

#include "phpwb.h"

//------------------------------------------------------------- GLOBAL VARIABLES

const char *pszWbobjName = "WinBinder Object";

//------------------------------------------------------------- PUBLIC FUNCTIONS

/* Accepts a limited subset of the parameters accepted by zend_parse_parameters() */

int parse_array(zval *array, const char *fmt, ...)
{
	int i, nelem;
	void *arg;
	zval *entry = NULL;
	va_list ap;
	HashTable *target_hash;
#if (PHP_MAJOR_VERSION >= 5)
	TSRMLS_FETCH();
#endif

	va_start(ap, fmt);

	target_hash = HASH_OF(array);
	if(!target_hash)
		return 0;

	nelem = zend_hash_num_elements(target_hash);
	zend_hash_internal_pointer_reset(target_hash);

	// Parse loop

	for(i = 0; i < (int)strlen(fmt); i++) {

		arg = va_arg(ap, void *);
		if(!arg)
			break;

		// Requested items past the length of the array must return NULL

		if(i >= nelem ) {

			switch(fmt[i]) {

				case 'l':
					*((long *)arg) = 0;
					break;

				case 'd':
					*((double *)arg) = 0;
					break;

				case 's':
					*((long *)arg) = (long)NULL;
					break;

				default:
					zend_error(E_WARNING, "Invalid format character '%c' in function %s()",
					  fmt[i], __FUNCTION__);
					continue;
			}
			continue;

		} else if((entry = zend_hash_get_current_data(target_hash)) == NULL) {

			zend_error(E_WARNING, "Could not retrieve element %d from array in function %s()",
			  i, __FUNCTION__);
			continue;

		} else {

			if(!entry)
				break;

			switch(fmt[i]) {

				case 'l':
					if(Z_TYPE_P(entry) == IS_NULL) {
						*((long *)arg) = (long)NULL;
					} else
						*((long *)arg) = Z_LVAL_P(entry);
					break;

				case 'd':
					if(Z_TYPE_P(entry) == IS_NULL) {
						*((long *)arg) = (long)NULL;
					} else
						*((double *)arg) = Z_DVAL_P(entry);
					break;

				case 's':
					if(Z_TYPE_P(entry) == IS_STRING) {
						*((long *)arg) = (long)(Z_STRVAL_P(entry));
					} else if(Z_TYPE_P(entry) == IS_NULL) {
						*((long *)arg) = (long)NULL;
					} else
						*((long *)arg) = (long)NULL;
					break;

				default:
					zend_error(E_WARNING, "Invalid format string '%s' in function %s()",
					  fmt, __FUNCTION__);
					continue;

			} // switch

		} // else

		if(i < nelem - 1)
			zend_hash_move_forward(target_hash);
	}

	va_end(ap);
	return nelem;
}

/* Function to process arrays. Returns a pointer to a zval element from array zitems or
  NULL if end of array is reached */

zval *process_array(zval *zitems)
{
	static int nelem = 0;
	static int nelems = 0;
	static HashTable *target_hash = NULL;
	zval *entry = NULL;

	if(Z_TYPE_P(zitems) != IS_ARRAY)
		return FALSE;

	// Prepare to read items from zitem array

	if(!nelems && !nelem) {

		target_hash = HASH_OF(zitems);
		if(!target_hash)
			return FALSE;

		nelems = zend_hash_num_elements(target_hash);

		if(!nelems) {

			// Array is empty: reset everything

			target_hash = NULL;
			nelems = 0;
			nelem = 0;
			return NULL;

		} else {

			// Start array

			zend_hash_internal_pointer_reset(target_hash);
			nelem = 0;
		}

	} else {

		if(nelem < nelems - 1) {

			zend_hash_move_forward(target_hash);
			nelem++;

		} else if(nelem == nelems - 1) {

			// End of array: reset everything

			target_hash = NULL;
			nelems = 0;
			nelem = 0;
			return NULL;
		}
	}

	// Get zval data

	if((entry = zend_hash_get_current_data(target_hash)) == NULL)
		zend_error(E_WARNING, "Could not retrieve element %d from array in function %s()",
		  nelem, get_active_function_name());

	return entry;
}

TCHAR * Utf82WideChar(const char *str, int len)
{
	TCHAR *wstr = NULL;
	int wlen = 0;
	if (!str)
		return NULL;

	if (len <= 0)
		len = -1; //len = strlen(str);

	wlen = MultiByteToWideChar(CP_UTF8, 0, str, len, NULL, 0);

	wstr = wbMalloc(sizeof(TCHAR)*(wlen+1));
	wstr[wlen] = '\0';

	MultiByteToWideChar(CP_UTF8, 0, str, len, wstr, wstr + 1);
	return wstr;
}

void Utf82WideCharCopy(const char *str, int str_len, TCHAR *wcs, int wcs_len)
{
	if (!str || !wcs)
		return NULL;

	MultiByteToWideChar(CP_UTF8, 0, str, str_len, wcs, wcs_len);
}

char *WideChar2Utf8(LPCTSTR wcs, int *plen)
{
	char *str = 0;
	int str_len = 0;

	if (!wcs)
		return NULL;

	str_len = WideCharToMultiByte(CP_UTF8, 0, wcs, -1, str, 0, NULL, NULL);
	if (str_len == 0)
		return NULL;

	str = wbMalloc(str_len);
	//str = wbMalloc(str_len + 1);
	//str[str_len] = '\0';
	//str[str_len-1] = '\0';

	if (plen)
		*plen = WideCharToMultiByte(CP_UTF8, 0, wcs, -1, str, str_len, NULL, NULL);
	else
		WideCharToMultiByte(CP_UTF8, 0, wcs, -1, str, str_len, NULL, NULL);
	return str;
}

void WideCharCopy(LPCTSTR wcs, char *s, int len)
{
	if (wcs && s)
		WideCharToMultiByte(CP_UTF8, 0, wcs, -1, s, len, NULL, NULL);
}

void dumptcs(TCHAR *str)
{
	int i;
	int len = wcslen(str);

	printf("Dump String (%d): ", len);
	for (i = 0; i < len; i++) {
		printf("%x,", str[i]);
	}
	printf("\n");
}

//------------------------------------------------------------------ END OF FILE
