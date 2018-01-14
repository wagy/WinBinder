/*******************************************************************************

 WINBINDER - The native Windows binding for PHP for PHP

 Copyright � Hypervisual - see LICENSE.TXT for details
 Author: Rubem Pechansky (http://winbinder.org/contact.php)

 ZEND exports module

*******************************************************************************/

//----------------------------------------------------------------- DEPENDENCIES

#include "phpwb.h"

#include "ext/standard/info.h"			// For ZEND_MINFO_FUNCTION

//-------------------------------------------------------------------- CONSTANTS

//----------------------------------------------------------------------- MACROS

#define WB_ZEND_CONST(type, str, val) REGISTER_##type##_CONSTANT((str), (val), CONST_CS | CONST_PERSISTENT);

//----------------------------------------------- PROTOTYPES FOR THE ZEND ENGINE

ZEND_MINIT_FUNCTION(winbinder);
ZEND_MSHUTDOWN_FUNCTION(winbinder);
ZEND_MINFO_FUNCTION(winbinder);

//----------------------------------------------------------- EXPORTED FUNCTIONS

/*
  NOTE:

  Functions starting with "wbtemp_" must not be used by applications. These are for
  internal use only and will disappear in the future. The "wb_" functions are
  intended for public usage.

*/

// PHPWB_WINSYS.C

ZEND_BEGIN_ARG_INFO_EX(arginfo_get_address, 0, 0, 1)
ZEND_ARG_INFO(1, source)
ZEND_END_ARG_INFO()

ZEND_FUNCTION(wb_main_loop);
ZEND_FUNCTION(wb_find_file);
ZEND_FUNCTION(wb_message_box);
ZEND_FUNCTION(wb_play_sound);
ZEND_FUNCTION(wb_stop_sound);
ZEND_FUNCTION(wb_exec);
ZEND_FUNCTION(wb_get_system_info);
ZEND_FUNCTION(wb_get_registry_key);
ZEND_FUNCTION(wb_set_registry_key);
ZEND_FUNCTION(wb_create_timer);
ZEND_FUNCTION(wb_destroy_timer);
ZEND_FUNCTION(wb_wait);
ZEND_FUNCTION(wbtemp_set_accel_table);

// PHPWB_CONTROL.C

ZEND_FUNCTION(wb_delete_items);
ZEND_FUNCTION(wb_destroy_control);
ZEND_FUNCTION(wb_get_class);
ZEND_FUNCTION(wb_get_control);
ZEND_FUNCTION(wb_get_enabled);
ZEND_FUNCTION(wb_get_focus);
ZEND_FUNCTION(wb_get_id);
ZEND_FUNCTION(wb_get_item_count);
ZEND_FUNCTION(wb_get_parent);
ZEND_FUNCTION(wb_get_selected);
ZEND_FUNCTION(wb_get_state);
ZEND_FUNCTION(wb_get_value);
ZEND_FUNCTION(wb_get_visible);
ZEND_FUNCTION(wb_refresh);
ZEND_FUNCTION(wb_set_cursor);
ZEND_FUNCTION(wb_set_enabled);
ZEND_FUNCTION(wb_set_focus);
ZEND_FUNCTION(wb_set_handler);
ZEND_FUNCTION(wb_set_image);
ZEND_FUNCTION(wb_set_item_image);
ZEND_FUNCTION(wb_set_location);
ZEND_FUNCTION(wb_set_range);
ZEND_FUNCTION(wb_set_state);
ZEND_FUNCTION(wb_set_style);
ZEND_FUNCTION(wb_set_visible);
ZEND_FUNCTION(wb_sort);
ZEND_FUNCTION(wbtemp_create_control);
ZEND_FUNCTION(wbtemp_create_item);
ZEND_FUNCTION(wbtemp_create_statusbar_items);
ZEND_FUNCTION(wbtemp_get_text);
ZEND_FUNCTION(wbtemp_select_tab);
ZEND_FUNCTION(wbtemp_set_text);
ZEND_FUNCTION(wbtemp_set_value);

// PHPWB_DRAW.C

ZEND_FUNCTION(wb_get_pixel);
ZEND_FUNCTION(wb_draw_point);
ZEND_FUNCTION(wb_draw_line);
ZEND_FUNCTION(wb_draw_rect);
ZEND_FUNCTION(wb_draw_ellipse);
ZEND_FUNCTION(wb_draw_text);
ZEND_FUNCTION(wb_draw_image);

// PHPWB_BITMAP.C

ZEND_FUNCTION(wb_load_image);
ZEND_FUNCTION(wb_save_image);
ZEND_FUNCTION(wb_create_image);
ZEND_FUNCTION(wb_create_mask);
ZEND_FUNCTION(wb_destroy_image);
ZEND_FUNCTION(wb_get_image_data);

// PHPWB_CONTROL_LISTVIEW.C

ZEND_FUNCTION(wbtemp_create_listview_item);
ZEND_FUNCTION(wbtemp_set_listview_item_checked);
ZEND_FUNCTION(wbtemp_get_listview_item_checked);
ZEND_FUNCTION(wbtemp_set_listview_item_text);
ZEND_FUNCTION(wbtemp_get_listview_text);
ZEND_FUNCTION(wbtemp_get_listview_columns);
ZEND_FUNCTION(wbtemp_clear_listview_columns);
ZEND_FUNCTION(wbtemp_create_listview_column);
ZEND_FUNCTION(wbtemp_select_listview_item);
ZEND_FUNCTION(wbtemp_select_all_listview_items);
//ZEND_FUNCTION(wbtemp_get_listview_column_widths);
//ZEND_FUNCTION(wbtemp_set_listview_column_widths);

// PHPWB_CONTROL_MENU.C

ZEND_FUNCTION(wbtemp_create_menu);
ZEND_FUNCTION(wbtemp_get_menu_item_checked);
ZEND_FUNCTION(wbtemp_set_menu_item_checked);
ZEND_FUNCTION(wbtemp_set_menu_item_selected);
ZEND_FUNCTION(wbtemp_set_menu_item_image);

// PHPWB_CONTROL_TOOLBAR.C

ZEND_FUNCTION(wbtemp_create_toolbar);

// PHPWB_CONTROL_TREEVIEW.C

ZEND_FUNCTION(wbtemp_create_treeview_item);
ZEND_FUNCTION(wbtemp_set_treeview_item_selected);
ZEND_FUNCTION(wbtemp_set_treeview_item_text);
ZEND_FUNCTION(wbtemp_set_treeview_item_value);
ZEND_FUNCTION(wbtemp_get_treeview_item_text);
ZEND_FUNCTION(wb_get_level);

// PHPWB_FONTS.C

ZEND_FUNCTION(wb_create_font);
ZEND_FUNCTION(wb_destroy_font);
ZEND_FUNCTION(wb_set_font);

// PHPWB_LOWLEVEL.C

ZEND_FUNCTION(wb_get_address);
ZEND_FUNCTION(wb_send_message);
ZEND_FUNCTION(wb_peek);
ZEND_FUNCTION(wb_poke);
ZEND_FUNCTION(wb_load_library);
ZEND_FUNCTION(wb_release_library);
ZEND_FUNCTION(wb_get_function_address);
ZEND_FUNCTION(wb_call_function);
ZEND_FUNCTION(wb_get_midi_callback);
ZEND_FUNCTION(wb_get_enum_callback);
ZEND_FUNCTION(wb_get_hook_callback);

// PHPWB_WINDOW.C

ZEND_FUNCTION(wb_get_size);
ZEND_FUNCTION(wb_set_size);
ZEND_FUNCTION(wb_set_position);
ZEND_FUNCTION(wb_get_position);
ZEND_FUNCTION(wb_destroy_window);
ZEND_FUNCTION(wb_create_window);
ZEND_FUNCTION(wb_get_item_list);
ZEND_FUNCTION(wb_get_instance);
ZEND_FUNCTION(wb_set_area);

// PHPWB_SYSDLG.C

ZEND_FUNCTION(wb_sys_dlg_path);
ZEND_FUNCTION(wb_sys_dlg_color);
ZEND_FUNCTION(wbtemp_sys_dlg_open);
ZEND_FUNCTION(wbtemp_sys_dlg_save);

//-------------------------------------------------------------------- VARIABLES

/* List of exported functions */

zend_function_entry winbinder_functions[] =
{
	// PHPWB_WINSYS.C

    ZEND_FE(wb_main_loop, NULL)
	ZEND_FE(wb_find_file, NULL)
	ZEND_FE(wb_message_box, NULL)
	ZEND_FE(wb_play_sound, NULL)
	ZEND_FE(wb_stop_sound, NULL)
	ZEND_FE(wb_exec, NULL)
    ZEND_FE(wb_get_system_info, NULL)
	ZEND_FE(wb_get_registry_key, NULL)
	ZEND_FE(wb_set_registry_key, NULL)
	ZEND_FE(wb_create_timer, NULL)
	ZEND_FE(wb_wait, NULL)
	ZEND_FE(wb_destroy_timer, NULL)
	ZEND_FE(wbtemp_set_accel_table, NULL)

	// PHPWB_BITMAP.C

	ZEND_FE(wb_load_image, NULL)
	ZEND_FE(wb_save_image, NULL)
	ZEND_FE(wb_create_image, NULL)
	ZEND_FE(wb_create_mask, NULL)
	ZEND_FE(wb_destroy_image, NULL)
	ZEND_FE(wb_get_image_data, NULL)

	// PHPWB_DRAW.C

	ZEND_FE(wb_get_pixel, NULL)
	ZEND_FE(wb_draw_point, NULL)
	ZEND_FE(wb_draw_line, NULL)
	ZEND_FE(wb_draw_rect, NULL)
	ZEND_FE(wb_draw_ellipse, NULL)
	ZEND_FE(wb_draw_text, NULL)
	ZEND_FE(wb_draw_image, NULL)

	// PHPWB_CONTROL.C

    ZEND_FE(wb_destroy_control, NULL)
    ZEND_FE(wb_get_value, NULL)
    ZEND_FE(wb_refresh, NULL)
    ZEND_FE(wb_set_enabled, NULL)
    ZEND_FE(wb_set_image, NULL)
    ZEND_FE(wb_set_item_image, NULL)
    ZEND_FE(wbtemp_create_control, NULL)
    ZEND_FE(wbtemp_create_item, NULL)
	ZEND_FE(wbtemp_create_statusbar_items, NULL)
    ZEND_FE(wbtemp_get_text, NULL)
    ZEND_FE(wbtemp_set_text, NULL)
	ZEND_FE(wb_delete_items, NULL)
	ZEND_FE(wb_get_class, NULL)
	ZEND_FE(wb_get_control, NULL)
	ZEND_FE(wb_get_enabled, NULL)
	ZEND_FE(wb_get_focus, NULL)
	ZEND_FE(wb_get_id, NULL)
	ZEND_FE(wb_get_item_count, NULL)
	ZEND_FE(wb_get_parent, NULL)
	ZEND_FE(wb_get_selected, NULL)
	ZEND_FE(wb_get_state, NULL)
	ZEND_FE(wb_get_visible, NULL)
	ZEND_FE(wb_set_cursor, NULL)
	ZEND_FE(wb_set_focus, NULL)
	ZEND_FE(wb_set_handler, NULL)
	ZEND_FE(wb_set_location, NULL)
	ZEND_FE(wb_set_range, NULL)
	ZEND_FE(wb_set_state, NULL)
	ZEND_FE(wb_set_style, NULL)
	ZEND_FE(wb_set_visible, NULL)
	ZEND_FE(wb_sort, NULL)
	ZEND_FE(wbtemp_select_tab, NULL)
	ZEND_FE(wbtemp_set_value, NULL)

	// PHPWB_CONTROL_LISTVIEW.C

	ZEND_FE(wbtemp_create_listview_item, NULL)
	ZEND_FE(wbtemp_set_listview_item_checked, NULL)
	ZEND_FE(wbtemp_get_listview_item_checked, NULL)
	ZEND_FE(wbtemp_set_listview_item_text, NULL)
	ZEND_FE(wbtemp_get_listview_text, NULL)
	ZEND_FE(wbtemp_get_listview_columns, NULL)
	ZEND_FE(wbtemp_create_listview_column, NULL)
	ZEND_FE(wbtemp_clear_listview_columns, NULL)
	ZEND_FE(wbtemp_select_listview_item, NULL)
	ZEND_FE(wbtemp_select_all_listview_items, NULL)
//	ZEND_FE(wbtemp_get_listview_column_widths, NULL)
//	ZEND_FE(wbtemp_set_listview_column_widths, NULL)

	// PHPWB_CONTROL_MENU.C

    ZEND_FE(wbtemp_create_menu, NULL)
	ZEND_FE(wbtemp_get_menu_item_checked, NULL)
	ZEND_FE(wbtemp_set_menu_item_checked, NULL)
	ZEND_FE(wbtemp_set_menu_item_selected, NULL)
	ZEND_FE(wbtemp_set_menu_item_image, NULL)

	// PHPWB_CONTROL_TOOLBAR.C

    ZEND_FE(wbtemp_create_toolbar, NULL)

	// PHPWB_CONTROL_TREEVIEW.C

	ZEND_FE(wbtemp_create_treeview_item, NULL)
	ZEND_FE(wbtemp_set_treeview_item_selected, NULL)
	ZEND_FE(wbtemp_set_treeview_item_text, NULL)
	ZEND_FE(wbtemp_set_treeview_item_value, NULL)
	ZEND_FE(wbtemp_get_treeview_item_text, NULL)
	ZEND_FE(wb_get_level, NULL)

	// PHPWB_FONTS.C

	ZEND_FE(wb_create_font, NULL)
	ZEND_FE(wb_destroy_font, NULL)
	ZEND_FE(wb_set_font, NULL)

	// PHPWB_LOWLEVEL.C

    ZEND_FE(wb_get_address, arginfo_get_address)
    ZEND_FE(wb_send_message, NULL)
    ZEND_FE(wb_peek, NULL)
    ZEND_FE(wb_poke, NULL)
	ZEND_FE(wb_load_library, NULL)
	ZEND_FE(wb_release_library, NULL)
	ZEND_FE(wb_get_function_address, NULL)
	ZEND_FE(wb_call_function, NULL)
	ZEND_FE(wb_get_midi_callback, NULL)
	ZEND_FE(wb_get_enum_callback, NULL)
	ZEND_FE(wb_get_hook_callback, NULL)
	// PHPWB_WINDOW.C

    ZEND_FE(wb_destroy_window, NULL)
    ZEND_FE(wb_get_size, NULL)
    ZEND_FE(wb_set_size, NULL)
    ZEND_FE(wb_set_position, NULL)
    ZEND_FE(wb_get_position, NULL)
	ZEND_FE(wb_create_window, NULL)
	ZEND_FE(wb_get_instance, NULL)
	ZEND_FE(wb_get_item_list, NULL)
	ZEND_FE(wb_set_area, NULL)

	// PHPWB_SYSDLG.C

	ZEND_FE(wb_sys_dlg_path, NULL)
	ZEND_FE(wb_sys_dlg_color, NULL)
	ZEND_FE(wbtemp_sys_dlg_open, NULL)
	ZEND_FE(wbtemp_sys_dlg_save, NULL)

	// The line below must be the last one

	{NULL, NULL, NULL}
};

/*
	Structure with module information
*/

zend_module_entry winbinder_module_entry = {
    STANDARD_MODULE_HEADER,
    MODULENAME,
    winbinder_functions,
	ZEND_MINIT(winbinder),
	ZEND_MSHUTDOWN(winbinder),
    NULL, NULL,
	ZEND_MINFO(winbinder),
    LIBVERSION,
    STANDARD_MODULE_PROPERTIES
};

//-------------------------------------------------------- ZEND MODULE FUNCTIONS

/* Module initialization function */

ZEND_MINIT_FUNCTION(winbinder)
{
	// Module initialization procedure

	wbInit();

	// Window classes

	WB_ZEND_CONST(LONG, "AppWindow", 		AppWindow)
	WB_ZEND_CONST(LONG, "ModalDialog",		ModalDialog)
	WB_ZEND_CONST(LONG, "ModelessDialog", 	ModelessDialog)
	WB_ZEND_CONST(LONG, "NakedWindow", 		NakedWindow)
	WB_ZEND_CONST(LONG, "PopupWindow", 		PopupWindow)
	WB_ZEND_CONST(LONG, "ResizableWindow",	ResizableWindow)
	WB_ZEND_CONST(LONG, "ToolDialog", 		ToolDialog)

	// WinBinder control classes

	WB_ZEND_CONST(LONG, "Accel",			Accel)
	WB_ZEND_CONST(LONG, "Calendar",			Calendar)
	WB_ZEND_CONST(LONG, "CheckBox", 		CheckBox)
	WB_ZEND_CONST(LONG, "ComboBox",			ComboBox)
	WB_ZEND_CONST(LONG, "EditBox", 			EditBox)
	WB_ZEND_CONST(LONG, "Frame",			Frame)
	WB_ZEND_CONST(LONG, "Gauge",			Gauge)
	WB_ZEND_CONST(LONG, "HTMLControl",		HTMLControl)
	WB_ZEND_CONST(LONG, "HyperLink",		HyperLink)
	WB_ZEND_CONST(LONG, "ImageButton",		ImageButton)
	WB_ZEND_CONST(LONG, "InvisibleArea",	InvisibleArea)
	WB_ZEND_CONST(LONG, "Label",			Label)
	WB_ZEND_CONST(LONG, "ListBox",			ListBox)
	WB_ZEND_CONST(LONG, "ListView",			ListView)
	WB_ZEND_CONST(LONG, "Menu",				Menu)
	WB_ZEND_CONST(LONG, "PushButton",		PushButton)
	WB_ZEND_CONST(LONG, "RTFEditBox",		RTFEditBox)
	WB_ZEND_CONST(LONG, "RadioButton",		RadioButton)
	WB_ZEND_CONST(LONG, "ScrollBar",		ScrollBar)
	WB_ZEND_CONST(LONG, "Slider",			Slider)
	WB_ZEND_CONST(LONG, "Spinner",			Spinner)
	WB_ZEND_CONST(LONG, "StatusBar",		StatusBar)
	WB_ZEND_CONST(LONG, "TabControl",		TabControl)
	WB_ZEND_CONST(LONG, "ToolBar",			ToolBar)
	WB_ZEND_CONST(LONG, "TreeView",			TreeView)

	// System constants

	WB_ZEND_CONST(STRING,"WBC_VERSION",		LIBVERSION)

	// Style flags

	WB_ZEND_CONST(LONG, "WBC_BORDER",		WBC_BORDER)
	WB_ZEND_CONST(LONG, "WBC_BOTTOM",		WBC_BOTTOM)
	WB_ZEND_CONST(LONG, "WBC_CENTER", 		WBC_CENTER)
	WB_ZEND_CONST(LONG, "WBC_CHECKBOXES",	WBC_CHECKBOXES)
	WB_ZEND_CONST(LONG, "WBC_CUSTOMDRAW",	WBC_CUSTOMDRAW)
	WB_ZEND_CONST(LONG, "WBC_DEFAULTPOS",	WBC_DEFAULTPOS)
	WB_ZEND_CONST(LONG, "WBC_DISABLED",		WBC_DISABLED)
	WB_ZEND_CONST(LONG, "WBC_ELLIPSIS",		WBC_ELLIPSIS)
	WB_ZEND_CONST(LONG, "WBC_ENABLED",		WBC_ENABLED)
	WB_ZEND_CONST(LONG, "WBC_GROUP",		WBC_GROUP)
	WB_ZEND_CONST(LONG, "WBC_IMAGE",		WBC_IMAGE)
	WB_ZEND_CONST(LONG, "WBC_INVISIBLE",	WBC_INVISIBLE)
	WB_ZEND_CONST(LONG, "WBC_LEFT", 		WBC_LEFT)
	WB_ZEND_CONST(LONG, "WBC_LINES",		WBC_LINES)
	WB_ZEND_CONST(LONG, "WBC_MASKED",		WBC_MASKED)
	WB_ZEND_CONST(LONG, "WBC_MIDDLE",		WBC_MIDDLE)
	WB_ZEND_CONST(LONG, "WBC_MULTILINE",	WBC_MULTILINE)
	WB_ZEND_CONST(LONG, "WBC_NOTIFY",		WBC_NOTIFY)
	WB_ZEND_CONST(LONG, "WBC_NUMBER",		WBC_NUMBER)
	WB_ZEND_CONST(LONG, "WBC_READONLY",		WBC_READONLY)
	WB_ZEND_CONST(LONG, "WBC_RIGHT", 		WBC_RIGHT)
	WB_ZEND_CONST(LONG, "WBC_SINGLE",		WBC_SINGLE)
	WB_ZEND_CONST(LONG, "WBC_SORT",			WBC_SORT)
	WB_ZEND_CONST(LONG, "WBC_TASKBAR",		WBC_TASKBAR)
	WB_ZEND_CONST(LONG, "WBC_AUTOREPEAT",	WBC_AUTOREPEAT)
	WB_ZEND_CONST(LONG, "WBC_TOP",			WBC_TOP)
	WB_ZEND_CONST(LONG, "WBC_VISIBLE",		WBC_VISIBLE)
	WB_ZEND_CONST(LONG, "WBC_TRANSPARENT",	WBC_TRANSPARENT)
	WB_ZEND_CONST(LONG, "WBC_DEFAULT",	WBC_DEFAULT)
	WB_ZEND_CONST(LONG, "WBC_MULTISELECT",	WBC_MULTISELECT)
	WB_ZEND_CONST(LONG, "WBC_NOHEADER",	WBC_NOHEADER)

	// Notification message flags

	WB_ZEND_CONST(LONG, "WBC_DBLCLICK",		WBC_DBLCLICK)
	WB_ZEND_CONST(LONG, "WBC_MOUSEMOVE",	WBC_MOUSEMOVE)
	WB_ZEND_CONST(LONG, "WBC_MOUSEDOWN",	WBC_MOUSEDOWN)
	WB_ZEND_CONST(LONG, "WBC_MOUSEUP",		WBC_MOUSEUP)
	WB_ZEND_CONST(LONG, "WBC_KEYDOWN",		WBC_KEYDOWN)
	WB_ZEND_CONST(LONG, "WBC_KEYUP",		WBC_KEYUP)
	WB_ZEND_CONST(LONG, "WBC_GETFOCUS",		WBC_GETFOCUS)
	WB_ZEND_CONST(LONG, "WBC_RESIZE",		WBC_RESIZE)
	WB_ZEND_CONST(LONG, "WBC_REDRAW",		WBC_REDRAW)
	WB_ZEND_CONST(LONG, "WBC_HEADERSEL",	WBC_HEADERSEL)

	// Additional notification message flags

	WB_ZEND_CONST(LONG, "WBC_ALT",			WBC_ALT)
	WB_ZEND_CONST(LONG, "WBC_CONTROL",		WBC_CONTROL)
	WB_ZEND_CONST(LONG, "WBC_SHIFT",		WBC_SHIFT)
	WB_ZEND_CONST(LONG, "WBC_LBUTTON",		WBC_LBUTTON)
	WB_ZEND_CONST(LONG, "WBC_MBUTTON",		WBC_MBUTTON)
	WB_ZEND_CONST(LONG, "WBC_RBUTTON",		WBC_RBUTTON)

	// Message box and beep types

	WB_ZEND_CONST(LONG, "WBC_BEEP",			WBC_BEEP)
	WB_ZEND_CONST(LONG, "WBC_INFO",			MB_ICONINFORMATION)
	WB_ZEND_CONST(LONG, "WBC_OK",			MB_OK)
	WB_ZEND_CONST(LONG, "WBC_OKCANCEL",		MB_OKCANCEL | MB_ICONQUESTION)
	WB_ZEND_CONST(LONG, "WBC_QUESTION",		MB_ICONQUESTION)
	WB_ZEND_CONST(LONG, "WBC_STOP",			MB_ICONSTOP)
	WB_ZEND_CONST(LONG, "WBC_WARNING",		MB_ICONWARNING)
	WB_ZEND_CONST(LONG, "WBC_YESNO",		MB_YESNO | MB_ICONQUESTION)
	WB_ZEND_CONST(LONG, "WBC_YESNOCANCEL",	MB_YESNOCANCEL | MB_ICONQUESTION)

	// Window states

	WB_ZEND_CONST(LONG, "WBC_MAXIMIZED",	WBC_MAXIMIZED)
	WB_ZEND_CONST(LONG, "WBC_MINIMIZED",	WBC_MINIMIZED)
	WB_ZEND_CONST(LONG, "WBC_NORMAL",		WBC_NORMAL)

	// Area types for wb_set_area()

	WB_ZEND_CONST(LONG, "WBC_MINSIZE",		WBC_MINSIZE);
	WB_ZEND_CONST(LONG, "WBC_MAXSIZE",		WBC_MAXSIZE);
	WB_ZEND_CONST(LONG, "WBC_TITLE",		WBC_TITLE);

	// Windows standard identifiers

	WB_ZEND_CONST(LONG, "IDABORT",			IDABORT);
	WB_ZEND_CONST(LONG, "IDCANCEL",			IDCANCEL);
	WB_ZEND_CONST(LONG, "IDCLOSE",			IDCLOSE);
	WB_ZEND_CONST(LONG, "IDDEFAULT",		IDDEFAULT);
	WB_ZEND_CONST(LONG, "IDHELP",			IDHELP);
	WB_ZEND_CONST(LONG, "IDIGNORE",			IDIGNORE);
	WB_ZEND_CONST(LONG, "IDNO",				IDNO);
	WB_ZEND_CONST(LONG, "IDOK",				IDOK);
	WB_ZEND_CONST(LONG, "IDRETRY",			IDRETRY);
	WB_ZEND_CONST(LONG, "IDYES",			IDYES);

	// Font attributes

	WB_ZEND_CONST(LONG, "FTA_BOLD",			FTA_BOLD);
	WB_ZEND_CONST(LONG, "FTA_ITALIC",		FTA_ITALIC);
	WB_ZEND_CONST(LONG, "FTA_NORMAL",		FTA_NORMAL);
	WB_ZEND_CONST(LONG, "FTA_REGULAR",		FTA_NORMAL);
	WB_ZEND_CONST(LONG, "FTA_STRIKEOUT",	FTA_STRIKEOUT);
	WB_ZEND_CONST(LONG, "FTA_UNDERLINE",	FTA_UNDERLINE);

	// RGB standard colors

	WB_ZEND_CONST(LONG, "BLACK",			RGB(  0,  0,  0))
	WB_ZEND_CONST(LONG, "BLUE",				RGB(  0,  0,255))
	WB_ZEND_CONST(LONG, "CYAN",				RGB(  0,255,255))
	WB_ZEND_CONST(LONG, "DARKBLUE",			RGB(  0,  0,128))
	WB_ZEND_CONST(LONG, "DARKCYAN",			RGB(  0,128,128))
	WB_ZEND_CONST(LONG, "DARKGRAY",			RGB(128,128,128))
	WB_ZEND_CONST(LONG, "DARKGREEN",		RGB(  0,128,  0))
	WB_ZEND_CONST(LONG, "DARKMAGENTA",		RGB(128,  0,128))
	WB_ZEND_CONST(LONG, "DARKRED",			RGB(128,  0,  0))
	WB_ZEND_CONST(LONG, "DARKYELLOW",		RGB(128,128,  0))
	WB_ZEND_CONST(LONG, "GREEN",			RGB(  0,255,  0))
	WB_ZEND_CONST(LONG, "LIGHTGRAY",		RGB(192,192,192))
	WB_ZEND_CONST(LONG, "MAGENTA",			RGB(255,  0,255))
	WB_ZEND_CONST(LONG, "RED",				RGB(255,  0,  0))
	WB_ZEND_CONST(LONG, "WHITE",			RGB(255,255,255))
	WB_ZEND_CONST(LONG, "YELLOW",			RGB(255,255,  0))
	WB_ZEND_CONST(LONG, "NOCOLOR",			NOCOLOR)

	// BGR standard colors

	WB_ZEND_CONST(LONG, "bgrBLACK",			RGB(  0,  0,  0))
	WB_ZEND_CONST(LONG, "bgrBLUE",			RGB(255,  0,  0))
	WB_ZEND_CONST(LONG, "bgrCYAN",			RGB(255,255,  0))
	WB_ZEND_CONST(LONG, "bgrDARKBLUE",		RGB(128,  0,  0))
	WB_ZEND_CONST(LONG, "bgrDARKCYAN",		RGB(128,128,  0))
	WB_ZEND_CONST(LONG, "bgrDARKGRAY",		RGB(128,128,128))
	WB_ZEND_CONST(LONG, "bgrDARKGREEN",		RGB(  0,128,  0))
	WB_ZEND_CONST(LONG, "bgrDARKMAGENTA",	RGB(128,  0,128))
	WB_ZEND_CONST(LONG, "bgrDARKRED",		RGB(  0,  0,128))
	WB_ZEND_CONST(LONG, "bgrDARKYELLOW",	RGB(  0,128,128))
	WB_ZEND_CONST(LONG, "bgrGREEN",			RGB(  0,255,  0))
	WB_ZEND_CONST(LONG, "bgrLIGHTGRAY",		RGB(192,192,192))
	WB_ZEND_CONST(LONG, "bgrMAGENTA",		RGB(255,  0,255))
	WB_ZEND_CONST(LONG, "bgrRED",			RGB(  0,  0,255))
	WB_ZEND_CONST(LONG, "bgrWHITE",			RGB(255,255,255))
	WB_ZEND_CONST(LONG, "bgrYELLOW",		RGB(  0,255,255))
	WB_ZEND_CONST(LONG, "bgrNOCOLOR",		NOCOLOR)

	// Listview custom color state
	WB_ZEND_CONST(LONG, "WBC_LV_NONE",		WBC_LV_NONE);
	WB_ZEND_CONST(LONG, "WBC_LV_FORE",		WBC_LV_FORE);
	WB_ZEND_CONST(LONG, "WBC_LV_BACK",		WBC_LV_BACK);
	WB_ZEND_CONST(LONG, "WBC_LV_DEFAULT",	WBC_LV_DEFAULT);
	WB_ZEND_CONST(LONG, "WBC_LV_DRAW",		WBC_LV_DRAW);
	WB_ZEND_CONST(LONG, "WBC_LV_COLUMNS",	WBC_LV_COLUMNS);
	return SUCCESS;
}

/* Provides module information (function provided by Hans Rebel) */

ZEND_MINFO_FUNCTION(winbinder)
{
	php_info_print_table_start();
	php_info_print_table_row(2, "WinBinder", "enabled");
	php_info_print_table_row(2, "Version", WINBINDER_VERSION);
	php_info_print_table_end();
}

/* Module shutdown function required by PHP */

ZEND_MSHUTDOWN_FUNCTION(winbinder)
{
	// End procedure

	wbEnd();
	return SUCCESS;
}

/* Stub function: implements the get_module() function, which is the only function actually exported */

ZEND_GET_MODULE(winbinder);

//------------------------------------------------------------------ END OF FILE
