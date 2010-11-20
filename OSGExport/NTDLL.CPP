/*****************************************************************************

	----------------------------------------------------------------
 	       Main Entry Point to the Export Converter DLL &
	     Dialog Box Handlers for "Options" + "About" Boxes
	----------------------------------------------------------------
	 This module forms the Microsoft Windows user interface to the
	      global options defined and used by this exporter. The
	     options are defined and handled in the options.cpp file.
	----------------------------------------------------------------

  Copyright (c) 1988, 2009 Okino Computer Graphics, Inc. All Rights Reserved.

This file is proprietary source code of Okino Computer Graphics, Inc. and it 
is not to be disclosed to third parties, published, adopted, distributed,
copied or duplicated in any form, in whole or in part without the prior 
authorization of Okino Computer Graphics, Inc. This file may, however, be
modified and recompiled solely for use as a PolyTrans export converter; the
changes and modifications made to this file remain the property and copyright
of Okino Computer Graphics, Inc. 

		U.S. GOVERNMENT RESTRICTED RIGHTS NOTICE

The PolyTrans Import/Export Converter Toolkit, the NuGraf Developer's 3D 
Toolkit, and their Technical Material are provided with RESTRICTED RIGHTS. 
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
//////////////////////////////////////////
//////////////////////////////////////////
//THIS FILE HAS A BUNCH OF OSG CODE IN IT
//DO NOT THROW IT AWAY UNTIL CHANGES FROM OKINO ARE MERGED WITH THIS FILES CONTENTS
//////////////////////////////////////////
//////////////////////////////////////////
// Standard Windows includes
#include	<stdio.h>
#include 	<windows.h>
#include 	<windowsx.h>

// Okino API includes
#include	"main.h"		// Main includes for this exporter

#ifndef _AFXDLL
#define INCLUDE_DLL_STUB_CODE		// Cause the DLL entry point code to be included from "implugin.h"
#endif
#include	<implugin.h>		// Geometry import plug-in definitions (from Import IO cover library)

#include	"resource.h"		// Resource definitions for this exporter

// ----------------------->>>>  Definitions   <<<<--------------------------

// Title for this options dialog box
#define	PROPERTY_SHEET_TITLE    "OpenSceneGraph Exporter"

// ----------------------->>>>  Local Variables  <<<<--------------------------

// Full path to the plug-in directory (with a \ at the end). Passed to us by the host program
char	*plugin_directory = NULL;

/* Global value to return a value from a property sheet. At present we just return IDOK or IDCANCEL */
static short	property_sheet_return_value;

// We need this to prevent the edit controls from being updated as we modify their values
static	short	updating_edit_control;

// ------------------>>>>  Function Prototypes <<<<--------------------------

static BOOL APIENTRY 	OSG1_DlgProc(HWND hDlg, UINT message, UINT wParam, LONG lParam);
static BOOL APIENTRY 	OSG2_DlgProc(HWND hDlg, UINT message, UINT wParam, LONG lParam);
static BOOL APIENTRY 	OSG3_DlgProc(HWND hDlg, UINT message, UINT wParam, LONG lParam);





static INT_PTR CALLBACK 	EnablesPanel1_Page_DlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
static INT_PTR CALLBACK 	EnablesPanel2_Page_DlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
static INT_PTR CALLBACK 	CurvesPanel_Page_DlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
static INT_PTR CALLBACK 	MeshProcessingPanel_Page_DlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
static INT_PTR CALLBACK 	BitmapConversionPanel_Page_DlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
static INT_PTR CALLBACK		PathsPanel_Page_DlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
static INT_PTR CALLBACK 	AnimationPanel_Page_DlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

static short			ShowOptionsDialogBox(HINSTANCE hDLLinstance, HWND parent_hwnd);
static void			FillInPropertyPage(PROPSHEETPAGE *psp, char *dlg_id_name, LPSTR pszProc, DLGPROC pfnDlgProc);
static int CALLBACK 		RemoveContextHelpProc(HWND hwnd, UINT message, LPARAM lParam);
static INT_PTR CALLBACK 	AboutDlgProc(HWND hDlg, UINT msg, WPARAM wparam, LPARAM lparam);

/* ------------------------------------------------------------------------- */

/* Source for the surface name assigned to each polygon */
static struct SURF_NAME_STRUCT {
	char	*desc;
	short	method;
} txtr_res_struct[] = {
	{ "Closest", 0 },
	{ "No Change", 1 },
	{ "2", 2 },
	{ "4", 4 },
	{ "8", 8 },
	{ "16", 16 },
	{ "32", 32 },
	{ "64", 64 },
	{ "128", 128 },
	{ "256", 256 },
	{ "512", 512 },
	{ NULL, 0 }
};

/* ------------------------------------------------------------------------- */

/* Euler rotation order for the animation panel */
static struct EULER_ORDER_STRUCT {
	char	*desc;
	short	order;
} euler_order_struct[] = {
	{ "xyz", Nc_EulerRotationOrder_XYZ },
	{ "xyx", Nc_EulerRotationOrder_XYX },
	{ "xzy", Nc_EulerRotationOrder_XZY },
	{ "xzx", Nc_EulerRotationOrder_XZX },
	{ "yzx", Nc_EulerRotationOrder_YZX },
	{ "yzy", Nc_EulerRotationOrder_YZY },
	{ "yxz", Nc_EulerRotationOrder_YXZ },
	{ "yxy", Nc_EulerRotationOrder_YXY },
	{ "zxy", Nc_EulerRotationOrder_ZXY },
	{ "zxz", Nc_EulerRotationOrder_ZXZ },
	{ "zyx", Nc_EulerRotationOrder_ZYX },
	{ "zyz", Nc_EulerRotationOrder_ZYZ },
	{ NULL, -1 }
};

/* ------------------------------------------------------------------------- */

/* Spline curve types (for resampling from NURBS curves to spline curves) */
static struct SPLINE_TYPE_STRUCT {
	char	*spline_name;
	char	*spline_token;
} spline_types_struct[] = {
	{ "[No Change]", 		NULL	 	},
	{ "Bezier Spline", 		"bezier" 	},
	{ "B-Spline", 			"bspline" 	},
	{ "Cardinal Spline", 		"cardinal" 	},
	{ "Linear Spline", 		"linear" 	},
	{ "Tensioned Spline", 		"tension" 	},
	{ "TCB Spline", 		"tcb" 		},
	{ NULL, 			""		}
};

/* ------------------------------------------------------------------------- */

// Spline Shape conversion modes for the drop-down boxes

// export_options->RenderableSplineShapeOutputMode
static struct RENDERABLE_SPLINE_SHAPE_OUTPUT_MODE_STRUCT {
	char	*desc;
	short	token;
} renderable_spline_shape_output_modes_struct[] = {
	{ "[No change]", 					Nc_RENDERABLE_SPLINE_SHAPE_OUTPUT_MODE___UNCHANGED			},
	{ "Convert and output as a polygon mesh", 		Nc_RENDERABLE_SPLINE_SHAPE_OUTPUT_MODE___AS_POLYGON_MESH		},
	{ "Convert and output as a trimmed NURBS surface", 	Nc_RENDERABLE_SPLINE_SHAPE_OUTPUT_MODE___AS_TRIMMED_NURBS_SURFACE	},
	{ NULL, 0 }
};

// export_options->NonRenderableSplineShapeOutputMode
static struct NON_RENDERABLE_SPLINE_SHAPE_OUTPUT_MODE_STRUCT {
	char	*desc;
	short	token;
} non_renderable_spline_shape_output_modes_struct[] = {
	{ "[No change]", 				Nc_NON_RENDERABLE_SPLINE_SHAPE_OUTPUT_MODE__UNCHANGED		},
	{ "Convert and output as NURBS curves", 	Nc_NON_RENDERABLE_SPLINE_SHAPE_OUTPUT_MODE__AS_NURBS_CURVES	},
	{ "Convert and output as Polylines", 		Nc_NON_RENDERABLE_SPLINE_SHAPE_OUTPUT_MODE__AS_POLYLINES	},
	{ NULL, 0 }
};

/* ---------------->>>>  NURBS Curve Conversion Modes  <<<<------------------ */

// NURBS Curve conversion modes for the drop-down boxes

// export_options->RenderableNURBSCurveOutputMode
static struct RENDERABLE_NURBS_CURVE_OUTPUT_MODE_STRUCT {
	char	*desc;
	short	token;
} renderable_nurbs_curve_output_modes_struct[] = {
	{ "[No change]", 				Nc_RENDERABLE_NURBS_CURVE_OUTPUT_MODE___UNCHANGED			},
	{ "Convert and output as polygon mesh", 	Nc_RENDERABLE_NURBS_CURVE_OUTPUT_MODE___AS_POLYGON_MESH			},
	{ "Convert and output as trimmed NURBS surface",Nc_RENDERABLE_NURBS_CURVE_OUTPUT_MODE___AS_TRIMMED_NURBS_SURFACE	},
	{ "Convert and output as Spline Shape", 	Nc_RENDERABLE_NURBS_CURVE_OUTPUT_MODE___AS_SPLINE_SHAPE			},
	{ NULL, 0 }
};

// export_options->NonRenderableNURBSCurveOutputMode
static struct NON_RENDERABLE_NURBS_CURVE_OUTPUT_MODE_STRUCT {
	char	*desc;
	short	token;
} non_renderable_nurbs_curve_output_modes_struct[] = {
	{ "[No change]", 					Nc_NON_RENDERABLE_NURBS_CURVE_OUTPUT_MODE__UNCHANGED			},
	{ "Output composite NURBS curve as single curve", 	Nc_NON_RENDERABLE_NURBS_CURVE_OUTPUT_MODE__COMPOSITE_TO_SINGLE_CURVE	},
	{ "Convert and output as Spline Shape", 		Nc_NON_RENDERABLE_NURBS_CURVE_OUTPUT_MODE__AS_SPLINE_SHAPE		},
	{ "Convert and output as Polylines", 			Nc_NON_RENDERABLE_NURBS_CURVE_OUTPUT_MODE__AS_POLYLINES			},
	{ NULL, 0 }
};

/* ----------------->>>>  Structure Definitions  <<<<---------------------- */

/* This is needed to remove the '?' context sensitive help from the property sheet */
#ifndef DLGTEMPLATEEX
#include <pshpack1.h>
typedef struct DLGTEMPLATEEX
{
    WORD dlgVer;
    WORD signature;
    DWORD helpID;
    DWORD exStyle;
    DWORD style;
    WORD cDlgItems;
    short x;
    short y;
    short cx;
    short cy;
} DLGTEMPLATEEX, *LPDLGTEMPLATEEX;
#include <poppack.h>
#endif

/* ------------------------------------------------------------------------- */

// This is the main entry point to the plug-in DLL. It is the only exported
// function from the DLL. The host program makes all contact with this DLL
// via this plug-in entry point.

// The commands are passed into this DLL via a token/data array, the last
// entry of which is NULL. 

// *** This function is not to be modified by a plug-in developer. ***

extern "C" {
	PLUGIN_RETURN_TYPE
NG_PLUGIN_ENTRYPOINT(char *tokens[], char *data[], long *return_status, 
	void *aux_ptr)
{
	HINSTANCE	parent_hInstance;
	short		count;
	char		*plugin_guid;	// The GUID contained in the .rti file used to invoke this exporter

	// *** This function is not to be modified by a plug-in developer. ***

	// Put at beginning of exported fns, in a DLL using shared MFC DLL.
#ifdef _AFXDLL
	AFX_MANAGE_STATE(AfxGetStaticModuleState( )); 
#endif

	aux_ptr = aux_ptr;		/* Unused auxiliary pointer */
	*return_status = FALSE;		/* No error */

	/* Parse the commands */
	count = 0;
	while (tokens != NULL && tokens[count] != NULL && tokens[count][0] != '\0') {
		if (!strcmp(tokens[count], "parent_hWnd")) {
			/* Handle of the parent window (always first) */
			parent_hwnd = (HWND) data[count];
		} else if (!strcmp(tokens[count], "parent_hInstance")) {
			/* Handle of the parent instance (always second) */
			parent_hInstance = (HINSTANCE) data[count];
		} else if (!strcmp(tokens[count], "plugin_guid")) {
			// This is the GUID contained in the .rti file which
			// is associated with the exporter being invoked.
			plugin_guid = data[count];
		} else if (!strcmp(tokens[count], "initialize")) {
			char ini_filepath[300];

			/* Make a copy of the directory where the plug-in module is located */
			plugin_directory = NULL;
			Import_IO_Copy_String(&plugin_directory, data[count]);

			/* Initialize the local variables of the converter */
			NI_Exporter_InitExportOptionVariables();

			/* Read in the default options from .INI file */
			strcpy(ini_filepath, plugin_directory);
			strcat(ini_filepath, INI_FILENAME);
			NI_Exporter_RestoreExportConvertOptions(ini_filepath);
		} else if (!strcmp(tokens[count], "shutdown")) {
			/* Shutdown the plug-in before it is unloaded */
			;
		} else if (!strcmp(tokens[count], "new_instance")) {
			/* Create a new instance of this plug-in. */
			/* data[count] will be set to the unique instance number. */
			/* Upon return, the plug-in can return a pointer to its */
			/* locally allocated data in the data[count] variable. */
			;
		} else if (!strcmp(tokens[count], "destroy_instance")) {
			/* Destroy a specific instance of this plug-in. */
			/* data[count] will contain the unique instance number */
			/* and 'aux_ptr' will contain the plug-in's pointer */
			/* which it returned from the "new_instance" command. */
			;
		} else if (!strcmp(tokens[count], "show_options_dialog")) {
			// Show the "Options" dialog box. Return FALSE if 'Cancel' button pressed.

			// The data argument is a string pointer (read only) which
			// represents the last filename exported previously.
			char	*last_filename_selected = data[count];

			// Show the options dialog box
			*return_status = ShowOptionsDialogBox(hDLLinstance, parent_hwnd);
		} else if (!strcmp(tokens[count], "show_about_dialog")) {
			// Show the "About" dialog box
			DialogBox(hDLLinstance, "AboutBox", parent_hwnd, (DLGPROC) AboutDlgProc);
		} else if (!strcmp(tokens[count], "get_SaveAs_filename")) {
			// If the 'Show_File_Selector_Before_Exporting' option inside
			// the .rti file is set to 0 (FALSE) then this command is called
			// for the exporter to ask the user for a Save-As filename. The
			// last filename selected by the user will be provided to this
			// DLL via the 'save_as_filename_io_buffer' pointer and likewise
			// the new selected filename will be returned in this same buffer.
			// The 'save_as_filename_io_buffer' is allocated by the host program.
			// If your exporter does not save out to files (such as saving to the
			// Windows clipboard instead) then set "save_as_filename_io_buffer[0] = '\0'" here.
			// Return FALSE if user pressed Cancel button, else TRUE

			char	*save_as_filename_io_buffer = data[count];			
			
#if 0	// Example code. Uncomment this code and change 'Show_File_Selector_Before_Exporting' in the .rti file to 0 to enable.
			if (!Export_IO_GetSaveAsFileName(parent_hwnd, parent_hInstance, "Select the export filename", save_as_filename_io_buffer, 300, "Text Files (*.txt)|*.txt|")) {
				// User pressed CANCEL on the dialog box
				save_as_filename_io_buffer[0] = '\0';
				*return_status = FALSE;	// Abort the export process because user pressed cancel button
			} else
#endif
				*return_status = TRUE;	// Don't cancel the export process
		} else if (!strcmp(tokens[count], "execute_geometry_converter")) {
			ExportConverter_Main(tokens+count+1, data+count+1, return_status, aux_ptr);
			break;
		}
		++count;
	} 
}
}

// ----------->>>>  Export Options Property Sheet Handler  <<<<----------------

#include <commctrl.h>
#include <prsht.h>      // includes the property sheet functionality

/* This is the main entry point to the options property sheet handler */

	static short
ShowOptionsDialogBox(HINSTANCE hDLLinstance, HWND hwndOwner)
{
	PROPSHEETPAGE 	psp[8];
	PROPSHEETHEADER psh;
	char		ini_filepath[512];
	Nd_Int		next_offset;

	// Create the property sheet panels
	next_offset = 0;
	FillInPropertyPage(&psp[next_offset++], "OptionsDialogOSG1", TEXT(""), (DLGPROC)OSG1_DlgProc);
	FillInPropertyPage(&psp[next_offset++], "OptionsDialogOSG2", TEXT(""), (DLGPROC)OSG2_DlgProc);
	FillInPropertyPage(&psp[next_offset++], "OptionsDialogOSG3", TEXT(""), (DLGPROC)OSG3_DlgProc);





    //These lines must be commented out or else there is a crash related to memory - mccdo
	//FillInPropertyPage(&psp[next_offset++], "OptionsDialogEnablesPanel1", TEXT(""), (DLGPROC) EnablesPanel1_Page_DlgProc);
	//FillInPropertyPage(&psp[next_offset++], "OptionsDialogEnablesPanel2", TEXT(""), (DLGPROC) EnablesPanel2_Page_DlgProc);
	//FillInPropertyPage(&psp[next_offset++], "OptionsDialogCurvePanel", TEXT(""), (DLGPROC) CurvesPanel_Page_DlgProc);
	//FillInPropertyPage(&psp[next_offset++], "OptionsDialogMeshProcessingPanel", TEXT(""), (DLGPROC) MeshProcessingPanel_Page_DlgProc);
	//FillInPropertyPage(&psp[next_offset++], "OptionsDialogBitmapConversionPanel", TEXT(""), (DLGPROC) BitmapConversionPanel_Page_DlgProc);
	//FillInPropertyPage(&psp[next_offset++], "OptionsDialogBitmapFilePathPanel", TEXT(""), (DLGPROC) PathsPanel_Page_DlgProc);
	//FillInPropertyPage(&psp[next_offset++], "OptionsDialogAnimationExportPanel", TEXT(""), (DLGPROC) AnimationPanel_Page_DlgProc);

	// Set up the main property sheet window
	psh.dwSize = sizeof(PROPSHEETHEADER);
	psh.dwFlags = PSH_PROPSHEETPAGE | PSH_HASHELP | PSH_PROPTITLE | PSH_USECALLBACK;	// | PSH_MODELESS;
	psh.dwFlags |= PSH_NOAPPLYNOW;		// Remove the Apply button
#ifdef PSH_NOCONTEXTHELP
	psh.dwFlags |= PSH_NOCONTEXTHELP;	// Remove the context sensitive '?'
#endif
	psh.hwndParent = hwndOwner;
	psh.pszCaption = (LPSTR) PROPERTY_SHEET_TITLE;
	psh.nPages = next_offset;
	psh.ppsp = (LPCPROPSHEETPAGE) &psp;
	psh.pfnCallback = RemoveContextHelpProc;
	psh.nStartPage = 0;

	/* First, grab the backup values in case we cancel */
	strcpy(ini_filepath, plugin_directory);
	strcat(ini_filepath, INI_FILENAME);
	NI_Exporter_SaveExportConvertOptions(ini_filepath);

	// Show the property sheet dialog box
	property_sheet_return_value = IDOK;
	PropertySheet(&psh);

	if (property_sheet_return_value == IDCANCEL) {
		// If the user cancelled then restore the parameters from the .ini file
		NI_Exporter_RestoreExportConvertOptions(ini_filepath);
		return(FALSE);
	} else {
		// Save the parameters to the .ini file
		NI_Exporter_SaveExportConvertOptions(ini_filepath);
		return(TRUE);
	}
}

// Callback to remove the '?' context sensitive help button from the main caption bar 

	static int CALLBACK 
RemoveContextHelpProc(HWND hwnd, UINT message, LPARAM lParam)
{
	switch (message) {
		case PSCB_PRECREATE:
			// Remove the DS_CONTEXTHELP style from the dialog template
			if (((LPDLGTEMPLATEEX)lParam)->signature ==  0xFFFF) {
			            ((LPDLGTEMPLATEEX)lParam)->style &= ~DS_CONTEXTHELP;
			} else {
				((LPDLGTEMPLATE)lParam)->style &= ~DS_CONTEXTHELP;
			}
			return TRUE;
	}
	return TRUE;
}

	static void 
FillInPropertyPage(PROPSHEETPAGE *psp, char *dlg_id_name, LPSTR pszProc, 
	DLGPROC pfnDlgProc)
{
	psp->dwSize = sizeof(PROPSHEETPAGE);
	psp->dwFlags = PSP_HASHELP;
	psp->hInstance = hDLLinstance;
	psp->pszTemplate = dlg_id_name;
	psp->pszIcon = NULL;
	psp->pfnDlgProc = pfnDlgProc;
	psp->pszTitle = pszProc;
	psp->lParam = 0;
}

static BOOL APIENTRY 
OSG1_DlgProc(HWND hDlg, UINT msg, UINT wParam, LONG lParam)
{
	static PROPSHEETPAGE *ps;
	LPPSHNOTIFY	lppsn;
	short		val, cmd_id, user_clicked_apply_button;

	switch (msg) {
	case WM_INITDIALOG:
		// Save the PROPSHEETPAGE information.
		ps = (PROPSHEETPAGE *) (LPARAM) lParam;

		// Center the window
		Import_IO_Center_Window(parent_hwnd, GetParent(hDlg));

		SendMessage(hDlg, WM_UPDATE_DIALOG_PARAMETERS, 0, 0L);
		return (TRUE);
	case WM_UPDATE_DIALOG_PARAMETERS:
		CheckDlgButton(hDlg, IDD_OSG_STRIP_TEXTURE_PATHS, export_options->osgStripTexturePaths);
		CheckDlgButton(hDlg, IDD_OSG_NORMALIZATION, export_options->osgNormalization);
		CheckDlgButton(hDlg, IDD_OSG_BACKFACE_CULLING, export_options->osgBackfaceCulling);
		CheckDlgButton(hDlg, IDD_OSG_DEBUG_TEXT_FILE, export_options->osgDebugTextFile);
		CheckDlgButton(hDlg, IDD_OSG_WRITE_HIDDEN_NODES, export_options->osgWriteHiddenNodes);

		if ( export_options->osgWriteFormat == 0 )
			val = IDD_OSG_WRITE_FORMAT_OSG;
		else if ( export_options->osgWriteFormat == 1 )
			val = IDD_OSG_WRITE_FORMAT_IVE;
		else
		{ // unknown
			export_options->osgWriteFormat = 0;
			val = IDD_OSG_WRITE_FORMAT_OSG;
		}
		CheckRadioButton( hDlg, IDD_OSG_WRITE_FORMAT_OSG, IDD_OSG_WRITE_FORMAT_IVE, val );

		/* Update the enable state of the controls */
		SendMessage(hDlg, WM_UPDATE_DIALOG_ENABLES, 0, 0L);

		return (TRUE);
	case WM_UPDATE_DIALOG_ENABLES:
		Import_IO_Set_Control_Enable(hDlg, IDD_OSG_WRITE_FORMAT_OSG, 1);
		Import_IO_Set_Control_Enable(hDlg, IDD_OSG_WRITE_FORMAT_IVE, 1);
		return(TRUE);
	case WM_NOTIFY:
    		switch (((NMHDR FAR *)  (LPARAM) lParam)->code) {
		case PSN_KILLACTIVE:
			// The page is either losing the activation, or the user pressed OK.
           		// SetWindowLong(hDlg, DWL_MSGRESULT, FALSE);	// No error
			break;
                case PSN_APPLY:
			// The Apply button was pressed, or the OK button was pressed
			// and this message is being sent to all panels to have it send
			// the control values to the toolkit.
                OKINO_SetWindowLong(hDlg, DWLP_MSGRESULT, PSNRET_INVALID);	// Error initially

			/* Only update the UI if the user clicked the 'Apply' button and not the 'Ok' button */
			lppsn = (LPPSHNOTIFY)  (LPARAM) lParam;
			user_clicked_apply_button = !lppsn->lParam;
			if (!user_clicked_apply_button)
				property_sheet_return_value = IDOK;

           		OKINO_SetWindowLong(hDlg, DWLP_MSGRESULT, PSNRET_NOERROR);
			return(TRUE);
		case PSN_QUERYCANCEL:
			// The user has pressed CANCEL. Return FALSE to allow canceling, or TRUE to prevent it
           		OKINO_SetWindowLong(hDlg, DWLP_MSGRESULT, FALSE);
			property_sheet_return_value = IDCANCEL;
			return(FALSE);
		case PSN_RESET:
			// User has pressed CANCEL and property sheet is about to be destroyed
           		OKINO_SetWindowLong(hDlg, DWLP_MSGRESULT, FALSE);
			property_sheet_return_value = IDCANCEL;
			break;
		case PSN_SETACTIVE:
			// Property sheet is about to be activated. We set up the values of the controls here

			/* Enable the 'Apply' button */
			SendMessage(GetParent(hDlg), PSM_CHANGED, (WPARAM) hDlg, 0L);

			/* Update the enable state of the controls */
			SendMessage(hDlg, WM_UPDATE_DIALOG_PARAMETERS, 0, 0L);
			SendMessage(hDlg, WM_UPDATE_DIALOG_ENABLES, 0, 0L);

			break;
                case PSN_HELP:
			// The Help button was pressed
			Import_IO_DisplayHelpTopic(hDlg, plugin_directory, HELP_FILENAME, "main");
			break;
		default:
			return FALSE;
	    	}
		break;
	case WM_COMMAND:
		cmd_id = LOWORD(wParam);
		switch (cmd_id) {
			case IDD_BUTTON_ABOUT:
				DialogBox(hDLLinstance, "AboutBox", parent_hwnd, AboutDlgProc);
				return(TRUE);
			case IDD_BUTTON_RESET:
				if (IDYES == MessageBox(hDlg, "This will reset the values of this dialog box to their power-up defaults.\n\nAre you sure you want to do this?", "Confirmation Request", MB_APPLMODAL | MB_YESNO | MB_DEFBUTTON1 | MB_ICONQUESTION)) {
					export_options->options_initialized = FALSE;
					NI_Exporter_InitExportOptionVariables();
					SendMessage(hDlg, WM_UPDATE_DIALOG_PARAMETERS, 0, 0L);
				}
				return(TRUE);

			case IDD_OSG_STRIP_TEXTURE_PATHS:
				export_options->osgStripTexturePaths = !export_options->osgStripTexturePaths;
				CheckDlgButton(hDlg, IDD_OSG_STRIP_TEXTURE_PATHS, export_options->osgStripTexturePaths);
				/* Update the enable state of the controls */
				SendMessage(hDlg, WM_UPDATE_DIALOG_ENABLES, 0, 0L);
				return(TRUE);
			case IDD_OSG_NORMALIZATION:
				export_options->osgNormalization = !export_options->osgNormalization;
				CheckDlgButton(hDlg, IDD_OSG_NORMALIZATION, export_options->osgNormalization);
				/* Update the enable state of the controls */
				SendMessage(hDlg, WM_UPDATE_DIALOG_ENABLES, 0, 0L);
				return(TRUE);
			case IDD_OSG_BACKFACE_CULLING:
				export_options->osgBackfaceCulling = !export_options->osgBackfaceCulling;
				CheckDlgButton(hDlg, IDD_OSG_BACKFACE_CULLING, export_options->osgBackfaceCulling);
				/* Update the enable state of the controls */
				SendMessage(hDlg, WM_UPDATE_DIALOG_ENABLES, 0, 0L);
				return(TRUE);
			case IDD_OSG_DEBUG_TEXT_FILE:
				export_options->osgDebugTextFile = !export_options->osgDebugTextFile;
				CheckDlgButton(hDlg, IDD_OSG_DEBUG_TEXT_FILE, export_options->osgDebugTextFile);
				/* Update the enable state of the controls */
				SendMessage(hDlg, WM_UPDATE_DIALOG_ENABLES, 0, 0L);
				return(TRUE);
			case IDD_OSG_WRITE_HIDDEN_NODES:
				export_options->osgWriteHiddenNodes = !export_options->osgWriteHiddenNodes;
				CheckDlgButton(hDlg, IDD_OSG_WRITE_HIDDEN_NODES, export_options->osgWriteHiddenNodes);
				/* Update the enable state of the controls */
				SendMessage(hDlg, WM_UPDATE_DIALOG_ENABLES, 0, 0L);
				return(TRUE);

			case IDD_OSG_WRITE_FORMAT_OSG:
				export_options->osgWriteFormat = 0;
				CheckRadioButton( hDlg, IDD_OSG_WRITE_FORMAT_OSG, IDD_OSG_WRITE_FORMAT_IVE, IDD_OSG_WRITE_FORMAT_OSG );
				/* Update the enable state of the controls */
				SendMessage( hDlg, WM_UPDATE_DIALOG_ENABLES, 0, 0L );
				return(TRUE);

            case IDD_OSG_WRITE_FORMAT_IVE:
				export_options->osgWriteFormat = 1;
				CheckRadioButton( hDlg, IDD_OSG_WRITE_FORMAT_OSG, IDD_OSG_WRITE_FORMAT_IVE, IDD_OSG_WRITE_FORMAT_IVE );
				/* Update the enable state of the controls */
				SendMessage( hDlg, WM_UPDATE_DIALOG_ENABLES, 0, 0L );
				return(TRUE);

			default:
				break;
		}
	}
	return (FALSE);
}

static BOOL APIENTRY 
OSG2_DlgProc(HWND hDlg, UINT msg, UINT wParam, LONG lParam)
{
	static PROPSHEETPAGE *ps;
	LPPSHNOTIFY	lppsn;
	short		cmd_id, user_clicked_apply_button;

	switch (msg) {
	case WM_INITDIALOG:
		// Save the PROPSHEETPAGE information.
		ps = (PROPSHEETPAGE *) (LPARAM) lParam;

		// Center the window
		Import_IO_Center_Window(parent_hwnd, GetParent(hDlg));

		SendMessage(hDlg, WM_UPDATE_DIALOG_PARAMETERS, 0, 0L);
		return (TRUE);
	case WM_UPDATE_DIALOG_PARAMETERS:
		CheckDlgButton(hDlg, IDD_OSG_RUN_OPTIMIZER, export_options->osgRunOptimizer);
		CheckDlgButton(hDlg, IDD_OSG_MERGE_GEODES, export_options->osgMergeGeodes);
		CheckDlgButton(hDlg, IDD_OSG_SHARE_STATE, export_options->osgShareState);
        CheckDlgButton(hDlg, IDD_OSG_MERGE_GEOMETRY, export_options->osgMergeGeometry);
        CheckDlgButton(hDlg, IDD_OSG_CREATE_TRIANGLE_STRIPS, export_options->osgCreateTriangleStrips);
        CheckDlgButton(hDlg, IDD_OSG_REMOVE_REDUNDANT_NODES, export_options->osgRemoveRedundantNodes);
        CheckDlgButton(hDlg, IDD_OSG_STRIP_IMPLICIT_NAMES, export_options->osgStripImplicitNames);

		/* Update the enable state of the controls */
		SendMessage(hDlg, WM_UPDATE_DIALOG_ENABLES, 0, 0L);

		return (TRUE);
	case WM_UPDATE_DIALOG_ENABLES:
        Import_IO_Set_Control_Enable(hDlg, IDD_OSG_MERGE_GEODES, export_options->osgRunOptimizer);
        Import_IO_Set_Control_Enable(hDlg, IDD_OSG_SHARE_STATE, export_options->osgRunOptimizer);
        Import_IO_Set_Control_Enable(hDlg, IDD_OSG_MERGE_GEOMETRY, export_options->osgRunOptimizer);
        Import_IO_Set_Control_Enable(hDlg, IDD_OSG_CREATE_TRIANGLE_STRIPS, export_options->osgRunOptimizer);
        Import_IO_Set_Control_Enable(hDlg, IDD_OSG_REMOVE_REDUNDANT_NODES, export_options->osgRunOptimizer);
        Import_IO_Set_Control_Enable(hDlg, IDD_OSG_STRIP_IMPLICIT_NAMES, export_options->osgRunOptimizer);

        return(TRUE);
	case WM_NOTIFY:
    		switch (((NMHDR FAR *)  (LPARAM) lParam)->code) {
		case PSN_KILLACTIVE:
			// The page is either losing the activation, or the user pressed OK.
           		// SetWindowLong(hDlg, DWL_MSGRESULT, FALSE);	// No error
			break;
                case PSN_APPLY:
			// The Apply button was pressed, or the OK button was pressed
			// and this message is being sent to all panels to have it send
			// the control values to the toolkit.
           		OKINO_SetWindowLong(hDlg, DWLP_MSGRESULT, PSNRET_INVALID);	// Error initially

			/* Only update the UI if the user clicked the 'Apply' button and not the 'Ok' button */
			lppsn = (LPPSHNOTIFY)  (LPARAM) lParam;
			user_clicked_apply_button = !lppsn->lParam;
			if (!user_clicked_apply_button)
				property_sheet_return_value = IDOK;

           		OKINO_SetWindowLong(hDlg, DWLP_MSGRESULT, PSNRET_NOERROR);
			return(TRUE);
		case PSN_QUERYCANCEL:
			// The user has pressed CANCEL. Return FALSE to allow canceling, or TRUE to prevent it
           		OKINO_SetWindowLong(hDlg, DWLP_MSGRESULT, FALSE);
			property_sheet_return_value = IDCANCEL;
			return(FALSE);
		case PSN_RESET:
			// User has pressed CANCEL and property sheet is about to be destroyed
           		OKINO_SetWindowLong(hDlg, DWLP_MSGRESULT, FALSE);
			property_sheet_return_value = IDCANCEL;
			break;
		case PSN_SETACTIVE:
			// Property sheet is about to be activated. We set up the values of the controls here

			/* Enable the 'Apply' button */
			SendMessage(GetParent(hDlg), PSM_CHANGED, (WPARAM) hDlg, 0L);

			/* Update the enable state of the controls */
			SendMessage(hDlg, WM_UPDATE_DIALOG_PARAMETERS, 0, 0L);
			SendMessage(hDlg, WM_UPDATE_DIALOG_ENABLES, 0, 0L);

			break;
                case PSN_HELP:
			// The Help button was pressed
			Import_IO_DisplayHelpTopic(hDlg, plugin_directory, HELP_FILENAME, "main");
			break;
		default:
			return FALSE;
	    	}
		break;
	case WM_COMMAND:
		cmd_id = LOWORD(wParam);
		switch (cmd_id) {
			case IDD_BUTTON_ABOUT:
				DialogBox(hDLLinstance, "AboutBox", parent_hwnd, AboutDlgProc);
				return(TRUE);
			case IDD_BUTTON_RESET:
				if (IDYES == MessageBox(hDlg, "This will reset the values of this dialog box to their power-up defaults.\n\nAre you sure you want to do this?", "Confirmation Request", MB_APPLMODAL | MB_YESNO | MB_DEFBUTTON1 | MB_ICONQUESTION)) {
					export_options->options_initialized = FALSE;
					NI_Exporter_InitExportOptionVariables();
					SendMessage(hDlg, WM_UPDATE_DIALOG_PARAMETERS, 0, 0L);
				}
				return(TRUE);

            case IDD_OSG_RUN_OPTIMIZER:
				export_options->osgRunOptimizer = !export_options->osgRunOptimizer;
				CheckDlgButton(hDlg, IDD_OSG_RUN_OPTIMIZER, export_options->osgRunOptimizer);
				/* Update the enable state of the controls */
				SendMessage(hDlg, WM_UPDATE_DIALOG_ENABLES, 0, 0L);
				return(TRUE);

            case IDD_OSG_MERGE_GEODES:
				export_options->osgMergeGeodes = !export_options->osgMergeGeodes;
				CheckDlgButton(hDlg, IDD_OSG_MERGE_GEODES, export_options->osgMergeGeodes);
				/* Update the enable state of the controls */
				SendMessage(hDlg, WM_UPDATE_DIALOG_ENABLES, 0, 0L);
				return(TRUE);

            case IDD_OSG_SHARE_STATE:
				export_options->osgShareState = !export_options->osgShareState;
				CheckDlgButton(hDlg, IDD_OSG_SHARE_STATE, export_options->osgShareState);
				/* Update the enable state of the controls */
				SendMessage(hDlg, WM_UPDATE_DIALOG_ENABLES, 0, 0L);
				return(TRUE);

            case IDD_OSG_MERGE_GEOMETRY:
				export_options->osgMergeGeometry = !export_options->osgMergeGeometry;
				CheckDlgButton(hDlg, IDD_OSG_MERGE_GEOMETRY, export_options->osgMergeGeometry);
				/* Update the enable state of the controls */
				SendMessage(hDlg, WM_UPDATE_DIALOG_ENABLES, 0, 0L);
				return(TRUE);

            case IDD_OSG_CREATE_TRIANGLE_STRIPS:
                export_options->osgCreateTriangleStrips = !export_options->osgCreateTriangleStrips;
                CheckDlgButton(hDlg, IDD_OSG_CREATE_TRIANGLE_STRIPS, export_options->osgCreateTriangleStrips);
				/* Update the enable state of the controls */
				SendMessage(hDlg, WM_UPDATE_DIALOG_ENABLES, 0, 0L);
				return(TRUE);

            case IDD_OSG_REMOVE_REDUNDANT_NODES:
				export_options->osgRemoveRedundantNodes = !export_options->osgRemoveRedundantNodes;
				CheckDlgButton(hDlg, IDD_OSG_REMOVE_REDUNDANT_NODES, export_options->osgRemoveRedundantNodes);
				/* Update the enable state of the controls */
				SendMessage(hDlg, WM_UPDATE_DIALOG_ENABLES, 0, 0L);
				return(TRUE);

            case IDD_OSG_STRIP_IMPLICIT_NAMES:
				export_options->osgStripImplicitNames = !export_options->osgStripImplicitNames;
				CheckDlgButton(hDlg, IDD_OSG_STRIP_IMPLICIT_NAMES, export_options->osgStripImplicitNames);
				/* Update the enable state of the controls */
				SendMessage(hDlg, WM_UPDATE_DIALOG_ENABLES, 0, 0L);
				return(TRUE);

            default:
				break;
		}
	}
	return (FALSE);
}

static BOOL APIENTRY 
OSG3_DlgProc(HWND hDlg, UINT msg, UINT wParam, LONG lParam)
{
	static PROPSHEETPAGE *ps;
	LPPSHNOTIFY	lppsn;
	short		cmd_id, val, user_clicked_apply_button;

	switch (msg) {
	case WM_INITDIALOG:
		// Save the PROPSHEETPAGE information.
		ps = (PROPSHEETPAGE *) (LPARAM) lParam;

		// Center the window
		Import_IO_Center_Window(parent_hwnd, GetParent(hDlg));

		SendMessage(hDlg, WM_UPDATE_DIALOG_PARAMETERS, 0, 0L);
		return (TRUE);
	case WM_UPDATE_DIALOG_PARAMETERS:
		if ( export_options->osgInstanceFile )
			val = IDD_OSG_INSTANCE_FILE_RB;
		else if ( export_options->osgInstanceShared )
			val = IDD_OSG_INSTANCE_SHARED_RB;
		else
			val = IDD_OSG_INSTANCE_IGNORE_RB;
		CheckRadioButton( hDlg, IDD_OSG_INSTANCE_FILE_RB, IDD_OSG_INSTANCE_IGNORE_RB, val );

		/* Update the enable state of the controls */
		SendMessage( hDlg, WM_UPDATE_DIALOG_ENABLES, 0, 0L );

		return (TRUE);
	case WM_UPDATE_DIALOG_ENABLES:
		Import_IO_Set_Control_Enable(hDlg, IDD_OSG_INSTANCE_FILE_RB, 1);
		Import_IO_Set_Control_Enable(hDlg, IDD_OSG_INSTANCE_SHARED_RB, 1);
		Import_IO_Set_Control_Enable(hDlg, IDD_OSG_INSTANCE_IGNORE_RB, 1);

        return(TRUE);
	case WM_NOTIFY:
    		switch (((NMHDR FAR *)  (LPARAM) lParam)->code) {
		case PSN_KILLACTIVE:
			// The page is either losing the activation, or the user pressed OK.
           		// SetWindowLong(hDlg, DWL_MSGRESULT, FALSE);	// No error
			break;
                case PSN_APPLY:
			// The Apply button was pressed, or the OK button was pressed
			// and this message is being sent to all panels to have it send
			// the control values to the toolkit.
           		OKINO_SetWindowLong(hDlg, DWLP_MSGRESULT, PSNRET_INVALID);	// Error initially

			/* Only update the UI if the user clicked the 'Apply' button and not the 'Ok' button */
			lppsn = (LPPSHNOTIFY)  (LPARAM) lParam;
			user_clicked_apply_button = !lppsn->lParam;
			if (!user_clicked_apply_button)
				property_sheet_return_value = IDOK;

           		OKINO_SetWindowLong(hDlg, DWLP_MSGRESULT, PSNRET_NOERROR);
			return(TRUE);
		case PSN_QUERYCANCEL:
			// The user has pressed CANCEL. Return FALSE to allow canceling, or TRUE to prevent it
           		OKINO_SetWindowLong(hDlg, DWLP_MSGRESULT, FALSE);
			property_sheet_return_value = IDCANCEL;
			return(FALSE);
		case PSN_RESET:
			// User has pressed CANCEL and property sheet is about to be destroyed
           		OKINO_SetWindowLong(hDlg, DWLP_MSGRESULT, FALSE);
			property_sheet_return_value = IDCANCEL;
			break;
		case PSN_SETACTIVE:
			// Property sheet is about to be activated. We set up the values of the controls here

			/* Enable the 'Apply' button */
			SendMessage(GetParent(hDlg), PSM_CHANGED, (WPARAM) hDlg, 0L);

			/* Update the enable state of the controls */
			SendMessage(hDlg, WM_UPDATE_DIALOG_PARAMETERS, 0, 0L);
			SendMessage(hDlg, WM_UPDATE_DIALOG_ENABLES, 0, 0L);

			break;
                case PSN_HELP:
			// The Help button was pressed
			Import_IO_DisplayHelpTopic(hDlg, plugin_directory, HELP_FILENAME, "main");
			break;
		default:
			return FALSE;
	    	}
		break;
	case WM_COMMAND:
		cmd_id = LOWORD(wParam);
		switch (cmd_id) {
			case IDD_BUTTON_ABOUT:
				DialogBox(hDLLinstance, "AboutBox", parent_hwnd, AboutDlgProc);
				return(TRUE);
			case IDD_BUTTON_RESET:
				if (IDYES == MessageBox(hDlg, "This will reset the values of this dialog box to their power-up defaults.\n\nAre you sure you want to do this?", "Confirmation Request", MB_APPLMODAL | MB_YESNO | MB_DEFBUTTON1 | MB_ICONQUESTION)) {
					export_options->options_initialized = FALSE;
					NI_Exporter_InitExportOptionVariables();
					SendMessage(hDlg, WM_UPDATE_DIALOG_PARAMETERS, 0, 0L);
				}
				return(TRUE);

			case IDD_OSG_INSTANCE_FILE_RB:
				export_options->osgInstanceFile = TRUE;
                export_options->osgInstanceShared = export_options->osgInstanceIgnore = FALSE;
				CheckRadioButton( hDlg, IDD_OSG_INSTANCE_FILE_RB, IDD_OSG_INSTANCE_IGNORE_RB, IDD_OSG_INSTANCE_FILE_RB );
				/* Update the enable state of the controls */
				SendMessage( hDlg, WM_UPDATE_DIALOG_ENABLES, 0, 0L );
				return(TRUE);

            case IDD_OSG_INSTANCE_SHARED_RB:
				export_options->osgInstanceShared = TRUE;
                export_options->osgInstanceFile = export_options->osgInstanceIgnore = FALSE;
				CheckRadioButton( hDlg, IDD_OSG_INSTANCE_FILE_RB, IDD_OSG_INSTANCE_IGNORE_RB, IDD_OSG_INSTANCE_SHARED_RB );
				/* Update the enable state of the controls */
				SendMessage( hDlg, WM_UPDATE_DIALOG_ENABLES, 0, 0L );
				return(TRUE);

            case IDD_OSG_INSTANCE_IGNORE_RB:
				export_options->osgInstanceIgnore = TRUE;
                export_options->osgInstanceShared = export_options->osgInstanceFile = FALSE;
				CheckRadioButton( hDlg, IDD_OSG_INSTANCE_FILE_RB, IDD_OSG_INSTANCE_IGNORE_RB, IDD_OSG_INSTANCE_IGNORE_RB );
				/* Update the enable state of the controls */
				SendMessage( hDlg, WM_UPDATE_DIALOG_ENABLES, 0, 0L );

			default:
				break;
		}
	}
	return (FALSE);
}


/* Enables Panel # 1*/

	static INT_PTR CALLBACK 
EnablesPanel1_Page_DlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static PROPSHEETPAGE *ps;
	LPPSHNOTIFY	lppsn;
	short		cmd_id, user_clicked_apply_button;

	switch (msg) {
	case WM_INITDIALOG:
		// Save the PROPSHEETPAGE information.
		ps = (PROPSHEETPAGE *) (LPARAM) lParam;

		// Center the window
		Import_IO_Center_Window(parent_hwnd, GetParent(hDlg));

		SendMessage(hDlg, WM_UPDATE_DIALOG_PARAMETERS, 0, 0L);
		return (TRUE);
	case WM_UPDATE_DIALOG_PARAMETERS:
		CheckDlgButton(hDlg, IDD_EXPORTER_ENA_MESHDATA, export_options->ena_mesh_data);
		CheckDlgButton(hDlg, IDD_EXPORTER_ENA_VERTEX_NORMALS, export_options->ena_vertex_normals);
		CheckDlgButton(hDlg, IDD_EXPORTER_ENA_TEXTURE_COORDS, export_options->ena_texture_coords);
		CheckDlgButton(hDlg, IDD_EXPORTER_OUTPUT_VERTEX_COLORS, export_options->output_vertex_colors);
		CheckDlgButton(hDlg, IDD_EXPORTER_OUTPUT_UV_TANGENTS, 	export_options->ena_uv_tangents);
		CheckDlgButton(hDlg, IDD_EXPORTER_OUTPUT_MESH_SKINNING_WEIGHTS,			export_options->ena_mesh_skinning_weights);
		CheckDlgButton(hDlg, IDD_EXPORTER_ENA_LINE_SETS, export_options->ena_lines);
		CheckDlgButton(hDlg, IDD_EXPORTER_ENA_POINT_SETS, export_options->ena_pointsets);

		CheckDlgButton(hDlg, IDD_EXPORTER_ENA_NURBS_PATCHES, export_options->ena_nurbs_patches);
		CheckDlgButton(hDlg, IDD_EXPORTER_NURBS_AS_POLYGONS, 	export_options->nurbs_as_polygons);
		CheckDlgButton(hDlg, IDD_EXPORTER_OUTPUT_TRIM_CURVES, 	export_options->ena_trim_curves);

		CheckDlgButton(hDlg, IDD_EXPORTER_OUTPUT_MATERIALS, export_options->ena_materials);
		CheckDlgButton(hDlg, IDD_EXPORTER_OUTPUT_SURFTXTR_LAYERS, export_options->ena_surftxtr_layers);

		/* Update the enable state of the controls */
		SendMessage(hDlg, WM_UPDATE_DIALOG_ENABLES, 0, 0L);

		return (TRUE);
	case WM_UPDATE_DIALOG_ENABLES:
		Import_IO_Set_Control_Enable(hDlg, IDD_EXPORTER_ENA_VERTEX_NORMALS, export_options->ena_mesh_data);
		Import_IO_Set_Control_Enable(hDlg, IDD_EXPORTER_ENA_TEXTURE_COORDS, export_options->ena_mesh_data);
		Import_IO_Set_Control_Enable(hDlg, IDD_EXPORTER_OUTPUT_VERTEX_COLORS, export_options->ena_mesh_data);
		Import_IO_Set_Control_Enable(hDlg, IDD_EXPORTER_OUTPUT_UV_TANGENTS, export_options->ena_mesh_data);

		Import_IO_Set_Control_Enable(hDlg, IDD_EXPORTER_NURBS_AS_POLYGONS, export_options->ena_nurbs_patches);
		Import_IO_Set_Control_Enable(hDlg, IDD_EXPORTER_OUTPUT_TRIM_CURVES, !export_options->nurbs_as_polygons && export_options->ena_nurbs_patches);

		Import_IO_Set_Control_Enable(hDlg, IDD_EXPORTER_OUTPUT_SURFTXTR_LAYERS, export_options->ena_materials);
		Import_IO_Set_Control_Enable(hDlg, IDD_EXPORTER_OUTPUT_TXTR_PROJ_INFO, export_options->ena_instance_attributes);

		return(TRUE);
	case WM_NOTIFY:
    		switch (((NMHDR FAR *)  (LPARAM) lParam)->code) {
		case PSN_KILLACTIVE:
			// The page is either losing the activation, or the user pressed OK.
           		// OKINO_SetWindowLong(hDlg, DWLP_MSGRESULT, FALSE);	// No error
			break;
                case PSN_APPLY:
			// The Apply button was pressed, or the OK button was pressed
			// and this message is being sent to all panels to have it send
			// the control values to the toolkit.
           		OKINO_SetWindowLong(hDlg, DWLP_MSGRESULT, PSNRET_INVALID);	// Error initially

			/* Only update the UI if the user clicked the 'Apply' button and not the 'Ok' button */
			lppsn = (LPPSHNOTIFY)  (LPARAM) lParam;
			user_clicked_apply_button = !lppsn->lParam;
			if (!user_clicked_apply_button)
				property_sheet_return_value = IDOK;

           		OKINO_SetWindowLong(hDlg, DWLP_MSGRESULT, PSNRET_NOERROR);
			return(TRUE);
		case PSN_QUERYCANCEL:
			// The user has pressed CANCEL. Return FALSE to allow canceling, or TRUE to prevent it
           		OKINO_SetWindowLong(hDlg, DWLP_MSGRESULT, FALSE);
			property_sheet_return_value = IDCANCEL;
			return(FALSE);
		case PSN_RESET:
			// User has pressed CANCEL and property sheet is about to be destroyed
           		OKINO_SetWindowLong(hDlg, DWLP_MSGRESULT, FALSE);
			property_sheet_return_value = IDCANCEL;
			break;
		case PSN_SETACTIVE:
			// Property sheet is about to be activated. We set up the values of the controls here

			/* Enable the 'Apply' button */
			SendMessage(GetParent(hDlg), PSM_CHANGED, (WPARAM) hDlg, 0L);

			// RCL, we don't want to do this here since it will reset all the type-in parameters!
			// SendMessage(hDlg, WM_UPDATE_DIALOG_PARAMETERS, 0, 0L);

			/* Update the enable state of the controls */
			SendMessage(hDlg, WM_UPDATE_DIALOG_ENABLES, 0, 0L);

			break;
                case PSN_HELP:
			// The Help button was pressed
			Import_IO_DisplayHelpTopic(hDlg, plugin_directory, HELP_FILENAME, "main");
			break;
		default:
			return FALSE;
	    	}
		break;
	case WM_COMMAND:
		cmd_id = LOWORD(wParam);
		switch (cmd_id) {
			case IDD_BUTTON_ABOUT:
				DialogBox(hDLLinstance, "AboutBox", parent_hwnd, (DLGPROC) AboutDlgProc);
				return(TRUE);
			case IDD_BUTTON_RESET:
				if (IDYES == MessageBox(hDlg, "This will reset the values of this dialog box to their power-up defaults.\n\nAre you sure you want to do this?", "Confirmation Request", MB_APPLMODAL | MB_YESNO | MB_DEFBUTTON1 | MB_ICONQUESTION)) {
					export_options->options_initialized = FALSE;
					NI_Exporter_InitExportOptionVariables();
					SendMessage(hDlg, WM_UPDATE_DIALOG_PARAMETERS, 0, 0L);
				}
				return(TRUE);

			case IDD_EXPORTER_ENA_MESHDATA:
				export_options->ena_mesh_data = !export_options->ena_mesh_data;
				CheckDlgButton(hDlg, IDD_EXPORTER_ENA_MESHDATA, export_options->ena_mesh_data);
				/* Update the enable state of the controls */
				SendMessage(hDlg, WM_UPDATE_DIALOG_ENABLES, 0, 0L);
				return(TRUE);
			case IDD_EXPORTER_ENA_VERTEX_NORMALS:
				export_options->ena_vertex_normals = !export_options->ena_vertex_normals;
				CheckDlgButton(hDlg, IDD_EXPORTER_ENA_VERTEX_NORMALS, export_options->ena_vertex_normals);
				/* Update the enable state of the controls */
				SendMessage(hDlg, WM_UPDATE_DIALOG_ENABLES, 0, 0L);
				return(TRUE);
			case IDD_EXPORTER_ENA_TEXTURE_COORDS:
				export_options->ena_texture_coords = !export_options->ena_texture_coords;
				CheckDlgButton(hDlg, IDD_EXPORTER_ENA_TEXTURE_COORDS, export_options->ena_texture_coords);
				return(TRUE);
			case IDD_EXPORTER_OUTPUT_VERTEX_COLORS:
				export_options->output_vertex_colors = !export_options->output_vertex_colors;
				CheckDlgButton(hDlg, IDD_EXPORTER_OUTPUT_VERTEX_COLORS, export_options->output_vertex_colors);
				return(TRUE);
			case IDD_EXPORTER_OUTPUT_UV_TANGENTS:
				export_options->ena_uv_tangents = !export_options->ena_uv_tangents;
				CheckDlgButton(hDlg, IDD_EXPORTER_OUTPUT_UV_TANGENTS, 	export_options->ena_uv_tangents);
				return(TRUE);

			case IDD_EXPORTER_ENA_NURBS_PATCHES:
				export_options->ena_nurbs_patches = !export_options->ena_nurbs_patches;
				CheckDlgButton(hDlg, IDD_EXPORTER_ENA_NURBS_PATCHES, export_options->ena_nurbs_patches);
				/* Update the enable state of the controls */
				SendMessage(hDlg, WM_UPDATE_DIALOG_ENABLES, 0, 0L);
				return(TRUE);
			case IDD_EXPORTER_NURBS_AS_POLYGONS:
				export_options->nurbs_as_polygons = !export_options->nurbs_as_polygons;
				CheckDlgButton(hDlg, IDD_EXPORTER_NURBS_AS_POLYGONS, 	export_options->nurbs_as_polygons);
				/* Update the enable state of the controls */
				SendMessage(hDlg, WM_UPDATE_DIALOG_ENABLES, 0, 0L);
				return(TRUE);
			case IDD_EXPORTER_OUTPUT_TRIM_CURVES:
				export_options->ena_trim_curves = !export_options->ena_trim_curves;
				CheckDlgButton(hDlg, IDD_EXPORTER_OUTPUT_TRIM_CURVES, 	export_options->ena_trim_curves);
				return(TRUE);


			case IDD_EXPORTER_OUTPUT_MATERIALS:
				export_options->ena_materials = !export_options->ena_materials;
				CheckDlgButton(hDlg, IDD_EXPORTER_OUTPUT_MATERIALS, export_options->ena_materials);
				/* Update the enable state of the controls */
				SendMessage(hDlg, WM_UPDATE_DIALOG_ENABLES, 0, 0L);
				return(TRUE);

			case IDD_EXPORTER_OUTPUT_MESH_SKINNING_WEIGHTS:
				export_options->ena_mesh_skinning_weights = !export_options->ena_mesh_skinning_weights;
				CheckDlgButton(hDlg, IDD_EXPORTER_OUTPUT_MESH_SKINNING_WEIGHTS,	export_options->ena_mesh_skinning_weights);
				return(TRUE);

			case IDD_EXPORTER_OUTPUT_SURFTXTR_LAYERS:
				export_options->ena_surftxtr_layers = !export_options->ena_surftxtr_layers;
				CheckDlgButton(hDlg, IDD_EXPORTER_OUTPUT_SURFTXTR_LAYERS, 	export_options->ena_surftxtr_layers);
				return(TRUE);

			case IDD_EXPORTER_ENA_LINE_SETS:
				export_options->ena_lines = !export_options->ena_lines;
				CheckDlgButton(hDlg, IDD_EXPORTER_ENA_LINE_SETS, export_options->ena_lines);
				return(TRUE);
			case IDD_EXPORTER_ENA_POINT_SETS:
				export_options->ena_pointsets = !export_options->ena_pointsets;
				CheckDlgButton(hDlg, IDD_EXPORTER_ENA_POINT_SETS, export_options->ena_pointsets);
				return(TRUE);

			default:
				break;
		}
	}
	return (FALSE);
}

/* Enables Panel # 2 */

	static INT_PTR CALLBACK 
EnablesPanel2_Page_DlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static PROPSHEETPAGE *ps;
	LPPSHNOTIFY	lppsn;
	short		cmd_id, user_clicked_apply_button;

	switch (msg) {
	case WM_INITDIALOG:
		// Save the PROPSHEETPAGE information.
		ps = (PROPSHEETPAGE *)  (LPARAM) lParam;

		SendMessage(hDlg, WM_UPDATE_DIALOG_PARAMETERS, 0, 0L);
		return (TRUE);
	case WM_UPDATE_DIALOG_PARAMETERS:
		CheckDlgButton(hDlg, IDD_EXPORTER_OUTPUT_INST_ATTR, 		export_options->ena_instance_attributes);
		CheckDlgButton(hDlg, IDD_EXPORTER_OUTPUT_TXTR_PROJ_INFO,	export_options->ena_texture_proj_info);

		CheckDlgButton(hDlg, IDD_EXPORTER_ENA_CAMERAS, export_options->ena_cameras);
		CheckDlgButton(hDlg, IDD_EXPORTER_ENA_LIGHTS, export_options->ena_lights);

		CheckDlgButton(hDlg, IDD_EXPORTER_ENA_HIERARCHY, export_options->ena_hierarchy);
		CheckDlgButton(hDlg, IDD_EXPORTER_ENA_INSTANCING, export_options->ena_instancing);

		CheckDlgButton(hDlg, IDD_EXPORTER_OUTPUT_ENVIRON_INFO, 	export_options->ena_env_info);
		CheckDlgButton(hDlg, IDD_EXPORTER_OUTPUT_BACKGROUND_INFO,	export_options->ena_background_info);

		CheckDlgButton(hDlg, IDD_EXPORTER_ENA_BICUBIC_PATCHES, export_options->ena_bicubic_patches);

		CheckDlgButton(hDlg, IDD_EXPORTER_OUTPUT_TXTR_DEFNS, 	export_options->ena_texture_definitions);

		/* Update the enable state of the controls */
		SendMessage(hDlg, WM_UPDATE_DIALOG_ENABLES, 0, 0L);

		return (TRUE);
	case WM_UPDATE_DIALOG_ENABLES:
		Import_IO_Set_Control_Enable(hDlg, IDD_EXPORTER_ENA_HIERARCHY, TRUE);
		return(TRUE);
	case WM_NOTIFY:
    		switch (((NMHDR FAR *)  (LPARAM) lParam)->code) {
		case PSN_KILLACTIVE:
			// The page is either losing the activation, or the user pressed OK.
           		// OKINO_SetWindowLong(hDlg, DWLP_MSGRESULT, FALSE);	// No error
			break;
                case PSN_APPLY:
			// The Apply button was pressed, or the OK button was pressed
			// and this message is being sent to all panels to have it send
			// the control values to the toolkit.
           		OKINO_SetWindowLong(hDlg, DWLP_MSGRESULT, PSNRET_INVALID);	// Error initially

			/* Only update the UI if the user clicked the 'Apply' button and not the 'Ok' button */
			lppsn = (LPPSHNOTIFY)  (LPARAM) lParam;
			user_clicked_apply_button = !lppsn->lParam;
			if (!user_clicked_apply_button)
				property_sheet_return_value = IDOK;

           		OKINO_SetWindowLong(hDlg, DWLP_MSGRESULT, PSNRET_NOERROR);
			return(TRUE);
		case PSN_QUERYCANCEL:
			// The user has pressed CANCEL. Return FALSE to allow canceling, or TRUE to prevent it
           		OKINO_SetWindowLong(hDlg, DWLP_MSGRESULT, FALSE);
			property_sheet_return_value = IDCANCEL;
			return(FALSE);
		case PSN_RESET:
			// User has pressed CANCEL and property sheet is about to be destroyed
           		OKINO_SetWindowLong(hDlg, DWLP_MSGRESULT, FALSE);
			property_sheet_return_value = IDCANCEL;
			break;
		case PSN_SETACTIVE:
			// Property sheet is about to be activated. We set up the values of the controls here
			if (export_options->ena_object_animation) {
				export_options->ena_hierarchy = TRUE;
				CheckDlgButton(hDlg, IDD_EXPORTER_ENA_HIERARCHY, export_options->ena_hierarchy);
			}

			/* Enable the 'Apply' button */
			SendMessage(GetParent(hDlg), PSM_CHANGED, (WPARAM) hDlg, 0L);

			// RCL, we don't want to do this here since it will reset all the type-in parameters!
			// SendMessage(hDlg, WM_UPDATE_DIALOG_PARAMETERS, 0, 0L);

			/* Update the enable state of the controls */
			SendMessage(hDlg, WM_UPDATE_DIALOG_ENABLES, 0, 0L);

			break;
                case PSN_HELP:
			// The Help button was pressed
			Import_IO_DisplayHelpTopic(hDlg, plugin_directory, HELP_FILENAME, "main");
			break;
		default:
			return FALSE;
	    	}
		break;
	case WM_COMMAND:
		cmd_id = LOWORD(wParam);
		switch (cmd_id) {
			case IDD_BUTTON_ABOUT:
				DialogBox(hDLLinstance, "AboutBox", parent_hwnd, (DLGPROC) AboutDlgProc);
				return(TRUE);
			case IDD_BUTTON_RESET:
				if (IDYES == MessageBox(hDlg, "This will reset the values of this dialog box to their power-up defaults.\n\nAre you sure you want to do this?", "Confirmation Request", MB_APPLMODAL | MB_YESNO | MB_DEFBUTTON1 | MB_ICONQUESTION)) {
					export_options->options_initialized = FALSE;
					NI_Exporter_InitExportOptionVariables();
					SendMessage(hDlg, WM_UPDATE_DIALOG_PARAMETERS, 0, 0L);
				}
				return(TRUE);

			case IDD_EXPORTER_OUTPUT_INST_ATTR:
				export_options->ena_instance_attributes = !export_options->ena_instance_attributes;
				CheckDlgButton(hDlg, IDD_EXPORTER_OUTPUT_INST_ATTR, 		export_options->ena_instance_attributes);
				/* Update the enable state of the controls */
				SendMessage(hDlg, WM_UPDATE_DIALOG_ENABLES, 0, 0L);
				return(TRUE);
			case IDD_EXPORTER_OUTPUT_TXTR_PROJ_INFO:
				export_options->ena_texture_proj_info = !export_options->ena_texture_proj_info;
				CheckDlgButton(hDlg, IDD_EXPORTER_OUTPUT_TXTR_PROJ_INFO,	export_options->ena_texture_proj_info);
				return(TRUE);

			case IDD_EXPORTER_ENA_CAMERAS:
				export_options->ena_cameras = !export_options->ena_cameras;
				CheckDlgButton(hDlg, IDD_EXPORTER_ENA_CAMERAS, export_options->ena_cameras);
				/* Update the enable state of the controls */
				SendMessage(hDlg, WM_UPDATE_DIALOG_ENABLES, 0, 0L);
				return(TRUE);
			case IDD_EXPORTER_ENA_LIGHTS:
				export_options->ena_lights = !export_options->ena_lights;
				CheckDlgButton(hDlg, IDD_EXPORTER_ENA_LIGHTS, export_options->ena_lights);
				return(TRUE);

			case IDD_EXPORTER_ENA_HIERARCHY:
				export_options->ena_hierarchy = !export_options->ena_hierarchy;
				CheckDlgButton(hDlg, IDD_EXPORTER_ENA_HIERARCHY, export_options->ena_hierarchy);
				/* Update the enable state of the controls */
				SendMessage(hDlg, WM_UPDATE_DIALOG_ENABLES, 0, 0L);
				return(TRUE);
			case IDD_EXPORTER_ENA_INSTANCING:
				export_options->ena_instancing = !export_options->ena_instancing;
				CheckDlgButton(hDlg, IDD_EXPORTER_ENA_INSTANCING, export_options->ena_instancing);
				return(TRUE);

			case IDD_EXPORTER_OUTPUT_ENVIRON_INFO:
				export_options->ena_env_info = !export_options->ena_env_info;
				CheckDlgButton(hDlg, IDD_EXPORTER_OUTPUT_ENVIRON_INFO, 	export_options->ena_env_info);
				return(TRUE);
			case IDD_EXPORTER_OUTPUT_BACKGROUND_INFO:
				export_options->ena_background_info = !export_options->ena_background_info;
				CheckDlgButton(hDlg, IDD_EXPORTER_OUTPUT_BACKGROUND_INFO,	export_options->ena_background_info);
				return(TRUE);		
			case IDD_EXPORTER_OUTPUT_TXTR_DEFNS:
				export_options->ena_texture_definitions = !export_options->ena_texture_definitions;
				CheckDlgButton(hDlg, IDD_EXPORTER_OUTPUT_TXTR_DEFNS, 	export_options->ena_texture_definitions);
				return(TRUE);		
			case IDD_EXPORTER_ENA_BICUBIC_PATCHES:
				export_options->ena_bicubic_patches = !export_options->ena_bicubic_patches;
				CheckDlgButton(hDlg, IDD_EXPORTER_ENA_BICUBIC_PATCHES, export_options->ena_bicubic_patches);
				return(TRUE);

			default:
				break;
		}
	}
	return (FALSE);
}


/* Curves Panel # 3 */

	static INT_PTR CALLBACK 
CurvesPanel_Page_DlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static PROPSHEETPAGE *ps;
	LPPSHNOTIFY	lppsn;
	UINT	wIndex;
	short	cmd_id, count, user_clicked_apply_button;

	switch (msg) {
	case WM_INITDIALOG:
		// Save the PROPSHEETPAGE information.
		ps = (PROPSHEETPAGE *)  (LPARAM) lParam;

		SendMessage(hDlg, WM_UPDATE_DIALOG_PARAMETERS, 0, 0L);
		return (TRUE);
	case WM_UPDATE_DIALOG_PARAMETERS:
		CheckDlgButton(hDlg, IDD_EXPORTER_ENA_3D_CURVES, export_options->ena_independent_3D_nurbs_curves);

		/* Update the renderable NURBS Curve output mode combo */
		SendDlgItemMessage(hDlg, IDD_EXPORTER_RENDERABLE_NURBS_COMBO, CB_RESETCONTENT, 0, 0L);
		SendDlgItemMessage(hDlg, IDD_EXPORTER_RENDERABLE_NURBS_COMBO, WM_SETREDRAW, FALSE, 0L);
		count = 0;
		while (renderable_nurbs_curve_output_modes_struct[count].desc) {
			SendDlgItemMessage(hDlg, IDD_EXPORTER_RENDERABLE_NURBS_COMBO, CB_ADDSTRING, (WPARAM) NULL, (LPARAM)(LPSTR) renderable_nurbs_curve_output_modes_struct[count].desc);
			++count;
		}
		SendDlgItemMessage(hDlg, IDD_EXPORTER_RENDERABLE_NURBS_COMBO, WM_SETREDRAW, TRUE, 0L);
		SendDlgItemMessage(hDlg, IDD_EXPORTER_RENDERABLE_NURBS_COMBO, CB_SETCURSEL, export_options->RenderableNURBSCurveOutputMode, 0L);

		/* Update the non-renderable NURBS Curve output mode combo */
		SendDlgItemMessage(hDlg, IDD_EXPORTER_NON_RENDERABLE_NURBS_COMBO, CB_RESETCONTENT, 0, 0L);
		SendDlgItemMessage(hDlg, IDD_EXPORTER_NON_RENDERABLE_NURBS_COMBO, WM_SETREDRAW, FALSE, 0L);
		count = 0;
		while (non_renderable_nurbs_curve_output_modes_struct[count].desc) {
			SendDlgItemMessage(hDlg, IDD_EXPORTER_NON_RENDERABLE_NURBS_COMBO, CB_ADDSTRING, (WPARAM) NULL, (LPARAM)(LPSTR) non_renderable_nurbs_curve_output_modes_struct[count].desc);
			++count;
		}
		SendDlgItemMessage(hDlg, IDD_EXPORTER_NON_RENDERABLE_NURBS_COMBO, WM_SETREDRAW, TRUE, 0L);
		SendDlgItemMessage(hDlg, IDD_EXPORTER_NON_RENDERABLE_NURBS_COMBO, CB_SETCURSEL, export_options->NonRenderableNURBSCurveOutputMode, 0L);


		CheckDlgButton(hDlg, IDD_EXPORTER_ENA_3D_SPLINE_SHAPES, export_options->ena_independent_3D_Spline_Shapes);

		/* Update the renderable spline shape output mode combo */
		SendDlgItemMessage(hDlg, IDD_EXPORTER_RENDERABLE_SPLINESHAPE_COMBO, CB_RESETCONTENT, 0, 0L);
		SendDlgItemMessage(hDlg, IDD_EXPORTER_RENDERABLE_SPLINESHAPE_COMBO, WM_SETREDRAW, FALSE, 0L);
		count = 0;
		while (renderable_spline_shape_output_modes_struct[count].desc) {
			SendDlgItemMessage(hDlg, IDD_EXPORTER_RENDERABLE_SPLINESHAPE_COMBO, CB_ADDSTRING, (WPARAM) NULL, (LPARAM)(LPSTR) renderable_spline_shape_output_modes_struct[count].desc);
			++count;
		}
		SendDlgItemMessage(hDlg, IDD_EXPORTER_RENDERABLE_SPLINESHAPE_COMBO, WM_SETREDRAW, TRUE, 0L);
		SendDlgItemMessage(hDlg, IDD_EXPORTER_RENDERABLE_SPLINESHAPE_COMBO, CB_SETCURSEL, export_options->RenderableSplineShapeOutputMode, 0L);

		/* Update the non-renderable spline shape output mode combo */
		SendDlgItemMessage(hDlg, IDD_EXPORTER_NON_RENDERABLE_SPLINESHAPE_COMBO, CB_RESETCONTENT, 0, 0L);
		SendDlgItemMessage(hDlg, IDD_EXPORTER_NON_RENDERABLE_SPLINESHAPE_COMBO, WM_SETREDRAW, FALSE, 0L);
		count = 0;
		while (non_renderable_spline_shape_output_modes_struct[count].desc) {
			SendDlgItemMessage(hDlg, IDD_EXPORTER_NON_RENDERABLE_SPLINESHAPE_COMBO, CB_ADDSTRING, (WPARAM) NULL, (LPARAM)(LPSTR) non_renderable_spline_shape_output_modes_struct[count].desc);
			++count;
		}
		SendDlgItemMessage(hDlg, IDD_EXPORTER_NON_RENDERABLE_SPLINESHAPE_COMBO, WM_SETREDRAW, TRUE, 0L);
		SendDlgItemMessage(hDlg, IDD_EXPORTER_NON_RENDERABLE_SPLINESHAPE_COMBO, CB_SETCURSEL, export_options->NonRenderableSplineShapeOutputMode, 0L);

		/* Update the spline conversion type combo box */
		SendDlgItemMessage(hDlg, IDD_EXPORTER_SPLINE_TO_SPLINE_CONVERSION_TYPE_COMBO, CB_RESETCONTENT, 0, 0L);
		SendDlgItemMessage(hDlg, IDD_EXPORTER_SPLINE_TO_SPLINE_CONVERSION_TYPE_COMBO, WM_SETREDRAW, FALSE, 0L);
		count = 0;
		while (spline_types_struct[count].spline_name != (char *) NULL) {
			SendDlgItemMessage(hDlg, IDD_EXPORTER_SPLINE_TO_SPLINE_CONVERSION_TYPE_COMBO, CB_ADDSTRING, (WPARAM) NULL, (LPARAM)(LPSTR) spline_types_struct[count].spline_name);
			++count;
		}
		SendDlgItemMessage(hDlg, IDD_EXPORTER_SPLINE_TO_SPLINE_CONVERSION_TYPE_COMBO, WM_SETREDRAW, TRUE, 0L);
		if (export_options->spline_to_spline_conversion_type == NULL || !strlen(export_options->spline_to_spline_conversion_type))
			SendDlgItemMessage(hDlg, IDD_EXPORTER_SPLINE_TO_SPLINE_CONVERSION_TYPE_COMBO, CB_SETCURSEL, 0, 0L);
		else {
			count = 1;
			while (spline_types_struct[count].spline_name != (char *) NULL) {
				if (!strcmp(spline_types_struct[count].spline_token, export_options->spline_to_spline_conversion_type))
					SendDlgItemMessage(hDlg, IDD_EXPORTER_SPLINE_TO_SPLINE_CONVERSION_TYPE_COMBO, CB_SETCURSEL, count, 0L);
				++count;
			}
		}


		/* Update the enable state of the controls */
		SendMessage(hDlg, WM_UPDATE_DIALOG_ENABLES, 0, 0L);

		return (TRUE);
	case WM_UPDATE_DIALOG_ENABLES:
		Import_IO_Set_Control_Enable(hDlg, IDD_EXPORTER_RENDERABLE_NURBS_TXT, export_options->ena_independent_3D_nurbs_curves);
		Import_IO_Set_Control_Enable(hDlg, IDD_EXPORTER_RENDERABLE_NURBS_COMBO, export_options->ena_independent_3D_nurbs_curves);
		Import_IO_Set_Control_Enable(hDlg, IDD_EXPORTER_NON_RENDERABLE_NURBS_TXT, export_options->ena_independent_3D_nurbs_curves);
		Import_IO_Set_Control_Enable(hDlg, IDD_EXPORTER_NON_RENDERABLE_NURBS_COMBO, export_options->ena_independent_3D_nurbs_curves);

		Import_IO_Set_Control_Enable(hDlg, IDD_EXPORTER_RENDERABLE_SPLINESHAPE_TXT, export_options->ena_independent_3D_Spline_Shapes);
		Import_IO_Set_Control_Enable(hDlg, IDD_EXPORTER_RENDERABLE_SPLINESHAPE_COMBO, export_options->ena_independent_3D_Spline_Shapes);
		Import_IO_Set_Control_Enable(hDlg, IDD_EXPORTER_NON_RENDERABLE_SPLINESHAPE_TXT, export_options->ena_independent_3D_Spline_Shapes);
		Import_IO_Set_Control_Enable(hDlg, IDD_EXPORTER_NON_RENDERABLE_SPLINESHAPE_COMBO, export_options->ena_independent_3D_Spline_Shapes);
		Import_IO_Set_Control_Enable(hDlg, IDD_EXPORTER_SPLINE_TO_SPLINE_CURVES_TXT, export_options->ena_independent_3D_Spline_Shapes || export_options->ena_independent_3D_nurbs_curves);
		Import_IO_Set_Control_Enable(hDlg, IDD_EXPORTER_SPLINE_TO_SPLINE_CONVERSION_TYPE_COMBO, export_options->ena_independent_3D_Spline_Shapes || export_options->ena_independent_3D_nurbs_curves);

		return(TRUE);
	case WM_NOTIFY:
    		switch (((NMHDR FAR *)  (LPARAM) lParam)->code) {
		case PSN_KILLACTIVE:
			// The page is either losing the activation, or the user pressed OK.
           		// OKINO_SetWindowLong(hDlg, DWLP_MSGRESULT, FALSE);	// No error
			break;
                case PSN_APPLY:
			// The Apply button was pressed, or the OK button was pressed
			// and this message is being sent to all panels to have it send
			// the control values to the toolkit.
           		OKINO_SetWindowLong(hDlg, DWLP_MSGRESULT, PSNRET_INVALID);	// Error initially

			/* Only update the UI if the user clicked the 'Apply' button and not the 'Ok' button */
			lppsn = (LPPSHNOTIFY)  (LPARAM) lParam;
			user_clicked_apply_button = !lppsn->lParam;
			if (!user_clicked_apply_button)
				property_sheet_return_value = IDOK;

           		OKINO_SetWindowLong(hDlg, DWLP_MSGRESULT, PSNRET_NOERROR);
			return(TRUE);
		case PSN_QUERYCANCEL:
			// The user has pressed CANCEL. Return FALSE to allow canceling, or TRUE to prevent it
           		OKINO_SetWindowLong(hDlg, DWLP_MSGRESULT, FALSE);
			property_sheet_return_value = IDCANCEL;
			return(FALSE);
		case PSN_RESET:
			// User has pressed CANCEL and property sheet is about to be destroyed
           		OKINO_SetWindowLong(hDlg, DWLP_MSGRESULT, FALSE);
			property_sheet_return_value = IDCANCEL;
			break;
		case PSN_SETACTIVE:
			// Property sheet is about to be activated. We set up the values of the controls here

			/* Enable the 'Apply' button */
			SendMessage(GetParent(hDlg), PSM_CHANGED, (WPARAM) hDlg, 0L);

			// RCL, we don't want to do this here since it will reset all the type-in parameters!
			// SendMessage(hDlg, WM_UPDATE_DIALOG_PARAMETERS, 0, 0L);

			/* Update the enable state of the controls */
			SendMessage(hDlg, WM_UPDATE_DIALOG_ENABLES, 0, 0L);

			break;
                case PSN_HELP:
			// The Help button was pressed
			Import_IO_DisplayHelpTopic(hDlg, plugin_directory, HELP_FILENAME, "main");
			break;
		default:
			return FALSE;
	    	}
		break;
	case WM_COMMAND:
		cmd_id = LOWORD(wParam);
		switch (cmd_id) {
			case IDD_BUTTON_ABOUT:
				DialogBox(hDLLinstance, "AboutBox", parent_hwnd, (DLGPROC) AboutDlgProc);
				return(TRUE);
			case IDD_BUTTON_RESET:
				if (IDYES == MessageBox(hDlg, "This will reset the values of this dialog box to their power-up defaults.\n\nAre you sure you want to do this?", "Confirmation Request", MB_APPLMODAL | MB_YESNO | MB_DEFBUTTON1 | MB_ICONQUESTION)) {
					export_options->options_initialized = FALSE;
					NI_Exporter_InitExportOptionVariables();
					SendMessage(hDlg, WM_UPDATE_DIALOG_PARAMETERS, 0, 0L);
				}
				return(TRUE);

			case IDD_EXPORTER_ENA_3D_CURVES:
				export_options->ena_independent_3D_nurbs_curves = !export_options->ena_independent_3D_nurbs_curves;
				CheckDlgButton(hDlg, IDD_EXPORTER_ENA_3D_CURVES, export_options->ena_independent_3D_nurbs_curves);
				/* Update the enable state of the controls */
				SendMessage(hDlg, WM_UPDATE_DIALOG_ENABLES, 0, 0L);
				return(TRUE);
			case IDD_EXPORTER_RENDERABLE_NURBS_COMBO:
				/* Update the renderable NURBS Curve output mode combo */
				if (GET_WM_COMMAND_CMD(wParam, lParam) == CBN_SELCHANGE) {
				 	wIndex = (UINT) SendDlgItemMessage(hDlg, IDD_EXPORTER_RENDERABLE_NURBS_COMBO, CB_GETCURSEL, 0, 0L);
					if (wIndex >= 0)
						export_options->RenderableNURBSCurveOutputMode = wIndex;
				}
				return (TRUE);
			case IDD_EXPORTER_NON_RENDERABLE_NURBS_COMBO:
				/* Update the non-renderable NURBS Curve output mode combo */

				if (GET_WM_COMMAND_CMD(wParam, lParam) == CBN_SELCHANGE) {
				 	wIndex = (UINT) SendDlgItemMessage(hDlg, IDD_EXPORTER_NON_RENDERABLE_NURBS_COMBO, CB_GETCURSEL, 0, 0L);
					if (wIndex >= 0)
						export_options->NonRenderableNURBSCurveOutputMode = wIndex;
				}
				return (TRUE);


			case IDD_EXPORTER_ENA_3D_SPLINE_SHAPES:
				export_options->ena_independent_3D_Spline_Shapes = !export_options->ena_independent_3D_Spline_Shapes;
				CheckDlgButton(hDlg, IDD_EXPORTER_ENA_3D_SPLINE_SHAPES, export_options->ena_independent_3D_Spline_Shapes);
				/* Update the enable state of the controls */
				SendMessage(hDlg, WM_UPDATE_DIALOG_ENABLES, 0, 0L);
				return(TRUE);
			case IDD_EXPORTER_RENDERABLE_SPLINESHAPE_COMBO:
				/* Update the renderable spline shape output mode combo */
				if (GET_WM_COMMAND_CMD(wParam, lparam) == CBN_SELCHANGE) {
				 	wIndex = (UINT) SendDlgItemMessage(hDlg, IDD_EXPORTER_RENDERABLE_SPLINESHAPE_COMBO, CB_GETCURSEL, 0, 0L);
					if (wIndex >= 0)
						export_options->RenderableSplineShapeOutputMode = wIndex;
				}
				return (TRUE);
			case IDD_EXPORTER_NON_RENDERABLE_SPLINESHAPE_COMBO:
				/* Update the non-renderable spline shape output mode combo */

				if (GET_WM_COMMAND_CMD(wParam, lparam) == CBN_SELCHANGE) {
				 	wIndex = (UINT) SendDlgItemMessage(hDlg, IDD_EXPORTER_NON_RENDERABLE_SPLINESHAPE_COMBO, CB_GETCURSEL, 0, 0L);
					if (wIndex >= 0)
						export_options->NonRenderableSplineShapeOutputMode = wIndex;
				}
				return (TRUE);

			case IDD_EXPORTER_SPLINE_TO_SPLINE_CONVERSION_TYPE_COMBO:
				if (GET_WM_COMMAND_CMD(wParam, lparam) == CBN_SELCHANGE) {
				 	wIndex = (UINT) SendDlgItemMessage(hDlg, IDD_EXPORTER_SPLINE_TO_SPLINE_CONVERSION_TYPE_COMBO, CB_GETCURSEL, 0, 0L);
					if (!wIndex)
						export_options->spline_to_spline_conversion_type = NULL;
					else if (wIndex >= 0)
						export_options->spline_to_spline_conversion_type = Ni_Inquire_Token(spline_types_struct[wIndex].spline_token);
				}
				return (TRUE);

			default:
				break;
		}
	}
	return (FALSE);
}

/* Mesh Processing Panel # 4 */

	static INT_PTR CALLBACK 
MeshProcessingPanel_Page_DlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static PROPSHEETPAGE *ps;
	LPPSHNOTIFY	lppsn;
	char		temp_buf[300];
	long		code;
	short		cmd_id, user_clicked_apply_button;
	float		temp_float;

	switch (msg) {
	case WM_INITDIALOG:
		// Save the PROPSHEETPAGE information.
		ps = (PROPSHEETPAGE *)  (LPARAM) lParam;

		SendMessage(hDlg, WM_UPDATE_DIALOG_PARAMETERS, 0, 0L);
		return (TRUE);
	case WM_UPDATE_DIALOG_PARAMETERS:
		CheckDlgButton(hDlg, IDD_EXPORTER_MESH_PROCESS_USE_SHARED_INDEX_ARRAY,		export_options->mesh_processing_use_shared_index_array);
		CheckDlgButton(hDlg, IDD_EXPORTER_MESH_PROCESS_NEG_INDICES_ALLOWED, 		export_options->mesh_processing_negative_indices_allowed);
		CheckDlgButton(hDlg, IDD_EXPORTER_MESH_PROCESS_HOLES_ALLOWED,			export_options->mesh_processing_holes_allowed);
		CheckDlgButton(hDlg, IDD_EXPORTER_MESH_PROCESS_ALWAYS_ADD_VERTE_NORMALS,	export_options->mesh_processing_always_add_vertex_normals);
		CheckDlgButton(hDlg, IDD_EXPORTER_MESH_PROCESS_OPTIMIZE_TXTR_COORD_LIST,	export_options->mesh_processing_optimize_texture_coordinate_list);
		CheckDlgButton(hDlg, IDD_EXPORTER_MESH_PROCESS_DUPL_VERT_FOR_UV_TEXTURE_MAPPING,export_options->mesh_processing_duplicate_vertices_for_uv_texture_mapping);
		CheckDlgButton(hDlg, IDD_EXPORTER_MESH_PROCESS_EXPLODE_MESHES_BY_MAT_NAME,	export_options->mesh_processing_explode_meshes_by_material_name);
		CheckDlgButton(hDlg, IDD_EXPORTER_MESH_PROCESS_TRIANGLES_ONLY, 			export_options->mesh_processing_want_triangles_only);
		CheckDlgButton(hDlg, IDD_EXPORTER_MESH_PROCESS_SORT_POLYS_BY_MATERIAL_NAME, 	export_options->mesh_processing_sort_polygons_by_material_assignments);
		CheckDlgButton(hDlg, IDD_EXPORTER_MESH_PROCESS_CONVEX_ONLY, 			export_options->mesh_processing_want_convex_only);
		CheckDlgButton(hDlg, IDD_EXPORTER_MESH_PROCESS_QUAD_ONLY, 			export_options->mesh_processing_want_quads_only);
		CheckDlgButton(hDlg, IDD_EXPORTER_MESH_PROCESS_TRANSFORM_TO_WORLDSPACE, 	export_options->mesh_processing_transform_to_worldspace);
		CheckDlgButton(hDlg, IDD_EXPORTER_ENA_POLYGON_REDUCTION, 			export_options->polygon_reduction_ena);
		CheckDlgButton(hDlg, IDD_EXPORTER_MESH_PROCESS_PLANAR_ONLY,			export_options->mesh_processing_want_planar_polygons_only);

	 	Import_IO_NdFloat_To_Dialog(hDlg, IDD_EXPORTER_MESH_PROCESS_PLANAR_ONLY_TOLERANCE, export_options->mesh_processing_want_planar_polygons_only_tolerance);

		/* Update the enable state of the controls */
		SendMessage(hDlg, WM_UPDATE_DIALOG_ENABLES, 0, 0L);

		return (TRUE);
	case WM_UPDATE_DIALOG_ENABLES:
		Import_IO_Set_Control_Enable(hDlg, IDD_EXPORTER_ENA_POLYGON_REDUCTION_OPTIONS, export_options->polygon_reduction_ena);
		return(TRUE);
	case WM_NOTIFY:
		code = ((NMHDR FAR *)  (LPARAM) lParam)->code;
    		switch (code) {
		case PSN_KILLACTIVE:	// The page is either losing the activation, or the user pressed OK.
                case PSN_APPLY:
			// The Apply button was pressed, or the OK button was pressed
			// and this message is being sent to all panels to have it send
			// the control values to the toolkit.
			if (code == PSN_APPLY)
	           		OKINO_SetWindowLong(hDlg, DWLP_MSGRESULT, PSNRET_INVALID);	// Error initially
			else
	           		OKINO_SetWindowLong(hDlg, DWLP_MSGRESULT, TRUE);	// Error

			Import_IO_Dialog_To_NdFloat(hDlg, IDD_EXPORTER_MESH_PROCESS_PLANAR_ONLY_TOLERANCE, &temp_float);
			if (temp_float < 1e-6) {
				MessageBox(hDlg, "The planar tolerance must be greater than 1e-6.", "Parameter Error", MB_OK | MB_APPLMODAL);
				Import_IO_SetFocus_Edit_Control(hDlg, IDD_EXPORTER_MESH_PROCESS_PLANAR_ONLY_TOLERANCE);
				return(FALSE);
			}
			export_options->mesh_processing_want_planar_polygons_only_tolerance = temp_float;

			/* Get the relative path name */
			GetDlgItemText(hDlg, IDD_EXPORTER_PREPEND_RELATIVE_FILEPATH_EDIT, temp_buf, sizeof(temp_buf));
			if (export_options->specific_relative_filepath_to_prepend)
				free(export_options->specific_relative_filepath_to_prepend);
			export_options->specific_relative_filepath_to_prepend = NULL;
			if (strlen(temp_buf))
				Import_IO_Copy_String(&export_options->specific_relative_filepath_to_prepend, temp_buf);

			/* Get the URL path name */
			GetDlgItemText(hDlg, IDD_EXPORTER_URL_EDIT, temp_buf, sizeof(temp_buf));
			if (export_options->url_string)
				free(export_options->url_string);
			export_options->url_string = NULL;
			if (strlen(temp_buf))
				Import_IO_Copy_String(&export_options->url_string, temp_buf);

			/* Only update the UI if the user clicked the 'Apply' button and not the 'Ok' button */
			if (code == PSN_APPLY) {
				lppsn = (LPPSHNOTIFY)  (LPARAM) lParam;
				user_clicked_apply_button = !lppsn->lParam;
				if (!user_clicked_apply_button)
					property_sheet_return_value = IDOK;
			}

			if (code == PSN_APPLY)
		           	OKINO_SetWindowLong(hDlg, DWLP_MSGRESULT, PSNRET_NOERROR);
			else
		           	OKINO_SetWindowLong(hDlg, DWLP_MSGRESULT, FALSE);	// No error

			return(TRUE);
		case PSN_QUERYCANCEL:
			// The user has pressed CANCEL. Return FALSE to allow canceling, or TRUE to prevent it
           		OKINO_SetWindowLong(hDlg, DWLP_MSGRESULT, FALSE);
			property_sheet_return_value = IDCANCEL;
			return(FALSE);
		case PSN_RESET:
			// User has pressed CANCEL and property sheet is about to be destroyed
           		OKINO_SetWindowLong(hDlg, DWLP_MSGRESULT, FALSE);
			property_sheet_return_value = IDCANCEL;
			break;
		case PSN_SETACTIVE:
			// Property sheet is about to be activated. We set up the values of the controls here

			/* Enable the 'Apply' button */
			SendMessage(GetParent(hDlg), PSM_CHANGED, (WPARAM) hDlg, 0L);

			// RCL, we don't want to do this here since it will reset all the type-in parameters!
			// SendMessage(hDlg, WM_UPDATE_DIALOG_PARAMETERS, 0, 0L);

			/* Update the enable state of the controls */
			SendMessage(hDlg, WM_UPDATE_DIALOG_ENABLES, 0, 0L);

			break;
                case PSN_HELP:
			// The Help button was pressed
			Import_IO_DisplayHelpTopic(hDlg, plugin_directory, HELP_FILENAME, "main");
			break;
		default:
			return FALSE;
	    	}
		break;
	case WM_COMMAND:
		cmd_id = LOWORD(wParam);
		switch (cmd_id) {
			case IDD_BUTTON_ABOUT:
				DialogBox(hDLLinstance, "AboutBox", parent_hwnd, (DLGPROC) AboutDlgProc);
				return(TRUE);
			case IDD_BUTTON_RESET:
				if (IDYES == MessageBox(hDlg, "This will reset the values of this dialog box to their power-up defaults.\n\nAre you sure you want to do this?", "Confirmation Request", MB_APPLMODAL | MB_YESNO | MB_DEFBUTTON1 | MB_ICONQUESTION)) {
					export_options->options_initialized = FALSE;
					NI_Exporter_InitExportOptionVariables();
					SendMessage(hDlg, WM_UPDATE_DIALOG_PARAMETERS, 0, 0L);
				}
				return(TRUE);

			case IDD_EXPORTER_MESH_PROCESS_USE_SHARED_INDEX_ARRAY:
				export_options->mesh_processing_use_shared_index_array = !export_options->mesh_processing_use_shared_index_array;
				CheckDlgButton(hDlg, IDD_EXPORTER_MESH_PROCESS_USE_SHARED_INDEX_ARRAY,	export_options->mesh_processing_use_shared_index_array);
				return(TRUE);
			case IDD_EXPORTER_MESH_PROCESS_NEG_INDICES_ALLOWED:
				export_options->mesh_processing_negative_indices_allowed = !export_options->mesh_processing_negative_indices_allowed;
				CheckDlgButton(hDlg, IDD_EXPORTER_MESH_PROCESS_NEG_INDICES_ALLOWED, 		export_options->mesh_processing_negative_indices_allowed);
				return(TRUE);
			case IDD_EXPORTER_MESH_PROCESS_HOLES_ALLOWED:
				export_options->mesh_processing_holes_allowed = !export_options->mesh_processing_holes_allowed;
				CheckDlgButton(hDlg, IDD_EXPORTER_MESH_PROCESS_HOLES_ALLOWED,		export_options->mesh_processing_holes_allowed);
				return(TRUE);
			case IDD_EXPORTER_MESH_PROCESS_ALWAYS_ADD_VERTE_NORMALS:
				export_options->mesh_processing_always_add_vertex_normals = !export_options->mesh_processing_always_add_vertex_normals;
				CheckDlgButton(hDlg, IDD_EXPORTER_MESH_PROCESS_ALWAYS_ADD_VERTE_NORMALS,	export_options->mesh_processing_always_add_vertex_normals);
				return(TRUE);
			case IDD_EXPORTER_MESH_PROCESS_OPTIMIZE_TXTR_COORD_LIST:
				export_options->mesh_processing_optimize_texture_coordinate_list = !export_options->mesh_processing_optimize_texture_coordinate_list;
				CheckDlgButton(hDlg, IDD_EXPORTER_MESH_PROCESS_OPTIMIZE_TXTR_COORD_LIST,	export_options->mesh_processing_optimize_texture_coordinate_list);
				return(TRUE);
			case IDD_EXPORTER_MESH_PROCESS_DUPL_VERT_FOR_UV_TEXTURE_MAPPING:
				export_options->mesh_processing_duplicate_vertices_for_uv_texture_mapping = !export_options->mesh_processing_duplicate_vertices_for_uv_texture_mapping;
				CheckDlgButton(hDlg, IDD_EXPORTER_MESH_PROCESS_DUPL_VERT_FOR_UV_TEXTURE_MAPPING, 	export_options->mesh_processing_duplicate_vertices_for_uv_texture_mapping);
				return(TRUE);
			case IDD_EXPORTER_MESH_PROCESS_EXPLODE_MESHES_BY_MAT_NAME:
				export_options->mesh_processing_explode_meshes_by_material_name = !export_options->mesh_processing_explode_meshes_by_material_name;
				CheckDlgButton(hDlg, IDD_EXPORTER_MESH_PROCESS_EXPLODE_MESHES_BY_MAT_NAME,	export_options->mesh_processing_explode_meshes_by_material_name);
				return(TRUE);
			case IDD_EXPORTER_MESH_PROCESS_CONVEX_ONLY:
				export_options->mesh_processing_want_convex_only = !export_options->mesh_processing_want_convex_only;
				CheckDlgButton(hDlg, IDD_EXPORTER_MESH_PROCESS_CONVEX_ONLY, 			export_options->mesh_processing_want_convex_only);
				return(TRUE);
			case IDD_EXPORTER_MESH_PROCESS_QUAD_ONLY:
				export_options->mesh_processing_want_quads_only = !export_options->mesh_processing_want_quads_only;
				CheckDlgButton(hDlg, IDD_EXPORTER_MESH_PROCESS_QUAD_ONLY, 			export_options->mesh_processing_want_quads_only);
				return(TRUE);
			case IDD_EXPORTER_MESH_PROCESS_TRIANGLES_ONLY:
				export_options->mesh_processing_want_triangles_only = !export_options->mesh_processing_want_triangles_only;
				CheckDlgButton(hDlg, IDD_EXPORTER_MESH_PROCESS_TRIANGLES_ONLY, 		export_options->mesh_processing_want_triangles_only);
				return(TRUE);
			case IDD_EXPORTER_MESH_PROCESS_PLANAR_ONLY:
				export_options->mesh_processing_want_planar_polygons_only = !export_options->mesh_processing_want_planar_polygons_only;
				CheckDlgButton(hDlg, IDD_EXPORTER_MESH_PROCESS_PLANAR_ONLY, export_options->mesh_processing_want_planar_polygons_only);
				return(TRUE);
			case IDD_EXPORTER_MESH_PROCESS_TRANSFORM_TO_WORLDSPACE:
				export_options->mesh_processing_transform_to_worldspace = !export_options->mesh_processing_transform_to_worldspace;
				CheckDlgButton(hDlg, IDD_EXPORTER_MESH_PROCESS_TRANSFORM_TO_WORLDSPACE, 	export_options->mesh_processing_transform_to_worldspace);
				return(TRUE);
			case IDD_EXPORTER_MESH_PROCESS_SORT_POLYS_BY_MATERIAL_NAME:
				export_options->mesh_processing_sort_polygons_by_material_assignments = !export_options->mesh_processing_sort_polygons_by_material_assignments;
				CheckDlgButton(hDlg, IDD_EXPORTER_MESH_PROCESS_SORT_POLYS_BY_MATERIAL_NAME, export_options->mesh_processing_sort_polygons_by_material_assignments);
				return(TRUE);
			case IDD_EXPORTER_ENA_POLYGON_REDUCTION:
				export_options->polygon_reduction_ena = !export_options->polygon_reduction_ena;
				CheckDlgButton(hDlg, IDD_EXPORTER_ENA_POLYGON_REDUCTION, export_options->polygon_reduction_ena);
				SendMessage(hDlg, WM_UPDATE_DIALOG_ENABLES, 0, 0L);
				return(TRUE);
			case IDD_EXPORTER_ENA_POLYGON_REDUCTION_OPTIONS:
				// Show the polygon reduction options dialog box
				{ char *ptr, ini_filepath[1024];

				// Create our own .ini file for the reduction options. This is
				// because our exporter overwrites its .ini file and hence we cannot store the reduction options in it.
				strcpy(ini_filepath, plugin_directory);
				strcat(ini_filepath, INI_FILENAME);
				ptr = strstr(ini_filepath, ".ini");
				if (ptr) *ptr = '\0';
				strcat(ini_filepath, "_reduction.ini");

				Import_IO_ShowPolygonReductionOptionsDialogBox(hDlg, NULL, NULL, NULL, ini_filepath);
				}
				return(TRUE);
			default:
				break;
		}
	}
	return (FALSE);
}

/* Bitmap Conversion Panel # 5 */
	
	static INT_PTR CALLBACK 
BitmapConversionPanel_Page_DlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static PROPSHEETPAGE *ps;
	LPPSHNOTIFY	lppsn;
	UINT		wIndex;
	short		cmd_id, count, val, user_clicked_apply_button;

	switch (msg) {
	case WM_INITDIALOG:
		// Save the PROPSHEETPAGE information.
		ps = (PROPSHEETPAGE *)  (LPARAM) lParam;

		SendMessage(hDlg, WM_UPDATE_DIALOG_PARAMETERS, 0, 0L);
		return (TRUE);
	case WM_UPDATE_DIALOG_PARAMETERS:
		/* Update the bitmap file formats drop-down boxes */
		SendDlgItemMessage(hDlg, IDD_EXPORTER_BITMAP_FORMAT_COMBO, CB_RESETCONTENT, 0, 0L);
		SendDlgItemMessage(hDlg, IDD_EXPORTER_BITMAP_FORMAT_COMBO, WM_SETREDRAW, FALSE, 0L);
		SendDlgItemMessage(hDlg, IDD_EXPORTER_FILE_EXTENSIONS, CB_RESETCONTENT, 0, 0L);
		SendDlgItemMessage(hDlg, IDD_EXPORTER_FILE_EXTENSIONS, WM_SETREDRAW, FALSE, 0L);
		count = 0;
		while (TRUE) {
			char 		*Nv_FileFormatDescription;
			Nd_Token 	Nv_FileFormatToolkitToken;
			char		*Nv_FileFormatExtension;
			short		Nv_HandlesColorMappedOutput;
			char		*Nv_FileFormatGUID;
			char		buf[1024];

// v4.1.9 (Oct 2004) of the toolkit allows for custom 2D file exporter's to be defined via a GUID, so
// this change will allow for exporters to be chosen via their extension (.tif) or GUID.
#if 00
			Ni_EnumerateSupportedExportBitmapFileFormats(count, &Nv_FileFormatDescription, &Nv_FileFormatToolkitToken, &Nv_FileFormatExtension, &Nv_HandlesColorMappedOutput);
#else
			Ni_EnumerateSupportedExportBitmapFileFormats2(TRUE, count, &Nv_FileFormatDescription, &Nv_FileFormatToolkitToken, &Nv_FileFormatExtension, &Nv_HandlesColorMappedOutput, &Nv_FileFormatGUID);
#endif
			if (!Nv_FileFormatDescription)
				// Break once no more file formats available
				break;

			if (Nv_FileFormatDescription)
				sprintf(buf, "%s (%s)", Nv_FileFormatDescription, Nv_FileFormatExtension);
			else
				strcpy(buf, Nv_FileFormatExtension);
			SendDlgItemMessage(hDlg, IDD_EXPORTER_BITMAP_FORMAT_COMBO, CB_ADDSTRING, (WPARAM) NULL, (LPARAM)(LPSTR) buf);
			SendDlgItemMessage(hDlg, IDD_EXPORTER_FILE_EXTENSIONS, CB_ADDSTRING, (WPARAM) NULL, (LPARAM)(LPSTR) Nv_FileFormatExtension);
			++count;
		}
		SendDlgItemMessage(hDlg, IDD_EXPORTER_BITMAP_FORMAT_COMBO, WM_SETREDRAW, TRUE, 0L);
		SendDlgItemMessage(hDlg, IDD_EXPORTER_FILE_EXTENSIONS, WM_SETREDRAW, TRUE, 0L);
		count = 0;
		while (TRUE) {
			char 		*Nv_FileFormatDescription;
			Nd_Token 	Nv_FileFormatToolkitToken;
			char		*Nv_FileFormatExtension;
			short		Nv_HandlesColorMappedOutput;
			char		*Nv_FileFormatGUID;

// v4.1.9 (Oct 2004) of the toolkit allows for custom 2D file exporter's to be defined via a GUID, so
// this change will allow for exporters to be chosen via their extension (.tif) or GUID.
#if 00
			Ni_EnumerateSupportedExportBitmapFileFormats(count, &Nv_FileFormatDescription, &Nv_FileFormatToolkitToken, &Nv_FileFormatExtension, &Nv_HandlesColorMappedOutput);
#else
			Ni_EnumerateSupportedExportBitmapFileFormats2(TRUE, count, &Nv_FileFormatDescription, &Nv_FileFormatToolkitToken, &Nv_FileFormatExtension, &Nv_HandlesColorMappedOutput, &Nv_FileFormatGUID);
#endif

			if (!Nv_FileFormatDescription)
				// Break once no more file formats available
				break;

			// For v4.1.9 or newer, the 'autoconvert_bitmap_format' can either be the file's extension (.jpg)
			// or the GUID for the 2D file exporter, such as 'Nc_NG_TOOLKIT_IMAGE_EXPORTER_GUID__JPEG'
			if (Nv_FileFormatGUID && !_stricmp(Nv_FileFormatGUID, export_options->autoconvert_bitmap_format))
 				SendDlgItemMessage(hDlg, IDD_EXPORTER_BITMAP_FORMAT_COMBO, CB_SETCURSEL, count, 0L);
			else if (!strncmp(Nv_FileFormatExtension, export_options->autoconvert_bitmap_format, 4))
 				SendDlgItemMessage(hDlg, IDD_EXPORTER_BITMAP_FORMAT_COMBO, CB_SETCURSEL, count, 0L);

			if (!strncmp(Nv_FileFormatExtension, export_options->convert_file_extension, 4))
				SendDlgItemMessage(hDlg, IDD_EXPORTER_FILE_EXTENSIONS, CB_SETCURSEL, count, 0L);

			++count;
		}

		/* Update the bits-per-pixel drop-down boxes */
		SendDlgItemMessage(hDlg, IDD_EXPORTER_BITMAP_BITSPERPIXEL_COMBO, CB_RESETCONTENT, 0, 0L);
		SendDlgItemMessage(hDlg, IDD_EXPORTER_BITMAP_BITSPERPIXEL_COMBO, WM_SETREDRAW, FALSE, 0L);
		SendDlgItemMessage(hDlg, IDD_EXPORTER_BITMAP_BITSPERPIXEL_COMBO, CB_ADDSTRING, (WPARAM) NULL, (LPARAM)(LPSTR) "2");
		SendDlgItemMessage(hDlg, IDD_EXPORTER_BITMAP_BITSPERPIXEL_COMBO, CB_ADDSTRING, (WPARAM) NULL, (LPARAM)(LPSTR) "4"); 
		SendDlgItemMessage(hDlg, IDD_EXPORTER_BITMAP_BITSPERPIXEL_COMBO, CB_ADDSTRING, (WPARAM) NULL, (LPARAM)(LPSTR) "8"); 
		SendDlgItemMessage(hDlg, IDD_EXPORTER_BITMAP_BITSPERPIXEL_COMBO, CB_ADDSTRING, (WPARAM) NULL, (LPARAM)(LPSTR) "24"); 
		SendDlgItemMessage(hDlg, IDD_EXPORTER_BITMAP_BITSPERPIXEL_COMBO, WM_SETREDRAW, TRUE, 0L);
		if (export_options->txtr_bitmap_depth == 2)
			SendDlgItemMessage(hDlg, IDD_EXPORTER_BITMAP_BITSPERPIXEL_COMBO, CB_SETCURSEL, 0, 0L);
		else if (export_options->txtr_bitmap_depth == 4)
			SendDlgItemMessage(hDlg, IDD_EXPORTER_BITMAP_BITSPERPIXEL_COMBO, CB_SETCURSEL, 1, 0L);
		else if (export_options->txtr_bitmap_depth == 8)
			SendDlgItemMessage(hDlg, IDD_EXPORTER_BITMAP_BITSPERPIXEL_COMBO, CB_SETCURSEL, 2, 0L);
		else if (export_options->txtr_bitmap_depth == 24)
			SendDlgItemMessage(hDlg, IDD_EXPORTER_BITMAP_BITSPERPIXEL_COMBO, CB_SETCURSEL, 3, 0L);

		/* Update the X & Y texture resolutions drop-down boxes */
		SendDlgItemMessage(hDlg, IDD_EXPORTER_TXTR_X_RES_COMBO, CB_RESETCONTENT, 0, 0L);
		SendDlgItemMessage(hDlg, IDD_EXPORTER_TXTR_X_RES_COMBO, WM_SETREDRAW, FALSE, 0L);
		SendDlgItemMessage(hDlg, IDD_EXPORTER_TXTR_Y_RES_COMBO, CB_RESETCONTENT, 0, 0L);
		SendDlgItemMessage(hDlg, IDD_EXPORTER_TXTR_Y_RES_COMBO, WM_SETREDRAW, FALSE, 0L);
		count = 0;
		while (txtr_res_struct[count].desc != (char *) NULL) {
			SendDlgItemMessage(hDlg, IDD_EXPORTER_TXTR_X_RES_COMBO, CB_ADDSTRING, (WPARAM) NULL, (LPARAM)(LPSTR) txtr_res_struct[count].desc);
			SendDlgItemMessage(hDlg, IDD_EXPORTER_TXTR_Y_RES_COMBO, CB_ADDSTRING, (WPARAM) NULL, (LPARAM)(LPSTR) txtr_res_struct[count].desc);
			++count;
		}
		SendDlgItemMessage(hDlg, IDD_EXPORTER_TXTR_X_RES_COMBO, WM_SETREDRAW, TRUE, 0L);
		SendDlgItemMessage(hDlg, IDD_EXPORTER_TXTR_Y_RES_COMBO, WM_SETREDRAW, TRUE, 0L);
		count = 0;
		while (txtr_res_struct[count].desc != (char *) NULL) {
			if (txtr_res_struct[count].method == export_options->txtr_x_res)
				SendDlgItemMessage(hDlg, IDD_EXPORTER_TXTR_X_RES_COMBO, CB_SETCURSEL, count, 0L);
			if (txtr_res_struct[count].method == export_options->txtr_y_res)
				SendDlgItemMessage(hDlg, IDD_EXPORTER_TXTR_Y_RES_COMBO, CB_SETCURSEL, count, 0L);
			++count;
		}

		CheckDlgButton(hDlg, IDD_EXPORTER_CONFIRM_BITMAP_OVERWRITES, export_options->ena_confirm_bitmap_overwrites);

		if (export_options->bitmap_saveto_directory_type == SAVE_BITMAPS_TO_ORIGINAL_DIRECTORY)
			val = IDD_EXPORTER_SAVEBITMAPSTO_ORIGINAL_DIR_RB;
		else if (export_options->bitmap_saveto_directory_type == SAVE_BITMAPS_TO_SPECIFIC_DIRECTORY)
			val = IDD_EXPORTER_SAVEBITMAPSTO_SPECIFIC_RB;
		CheckRadioButton(hDlg, IDD_EXPORTER_SAVEBITMAPSTO_ORIGINAL_DIR_RB, IDD_EXPORTER_SAVEBITMAPSTO_SPECIFIC_RB, val);

		if (export_options->save_bitmaps_to_specific_directory != NULL)
			SendDlgItemMessage(hDlg, IDD_EXPORTER_SAVEBITMAPSTO_SPECIFIC_EDIT, WM_SETTEXT, 0,  (LPARAM)(LPSTR) export_options->save_bitmaps_to_specific_directory);
		else
			SendDlgItemMessage(hDlg, IDD_EXPORTER_SAVEBITMAPSTO_SPECIFIC_EDIT, WM_SETTEXT, 0,  (LPARAM)(LPSTR) "");

		/* Update the enable state of the controls */
		SendMessage(hDlg, WM_UPDATE_DIALOG_ENABLES, 0, 0L);

		return (TRUE);
	case WM_UPDATE_DIALOG_ENABLES:
		/* Enable these to start off */

		if (export_options->ena_convert_txtr_bitmaps)
			val = IDD_EXPORTER_CONVERT_TXTR_BITMAPS_RB;
		else if (export_options->ena_use_specific_bitmap_extension)
			val = IDD_EXPORTER_CONVERT_BITMAP_FILE_XTNS_RB;
		else
			val = IDD_EXPORTER_DONT_DO_ANY_CONVERSIONS_RB;
		CheckRadioButton(hDlg, IDD_EXPORTER_DONT_DO_ANY_CONVERSIONS_RB, IDD_EXPORTER_CONVERT_TXTR_BITMAPS_RB, val);

		Import_IO_Set_Control_Enable(hDlg, IDD_EXPORTER_FILE_EXTENSIONS, export_options->ena_use_specific_bitmap_extension);

		Import_IO_Set_Control_Enable(hDlg, IDD_EXPORTER_AUTO_CONVERT_TXT1, export_options->ena_convert_txtr_bitmaps);
		Import_IO_Set_Control_Enable(hDlg, IDD_EXPORTER_AUTO_CONVERT_TXT2, export_options->ena_convert_txtr_bitmaps);
		Import_IO_Set_Control_Enable(hDlg, IDD_EXPORTER_CONFIRM_BITMAP_OVERWRITES, export_options->ena_convert_txtr_bitmaps);
		Import_IO_Set_Control_Enable(hDlg, IDD_EXPORTER_BITMAP_FORMAT_COMBO, export_options->ena_convert_txtr_bitmaps);
		Import_IO_Set_Control_Enable(hDlg, IDD_EXPORTER_SAVEBITMAPSTO_ORIGINAL_DIR_RB, export_options->ena_convert_txtr_bitmaps);
		Import_IO_Set_Control_Enable(hDlg, IDD_EXPORTER_SAVEBITMAPSTO_SPECIFIC_RB, export_options->ena_convert_txtr_bitmaps);
		Import_IO_Set_Control_Enable(hDlg, IDD_EXPORTER_BITMAP_BITSPERPIXEL_COMBO, export_options->ena_convert_txtr_bitmaps);
		Import_IO_Set_Control_Enable(hDlg, IDD_EXPORTER_SAVEBITMAPSTO_SPECIFIC_BROWSE, export_options->ena_convert_txtr_bitmaps);
		Import_IO_Set_Control_Enable(hDlg, IDD_EXPORTER_BITMAP_FORMAT_OPTIONS_BUTTON, export_options->ena_convert_txtr_bitmaps);
		Import_IO_Set_Control_Enable(hDlg, IDD_EXPORTER_TXTR_X_RES_COMBO, export_options->ena_convert_txtr_bitmaps);
		Import_IO_Set_Control_Enable(hDlg, IDD_EXPORTER_TXTR_Y_RES_COMBO, export_options->ena_convert_txtr_bitmaps);
		Import_IO_Set_Control_Enable(hDlg, IDD_EXPORTER_AUTO_CONVERT_TXT4, export_options->ena_convert_txtr_bitmaps);
		Import_IO_Set_Control_Enable(hDlg, IDD_EXPORTER_AUTO_CONVERT_TXT5, export_options->ena_convert_txtr_bitmaps);
		Import_IO_Set_Control_Enable(hDlg, IDD_EXPORTER_TXTR_X_RES_COMBO, export_options->ena_convert_txtr_bitmaps);
		Import_IO_Set_Control_Enable(hDlg, IDD_EXPORTER_AUTO_CONVERT_TXT3, export_options->ena_convert_txtr_bitmaps);

		return(TRUE);
	case WM_NOTIFY:
    		switch (((NMHDR FAR *)  (LPARAM) lParam)->code) {
		case PSN_KILLACTIVE:
			// The page is either losing the activation, or the user pressed OK.
           		// OKINO_SetWindowLong(hDlg, DWLP_MSGRESULT, FALSE);	// No error
			break;
                case PSN_APPLY:
			// The Apply button was pressed, or the OK button was pressed
			// and this message is being sent to all panels to have it send
			// the control values to the toolkit.
           		OKINO_SetWindowLong(hDlg, DWLP_MSGRESULT, PSNRET_INVALID);	// Error initially

			/* Only update the UI if the user clicked the 'Apply' button and not the 'Ok' button */
			lppsn = (LPPSHNOTIFY)  (LPARAM) lParam;
			user_clicked_apply_button = !lppsn->lParam;
			if (!user_clicked_apply_button)
				property_sheet_return_value = IDOK;

           		OKINO_SetWindowLong(hDlg, DWLP_MSGRESULT, PSNRET_NOERROR);
			return(TRUE);
		case PSN_QUERYCANCEL:
			// The user has pressed CANCEL. Return FALSE to allow canceling, or TRUE to prevent it
           		OKINO_SetWindowLong(hDlg, DWLP_MSGRESULT, FALSE);
			property_sheet_return_value = IDCANCEL;
			return(FALSE);
		case PSN_RESET:
			// User has pressed CANCEL and property sheet is about to be destroyed
           		OKINO_SetWindowLong(hDlg, DWLP_MSGRESULT, FALSE);
			property_sheet_return_value = IDCANCEL;
			break;
		case PSN_SETACTIVE:
			// Property sheet is about to be activated. We set up the values of the controls here

			/* Enable the 'Apply' button */
			SendMessage(GetParent(hDlg), PSM_CHANGED, (WPARAM) hDlg, 0L);

			// RCL, we don't want to do this here since it will reset all the type-in parameters!
			// SendMessage(hDlg, WM_UPDATE_DIALOG_PARAMETERS, 0, 0L);

			/* Update the enable state of the controls */
			SendMessage(hDlg, WM_UPDATE_DIALOG_ENABLES, 0, 0L);

			break;
                case PSN_HELP:
			// The Help button was pressed
			Import_IO_DisplayHelpTopic(hDlg, plugin_directory, HELP_FILENAME, "main");
			break;
		default:
			return FALSE;
	    	}
		break;
	case WM_COMMAND:
		cmd_id = LOWORD(wParam);
		switch (cmd_id) {
			case IDD_BUTTON_ABOUT:
				DialogBox(hDLLinstance, "AboutBox", parent_hwnd, (DLGPROC) AboutDlgProc);
				return(TRUE);
			case IDD_BUTTON_RESET:
				if (IDYES == MessageBox(hDlg, "This will reset the values of this dialog box to their power-up defaults.\n\nAre you sure you want to do this?", "Confirmation Request", MB_APPLMODAL | MB_YESNO | MB_DEFBUTTON1 | MB_ICONQUESTION)) {
					export_options->options_initialized = FALSE;
					NI_Exporter_InitExportOptionVariables();
					SendMessage(hDlg, WM_UPDATE_DIALOG_PARAMETERS, 0, 0L);
				}
				return(TRUE);

			case IDD_EXPORTER_DONT_DO_ANY_CONVERSIONS_RB:
				export_options->ena_convert_txtr_bitmaps = export_options->ena_use_specific_bitmap_extension = FALSE;
				CheckRadioButton(hDlg, IDD_EXPORTER_DONT_DO_ANY_CONVERSIONS_RB, IDD_EXPORTER_CONVERT_TXTR_BITMAPS_RB, IDD_EXPORTER_DONT_DO_ANY_CONVERSIONS_RB);
				/* Update the enable state of the controls */
				SendMessage(hDlg, WM_UPDATE_DIALOG_ENABLES, 0, 0L);
				return(TRUE);
			case IDD_EXPORTER_CONVERT_BITMAP_FILE_XTNS_RB:
				export_options->ena_use_specific_bitmap_extension = !export_options->ena_use_specific_bitmap_extension;
				CheckRadioButton(hDlg, IDD_EXPORTER_DONT_DO_ANY_CONVERSIONS_RB, IDD_EXPORTER_CONVERT_TXTR_BITMAPS_RB, IDD_EXPORTER_CONVERT_BITMAP_FILE_XTNS_RB);
				if (export_options->ena_convert_txtr_bitmaps)
					export_options->ena_convert_txtr_bitmaps = FALSE;
				/* Update the enable state of the controls */
				SendMessage(hDlg, WM_UPDATE_DIALOG_ENABLES, 0, 0L);
				return(TRUE);
			case IDD_EXPORTER_CONVERT_TXTR_BITMAPS_RB:
				export_options->ena_convert_txtr_bitmaps = !export_options->ena_convert_txtr_bitmaps;
				CheckRadioButton(hDlg, IDD_EXPORTER_DONT_DO_ANY_CONVERSIONS_RB, IDD_EXPORTER_CONVERT_TXTR_BITMAPS_RB, IDD_EXPORTER_CONVERT_TXTR_BITMAPS_RB);
				if (export_options->ena_use_specific_bitmap_extension)
					export_options->ena_use_specific_bitmap_extension = FALSE;
				/* Update the enable state of the controls */
				SendMessage(hDlg, WM_UPDATE_DIALOG_ENABLES, 0, 0L);
				return(TRUE);

			case IDD_EXPORTER_FILE_EXTENSIONS:
				if (GET_WM_COMMAND_CMD(wParam, lParam) == CBN_SELCHANGE) {
					char 		*Nv_FileFormatDescription;
					Nd_Token 	Nv_FileFormatToolkitToken;
					char		*Nv_FileFormatExtension;
					short		Nv_HandlesColorMappedOutput;
					char		*Nv_FileFormatGUID;

				 	wIndex = (UINT) SendDlgItemMessage(hDlg, IDD_EXPORTER_FILE_EXTENSIONS, CB_GETCURSEL, 0, 0L);

// v4.1.9 (Oct 2004) of the toolkit allows for custom 2D file exporter's to be defined via a GUID, so
// this change will allow for exporters to be chosen via their extension (.tif) or GUID.
#if 00
					Ni_EnumerateSupportedExportBitmapFileFormats(wIndex, &Nv_FileFormatDescription, &Nv_FileFormatToolkitToken, &Nv_FileFormatExtension, &Nv_HandlesColorMappedOutput, &Nv_FileFormatGUID);
#else
					Ni_EnumerateSupportedExportBitmapFileFormats2(TRUE, wIndex, &Nv_FileFormatDescription, &Nv_FileFormatToolkitToken, &Nv_FileFormatExtension, &Nv_HandlesColorMappedOutput, &Nv_FileFormatGUID);
#endif
					if (!Nv_FileFormatExtension)
						break;
					Import_IO_Copy_String(&export_options->convert_file_extension, Nv_FileFormatExtension);
				}
				return (TRUE);

			case IDD_EXPORTER_BITMAP_FORMAT_COMBO:
				if (GET_WM_COMMAND_CMD(wParam, lParam) == CBN_SELCHANGE) {
					char 		*Nv_FileFormatDescription;
					Nd_Token 	Nv_FileFormatToolkitToken;
					char		*Nv_FileFormatExtension;
					short		Nv_HandlesColorMappedOutput;
					char		*Nv_FileFormatGUID;

				 	wIndex = (UINT) SendDlgItemMessage(hDlg, IDD_EXPORTER_BITMAP_FORMAT_COMBO, CB_GETCURSEL, 0, 0L);

// v4.1.9 (Oct 2004) of the toolkit allows for custom 2D file exporter's to be defined via a GUID, so
// this change will allow for exporters to be chosen via their extension (.tif) or GUID.
#if 00
					Ni_EnumerateSupportedExportBitmapFileFormats(wIndex, &Nv_FileFormatDescription, &Nv_FileFormatToolkitToken, &Nv_FileFormatExtension, &Nv_HandlesColorMappedOutput, &Nv_FileFormatGUID);
#else
					Ni_EnumerateSupportedExportBitmapFileFormats2(TRUE, wIndex, &Nv_FileFormatDescription, &Nv_FileFormatToolkitToken, &Nv_FileFormatExtension, &Nv_HandlesColorMappedOutput, &Nv_FileFormatGUID);
#endif

					if (Nv_FileFormatGUID)
						Import_IO_Copy_String(&export_options->autoconvert_bitmap_format, Nv_FileFormatGUID);
					else if (Nv_FileFormatExtension)
						Import_IO_Copy_String(&export_options->autoconvert_bitmap_format, Nv_FileFormatExtension);
				}
				return(TRUE);

			case IDD_EXPORTER_BITMAP_FORMAT_OPTIONS_BUTTON:
				// Allow the parameters of the currently selected output driver to be modified

				{ char	*guid, *caption_title = "Set the image output options";;

				guid = export_options->autoconvert_bitmap_format;	// Either the exporter's GUID or the exporter's desired file extension
				if (guid && guid[0] != '{')
					// Convert the file extension (like .tif) to its corresponding 2D file exporter's GUID
					guid = Ni_Determine_Export_Image_File_Type_As_GUID(export_options->autoconvert_bitmap_format);

				// Show the dynamic 2D file exporter's options dialog box
				if (guid && guid[0] == '{')
					Import_IO_ShowDynamic2DFileOutputOptionsDialogBox(hDlg, guid, TRUE, NULL, caption_title, NULL, NULL, NULL);

				}
				return(TRUE);

			case IDD_EXPORTER_BITMAP_BITSPERPIXEL_COMBO:
				if (GET_WM_COMMAND_CMD(wParam, lParam) == CBN_SELCHANGE) {
				 	wIndex = (UINT) SendDlgItemMessage(hDlg, IDD_EXPORTER_BITMAP_BITSPERPIXEL_COMBO, CB_GETCURSEL, 0, 0L);
					if (wIndex == 0)
						export_options->txtr_bitmap_depth = 2;
					else if (wIndex == 1)
						export_options->txtr_bitmap_depth = 4;
					else if (wIndex == 2)
						export_options->txtr_bitmap_depth = 8;
					else if (wIndex == 3)
						export_options->txtr_bitmap_depth = 24;
				}
				return(TRUE);

			case IDD_EXPORTER_TXTR_X_RES_COMBO:
				if (GET_WM_COMMAND_CMD(wParam, lParam) == CBN_SELCHANGE) {
				 	wIndex = (UINT) SendDlgItemMessage(hDlg, IDD_EXPORTER_TXTR_X_RES_COMBO, CB_GETCURSEL, 0, 0L);
					if (wIndex >= 0)
						export_options->txtr_x_res = txtr_res_struct[wIndex].method;
				}
				return (TRUE);
			case IDD_EXPORTER_TXTR_Y_RES_COMBO:
				if (GET_WM_COMMAND_CMD(wParam, lParam) == CBN_SELCHANGE) {
				 	wIndex = (UINT) SendDlgItemMessage(hDlg, IDD_EXPORTER_TXTR_Y_RES_COMBO, CB_GETCURSEL, 0, 0L);
					if (wIndex >= 0)
						export_options->txtr_y_res = txtr_res_struct[wIndex].method;
				}
				return(TRUE);

			case IDD_EXPORTER_CONFIRM_BITMAP_OVERWRITES:
				export_options->ena_confirm_bitmap_overwrites = !export_options->ena_confirm_bitmap_overwrites;
				CheckDlgButton(hDlg, IDD_EXPORTER_CONFIRM_BITMAP_OVERWRITES, export_options->ena_confirm_bitmap_overwrites);
				return(TRUE);
			case IDD_EXPORTER_SAVEBITMAPSTO_SPECIFIC_BROWSE:
				{ char *temp_ptr = NULL;
	
				if (export_options->save_bitmaps_to_specific_directory == NULL)
					Import_IO_Copy_String(&temp_ptr, "c:\\");
				else
					Import_IO_Copy_String(&temp_ptr, export_options->save_bitmaps_to_specific_directory);
				if (Import_IO_GetWindowsOutputDirectory(hDlg, &temp_ptr, "Bitmap Directory Location", "Please choose the bitmap directory:")) {
					Import_IO_Copy_String(&export_options->save_bitmaps_to_specific_directory, temp_ptr);
					SetDlgItemText(hDlg, IDD_EXPORTER_SAVEBITMAPSTO_SPECIFIC_EDIT, export_options->save_bitmaps_to_specific_directory);
				}
				if (temp_ptr != NULL)
					Ni_Free_Memory((char *) temp_ptr);
				}
				return(TRUE);

			case IDD_EXPORTER_SAVEBITMAPSTO_ORIGINAL_DIR_RB:
				export_options->bitmap_saveto_directory_type = SAVE_BITMAPS_TO_ORIGINAL_DIRECTORY;
				CheckRadioButton(hDlg, IDD_EXPORTER_SAVEBITMAPSTO_ORIGINAL_DIR_RB, IDD_EXPORTER_SAVEBITMAPSTO_SPECIFIC_RB, LOWORD(wParam));
				return(TRUE);
			case IDD_EXPORTER_SAVEBITMAPSTO_SPECIFIC_RB:
				export_options->bitmap_saveto_directory_type = SAVE_BITMAPS_TO_SPECIFIC_DIRECTORY;
				CheckRadioButton(hDlg, IDD_EXPORTER_SAVEBITMAPSTO_ORIGINAL_DIR_RB, IDD_EXPORTER_SAVEBITMAPSTO_SPECIFIC_RB, LOWORD(wParam));
				return(TRUE);

			default:
				break;
		}
	}
	return (FALSE);
}


/* Paths Panel # 6 */

	static INT_PTR CALLBACK 
PathsPanel_Page_DlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static PROPSHEETPAGE *ps;
	LPPSHNOTIFY	lppsn;
	char		temp_buf[512];
	short		cmd_id, code, user_clicked_apply_button;

	switch (msg) {
	case WM_INITDIALOG:
		// Save the PROPSHEETPAGE information.
		ps = (PROPSHEETPAGE *)  (LPARAM) lParam;

		updating_edit_control = FALSE;

		SendMessage(hDlg, WM_UPDATE_DIALOG_PARAMETERS, 0, 0L);
		return (TRUE);
	case WM_UPDATE_DIALOG_PARAMETERS:
		updating_edit_control = TRUE;

		CheckDlgButton(hDlg, IDD_EXPORTER_CONVERT_TO_UNIX_PATHS, export_options->convert_to_unix_paths);
		CheckDlgButton(hDlg, IDD_EXPORTER_STRIP_ABS_FILE_PATHS, export_options->ena_strip_abs_filepaths);
		CheckDlgButton(hDlg, IDD_EXPORTER_CONVERT_TO_RFC1738_PATHS, export_options->convert_paths_to_rfc1738_standard);
		CheckDlgButton(hDlg, IDD_EXPORTER_USE_ABSOLUTE_FILE_PATHS, export_options->use_absolute_file_paths);

		/* Absolute file path */
		if (export_options->specific_filepath_to_prepend != NULL) {
			if (export_options->specific_relative_filepath_to_prepend != NULL) {
				free(export_options->specific_relative_filepath_to_prepend);
				export_options->specific_relative_filepath_to_prepend = NULL;
			}
			SetDlgItemText(hDlg, IDD_EXPORTER_PREPEND_SPECIFIC_FILEPATH_EDIT, export_options->specific_filepath_to_prepend);
		} else
			SetDlgItemText(hDlg, IDD_EXPORTER_PREPEND_SPECIFIC_FILEPATH_EDIT, "");

		/* Relative file path */
		if (export_options->specific_relative_filepath_to_prepend != NULL)
			SetDlgItemText(hDlg, IDD_EXPORTER_PREPEND_RELATIVE_FILEPATH_EDIT, export_options->specific_relative_filepath_to_prepend);
		else
			SetDlgItemText(hDlg, IDD_EXPORTER_PREPEND_RELATIVE_FILEPATH_EDIT, "");

		/* URL bitmap path prefix */
		if (export_options->url_string != NULL)
			SetDlgItemText(hDlg, IDD_EXPORTER_URL_EDIT, export_options->url_string);
		else
			SetDlgItemText(hDlg, IDD_EXPORTER_URL_EDIT, "");

		updating_edit_control = FALSE;

		/* Update the enable state of the controls */
		SendMessage(hDlg, WM_UPDATE_DIALOG_ENABLES, 0, 0L);

		return (TRUE);
	case WM_UPDATE_DIALOG_ENABLES:
		Import_IO_Set_Control_Enable(hDlg, IDD_EXPORTER_CONVERT_TO_RFC1738_PATHS, TRUE);

		return(TRUE);
	case WM_NOTIFY:
		code = ((NMHDR FAR *)  (LPARAM) lParam)->code;
    		switch (code) {
		case PSN_KILLACTIVE:
                case PSN_APPLY:
			// The Apply button was pressed, or the OK button was pressed
			// and this message is being sent to all panels to have it send
			// the control values to the toolkit.
			if (code == PSN_APPLY)
	           		OKINO_SetWindowLong(hDlg, DWLP_MSGRESULT, PSNRET_INVALID);	// Error initially
			else
	           		OKINO_SetWindowLong(hDlg, DWLP_MSGRESULT, TRUE);	// Error

			/* Get the relative path name */
			GetDlgItemText(hDlg, IDD_EXPORTER_PREPEND_RELATIVE_FILEPATH_EDIT, temp_buf, sizeof(temp_buf));
			if (export_options->specific_relative_filepath_to_prepend)
				free(export_options->specific_relative_filepath_to_prepend);
			export_options->specific_relative_filepath_to_prepend = NULL;
			if (strlen(temp_buf))
				Import_IO_Copy_String(&export_options->specific_relative_filepath_to_prepend, temp_buf);

			/* Get the URL path name */
			GetDlgItemText(hDlg, IDD_EXPORTER_URL_EDIT, temp_buf, sizeof(temp_buf));
			if (export_options->url_string)
				free(export_options->url_string);
			export_options->url_string = NULL;
			if (strlen(temp_buf))
				Import_IO_Copy_String(&export_options->url_string, temp_buf);

			/* Only update the UI if the user clicked the 'Apply' button and not the 'Ok' button */
			if (code == PSN_APPLY) {
				lppsn = (LPPSHNOTIFY)  (LPARAM) lParam;
				user_clicked_apply_button = !lppsn->lParam;
				if (!user_clicked_apply_button)
					property_sheet_return_value = IDOK;
			}

			if (code == PSN_APPLY)
		           	OKINO_SetWindowLong(hDlg, DWLP_MSGRESULT, PSNRET_NOERROR);
			else
		           	OKINO_SetWindowLong(hDlg, DWLP_MSGRESULT, FALSE);	// No error

			return(TRUE);
		case PSN_QUERYCANCEL:
			// The user has pressed CANCEL. Return FALSE to allow canceling, or TRUE to prevent it
           		OKINO_SetWindowLong(hDlg, DWLP_MSGRESULT, FALSE);
			property_sheet_return_value = IDCANCEL;
			return(FALSE);
		case PSN_RESET:
			// User has pressed CANCEL and property sheet is about to be destroyed
           		OKINO_SetWindowLong(hDlg, DWLP_MSGRESULT, FALSE);
			property_sheet_return_value = IDCANCEL;
			break;
		case PSN_SETACTIVE:
			// Property sheet is about to be activated. We set up the values of the controls here

			/* Enable the 'Apply' button */
			SendMessage(GetParent(hDlg), PSM_CHANGED, (WPARAM) hDlg, 0L);

			// RCL, we don't want to do this here since it will reset all the type-in parameters!
			// SendMessage(hDlg, WM_UPDATE_DIALOG_PARAMETERS, 0, 0L);

			/* Update the enable state of the controls */
			SendMessage(hDlg, WM_UPDATE_DIALOG_ENABLES, 0, 0L);

			break;
                case PSN_HELP:
			// The Help button was pressed
			Import_IO_DisplayHelpTopic(hDlg, plugin_directory, HELP_FILENAME, "main");
			break;
		default:
			return FALSE;
	    	}
		break;
	case WM_COMMAND:
		cmd_id = LOWORD(wParam);
		switch (cmd_id) {
			case IDD_BUTTON_ABOUT:
				DialogBox(hDLLinstance, "AboutBox", parent_hwnd, (DLGPROC) AboutDlgProc);
				return(TRUE);
			case IDD_BUTTON_RESET:
				if (IDYES == MessageBox(hDlg, "This will reset the values of this dialog box to their power-up defaults.\n\nAre you sure you want to do this?", "Confirmation Request", MB_APPLMODAL | MB_YESNO | MB_DEFBUTTON1 | MB_ICONQUESTION)) {
					export_options->options_initialized = FALSE;
					NI_Exporter_InitExportOptionVariables();
					SendMessage(hDlg, WM_UPDATE_DIALOG_PARAMETERS, 0, 0L);
				}
				return(TRUE);

			case IDD_EXPORTER_CONVERT_TO_RFC1738_PATHS:
				export_options->convert_paths_to_rfc1738_standard = !export_options->convert_paths_to_rfc1738_standard;
				CheckDlgButton(hDlg, IDD_EXPORTER_CONVERT_TO_RFC1738_PATHS, export_options->convert_paths_to_rfc1738_standard);
				return(TRUE);
			case IDD_EXPORTER_CONVERT_TO_UNIX_PATHS:
				export_options->convert_to_unix_paths = !export_options->convert_to_unix_paths;
				CheckDlgButton(hDlg, IDD_EXPORTER_CONVERT_TO_UNIX_PATHS, export_options->convert_to_unix_paths);
				return(TRUE);
			case IDD_EXPORTER_STRIP_ABS_FILE_PATHS:
				export_options->ena_strip_abs_filepaths = !export_options->ena_strip_abs_filepaths;
				CheckDlgButton(hDlg, IDD_EXPORTER_STRIP_ABS_FILE_PATHS, export_options->ena_strip_abs_filepaths);
				return(TRUE);
			case IDD_EXPORTER_USE_ABSOLUTE_FILE_PATHS:
				export_options->use_absolute_file_paths = !export_options->use_absolute_file_paths;
				CheckDlgButton(hDlg, IDD_EXPORTER_USE_ABSOLUTE_FILE_PATHS, export_options->use_absolute_file_paths);
				return(TRUE);

			case IDD_EXPORTER_PREPEND_SPECIFIC_FILEPATH_BROWSE:
				{ char *temp_ptr = NULL;

				if (export_options->specific_filepath_to_prepend == NULL)
					Import_IO_Copy_String(&temp_ptr, "c:\\");
				else
					Import_IO_Copy_String(&temp_ptr, export_options->specific_filepath_to_prepend);
				if (Import_IO_GetWindowsOutputDirectory(hDlg, &temp_ptr, "Directory Prefix Location", "Please choose the new filepath:")) {
					Import_IO_Copy_String(&export_options->specific_filepath_to_prepend, temp_ptr);
					SetDlgItemText(hDlg, IDD_EXPORTER_PREPEND_SPECIFIC_FILEPATH_EDIT, export_options->specific_filepath_to_prepend);
					updating_edit_control = TRUE;
					SetDlgItemText(hDlg, IDD_EXPORTER_PREPEND_RELATIVE_FILEPATH_EDIT, "");
					SetDlgItemText(hDlg, IDD_EXPORTER_URL_EDIT, "");
					updating_edit_control = FALSE;
				}
				if (temp_ptr != NULL)
					Ni_Free_Memory((char *) temp_ptr);
				}
				return(TRUE);

			case IDD_EXPORTER_PREPEND_RELATIVE_FILEPATH_EDIT:
				if (GET_WM_COMMAND_CMD(wParam, lParam) == EN_CHANGE && !updating_edit_control) {
					if (export_options->specific_filepath_to_prepend != NULL) {
						free((char *) export_options->specific_filepath_to_prepend);
						export_options->specific_filepath_to_prepend = NULL;
					}
					if (export_options->url_string != NULL) {
						free((char *) export_options->url_string);
						export_options->url_string = NULL;
					}
					updating_edit_control = TRUE;
					SetDlgItemText(hDlg, IDD_EXPORTER_PREPEND_SPECIFIC_FILEPATH_EDIT, "");
					SetDlgItemText(hDlg, IDD_EXPORTER_URL_EDIT, "");
					updating_edit_control = FALSE;
				}
				return(TRUE);

			case IDD_EXPORTER_URL_EDIT:
				if (GET_WM_COMMAND_CMD(wParam, lParam) == EN_CHANGE && !updating_edit_control) {
					if (export_options->specific_filepath_to_prepend != NULL) {
						free((char *) export_options->specific_filepath_to_prepend);
						export_options->specific_filepath_to_prepend = NULL;
					}
					if (export_options->specific_relative_filepath_to_prepend != NULL) {
						free((char *) export_options->specific_relative_filepath_to_prepend);
						export_options->specific_relative_filepath_to_prepend = NULL;
					}
					updating_edit_control = TRUE;
					SetDlgItemText(hDlg, IDD_EXPORTER_PREPEND_RELATIVE_FILEPATH_EDIT, "");
					SetDlgItemText(hDlg, IDD_EXPORTER_PREPEND_SPECIFIC_FILEPATH_EDIT, "");
					updating_edit_control = FALSE;
				}
				return(TRUE);

			default:
				break;
		}
	}
	return (FALSE);
}

/* Animation Panel # 7 */

	static INT_PTR CALLBACK 
AnimationPanel_Page_DlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static PROPSHEETPAGE *ps;
	LPPSHNOTIFY	lppsn;
	UINT		wIndex;
	short		cmd_id, state, count, user_clicked_apply_button;

	switch (msg) {
	case WM_INITDIALOG:
		// Save the PROPSHEETPAGE information.
		ps = (PROPSHEETPAGE *)  (LPARAM) lParam;

		SendMessage(hDlg, WM_UPDATE_DIALOG_PARAMETERS, 0, 0L);
		return (TRUE);
	case WM_UPDATE_DIALOG_PARAMETERS:
		CheckDlgButton(hDlg, IDD_EXPORTER_ENA_OBJECT_ANIMATION, export_options->ena_object_animation);
		CheckDlgButton(hDlg, IDD_EXPORTER_ENA_CAMERA_ANIMATION, export_options->ena_camera_animation);
		CheckDlgButton(hDlg, IDD_EXPORTER_ENA_LIGHT_ANIMATION, export_options->ena_light_animation);
		CheckDlgButton(hDlg, IDD_EXPORTER_ENA_MATERIAL_ANIMATION, export_options->ena_material_animation);
		CheckDlgButton(hDlg, IDD_EXPORTER_ANIM_EXPORT_TRIPLE_CHANNELS_AS_SEP_CTRLS, export_options->use_independent_controllers_for_tripled_anim_channels);
		CheckDlgButton(hDlg, IDD_EXPORTER_FIX_UP_EULER_180_ROT_FLIPS, export_options->flip_Euler_180flips_via_resampler);

		if (export_options->export_raw_unsampled_anim_data_if_possible)
			cmd_id = IDD_EXPORTER_ANIM_EXPORT_RAW_KEYFRAMES_RB;
		else
			cmd_id = IDD_EXPORTER_ANIM_EXPORT_RESAMPLED_KEYFRAMES_RB;
		CheckRadioButton(hDlg, IDD_EXPORTER_ANIM_EXPORT_RAW_KEYFRAMES_RB, IDD_EXPORTER_ANIM_EXPORT_RESAMPLED_KEYFRAMES_RB, cmd_id);

		// Update the default animation interpolation import method
		SendDlgItemMessage(hDlg, IDD_EXPORTER_ANIM_EXPORT_DEFAULT_INTERP_METHOD, CB_RESETCONTENT, 0, 0L);
		SendDlgItemMessage(hDlg, IDD_EXPORTER_ANIM_EXPORT_DEFAULT_INTERP_METHOD, WM_SETREDRAW, FALSE, 0L);
		SendDlgItemMessage(hDlg, IDD_EXPORTER_ANIM_EXPORT_DEFAULT_INTERP_METHOD, CB_ADDSTRING, (WPARAM) NULL, (LPARAM)(LPSTR) "Original");
		SendDlgItemMessage(hDlg, IDD_EXPORTER_ANIM_EXPORT_DEFAULT_INTERP_METHOD, CB_ADDSTRING, (WPARAM) NULL, (LPARAM)(LPSTR) "Constant");
		SendDlgItemMessage(hDlg, IDD_EXPORTER_ANIM_EXPORT_DEFAULT_INTERP_METHOD, CB_ADDSTRING, (WPARAM) NULL, (LPARAM)(LPSTR) "Linear");
		SendDlgItemMessage(hDlg, IDD_EXPORTER_ANIM_EXPORT_DEFAULT_INTERP_METHOD, CB_ADDSTRING, (WPARAM) NULL, (LPARAM)(LPSTR) "TCB");
		SendDlgItemMessage(hDlg, IDD_EXPORTER_ANIM_EXPORT_DEFAULT_INTERP_METHOD, CB_ADDSTRING, (WPARAM) NULL, (LPARAM)(LPSTR) "Bezier");
		SendDlgItemMessage(hDlg, IDD_EXPORTER_ANIM_EXPORT_DEFAULT_INTERP_METHOD, WM_SETREDRAW, TRUE, 0L);
		if (export_options->interpolation_type_to_use_during_export == Nc_NRS_ANIM_INTERPOLATION_TYPE_ORIGINAL)
			SendDlgItemMessage(hDlg, IDD_EXPORTER_ANIM_EXPORT_DEFAULT_INTERP_METHOD, CB_SETCURSEL, 0, 0L);
		else if (export_options->interpolation_type_to_use_during_export == Nc_NRS_ANIM_INTERPOLATION_TYPE_CONSTANT)
			SendDlgItemMessage(hDlg, IDD_EXPORTER_ANIM_EXPORT_DEFAULT_INTERP_METHOD, CB_SETCURSEL, 1, 0L);
		else if (export_options->interpolation_type_to_use_during_export == Nc_NRS_ANIM_INTERPOLATION_TYPE_LINEAR)
			SendDlgItemMessage(hDlg, IDD_EXPORTER_ANIM_EXPORT_DEFAULT_INTERP_METHOD, CB_SETCURSEL, 2, 0L);
		else if (export_options->interpolation_type_to_use_during_export == Nc_NRS_ANIM_INTERPOLATION_TYPE_TCB)
			SendDlgItemMessage(hDlg, IDD_EXPORTER_ANIM_EXPORT_DEFAULT_INTERP_METHOD, CB_SETCURSEL, 3, 0L);
		else
			SendDlgItemMessage(hDlg, IDD_EXPORTER_ANIM_EXPORT_DEFAULT_INTERP_METHOD, CB_SETCURSEL, 4, 0L);

		// Update the default animation rotation controller to use
		SendDlgItemMessage(hDlg, IDD_EXPORTER_ANIM_EXPORT_DEFAULT_ROT_CNTRL, CB_RESETCONTENT, 0, 0L);
		SendDlgItemMessage(hDlg, IDD_EXPORTER_ANIM_EXPORT_DEFAULT_ROT_CNTRL, WM_SETREDRAW, FALSE, 0L);
		SendDlgItemMessage(hDlg, IDD_EXPORTER_ANIM_EXPORT_DEFAULT_ROT_CNTRL, CB_ADDSTRING, (WPARAM) NULL, (LPARAM)(LPSTR) "Quaternion");
		SendDlgItemMessage(hDlg, IDD_EXPORTER_ANIM_EXPORT_DEFAULT_ROT_CNTRL, CB_ADDSTRING, (WPARAM) NULL, (LPARAM)(LPSTR) "Euler");
		SendDlgItemMessage(hDlg, IDD_EXPORTER_ANIM_EXPORT_DEFAULT_ROT_CNTRL, CB_ADDSTRING, (WPARAM) NULL, (LPARAM)(LPSTR) "Same as original keyframes");
		SendDlgItemMessage(hDlg, IDD_EXPORTER_ANIM_EXPORT_DEFAULT_ROT_CNTRL, WM_SETREDRAW, TRUE, 0L);
		if (export_options->rotation_controller_type_to_use_during_export == Nc_NRS_ROTATION_CONTROLLER_QUATERNION)
			SendDlgItemMessage(hDlg, IDD_EXPORTER_ANIM_EXPORT_DEFAULT_ROT_CNTRL, CB_SETCURSEL, 0, 0L);
		else if (export_options->rotation_controller_type_to_use_during_export == Nc_NRS_ROTATION_CONTROLLER_EULER)
			SendDlgItemMessage(hDlg, IDD_EXPORTER_ANIM_EXPORT_DEFAULT_ROT_CNTRL, CB_SETCURSEL, 1, 0L);
		else
			// Else, use a controller which corresponds to the type of rotation controller used in the toolkit
			SendDlgItemMessage(hDlg, IDD_EXPORTER_ANIM_EXPORT_DEFAULT_ROT_CNTRL, CB_SETCURSEL, 2, 0L);

		/* Update the object animation euler order combo box */
		SendDlgItemMessage(hDlg, IDD_EXPORTER_ROT_ORDER_COMBO, CB_RESETCONTENT, 0, 0L);
		SendDlgItemMessage(hDlg, IDD_EXPORTER_ROT_ORDER_COMBO, WM_SETREDRAW, FALSE, 0L);
		count = 0;
		while (euler_order_struct[count].desc != (char *) NULL) {
			SendDlgItemMessage(hDlg, IDD_EXPORTER_ROT_ORDER_COMBO, CB_ADDSTRING, (WPARAM) NULL, (LPARAM)(LPSTR) euler_order_struct[count].desc);
			++count;
		}
		SendDlgItemMessage(hDlg, IDD_EXPORTER_ROT_ORDER_COMBO, WM_SETREDRAW, TRUE, 0L);
		count = 0;
		while (euler_order_struct[count].desc != (char *) NULL) {
			if (euler_order_struct[count].order == export_options->ena_object_anim_resampled_rot_order)
				SendDlgItemMessage(hDlg, IDD_EXPORTER_ROT_ORDER_COMBO, CB_SETCURSEL, count, 0L);
			++count;
		}

		/* Update the enable state of the controls */
		SendMessage(hDlg, WM_UPDATE_DIALOG_ENABLES, 0, 0L);

		return (TRUE);
	case WM_UPDATE_DIALOG_ENABLES:
		state = export_options->ena_hierarchy;
		Import_IO_Set_Control_Enable(hDlg, IDD_EXPORTER_ENA_OBJECT_ANIMATION, state);
		Import_IO_Set_Control_Enable(hDlg, IDD_EXPORTER_ENA_CAMERA_ANIMATION, state);
		Import_IO_Set_Control_Enable(hDlg, IDD_EXPORTER_ENA_LIGHT_ANIMATION, state);
		Import_IO_Set_Control_Enable(hDlg, IDD_EXPORTER_ENA_MATERIAL_ANIMATION, state);

		state = !export_options->export_raw_unsampled_anim_data_if_possible;
		Import_IO_Set_Control_Enable(hDlg, IDD_EXPORTER_ANIM_EXPORT_TRIPLE_CHANNELS_AS_SEP_CTRLS, state);
		Import_IO_Set_Control_Enable(hDlg, IDD_EXPORTER_ANIM_EXPORT_DEFAULT_INTERP_METHOD, state);
		Import_IO_Set_Control_Enable(hDlg, IDD_EXPORTER_ANIM_FRAME_TXT4, state);
		Import_IO_Set_Control_Enable(hDlg, IDD_EXPORTER_ANIM_EXPORT_DEFAULT_ROT_CNTRL, state);
		Import_IO_Set_Control_Enable(hDlg, IDD_RESAMPLING_AND_REDUCTION_OPTIONS_BUTTON, state);
		Import_IO_Set_Control_Enable(hDlg, IDD_EXPORTER_ROT_ORDER_COMBO, state);
		Import_IO_Set_Control_Enable(hDlg, IDD_EXPORTER_ROT_ORDER_TXT1, state);
		Import_IO_Set_Control_Enable(hDlg, IDD_EXPORTER_FIX_UP_EULER_180_ROT_FLIPS, state);

		if (!export_options->export_raw_unsampled_anim_data_if_possible) {
			if (export_options->rotation_controller_type_to_use_during_export == Nc_NRS_ROTATION_CONTROLLER_QUATERNION) {
				Import_IO_Set_Control_Enable(hDlg, IDD_EXPORTER_ROT_ORDER_COMBO, FALSE);
				Import_IO_Set_Control_Enable(hDlg, IDD_EXPORTER_ROT_ORDER_TXT1, FALSE);
				Import_IO_Set_Control_Enable(hDlg, IDD_EXPORTER_FIX_UP_EULER_180_ROT_FLIPS, FALSE);
			}
		}

		return(TRUE);
	case WM_NOTIFY:
    		switch (((NMHDR FAR *)  (LPARAM) lParam)->code) {
		case PSN_KILLACTIVE:
			// The page is either losing the activation, or the user pressed OK.
           		// OKINO_SetWindowLong(hDlg, DWLP_MSGRESULT, FALSE);	// No error
			break;
                case PSN_APPLY:
			// The Apply button was pressed, or the OK button was pressed
			// and this message is being sent to all panels to have it send
			// the control values to the toolkit.
           		OKINO_SetWindowLong(hDlg, DWLP_MSGRESULT, PSNRET_INVALID);	// Error initially

			/* Only update the UI if the user clicked the 'Apply' button and not the 'Ok' button */
			lppsn = (LPPSHNOTIFY)  (LPARAM) lParam;
			user_clicked_apply_button = !lppsn->lParam;
			if (!user_clicked_apply_button)
				property_sheet_return_value = IDOK;

           		OKINO_SetWindowLong(hDlg, DWLP_MSGRESULT, PSNRET_NOERROR);
			return(TRUE);
		case PSN_QUERYCANCEL:
			// The user has pressed CANCEL. Return FALSE to allow canceling, or TRUE to prevent it
           		OKINO_SetWindowLong(hDlg, DWLP_MSGRESULT, FALSE);
			property_sheet_return_value = IDCANCEL;
			return(FALSE);
		case PSN_RESET:
			// User has pressed CANCEL and property sheet is about to be destroyed
           		OKINO_SetWindowLong(hDlg, DWLP_MSGRESULT, FALSE);
			property_sheet_return_value = IDCANCEL;
			break;
		case PSN_SETACTIVE:
			// Property sheet is about to be activated. We set up the values of the controls here

			/* Enable the 'Apply' button */
			SendMessage(GetParent(hDlg), PSM_CHANGED, (WPARAM) hDlg, 0L);

			// RCL, we don't want to do this here since it will reset all the type-in parameters!
			// SendMessage(hDlg, WM_UPDATE_DIALOG_PARAMETERS, 0, 0L);

			/* Update the enable state of the controls */
			SendMessage(hDlg, WM_UPDATE_DIALOG_ENABLES, 0, 0L);

			break;
                case PSN_HELP:
			// The Help button was pressed
			Import_IO_DisplayHelpTopic(hDlg, plugin_directory, HELP_FILENAME, "main");
			break;
		default:
			return FALSE;
	    	}
		break;
	case WM_COMMAND:
		cmd_id = LOWORD(wParam);
		switch (cmd_id) {
			case IDD_BUTTON_ABOUT:
				DialogBox(hDLLinstance, "AboutBox", parent_hwnd, (DLGPROC) AboutDlgProc);
				return(TRUE);
			case IDD_BUTTON_RESET:
				if (IDYES == MessageBox(hDlg, "This will reset the values of this dialog box to their power-up defaults.\n\nAre you sure you want to do this?", "Confirmation Request", MB_APPLMODAL | MB_YESNO | MB_DEFBUTTON1 | MB_ICONQUESTION)) {
					export_options->options_initialized = FALSE;
					NI_Exporter_InitExportOptionVariables();
					SendMessage(hDlg, WM_UPDATE_DIALOG_PARAMETERS, 0, 0L);
				}
				return(TRUE);

			case IDD_EXPORTER_ENA_OBJECT_ANIMATION:
				export_options->ena_object_animation = !export_options->ena_object_animation;
				CheckDlgButton(hDlg, IDD_EXPORTER_ENA_OBJECT_ANIMATION, export_options->ena_object_animation);
				/* Update the enable state of the controls */
				SendMessage(hDlg, WM_UPDATE_DIALOG_ENABLES, 0, 0L);
				return(TRUE);
			case IDD_EXPORTER_ENA_CAMERA_ANIMATION:
				export_options->ena_camera_animation = !export_options->ena_camera_animation;
				CheckDlgButton(hDlg, IDD_EXPORTER_ENA_CAMERA_ANIMATION, export_options->ena_camera_animation);
				/* Update the enable state of the controls */
				SendMessage(hDlg, WM_UPDATE_DIALOG_ENABLES, 0, 0L);
				return(TRUE);
			case IDD_EXPORTER_ENA_LIGHT_ANIMATION:
				export_options->ena_light_animation = !export_options->ena_light_animation;
				CheckDlgButton(hDlg, IDD_EXPORTER_ENA_LIGHT_ANIMATION, export_options->ena_light_animation);
				return(TRUE);
			case IDD_EXPORTER_ENA_MATERIAL_ANIMATION:
				export_options->ena_material_animation = !export_options->ena_material_animation;
				CheckDlgButton(hDlg, IDD_EXPORTER_ENA_MATERIAL_ANIMATION, export_options->ena_material_animation);
				return(TRUE);
			case IDD_EXPORTER_ANIM_EXPORT_RAW_KEYFRAMES_RB:
				export_options->export_raw_unsampled_anim_data_if_possible = TRUE;
				CheckRadioButton(hDlg, IDD_EXPORTER_ANIM_EXPORT_RAW_KEYFRAMES_RB, IDD_EXPORTER_ANIM_EXPORT_RESAMPLED_KEYFRAMES_RB, cmd_id);
				/* Update the enable state of the controls */
				SendMessage(hDlg, WM_UPDATE_DIALOG_ENABLES, 0, 0L);
				return(TRUE);
			case IDD_EXPORTER_ANIM_EXPORT_RESAMPLED_KEYFRAMES_RB:
				export_options->export_raw_unsampled_anim_data_if_possible = FALSE;
				CheckRadioButton(hDlg, IDD_EXPORTER_ANIM_EXPORT_RAW_KEYFRAMES_RB, IDD_EXPORTER_ANIM_EXPORT_RESAMPLED_KEYFRAMES_RB, cmd_id);
				/* Update the enable state of the controls */
				SendMessage(hDlg, WM_UPDATE_DIALOG_ENABLES, 0, 0L);
				return(TRUE);
			case IDD_EXPORTER_ANIM_EXPORT_DEFAULT_INTERP_METHOD:
				if (GET_WM_COMMAND_CMD(wParam, lParam) == CBN_SELCHANGE) {
			 		wIndex = (UINT) SendDlgItemMessage(hDlg, IDD_EXPORTER_ANIM_EXPORT_DEFAULT_INTERP_METHOD, CB_GETCURSEL, 0, 0L);
					if (!wIndex) 
						export_options->interpolation_type_to_use_during_export = Nc_NRS_ANIM_INTERPOLATION_TYPE_ORIGINAL;
					else if (wIndex == 1)
						export_options->interpolation_type_to_use_during_export = Nc_NRS_ANIM_INTERPOLATION_TYPE_CONSTANT;
					else if (wIndex == 2)
						export_options->interpolation_type_to_use_during_export = Nc_NRS_ANIM_INTERPOLATION_TYPE_LINEAR;
					else if (wIndex == 3)
						export_options->interpolation_type_to_use_during_export = Nc_NRS_ANIM_INTERPOLATION_TYPE_TCB;
					else if (wIndex == 4)
						export_options->interpolation_type_to_use_during_export = Nc_NRS_ANIM_INTERPOLATION_TYPE_BEZIER;
				}
				return(TRUE);
			case IDD_EXPORTER_ANIM_EXPORT_DEFAULT_ROT_CNTRL:
				if (GET_WM_COMMAND_CMD(wParam, lParam) == CBN_SELCHANGE) {
				 	wIndex = (UINT) SendDlgItemMessage(hDlg, IDD_EXPORTER_ANIM_EXPORT_DEFAULT_ROT_CNTRL, CB_GETCURSEL, 0, 0L);
					if (!wIndex) 
						export_options->rotation_controller_type_to_use_during_export = Nc_NRS_ROTATION_CONTROLLER_QUATERNION;
					else if (wIndex == 1)
						export_options->rotation_controller_type_to_use_during_export = Nc_NRS_ROTATION_CONTROLLER_EULER;
					else if (wIndex == 2)
						export_options->rotation_controller_type_to_use_during_export = Nc_NRS_ROTATION_CONTROLLER_AS_PER_NRS_TOOLKIT;
				}
				/* Update the enable state of the controls */
				SendMessage(hDlg, WM_UPDATE_DIALOG_ENABLES, 0, 0L);
				return(TRUE);
			case IDD_RESAMPLING_AND_REDUCTION_OPTIONS_BUTTON:
				// Show the common animation resampling and reduction options
				// dialog box, which is hosted in the Import IO library.
				{ char ini_filepath[300];

				strcpy(ini_filepath, plugin_directory);
				strcat(ini_filepath, INI_FILENAME);

				Import_IO_ShowCommonAnimationResamplingAndReductionOptionsDialogBox(hDlg, FALSE, ini_filepath);
				}
				return(TRUE);
			case IDD_EXPORTER_ROT_ORDER_COMBO:
				if (GET_WM_COMMAND_CMD(wParam, lParam) == CBN_SELCHANGE) {
				 	wIndex = (UINT) SendDlgItemMessage(hDlg, IDD_EXPORTER_ROT_ORDER_COMBO, CB_GETCURSEL, 0, 0L);
					if (wIndex >= 0)
						 export_options->ena_object_anim_resampled_rot_order = euler_order_struct[wIndex].order;
				}
				return (TRUE);
			case IDD_EXPORTER_ANIM_EXPORT_TRIPLE_CHANNELS_AS_SEP_CTRLS:
				export_options->use_independent_controllers_for_tripled_anim_channels = !export_options->use_independent_controllers_for_tripled_anim_channels;
				CheckDlgButton(hDlg, IDD_EXPORTER_ANIM_EXPORT_TRIPLE_CHANNELS_AS_SEP_CTRLS, export_options->use_independent_controllers_for_tripled_anim_channels);
				return (TRUE);
			case IDD_EXPORTER_FIX_UP_EULER_180_ROT_FLIPS:
				export_options->flip_Euler_180flips_via_resampler = !export_options->flip_Euler_180flips_via_resampler;
				CheckDlgButton(hDlg, IDD_EXPORTER_FIX_UP_EULER_180_ROT_FLIPS, export_options->flip_Euler_180flips_via_resampler);
				if (export_options->flip_Euler_180flips_via_resampler)
					MessageBox(hDlg, "Enabling this option will automatically cause imported rotation keys to always be 'resampled'.", "Notice", MB_OK | MB_APPLMODAL);
				return (TRUE);
			default:
				break;
		}
	}
	return (FALSE);
}

// ----------------->>>>  "About" Dialog Box Handler  <<<<-------------------

// This is the dialog box handler for the "About" box

	static INT_PTR CALLBACK
AboutDlgProc(HWND hDlg, UINT msg, WPARAM wparam, LPARAM lparam)
{
	switch (msg) {
	case WM_INITDIALOG:
		Import_IO_Center_Window(parent_hwnd, hDlg);
		SendMessage(hDlg, WM_UPDATE_DIALOG_PARAMETERS, 0, 0L);
		return (TRUE);
	case WM_UPDATE_DIALOG_PARAMETERS:
		SetDlgItemText( hDlg, IDD_OSG_EXPORTER_VERSION, export_options->osgExporterVersion );
		return (TRUE);
	case WM_COMMAND:
		if (LOWORD(wparam) == IDOK) {
			EndDialog(hDlg, TRUE);
			return (TRUE);
		} else if (LOWORD(wparam) == IDD_BUTTON_HELP) {
			Import_IO_DisplayHelpTopic(hDlg, plugin_directory, HELP_FILENAME, "main");
			return (TRUE);
		}
	}
	return (FALSE);
}

