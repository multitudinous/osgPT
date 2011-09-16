/*****************************************************************************

	----------------------------------------------------------------
           NURBS Curve Geometric Entity Query Functions for PolyTrans
	----------------------------------------------------------------
	This module queries all the parameters of a NURBS Curve entity.
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

/* -------------------->>>>  NURBS Curve Output  <<<<----------------------- */

// This outputs a NURBS curve object. For the sake of how PolyTrans works
// we will assume that the object definition only contains a single NURBS
// curve primitive (assume 'Nv_Primitive_Number' is 0). Each NURBS curve 
// primitive can have either a single NURBS curve defined in it, or a series 
// of NURBS curve segments as a composite NURBS curve. The default material 
// for the NURBS curve will be 'Nv_Inherited_Surface_Name'.

// NOTE: The NURBS Curve primitive has no correlation to a trimmed NURBS surface.
// This NURBS curve primitive defines one NURBS curve in 3D space, as a single
// curve or as a composite. The trimmed NURBS surface primitive has 1 NURBS patch 
// with zero or more NURBS trim curves in the 2D uv-space of the NURBS surface.

	Nd_Void
NI_Exporter_List_NURBS_Curve_Primitives(
	FILE 			*ofp,
	Nd_Walk_Tree_Info 	*Nv_Walk_Tree_Info_Ptr,
	char 			*Nv_Master_Object,
	Nd_Int 			Nv_Primitive_Number,
	char 			*Nv_Inherited_Surface_Name,
	char 			*instance_name)
{
	Nd_Int		i, curve_num, temp_num_curves, Nv_Return_Status;
	Nd_Token	primitive_type;
	Nd_Token	Nv_NURB_Show_CVs_In_UI, Nv_NURB_Show_Control_Polygon_In_UI;
	Nd_Int		Nv_NurbCurveNum, Nv_NumNURBSCurves;
	Nd_Token 	Nv_NURBS_Curves_Are_Renderable;
	Nd_Token 	Nv_NURBS_Curve_Closed;
	Nd_Token	Nv_NURB_Consider_Curves_As_One_Loop;
	Nd_Float 	Nv_Adaptive_Curve_Deviation;
	Nd_Token 	Nv_NURBS_Curve_SubDiv_Type;
	Nd_Int 		Nv_Uniform_Subdiv;
	Nd_Int 		Nv_Num_Curve_Control_Points;
	Nd_Int 		Nv_Curve_Order;
	Nd_Double 	*Nv_Curve_Knots;
	Nd_Double 	Nv_Curve_Ranges[2];
	Nd_Double 	*Nv_Curve_CVs;
	Nd_Token	Nv_Curve_Is_Rational;
	Nd_Token	primitive_query_type;
	Nd_Token        Nv_NURBS_Composite_Curve_Renderable;
	Nd_Token        Nv_Consider_Curves_As_One_Loop;
	Nd_Double	Nv_CompositeCurveTolerance;
	char 		*Nv_CV_Flags_Array, *selected;
	short		animation_data_is_available = Nc_FALSE;
	Nd_Matrix 	pivot_inverse_pivot_matrix;
	Nd_Matrix 	normal_transform_matrix;
	short		embed_pivot_axes_in_vertices = Nc_FALSE;

	// Let's see if we've output this object definition before. If so, create a new instance of the existing object
	if (NI_Handle_Okino_Object_Instancing(Nv_Walk_Tree_Info_Ptr))
		return;	// Instance was created. Return no error

	// If the caller is telling us that the primitive contains a 'Spline Shape' primitive,
	// but a 'NURBS curve' primitive is wanted instead, then feed the 'Nt_TEMPNURBSCURVE'
	// token to the primitive inquiry function. This will internally (inside the toolkit)
	// convert the spline shape to a temporary NURBS curve primitive which can be 
	// accessed by the normal NURBS curve toolkit query functions,
	// except that the main 'Nt_NURBCURVE' parameter is replaced by 'Nt_TEMPNURBSCURVE'.
	Ni_Inquire_Primitive(Nt_OBJECT, Nv_Master_Object, Nv_Primitive_Number, &primitive_type, Nt_CMDEND);	// Get the type of this primitive
	if (primitive_type == Nt_SPLINESHAPE)
		// Oops, the caller of this function is handing us a Spline Shape 
		// primitive but we want to export this as a NURBS Curve instead.
		// We'll query via the Nt_TEMPNURBSCURVE so that a temporary NURBS 
		// Curve primitive is created instead.
		primitive_query_type = Nt_TEMPNURBSCURVE;
	else if (primitive_type == Nt_INDEXEDPOLYLINES)
		// Oops, the caller of this function is handing us an indexed polyline 
		// primitive but we want to export this as a NURBS Curve instead.
		// We'll query via the Nt_TEMPNURBSCURVE so that a temporary NURBS 
		// Curve primitive is created instead.
		primitive_query_type = Nt_TEMPNURBSCURVE;
	else
		// Ok, this is a pure NURBS curve so let's output it.
		primitive_query_type = Nt_NURBCURVE;

	// Find out how many sub-NURBS curves are associated with the Nth
	// primitive of this object definition (usually Nv_Primitive_Number never goes about 0)
	Nv_NurbCurveNum = 0;
	Ni_Inquire_Primitive(Nt_OBJECT, Nv_Master_Object, Nv_Primitive_Number, &primitive_type, 
		/* 'Nv_NurbCurveNum' specifies which NURBS curve will be queried (0 if you don't know how many curves there are) */
		primitive_query_type, (Nd_Int *) &Nv_NurbCurveNum,
			/* Return the number of NURBS curves in this composite curve */
			Nt_NUMCURVES, (Nd_Int *) &Nv_NumNURBSCurves, Nt_CMDSEP,
			/* The composite NURBS curve shape will be rendered as a 3D object if the NURBS curves form a closed loop (first and last CV are the same) */
			Nt_RENDERABLE, Nt_ENABLED, (Nd_Token *) &Nv_NURBS_Composite_Curve_Renderable, Nt_CMDSEP,
			/* See explanation above */
			Nt_CONSIDERCURVESASONELOOP, Nt_ENABLED, (Nd_Token *) &Nv_Consider_Curves_As_One_Loop, Nt_CMDSEP,
		Nt_CMDEND);

	// Let's see if the user wants to output a composite NURBS curve as a single NURBS curve
	// (but only if the multiple curves of this object are considered to create a single
	// composite curve and not multiple closed loops)
	if (Nv_Consider_Curves_As_One_Loop == Nt_ON && Nv_NURBS_Composite_Curve_Renderable == Nt_OFF && export_options->NonRenderableNURBSCurveOutputMode == Nc_NON_RENDERABLE_NURBS_CURVE_OUTPUT_MODE__COMPOSITE_TO_SINGLE_CURVE) {
		// Output the composite NURBS curve object as a single NURBS curve object

		// First, go copy the NURBS curve object to the temporary NURBS
		// curve object which hangs off the primitive node. We need to do 
		// this since we will be changing the composite curve to a single
		// curve and we don't want to destroy the original version.
		Ni_Object(Nv_Master_Object,
			Nt_COPYNURBSCURVETOTEMPSTRUCT, Nt_ALL, 
			Nt_CMDEND);

		// Let's make sure that the new temp. NURBS curve has been copied ok
		Ni_Inquire_Primitive(Nt_OBJECT, Nv_Master_Object, Nv_Primitive_Number, &primitive_type,
			Nt_TEMPNURBSCURVE, (Nd_Int *) &Nv_NurbCurveNum,
				Nt_NUMCURVES, (Nd_Int *) &temp_num_curves, Nt_CMDSEP,
				Nt_CMDEND);
		if (!temp_num_curves)
			// Oops, the conversion failed so exit early
			return;

		// Go convert any composite NURBS curves into single NURBS curves
		Nv_CompositeCurveTolerance = 5e-6;
		Ni_Object(Nv_Master_Object,
			Nt_CONVERTTEMPTOSINGLECURVE,
				(Nd_Double *) 	&Nv_CompositeCurveTolerance,
				(Nd_Int *) 	&Nv_Return_Status,
			Nt_CMDEND);

		// Let's make sure that the new temp. NURBS curve is ok
		Ni_Inquire_Primitive(Nt_OBJECT, Nv_Master_Object, Nv_Primitive_Number, &primitive_type,
			Nt_TEMPNURBSCURVE, (Nd_Int *) &Nv_NurbCurveNum,
				Nt_NUMCURVES, (Nd_Int *) &temp_num_curves, Nt_CMDSEP,
				Nt_CMDEND);
		if (!temp_num_curves)
			// Oops, the conversion failed so exit early
			return;

		// Use the temporary NURBS curve for output
		primitive_query_type = Nt_TEMPNURBSCURVE;
	}

	// Sanity check: no curves associated with this primitive (this should never happen)
	if (!Nv_NumNURBSCurves)
		return;

	/* Inquire about the common NURBS curve options */
	Ni_Inquire_Primitive(Nt_OBJECT, Nv_Master_Object, 0, &primitive_type, 
		/* 'Nv_NurbCurveNum' specifies which NURBS curve will be queried (0 if you don't know how many curves there are) */
		primitive_query_type, (Nd_Int *) &Nv_NurbCurveNum,
			/* Return the number of NURBS curves in this composite curve */
			Nt_NUMCURVES, (Nd_Int *) &Nv_NumNURBSCurves, Nt_CMDSEP,

			/* The composite NURBS curve's shape will be rendered as a 3D object if the NURBS curves form a closed loop */
			Nt_RENDERABLE, Nt_ENABLED, (Nd_Token *) &Nv_NURBS_Curves_Are_Renderable, Nt_CMDSEP,

			/* If set to Nt_ON then if there are multiple curves defined they will be considered */
			/* as a single composite NURBS curve (each curve touches back-to-back and the last CV */
			/* optionally touches the first CV if it is to be considered closed). */
			/* If set to Nt_OFF, and there are multiple curves defined, then the curves */
			/* are considered distinct (none touch each other). Also if each sub-curve is closed */
			/* (its last CV touches its first CV), the Nt_RENDERABLE option is set to Nt_ON, */
			/* the first curve is the boundary and is oriented counter-clockwise, and all */
			/* subsquent sub-curves are oriented clockwise, then the curves will be rendered */
			/* as a trimmed NURBS surface. This latter mode is used, for example, by the */
			/* SoftImage importer to import FACE entities (letter 'B') for which multiple */
			/* NURBS curves define a trimmed NURBS surface. */
			Nt_CONSIDERCURVESASONELOOP, Nt_ENABLED, &Nv_NURB_Consider_Curves_As_One_Loop, Nt_CMDSEP,

			// Okino flag: show CVs in the user-interface
			Nt_SHOWCVSINUI, Nt_ENABLED, (Nd_Token *) &Nv_NURB_Show_CVs_In_UI, Nt_CMDSEP,

			// Okino flag: show control polygon in user-interface
			Nt_SHOWCTRLPOLYGONINUI, Nt_ENABLED, (Nd_Token *) &Nv_NURB_Show_Control_Polygon_In_UI, Nt_CMDSEP,

			/* Nt_ON returned if that displayable start and end of the NURBS curve are at the same location in space */
			Nt_CLOSED, Nt_ENABLED, (Nd_Token *) &Nv_NURBS_Curve_Closed, Nt_CMDSEP,

			/* Maximum deviation between the tesselated curve and the real curve. */
			/* Used only for adaptive curve tessellation within the PolyTrans user interface. */
			Nt_CURVEDEV, (Nd_Float *) &Nv_Adaptive_Curve_Deviation, Nt_CMDSEP,

			/* Curve subdivision type: Nt_DISABLED = use global type, Nt_UNIFORM = uniform subdivision, Nt_ADAPTIVE = adaptive subdivision */
			Nt_SUBDIVTYPE, (Nd_Token *) &Nv_NURBS_Curve_SubDiv_Type, Nt_CMDSEP,

			/* Not used by PolyTrans: Uniform subdivision value. 1 or greater. */
			Nt_SUBDIV, (Nd_Int *) &Nv_Uniform_Subdiv, Nt_CMDSEP,
		Nt_CMDEND);

	OPTIONAL_FPRINTF(ofp, "\n\t'NURBS Curve' primitive defined in master object definition '%s':\n", Nv_Master_Object);

	if (Nv_NumNURBSCurves > 1)
		OPTIONAL_FPRINTF(ofp, "\t\tConsists of %d composite NURBS curves.\n", Nv_NumNURBSCurves);

	if (Nv_NURBS_Curve_Closed == Nt_ON)
		OPTIONAL_FPRINTF(ofp, "\t\tSpline is closed (first and last CVs are the same).\n");

	if (Nv_NumNURBSCurves > 1) {
		if (Nv_NURB_Consider_Curves_As_One_Loop == Nt_ON)
			OPTIONAL_FPRINTF(ofp, "\t\tThe multiple NURBS curves are considered as one composite curve.\n");
		else
			OPTIONAL_FPRINTF(ofp, "\t\tThe multiple NURBS curves are considered as distinct curves (not as a single composite curve).\n");
	}

	// If Nt_ON then the closed NURBS curved will be rendered as a planar 
	// object which has the boundary of the first NURBS curve. If there 
	// are multiple curves and the 'Nt_CONSIDERCURVESASONELOOP' option is
	// enabled then the second and subsequent curves will be considered as
	// trimming holes inside the first boundary curve.
	if (Nv_NURBS_Curves_Are_Renderable == Nt_ON)
		OPTIONAL_FPRINTF(ofp, "\t\tThe planar NURBS curve object is renderable when closed.\n");

	if (Nv_NURB_Show_CVs_In_UI == Nt_ON)
		OPTIONAL_FPRINTF(ofp, "\t\tNURBS curve CVs are shown in the GUI.\n");

	if (Nv_NURB_Show_Control_Polygon_In_UI == Nt_ON)
		OPTIONAL_FPRINTF(ofp, "\t\tNURBS curve control polygon is shown in the GUI.\n");

	OPTIONAL_FPRINTF(ofp, "\t\tSubdivision type = %s", Nv_NURBS_Curve_SubDiv_Type);
	if (Nv_NURBS_Curve_SubDiv_Type == Nt_ADAPTIVE)
		OPTIONAL_FPRINTF(ofp, ". Curve deviation = %g.\n", Nv_Adaptive_Curve_Deviation);
	else if (Nv_NURBS_Curve_SubDiv_Type == Nt_UNIFORM)
		OPTIONAL_FPRINTF(ofp, ". Curve subdivisions = %d.\n", Nv_Uniform_Subdiv);
	else
		OPTIONAL_FPRINTF(ofp, ". Parameters taken from global subdivision method.\n");

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

	/* Loop over all curves of this composite curve */
	for (curve_num = 0; curve_num < Nv_NumNURBSCurves; ++curve_num) {
		Ni_Inquire_Primitive(Nt_OBJECT, Nv_Master_Object, 0, &primitive_type, 
			/* 'Nv_NurbCurveNum' specifies which NURBS curve will be queried (0 if you don't know how many curves there are) */
			primitive_query_type, (Nd_Int *) &curve_num,

			/* Number of control points in the NURBS curve */
			Nt_NUMCTRLPNTS, (Nd_Int *) &Nv_Num_Curve_Control_Points, Nt_CMDSEP,

			/* Order of the NURBS curve (degree+1) */
			Nt_ORDER, (Nd_Int *) &Nv_Curve_Order, Nt_CMDSEP,

			// Upon return 'Nv_Curve_Knots' will point at the internal curve knot array (read only)
			Nt_DBLKNOTS, (Nd_Double *) &Nv_Curve_Knots, Nt_CMDSEP,

			// Input: 'Nv_Curve_Ranges' must point to an array of 2 doubles.
			// Output: These two doubles will be returned with the curve's min/max u values.
			Nt_DBLRANGE, (Nd_Double *) &Nv_Curve_Ranges, Nt_CMDSEP,

			/* The 'Nt_DBLVERTICES' includes the weight in the 4th component if 'rational' is Nt_ON */
			/* 	The 'Nt_DBLWEIGHTS' are not used when retrieving the control points. */
			// Upon return 'Nv_Curve_CVs' will point at the internal curve CV array (read only)
			Nt_DBLVERTICES, (Nd_Double **) &Nv_Curve_CVs, Nt_CMDSEP,

			/* The 'Nt_RATIONAL' flag is used to determine if the 'Nt_DBLVERTICES' array has a 'w' component */
			Nt_RATIONAL, Nt_ENABLED, &Nv_Curve_Is_Rational, Nt_CMDSEP,

			/* Optional: An array which specifies flags for each CV, */
			/* such as if the CV is selected in the UI, or if the CV's position */
			/* is locked in the UI (see 'Nc_NURBCURVE_CV_FLAG_...' definitions in ni3.h) */
			/* Length of the array is the number of control points. */
			// Upon return 'Nv_CV_Flags_Array' will point at the internal CV flags array (read only)
			Nt_FLAGS, (char **) &Nv_CV_Flags_Array, Nt_CMDSEP,	/* 8-bit */
		Nt_CMDEND);

		OPTIONAL_FPRINTF(ofp, "\t\tCurve # %d\n", curve_num+1);

		OPTIONAL_FPRINTF(ofp, "\t\t\tNumber of control points = %ld\n", Nv_Num_Curve_Control_Points);
		OPTIONAL_FPRINTF(ofp, "\t\t\tCurve order = %d\n", Nv_Curve_Order);
		OPTIONAL_FPRINTF(ofp, "\t\t\tU min = %g, U max = %g\n", Nv_Curve_Ranges[0], Nv_Curve_Ranges[1]);

		OPTIONAL_FPRINTF(ofp, "\n\t\t\tControl vertices:\n");
		for (i=0; i < Nv_Num_Curve_Control_Points; ++i) {
			Nd_DblVector 	vec;
			char		*locked;
			double		w;

			// Update the stats
        	        ++export_options->total_nurbs_curves;

			if (Nv_CV_Flags_Array && (Nv_CV_Flags_Array[i] & Nc_NURBCURVE_CV_FLAG_SELECTED))
				selected = " Selected in UI.";
			else
				selected = "";

			if (Nv_CV_Flags_Array && (Nv_CV_Flags_Array[i] & Nc_NURBCURVE_CV_FLAG_LOCKED))
				locked = " Locked in UI.";
			else
				locked = "";

			vec[0] = *(Nv_Curve_CVs++);
			vec[1] = *(Nv_Curve_CVs++);	
			vec[2] = *(Nv_Curve_CVs++);
			if (Nv_Curve_Is_Rational == Nt_ON) {
				w = *(Nv_Curve_CVs++);
				if (fabs(w) < 1e-4)
					w = 1.0;
			} else
				w = 1.0;

// !! DEVELOPER: ANIMATION CODE CHANGE NEEDED !! 
// 
// If we are outputting animation data then go an embed the inverse pivot 
// matrix of the instance (if it is valid) directly into the curve CVs. 
// If your export file format actually allows for explicit pivot points & 
// orientations to be assigned/associated with a geometry node then don't
// multiply the inverse pivot matrix into the curve data here. 
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

			if (Nv_Curve_Is_Rational == Nt_ON)
				OPTIONAL_FPRINTF(ofp, "\t\t\t%ld: (%g, %g, %g), w = %g.%s%s\n", i+1, vec[0], vec[1], vec[2], w, selected, locked);
			else
				OPTIONAL_FPRINTF(ofp, "\t\t\t%ld: (%g, %g, %g).%s%s\n", i+1, vec[0], vec[1], vec[2], selected, locked);
		}

		OPTIONAL_FPRINTF(ofp, "\n\t\t\tClamped knot vector:\n");
		for (i=0; i < Nv_Num_Curve_Control_Points+Nv_Curve_Order; ++i)
			OPTIONAL_FPRINTF(ofp, "\t\t\t%ld: %g\n", i, *(Nv_Curve_Knots++));
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
