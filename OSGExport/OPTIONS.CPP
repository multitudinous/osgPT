/*****************************************************************************

	----------------------------------------------------------------
             (Dialog box) Option High Level Functions for PolyTrans
	----------------------------------------------------------------
	   This module handles the initialization/de-initialization
	   of the global options associated with this exporter (such
	   as the ones shown on the exporter options dialog box) as well
	   as the routines to save/load these options to an .ini file.
	----------------------------------------------------------------

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

	Last change:  RCL  20 May 2001    4:28 am
*****************************************************************************/

#include "main.h"	// Main include file for a PolyTrans exporter

/* ----------------------->>>>  Definitions  <<<<--------------------------- */

/* -------------------->>>>  Local Variables  <<<<-------------------------- */

/* ------------------>>>>  Function Prototypes  <<<<------------------------ */

static Nd_Void	NI_Exporter_Handle_Quoted_INI_String(char *in_string, char **out_string);

/* ------------------------------------------------------------------------- */

/* External entry point to write out the default converter options to */
/* the .INI file. Returns Nc_TRUE if an error, else Nc_FALSE. */

#undef CURR_FN_NAME
#define CURR_FN_NAME	"NI_Exporter_SaveExportConvertOptions()"

	short
NI_Exporter_SaveExportConvertOptions(char *ini_filepath)
{
	FILE   *fp;

	Ni_Convert_Pathname(ini_filepath);
	if ((fp = fopen(ini_filepath, "w")) == (FILE *) NULL) {
		/* %s: Could not open /Text .INI/ file '%s' for writing. */
		Ni_Report_Error(Nc_ERR_ERROR, "OUTERR_FILE_OPEN_WRITE", CURR_FN_NAME, "Text .INI", ini_filepath);
		return (Nc_TRUE);
	}

    fprintf(fp, "# Default options for the OSG exporter.\n");
	fprintf(fp, "# !! Do not modify this file. It is written by the exporter.\n\n");

	fprintf(fp, "[Preferences]\n");
	fprintf(fp, "# OSG options...\n");
    fprintf(fp, "Strip-Texture-Paths = %d\n", export_options->osgStripTexturePaths);
    fprintf(fp, "Normalization = %d\n", export_options->osgNormalization);
    fprintf(fp, "Backface-Culling = %d\n", export_options->osgBackfaceCulling);
    fprintf(fp, "Debug-Text-File = %d\n", export_options->osgDebugTextFile);
    fprintf(fp, "Instance-File = %d\n", export_options->osgInstanceFile);
    fprintf(fp, "Instance-Shared = %d\n", export_options->osgInstanceShared);
    fprintf(fp, "Instance-Ignore = %d\n", export_options->osgInstanceIgnore);

    fprintf(fp, "Run-Optimizer = %d\n", export_options->osgRunOptimizer);
    fprintf(fp, "Create-Triangle-Strips = %d\n", export_options->osgCreateTriangleStrips);
    fprintf(fp, "Merge-Geometry = %d\n", export_options->osgMergeGeometry);

    fprintf(fp, "# Non-OSG options...\n");
	fprintf(fp, "Output-Mesh-Data = %d\n", export_options->ena_mesh_data);
	fprintf(fp, "Output-Mesh-Skinning-Weights = %d\n", export_options->ena_mesh_skinning_weights);
	fprintf(fp, "Output-Hierarchy = %d\n", export_options->ena_hierarchy);
	fprintf(fp, "Output-Cameras = %d\n", export_options->ena_cameras);
	fprintf(fp, "Output-Lights = %d\n", export_options->ena_lights);
	fprintf(fp, "Output-Texture-Coords = %d\n", export_options->ena_texture_coords);
	fprintf(fp, "Output-Materials = %d\n", export_options->ena_materials);
	fprintf(fp, "ena_nurbs_patches = %d\n", export_options->ena_nurbs_patches);
	fprintf(fp, "ena_lines = %d\n", export_options->ena_lines);
	fprintf(fp, "ena_pointsets = %d\n", export_options->ena_pointsets);
	fprintf(fp, "ena_instancing = %d\n", export_options->ena_instancing);

	fprintf(fp, "Use-Absolute-File-Paths = %d\n", export_options->use_absolute_file_paths);
	fprintf(fp, "Ena-Vertex-Normals = %d\n", export_options->ena_vertex_normals);
	fprintf(fp, "Export-Colors = %d\n", export_options->output_vertex_colors);
	fprintf(fp, "Convert-To-Unix-Paths = %d\n", export_options->convert_to_unix_paths);
	fprintf(fp, "Convert-To-RFC1738-Paths = %d\n", export_options->convert_paths_to_rfc1738_standard);
	fprintf(fp, "ConvertTxtrBitmaps = %d\n", export_options->ena_convert_txtr_bitmaps);
	fprintf(fp, "ConfirmBitmapOverwrites = %d\n", export_options->ena_confirm_bitmap_overwrites);
	fprintf(fp, "UseSpecificBitmapExtension = %d\n", export_options->ena_use_specific_bitmap_extension);
	fprintf(fp, "StripAbsFilePaths = %d\n", export_options->ena_strip_abs_filepaths	);
	fprintf(fp, "TxtrBitmapDepth = %d\n", export_options->txtr_bitmap_depth);
	fprintf(fp, "Txtr-X-Res = %d\n", export_options->txtr_x_res);
	fprintf(fp, "Txtr-Y-Res = %d\n", export_options->txtr_y_res);
	fprintf(fp, "Save-Bitmaps-Directory-Type = %d\n", export_options->bitmap_saveto_directory_type);
	fprintf(fp, "ena_uv_tangents = %d\n", export_options->ena_uv_tangents);
	fprintf(fp, "nurbs_as_polygons = %d\n", export_options->nurbs_as_polygons);
	fprintf(fp, "ena_trim_curves = %d\n", export_options->ena_trim_curves);
	fprintf(fp, "ena_bicubic_patches = %d\n", export_options->ena_bicubic_patches);
	fprintf(fp, "ena_independent_3D_nurbs_curves = %d\n", export_options->ena_independent_3D_nurbs_curves);
	fprintf(fp, "ena_independent_3D_Spline_Shapes = %d\n", export_options->ena_independent_3D_Spline_Shapes);
	if (!export_options->spline_to_spline_conversion_type)
		fprintf(fp, "spline_to_spline_conversion_type = NoChange\n");
	else
		fprintf(fp, "spline_to_spline_conversion_type = %s\n", export_options->spline_to_spline_conversion_type);
	fprintf(fp, "RenderableNURBSCurveOutputMode = %d\n", export_options->RenderableNURBSCurveOutputMode);
	fprintf(fp, "NonRenderableNURBSCurveOutputMode = %d\n", export_options->NonRenderableNURBSCurveOutputMode);
	fprintf(fp, "RenderableSplineShapeOutputMode = %d\n", export_options->RenderableSplineShapeOutputMode);
	fprintf(fp, "NonRenderableSplineShapeOutputMode = %d\n", export_options->NonRenderableSplineShapeOutputMode);

	fprintf(fp, "ena_surftxtr_layers = %d\n", export_options->ena_surftxtr_layers);
	fprintf(fp, "ena_texture_definitions = %d\n", export_options->ena_texture_definitions);
	fprintf(fp, "ena_instance_attributes = %d\n", export_options->ena_instance_attributes);
	fprintf(fp, "ena_texture_proj_info = %d\n", export_options->ena_texture_proj_info);
	fprintf(fp, "ena_env_info = %d\n", export_options->ena_env_info);
	fprintf(fp, "ena_background_info = %d\n", export_options->ena_background_info);
	fprintf(fp, "mesh_processing_always_add_vertex_normals = %d\n", export_options->mesh_processing_always_add_vertex_normals);
	fprintf(fp, "mesh_processing_optimize_texture_coordinate_list = %d\n", export_options->mesh_processing_optimize_texture_coordinate_list);
	fprintf(fp, "mesh_processing_duplicate_vertices_for_uv_texture_mapping = %d\n", export_options->mesh_processing_duplicate_vertices_for_uv_texture_mapping);
	fprintf(fp, "mesh_processing_explode_meshes_by_material_name = %d\n", export_options->mesh_processing_explode_meshes_by_material_name);
	fprintf(fp, "mesh_processing_transform_to_worldspace = %d\n", export_options->mesh_processing_transform_to_worldspace);
	fprintf(fp, "mesh_processing_sort_polygons_by_material_assignments = %d\n", export_options->mesh_processing_sort_polygons_by_material_assignments);
	fprintf(fp, "mesh_processing_want_planar_polygons_only = %d\n", export_options->mesh_processing_want_planar_polygons_only);
	fprintf(fp, "mesh_processing_want_planar_polygons_only_tolerance = %g\n", export_options->mesh_processing_want_planar_polygons_only_tolerance);
	fprintf(fp, "polygon_reduction_ena = %d\n", export_options->polygon_reduction_ena);
	fprintf(fp, "ena_object_animation = %d\n", export_options->ena_object_animation);
	fprintf(fp, "ena_camera_animation = %d\n", export_options->ena_camera_animation);
	fprintf(fp, "ena_light_animation = %d\n", export_options->ena_light_animation);
	fprintf(fp, "ena_material_animation = %d\n", export_options->ena_material_animation);
	fprintf(fp, "use_independent_controllers_for_tripled_anim_channels = %d\n", export_options->use_independent_controllers_for_tripled_anim_channels);
	fprintf(fp, "export_raw_unsampled_anim_data_if_possible = %d\n", export_options->export_raw_unsampled_anim_data_if_possible);
	fprintf(fp, "interpolation_type_to_use_during_export = %d\n", export_options->interpolation_type_to_use_during_export);
	fprintf(fp, "rotation_controller_type_to_use_during_export = %d\n", export_options->rotation_controller_type_to_use_during_export);
	fprintf(fp, "ena_object_anim_resampled_rot_order = %d\n", export_options->ena_object_anim_resampled_rot_order);
	fprintf(fp, "flip_Euler_180flips_via_resampler = %d\n", export_options->flip_Euler_180flips_via_resampler);

	if (export_options->save_bitmaps_to_specific_directory == NULL)
		fprintf(fp, "Bitmaps-Specific-Save-Directory = \"\"\n");
	else
		fprintf(fp, "Bitmaps-Specific-Save-Directory = \"%s\"\n", export_options->save_bitmaps_to_specific_directory);
	if (export_options->specific_filepath_to_prepend == NULL)
		fprintf(fp, "Prepend-Directory-Path = \"\"\n");
	else
		fprintf(fp, "Prepend-Directory-Path = \"%s\"\n", export_options->specific_filepath_to_prepend);
	if (export_options->specific_relative_filepath_to_prepend == NULL)
		fprintf(fp, "Prepend-Relative-Directory-Path = \"\"\n");
	else
		fprintf(fp, "Prepend-Relative-Directory-Path = \"%s\"\n", export_options->specific_relative_filepath_to_prepend);
	if (export_options->autoconvert_bitmap_format == NULL)
		fprintf(fp, "Autoconvert-Bitmap-Format = \"\"\n");
	else
		fprintf(fp, "Autoconvert-Bitmap-Format = \"%s\"\n", export_options->autoconvert_bitmap_format);
	if (export_options->convert_file_extension == NULL)
		fprintf(fp, "Convert-Bitmap-Extension = \"\"\n");
	else
		fprintf(fp, "Convert-Bitmap-Extension = \"%s\"\n", export_options->convert_file_extension);
	if (export_options->url_string == NULL)
		fprintf(fp, "URL-String = \"\"\n");
	else
		fprintf(fp, "URL-String = \"%s\"\n", export_options->url_string);

	fflush(fp);
	fclose(fp);
	return (Nc_FALSE);
}
#undef CURR_FN_NAME

/* External entry point to read in the default converter options from the */
/* .INI file. Returns Nc_TRUE Nc_if an error, else Nc_FALSE. */

	short
NI_Exporter_RestoreExportConvertOptions(char *ini_filepath)
{
	char    token[128], data[128];
	FILE   *fp;

	Ni_Convert_Pathname(ini_filepath);
	if ((fp = fopen(ini_filepath, "r")) == NULL)
		return (Nc_TRUE);
	while (!feof(fp)) {
		if (!Import_IO_Get_INI_File_Entry(fp, token, data))
			goto exit;

        // OSG...
		if (!strcmp(token, "Strip-Texture-Paths"))
            export_options->osgStripTexturePaths = atoi(data);
		else if (!strcmp(token, "Normalization"))
            export_options->osgNormalization = atoi(data);
		else if (!strcmp(token, "Backface-Culling"))
            export_options->osgBackfaceCulling = atoi(data);
		else if (!strcmp(token, "Debug-Text-File"))
            export_options->osgDebugTextFile = atoi(data);
		else if (!strcmp(token, "Instance-File"))
            export_options->osgInstanceFile = atoi(data);
		else if (!strcmp(token, "Instance-Shared"))
            export_options->osgInstanceShared = atoi(data);
		else if (!strcmp(token, "Instance-Ignore"))
            export_options->osgInstanceIgnore = atoi(data);

		else if (!strcmp(token, "Run-Optimizer"))
            export_options->osgRunOptimizer = atoi(data);
		else if (!strcmp(token, "Create-Triangle-Strips"))
            export_options->osgCreateTriangleStrips = atoi(data);
		else if (!strcmp(token, "Merge-Geometry"))
			export_options->osgMergeGeometry = atoi(data);

        // Non-OSG...
		else if (!strcmp(token, "Output-Mesh-Data"))
			export_options->ena_mesh_data = atoi(data);
		else if (!strcmp(token, "Output-Mesh-Skinning-Weights"))
			export_options->ena_mesh_skinning_weights = atoi(data);
		else if (!strcmp(token, "Output-Hierarchy"))
			export_options->ena_hierarchy = atoi(data);
		else if (!strcmp(token, "Output-Cameras"))
			export_options->ena_cameras = atoi(data);
		else if (!strcmp(token, "Output-Lights"))
			export_options->ena_lights = atoi(data);
		else if (!strcmp(token, "Output-Texture-Coords"))
			export_options->ena_texture_coords = atoi(data);
		else if (!strcmp(token, "Output-Materials"))
			export_options->ena_materials = atoi(data);
		else if (!strcmp(token, "Use-Absolute-File-Paths"))
			export_options->use_absolute_file_paths = atoi(data);
		else if (!strcmp(token, "Ena-Vertex-Normals"))
			export_options->ena_vertex_normals = atoi(data);
		else if (!strcmp(token, "Export-Colors"))
			export_options->output_vertex_colors = atoi(data);
		else if (!strcmp(token, "Convert-To-Unix-Paths"))
			export_options->convert_to_unix_paths = atoi(data);
		else if (!strcmp(token, "Convert-To-RFC1738-Paths"))
			export_options->convert_paths_to_rfc1738_standard = atoi(data);
		else if (!strcmp(token, "ConvertTxtrBitmaps"))
			export_options->ena_convert_txtr_bitmaps = atoi(data);
		else if (!strcmp(token, "ConfirmBitmapOverwrites"))
			export_options->ena_confirm_bitmap_overwrites = atoi(data);
		else if (!strcmp(token, "UseSpecificBitmapExtension"))
			export_options->ena_use_specific_bitmap_extension = atoi(data);
		else if (!strcmp(token, "StripAbsFilePaths"))
			export_options->ena_strip_abs_filepaths = atoi(data);	
		else if (!strcmp(token, "TxtrBitmapDepth"))
			export_options->txtr_bitmap_depth = atoi(data);
		else if (!strcmp(token, "Txtr-X-Res"))
			export_options->txtr_x_res = atoi(data);
		else if (!strcmp(token, "Txtr-Y-Res"))
			export_options->txtr_y_res = atoi(data);
		else if (!strcmp(token, "Save-Bitmaps-Directory-Type"))
			export_options->bitmap_saveto_directory_type = atoi(data);
		else if (!strcmp(token, "ena_uv_tangents"))
			export_options->ena_uv_tangents = atoi(data);
		else if (!strcmp(token, "nurbs_as_polygons"))
			export_options->nurbs_as_polygons = atoi(data);
		else if (!strcmp(token, "ena_trim_curves"))
			export_options->ena_trim_curves = atoi(data);
		else if (!strcmp(token, "ena_surftxtr_layers"))
			export_options->ena_surftxtr_layers = atoi(data);
		else if (!strcmp(token, "ena_texture_definitions"))
			export_options->ena_texture_definitions = atoi(data);
		else if (!strcmp(token, "ena_instance_attributes"))
			export_options->ena_instance_attributes = atoi(data);
		else if (!strcmp(token, "ena_texture_proj_info"))
			export_options->ena_texture_proj_info = atoi(data);
		else if (!strcmp(token, "ena_env_info"))
			export_options->ena_env_info = atoi(data);
		else if (!strcmp(token, "ena_background_info"))
			export_options->ena_background_info = atoi(data);
		else if (!strcmp(token, "ena_lines"))
			export_options->ena_lines = atoi(data);
		else if (!strcmp(token, "ena_pointsets"))
			export_options->ena_pointsets = atoi(data);
		else if (!strcmp(token, "ena_instancing"))
			export_options->ena_instancing = atoi(data);

		else if (!strcmp(token, "ena_bicubic_patches"))
			export_options->ena_bicubic_patches = atoi(data);
		else if (!strcmp(token, "ena_independent_3D_nurbs_curves"))
			export_options->ena_independent_3D_nurbs_curves = atoi(data);
		else if (!strcmp(token, "ena_independent_3D_Spline_Shapes"))
			export_options->ena_independent_3D_Spline_Shapes = atoi(data);
		else if (!strcmp(token, "RenderableNURBSCurveOutputMode"))
			export_options->RenderableNURBSCurveOutputMode = atoi(data);
		else if (!strcmp(token, "NonRenderableNURBSCurveOutputMode"))
			export_options->NonRenderableNURBSCurveOutputMode = atoi(data);
		else if (!strcmp(token, "RenderableSplineShapeOutputMode"))
			export_options->RenderableSplineShapeOutputMode = atoi(data);
		else if (!strcmp(token, "NonRenderableSplineShapeOutputMode"))
			export_options->NonRenderableSplineShapeOutputMode = atoi(data);
		else if (!strcmp(token, "ena_nurbs_patches"))
			export_options->ena_nurbs_patches = atoi(data);
		else if (!strcmp(token, "spline_to_spline_conversion_type")) {
			if (!strcmp(data, "NoChange"))
				export_options->spline_to_spline_conversion_type = NULL;
			else
				export_options->spline_to_spline_conversion_type = Ni_Inquire_Token(data);
		}
		else if (!strcmp(token, "polygon_reduction_ena"))
			export_options->polygon_reduction_ena = atoi(data);
		else if (!strcmp(token, "mesh_processing_always_add_vertex_normals"))
			export_options->mesh_processing_always_add_vertex_normals = atoi(data);
		else if (!strcmp(token, "mesh_processing_optimize_texture_coordinate_list"))
			export_options->mesh_processing_optimize_texture_coordinate_list = atoi(data);
		else if (!strcmp(token, "mesh_processing_duplicate_vertices_for_uv_texture_mapping"))
			export_options->mesh_processing_duplicate_vertices_for_uv_texture_mapping = atoi(data);
		else if (!strcmp(token, "mesh_processing_explode_meshes_by_material_name"))
			export_options->mesh_processing_explode_meshes_by_material_name = atoi(data);
		else if (!strcmp(token, "mesh_processing_transform_to_worldspace"))
			export_options->mesh_processing_transform_to_worldspace = atoi(data);
		else if (!strcmp(token, "mesh_processing_sort_polygons_by_material_assignments"))
			export_options->mesh_processing_sort_polygons_by_material_assignments = atoi(data);
		else if (!strcmp(token, "mesh_processing_want_planar_polygons_only"))
			export_options->mesh_processing_want_planar_polygons_only = atoi(data);
		else if (!strcmp(token, "mesh_processing_want_planar_polygons_only_tolerance"))
			export_options->mesh_processing_want_planar_polygons_only_tolerance = atof(data);

		else if (!strcmp(token, "ena_object_animation"))
		 	export_options->ena_object_animation = atoi(data);
		else if (!strcmp(token, "ena_camera_animation"))
			export_options->ena_camera_animation = atoi(data);
		else if (!strcmp(token, "ena_light_animation"))
			export_options->ena_light_animation = atoi(data);
		else if (!strcmp(token, "ena_material_animation"))
			export_options->ena_material_animation = atoi(data);
		else if (!strcmp(token, "use_independent_controllers_for_tripled_anim_channels"))
			export_options->use_independent_controllers_for_tripled_anim_channels = atoi(data);
		else if (!strcmp(token, "export_raw_unsampled_anim_data_if_possible"))
			export_options->export_raw_unsampled_anim_data_if_possible = atoi(data);
		else if (!strcmp(token, "interpolation_type_to_use_during_export"))
			export_options->interpolation_type_to_use_during_export = atoi(data);
		else if (!strcmp(token, "rotation_controller_type_to_use_during_export"))
			export_options->rotation_controller_type_to_use_during_export = atoi(data);
		else if (!strcmp(token, "ena_object_anim_resampled_rot_order"))
			export_options->ena_object_anim_resampled_rot_order = atoi(data);
		else if (!strcmp(token, "flip_Euler_180flips_via_resampler"))
			export_options->flip_Euler_180flips_via_resampler = atoi(data);

		else if (!strcmp(token, "Bitmaps-Specific-Save-Directory") && data && strlen(data))
			NI_Exporter_Handle_Quoted_INI_String(data, &export_options->save_bitmaps_to_specific_directory);
		else if (!strcmp(token, "Prepend-Directory-Path") && data && strlen(data))
			NI_Exporter_Handle_Quoted_INI_String(data, &export_options->specific_filepath_to_prepend);
		else if (!strcmp(token, "Prepend-Relative-Directory-Path") && data && strlen(data))
			NI_Exporter_Handle_Quoted_INI_String(data, &export_options->specific_relative_filepath_to_prepend);
		else if (!strcmp(token, "Autoconvert-Bitmap-Format") && data && strlen(data))
			NI_Exporter_Handle_Quoted_INI_String(data, &export_options->autoconvert_bitmap_format);
		else if (!strcmp(token, "Convert-Bitmap-Extension") && data && strlen(data))
			NI_Exporter_Handle_Quoted_INI_String(data, &export_options->convert_file_extension);
		else if (!strcmp(token, "URL-String") && data && strlen(data))
			NI_Exporter_Handle_Quoted_INI_String(data, &export_options->url_string);
	}

exit:	fclose(fp);
	return (Nc_FALSE);
}

/* Remove the quotes from a string in the .ini file. If resulting length */
/* is zero then delete the output string. */

	static Nd_Void
NI_Exporter_Handle_Quoted_INI_String(char *in_string, char **out_string)
{
	char	*start;

	if (in_string[0] == '\"')
		start = in_string+1;
	else
		start = in_string;
	if (in_string[strlen(in_string)-1] == '\"')
		in_string[strlen(in_string)-1] = '\0';
	if (!strlen(start)) {
		if (*out_string != NULL) {
			Ni_Free_Memory(*out_string);
			*out_string = NULL;
		}
	} else
		Import_IO_Copy_String(out_string, start);
}

/* This initializes the global options of this exporter */

	short
NI_Exporter_InitExportOptionVariables()
{
	// Allocate space for the variables
	if (!export_options) {
		export_options = (Exporter_Options *) Ni_Alloc_Memory(Nc_TRUE, Nc_TRUE, sizeof(Exporter_Options));
		if (!export_options)
			return(Nc_FALSE);
	}

	if (!export_options->options_initialized) {
		export_options->options_initialized = Nc_TRUE;

        // OSG...
        export_options->osgStripTexturePaths = Nc_TRUE;
        export_options->osgNormalization = Nc_TRUE;
        export_options->osgBackfaceCulling = Nc_TRUE;
        export_options->osgDebugTextFile = Nc_FALSE;
        export_options->osgInstanceFile = Nc_FALSE; // write them as separate files
        export_options->osgInstanceShared = Nc_TRUE; // share instances within a single-file scene graph
        export_options->osgInstanceIgnore = Nc_FALSE; // ignore (copy) instances.

        export_options->osgRunOptimizer = Nc_TRUE;
        export_options->osgCreateTriangleStrips = Nc_FALSE;
        export_options->osgMergeGeometry = Nc_TRUE;

        // Non-OSG...
        export_options->ena_hierarchy = Nc_TRUE; 
		export_options->ena_cameras = Nc_TRUE;
		export_options->ena_lights = Nc_TRUE;
		export_options->ena_materials = Nc_TRUE;
		export_options->ena_bicubic_patches = Nc_TRUE;
		export_options->ena_pointsets = Nc_TRUE;
		export_options->ena_lines = Nc_TRUE;
		export_options->ena_instancing = Nc_TRUE;

		export_options->ena_mesh_data = Nc_TRUE;
		export_options->ena_mesh_skinning_weights = Nc_FALSE;
		export_options->ena_vertex_normals = Nc_TRUE;
		export_options->ena_texture_coords = Nc_TRUE;
		export_options->output_vertex_colors = Nc_FALSE;
		export_options->ena_uv_tangents = Nc_TRUE;
		export_options->ena_instance_attributes = Nc_TRUE;

		export_options->ena_nurbs_patches = Nc_TRUE;
		export_options->nurbs_as_polygons = Nc_FALSE;
		export_options->ena_trim_curves = Nc_TRUE;

		export_options->ena_surftxtr_layers = Nc_TRUE;
		export_options->ena_texture_definitions = Nc_TRUE;
		export_options->ena_texture_proj_info = Nc_TRUE;
		export_options->ena_env_info = Nc_TRUE;
		export_options->ena_background_info = Nc_TRUE;

		/* NURBS curves */
		export_options->ena_independent_3D_nurbs_curves = Nc_TRUE;
		export_options->RenderableNURBSCurveOutputMode = Nc_RENDERABLE_NURBS_CURVE_OUTPUT_MODE___UNCHANGED;
		export_options->NonRenderableNURBSCurveOutputMode = Nc_NON_RENDERABLE_NURBS_CURVE_OUTPUT_MODE__UNCHANGED;

		/* Spline Shapes */
		export_options->ena_independent_3D_Spline_Shapes = Nc_TRUE;
		export_options->spline_to_spline_conversion_type = NULL;	// No change
		export_options->RenderableSplineShapeOutputMode = Nc_RENDERABLE_SPLINE_SHAPE_OUTPUT_MODE___UNCHANGED;
		export_options->NonRenderableSplineShapeOutputMode = Nc_NON_RENDERABLE_SPLINE_SHAPE_OUTPUT_MODE__UNCHANGED;

		export_options->mesh_processing_always_add_vertex_normals = Nc_TRUE;
		export_options->mesh_processing_optimize_texture_coordinate_list = Nc_TRUE;
		export_options->mesh_processing_duplicate_vertices_for_uv_texture_mapping = Nc_FALSE;
		export_options->mesh_processing_explode_meshes_by_material_name = Nc_TRUE;
		export_options->mesh_processing_transform_to_worldspace = Nc_FALSE;
		export_options->mesh_processing_sort_polygons_by_material_assignments = Nc_TRUE;
		export_options->polygon_reduction_ena = Nc_FALSE;
		export_options->mesh_processing_want_planar_polygons_only = Nc_FALSE;
	 	export_options->mesh_processing_want_planar_polygons_only_tolerance = 1e-4;

	 	export_options->ena_object_animation = Nc_FALSE;
		export_options->ena_camera_animation = Nc_FALSE;
		export_options->ena_light_animation = Nc_FALSE;
		export_options->ena_material_animation = Nc_FALSE;

		// If export XYZ (3 channel) animation, such as XYZ translation, setting this to TRUE will cause
		// the 3 channels to be exported using 3 independent controllers animation channels. 
		// If set to FALSE then the internal toolkit will be asked to make sure that the 3
		// keyframe lists have the same key times and the same number of keys (using temp. key lists) so
		// that we can properly export the animation using a single keyframe controller.
		export_options->use_independent_controllers_for_tripled_anim_channels = Nc_TRUE;

		// If set to TRUE then the raw toolkit keyframe data will be attempted to be exported
		// otherwise the resampled versions will be used.
		export_options->export_raw_unsampled_anim_data_if_possible = Nc_FALSE;

		// This is the interpolation controller to create when exporting resampled animation 
		// (Nc_NRS_ANIM_INTERPOLATION_TYPE_ORIGINAL, Nc_NRS_ANIM_INTERPOLATION_TYPE_CONSTANT, Nc_NRS_ANIM_INTERPOLATION_TYPE_LINEAR, Nc_NRS_ANIM_INTERPOLATION_TYPE_TCB, or Nc_NRS_ANIM_INTERPOLATION_TYPE_BEZIER)
		export_options->interpolation_type_to_use_during_export = Nc_NRS_ANIM_INTERPOLATION_TYPE_ORIGINAL;	// Use the same interpolation type as the source function curve

		// This will try to "fix" Euler rotation curves upon export (when
		// resampling is used) where the angles flip back and forth on either
		// side of -180/180 degrees. 
		export_options->flip_Euler_180flips_via_resampler = Nc_FALSE;

		// This is the rotation controller type to create when exporting resampled animation data 
		// (Nc_NRS_ROTATION_CONTROLLER_QUATERNION or Nc_NRS_ROTATION_CONTROLLER_EULER)
		export_options->rotation_controller_type_to_use_during_export = Nc_NRS_ROTATION_CONTROLLER_QUATERNION;

		// This is the desired Euler rotation order when the Euler rotation
		// keyframe resampler is enabled. In most cases you will just want to
		// hard code this in an exporter and not make it available to the end-user.
		export_options->ena_object_anim_resampled_rot_order = Nc_EulerRotationOrder_XYZ;
	
		export_options->txtr_x_res = 0;	/* Closest */
		export_options->txtr_y_res = 0;	/* Closest */
		export_options->txtr_bitmap_depth = 24;
		export_options->ena_confirm_bitmap_overwrites = Nc_TRUE;
		export_options->ena_convert_txtr_bitmaps = Nc_FALSE;
		export_options->ena_use_specific_bitmap_extension = Nc_FALSE;
		export_options->bitmap_saveto_directory_type = SAVE_BITMAPS_TO_ORIGINAL_DIRECTORY;
		export_options->specific_filepath_to_prepend = NULL;
		export_options->specific_relative_filepath_to_prepend = NULL;
		export_options->save_bitmaps_to_specific_directory = NULL;
		export_options->autoconvert_bitmap_format = NULL;
		Import_IO_Copy_String(&export_options->autoconvert_bitmap_format, ".jpg");	// NOTE: this can also be the GUID of the 2D file exporter, such as 'Nc_NG_TOOLKIT_IMAGE_EXPORTER_GUID__JPEG'
		export_options->convert_file_extension = NULL;
		Import_IO_Copy_String(&export_options->convert_file_extension, ".jpg");
		export_options->url_string = NULL;
		export_options->export_directory = NULL;	// A runtime string
	
		export_options->convert_to_unix_paths = Nc_FALSE;
		export_options->convert_paths_to_rfc1738_standard = Nc_FALSE;
		export_options->ena_strip_abs_filepaths = Nc_FALSE;
		export_options->use_absolute_file_paths = Nc_FALSE;
	}

	return(Nc_TRUE);
}

/* This de-initializes the global options of this exporter */

	Nd_Void
NI_Exporter_DeInitExportOptionVariables()
{
	if (!export_options)
		return;

	if (export_options->autoconvert_bitmap_format)
		Ni_Free_Memory(export_options->autoconvert_bitmap_format);
	if (export_options->convert_file_extension)
		Ni_Free_Memory(export_options->convert_file_extension);
	if (export_options->specific_filepath_to_prepend)
		Ni_Free_Memory(export_options->specific_filepath_to_prepend);
	if (export_options->specific_relative_filepath_to_prepend)
		Ni_Free_Memory(export_options->specific_relative_filepath_to_prepend);
	if (export_options->save_bitmaps_to_specific_directory)
		Ni_Free_Memory(export_options->save_bitmaps_to_specific_directory);
	if (export_options->url_string)
		Ni_Free_Memory(export_options->url_string);
	if (export_options->export_directory)
		Ni_Free_Memory(export_options->export_directory);

	Ni_Free_Memory(export_options);

	export_options = NULL;
}
