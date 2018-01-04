/*******************************************************************************

 WINBINDER - The native Windows binding for PHP for PHP

 Copyright © Hypervisual - see LICENSE.TXT for details
 Author: Rubem Pechansky (http://winbinder.org/contact.php)

 ZEND wrapper for low-level functions

*******************************************************************************/

//----------------------------------------------------------------- DEPENDENCIES

#include "phpwb.h"

//-------------------------------------------------------------------- CONSTANTS

#define param09			param[0], param[1], param[2], param[3], param[4], param[5], param[6], param[7], param[8], param[9]

//----------------------------------------------------------- EXPORTED FUNCTIONS


ZEND_FUNCTION(wb_send_message)
{
	zend_long msg, w, l;
	zend_long pwbo;

    if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,
	 "ll|ll", &pwbo, &msg, &w, &l) == FAILURE)
		return;

	RETURN_LONG(wbSendMessage((PWBOBJ)pwbo, (UINT)msg, (WPARAM)w, (LPARAM)l));
}

/* Get the contents of a memory area */

ZEND_FUNCTION(wb_peek)
{
	zend_long address, bytes = 0;
	char *ptr;

    if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,
	  "l|l", &address, &bytes) == FAILURE)
		return;

	if(!address)
		RETURN_NULL();

	bytes = max(0, bytes);
	ptr = (char *)address;
	if(bytes == 0) {							// Want a zero-terminated string?
		if(!IsBadStringPtr(ptr, 32767))
			RETURN_STRINGL(ptr, strlen(ptr), TRUE)
	} else {									// No, want a memory dump
		if(!IsBadReadPtr(ptr, bytes))
			RETURN_STRINGL(ptr, bytes, TRUE)
	}
	zend_error(E_WARNING, "%s(): Cannot read from location %d",
	  get_active_function_name(TSRMLS_C), (int)ptr);
	RETURN_NULL();
}

/* Change the contents of a memory area */

ZEND_FUNCTION(wb_poke)
{
	zend_long address, bytes = 0;
	char *contents;
	int contents_len;
	void *ptr;

    if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,
	  "ls|l", &address, &contents, &contents_len, &bytes) == FAILURE)
		return;

	if(!address) {
		zend_error(E_WARNING, "%s(): Invalid address",
		  get_active_function_name(TSRMLS_C));
		RETURN_NULL();
	}

	if(!contents_len) {
		zend_error(E_WARNING, "%s(): Zero length contents",
		  get_active_function_name(TSRMLS_C));
		RETURN_NULL();
	}

	if(!bytes)
		bytes = contents_len;

	ptr = (void *)address;

	if(IsBadWritePtr(ptr, bytes)) {
		zend_error(E_WARNING, "%s(): Cannot write to location %d",
		  get_active_function_name(TSRMLS_C), (int)ptr);
		RETURN_NULL();
	}

	memcpy(ptr, contents, bytes);

	RETURN_BOOL(TRUE);
}

/*

The parameter must by reference for php 7

Get the address of a string, double or integer
// TODO: get the address of other objects

*/

ZEND_FUNCTION(wb_get_address)
{
    zval *source;

    if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,
	  "z/", &source) == FAILURE)
		return;

	zend_uchar sourcetype = Z_TYPE_P(source);
	if(sourcetype == IS_LONG) {
		RETURN_LONG((LONG)(void *)&Z_LVAL_P(source));
	} else if(sourcetype == IS_TRUE) {
		RETURN_LONG((LONG)(void *)&Z_LVAL_P(source));
	} else if (sourcetype == IS_FALSE) {
		RETURN_LONG((LONG)(void *)&Z_LVAL_P(source));
	} else if(sourcetype == IS_DOUBLE) {
		RETURN_LONG((LONG)(void *)&Z_DVAL_P(source));
	} else if(sourcetype == IS_STRING) {
		RETURN_LONG((LONG)(void *)Z_STRVAL_P(source));
	} else
		RETURN_LONG((LONG)(void *)source)
}

ZEND_FUNCTION(wb_load_library)
{
	char *lib;
	int lib_len;
	LONG hlib;
	
	//TCHAR *wcs = 0; // not sure if this is needed

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,
	  "s", &lib, &lib_len) == FAILURE)
		return;

	hlib = (LONG)wbLoadLibrary(Utf82WideChar(lib, lib_len));
	//hlib = (LONG)wbLoadLibrary(lib);

	if(hlib)
		RETURN_LONG(hlib)
	else {
		zend_error(E_WARNING, "%s(): Unable to locate library %s",
		  get_active_function_name(TSRMLS_C), lib);
		RETURN_NULL();
	}
}

ZEND_FUNCTION(wb_release_library)
{
	zend_long hlib;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,
	  "l", &hlib) == FAILURE)
		return;

	// Is the library handle valid?

	if(IsBadCodePtr((FARPROC)hlib)) {
		zend_error(E_WARNING, "%s(): Invalid library address %d",
		  get_active_function_name(TSRMLS_C), hlib);
		RETURN_NULL();
	}

	RETURN_BOOL(wbReleaseLibrary((HMODULE)hlib));
}

ZEND_FUNCTION(wb_get_function_address)
{
	char *fun;
	int fun_len;
	zend_long addr, hlib = (LONG)NULL;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,
	  "s|l", &fun, &fun_len, &hlib) == FAILURE)
		return;

	// Is the library handle valid?

	if(IsBadCodePtr((FARPROC)hlib)) {
		zend_error(E_WARNING, "%s(): Invalid library address %d",
		  get_active_function_name(TSRMLS_C), hlib);
		RETURN_NULL();
	}

	addr = (LONG)wbGetLibraryFunction((HMODULE)hlib, fun);

	if(addr)
		RETURN_LONG(addr)
	else {
		zend_error(E_WARNING, "%s(): Unable to locate function %s() in library",
		  get_active_function_name(TSRMLS_C), fun);
		RETURN_NULL();
	}
}

ZEND_FUNCTION(wb_call_function)
{
	zend_long addr;
	LONG retval = 0;
	DWORD *param = NULL;
	zval *array = NULL, *entry = NULL;
	int i, nelem = 0;
	HashTable *target_hash;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,
	  "l|a!", &addr, &array) == FAILURE)
		return;

	// Is the address valid?

	if(IsBadCodePtr((FARPROC)addr)) {
		zend_error(E_WARNING, "%s(): Invalid address %d",
		  get_active_function_name(TSRMLS_C), addr);
		RETURN_NULL();
	}

	// Does the function have arguments?

	if(array) {
		target_hash = HASH_OF(array);
		if(target_hash) {

			nelem = zend_hash_num_elements(target_hash);
			zend_hash_internal_pointer_reset(target_hash);

			// Allocate memory for argument array

			param = emalloc(nelem * sizeof(DWORD));
			memset(param, nelem, 0);

			// Loop to read array items

			for(i = 0; i < nelem; i++) {
				if((entry = zend_hash_get_current_data(target_hash)) == NULL) {
					zend_error(E_WARNING, "%s(): Could not retrieve element %d from array",
					  get_active_function_name(TSRMLS_C), i);
					RETURN_NULL();
				}
				switch(Z_TYPE_P(entry)) {

					case IS_ARRAY:				// Invalid types
					case IS_OBJECT:
					case IS_RESOURCE:
						param[i] = (LONG)NULL;
						break;

					case IS_NULL:
						param[i] = (LONG)NULL;
						break;

					case IS_STRING:
						param[i] = (LONG)Z_STRVAL_P(entry);
						break;

					case IS_DOUBLE:
						param[i] = (DWORD)Z_DVAL_P(entry);
						break;

					case IS_LONG:
					case IS_TRUE:
					case IS_FALSE:
						param[i] = Z_LVAL_P(entry);
						break;
				}

				if(i < nelem - 1)
					zend_hash_move_forward(target_hash);
			}
		}
	}

	// This ugly code calls the function with parameters. Maximum is 20 parameters which should be
	// enough for everyone.
	// Should I have to use assembly for an elegant solution?

	switch(nelem) {
		case 0:	 retval = ((FARPROC)addr)(); break;
		case 1:	 retval = ((FARPROC)addr)(param[0]); break;
		case 2:	 retval = ((FARPROC)addr)(param[0], param[1]); break;
		case 3:	 retval = ((FARPROC)addr)(param[0], param[1], param[2]); break;
		case 4:	 retval = ((FARPROC)addr)(param[0], param[1], param[2], param[3]); break;
		case 5:	 retval = ((FARPROC)addr)(param[0], param[1], param[2], param[3], param[4]); break;
		case 6:	 retval = ((FARPROC)addr)(param[0], param[1], param[2], param[3], param[4], param[5]); break;
		case 7:	 retval = ((FARPROC)addr)(param[0], param[1], param[2], param[3], param[4], param[5], param[6]); break;
		case 8:	 retval = ((FARPROC)addr)(param[0], param[1], param[2], param[3], param[4], param[5], param[6], param[7]); break;
		case 9:	 retval = ((FARPROC)addr)(param[0], param[1], param[2], param[3], param[4], param[5], param[6], param[7], param[8]); break;
		case 10: retval = ((FARPROC)addr)(param09); break;
		case 11: retval = ((FARPROC)addr)(param09, param[10]); break;
		case 12: retval = ((FARPROC)addr)(param09, param[10], param[11]); break;
		case 13: retval = ((FARPROC)addr)(param09, param[10], param[11], param[12]); break;
		case 14: retval = ((FARPROC)addr)(param09, param[10], param[11], param[12], param[13]); break;
		case 15: retval = ((FARPROC)addr)(param09, param[10], param[11], param[12], param[13], param[14]); break;
		case 16: retval = ((FARPROC)addr)(param09, param[10], param[11], param[12], param[13], param[14], param[15]); break;
		case 17: retval = ((FARPROC)addr)(param09, param[10], param[11], param[12], param[13], param[14], param[15], param[16]); break;
		case 18: retval = ((FARPROC)addr)(param09, param[10], param[11], param[12], param[13], param[14], param[15], param[16], param[17]); break;
		case 19: retval = ((FARPROC)addr)(param09, param[10], param[11], param[12], param[13], param[14], param[15], param[16], param[17], param[18]); break;
		case 20: retval = ((FARPROC)addr)(param09, param[10], param[11], param[12], param[13], param[14], param[15], param[16], param[17], param[18], param[19]); break;
		default:
			retval = ((FARPROC)addr)(param09, param[10], param[11], param[12], param[13], param[14], param[15], param[16], param[17], param[18], param[19]);
			zend_error(E_WARNING, "Foreign function: maximum allowed is 20 parameters, some arguments ignored in function %s()",
			  get_active_function_name(TSRMLS_C));
			break;
	}

	// Cannot cleanup here
	efree(param);

	RETURN_LONG(retval);
}

PWBOBJ pwndMain;

// Midi callback
void CALLBACK wbMidiInProc(
     DWORD_PTR hMidiIn,
    UINT wMsg,
    DWORD_PTR dwInstance,
    DWORD_PTR dwParam1,
    DWORD_PTR dwParam2
)
{
		
	DWORD param[4];
	//DWORD *ptrParam;
	//pwbo = (PWBOBJ)dwInstance;
	
	param[0] = wMsg;
	param[1] = dwParam1;
	param[2] = dwParam2;
	param[3] = dwInstance;

	/*if (!pwbo && pwndMain)
	{	pwbo = pwndMain;
	}*/

	if(pwndMain) 
	{
		//SendMessage(pwbo->hwnd, wMsg, dwParam1,dwParam2);	
		SendMessage(pwndMain->hwnd, WBWM_MIDI, hMidiIn, &param);	
	}
	
	
	return;
}

ZEND_FUNCTION(wb_get_midi_callback)
{
	RETURN_LONG(wbMidiInProc);
}

// Enum callback
void CALLBACK wbEnumProc(
     HWND hwnd,
    LPARAM lParam
)
{
	
	if(pwndMain) 
	{
		//SendMessage(pwbo->hwnd, wMsg, dwParam1,dwParam2);	
		SendMessage(pwndMain->hwnd, WBWM_ENUM, hwnd, lParam);	
	}
	
	
	return;
}

ZEND_FUNCTION(wb_get_enum_callback)
{
	
	RETURN_LONG(wbEnumProc);
}

DWORD WINAPI wbHookProc(
   int code,
   WPARAM wParam,
   LPARAM lParam
)
{
	DWORD param[2];
	//DWORD *ptrParam;
	//pwbo = (PWBOBJ)dwInstance;
		
	param[0] = wParam;
	param[1] = lParam;	
	
	if(pwndMain) 
	{
		//SendMessage(pwbo->hwnd, wMsg, dwParam1,dwParam2);	
		SendMessage(pwndMain->hwnd, WBWM_HOOK, code, &param);	
	}
	if (code < 0)
	{  CallNextHookEx(0, code, wParam, lParam);
	}

	return 0;
}


ZEND_FUNCTION(wb_get_hook_callback)
{
	RETURN_LONG(wbHookProc);
}

//------------------------------------------------------------------ END OF FILE
