/*****************************************************************************

	----------------------------------------------------------------
           3D Point Set Geometric Entity Query Functions for PolyTrans
	----------------------------------------------------------------
	     This module receives all the data of a 2D/3D PointSets.
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

/* -------------------->>>>  Local Variables  <<<<-------------------------- */

/* ------------------>>>>  Function Prototypes  <<<<------------------------ */

/* ------------------------>>>>  Point Set Output  <<<<-------------------------- */

// This outputs a "3D point set" object. These primitives contains one
// or more 3D point. Each point can be assigned a vertex color. Each pointset can 
// be assigned its own material, or if none is provided then the material is inherited 
// from the instance level.

// Set the "PointSet" is derived from the "IndexedPolygons" primitive
// we can also use the 'Nt_USERCOORDARRAYS' command of the Ni_Modify_Primitive()
// to associate any arbitrary and generic data with each and every point (such
// as multiple sets of uv texture coordinates, more point colors, point normals,
// matrices per points, floats per points, doubles per points, strings per points, etc)
// See the "indexedpointset.cpp" file in the "import/examples" SDK directory for
// methods on how to associate arbitrary data with each point set. 

// For the sake of how PolyTrans works we will assume that the object 
// definition only contains a single indexed pointset primitive 
// (assume 'Nv_Primitive_Number' is 0). 

	Nd_Void
NI_Exporter_List_PointSet_Primitive(
	FILE 			*ofp,
	Nd_Walk_Tree_Info 	*Nv_Walk_Tree_Info_Ptr,
	char 			*Nv_Master_Object,
	Nd_Int 			Nv_Primitive_Number,
	char 			*Nv_Inherited_Surface_Name,
	char 			*instance_name)
{
	Export_IO_IP_Info_Type	ip_vertex_info, ip_color_info;
	Nd_Int			i, child_num;
	Nd_Token		primitive_type;
	short			animation_data_is_available = Nc_FALSE;

	// Let's see if we've output this object definition before. If so, create a new instance of the existing object
	if (NI_Handle_Okino_Object_Instancing(Nv_Walk_Tree_Info_Ptr))
		return;	// Instance was created. Return no error

	child_num = 0;
	ip_vertex_info.indices = NULL;	// No index array for vertex coordinates of a 3D pointset
	Ni_Inquire_Primitive(Nt_OBJECT, Nv_Master_Object, child_num, &primitive_type,
		Nt_POINTSET,
			Nt_POINTS,
				// 'num_coords' defines the total number of points.
				// NOTE: there is no index array for the vertex coordinates.
				(Nd_Int *) &ip_vertex_info.num_coords,
				(Nd_Struct_XYZ **) &ip_vertex_info.coords,
			Nt_POINTSETCOLORS,
				// Since the colors are indexed, the number of colors defined can be
				// less than the number of vertex coordinates
				(Nd_Int *) &ip_color_info.num_coords,
				(Nd_Struct_XYZ **) &ip_color_info.coords,
				(Nd_Int **) &ip_color_info.indices,	// 1- based indices
				(Nd_Int *) &ip_color_info.indices_shared_with_vertex_indices,
			Nt_CMDSEP,
		Nt_CMDEND);

	// Update the stats
	++export_options->total_pointsets;
	export_options->total_pointset_points += ip_vertex_info.num_coords;

	// Go and see if there is any animation channel data associated
	// with this geometry instance that we are interested about. If resampled
	// keyframe mode is in effect then temporary resampled keyframe lists
	// will have been created and stored in the core toolkit after this 
	// function finishes and returns here. 
	if (export_options->ena_object_animation && export_options->ena_hierarchy) {
		Nd_Int Nv_Active_Channels_Found = Nc_FALSE;
		Ni_NodeHasActiveChannels( Nt_INSTANCE, instance_name, Nc_TRUE /* keyframe controllers only */, &Nv_Active_Channels_Found, Nt_CMDEND );
		if ( Nv_Active_Channels_Found )
		animation_data_is_available = NI_Exporter_QueryAndSetup_Object_Animation_Keyframe_Data(ofp, instance_name);
	}

	OPTIONAL_FPRINTF(ofp, "\t\tTotal number of point coordinates = %ld\n", ip_vertex_info.num_coords);
	if (export_options->output_vertex_colors)
		OPTIONAL_FPRINTF(ofp, "\t\tNumber of colours = %ld\n", ip_color_info.num_coords);

	/* Print out all coordinates and index lists */
	NI_Exporter_List_Indice_Info__Original_API(ofp, &ip_vertex_info, Nc_TRUE, Nc_TRUE, "Points", 0, NULL, animation_data_is_available, instance_name, ip_vertex_info.num_coords);
	// Each vertex of the pointset can optionally have a color assigned to it.
	if (export_options->output_vertex_colors)
		NI_Exporter_List_Indice_Info__Original_API(ofp, &ip_color_info,    Nc_FALSE, Nc_FALSE, "Colors", 0, NULL, animation_data_is_available, instance_name, ip_vertex_info.num_coords);

	// --------------------------------------------

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

	// Determine how many user defined coordinate arrays are defined for this 3D pointset
	// primitive and query the handle names for all of these arrays so that we can query the
	// parameters of each array further below.
	Nv_Num_User_Coords_Arrays = 0;
	Nv_Coord_Array_Names = NULL;
	Ni_Inquire_Primitive(Nt_OBJECT, Nv_Master_Object, child_num, (Nd_Token *) &primitive_type,
		Nt_POINTSET,
			// Return the number of user defined coordinate arrays
			Nt_NUMUSERCOORDARRAYS, (Nd_Int *) &Nv_Num_User_Coords_Arrays,

			// Returns a (char **) array of all coordinate lists array names
			//  with the last entry NULL. These names are used to access each
			// list uniquely.
			Nt_GETUSERCOORDARRAYNAMES, (char ***) &Nv_Coord_Array_Names,
		Nt_CMDEND);

	// Now list the of coordinates in the extended user defined vertex coordinate arrays
	if (Nv_Num_User_Coords_Arrays && Nv_Coord_Array_Names) {
		for (i=0; i <Nv_Num_User_Coords_Arrays; ++i) {
			if (Nv_Coord_Array_Names[i] != NULL) {
				// Pick up the information about one user defined coordinate array
				Ni_Inquire_Primitive(Nt_OBJECT, Nv_Master_Object, child_num, (Nd_Token *) &primitive_type,
					Nt_POINTSET,
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

				NI_Exporter_List_Indice_Info__New_Extended_API(ofp, 0, NULL, user_defined_arrays__num_coords, user_defined_arrays__element_list,
					user_defined_arrays__indices, user_defined_arrays__shared_with_vertices_enable == Nt_ON, Nc_TRUE, "Custom Data", animation_data_is_available, instance_name,
					user_defined_arrays__handle_name, user_defined_arrays__guid, user_defined_arrays__flags, user_defined_arrays__datatype,
					user_defined_arrays__size_of_each_element_in_coord_array, ip_vertex_info.num_coords);
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

	// --------------------------------------------

// Developer: this is reference code which shows you how our JT exporter creates the JT part
// definition, attaches it to the JT hierarchy, assigns the material, assigns the transformation,
// assigns the meta data, and adds our object definition to the instancing list for future lookup.
#if 00
	try {
		// Make sure the JT assembly name becomes unique within the JT namespace
		char part_name[1024];
		NI_FixUpJTHandleName(Nt_INSTANCE, Nv_Walk_Tree_Info_Ptr->Nv_Handle_Name, part_name);

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
			NI_JT_Assign_Transform_Material_Texture_To_Geometry_Part(Part, Nv_Walk_Tree_Info_Ptr->Nv_CTM, Nv_Walk_Tree_Info_Ptr->Nv_Surface_Name);
		else
			NI_JT_Assign_Transform_Material_Texture_To_Geometry_Part(Part, Nv_Walk_Tree_Info_Ptr->Nv_Hierarchical_CTM, Nv_Walk_Tree_Info_Ptr->Nv_Surface_Name);

		// Assign the instance meta dat
		NI_JT_Output_Meta_Data(Part, Nt_INSTANCE, instance_name);

		// Add the part to our object look-up map for future instancing reference by NI_Handle_Okino_Object_Instancing()
		char 	dummy_string[300];
		Ni_StringDictionary_UniqueInsert(export_options->list_of_multiply_instanced_nugraf_objects,
			Nv_Master_Object,	// The Okino object definition name
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

	// If animation data was previously found and set up, then go output
	// the animation data associated with this instance. 
	if (animation_data_is_available)
		NI_Exporter_Output_Object_Animation_Data(ofp, instance_name);
}
