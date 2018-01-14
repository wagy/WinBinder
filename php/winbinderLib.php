<?php
/**
 * Created by JetBrains PhpStorm.
 * User: walter
 * Date: 10.03.13
 * Time: 09:57
 * To change this template use File | Settings | File Templates.
 */

/** --- class of the controls --- */
/**
 * A Frame is a static group box, image, etched rectangle, horizontal or vertical line
 * used for visual grouping or separation of controls. A Frame can also display an Image
 **/
define('Frame', Frame);

/** A ComboBox is a collapsible list of selectable items. */
define('ComboBox', ComboBox);

/** An edit box is an editable field that allows the user to enter plain text using the keyboard. */
define('EditBox', EditBox);

/**
 * A PushButton is a regular button that may be clicked and returns to its original position when released.
 * A PushButton may have an image or not, but it always has a standard button face.
 */
define('PushButton', PushButton);

/** A Label is a static control that displays text. */
define('Label', Label);

/** A CheckBox is a two-state button that the user can click to set or unset a particular option. */
define('CheckBox', CheckBox);

/** An RTFEditBox is a special type of text field than can display and edit formatted text. */
define('RTFEditBox', RTFEditBox);

/**
 * An HTMLControl object essentially embeds an Interned Explorer browser in a window.
 * This control can display and operate an URL, and HTML page,
 * an FTP location and everything else that a browser can do.
 */
define('HTMLControl', HTMLControl);

/**
 * A ListView is a control that displays data arranged as a rectangular grid of rows and columns.
 * Each row is a selectable item. Each element in the row is called a cell.
 */
define('ListView', ListView);

/**
 * A radio button is a control used for making a choice.
 * It is typically used in groups so the user may select one from several options.
 */
define('RadioButton', RadioButton);

/**
 * This class creates keyboard accelerators.
 * Keyboard accelerators, or just accelerators, are key combinations that can generate commands to be processed by a
 * callback function. The function wb_create_control() will create and apply an accelerator table to the application.
 */
define('Accel', Accel);

/** A gauge is a control used to visually indicate the progress of an action. */
define('Gauge', Gauge);

/**
 * Tree views are controls that display hierarchical information as a tree of nodes (or treeview items)
 * that can be expanded and collapsed.
 */
define('TreeView', TreeView);

/**
 * The status bar is an optional horizontal bar that appears on the bottom of a parent window.
 * To create a status bar, use the function wb_create_control() passing StatusBar as the parameter cclass.
 */
define('StatusBar', StatusBar);

/**
 * A TabControl is a special kind of container window that organizes several controls in multiple pages.
 * When clicked, the page displays only the controls assigned to it and hides the rest.
 */
define('TabControl', TabControl);

/** --- Windows standard identifiers --- */
define('IDABORT', IDABORT);
define('IDCANCEL', IDCANCEL);
define('IDCLOSE', IDCLOSE);
define('IDDEFAULT', IDDEFAULT);
define('IDHELP', IDHELP);
define('IDIGNORE', IDIGNORE);
define('IDNO', IDNO);
define('IDOK', IDOK);
define('IDRETRY', IDRETRY);
define('IDYES', IDYES);




/** --- Window classes --- */
/** A fixed-size application window. */
define('AppWindow', AppWindow);
/** A normal application window with a resizable border. */
define('ResizableWindow', ResizableWindow);
/** A modal dialog box (requires to be closed before continuing to other tasks). */
define('ModalDialog', ModalDialog);
/**  A modeless dialog box (other tasks can be performed while it is open). */
define('ModelessDialog', ModelessDialog);
/**  A fixed-size application window with no border and no title bar. */
define('NakedWindow', NakedWindow);
/**  An fixed-size application window that cannot be minimized. */
define('PopupWindow', PopupWindow);
/**  A modeless dialog box with a small caption. */
define('ToolDialog', ToolDialog);

/** --- Window style definition --- */
define('WBC_READONLY', WBC_READONLY);
define('WBC_RESIZE', WBC_RESIZE);
define('WBC_INVISIBLE', WBC_INVISIBLE);
define('WBC_NOTIFY', WBC_NOTIFY);
define('WBC_MAXIMIZED', WBC_MAXIMIZED);
define('WBC_DBLCLICK', WBC_DBLCLICK);
define('WBC_HEADERSEL', WBC_HEADERSEL);
define('WBC_GETFOCUS', WBC_GETFOCUS);
define('WBC_VISIBLE', WBC_VISIBLE);
define('WBC_ENABLED', WBC_ENABLED);
define('WBC_LINES', WBC_LINES);
define('WBC_CHECKBOXES', WBC_CHECKBOXES);
define('WBC_SORT', WBC_SORT);
define('WBC_CENTER', WBC_CENTER);
define('WBC_RIGHT', WBC_RIGHT);
define('WBC_MASKED', WBC_MASKED);
define('WBC_GROUP', WBC_GROUP);
define('WBC_REDRAW', WBC_REDRAW);
define('WBC_MOUSEDOWN', WBC_MOUSEDOWN);
define('WBC_NUMBER', WBC_NUMBER);
define('WBC_MULTILINE', WBC_MULTILINE);
define('WBC_KEYDOWN', WBC_MULTILINE);
define('WBC_NOHEADER', WBC_NOHEADER);

/** --- Font attributes --- */
define('FTA_BOLD',FTA_BOLD);
define('FTA_ITALIC',FTA_ITALIC);
define('FTA_NORMAL',FTA_NORMAL);
define('FTA_REGULAR',FTA_REGULAR);
define('FTA_STRIKEOUT',FTA_STRIKEOUT);
define('FTA_UNDERLINE',FTA_UNDERLINE);

/** ---  RGB standard colors --- */
define('BLACK',BLACK);
define('BLUE', BLUE);
define('CYAN',CYAN);
define('DARKBLUE',DARKBLUE);
define('DARKCYAN',DARKCYAN);
define('DARKGRAY',DARKGRAY);
define('DARKGREEN',DARKGREEN);
define('DARKMAGENTA',DARKMAGENTA);
define('DARKRED',DARKRED);
define('DARKYELLOW',DARKYELLOW);
define('GREEN',GREEN);
define('LIGHTGRAY',LIGHTGRAY);
define('MAGENTA',MAGENTA);
define('RED',RED);
define('WHITE',WHITE);
define('YELLOW',YELLOW);
define('NOCOLOR',NOCOLOR);

//BGR standard colors
define('bgrBLACK',bgrBLACK);
define('bgrBLUE',bgrBLUE);
define('bgrCYAN',bgrCYAN);
define('bgrDARKBLUE',bgrDARKBLUE);
define('bgrDARKCYAN',bgrDARKCYAN);
define('bgrDARKGRAY',bgrDARKGRAY);
define('bgrDARKGREEN',bgrDARKGREEN);
define('bgrDARKMAGENTA',bgrDARKMAGENTA);
define('bgrDARKRED',bgrDARKRED);
define('bgrDARKYELLOW',bgrDARKYELLOW);
define('bgrGREEN',bgrGREEN);
define('bgrLIGHTGRAY',bgrLIGHTGRAY);
define('bgrMAGENTA',bgrMAGENTA);
define('bgrRED',bgrRED);
define('bgrWHITE',bgrWHITE);
define('bgrYELLOW',bgrYELLOW);
define('bgrNOCOLOR',bgrNOCOLOR);

// Listview custom color state
/** Do not draw custom color */
define('WBC_LV_NONE', WBC_LV_NONE);
/** Draw custom foregrund color */
define('WBC_LV_FORE', WBC_LV_FORE);
/** Draw custom backgrund color */
define('WBC_LV_BACK', WBC_LV_BACK);
/** Use default colors */
define('WBC_LV_DEFAULT', WBC_LV_DEFAULT);
/** Use custum colors */
define('WBC_LV_DRAW', WBC_LV_DRAW);
/** Draw color for each column */
define('WBC_LV_COLUMNS', WBC_LV_COLUMNS);

/**
 * Displays the standard Select Path dialog box. Returns the name of the selected path,
 * if any, or a blank string if the dialog box was canceled. Returns NULL if not successful.
 *
 * @param integer   $parent             is a handle to the WinBinder object that will serve as the parent for the dialog box.
 * @param string    $title [optional]   is the string to be displayed as the dialog box caption.
 * @param string    $path [optional]    is an folder used to initialize the dialog box.
 * @return string
 */
function wb_sys_dlg_path($parent, $title, $path) { }

define('WBC_OK', WBC_OK);                  // An OK button.
define('WBC_INFO', WBC_INFO);              // An information icon and an OK button.
define('WBC_WARNING', WBC_WARNING);        // An exclamation point icon and an OK button.
define('WBC_STOP', WBC_STOP);              // A stop icon and an OK button.
define('WBC_QUESTION', WBC_QUESTION);      // A question mark icon and an OK button.
define('WBC_OKCANCEL', WBC_OKCANCEL);      // A question mark icon, an OK button and a Cancel button.
define('WBC_YESNO', WBC_YESNO);            // A question mark icon, a Yes button and a No button.
define('WBC_YESNOCANCEL', WBC_YESNOCANCEL);//  A question mark icon, a Yes button, a No button and a Cancel button.

/**
 * Creates and displays a message box under the style supplied and returns a value according to the button pressed.
 *
 * @param integer   $parent             is a handle to the WinBinder object that will serve as the parent for the dialog box.
 * @param string    $message            message text
 * @param string    $title [optional]   is the string to be displayed as the dialog box caption.
 * @param integer   $style [optional]
 * @return integer
 */
function wb_message_box ($parent, $message, $title, $style) { }

/**
 * @param integer   $control            Integer handle that corresponds to the WinBinder object
 * @param bool      $enabled            Set enabled of control
 * @return bool
 */
function wb_set_enabled($control, $enabled) { }

/**
 * @param integer   $control            Integer handle that corresponds to the WinBinder object
 * @return bool                         Returns TRUE if wbobject is enabled or FALSE otherwise.
 */
function wb_get_enabled($control) { }

/**
 * This function is typically used to retrieve the handle of a child control in a dialog box or in a menu item.
 *
 * @param integer   $wbobject           WinBinder object
 * @param integer   $id
 * @return          Returns an integer handle that corresponds to the WinBinder object (control, toolbar item or menu item)
 *                  wbobject that has the supplied identifier id.
 */
function wb_get_control ($wbobject, $id) { }

/**
 * Assigns the keyboard focus to wbobject. Returns TRUE on success or FALSE if an error occurs.
 *
 * @param $wbobject                     WinBinder object
 * @return bool
 */
function wb_set_focus($wbobject) { }

/**
 * Returns a handle to the window or control that currently has the keyboard focus,
 * or NULL if there is no control with the keyboard focus.
 *
 * @return bool
 */
function wb_get_focus() { }

/**
 * Returns a value or array with the indices or identifiers of the selected elements or items in wbobject.
 *
 * Class            What it retrieves
 * ComboBox         The index of the currently selected item.
 * ListBox          The index of the currently selected item.
 * ListView         An array with the indices of the selected items. ¹
 * TabControl       The index of the selected tab page.
 * TreeView         The handle of the currently selected node.
 * Window           0 (zero).
 * Other controls   0 (zero).
 *
 * @param $wbobject                     WinBinder object
 * @return mixed
 */
function wb_get_selected($wbobject) { }

/**
 * Returns a value or array with the indices or identifiers of the selected elements or items in wbobject.
 *
 * Class            What it retrieves
 * ComboBox         Selects the specified item.
 * ListBox          Selects the specified item.
 * ListView         Adds or removes the specified items from the current selection. ¹
 * Menu             Selects the specified menu item and unselects all other items in the menu. ¹
 * TabControl       Changes the current tab to the specified one.
 * TreeView         Changes the selected node to the specified node.
 *                  If items is zero, the root will be selected. Set items to -1 to deselect all nodes.
 *
 * @param integer   $control            Integer handle that corresponds to the WinBinder object
 * @param mixed     $items
 * @param bool      $selected [optional]
 * @return bool
 */
function wb_set_selected($control,$items, $selected) { }


/**
 * Creates a timer in the specified window. The timer must be given an integer id that must be unique
 * to all timers and controls. interval specifies the time-out value in milliseconds.
 * Timer events are passed to and processed by the window callback function
 *
 * @param null|integer  $window
 * @param integer       $id
 * @param integer       $interval
 */
function wb_create_timer($window, $id, $interval) { }

/**
 * Destroys a timer created with wb_create_timer().
 * The window and the id parameters must be the same that were passed to wb_create_timer() when the timer was created.
 *
 * @param null|integer  $window
 * @param integer       $id
 */
function wb_destroy_timer($window, $id) { }

/**
 * Refreshes or redraws the WinBinder object wbobject, forcing an immediate redraw if the parameter now is TRUE (the default).
 * If now is FALSE, the redraw command is posted to the Windows message queue.
 *
 * @param integer   $wbobject               WinBinder object
 * @param bool       $now
 * @param integer   $xpos   [optional]
 * @param integer   $ypos   [optional]
 * @param integer   $width  [optional]
 * @param integer   $height [optional]
 */
function wb_refresh($wbobject, $now, $xpos, $ypos, $width, $height) { }

/**
 * Retrieves the value of a control or control item. The item and subitem parameters are set to -1 if absent.
 *
 * @param integer   $wbobject               WinBinder object
 * @param integer   $item   [optional]
 * @param integer   $subitem[optional]
 * @return mixed
 */
function wb_get_value($wbobject, $item, $subitem) { }

/**
 * @param integer   $wbobject               WinBinder object
 * @return integer                          Returns the integer identifier of the wbobject control.
 */
function wb_get_id($wbobject) { }

/**
 * Returns an integer that corresponds to the class of the object (control, window or menu) passed as the parameter.
 * The class is passed as a parameter to functions wb_create_control() and wb_create_window().
 *
 * @param integer   $wbobject               WinBinder object
 * @return string
 */
function wb_get_class($wbobject) { }

/**
 * Destroys a control created by wb_create_control().
 *
 * @param integer $control                  Integer handle that corresponds to the WinBinder object
 * @return bool                             Returns TRUE on success or FALSE if an error occurs.
 */
function wb_destroy_control($control) { }


/**
 * Assigns the callback function fn_handler to window.
 * The handler function may be a regular PHP function or class method that is used to process events for this particular window.
 * wb_set_handler() must be called whenever the window needs to process messages and events from its controls.
 *
 * @param null|integer  $window
 * @param callable  $fn_handler
 * @return integer
 */
function wb_set_handler($window, $fn_handler) { }

/**
 * Sends a Windows message to the HWND handle of the WinBinder object wbobject.
 * The parameters wparam and lparam, as well as the return value, depend on message.
 * See SendMessage() in the Windows API documentation for more information.
 *
 * The following constant may be used as the wbobject parameter: 0xFFFF
 * This constant is the value of HWND_BROADCAST in the Windows API. For more information consult the Windows API documentation.
 *
 * @param integer   $wbobject               WinBinder object
 * @param integer   $message
 * @param integer   $wparam
 * @param integer   $lparam
 * @return integer
 */
function wb_send_message($wbobject, $message, $wparam, $lparam) { }

/**
 * Deletes an item, a range of items, or all items from a control. Returns TRUE on success or FALSE if an error occurs.
 *
 * items                What it does
 * integer              Deletes the specified item.
 * array of integers    Deletes the specified items.
 * zero                 Deletes item zero.
 * null                 Deletes all items.
 *
 * @param integer   $control                  Integer handle that corresponds to the WinBinder object
 * @param integer   $items  [optional]
 * @return bool
 */
function wb_delete_items($control, $items) { }

/**
 * Moves the object wbobject to the coordinates xpos, ypos in relation to its parent window.
 * If both xpos and ypos have the value WBC_CENTER or are not given, the window is centered on its parent window.
 *
 * @param integer   $wbobject               WinBinder object
 * @param integer   $xpos   [optonal]
 * @param integer   $ypos   [optonal]
 * @return bool                             Returns TRUE on success or FALSE if an error occurs.
 */
function wb_set_position($wbobject, $xpos, $ypos) { }

/**
 * Returns an array with the position of the control or window related to its parent, in pixels.
 * The first element is the horizontal position and the second is the vertical position.
 * If clientarea is TRUE, the area returned will not include the title bar and borders. The default is FALSE.
 *
 * @param integer   $wbobject               WinBinder object
 * @param bool      $clientarea
 * @return array
 */
function wb_get_position($wbobject, $clientarea) { }

/**
 * Sizes the object wbobject to width and height pixels. Parameters width and height may be used as follows:
 * width                height              Applies to          What it does
 * Positive integer     Positive integer    window or control   Sets the window or control size to width and height pixels.
 * WBC_NORMAL           (not supplied)      window              Restores the window, if it is not already.
 * WBC_MINIMIZED        (not supplied)      window              Minimizes the window, if it is not already.
 * WBC_MAXIMIZED        (not supplied)      window              Maximizes the window, if it is not already.
 * Array of integers    (not supplied)      ListView            Changes the column widths of the control.
 *
 * @param integer       $wbobject               WinBinder object
 * @param integer|array $width
 * @param integer       $height [optional]
 * @return bool
 */
function wb_set_size($wbobject, $width, $height) { }

/**
 * Gets the dimensions of a control, window, image or string. The image handle must have been obtained with wb_create_image(), wb_create_mask() or wb_load_image().
 * This function generally returns an array where the first element is the width and the second is the height.
 * Measurements are in pixels. If param is TRUE, the area returned will not include the title bar and borders.
 * Default is FALSE. The function will return the integer WBC_MINIMIZED instead of an array if the requested window is minimized, or NULL on error.
 * If object is a ListView handle and param is TRUE, the function returns an array with the widths of the column headers.
 * If param is omitted or FALSE, the function behaves normally like described above
 *
 * If object is a text string, param is optionally used to pass the handle to a font created with wb_create_font().
 * If param is null or not used, the default font is used.
 *
 * @param mixed     $object
 * @param bool      $param
 * @return array
 */
function wb_get_size($object, $param) { }

/**
 * Shows or hides the WinBinder object wbobject according to the value of visible.
 *
 * @param integer       $wbobject               WinBinder object
 * @param bool          $visible
 * @return bool                                 Returns TRUE on success or FALSE if an error occurs.
 */
function wb_set_visible($wbobject, $visible) { }

/**
 * Sets the location of an HTMLControl or sends a special command to it. Returns TRUE on success or FALSE
 * if an error occurs (except when using "cmd:busy" as explained below).
 *
 * Type                 Examples
 * URL                  http://winbinder.org/index.php
 *                      http://127.0.0.1/
 *                      http://pecl.php.net/packages.php
 *                      ftp://plaza.aarnet.edu.au/
 * File name            C:\WinBinder\Website\manual\index.html
 *                      E:\WinBinder\package.xml
 *                      E:\WinBinder\Readme.txt
 *
 * The function also recognizes some special commands as the location parameter.
 * All special commands must be preceded by the cmd: prefix.
 *
 * Value of location    Meaning
 * "cmd:back"           Go to previously visited page.
 * "cmd:forward"        Go to a page previously viewed before issuing the back command.
 * "cmd:refresh"        Redraw the current page.
 * "cmd:stop"           Stop the current action, like loading a page.
 * "cmd:busy"           Return TRUE if the browser is busy or FALSE if idle.
 * "cmd:blank"          Clear the page.
 *
 * @param integer       $wbobject               WinBinder object
 * @param string        $location
 * @return bool
 */
function wb_set_location($wbobject, $location) { }

/**
 * Creates a window of class wclass. Click here for a list of the available window classes.
 * Windows created with this function must be destroyed with a call to wb_destroy_window().
 * Optional style flags may be passed through parameter style.
 * To enable additional messages in a particular window, include the WBC_NOTIFY style in the style parameter and use param to indicate which additional notification messages you want to process.
 *
 * This function may set the text and/or the tooltip (small hint window) of the window when it is created.
 * To create a tooltip, text must be an array with two elements.
 * The first one is the new caption (or NULL if one is not required) and the second one is the new tooltip (or NULL if one is not required). All classes support tooltips.
 *
 * @param integer   $parent
 * @param integer   $wclass
 * @param string    $caption    [optional]
 * @param integer   $xpos       [optional]
 * @param integer   $ypos       [optional]
 * @param integer   $width      [optional]
 * @param integer   $height     [optional]
 * @param integer   $style      [optional]
 * @param integer   $param      [optional]
 * @return integer                          Returns the handle of the newly created window or NULL or zero if an error occurs.
 */
function wb_create_window($parent, $wclass, $caption, $xpos, $ypos, $width, $height, $style, $param) { }

/**
 * Destroys a window created by wb_create_window(). Returns TRUE on success or FALSE if an error occurs.
 *
 * @param integer   $window
 * @return bool
 */
function wb_destroy_window($window) { }

/**
 * This function creates a delay and verifies if mouse buttons are pressed and/or the keyboard state.
 * This function is useful for lengthy operations.
 * In this case, wb_wait guarantees that the message control is sent back to the main loop, avoiding an unpleasant "freezing" effect.
 * Using this function also provides an way to easily exit lengthy operations by constantly monitoring the keyboard and mouse.
 *
 * All parameters are optional. If window is null or not provided, the function will monitor the application's foreground window.
 * An optional pause value, in milliseconds, can be passed to the function to provide accurate time delays.
 * The default is 0 ms (no pause). The following constants are accepted in the flags argument to specify the events that will be monitored:
 * WBC_MOUSEDOWN
 * WBC_MOUSEUP
 * WBC_KEYDOWN
 * WBC_KEYUP
 *
 * The default is WBC_KEYDOWN. Note that if you set flags to zero, the function will not monitor any keyboard or mouse button events.
 *
 * The following table shows the possible return values for wb_wait.
 * Condition                Return value
 * Key pressed              Key code
 * Key released             Key code
 * Mouse button pressed     A double word (LONG) with the mouse button code¹ in the low-order word and WBC_MOUSEDOWN in the high-order word
 * Mouse button released    A double word (LONG) with the mouse button code¹ in the low-order word and WBC_MOUSEUP in the high-order word
 *
 * ¹ The mouse button code is: 1 for left button, 2 for middle button, 3 for right button.
 *
 * If you monitor the return value, you may provide action based on the specific key or mouse button that generated the event.
 *
 * @param integer   $window     [optional]
 * @param integer   $pause      [optional]
 * @param integer   $flags      [optional]
 * @return integer
 */
function wb_wait($window, $pause, $flags) { }

/**
 *Loads a DLL into memory. Returns an integer identifying libname.
 * If libname is NULL then returns the identifier of the last library returned.
 * The function accepts fully qualified and raw names. Returns NULL if no library was found.
 *
 * @param string    $libname
 * @return int
 */
function wb_load_library ($libname) { }

/**
 * Returns the address of a library function. fname is the function name and idlib identifies a library already loaded.
 * The idlib identifier must have been obtained with a call to wb_load_library().
 * If idlib is not set or is set to NULL, the last library sent to the function will be used.
 *
 * @param string    $fname
 * @param int       $idlib [optional]
 * @return int
 */
function wb_get_function_address($fname , $idlib) { }

/**
 *Calls the DLL function pointed by address. args is an optional array of parameters that must match those of the function being called.
 * Returns an integer that may be a valid value or a pointer to one object, according to the library function called.
 *
 * @param int   $address
 * @param array $args   [optional]
 * @return int
 */
function wb_call_function($address, $args) { }

/**
 * Returns the address (as an integer pointer) of the variable var. var can be a string, integer, boolean, or double.
 * This function is specially suited to use with wb_peek() and wb_poke().
 *
 * @param mixed     $var
 * @return int
 */
function wb_get_address(&$var) { }

/**
 * Gets the contents of a memory area pointed by address.
 * If length is empty or zero, returns bytes up to the first NUL character (zero-character) or up to 32767 bytes,
 * whichever comes first. If length is greater than zero, returns length bytes.
 *
 * @param int   $address
 * @param int   $length [optional]
 * @return string
 */
function wb_peek($address, $length=null) { }

/**
 * @param int       $address
 * @param string    $contents
 * @param int       $length [optional]
 */
function wb_poke($address, $contents, $length=null) { }

/**
 * Sets the font of control. font is a unique integer value returned by wb_create_font().
 * If font is zero or not given, the most recently created font is used.
 * If font is a negative number, it means the system default font.
 *
 * Returns TRUE on success or FALSE if an error occurs.
 *
 * @param integer $wbobject WinBinder object
 * @param integer $font [optional]
 * @param bool $redraw [optional]
 * @return bool
 */
function wb_set_font($wbobject, $font=null, $redraw=false) { }

/**
 * Creates a new font. name is the font name, height is its height in points (not pixels),
 * and color is a RGB color value. flags can be a combination of the following values:
 *
 * FTA_NORMAL
 * FTA_REGULAR
 * FTA_BOLD
 * FTA_ITALIC
 * FTA_UNDERLINE
 * FTA_STRIKEOUT
 * Constants of FTA_NORMAL and FTA_REGULAR mean the same thing and are defined as zero.
 *
 * The function returns an integer value that is the font identifier.
 *
 * $font = wb_create_font("Arial", 14, BLACK, FTA_BOLD);
 *
 * @param string $name
 * @param integer $height
 * @param integer $color [optional]
 * @param integer $flags [optional]
 * @return integer
 */
function wb_create_font($name, $height, $color=null, $flags=null) { }

/**
 * @param string $info
 *
 *  "appmemory"         integer     The total memory used by the application¹
 *  "backgroundcolor"   integer     The main face color for Windows dialog boxes and controls
 *  "colordepth"        integer     The current color depth in bits per pixel
 *  "commandline"       string      The original Windows command line including the executable file
 *  "computername"      string      The name of the computer inside the network
 *  "consolemode"       integer     1 indicates that console mode (DOS box) is active, 0 otherwise
 *  "diskdrives"        string      The list of all available disk drives
 *  "exepath"           string      The path to the main executable (PHP.EXE)
 *  "fontpath"          string      The current font path
 *  "freememory"        integer     The available physical memory
 *  "gdiobjects"        integer     The number of currently allocated GDI handles
 *  "instance"          integer     The instance identifier of the current application
 *  "osnumber"          string      The numeric OS version number (5.1 etc)
 *  "ospath"            string      The current OS path
 *  "osversion"         string      The complete OS version name
 *  "pgmpath"           string      The default OS application path
 *  "screenarea"        string      The total area (x, y, width, height) of the screen, in pixels (0 0 1280 960)
 *  "systemfont"        string      The common (default) system font for dialog boxes
 *  "systempath"        string      The OS system path
 *  "temppath"          string      The path used by the OS to hold temporary files
 *  "totalmemory"       string      The total physical memory installed
 *  "username"          string      The name of the currently logged user
 *  "userobjects"       integer     The number of currently allocated USER handles
 *  "workarea"          string      The valid area (x, y, width, height) of the screen, in pixels
 * @return string
 */
function wb_get_system_info($info) {}

/**
 * Assigns the image source to the WinBinder object wbobject.
 * Parameter source can be either an image, icon or cursor handle or a path to an image file name.
 * If a handle, it must have been obtained with wb_create_image(), wb_create_mask() or wb_load_image().
 * The optional parameter transparentcolor tells the function which color is to be considered transparent.
 * The default is NOCOLOR (no transparency). index is used to select a specific image from a multi-image file (such as a DLL or executable).
 *
 * If source is an icon or a cursor, if param is 0 (the default), the function sets a large icon or cursor;
 * if param is 1, it sets a small icon or cursor; if param is -1, the function sets the default icon or cursor.
 * For minimized windows, this function will also change the icon that is displayed on the task bar.
 *
 *
 * @param int $wbobject
 * @param mixed $source
 * @param int $transparentcolor [optional]
 * @param int $index [optional]
 * @param int $param [optional]
 * @return boolean Returns TRUE on success or FALSE if an error occurs.
 */
function wb_set_image($wbobject, $source, $transparentcolor, $index, $param) { }

/**
 * Sets the valid range of values (vmin and vmax) of a control.
 * Valid classes are Gauge, ScrollBar, Slider and Spinner.
 *
 * @param int $control
 * @param int $vmin
 * @param int $vmax
 * @return boolean Returns TRUE on success or FALSE if an error occurs.
 */
function wb_set_range($control, $vmin, $vmax) {  }

/**
 * Enters the Windows main loop.
 * This function must be called if the application has a window.
 * The call to wb_main_loop() must be the last executable statement of the PHP script: All statements after it will be ignored.
 *
 * @return int The return value is used for debugging purposes only and may be ignored.
 */
function wb_main_loop() { }