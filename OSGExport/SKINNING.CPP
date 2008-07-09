/*****************************************************************************

	----------------------------------------------------------------
           Mesh Deformation via Rigid/Smooth Skin Binding of a Skeleton
		     Query & Output Functions for PolyTrans
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

*****************************************************************************/

// *****************
//
// NOTE: See the NuGraf 3D Toolkit for complete docs on the Ni_Skinning() and
//       Ni_Inquire_Skinning() commands. There are quite a few extra methods to
//	 set/delete skin weights, or query skin weights in various fashions
//	 beyond what is shown below but which is documented in the toolkit manual.
//
// *****************

#include "main.h"	// Main include file for a PolyTrans exporter

// ----------------------->>>>  Definitions  <<<<---------------------------

// -------------------->>>>  Function Prototypes <<<<--------------------------

static void	NI_Exporter_Output_Skin_Weights_By_Bone_Name(FILE *fp, char *instance_handle_name, char *master_object);
static short	NI_Exporter_DoesGeometryObjectHaveValidSkinWeights(char *instance_name);
static Nd_Short	NI_Exporter_Is_Object_Empty(char *object_name);

// -------------------------------------------------------------------------

	Nd_Void
NI_Exporter_List_Mesh_Deformation_Skin_Weights_And_BoneBinding(FILE *ofp,
	char *Nv_Master_Object, Nd_Int Nv_Primitive_Number, 
	char *instance_name)
{
	// Skinning query variables
	Nd_Int 		Nv_NumWeightedVertices;
	Nd_Int 		Nv_Num_Vertices_With_Weights;
	Nd_Int 		*Nv_Vertex_Offsets;
	Nd_Int 		*Nv_BonesPerVertex;
	Nd_Double 	*Nv_Weight_Array;
	char 		**Nv_Joint_Names;

	// See if there are any bone weights associated with this mesh
	Ni_Inquire_Skinning(Nv_Master_Object, Nv_Primitive_Number,
		// Get the number of vertices that have skinned weights associated with them
		Nt_GETNUMWEIGHTEDVERTICES,
			(Nd_Int *) &Nv_NumWeightedVertices,
			Nt_CMDSEP,
//		// Get the number of unique bones that are associated with this skinned mesh
//		Nt_GETNUMREFERENCEDBONES,
//			(Nd_Int *) &Nv_NumBonesInSkinnedMesh,
//			Nt_CMDSEP,
		// Get all vertex weights associated with every (weighted vertex) and
		// their associated bone names.
		Nt_GETVERTEXWEIGHTS, Nt_ALL,
			// This is the number of vertices that have weights assigned to them
			// and which are being returned from the toolkit to the host program.
			(Nd_Int *)	&Nv_Num_Vertices_With_Weights,
			// This is an array containing the 0-based vertex indices that that have one
			// or more weights associated with them. Must be freed up by the caller when done.
			// The length of this array can be Nv_Num_Vertices_With_Weights.
			(Nd_Int **)	&Nv_Vertex_Offsets,	// 0-based
			// This is how many bones (and their weights) will are assigned to each vertex
			// returned from this call. Must be freed up by the caller when done.
			// The length of this array can be Nv_Num_Vertices_With_Weights.
			(Nd_Int **)	&Nv_BonesPerVertex,
			// This is all of the returned weights in a single array of length "SUM(Nv_BonesPerVertex[1..Nv_Num_Vertices_With_Weights])"
			// Must be freed up by the caller when done.
			(Nd_Double **) 	&Nv_Weight_Array,
			// This is the joint (bone) instance name associated with each weight of the Nv_Weight_Array[]. Array length of "SUM(Nv_BonesPerVertex[1..Nv_Num_Vertices_With_Weights])"
			// Must be freed up by the caller when done.
			(char ***)	&Nv_Joint_Names,
			Nt_CMDEND);

	if (Nv_NumWeightedVertices) {
		long	offset, j, i;

		OPTIONAL_FPRINTF(ofp, "\n\t---------->>>> Weighted skinning information <<<<---------\n\n");

		offset = 0;
		for (i=0; i < Nv_Num_Vertices_With_Weights; ++i) {
			OPTIONAL_FPRINTF(ofp, "\tVertex offset = %d\n", Nv_Vertex_Offsets[i]);

			for (j=0; j < Nv_BonesPerVertex[i]; ++j) {
				OPTIONAL_FPRINTF(ofp, "\t\tWeight = %g, influencing joint name = '%s'\n", Nv_Weight_Array[offset], Nv_Joint_Names[offset]);
				++offset;
			}
		}
	}

	// Free up the arrays
	if (Nv_Joint_Names) {
		long i, j, offset = 0;
		for (i=0; i < Nv_Num_Vertices_With_Weights; ++i) {
			for (j=0; j < Nv_BonesPerVertex[i]; ++j) {
				if (Nv_Joint_Names[offset])
					Ni_Free_Memory((char *) Nv_Joint_Names[offset]);
				++offset;
			}
		}
		Ni_Free_Memory((char *) Nv_Joint_Names);
	}
	if (Nv_Vertex_Offsets)
		Ni_Free_Memory((char *) Nv_Vertex_Offsets);
	if (Nv_BonesPerVertex)
		Ni_Free_Memory((char *) Nv_BonesPerVertex);
	if (Nv_Weight_Array)
		Ni_Free_Memory((char *) Nv_Weight_Array);

	// Now show an alternative way to query and output the skin weights,
	// by doing it one bone at a time
	NI_Exporter_Output_Skin_Weights_By_Bone_Name(ofp, instance_name, Nv_Master_Object);
}

// This is one of several alternative methods to query skin weights of an
// object definition. This one queries the skin weights one bone name at
// a time.

	static void
NI_Exporter_Output_Skin_Weights_By_Bone_Name(FILE *ofp, 
	char *instance_handle_name, char *master_object)
{
	Nd_Int 		Nv_NumWeightedVertices;
	Nd_Int  	Nv_NumUniqueBones; 	// Number of unique bone names in the list
	char 	 	**Nv_BoneNameList; 	// An array of string pointers, each pointer to the unique bone name
	short		bone_num;
	Nd_Int 		Nv_Num_Vertices_With_Weights;
	Nd_Int 		*Nv_Vertex_Offsets;
	Nd_Double  	*Nv_Weight_Array;
	Nd_Int		i;

	// See if this mesh or NURBS patch has any skinned/weighted vertices
	Ni_Inquire_Skinning(master_object, 0, Nt_GETNUMWEIGHTEDVERTICES, (Nd_Int *) &Nv_NumWeightedVertices, Nt_CMDEND);
	if (!Nv_NumWeightedVertices)
		return;

	// This returns a list of all the unique bone names assigned to all vertices.
	// The caller must free up the strings of each list entry as well as the list itself. 
	// The (char **) array and each string is allocated by the toolkit. 
	Ni_Inquire_Skinning(master_object, 0, 
		Nt_GETUNIQUEBONENAMES,
			(Nd_Int **) 	&Nv_NumUniqueBones, 	// Number of unique bone names in the list
			(char ***) 	&Nv_BoneNameList, 	// An array of string pointers, each pointer to the unique bone name
		Nt_CMDEND);
	if (!Nv_NumUniqueBones || !Nv_BoneNameList)
		return;

	OPTIONAL_FPRINTF(ofp, "\n\t---------->>>> Weighted skinning information, Per Bone <<<<---------\n\n");

	// Loop over all of the bones associated with this object
	for (bone_num=0; bone_num < Nv_NumUniqueBones; ++bone_num) {
		Ni_Inquire_Skinning(master_object, 0, 
			// Get all vertex weights associated with every (weighted vertex) which
			// is associated with the specified single bone name
			Nt_GETVERTEXWEIGHTS, Nt_BYBONENAME,
				// This is the name of the bone we're querying about
				(char *) Nv_BoneNameList[bone_num],
				// This is the number of vertices that have weights assigned to them
				// and which are being returned from the toolkit to the host program.
				(Nd_Int *)	&Nv_Num_Vertices_With_Weights,
				// This is an array containing the 0-based vertex indices that that have one
				// or more weights associated with them. Must be freed up by the caller when done.
				// The length of this array can be Nv_Num_Vertices_With_Weights.
				(Nd_Int **)	&Nv_Vertex_Offsets,	// 0-based
				// This is all of the returned weights in a single array of length "SUM(Nv_BonesPerVertex[1..Nv_Num_Vertices_With_Weights])"
				// Must be freed up by the caller when done.
				(Nd_Double **) 	&Nv_Weight_Array,
				Nt_CMDEND);

		OPTIONAL_FPRINTF(ofp, "\tSkeleton joint (bone) name = '%s'\n", Nv_BoneNameList[bone_num]);
		OPTIONAL_FPRINTF(ofp, "\t\tNumber of vertices with weights = '%d'\n", Nv_Num_Vertices_With_Weights);
		for (i=0; i < Nv_Num_Vertices_With_Weights; ++i)
			OPTIONAL_FPRINTF(ofp, "\t\t\tVertex offset = %d, weight = %g\n", Nv_Vertex_Offsets[i], Nv_Weight_Array[i]);
	}

	// Free up the arrays
	if (Nv_BoneNameList) {
		for (i=0; i < Nv_NumUniqueBones; ++i) {
			if (Nv_BoneNameList[i])
				Ni_Free_Memory((char *) Nv_BoneNameList[i]);
		}
		Ni_Free_Memory((char *) Nv_BoneNameList);
	}
	if (Nv_Vertex_Offsets)
		Ni_Free_Memory((char *) Nv_Vertex_Offsets);
	if (Nv_Weight_Array)
		Ni_Free_Memory((char *) Nv_Weight_Array);
}

// For reference purposes, this routine determines if a mesh or NURBS object 
// has valid skinning weights associated with it. 

	static short
NI_Exporter_DoesGeometryObjectHaveValidSkinWeights(char *instance_name)
{
	Nd_Int  	Nv_NumUniqueBones; 	// Number of unique bone names in the list
	char 	 	**Nv_BoneNameList; 	// An array of string pointers, each pointer to the unique bone name
	Nd_Int		i, Nv_NumWeightedVertices;
	char 		*master_object;

	Ni_Inquire_Instance(instance_name,
		Nt_MASTEROBJECT, (char **) &master_object, Nt_CMDEND);
	if (!master_object)
		return(Nc_FALSE);
	if (NI_Exporter_Is_Object_Empty(master_object))
		return(Nc_FALSE);

	// See if this mesh or NURBS patch has any skinned/weighted vertices
	Ni_Inquire_Skinning(master_object, 0, Nt_GETNUMWEIGHTEDVERTICES, (Nd_Int *) &Nv_NumWeightedVertices, Nt_CMDEND);
	if (!Nv_NumWeightedVertices)
		return(Nc_FALSE);

	Ni_Inquire_Skinning(master_object, 0, 
		Nt_GETUNIQUEBONENAMES,
			(Nd_Int **) 	&Nv_NumUniqueBones, 	// Number of unique bone names in the list
			(char ***) 	&Nv_BoneNameList, 	// An array of string pointers, each pointer to the unique bone name
		Nt_CMDEND);
	if (!Nv_NumUniqueBones || !Nv_BoneNameList)
		return(Nc_FALSE);

	// Free up the arrays
	if (Nv_BoneNameList) {
		for (i=0; i < Nv_NumUniqueBones; ++i) {
			if (Nv_BoneNameList[i])
				Ni_Free_Memory((char *) Nv_BoneNameList[i]);
		}
		Ni_Free_Memory((char *) Nv_BoneNameList);
	}

	return(Nc_TRUE);
}

	static Nd_Short
NI_Exporter_Is_Object_Empty(char *object_name)
{
	Nd_Int	num_primitives;

	Ni_Inquire_Object(object_name, 
		Nt_NUMPRIMITIVES, (Nd_Int *) &num_primitives, Nt_CMDSEP,
		Nt_CMDEND);

	if (!num_primitives)
		return(Nc_TRUE);
	else
		return(Nc_FALSE);
}
