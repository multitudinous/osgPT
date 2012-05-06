/*****************************************************************************

    ----------------------------------------------------------------
           Surface (Material) Parameter Query Functions for PolyTrans
    ----------------------------------------------------------------
    In PolyTrans a 'Surface Definition' (a 'Material') can be assigned
    to each object in the scene and/or assigned to individual polygons
    of a mesh object. This file shows how to query all the parameters
    of a PolyTrans Surface Definition. Each surface definition may
    have 1 or more 'texture layers' where each layer points towards
    a Texture Definition (see texture.cpp).
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

#include "main.h"    // Main include file for a PolyTrans exporter

/* ----------------------->>>>  Definitions  <<<<--------------------------- */

/* -------------------->>>>  Local Variables  <<<<-------------------------- */

/* ------------------>>>>  Function Prototypes  <<<<------------------------ */

static Nd_Int    NI_Exporter_List_SurfTxtr_Callback(Nd_Enumerate_Callback_Info *cbi_ptr);

/* ------------------>>>>  Surface Definitions Output  <<<<----------------- */

/* This callback routine is repeatedly called from the toolkit (once for */
/* each surface definition matched to the handle name search pattern) */
/* with the corresponding handle name pointed to by 'cbi_ptr->Nv_Handle_Name1'. */
/* The handle name is then used to inquire about all of the parameters */
/* associated with its associated surface definition. */

/* This function should be called indirectly via this enumerator:
    Ni_Enumerate(&dummy, "*", Nc_FALSE, (Nd_Void *) ofp, (Nd_Void *) 0, 
        NI_Exporter_List_Surface_Callback, Nt_SURFACE, Nt_CMDEND);
*/

    Nd_Int
NI_Exporter_List_Surface_Callback(Nd_Enumerate_Callback_Info *cbi_ptr)
{
    FILE                *ofp;
    Export_IO_SurfaceParameters     surf_info;
    Nd_Int                dummy;
    const char            *surface_name = cbi_ptr->Nv_Handle_Name1;

    ofp = (FILE *) cbi_ptr->Nv_User_Data_Ptr1;    // Pick up user-defined parameter #1; we're passing in the ASCII file output handle

    if (cbi_ptr->Nv_Matches_Made == 0) {
        OPTIONAL_FPRINTF(ofp, "There are no surfaces currently defined.\n");
        return(Nc_FALSE);
    }

    // Camera names prefixed with "NUGRAF___" are used internally in the
    // Okino PolyTrans & NuGraf user interface. Ignore them. 
    if (!strncmp(surface_name, "NUGRAF___", 9))
        return(Nc_FALSE);

    // Update the status display with the current surface definition being exported
    Export_IO_UpdateStatusDisplay("surface", (char *) surface_name, "Exporting surface definitions (materials)."); 

    // Update the statistics
    ++export_options->total_materials;

    // And check for user abort
    if (Export_IO_Check_For_User_Interrupt_With_Stats(cbi_ptr->Nv_Call_Count, cbi_ptr->Nv_Matches_Made))
        return(Nc_TRUE);    // Abort the enumeration (nothing gets returned from the Ni_Enumerate() function

    // Inquire about all of the surface parameters 
    Export_IO_Inquire_Surface((char *) surface_name, &surf_info);

    /* Next, we'll print out the values returned from the surface */
    /* inquiry function to show what can be done with them. */

    OPTIONAL_FPRINTF(ofp, "\nSurface definition name = '%s'\n", surface_name);
    OPTIONAL_FPRINTF(ofp, "\tAmbient surface color enable = '%s'\n", surf_info.Nv_AmbientColorToggle);
    OPTIONAL_FPRINTF(ofp, "\t\tAmbient surface color = %g,%g,%g\n",
        surf_info.Nv_Ambient_Color[Na_RED], surf_info.Nv_Ambient_Color[Na_GREEN], 
        surf_info.Nv_Ambient_Color[Na_BLUE]);
    OPTIONAL_FPRINTF(ofp, "\tBase surface color (diffuse) = %g,%g,%g\n",
        surf_info.Nv_Surface_Color[Na_RED], surf_info.Nv_Surface_Color[Na_GREEN], 
        surf_info.Nv_Surface_Color[Na_BLUE]);
    OPTIONAL_FPRINTF(ofp, "\tCompute fresnel reflections = '%s'\n", surf_info.Nv_Compute_Fresnel_Reflections);
    if (surf_info.Nv_CubicalEnvMapToggle == Nt_ON) {
        OPTIONAL_FPRINTF(ofp, "\tCubical environment mapping enable toggle = '%s'\n", 
            surf_info.Nv_CubicalEnvMapToggle);
        OPTIONAL_FPRINTF(ofp, "\t\tAutomatic cubical reflection mapping is = %s\n", 
            surf_info.Nv_CubicalEnvMapAutomaticToggle);
        OPTIONAL_FPRINTF(ofp, "\t\tFilter type = %s\n", surf_info.Nv_CubicalEnvMapSamplingMethod);
        OPTIONAL_FPRINTF(ofp, "\t\tReflection coefficient = %g\n", 
            surf_info.Nv_CubicalEnvMap_Reflect_Coeff);
        OPTIONAL_FPRINTF(ofp, "\t\tTexture image filenames:\n");
        if (surf_info.Nv_CubicalEnvMap_Filenames[0] != (char *) NULL)
            OPTIONAL_FPRINTF(ofp, "\t\t\tFront face = %s\n", surf_info.Nv_CubicalEnvMap_Filenames[0]);
        if (surf_info.Nv_CubicalEnvMap_Filenames[1] != (char *) NULL)
            OPTIONAL_FPRINTF(ofp, "\t\t\tBack face = %s\n", surf_info.Nv_CubicalEnvMap_Filenames[1]);
        if (surf_info.Nv_CubicalEnvMap_Filenames[2] != (char *) NULL)
            OPTIONAL_FPRINTF(ofp, "\t\t\tLeft face = %s\n", surf_info.Nv_CubicalEnvMap_Filenames[2]);
        if (surf_info.Nv_CubicalEnvMap_Filenames[3] != (char *) NULL)
            OPTIONAL_FPRINTF(ofp, "\t\t\tRight face = %s\n", surf_info.Nv_CubicalEnvMap_Filenames[3]);
        if (surf_info.Nv_CubicalEnvMap_Filenames[4] != (char *) NULL)
            OPTIONAL_FPRINTF(ofp, "\t\t\tTop face = %s\n", surf_info.Nv_CubicalEnvMap_Filenames[4]);
        if (surf_info.Nv_CubicalEnvMap_Filenames[5] != (char *) NULL)
            OPTIONAL_FPRINTF(ofp, "\t\t\tBottom face = %s\n", surf_info.Nv_CubicalEnvMap_Filenames[5]);
    }
    OPTIONAL_FPRINTF(ofp, "\tLuminous color enable = '%s'\n", surf_info.Nv_LuminousColorToggle);
    OPTIONAL_FPRINTF(ofp, "\t\tLuminous color = %g,%g,%g\n",
        surf_info.Nv_Luminous_Color[Na_RED], surf_info.Nv_Luminous_Color[Na_GREEN], 
        surf_info.Nv_Luminous_Color[Na_BLUE]);
    OPTIONAL_FPRINTF(ofp, "\tSpecular model = ");
    if (surf_info.Nv_Specular_Model == Nt_PHONG)
        OPTIONAL_FPRINTF(ofp, "Phong. Shininess = %ld, metal = %g\n",
            surf_info.Nv_Phong_Shininess, surf_info.Nv_Metal_Coefficient);
    else {
        OPTIONAL_FPRINTF(ofp, "Blinn.\n");
        OPTIONAL_FPRINTF(ofp, "\t\t'c3' = %g\n", surf_info.Nv_C3_Coefficient);
        OPTIONAL_FPRINTF(ofp, "\t\tIndex of refraction = %g\n", surf_info.Nv_Index_of_Refraction);
        OPTIONAL_FPRINTF(ofp, "\t\tMetal = %g\n", surf_info.Nv_Metal_Coefficient);
    }        

    OPTIONAL_FPRINTF(ofp, "\tOpacity values:\n");
    OPTIONAL_FPRINTF(ofp, "\t\tFace opacity = %g\n", surf_info.Nv_Opacity_FaceCoeff);
    OPTIONAL_FPRINTF(ofp, "\t\tReflect opacity = %g\n", surf_info.Nv_Opacity_ReflectCoeff);
    OPTIONAL_FPRINTF(ofp, "\t\tEdge opacity = %g\n", surf_info.Nv_Opacity_EdgeCoeff);
    OPTIONAL_FPRINTF(ofp, "\t\tEdge exponent = %g\n", surf_info.Nv_Opacity_EdgeExp);
    OPTIONAL_FPRINTF(ofp, "\t\tEdge opacity enable = %s\n", surf_info.Nv_OpacityEdgeToggle);

    OPTIONAL_FPRINTF(ofp, "\t`Pass-to-children' enable toggle = '%s'\n", surf_info.Nv_PassToChildrenEnable);

    if (surf_info.Nv_ReflectmapToggle == Nt_ON) {
        OPTIONAL_FPRINTF(ofp, "\tPlaner reflectance map:\n");
        OPTIONAL_FPRINTF(ofp, "\t\tEnable toggle = '%s'\n", surf_info.Nv_ReflectmapToggle);
        OPTIONAL_FPRINTF(ofp, "\t\tAutomatic planar reflectance mapping is = %s\n", 
            surf_info.Nv_ReflectmapAutomaticToggle);
        OPTIONAL_FPRINTF(ofp, "\t\tFilter type = %s\n", surf_info.Nv_ReflectmapSamplingMethod);
        OPTIONAL_FPRINTF(ofp, "\t\tReflection coefficient = %g\n", surf_info.Nv_Reflectmap_Reflect_Coeff);
        if (surf_info.Nv_Reflectmap_Filename != (char *) NULL)
            OPTIONAL_FPRINTF(ofp, "\t\tTexture image filename = '%s'\n", surf_info.Nv_Reflectmap_Filename);
    }
    OPTIONAL_FPRINTF(ofp, "\tShading coefficients:\n");
        OPTIONAL_FPRINTF(ofp, "\t\tLuminous = %g\n", surf_info.Nv_Luminous_Coeff);
        OPTIONAL_FPRINTF(ofp, "\t\tAmbient = %g\n", surf_info.Nv_Ambient_Coeff);
        OPTIONAL_FPRINTF(ofp, "\t\tDiffuse = %g\n", surf_info.Nv_Diffuse_Coeff);
        OPTIONAL_FPRINTF(ofp, "\t\tSpecular = %g\n", surf_info.Nv_Specular_Coeff);
        OPTIONAL_FPRINTF(ofp, "\t\tReflect = %g\n", surf_info.Nv_Reflect_Coeff);
        OPTIONAL_FPRINTF(ofp, "\t\tIndex-of-refraction = %g\n", surf_info.Nv_IOR_Coeff);
    OPTIONAL_FPRINTF(ofp, "\tShading level override = '%s'\n", surf_info.Nv_ShadingLevelOverride);
    if (surf_info.Nv_SphenvmapToggle == Nt_ON) {
        OPTIONAL_FPRINTF(ofp, "\tSpherical environment mapping enable toggle = '%s'\n",
            surf_info.Nv_SphenvmapToggle);
        OPTIONAL_FPRINTF(ofp, "\t\tTexture definition link name = '%s'\n", 
            surf_info.Nv_Sphenvmap_Texturelink_Name);
        OPTIONAL_FPRINTF(ofp, "\t\tU,V texture scaling = (%g,%g)\n", 
            surf_info.Nv_Sphenvmap_U_Scale, surf_info.Nv_Sphenvmap_V_Scale);
        OPTIONAL_FPRINTF(ofp, "\t\tU,V texture offset = (%g,%g)\n", 
            surf_info.Nv_Sphenvmap_U_Offset, surf_info.Nv_Sphenvmap_V_Offset);
        OPTIONAL_FPRINTF(ofp, "\t\tReflection coefficient = %g\n", 
            surf_info.Nv_Sphenvmap_Reflect_Coeff);
        OPTIONAL_FPRINTF(ofp, "\t\tRotation values = (%g, %g, %g)\n",
            surf_info.Nv_Sphenvmap_X_Rot_Angle, surf_info.Nv_Sphenvmap_Y_Rot_Angle, surf_info.Nv_Sphenvmap_Z_Rot_Angle);
    }
    OPTIONAL_FPRINTF(ofp, "\tSpecular highlight color enable = '%s'\n", surf_info.Nv_SpecularColorToggle);
    OPTIONAL_FPRINTF(ofp, "\t\tSpecular highlight color = %g,%g,%g\n",
        surf_info.Nv_Specular_Color[Na_RED], surf_info.Nv_Specular_Color[Na_GREEN], 
        surf_info.Nv_Specular_Color[Na_BLUE]);

    // Output any meta data associated with this surface definition
    NI_Exporter_Output_Meta_Data(ofp, Nt_SURFACE, (char *) surface_name);

    /* Call 'List_SurfTxtr_Callback()' for every surface texture */
    /* that is currently defined in the surface. */
    if (export_options->ena_surftxtr_layers)
        Ni_Enumerate(&dummy, cbi_ptr->Nv_Handle_Name1, Nc_FALSE, 
            (Nd_Void *) ofp, (Nd_Void *) 0, NI_Exporter_List_SurfTxtr_Callback, 
            Nt_SURFACE, Nt_TEXTURE, Nt_CMDEND);

    // If any animation is associated with this surface definition then output it now
    if (export_options->ena_material_animation) {
        Nd_Int Nv_Active_Channels_Found = Nc_FALSE;
        Ni_NodeHasActiveChannels( Nt_SURFACE, (char *) surface_name, Nc_TRUE /* keyframe controllers only */, &Nv_Active_Channels_Found, Nt_CMDEND );
        if ( Nv_Active_Channels_Found )
            NI_Exporter_Output_Material_Animation_Data(ofp, (char *) surface_name);
    }

    return(Nc_FALSE);    /* Do not terminate the enumeration */
}

/* This callback routine is repeatedly called from the toolkit for each */
/* surface texture that is associated with a specific surface definition. */
/* The variable 'cbi_ptr->Nv_Handle_Name1' is a pointer to the surface */
/* handle name and 'cbi_ptr->Nv_Handle_Name2' is a pointer to the surface */
/* texture handle name. */

    static Nd_Int
NI_Exporter_List_SurfTxtr_Callback(Nd_Enumerate_Callback_Info *cbi_ptr)
{
    FILE                *ofp;
    Export_IO_SurfTxtrParameters    surftxtr_info;
    const char            *surface_name = cbi_ptr->Nv_Handle_Name1;
    const char            *texture_layer_name = cbi_ptr->Nv_Handle_Name2;

    ofp = (FILE *) cbi_ptr->Nv_User_Data_Ptr1;    // Pick up user-defined parameter #1; we're passing in the ASCII file output handle

    if (cbi_ptr->Nv_Matches_Made == 0) {
        OPTIONAL_FPRINTF(ofp, "\tThere are no surface textures currently associated with the surface.\n");
        return(Nc_FALSE);
    }

    if (cbi_ptr->Nv_Call_Count == 1)
        OPTIONAL_FPRINTF(ofp, "\tTexture layers associated with surface:\n");

    Export_IO_Inquire_SurfaceTextureLayerParameters((char *) surface_name, (char *) texture_layer_name, &surftxtr_info);

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

    return(Nc_FALSE);    /* Do not terminate the enumeration */
}

#if 00
// ----------------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------------

// The following code is a much better, "real world" example of how you would actually process
// Okino surface definitions and their diffuse texture maps in an export converter. This example
// drop-in code shows how to enumerate the materials assigned to an instance definition (which
// holds the geometry), which in turn goes and finds the first diffuse texture map enabled on that
// specific surface (material) definition.

// This is a fairly requested Q&A. Okino material's have one or more "texture layers", each one
// which is associated with a single texture map feeding into different shader channels. This code
// basically  enumerates all the texture layers and finds the first texture used to feed
// into the diffuse color shader channel. It also shows how to pick off the first
// texture map used for bump mapping.

// We'll use this struct to return the info about the first diffuse texture map assigned to an
// Okino material definition.
typedef Nd_First_Found_TextureMap_Struct {
    char    image_filename[1024];
    float    Nv_Texture_U_Offset,
        Nv_Texture_V_Offset,
        Nv_Texture_U_Scale,
        Nv_Texture_V_Scale;
} Nd_First_Found_TextureMap_Struct;

// This stub code shows how to enumerate through all the surface-texture layers assigned to an
// Okino surface definition.
NI_Exporter_ConvertandProcessRawMeshPrimitive_Callback(Nd_ConvertandProcessRawPrimitive_Info *Nv_Prp_Ptr, Nd_Int *Nv_Return_Status) 
{
    short        found;
    Nd_First_Found_TextureMap_Struct info;

    ... other code ....

    // Let's enumerate the texture layers associated with the default
    // surface (material) of this instance. "Nv_Prp_Ptr->Nv_Surface_Name"
    // is the default material assigned to this object.
    found = FALSE;
    Ni_Enumerate(&dummy, Nv_Prp_Ptr->Nv_Surface_Name, Nc_FALSE, 
        (Nd_Void *) &info, (Nd_Void *) &found, 
        NI_Exporter_List_SurfTxtr_Callback, 
        Nt_SURFACE, Nt_TEXTURE, Nt_CMDEND);

    if (found) {
        The "info" struct has the texture info you need.
    }
}

// This is the callback called for each surface-texture layer of an Okino surface definition.
// We'll examine each surface-texture layer for an active 2D bitmap texture image.

    static Nd_Int
NI_Exporter_List_SurfTxtr_Callback(Nd_Enumerate_Callback_Info *cbi_ptr)
{
    Nd_First_Found_TextureMap_Struct *info;
    Export_IO_SurfTxtrParameters    surftxtr_info;
    char                *surface_name = cbi_ptr->Nv_Handle_Name1;
    char                *texture_layer_name = cbi_ptr->Nv_Handle_Name2;
    Export_IO_TextureParameters    txtr_ptr;
    char                processed_image_filename[512];
    short                *found;
    Nd_Token             Nv_Type;
    Nd_Token             Nv_Image_Filetype;
    Nd_String             image_filename;
    
    // Pick up a pointer to our parameter passing struct, if we need to
    // use it.
    info = (Nd_First_Found_TextureMap_Struct *) cbi_ptr->Nv_User_Data_Ptr1;
    found = (short *) cbi_ptr->Nv_User_Data_Ptr2;    

    if (cbi_ptr->Nv_Matches_Made == 0) {
        /* There are no surface textures currently associated with the surface */
        return(Nc_FALSE);
    }

    // Get all parameters about this layer.
    Export_IO_Inquire_SurfaceTextureLayerParameters(surface_name, texture_layer_name, &surftxtr_info);

    // Don't use it if this layer is disabled
    if (surftxtr_info.Nv_TextureToggle == Nt_OFF)
        return(Nc_FALSE);

    // Only look for color maps
//    if (surftxtr_info.Nv_ModulateDiffuseColorToggle == Nt_OFF && surftxtr_info.Nv_ModulateBumpToggle == Nt_OFF)
    if (surftxtr_info.Nv_ModulateDiffuseColorToggle == Nt_OFF)
        return(Nc_FALSE);

    // get the texture filename from the texture link and assign it to pmaterial
    Export_IO_Inquire_Texture(surftxtr_info.Nv_Texture_Texturelink_Name, &txtr_ptr);

    // We only want 2D textures and not 3D procedurals
    if (txtr_ptr.Nv_Type != Nt_IMAGE || !strlen(txtr_ptr.Nv_Image_Filename))
        return(Nc_FALSE);

    // Oops, this texture is disabled.
    if (txtr_ptr.Nv_Texture_EnableToggle == Nt_OFF)
        return(Nc_FALSE);

    // Get the file type and filename of this 2D texture
    Ni_Inquire_Texture(surftxtr_info.Nv_Texture_Texturelink_Name, 
        (Nd_Token *) &Nv_Type,
        Nt_TEXTUREIMAGE, 
            (Nd_Token *) &Nv_Image_Filetype, 
            (Nd_String *) &image_filename, 
        Nt_CMDEND);

    // Only consider 2D color texture maps
    if (surftxtr_info.Nv_ModulateDiffuseColorToggle == Nt_ON) {
        // Tell our parent code we found something useful
        *found = Nc_TRUE;

        // Ok, we found a valid texture layer so let's return the
        // data to 'Nd_First_Found_TextureMap_Struct' in the mainline code above.
        strcpy(info->image_filename, image_filename);
        info->Nv_Texture_U_Offset = surftxtr_info.Nv_Texture_U_Offset;
        info->Nv_Texture_V_Offset = surftxtr_info.Nv_Texture_V_Offset;
        info->Nv_Texture_U_Scale = surftxtr_info.Nv_Texture_U_Scale;
        info->Nv_Texture_V_Scale = surftxtr_info.Nv_Texture_V_Scale;

        // Let's terminate the enumeration
        return(Nc_TRUE);
    }
    
    // And this is code to pick up info about bump maps
//    if (surftxtr_info.Nv_ModulateBumpToggle == Nt_ON ) {
//        pmaterial->SetBumpBitmapFileName(image_filename);
//        // Rhino doesn't support setting bump map scale for u,v axis separately.  We are ignoring surftxtr_info.Nv_Bump_V_Multiplier !
//        pmaterial->SetBumpScale(surftxtr_info.Nv_Bump_U_Multiplier);
//        // pmaterial->SetBumpMode( ON::modulate_texture );
//    }

exit:    return(Nc_FALSE);    /* Do not terminate the enumeration */
}

// ----------------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------------
#endif

