/*****************************************************************************

	This is the common source code that can be used to create
	Okino COM client applications. See the "Okino Plug-Ins SDK",
	 COM Interface section, for explanations about this file.


     This file implements the 3D batch conversion job handling functions.


  Copyright (c) 1988, 2002 Okino Computer Graphics, Inc. All Rights Reserved.

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

	Last change:  RCL  12 Nov 2001    5:18 am
*****************************************************************************/

#include "COMHelper/shared_okino_com_src.h"
#include "COMHelper/shared_okino_com_src_resources.h"

// These are only defined for the 'testclient' example program. 'COMPILING_TEST_CLIENT' 
// is defined in the VC++ project settins, under compiler options. 
#ifdef COMPILING_TEST_CLIENT
#define		PROGRAM_INI_FILENAME	"okino_testcomclient.ini"
extern void	NI_UpdateBatchConversionListViewWithCurrentJobs(HWND hWndList, Nd_Com_Batch_Converter_JobInfoStruct *job_list);
extern HWND     hWndBatchConversionListView;	// "Job Done" event sink needs to see this globally
#endif

// ----------------------->>>>  Definitions   <<<<--------------------------

// When the user selects a file to import, this dialog box pops up to allow the user to select
// the export file format and all options to be used during the batch conversion process of this
// select input file.

struct {
	short	lock_import_and_export_directories;
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
		short	x_antialias;		// Scanline X anti-aliasing
		short	y_antialias;		// Scanline Y anti-aliasing
		char	renderlevel[20];	// Rendering level "scanline" or "raytrace"
		char 	format_str[20];		// File format desired, such as "jpeg", "tiff"
	} render;
} batch_convert_options;

static struct AntiAliasParams {
	char	*desc;
	short	x, y;
} antialias_params[] = {
	{ "No anti-aliasing", 		0, 0 },
	{ "x=1, y=2 (Ok)", 		1, 2 }, 
	{ "x=2, y=2 (Better)", 		2, 2 }, 
	{ "x=1, y=4 (Good)", 		1, 4 }, 
	{ "x=2, y=4 (Very good)",	2, 4 }, 
	{ NULL, 0, 0 }
};

/* Bitmap extensions and filetypes */
static struct BITMAP_NAME_STRUCT {
	char		*desc;
	char		*extension;
} bitmap_format_struct[] = {
	{ "BMP", ".bmp" },
	{ "JPEG", ".jpg" },
	{ "EIAS", ".img" },
	{ "IFF", ".iff" },
	{ "PIC", ".pic" },
	{ "PPM", ".ppm" },
	{ "PSD", ".psd" },
	{ "SGI", ".sgi" },
	{ "Targa", ".tga" },
	{ "TIFF", ".tif" },
	{ NULL, 0 }
};

// ----------------------->>>>  Local Variables  <<<<--------------------------

// This is our list of 3D batch conversion jobs that have been queued up
Nd_Com_Batch_Converter_JobInfoStruct *OkinoCommonComSource___BatchConversionJobList = NULL;

// For thread locking while the job list is being modified 
static CRITICAL_SECTION job_list_critical_section;
static short		job_list_critical_section_initialized = FALSE;
static HRESULT		hresult;

// -------------------------------------------------------------------------

// This is the dialog box handler which allows the end-user to select an
// import file, and export file + format, and batch conversion options.

	BOOL WINAPI
OkinoCommonComSource___SelectAndAddNewBatchConversionJobDialogProc(HWND hDlg, UINT msg, UINT wparam, LONG lparam)
{
	USES_CONVERSION;
	static BSTR	return_importer_guid;

	static short	vars_init = FALSE;
	static char	*curr_export_dir = NULL;
	static char	*selected_input_filename;	// Filename selected by the user for import
	static Nd_Export_Converter_Info_List	*curr_exporter_list = NULL;	// List of all known export converters and their attributes queried from the COM server
	static Nd_Import_Converter_Info_List	*curr_importer_list = NULL;	// List of all known import converters and their attributes queried from the COM server
	Nd_Com_Batch_Converter_JobInfoStruct *job_ptr;
	char		*exporter_guid;
	char		temp_buffer[300];
	UINT		wIndex;
	short		i, cmd_id, count;

	switch(msg) {
		case WM_INITDIALOG:
			OkinoCommonComSource___Center_Window(GetParent(hDlg), hDlg);

			// List of all known export converters and their attributes queried from the COM server
			curr_exporter_list = OkinoCommonComSource___GetListofExportersAndTheirAttributesFromCOMServer();
			// List of all known import converters and their attributes queried from the COM server
			curr_importer_list = OkinoCommonComSource___GetListofImportersAndTheirAttributesFromCOMServer();

			// Go ask the user for the input filename
			selected_input_filename = OkinoCommonComSource___SelectGeometryImportFilename(hDlg);
			if (!selected_input_filename) {
				EndDialog(hDlg, FALSE);
				return (TRUE);
			}

			// Ask the COM server which importer is best suited for the input file chosen.
			if (OkinoCommonComSource___Check_For_File_Extension(selected_input_filename, ".bdf")) {
				SetDlgItemText(hDlg, IDD_BATCHCONVERT_INPUT_FILE_FORMAT, "Okino .bdf scene file format");
				return_importer_guid = NULL;
			} else {
				if (!OkinoCommonComSource___DetermineImporterGUIDBasedOnFilenameOnly(hDlg, selected_input_filename, &return_importer_guid)) {
					// If the COM server cannot determine the appropriate import converter, then tell the user that we can't continue
					EndDialog(hDlg, FALSE);
					return (TRUE);
				} 
				SetDlgItemText(hDlg, IDD_BATCHCONVERT_INPUT_FILENAME, selected_input_filename);
				for (i=0; i < curr_importer_list[0].num_importers_in_list; ++i) {
					// Find the importer in our current importer list to find its "description" string
					if (!strcmp(curr_importer_list[i].guid, OLE2T(return_importer_guid))) {
						SetDlgItemText(hDlg, IDD_BATCHCONVERT_INPUT_FILE_FORMAT, curr_importer_list[i].menu_description);
						break;
					}
				}
			}

			// Fill in the export combo box with all the known importers
			SendDlgItemMessage(hDlg, IDD_BATCHCONVERT_EXPORT_TYPE_COMBO, CB_RESETCONTENT, 0, 0L);
			SendDlgItemMessage(hDlg, IDD_BATCHCONVERT_EXPORT_TYPE_COMBO, WM_SETREDRAW, FALSE, 0L);
			// First entry is special, Okino .bdf format
			SendDlgItemMessage(hDlg, IDD_BATCHCONVERT_EXPORT_TYPE_COMBO, CB_ADDSTRING, (WPARAM) NULL, (LPARAM)(LPSTR) "Okino .bdf scene files (*.bdf)");
			for (i=0; i < curr_exporter_list[0].num_exporters_in_list; ++i) {
				char	str[256];

				sprintf(str, "%s (%s)", curr_exporter_list[i].file_type_desc, curr_exporter_list[i].file_extensions);
				SendDlgItemMessage(hDlg, IDD_BATCHCONVERT_EXPORT_TYPE_COMBO, CB_ADDSTRING, (WPARAM) NULL, (LPARAM)(LPSTR) str);
			}
			SendDlgItemMessage(hDlg, IDD_BATCHCONVERT_EXPORT_TYPE_COMBO, WM_SETREDRAW, TRUE, 0L);
			// Make the last one in the list current & highlighted
			SendDlgItemMessage(hDlg, IDD_BATCHCONVERT_EXPORT_TYPE_COMBO, CB_SETCURSEL, curr_exporter_list[0].num_exporters_in_list, 0L);

			if (!vars_init) {
				batch_convert_options.lock_import_and_export_directories = FALSE;
				batch_convert_options.resize_model_to_viewport_upon_import= TRUE;
				batch_convert_options.add_lights_to_scene_if_none= TRUE;
				batch_convert_options.perform_polygon_processing_on_converted_model= FALSE;
				batch_convert_options.perform_polygon_reduction_on_converted_model= FALSE;
				batch_convert_options.create_backup_file = FALSE;
				batch_convert_options.compress_all_children_of_each_grouping_folder_to_single_object = FALSE;
				batch_convert_options.hierarchy_optimizer1 = FALSE;
				batch_convert_options.hierarchy_optimizer2 = FALSE;
				batch_convert_options.apply_global_transform = FALSE;
				batch_convert_options.make_batch_job_conversion_viewable_in_view_ports = TRUE;

				// Set up the image snapshot rendering options
				batch_convert_options.render.enable = TRUE;
				batch_convert_options.render.res_x = 120;
				batch_convert_options.render.res_y = 90;
				strcpy(batch_convert_options.render.renderlevel, "scanline");
				strcpy(batch_convert_options.render.format_str, ".jpg");
				batch_convert_options.render.x_antialias = 1;
				batch_convert_options.render.y_antialias = 4;

                                vars_init = TRUE;
			}

			if (batch_convert_options.lock_import_and_export_directories)
				OkinoCommonComSource___ExtractBaseFilePath(selected_input_filename, temp_buffer);
			else
#ifdef PROGRAM_INI_FILENAME
				GetPrivateProfileString("DefaultDirectories", "Default-Export-Directory", "c:\\", temp_buffer, sizeof(temp_buffer), PROGRAM_INI_FILENAME);
#else
				strcpy(temp_buffer, "c:\\");
#endif
			SendDlgItemMessage(hDlg, IDD_BATCHCONVERT_EXPORT_DIRECTORY_EDIT, WM_SETTEXT, 0, (LONG)(LPSTR) temp_buffer);
			OkinoCommonComSource___Copy_String(&curr_export_dir, temp_buffer);

			CheckDlgButton(hDlg, IDD_BATCHCONVERT_LOCK_IMPORT_EXPORT_DIRS, batch_convert_options.lock_import_and_export_directories);
			CheckDlgButton(hDlg, IDD_BATCHCONVERT_RESIZEMODELTOEXTENTS,batch_convert_options.resize_model_to_viewport_upon_import);
			CheckDlgButton(hDlg, IDD_BATCHCONVERT_ADDLIGHTSTOSCENEIFNONE,batch_convert_options.add_lights_to_scene_if_none);
			CheckDlgButton(hDlg, IDD_BATCHCONVERT_DOPOLYPROCESSING,batch_convert_options.perform_polygon_processing_on_converted_model);
			CheckDlgButton(hDlg, IDD_BATCHCONVERT_DOPOLYREDUCTION,batch_convert_options.perform_polygon_reduction_on_converted_model);
			CheckDlgButton(hDlg, IDD_BATCHCONVERT_CREATEBACKUPSDURINGEXPORT,batch_convert_options.create_backup_file);
			CheckDlgButton(hDlg, IDD_BATCHCONVERT_COMPRESS_CHILDREN_ENA, batch_convert_options.compress_all_children_of_each_grouping_folder_to_single_object);
			CheckDlgButton(hDlg, IDD_BATCHCONVERT_ENA_HIER_OPT1, batch_convert_options.hierarchy_optimizer1);
			CheckDlgButton(hDlg, IDD_BATCHCONVERT_ENA_HIER_OPT2, batch_convert_options.hierarchy_optimizer2);
			CheckDlgButton(hDlg, IDD_BATCHCONVERT_APPLY_GLOBAL_XFORM_ENA, batch_convert_options.apply_global_transform);
			CheckDlgButton(hDlg, IDD_BATCHCONVERT_PERFORM_CONVERSION_VISIBLE_IN_VIEWPORTS, batch_convert_options.make_batch_job_conversion_viewable_in_view_ports);
			CheckDlgButton(hDlg, IDD_BATCHCONVERT_RENDER_SNAPSHOT_ENABLE, batch_convert_options.render.enable);

			OkinoCommonComSource___NdInt_To_Dialog(hDlg, IDD_BATCHCONVERT_RENDER_RES_X, batch_convert_options.render.res_x);
			OkinoCommonComSource___NdInt_To_Dialog(hDlg, IDD_BATCHCONVERT_RENDER_RES_Y, batch_convert_options.render.res_y);

			if (!strcmp(batch_convert_options.render.renderlevel, "scanline"))
				cmd_id = IDD_BATCHCONVERT_RENDERLEVEL_SCANLINE_RB;
			else
				cmd_id = IDD_BATCHCONVERT_RENDERLEVEL_RAYTRACE_RB;
			CheckRadioButton(hDlg, IDD_BATCHCONVERT_RENDERLEVEL_SCANLINE_RB, IDD_BATCHCONVERT_RENDERLEVEL_RAYTRACE_RB, cmd_id);

			/* Reset the contents of the anti-alias combo box */
			SendDlgItemMessage(hDlg, IDD_BATCHCONVERT_RENDER_SCANLINE_AA_COMBO, CB_RESETCONTENT, 0, 0L);
			SendDlgItemMessage(hDlg, IDD_BATCHCONVERT_RENDER_SCANLINE_AA_COMBO, WM_SETREDRAW, FALSE, 0L);
			count = 0;
			while (antialias_params[count].desc != (char *) NULL) {
			SendDlgItemMessage(hDlg, IDD_BATCHCONVERT_RENDER_SCANLINE_AA_COMBO, CB_ADDSTRING, (WPARAM) NULL,
					(LPARAM)(LPSTR) antialias_params[count].desc);
				++count;
			}
			SendDlgItemMessage(hDlg, IDD_BATCHCONVERT_RENDER_SCANLINE_AA_COMBO, WM_SETREDRAW, TRUE, 0L);

			/* Update the current selection in the anti-alias combo box */
			count = 0;
			SendDlgItemMessage(hDlg, IDD_BATCHCONVERT_RENDER_SCANLINE_AA_COMBO, CB_SETCURSEL, -1, 0L);
			while (antialias_params[count].desc != (char *) NULL) {
				if (batch_convert_options.render.x_antialias == antialias_params[count].x &&
						 batch_convert_options.render.y_antialias == antialias_params[count].y) {
					SendDlgItemMessage(hDlg, IDD_BATCHCONVERT_RENDER_SCANLINE_AA_COMBO, CB_SETCURSEL, count, 0L);
					break;
				} else
					++count;
			}

			/* Update the bitmap file formats drop-down boxes */
			SendDlgItemMessage(hDlg, IDD_BATCHCONVERT_RENDER_FILE_FORMAT_COMBO, CB_RESETCONTENT, 0, 0L);
			count = 0;
			while (bitmap_format_struct[count].desc != (char *) NULL) {
				SendDlgItemMessage(hDlg, IDD_BATCHCONVERT_RENDER_FILE_FORMAT_COMBO, CB_ADDSTRING, (WPARAM) NULL, (LPARAM)(LPSTR) bitmap_format_struct[count].extension);
				++count;
			}
			SendDlgItemMessage(hDlg, IDD_BATCHCONVERT_RENDER_FILE_FORMAT_COMBO, WM_SETREDRAW, TRUE, 0L);
			count = 0;
			while (bitmap_format_struct[count].desc != (char *) NULL) {
				if (!strncmp(bitmap_format_struct[count].extension, batch_convert_options.render.format_str, 4))
					SendDlgItemMessage(hDlg, IDD_BATCHCONVERT_RENDER_FILE_FORMAT_COMBO, CB_SETCURSEL, count, 0L);
				++count;
			}

			return(TRUE);
		case WM_DESTROY:
			if (curr_exporter_list)
				OkinoCommonComSource___FreeListofExportersAndTheirAttributesObtainedFromCOMServer(curr_exporter_list);
			curr_exporter_list = NULL;
			if (curr_importer_list)
				OkinoCommonComSource___FreeListofImportersAndTheirAttributesObtainedFromCOMServer(curr_importer_list);
			curr_importer_list = NULL;
			if (curr_export_dir)
				free((char *) curr_export_dir);
			curr_export_dir = NULL;
			return (TRUE);
		case WM_COMMAND:
			cmd_id = LOWORD(wparam);
			switch(cmd_id) {
				case IDOK:
					{ long temp_long;

					// Determine which exporter in the combo box has been selected
				 	wIndex = (UINT) SendDlgItemMessage(hDlg, IDD_BATCHCONVERT_EXPORT_TYPE_COMBO, CB_GETCURSEL, 0, 0L);
					if (wIndex >= 1)
						exporter_guid = curr_exporter_list[wIndex-1].guid;
					else
						// Special case, first entry is the Okino .bdf file format
						exporter_guid = Nc_BDF_FILE_FORMAT_GUID;

					// Get the X and Y resolution

					OkinoCommonComSource___Dialog_To_NdInt(hDlg, IDD_BATCHCONVERT_RENDER_RES_X, &temp_long);
					batch_convert_options.render.res_x = (short) temp_long;
					OkinoCommonComSource___Dialog_To_NdInt(hDlg, IDD_BATCHCONVERT_RENDER_RES_Y, &temp_long);
					batch_convert_options.render.res_y = (short) temp_long;

					// Allocate a new batch conversion job struct
					job_ptr = OkinoCommonComSource___Alloc_Com_Batch_Converter_JobInfoStruct(&OkinoCommonComSource___BatchConversionJobList);
					if (job_ptr) {
						job_ptr->job_id = NULL;	// Assigned by COM server

						// Input filename
						OkinoCommonComSource___Copy_String(&job_ptr->filename_and_filepath, selected_input_filename);
						// Exporter GUID
						if (exporter_guid)
							OkinoCommonComSource___Copy_String(&job_ptr->Exporter_GUID, exporter_guid);
						else
							job_ptr->Exporter_GUID = NULL;
						// Importer GUID which the COM server just qualified for us. 
						if (return_importer_guid)
							OkinoCommonComSource___Copy_String(&job_ptr->Importer_GUID, OLE2T(return_importer_guid));
						else
							job_ptr->Importer_GUID = NULL;
						// No file to execute yet
						job_ptr->file_to_execute_when_conversion_done = NULL;
						// A descriptive name for the exporter. Only used in this test client for the list view control.
						if (wIndex >= 1)
							OkinoCommonComSource___Copy_String(&job_ptr->exporter_descriptive_name, curr_exporter_list[wIndex-1].file_type_desc);
						else
							// Special case, first entry is the Okino .bdf file format
							OkinoCommonComSource___Copy_String(&job_ptr->exporter_descriptive_name, "Okino .bdf scene");

						// Export directory
						if (batch_convert_options.lock_import_and_export_directories) {
							OkinoCommonComSource___ExtractBaseFilePath(selected_input_filename, temp_buffer);
							OkinoCommonComSource___Copy_String(&job_ptr->export_directory, temp_buffer);
						} else
							OkinoCommonComSource___Copy_String(&job_ptr->export_directory, curr_export_dir);

						job_ptr->resize_model_to_viewport_upon_import = batch_convert_options.resize_model_to_viewport_upon_import;
						job_ptr->add_lights_to_scene_if_none = batch_convert_options.add_lights_to_scene_if_none;
						job_ptr->perform_polygon_processing_on_converted_model = batch_convert_options.perform_polygon_processing_on_converted_model;
						job_ptr->perform_polygon_reduction_on_converted_model = batch_convert_options.perform_polygon_reduction_on_converted_model;
						job_ptr->create_backup_file = batch_convert_options.create_backup_file;
						job_ptr->compress_all_children_of_each_grouping_folder_to_single_object = batch_convert_options.compress_all_children_of_each_grouping_folder_to_single_object;
						job_ptr->hierarchy_optimizer1 = batch_convert_options.hierarchy_optimizer1;
						job_ptr->hierarchy_optimizer2 = batch_convert_options.hierarchy_optimizer2;
						job_ptr->apply_global_transform = batch_convert_options.apply_global_transform;
						job_ptr->make_batch_job_conversion_viewable_in_view_ports = batch_convert_options.make_batch_job_conversion_viewable_in_view_ports;

						// Copy the image snapshot rendering options
						job_ptr->render.enable = batch_convert_options.render.enable;
						job_ptr->render.res_x = batch_convert_options.render.res_x;
						job_ptr->render.res_y = batch_convert_options.render.res_y;
						strcpy(job_ptr->render.renderlevel, batch_convert_options.render.renderlevel);
						strcpy(job_ptr->render.format_str, batch_convert_options.render.format_str);
						job_ptr->render.x_antialias = batch_convert_options.render.x_antialias;
						job_ptr->render.y_antialias = batch_convert_options.render.y_antialias;

						// Go and submit the job to the COM server
						if (!OkinoCommonComSource___Submit3DBatchConversionJobToCOMServer(job_ptr)) {
							EndDialog(hDlg, FALSE);
							return (TRUE);
						}
					}

					EndDialog(hDlg, TRUE);
					}
					return (TRUE);
				case IDCANCEL:
					EndDialog(hDlg, FALSE);
					return (TRUE);
				case IDD_BATCHCONVERT_RENDER_SCANLINE_AA_COMBO:
					/* User has chosen an anti-alias level from the combo box */
					if (GET_WM_COMMAND_CMD(wparam, lparam) == CBN_SELCHANGE) {
						wIndex = (UINT) SendDlgItemMessage(hDlg, IDD_BATCHCONVERT_RENDER_SCANLINE_AA_COMBO, CB_GETCURSEL, 0, 0L);

						if (!antialias_params[wIndex].x && !antialias_params[wIndex].y) {
							batch_convert_options.render.x_antialias = batch_convert_options.render.y_antialias = 0;
						} else {
							/* List-selected resolution */
							batch_convert_options.render.x_antialias = antialias_params[wIndex].x;
							batch_convert_options.render.y_antialias = antialias_params[wIndex].y;
						}
					}
					return(TRUE);
				case IDD_BATCHCONVERT_RENDER_FILE_FORMAT_COMBO:
					if (GET_WM_COMMAND_CMD(wparam, lparam) == CBN_SELCHANGE) {
					 	wIndex = (UINT) SendDlgItemMessage(hDlg, IDD_BATCHCONVERT_RENDER_FILE_FORMAT_COMBO, CB_GETCURSEL, 0, 0L);
						if (wIndex >= 0)
							strcpy(batch_convert_options.render.format_str, bitmap_format_struct[wIndex].extension);
					}
					return(TRUE);
				case IDD_BATCHCONVERT_SHOWEXPORTEROPTIONS:
					// Go show one of the export converter options dialog boxes
					{ _bstr_t bstr_guid, dialog_box_name_bstr;

					// Determine which exporter in the combo box has been selected
				 	wIndex = (UINT) SendDlgItemMessage(hDlg, IDD_BATCHCONVERT_EXPORT_TYPE_COMBO, CB_GETCURSEL, 0, 0L);
					if (!wIndex)
						// Special case: first entry is the Okino .bdf format
						break;
					bstr_guid = curr_exporter_list[wIndex-1].guid;

					// Show the export converter dialog box
					dialog_box_name_bstr = "ExportConverterOptionsDialogBox";
					try {
						short		com_error_result;

loop:						hresult = pNuGrafIO->ShowInternalDialogBox(dialog_box_name_bstr, (long) hDlg, bstr_guid, NULL, 0, 0);
						// E_FAIL means that S_FALSE is being returned from the dialog box
						if (hresult == E_FAIL)
							hresult = S_FALSE;

						// If fatal, ask the user what to do: retry, restart or abort. If restart, try restarting the server,
						com_error_result = OkinoCommonComSource___Check_COM_HRESULT_Error_CODE(hresult, "Error while asking COM server to show an export converter options dialog box. COM server has either crashed or is waiting for a user response.");
						if (com_error_result == IDC_SERVERFAULT_RETRY_OPERATION)
							goto loop;
						else if (com_error_result == IDC_SERVERFAULT_CANCEL_OPERATION)
							return(FALSE);
					} catch (_com_error &e) {
						OkinoCommonComSource___Display_COM_Error(e, "Error while asking COM server to show an export converter options dialog box. COM server has either crashed or is waiting for a user response.");
						return(FALSE);
					}
					}
					return (TRUE);
				case IDD_BATCHCONVERT_SHOW_IMPORT_OPTIONS:
					// Go show one of the import converter options dialog boxes
					{ _bstr_t bstr_guid, dialog_box_name_bstr;

					if (!selected_input_filename)
						break;
					if (!return_importer_guid)
						break;

					// Determine which exporter in the combo box has been selected
					if (OkinoCommonComSource___Check_For_File_Extension(selected_input_filename, ".bdf"))
						// Special case: first entry is the Okino .bdf format
						break;
					bstr_guid = return_importer_guid;

					// Show the import converter dialog box
					dialog_box_name_bstr = "ImportConverterOptionsDialogBox";
					try {
						short		com_error_result;

loop2:						hresult = pNuGrafIO->ShowInternalDialogBox(dialog_box_name_bstr, (long) hDlg, bstr_guid, NULL, 0, 0);
						// E_FAIL means that S_FALSE is being returned from the dialog box
						if (hresult == E_FAIL)
							hresult = S_FALSE;

						// If fatal, ask the user what to do: retry, restart or abort. If restart, try restarting the server,
						com_error_result = OkinoCommonComSource___Check_COM_HRESULT_Error_CODE(hresult, "Error while asking COM server to show an import converter options dialog box. COM server has either crashed or is waiting for a user response.");
						if (com_error_result == IDC_SERVERFAULT_RETRY_OPERATION)
							goto loop2;
						else if (com_error_result == IDC_SERVERFAULT_CANCEL_OPERATION)
							return(FALSE);
					} catch (_com_error &e) {
						OkinoCommonComSource___Display_COM_Error(e, "Error while asking COM server to show an import converter options dialog box. COM server has either crashed or is waiting for a user response.");
						return(FALSE);
					}
					}
					return (TRUE);
				case IDD_BATCHCONVERT_LOCK_IMPORT_EXPORT_DIRS:
					batch_convert_options.lock_import_and_export_directories = !batch_convert_options.lock_import_and_export_directories;
					CheckDlgButton(hDlg, IDD_BATCHCONVERT_LOCK_IMPORT_EXPORT_DIRS, batch_convert_options.lock_import_and_export_directories);

					if (batch_convert_options.lock_import_and_export_directories) {
						OkinoCommonComSource___ExtractBaseFilePath(selected_input_filename, temp_buffer);
						SendDlgItemMessage(hDlg, IDD_BATCHCONVERT_EXPORT_DIRECTORY_EDIT, WM_SETTEXT, 0, (LONG)(LPSTR) temp_buffer);
						OkinoCommonComSource___Copy_String(&curr_export_dir, temp_buffer);
					}
					return (TRUE);
				case IDD_BATCHCONVERT_RESIZEMODELTOEXTENTS:
					batch_convert_options.resize_model_to_viewport_upon_import = !batch_convert_options.resize_model_to_viewport_upon_import;
					CheckDlgButton(hDlg, IDD_BATCHCONVERT_RESIZEMODELTOEXTENTS,batch_convert_options.resize_model_to_viewport_upon_import);
					return (TRUE);
				case IDD_BATCHCONVERT_ADDLIGHTSTOSCENEIFNONE:
					batch_convert_options.add_lights_to_scene_if_none = !batch_convert_options.add_lights_to_scene_if_none;
					CheckDlgButton(hDlg, IDD_BATCHCONVERT_ADDLIGHTSTOSCENEIFNONE,batch_convert_options.add_lights_to_scene_if_none);
					return (TRUE);
				case IDD_BATCHCONVERT_PERFORM_CONVERSION_VISIBLE_IN_VIEWPORTS:
					batch_convert_options.make_batch_job_conversion_viewable_in_view_ports = !batch_convert_options.make_batch_job_conversion_viewable_in_view_ports;
					CheckDlgButton(hDlg, IDD_BATCHCONVERT_PERFORM_CONVERSION_VISIBLE_IN_VIEWPORTS, batch_convert_options.make_batch_job_conversion_viewable_in_view_ports);
					return (TRUE);
				case IDD_BATCHCONVERT_DOPOLYPROCESSING:
					batch_convert_options.perform_polygon_processing_on_converted_model = !batch_convert_options.perform_polygon_processing_on_converted_model;
					CheckDlgButton(hDlg, IDD_BATCHCONVERT_DOPOLYPROCESSING,batch_convert_options.perform_polygon_processing_on_converted_model);
					return (TRUE);
				case IDD_BATCHCONVERT_DOPOLYREDUCTION:
					batch_convert_options.perform_polygon_reduction_on_converted_model = !batch_convert_options.perform_polygon_reduction_on_converted_model;
					CheckDlgButton(hDlg, IDD_BATCHCONVERT_DOPOLYREDUCTION,batch_convert_options.perform_polygon_reduction_on_converted_model);
					return (TRUE);
				case IDD_BATCHCONVERT_CREATEBACKUPSDURINGEXPORT:
					batch_convert_options.create_backup_file = !batch_convert_options.create_backup_file;
					CheckDlgButton(hDlg, IDD_BATCHCONVERT_CREATEBACKUPSDURINGEXPORT,batch_convert_options.create_backup_file);
					return (TRUE);
				case IDD_BATCHCONVERT_SHOW_POLYPROCESSING_OPTIONS:
					// Go show the polygon procssing options dialog box in the host program
					OkinoCommonComSource___ShowInternalDialogBox(hDlg, "PolygonProcessingOptionsDialogBox");
					return (TRUE);
				case IDD_BATCHCONVERT_SHOW_POLYREDUCTION_OPTIONS:
					// Go show the polygon reduction options dialog box in the host program
					OkinoCommonComSource___ShowInternalDialogBox(hDlg, "PolygonReductionOptionsDialogBox");
					return (TRUE);
				case IDD_BATCHCONVERT_EXPORT_DIRECTORY_BROWSE:
					if (OkinoCommonComSource___NewWindowsBrowseForFolder2(hDlg, &curr_export_dir, "Select new export directory", "Directory Path")) {
#ifdef COMPILING_TEST_CLIENT
						WritePrivateProfileString("DefaultDirectories", "Default-Export-Directory", curr_export_dir, PROGRAM_INI_FILENAME);
#endif
						SendDlgItemMessage(hDlg, IDD_BATCHCONVERT_EXPORT_DIRECTORY_EDIT, WM_SETTEXT, 0, (LONG)(LPSTR) curr_export_dir);
					}
					return (TRUE);
				case IDD_BATCHCONVERT_SHOW_COMPRESS_CHILDREN_OPTIONS:
					// Go show the compress children options dialog box
					OkinoCommonComSource___ShowInternalDialogBox(hDlg, "MoveChildrenOfYellowFolderOptionsDialog");
					return (TRUE);
				case IDD_BATCHCONVERT_SHOW_GLOBAL_XFORM_OPTIONS:
					// Go show the global batch translation transformation
					OkinoCommonComSource___ShowInternalDialogBox(hDlg, "GlobalTransformUsedDuringBatchTranslations");
					return (TRUE);
				case IDD_BATCHCONVERT_COMPRESS_CHILDREN_ENA:
					batch_convert_options.compress_all_children_of_each_grouping_folder_to_single_object = !batch_convert_options.compress_all_children_of_each_grouping_folder_to_single_object;
					CheckDlgButton(hDlg, IDD_BATCHCONVERT_COMPRESS_CHILDREN_ENA, batch_convert_options.compress_all_children_of_each_grouping_folder_to_single_object);
					return (TRUE);
				case IDD_BATCHCONVERT_ENA_HIER_OPT1:
					batch_convert_options.hierarchy_optimizer1 = !batch_convert_options.hierarchy_optimizer1;
					CheckDlgButton(hDlg, IDD_BATCHCONVERT_ENA_HIER_OPT1, batch_convert_options.hierarchy_optimizer1);
					return (TRUE);
				case IDD_BATCHCONVERT_ENA_HIER_OPT2:
					batch_convert_options.hierarchy_optimizer2 = !batch_convert_options.hierarchy_optimizer2;
					CheckDlgButton(hDlg, IDD_BATCHCONVERT_ENA_HIER_OPT2, batch_convert_options.hierarchy_optimizer2);
					return (TRUE);
				case IDD_BATCHCONVERT_APPLY_GLOBAL_XFORM_ENA:
					batch_convert_options.apply_global_transform = !batch_convert_options.apply_global_transform;
					CheckDlgButton(hDlg, IDD_BATCHCONVERT_APPLY_GLOBAL_XFORM_ENA, batch_convert_options.apply_global_transform);
					return (TRUE);
				case IDD_BATCHCONVERT_RENDER_SNAPSHOT_ENABLE:
					batch_convert_options.render.enable = !batch_convert_options.render.enable;
					CheckDlgButton(hDlg, IDD_BATCHCONVERT_RENDER_SNAPSHOT_ENABLE, batch_convert_options.render.enable);
					return (TRUE);
				case IDD_BATCHCONVERT_RENDERLEVEL_SCANLINE_RB:
					strcpy(batch_convert_options.render.renderlevel, "scanline");
					CheckRadioButton(hDlg, IDD_BATCHCONVERT_RENDERLEVEL_SCANLINE_RB, IDD_BATCHCONVERT_RENDERLEVEL_RAYTRACE_RB, cmd_id);
					return (TRUE);
				case IDD_BATCHCONVERT_RENDERLEVEL_RAYTRACE_RB:
					strcpy(batch_convert_options.render.renderlevel, "raytrace");
					CheckRadioButton(hDlg, IDD_BATCHCONVERT_RENDERLEVEL_SCANLINE_RB, IDD_BATCHCONVERT_RENDERLEVEL_RAYTRACE_RB, cmd_id);
					return (TRUE);
				default:
					break;
			}
	}

	/* Returning FALSE signifies that we didn't process any messages */
	return(FALSE);
}

// ========================================================================
// Batch Job Queue Handling Code.
//
// The following help code maintains in internal linked list of all jobs
// submitted to the batch conversion queue. Every time a new job is added
// a new node is added to the linked list. Each job is also submitted to the
// COM server which maintains its own internal job queue. Each job is assigned
// a GUID by the COM server which allows us (the test client) to keep sync'd
// with the COM server. When the COM server finishes the conversion it informs
// us that the job has completed (by passing us the GUID) and by running an
// external .bat or .exe file which we can specify.
//
// ========================================================================

// Allocate a new 3d batch conversion job queue object

	Nd_Com_Batch_Converter_JobInfoStruct *
OkinoCommonComSource___Alloc_Com_Batch_Converter_JobInfoStruct(Nd_Com_Batch_Converter_JobInfoStruct **batch_list)
{
	Nd_Com_Batch_Converter_JobInfoStruct *ptr, *ptr2;

	ptr = (Nd_Com_Batch_Converter_JobInfoStruct *) malloc(sizeof(Nd_Com_Batch_Converter_JobInfoStruct));
	memset((char *) ptr, 0, sizeof(Nd_Com_Batch_Converter_JobInfoStruct));

	// Since the job list can be accessed from the background "job done" event sink
	// as well as the foreground process, we need critical sections around the job list.
	OkinoCommonComSource___EnterJobListCriticalSection();

	// Add it to the end of the current batch conversion list
	if (*batch_list == NULL)
                *batch_list = ptr;
	else {
		ptr2 = *batch_list;
		while (ptr2->next_ptr)
			ptr2 = ptr2->next_ptr;
		ptr2->next_ptr = ptr;
	}

	OkinoCommonComSource___ExitJobListCriticalSection();

	return(ptr);
}

// Free up a new 3d batch conversion job queue object

	void
OkinoCommonComSource___Free_Com_Batch_Converter_JobInfoStruct(Nd_Com_Batch_Converter_JobInfoStruct *job)
{
	if (!job)
		return;

	if (job->job_id)
		free(job->job_id);
	if (job->filename_and_filepath)
		free(job->filename_and_filepath);
	if (job->export_directory)
		free(job->export_directory);
	if (job->Importer_GUID)
		free(job->Importer_GUID);
	if (job->Exporter_GUID)
		free(job->Exporter_GUID);
	if (job->file_to_execute_when_conversion_done)
		free(job->file_to_execute_when_conversion_done);
	if (job->exporter_descriptive_name)
		free(job->exporter_descriptive_name);
	if (job->job_completed.status_text)
		free(job->job_completed.status_text);
	if (job->job_completed.time_string)
		free(job->job_completed.time_string);
	if (job->job_completed.job_temp_directory)
		free(job->job_completed.job_temp_directory);

	free(job);
}

// Clear out the jobs marked as "done" from the job list, as well as deleting
// their output job files and .jpeg renderings from the Windows TEMP directory.

	void
OkinoCommonComSource___Clear_Done_Jobs_From_List()
{
	Nd_Com_Batch_Converter_JobInfoStruct *ptr, *last_ptr, *temp;

	// Don't forget to remove the "job done" text file for this job in the TEMP directory
	// as well as the .jpg rendering image

	ptr = OkinoCommonComSource___BatchConversionJobList;
	last_ptr = NULL;
	while (ptr) {
		if (ptr->job_completed.done) {
			struct _finddata_t fileinfo;
			long		handle;
			long	 	rc;
			char		temp_path_buffer[512], filespec[512], buf[512];
			short		len;

			if (ptr && strlen(ptr->job_completed.job_temp_directory) && ptr->job_id && strlen(ptr->job_id)) {
				strcpy(temp_path_buffer, ptr->job_completed.job_temp_directory);
				len = strlen(temp_path_buffer);
				if (temp_path_buffer[len-1] != '/' && temp_path_buffer[len-1] != '\\')
					strcat(temp_path_buffer, "\\");
				strcpy(filespec, temp_path_buffer);
				strcat(filespec, ptr->job_id);
				strcat(filespec, "*");

				// Delete all files in the job's output directory which
				// have the same root name as the job ID. 
				rc = handle = _findfirst(filespec, &fileinfo);
				while (rc != -1) {
					/* If the find routine has stripped off the filepath then prepend it again */
					if (fileinfo.name[0] != '\\' && fileinfo.name[0] != '/')
						sprintf(buf, "%s%s", temp_path_buffer, fileinfo.name);
					else
						strcpy(buf, fileinfo.name);
					if (!_access(buf, 0))
						remove(buf);
					rc = _findnext(handle, &fileinfo);
				}
				_findclose(handle);
			}

			if (last_ptr == NULL)
				OkinoCommonComSource___BatchConversionJobList = ptr->next_ptr;
			else
				last_ptr->next_ptr = ptr->next_ptr;

			temp = ptr;
			ptr = ptr->next_ptr;
			OkinoCommonComSource___Free_Com_Batch_Converter_JobInfoStruct(temp);
		} else {
			last_ptr = ptr;
			ptr = ptr->next_ptr;
		}
	}

#ifdef COMPILING_TEST_CLIENT	// Only applicable to the UI of the test client example application
	if (hWndBatchConversionListView)
		NI_UpdateBatchConversionListViewWithCurrentJobs(hWndBatchConversionListView, OkinoCommonComSource___BatchConversionJobList);
#endif
}

// This frees up the entire linked list of jobs that are passed in as the argument

	void
OkinoCommonComSource___Free_Com_Batch_Converter_JobInfo_LinkedList(Nd_Com_Batch_Converter_JobInfoStruct *job_linked_list)
{
	Nd_Com_Batch_Converter_JobInfoStruct *next_ptr;

	// Since the job list can be accessed from the background "job done" event sink
	// as well as the foreground process, we need critical sections around the job list.
	OkinoCommonComSource___EnterJobListCriticalSection();

	while (job_linked_list) {
		next_ptr = job_linked_list->next_ptr;
		OkinoCommonComSource___Free_Com_Batch_Converter_JobInfoStruct(job_linked_list);
		job_linked_list = next_ptr;
	}

	OkinoCommonComSource___ExitJobListCriticalSection();
}

// This is the key routine to submit a 3D batch conversion job to the COM server. It simply takes
// some predefined user parameters and sends them to the server. The server will then queue up
// the job for batch conversion. The server returns to us a GUID handle to identify this job. When
// the job is complete the server will inform us by passing back a GUID to us directly and/or
// executing a .exe/.bat file which we provide to the COM server as a filename.
// These parameters are obtained from the user of the test client via the SelectAndAddNewBatchConversionJobDialogProc() routine above.

// Note, for Okino .bdf file format, set the importer or exporter GUID to 'Nc_BDF_FILE_FORMAT_GUID'

// Returns: 1 = OK, 0 = failed, -1 = Try again (server is busy)

	short
OkinoCommonComSource___Submit3DBatchConversionJobToCOMServer(Nd_Com_Batch_Converter_JobInfoStruct *job)
{
	USES_CONVERSION;
	SAFEARRAY 	*in_options;		// Input parameters
	SAFEARRAY 	*in_render_options;	// Optional input parameters related to the snapshot rendering made after the conversion process is done.
	SAFEARRAY 	*out_options;		// Resulting output parameters
	LPTSTR 		Return_Batch_Job_ID;
	CComVariant 	val;
	CComVariant 	str_var, short_var;	// These hold our variant types (string or short)
	short		status = 0;		// Failed
	long		index;

	// ------>> Input, general options

	// Initialize the bounds for the 1D array
	SAFEARRAYBOUND safeBound[1]; 
	
#define	NUM_SUBMIT3D_INPUT_PARAMS	17
#define	NUM_SUBMIT3D_OUTPUT_PARAMS	2

	// Set up the bounds of the 1D array
	safeBound[0].cElements = NUM_SUBMIT3D_INPUT_PARAMS;	// 2 arguments
	safeBound[0].lLbound = 0;				// Lower bound is 0

	// The array type is VARIANT so that we can stuff any type into each element
	// Storage will accomodate a BSTR and a short
	in_options = SafeArrayCreate(VT_VARIANT, 1, &safeBound[0]);
	if (in_options == NULL)
		return(0);		// Failed

	index = 0;

	// String input arguments

	// Filename + path and extension of the input file to be converted
	str_var = job->filename_and_filepath;
	SafeArrayPutElement(in_options, &index, &str_var); index++;

	// Directory where to place the converted file(s). If NULL then place in the same directory as the input file
	str_var = job->export_directory;
	SafeArrayPutElement(in_options, &index, &str_var); index++;

	// GUID of the import converter (or NULL if auto-load is to be used)
	str_var = job->Importer_GUID;
	SafeArrayPutElement(in_options, &index, &str_var); index++;

	// GUID of the export converter
	str_var = job->Exporter_GUID;
	SafeArrayPutElement(in_options, &index, &str_var); index++;

	// Optional external .exe or script file name to execute once the conversion is done.
	str_var = job->file_to_execute_when_conversion_done;
	SafeArrayPutElement(in_options, &index, &str_var); index++;

	// Short input arguments

	// FALSE: submit the job and then return right away. Job will be queue for conversion.
	// TRUE: if no job waiting, perform the job right away and return only when the converion is done.
	short_var = FALSE;
	SafeArrayPutElement(in_options, &index, &short_var);index++;

	// TRUE: resize the imported model to the camera viewport before exporting
	short_var = job->resize_model_to_viewport_upon_import;
	SafeArrayPutElement(in_options, &index, &short_var);index++;

	// TRUE: add two white lights to the scene if none in the imported scene
	short_var = job->add_lights_to_scene_if_none;
	SafeArrayPutElement(in_options, &index, &short_var);index++;

	// TRUE: perform polygon processing before exporting the model
	short_var = job->perform_polygon_processing_on_converted_model;
	SafeArrayPutElement(in_options, &index, &short_var);index++;

	// TRUE: compress all children objects on a grouping folder into a single object
	short_var = job->compress_all_children_of_each_grouping_folder_to_single_object;
	SafeArrayPutElement(in_options, &index, &short_var);index++;

	// TRUE: Enable hierarchy optimizer # 1
	short_var = job->hierarchy_optimizer1;
	SafeArrayPutElement(in_options, &index, &short_var);index++;

	// TRUE: Enable hierarchy optimizer # 2
	short_var = job->hierarchy_optimizer2;
	SafeArrayPutElement(in_options, &index, &short_var);index++;

	// TRUE: perform polygon reduction before exporting the model
	short_var = job->perform_polygon_reduction_on_converted_model;
 	SafeArrayPutElement(in_options, &index, &short_var); index++;

	// TRUE: apply a global transform to the exported data
	short_var = job->apply_global_transform;
	SafeArrayPutElement(in_options, &index, &short_var);index++;

	// TRUE: create a backup file if the exported file already exists
	short_var = job->create_backup_file;
	SafeArrayPutElement(in_options, &index, &short_var);index++;

	// TRUE: inform the client via one of its event sinks that the batch
	// conversion process was completed. Not used if the 'start_immediately_and_wait_for_completion'
	// option is set to TRUE
	short_var = TRUE;
	SafeArrayPutElement(in_options, &index, &short_var);index++;

	// TRUE: Perform the conversion in the foreground so that we can view the conversion on
	// the view windows. FALSE: perform the conversion like the batch converter whereby we
	// can't see the file being imported & exported.
	short_var = job->make_batch_job_conversion_viewable_in_view_ports;
	SafeArrayPutElement(in_options, &index, &short_var);index++;

	// ------>> Input, render options

	// If image snapshot rendering is enabled, then a small bitmap image will be created
	// after the export process has finished. The bitmap will be named with the root name
	// of the job ID and a file extension to match the desired bitmap format. The image will
	// be stored in the Windows TEMP directory.

	// Set up the bounds of the 1D array
	safeBound[0].cElements = 7;	// 7 arguments
	safeBound[0].lLbound = 0;	// Lower bound is 0

	// The array type is VARIANT so that we can stuff any type into each element
	// Storage will accomodate a BSTR and a short
	in_render_options = SafeArrayCreate(VT_VARIANT, 1, &safeBound[0]);
	if (in_render_options == NULL)
		return(0);		// Failed

	index = 0;

	// String input arguments

	// Rendering level "scanline" or "raytrace"
	str_var = job->render.renderlevel;
	SafeArrayPutElement(in_render_options, &index, &str_var); index++;

	// File format desired, such as ".jpg", ".tif"
	str_var = job->render.format_str;
	SafeArrayPutElement(in_render_options, &index, &str_var); index++;

	// Default background color override (any color name from nicolours.db)
	str_var = "";	// None for now. You could also try "white" as an example
	SafeArrayPutElement(in_render_options, &index, &str_var); index++;

	// Short input arguments

	// Enable rendering snapshot
	short_var = job->render.enable;
	SafeArrayPutElement(in_render_options, &index, &short_var);index++;

	// X resolution
	short_var = job->render.res_x;
	SafeArrayPutElement(in_render_options, &index, &short_var);index++;

	// Y resolution
	short_var = job->render.res_y;
	SafeArrayPutElement(in_render_options, &index, &short_var);index++;

	// Scanline X anti-aliasing (0 = disabled)
	short_var = job->render.x_antialias;
	SafeArrayPutElement(in_render_options, &index, &short_var);index++;

	// Scanline Y anti-aliasing (0 = disabled)
	short_var = job->render.y_antialias;
	SafeArrayPutElement(in_render_options, &index, &short_var);index++;

	// TRUE: resize the imported model to the camera viewport before rendering
	short_var = FALSE;
	SafeArrayPutElement(in_render_options, &index, &short_var);index++;

	// TRUE: add two white lights to the scene if none in the imported scene before rendering
	short_var = FALSE;
	SafeArrayPutElement(in_render_options, &index, &short_var);index++;

	// ------>> Call the COM server to submit the job

	VARIANT in_options_variant;
	VariantInit(&in_options_variant);
	in_options_variant.vt = VT_VARIANT | VT_ARRAY;
	V_ARRAY(&in_options_variant) = in_options;

	VARIANT in_render_options_variant;
	VariantInit(&in_render_options_variant);
	in_render_options_variant.vt = VT_VARIANT | VT_ARRAY;
	V_ARRAY(&in_render_options_variant) = in_render_options;

	VARIANT out_options_variant;
	VariantInit(&out_options_variant);

	index = 0;
	try {
		short	com_error_result;

loop:		hresult = pConvertIO->Submit3DModelForBatchConversion(
			1,				// The version of this function's parameter passing interface: version 1 for now. If we add new parameters via the SAFEARRAY then the client should increment this value to the current interface version it supports
			&in_options_variant,		// Input parameters
			&in_render_options_variant,	// Optional input parameters related to the snapshot rendering made after the conversion process is done.
			&out_options_variant);		// Resulting output parameters

		// If fatal, ask the user what to do: retry, restart or abort. If restart, try restarting the server,
		com_error_result = OkinoCommonComSource___Check_COM_HRESULT_Error_CODE(status, "Error while asking COM server to submit a 3d batch conversion job. COM server has either crashed or is waiting for a user response.");
		if (com_error_result == IDC_SERVERFAULT_RETRY_OPERATION)
			goto loop;
		else if (com_error_result == IDC_SERVERFAULT_CANCEL_OPERATION) {
			goto exit;
		}
	} catch (_com_error &e) {
		OkinoCommonComSource___Display_COM_Error(e, "Could not submit a 3D batch conversion job to the COM server. 'Submit3DModelForBatchConversion' COM interface function failed.");
		goto exit;
	}

	out_options = V_ARRAY(&out_options_variant);	// Get the safe array out of the variant
	if (out_options) {
		// Short return arguments

		// Status: 1 = conversion went ok, 0 = conversion failed
		SafeArrayGetElement(out_options, &index, (void *) &val); ++index;
		status = val.iVal;

		// String return arguments

		// Keep a copy of the GUID returned to us by the COM server. This will be the unique
		// ID for the job which is necessary to track it once the COM server has finished with the job.
		SafeArrayGetElement(out_options, &index, (void *) &val); ++index;
		Return_Batch_Job_ID = OLE2T(val.bstrVal);
		if (Return_Batch_Job_ID)
			OkinoCommonComSource___Copy_String(&job->job_id, Return_Batch_Job_ID);

		SafeArrayDestroy(out_options);
	}

exit:   SafeArrayDestroy(in_options);		// Clean up after we are done with the input parameter safe array
	SafeArrayDestroy(in_render_options);	// Clean up after we are done with the input parameter safe array

	return(status);
}

// After a batch conversion job is done, the COM server informs us via an
// event sink in the test_client.cpp file. The event sink then calls this
// routine to change our internal status to "Completed" on the job then
// update our user interface.

// NOTE:: This is a background async event in a different thread than the
//        main running app. Thus, we need semaphores around all code which
//	  modifies the job queue. 

	void
OkinoCommonComSource___HandleJobCompletedNotificationFromCOMServer(
	char *job_guid, 		// The Job ID
	char *status_text, 		// Signifies if the job completed, crashed or aborted
	char *time_string, 		// Time completion string
	char *windows_temp_dir)		// Windows TEMP directory we are using to store the queue files
{
	Nd_Com_Batch_Converter_JobInfoStruct *job;

	// Since the job list can be accessed from the background "job done" event sink
	// as well as the foreground process, we need critical sections around the job list.
	OkinoCommonComSource___EnterJobListCriticalSection();

	job = OkinoCommonComSource___BatchConversionJobList;
	while (job) {
		if (!strcmp(job->job_id, job_guid)) {
			job->job_completed.done = TRUE;

			OkinoCommonComSource___Copy_String(&job->job_completed.status_text, status_text);
			OkinoCommonComSource___Copy_String(&job->job_completed.time_string, time_string);
			OkinoCommonComSource___Copy_String(&job->job_completed.job_temp_directory, windows_temp_dir);
			break;
		}
		job = job->next_ptr;
	}

	OkinoCommonComSource___ExitJobListCriticalSection();
}

// Since the job list can be accessed by the background "job done" event sink, we need
// a critical section around it.

	void
OkinoCommonComSource___EnterJobListCriticalSection()
{
	if (!job_list_critical_section_initialized) {
		InitializeCriticalSection(&job_list_critical_section);
		job_list_critical_section_initialized = TRUE;
	}

	// Enter a critical section while the file is open so that no one else can access it.
	EnterCriticalSection(&job_list_critical_section);

}

	void
OkinoCommonComSource___ExitJobListCriticalSection()
{
	LeaveCriticalSection(&job_list_critical_section);
}
