/*****************************************************************************

	----------------------------------------------------------------
         Main Export Converter Module for PolyTrans - A Working Example
	----------------------------------------------------------------

	This code is a fully functional PolyTrans plug-in export converter
	module. It demonstrates how to query the Okino core database for
	everything from geometry to lights, cameras, materials, textures,
	hierarchy and animation. It is an ideal starting point to create
	a new export converter for PolyTrans. Step-by-step instructions
	to derive a new export converter from the files contained in
	this directory are provided in the online API documentation.

  Copyright (c) 1988, 2006 Okino Computer Graphics, Inc. All Rights Reserved.

This file is proprietary source code of Okino Computer Graphics, Inc. and it 
is not to be disclosed to third parties, published, adopted, distributed,
copied or duplicated in any form, in whole or in part without the prior 
authorization of Okino Computer Graphics, Inc. This file may, however, be
modified and recompiled solely for use as a PolyTrans export converter as
per the "PolyTrans Import/Export SDK & Redistribution Agreement", to be read 
and signed by the developer.

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

#include "main.h"	// Main includes for all modules

#include <windows.h>	// Only needed for the MessageBox() call

/* ---------------------->>>>  Definitions  <<<<--------------------------- */

// If we don't care about "red folders" in the hierarchy then set this option
// to Nc_TRUE. The red folder will be added to the DAG Path lists (via the call
// to 'Ni_AddHierarchyDAGPath()') and the 'Nv_IgnoreEmptyObjectNodesInTree' option
// set to Nc_TRUE will effectively make them invisible to this exporter.
// In general you would only want to deal with red folders if your
// exporter allows sub-sections of a hierarchy tree to be instantiated
// in some manner; normally no file formats allow this to happen.
#define	IGNORE_RED_FOLDERS_IN_HIERARCHY	Nc_TRUE

/* -------------------->>>>  Global Variables  <<<<-------------------------- */

// All dialog box options are stored in this structure
Exporter_Options *export_options = NULL;

/* -------------------->>>>  Local Variables  <<<<-------------------------- */

/* ----------------->>>>  Local Function Prototypes  <<<<---------------------- */

// Main routines to query and print out the Okino database
static Nd_Void	NI_Exporter_WalkTree_Main_Callback(Nd_Walk_Tree_Info *wt_ptr, Nd_Int *Nv_Status);

// A callback for Ni_Walk_Tree() to specify when a new hierarchy level has started/ended
static Nd_Void	NI_Exporter_StartEnd_Hierarchy_Level_Callback(Nd_Token Nv_Parent_Type, char *Nv_Parent_Handle_Name, Nd_Short Nv_Current_Hierarchy_Level, Nd_Short Nv_Starting_New_Level);

// Utility routines
static Nd_Void	NI_Exporter_DAGPath_UserDataMemoryFreeRoutine(void *data);

void writeOSG( const char* out_filename, long *return_result );

/* ------------------------------------------------------------------------ */

// Main entry point to perform the actual export conversion.

// Return Nc_TRUE if an error, else Nc_FALSE. 

#undef CURR_FN_NAME
#define CURR_FN_NAME    "ExportConverter_Main()"


	Nd_Void
ExportConverter_Main(char *tokens[], char *data[], long *return_result, void *aux_ptr)
{
	FILE		*ofp;
	Nd_Int		Nv_Walk_Tree_Return_Status, Nv_IgnoreEmptyObjectNodesInTree;
	Nd_Token	Nv_Confirm_Overwrites_EnableToggle;
	char   		*filename, out_filename[300];
#if _MSC_VER < 1300	// MSC v7 or newer (2003)
    char filepath_used[300];
#endif
	char		export_directory[300];
	long    	count;
	short   	return_status;	// Nc_TRUE if error, Nc_FALSE if no error
	Nd_Int		dummy;

	/* Initialize our local variables */
	NI_Exporter_InitExportOptionVariables();
	aux_ptr = aux_ptr;
	filename = NULL;
	return_status = Nc_FALSE;		// No error
	filename = (char *) NULL;
	ofp = NULL;

	// When instancing is used by the exporter, this list maps an Okino object definition
	// to the exported object definition. See its usage in NI_Handle_Okino_Object_Instancing()
	// at the end of this file.
	Ni_StringDictionary_Init(&export_options->list_of_multiply_instanced_nugraf_objects);

	// Initialize the name-space management helper routines. These powerful
	// routines can be used to create unique handle names for export. For
	// example, if you have to convert spaces and non-numeric/alpha characters
	// of an Okino handle name (such as an instance/light/camera name) to 
	// underscores then there is always the chance that multiple names can
	// collide to a single name. To circumvent this problem, Okino software 
	// has always implemented name-space collision prevention code in every
	// import and export converter. The main helper function, which unfortumately
	// is not used in this example exporter, is Export_IO_NameSpaceManagement_MakeHandleNameUnique()
	// which is defined in the importio.h header file. 
	Export_IO_NameSpaceManagement_Init();

	// Pick up any important options sent to us by the host program.
	// For most exporters this will only be the save-as filename.
 	count = 0;
	while (tokens[count] != NULL) {
		if (!strcmp(tokens[count], "filename"))
			filename = data[count];
		else
			/* %s: Unknown token '%s' sent to geometry export converter. */
			Ni_Report_Error(Nc_ERR_ERROR, "ERR_GEOM_EXPORT_BAD_TOKEN", CURR_FN_NAME, tokens[count]);
		++count;
	}

	/* Determine the directory where the file is being read from */
	Ni_ExtractBaseFilePath((char *) filename, export_directory);
	if (export_directory[strlen(export_directory)-1] == '/' || export_directory[strlen(export_directory)-1] == '\\')
		export_directory[strlen(export_directory)-1] = '\0';
	Import_IO_Copy_String(&export_options->export_directory, export_directory);

    // Force desired extension.
    short extOK = Ni_Check_Multiple_File_Extensions( filename, ".osg;.ive" );
	if (!extOK) {
		char	buf[300];

		sprintf(buf, "Invalid extension for this exporter.\nWrite .ive instead?\n\nSelect 'Yes' to write in .ive format.\nSelect 'No' to write in .osg format.\nSelect 'Abort' to abort.\n", out_filename);
		Nd_Int result = MessageBox(GetFocus(), buf, "Info Request", MB_APPLMODAL | MB_YESNOCANCEL | MB_DEFBUTTON1 | MB_ICONQUESTION);
		if (result == IDYES)
    	    Ni_Check_File_Extension( filename, out_filename, ".ive" );
		else if (result == IDNO)
    	    Ni_Check_File_Extension( filename, out_filename, ".osg" );
		else
        {
			return_status = Nc_TRUE;	// Aborting
			goto error;
		}
	}
    else
        strncpy( out_filename, filename, 299 );

	/* Return if no instances attached to the "world" instance node */
#if 0
	Ni_Inquire_Instance("world", Nt_NUMCHILDREN, (Nd_Int *) &num_children_off_world_root, Nt_CMDEND);
	if (!num_children_off_world_root) {
		return_status = Nc_TRUE;	// Aborting
		goto exit;
	}
#endif

    /* Open the file for writing with optional 'Do you want to overwrite file?' checking */
	Ni_Inquire_Option(Nt_CONFIRM, Nt_OVERWRITES, Nt_ENABLED, (Nd_Token *) &Nv_Confirm_Overwrites_EnableToggle, Nt_CMDEND);
	if (Nv_Confirm_Overwrites_EnableToggle == Nt_ON) {
		Nd_Int	result;

		Ni_File_Exists(out_filename, &result);
		if (result) {
			char	buf[300];

			sprintf(buf, "Filename '%s' already exists.\n\nReplace existing file?", out_filename);
			result = MessageBox(GetFocus(), buf, "Info Request", MB_APPLMODAL | MB_YESNO | MB_DEFBUTTON2 | MB_ICONQUESTION);
			if (result == IDNO) {
				return_status = Nc_TRUE;	// Aborting
				goto error;
			}
		}
	}

    writeOSG( out_filename, return_result );

    if (!export_options->osgDebugTextFile)
        goto exit;

    // Make sure the file extension is correct
	Ni_Check_File_Extension(filename, out_filename, ".txt");

	/* Open the file for writing with optional 'Do you want to overwrite file?' checking */
	Ni_Inquire_Option(Nt_CONFIRM, Nt_OVERWRITES, Nt_ENABLED, (Nd_Token *) &Nv_Confirm_Overwrites_EnableToggle, Nt_CMDEND);
#if _MSC_VER >= 1300	// MSC v7 or newer (2003)
	if (Nv_Confirm_Overwrites_EnableToggle == Nt_ON) {
		Nd_Int	result;

		Ni_File_Exists(out_filename, &result);
		if (result) {
			char	buf[300];

			sprintf(buf, "Filename '%s' already exists.\n\nReplace existing file?", out_filename);
			result = MessageBox(GetFocus(), buf, "Info Request", MB_APPLMODAL | MB_YESNO | MB_DEFBUTTON2 | MB_ICONQUESTION);
			if (result == IDNO) {
				return_status = Nc_TRUE;	// Aborting
				goto error;
			}
		}
	}
	ofp = fopen(out_filename, "w");
#else
	ofp = Ni_Search_And_Open_File(out_filename, "w", (Nv_Confirm_Overwrites_EnableToggle == Nt_ON), "NG_WORKING_DIR", filepath_used);
#endif
	if (ofp == NULL) {
		/* %s: Could not open /text dump/ file '%s' for writing. */
		Ni_Report_Error(Nc_ERR_ERROR, 
			"OUTERR_FILE_OPEN_WRITE", 
			CURR_FN_NAME, 
			"ISU OpenSceneGraph Export", filename);
		return_status = Nc_TRUE;	// Aborting
		goto error;
	}

	/* Print header message to the file */
	OPTIONAL_FPRINTF(ofp, "# This file lists some of the contents of the PolyTrans 3D database in an easy to\n");
	OPTIONAL_FPRINTF(ofp, "# understand ASCII text form. More information can be output to this file by\n");
	OPTIONAL_FPRINTF(ofp, "# expanding the 'exp_isu_osg.cpp' example source code file.\n\n");

	// First, go and output some global information about the animation data currently
	// defined in the dataset and the current/start/end animation times + playvback rate.
	NI_Exporter_QueryAndOutputGeneralAnimationInformation(ofp);

	// Output any meta data associated with global scene
	NI_Exporter_Output_Meta_Data(ofp, NULL, NULL);

	// --------------->>>>  Light Source Output

	OPTIONAL_FPRINTF(ofp, "--------------->>>>  Light Source Output  <<<<----------------\n\n");

	/* Call 'List_Light_Callback()' for every light source defined. */
	if (export_options->ena_lights)
		Ni_Enumerate(&dummy, "*", Nc_FALSE, (Nd_Void *) ofp, (Nd_Void *) 0, 
			NI_Exporter_List_Light_Callback, Nt_LIGHT, Nt_CMDEND);

	// --------------->>>>  Camera Definitions Output

	OPTIONAL_FPRINTF(ofp, "\n--------------->>>>  Camera Definitions Output  <<<<----------------\n\n");

	/* Call 'List_Camera_Callback()' for every camera defined. */
	if (export_options->ena_cameras)
		Ni_Enumerate(&dummy, "*", Nc_FALSE, (Nd_Void *) ofp, (Nd_Void *) 0, 
			NI_Exporter_List_Camera_Callback, Nt_CAMERA, Nt_CMDEND);

	// --------------->>>>  Surface (Material) Definition Output

	OPTIONAL_FPRINTF(ofp, "\n--------------->>>>  Surface (Material) Definition Output  <<<<----------------\n");

	/* Call 'List_Surface_Callback()' for every surface defined. */
	if (export_options->ena_materials)
		Ni_Enumerate(&dummy, "*", Nc_FALSE, (Nd_Void *) ofp, (Nd_Void *) 0, 
			NI_Exporter_List_Surface_Callback, Nt_SURFACE, Nt_CMDEND);

	// --------------->>>>  Texture Definition Output

	OPTIONAL_FPRINTF(ofp, "\n--------------->>>>  Texture Definition Output  <<<<----------------\n\n");

	/* Call 'List_Texture_Callback()' for every texture defined. */
	if (export_options->ena_texture_definitions)
		Ni_Enumerate(&dummy, "*", Nc_FALSE, (Nd_Void *) ofp, (Nd_Void *) 0, 
			NI_Exporter_List_Texture_Callback, Nt_TEXTURE, Nt_CMDEND);

	// --------------->>>>  Atmospherics Output

	OPTIONAL_FPRINTF(ofp, "\n--------------->>>>  Atmospherics Output  <<<<----------------\n\n");

	/* List the atmospheric parameters */
	if (export_options->ena_env_info)
		NI_Exporter_List_Atmospheric_Values(ofp);

	// --------------->>>>  Background Output

	OPTIONAL_FPRINTF(ofp, "\n--------------->>>>  Background Output  <<<<----------------\n\n");

	/* List the background parameters */
	if (export_options->ena_background_info)
		NI_Exporter_List_Background_Values(ofp);

	// --------------->>>>  Hierarchy & Geometry Output

	OPTIONAL_FPRINTF(ofp, "\n--------------->>>>  Hierarchy & Geometry Output  <<<<-----------------\n");

	// Init the DAG Path hierarchy lists (used to uniquely identify any 
	// instance or empty object (red folder) in the hierarchy tree)
#if IGNORE_RED_FOLDERS_IN_HIERARCHY
	Nv_IgnoreEmptyObjectNodesInTree = Nc_TRUE;
#else
	Nv_IgnoreEmptyObjectNodesInTree = Nc_FALSE;
#endif
	Ni_InitHierarchyDAGPathLists(Nv_IgnoreEmptyObjectNodesInTree);

	// Make sure we get the Import I/O toolkit to send the current polygon reduction options
	// from the main UI (dialog box level) into the toolkit, so that polygon reduction can be
	// performed during the export phase.
	if (export_options->polygon_reduction_ena) {
		char *ptr, ini_filepath[1024];

		// The reduction options are in their own .ini file since the main .ini file overwrites them.
		strcpy(ini_filepath, plugin_directory);
		strcat(ini_filepath, INI_FILENAME);
		ptr = strstr(ini_filepath, ".ini");
		if (ptr) *ptr = '\0';
			strcat(ini_filepath, "_reduction.ini");

		Import_IO_Send_Polygon_Reduction_UI_Options_To_Toolkit(ini_filepath);
	}

	// Walk the scene graph. This is where the action starts!
	try {
		// If your export file format allows for instancing of object geometry then you will want
		// to enable this option when instancing is enabled in the exporter. It invokes internal
		// helper routines which pre-walk the tree before the main tree walk is done, and sets up
		// the 'instancing detection' variables found at the end of the Nd_Walk_Tree data stucture (see ni.h)
		//
		// These instancing-detection helper variables help your exporter determine:
		//
		// (1) How many times an object definition is referenced (if 2 or more times, then the
		//     object definition is a candidate for instancing during output).
		// (2) If a Nt_INDEXEDPOLYGON mesh is instanced 2 or more times and that mesh inherits different polygon-level
		//     materials from each different instance. In such cases the object definition usually
		//     can't be instanced since most export file formats cannot output a multiply instanced
		//     polygon mesh which has different polygon-level materials on each instance.
               	Nd_Token ena_instancing_detection_counts = (export_options->ena_instancing) ? Nt_ON : Nt_OFF;

		Ni_Walk_Tree(NULL, (Nd_Void *) ofp, (Nd_Void *) NULL,
			NI_Exporter_WalkTree_Main_Callback,
			Nt_RETURNSTATUS, (Nd_Int *) &Nv_Walk_Tree_Return_Status, Nt_CMDSEP,
			// Make sure some internal state is set up for data export queries
			Nt_SETUPFOREXPORTER, Nt_ENABLED, Nt_ON, Nt_CMDSEP,
			// Specify a callback which will inform us when a new hierarchy level is started/ended
			Nt_STARTENDHIERARCHYCB, (void *) NI_Exporter_StartEnd_Hierarchy_Level_Callback, Nt_CMDSEP,
			// This enables the internal pre-walk routines which determine the state and counts for the instancing
			// helper detection variables found at the end of the Nd_Walk_Tree data structure (see ni.h). See above
			// for a further explanation, or the Nd_Walk_Tree data structure for full variables explanations.
			Nt_ENABLEINSTANCINGDETECTIONCOUNTS, Nt_ENABLED, ena_instancing_detection_counts, Nt_CMDSEP,
			Nt_CMDEND);
		}
	catch( ... )
	{
		Ni_Report_Error_printf(Nc_ERR_ERROR, "Exporter is raising an unknown exception while walking the geometry tree.");
		*return_result = Nc_TRUE;
		goto error;
	}

	// If the user aborted the tree walk then return now
	if (Nv_Walk_Tree_Return_Status == Nc_WALKTREE_ABORT) {
		return_status = Nc_TRUE;	// Error
		goto exit;
	}

	// Optionally perform automatic bitmap conversion for those bitmap
	// images which were queued up during the material export phase.
	NI_Exporter_Convert_Bitmap_Files(filename);

	// Output some statistics
	if (TRUE) {
		Ni_Report_Error_printf(Nc_ERR_RAW_MSG, "Export statistics:");
		if (export_options->total_meshes)
			Ni_Report_Error_printf(Nc_ERR_RAW_MSG, "\tNumber of meshes = %ld", export_options->total_meshes);
		if (export_options->total_polygons)
			Ni_Report_Error_printf(Nc_ERR_RAW_MSG, "\tNumber of polygons = %ld", export_options->total_polygons);
		if (export_options->total_polylines)
			Ni_Report_Error_printf(Nc_ERR_RAW_MSG, "\tNumber of polylines = %ld", export_options->total_polylines);
		if (export_options->total_spline_shapes)
			Ni_Report_Error_printf(Nc_ERR_RAW_MSG, "\tNumber of spline shapes = %ld", export_options->total_spline_shapes);
		if (export_options->total_nurbs_curves)
			Ni_Report_Error_printf(Nc_ERR_RAW_MSG, "\tNumber of independent NURBS curves = %ld", export_options->total_nurbs_curves);
		if (export_options->total_nurbs_surfaces)
			Ni_Report_Error_printf(Nc_ERR_RAW_MSG, "\tNumber of NURBS surfaces = %ld", export_options->total_nurbs_surfaces);
		if (export_options->total_pointsets)
			Ni_Report_Error_printf(Nc_ERR_RAW_MSG, "\tNumber of pointsets = %ld (%ld total vertices)", export_options->total_pointsets, export_options->total_pointset_points);
		if (export_options->total_materials)
			Ni_Report_Error_printf(Nc_ERR_RAW_MSG, "\tNumber of materials = %ld", export_options->total_materials);
		if (export_options->total_texture_maps)
			Ni_Report_Error_printf(Nc_ERR_RAW_MSG, "\tNumber of texture maps = %ld", export_options->total_texture_maps);
		if (export_options->total_groups)
			Ni_Report_Error_printf(Nc_ERR_RAW_MSG, "\tNumber of grouping nodes = %ld", export_options->total_groups);
		if (export_options->total_instances)
			Ni_Report_Error_printf(Nc_ERR_RAW_MSG, "\tNumber of instances = %ld", export_options->total_instances);
	}

exit:
	if (ofp)
    {
        if (ferror(ofp))
	    	return_status = Nc_TRUE;	// Error while writing file (out of disk space?)
		fclose(ofp);
    }

error:
    *return_result = return_status;

	try {
        	// Free up the multiple-instancing list
        	if (!Ni_StringDictionary_IsNull(&export_options->list_of_multiply_instanced_nugraf_objects)) {
        		Ni_StringDictionary_Destroy(&export_options->list_of_multiply_instanced_nugraf_objects, FALSE);
        		Ni_StringDictionary_Init(&export_options->list_of_multiply_instanced_nugraf_objects);
        	}

		// Shutdown the name-space management helper routines.
		Export_IO_NameSpaceManagement_Shutdown();

		// Free up the texture image list used for automatic bitmap conversion
		NI_Exporter_Cleanup_TxtrFileName_List();

		// Free up the DAG Path hierarchy lists (used to uniquely identify any
		// instance or empty object (red folder) in the hierarchy tree)
		Ni_FreeHierarchyDAGPathLists();

		// Free up any data associated with the global variables
		NI_Exporter_DeInitExportOptionVariables();
	}
	catch( ... )
	{
		Ni_Report_Error_printf(Nc_ERR_ERROR, "Exporter is raising an unknown exception while shutting down the exporter.");
		*return_result = Nc_TRUE;
		return;
	}
}

// -----------------------------------------------------------------------------------------

/* This callback is called for each hierarchically-attached instance in */
/* the instance tree. 'default_ctm' points to the CTM that is to xform the */
/* instance's primitives from primitive space to world space. */

#undef CURR_FN_NAME
#define CURR_FN_NAME    "NI_Exporter_WalkTree_Main_Callback()"

	static Nd_Void
NI_Exporter_WalkTree_Main_Callback(Nd_Walk_Tree_Info *Nv_Info, Nd_Int *Nv_Status)
{
	FILE			*ofp;
	Nd_HierarchyDAGPath_Info *hierarchy_node;
	Nd_Void			*Nv_DAGPath_User_Data_Pointer;
	Nd_Token 		primitive_type, Nv_Parent_Type;
	Nd_Token		Nv_TxtrparamInstanceToggle, Nv_TxtrparamObjectToggle;
	Nd_Int			Nv_Num_Defined_Primitives, j;
	short                   do_mesh_output = Nc_FALSE;
	short			animation_data_is_available = Nc_FALSE;
	short			pivot_point_matrix_is_valid = Nc_FALSE;
	char			*handle_name, *master_object, *Nv_Parent_Name;

	/* Pick up the arguments sent to this callback */
	ofp = (FILE *) Nv_Info->Nv_User_Data_Ptr1;

	// In case there are 'red folders' (empty objects) in the hierarchy
	// tree (cases of object instancing) we have to keep track of each
	// instance in the tree based on DAG Paths (see explanation in the 
	// file ni4_aux.h). This function adds a new DAG Path to this current
	// empty instance. To uniquely find the parent of this instance in the 
	// the tree call the 'Ni_FindParentViaDAGPathLineage()' function (see above)
	//
	// NOTE: An exporter can associate its own local data with this newly
	//       allocated DAG Path node by supplying its pointer to the 
	//	'Nv_DAGPath_User_Data_Pointer' argument. If you allocate memory and pass in this memory pointer
	// 	then you will have to specify the 'NI_Exporter_DAGPath_UserDataMemoryFreeRoutine'
	//	argument which is the callback function called by the toolkit
	//	to free up this memory at the end of the export process, otherwise
	//	set the function pointer argument to NULL and no callbacks will be
	// 	called by the toolkit.
	Nv_DAGPath_User_Data_Pointer = NULL;
	hierarchy_node = Ni_AddHierarchyDAGPath(Nv_Info, NI_Exporter_DAGPath_UserDataMemoryFreeRoutine, (void *) Nv_DAGPath_User_Data_Pointer);

	// If we don't care about "red folders" in the hierarchy then let's
	// return now. The red folder will be added to the DAG Path lists
	// above (via 'Ni_AddHierarchyDAGPath()') and the 'Nv_IgnoreEmptyObjectNodesInTree'
	// option above will effectively make them invisible to this exporter.
	// In general you would only want to deal with red folders if your
	// exporter allows sub-sections of a hierarchy tree to be instantiated
	// in some manner; normally no file formats allow this to happen.
#if IGNORE_RED_FOLDERS_IN_HIERARCHY
	if (Nv_Info->Nv_Empty_Object)
		return;
#endif

	// This is the all-important instance handle name. This callback
	// routine is called based on the data associated with this instance. 
	// If the node is an empty object then this is the empty object handle name.
	handle_name = Nv_Info->Nv_Handle_Name;

	// If this is not an empty instance ('yellow folder' or grouping instance node)...
	if (!Nv_Info->Nv_Empty_Instance && !Nv_Info->Nv_Empty_Object) {
		/* Get the master object from which this instance was derived */
		Ni_Inquire_Instance(handle_name,
			Nt_MASTEROBJECT, (char **) &master_object, Nt_CMDSEP,
			Nt_CMDEND);

		/* Determine how many primitives this object has (should only be 1 in all modern cases) */
		Ni_Inquire_Object(master_object,
			Nt_NUMPRIMITIVES, (Nd_Int *) &Nv_Num_Defined_Primitives, Nt_CMDSEP,
			Nt_CMDEND);

		/* See if a texture projection override is enabled for the instance */
		Ni_Inquire_TxtrParam(Nt_INSTANCE, handle_name,
			Nt_ENABLED, (Nd_Token *) &Nv_TxtrparamInstanceToggle, Nt_CMDSEP,
			Nt_CMDEND);

	} else {
		Nv_Num_Defined_Primitives = 0;
		Nv_TxtrparamInstanceToggle = Nt_OFF;
		master_object = NULL;
	}

	/* See if a texture projection override is enabled for the empty object */
	if (master_object)
		Ni_Inquire_TxtrParam(Nt_OBJECT, master_object,
			Nt_ENABLED, (Nd_Token *) &Nv_TxtrparamObjectToggle, Nt_CMDSEP,
			Nt_CMDEND);
	else
		Nv_TxtrparamObjectToggle = Nt_OFF;

	if (Nv_Info->Nv_Empty_Object)
		OPTIONAL_FPRINTF(ofp, "\nEmpty object \"%s\"\n", handle_name);
	else
		OPTIONAL_FPRINTF(ofp, "\nInstance \"%s\"\n", handle_name);
	OPTIONAL_FPRINTF(ofp, "\tLevel in the hierarchy = %d\n", Nv_Info->Nv_Hierarchy_Level);
	OPTIONAL_FPRINTF(ofp, "\t'Has More Siblings' = %d\n", Nv_Info->Nv_Has_More_Siblings);
	OPTIONAL_FPRINTF(ofp, "\t'Children to Follow' = %d\n", Nv_Info->Nv_Children_To_Follow);
	OPTIONAL_FPRINTF(ofp, "\t'NULL Object to Follow' = %d\n", Nv_Info->Nv_Null_Object_To_Follow);
	OPTIONAL_FPRINTF(ofp, "\t'Inherited Hidden Flag' = %d\n", Nv_Info->Nv_Inherited_Hidden_Flag);
	OPTIONAL_FPRINTF(ofp, "\t'Empty Instance' = %d\n", Nv_Info->Nv_Empty_Instance);
	OPTIONAL_FPRINTF(ofp, "\t'Empty (master) Object' = %d\n", Nv_Info->Nv_Empty_Object);

	if (master_object)
		// Output any meta data associated with this object definition
		NI_Exporter_Output_Meta_Data(ofp, Nt_OBJECT, master_object);

	if (!Nv_Info->Nv_Empty_Object) {
		Nd_Matrix	Nv_BindPoseWorldSpaceHierarhicalMatrixForJoint;

		OPTIONAL_FPRINTF(ofp, "\tSurface (material) assigned at instance level = ");
		if (Nv_Info->Nv_Surface_Name != (char *) NULL)
			OPTIONAL_FPRINTF(ofp, "\"%s\"\n", Nv_Info->Nv_Surface_Name);
		else
			OPTIONAL_FPRINTF(ofp, "(no surface assigned to instance)\n");

		/* Find out who the directly connected NULL parent instance or object is */
		Ni_Inquire_Instance(handle_name, Nt_PARENT, (Nd_Token *) &Nv_Parent_Type, (char **) &Nv_Parent_Name, Nt_CMDEND);
		OPTIONAL_FPRINTF(ofp, "\tParent %s name = \"%s\"\n", Nv_Parent_Type, Nv_Parent_Name);

		if (master_object != (char *) NULL)
			OPTIONAL_FPRINTF(ofp, "\tMaster object that this instance is derived from = \"%s\"\n", master_object);
		OPTIONAL_FPRINTF(ofp, "\tNumber of primitives in master object = %d\n", Nv_Num_Defined_Primitives);

		/* Display the two matrices associated with this instance. */
		/* The hierarchical matrix is = inherited * local matrix. */
		OPTIONAL_FPRINTF(ofp, "\tHierarchical instance transformation matrix:\n");
		OPTIONAL_DISPLAY_MATRIX(ofp, Nv_Info->Nv_Hierarchical_CTM, 2);
		OPTIONAL_FPRINTF(ofp, "\tInstance's local transformation matrix:\n");
		OPTIONAL_DISPLAY_MATRIX(ofp, Nv_Info->Nv_CTM, 2);

		// Query and print out the instance's "bind pose" matrix. This is only 
		// used on empty instances (null nodes) which define skeleton bone/joints
		// or on the instance node of a mesh object which has skinned vertices
		// in it (vertices which are bound to a skeleton, with associated weight values)
		OPTIONAL_FPRINTF(ofp, "\tInstance's \"bind pose\" transformation matrix:\n");
		Ni_Inquire_Instance(handle_name, Nt_BINDPOSE, Nt_MATRIX, (Nd_Matrix) Nv_BindPoseWorldSpaceHierarhicalMatrixForJoint, Nt_CMDEND);
		OPTIONAL_DISPLAY_MATRIX(ofp, Nv_BindPoseWorldSpaceHierarhicalMatrixForJoint, 2);
	}

	/* List any texture projection that is assigned to the instance or object */
	if (export_options->ena_texture_proj_info && !Nv_Info->Nv_Empty_Object) {
		if (master_object != (char *) NULL)
			NI_Exporter_List_Texture_Projection_Values(ofp, Nt_OBJECT, master_object);
		NI_Exporter_List_Texture_Projection_Values(ofp, Nt_INSTANCE, handle_name);
	}

	// When there are 'red folders' in the hierarchy (instancing is occuring) 
	// then the only true way to find the parent of an instance is with the
	// 'DAG Path' mechanism built into the toolkit and used by this exporter.
	// This code shows how to unique find a parent given the current location 
	// in the hierarchy (current location defined by 'Nd_Walk_Tree_Info *Nv_Info')
	//
	// NOTE: If you want your exporter to never know about 'red folders'
	// as being the real parents of instances in the tree, then set the
	// 'Nv_IgnoreEmptyObjectNodesInTree' variable to Nc_TRUE (see above).
	// This will effectively make the 'red folders' completely invisible
	// to the 'Ni_FindParentViaDAGPathLineage()'. Instead of returning
	// a red folder as the parent, the routine will return the next
	// (real) instance of the red folder instead. 
	if (Nv_Info->Nv_Hierarchy_Level > 1) {	// Ignore the "world"
		hierarchy_node = Ni_FindParentViaDAGPathLineage(Nv_Info);
		if (hierarchy_node) {
			OPTIONAL_FPRINTF(ofp, "\tDAG Path hierarchy information:\n");
			OPTIONAL_FPRINTF(ofp, "\t\tParent %s = '%s'\n", 
				hierarchy_node->Nv_Handle_Type, 
				hierarchy_node->Nv_Handle_Name);

			/* See the 'Nd_HierarchyDAGPath_Info' for more information that is stored */
			/* for the current instance and its parent (such as ctm's and hidden state) */
		}
	}

	/* If this is an empty instance (yellow folder) or empty object (red folder) then process them here as grouping nodes */
	if (Nv_Info->Nv_Empty_Instance || Nv_Info->Nv_Empty_Object || Nv_Info->Nv_Null_Object_To_Follow) {
		Nd_Matrix	pivot_inverse_pivot_matrix, normal_transform_matrix;

		// This is where you can define your grouping or hierarchy
		// nodes in the exported file format. 'Empty instances' are
		// the generic way that PolyTrans creates hierarchy. Rarely,
		// (such as with the Lightwave or IGES importers) it will also
		// use 'red folders' which allow everything below them in the
		// hierarchy tree to be instantiated multiple times. The 'DAG
		// Path' handler routines can help you eradicate the "red folder"
		// from the export process, effectively making them invisible.

		if (!export_options->ena_hierarchy)
			goto skip_to_attributes;

		// Update the statistics
		++export_options->total_groups;

		// Ignore the top-most "world" empty instance. All PolyTrans
		// exporters ignore the "world" node since it is not meant
		// to be exported to a file (it is the "holder" node in our UI)
		if (!strcmp(handle_name, "world"))
			goto skip_to_attributes;

		/* If this is an empty object (red folder) then skip it. */
		/* This node type is used quite often by the Lightwave import converter */
		/* to allow for multiple instatiation capabilities of a model. */
		/* Alternatively you could always just output a 'NULL grouping node' */
		/* to your export file in place of this red folder. */
		if (Nv_Info->Nv_Empty_Object)
			goto skip_to_attributes;

		// Go and see if there is any animation channel data associated
		// with this Empty Instance that we are interested about. If resampled
		// keyframe mode is in effect then temporary resampled keyframe lists
		// will have been created and stored in the core toolkit after this 
		// function finishes and returns here. 
	 	if (export_options->ena_object_animation) {
			animation_data_is_available = NI_Exporter_QueryAndSetup_Object_Animation_Keyframe_Data( ofp, handle_name);

			// If we are outputting animation data, then let's go examine the 
			// pivot point matrix associated with this instance. 
			pivot_point_matrix_is_valid = NI_Exporter_GetValidPivotPointMatricesForAnimationExport(handle_name, pivot_inverse_pivot_matrix, normal_transform_matrix);
		}

// !! DEVELOPER: ANIMATION CODE CHANGE NEEDED !! 
// 
// If we going to output animation data for an empty instance ('animation_data_is_available' is TRUE)
// and the instance's pivot point matrix was found to be non-identity ('pivot_point_matrix_is_valid' is TRUE)
// then what you really need to do at this very section of code is output two NULL
// nodes to your scene graph:
//
//	top_NULL_node			--> Has the animation curves assigned to it
//		child_NULL_node		--> Has the 'pivot_inverse_pivot_matrix' assigned to its transformation matrix
//
// See the top of 'anim.cpp' for a more extensive explanation.
//

		// If no children are attached to this empty instance, and no animation, then ignore it 
		if (!Nv_Info->Nv_Children_To_Follow && Nv_Info->Nv_Empty_Instance && !animation_data_is_available)
			goto skip_to_attributes;

		// If animation data was previously found and set up, then go output
		// the animation data associated with this instance. See the explanation
		// above. If you need to output two NULL NODES, due to the valid pivot
		// point matrix on this empty instance, then make sure you assign the
		// animation to the parent node of the two nodes being output.
		if (animation_data_is_available) 
			NI_Exporter_Output_Object_Animation_Data(ofp, handle_name);

		goto skip_to_attributes;
	}

	/* Oops, this object has no primitives */
	if (!Nv_Num_Defined_Primitives)
		return;

	// If instance's "hidden" flag turned on then return now. 
	if (Nv_Info->Nv_Hidden_Flag) {
		*Nv_Status = Nc_WALKTREE_IGNORE_SUBTREE;
		return;
	}
	// If instance's inherited "hidden" flag turned on then return now. 
	if (Nv_Info->Nv_Inherited_Hidden_Flag) {
		*Nv_Status = Nc_WALKTREE_IGNORE_SUBTREE;
		return;
	}

	// If this instance is derived from a null object then don't output it 
	// (any children will be attached to the parent of this derived instance) 
	if (Nv_Info->Nv_Null_Object_To_Follow)
		return;

// RCL: Don't call this for every callback!
//	if (Export_IO_Check_For_User_Interrupt_With_Stats(0, 100))
//		goto error;

	// -------------------->>>>>  Selective Primitive Export Code  <<<---------------------------

	// If you simply want to export all primitives types as polygon meshes then set the '#if 0'
	// to '#if 1' and delete all code between the '#else' and '#endif'. Otherwise, the code
	// after the '#else' statements allows you to selectively pick off specific primitive types to
	// export. For example, your exporter might understand NURBS Surfaces and polygon meshes
	// so this special code below would branch off to your special NURBS Surface output
	// routines if a NURBS Surface primitive was found in this object definition. Any primitive
	// not selectively picked off via the code below will be converted and output as a polygon mesh.

#if 0
	// Convert and output all primitives (NURBS surfaces, closed spline shapes, bicubics, meshes)
	// as polygons meshes.
	do_mesh_output = TRUE;
#else
	// Loop through all geometric primitives (meshes, NURBS, curves) which
	// are stored in this object definition. By modern rules a PolyTrans
	// object definition is only allowed to contain one single primitive,
	// although there might be a rare case where multiple trimmed NURBS patches
	// are stored inside a single object definition.
	for (j=0; j < Nv_Num_Defined_Primitives; ++j) {
		/* Get the type of primitive. Most will be indexed polygon meshes, NURBS surfaces, */
		/* NURBS curves or spline shapes. */
		Ni_Inquire_Primitive(Nt_OBJECT, master_object, j, (Nd_Token *) &primitive_type, Nt_CMDEND);

		// Special case: this is a spline shape primitive. Let's see what we should do with it.
		if (primitive_type == Nt_SPLINESHAPE) {
			Nd_Int		Nv_SplineCurveNum=0;
			Nd_Int		Nv_NumSplineCurvesInShape;
			Nd_Token	Nv_Spline_Renderable, Nv_Spline_Closed;

			// Skip over this primitive if output of 3D spline shapes is disabled
			if (!export_options->ena_independent_3D_Spline_Shapes)
				continue;

			// Let's see if this spline is 'Renderable', meaning that it forms a closed
			// outer loop and can be turned into a polygon mesh or trimmed NURBS surface if need be.
			Ni_Inquire_Primitive(Nt_OBJECT, master_object, j, &primitive_type,
				Nt_SPLINESHAPE, (Nd_Int *) &Nv_SplineCurveNum,
					Nt_NUMCURVES, (Nd_Int *) &Nv_NumSplineCurvesInShape, Nt_CMDSEP,
					Nt_RENDERABLE, Nt_ENABLED, (Nd_Token *) &Nv_Spline_Renderable, Nt_CMDSEP,
					Nt_CMDEND);

			// If the user wants us to output the spline shape as a polygonal mesh or
			// as a trimmed NURBS surface then check here if we can convert the spline
			// shape to a mesh or NURBS surface.
			if ((export_options->RenderableSplineShapeOutputMode == Nc_RENDERABLE_SPLINE_SHAPE_OUTPUT_MODE___AS_POLYGON_MESH && Nv_Spline_Renderable == Nt_ON) ||
					(export_options->RenderableSplineShapeOutputMode == Nc_RENDERABLE_SPLINE_SHAPE_OUTPUT_MODE___AS_TRIMMED_NURBS_SURFACE && Nv_Spline_Renderable == Nt_ON)) {

				// Let's make sure all the curves of the shape are closed.
				for (int Nv_SplineCurveNum=0; Nv_SplineCurveNum < Nv_NumSplineCurvesInShape; ++Nv_SplineCurveNum) {
					Ni_Inquire_Primitive(Nt_OBJECT, master_object, j, &primitive_type,
						/* 'Nv_SplineCurveNum' specifies which spline curve will be queried */
						Nt_SPLINESHAPE, (Nd_Int *) &Nv_SplineCurveNum,
						Nt_CLOSED, Nt_ENABLED, (Nd_Token *) &Nv_Spline_Closed, Nt_CMDSEP,
						Nt_CMDEND);
					if (Nv_Spline_Closed == Nt_OFF)
						// Oops, at least one curve is not closed, so output as a bunch of splines instead.
						goto skip;
				}

				if (export_options->RenderableSplineShapeOutputMode == Nc_RENDERABLE_SPLINE_SHAPE_OUTPUT_MODE___AS_TRIMMED_NURBS_SURFACE)
					// Ok, this spline shape qualifies to be converted into a temporary
					// trimmed NURBS surface for output, so skip to the NURBS surface output section
					// where it will automatically detect this and perform the conversion internally.
					goto output_primitive_type_as_a_nurbs_surface;
				else {
					// Ok, this spline shape qualifies to be converted into a temporary
					// polygon mesh for output, so set a flag so that the mesh output
					// code below turns the spline shape into a polygon mesh and outputs it.
					do_mesh_output = Nc_TRUE;
					continue;
				}
			}

skip:			if (export_options->NonRenderableSplineShapeOutputMode == Nc_NON_RENDERABLE_SPLINE_SHAPE_OUTPUT_MODE__AS_NURBS_CURVES)
				// Let the NURBS curve exporter turn the spline shape into a
				// number of independent NURBS curves for export (considered to be one object)
				NI_Exporter_List_NURBS_Curve_Primitives(ofp, Nv_Info, master_object, j, Nv_Info->Nv_Surface_Name, Nv_Info->Nv_Handle_Name);
			else if (export_options->NonRenderableSplineShapeOutputMode == Nc_NON_RENDERABLE_SPLINE_SHAPE_OUTPUT_MODE__AS_POLYLINES)
				// Else, ask the indexed polyline code to convert the spline shape primitive into a series of line segments
				NI_Exporter_List_IndexedPolyline_Primitive(ofp, Nv_Info, master_object, j, Nv_Info->Nv_Surface_Name, Nv_Info->Nv_Handle_Name);
			else
				// Else, just dump out the spline shape as itself, a bunch of hierarchical, oriented spline curves
				NI_Exporter_List_Spline_Shape_Primitive(ofp, Nv_Info, master_object, j, Nv_Info->Nv_Surface_Name, Nv_Info->Nv_Handle_Name);

		// Special case: this is a NURBS curve primitive. Let's see what we should do with it.
		} else if (primitive_type == Nt_NURBCURVE) {
			Nd_Int		Nv_NURBSCurveNum=0;
			Nd_Int		Nv_NumNURBSCurves;
			Nd_Token	Nv_NURBS_Composite_Curve_Renderable, Nv_Consider_Curves_As_One_Loop;

			// Skip over this primitive if output of 3D NURBS curves is disabled
			if (!export_options->ena_independent_3D_nurbs_curves)
				continue;

			// Let's see if this NURBS curve is 'Renderable', meaning that it forms a closed
			// outer loop and can be turned into a polygon mesh, spline shape or trimmed NURBS surface if need be.
			Ni_Inquire_Primitive(Nt_OBJECT, master_object, j, &primitive_type,
				Nt_NURBCURVE, (Nd_Int *) &Nv_NURBSCurveNum,
					/* Return the number of NURBS curves in this composite curve */
					Nt_NUMCURVES, (Nd_Int *) &Nv_NumNURBSCurves, Nt_CMDSEP,
					/* The composite NURBS curve shape will be rendered as a 3D object if the NURBS curves form a closed loop (first and last CV are the same) */
					Nt_RENDERABLE, Nt_ENABLED, (Nd_Token *) &Nv_NURBS_Composite_Curve_Renderable, Nt_CMDSEP,
					// If set to Nt_ON, the multiple NURBS curves form one composite curve.
					// If set to Nt_OFF, the multuple NURBS curves of this object are independent and form their own closed but oriented loops.
					Nt_CONSIDERCURVESASONELOOP, Nt_ENABLED, (Nd_Token *) &Nv_Consider_Curves_As_One_Loop, Nt_CMDSEP,
					Nt_CMDEND);

			// If the user wants us to output the (renderable) NURBS curve as a polygonal mesh,
			// a trimmed NURBS surface, or a spline shape then check here if we can convert the NURBS
			// curve object into one of these corresponding 3d entities.
			if (Nv_NURBS_Composite_Curve_Renderable == Nt_ON &&
				((export_options->RenderableNURBSCurveOutputMode == Nc_RENDERABLE_NURBS_CURVE_OUTPUT_MODE___AS_POLYGON_MESH) ||
					(export_options->RenderableNURBSCurveOutputMode == Nc_RENDERABLE_NURBS_CURVE_OUTPUT_MODE___AS_TRIMMED_NURBS_SURFACE) ||
					(export_options->RenderableNURBSCurveOutputMode == Nc_RENDERABLE_NURBS_CURVE_OUTPUT_MODE___AS_SPLINE_SHAPE))) {

				if (export_options->RenderableNURBSCurveOutputMode == Nc_RENDERABLE_NURBS_CURVE_OUTPUT_MODE___AS_TRIMMED_NURBS_SURFACE) {
					// First, let's see if we can actually turn the open or closed NURBS curve into a NURBS surface
					Nd_Token 	primitive_type;
					Nd_Int		Nv_Num_U_Control_Points;
					Ni_Inquire_Primitive(Nt_OBJECT, master_object, 0, &primitive_type,
						Nt_TEMPNURBSPATCH,
						Nt_NUMUCTRLPNTS, (Nd_Int *) &Nv_Num_U_Control_Points, Nt_CMDEND);
					// Oops. We can't create a NURBS surface from the NURBS curves, so just output the curves instead.
					if (!Nv_Num_U_Control_Points)
						goto output_curve;

					// Ok, this NURBS Curve qualifies to be converted into a temporary
					// trimmed NURBS surface for output, so skip to the NURBS surface output section
					// where it will automatically detect this and perform the conversion internally.
					goto output_primitive_type_as_a_nurbs_surface;
				} else if (export_options->RenderableNURBSCurveOutputMode == Nc_RENDERABLE_NURBS_CURVE_OUTPUT_MODE___AS_SPLINE_SHAPE) {
					// First, let's see if we can actually turn the open or closed NURBS curve into a spline shape
					Nd_Token 	primitive_type;
					Nd_Int		Nv_Num_SplineCurves_In_SplineShape, Nv_SplineCurveNum=0;
					Ni_Inquire_Primitive(Nt_OBJECT, master_object, 0, &primitive_type,
						Nt_TEMPSPLINESHAPE, (Nd_Int *) &Nv_SplineCurveNum,
						Nt_NUMCURVES, (Nd_Int *) &Nv_Num_SplineCurves_In_SplineShape, Nt_CMDEND);
					// Oops. We can't create a spline shape from the NURBS curves, so just output the curves instead.
					if (!Nv_Num_SplineCurves_In_SplineShape)
						goto output_curve;

					// Let the spline shape output routine convert this closed, renderable NURBS
					// CUrve object into a corresponding spline shape object.
					NI_Exporter_List_Spline_Shape_Primitive(ofp, Nv_Info, master_object, j, Nv_Info->Nv_Surface_Name, Nv_Info->Nv_Handle_Name);
				} else {
					// Ok, this closed NURBS Curve qualifies to be converted into a temporary
					// polygon mesh for output, so set a flag so that the mesh output
					// code below turns the NURBS Curve into a polygon mesh and outputs it.
					do_mesh_output = Nc_TRUE;
					continue;
				}
			}

			if (export_options->NonRenderableNURBSCurveOutputMode == Nc_NON_RENDERABLE_NURBS_CURVE_OUTPUT_MODE__AS_SPLINE_SHAPE)
				// Let the Spline Shape exporter turn the NURBS Curve object into a
				// corresponding Spline Shape object
				NI_Exporter_List_Spline_Shape_Primitive(ofp, Nv_Info, master_object, j, Nv_Info->Nv_Surface_Name, Nv_Info->Nv_Handle_Name);
			else if (export_options->NonRenderableNURBSCurveOutputMode == Nc_NON_RENDERABLE_NURBS_CURVE_OUTPUT_MODE__AS_POLYLINES)
				// Else, ask the indexed polyline code to convert the NURBS curve primitive into a series of line segments
				NI_Exporter_List_IndexedPolyline_Primitive(ofp, Nv_Info, master_object, j, Nv_Info->Nv_Surface_Name, Nv_Info->Nv_Handle_Name);
			else
				// Else, just dump out the NURBS Curve as itself, a bunch of hierarchical, oriented NURBS curves.
				// The NURBS curve output routine will also handle the case where a single NURBS curve is wanted (instead of a composite)
output_curve:			NI_Exporter_List_NURBS_Curve_Primitives(ofp, Nv_Info, master_object, j, Nv_Info->Nv_Surface_Name, Nv_Info->Nv_Handle_Name);

		// Special case: this is an indexed 2D/3D polylines primitive. This has no area so can't be "rendered" in the normal sense
		} else if (primitive_type == Nt_INDEXEDPOLYLINES) {
			if (!export_options->ena_lines)
				continue;

			// Call the output routine in ip_lines.c
			NI_Exporter_List_IndexedPolyline_Primitive(ofp, Nv_Info, master_object, j, Nv_Info->Nv_Surface_Name, Nv_Info->Nv_Handle_Name);

		// Special case: this is a 3D pointset primitive. This has no area so can't be "rendered" in the normal sense
		} else if (primitive_type == Nt_POINTSET) {
			if (!export_options->ena_pointsets)
				continue;

			// Call the output routine in ip_points.c
			NI_Exporter_List_PointSet_Primitive(ofp, Nv_Info, master_object, j, Nv_Info->Nv_Surface_Name, Nv_Info->Nv_Handle_Name);

		// Special case: this is a bicubic patch primitive. Let's see what we should do with it.
		// In general this primitive is never used inside PolyTrans any more because the
		// trimmed NURBS patch (Nt_NURBPATCH) is far superior in terms of functionality.
		} else if (primitive_type == Nt_PATCH) {
			if (!export_options->ena_bicubic_patches)
				continue;

			// If you want to export the bicubic patch as polygons then simply
			// comment this line out and replace it with 'do_mesh_output = Nc_TRUE'.
			NI_Exporter_List_Bicubic_Patch_Primitive(ofp, Nv_Info, master_object, j, Nv_Info->Nv_Surface_Name, Nv_Info->Nv_Handle_Name);

		// Special case: this is a NURBS patch primitive. Let's see what we should do with it.
		} else if (primitive_type == Nt_NURBPATCH) {
output_primitive_type_as_a_nurbs_surface:
			if (!export_options->ena_nurbs_patches)
				continue;

			if (export_options->nurbs_as_polygons)
				// The user wants us to output the NURBS surface as a polygon mesh instead
				// so set a flag so that the mesh output code below turns the NURBS surface
				// into a polygon mesh and outputs it.
				do_mesh_output = Nc_TRUE;
			else {
				if (NI_Exporter_List_NURBS_Surface_Primitive(ofp, Nv_Info, master_object, j, Nv_Info->Nv_Surface_Name, Nv_Info->Nv_Handle_Name))
					// The NURBS surface was invalid so set a flag so that this primitive is output as a mesh object instead.
					do_mesh_output = Nc_TRUE;
			}

		// Else, output the original polygon mesh primitive unchanged, or output any other
		// non-mesh primitive as a polygonal mesh (through internal toolkit conversion facilities)
		} else {
			do_mesh_output = Nc_TRUE;
			OPTIONAL_FPRINTF(ofp, "\tPrimitive # %d of object definition '%s' = %s\n", j+1, master_object, primitive_type);
		}
	}
#endif
	// ----------------->>>>>  End of Selective Primitive Export Code  <<<-----------------------

	if (do_mesh_output == Nc_TRUE && export_options->ena_mesh_data) {
        	// If we get to this point then the primitive (whatever it may be, such
        	// as NURBS patch, bicubic patch, quadric, superquadric, mesh, etc.) will
         	// be turned into an optimized indexed polygon mesh for export.

		if (NI_OutputMesh(ofp, Nv_Info, master_object))	// See top of ip_mesh.cpp
			goto error;
	}

	/* Print out the instance-specific flags and info */
skip_to_attributes:
	if (export_options->ena_instance_attributes) {
		Nd_Token	Nv_CastsshadowsToggle, Nv_HiddenToggle,
				Nv_ShadowedToggle, Nv_Inherited_Hidden_Flag,
				Nv_SkeletonJointNodeEnable;

		Ni_Inquire_Instance(handle_name,
			Nt_CASTSSHADOWS, Nt_ENABLED, (Nd_Token *) &Nv_CastsshadowsToggle, Nt_CMDSEP,
			Nt_HIDDEN, Nt_ENABLED, (Nd_Token *) &Nv_HiddenToggle, Nt_CMDSEP,
			Nt_INHERITHIDDENFLAG, Nt_ENABLED, (Nd_Token *) &Nv_Inherited_Hidden_Flag, Nt_CMDSEP,
			Nt_SHADOWED, Nt_ENABLED, (Nd_Token *) &Nv_ShadowedToggle, Nt_CMDSEP,
			Nt_SKELETON, 
				Nt_JOINTNODE, Nt_ENABLED, (Nd_Token *) &Nv_SkeletonJointNodeEnable, Nt_CMDSEP,
			Nt_CMDEND);

		OPTIONAL_FPRINTF(ofp, "\tInstance flags:\n");
		OPTIONAL_FPRINTF(ofp, "\t\tCast Shadows = %s\n", Nv_CastsshadowsToggle);
		OPTIONAL_FPRINTF(ofp, "\t\tReceive Shadows = %s\n", Nv_ShadowedToggle);
		OPTIONAL_FPRINTF(ofp, "\t\tHidden = %s\n", Nv_HiddenToggle);
		OPTIONAL_FPRINTF(ofp, "\t\tInherit hidden flag = %s\n", Nv_Inherited_Hidden_Flag);
		OPTIONAL_FPRINTF(ofp, "\t\tInstance is a 'skeleton joint/bone node' = %s\n", Nv_SkeletonJointNodeEnable);

		// Output any meta data associated with this instance
		NI_Exporter_Output_Meta_Data(ofp, Nt_INSTANCE, handle_name);
	}

//exit:	
	return;

error:  *Nv_Status = Nc_WALKTREE_ABORT;
}

// -----------------------------------------------------------------------------------------

// This callback is called indirectly by Ni_Walk_Tree() whenever a new level in the */
// hierarchy tree is found (an empty instance or empty object is encountered). */
// and when that level of the tree has completed.
//
// For this example exporter, this routine doesn't do anything. It's included
// here purely for reference. Few exporters need to use this callback routine,
// mostly those which output ASCII text files for which hierarchy levels are
// delimited by tokens (such as { and } ).
//
// Use the "Nt_STARTENDHIERARCHYCB, (void *) NI_SW3D_StartEnd_Hierarchy_Level_Callback"
// argument to specify this callback to the Ni_Walk_Tree() routine.
//
// Arguments:
// 	Nv_Parent_Type
// 		'Nt_INSTANCE' if the parent of this hierarchy level is an empty instance ('yellow folder').
// 		'Nt_OBJECT' if the parent of this hierarchy level is an empty obejct ('red folder').
// 	Nv_Parent_Handle_Name
// 		The instance or object handle name of the parent.
// 	Nv_Current_Hierarchy_Level
// 		This is the new level of the tree hierarchy.
// 		Level 2 = This is the children level attached to the "world" node. The Nv_Parent_Handle_Name is "world".
// 		NOTE: Level 1 is never provided to this callback routine because there are no siblings to the "world" instance.
// 	Nv_Starting_New_Level
// 		TRUE if we are starting a new level else FALSE to finish a level

	static Nd_Void
NI_Exporter_StartEnd_Hierarchy_Level_Callback(Nd_Token Nv_Parent_Type, char *Nv_Parent_Handle_Name, 
	Nd_Short Nv_Current_Hierarchy_Level, Nd_Short Nv_Starting_New_Level)
{
	// Ignore cases where the parent is a red folder 
#if IGNORE_RED_FOLDERS_IN_HIERARCHY
	if (Nv_Parent_Type == Nt_OBJECT)
		return;
#endif
}

// -----------------------------------------------------------------------------------------

// When local memory is allocated and sent to the Ni_AddHierarchyDAGPath()
// toolkit function 'for safe keeping' the following callback function is
// called by the toolkit later to free up this memory once the 
// Ni_FreeHierarchyDAGPathLists() function is called at the end of the export
// process. You don't ever have to touch or modify this function.

	static Nd_Void
NI_Exporter_DAGPath_UserDataMemoryFreeRoutine(void *data)
{
	Ni_Free_Memory((char *) data);
}

// -----------------------------------------------------------------------------------------

/* Utility function: Display a matrix to the text file */

	Nd_Void
NI_Exporter_Display_Matrix(FILE *ofp, Nd_Matrix m_ptr, Nd_Short num_tabs)
{
	short	row, colm, i;

	for (row = 0; row < 4; row++) {
		for (i=0; i < num_tabs; ++i)
			fprintf(ofp, "\t");
		for (colm = 0; colm < 4; colm++)
			fprintf(ofp, "%-8.3g ", m_ptr[row][colm]);
		fprintf(ofp, "\n");
	}
}

// -----------------------------------------------------------------------------------------

// For exporters which will handle object instancing, this is the main helper function which
// all geometry output routines call to determine if the geometry node should be instanced or not.
// It will have to be modified on a case by case basis for each custom exporter.

	Nd_Bool
NI_Handle_Okino_Object_Instancing(Nd_Walk_Tree_Info *Nv_Walk_Tree_Info_Ptr)
{
	void 		*find_result;
	char 		*nrs_object_name;

	if (!export_options->ena_instancing)
		return Nc_FALSE;

	// Let's see how many times this object definition has been instanced in the tree
	// (this variable is only valid when we set 'Nt_ENABLEINSTANCINGDETECTIONCOUNTS' in our main
	// call to Ni_Walk_Tree(). See the code near the top of this file)
	if (Nv_Walk_Tree_Info_Ptr->Nv_ObjectDefinition__InstanceReferenceCountInHierarchyTree < 2)
		return Nc_FALSE;	// We want 2 or more instances before we use instancing

	// We can't instance this object definition if it inherits different instance-level materials
	// to one or more polygons, for 2 or more instances. It is not possible to output one mesh,
	// instance it multiple times, and have each instance have different polygon-level inherited
	// materials. (this variable is only valid when we set 'Nt_ENABLEINSTANCINGDETECTIONCOUNTS' in our
	// main call to Ni_Walk_Tree(). See the code near the top of this file)
	if (Nv_Walk_Tree_Info_Ptr->Nv_ObjectDefinition__AssignsDifferentInstanceLevelMaterialsToTheSamePolygons)
		return Nc_FALSE;

	// Let's see if we've output this object definition before.

	// Get the object definition handle name
	Ni_Inquire_Instance(Nv_Walk_Tree_Info_Ptr->Nv_Handle_Name,
		Nt_MASTEROBJECT, (char **) &nrs_object_name, Nt_CMDSEP,
		Nt_CMDEND);

	// See if this object definition has been output before
	find_result = Ni_StringDictionary_Find(export_options->list_of_multiply_instanced_nugraf_objects, nrs_object_name);
	if (find_result != Ni_StringDictionary_ITEM_NOT_FOUND()) {
//
// Developer: Handling instance output will be very specific to your export file format. The following
// code shows how we do it for the Okino JT exporter. 'pParentAssemblyNode' is a variable we pass
// down the stack all the way to this routine. It defines the parent assembly (group) node in JT.
#if 00
		JtkPart	*original_Part_Defn = (JtkPart *) find_result;

	        // Make a new SketchUp instance
		if (original_Part_Defn) {
		        // Make a new JT instance
			++export_options->total_instances;

			JtkInstance *instance = NULL;
			instance = JtkEntityFactory::createInstance(original_Part_Defn);
			pParentAssemblyNode->addChild(instance);

			// Make sure the JT assembly name becomes unique within the JT namespace
			char instance_name[1024];
			NI_FixUpJTHandleName(Nt_INSTANCE, Nv_Info->Nv_Handle_Name, instance_name);
			instance->setName(instance_name);

			// Set the transform. Create & set the material, and find a good texture definition (optional) to assign to this part
			if (export_options->ena_hierarchy)
				NI_JT_Assign_Transform_Material_Texture_To_Geometry_Part(instance, Nv_Walk_Tree_Info_Ptr->Nv_CTM, Nv_Walk_Tree_Info_Ptr->Nv_Surface_Name);
			else
				NI_JT_Assign_Transform_Material_Texture_To_Geometry_Part(instance, Nv_Walk_Tree_Info_Ptr->Nv_Hierarchical_CTM, Nv_Walk_Tree_Info_Ptr->Nv_Surface_Name);

			return Nc_TRUE;
		}
#endif
	}

	return Nc_FALSE;
}
