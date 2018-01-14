<?php

//====================================================================
//	WHACK 1.0	by Sippy										[_][X]|
//====================================================================
//	part of WHACK 													  |
//--------------------------------------------------------------------|
//	w32lib.inc.php											4 Apr 08  |
//																	  |
//	OVERVIEW:														  |
//	this file defines all of the functions necessary to call into	  |
//	the Win32 API.  It utilizes Winbinder's low-level functions to	  |
//	do this.														  |
//																	  |
//	*IMPORTANT*:  Remember that some of these functions can be called |
//	from within the child process, which does not instantiate or use  |
//	any of the winbinder/GUI functions or objects that are used in    |
//	the main GUI program - such as $log_RTF for the log frame RTF	  |
//	textarea.  This means you CANNOT call functions that manipulate   |
//	or rely on these items, such as trigger_error() inside whacklib.php!	  |
//	If you call trigger_error(), it will try to write to the $log_RTF obj,  |
//	which doesn't exist because you are in the child process that	  |
//	doesn't use any GUI objects!  Thus, always RETURN error messages  |
//	and handle them properly in the calling routine; or, simply use	  |
//	php's trigger_error() to write the error to a log file -- this	  |
//	should only be done in fatal errors.							  |
//																	  |
//	FUNCTION MANIFEST:												  |
//		call_win32() - calls given function in given library		  |
//		w32_createProcess() - calls win32 CreateProcess()			  |
//		signalSynchronizationEvent() - set state to signaled/nonsig	  |
//		pollSynchronizationEvent() - check signaled state			  |
//		killAnyScanProcesses() - sends the kill signal to all scans	  |
//		win32_flush() - flushes all file/pipe buffers				  |
//																	  |
//	REQUIREMENTS:													  |
//		None.														  |
//																      |
//	REVISION HISTORY:												  |
//																      |
//	18 Apr 08 - version 1.0 released to public						  |
//																	  |
//	------------------------------------------------------------------|
//	WHACK  LICENSE:											  		  |
//	Copyright 2008 Sippy.  All rights reserved.						  |
//																	  |
//	This file is part of WHACK.									 	  |
//																	  |
//  WHACK is free software: you can redistribute it and/or modify	  |
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or |
//  at your option) any later version.								  |
//																	  |
//  WHACK is distributed in the hope that it will be useful,		  |
//  but WITHOUT ANY WARRANTY; without even the implied warranty of	  |
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the	  |
//  GNU General Public License for more details.					  |
//																	  |
//  You should have received a copy of the GNU General Public License |
//  along with WHACK.  If not, see <http://www.gnu.org/licenses/>.    |
//	------------------------------------------------------------------|
//	WINBINDER LICENSE:												  |
//	Copyright (C) 2004-2005 Hypervisual. All rights reserved.			  |
//																	  |
//====================================================================
///////////////////////////////////////////////////////////////////////

define("PROCESS_INFORMATION_PACK","VVVV");
define("PROCESS_INFORMATION_UNPACK","VhProcess/VhThread/VdwProcessId/VdwThreadId");
define("PROCESS_INFORMATION_SIZE",4*4);

define("STARTUPINFO_PACK","VVVVVVVVVVVVvvVVVV");
define("STARTUPINFO_UNPACK","VS_cb/VS_lpReserved/VS_lpDesktop/VS_lpTitle/VS_dwX/VS_dwY/VS_dwXSize/VS_dwYSize/VS_dwXCountChars/VS_dwYCountChars/VS_dwFillAttribute/VS_dwFlags/vS_wShowWindow/vS_cbReserved2/VS_lpReserved2/VS_hStdInput/VS_hStdOutput/VS_hStdError");
define("STARTUPINFO_SIZE",12*4+2*2+4*4);

define("SECURITY_ATTRIBUTES_PACK","VVC");
define("SECURITY_ATTRIBUTES_UNPACK","VSA_nLength/VSA_lpSecurityDescriptor/ISA_bInheritHandle");
define("SECURITY_ATTRIBUTES_SIZE",4+4+1);

define("CREATE_NO_WINDOW",0x08000000);  //this value hides the DOS prompt window
define("STARTF_USESTDHANDLES",0x00000100);
define("HANDLE_FLAG_INHERIT",0x00000001);
define("PIPE_TYPE_MESSAGE",0x00000004);
define("PIPE_READMODE_MESSAGE",0x00000002);
define("PIPE_NOWAIT",0x00000001);
define("STD_INPUT_HANDLE",-10);
define("STD_OUTPUT_HANDLE",-11);
define("STD_ERROR_HANDLE", -12);
define("INVALID_HANDLE_VALUE",-1);

define("INFINITE",0xffffffff);
define("HEAP_ZERO_MEMORY",0x00000008);
define("EVENT_ALL_ACCESS",0x1F0003);
//timeout for ReadFile() to read data from the pipe (in ms)
define("GLOBAL_PARENT_READ_TIMEOUT",5000);



function call_win32($libaddr,$funcname,$args)
{
	$func_addr=wb_get_function_address($funcname,$libaddr);
	
	//this is equivalent of a BSOD!
	if ($func_addr == 0)
	{
		trigger_error("CORE ERROR:  Could not locate function '$funcname' inside given DLL address.", ERROR);
		exit(-1);
	}

	return wb_call_function($func_addr,$args);
}

function w32_CreateProcess($cmd, $cmdline, $withSignal=FALSE)
{
	/*
	**************************************************************************
    **						CREATE COMM PIPES								**
    **************************************************************************
    
    These pipes will be used by the child process to send messages to the parent
    process.  The messages are warnings, errors, or information messages during 
    the scan process.  the parent is constantly looping, waiting for messages, 
    while the child is scanning.
    
    -----------------------------
    	Win32 function prototype:
    -----------------------------
    BOOL WINAPI CreatePipe(
    	__out		PHANDLE hReadPipe,
    	__out		PHANDLE hWritePipe,
    	__in_opt	LPSECURITY_ATTRIBUTES lpPipeAttributes,
    	__in	DWORD nSize
    	);
    */
    
    //define the stdout and stdin handles for this new child process so we can comm with it
	$StdinRead=" ";
	$StdoutRead=" ";
	$StdinWrite=" ";
	$StdoutWrite=" ";

    //create read/write handles to the child process's STDIN (input)
    $NTSUCCESS=call_win32($GLOBALS["WINLIB_KERNEL"],"CreatePipe",Array(wb_get_address($StdinRead),wb_get_address($StdinWrite),pack(SECURITY_ATTRIBUTES_PACK,9,0,1),0));
    
    if ($NTSUCCESS == 0)
	    return Array("CreatePipe() failed.  Could not create child's STDIN pipe.");
	    
    //create read/write handles to the child process's STDOUT (output)
    $NTSUCCESS=call_win32($GLOBALS["WINLIB_KERNEL"],"CreatePipe",Array(wb_get_address($StdoutRead),wb_get_address($StdoutWrite),pack(SECURITY_ATTRIBUTES_PACK,9,0,1),4096));
	    
    if ($NTSUCCESS == 0)
	    return Array("CreatePipe() failed.  Could not create child's STDOUT pipe.");

	// Note that when MSDN mentions pointer - then we use wb_get_address - 
    // but when we talk about handlers - then we have to wb_peek (get the value) and unpack the pointer
    $ar = unpack('V',wb_peek(wb_get_address($StdinRead),4)); $hStdinRead=array_pop($ar);
    $ar = unpack('V',wb_peek(wb_get_address($StdoutRead),4)); $hStdoutRead=array_pop($ar);
    $ar = unpack('V',wb_peek(wb_get_address($StdinWrite),4)); $hStdinWrite=array_pop($ar);
    $ar = unpack('V',wb_peek(wb_get_address($StdoutWrite),4)); $hStdoutWrite=array_pop($ar);   	
	
	//since we are at the moment in the context of the parent process, we need to 
	//save the current handle to STDOUT, so we can restore it after creating child
	$origStdout=call_win32($GLOBALS["WINLIB_KERNEL"],"GetStdHandle",Array(STD_OUTPUT_HANDLE));
	
	if ($origStdout == INVALID_HANDLE_VALUE)
	    return Array("GetStdHandle() failed.  Could not get stdout before creating child process.");
	
	//now set our STDOUT handle for parent process to the STDOUT write of the child process
	//this way, the child process will inherit the appropriate handle
	$NTSUCCESS=call_win32($GLOBALS["WINLIB_KERNEL"],"SetStdHandle",Array(STD_OUTPUT_HANDLE,$hStdoutWrite));
	
	if ($NTSUCCESS == 0)
	    return Array("SetStdHandle() failed.  Could not set the current stdout handle to child's new stdout.");
    //**************************************************************************
    
    if ($withSignal)
    {
    
	    /*
	    **************************************************************************
	    **						CREATE EVENT OBJECTS							**
	    **************************************************************************
	    
	    The event objects are used for interprocess synchronization.  The parent
	    process creates events that are inherited when the child process is
	    created.  All events are Manual-Reset type events, meaning they must be
	    manually signaled and reset.  All events are initialized non-signaled.
	    
	    These are the synchronization events created/inherited:
	    
	    		1) SS_ASK_EVENT
	    		2) SS_KILL_SIGNAL
	    		3) SS_FLUSH_EVENT
	    		    
	    #1 is used for the child process to tell the parent process it needs to 
	    prompt the user for some input.  Usually the use case here is when the 
	    child process needs to ask the parent process to ask the user if they 
	    would like to terminate the scan.  The child process must wait for this
	    response in the GUI from the parent process.  
	    
	    #2 is used by the parent process to send the child process a "kill signal",
	    meaning terminate now.  This is usually done after an unrecoverable error.
	    
	    #3 is used by the parent process to let the child process know that it has
	    read all the data in the pipe and it can safely flush it out.  This signal
	    is sent rapidly, ever 100 milliseconds.
	    */
		$event_name="SS_ASK_EVENT";
		$evt_handle=call_win32($GLOBALS["WINLIB_KERNEL"],"CreateEvent",array(pack(SECURITY_ATTRIBUTES_PACK,9,0,1),1,0,wb_get_address($event_name)));
		
		if ($evt_handle == NULL)
			return Array("CreateEvent() failed.  Could not create SS_ASK_EVENT synchronization event.");
				
		//create named Event called SS_KILL_SIGNAL
		$event_name="SS_KILL_SIGNAL";
		$evt_handle=call_win32($GLOBALS["WINLIB_KERNEL"],"CreateEvent",array(pack(SECURITY_ATTRIBUTES_PACK,9,0,1),1,0,wb_get_address($event_name)));
		
		if ($evt_handle == NULL)
			return Array("CreateEvent() failed.  Could not create SS_KILL_SIGNAL synchronization event.");
			
		//create named Event called SS_FLUSH_EVENT
		$event_name="SS_FLUSH_EVENT";
		$evt_handle=call_win32($GLOBALS["WINLIB_KERNEL"],"CreateEvent",array(pack(SECURITY_ATTRIBUTES_PACK,9,0,1),1,0,wb_get_address($event_name)));
		
		if ($evt_handle == NULL)
			return Array("CreateEvent() failed.  Could not create SS_FLUSH_EVENT synchronization event.");
    }
		
    /*
    **************************************************************************
    **						CREATE CHILD PROCESS							**
    **************************************************************************

	-------------------------
	Win32 function prototype:
	-------------------------
	BOOL WINAPI CreateProcess(
		__in_opt		LPCTSTR 				lpApplicationName,
		__inout_opt		LPTSTR					lpCommandLine,
		__in_opt		LPSECURITY_ATTRIBUTES	lpProcessAttributes,
		__in_opt		LPSECURITY_ATTRIBUTES	lpThreadAttributes,
		__in			BOOL					bInheritHandles,
		__in			DWORD					dwCreationFlags,
		__in_opt		LPVOID					lpEnvironment,
		__in_opt		LPCTSTR					lpCurrentDirectory,
		__in			LPSTARTUPINFO			lpStartupInfo,
		__out			LPPROCESS_INFORMATION	lpProcessInformation
	);
	*/
    
    
	
	$currdir= realpath("..\\php")."\\";
	$lpApplicationName		= NULL;
	//define parameters to win32 API CreateProcess()
	if (!is_null($cmd))
	{
		$lpApplicationName		=wb_get_address($cmd);
		$currdir= realpath(dirname($cmd))."\\";
	}
	trigger_error("currentdir : ".$currdir, INFO);
	
	$lpCommandLine			=wb_get_address($cmdline);
	$lpProcessAttributes	=pack(SECURITY_ATTRIBUTES_PACK,9,0,1);
	$lpThreadAttributes		=pack(SECURITY_ATTRIBUTES_PACK,9,0,1);
	$bInheritHandles		=1;
	$dwCreationFlags		=CREATE_NO_WINDOW;
	$lpEnvironment			=NULL;
	$lpCurrentDirectory		=wb_get_address($currdir);	//since we want the child process to use the same CWD as
	
														//the parent process, we need to explicitly set this..
	//STARTUPINFO structure is 68 bytes in size
	//dwFlags= (STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW) or (0x100 | 0x01)
	//0=wShowWindow=SW_HIDE
	$lpStartupInfo			=pack(STARTUPINFO_PACK,68,0,0,0,0,0,0,0,0,0,0,STARTF_USESTDHANDLES,0,0,0,$hStdinRead,$hStdoutWrite,$hStdoutWrite);
	$lpProcessInformation	=pack(PROCESS_INFORMATION_PACK,0,0,0,0); //this will be filled by CreateProcess()

	$args=Array($lpApplicationName, $lpCommandLine, $lpProcessAttributes, $lpThreadAttributes, $bInheritHandles, $dwCreationFlags, $lpEnvironment, $lpCurrentDirectory, $lpStartupInfo, $lpProcessInformation );
	
	$NTSUCCESS=call_win32($GLOBALS["WINLIB_KERNEL"],"CreateProcess",$args);

	$proc_info=unpack(PROCESS_INFORMATION_UNPACK,$lpProcessInformation);
		
	//eventually replace this with WaitForMultipleObjects and add in the "ESC" key, so we can 
	//stop the scan if the process hangs.
	
	if ($NTSUCCESS == 0)
		return Array("CreateProcess() failed.  Could not create child process!");

	//**************************************************************************
		
	//cleanup...
	    
	//we have four pipes open - stdin R/W and stdout R/W
	//now close StdoutWrite and hStdinRead, we don't need them anymore; child has inherited its own copy of the handles
	
	$NTSUCCESS=call_win32($GLOBALS["WINLIB_KERNEL"],"CloseHandle",Array($hStdoutWrite));
	if ($NTSUCCESS == 0)
	    return Array("CloseHandle() failed.  Could not destroy parent's handle to child stdout (W).");
	
	$NTSUCCESS=call_win32($GLOBALS["WINLIB_KERNEL"],"CloseHandle",Array($hStdinRead));
	if ($NTSUCCESS == 0)
	    return Array("CloseHandle() failed.  Could not destroy parent's handle to child stdin (R).");
	$NTSUCCESS=call_win32($GLOBALS["WINLIB_KERNEL"],"CloseHandle",Array($hStdinWrite));
	if ($NTSUCCESS == 0)
	    return Array("CloseHandle() failed.  Could not destroy parent's handle to child stdin (W).");
	   
	//and finally, reset the parent process's stdout to original value
	$NTSUCCESS=call_win32($GLOBALS["WINLIB_KERNEL"],"SetStdHandle",Array(STD_OUTPUT_HANDLE,$origStdout));
	
	if ($NTSUCCESS == 0)
	    return Array("SetStdHandle() failed.  Could not restore parent's original stdout.");
	    
	//
	//NOTE:  WE ARE LEAVING A HANDLE OPEN!
	//		this handle points to the read end of the child's STDOUT.
	//		the parent process needs this handle, so we will rely on it to close it as well
	//
	//return the unpacked data (which is process info) and the handle to the child process' Stdout
	return Array($proc_info,$hStdoutRead);
}

function w32_ExitProcess($exit_code)
{
	call_win32($GLOBALS["WINLIB_KERNEL"],"ExitProcess",array($exit_code));
} 

function getProcessState($proc_handle)
{
	$process_state=0;

	$NTSUCCESS=call_win32($GLOBALS["WINLIB_KERNEL"],"GetExitCodeProcess",array($proc_handle,wb_get_address($process_state)));

	if ($NTSUCCESS == 0)
	{
		trigger_error("FATAL:  Could not retrieve status of scan process in getProcessState() in w32lib.inc.php.", ERROR);
		return -125;
	}
	
	return $process_state;
}

function signalSynchronizationEvent($event_name,$signaled)
{
	//open the synchronization event
	$evt_handle=call_win32($GLOBALS["WINLIB_KERNEL"],"OpenEvent",array(EVENT_ALL_ACCESS,0,wb_get_address($event_name)));
		    
	if ($evt_handle == NULL)
	{
		trigger_error("signalSynchronizationEvent():  Could not OpenEvent() $event_name.", ERROR);
		return false;
	}
	
	//set the Event state to SIGNALED
	if ($signaled == 1)
	{
		$NTSUCCESS=call_win32($GLOBALS["WINLIB_KERNEL"],"SetEvent",array($evt_handle));
	
		if ($NTSUCCESS == 0)
		{
			trigger_error("signalSynchronizationEvent():  Could not SetEvent() $event_name.", ERROR);
			return false;
		}
	}
	//set the Event state to NON-SIGNALED
	else if ($signaled == 0)
	{
		//and set the event to non-signaled
		$NTSUCCESS=call_win32($GLOBALS["WINLIB_KERNEL"],"ResetEvent",array($evt_handle));
		
		if ($NTSUCCESS == 0)
		{
			trigger_error("signalSynchronizationEvent():  Could not ResetEvent() $event_name.", ERROR);
			return false;
		}
	}
	
	//destroy the handle - note, we dont care about whether or not it succeeds
	//its handle will be destroyed on process exit
	$NTSUCCESS=call_win32($GLOBALS["WINLIB_KERNEL"],"CloseHandle",array($evt_handle));
	
	if ($NTSUCCESS == 0)
		trigger_error("signalSynchronizationEvent():  Could not CloseHandle() $event_name.", ERROR);
		
	return true;
}


function pollSynchronizationEvent($event,$reset,$wait)
{
	//open the synchronization event I inherited from my parent process
	$evt_handle=call_win32($GLOBALS["WINLIB_KERNEL"],"OpenEvent",array(EVENT_ALL_ACCESS,0,wb_get_address($event)));
		    
	if ($evt_handle == NULL)
		return "<BR><BR><font color=red><B>ERROR:</b>  Child process could not open $event synchronization Event.</font>";
	
	//in some cases, we need to insure the event is in the non-signaled state before we wait
	if ($reset)
	{
		$NTSUCCESS=call_win32($GLOBALS["WINLIB_KERNEL"],"ResetEvent",array($evt_handle));
		
		if ($NTSUCCESS == 0)
			return "<BR><BR><font color=red><B>ERROR:</b>  Child process could not reset signal for $event synchronization Event.</font>";
	}
	
	//wait for the event to be signaled
	$NTSIGNAL=call_win32($GLOBALS["WINLIB_KERNEL"],"WaitForSingleObject",array($evt_handle,$wait));
	
	//destroy the handle - note, we dont care about whether or not it succeeds
	//its handle will be destroyed on process exit
	$NTSUCCESS=call_win32($GLOBALS["WINLIB_KERNEL"],"CloseHandle",array($evt_handle));
	
	//$d=$NTSIGNAL==258 ? "WAIT_TIMEOUT" : "WAIT_OBJECT_0";
	//trigger_error("DEBUG:  pollSynchronizationEvent():  WaitForSingleObject($event) = $d", ERROR);
	
	//if we get a signal back, that means the event was signaled
	if ($NTSIGNAL == WAIT_OBJECT_0)
		return true;
	else
		return false;
}

function killAnyScanProcesses()
{
	$event="SS_KILL_SIGNAL";
	
	//signal the child process to terminate..
	signalSynchronizationEvent($event,1);
	
	//we MUST wait until we receive notification from the child process that it has RECEIVED our
	//request to terminate.. then we are confident we won't create an orphaned process!!
	$evt_handle=call_win32($GLOBALS["WINLIB_KERNEL"],"OpenEvent",array(EVENT_ALL_ACCESS,0,wb_get_address($event)));
	
	$NTSIGNAL=call_win32($GLOBALS["WINLIB_KERNEL"],"WaitForSingleObject",array($evt_handle,INFINITE));
	
	return;
}

function clearAllSignals()
{
	$events=Array("SS_ASK_EVENT","SS_KILL_SIGNAL","SS_FLUSH_EVENT");
	
	for ($i=0;$i<count($events);$i++)
	{
		$event=$events[$i];
		
		//open the synchronization event I inherited from my parent process
		$evt_handle=call_win32($GLOBALS["WINLIB_KERNEL"],"OpenEvent",array(EVENT_ALL_ACCESS,0,wb_get_address($event)));
		    
		if ($evt_handle == NULL)
			continue;

		$NTSUCCESS=call_win32($GLOBALS["WINLIB_KERNEL"],"ResetEvent",array($evt_handle));
	}
}
	
function win32_sleep($ms)
{
	$NTSUCCESS=call_win32($GLOBALS["WINLIB_KERNEL"],"Sleep",array($ms));
}

function win32_flush()
{
	//get a handle to my stdout .. which is actually a handle to an
	//anonymous PIPE given to me by my nice parent process .. so he 
	//can hear what I say.  i'm sending him a message in a bottle!
	//ok..seriously..
	//we need to flush the pipe so he only gets a single msg at a time
	$stdout_handle=call_win32($GLOBALS["WINLIB_KERNEL"],"GetStdHandle",array(STD_OUTPUT_HANDLE));
	
	//meh..we tried
	if ($stdout_handle == INVALID_HANDLE_VALUE or $stdout_handle == NULL)
		return false;
	
	//yay, we have a handle!
	//now lets try to flush it
	$NTSUCCESS1=call_win32($GLOBALS["WINLIB_KERNEL"],"FlushFileBuffers",array($stdout_handle));
	    
	//darn..
	if ($NTSUCCESS1 == 0)
		return false;
	
	//eureka!
	return true;
}

function getHandleToWindow($window)
{
	$wbobj=unpack(WBOBJ_UNPACK,wb_peek($window, WBOBJ_SIZE));
	$hwnd=$wbobj["hwnd"];
	
	return $hwnd;
}

function isOtherDlgErr($errcode)
{
	$errcodes=Array(CDERR_DIALOGFAILURE,CDERR_FINDRESFAILURE,CDERR_INITIALIZATION,CDERR_LOADRESFAILURE,CDERR_LOADSTRFAILURE,CDERR_LOCKRESFAILURE,CDERR_MEMALLOCFAILURE,CDERR_MEMLOCKFAILURE,CDERR_NOHINSTANCE,CDERR_NOHOOK,CDERR_NOTEMPLATE,CDERR_REGISTERMSGFAIL,CDERR_STRUCTSIZE,FNERR_SUBCLASSFAILURE);
	
	if (array_search($errcode,$errcodes) !== false)
		return true;
	return false;
}

function getMessageBoxType($type)
{
	if ($type == WBC_YESNO)
		return 0x00000004;
	else if ($type == WBC_YESNOCANCEL)
		return 0x00000003;
	else
		return 0x00000004;
}

function MessageBox($window,$text,$caption,$type)
{
	$hwnd=getHandleToWindow($window);
	$mbc_type=getMessageBoxType($type);
	$return_code=call_win32($GLOBALS["WINLIB_USER32"],"MessageBox",array($hwnd,wb_get_address($text),wb_get_address($caption),$mbc_type|MB_ICONEXCLAMATION));
	return $return_code;
}

/**
 * Auslesen des aktuellen Zwischensperichers
 *
 * @return string
 */
function getClipboard()
{
    Global $GLOBALS;

    $result = call_win32($GLOBALS["WINLIB_USER32"], "OpenClipboard", array(0));
    $result = call_win32($GLOBALS["WINLIB_USER32"], "GetClipboardData", array(1));
    $clipboardContent = unpack("a*", wb_peek($result));
    $result = call_win32($GLOBALS["WINLIB_USER32"], "CloseClipboard",array());
    return $clipboardContent[1];
}

/**
 * Allgemeines Windows Meldungsfenster mit Timeout
 *
 * @param $strText
 * @param int $nSecondsToWait
 * @param string $strTitle
 * @param int $nType            Button Types + Icon Types  + Other Types
 * Button Types     0x0         Show OK button.
 *                  0x1         Show OK and Cancel buttons.
 *                  0x2         Show Abort, Retry, and Ignore buttons.
 *                  0x3         Show Yes, No, and Cancel buttons.
 *                  0x4         Show Yes and No buttons.
 *                  0x5         Show Retry and Cancel buttons.
 *                  0x6         Show Cancel, Try Again, and Continue buttons.
 * Icon Types       0x10        Show "Stop Mark" icon.
 *                  0x20        Show "Question Mark" icon.
 *                  0x30        Show "Exclamation Mark" icon.
 *                  0x40        Show "Information Mark" icon.
 * Other Types      0x100       The second button is the default button.
 *                  0x200       The third button is the default button.
 *                  0x1000      The message box is a system modal message box and appears in a topmost window.
 *                  0x80000     The text is right-justified.
 *                  0x100000    The message and caption text display in right-to-left reading order, which is useful for some languages.
 * @return int      -1          Timeout
 *                  1           OK button
 *                  2           Cancel button
 *                  3           Abort button
 *                  4           Retry button
 *                  5           Ignore button
 *                  6           Yes button
 *                  7           No button
 *                  10          Try Again button
 *                  11          Continue button
 */
function messagePopup($strText,$nSecondsToWait=0,$strTitle="Meldung",$nType=0)
{
    $wsh = new COM("WScript.Shell");
    return $wsh->Popup($strText, $nSecondsToWait, $strTitle, $nType);
}

?>