/*****************************************************************************

	----------------------------------------------------------------
                   Atmospheric Query Functions for PolyTrans
	----------------------------------------------------------------
   	   This module queries all the parameters of the 'Atmosphere'
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

#include "main.h"	// Main include file for a PolyTrans exporter

/* ----------------------->>>>  Definitions  <<<<--------------------------- */

/* -------------------->>>>  Local Variables  <<<<-------------------------- */

/* ------------------>>>>  Function Prototypes  <<<<------------------------ */

/* -------------------->>>>  Atmospheric Output  <<<<----------------------- */

	Nd_Void
NI_Exporter_List_Atmospheric_Values(FILE *ofp)
{
	/* Fog variables */
	Nd_Token	fog_enabled_state,
			fog_type,
			fog_BGFogged_enabled_state,
			fog_UseBGColor_enabled_state;
	Nd_Color	fog_color;
	Nd_Float	fog_near_distance, fog_far_distance,
			fog_near_blend, fog_far_blend;

	/* Mist variables */
	Nd_Token	mist_enabled_state,
			mist_BGMisted_enabled_state,
			mist_UseBGColor_enabled_state,
			mist_axis;
	Nd_Color	mist_color;
	Nd_Float	mist_base_altitude, mist_distance, mist_thickness;

	/* Inquire about all of the atmospheric shader parameters */
	Ni_Inquire_Atmosphere(
		Nt_FOG,
			/* All the following are optional */
			Nt_ENABLED, 	(Nd_Token *) 	&fog_enabled_state,
			Nt_FOGTYPE, 	(Nd_Token *) 	&fog_type,
			Nt_COLOR, 	(Nd_Color *) 	fog_color,
			Nt_NEARDISTANCE, (Nd_Float *) 	&fog_near_distance,
			Nt_FARDISTANCE, (Nd_Float *) 	&fog_far_distance,
			Nt_NEARBLEND, 	(Nd_Float *) 	&fog_near_blend,
			Nt_FARBLEND, 	(Nd_Float *) 	&fog_far_blend,
			Nt_BGFOGGED, Nt_ENABLED, (Nd_Token *) &fog_BGFogged_enabled_state,
			Nt_USEBGCOLOR, Nt_ENABLED, (Nd_Token *) &fog_UseBGColor_enabled_state,
			Nt_CMDSEP,
		Nt_MIST,
			Nt_BGMISTED, Nt_ENABLED, (Nd_Token *) &mist_BGMisted_enabled_state,
			Nt_BASEALTITUDE, (Nd_Float *) 	&mist_base_altitude,
			Nt_COLOR, 	(Nd_Color *) 	mist_color,
			Nt_DISTANCE, 	(Nd_Float *) 	&mist_distance,
			Nt_ENABLED, 	(Nd_Token *) 	&mist_enabled_state,
			Nt_THICKNESS, 	(Nd_Float *) 	&mist_thickness,
			Nt_UPAXIS, 	(Nd_Token *) 	&mist_axis,
			Nt_USEBGCOLOR, Nt_ENABLED, (Nd_Token *) &mist_UseBGColor_enabled_state,
			Nt_CMDSEP,
		Nt_CMDEND
	);

	if (fog_enabled_state == Nt_ON) {
		OPTIONAL_FPRINTF(ofp, "Fog options:\n");
		OPTIONAL_FPRINTF(ofp, "\tType = '%s'\n", fog_type);
		OPTIONAL_FPRINTF(ofp, "\tEnabled = %s\n", fog_enabled_state);
		OPTIONAL_FPRINTF(ofp, "\tFog colour = %g %g %g\n",fog_color[Na_RED], fog_color[Na_GREEN], fog_color[Na_BLUE]);
		OPTIONAL_FPRINTF(ofp, "\tNear distance = %g\n", fog_near_distance);
		OPTIONAL_FPRINTF(ofp, "\tNear blend = %g\n", fog_near_blend);
		OPTIONAL_FPRINTF(ofp, "\tFar distance = %g\n", fog_far_distance);
		OPTIONAL_FPRINTF(ofp, "\tFar blend = %g\n", fog_far_blend);
		OPTIONAL_FPRINTF(ofp, "\tBackground fogged flag is %s\n", fog_BGFogged_enabled_state);
		OPTIONAL_FPRINTF(ofp, "\tUse background color flag is %s\n", fog_UseBGColor_enabled_state);
	} else
		OPTIONAL_FPRINTF(ofp, "Fog option disabled.\n");
	OPTIONAL_FPRINTF(ofp, "\n");

	if (mist_enabled_state == Nt_ON) {
		OPTIONAL_FPRINTF(ofp, "Mist options:\n");
		OPTIONAL_FPRINTF(ofp, "\tEnabled = %s\n", mist_enabled_state);
		OPTIONAL_FPRINTF(ofp, "\tMist colour = %g %g %g\n", mist_color[Na_RED], mist_color[Na_GREEN], mist_color[Na_BLUE]);
		OPTIONAL_FPRINTF(ofp, "\tMist distance = %g\n", mist_distance);
		OPTIONAL_FPRINTF(ofp, "\tBase altitude = %g\n", mist_base_altitude);
		OPTIONAL_FPRINTF(ofp, "\tMist thickness = %g\n", mist_thickness);
		OPTIONAL_FPRINTF(ofp, "\tBackground misted flag is %s\n", mist_BGMisted_enabled_state);
		OPTIONAL_FPRINTF(ofp, "\tUp axis is: %s\n", mist_axis);
		OPTIONAL_FPRINTF(ofp, "\tUse background colour flag is %s\n", mist_UseBGColor_enabled_state);
	} else
		OPTIONAL_FPRINTF(ofp, "Mist option disabled.\n");
}
