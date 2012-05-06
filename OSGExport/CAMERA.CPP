/*****************************************************************************

    ----------------------------------------------------------------
                 Camera Parameter Query Functions for PolyTrans
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

static Nd_Void    NI_Exporter_Compute_Window_Scale_Values(char *camera_name, float *x_scale, float *y_scale);

/* ----------------------->>>>  Camera Output  <<<<------------------------- */

/* This callback routine is repeatedly called from the toolkit (once for */
/* each camera definition matched to the handle name search pattern) */
/* with the corresponding handle name pointed to by 'cbi_ptr->Nv_Handle_Name1'. */
/* The handle name is then used to inquire about all of the parameters */
/* associated with its associated camera definition. */

/* This function should be called indirectly via this enumerator:
    Ni_Enumerate(&dummy, "*", Nc_FALSE, (Nd_Void *) ofp, (Nd_Void *) 0, 
        NI_Exporter_List_Camera_Callback, Nt_CAMERA, Nt_CMDEND);
*/

    Nd_Int
NI_Exporter_List_Camera_Callback(Nd_Enumerate_Callback_Info *cbi_ptr)
{
    FILE                *ofp;
    Export_IO_CameraParameters    cam_info;
    const char            *camera_name = cbi_ptr->Nv_Handle_Name1;
    float                x_scale, y_scale;
    float                horiz_fov, vert_fov;

    ofp = (FILE *) cbi_ptr->Nv_User_Data_Ptr1;    // Pick up user-defined parameter #1; we're passing in the ASCII file output handle

    if (cbi_ptr->Nv_Matches_Made == 0) {
        OPTIONAL_FPRINTF(ofp, "\tThere are no cameras currently defined.\n");
        return(Nc_FALSE);
    }

    // Camera names prefixed with "NUGRAF___" are used internally in the
    // Okino PolyTrans & NuGraf user interface. Ignore them. 
    if (!strncmp(camera_name, "NUGRAF___", 9))
        return(Nc_FALSE);        

    // Update the status display with the current camera definition being exported
    Export_IO_UpdateStatusDisplay("camera", (char *) camera_name, "Exporting camera definitions."); 

    // And check for user abort
    if (Export_IO_Check_For_User_Interrupt_With_Stats(cbi_ptr->Nv_Call_Count, cbi_ptr->Nv_Matches_Made))
        return(Nc_TRUE);    // Abort the enumeration (nothing gets returned from the Ni_Enumerate() function

    // Inquire about all of the camera parameters
    Export_IO_Inquire_Camera((char *) camera_name, &cam_info);

    // Calculate the horizontal and vertical field of views
    NI_Exporter_Compute_Window_Scale_Values((char *) camera_name, &x_scale, &y_scale);
    horiz_fov = 2.0*atan2(cam_info.window_params[Na_X_VALUE]*x_scale/2.0, (double) cam_info.focal_length);
    vert_fov = 2.0*atan2(cam_info.window_params[Na_Y_VALUE]*y_scale/2.0, (double) cam_info.focal_length);

    /* Next, we'll print out the values returned from the camera */
    /* inquiry function to show what can be done with them. */


    OPTIONAL_FPRINTF(ofp, "Camera definition '%s':\n", camera_name);
    if (cam_info.active_state)
        OPTIONAL_FPRINTF(ofp, "\t'%s' is presently the active camera\n", camera_name);
    else
        OPTIONAL_FPRINTF(ofp, "\t'%s' is not the active camera\n", camera_name);
    OPTIONAL_FPRINTF(ofp, "\tClipping plane distances: near = %g, far = %g\n", 
        cam_info.clipping_params[Na_CLIPPING_NEAR], 
        cam_info.clipping_params[Na_CLIPPING_FAR]);
    OPTIONAL_FPRINTF(ofp, "\tFocal length = %g\n", cam_info.focal_length);
    OPTIONAL_FPRINTF(ofp, "\tForm aspect ratio = %g\n", cam_info.form_aspect_ratio);
    OPTIONAL_FPRINTF(ofp, "\tForm mapping type = '%s'\n", cam_info.form_mapping_type);
    OPTIONAL_FPRINTF(ofp, "\tLook-from location = (%g, %g, %g)\n", 
        cam_info.look_from[0], cam_info.look_from[1], cam_info.look_from[2]);
    OPTIONAL_FPRINTF(ofp, "\tLook-at location = (%g, %g, %g)\n", 
        cam_info.look_at[0], cam_info.look_at[1], cam_info.look_at[2]);
    OPTIONAL_FPRINTF(ofp, "\tLook-up vector = (%g, %g, %g)\n", 
        cam_info.look_up[0], cam_info.look_up[1], cam_info.look_up[2]);
    OPTIONAL_FPRINTF(ofp, "\tPixel aspect ratio override value = %g\n", cam_info.pixel_aspect_ratio_override);
    OPTIONAL_FPRINTF(ofp, "\tProjection type = '%s'\n", cam_info.proj_type);
    OPTIONAL_FPRINTF(ofp, "\tTwist value = %g\n", cam_info.twist_angle);
    OPTIONAL_FPRINTF(ofp, "\tWindow size = %g,%g\n", 
        cam_info.window_params[Na_X_VALUE], cam_info.window_params[Na_Y_VALUE]);
    OPTIONAL_FPRINTF(ofp, "\tHorizontal field-of-view = %g\n", Nm_RADIAN2DEGREES(horiz_fov));
    OPTIONAL_FPRINTF(ofp, "\tVertical field-of-view = %g\n", Nm_RADIAN2DEGREES(vert_fov));

    // Note: to determine which camera is the currently active one,
    // without going through the camera enumeration as we do above,
    // call this routine:
//    {
//    char    *Nv_Active_Camera_Name;    
//    Ni_Inquire_Active_Camera(&Nv_Active_Camera_Name);
//    }

    // Output any meta data associated with this camera
    NI_Exporter_Output_Meta_Data(ofp, Nt_CAMERA, (char *) camera_name);

    // Output camera animation data, if there is any associated with the camera
    if (export_options->ena_camera_animation) {
        Nd_Int Nv_Active_Channels_Found = Nc_FALSE;
        Ni_NodeHasActiveChannels( Nt_CAMERA, (char *) camera_name, Nc_TRUE /* keyframe controllers only */, &Nv_Active_Channels_Found, Nt_CMDEND );
        if ( Nv_Active_Channels_Found )
            NI_Exporter_Output_Camera_Animation_Data(ofp, (char *) camera_name);
    }

    return(Nc_FALSE);    /* Do not terminate the enumeration */
}

/* Compute the values that scale the camera projection plane to the */
/* dimensions of the output window (only if TYPE4 or TYPE5 mappings enabled). */

    static Nd_Void
NI_Exporter_Compute_Window_Scale_Values(char *camera_name, float *x_scale, float *y_scale)
{
    Nd_Token    cam_form_mapping_type;
    Nd_Float    cam_window[2], ratio, pw_ar, image_ar;

    Ni_Inquire_Camera(camera_name, 
        Nt_FORMMAPPINGTYPE, (Nd_Token *) &cam_form_mapping_type, Nt_CMDSEP,
        Nt_WINDOW, (Nd_Float *) cam_window, Nt_CMDSEP,
        Nt_CMDEND);

    if (cam_form_mapping_type == Nt_TYPE4 || cam_form_mapping_type == Nt_TYPE5) {
        /* Compute the aspect ratio of the projection window */
        pw_ar = cam_window[Na_X_VALUE] / cam_window[Na_Y_VALUE];
        /* Compute the aspect ratio of the output image (the view window) */
//        image_ar = vi->actual_x_res / (float) vi->actual_y_res;
        image_ar = 1.0;
        /* Compute the new projection window scaling factors */
        ratio = pw_ar / image_ar;
        if ((ratio > 1.0 && cam_form_mapping_type == Nt_TYPE4) || (ratio <= 1.0 && cam_form_mapping_type == Nt_TYPE5)) {
            *x_scale = 1.0;
            *y_scale = ratio;
        } else {
            *x_scale = 1.0 / ratio;
            *y_scale = 1.0;
        }
    } else
        *x_scale = *y_scale = 1.0;
}

