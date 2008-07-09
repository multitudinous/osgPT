/*****************************************************************************

	----------------------------------------------------------------
              Background Color Scheme Query Functions for PolyTrans
	----------------------------------------------------------------
     This module queries all the parameters of the Background Color Scheme.
	----------------------------------------------------------------

  Copyright (c) 1988, 2004 Okino Computer Graphics, Inc. All Rights Reserved.

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

/* ----------------------->>>>  Background Output  <<<<--------------------- */

	Nd_Void
NI_Exporter_List_Background_Values(FILE *ofp)
{
	Nd_Token	Nv_Enabled_State, Nv_Scheme_Type, Nv_Pattern_Type;
	Nd_Color	Nv_Color1, Nv_Color2, Nv_Background_Color;
	Nd_Float	Nv_X_Repeat, Nv_Y_Repeat;
	char 		*Nv_Colormap_Filename;
	char		*Nv_Surface_Name;
	/* Background gradient parameters */
	Nd_Color	Nv_Grad_Color1, Nv_Grad_Color2, Nv_Grad_Color3, Nv_Grad_Color4;
	Nd_Float	Nv_Horiz_Position;
	Nd_Token	Nv_Horiz_Top_Enable, Nv_Horiz_Bottom_Enable;

	/* Inquire about all of the background parameters */
	Ni_Inquire_Background(
		Nt_ENABLED, (Nd_Token *) &Nv_Enabled_State, Nt_CMDSEP,

		/* This inquires about the current color scheme type */
		Nt_COLORSCHEME, (Nd_Token *) &Nv_Scheme_Type, Nt_CMDSEP,

		/* And the following inquire about the sub-options of each color scheme */
		Nt_COLOR, (Nd_Color *) Nv_Background_Color, Nt_CMDSEP,
		Nt_COLORMAP, Nt_FILENAME, (char **) &Nv_Colormap_Filename, Nt_CMDSEP,
		Nt_GRADIENT, 
			Nt_COLOR1, (Nd_Color *) Nv_Grad_Color1,
			Nt_COLOR2, (Nd_Color *) Nv_Grad_Color2,
			Nt_COLOR3, (Nd_Color *) Nv_Grad_Color3,
			Nt_COLOR4, (Nd_Color *) Nv_Grad_Color4,
			Nt_HORIZTOP, Nt_ENABLED, (Nd_Token *) &Nv_Horiz_Top_Enable,
			Nt_HORIZBOT, Nt_ENABLED, (Nd_Token *) &Nv_Horiz_Bottom_Enable,
			Nt_HORIZPOS, (Nd_Float *) &Nv_Horiz_Position,
			Nt_CMDSEP, 
		Nt_PATTERN, (Nd_Token *) &Nv_Pattern_Type, 
			Nt_COLOR1, (Nd_Color *) Nv_Color1,
			Nt_COLOR2, (Nd_Color *) Nv_Color2,
			Nt_XREPEAT, (Nd_Float *) &Nv_X_Repeat,
			Nt_YREPEAT, (Nd_Float *) &Nv_Y_Repeat,
			Nt_CMDSEP,
		Nt_SURFACELINK, (char **) &Nv_Surface_Name, Nt_CMDSEP,
		Nt_CMDEND
	);

	OPTIONAL_FPRINTF(ofp, "Background enable state is %s\n", Nv_Enabled_State);
	if (Nv_Scheme_Type == Nt_COLOR) {
		OPTIONAL_FPRINTF(ofp, "\tBackground color is the constant color %g %g %g\n",
			Nv_Background_Color[Na_RED], Nv_Background_Color[Na_GREEN],
			Nv_Background_Color[Na_BLUE]);
	} else if (Nv_Scheme_Type == Nt_GRADIENT) {
		OPTIONAL_FPRINTF(ofp, "\tBackground color is a linear gradient\n");
		OPTIONAL_FPRINTF(ofp, "\t\tTop color = %g %g %g\n",
			Nv_Grad_Color1[0], Nv_Grad_Color1[1], Nv_Grad_Color1[2]);
		OPTIONAL_FPRINTF(ofp, "\t\tTop horizon color = %g %g %g\n",
			Nv_Grad_Color2[0], Nv_Grad_Color2[1], Nv_Grad_Color2[2]);
		OPTIONAL_FPRINTF(ofp, "\t\tBottom horizon color = %g %g %g\n",
			Nv_Grad_Color3[0], Nv_Grad_Color3[1], Nv_Grad_Color3[2]);
		OPTIONAL_FPRINTF(ofp, "\t\tBottom color = %g %g %g\n",
			Nv_Grad_Color4[0], Nv_Grad_Color4[1], Nv_Grad_Color4[2]);
		OPTIONAL_FPRINTF(ofp, "\t\tHorizon position = %g\n", Nv_Horiz_Position);
		OPTIONAL_FPRINTF(ofp, "\t\tTop horizon enable flag = %s\n", Nv_Horiz_Top_Enable);
		OPTIONAL_FPRINTF(ofp, "\t\tBottom horizon enable flag = %s\n", Nv_Horiz_Bottom_Enable);
	} else if (Nv_Scheme_Type == Nt_PATTERN) {
		OPTIONAL_FPRINTF(ofp, "\tBackground color is the pattern: %s\n", Nv_Pattern_Type);
		OPTIONAL_FPRINTF(ofp, "\t\t\tColor 1 = %g %g %g\n",
			Nv_Color1[Na_RED], Nv_Color1[Na_GREEN], Nv_Color1[Na_BLUE]);
		OPTIONAL_FPRINTF(ofp, "\t\t\tColor 2 = %g %g %g\n",
			Nv_Color2[Na_RED], Nv_Color2[Na_GREEN], Nv_Color2[Na_BLUE]);
		OPTIONAL_FPRINTF(ofp, "\t\tX repeat factor = %g\n", Nv_X_Repeat);
		OPTIONAL_FPRINTF(ofp, "\t\tY repeat factor = %g\n", Nv_Y_Repeat);
	} else if (Nv_Scheme_Type == Nt_COLORMAP) {
		OPTIONAL_FPRINTF(ofp, "\tBackground color taken from colormap \"%s\"\n", Nv_Colormap_Filename);
	} else if (Nv_Scheme_Type == Nt_SURFACELINK) {
		OPTIONAL_FPRINTF(ofp, "\tBackground color taken from surface \"%s\"", Nv_Surface_Name);
	}
}

