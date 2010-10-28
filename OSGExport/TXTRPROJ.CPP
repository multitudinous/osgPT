/*****************************************************************************

	----------------------------------------------------------------
           Texture Projection Parameter Query Functions for PolyTrans
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

/* ------------------>>>>  Texture Projection Output  <<<<------------------ */

/* If an instance (most likely) or object (less likely) has an implicit */
/* texture projection applied to it (ie: spherical, cylindrical, cubical or */
/* planar) then print out the parameters here. Models imported from Lightwave */
/* usually have a projection method applied to them instead of explicit */
/* (u,v) texture coordinates. Texture projections are normally converted */
/* to explicit uv texture coordinates for a mesh by the Ni_ConvertandProcessRawPrimitive() */
/* function prior to an export converter gaining access to the mesh. Thus, */
/* few export converters ever have to deal with the raw texture projection */
/* parameters directly. */

	Nd_Void
NI_Exporter_List_Texture_Projection_Values(FILE *ofp, Nd_Token type, char *handle_name)
{
	Nd_Token	Nv_TxtrparamToggle;
	Nd_Token	Nv_Txtrparam_CoordSystem, Nv_Txtrparam_Model;
	Nd_Vector	Nv_Txtrparam_Origin,
			Nv_Txtrparam_U_Point,
			Nv_Txtrparam_V_Point,
			Nv_Txtrparam_W_Point;

	/* Inquire about all of the texture parameterization parameters */
	Ni_Inquire_TxtrParam(type, handle_name,
		Nt_ENABLED, (Nd_Token *) &Nv_TxtrparamToggle, Nt_CMDSEP,
		Nt_COORDSYSTEM, (Nd_Token *) &Nv_Txtrparam_CoordSystem, Nt_CMDSEP,
		Nt_ORIGIN, (Nd_Vector *) Nv_Txtrparam_Origin, Nt_CMDSEP,
		Nt_UPOINT, (Nd_Vector *) Nv_Txtrparam_U_Point, Nt_CMDSEP,
		Nt_VPOINT, (Nd_Vector *) Nv_Txtrparam_V_Point, Nt_CMDSEP,
		Nt_WPOINT, (Nd_Vector *) Nv_Txtrparam_W_Point, Nt_CMDSEP,
		Nt_MODEL, (Nd_Token *) &Nv_Txtrparam_Model, Nt_CMDSEP,
		Nt_CMDEND);

	if (Nv_TxtrparamToggle == Nt_ON) {
		OPTIONAL_FPRINTF(ofp, "\tTexture coordinate projection:\n");
		OPTIONAL_FPRINTF(ofp, "\t\tModel = '%s'\n", Nv_Txtrparam_Model);
		OPTIONAL_FPRINTF(ofp, "\t\tEnable toggle = '%s'\n", Nv_TxtrparamToggle);
		OPTIONAL_FPRINTF(ofp, "\t\tCoordinate system = '%s'\n", Nv_Txtrparam_CoordSystem);
		OPTIONAL_FPRINTF(ofp, "\t\tOrigin = %g,%g,%g\n",
			Nv_Txtrparam_Origin[0], Nv_Txtrparam_Origin[1],
			Nv_Txtrparam_Origin[2]);
		OPTIONAL_FPRINTF(ofp, "\t\tU axis point = %g,%g,%g\n",
			Nv_Txtrparam_U_Point[0], Nv_Txtrparam_U_Point[1],
			Nv_Txtrparam_U_Point[2]);
		OPTIONAL_FPRINTF(ofp, "\t\tV axis point = %g,%g,%g\n",
			Nv_Txtrparam_V_Point[0], Nv_Txtrparam_V_Point[1],
			Nv_Txtrparam_V_Point[2]);
		OPTIONAL_FPRINTF(ofp, "\t\tW axis point = %g,%g,%g\n",
			Nv_Txtrparam_W_Point[0], Nv_Txtrparam_W_Point[1],
			Nv_Txtrparam_W_Point[2]);
	}
}
