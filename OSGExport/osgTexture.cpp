
#include "main.h"
#include "osgTexture.h"

#include <osg/Texture2D>
#include <string>
#include <map>


typedef std::map< std::string, osg::ref_ptr< osg::Texture2D > > TextureMap;
TextureMap _textureMap;


osg::Texture2D*
lookupTexture( const std::string& name )
{
    TextureMap::const_iterator it = _textureMap.find( name );
    if (it != _textureMap.end())
        return (*it).second.get();
    else
        return NULL;
}


Nd_Int
createTextureCB( Nd_Enumerate_Callback_Info *cbi_ptr )
{
	Export_IO_TextureParameters	txtr_info;
	Nd_Matrix			Nv_Texture_Matrix;
	char				*texture_defn_name = cbi_ptr->Nv_Handle_Name1;


	if (cbi_ptr->Nv_Matches_Made == 0)
		return(Nc_FALSE);

	// Names prefixed with "NUGRAF___" are used internally in the
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

	Export_IO_Inquire_Texture( texture_defn_name, &txtr_info );


    std::string nameStr( texture_defn_name );
    osg::ref_ptr< osg::Texture2D > tex = new osg::Texture2D;
    _textureMap[ nameStr ] = tex.get();

    tex->setName( nameStr );


#if 0
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
#endif
	return(Nc_FALSE);	/* Do not terminate the enumeration */
}
