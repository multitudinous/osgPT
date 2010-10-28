/*****************************************************************************

	----------------------------------------------------------------
          NURBS Surface Geometric Entity Query Functions for PolyTrans
	----------------------------------------------------------------
	This module queries all the parameters of a NURBS Surface entity.
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

static Nd_Void	NI_Exporter_List_NURBS_Surface_Trim_Curves_Info(FILE *ofp, char *Nv_Master_Object, Nd_Token primitive_query_type, Nd_Int Nv_Primitive_Number);

/* -------------------->>>>  NURBS Surface Output  <<<<----------------------- */

// This outputs a trimmed NURBS Surface object. For the sake of how PolyTrans works
// we will assume that the object definition only contains a single NURBS
// surface primitive (assume 'Nv_Primitive_Number' is 0) although there might
// be cases where a single object contains more than one NURBS surface.
// The default material for the spline shape will be 'Nv_Inherited_Surface_Name'.

// Returns TRUE if no valid NURBS patch was found (such as if a NURBS curve
// or Spline Shape was asked to be temporarily converted into a NURBS surface
// for export.

	Nd_Short
NI_Exporter_List_NURBS_Surface_Primitive(
	FILE 			*ofp,
	Nd_Walk_Tree_Info 	*Nv_Walk_Tree_Info_Ptr,
	char 			*Nv_Master_Object,
	Nd_Int 			Nv_Primitive_Number,
	char 			*Nv_Inherited_Surface_Name,
	char 			*instance_name)
{
	Export_IO_NURBPatchParameters 	nurb_info;
	Nd_Double	*ptr;
	int		count;
	Nd_Token	primitive_query_type, primitive_type;
	short		animation_data_is_available = Nc_FALSE;
	Nd_Matrix 	pivot_inverse_pivot_matrix;
	Nd_Matrix 	normal_transform_matrix;
	short		embed_pivot_axes_in_vertices = Nc_FALSE;

	// Let's see if we've output this object definition before. If so, create a new instance of the existing object
	if (NI_Handle_Okino_Object_Instancing(Nv_Walk_Tree_Info_Ptr))
		return Nc_FALSE;	// Instance was created. Return no error

	// If the caller is telling us that the primitive contains either 
	// a 'Spline Shape' primitive or a NURBS Curve primitive, but a 'NURBS Surface' 
	// (Patch) primitive is wanted instead, then feed the 'Nt_TEMPNURBSPATCH'
	// token to the primitive inquiry function. This will internally (inside the toolkit)
	// convert the spline shape or NURBS curve primitive to a temporary NURBS surface (trimmed)
	// primitive which can be accessed by the normal NURBS surface/patch toolkit query functions,
	// except that the main 'Nt_NURBPATCH' parameter is replaced by 'Nt_TEMPNURBSPATCH'.
	Ni_Inquire_Primitive(Nt_OBJECT, Nv_Master_Object, Nv_Primitive_Number, &primitive_type, Nt_CMDEND);
	if (primitive_type == Nt_NURBCURVE)
		// Oops, the caller of this function is handing us a NURBS Curve
		// primitive but we want to export this as a NURBS Patch instead.
		// We'll query via the Nt_TEMPNURBSPATCH so that a temporary NURBS 
		// patch primitive is created instead.
		primitive_query_type = Nt_TEMPNURBSPATCH;
	else if (primitive_type == Nt_SPLINESHAPE)
		// Oops, the caller of this function is handing us a Spline Shape
		// primitive but we want to export this as a NURBS Patch instead.
		// We'll query via the Nt_TEMPNURBSPATCH so that a temporary NURBS 
		// patch primitive is created instead.
		primitive_query_type = Nt_TEMPNURBSPATCH;
	else
		// Ok, this is a pure NURBS patch so let's output it.
		primitive_query_type = Nt_NURBPATCH;

	// Get all of the parameters related to the Nth NURBS surface primitive 
	// of this object definition (but not its trim curves). 'Nv_Primitive_Number'
	// never usually goes beyond 0. 'primitive_query_type' is either set to
	// Nt_NURBPATCH or Nt_TEMPNURBSPATCH.
	Export_IO_Inquire_NURB_Patch(primitive_query_type, Nv_Master_Object, Nv_Primitive_Number, &nurb_info);

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

	// Let's see if we found a valid NURBS surface. If we are converting a closed
	// NURBS curve or Spline Shape primitive to a temporary NURBS surface for
	// export then it is possible that this conversion process has failed.
	if (!nurb_info.Nv_Num_U_Control_Points || !nurb_info.Nv_Num_V_Control_Points)
		return(Nc_TRUE);

       	// Update the stats
       	++export_options->total_nurbs_surfaces;

	OPTIONAL_FPRINTF(ofp, "\n\t'NURBS Surface' primitive defined in master object definition '%s':\n", Nv_Master_Object);

	OPTIONAL_FPRINTF(ofp, "\t\tNumber of control points: u = %d, v = %d\n", nurb_info.Nv_Num_U_Control_Points, nurb_info.Nv_Num_V_Control_Points);

	OPTIONAL_FPRINTF(ofp, "\t\tPatch order: u = %d, v = %d\n", nurb_info.Nv_U_Order, nurb_info.Nv_V_Order);

	// Okino products use adaptive NURBS patch tessellation, so these parameters can be ignored
	if (nurb_info.Nv_U_Subdiv < 1)
		OPTIONAL_FPRINTF(ofp, "\t\tSubdiv. in U set by global subdiv parameter\n");
	else 
		OPTIONAL_FPRINTF(ofp, "\t\tSubdivisions in U = %d\n", nurb_info.Nv_U_Subdiv);
	if (nurb_info.Nv_V_Subdiv < 1)
		OPTIONAL_FPRINTF(ofp, "\t\tSubdiv. in V set by global subdiv parameter\n");
	else 
		OPTIONAL_FPRINTF(ofp, "\t\tSubdivisions in V = %d\n", nurb_info.Nv_V_Subdiv);

	OPTIONAL_FPRINTF(ofp, "\t\tUrange: umin = %g, umax = %g\n", nurb_info.Nv_U_Ranges[0], nurb_info.Nv_U_Ranges[1]);
	OPTIONAL_FPRINTF(ofp, "\t\tVrange: vmin = %g, vmax = %g\n", nurb_info.Nv_V_Ranges[0], nurb_info.Nv_V_Ranges[1]);

	// This is Nt_ON if the control vertices have 1 or more non-1.0 weight values
	if (nurb_info.Nv_Surface_Is_Rational == Nt_ON)
		OPTIONAL_FPRINTF(ofp, "\t\tPatch is rational\n");

	// This is an Okino runtime flag. If Nt_ON then the vertex normals 
	// generated from the tessellated surface will be reversed.
	if (nurb_info.Nv_Reverse_Normal_Enable == Nt_ON)
		OPTIONAL_FPRINTF(ofp, "\t\tNormals will be reversed\n");

	// Clamped U knots
	OPTIONAL_FPRINTF(ofp, "\t\tU knots: ");
	for (count = 0; count < nurb_info.Nv_Num_U_Control_Points + nurb_info.Nv_U_Order; ++count)
		OPTIONAL_FPRINTF(ofp, "%g ", nurb_info.Nv_U_Knots[count]);

	// Clamped V knots
	OPTIONAL_FPRINTF(ofp, "\n\t\tV knots: ");
	for (count = 0; count < nurb_info.Nv_Num_V_Control_Points + nurb_info.Nv_V_Order; ++count)
		OPTIONAL_FPRINTF(ofp, "%g ", nurb_info.Nv_V_Knots[count]);
	OPTIONAL_FPRINTF(ofp, "\n");

	// uv texture coordinates per patch corner: this is really not used anywhere
	if (nurb_info.Nv_Texture_Coordinates != (float *) NULL) {
		OPTIONAL_FPRINTF(ofp, "\t\tTexture coordinates:\n");
		for (count = 0; count < 8; count += 2)
			OPTIONAL_FPRINTF(ofp, "\t\t\tu = %g, v = %g\n", nurb_info.Nv_Texture_Coordinates[count], nurb_info.Nv_Texture_Coordinates[count+1]);
	}

	// Control vertices. Stored as single array of doubles. If the surface
	// is rational then stored as (x,y,z,w) else (x,y,z). The CVs are stored in 
	// u direction order.
	OPTIONAL_FPRINTF(ofp, "\t\tControl points:\n");
	ptr = nurb_info.Nv_Vertex_Array;
	for (count = 0; count < nurb_info.Nv_Num_U_Control_Points * nurb_info.Nv_Num_V_Control_Points; ++count) {
		Nd_DblVector	vec;
		double		w;

		vec[0] = *(ptr++);
		vec[1] = *(ptr++);
		vec[2] = *(ptr++);
		if (nurb_info.Nv_Surface_Is_Rational == Nt_ON) {
			w = *(ptr++);
			if (fabs(w) < 1e-4)
				w = 1.0;
		} else
			w = 1.0;

// !! DEVELOPER: ANIMATION CODE CHANGE NEEDED !! 
// 
// If we are outputting animation data then go an embed the inverse pivot 
// matrix of the instance (if it is valid) directly into the NURBS patch CVs. 
// If your export file format actually allows for explicit pivot points & 
// orientations to be assigned/associated with a geometry node then don't
// multiply the inverse pivot matrix into the NURBS patch data here. 
#if 00
		if (embed_pivot_axes_in_vertices && !strcmp(name, "Vertices"))  {
			vec[0] *= w;
			vec[1] *= w;
			vec[2] *= w;
			Ni_DblVector_Transform(1, Nc_TRUE, Nc_TRUE, Nc_TRUE, Nc_TRUE, Nc_TRUE, (Nd_Vector *) vec, pivot_inverse_pivot_matrix, (Nd_Vector *) vec);
			vec[0] /= w;
			vec[1] /= w;
			vec[2] /= w;
		}
#endif

		OPTIONAL_FPRINTF(ofp, "\t\t\t%.5f, %.5f , %.5f , %.5f \n", vec[0], vec[1], vec[2], w);
		OPTIONAL_FPRINTF(ofp, "\n");
	}

	/* Optionally print out the trimming curve info */
	if (export_options->ena_trim_curves)
		NI_Exporter_List_NURBS_Surface_Trim_Curves_Info(ofp, Nv_Master_Object, primitive_query_type, Nv_Primitive_Number);

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

	return(Nc_FALSE);	// No error encountered during export
}

// List all of the 2D uv-space and (optionally the) 3D world-space trim curves 
// associated with a NURBS surface primitive

	static Nd_Void
NI_Exporter_List_NURBS_Surface_Trim_Curves_Info(FILE *ofp, char *Nv_Master_Object,
	Nd_Token primitive_query_type, Nd_Int Nv_Primitive_Number)
{
	Export_IO_NURBPatchTrimCurveParameters	trim_curve_info;
	long		curve_num_offset, knot_offset, trim_loop_num, cv_offset, ws_cv_offset;
	long		worldspace_curve_num_offset, worldspace_knot_offset;
	short		curve_num;

	// Inquire about every trim loop and curve associated with this NURBS surface
	Export_IO_Inquire_NURB_Patch_Trim_Curve_Info(primitive_query_type, Nv_Master_Object, Nv_Primitive_Number, &trim_curve_info);

	// All the trim curve parameters are embedded in multiple linear
	// arrays, so we have to extract these parameters using offsets.
	curve_num_offset = 0;
	knot_offset = 0;
	cv_offset = ws_cv_offset = 0;
	worldspace_curve_num_offset = 0;
	worldspace_knot_offset = 0;

	// Loop over each trim loop
	for (trim_loop_num=0; trim_loop_num < trim_curve_info.Nv_Num_Trim_Loops; ++trim_loop_num) {
		OPTIONAL_FPRINTF(ofp, "\t\tTrim loop # %d:\n", trim_loop_num+1);
		if (trim_curve_info.Nv_TrimLoop_Is_Boundary[trim_loop_num])
			OPTIONAL_FPRINTF(ofp, "\t\t\tTrim loop is a boundary loop\n");

		// Loop over the uv-space curves which comprise this trim loop
		if (trim_curve_info.Nv_Trim_NumCurves) {
			for (curve_num = 0; curve_num < trim_curve_info.Nv_Trim_NumCurves[trim_loop_num]; ++curve_num, ++curve_num_offset) {
				short	num_ctrl_points, order, num_knots, count;
				double	u_min, u_max;

				num_ctrl_points = trim_curve_info.Nv_Trim_Num_Control_Points[curve_num_offset];
				order = trim_curve_info.Nv_Trim_Order[curve_num_offset];
				num_knots = num_ctrl_points + order;

				u_min = trim_curve_info.Nv_Trim_Ranges[2*curve_num_offset];
				u_max = trim_curve_info.Nv_Trim_Ranges[2*curve_num_offset+1];

				// Print out information about this 2D uv-space NURBS trim curve
				OPTIONAL_FPRINTF(ofp, "\t\t\t2D uv-space NURBS Trim Curve, %d of %d\n", curve_num+1, trim_curve_info.Nv_Trim_NumCurves[trim_loop_num]);
				OPTIONAL_FPRINTF(ofp, "\t\t\t\tNumber of control points = %d\n", num_ctrl_points);
				OPTIONAL_FPRINTF(ofp, "\t\t\t\tCurve order = %d\n", order);
				OPTIONAL_FPRINTF(ofp, "\t\t\t\tUrange: umin = %g, umax = %g\n", u_min, u_max);

				if (trim_curve_info.Nv_Trim_Control_Point_Weights)
					OPTIONAL_FPRINTF(ofp, "\t\t\t\tCurve is rational\n");

				OPTIONAL_FPRINTF(ofp, "\t\t\t\tKnots: ");
				for (count = 0; count < num_knots; ++count)
					OPTIONAL_FPRINTF(ofp, "%g ", trim_curve_info.Nv_Trim_Knots[knot_offset++]);
				OPTIONAL_FPRINTF(ofp, "\n");

				// Print the (u,v,w) trim curve CV. CVs are listed in u order.
				OPTIONAL_FPRINTF(ofp, "\t\t\t\tControl points:\n");
				for (count = 0; count < num_ctrl_points; ++count) {
					OPTIONAL_FPRINTF(ofp, "\t\t\t\t");
					OPTIONAL_FPRINTF(ofp, "u = %.5f, ", trim_curve_info.Nv_Trim_Control_Points[cv_offset][0]);
					OPTIONAL_FPRINTF(ofp, "v = %.5f",   trim_curve_info.Nv_Trim_Control_Points[cv_offset][1]);
					// Skip Z, since it has to be 0 for 2D trim curves
					if (trim_curve_info.Nv_Trim_Control_Point_Weights)
						OPTIONAL_FPRINTF(ofp, ", w = %.5f", trim_curve_info.Nv_Trim_Control_Point_Weights[cv_offset]);
					++cv_offset;
					OPTIONAL_FPRINTF(ofp, "\n");
				}
			}
		}

		// Loop over the worldspace-space curves which comprise this trim loop
		if (trim_curve_info.Nv_Worldspace_Trim_NumCurves && trim_curve_info.Nv_UvSpaceToWorldSpaceLiftingApproximationFailed == Nt_OFF && trim_curve_info.Nv_Worldspace_Trim_Num_Control_Points) {
			for (curve_num = 0; curve_num < trim_curve_info.Nv_Worldspace_Trim_NumCurves[trim_loop_num]; ++curve_num, ++worldspace_curve_num_offset) {
				short	num_ctrl_points, order, num_knots, count;
				double	u_min, u_max;

				num_ctrl_points = trim_curve_info.Nv_Worldspace_Trim_Num_Control_Points[worldspace_curve_num_offset];
				order = trim_curve_info.Nv_Worldspace_Trim_Order[worldspace_curve_num_offset];
				num_knots = num_ctrl_points + order;

				u_min = trim_curve_info.Nv_Worldspace_Trim_Ranges[2*worldspace_curve_num_offset];
				u_max = trim_curve_info.Nv_Worldspace_Trim_Ranges[2*worldspace_curve_num_offset+1];

				// Print out information about this 2D uv-space NURBS trim curve
				OPTIONAL_FPRINTF(ofp, "\t\t\t3D World-space NURBS Trim Curve, %d of %d\n", curve_num+1, trim_curve_info.Nv_Worldspace_Trim_NumCurves[trim_loop_num]);
				OPTIONAL_FPRINTF(ofp, "\t\t\t\tNumber of control points = %d\n", num_ctrl_points);
				OPTIONAL_FPRINTF(ofp, "\t\t\t\tCurve order = %d\n", order);
				OPTIONAL_FPRINTF(ofp, "\t\t\t\tUrange: umin = %g, umax = %g\n", u_min, u_max);

				if (trim_curve_info.Nv_Worldspace_Trim_Control_Point_Weights)
					OPTIONAL_FPRINTF(ofp, "\t\t\t\tCurve is rational\n");

				OPTIONAL_FPRINTF(ofp, "\t\t\t\tKnots: ");
				for (count = 0; count < num_knots; ++count)
					OPTIONAL_FPRINTF(ofp, "%g ", trim_curve_info.Nv_Worldspace_Trim_Knots[worldspace_knot_offset++]);
				OPTIONAL_FPRINTF(ofp, "\n");

				// Print the (x,y,z,w) trim curve CV. CVs are listed in u order.
				OPTIONAL_FPRINTF(ofp, "\t\t\t\tControl points:\n");
				for (count = 0; count < num_ctrl_points; ++count) {
					OPTIONAL_FPRINTF(ofp, "\t\t\t\t");
					OPTIONAL_FPRINTF(ofp, "x = %.5f, ", trim_curve_info.Nv_Worldspace_Trim_Control_Points[ws_cv_offset][0]);
					OPTIONAL_FPRINTF(ofp, "y = %.5f, ", trim_curve_info.Nv_Worldspace_Trim_Control_Points[ws_cv_offset][1]);
					OPTIONAL_FPRINTF(ofp, "z = %.5f",   trim_curve_info.Nv_Worldspace_Trim_Control_Points[ws_cv_offset][2]);
					if (trim_curve_info.Nv_Worldspace_Trim_Control_Point_Weights)
						OPTIONAL_FPRINTF(ofp, ", w = %.5f", trim_curve_info.Nv_Worldspace_Trim_Control_Point_Weights[ws_cv_offset]);
					++ws_cv_offset;
					OPTIONAL_FPRINTF(ofp, "\n");
				}
			}
		}
	}

	OPTIONAL_FPRINTF(ofp, "\t\tGeneral patch/trimming parameters:\n");
	// If Nt_ON then the there is no outer boundary trim curve. Instead, the
	// outer boundary trim curve is the entire boundary of the NURBS surface
	OPTIONAL_FPRINTF(ofp, "\t\t\tAuto-trim boundary is '%s'\n", trim_curve_info.Nv_Auto_Boundary);
	// Nt_ON if the knots of the surface have already been cleansed
	OPTIONAL_FPRINTF(ofp, "\t\t\tSurface is clean is '%s'\n", trim_curve_info.Nv_Surface_Is_Clean);
	// Nt_ON if the knots of the trim curves have already been cleansed
	OPTIONAL_FPRINTF(ofp, "\t\t\tCurves are clean is '%s'\n", trim_curve_info.Nv_Curves_Are_Clean);

	// Call this once the trim curve info is no longer needed
	Export_IO_Dispose_NURB_Patch_Trim_Curve_Info(&trim_curve_info);
}
