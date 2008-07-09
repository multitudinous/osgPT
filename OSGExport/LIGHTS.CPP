/*****************************************************************************

	----------------------------------------------------------------
                 Lights Parameter Query Functions for PolyTrans
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

/* --------------------->>>>  Light Source Output  <<<<--------------------- */

/* This callback routine is repeatedly called from the toolkit (once for */
/* each light source definition matched to the handle name search pattern) */
/* with the corresponding handle name pointed to by 'cbi_ptr->Nv_Handle_Name1'. */
/* The handle name is then used to inquire about all of the parameters */
/* associated with its associated light source definition. */

/* This function should be called indirectly via this enumerator:
	Ni_Enumerate(&dummy, "*", Nc_FALSE, (Nd_Void *) ofp, (Nd_Void *) 0, 
		NI_Exporter_List_Light_Callback, Nt_LIGHT, Nt_CMDEND);
*/

	Nd_Int
NI_Exporter_List_Light_Callback(Nd_Enumerate_Callback_Info *cbi_ptr)
{
	FILE				*ofp;
	Export_IO_LightParameters 	lp;
	char				*light_name = cbi_ptr->Nv_Handle_Name1;

	ofp = (FILE *) cbi_ptr->Nv_User_Data_Ptr1;	// Pick up user-defined parameter #1; we're passing in the ASCII file output handle

	if (cbi_ptr->Nv_Matches_Made == 0) {
		OPTIONAL_FPRINTF(ofp, "\tThere are no light sources currently defined.\n");
		return(Nc_FALSE);
	}

	// Camera names prefixed with "NUGRAF___" are used internally in the
	// Okino PolyTrans & NuGraf user interface. Ignore them. 
	if (!strncmp(light_name, "NUGRAF___", 9))
		return(Nc_FALSE);

	// Update the status display with the current light definition being exported
	Export_IO_UpdateStatusDisplay("light", light_name, "Exporting light definitions."); 

	// And check for user abort
	if (Export_IO_Check_For_User_Interrupt_With_Stats(cbi_ptr->Nv_Call_Count, cbi_ptr->Nv_Matches_Made))
		return(Nc_TRUE);	// Abort the enumeration (nothing gets returned from the Ni_Enumerate() function

	// Inquire about all light parameters
	Export_IO_Inquire_Light(light_name, &lp);

	/* Next, we'll print out the values returned from the light */
	/* inquiry function to show what can be done with them. */

	OPTIONAL_FPRINTF(ofp, "Light source handle name = '%s'\n", light_name);
	OPTIONAL_FPRINTF(ofp, "\tColor of light = %g,%g,%g\n", lp.Nv_Light_Color[Na_RED], lp.Nv_Light_Color[Na_GREEN], lp.Nv_Light_Color[Na_BLUE]);
	OPTIONAL_FPRINTF(ofp, "\tSpot light cutoff angle = %g\n", lp.Nv_Light_CutoffAngle);
	OPTIONAL_FPRINTF(ofp, "\tDecay value = %ld\n", lp.Nv_Light_Decay);
	OPTIONAL_FPRINTF(ofp, "\tSpot light delta angle = %g\n", lp.Nv_Light_DeltaAngle);
	OPTIONAL_FPRINTF(ofp, "\tDiffuse enable toggle = '%s'\n", lp.Nv_Light_DiffuseToggle);
	OPTIONAL_FPRINTF(ofp, "\tDistance measure = %g\n", lp.Nv_Light_Distance);
	OPTIONAL_FPRINTF(ofp, "\tDrop off factor = %ld\n", lp.Nv_Light_Dropoff);
	OPTIONAL_FPRINTF(ofp, "\tLight enable toggle = '%s'\n", lp.Nv_Light_EnableToggle);
	OPTIONAL_FPRINTF(ofp, "\tIntensity value = %g\n", lp.Nv_Light_Intensity);
	OPTIONAL_FPRINTF(ofp, "\tLight model = '%s'\n", lp.Nv_Light_Model);
	OPTIONAL_FPRINTF(ofp, "\tShine-at location = (%g,%g,%g)\n", lp.Nv_Light_ShineAt[0], lp.Nv_Light_ShineAt[1], lp.Nv_Light_ShineAt[2]);
	OPTIONAL_FPRINTF(ofp, "\tShine-from location = (%g,%g,%g)\n", lp.Nv_Light_ShineFrom[0], lp.Nv_Light_ShineFrom[1], lp.Nv_Light_ShineFrom[2]);
	OPTIONAL_FPRINTF(ofp, "\tShine-Direction vector = (%g,%g,%g)\n", lp.Nv_Light_ShineDir[0], lp.Nv_Light_ShineDir[1], lp.Nv_Light_ShineDir[2]);
	OPTIONAL_FPRINTF(ofp, "\tSpecular enable toggle = '%s'\n", lp.Nv_Light_SpecularToggle);

	// Output any meta data associated with this light
	NI_Exporter_Output_Meta_Data(ofp, Nt_LIGHT, light_name);

	// If any animation is associated with this light definition then output it now
	if (export_options->ena_light_animation)
		NI_Exporter_Output_Light_Animation_Data(ofp, light_name);

	return(Nc_FALSE);	/* Do not terminate the enumeration */
}
