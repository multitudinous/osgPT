/*****************************************************************************

	----------------------------------------------------------------
          2D Spline Shape Geometry Entity Query Functions for PolyTrans
	----------------------------------------------------------------
	This module queries all the parameters of a Spline Shape entity.
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

	Last change:  RCL  20 May 2001    4:45 am
*****************************************************************************/

#include "main.h"	// Main include file for a PolyTrans exporter

/* ----------------------->>>>  Definitions  <<<<--------------------------- */

/* -------------------->>>>  Local Variables  <<<<-------------------------- */

/* ------------------>>>>  Function Prototypes  <<<<------------------------ */

/* ----------->>>>  Spline Shape Geometry Entity Output  <<<<--------------- */

// This routine queries all the data associated with a spline shape geometric
// entity. The shape is defined by 1 or more spline curves. Each curve is 
// disjoint from the others and each curve is made up of only 1 physical
// spline segment of type: cardinal, bezier, b-spline, tensioned or tcb 
// (tcb is a hermite spline). The default material for the spline shape will
// be 'Nv_Inherited_Surface_Name'.

// For example, the letter "B" would be 1 outer spline and two inner splines. 

// For the sake of how PolyTrans works, we will assume that the object
// definition only has a single Spline Shape primitive (Nv_Primitive_Number=0)

	Nd_Void
NI_Exporter_List_Spline_Shape_Primitive(
	FILE 			*ofp,
	Nd_Walk_Tree_Info 	*Nv_Walk_Tree_Info_Ptr,
	char 			*Nv_Master_Object,
	Nd_Int 			Nv_Primitive_Number,
	char 			*Nv_Inherited_Surface_Name,
	char 			*instance_name)
{
	Nd_Token	Nv_Spline_Renderable;
	Nd_Token	Nv_Spline_Closed;
	Nd_Token	Nv_Spline_SubDiv_Type;
	Nd_Float	Nv_Adaptive_Curve_Deviation;
	Nd_Int		Nv_Uniform_Subdiv;
	Nd_Token	Nv_Spline_Curve_Type;
	Nd_Float	Nv_Spline_Tension;
	Nd_Int		Nv_NumControlPoints_InSplineCurve;
	Nd_Int		Nv_SplineCurveNum, Nv_Num_SplineCurves_In_SplineShape;
	Nd_Token	Nv_ControlPoint_Selected;
	Nd_Token	Nv_ControlPoint_ValueLocked;
	Nd_Token	Nv_ControlPoint_TangentLocked;
	Nd_Vector	Nv_ControlPoint;
	Nd_Float	Nv_TCB_Tension, Nv_TCB_Continuity, Nv_TCB_Bias;
	Nd_Token	Nv_ControlPoint_TangentInType;
	Nd_Token	Nv_ControlPoint_TangentOutType;
	Nd_Vector	Nv_Tangent_In, Nv_Tangent_Out;
	Nd_Token	primitive_type;
	long		curve_num, control_pnt_num;
	Nd_Token	primitive_query_type;
	short		animation_data_is_available = Nc_FALSE;
	Nd_Matrix 	pivot_inverse_pivot_matrix;
	Nd_Matrix 	normal_transform_matrix;
	short		embed_pivot_axes_in_vertices = Nc_FALSE;

	// Let's see if we've output this object definition before. If so, create a new instance of the existing object
	if (NI_Handle_Okino_Object_Instancing(Nv_Walk_Tree_Info_Ptr))
		return;	// Instance was created. Return no error

	// If the caller is telling us that the primitive contains a 'NURBS Curve' primitive
	// but a 'Spline Shape' primitive is wanted instead, then feed the 'Nt_TEMPSPLINESHAPE'
	// token to the primitive inquiry function. This will internally (inside the toolkit)
	// convert the NURBS curve primitive to a temporary spline shape primitive which can be 
	// accessed by the normal Spline Shape toolkit query functions,
	// except that the main 'Nt_SPLINESHAPE' parameter is replaced by 'Nt_TEMPSPLINESHAPE'.
	Ni_Inquire_Primitive(Nt_OBJECT, Nv_Master_Object, Nv_Primitive_Number, &primitive_type, Nt_CMDEND);	// Get the type of this primitive
	if (primitive_type == Nt_NURBCURVE)
		// Oops, the caller of this function is handing us a NURBS Curve
		// primitive but we want to export this as a Spline Shape instead.
		// We'll query via the Nt_TEMPSPLINESHAPE so that a temporary spline 
		// shape primitive is created instead.
		primitive_query_type = Nt_TEMPSPLINESHAPE;
	else
		// Ok, this is a pure Spline Shape so let's output it.
		primitive_query_type = Nt_SPLINESHAPE;

	// Fill the 'Nv_Num_SplineCurves_In_SplineShape' with how many curves there are in this spline shape.
	// Query the Nth primitive of this object definition (Nth = Nv_Primitive_Number+1).
	// Normally 'Nv_Primitive_Number' never usually goes beyond 0.
	Nv_SplineCurveNum = 0;	// Which curve to query inside this single spline shape primitive.
	Ni_Inquire_Primitive(Nt_OBJECT, Nv_Master_Object, Nv_Primitive_Number, &primitive_type, 
		primitive_query_type, 
			/* This is ignored during the first call */
			(Nd_Int *) &Nv_SplineCurveNum,	
		/* Out argument: The number of curves which makes up this spline is returned in this optional argument */
		Nt_NUMCURVES, (Nd_Int *) &Nv_Num_SplineCurves_In_SplineShape,
		Nt_CMDEND);

	// If the desired spline output type is different from that currently used by the spline
	// shape then go and create a temporary spline shape which uses the new spline type
	if (primitive_query_type == Nt_SPLINESHAPE && (export_options->spline_to_spline_conversion_type != NULL && strlen(export_options->spline_to_spline_conversion_type))) {
		Nd_Int	Nv_Return_Status;
		short	curves_need_changing = Nc_FALSE;

		// Let's see if any spline curve of the spline shape uses a different spline type
		for (int Nv_SplineCurveNum=0; Nv_SplineCurveNum < Nv_Num_SplineCurves_In_SplineShape; ++Nv_SplineCurveNum) {
			Ni_Inquire_Primitive(Nt_OBJECT, Nv_Master_Object, Nv_Primitive_Number, &primitive_type,
				/* 'Nv_SplineCurveNum' specifies which spline curve will be queried */
				Nt_SPLINESHAPE, (Nd_Int *) &Nv_SplineCurveNum,
				/* The following parameters relate to the single curve */
				Nt_TYPE, (Nd_Token *) &Nv_Spline_Curve_Type, Nt_CMDSEP,
				Nt_CMDEND);

			if (Nv_Spline_Curve_Type != export_options->spline_to_spline_conversion_type) {
				curves_need_changing = Nc_TRUE;
				break;
			}
		}

		// Go and change the spline type for all spline curves of this specific primitive
		if (curves_need_changing) {
			Ni_Object(Nv_Master_Object,
				Nt_CHANGESPLINESHAPEBASIS,
				Nt_PRIMITIVE, (Nd_Int *) &Nv_Primitive_Number,		// Change every primitive of the object
				export_options->spline_to_spline_conversion_type,	// The new spline curve type
				(Nd_Int *) &Nv_Return_Status,				// Return Nc_TRUE if the spline type could not be changed.
				Nt_CMDEND);
		}
	}


	/* Ignore if there are no curves in this spline shape (this should never happen) */
	if (!Nv_Num_SplineCurves_In_SplineShape)
		return;

	/* Get the shared parameters of all the spline curves for the spline shape */
	Ni_Inquire_Primitive(Nt_OBJECT, Nv_Master_Object, 0, &primitive_type, 
		primitive_query_type, (Nd_Int *) &Nv_SplineCurveNum,	/* In argument: 'Nv_SplineCurveNum' specifies which spline curve will be queried (0 if you don't know how many sub-curves there are) */
		
		/* --- Everything after this point is optional --- */
		/* --- Make sure 'Nv_Num_SplineCurves_In_SplineShape' > 0 before sending in the following options --- */

		/* This spline will be rendered if it is closed */
		Nt_RENDERABLE, Nt_ENABLED, (Nd_Token *) &Nv_Spline_Renderable, Nt_CMDSEP,

		/* Curve subdivision type: Nt_DISABLED = use global type, Nt_UNIFORM = uniform subdivision, Nt_ADAPTIVE = adaptive subdivision */
		Nt_SUBDIVTYPE, (Nd_Token *) &Nv_Spline_SubDiv_Type, Nt_CMDSEP,
		/* Maximum deviation between the tesselated curve and the real curve */
		Nt_CURVEDEV, (Nd_Float *) &Nv_Adaptive_Curve_Deviation, Nt_CMDSEP,
		/* Uniform subdivision value */
		Nt_USUBDIV, (Nd_Int *) &Nv_Uniform_Subdiv, Nt_CMDSEP,
		Nt_CMDEND);

	// Go and see if there is any animation channel data associated
	// with this geometry instance that we are interested about. If resampled
	// keyframe mode is in effect then temporary resampled keyframe lists
	// will have been created and stored in the core toolkit after this 
	// function finishes and returns here. 
	if (export_options->ena_object_animation && export_options->ena_hierarchy) {
		animation_data_is_available = NI_Exporter_QueryAndSetup_Object_Animation_Keyframe_Data(ofp, instance_name);

		// If we are outputting animation data, then let's go examine the 
		// pivot point matrix associated with this instance. If it is valid
		// then we'll really need to multiple the inverse pivot point into
		// the vertices and normals (see the 'anim.cpp' file for full docs
		// about pivot points and why you need to do this).
		embed_pivot_axes_in_vertices = NI_Exporter_GetValidPivotPointMatricesForAnimationExport(instance_name, pivot_inverse_pivot_matrix, normal_transform_matrix);
	}

       	// Update the stats
       	++export_options->total_spline_shapes;

	OPTIONAL_FPRINTF(ofp, "\n\t'Spline Shape' primitive defined in master object definition '%s':\n", Nv_Master_Object);

	OPTIONAL_FPRINTF(ofp, "\t\tConsists of %d spline curve(s).\n", Nv_Num_SplineCurves_In_SplineShape);

	if (Nv_Spline_Renderable == Nt_ON)
		OPTIONAL_FPRINTF(ofp, "\t\tSpline shape is renderable when closed.\n");

	OPTIONAL_FPRINTF(ofp, "\t\tSubdivision type = %s", Nv_Spline_SubDiv_Type);
	if (Nv_Spline_SubDiv_Type == Nt_ADAPTIVE)
		OPTIONAL_FPRINTF(ofp, ". Curve deviation = %g.\n", Nv_Adaptive_Curve_Deviation);
	else if (Nv_Spline_SubDiv_Type == Nt_UNIFORM)
		OPTIONAL_FPRINTF(ofp, ". Curve subdivisions = %d.\n", Nv_Uniform_Subdiv);
	else
		OPTIONAL_FPRINTF(ofp, ". Parameters taken from global subdivision method.\n");

	/* Now loop over all of the spline curves of the spline shape */
	for (curve_num = 0; curve_num < Nv_Num_SplineCurves_In_SplineShape; ++curve_num) {
		Ni_Inquire_Primitive(Nt_OBJECT, Nv_Master_Object, 0, &primitive_type, 
			primitive_query_type, (Nd_Int *) &curve_num,	/* This selects the spline curve to inquire about */

			/* The following parameters relate to the single curve */
			Nt_TYPE, (Nd_Token *) &Nv_Spline_Curve_Type, Nt_CMDSEP,
			/* For Nt_CARDINAL and Nt_TENSION */
			Nt_UTENSION, (Nd_Float *) &Nv_Spline_Tension, Nt_CMDSEP,
			/* Make the spline curve open/closed */
			Nt_CLOSED, Nt_ENABLED, (Nd_Token *) &Nv_Spline_Closed, Nt_CMDSEP,

			/* The number of control points is the curve specified by the 'Nv_SplineCurveNum' parameter above. */
			Nt_NUMCTRLPNTS, (Nd_Int *) &Nv_NumControlPoints_InSplineCurve,
			Nt_CMDEND);

		OPTIONAL_FPRINTF(ofp, "\t\tSpline curve # %d:\n", curve_num+1);

		OPTIONAL_FPRINTF(ofp, "\t\t\tSpline type = %s\n", Nv_Spline_Curve_Type);
		if (Nv_Spline_Curve_Type == Nt_CARDINAL || Nv_Spline_Curve_Type == Nt_TENSION)
			OPTIONAL_FPRINTF(ofp, "\t\t\tSpline tension = %g\n", Nv_Spline_Tension);
		if (Nv_Spline_Closed == Nt_ON)
			OPTIONAL_FPRINTF(ofp, "\t\t\tSpline is closed.\n");
		OPTIONAL_FPRINTF(ofp, "\t\t\tNumber of control points = %d\n", Nv_NumControlPoints_InSplineCurve);

		/* Now loop over all of the control points of this spline curve */
		for (control_pnt_num = 0; control_pnt_num < Nv_NumControlPoints_InSplineCurve; ++control_pnt_num) {
			Ni_Inquire_Primitive(Nt_OBJECT, Nv_Master_Object, 0, &primitive_type, 
				primitive_query_type, (Nd_Int *) &curve_num,	/* In argument: 'Nv_SplineCurveNum' specifies which spline curve will be queried (0 if you don't know how many curves there are) */

				/* 'control_pnt_num' specifies which control point will be queried */
				Nt_CONTROLPOINTPARAMS, (Nd_Int *) &control_pnt_num,
					/* TRUE if this control point is selected in the user interface */
					Nt_SELECTED, Nt_ENABLED, (Nd_Token *) &Nv_ControlPoint_Selected, 

					/* TRUE if the control point value is locked */
					Nt_VALUELOCKED, Nt_ENABLED, (Nd_Token *) &Nv_ControlPoint_ValueLocked,

					/* TRUE if the tangent values are locked */
					Nt_TANGENT, Nt_LOCKED, Nt_ENABLED, (Nd_Token *) &Nv_ControlPoint_TangentLocked,

					/* The 2D or 3D control point */
					Nt_VALUE, (Nd_Vector *) Nv_ControlPoint,

					/* Tension/continuity/bias values for TCB curves */
					Nt_TCB, Nt_TENSION, (Nd_Float *) &Nv_TCB_Tension,
					Nt_TCB, Nt_CONT, (Nd_Float *) &Nv_TCB_Continuity,
					Nt_TCB, Nt_BIAS, (Nd_Float *) &Nv_TCB_Bias,

					/* Default tangent types for 'Nt_BEZIER' spline type */
					Nt_TANGENT, Nt_INTYPE, (Nd_Token *) &Nv_ControlPoint_TangentInType,
					Nt_TANGENT, Nt_OUTTYPE, (Nd_Token *) &Nv_ControlPoint_TangentOutType,

					/* Bezier tangent values and parameters */
					/* NOTE: These tangent values can only be specified in Nt_CUSTOM tangent mode */
					Nt_TANGENT, Nt_INVALUE, (Nd_Vector *) Nv_Tangent_In,
					Nt_TANGENT, Nt_OUTVALUE, (Nd_Vector *) Nv_Tangent_Out,
				Nt_CMDSEP,
			Nt_CMDEND);

			OPTIONAL_FPRINTF(ofp, "\t\t\tControl point # %d of %d:\n", control_pnt_num+1, Nv_NumControlPoints_InSplineCurve);

			if (Nv_ControlPoint_Selected == Nt_ON)
				OPTIONAL_FPRINTF(ofp, "\t\t\t\tControl point is selected.\n");
			if (Nv_ControlPoint_ValueLocked == Nt_ON)
				OPTIONAL_FPRINTF(ofp, "\t\t\t\tValue is locked.\n");
			if (Nv_ControlPoint_TangentLocked == Nt_ON)
				OPTIONAL_FPRINTF(ofp, "\t\t\t\tTangent is locked.\n");

// !! DEVELOPER: ANIMATION CODE CHANGE NEEDED !! 
// 
// If we are outputting animation data then go an embed the inverse pivot 
// matrix of the instance (if it is valid) directly into the spline mesh CVs. 
// If your export file format actually allows for explicit pivot points & 
// orientations to be assigned/associated with a geometry node then don't
// multiply the inverse pivot matrix into the spline mesh data here. 
#if 00
			if (embed_pivot_axes_in_vertices && !strcmp(name, "Vertices")) 
				Ni_Vector_Transform(1, Nc_TRUE, Nc_TRUE, Nc_TRUE, Nc_TRUE, Nc_TRUE, (Nd_Vector *) Nv_ControlPoint, pivot_inverse_pivot_matrix, (Nd_Vector *) Nv_ControlPoint);
#endif

			OPTIONAL_FPRINTF(ofp, "\t\t\t\tControl point value = (%g,%g,%g)\n",
				Nv_ControlPoint[0], Nv_ControlPoint[1], Nv_ControlPoint[2]);
			
			if (Nv_Spline_Curve_Type == Nt_BEZIER) {
				OPTIONAL_FPRINTF(ofp, "\t\t\t\tBezier in-tangent absolute handle location = (%g,%g,%g)\n",
					Nv_Tangent_In[0], Nv_Tangent_In[1], Nv_Tangent_In[2]);
				OPTIONAL_FPRINTF(ofp, "\t\t\t\tBezier out-tangent absolute handle location = (%g,%g,%g)\n",
					Nv_Tangent_Out[0], Nv_Tangent_Out[1], Nv_Tangent_Out[2]);
				OPTIONAL_FPRINTF(ofp, "\t\t\t\tBezier in-tangent type = %s\n", Nv_ControlPoint_TangentInType);
				OPTIONAL_FPRINTF(ofp, "\t\t\t\tBezier out-tangent type = %s\n", Nv_ControlPoint_TangentOutType);
			}

			if (Nv_Spline_Curve_Type == Nt_TCB) {
				OPTIONAL_FPRINTF(ofp, "\t\t\t\tTCB spline tension = %g\n", Nv_TCB_Tension);
				OPTIONAL_FPRINTF(ofp, "\t\t\t\tTCB spline continuity = %g\n", Nv_TCB_Continuity);
				OPTIONAL_FPRINTF(ofp, "\t\t\t\tTCB spline bias = %g\n", Nv_TCB_Bias);
			}
		}
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
