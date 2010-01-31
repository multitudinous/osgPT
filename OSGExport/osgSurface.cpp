
#include "main.h"
#include "osgSurface.h"

#include <osg/Material>
#include <osg/Texture2D>
#include <osgDB/FileNameUtils>
#include <string>
#include <map>


// Forward declarations
static Nd_Int surfaceTextureCB( Nd_Enumerate_Callback_Info *cbi_ptr );



typedef std::map< std::string, osg::ref_ptr< osg::Texture2D > > TextureMap;
TextureMap _textureMap;

typedef std::map< std::string, SurfaceInfo > SurfaceMap;
SurfaceMap _surfaceMap;



osg::Texture2D*
lookupTexture( const std::string& name )
{
    TextureMap::const_iterator it = _textureMap.find( name );
    if (it != _textureMap.end())
        return (*it).second.get();
    else
        return NULL;
}

const SurfaceInfo&
lookupSurface( const std::string& name )
{
    SurfaceMap::const_iterator it = _surfaceMap.find( name );
    if (it != _surfaceMap.end())
    {
        const SurfaceInfo& si = (*it).second;
        return si;
    }
    else
    {
        Ni_Report_Error_printf( Nc_ERR_RAW_MSG, "lookupSurface: Could not find %s\n", name );
        return( *(new SurfaceInfo) );
    }
}



Nd_Int
createMaterialCB( Nd_Enumerate_Callback_Info *cbi_ptr )
{
	Export_IO_SurfaceParameters 	surf_info;
	Nd_Int				dummy;
	Nd_ConstString				surface_name = cbi_ptr->Nv_Handle_Name1;

	if (cbi_ptr->Nv_Matches_Made == 0)
		return(Nc_FALSE);

	// Camera names prefixed with "NUGRAF___" are used internally in the
	// Okino PolyTrans & NuGraf user interface. Ignore them. 
	if (!strncmp(surface_name, "NUGRAF___", 9))
		return(Nc_FALSE);

	// Update the status display with the current surface definition being exported
	Export_IO_UpdateStatusDisplay("surface", (char*)surface_name, "Exporting surface definitions (materials)."); 

	// And check for user abort
	if (Export_IO_Check_For_User_Interrupt_With_Stats(cbi_ptr->Nv_Call_Count, cbi_ptr->Nv_Matches_Made))
		return(Nc_TRUE);	// Abort the enumeration (nothing gets returned from the Ni_Enumerate() function

	// Inquire about all of the surface parameters 
	Export_IO_Inquire_Surface( (char*) surface_name, &surf_info);


    SurfaceInfo si;
    si._faceAlpha = surf_info.Nv_Opacity_FaceCoeff;
    si._reflectAlpha = surf_info.Nv_Opacity_ReflectCoeff;

    std::string nameStr( surface_name );
    osg::ref_ptr< osg::Material > mat = new osg::Material;

    si._mat = mat.get();

    mat->setName( nameStr );

    osg::Vec3 diffuse( surf_info.Nv_Surface_Color[Na_RED], surf_info.Nv_Surface_Color[Na_GREEN], 
		    surf_info.Nv_Surface_Color[Na_BLUE] );

    osg::Vec3 ambient;
    if ( !strncmp( surf_info.Nv_AmbientColorToggle, "on", 2) )
    {
        ambient.set( surf_info.Nv_Ambient_Color[Na_RED], surf_info.Nv_Ambient_Color[Na_GREEN], 
		    surf_info.Nv_Ambient_Color[Na_BLUE] );
    }
    else
    {
        ambient = diffuse;
    }
    mat->setAmbient( osg::Material::FRONT_AND_BACK,
        osg::Vec4( ambient * surf_info.Nv_Ambient_Coeff, 1. ) );

    mat->setDiffuse( osg::Material::FRONT_AND_BACK,
        osg::Vec4( diffuse * surf_info.Nv_Diffuse_Coeff, 1. ) );

    osg::Vec3 emissive;
	if ( !strncmp( surf_info.Nv_LuminousColorToggle, "on", 2) )
    {
        emissive.set( surf_info.Nv_Luminous_Color[Na_RED], surf_info.Nv_Luminous_Color[Na_GREEN], 
		    surf_info.Nv_Luminous_Color[Na_BLUE] );
    }
    else
    {
        emissive = diffuse;
    }
    mat->setEmission( osg::Material::FRONT_AND_BACK,
        osg::Vec4( emissive * surf_info.Nv_Luminous_Coeff, 1. ) );

	if (surf_info.Nv_Specular_Model == Nt_PHONG)
    {
        mat->setShininess( osg::Material::FRONT_AND_BACK, surf_info.Nv_Phong_Shininess );
		//OPTIONAL_FPRINTF(ofp, "Phong. Shininess = %ld, metal = %g\n",
		//	surf_info.Nv_Phong_Shininess, surf_info.Nv_Metal_Coefficient);
    }
	else {
        mat->setShininess( osg::Material::FRONT_AND_BACK, 10. );
		//OPTIONAL_FPRINTF(ofp, "Blinn.\n");
		//OPTIONAL_FPRINTF(ofp, "\t\t'c3' = %g\n", surf_info.Nv_C3_Coefficient);
		//OPTIONAL_FPRINTF(ofp, "\t\tIndex of refraction = %g\n", surf_info.Nv_Index_of_Refraction);
		//OPTIONAL_FPRINTF(ofp, "\t\tMetal = %g\n", surf_info.Nv_Metal_Coefficient);
	}		

    osg::Vec3 specular;
	if ( !strncmp( surf_info.Nv_SpecularColorToggle, "on", 2) )
    {
        specular.set( surf_info.Nv_Specular_Color[Na_RED], surf_info.Nv_Specular_Color[Na_GREEN], 
		    surf_info.Nv_Specular_Color[Na_BLUE] );
    }
    else
    {
        specular = diffuse;
    }
    mat->setSpecular( osg::Material::FRONT_AND_BACK,
        osg::Vec4( specular * surf_info.Nv_Specular_Coeff, 1. ) );

	// Enumerate texture definitions associated with this surface.
	Ni_Enumerate( &dummy, cbi_ptr->Nv_Handle_Name1, Nc_FALSE, 
		(Nd_Void *) &si, (Nd_Void *) 0, surfaceTextureCB, 
		Nt_SURFACE, Nt_TEXTURE, Nt_CMDEND);

    _surfaceMap[ nameStr ] = si;

	return(Nc_FALSE);	/* Do not terminate the enumeration */
}




Nd_Int
createTextureCB( Nd_Enumerate_Callback_Info *cbi_ptr )
{
	Export_IO_TextureParameters	txtr_info;
	//Nd_Matrix			Nv_Texture_Matrix;
	Nd_ConstString texture_defn_name = cbi_ptr->Nv_Handle_Name1;


	if (cbi_ptr->Nv_Matches_Made == 0)
		return(Nc_FALSE);

	// Names prefixed with "NUGRAF___" are used internally in the
	// Okino PolyTrans & NuGraf user interface. Ignore them. 
	if (!strncmp(texture_defn_name, "NUGRAF___", 9))
		return(Nc_FALSE);

	// Update the stats
	++export_options->total_texture_maps;

	// Update the status display with the current texture definition being exported
	Export_IO_UpdateStatusDisplay("texture", (char*)texture_defn_name, "Exporting texture definitions."); 

	// And check for user abort
	if (Export_IO_Check_For_User_Interrupt_With_Stats(cbi_ptr->Nv_Call_Count, cbi_ptr->Nv_Matches_Made))
		return(Nc_TRUE);	// Abort the enumeration (nothing gets returned from the Ni_Enumerate() function

	Export_IO_Inquire_Texture( (char*)texture_defn_name, &txtr_info );


    std::string nameStr( texture_defn_name );
    osg::ref_ptr< osg::Texture2D > tex = new osg::Texture2D;
    _textureMap[ nameStr ] = tex.get();

    tex->setName( nameStr );


	if (txtr_info.Nv_Image_Filename != (char *) 0)
    {
        std::string origName( txtr_info.Nv_Image_Filename );
        std::string outName;
        if (export_options->osgStripTexturePaths)
            outName = osgDB::getSimpleFileName( origName );
        else
            outName = origName;

        osg::Image* image = new osg::Image;
        image->setFileName( outName );
        tex->setImage( image );
	}

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


static Nd_Int
surfaceTextureCB( Nd_Enumerate_Callback_Info *cbi_ptr )
{
	Export_IO_SurfTxtrParameters	surftxtr_info;
	Nd_ConstString surface_name = cbi_ptr->Nv_Handle_Name1;
	Nd_ConstString texture_layer_name = cbi_ptr->Nv_Handle_Name2;

	SurfaceInfo* si = (SurfaceInfo*) cbi_ptr->Nv_User_Data_Ptr1;	// Pick up user-defined parameter #1; we're passing in the ASCII file output handle

	if (cbi_ptr->Nv_Matches_Made == 0)
		return(Nc_FALSE);

	if (cbi_ptr->Nv_Call_Count == 0)
        return(Nc_FALSE);

	Export_IO_Inquire_SurfaceTextureLayerParameters( (char*)surface_name, (char*)texture_layer_name, &surftxtr_info);

    si->_tex = lookupTexture( surftxtr_info.Nv_Texture_Texturelink_Name );

#if 0
	Export_IO_Inquire_SurfaceTextureLayerParameters(surface_name, texture_layer_name, &surftxtr_info);

	/* Next, we'll print out the values returned from the surface */
	/* texture inquiry function to show what can be done with them. */

	OPTIONAL_FPRINTF(ofp, "\t\tTexture layer name: '%s'\n", texture_layer_name);
	OPTIONAL_FPRINTF(ofp, "\t\t\tEnabled = '%s'\n", surftxtr_info.Nv_TextureToggle);
	OPTIONAL_FPRINTF(ofp, "\t\t\tTexture definition linked to this layer = '%s'\n", surftxtr_info.Nv_Texture_Texturelink_Name);
	OPTIONAL_FPRINTF(ofp, "\t\t\tTexture modulation parameters:\n");
		OPTIONAL_FPRINTF(ofp, "\t\t\t\tDiffuse color blending mask source is '%s'\n", 
			surftxtr_info.Nv_DiffuseColorBlendMask);
		OPTIONAL_FPRINTF(ofp, "\t\t\t\tAmbient color blending mask source is '%s'\n", 
			surftxtr_info.Nv_AmbientColorBlendMask);
		OPTIONAL_FPRINTF(ofp, "\t\t\t\tLuminous color blending mask source is '%s'\n", 
			surftxtr_info.Nv_LuminousColorBlendMask);
		OPTIONAL_FPRINTF(ofp, "\t\t\t\tSpecular color blending mask source is '%s'\n", 
			surftxtr_info.Nv_SpecularColorBlendMask);
		OPTIONAL_FPRINTF(ofp, "\t\t\t\tDiffuse surface color modulation toggle is '%s'\n", 
			surftxtr_info.Nv_ModulateDiffuseColorToggle);
		OPTIONAL_FPRINTF(ofp, "\t\t\t\tAmbient surface color modulation toggle is '%s'\n", 
			surftxtr_info.Nv_ModulateAmbientColorToggle);
		OPTIONAL_FPRINTF(ofp, "\t\t\t\tLuminous surface color modulation toggle is '%s'\n", 
			surftxtr_info.Nv_ModulateLuminousColorToggle);
		OPTIONAL_FPRINTF(ofp, "\t\t\t\tSpecular surface color modulation toggle is '%s'\n", 
			surftxtr_info.Nv_ModulateSpecularColorToggle);
		OPTIONAL_FPRINTF(ofp, "\t\t\t\tBump mapping modulation toggle is '%s'\n", 
			surftxtr_info.Nv_ModulateBumpToggle);
		if (surftxtr_info.Nv_ModulateFaceOpacityInvertToggle == Nt_OFF)
			OPTIONAL_FPRINTF(ofp, "\t\t\t\tFace opacity modulation toggle is '%s'\n", 
				surftxtr_info.Nv_ModulateFaceOpacityToggle);
		else
			OPTIONAL_FPRINTF(ofp, "\t\t\t\tFace transparency modulation toggle is '%s'\n", 
				surftxtr_info.Nv_ModulateFaceOpacityToggle);
		if (surftxtr_info.Nv_ModulateReflectOpacityInvertToggle == Nt_OFF)
			OPTIONAL_FPRINTF(ofp, "\t\t\t\tReflect opacity modulation toggle is '%s'\n", 
				surftxtr_info.Nv_ModulateReflectOpacityToggle);
		else
			OPTIONAL_FPRINTF(ofp, "\t\t\t\tReflect transparency modulation toggle is '%s'\n", 
				surftxtr_info.Nv_ModulateReflectOpacityToggle);
		OPTIONAL_FPRINTF(ofp, "\t\t\t\tDiffuse coefficient mapping modulation toggle is '%s'\n", 
			surftxtr_info.Nv_ModulateDiffuseCoeffToggle);
		OPTIONAL_FPRINTF(ofp, "\t\t\t\tAmbient coefficient mapping modulation toggle is '%s'\n", 
			surftxtr_info.Nv_ModulateAmbientCoeffToggle);
		OPTIONAL_FPRINTF(ofp, "\t\t\t\tLuminous coefficient mapping modulation toggle is '%s'\n", 
			surftxtr_info.Nv_ModulateLuminousCoeffToggle);
		OPTIONAL_FPRINTF(ofp, "\t\t\t\tSpecular coefficient mapping modulation toggle is '%s'\n", 
			surftxtr_info.Nv_ModulateSpecularCoeffToggle);
		OPTIONAL_FPRINTF(ofp, "\t\t\t\tReflected coefficient mapping modulation toggle is '%s'\n", 
			surftxtr_info.Nv_ModulateReflectCoeffToggle);
		OPTIONAL_FPRINTF(ofp, "\t\t\t\tDiffuse color mix with previous layer = %g\n", surftxtr_info.Nv_DiffuseColorBlend);
		OPTIONAL_FPRINTF(ofp, "\t\t\t\tAmbient color mix with previous layer = %g\n", surftxtr_info.Nv_AmbientColorBlend);
		OPTIONAL_FPRINTF(ofp, "\t\t\t\tLuminous color mix with previous layer = %g\n", surftxtr_info.Nv_LuminousColorBlend);
		OPTIONAL_FPRINTF(ofp, "\t\t\t\tSpecular color mix with previous layer = %g\n", surftxtr_info.Nv_SpecularColorBlend);
		OPTIONAL_FPRINTF(ofp, "\t\t\t\tFace opacity mix with previous layer = %g\n", surftxtr_info.Nv_Face_Opacity_Mix);
		OPTIONAL_FPRINTF(ofp, "\t\t\t\tReflect opacity mix with previous layer = %g\n",
			surftxtr_info.Nv_Reflect_Opacity_Mix);
		OPTIONAL_FPRINTF(ofp, "\t\t\t\tDiffuse color R/G/B texture multiplier = (%g,%g,%g)\n", 
			surftxtr_info.Nv_DiffuseColor_Multiplier[0], surftxtr_info.Nv_DiffuseColor_Multiplier[1], surftxtr_info.Nv_DiffuseColor_Multiplier[2]);
		OPTIONAL_FPRINTF(ofp, "\t\t\t\tDiffuse color R/G/B texture bias = (%g,%g,%g)\n", 
			surftxtr_info.Nv_DiffuseColor_Bias[0], surftxtr_info.Nv_DiffuseColor_Bias[1], surftxtr_info.Nv_DiffuseColor_Bias[2]);
		OPTIONAL_FPRINTF(ofp, "\t\t\t\tAmbient color R/G/B texture multiplier = (%g,%g,%g)\n", 
			surftxtr_info.Nv_AmbientColor_Multiplier[0], surftxtr_info.Nv_AmbientColor_Multiplier[1], surftxtr_info.Nv_AmbientColor_Multiplier[2]);
		OPTIONAL_FPRINTF(ofp, "\t\t\t\tAmbient color R/G/B texture bias = (%g,%g,%g)\n", 
			surftxtr_info.Nv_AmbientColor_Bias[0], surftxtr_info.Nv_AmbientColor_Bias[1], surftxtr_info.Nv_AmbientColor_Bias[2]);
		OPTIONAL_FPRINTF(ofp, "\t\t\t\tLuminous color R/G/B texture multiplier = (%g,%g,%g)\n", 
			surftxtr_info.Nv_LuminousColor_Multiplier[0], surftxtr_info.Nv_LuminousColor_Multiplier[1], surftxtr_info.Nv_LuminousColor_Multiplier[2]);
		OPTIONAL_FPRINTF(ofp, "\t\t\t\tLuminous color R/G/B texture bias = (%g,%g,%g)\n", 
			surftxtr_info.Nv_LuminousColor_Bias[0], surftxtr_info.Nv_LuminousColor_Bias[1], surftxtr_info.Nv_LuminousColor_Bias[2]);
		OPTIONAL_FPRINTF(ofp, "\t\t\t\tSpecular color R/G/B texture multiplier = (%g,%g,%g)\n", 
			surftxtr_info.Nv_SpecularColor_Multiplier[0], surftxtr_info.Nv_SpecularColor_Multiplier[1], surftxtr_info.Nv_SpecularColor_Multiplier[2]);
		OPTIONAL_FPRINTF(ofp, "\t\t\t\tSpecular color R/G/B texture bias = (%g,%g,%g)\n", 
			surftxtr_info.Nv_SpecularColor_Bias[0], surftxtr_info.Nv_SpecularColor_Bias[1], surftxtr_info.Nv_SpecularColor_Bias[2]);
		OPTIONAL_FPRINTF(ofp, "\t\t\t\tBump mapping U texture multiplier = %g\n",
			surftxtr_info.Nv_Bump_U_Multiplier);
		OPTIONAL_FPRINTF(ofp, "\t\t\t\tBump mapping U texture bias = %g\n",
			surftxtr_info.Nv_Bump_U_Bias);
		OPTIONAL_FPRINTF(ofp, "\t\t\t\tBump mapping V texture multiplier = %g\n",
			surftxtr_info.Nv_Bump_V_Multiplier);
		OPTIONAL_FPRINTF(ofp, "\t\t\t\tBump mapping V texture bias = %g\n",
			surftxtr_info.Nv_Bump_V_Bias);
		OPTIONAL_FPRINTF(ofp, "\t\t\t\tFace opacity texture multiplier = %g\n",
			surftxtr_info.Nv_Face_Opacity_Multiplier);
		OPTIONAL_FPRINTF(ofp, "\t\t\t\tFace opacity texture bias = %g\n",
			surftxtr_info.Nv_Face_Opacity_Bias);
		OPTIONAL_FPRINTF(ofp, "\t\t\t\tReflect opacity texture multiplier = %g\n",
			surftxtr_info.Nv_Reflect_Opacity_Multiplier);
		OPTIONAL_FPRINTF(ofp, "\t\t\t\tReflect opacity texture bias = %g\n",
			surftxtr_info.Nv_Reflect_Opacity_Bias);
		OPTIONAL_FPRINTF(ofp, "\t\t\t\tDiffuse coefficient texture multiplier = %g\n",
			surftxtr_info.Nv_DiffuseCoeff_Multiplier);
		OPTIONAL_FPRINTF(ofp, "\t\t\t\tDiffuse coefficient texture bias = %g\n",
			surftxtr_info.Nv_DiffuseCoeff_Bias);
		OPTIONAL_FPRINTF(ofp, "\t\t\t\tAmbient coefficient texture multiplier = %g\n",
			surftxtr_info.Nv_AmbientCoeff_Multiplier);
		OPTIONAL_FPRINTF(ofp, "\t\t\t\tAmbient coefficient texture bias = %g\n",
			surftxtr_info.Nv_AmbientCoeff_Bias);
		OPTIONAL_FPRINTF(ofp, "\t\t\t\tLuminous coefficient texture multiplier = %g\n",
			surftxtr_info.Nv_LuminousCoeff_Multiplier);
		OPTIONAL_FPRINTF(ofp, "\t\t\t\tLuminous coefficient texture bias = %g\n",
			surftxtr_info.Nv_LuminousCoeff_Bias);
		OPTIONAL_FPRINTF(ofp, "\t\t\t\tSpecular coefficient texture multiplier = %g\n",
			surftxtr_info.Nv_SpecularCoeff_Multiplier);
		OPTIONAL_FPRINTF(ofp, "\t\t\t\tSpecular coefficient texture bias = %g\n",
			surftxtr_info.Nv_SpecularCoeff_Bias);
		OPTIONAL_FPRINTF(ofp, "\t\t\t\tReflected coefficient texture multiplier = %g\n",
			surftxtr_info.Nv_ReflectCoeff_Multiplier);
		OPTIONAL_FPRINTF(ofp, "\t\t\t\tReflect coefficient texture bias = %g\n",
			surftxtr_info.Nv_ReflectCoeff_Bias);

		OPTIONAL_FPRINTF(ofp, "\t\t\t\tDiffuse surface color modulation source = %s\n", 
			surftxtr_info.Nv_ModulateSrc_DiffuseColor);
		OPTIONAL_FPRINTF(ofp, "\t\t\t\tAmbient surface color modulation source = %s\n", 
			surftxtr_info.Nv_ModulateSrc_AmbientColor);
		OPTIONAL_FPRINTF(ofp, "\t\t\t\tLuminous surface color modulation source = %s\n", 
			surftxtr_info.Nv_ModulateSrc_LuminousColor);
		OPTIONAL_FPRINTF(ofp, "\t\t\t\tSpecular surface color modulation source = %s\n", 
			surftxtr_info.Nv_ModulateSrc_SpecularColor);
		OPTIONAL_FPRINTF(ofp, "\t\t\t\tBump mapping U modulation source = %s\n",
			surftxtr_info.Nv_ModulateSrc_BumpU);
		OPTIONAL_FPRINTF(ofp, "\t\t\t\tBump mapping V modulation source = %s\n", 
			surftxtr_info.Nv_ModulateSrc_BumpV);
		OPTIONAL_FPRINTF(ofp, "\t\t\t\tFace opacity modulation source = %s\n", 
			surftxtr_info.Nv_ModulateSrc_FaceOpacity);
		OPTIONAL_FPRINTF(ofp, "\t\t\t\tReflect opacity modulation source = %s\n", 
			surftxtr_info.Nv_ModulateSrc_ReflectOpacity);
		OPTIONAL_FPRINTF(ofp, "\t\t\t\tDiffuse coefficient modulation source = %s\n",
			surftxtr_info.Nv_ModulateSrc_DiffuseCoeff);
		OPTIONAL_FPRINTF(ofp, "\t\t\t\tAmbient coefficient modulation source = %s\n",
			surftxtr_info.Nv_ModulateSrc_AmbientCoeff);
		OPTIONAL_FPRINTF(ofp, "\t\t\t\tLuminous coefficient modulation source = %s\n",
			surftxtr_info.Nv_ModulateSrc_LuminousCoeff);
		OPTIONAL_FPRINTF(ofp, "\t\t\t\tSpecular coefficient modulation source = %s\n",
			surftxtr_info.Nv_ModulateSrc_SpecularCoeff);
		OPTIONAL_FPRINTF(ofp, "\t\t\t\tReflected coefficient modulation source = %s\n",
			surftxtr_info.Nv_ModulateSrc_ReflectCoeff);

	OPTIONAL_FPRINTF(ofp, "\t\t\t2d image texture parameters:\n");
		OPTIONAL_FPRINTF(ofp, "\t\t\t\tU,V scaling factors = %g, %g\n", 
			surftxtr_info.Nv_Texture_U_Scale, surftxtr_info.Nv_Texture_V_Scale);
		OPTIONAL_FPRINTF(ofp, "\t\t\t\tU,V offset factors = %g, %g\n", 
			surftxtr_info.Nv_Texture_U_Offset, surftxtr_info.Nv_Texture_V_Offset);

	OPTIONAL_FPRINTF(ofp, "\t\t\t3d procedural texture parameters:\n");
		OPTIONAL_FPRINTF(ofp, "\t\t\t\tWorldspace option is '%s'\n", 
			surftxtr_info.Nv_ProcTxtr_WorldspaceToggle);
		OPTIONAL_FPRINTF(ofp, "\t\t\t\tMatrix scale factors = (%g, %g, %g)\n",
			surftxtr_info.Nv_ProcTxtr_Scale_Factors[0], 
			surftxtr_info.Nv_ProcTxtr_Scale_Factors[1], 
			surftxtr_info.Nv_ProcTxtr_Scale_Factors[2]);
		OPTIONAL_FPRINTF(ofp, "\t\t\t\tMatrix rotation factors = (%g, %g, %g)\n",
			surftxtr_info.Nv_ProcTxtr_X_Rot_Angle,
			surftxtr_info.Nv_ProcTxtr_Y_Rot_Angle,
			surftxtr_info.Nv_ProcTxtr_Z_Rot_Angle);
		OPTIONAL_FPRINTF(ofp, "\t\t\t\tMatrix translation factors = (%g, %g, %g)\n",
			surftxtr_info.Nv_ProcTxtr_Translate_Factors[0], 
			surftxtr_info.Nv_ProcTxtr_Translate_Factors[1], 
			surftxtr_info.Nv_ProcTxtr_Translate_Factors[2]);
#endif
	return(Nc_FALSE);	/* Do not terminate the enumeration */
}

