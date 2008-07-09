/*****************************************************************************

	----------------------------------------------------------------
                 Texture Definition Query Functions for PolyTrans
	----------------------------------------------------------------
	In PolyTrans a 'Material' (Surface Definition) is assigned to 
	each object and optionally to one or more polygons of an object.
	Each Material can then have one or more texture layers, with
	each layer pointing to a 'Texture Definition'. A Texture Defintion
	can be a 2d bitmap or a 3d procedural function. This file shows
	how to query all of the parameters related to the Texture Definition.
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

// This list will hold the name of all texture map images being exported to
// the output file. If automatic bitmap conversion is enabled then this
// list defines which files on disk should be converted to the new format.
typedef struct Exporter_TxtrFileName_Info {
	char		*orig_filename;
	char		*unique_filename;
	// This is the internal toolkit token for the 2D file exporter which will 
	// be used to auto-convert the 'orig_filename' into the new file format.
	// If the exporter is custom then this token will usually be NULL and the 
	// GUID will be used instead (export_converter_guid). 
	Nd_Token	export_image_filetype;
	// This is the GUID corresponding to the 2D file exporter which will 
	// be used to auto-convert the 'orig_filename' into the new file format
	char		*export_converter_guid;
	struct Exporter_TxtrFileName_Info *next_ptr;
} Exporter_TxtrFileName_Info;
static Exporter_TxtrFileName_Info	*Exporter_txtrfilename_list = NULL;

/* -------------------->>>>  Local Variables  <<<<-------------------------- */

/* ------------------>>>>  Function Prototypes  <<<<------------------------ */

static Nd_Void	NI_Exporter_AddNewTxtrFilenameNode(char *orig_filename, char *unique_filename, Nd_Token export_image_filetype, char *exporter_guid);
static Nd_Void	NI_Exporter_MakeValidTextureBitmapFilename(char *in_filename, char *out_unique_name);
static size_t	NI_Exporter_Case_Insensitive_Strcmp(char *s, char *t);

/* ------------------>>>>  Texture Definitions Output  <<<<----------------- */

/* This callback routine is repeatedly called from the renderer (once for */
/* each texture definition matched to the handle name search pattern) */
/* with the corresponding handle name pointed to by 'cbi_ptr->Nv_Handle_Name1'. */
/* The handle name is then used to inquire about all of the parameters */
/* associated with its associated textue definition. */

/* This function should be called indirectly via this enumerator:
	Ni_Enumerate(&dummy, "*", Nc_FALSE, (Nd_Void *) ofp, (Nd_Void *) 0, 
		NI_Exporter_List_Texture_Callback, Nt_TEXTURE, Nt_CMDEND);
*/

	Nd_Int
NI_Exporter_List_Texture_Callback(Nd_Enumerate_Callback_Info *cbi_ptr)
{
	FILE				*ofp;
	Export_IO_TextureParameters	txtr_info;
	Nd_Matrix			Nv_Texture_Matrix;
	char				*texture_defn_name = cbi_ptr->Nv_Handle_Name1;

	ofp = (FILE *) cbi_ptr->Nv_User_Data_Ptr1;	// Pick up user-defined parameter #1; we're passing in the ASCII file output handle

	if (cbi_ptr->Nv_Matches_Made == 0) {
		OPTIONAL_FPRINTF(ofp, "\tThere are no textures currently defined.\n");
		return(Nc_FALSE);
	}

	// Camera names prefixed with "NUGRAF___" are used internally in the
	// Okino PolyTrans & NuGraf user interface. Ignore them. 
	if (!strncmp(texture_defn_name, "NUGRAF___", 9))
		return(Nc_FALSE);

	// Update the stats
	++export_options->total_texture_maps;

	// Update the status display with the current texture definition being exported
	Export_IO_UpdateStatusDisplay("texture", texture_defn_name, "Exporting texture definitions."); 

	// And check for user abort
	if (Export_IO_Check_For_User_Interrupt_With_Stats(cbi_ptr->Nv_Call_Count, cbi_ptr->Nv_Matches_Made))
		return(Nc_TRUE);	// Abort the enumeration (nothing gets returned from the Ni_Enumerate() function

	Export_IO_Inquire_Texture(texture_defn_name, &txtr_info);

	/* Next, we'll print out the values returned from the texture */
	/* inquiry function to show what can be done with them. */

	if (txtr_info.Nv_Type == Nt_IMAGE)  {
		OPTIONAL_FPRINTF(ofp, "\nTexture definition '%s' uses a 2d Texture Image.\n", texture_defn_name);
		if (txtr_info.Nv_Image_Filename == (char *) 0) {
			OPTIONAL_FPRINTF(ofp, "\tThere is no valid texture image associated\n");
			OPTIONAL_FPRINTF(ofp, "\twith the texture definition.\n");
		} else {
			char	processed_image_filename[512];

			OPTIONAL_FPRINTF(ofp, "\tOriginal image filename = '%s', filetype = '%s'\n", 
				txtr_info.Nv_Image_Filename, txtr_info.Nv_Image_Filetype);

			// Go and apply our texture filename processing options
			// and optionally go and queue up the image for automatic bitmap conversion
			NI_Exporter_MakeValidTextureBitmapFilename(txtr_info.Nv_Image_Filename, processed_image_filename);

			OPTIONAL_FPRINTF(ofp, "\tProcessed image filename = '%s'\n", processed_image_filename);
		}
		OPTIONAL_FPRINTF(ofp, "\tTexture definition enable toggle = '%s'\n", txtr_info.Nv_Texture_EnableToggle);
		OPTIONAL_FPRINTF(ofp, "\tTexture filter type = '%s'\n", txtr_info.Nv_Filter_Type);
		OPTIONAL_FPRINTF(ofp, "\tAlpha channel type is %s\n", txtr_info.Nv_AlphaChannelType);
			OPTIONAL_FPRINTF(ofp, "\t\tAlpha channel 'invert' enable toggle = '%s'\n", 
				txtr_info.Nv_AlphaChannel_InvertEnableToggle);
			OPTIONAL_FPRINTF(ofp, "\t\tChroma key color is (%g,%g,%g)\n", 
				txtr_info.Nv_ChromaKey_Color[Na_RED],
				txtr_info.Nv_ChromaKey_Color[Na_GREEN],
				txtr_info.Nv_ChromaKey_Color[Na_BLUE]);
			OPTIONAL_FPRINTF(ofp, "\t\tChroma key tolerance is (%d,%d,%d)\n",
				txtr_info.Nv_ChromaKey_Tolerance[0], 
				txtr_info.Nv_ChromaKey_Tolerance[1], 
				txtr_info.Nv_ChromaKey_Tolerance[2]);
		OPTIONAL_FPRINTF(ofp, "\tCrop window = (%g,%g) to (%g,%g)\n", 
			txtr_info.Nv_Cropwindow_Params[Na_CROPWINDOW_LEFT],
			txtr_info.Nv_Cropwindow_Params[Na_CROPWINDOW_BOTTOM],
			txtr_info.Nv_Cropwindow_Params[Na_CROPWINDOW_RIGHT],
			txtr_info.Nv_Cropwindow_Params[Na_CROPWINDOW_TOP]);
		OPTIONAL_FPRINTF(ofp, "\tMIPmap blurring = %g\n", txtr_info.Nv_MIPMAP_Blur);
		OPTIONAL_FPRINTF(ofp, "\tWrap-U enable toggle = '%s'\n", 
			txtr_info.Nv_WrapU_EnableToggle);
		OPTIONAL_FPRINTF(ofp, "\tWrap-V enable toggle = '%s'\n", 
			txtr_info.Nv_WrapV_EnableToggle);
	} else if (txtr_info.Nv_Type == Nt_PROCEDURAL)  {
		OPTIONAL_FPRINTF(ofp, "Texture definition '%s' uses a 3d Procedural Texture.\n", texture_defn_name);
		OPTIONAL_FPRINTF(ofp, "\tProcedural texture function name = '%s'\n", 
			txtr_info.Nv_Procedural_Function_Name);
		OPTIONAL_FPRINTF(ofp, "\tTexture color # 1 (RGB space) = %g, %g, %g\n",
			txtr_info.Nv_Color1[Na_RED], txtr_info.Nv_Color1[Na_GREEN], txtr_info.Nv_Color1[Na_BLUE]);
		OPTIONAL_FPRINTF(ofp, "\tTexture color # 2 (RGB space) = %g, %g, %g\n",
			txtr_info.Nv_Color2[Na_RED], txtr_info.Nv_Color2[Na_GREEN], txtr_info.Nv_Color2[Na_BLUE]);
		OPTIONAL_FPRINTF(ofp, "\tTexture color # 3 (RGB space) = %g, %g, %g\n",
			txtr_info.Nv_Color3[Na_RED], txtr_info.Nv_Color3[Na_GREEN], txtr_info.Nv_Color3[Na_BLUE]);
		if (txtr_info.Nv_Colormap_Filename != (char *) 0)
			OPTIONAL_FPRINTF(ofp, "\tColormap filename is '%s'\n", 
				txtr_info.Nv_Colormap_Filename);
		OPTIONAL_FPRINTF(ofp, "\tTexture definition enable toggle = '%s'\n", 
			txtr_info.Nv_Texture_EnableToggle);
		OPTIONAL_FPRINTF(ofp, "\tStarting turbulence frequency = %g\n", txtr_info.Nv_Starting_Frequency);
		OPTIONAL_FPRINTF(ofp, "\tNumber of octaves of noise for turbulence = %d\n", 
			txtr_info.Nv_Number_Octaves);
		OPTIONAL_FPRINTF(ofp, "\tProcedural texture run-time parameters:\n");
		OPTIONAL_FPRINTF(ofp, "\t\tParameter 1 = %g\n", txtr_info.Nv_Param[0]);
		OPTIONAL_FPRINTF(ofp, "\t\tParameter 2 = %g\n", txtr_info.Nv_Param[1]);
		OPTIONAL_FPRINTF(ofp, "\t\tParameter 3 = %g\n", txtr_info.Nv_Param[2]);
		OPTIONAL_FPRINTF(ofp, "\t\tParameter 4 = %g\n", txtr_info.Nv_Param[3]);
		OPTIONAL_FPRINTF(ofp, "\t\tParameter 5 = %g\n", txtr_info.Nv_Param[4]);
		OPTIONAL_FPRINTF(ofp, "\t\tParameter 6 = %g\n", txtr_info.Nv_Param[5]);
		OPTIONAL_FPRINTF(ofp, "\tTurbulence scale factor = %g\n", txtr_info.Nv_Turbulence_Scale);

		Ni_Inquire_Texture(texture_defn_name, (Nd_Token *) &txtr_info.Nv_Type,
			Nt_MATRIX, Nv_Texture_Matrix, Nt_CMDEND);
		OPTIONAL_FPRINTF(ofp, "\tTexture's current transformation matrix:\n");
		OPTIONAL_DISPLAY_MATRIX(ofp, Nv_Texture_Matrix, 2);
	} 

	// Output any meta data associated with this texture definition
	NI_Exporter_Output_Meta_Data(ofp, Nt_TEXTURE, texture_defn_name);

	return(Nc_FALSE);	/* Do not terminate the enumeration */
}

// ----------->>>>  Texture Image Filename Cleaning Routine  <<-------------- 

/* Given a bitmap texture filepath and filename, this routine */
/* prepends an optional filepath then adds the necessary extension. */

	static Nd_Void
NI_Exporter_MakeValidTextureBitmapFilename(char *in_filename, char *out_unique_name)
{
	Exporter_TxtrFileName_Info *ptr;
	Nd_Token export_image_filetype;
	char    orig_filename[300], extension[300], *save_path;
	long    result, len;
	short   found, do_auto_convert;

	extension[0] = '\0';
	strcpy(orig_filename, in_filename);

	/* See if this filename has already been converted previously */
	found = Nc_FALSE;
	ptr = Exporter_txtrfilename_list;
	while (ptr != NULL) {
		if (!NI_Exporter_Case_Insensitive_Strcmp(ptr->orig_filename, orig_filename)) {
			strcpy(out_unique_name, ptr->unique_filename);
			goto exit;
		}
		ptr = ptr->next_ptr;
	}

	// Optionally make the file path absolute
	if (export_options->use_absolute_file_paths && orig_filename[0] != '/' && orig_filename[0] != '\\' && export_options->export_directory) {
		char	buf[300];

		Ni_ExtractBaseFileName(orig_filename, buf, Nc_FALSE);
		Ni_ConcatenatePathAndFilename(Nc_TRUE, export_options->export_directory, buf, out_unique_name);
	} else
		strcpy(out_unique_name, orig_filename);

	/* Strip off the file extension. */
	len = (long) strlen(out_unique_name);
	while (len > 0) {
		len--;
		if (out_unique_name[len] == '.')
			break;
	}
	out_unique_name[len] = '\0';
	strcpy(extension, out_unique_name + len + 1);

	/* Add the proper file extension */
	do_auto_convert = Nc_FALSE;
	if (!export_options->ena_convert_txtr_bitmaps && export_options->ena_use_specific_bitmap_extension && export_options->convert_file_extension != NULL) {
		strcat(out_unique_name, export_options->convert_file_extension);
	} else if (export_options->ena_convert_txtr_bitmaps && export_options->autoconvert_bitmap_format) {
		char	autoconvert_bitmap_format[50];

		/* Prepend a specific directory where to store the converted
		 * bitmap */
		save_path = NULL;
		if (export_options->bitmap_saveto_directory_type == SAVE_BITMAPS_TO_ORIGINAL_DIRECTORY && export_options->export_directory && strlen(export_options->export_directory))
			save_path = export_options->export_directory;
		else if (export_options->bitmap_saveto_directory_type == SAVE_BITMAPS_TO_SPECIFIC_DIRECTORY && export_options->save_bitmaps_to_specific_directory && strlen(export_options->save_bitmaps_to_specific_directory))
			save_path = export_options->save_bitmaps_to_specific_directory;

		if (save_path) {
			char    temp_buf[512];

			/* Make sure the directory exists */
			Ni_File_Exists(save_path, &result);
			if (!result)
				mkdir(save_path);

			/* Prepend the path on the output filename */
			Ni_ExtractBaseFileName(out_unique_name, temp_buf, Nc_FALSE);
			Ni_ConcatenatePathAndFilename(Nc_TRUE, save_path, temp_buf, out_unique_name);
		}

// RCL: Modified October 22 2004.
#if 11
		if (export_options->autoconvert_bitmap_format[0] == '{') {
			char	*Nv_File_Extension_List;

			// The auto-convert file format is being specified but its
			// corresponding GUID so we need to look-up its corresponding
			// file extension via the File Format Handler mechanism.
			Ni_Inquire_FileFormatHandler(export_options->autoconvert_bitmap_format,
				Nt_FILETYPES, (char **) &Nv_File_Extension_List, Nt_CMDEND);
			if (Nv_File_Extension_List)
				strcpy(autoconvert_bitmap_format, Nv_File_Extension_List);
			else
				strcpy(autoconvert_bitmap_format, ".tif");	// Should not happen
		} else
			// Else the string is the real extension like ".tiff")
			strcpy(autoconvert_bitmap_format, export_options->autoconvert_bitmap_format);

		/* If the file's extension is the same as the desired output file format, */
		/* and the user does not want to resample it, then don't resample image */
		if (!strcmp(autoconvert_bitmap_format + 1, extension) && export_options->txtr_x_res == 1 && export_options->txtr_y_res == 1 && export_options->txtr_bitmap_depth == 24)
			goto output2;

		/* Add the new file extension */
		strcat(out_unique_name, autoconvert_bitmap_format);
#endif
		do_auto_convert = Nc_TRUE;
	} else {
		/* Else, add the original file extension if it is acceptable to 3DSMAX */
output2:	strcat(out_unique_name, ".");
		strcat(out_unique_name, extension);
	}

	// Optionally queue up the filename for automatic bitmap conversion
	// which happens after the export process has completed.
	if (do_auto_convert && export_options->ena_convert_txtr_bitmaps) {
		char *exporter_guid;

		// Determine the toolkit's output driver token (like Nt_TIFF) corresponding
		// to this output filename. If no internal toolkit driver is matched then
		// we'll use the GUID of the exporter (see next)
		Ni_Determine_Export_Image_File_Type(out_unique_name, &export_image_filetype);

		// Determine the 2D file exporter driver's GUID corresponding to the new output filename
		exporter_guid = Ni_Determine_Export_Image_File_Type_As_GUID(out_unique_name);

		NI_Exporter_AddNewTxtrFilenameNode(orig_filename, out_unique_name, export_image_filetype, exporter_guid);
	}
exit:
	/* If absolute paths are not wanted then strip off path from filename */
	if (export_options->ena_strip_abs_filepaths) {
		char    temp_buf[512];

		Ni_ExtractBaseFileName(out_unique_name, temp_buf, Nc_FALSE);
		strcpy(out_unique_name, temp_buf);
	}
	/* Strip off filepath and add new one (absolute) if enabled */
	if (export_options->specific_filepath_to_prepend) {
		char    temp_buf[512];

		Ni_ExtractBaseFileName(out_unique_name, temp_buf, Nc_FALSE);
		Ni_ConcatenatePathAndFilename(Nc_TRUE, export_options->specific_filepath_to_prepend, temp_buf, out_unique_name);
	}
	/* Strip off filepath and add new one (relative) if enabled */
	if (export_options->specific_relative_filepath_to_prepend) {
		char    temp_buf[512];

		Ni_ExtractBaseFileName(out_unique_name, temp_buf, Nc_FALSE);
		Ni_ConcatenatePathAndFilename(Nc_TRUE, export_options->specific_relative_filepath_to_prepend, temp_buf, out_unique_name);
	}

	if (export_options->convert_paths_to_rfc1738_standard && (export_options->url_string == (char *) NULL || !strlen(export_options->url_string)))
		/* Convert to RFC1738 standard (c:\file.bmp -> file:///C|/file.bmp) */
		Ni_Convert_To_RFC1738_Pathname(out_unique_name);
	else if (export_options->convert_to_unix_paths)
		/* UNIX path conversion */
		Ni_Convert_To_UNIX_Pathname(out_unique_name);

	/* URL prefix - overrides ALL 4 options above. Don't cleanup the URL prefix at all, or we'll lose http/ftp syntax */
	if (export_options->url_string != (char *) NULL) {
		char    temp_url[512], temp_base[300];

		Ni_ExtractBaseFileName(out_unique_name, temp_base, Nc_FALSE);
		strcpy(temp_url, export_options->url_string);
		if (temp_url[strlen(temp_url) - 1] != '/' && temp_url[strlen(temp_url) - 1] != '\\')
			strcat(temp_url, "/");
		strcpy(out_unique_name, temp_url);
		strcat(out_unique_name, temp_base);
	}
}

#undef TO_LOWER
#define	TO_LOWER(a)	((a >= 'A' && a <= 'Z') ? (a + 0x20) : a)

	static size_t
NI_Exporter_Case_Insensitive_Strcmp(char *s, char *t)
{
	for ( ; TO_LOWER(*s) == TO_LOWER(*t); s++, t++)
		if (*s == '\0')
			return(0);
	return(TO_LOWER(*s) - TO_LOWER(*t));
}

// ----------->>>>  Texture Image Filename Queuing Routines  <<-------------- 

// Queue up a new bitmap image filename and its path in a global list for
// subsequent automatic bitmap conversion at the end of the export phase.

	static Nd_Void
NI_Exporter_AddNewTxtrFilenameNode(char *orig_filename, char *unique_filename,
	Nd_Token export_image_filetype, char *exporter_guid)
{
	Exporter_TxtrFileName_Info *ptr, *ptr2;

	/* See if the filename exists in the list yet */
	ptr = Exporter_txtrfilename_list;
	while (ptr != NULL) {
		if (!strcmp(ptr->orig_filename, orig_filename))
			return;
		ptr = ptr->next_ptr;
	}

	ptr = (Exporter_TxtrFileName_Info *) Ni_Alloc_Memory(Nc_TRUE, Nc_TRUE, sizeof(Exporter_TxtrFileName_Info));
	ptr->next_ptr = NULL;
	/* Add the pointer to the end of the list */
	if (Exporter_txtrfilename_list == NULL)
		Exporter_txtrfilename_list = ptr;
	else {
		ptr2 = Exporter_txtrfilename_list;
		while (ptr2->next_ptr != NULL)
			ptr2 = ptr2->next_ptr;
		ptr2->next_ptr = ptr;
	}

	ptr->export_image_filetype = export_image_filetype;
	ptr->orig_filename = NULL;
	ptr->unique_filename = NULL;
	Import_IO_Copy_String(&ptr->export_converter_guid, exporter_guid);
	Import_IO_Copy_String(&ptr->orig_filename, orig_filename);
	Import_IO_Copy_String(&ptr->unique_filename, unique_filename);
	return;
}

// Free up the texture image bitmap list

	Nd_Void
NI_Exporter_Cleanup_TxtrFileName_List()
{
	Exporter_TxtrFileName_Info 	*ptr, *next;

	ptr = Exporter_txtrfilename_list;
	while (ptr != NULL) {
		if (ptr->export_converter_guid)
			Ni_Free_Memory(ptr->export_converter_guid);
		if (ptr->orig_filename != NULL)
			Ni_Free_Memory(ptr->orig_filename);
		if (ptr->unique_filename != NULL)
			Ni_Free_Memory(ptr->unique_filename);
		next = ptr->next_ptr;
		Ni_Free_Memory(ptr);
		ptr = next;
	}
}

// After the export phase has completed this routine can optionally cross
// convert 2d bitmap files from one file format to another. It is controlled
// by the options on the export options dialog box.

	Nd_Void
NI_Exporter_Convert_Bitmap_Files(char *input_filename)
{
	Exporter_TxtrFileName_Info *ptr;
	Nd_Token Saved_Confirm_Overwrites, Confirm_Overwrites;
	short	do_image_quantize, num_filenames, filename_num, count;
	// short	saved_confirm_overwrites;
	short	handles_colormapped_output, Nv_HandlesColorMappedOutput;
	char	option_str[128];
	char 		*Nv_FileFormatDescription;
	Nd_Token 	Nv_FileFormatToolkitToken;
	char		*Nv_FileFormatExtension;

	/* Count how many bitmaps there are to convert */
	num_filenames = 0;
	ptr = Exporter_txtrfilename_list;
	while (ptr != NULL) {
		++num_filenames;
		ptr = ptr->next_ptr;
	}

	/* Enable or disable overwrite confirmation */
	Ni_Inquire_Option(Nt_CONFIRM, Nt_OVERWRITES, Nt_ENABLED, &Saved_Confirm_Overwrites, Nt_CMDEND);
	if (export_options->ena_confirm_bitmap_overwrites)
		Confirm_Overwrites = Nt_ON;
	else
		Confirm_Overwrites = Nt_OFF;
	Ni_Option(Nt_CONFIRM, Nt_OVERWRITES, Nt_ENABLED, Confirm_Overwrites, Nt_CMDEND);

	if (Export_IO_Check_For_User_Interrupt_With_Stats(0, 0))
		return;

	filename_num = 0;
	ptr = Exporter_txtrfilename_list;
	while (ptr != NULL) {
		/* Convert the non-supported file formats */
		do_image_quantize = (export_options->txtr_bitmap_depth > 0 && export_options->txtr_bitmap_depth <= 8);

		/* Determine if the output driver handles color maps */
		count = 0;
		handles_colormapped_output = Nc_FALSE;
		while (Nc_TRUE) {
			char		*Nv_FileFormatGUID;

// v4.1.9 (Oct 2004) of the toolkit allows for custom 2D file exporter's to be defined via a GUID, so
// this change will allow for exporters to be chosen via their extension (.tif) or GUID.
#if 00
			Ni_EnumerateSupportedExportBitmapFileFormats(count, &Nv_FileFormatDescription, &Nv_FileFormatToolkitToken, &Nv_FileFormatExtension, &Nv_HandlesColorMappedOutput);
#else
			Ni_EnumerateSupportedExportBitmapFileFormats2(Nc_TRUE, count, &Nv_FileFormatDescription, &Nv_FileFormatToolkitToken, &Nv_FileFormatExtension, &Nv_HandlesColorMappedOutput, &Nv_FileFormatGUID);
#endif

			if (!Nv_FileFormatDescription)
				break;

			if (Nv_FileFormatGUID && !stricmp(Nv_FileFormatGUID, ptr->export_converter_guid)) {
				handles_colormapped_output = Nv_HandlesColorMappedOutput;
				break;
			} if (Nv_FileFormatToolkitToken && !strcmp(ptr->export_image_filetype, Nv_FileFormatToolkitToken)) {
				handles_colormapped_output = Nv_HandlesColorMappedOutput;
				break;
			}
			++count;
		}

		if (do_image_quantize && handles_colormapped_output)
			sprintf(option_str, "colormaplength = %d", export_options->txtr_bitmap_depth);
		else
			option_str[0] = '\0';

		if (ptr->export_converter_guid) {
			char	*Nv_File_Extension_List;
			char	file_extension[256];

			// Get the standard file extension used by this 2D file exporter
			Ni_Inquire_FileFormatHandler(ptr->export_converter_guid,
				Nt_FILETYPES, (char **) &Nv_File_Extension_List, Nt_CMDEND);
			if (Nv_File_Extension_List)
				strcpy(file_extension, Nv_File_Extension_List);
			else
				strcpy(file_extension, ".tif");	// Should not happen

			// Choose the exporter based on its GUID
			Ni_Convert_Image_File_Format(ptr->orig_filename, ptr->unique_filename, export_options->export_directory, ptr->export_converter_guid, file_extension, option_str, Nc_TRUE, export_options->txtr_x_res, export_options->txtr_y_res, do_image_quantize, Nt_CMDEND);
		} else
			// Choose the exporter based on its file extension
			Ni_Convert_Image_File_Format(ptr->orig_filename, ptr->unique_filename, export_options->export_directory, ptr->export_image_filetype, export_options->autoconvert_bitmap_format, option_str, Nc_TRUE, export_options->txtr_x_res, export_options->txtr_y_res, do_image_quantize, Nt_CMDEND);

		// Check for user abort
		if (num_filenames) {
			if (Export_IO_Check_For_User_Interrupt_With_Stats(filename_num, num_filenames))
				return;
		}

		ptr = ptr->next_ptr;
	}

	/* Restore overwrite confirmation */
	Ni_Option(Nt_CONFIRM, Nt_OVERWRITES, Nt_ENABLED, Saved_Confirm_Overwrites, Nt_CMDEND);
}

// ----------->>>>  Embed Texture Image Within the Output File  <<-------------- 

// Some file formats, such as Strata StudioPro, Viewpoint VET (Metastream),
// and VRML 1.0/2.0 allow bitmap images to be directly embedded inside the file.
// The following code shows how to take a texture bitmap reference, load the
// image into memory, optionally resize it to be a power-of-two, and have
// the image embedded within the output file.

#if 0
	static Nd_Void
NI_Exporter_EmbedTextureImageInOutputFile(char *bitmap_filename)
{
	short           rescale_image, rescaled_x_res, rescaled_y_res;
	char		*filetype_guid;
	CPB_Offscreen   *off;
	PB_Pixel32      pixelToSet; 
	META_Texture_List *ptr;

	if (TRUE) {
		char    filepath_used[300];
		FILE    *tmp_file;
		Nd_Image_Load_Info load_info;
		Nd_Int  status, y, row, col;
		short   new_xRes, new_yRes;
		unsigned char   *ptr, *r_ptr, *g_ptr, *b_ptr, *a_ptr, *image_buf;
		unsigned char   *new_r_ptr, *new_g_ptr, *new_b_ptr, *new_a_ptr;

		// First make sure this is a valid texture, and that it can be found
		if ((filetype_guid = NI_Determine_File_Type_As_GUID(bitmap_filename)) == NULL) {
			// %s: The bitmap file format of image file '%s' is not supported.
			Ni_Report_Error(Nc_ERR_ERROR, "ERR_EXPORT_GENERIC_EMBED2", CURR_FN_NAME, fileName);
			goto exit;
		}
			
		if ((tmp_file = NI_Search_Open_File(fileName, READ_FILE_MODE, FALSE, "NG_SEARCHPATH_TEXTURES", filepath_used)) == (FILE *) NULL) {
			// %s: Could not locate or open %s file '%s' for reading. 
			Ni_Report_Error(Nc_ERR_ERROR, "ERR_FILE_OPEN_READ", CURR_FN_NAME, "texture image", fileName);
			goto exit;
		}
		if (tmp_file)
			Ni_Fclose_File(tmp_file);
						
		Ni_Image_Load(&load_info, "NG_WORKING_DIR", &status,
			Nt_READHEADER, Nt_CUSTOM, filetype_guid, filepath_used,
			Nt_CMDEND);
		if (status == Nc_TRUE) {
			/* %s: Error while reading %s from file '%s'. */
			Ni_Report_Error(Nc_ERR_ERROR, "ERR_FILE_READ", CURR_FN_NAME, "texture image", filepath_used);
			goto exit;
		}

		rescale_image = TRUE;
		if (adding_as_lightmap) {
			// Always make lightmaps square and 256x256
			rescaled_x_res = 256;
			rescaled_y_res = 256;
		} else {
			rescaled_x_res = UIopts.txtr_x_res;
			rescaled_y_res = UIopts.txtr_y_res;
		}

		/* If the image is to be rescaled then read it into memory and rescale it */
		if (rescale_image) {
			if (rescaled_x_res == 1 && rescaled_y_res == 1)
				/* No change requested */
				goto skip;
			if (rescaled_x_res == load_info.xRes && rescaled_y_res == load_info.yRes)
				/* No scaling necessary */
				goto skip;

			if (!rescaled_x_res) {
				/* Choose next power of 2 */
				new_xRes = 1;
				while (new_xRes < load_info.xRes)
					new_xRes *= 2;
			} else if (rescaled_x_res == 1) {
				/* No scaling wanted */
				new_xRes = load_info.xRes;
			} else 
				new_xRes = rescaled_x_res;

			if (!rescaled_y_res) {
				/* Choose next power of 2 */
				new_yRes = 1;
				while (new_yRes < load_info.yRes)
					new_yRes *= 2;
			} else if (rescaled_y_res == 1) {
				/* No scaling wanted */
				new_yRes = load_info.yRes;
			} else
				new_yRes = rescaled_y_res;

			ptr = image_buf = (unsigned char *) NI_Alloc_Mem(CURR_FN_NAME, FALSE, TRUE, load_info.xRes * load_info.yRes * load_info.zRes);
			if (image_buf == NULL) 
				goto error;

			r_ptr = ptr;
			g_ptr = r_ptr + (load_info.xRes * load_info.yRes);
			b_ptr = g_ptr + (load_info.xRes * load_info.yRes);
			a_ptr = b_ptr + (load_info.xRes * load_info.yRes);


// !! NOTE !!
// 'load_info.image_orientation' is Nc_ORIGIN_UPPERLEFT or Nc_ORIGIN_LOWERLEFT
//
// Make sure you write your bitmap manipulation code to respect the row order!
//

			/* Read the image data into memory */
			for (y = 0; y < load_info.yRes; ++y) {
				Ni_Image_Load(&load_info, (char *) NULL, &status, Nt_READSCANLINE, Nt_CMDEND);
				if (status == Nc_TRUE) {
					free(image_buf);
					goto exit;
				}
				memcpy(r_ptr, load_info.r_ptr, load_info.xRes);
				r_ptr += load_info.xRes;
				if (load_info.zRes >= 3) {
					memcpy(g_ptr, load_info.g_ptr, load_info.xRes);
					g_ptr += load_info.xRes;
					memcpy(b_ptr, load_info.b_ptr, load_info.xRes);
					b_ptr += load_info.xRes;
				}
				if (load_info.zRes >= 4) {
					memcpy(a_ptr, load_info.a_ptr, load_info.xRes);
					a_ptr += load_info.xRes;
				}
			} 

			/* Reinitialize the pointers */
			r_ptr = ptr;
			g_ptr = r_ptr + (load_info.xRes * load_info.yRes);
			b_ptr = g_ptr + (load_info.xRes * load_info.yRes);
			a_ptr = b_ptr + (load_info.xRes * load_info.yRes);

			/* %s: Rescaling image file '%s'from resolution (%d, %d) to (%d, %d) */
	                Ni_Report_Error(Nc_ERR_INFO, "INFO_IMG_RESCALING", CURR_FN_NAME, filepath_used, load_info.xRes, load_info.yRes, new_xRes, new_yRes);

			/* Go rescale each channel of the image */
			new_r_ptr = new_g_ptr = new_b_ptr = new_a_ptr = NULL;
			if ((r_ptr = new_r_ptr = Ni_Image_Scale(r_ptr, load_info.xRes, load_info.yRes, 1, NULL, new_xRes, new_yRes)) == (Nd_UChar *) NULL) {
				free(image_buf);
				goto error;
			}
			if (load_info.zRes >= 3) {
				if ((g_ptr = new_g_ptr = Ni_Image_Scale(g_ptr, load_info.xRes, load_info.yRes, 1, NULL, new_xRes, new_yRes)) == (Nd_UChar *) NULL) {
					free(image_buf);
					free(new_r_ptr);
					goto error;
				}
				if ((b_ptr = new_b_ptr = Ni_Image_Scale(b_ptr, load_info.xRes, load_info.yRes, 1, NULL, new_xRes, new_yRes)) == (Nd_UChar *) NULL) {
					free(new_r_ptr);
					free(new_g_ptr);
					free(image_buf);
					goto error;
				}
			}

			if (load_info.zRes >= 4) {
				if ((a_ptr = new_a_ptr = Ni_Image_Scale(a_ptr, load_info.xRes, load_info.yRes, 1, NULL, new_xRes, new_yRes)) == (Nd_UChar *) NULL) {
					free(image_buf);
					free(new_r_ptr);
					if (new_g_ptr != NULL)
						free(new_g_ptr);
					if (new_b_ptr != NULL)
						free(new_b_ptr);
					goto error;
				}
			}
			free(image_buf);

			// Copy the bitmap into a Metastream offscreen bitmap
//			off = new CPB_Offscreen(new_yRes, new_xRes, 32, load_info.image_orientation == Nc_ORIGIN_UPPERLEFT);
			off = new CPB_Offscreen(new_xRes, new_yRes, 32, TRUE);
			if (off == NULL) {
				free(new_r_ptr);
				if (new_g_ptr != NULL)
					free(new_g_ptr);
				if (new_b_ptr != NULL)
					free(new_b_ptr);
				if (new_a_ptr != NULL)
					free(new_a_ptr);
				goto error;
			}

			for (uint32 row = 0; row < new_yRes; row++) {
				for (uint32 col = 0; col < new_xRes; col++) {
					pixelToSet.r = r_ptr[col];
					if (g_ptr && load_info.zRes >= 2)
						pixelToSet.g = g_ptr[col];
					else
						pixelToSet.g = pixelToSet.r;
					if (b_ptr && load_info.zRes >= 3)
						pixelToSet.b = b_ptr[col];
					else
						pixelToSet.b = pixelToSet.r;
					if (a_ptr && load_info.zRes >= 4)
						pixelToSet.a = a_ptr[col];
					else
						pixelToSet.a = 1.0;
					if (intensity_multiplier != 1.0) {
						pixelToSet.r = ((float) pixelToSet.r * intensity_multiplier);
						pixelToSet.g = ((float) pixelToSet.g * intensity_multiplier);
						pixelToSet.b = ((float) pixelToSet.b * intensity_multiplier);
					}

					if (load_info.image_orientation == Nc_ORIGIN_UPPERLEFT)
						off->SetPixel(col, row, pixelToSet);
					else
						off->SetPixel(col, new_yRes-row-1, pixelToSet);
				}
				if (r_ptr)
					r_ptr += new_xRes;
				if (g_ptr)
					g_ptr += new_xRes;
				if (b_ptr)
					b_ptr += new_xRes;
				if (a_ptr)
					a_ptr += new_xRes;
			}

			if (new_r_ptr != NULL)
				free(new_r_ptr);
			if (new_g_ptr != NULL)
				free(new_g_ptr);
			if (new_b_ptr != NULL)
				free(new_b_ptr);
			if (new_a_ptr != NULL)
				free(new_a_ptr);

			goto done;      /* We are done */
		}

		// Copy the texture into an off-screen bitmap
skip:           // off = new CPB_Offscreen(load_info.yRes, load_info.xRes, 32, load_info.image_orientation == Nc_ORIGIN_UPPERLEFT);
		off = new CPB_Offscreen(load_info.xRes, load_info.yRes, 32, TRUE);
		if (off == NULL)
			goto error;

		/* Load the texture in here */
		ptr = image_buf = (unsigned char *) NI_Alloc_Mem(CURR_FN_NAME, FALSE, TRUE, load_info.xRes * load_info.yRes * load_info.zRes);
		if (image_buf == NULL) 
			goto error;

		r_ptr = ptr;
		g_ptr = r_ptr + (load_info.xRes * load_info.yRes);
		b_ptr = g_ptr + (load_info.xRes * load_info.yRes);
		a_ptr = b_ptr + (load_info.xRes * load_info.yRes);

		/* Read the image data into memory */
		for (y = 0; y < load_info.yRes; ++y) {
			Ni_Image_Load(&load_info, (char *) NULL, &status, Nt_READSCANLINE, Nt_CMDEND);
			if (status == Nc_TRUE) {
				free(image_buf);
				goto exit;
			}
			memcpy(r_ptr, load_info.r_ptr, load_info.xRes);
			r_ptr += load_info.xRes;
			if (load_info.zRes >= 3) {
				memcpy(g_ptr, load_info.g_ptr, load_info.xRes);
				g_ptr += load_info.xRes;
				memcpy(b_ptr, load_info.b_ptr, load_info.xRes);
				b_ptr += load_info.xRes;
			}
			if (load_info.zRes >= 4) {
				memcpy(a_ptr, load_info.a_ptr, load_info.xRes);
				a_ptr += load_info.xRes;
			}
		} 

		/* Reinitialize the pointers */
		r_ptr = ptr;
		g_ptr = r_ptr + (load_info.xRes * load_info.yRes);
		b_ptr = g_ptr + (load_info.xRes * load_info.yRes);
		a_ptr = b_ptr + (load_info.xRes * load_info.yRes);
	
		for (row = 0; row < load_info.yRes; row++) {
			for (col = 0; col < load_info.xRes; col++) {
				pixelToSet.r = r_ptr[col];
				if (g_ptr && load_info.zRes >= 2)
					pixelToSet.g = g_ptr[col];
				else
					pixelToSet.g = pixelToSet.r;
				if (b_ptr && load_info.zRes >= 3)
					pixelToSet.b = b_ptr[col];
				else
					pixelToSet.b = pixelToSet.r;
				if (a_ptr && load_info.zRes >= 4)
					pixelToSet.a = a_ptr[col];
				else
					pixelToSet.a = 1.0;
				if (intensity_multiplier != 1.0) {
					pixelToSet.r = ((float) pixelToSet.r * intensity_multiplier);
					pixelToSet.g = ((float) pixelToSet.g * intensity_multiplier);
					pixelToSet.b = ((float) pixelToSet.b * intensity_multiplier);
				}
				if (load_info.image_orientation == Nc_ORIGIN_UPPERLEFT)
					off->SetPixel(col, row, pixelToSet);
				else
					off->SetPixel(col, load_info.yRes-row-1, pixelToSet);
			}
			if (r_ptr)
				r_ptr += load_info.xRes;
			if (g_ptr)
				g_ptr += load_info.xRes;
			if (b_ptr)
				b_ptr += load_info.xRes;
			if (a_ptr)
				a_ptr += load_info.xRes;
		}

done:           texture = new MTS_RefObj<MTS_Texture>;
		if (texture) {
			META_Texture_List *ptr, *ptr2;

			texture->SetTexture(off, false);
			texture->SetShared(TRUE);
			// texture->SetName(offPath);

			ptr = (META_Texture_List *) Ni_Alloc_Memory(Nc_TRUE, Nc_TRUE, sizeof(META_Texture_List));

			// Add node to the end of the list
			if (meta_texture_list == NULL) {
				ptr->next_ptr = NULL;
				meta_texture_list = ptr;
			} else {
				ptr2 = meta_texture_list;
				while (ptr2 != NULL && ptr2->next_ptr != NULL)
					ptr2 = ptr2->next_ptr;
				ptr2->next_ptr= ptr;
				ptr->next_ptr = NULL;
			}
			ptr->textID = textID;
			ptr->texture = texture;
			ptr->intensity_multiplier = intensity_multiplier;
		}

		/* Close the input file */
error:          Ni_Image_Load(&load_info, (char *) NULL, &status, Nt_CLOSE, Nt_CMDEND);
	} 

exit:   return(texture);
}
#endif
