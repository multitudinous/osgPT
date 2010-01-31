/*****************************************************************************

	----------------------------------------------------------------
                   Meta Data Query Functions for PolyTrans
	----------------------------------------------------------------
	 This module queries and displays all meta data associated with
	 the global scene, or with individual instances, cameras, etc.
	Refer to the PolyTrans help file (pt.hlp or nugraf.hlp) for info
	 about common meta data tags (such as for FLT & DXF importers).
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
#include <stdlib.h>

/* ----------------------->>>>  Definitions  <<<<--------------------------- */

/* -------------------->>>>  Local Variables  <<<<-------------------------- */

/* ------------------>>>>  Function Prototypes  <<<<------------------------ */

static Nd_Int	NI_Enumerate_MetaData_Callback(Nd_Enumerate_Callback_Info *cbi_ptr);
static Nd_Int	NI_Enumerate_GlobalSceneMetaData_Callback(Nd_Enumerate_Callback_Info *cbi_ptr);
static void	NI_OutputMetaDataInfo(FILE *ofp, char *Nv_Handle_Name, Nd_Int Nv_Num_Items, Nd_UShort Nv_Data_Type, char *Nv_Data_Ptr);

/* ----------------------->>>>  Meta Data Output  <<<<------------------------- */

// Enumerate the 'meta data' values which are associated with one of these:
//	Nt_INSTANCE, Nt_OBJECT, Nt_LIGHT, Nt_CAMERA, Nt_SURFACE, Nt_TEXTURE, Nt_ANIMCHAN, Nt_ANIMCTRLR
// Note: we cannot enumerate the user data items associated with a surface-texture
//	node (Nt_SURFTXTRNAME) due to lack of argument space in the 'Nd_Enumerate_Callback_Info' struct.
//
// The meta data associated with the global scene can be output by setting 'Nv_Handle_Name' and Nv_Handle_Type to NULL.
//

	Nd_Int
NI_Exporter_Output_Meta_Data(FILE *ofp, Nd_Token Nv_Handle_Type, char *Nv_Handle_Name)
{
	Nd_Int num_output = 0;
	int 	index;

	// Enumerate all of the meta data items associated with a specific
	// handle name. We'll use the 'search string' argument as the
	// method to feed in the handle name.
	index = 0;
	if (Nv_Handle_Name == NULL) {
		/* Enumerate all of the user data items currently in the database */
		Ni_Enumerate(&num_output, "*", Nc_FALSE,
			(Nd_Void *) ofp, (Nd_Void *) NULL,
			NI_Enumerate_GlobalSceneMetaData_Callback, Nt_USERDATA, Nt_CMDEND);
	} else
		Ni_Enumerate(&num_output, Nv_Handle_Name, Nc_FALSE,
			(Nd_Void *) ofp, (Nd_Void *) NULL,
			NI_Enumerate_MetaData_Callback,
			Nv_Handle_Type, Nt_USERDATA, Nt_CMDEND);

	return(num_output);
}

// Enumerate the 'meta data' values which are associated with one of these:
//	Nt_INSTANCE, Nt_OBJECT, Nt_LIGHT, Nt_CAMERA, Nt_SURFACE, Nt_TEXTURE, Nt_ANIMCHAN, Nt_ANIMCTRLR
// Note: we cannot enumerate the user data items associated with a surface-texture
//	node (Nt_SURFTXTRNAME) due to lack of argument space in the 'Nd_Enumerate_Callback_Info' struct.

	static Nd_Int
NI_Enumerate_MetaData_Callback(Nd_Enumerate_Callback_Info *cbi_ptr)
{
	Nd_UShort	Nv_BDF_Save_Flag;
	Nd_Int		Nv_Num_Items;
	Nd_UShort	Nv_Data_Type;
	char	 	*Nv_Data_Ptr;
	FILE		*ofp;

	ofp = (FILE *) cbi_ptr->Nv_User_Data_Ptr1;

	if (!cbi_ptr->Nv_Matches_Made) 
		return(Nc_FALSE);
	
	if (cbi_ptr->Nv_Call_Count == 1)
		OPTIONAL_FPRINTF(ofp, "\tMeta data:\n");

	// Determine if this is one of the hidden or private internal Okino meta data
	// items. This is an 'inline' function defined in ni4_aux.h, as is the list of
	// known hidden/private Okino meta data names. We want to ignore these meta data items.
	if ( !Ni_User_Data_IsHandleNameReservedForInternalUsage( cbi_ptr->Nv_Handle_Name1 ) ) {
		OPTIONAL_FPRINTF(ofp, "\t\t");

		// Go inquire about one meta data item ("cbi_ptr->Nv_Handle_Name1")
		// from the specified instance/object/camera/etc (as defined by
		// the "cbi_ptr->Nv_Handle_Type2, cbi_ptr->Nv_Handle_Name2" arguments)
		Ni_User_Data_Inquire2(cbi_ptr->Nv_Handle_Name1, 		// This is the ID name of the user data item
			&Nv_BDF_Save_Flag, &Nv_Num_Items, &Nv_Data_Type, (Nd_Void **) &Nv_Data_Ptr,	// These are the returned meta data item contents
			cbi_ptr->Nv_Handle_Type2, cbi_ptr->Nv_Handle_Name2, 	// This is the handle type and name, such as 'Nt_INSTANCE, "world"' 
			Nt_CMDEND);

		NI_OutputMetaDataInfo(ofp, (char *) cbi_ptr->Nv_Handle_Name1, Nv_Num_Items, Nv_Data_Type, Nv_Data_Ptr);
	}

	if (cbi_ptr->Nv_Call_Count == cbi_ptr->Nv_Matches_Made)
		OPTIONAL_FPRINTF(ofp, "\n");

	return(Nc_FALSE);	/* Do not terminate the enumeration */
}

// Same as above but is used to enumerate the scene meta data

	static Nd_Int
NI_Enumerate_GlobalSceneMetaData_Callback(Nd_Enumerate_Callback_Info *cbi_ptr)
{
	Nd_UShort	Nv_BDF_Save_Flag;
	Nd_Int		Nv_Num_Items;
	Nd_UShort	Nv_Data_Type;
	char	 	*Nv_Data_Ptr;
	FILE		*ofp;

	if (!cbi_ptr->Nv_Matches_Made) 
		return(Nc_FALSE);
	
	ofp = (FILE *) cbi_ptr->Nv_User_Data_Ptr1;

	if (cbi_ptr->Nv_Call_Count == 1)
		OPTIONAL_FPRINTF(ofp, "Meta data assigned to the global scene:\n");

	// Determine if this is one of the hidden or private internal Okino meta data
	// items. This is an 'inline' function defined in ni4_aux.h, as is the list of
	// known hidden/private Okino meta data names. We want to ignore these meta data items.
	if ( !Ni_User_Data_IsHandleNameReservedForInternalUsage( cbi_ptr->Nv_Handle_Name1 ) ) {
		OPTIONAL_FPRINTF(ofp, "\t");

		// Go inquire about one meta data item from the global scene
		Ni_User_Data_Inquire(cbi_ptr->Nv_Handle_Name1, 		// This is the ID name of the meta data item
			&Nv_BDF_Save_Flag, &Nv_Num_Items, &Nv_Data_Type, (Nd_Void **) &Nv_Data_Ptr);	// These are the returned user data item contents

		NI_OutputMetaDataInfo(ofp, (char *) cbi_ptr->Nv_Handle_Name1, Nv_Num_Items, Nv_Data_Type, Nv_Data_Ptr);
	}

	if (cbi_ptr->Nv_Call_Count == cbi_ptr->Nv_Matches_Made)
		OPTIONAL_FPRINTF(ofp, "\n");

	return(Nc_FALSE);	/* Do not terminate the enumeration */
}

	static void
NI_OutputMetaDataInfo(
	FILE		*ofp,
	char		*Nv_Handle_Name,
	Nd_Int		Nv_Num_Items,
	Nd_UShort	Nv_Data_Type,
	char	 	*Nv_Data_Ptr)
{
	long	i, j;

// See also,
// 	Ni_User_Data_Format_DataElements_Into_Standardized_String()
// 	Ni_User_Data_Format_DataElements_Into_Standardized_String2()
// These 2 functions act similar to Ni_User_Data_Inquire() and Ni_User_Data_Inquire2()
// except that they format all non-character data elements (shorts, floats, matrices, etc)
// into a single string with an Okino prefix on them (like "ok_short: 1; 2; 3; 4"). This
// is useful for 3D exporters which can only accepts strings for their exported meta data.
// Likewise, Ni_User_Data_ParseAndSet_DataElements_From_Standardized_String() and
// Ni_User_Data_ParseAndSet_DataElements_From_Standardized_String2() can be used by 3D importers
// to parse these Okino syntax qualified strings and extract the array of elements out of the
// strings into their usser-defined data elements again. The use of the Okino encoding syntax
// allows for proper round-tripping of meta data.

	switch(Nv_Data_Type) {
	case Nc_BDF_USERDATA_NdString:
		OPTIONAL_FPRINTF(ofp, "Name = '%s', Data type = 'String', Data item = '%s'\n", Nv_Handle_Name, (char *) Nv_Data_Ptr);
		break;
	case Nc_BDF_USERDATA_NdChar:
		// This could be a string or a raw data array
		{ char *ptr = (char *) Nv_Data_Ptr;
		  long	i;
		  short is_binary = Nc_FALSE;

		// Let's see if the data is displayable
		for (i=0; i < Nv_Num_Items; ++i) {
			if (i == Nv_Num_Items-1 && ptr[i] == '\0')
				break;
			if (ptr[i] < 0x20 || ptr[i] > 0x7f) {
				is_binary = Nc_TRUE;
				break;
			}
		}

		if (is_binary) {
			OPTIONAL_FPRINTF(ofp, "Name = '%s', Data type = 'Binary Data', Data item = ", Nv_Handle_Name);
		} else {
			OPTIONAL_FPRINTF(ofp, "Name = '%s', Data type = 'String', Data item = ", Nv_Handle_Name);
		}

		if (is_binary) {
			short len = Nm_MIN(16, Nv_Num_Items);	// Show about 16 bytes of the raw array
			char buf[1024], buf2[128];

			buf[0] = '\0';
			for (i=0; i < len; ++i) {
				sprintf(buf2, "%2x ", ptr[i]);
				strcat(buf, buf2);
				if (i == len-1)
					strcat(buf, "..");
			}
			OPTIONAL_FPRINTF(ofp, "%s\n", buf);
		} else {
			OPTIONAL_FPRINTF(ofp, "%s\n", ptr);
		}
		}
		break;
	case Nc_BDF_USERDATA_NdShort:
		for (i = 0; i < Nv_Num_Items; ++i, Nv_Data_Ptr += sizeof(short)) {
			char	buf[256];

			if (!i)
				OPTIONAL_FPRINTF(ofp, "Name = '%s', ", Nv_Handle_Name);
			else
				OPTIONAL_FPRINTF(ofp, "\t");

			OPTIONAL_FPRINTF(ofp, "Data type = '16-bit Short Integer', %ld of %ld, ", i+1, Nv_Num_Items);

			sprintf(buf, "%ld", *((short *) Nv_Data_Ptr));
			OPTIONAL_FPRINTF(ofp, "%s\n", buf);
		}
		break;
	case Nc_BDF_USERDATA_NdInt:
		for (i = 0; i < Nv_Num_Items; ++i, Nv_Data_Ptr += sizeof(Nd_Int)) {
			char	buf[256];

			if (!i)
				OPTIONAL_FPRINTF(ofp, "Name = '%s', ", Nv_Handle_Name);
			else
				OPTIONAL_FPRINTF(ofp, "\t");

			OPTIONAL_FPRINTF(ofp, "Data type = '32-bit Integer', %ld of %ld, ", i+1, Nv_Num_Items);

			sprintf(buf, "%ld", *((Nd_Int *) Nv_Data_Ptr));
			OPTIONAL_FPRINTF(ofp, "%s\n", buf);
		}
		break;
	case Nc_BDF_USERDATA_NdFloat:
		for (i = 0; i < Nv_Num_Items; ++i, Nv_Data_Ptr += sizeof(float)) {
			char	buf[256];

			if (!i)
				OPTIONAL_FPRINTF(ofp, "Name = '%s', ", Nv_Handle_Name);
			else
				OPTIONAL_FPRINTF(ofp, "\t");

			OPTIONAL_FPRINTF(ofp, "Data type = '32-bit Float', %ld of %ld, ", i+1, Nv_Num_Items);

			sprintf(buf, "%f", *((float *) Nv_Data_Ptr));
			OPTIONAL_FPRINTF(ofp, "%s\n", buf);
		}
		break;
	case Nc_BDF_USERDATA_NdColor:
		for (i = 0; i < Nv_Num_Items; ++i, Nv_Data_Ptr += sizeof(Nd_Color)) {
			char	buf[256];

			if (!i)
				OPTIONAL_FPRINTF(ofp, "Name = '%s', ", Nv_Handle_Name);
			else
				OPTIONAL_FPRINTF(ofp, "\t");

			OPTIONAL_FPRINTF(ofp, "Data type = 'RGB Color', %ld of %ld, ", i+1, Nv_Num_Items);

			sprintf(buf, "%f %f %f", ((Nd_Float *) Nv_Data_Ptr)[0], ((Nd_Float *) Nv_Data_Ptr)[1], ((Nd_Float *) Nv_Data_Ptr)[2]);
			OPTIONAL_FPRINTF(ofp, "%s\n", buf);
		}
		break;
	case Nc_BDF_USERDATA_NdVector:
		for (i = 0; i < Nv_Num_Items; ++i, Nv_Data_Ptr += sizeof(Nd_Vector)) {
			char	buf[256];

			if (!i)
				OPTIONAL_FPRINTF(ofp, "Name = '%s', ", Nv_Handle_Name);
			else
				OPTIONAL_FPRINTF(ofp, "\t");

			OPTIONAL_FPRINTF(ofp, "Data type = 'XYZ Vector', %ld of %ld, ", i+1, Nv_Num_Items);

			sprintf(buf, "%f %f %f", ((Nd_Float *) Nv_Data_Ptr)[0], ((Nd_Float *) Nv_Data_Ptr)[1], ((Nd_Float *) Nv_Data_Ptr)[2]);
			OPTIONAL_FPRINTF(ofp, "%s\n", buf);
		}
		break;
	case Nc_BDF_USERDATA_NdMatrix:
		for (i = 0; i < Nv_Num_Items; ++i, Nv_Data_Ptr += sizeof(Nd_Matrix)) {
			char	buf2[256], buf[1024];

			if (!i)
				OPTIONAL_FPRINTF(ofp, "Name = '%s', ", Nv_Handle_Name);
			else
				OPTIONAL_FPRINTF(ofp, "\t");

			OPTIONAL_FPRINTF(ofp, "Data type = '16x16 Matrix', %ld of %ld, ", i+1, Nv_Num_Items);

			buf[0] = '\0';
			for (j = 0; j < 16; ++j, Nv_Data_Ptr) {
				sprintf(buf2, "%f ", ((Nd_Float *) Nv_Data_Ptr)[j]);
				strcat(buf, buf2);
			}
			OPTIONAL_FPRINTF(ofp, "%s\n", buf);		}
		break;
	case Nc_BDF_USERDATA_NdDblMatrix:
		for (i = 0; i < Nv_Num_Items; ++i, Nv_Data_Ptr += sizeof(Nd_DblMatrix)) {
			char	buf2[256], buf[1024];

			if (!i)
				OPTIONAL_FPRINTF(ofp, "Name = '%s', ", Nv_Handle_Name);
			else
				OPTIONAL_FPRINTF(ofp, "\t");

			OPTIONAL_FPRINTF(ofp, "Data type = 'Dbl 16x16 Matrix', %ld of %ld, ", i+1, Nv_Num_Items);

			buf[0] = '\0';
			for (j = 0; j < 16; ++j, Nv_Data_Ptr) {
				sprintf(buf2, "%f ", ((Nd_Double *) Nv_Data_Ptr)[j]);
				strcat(buf, buf2);
			}
			OPTIONAL_FPRINTF(ofp, "%s\n", buf);
		}
		break;
	case Nc_BDF_USERDATA_NdTime:
		for (i = 0; i < Nv_Num_Items; ++i, Nv_Data_Ptr += sizeof(Nd_Time)) {
			char	buf[256];

			if (!i)
				OPTIONAL_FPRINTF(ofp, "Name = '%s', ", Nv_Handle_Name);
			else
				OPTIONAL_FPRINTF(ofp, "\t");

			OPTIONAL_FPRINTF(ofp, "Data type = 'Ticks-Time', %ld of %ld, ", i+1, Nv_Num_Items);

			sprintf(buf, "%ld", *((Nd_Time *) Nv_Data_Ptr));
			OPTIONAL_FPRINTF(ofp, "%s\n", buf);
		}
		break;
	case Nc_BDF_USERDATA_NdDouble:
		for (i = 0; i < Nv_Num_Items; ++i, Nv_Data_Ptr += sizeof(double)) {
			char	buf[256];

			if (!i)
				OPTIONAL_FPRINTF(ofp, "Name = '%s', ", Nv_Handle_Name);
			else
				OPTIONAL_FPRINTF(ofp, "\t");

			OPTIONAL_FPRINTF(ofp, "Data type = 'Dbl Float', %ld of %ld, ", i+1, Nv_Num_Items);

			sprintf(buf, "%f", *((double *) Nv_Data_Ptr));
			OPTIONAL_FPRINTF(ofp, "%s\n", buf);
		}
		break;
	case Nc_BDF_USERDATA_NdVector4:
		for (i = 0; i < Nv_Num_Items; ++i, Nv_Data_Ptr += sizeof(Nd_Vector4)) {
			char	buf[256];

			if (!i)
				OPTIONAL_FPRINTF(ofp, "Name = '%s', ", Nv_Handle_Name);
			else
				OPTIONAL_FPRINTF(ofp, "\t");

			OPTIONAL_FPRINTF(ofp, "Data type = 'XYZW Vector', %ld of %ld, ", i+1, Nv_Num_Items);

			sprintf(buf, "%f %f %f %f", ((Nd_Float *) Nv_Data_Ptr)[0], ((Nd_Float *) Nv_Data_Ptr)[1], ((Nd_Float *) Nv_Data_Ptr)[2], ((Nd_Float *) Nv_Data_Ptr)[3]);
			OPTIONAL_FPRINTF(ofp, "%s\n", buf);
		}
		break;
	case Nc_BDF_USERDATA_NdDblVector:
		for (i = 0; i < Nv_Num_Items; ++i, Nv_Data_Ptr += sizeof(Nd_DblVector)) {
			char	buf[256];

			if (!i)
				OPTIONAL_FPRINTF(ofp, "Name = '%s', ", Nv_Handle_Name);
			else
				OPTIONAL_FPRINTF(ofp, "\t");

			OPTIONAL_FPRINTF(ofp, "Data type = 'Dbl XYZ Vector', %ld of %ld, ", i+1, Nv_Num_Items);

			sprintf(buf, "%f %f %f",
				((Nd_Double *) Nv_Data_Ptr)[0], 
				((Nd_Double *) Nv_Data_Ptr)[1], 
				((Nd_Double *) Nv_Data_Ptr)[2]);
			OPTIONAL_FPRINTF(ofp, "%s\n", buf);
		}
		break;
	case Nc_BDF_USERDATA_NdDblVector4:
		for (i = 0; i < Nv_Num_Items; ++i, Nv_Data_Ptr += sizeof(Nd_DblVector4)) {
			char	buf[256];

			if (!i)
				OPTIONAL_FPRINTF(ofp, "Name = '%s', ", Nv_Handle_Name);
			else
				OPTIONAL_FPRINTF(ofp, "\t");

			OPTIONAL_FPRINTF(ofp, "Data type = 'Dbl XYZW Vector', %ld of %ld, ", i+1, Nv_Num_Items);
			sprintf(buf, "%f %f %f %f",
				((Nd_Double *) Nv_Data_Ptr)[0], 
				((Nd_Double *) Nv_Data_Ptr)[1], 
				((Nd_Double *) Nv_Data_Ptr)[2],
				((Nd_Double *) Nv_Data_Ptr)[3]);
			OPTIONAL_FPRINTF(ofp, "%s\n", buf);
		}
		break;
	}
}

