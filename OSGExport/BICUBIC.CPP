/*****************************************************************************

	----------------------------------------------------------------
          Bicubic Patch Geometric Entity Query Functions for PolyTrans
	----------------------------------------------------------------
	This module queries all the parameters of a Bicubic Patch entity.
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

static Nd_Void	NI_Exporter_List_Bicubic_Parameters(FILE *ofp, Nd_Token type, float tension, float bias);

/* -------------------->>>>  Bicubic Patch Output  <<<<----------------------- */

// This outputs a bicubic patch object. For the sake of how PolyTrans works
// we will assume that the object definition only contains a single bicubic
// patch primitive (assume 'Nv_Primitive_Number' is 0) although there might
// be cases where a single object contains more than one bicubic patch 
// primitive (such as the teapot2.bdf file). The default material for the 
// bicubic patch will be 'Nv_Inherited_Surface_Name'.

	Nd_Void
NI_Exporter_List_Bicubic_Patch_Primitive(
	FILE 			*ofp,
	Nd_Walk_Tree_Info 	*Nv_Walk_Tree_Info_Ptr,
	char 			*Nv_Master_Object,
	Nd_Int 			Nv_Primitive_Number,
	char 			*Nv_Inherited_Surface_Name,
	char 			*instance_name)
{
	Nd_Int 		Nv_Num_U_Control_Points;
	Nd_Int 		Nv_Num_V_Control_Points;
	Nd_Vector	*Nv_Vertex_Array;
	Nd_Token	Nv_U_Basis_Type, Nv_V_Basis_Type;
	Nd_Float 	Nv_U_Bias;
	Nd_Float 	Nv_V_Bias;
	Nd_Float 	Nv_U_Tension;
	Nd_Float 	Nv_V_Tension;
	Nd_Int 		Nv_U_Subdiv;
	Nd_Int 		Nv_V_Subdiv;
	Nd_Token	primitive_type;
	short		animation_data_is_available = Nc_FALSE;
	Nd_Matrix 	pivot_inverse_pivot_matrix;
	Nd_Matrix 	normal_transform_matrix;
	short		embed_pivot_axes_in_vertices = Nc_FALSE;
	int		count;

	// Let's see if we've output this object definition before. If so, create a new instance of the existing object
	if (NI_Handle_Okino_Object_Instancing(Nv_Walk_Tree_Info_Ptr))
		return;	// Instance was created. Return no error

	Ni_Inquire_Primitive(Nt_OBJECT, Nv_Master_Object, Nv_Primitive_Number, &primitive_type,
		Nt_PATCH,
			Nt_NUMUCTRLPNTS, (Nd_Int *) &Nv_Num_U_Control_Points, Nt_CMDSEP,
			Nt_NUMVCTRLPNTS, (Nd_Int *) &Nv_Num_V_Control_Points, Nt_CMDSEP,
			Nt_VERTICES,	(Nd_Vector *) &Nv_Vertex_Array, Nt_CMDSEP,
			Nt_UBASIS,	&Nv_U_Basis_Type, Nt_CMDSEP,
			Nt_VBASIS,	&Nv_V_Basis_Type, Nt_CMDSEP,
			Nt_UBIAS,	(Nd_Float *) &Nv_U_Bias, Nt_CMDSEP,
			Nt_VBIAS,	(Nd_Float *) &Nv_V_Bias, Nt_CMDSEP,
			Nt_UTENSION,	(Nd_Float *) &Nv_U_Tension, Nt_CMDSEP,
			Nt_VTENSION,	(Nd_Float *) &Nv_V_Tension, Nt_CMDSEP,
			Nt_USUBDIV,	(Nd_Int *) &Nv_U_Subdiv, Nt_CMDSEP,
			Nt_VSUBDIV,	(Nd_Int *) &Nv_V_Subdiv, Nt_CMDSEP,
			Nt_CMDEND);

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

		// If we are outputting animation data, then let's go examine the 
		// pivot point matrix associated with this instance. If it is valid
		// then we'll really need to multiple the inverse pivot point into
		// the vertices and normals (see the 'anim.cpp' file for full docs
		// about pivot points and why you need to do this).
		embed_pivot_axes_in_vertices = NI_Exporter_GetValidPivotPointMatricesForAnimationExport(instance_name, pivot_inverse_pivot_matrix, normal_transform_matrix);
	}

	OPTIONAL_FPRINTF(ofp, "\t\tBicubic patch\n");

	OPTIONAL_FPRINTF(ofp, "\t\t\tNumber of control points: u = %d, v = %d\n",
		Nv_Num_U_Control_Points, Nv_Num_V_Control_Points);
	NI_Exporter_List_Bicubic_Parameters(ofp, Nv_U_Basis_Type, Nv_U_Tension, Nv_U_Bias);
	NI_Exporter_List_Bicubic_Parameters(ofp, Nv_V_Basis_Type, Nv_V_Tension, Nv_V_Bias);

	if (Nv_U_Subdiv < 1)
		OPTIONAL_FPRINTF(ofp, "\t\t\tSubdiv. in U set by global subdiv parameter\n");
	else 
		OPTIONAL_FPRINTF(ofp, "\t\t\tSubdivisions in U = %d\n", Nv_U_Subdiv);
	if (Nv_V_Subdiv < 1)
		OPTIONAL_FPRINTF(ofp, "\t\t\tSubdiv. in V set by global subdiv parameter\n");
	else 
		OPTIONAL_FPRINTF(ofp, "\t\t\tSubdivisions in V = %d\n", Nv_V_Subdiv);

	OPTIONAL_FPRINTF(ofp, "\t\t\tControl points:\n");
	for (count = 0; count < Nv_Num_U_Control_Points * Nv_Num_V_Control_Points; ++count) {
		Nd_Vector vec;

		Nm_COPY_VECTOR(Nv_Vertex_Array[count], vec);

// !! DEVELOPER: ANIMATION CODE CHANGE NEEDED !! 
// 
// If we are outputting animation data then go an embed the inverse pivot 
// matrix of the instance (if it is valid) directly into the bicubic patch CVs. 
// If your export file format actually allows for explicit pivot points & 
// orientations to be assigned/associated with a geometry node then don't
// multiply the inverse pivot matrix into the mesh data here. 
#if 00
		if (embed_pivot_axes_in_vertices) 
			Ni_Vector_Transform(1, Nc_TRUE, Nc_TRUE, Nc_TRUE, Nc_TRUE, Nc_TRUE, (Nd_Vector *) vec, pivot_inverse_pivot_matrix, (Nd_Vector *) vec);
#endif

		OPTIONAL_FPRINTF(ofp, "\t\t\t\t( %.5f, %.5f, %.5f )\n", vec[0], vec[1], vec[2]);
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

/* List the U or V patch parameters */

	static Nd_Void
NI_Exporter_List_Bicubic_Parameters(FILE *ofp, Nd_Token type, float tension, float bias)
{
	if (type == Nt_BETA)
		OPTIONAL_FPRINTF(ofp, "\t\t\tBeta spline. Bias = %.2f, tension = %.2f\n", bias, tension);
	else if (type == Nt_BETA2)
		OPTIONAL_FPRINTF(ofp, "\t\t\tBeta-2 spline. Tension = %.2f\n", tension);
	else if (type == Nt_BEZIER)
		OPTIONAL_FPRINTF(ofp, "\t\t\tBezier spline.\n");
	else if (type == Nt_CARDINAL)
		OPTIONAL_FPRINTF(ofp, "\t\t\tCardinal spline. Tension = %.2f\n", tension);
	else if (type == Nt_TENSION)
		OPTIONAL_FPRINTF(ofp, "\t\t\tTensioned spline. Tension = %.2f\n", tension);
}
