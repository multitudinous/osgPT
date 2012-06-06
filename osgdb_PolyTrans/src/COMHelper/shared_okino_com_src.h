/*****************************************************************************

	This is the common include file that can be used to create
	Okino COM client applications. See the "Okino Plug-Ins SDK",
	 COM Interface section, for explanations about this file.

  Copyright (c) 1988, 2012 Okino Computer Graphics, Inc. All Rights Reserved.

This file is proprietary source code of Okino Computer Graphics, Inc. and it 
is not to be disclosed to third parties, published, adopted, distributed,
copied or duplicated in any form, in whole or in part without the prior 
authorization of Okino Computer Graphics, Inc. This file may, however, be
modified and recompiled solely for use with the NuGraf Rendering Toolkit; all
changes and modifications made to this file remain the property and copyright
of Okino Computer Graphics, Inc. 

		U.S. GOVERNMENT RESTRICTED RIGHTS NOTICE

This Source Code and Technical Material are provided with RESTRICTED RIGHTS. 
Use, duplication or disclosure by the U.S. Government is subject to restriction 
as set forth in subparagraph (c)(1) and (2) of FAR 52.227-19 or subparagraph 
(c)(1)(ii) of the Rights in Technical Data and Computer Software Clause at 
252.227-7013. Contractor/manufacturer is:

			Okino Computer Graphics, Inc. 
			3397 American Drive, Unit # 1
			Mississauga, Ontario
			L4V 1T8, Canada

OKINO COMPUTER GRAPHICS, INC. MAKES NO WARRANTY OF ANY KIND, EXPRESSED OR  
IMPLIED, INCLUDING WITHOUT LIMITATION ANY WARRANTIES OF MERCHANTABILITY AND/OR 
FITNESS FOR A PARTICULAR PURPOSE OF THIS SOFTWARE. OKINO COMPUTER GRAPHICS, INC. 
DOES NOT ASSUME ANY LIABILITY FOR THE USE OF THIS SOFTWARE.

IN NO EVENT WILL OKINO COMPUTER GRAPHICS, INC. BE LIABLE TO YOU FOR ANY ADDITIONAL 
DAMAGES, INCLUDING ANY LOST PROFITS, LOST SAVINGS, OR OTHER INCIDENTAL OR 
CONSEQUENTIAL DAMAGES ARISING FROM THE USE OF, OR INABILITY TO USE, THIS 
SOFTWARE AND ITS ACCOMPANYING DOCUMENTATION, EVEN IF OKINO COMPUTER GRAPHICS,
INC., OR ANY AGENT OF OKINO COMPUTER GRAPHICS, INC. HAS BEEN ADVISED OF THE   
POSSIBILITY OF SUCH DAMAGES.

*****************************************************************************/

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/* ----------------------->>>>  Includes  <<<<--------------------------- */

#ifdef _AFXDLL	// MFC specific
#include 	<afxwin.h>         		// MFC core and standard components
#include 	<afxext.h>         		// MFC extensions
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include 	<afxcmn.h>			// MFC support for Windows Common Controls
#endif
#else		// WIN32 specific
#define 	WIN32_LEAN_AND_MEAN	/* Don't include some of the Windows NT files */
#define		INCLUDE_COMMDLG_H	/* Include the common dialog include */
#include 	<windows.h>
#include	<commdlg.h>
#include	<stdio.h>
#endif

// ATL functions (the COM server/client is implemented in ATL, which is not related to MFC)
#include 	<atlbase.h>
extern 	CComModule _Module;	// Handle to our local defined ATL module
#include 	<atlcom.h>

// Miscellaneous includes
#include	<windowsx.h>	// Include the message cracker definitions
#include	<comdef.h>	// Windows COM interface defines
#include	<io.h>
#include	<direct.h>
#include 	<shlobj.h>
#include 	<shellapi.h>

// Extract the COM interface type library from the Okino COM server DLL 
// plug-in module. This will only work with Microsoft Visual C++ probably.
// If you have another compiler then comment out the #import line and 
// uncomment the #include line. Also, in your compilation project you will
// have to include the file "comsrv_i.c". The files comsrv.h and comsrv_i.c
// have to be obtained from Okino. Please send email to support@okino.com.
//
// NOTE: Microsoft has chosen to not allow both 32-bit and 64-bit versions
//       of a COM server on the same machine. Only one will be registered
//	 and run. "InProcHandl32" and "LocalServer32" now both refer to 32-bit
//	 and 64-bit COM servers. If you have both a 32-bit and 64-bit version
//	 of Okino software on the same machine then the program which was last
//	 executed will become the default COM server.
//
#ifdef _WIN64
#import "d:\\nugraf\\win64\\vcplugin64\\ui_dcom.dll" no_namespace named_guids raw_interfaces_only
#else
#import "C:\\Program Files (x86)\\polytrans\\vcplugin\\ui_dcom.dll" no_namespace named_guids raw_interfaces_only 
#endif
// #include "..\..\comsrv.h"

/* ------------------->>>>  Macro Definitions  <<<<--------------------- */

// Macro used to fill in the report controls
#define Nc_Add_Column_Heading(val, str, frmt) \
	lvC.iSubItem = val; \
	lvC.pszText = str;	\
	lvC.fmt = frmt;  	\
	GetTextExtentPoint(hdc, str, strlen(str), &Size); \
	lvC.cx = Size.cx+10;	\
	ListView_InsertColumn(hWndList, val, &lvC)

#define MAXFILETITLELEN	300		// Maximum filename length

#ifndef BIF_NEWDIALOGSTYLE
#define BIF_NEWDIALOGSTYLE     0x0040   // Use the new dialog layout with the ability to resize
#endif
#ifndef BIF_USENEWUI
#define BIF_USENEWUI           (BIF_NEWDIALOGSTYLE | BIF_EDITBOX)
#endif

// The Okino .bdf file format is not considered an import or export converter so
// there is no implied GUID for it. Thus, we'll assign this GUID to the .bdf format.
// This is needed, for example, when submitting a batch conversion job with .bdf output
#define	Nc_BDF_FILE_FORMAT_GUID		"{88B03069-2105-4413-874B-8708AFA348C2}"

/* ------------------->>>>  Global Variables  <<<<--------------------- */

// Our global interface pointers
extern	INuGrafIO	*pNuGrafIO;
extern	IConvertIO	*pConvertIO;
extern	IRenderIO	*pRenderIO;

extern	char 		last_geometry_filename_selected[];	// Used for end-user file selection
extern	short		updating_edit_control;			// We need this to prevent the edit controls from being updated as we modify their values
extern	short		abort_flag;				// A flag used to signify if the user pressed the CANCEL button on the import or export status dialog box
extern	HINSTANCE 	gInstance;				// Global instance for this module

// This is the current main dialog box window being shown. Anyone can use
// this to connect to the dialog box as a child window
extern	HWND	parent_hwnd;

// This is a handle to one of the listboxes on the dialog box panels which
// serve to be a dumping ground from error and status messages passed to us
// via an event sink from the COM server. Whenever you change a panel and
// wish to have messages shown on that panel, set this HWND to a listbox 
// shown on that panel. 
extern	HWND	curr_Error_Msg_Listbox_hwnd;

// This is a handle to one of the text lines on the dialog box panels which
// serve to receive the information status messages passes to us 
// via an event sink from the COM server. Whenever you change a panel and
// wish to have status messages shown on that panel, set this HWND to a text 
// line shown on that panel. 
extern	HWND	curr_Status_Msg_Text_Line_hwnd;

// -------------->>>>  Batch Conversion Struct Definitions   <<<<--------------

// This struct contains one batch conversion job to be queued up. It is a helper function for
// this test client only. When a new batch conversion is chosen by the user all info is stored
// in this struct. This info is then sent to a mirror copy in the host COM server which handles
// the batch conversion for us. When the job has completed the host will inform us and then we
// will delete the struct from our local linked list.
typedef struct Nd_Com_Batch_Converter_JobInfoStruct {
	// This is the unique Job ID (GUID) that uniquely identifies this job across time, space and invocations of PolyTrans. It is assigned by the COM server
	char	*job_id;
	
	// Information related to completed jobs
	struct {
		short	done;				// TRUE once the job has completed
		char	*status_text;			// "Completed", "Crashed" or "Aborted"
		char	*time_string;			// How long the job took to complete
		char	*job_temp_directory;		// Where the final job completion file was stored
	} job_completed;

	char	*filename_and_filepath; 			// Filename + path and extension of the input file to be converted
	char	*export_directory;				// Directory where to place the converted file(s). If NULL then place in the same directory as the input file
	char	*Importer_GUID;					// GUID of the import converter (or NULL if auto-load is to be used)
	char	*Exporter_GUID;					// GUID of the export converter
	char	*file_to_execute_when_conversion_done;		// Optional external .exe or script file name to execute once the conversion is done.
	char	*exporter_descriptive_name;			// A descriptive name of the exporter. Used internally only for the listview control.
	short	resize_model_to_viewport_upon_import;		// TRUE: resize the imported model to the camera viewport before exporting
	short	add_lights_to_scene_if_none;			// Optional: add two white lights to the scene if none in the imported scene
	short	perform_polygon_processing_on_converted_model;	// TRUE: perform polygon processing before exporting the model
	short	perform_polygon_reduction_on_converted_model;	// TRUE: perform polygon reduction before exporting the model
	short	create_backup_file;				// TRUE: create a backup file if the exported file already exists
	short 	compress_all_children_of_each_grouping_folder_to_single_object;	// TRUE: compress children of grouping nodes to single objects
	short 	hierarchy_optimizer1;				// TRUE: Enable hierarchy optimizer # 1
	short 	hierarchy_optimizer2;				// TRUE: Enable hierarchy optimizer # 2
	short 	apply_global_transform;				// TRUE: apply the global transformation during conversion
	short	make_batch_job_conversion_viewable_in_view_ports; // TRUE: Perform the conversion in the foreground so that we can view the conversion on the view windows. FALSE: perform the conversion like the batch converter whereby we can't see the file being imported & exported.

	// "Image snapshot " rendering related parameters

	// If image snapshot rendering is enabled, then a small bitmap image will be created
	// after the export process has finished. The bitmap will be named with the root name
	// of the job ID and a file extension to match the desired bitmap format. The image will
	// be stored in the Windows TEMP directory.

	struct {
		short	enable;			// Enable rendering snapshot
		short	res_x;			// X resolution
		short	res_y;			// Y resolution
		short	x_antialias;		// Scanline X anti-aliasing (0 = disabled)
		short	y_antialias;		// Scanline Y anti-aliasing (0 = disabled)
		char	renderlevel[20];	// Rendering level "scanline" or "raytrace"
		char 	format_str[20];		// File format desired, such as ".jpg", ".tif"
	} render;

	struct Nd_Com_Batch_Converter_JobInfoStruct *next_ptr;
} Nd_Com_Batch_Converter_JobInfoStruct;

// This is our list of 3D batch conversion jobs that have been queued up
extern Nd_Com_Batch_Converter_JobInfoStruct *OkinoCommonComSource___BatchConversionJobList;

/* ----------------->>>>  Structure Definitions  <<<<---------------------- */

// This struct is allocated in array fashion to list all the attributes
// about each exporter. Each exporter is queried from the host COM server
// program. The first element of the array, 'num_exporters_in_list' parameter,
// determines how many exporters there are in the list. 
// Use these corresponding functions from testclient.cpp:
//	OkinoCommonComSource___GetListofExportersAndTheirAttributesFromCOMServer()
//	OkinoCommonComSource___FreeListofExportersAndTheirAttributesObtainedFromCOMServer(Nd_Export_Converter_Info_List *list)
typedef struct Nd_Export_Converter_Info_List {
	short	num_exporters_in_list;		// Only valid in the first entry of the list

	// NOTE: If the export converter is an internal exporter and not 
	//	 one of the newfangled plug-in exporters, then the following
	//	 variables will have these values:
	//
	//	*dll_filepath = NULL
	//	*plugin_directory = NULL
	//	plugin_version = -1;
	//	has_options_dialog_box = TRUE
	//	has_about_dialog_box = TRUE
	//	*plugin_type_description = NULL
	//	*plugin_descriptive_name = NULL
	//	*menu_description = NULL
	//	*fileopen_filter_spec = NULL
	//	show_file_selector_before_exporting = TRUE

	// Output parameters
	short	menu_id;			// Dynamic menu ID assigned to this exporter
	short	about_menu_id;			// Dynamic menu ID assigned to this exporter
	short	geom_token;			// 'GEOMCONV_DXF'
	char	*file_type_desc;		// "3D Studio"; menu description
	char	*file_extensions;		// "(*.3ds; *.prj)" 
	char	*help_ref;			// Help string for the .hlp file
	char	*guid;				// Globally unique identifier for this exporter so that DCOM and other apps can reference this exporter directyly
	// The following are only valid if the export converter is a DLL plugin module and not one of the internal linked-in exporters
	char	*dll_filepath;			/* Absolute and valid filepath to the DLL */
	char	*plugin_directory;		/* The directory where this plug-in was found */
	short	plugin_version;			/* Version of the plug-in (starts at 1; increments by 1) */
	short	has_options_dialog_box;		/* TRUE if the plug-in has an options dialog box */
	short	has_about_dialog_box;		/* TRUE if the plug-in has an "About" dialog box */
	char	*plugin_type_description;	/* A description of this plug-in type (ie: "geometry import") */
	char	*plugin_descriptive_name;	/* Simplified/generic name for this DLL function (ie: "3D Studio") */
	char	*menu_description;		/* Entry for the execution menu (ie: "3D Studio .3DS") */
	char	*fileopen_filter_spec;		/* FileOpen filter specification (ie: "Lightwave Files (*.lw)|*.lw|") */
	short	show_file_selector_before_exporting; /* 1 = Show file selector prior to export, 0 = don't show file selector and don't pass in last filename to export converter. Version 2 interface. */
} Nd_Export_Converter_Info_List;

// This struct is allocated in array fashion to list all the attributes
// about each importer. Each importer is queried from the host COM server
// program. The first element of the array, 'num_importers_in_list' parameter,
// determines how many importers there are in the list. 
// Use these corresponding functions from testclient.cpp:
//	OkinoCommonComSource___GetListofImportersAndTheirAttributesFromCOMServer()
//	OkinoCommonComSource___FreeListofImportersAndTheirAttributesObtainedFromCOMServer(Nd_Import_Converter_Info_List *list)
typedef struct Nd_Import_Converter_Info_List {
	short	num_importers_in_list;		// Only valid in the first entry of the list

	// Import parameters
	char	*file_extensions;		// "(*.3ds; *.prj)" 
	char	*guid;				// Globally unique identifier for this exporter so that DCOM and other apps can reference this exporter directyly
	char	*dll_filepath;			/* Absolute and valid filepath to the DLL */
	char	*plugin_directory;		/* The directory where this plug-in was found */
	short	plugin_version;			/* Version of the plug-in (starts at 1; increments by 1) */
	short	has_options_dialog_box;		/* TRUE if the plug-in has an options dialog box */
	short	has_about_dialog_box;		/* TRUE if the plug-in has an "About" dialog box */
	char	*plugin_type_description;	/* A description of this plug-in type (ie: "geometry import") */
	char	*plugin_descriptive_name;	/* Simplified/generic name for this DLL function (ie: "3D Studio") */
	char	*menu_description;		/* Entry for the execution menu (ie: "3D Studio .3DS") */
	char	*fileopen_filter_spec;		/* FileOpen filter specification (ie: "Lightwave Files (*.lw)|*.lw|") */
	short	show_file_selector_before_importing; /* 1 = Show file selector prior to import, 0 = don't show file selector and don't pass in last filename to import converter (ie: SolidEdge and SolidWorks import converter) */
} Nd_Import_Converter_Info_List;

Nd_Export_Converter_Info_List 	*OkinoCommonComSource___GetListofExportersAndTheirAttributesFromCOMServer();
void				OkinoCommonComSource___FreeListofExportersAndTheirAttributesObtainedFromCOMServer(Nd_Export_Converter_Info_List *list);
Nd_Import_Converter_Info_List 	*OkinoCommonComSource___GetListofImportersAndTheirAttributesFromCOMServer();
void				OkinoCommonComSource___FreeListofImportersAndTheirAttributesObtainedFromCOMServer(Nd_Import_Converter_Info_List *list);

/* Pointer to a function returning an argument */
typedef void	(*Nd_Func)();
typedef int	(*Nd_Func2)();

// Struct which holds all the info about the test functions
typedef struct FNS {
	Nd_Func	func;
	char	*desc;
} FNS;
extern FNS test_functions[];

// Bit flags for 'NRS_IO_GetProgramFlags()'. Borrowed verbatim from wplugapi.h
#ifndef Nc_NRS_IO_FLAGS1_DEMO_MODE
#define	Nc_NRS_IO_FLAGS1_DEMO_MODE				0x2
#define	Nc_NRS_IO_FLAGS1_COMPILED_AS_DEMO_VERSION		0x200
#endif

// ------------------>>>>  Function Prototypes <<<<--------------------------

extern	void	OkinoCommonComSource___Init_ATL_COM_Module( HINSTANCE hInstance );
extern	void	OkinoCommonComSource___Terminate_ATL_COM_Module();
extern	void	OkinoCommonComSource___Initialization();
extern	BOOL	OkinoCommonComSource___AttachToCOMInterface(HWND parent_hwnd, HINSTANCE hInstance);
extern	void	OkinoCommonComSource___DetachFromCOMInterface();
extern	void 	OkinoCommonComSource___Set_Control_Enable(HWND hwndDlg, WORD id, BOOL bState);
extern	short	OkinoCommonComSource___HandleCompleteFileImportProcess(HWND parent_window_handle, char *input_filename_and_path, char *Importer_GUID_char, short show_import_converter_option_dialog_box, short reset_program_before_import, short resize_model_to_viewport_upon_import, short add_default_lights_to_scene_if_none, short ask_user_for_fileformat_if_unknown);
extern	short	OkinoCommonComSource___HandleCompleteFileExportProcess(HWND parent_window_handle, char *export_filename_and_path, char *Exporter_GUID_char, short show_export_converter_option_dialog_box, short create_backup_file);
extern	short 	OkinoCommonComSource___WGetListboxStringExtent(HWND hList, char *psz);
extern	void	OkinoCommonComSource___SimulateAMeter(HWND hDlg, short Control_ID, unsigned long curr_value, unsigned long total);
extern	BOOL 	OkinoCommonComSource___CheckAndProcessWindowsMessageQueue();
extern	BOOL	OkinoCommonComSource___HandleRawMessage(MSG *msg, short *aborted);
extern	void	OkinoCommonComSource___OutputInfoTextToGeomExportAbortStatusDlg(char *text);
extern	short	OkinoCommonComSource___StartComClassInterfacesAndEventSink();
extern	short	OkinoCommonComSource___RestartStandAloneNuGraforPolyTrans(HWND);
extern	short 	OkinoCommonComSource___StartEventSink();
extern	void 	OkinoCommonComSource___StopEventSink();
extern	bool	OkinoCommonComSource___IsTypeLibRegistered();
extern	short	OkinoCommonComSource___ShowCOMServerClientOptionsDialogBox(HINSTANCE hDLLinstance, HWND hwndOwner);
extern	void	OkinoCommonComSource___Center_Window(HWND hParent, HWND hChild);
extern  void	OkinoCommonComSource___Copy_String(char **string, char *new_contents);
extern  short	OkinoCommonComSource___DetermineImporterGUIDBasedOnFilenameOnly(HWND hDlg, char *input_filename, BSTR *return_importer_guid);
extern	void	OkinoCommonComSource___Display_COM_Error(_com_error &e, char *com_func_name);
extern	short	OkinoCommonComSource___Check_COM_HRESULT_Error_CODE(HRESULT hr, char *com_func_name);
extern	BOOL 	OkinoCommonComSource___NewWindowsBrowseForFolder2(HWND pParentWnd, char **initial_directory, char *title, char *display_name);
extern	int CALLBACK  OkinoCommonComSource___BrowseCtrlCallback(HWND hwnd, UINT uMsg, LPARAM /*lParam*/, LPARAM lpData);
extern	void	OkinoCommonComSource___ComputeCenteredWindowPosition(HWND hParent, short center_in_screen, short is_a_child_window, short child_width, short child_height, POINT *computed_offset);
extern	void	OkinoCommonComSource___ExtractBaseFilePath(char *in_filename, char *out_path);
extern	short	OkinoCommonComSource___ExtractBaseFileName(char *in_name, char *out_name, short root_filename_only);
extern	short	OkinoCommonComSource___GetLoadSaveFileName(short loading, HWND hDlg, HINSTANCE hInstance, char *window_title, char *selected_filename, short sizeof_selected_filename_buffer, char *FileOpen_Filter_Spec);
extern	short	OkinoCommonComSource___Setup_Select_Geometry_File(OPENFILENAME *of, short load_save, HWND hwnd, char *sub_directory, char *gszFilter, char *gszTitle, char *selected_filename, short ok_to_extract_filename, long Flags, LPOFNHOOKPROC lpfnHook, LPCSTR lpTemplateName, HINSTANCE hInstance, char *szFile, long sizeof_szFile);
extern	void	OkinoCommonComSource___CreateRelativeGeometryFilePath(char *sub_directory, char *output_filepath);
extern	char 	*OkinoCommonComSource___SelectGeometryImportFilename(HWND hDlg);
extern	void	OkinoCommonComSource___ShowInternalDialogBox(HWND hDlg, char *dialog_box_name);
extern	void	OkinoCommonComSource___NdInt_To_Dialog(HWND hDlg, WORD Control_ID, long val);
extern	void	OkinoCommonComSource___Dialog_To_NdInt(HWND hDlg, WORD Control_ID, long *val);
extern	short	OkinoCommonComSource___LoadOkinoBDFFile(char *filename_and_path, short confirm_reset);
extern	short	OkinoCommonComSource___SaveOkinoBDFFile(char *filename_and_path);
extern	short	OkinoCommonComSource___Check_For_File_Extension(char *in_file, char *extension);
extern	short	OkinoCommonComSource___MakeCOMServerGUIVisible();
extern	void	OkinoCommonComSource___SetEventSinkOverrideCallbackFuntion(char *event_sink_name, Nd_Func2 callback);
extern	bool	OkinoCommonComSource___IsCOMServerLicensed();
extern	void	OkinoCommonComSource___ShowCOMServerRegistrationDialogBox(HWND hwndParent, BOOL ok_to_ask_user_to_register_online);
extern	unsigned long	OkinoCommonComSource___get_ProgramFlags();
extern 	BOOL	OkinoCommonComSource___get_ProgramVersionInfo(short *major_version, short *minor_version, short *sub_minor_version);
extern	BOOL	OkinoCommonComSource___IsHostProgramVersionGreaterThanOrEqualTo(short major_version, short minor_version, short sub_minor_version);

// Batch conversion related functions
Nd_Com_Batch_Converter_JobInfoStruct 	*OkinoCommonComSource___Alloc_Com_Batch_Converter_JobInfoStruct(Nd_Com_Batch_Converter_JobInfoStruct **batch_list);
short		OkinoCommonComSource___Submit3DBatchConversionJobToCOMServer(Nd_Com_Batch_Converter_JobInfoStruct *job);
void		OkinoCommonComSource___HandleJobCompletedNotificationFromCOMServer(char *job_guid, char *status_text, char *time_string, char *windows_temp_dir);
void		OkinoCommonComSource___Free_Com_Batch_Converter_JobInfoStruct(Nd_Com_Batch_Converter_JobInfoStruct *job);
void		OkinoCommonComSource___Free_Com_Batch_Converter_JobInfo_LinkedList(Nd_Com_Batch_Converter_JobInfoStruct *job_linked_list);
void		OkinoCommonComSource___EnterJobListCriticalSection();
void		OkinoCommonComSource___ExitJobListCriticalSection();
void		OkinoCommonComSource___Clear_Done_Jobs_From_List();
INT_PTR CALLBACK OkinoCommonComSource___SelectAndAddNewBatchConversionJobDialogProc(HWND hDlg, UINT msg, WPARAM wparam, LPARAM lparam);

INT_PTR CALLBACK OkinoCommonComSource___GeometryImportStatusDlgProc(HWND hDlg, UINT wMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK OkinoCommonComSource___GeometryExportStatusDlgProc(HWND hDlg, UINT wMsg, WPARAM wParam, LPARAM lParam);

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

