/*****************************************************************************

	----------------------------------------------------------------
                   Main Header File For a PolyTrans Exporter
	----------------------------------------------------------------

  Copyright (c) 1988, 2006 Okino Computer Graphics, Inc. All Rights Reserved.
	  See the file main.cpp for Copyright and Ownership Descriptions

	Last change:  RCL  17 Jan 2006    8:56 am
*****************************************************************************/

#ifndef __OSG_MAIN_H__
#define __OSG_MAIN_H__


#include <ni.h>		// Okino NuGraf toolkit main include file
#include <ni4_aux.h>	// Okino NuGraf toolkit auxiliary include file

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <importio.h>	// Okino Import I/O cover function library
#include <im_ini.h>	// Okino .INI file common parsing code from Import I/O convert library

#ifndef UNIX
#include	<direct.h>		/* For mkdir() */
#endif

/* ---------------------->>>>  Definitions  <<<<--------------------------- */

// If you don't want text sent to the file, then uncomment the second lines
// which will basically nullify all of the fprintf() and Export_IO_Display_Matrix() statements
#define		OPTIONAL_FPRINTF		fprintf	
#define		OPTIONAL_DISPLAY_MATRIX		NI_Exporter_Display_Matrix
//#define	OPTIONAL_FPRINTF		(void)
//#define	OPTIONAL_DISPLAY_MATRIX		(void)

// Name of the export converter help file. Needs to be modified in the VC++ project as well
#define	HELP_FILENAME	"exp_isu_osg.hlp"
// Name of the export converter .ini file to save the global options
#define	INI_FILENAME	"exp_isu_osg.ini"

/* These are the destinations for automatic bitmap conversions */
#define SAVE_BITMAPS_TO_ORIGINAL_DIRECTORY  0
#define SAVE_BITMAPS_TO_SPECIFIC_DIRECTORY  1

/* ---------------->>>>  Anmiation Conversion Defines  <<<<------------------ */

// These define is resampled rotations will be exported as quaternions or Euler angles
#define Nc_NRS_ROTATION_CONTROLLER_QUATERNION		0
#define Nc_NRS_ROTATION_CONTROLLER_EULER		1
#define Nc_NRS_ROTATION_CONTROLLER_AS_PER_NRS_TOOLKIT	2	// Use either Euler or Quat, depending upon which was read from the Okino toolkit

// These define which interpolation controller will be used when exporting animation 
#define Nc_NRS_ANIM_INTERPOLATION_TYPE_ORIGINAL	0
#define Nc_NRS_ANIM_INTERPOLATION_TYPE_CONSTANT	1
#define Nc_NRS_ANIM_INTERPOLATION_TYPE_LINEAR	2
#define Nc_NRS_ANIM_INTERPOLATION_TYPE_TCB	3
#define Nc_NRS_ANIM_INTERPOLATION_TYPE_BEZIER	4

/* ---------------->>>>  Spline Shape Conversion Modes  <<<<------------------ */

// export_options->RenderableSplineShapeOutputMode
#define Nc_RENDERABLE_SPLINE_SHAPE_OUTPUT_MODE___UNCHANGED			0
#define Nc_RENDERABLE_SPLINE_SHAPE_OUTPUT_MODE___AS_POLYGON_MESH		1
#define Nc_RENDERABLE_SPLINE_SHAPE_OUTPUT_MODE___AS_TRIMMED_NURBS_SURFACE	2

// export_options->NonRenderableSplineShapeOutputMode
#define Nc_NON_RENDERABLE_SPLINE_SHAPE_OUTPUT_MODE__UNCHANGED			0
#define Nc_NON_RENDERABLE_SPLINE_SHAPE_OUTPUT_MODE__AS_NURBS_CURVES		1
#define Nc_NON_RENDERABLE_SPLINE_SHAPE_OUTPUT_MODE__AS_POLYLINES		2

/* ---------------->>>>  NURBS Curve Conversion Modes  <<<<------------------ */

// export_options->RenderableNURBSCurveOutputMode
#define Nc_RENDERABLE_NURBS_CURVE_OUTPUT_MODE___UNCHANGED			0
#define Nc_RENDERABLE_NURBS_CURVE_OUTPUT_MODE___AS_POLYGON_MESH			1
#define Nc_RENDERABLE_NURBS_CURVE_OUTPUT_MODE___AS_TRIMMED_NURBS_SURFACE	2
#define Nc_RENDERABLE_NURBS_CURVE_OUTPUT_MODE___AS_SPLINE_SHAPE			3

// export_options->NonRenderableNURBSCurveOutputMode
#define Nc_NON_RENDERABLE_NURBS_CURVE_OUTPUT_MODE__UNCHANGED			0
#define Nc_NON_RENDERABLE_NURBS_CURVE_OUTPUT_MODE__COMPOSITE_TO_SINGLE_CURVE	1
#define Nc_NON_RENDERABLE_NURBS_CURVE_OUTPUT_MODE__AS_SPLINE_SHAPE		2
#define Nc_NON_RENDERABLE_NURBS_CURVE_OUTPUT_MODE__AS_POLYLINES			3

/* -------------------->>>>  Dialog Box Options  <<<<----------------------- */

/* Export converter dialog box variables */

typedef struct Exporter_Options {
	/* Nc_TRUE once these option variables have been initialized */
	char 	options_initialized;

	// Stats
	int
		total_meshes,
		total_polygons,
		total_polylines,
                total_nurbs_curves,
                total_nurbs_surfaces,
		total_spline_shapes,
		total_pointsets,
		total_pointset_points,
		total_materials,
		total_texture_maps,
		total_groups,
		total_instances;

	// Used to keep track of which NuGraf objects have already been output,
	// those which have 2 or more instances pointing at them.
	Nd_StringDictionary list_of_multiply_instanced_nugraf_objects;

	// ------------------------------------------------------------------------

    // OSG
    char osgStripTexturePaths;
    char osgNormalization;
    char osgBackfaceCulling;
    char osgDebugTextFile;

    char osgInstanceFile;
    char osgInstanceShared;
    char osgInstanceIgnore;

    char osgRunOptimizer;
    char osgMergeGeodes;
    char osgShareState;
    char osgMergeGeometry;
    char osgCreateTriangleStrips;
    char osgRemoveRedundantNodes;
    char osgStripImplicitNames;

    char* osgExporterVersion;
    // END OSG

	char
		// Mesh options
		ena_mesh_data,
		output_vertex_colors,
		ena_uv_tangents,
		ena_vertex_normals,
		ena_texture_coords,
		ena_mesh_skinning_weights,

		// Enables
		ena_nurbs_patches,
		nurbs_as_polygons,
		ena_trim_curves,
		ena_surftxtr_layers,
		ena_texture_definitions,
		ena_instance_attributes,
		ena_texture_proj_info,
		ena_env_info,
		ena_background_info,
		ena_hierarchy,
		ena_cameras,
		ena_lights,
		ena_materials,
		ena_bicubic_patches,
		ena_pointsets,
		ena_lines,
		ena_instancing,

		/* NURBS curves */
		ena_independent_3D_nurbs_curves,	// TRUE to enable NURBS curve output
		RenderableNURBSCurveOutputMode,		// If the NURBS curve primitive has the renderable flag enabled, then this option determines how it is to be output (ie: unchanged, as trimmed NURBS surface, as spline shape or as a mesh)
		NonRenderableNURBSCurveOutputMode,	// If the NURBS curve primitive has the renderable flag disabled, then this option determines how it is to be output (ie: unchanged, as a single NURBS curve or as a spline shape)

		/* Spline Shapes */
		ena_independent_3D_Spline_Shapes,	// TRUE to enable spline shape output
		RenderableSplineShapeOutputMode,	// If the spline shape primitive has the renderable flag enabled, then this option determines how it is to be output (ie: unchanged, as trimmed NURBS surface or as a mesh)
		NonRenderableSplineShapeOutputMode,	// If the spline shape primitive has the renderable flag disabled, then this option determines how it is to be output (ie: unchanged, or as a NURBS curve equivalent)

		// Bitmap conversion
		*autoconvert_bitmap_format,
		*convert_file_extension,
		txtr_bitmap_depth,
		ena_confirm_bitmap_overwrites,
		ena_convert_txtr_bitmaps,
		ena_use_specific_bitmap_extension,
		bitmap_saveto_directory_type,

		// File path conversion
		convert_to_unix_paths,
		convert_paths_to_rfc1738_standard,
		ena_strip_abs_filepaths,
		*specific_filepath_to_prepend,
		*specific_relative_filepath_to_prepend,
		*save_bitmaps_to_specific_directory,
		*url_string,
		*export_directory,

		// Mesh processing
		mesh_processing_always_add_vertex_normals,
		mesh_processing_optimize_texture_coordinate_list,
		mesh_processing_duplicate_vertices_for_uv_texture_mapping,
		mesh_processing_explode_meshes_by_material_name,
		mesh_processing_transform_to_worldspace,
        mesh_processing_sort_polygons_by_material_assignments,
		mesh_processing_want_planar_polygons_only,
		use_absolute_file_paths,
		polygon_reduction_ena,

		// Animation
		ena_object_animation,
		ena_camera_animation,
		ena_light_animation,
		ena_material_animation,

		// If export XYZ (3 channel) animation, such as XYZ translation, setting this to TRUE will cause
		// the 3 channels to be exported using 3 independent controllers animation channels. 
		// If set to FALSE then the internal toolkit will be asked to make sure that the 3
		// keyframe lists have the same key times and the same number of keys (using temp. key lists) so
		// that we can properly export the animation using a single keyframe controller.
		use_independent_controllers_for_tripled_anim_channels,

		// If set to TRUE then the raw toolkit keyframe data will be attempted to be exported
		// otherwise the resampled versions will be used.
		export_raw_unsampled_anim_data_if_possible,

		// This is the interpolation controller to create when exporting resampled animation 
		// (Nc_NRS_ANIM_INTERPOLATION_TYPE_LINEAR, Nc_NRS_ANIM_INTERPOLATION_TYPE_TCB, or Nc_NRS_ANIM_INTERPOLATION_TYPE_BEZIER)
		interpolation_type_to_use_during_export,

		// This is the rotation controller type to create when exporting resampled animation data 
		// (Nc_NRS_ROTATION_CONTROLLER_QUATERNION or Nc_NRS_ROTATION_CONTROLLER_EULER)
		rotation_controller_type_to_use_during_export,

		// This will try to "fix" Euler rotation curves upon export (when
		// resampling is used) where the angles flip back and forth on either
		// side of -180/180 degrees. 
		flip_Euler_180flips_via_resampler;

	float
	 	mesh_processing_want_planar_polygons_only_tolerance;

	Nd_Token
		// When outputting a spline shape, this determine if we should resample the spline to another spline type
		spline_to_spline_conversion_type;

	Nd_Int
		// Euler rotation order for animation export. Normally an exporter
		// would hard code this and not provide a user interface option.
		ena_object_anim_resampled_rot_order;

	short
		// Automatic bitmap conversion X & Y output resolutions
		txtr_x_res,
		txtr_y_res;
} Exporter_Options;
extern Exporter_Options *export_options;

// Full path to the plug-in directory (with a \ at the end). Passed to us by the host program
extern char	*plugin_directory;

/* ------------------>>>>  Global Function Prototypes  <<<<--------------------- */

extern "C" {
	// Main entry point into the exporter from the host
	Nd_Void	ExportConverter_Main(char *tokens[], char *data[], long *return_result, void *aux_ptr);
	// Routine to save the current dialog box options to an .ini file
	short	NI_Exporter_SaveExportConvertOptions(char *ini_filepath);
	// Routine to load the current dialog box options from an .ini file
	short	NI_Exporter_RestoreExportConvertOptions(char *ini_filepath);
	// Routine to initialize the current dialog box options to start-up values
	short	NI_Exporter_InitExportOptionVariables();
	// Routine to de-initialize the current dialog box options to start-up values
	Nd_Void	NI_Exporter_DeInitExportOptionVariables();
};

extern Nd_Void	NI_Exporter_List_Atmospheric_Values(FILE *ofp);
extern Nd_Void	NI_Exporter_List_Background_Values(FILE *ofp);
extern Nd_Void	NI_Exporter_List_Spline_Shape_Primitive(FILE *ofp, Nd_Walk_Tree_Info *Nv_Walk_Tree_Info_Ptr, char *Nv_Master_Object, Nd_Int Nv_Primitive_Number, char *Nv_Inherited_Surface_Name, char *handle_name);
extern Nd_Void	NI_Exporter_List_NURBS_Curve_Primitives(FILE *ofp, Nd_Walk_Tree_Info *Nv_Walk_Tree_Info_Ptr, char *Nv_Master_Object, Nd_Int Nv_Primitive_Number, char *Nv_Inherited_Surface_Name, char *handle_name);
extern short	NI_Exporter_List_NURBS_Surface_Primitive(FILE *ofp, Nd_Walk_Tree_Info *Nv_Walk_Tree_Info_Ptr, char *Nv_Master_Object, Nd_Int Nv_Primitive_Number, char *Nv_Inherited_Surface_Name, char *handle_name);
extern Nd_Void	NI_Exporter_List_Bicubic_Patch_Primitive(FILE *ofp, Nd_Walk_Tree_Info *Nv_Walk_Tree_Info_Ptr, char *Nv_Master_Object, Nd_Int Nv_Primitive_Number, char *Nv_Inherited_Surface_Name, char *handle_name);
extern Nd_Void	NI_Exporter_List_Texture_Projection_Values(FILE *ofp, Nd_Token type, char *handle_name);
extern Nd_Void	NI_Exporter_Display_Matrix(FILE *ofp, Nd_Matrix m_ptr, Nd_Short num_tabs);
extern Nd_Void	NI_Exporter_Cleanup_TxtrFileName_List();
extern Nd_Void	NI_Exporter_Convert_Bitmap_Files(char *);
extern void	NI_Exporter_QueryAndOutputGeneralAnimationInformation(FILE *ofp);
extern short	NI_Exporter_QueryAndSetup_Object_Animation_Keyframe_Data( FILE *ofp, char *instance_name);
extern short	NI_Exporter_Output_Object_Animation_Data(FILE *ofp, char *instance_name);
extern Nd_Void	NI_Exporter_Output_Camera_Animation_Data( FILE *ofp, char *camera_name);
extern Nd_Void	NI_Exporter_Output_Light_Animation_Data( FILE *ofp, char *light_name);
extern Nd_Void	NI_Exporter_Output_Material_Animation_Data( FILE *ofp, char *surface_name);
extern 	short	NI_Exporter_GetValidPivotPointMatricesForAnimationExport(char *instance_name, Nd_Matrix pivot_inverse_pivot_matrix, Nd_Matrix normal_transform_matrix);
extern 	Nd_Void	NI_Exporter_List_Mesh_Deformation_Skin_Weights_And_BoneBinding(FILE *fp, char *Nv_Master_Object, Nd_Int Nv_Primitive_Number, char *instance_name);
extern 	Nd_Void	NI_Exporter_List_IndexedPolyline_Primitive(FILE *ofp, Nd_Walk_Tree_Info *Nv_Walk_Tree_Info_Ptr, char *Nv_Master_Object, Nd_Int Nv_Primitive_Number, char *Nv_Inherited_Surface_Name, char *instance_name);
extern 	Nd_Void	NI_Exporter_List_PointSet_Primitive(FILE *ofp, Nd_Walk_Tree_Info *Nv_Walk_Tree_Info_Ptr, char *Nv_Master_Object, Nd_Int Nv_Primitive_Number, char *Nv_Inherited_Surface_Name, char *instance_name);
extern 	Nd_Void	NI_Exporter_List_Indice_Info__Original_API(FILE *ofp, Export_IO_IP_Info_Type *ip, short print_indice_count, short print_surface_names, char *name, long num_polygons, unsigned short *verticesperpoly_ptr, short animation_data_is_available, char *instance_name, long num_indices);
extern 	Nd_Void	NI_Exporter_Display_Matrix(FILE *ofp, Nd_Matrix m_ptr, Nd_Short num_tabs);
extern 	Nd_Int	NI_Exporter_Output_Meta_Data(FILE *ofp, Nd_Token Nv_Handle_Type, char *Nv_Handle_Name);
extern 	Nd_Bool	NI_OutputMesh(FILE *ofp, Nd_Walk_Tree_Info *Nv_Info, char *master_object);
extern	Nd_Bool	NI_Handle_Okino_Object_Instancing(Nd_Walk_Tree_Info *Nv_Walk_Tree_Info_Ptr);

extern	Nd_Void	NI_Exporter_List_Indice_Info__New_Extended_API(FILE *ofp, Nd_Int num_polygons, Nd_UShort *verticesperpoly_ptr, Nd_Int num_coordinates, void *coords, Nd_Int *indices, Nd_Int indices_shared_with_vertices, short print_indice_count, char *name, short animation_data_is_available, char *instance_name, char *user_defined_arrays__handle_name, char *user_defined_arrays__guid, unsigned long user_defined_arrays__flags, Nd_Int user_defined_arrays__datatype, Nd_Int *user_defined_arrays__size_of_each_element_in_coord_array, long num_indices);

// Callback functions
extern Nd_Int	NI_Exporter_List_Camera_Callback(Nd_Enumerate_Callback_Info *cbi_ptr);
extern Nd_Int	NI_Exporter_List_Light_Callback(Nd_Enumerate_Callback_Info *cbi_ptr);
extern Nd_Int	NI_Exporter_List_Texture_Callback(Nd_Enumerate_Callback_Info *cbi_ptr);
extern Nd_Void	NI_Exporter_ConvertandProcessRawMeshPrimitive_Callback(Nd_ConvertandProcessRawPrimitive_Info *Nv_Info, Nd_Int *Nv_Return_Status);
extern Nd_Int	NI_Exporter_List_Surface_Callback(Nd_Enumerate_Callback_Info *cbi_ptr);


#endif
