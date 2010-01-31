/*****************************************************************************

	----------------------------------------------------------------
       Indexed Polygon Mesh Geometric Entity Query Functions for PolyTrans
	----------------------------------------------------------------
	  This module receives all the data of an Indexed Polygon Mesh.
	   This is the 'work horse' primitive of an exporter. If your 
 	exporter can only understand polygons then this is the geometric
	  primitive whereby the data can be queried from the toolkit.
	----------------------------------------------------------------


  Copyright (c) 1988, 2009 Okino Computer Graphics, Inc. All Rights Reserved.

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

#include "main.h"	// Main include file for a PolyTrans exporter

/* ----------------------->>>>  Definitions  <<<<--------------------------- */

// v4.1.8 of the toolkit (September 2004) provides an entirely new and revamped
// API dealing with vertex coordinate arrays associated with an indexed polygon
// mesh primitive (Nt_INDEXEDPOLYGON). The standard arrays are the vertex normals,
// vertex uv texture coordinates, vertex colors and vertex tangents. With this
// new API (as fully documented in the 'NuGraf Developer's 3D Toolkit'), any
// number of vertex coordinate arrays can be associated with a mesh primitive,
// and each array can use any number of different data types (such as an array
// of floats, vectors, matrices, strings-per-coordinate, raw binary data per
// coordinate, etc). Most commonly you would only want/need to support this API
// for export if your file format supports multi-texturing, where a mesh is
// allowed to have multiple uv texture coordinates per vertex.
//
// Note that the vertex normals, vertex uv texture coordinates, vertex colors and
// vertex tangent arrays accessible via the old API are now accessed (ghosted) in
// the new API via the array handle names of "norm0", "uv0", "col0", "utang0" and
// "vtang0".
//
// If you wish to test the new vertex coordinate API then execute the "Okino
// Example Importer" and choose the "Mesh with multi-vertex coordinate Arrays"
// example. This will define a scene with 2 polygons and many different types
// of user defined vertex coordinate arrays.
//
// If your file format can handle multiple uv texture coordinates per vertex
// ('multi-texturing') then set this to'1'. If you also want to extract any
// other vertex coordinate arrays associated with the mesh then set it to '1'
// as well. Otherwise, if you just want to use the original, simpler API to
// query the normals/uv's/colors/tangents then set this to '0'. If you set this
// to '1' then you need to ensure that the end-user has PolyTrans v4.1.8 or
// newer on their machine, whereas setting this to '0' will work for any v4.1.x
// version of Okino software.
//
#define	USE_NEW_USER_DEFINED_COORDINATES_API	1

// Enable this code if you need the ability to map a (v1,v2) vertex pair from a
// mesh into its corresponding edge from the mesh's internal edge list. This is
// basically never needed by an exporter (it is used by the Okino SketchUp
// exporter to reconstruct edge lists for SketchUp) but provided here for example.
#define	INCLUDE_EDGE_LOOKUP_CODE	0

/* ------------------>>>>  Function Prototypes  <<<<------------------------ */

/* ------------------------>>>>  Mesh Output  <<<<-------------------------- */

// Main entry point to output a mesh. Returns Nc_TRUE if an error

	Nd_Bool
NI_OutputMesh(FILE *ofp, Nd_Walk_Tree_Info *Nv_Info, char *master_object)
{
	Nd_Token	mesh_processing_use_shared_index_array;
	Nd_Token	mesh_processing_negative_indices_allowed;
	Nd_Token	mesh_processing_holes_allowed;
	Nd_Token	mesh_processing_always_add_vertex_normals;
	Nd_Token	mesh_processing_optimize_texture_coordinate_list;
	Nd_Token	mesh_processing_duplicate_vertices_for_uv_texture_mapping;
	Nd_Token	mesh_processing_explode_meshes_by_material_name;
	Nd_Token	mesh_processing_want_convex_only;
	Nd_Token	mesh_processing_want_quads_only;
	Nd_Token	mesh_processing_want_triangles_only;
	Nd_Token	mesh_processing_want_undeformed_mesh;
	Nd_Token      	mesh_processing_want_planar_polygons_only;
	Nd_Int		mesh_processing_status;
	Nd_Token	mesh_processing_sort_polygons_by_material_assignments;
	Nd_Token	Nv_TransformByDefaultCTM;
	Nd_Token	Nv_EnablePolygonReduction;
	short		animation_data_is_available = Nc_FALSE;

	try {
        	// Let's see if we've output this object definition before. If so, create a new instance of the existing JT part
        	if (NI_Handle_Okino_Object_Instancing(Nv_Info))
        		return Nc_FALSE;	// Instance was created. Return no error

		// Go and see if there is any animation channel data associated
		// with this geometry instance that we are interested about. If resampled
		// keyframe mode is in effect then temporary resampled keyframe lists
		// will have been created and stored in the core toolkit after this
		// function finishes and returns here.
		if (export_options->ena_object_animation && export_options->ena_hierarchy) {
			Nd_Int Nv_Active_Channels_Found = Nc_FALSE;
			Ni_NodeHasActiveChannels( Nt_INSTANCE, Nv_Info->Nv_Handle_Name, Nc_TRUE /* keyframe controllers only */, &Nv_Active_Channels_Found, Nt_CMDEND );
			if ( Nv_Active_Channels_Found )
				animation_data_is_available = NI_Exporter_QueryAndSetup_Object_Animation_Keyframe_Data(ofp, Nv_Info->Nv_Handle_Name);
		}

        	// Developer Notice:: Normally the options to the Ni_ConvertandProcessRawPrimitive()
        	// function (see below) are hand set to Nt_ON or Nt_OFF when you write your custom
        	// exporter. This is because most of the arguments can be set statically for a
        	// specific exporter file format. However, for the sake of generality in this
        	// demonstration exporter we are making it possible to set all of these options
        	// via the Windows options dialog box. Thus, you have the option to configure the
        	// Ni_ConvertandProcessRawPrimitive() function call arguments entirely via the
        	// dialog box or else you can delete those checkboxes from the dialog box and
        	// statically set the parameters to Nt_ON or Nt_OFF in the function below.
        	mesh_processing_use_shared_index_array = export_options->mesh_processing_use_shared_index_array ? Nt_ON : Nt_OFF;
        	mesh_processing_negative_indices_allowed = export_options->mesh_processing_negative_indices_allowed ? Nt_ON : Nt_OFF;
        	mesh_processing_holes_allowed = export_options->mesh_processing_holes_allowed ? Nt_ON : Nt_OFF;
        	mesh_processing_always_add_vertex_normals = export_options->mesh_processing_always_add_vertex_normals ? Nt_ON : Nt_OFF;
        	mesh_processing_optimize_texture_coordinate_list = export_options->mesh_processing_optimize_texture_coordinate_list ? Nt_ON : Nt_OFF;
        	mesh_processing_duplicate_vertices_for_uv_texture_mapping = export_options->mesh_processing_duplicate_vertices_for_uv_texture_mapping ? Nt_ON : Nt_OFF;
        	mesh_processing_explode_meshes_by_material_name = export_options->mesh_processing_explode_meshes_by_material_name ? Nt_ON : Nt_OFF;
        	mesh_processing_want_convex_only = export_options->mesh_processing_want_convex_only ? Nt_ON : Nt_OFF;
        	mesh_processing_want_quads_only = export_options->mesh_processing_want_quads_only ? Nt_ON : Nt_OFF;
        	mesh_processing_want_triangles_only = export_options->mesh_processing_want_triangles_only ? Nt_ON : Nt_OFF;
		mesh_processing_sort_polygons_by_material_assignments = export_options->mesh_processing_sort_polygons_by_material_assignments ? Nt_ON : Nt_OFF;
		mesh_processing_want_planar_polygons_only = export_options->mesh_processing_want_planar_polygons_only ? Nt_ON : Nt_OFF;
		// If we are outputting skinning weights (ie: deformation via bones and skeletons)
		// then make sure we ask for the undeformed mesh, else ask for the final deformed mesh.
		mesh_processing_want_undeformed_mesh = export_options->ena_mesh_skinning_weights ? Nt_ON : Nt_OFF;
		// Optionally enable polygon reduction
		Nv_EnablePolygonReduction = export_options->polygon_reduction_ena ? Nt_ON : Nt_OFF;

		if (export_options->ena_object_animation)
			Nv_TransformByDefaultCTM = Nt_OFF;
		else
			// Else, transform to world-space based on the option set on the options dialog box
			Nv_TransformByDefaultCTM = (export_options->mesh_processing_transform_to_worldspace ? Nt_ON : Nt_OFF);

        	// This is the monster of all cover functions. It processes the geometric
        	// primitives associated with the current instance (usually only one).
        	// It turns each internal geometric primitive into an optimized polygon mesh
        	// and passes it back to this exporter via the NI_Exporter_ConvertandProcessRawMeshPrimitive_Callback()
        	// callback function defined below. A temporary object and instance definition
        	// is used to encapsulate the optimized (and temporary) mesh primitive.
        	Ni_ConvertandProcessRawPrimitive(
        		/* This is the Walk_Tree_Info pointer passed in to the currnet function. */
        		/* It defines which instance in the tree is to be processed. */
        		Nv_Info,

        		/* Two user-specified 32-bit parameters that are passed to the callback routine */
        		(void *) ofp, (void *) animation_data_is_available,

        		/* Pointer to the callback function in the host program to receive the temporary processed mesh data (in an instance wrapper) */
        		NI_Exporter_ConvertandProcessRawMeshPrimitive_Callback,

        		/* The status of this function is returned here */
        		&mesh_processing_status,

        		// If set to Nt_ON then the normals, uv texture coordinates, vertex
        		// colors and UV tangent vectors will be (optionally) duplicated
        		// in place so that all share the same index array as the vertices.
        		// This is required, for example, by the 3D Studio (.3ds) and RIB exporters.
        		Nt_USESHAREDINDICEARRAY, Nt_ENABLED, mesh_processing_use_shared_index_array, Nt_CMDSEP,

        		// Set this option to Nt_ON if the exporter module can handle indices
        		// which are negative (for the normals, uv texture coordinates, vertex
        		// colors and UV tangent index arrays). A negative value in any of these
        		// arrays signifies that no corresponding coordinate exists. For example,
        		// the vertex normal indices for a 4-sided polygon might be (1, 2, 3, -1)
        		// for which the fourth vertex has no normal. If this option is set to
        		// Nt_OFF then the negative indices will be removed by having new
        		// geoemetric normals to be computed or dummy color/texture/tangent
        		// coords to be inserted into their respective lists.
        		Nt_NEGATIVEINDICESALLOWED, Nt_ENABLED, mesh_processing_negative_indices_allowed, Nt_CMDSEP,

        		// Set this option to Nt_ON to have the geometric normal of a polygon
        		// assigned to a vertex if it does not have any vertex normals.
        		Nt_ALWAYSADDVERTEXNORMALS, Nt_ENABLED, mesh_processing_always_add_vertex_normals, Nt_CMDSEP,

        		// Set this option to Nt_ON to have the any duplicated uv texture
        		// coordinates removed.
        		Nt_OPTIMIZETEXTURECOORDLIST, Nt_ENABLED, mesh_processing_optimize_texture_coordinate_list, Nt_CMDSEP,

        		// Set this option to Nt_ON if holes are allowed in the exported mesh
        		// data. If set to Nt_OFF then the Nt_WANTTRIANGLESONLY option will
        		// automatically be overriden and set to Nt_ON so that all holes
        		// get triangulated.
        		Nt_HOLESALLOWED, Nt_ENABLED, mesh_processing_holes_allowed, Nt_CMDSEP,

        		// This routine inserts new vertices into a mesh if a shared vertex has
        		// (u,v) texture coordinates BUT each polygon has a different material
        		// which uses different texture scale/offset values. For example, this
        		// routine is used by the DirectX exporter since it does not allow scale/offset
        		// values to be associated with each material. Note: the index arrays
        		// will become 'unshared' after this routine is executed.
        		Nt_DUPLICATEVERTICESFORUVMAPPING, Nt_ENABLED, mesh_processing_duplicate_vertices_for_uv_texture_mapping, Nt_CMDSEP,

        		// Set to Nt_ON to cause the mesh to be exploded by the assigned
        		// surface definitions. One or more meshes will be output to the callback
        		// routine in succession, each using a different surface assignment.
        		// The name of the surface for each mesh will be passed to the callback
        		// via the 'Nv_Surface_Name' of the 'Nd_ConvertandProcessRawPrimitive_Info'.
        		Nt_EXPLODEBYSURFACENAME, Nt_ENABLED, mesh_processing_explode_meshes_by_material_name, Nt_CMDSEP,

        		// Set to Nt_ON to cause uv texture coordinates to be included in the processed mesh data
        		Nt_SAVETEXTUREDATA, Nt_ENABLED, Nt_ON, Nt_CMDSEP,

        		// Set to Nt_ON to cause vertex colors (if any) to be included in the processed mesh data
        		Nt_SAVECOLORDATA, Nt_ENABLED, Nt_ON, Nt_CMDSEP,

        		// Set to Nt_ON to cause vertex normal data to be included in the processed mesh data
        		Nt_SAVENORMALDATA, Nt_ENABLED, Nt_ON, Nt_CMDSEP,

        		// Set to Nt_ON to cause UV tangent vectors to be included in the processed mesh data
        		Nt_SAVETANGENTDATA, Nt_ENABLED, Nt_ON, Nt_CMDSEP,

        		// Set to Nt_ON to cause "custom" vertex data of the "user defined coordinate arrays" 
			// to be included in the processed mesh data. Normally you would want to 
			// keep this to Nt_OFF since custom vertex data (like binary data associated with each
			// vertex) is very rarely used.
        		Nt_SAVECUSTOMDATA, Nt_ENABLED, Nt_OFF, Nt_CMDSEP,

			// Set to Nt_ON to cause the order of the polygons in the processed mesh data to be
			// sorted according to the names of the material (surface definitions) assigned to them.
			// Normally the polygons are output in the same order in which they exist internally
			// in the database. However, some exporters (such as Renderware, OpenGL C Code) would
			// benefit from fewer material state changes from one polygon to the next, so enabling
			// this option will cause the polygon order to be changed so that each run of polygons
			// will only use the same material assignment.
			Nt_SORTPOLYGONSBYMATERIALASSIGNMENTS, Nt_ENABLED, mesh_processing_sort_polygons_by_material_assignments, Nt_CMDSEP,

        		// Set to Nt_ON to cause non-convex polygons to become triangulated
        		Nt_WANTCONVEXONLY, Nt_ENABLED, mesh_processing_want_convex_only, Nt_CMDSEP,

        		// Set to Nt_ON to cause 5 or more sided polygons to become triangulated
        		Nt_WANTQUADSONLY, Nt_ENABLED, mesh_processing_want_quads_only, Nt_CMDSEP,

        		// Set to Nt_ON to cause 4 or more sided polygons to become triangulated
        		Nt_WANTTRIANGLESONLY, Nt_ENABLED, mesh_processing_want_triangles_only, Nt_CMDSEP,

			// Set to Nt_ON to cause polygons which are not entirely planar to be triangulated.
			// Polygons are deemed non-planar if any vertex (in object-space coordinates) is
			// further from its least-square-fitted plane by the 'mesh_processing_want_planar_polygons_only_tolerance'
			// value (which defaults to the arbitrary 1e-4 value). Normally this should be set to Nt_OFF.
			Nt_WANTPLANARPOLYGONSONLY, Nt_ENABLED, mesh_processing_want_planar_polygons_only, (Nd_Float *) &export_options->mesh_processing_want_planar_polygons_only_tolerance, Nt_CMDSEP,

			// Set to Nt_ON to query the mesh before any deformations have been
			// applied to it (in particular mesh skinning == deformations via bones).
			// Set to Nt_OFF to query the mesh after it has been deformed. If
			// you are outputting a mesh for skinning (with associate vertex weights)
			// then you should set this to Nt_ON so that you get the undeformed 
			// base pose mesh.
			Nt_WANTUNDEFORMEDMESH, Nt_ENABLED, mesh_processing_want_undeformed_mesh, Nt_CMDSEP, 

        		// Set to Nt_ON to cause the entire mesh to be transformed to the
        		// default current transformation matrix passed in via the
        		// 'Nd_Walk_Tree_Info' data structure. Doing this will essentially
        		// convert the mesh to world space coordinates.
        		Nt_TRANSFORMBYDEFAULTCTM, Nt_ENABLED, Nv_TransformByDefaultCTM, Nt_CMDSEP,

			// Set to Nt_ON to enable polygon reduction on the the temporary intermediate
			// mesh sent to ip_mesh.cpp (but not the original mesh inside the toolkit). 
			// NOTE: this requires that you call "Import_IO_Send_Polygon_Reduction_UI_Options_To_Toolkit()"
			// so that the user's options (set by dialog box and stored in the parent
			// application's .ini file gets sent to the toolkit ahead of time).
			Nt_ENABLEPOLYGONREDUCTION, Nt_ENABLED, Nv_EnablePolygonReduction, Nt_CMDSEP,

			// Okino's mesh primitive allows each polygon to be assigned its own
			// material (surface definition). If there are no polygon-level materials
			// then the entire mesh uses the material assigned to the instance. If
			// there are polygon level material assignments, and a polygon is assigned
			// the special 'Nc_NUGRAF_INHERITED_SURFACE_MARKER' material name, then
			// that polygon (and all subsequent polygons until a new material is
			// assigned) will again inherit the current instance-level material in
			// effect during the scene graph tree walk.
			//
			// If this option is set to "Nt_OFF" then the raw material list for the
			// polygons will be passed through to the exporter without modifying
			// the 'Nc_NUGRAF_INHERITED_SURFACE_MARKER' markers, if they exist in the
			// material list. This is needed when you are using geometry instancing
			// and need to determine the case where the use of instancing in an exporter
			//  will cause materials to be inherited at the raw mesh level (such as used
			// by the U3D exporter where it can assign a completely new set of polygon-level
			// materials on an instance-by-instancd basis -- unique to U3D and few others).
			//
			// If this option is set to "Nt_ON" then each 'Nc_NUGRAF_INHERITED_SURFACE_MARKER' marker
			// in the polygon-level material list of the mesh will be temporarily replaced
			// with the material assigned to the instance in effect while the scene graph
			// hierarchy is being traversed. In other words, the mesh's material list will be
			// modified to reflect the current instance-level materials which it can inherit
			// via its 'Nc_NUGRAF_INHERITED_SURFACE_MARKER' markers assigned to 1 or more of
			// its polygons. This is the default option since it makes generic exporter
			// processing of materials easier. In the case of the U3D exporter we do not want
			// to enable this option since it masks which polygons use inherited materials and
			// hence the mesh can't be instanced N-times and properly have the mesh inherit
			// the proper instance-level materials on an instance-by-instance basis (U3D is unique
			// in that it allows a new mesh material list to be assigned on an instance-by-instance basis).
			Nt_REPLACEPERPOLYGONINHERITEDSURFACEMARKERS, Nt_ENABLED, Nt_ON, Nt_CMDSEP,

        	Nt_CMDEND);

        	// Check to see if the user aborted during the processing stage, or a fatal error was encountered
        	switch (mesh_processing_status) {
        		case Nc_CONVERTANDPROCESS_OK:
        			/* Valid data was processed */
        			break;
        		case Nc_CONVERTANDPROCESS_ABORT:
        			/* User has aborted */
        			goto error;
        		case Nc_CONVERTANDPROCESS_FATAL_ERROR:
        			/* Error encountered during export (out of memory or out of disk space) */
        			goto error;
        		case Nc_CONVERTANDPROCESS_NODATA:
        			/* No valid polygon data to output */
        			break;
        	}
	}
	catch( ... )
	{
		Ni_Report_Error_printf(Nc_ERR_ERROR, "Exporter is raising an unknown exception while creating a mesh object for instance '%s'.", Nv_Info->Nv_Handle_Name);
		return Nc_TRUE;
	}

	// If animation data was previously found and set up, then go output
	// the animation data associated with this instance. 
	if (animation_data_is_available)
		NI_Exporter_Output_Object_Animation_Data(ofp, Nv_Info->Nv_Handle_Name);

	// --------------------------------------------

// Developer: this is reference code which shows you how our JT exporter creates the JT part
// definition, attaches it to the JT hierarchy, assigns the material, assigns the transformation,
// assigns the meta data, and adds our object definition to the instancing list for future lookup.
#if 00
	try {
		// Make sure the JT assembly name becomes unique within the JT namespace
		char part_name[1024];
		NI_FixUpJTHandleName(Nt_INSTANCE, Nv_Info->Nv_Handle_Name, part_name);

		// Create the JT part
		JtkPart* Part = NULL;
		Part = JtkEntityFactory::createPart(part_name);
		if (!Part ) {
			Ni_Report_Error_printf(Nc_ERR_ERROR, "JtkEntityFactory::createPart() failed. Out of memory?\n");
			goto exit;
		}

		// Add the part to the current JT hierarchy (we pass 'pParentAssemblyNode' into the current routine)
		pParentAssemblyNode->addChild(Part);

		// Set the transform. Create & set the material, and find a good texture definition (optional) to assign to this part
		if (export_options->ena_hierarchy)
			NI_JT_Assign_Transform_Material_Texture_To_Geometry_Part(Part, Nv_Info->Nv_CTM, Nv_Info->Nv_Surface_Name);
		else
			NI_JT_Assign_Transform_Material_Texture_To_Geometry_Part(Part, Nv_Info->Nv_Hierarchical_CTM, Nv_Info->Nv_Surface_Name);

		// Assign the instance meta data
		NI_JT_Output_Meta_Data(Part, Nt_INSTANCE, Nv_Info->Nv_Handle_Name);

		// Add the part to our object look-up map for future instancing reference by NI_Handle_Okino_Object_Instancing()
		char 	dummy_string[300];
		Ni_StringDictionary_UniqueInsert(export_options->list_of_multiply_instanced_nugraf_objects,
			master_object,	// The Okino object definition name
			dummy_string,		// A dummy return string
			(void *) Part,		// The JT part pointer to be associated with the Okino object definition
			Nc_TRUE, Nc_FALSE);
	}
	catch (...)
	{
		Ni_Report_Error_printf(Nc_ERR_ERROR, "Error while creating JT object.\n");
	}
#endif

	// --------------------------------------------

	return Nc_FALSE;	// No error

error:	return Nc_TRUE;		// Error
}

// This callback receives the optimized mesh from the Ni_ConvertandProcessRawMeshPrimitive() function called from the routine above.

// This callback is the back-end of a very powerful geometry processing 
// system. If your exporter only understands polygon mesh data then no matter
// what data exists inside the PolyTrans internal database (such as trimmed
// NURBS, bicubic patches, quadrics, superquadrics, etc.) this function can
// have those non-mesh primitives turned into optimized indexed polygons
// meshes for simple and efficient export.

// NOTE: If you are outputting 'skinned meshes' (meshes whose vertives are to be
// deformed via associated bones and skeletons) then make sure you set the
// 'Nt_WANTUNDEFORMEDMESH' in the main.cpp file to Nt_ON when skinned output
// mode is wanted. This is to ensure that the routine below is sent the original
// bind pose mesh (undeformed mesh) and not the mesh after the deformation has
// been applied.

// !! NOTE !! This routine can be called multiple times for the same Okino object definition if you
// have the 'Nt_EXPLODEBYSURFACENAME' mesh procesing option enabled (see above routine) in which
// case the source mesh of the object definition will be exploded into multiple meshes, each being
// output with one call to the following routine.

	Nd_Void	
NI_Exporter_ConvertandProcessRawMeshPrimitive_Callback(Nd_ConvertandProcessRawPrimitive_Info *Nv_Prp_Ptr, Nd_Int *Nv_Return_Status) 
{
	FILE			*ofp;
	Export_IO_IP_Info_Type	ip_vertex_info;
	Nd_Int			child_num, num_polygons;
	Nd_Token		primitive_type;
	Nd_UShort		*vertices_per_polygon;
	Nd_UInt3264             Nv_NumEdgesInList = 0;
	Nd_IndexedPolygons_Edge_Info *Nv_EdgeList = NULL;
	char			*object_name, *instance_name;
	char			**surface_names;
	short			animation_data_is_available;
	Nd_HashItem		**edge_hash_table = NULL;
	Nd_UInt3264 		i, j;

#if USE_NEW_USER_DEFINED_COORDINATES_API
	// User defined coordinate arrays (mult-uv's, etc)
	Nd_Int	Nv_Num_User_Coords_Arrays = 0L;
	char	**Nv_Coord_Array_Names = NULL;

	char 	*user_defined_arrays__handle_name = NULL;
	char 	*user_defined_arrays__guid = NULL;
	Nd_Token user_defined_arrays__type = NULL;
	Nd_Int 	user_defined_arrays__num_coords;
	Nd_Int  user_defined_arrays__flags = 0L;
	Nd_Int	user_defined_arrays__datatype = 0L;
	void	*user_defined_arrays__element_list = NULL;
	Nd_Int	*user_defined_arrays__size_of_each_element_in_coord_array = NULL;
	Nd_Int	*user_defined_arrays__indices = NULL;
	Nd_Token user_defined_arrays__shared_with_vertices_enable = Nt_OFF;

	Nd_Int	num_normal_coords;
	Nd_Int 	num_texture_coords, num_color_coords;
	Nd_Int 	num_utangent_coords, num_vtangent_coords;
#else
	Export_IO_IP_Info_Type	ip_normal_info, ip_texture_info;
	Export_IO_IP_Info_Type	ip_color_info, ip_utangent_info, ip_vtangent_info;
#endif

	/* Pick up the arguments sent to this callback */
	ofp = (FILE *) Nv_Prp_Ptr->Nv_User_Data_Ptr1;
	animation_data_is_available = (short) Nv_Prp_Ptr->Nv_User_Data_Ptr2;

	// Pick up the temporary instance/object encapsulting the mesh data.
	// These names are always static and never change inside the toolkit.
	instance_name = Nv_Prp_Ptr->Nv_InstanceContainingMeshData_Handle_Name;
	object_name = Nv_Prp_Ptr->Nv_ObjectContainingMeshData_Handle_Name;

	child_num = 0;
	Ni_Inquire_Primitive(Nt_OBJECT, object_name, child_num, &primitive_type,
		Nt_INDEXEDPOLYGONS,
			Nt_NUMPOLYGONS, (Nd_Int *) &num_polygons,
			Nt_VERTICESPERPOLY, (Nd_UShort *) &vertices_per_polygon, 
			Nt_VERTEX,
				(Nd_Int *) &ip_vertex_info.num_coords,
				(Nd_Struct_XYZ **) &ip_vertex_info.coords,
				(Nd_Int **) &ip_vertex_info.indices,
#if !USE_NEW_USER_DEFINED_COORDINATES_API
			// NOTE:: The following is the original but still valid method to
			// query for vertex normals, colors, uv texture coordinates and tangent
			// values. However, v4.1.8 of the toolkit and newer provides an extended
			// API (the 'user defined vertex coordinate arrays') which allows any
			// number of vertex coordinate arrays to be defined and associated with
			// the indexed polygon mesh, such as multiple uv texture coordinate
			// arrays per per ("Multi-uv" texturing). The new API uses the array
			// names "norm0", "col0", "uv0", "opac0", "utang0" and "vtang0" to
			// import/modify/query the vertex coordinate arrays correspond to those
			// listed below. If you want to use the new API then don't import the
			// standard vertex coordinate arrays as per the sub-API below this line:

			Nt_NORMAL,
				(Nd_Int *) &ip_normal_info.num_coords,
				(Nd_Struct_XYZ **) &ip_normal_info.coords,
				(Nd_Int **) &ip_normal_info.indices,
				(Nd_Int *) &ip_normal_info.indices_shared_with_vertex_indices,
			Nt_TEXTURE,
				(Nd_Int *) &ip_texture_info.num_coords,
				(Nd_Struct_XYZ **) &ip_texture_info.coords,
				(Nd_Int **) &ip_texture_info.indices,
				(Nd_Int *) &ip_texture_info.indices_shared_with_vertex_indices,
			Nt_COLOR,
				(Nd_Int *) &ip_color_info.num_coords,
				(Nd_Struct_XYZ **) &ip_color_info.coords,
				(Nd_Int **) &ip_color_info.indices,
				(Nd_Int *) &ip_color_info.indices_shared_with_vertex_indices,
			Nt_UTANGENT,
				(Nd_Int *) &ip_utangent_info.num_coords,
				(Nd_Struct_XYZ **) &ip_utangent_info.coords,
				(Nd_Int **) &ip_utangent_info.indices,
				(Nd_Int *) &ip_utangent_info.indices_shared_with_vertex_indices,
			Nt_VTANGENT,
				(Nd_Int *) &ip_vtangent_info.num_coords,
				(Nd_Struct_XYZ **) &ip_vtangent_info.coords,
				(Nd_Int **) &ip_vtangent_info.indices,
				(Nd_Int *) &ip_vtangent_info.indices_shared_with_vertex_indices,
#endif
// Developer: only include the 'Nt_EDGEINFO' query if you really need the edge information.
#if 11
			// The edge list is computed "on the fly" the first time the 'Nt_EDGEINFO'
			// query is made, so you should only provide this query token if you really
			// need the edge info (ie. it takes time to compute)
			Nt_EDGEINFO,
				(Nd_UInt3264 *) &Nv_NumEdgesInList,
				(Nd_IndexedPolygons_Edge_Info *) &Nv_EdgeList,
#endif
// Developer:Enable this code if you need the ability to map a (v1,v2) vertex pair from a
// mesh into its corresponding edge from the mesh's internal edge list. This is
// basically never needed by an exporter (it is used by the Okino SketchUp
// exporter to reconstruct edge lists for SketchUp).
#if INCLUDE_EDGE_LOOKUP_CODE
			// Compute the mesh's edge list then return a hash table for the internal edge list. This allows us to easily map (v1,v2) into an edge in this list
			Nt_EDGELISTHASHTABLE, (Nd_HashItem ***) &edge_hash_table,
#endif
			Nt_SURFACELINK, (char ***) &surface_names,
			Nt_CMDSEP,
		Nt_CMDEND);

	// Update the stats
	export_options->total_polygons += num_polygons;
	++export_options->total_meshes;

// See explanation at the top of this file.
#if USE_NEW_USER_DEFINED_COORDINATES_API
	// Determine how many user defined coordinate arrays are defined for this indexed polygon
	// primitive and query the handle names for all of these arrays so that we can query the
	// parameters of each array further below. Note that the standard, pre-defined vertex
	// coordinate arrays are returned as the array names "norm0", "uv0", "col0", "utang0" and
	// "vtang0" (corresponding to the Nt_NORMAL, Nt_TEXTURE, Nt_COLOR, Nt_UTANGENT and Nt_VTANGENT
	// commented out in the code above).
	Nv_Num_User_Coords_Arrays = 0;
	Nv_Coord_Array_Names = NULL;
	Ni_Inquire_Primitive(Nt_OBJECT, object_name, child_num, (Nd_Token *) &primitive_type,
		Nt_INDEXEDPOLYGONS,
			// Return the number of user defined coordinate arrays
			Nt_NUMUSERCOORDARRAYS, (Nd_Int *) &Nv_Num_User_Coords_Arrays,

			// Returns a (char **) array of all coordinate lists array names
			//  with the last entry NULL. These names are used to access each
			// list uniquely.
			Nt_GETUSERCOORDARRAYNAMES, (char ***) &Nv_Coord_Array_Names,
		Nt_CMDEND);

	// Count up the number of coordinates in the extended user defined vertex coordinate arrays.
	// This chunk of code can be ignored by most export converters. It is only used in this
	// example exporter for the sake of illustration.
	num_normal_coords = num_texture_coords = num_color_coords = num_utangent_coords = num_vtangent_coords = 0L;
	if (Nv_Num_User_Coords_Arrays && Nv_Coord_Array_Names) {
		for (i=0; i <Nv_Num_User_Coords_Arrays; ++i) {
			if (Nv_Coord_Array_Names[i] != NULL) {
				// Pick up the information about one user defined vertex coordinate array
				Ni_Inquire_Primitive(Nt_OBJECT, object_name, child_num, (Nd_Token *) &primitive_type,
					Nt_INDEXEDPOLYGONS,
						Nt_USERCOORDARRAYS, Nv_Coord_Array_Names[i],
							// The following are all output parameters
							Nt_TYPE, (Nd_Token *) &user_defined_arrays__type,	// One of: Nt_VERTICES, Nt_NORMAL, Nt_TEXTURE, Nt_COLOR, Nt_CUSTOM ?>,
							Nt_NUMCOORDS, (Nd_Int *) &user_defined_arrays__num_coords,
						Nt_CMDEND);

				if (user_defined_arrays__type == Nt_NORMAL)
					num_normal_coords += user_defined_arrays__num_coords;
				else if (user_defined_arrays__type == Nt_TEXTURE)
					num_texture_coords += user_defined_arrays__num_coords;
				else if (user_defined_arrays__type == Nt_COLOR)
					num_color_coords += user_defined_arrays__num_coords;
				else if (user_defined_arrays__type == Nt_UTANGENT)
					num_utangent_coords += user_defined_arrays__num_coords;
				else if (user_defined_arrays__type == Nt_VTANGENT)
					num_vtangent_coords += user_defined_arrays__num_coords;
			}
		}
	}

	OPTIONAL_FPRINTF(ofp, "\t\tNumber of polygons = %ld\n", num_polygons);
	OPTIONAL_FPRINTF(ofp, "\t\tNumber of vertices = %ld\n", ip_vertex_info.num_coords);
	if (export_options->ena_vertex_normals)
		OPTIONAL_FPRINTF(ofp, "\t\tNumber of normals = %ld\n", num_normal_coords);
	if (export_options->output_vertex_colors)
		OPTIONAL_FPRINTF(ofp, "\t\tNumber of colours = %ld\n", num_color_coords);
	if (export_options->ena_texture_coords)
		OPTIONAL_FPRINTF(ofp, "\t\tNumber of texture coordinates = %ld\n", num_texture_coords);
	if (export_options->ena_uv_tangents) {
		OPTIONAL_FPRINTF(ofp, "\t\tNumber of U tangents = %ld\n", num_utangent_coords);
		OPTIONAL_FPRINTF(ofp, "\t\tNumber of V tangents = %ld\n", num_vtangent_coords);
	}
	if (Nv_NumEdgesInList)
		OPTIONAL_FPRINTF(ofp, "\t\tNumber of edges = %ld\n", Nv_NumEdgesInList);

	// List the vertex XYZ location indices and coordinates
	NI_Exporter_List_Indice_Info__New_Extended_API(ofp, num_polygons, vertices_per_polygon, ip_vertex_info.num_coords,
		ip_vertex_info.coords, ip_vertex_info.indices, Nc_FALSE, Nc_TRUE, "Vertex", animation_data_is_available, instance_name,
		NULL, NULL, 0L, Nc_BDF_USERDATA_NdVector, NULL, 0L);

	// Now list the of coordinates in the extended user defined vertex coordinate arrays
	if (Nv_Num_User_Coords_Arrays && Nv_Coord_Array_Names) {
		for (i=0; i <Nv_Num_User_Coords_Arrays; ++i) {
			if (Nv_Coord_Array_Names[i] != NULL) {
				// Pick up the information about one user defined coordinate array
				Ni_Inquire_Primitive(Nt_OBJECT, object_name, child_num, (Nd_Token *) &primitive_type,
					Nt_INDEXEDPOLYGONS,
						Nt_USERCOORDARRAYS, Nv_Coord_Array_Names[i],
							Nt_HANDLENAME, (char **) &user_defined_arrays__handle_name,
							Nt_GUID, (char **) &user_defined_arrays__guid,
							Nt_TYPE, (Nd_Token *) &user_defined_arrays__type,	// One of: Nt_VERTICES, Nt_NORMAL, Nt_TEXTURE, Nt_COLOR, Nt_CUSTOM ?>,
							Nt_NUMCOORDS, (Nd_Int *) &user_defined_arrays__num_coords,
							Nt_FLAGS, (Nd_Int *) &user_defined_arrays__flags,
							Nt_DATATYPE, (Nd_Int *) &user_defined_arrays__datatype,
							Nt_COORDS, (void **) &user_defined_arrays__element_list,
							Nt_ELEMENTSIZEPERCOORD, (Nd_Int *) &user_defined_arrays__size_of_each_element_in_coord_array,
							Nt_INDICES, (Nd_Int *) &user_defined_arrays__indices,
							Nt_SHAREDWITHVERTICES, Nt_ENABLED, (Nd_Token *) &user_defined_arrays__shared_with_vertices_enable,	// Returned as Nt_ON or Nt_OFF
						Nt_CMDEND);

				// List the indice info about specific, known vertex coordinate arrays
				if (user_defined_arrays__type == Nt_NORMAL)
					NI_Exporter_List_Indice_Info__New_Extended_API(ofp, num_polygons, vertices_per_polygon, user_defined_arrays__num_coords, user_defined_arrays__element_list,
						user_defined_arrays__indices, user_defined_arrays__shared_with_vertices_enable == Nt_ON, Nc_TRUE, "Normal", animation_data_is_available, instance_name,
						user_defined_arrays__handle_name, user_defined_arrays__guid, user_defined_arrays__flags, user_defined_arrays__datatype, user_defined_arrays__size_of_each_element_in_coord_array, 0L);
				else if (user_defined_arrays__type == Nt_TEXTURE)
					NI_Exporter_List_Indice_Info__New_Extended_API(ofp, num_polygons, vertices_per_polygon, user_defined_arrays__num_coords, user_defined_arrays__element_list,
						user_defined_arrays__indices, user_defined_arrays__shared_with_vertices_enable == Nt_ON, Nc_TRUE, "Texture", animation_data_is_available, instance_name,
						user_defined_arrays__handle_name, user_defined_arrays__guid, user_defined_arrays__flags, user_defined_arrays__datatype, user_defined_arrays__size_of_each_element_in_coord_array, 0L);
				else if (user_defined_arrays__type == Nt_COLOR)
					NI_Exporter_List_Indice_Info__New_Extended_API(ofp, num_polygons, vertices_per_polygon, user_defined_arrays__num_coords, user_defined_arrays__element_list,
						user_defined_arrays__indices, user_defined_arrays__shared_with_vertices_enable == Nt_ON, Nc_TRUE, "Color", animation_data_is_available, instance_name,
						user_defined_arrays__handle_name, user_defined_arrays__guid, user_defined_arrays__flags, user_defined_arrays__datatype, user_defined_arrays__size_of_each_element_in_coord_array, 0L);
				else if (user_defined_arrays__type == Nt_UTANGENT)
					NI_Exporter_List_Indice_Info__New_Extended_API(ofp, num_polygons, vertices_per_polygon, user_defined_arrays__num_coords, user_defined_arrays__element_list,
						user_defined_arrays__indices, user_defined_arrays__shared_with_vertices_enable == Nt_ON, Nc_TRUE, "U-Tangent", animation_data_is_available, instance_name,
						user_defined_arrays__handle_name, user_defined_arrays__guid, user_defined_arrays__flags, user_defined_arrays__datatype, user_defined_arrays__size_of_each_element_in_coord_array, 0L);
				else if (user_defined_arrays__type == Nt_VTANGENT)
					NI_Exporter_List_Indice_Info__New_Extended_API(ofp, num_polygons, vertices_per_polygon, user_defined_arrays__num_coords, user_defined_arrays__element_list,
						user_defined_arrays__indices, user_defined_arrays__shared_with_vertices_enable == Nt_ON, Nc_TRUE, "V-Tangent", animation_data_is_available, instance_name,
						user_defined_arrays__handle_name, user_defined_arrays__guid, user_defined_arrays__flags, user_defined_arrays__datatype, user_defined_arrays__size_of_each_element_in_coord_array, 0L);
				else // Custom data
					NI_Exporter_List_Indice_Info__New_Extended_API(ofp, num_polygons, vertices_per_polygon, user_defined_arrays__num_coords, user_defined_arrays__element_list,
						user_defined_arrays__indices, user_defined_arrays__shared_with_vertices_enable == Nt_ON, Nc_TRUE, "Custom Data", animation_data_is_available, instance_name,
						user_defined_arrays__handle_name, user_defined_arrays__guid, user_defined_arrays__flags, user_defined_arrays__datatype, user_defined_arrays__size_of_each_element_in_coord_array, 0L);
			}
		}
	}

	// Free up the user defined coordinate array handle names which we had queried from the toolkit
	if (Nv_Num_User_Coords_Arrays && Nv_Coord_Array_Names) {
		for (i=0; i < Nv_Num_User_Coords_Arrays; ++i) {
			if (Nv_Coord_Array_Names[i] != NULL)
				Ni_Free_Memory(Nv_Coord_Array_Names[i]);
		}
		Ni_Free_Memory(Nv_Coord_Array_Names);
	}
#else
	OPTIONAL_FPRINTF(ofp, "\t\tNumber of polygons = %ld\n", num_polygons);
	OPTIONAL_FPRINTF(ofp, "\t\tNumber of vertices = %ld\n", ip_vertex_info.num_coords);
	if (export_options->ena_vertex_normals)
		OPTIONAL_FPRINTF(ofp, "\t\tNumber of normals = %ld\n", ip_normal_info.num_coords);
	if (export_options->output_vertex_colors)
		OPTIONAL_FPRINTF(ofp, "\t\tNumber of colours = %ld\n", ip_color_info.num_coords);
	if (export_options->ena_texture_coords)
		OPTIONAL_FPRINTF(ofp, "\t\tNumber of texture coordinates = %ld\n", ip_texture_info.num_coords);
	if (export_options->ena_uv_tangents) {
		OPTIONAL_FPRINTF(ofp, "\t\tNumber of U tangents = %ld\n", ip_utangent_info.num_coords);
		OPTIONAL_FPRINTF(ofp, "\t\tNumber of V tangents = %ld\n", ip_vtangent_info.num_coords);
	}
	if (Nv_NumEdgesInList)
		OPTIONAL_FPRINTF(ofp, "\t\tNumber of edges = %ld\n", Nv_NumEdgesInList);

	/* Print out all coordinates and index lists */
	NI_Exporter_List_Indice_Info__Original_API(ofp, &ip_vertex_info,   Nc_TRUE,  Nc_TRUE,  "Vertices", 		num_polygons, 	vertices_per_polygon, animation_data_is_available, instance_name, 0L);
	if (export_options->ena_vertex_normals)
		NI_Exporter_List_Indice_Info__Original_API(ofp, &ip_normal_info,   Nc_FALSE, Nc_FALSE, "Normals", 		num_polygons, 	vertices_per_polygon, animation_data_is_available, instance_name, 0L);
	if (export_options->output_vertex_colors)
		NI_Exporter_List_Indice_Info__Original_API(ofp, &ip_color_info,    Nc_FALSE, Nc_FALSE, "Colors", 		num_polygons, 	vertices_per_polygon, animation_data_is_available, instance_name, 0L);
	if (export_options->ena_texture_coords)
		NI_Exporter_List_Indice_Info__Original_API(ofp, &ip_texture_info,  Nc_FALSE, Nc_FALSE, "Texture Coordinates", num_polygons, 	vertices_per_polygon, animation_data_is_available, instance_name, 0L);
	if (export_options->ena_uv_tangents) {
		NI_Exporter_List_Indice_Info__Original_API(ofp, &ip_utangent_info, Nc_FALSE, Nc_FALSE, "U tangents", 		num_polygons, 	vertices_per_polygon, animation_data_is_available, instance_name, 0L);
		NI_Exporter_List_Indice_Info__Original_API(ofp, &ip_vtangent_info, Nc_FALSE, Nc_FALSE, "V tangents", 		num_polygons, 	vertices_per_polygon, animation_data_is_available, instance_name, 0L);
	}
#endif

	// Optionally print out the edge list
	if (Nv_NumEdgesInList && Nv_EdgeList != NULL) {
		Nd_IndexedPolygons_Edge_Info *edge_ptr = Nv_EdgeList;

		OPTIONAL_FPRINTF(ofp, "\t\tEdge list:\n");
		for (i=0; i < Nv_NumEdgesInList && edge_ptr != NULL; ++i) {
			OPTIONAL_FPRINTF(ofp, "\t\t\tEdge # %ld: v1 index = %ld, v2 index = %ld\n", i, edge_ptr->v1, edge_ptr->v2);
			OPTIONAL_FPRINTF(ofp, "\t\t\t\t%ld polygons reference this edge. ", edge_ptr->num_polygon_refs);
			if (edge_ptr->polygon_indices) {
				// These are the 0-based indices of the polygons which reference this edge
				OPTIONAL_FPRINTF(ofp, "Poly indices = ");
				for (j=0; j < edge_ptr->num_polygon_refs; ++j)
					OPTIONAL_FPRINTF(ofp, "%ld ", edge_ptr->polygon_indices[j]);
			}
			OPTIONAL_FPRINTF(ofp, "\n");

			if (edge_ptr->flags & Nc_IP_EDGE_INFO__EDGE_HAS_SAME_NORMALS)
				OPTIONAL_FPRINTF(ofp, "\t\t\t\tNormals are identical at v1 and v2 (edge is 'smooth')\n");
			if (edge_ptr->flags & Nc_IP_EDGE_INFO__EDGE_HAS_SAME_TXTR_UVs)
				OPTIONAL_FPRINTF(ofp, "\t\t\t\tTexture uv's are identical at v1 and v2\n");
			if (edge_ptr->flags & Nc_IP_EDGE_INFO__EDGE_HAS_SAME_COLORs)
				OPTIONAL_FPRINTF(ofp, "\t\t\t\tVertex colors are identical at v1 and v2\n");

			edge_ptr = edge_ptr->next_ptr;
		}
	}

	// This is *purely* example code for your own reference. It shows how you can map a vertex
	// (v1, v2) pair (where v1 and v2 are 0-based offsets into the mesh's vertex array) into
	// a corresponding edge node pointer. The edge list is internally computed by the toolkit
	// on the fly (and accessible in a linear manner via the 'Nv_EdgeList' pointer). The following
	// code takes the (v1,v2) pair and returns a direct pointer to the edge struct.
#if INCLUDE_EDGE_LOOKUP_CODE
	{ Nd_IndexedPolygons_Edge_Info *edge;

	if (edge_hash_table != NULL) {
		Nd_UInt3264	v1, v2;
		Nd_HashString	*edgeTblItem;
		char		edgeSym[22];

		// Developer: set (v1,v2) to the 0-based offsets into the mesh's vertex list. We'll use 0 and 1 as an example.
		v1 = 0;
		v2 = 1;

		if (v1 < v2)
			sprintf(edgeSym, "%d,%d", v1, v2);
		else
			sprintf(edgeSym, "%d,%d", v2, v1);

		// Use the hash table queried from the toolkit to map the (v1,v2) into a 'Nd_IndexedPolygons_Edge_Info *'
		// which points to one of the edges stored along with the mesh internally in the toolkit.
		edgeTblItem = Ni_Hash_Table_LookUpAndAdd(edge_hash_table, (short) Nc_EDGELIST_MAX_HASH_BCKTS, edgeSym, strlen(edgeSym), NULL);
		if (edgeTblItem != NULL)
			edge = ((Nd_IndexedPolygons_Edge_Info *) edgeTblItem->user_data);
		else
			edge = NULL;

		// Developer: at this point you can then use the info in the edge_ptr.
	}}
#endif

	/* Optionally print out the surfaces assigned at the polygon level. */
	if (surface_names != (char **) NULL) {
		OPTIONAL_FPRINTF(ofp, "\t\tSurfaces assigned at the polygon level:\n");
		for (i=0; i < num_polygons; ++i) {
			if (surface_names[i] != (char *) NULL) {
				if (!strcmp(surface_names[i], "NUGRAF___SPECIAL_INHERITED_SURFACE_MARKER"))
					/* This special marker signfies that the surface should be inherited from the instance level */
					OPTIONAL_FPRINTF(ofp, "\t\t\t#%d = \"(surface inherited from parent)\"\n", i);
				else
					OPTIONAL_FPRINTF(ofp, "\t\t\t#%ld = \"%s\"\n", i, surface_names[i]);
			}
		}		
	}

	/* Free up the memory allocated by the toolkit for the surface names list */
	if (surface_names != (char **) NULL) {
		for (i=0; i < num_polygons; ++i) {
			if (surface_names[i] != NULL)
				Ni_Free_Memory((char *) surface_names[i]);
		}
		Ni_Free_Memory((char *) surface_names);
	}

	// If the raw object definition of this instance has skinning weights
	// associated with it, then go print them out. Skinning weights are
	// used for "mesh deformation via skeletons". See the NuGraf 3D Toolkit
	// manual for a full explanation of mesh/NURBS skinning under the influence
	// of a skeleton of bones/joints. Each vertex or CV can be associated with one
	// or more bone/joint instance nodes, each of which is also associated with a
	// double precision "weighting" value that determines how much that bone/joint
	// node influences the vertex. 
	//
	// !! NOTE !! Since we are accessing the skinning weights within this callback,
	//	      the skinning weights will be output from the 'temporary modified'
	//	      mesh, such as that which has had its index arrays shared (all
	//	      depending on which options you have chosen in main.cpp as mesh
	//	      processing options). 
	if (export_options->ena_mesh_skinning_weights)
		NI_Exporter_List_Mesh_Deformation_Skin_Weights_And_BoneBinding(ofp, object_name, child_num, instance_name);

	// Return one of these status values:
	//	Nc_CONVERTANDPROCESS_CALLBACK_OK
	//	Nc_CONVERTANDPROCESS_CALLBACK_ABORT
	//	Nc_CONVERTANDPROCESS_CALLBACK_FATAL_ERROR
	*Nv_Return_Status = Nc_CONVERTANDPROCESS_CALLBACK_OK;
}

// --------------------------------------------------------------------------------------

// List info about vertices/normals/colors/textures/tangents of an indexed polygon mesh
// and their indice list. This relates the OLD API which deals with the static, pre-defined and
// standard vertex coordinate arrays (Nt_NORMAL, Nt_TEXTURE, Nt_COLOR, Nt_UTANGENT and Nt_VTANGENT).
// This is also used to print out the indice info for the Nt_INDEXEDLINES primitive (see ip_lines.cpp).

	Nd_Void
NI_Exporter_List_Indice_Info__Original_API(
	FILE	*ofp, 
	Export_IO_IP_Info_Type *ip,	/* Pointer to the data */
	short	print_indice_count,	/* Nc_TRUE to print out how many indices/poly */
	short	print_surface_names,	/* Nc_TRUE to print out surface names */
	char	*name,			/* "Vertices", "Normals", etc. */
	long	num_polygons, 
	unsigned short	*verticesperpoly_ptr,
	short	animation_data_is_available,	// Nc_TRUE if animation data is available for output
	char	*instance_name,	
	long	num_indices)		// This is only needed, and valid, for the 3D pointset primitive
{
	Nd_Matrix 	pivot_inverse_pivot_matrix;
	Nd_Matrix 	normal_transform_matrix;
	short		embed_pivot_axes_in_vertices = Nc_FALSE;
	long 		i, index_count; 
	Nd_Int		*indice_ptr;
	Nd_Struct_XYZ	*vp_ptr;

	// If we are outputting animation data, then let's go examine the 
	// pivot point matrix associated with this instance. If it is valid
	// then we'll really need to multiple the inverse pivot point into
	// the vertices and normals (see the 'anim.cpp' file for full docs
	// about pivot points and why you need to do this).
	if (animation_data_is_available) 
		embed_pivot_axes_in_vertices = NI_Exporter_GetValidPivotPointMatricesForAnimationExport(instance_name, pivot_inverse_pivot_matrix, normal_transform_matrix);

	if (ip->num_coords) {
		OPTIONAL_FPRINTF(ofp, "\t\t%s:\n", name);

		/* List the vertices */
		vp_ptr = ip->coords;
		for (i = 0; i < ip->num_coords; i++, vp_ptr++) {
			Nd_Vector vec;

			vec[0] = vp_ptr->x;
			vec[1] = vp_ptr->y;
			vec[2] = vp_ptr->z;

// !! DEVELOPER: ANIMATION CODE CHANGE NEEDED !! 
// 
// If we are outputting animation data then go an embed the inverse pivot 
// matrix of the instance (if it is valid) directly into the mesh vertices & normals. 
// If your export file format actually allows for explicit pivot points & 
// orientations to be assigned/associated with a geometry node then don't
// multiply the inverse pivot matrix into the mesh data here. 
#if 00
			if (embed_pivot_axes_in_vertices && !strcmp(name, "Vertices")) 
				Ni_Vector_Transform(1, Nc_TRUE, Nc_TRUE, Nc_TRUE, Nc_TRUE, Nc_TRUE, (Nd_Vector *) vec, pivot_inverse_pivot_matrix, (Nd_Vector *) vec);
			else if (embed_pivot_axes_in_vertices && !strcmp(name, "Normals")) 
				/* Transform the normal by the rows vectors of the forward pivot matrix */
				Ni_Vector_Transform(1, Nc_FALSE, Nc_TRUE, Nc_TRUE, Nc_TRUE, Nc_TRUE, (Nd_Vector *) vec, normal_transform_matrix, (Nd_Vector *) vec);
#endif

			OPTIONAL_FPRINTF(ofp, "\t\t\t%.3f %.3f %.3f\n", vec[0], vec[1], vec[2]);
		}

		/* List the indices list */
		indice_ptr = ip->indices;
		if (!print_indice_count && indice_ptr == NULL) {
			if (strcmp(name, "Points")) {
				OPTIONAL_FPRINTF(ofp, "\t\t%s indice list:\n", name);
				OPTIONAL_FPRINTF(ofp, "\t\t\t%s indices shared with vertex indices\n", name);
			}
		} else if (indice_ptr && num_polygons) {
			OPTIONAL_FPRINTF(ofp, "\t\t%s indice list (1-based):\n", name);

			for (i = 0; i < num_polygons; i++) {
				if (print_indice_count)
					OPTIONAL_FPRINTF(ofp, "\t\t\t(%ld)  ", verticesperpoly_ptr[i]);
				else
					OPTIONAL_FPRINTF(ofp, "\t\t\t");
				if (indice_ptr != NULL) {
					/* List the indices for this polygon */
					for (index_count = 0; index_count < verticesperpoly_ptr[i]; index_count++) {
						if (*indice_ptr == Nc_HOLE_SEPARATOR) {
							/* This separates holes in a single polygon */
							OPTIONAL_FPRINTF(ofp, "HOLE_SEPARATOR ");
							indice_ptr++;
						} else
							OPTIONAL_FPRINTF(ofp, "%ld ", *(indice_ptr++));
					}
				} else
					OPTIONAL_FPRINTF(ofp, "shared with vertex indices, ");
				OPTIONAL_FPRINTF(ofp, "\n");
			}
		} else if (indice_ptr && !num_polygons && num_indices) {
			// This is for the '3d point set' primitive which has zero
			// polygons, and for which the index list length is equal to the
			// length of the vertex coordinates list.

			OPTIONAL_FPRINTF(ofp, "\t\t%s indice list (1-based):\n", name);
			OPTIONAL_FPRINTF(ofp, "\t\t\t");
			for (i = 0; i < num_indices; i++) 
				OPTIONAL_FPRINTF(ofp, "%ld ", *(indice_ptr++));
			OPTIONAL_FPRINTF(ofp, "\n");
		} else if (!indice_ptr && strcmp(name, "Points")) {
			OPTIONAL_FPRINTF(ofp, "\t\t%s indice list:\n", name);
			OPTIONAL_FPRINTF(ofp, "\t\t\tshared with vertex indices,\n");
		}
	}
}

// This prints out the coordinate and indices info associated with one coordinate array of
// an indexed polygon primitive. This version is used with the NEW EXTENDED API which
// handles arbitrary number of arrays per indexed polygon primitive, and for which each array
// can encapsulate any number of different elemental data types (such as vectors, floats, ints,
// matrices, strings-per-coordinate, binary data per coordinate, etc).

// If you wish to test the new vertex coordinate API then execute the "Okino
// Example Importer" and choose the "Mesh with multi-vertex coordinate Arrays"
// example. This will define a scene with 2 polygons and many different types
// of user defined vertex coordinate arrays.

#if USE_NEW_USER_DEFINED_COORDINATES_API
	Nd_Void
NI_Exporter_List_Indice_Info__New_Extended_API(
 	FILE		*ofp,
	Nd_Int		num_polygons,
	Nd_UShort 	*verticesperpoly_ptr,
	Nd_Int 		num_coordinates,
	void 		*coords,
	Nd_Int 		*indices,
	Nd_Int 		indices_shared_with_vertices,
	short		print_indice_count,		// TRUE to print out how many indices/poly
	char		*name,				// "Vertices", "Normals", etc.
	short		animation_data_is_available,	// Nc_TRUE if animation data is available for output
	char		*instance_name,
	// Extended coordinate array info (for multi-uv's, multiple vertex color arrays, etc)
	char 		*user_defined_arrays__handle_name,
	char 		*user_defined_arrays__guid,
	unsigned long 	user_defined_arrays__flags,
	Nd_Int 		user_defined_arrays__datatype,
	Nd_Int 		*user_defined_arrays__size_of_each_element_in_coord_array,
	long		num_indices)		// This is only needed, and valid, for the 3D pointset primitive
{
	Nd_Matrix 	pivot_inverse_pivot_matrix;
	Nd_Matrix 	normal_transform_matrix;
	short		embed_pivot_axes_in_vertices = Nc_FALSE;
	Nd_Vector 	vec;
	long		i, x, y, index_count;
	Nd_Int		*indice_ptr;
	void		*vc;
	Nd_Struct_XYZ	*vp_ptr;

	// If we are outputting animation data, then let's go examine the
	// pivot point matrix associated with this instance. If it is valid
	// then we'll really need to multiple the inverse pivot point into
	// the vertices and normals (see the 'anim.cpp' file for full docs
	// about pivot points and why you need to do this).
	if (animation_data_is_available) 
		embed_pivot_axes_in_vertices = NI_Exporter_GetValidPivotPointMatricesForAnimationExport(instance_name, pivot_inverse_pivot_matrix, normal_transform_matrix);

	if (num_coordinates) {
		OPTIONAL_FPRINTF(ofp, "\t\t%s:\n", name);

		// A general, descriptive name associated with a vertex coordinate array. Can be set to anything.
		if (user_defined_arrays__handle_name)
			OPTIONAL_FPRINTF(ofp, "\t\t\tGeneric handle name = '%s'\n", user_defined_arrays__handle_name);
		// A general, GUID associated with a vertex coordinate array. Can be set to anything.
		if (user_defined_arrays__guid)
			OPTIONAL_FPRINTF(ofp, "\t\t\tGUID = '%s'\n", user_defined_arrays__guid);

		// These flags are usually useless to any exporter. They are general info flags
		// associated with each vertex coordinate array.
		if (user_defined_arrays__flags) {
			OPTIONAL_FPRINTF(ofp, "\t\t\tVertex coordinate flags = %ld ", user_defined_arrays__flags);
			if (user_defined_arrays__flags & Nc_IP_USER_COORDS_FLAGS__UNIT_VECTORS)
				OPTIONAL_FPRINTF(ofp, "(unit vectors) ");
			if (user_defined_arrays__flags & Nc_IP_USER_COORDS_FLAGS__NORMALIZED_COLORS)
				OPTIONAL_FPRINTF(ofp, "(normalized colors) ");
			if (user_defined_arrays__flags & Nc_IP_USER_COORDS_FLAGS__2D_UV_TEXTURES)
				OPTIONAL_FPRINTF(ofp, "(2D uv textures) ");
			if (user_defined_arrays__flags & Nc_IP_USER_COORDS_FLAGS__3D_UV_TEXTURES)
				OPTIONAL_FPRINTF(ofp, "(3D uv textures) ");
			OPTIONAL_FPRINTF(ofp, "\n");
		} else
			OPTIONAL_FPRINTF(ofp, "\t\t\tVertex coordinate flags = %ld\n", user_defined_arrays__flags);

		OPTIONAL_FPRINTF(ofp, "\t\t\t%s vertex coordinate list:\n", name);
		vp_ptr = (Nd_Struct_XYZ	*) coords;
		vc = (void *) coords;
		for (i = 0; i < num_coordinates; i++, vp_ptr++) {
			// The coordinates can (be in theory) different data types...
			switch(user_defined_arrays__datatype) {
	                        // ** This is the most common data type
				case Nc_BDF_USERDATA_NdVector:	

					vec[0] = vp_ptr->x;
					vec[1] = vp_ptr->y;
					vec[2] = vp_ptr->z;

// !! DEVELOPER: ANIMATION CODE CHANGE NEEDED !! 
// 
// If we are outputting animation data then go an embed the inverse pivot 
// matrix of the instance (if it is valid) directly into the mesh vertices & normals. 
// If your export file format actually allows for explicit pivot points & 
// orientations to be assigned/associated with a geometry node then don't
// multiply the inverse pivot matrix into the mesh data here. 
#if 00
					if (embed_pivot_axes_in_vertices && !strcmp(name, "Vertices"))
						Ni_Vector_Transform(1, Nc_TRUE, Nc_TRUE, Nc_TRUE, Nc_TRUE, Nc_TRUE, (Nd_Vector *) vec, pivot_inverse_pivot_matrix, (Nd_Vector *) vec);
					else if (embed_pivot_axes_in_vertices && !strcmp(name, "Normals"))
						/* Transform the normal by the rows vectors of the forward pivot matrix */
						Ni_Vector_Transform(1, Nc_FALSE, Nc_TRUE, Nc_TRUE, Nc_TRUE, Nc_TRUE, (Nd_Vector *) vec, normal_transform_matrix, (Nd_Vector *) vec);
#endif

					OPTIONAL_FPRINTF(ofp, "\t\t\t\t%.3f %.3f %.3f\n", vec[0], vec[1], vec[2]);
					break;

				// -------------------------------------------------------------------------------
				//
				// The remainder of the element data types will rarely if ever be used, but they are
				// available to a developer to associate with each indexed polygon mesh.
				//

				case Nc_BDF_USERDATA_NdColor:
					OPTIONAL_FPRINTF(ofp, "\t\t\t\t%ld: (%g, %g, %g)\n", i, ((Nd_Color *) vc)[i][0], ((Nd_Color *) vc)[i][1], ((Nd_Color *) vc)[i][2] );
					break;
				case Nc_BDF_USERDATA_NdChar:
					// This is binary data of arbitray length. The length of each
					// coordinate element is set by the "user_defined_arrays__size_of_each_element_in_coord_array[i]" array.

#define MAX_ITEMS_TO_LIST	10
					// Print out about 10 items for the list
					if (user_defined_arrays__size_of_each_element_in_coord_array && user_defined_arrays__size_of_each_element_in_coord_array[i]) {
						OPTIONAL_FPRINTF(ofp, "\t\t\t\t%ld: ", i);

						for (x = 0; x < Nm_MIN(MAX_ITEMS_TO_LIST, user_defined_arrays__size_of_each_element_in_coord_array[i]); ++x)
							OPTIONAL_FPRINTF(ofp, "%d, ", ((char **) vc)[i][x]);

						if (user_defined_arrays__size_of_each_element_in_coord_array[i] > MAX_ITEMS_TO_LIST)
							OPTIONAL_FPRINTF(ofp, "...\n");
						else
							OPTIONAL_FPRINTF(ofp, "\n");
					}

					break;
				case Nc_BDF_USERDATA_NdShort:
					OPTIONAL_FPRINTF(ofp, "\t\t\t\t%ld: %d\n", i, ((Nd_Short *) vc)[i]);
					break;
				case Nc_BDF_USERDATA_NdInt:
					OPTIONAL_FPRINTF(ofp, "\t\t\t\t%ld: %ld\n", i, ((Nd_Int *) vc)[i]);
					break;
				case Nc_BDF_USERDATA_NdFloat:
					OPTIONAL_FPRINTF(ofp, "\t\t\t\t%ld: %lf\n", i, ((Nd_Float *) vc)[i]);
					break;
				case Nc_BDF_USERDATA_NdMatrix:
					OPTIONAL_FPRINTF(ofp, "\t\t\t\t%ld: ", i);
					for (x=0; x < 4; ++x)
						for (y=0; y < 4; ++y)
							OPTIONAL_FPRINTF(ofp, "%lf, ", ((Nd_Matrix *) vc)[i][x][y]);
						OPTIONAL_FPRINTF(ofp, "\n");
					break;
				case Nc_BDF_USERDATA_NdDblMatrix:
					OPTIONAL_FPRINTF(ofp, "\t\t\t\t%ld: ", i);
					for (x=0; x < 4; ++x)
						for (y=0; y < 4; ++y)
							OPTIONAL_FPRINTF(ofp, "%lf, ", ((Nd_DblMatrix *) vc)[i][x][y]);
						OPTIONAL_FPRINTF(ofp, "\n");
					break;
				case Nc_BDF_USERDATA_NdTime:
					OPTIONAL_FPRINTF(ofp, "\t\t\t\t%ld: %ld\n", i, ((Nd_Time *) vc)[i]);
					break;
				case Nc_BDF_USERDATA_NdDouble:
					OPTIONAL_FPRINTF(ofp, "\t\t\t\t%ld: %lf\n", i, ((Nd_Double *) vc)[i]);
					break;
				case Nc_BDF_USERDATA_NdVector4:
					OPTIONAL_FPRINTF(ofp, "\t\t\t\t%ld: (%f %f %f %f)\n", i, ((Nd_Vector4 *) vc)[i][0], ((Nd_Vector4 *) vc)[i][1], ((Nd_Vector4 *) vc)[i][2], ((Nd_Vector4 *) vc)[i][3]);
					break;
				case Nc_BDF_USERDATA_NdDblVector:
					OPTIONAL_FPRINTF(ofp, "\t\t\t\t%ld: (%lf %lf %lf)\n", i, ((Nd_DblVector4 *) vc)[i][0], ((Nd_DblVector4 *) vc)[i][1], ((Nd_DblVector4 *) vc)[i][2]);
					break;
				case Nc_BDF_USERDATA_NdDblVector4:
					OPTIONAL_FPRINTF(ofp, "\t\t\t\t%ld: (%lf %lf %lf %lf)\n", i, ((Nd_DblVector4 *) vc)[i][0], ((Nd_DblVector4 *) vc)[i][1], ((Nd_DblVector4 *) vc)[i][2], ((Nd_DblVector4 *) vc)[i][3]);
					break;
				case Nc_BDF_USERDATA_NdString:
					// One string, with terminating zero. It's not possible to associate multiple strings with
					// one user-defined data item (like it is to associate a 20 float array)
					OPTIONAL_FPRINTF(ofp, "\t\t\t\t%ld: '%s'\n", i, ((char **) vc)[i]);
					break;
			}
		}

		/* List the indices list */

		indice_ptr = indices;
		if (!print_indice_count && (indices == NULL || indices_shared_with_vertices)) {
			OPTIONAL_FPRINTF(ofp, "\t\t\t%s indice list (1-based):\n", name);
			// Normal/texture/color/etc indices are shared with the vertex indices
			OPTIONAL_FPRINTF(ofp, "\t\t\t\t%s indices shared with vertex indices\n", name);
		} else if (indice_ptr && num_polygons) {
			OPTIONAL_FPRINTF(ofp, "\t\t\t%s indice list (1-based):\n", name);

			for (i = 0; i < num_polygons; i++) {
				if (print_indice_count)
					OPTIONAL_FPRINTF(ofp, "\t\t\t\t(%ld)  ", verticesperpoly_ptr[i]);
				else
					OPTIONAL_FPRINTF(ofp, "\t\t\t\t");
				if (indice_ptr != NULL) {
					/* List the indices for this polygon */
					for (index_count = 0; index_count < verticesperpoly_ptr[i]; index_count++) {
						if (*indice_ptr == Nc_HOLE_SEPARATOR) {
							/* This separates holes in a single polygon */
							OPTIONAL_FPRINTF(ofp, "HOLE_SEPARATOR ");
							indice_ptr++;
						} else
							OPTIONAL_FPRINTF(ofp, "%ld ", *(indice_ptr++));
					}
				} else
					OPTIONAL_FPRINTF(ofp, "shared with vertex indices, ");
				OPTIONAL_FPRINTF(ofp, "\n");
			}
		} else if (indice_ptr && !num_polygons && num_indices) {
			// This is for the '3d point set' primitive which has zero
			// polygons, and for which the index list length is equal to the
			// length of the vertex coordinates list.

			OPTIONAL_FPRINTF(ofp, "\t\t\t%s indice list (1-based):\n", name);
			OPTIONAL_FPRINTF(ofp, "\t\t\t\t");
			for (i = 0; i < num_indices; i++) 
				OPTIONAL_FPRINTF(ofp, "%ld ", *(indice_ptr++));
			OPTIONAL_FPRINTF(ofp, "\n");
		} else {
			OPTIONAL_FPRINTF(ofp, "\t\t\t%s indice list (1-based):\n", name);
			// Normal/texture/color/etc indices are shared with the vertex indices
			OPTIONAL_FPRINTF(ofp, "\t\t\t\t%s indices shared with vertex indices\n", name);
		}
	}
}
#endif

// --------------------------------------------------------------------------------------

// This routine checks to see if the local axes (pivot matrix) associated 
// with an instance is valid (ie: not identity). If animation is being 
// output for an instance (a geometry instance or a NULL/Empty instance)
// then we need to associated the inverse pivot matrix with that instance.
// One solution is to multiply the inverse pivot matrix into the mesh
// vertices. For Empty Instances, two node should be output instead, top
// top NULL node with the animation assigned to it and the child node which
// has been given a transformation matrix set to the inverse pivot matrix
// (see the top section of 'anim.cpp' for a full explanation of how pivot
// matrices need to be output)

	short
NI_Exporter_GetValidPivotPointMatricesForAnimationExport(char *instance_name, 
	Nd_Matrix pivot_inverse_pivot_matrix,
	Nd_Matrix normal_transform_matrix)
{
	short classification;
	Nd_Axes	*pivot_axes;

	// Get the local axes (pivot matrix) from the instance
	Ni_Inquire_Instance(instance_name, Nt_LOCALAXES, (Nd_Axes **) &pivot_axes, Nt_CMDEND);
	classification = Nc_MATRIX_CLASSIFICATION_IDENTITY;
	// See if the matrix is not an identity...
	if (pivot_axes && (pivot_axes->axes_valid || pivot_axes->origin_valid)) {
		classification = Ni_DetermineMatrixClassification(pivot_axes->matrix);
		if (classification != Nc_MATRIX_CLASSIFICATION_IDENTITY && classification != Nc_MATRIX_CLASSIFICATION_SCALE_ONLY) {
			// Create the inverse pivot matrix which will transform 
			// the vertices from local space to "keyframer space"
			if (!Ni_Matrix_Invert(pivot_axes->matrix, pivot_inverse_pivot_matrix)) 
				Ni_Matrix_Identity(pivot_inverse_pivot_matrix);

			/* And then create the normal transformation matrix */
			Ni_Matrix_Copy(pivot_axes->matrix, normal_transform_matrix);
			normal_transform_matrix[3][0] = normal_transform_matrix[3][1] = normal_transform_matrix[3][2] = 0.0;

			return(Nc_TRUE);
		}
	}

	return(Nc_FALSE);
}

