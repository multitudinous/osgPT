/*****************************************************************************

	----------------------------------------------------------------
                Animatiom Query & Output Functions for PolyTrans
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

// Main include file for a PolyTrans exporter
#include "main.h"

#if 00
/* ---------------->>>>  Where do I Start With This Code?  <<<<-------------- */

Please refer to the "Animation" chapter of the "NuGraf Developer's 3D Toolkit"
online help system for explanations of the Okino animation system and of this
example exporter file. 

You may be looking at this file and ask, where do I start? This short section
will give you a quick jump start where to start with customizing this module.

Second, after getting deeper into this code you will most probably ask, "Why
can't animation export just be 50 lines of code like in other SDKs? Why 1500
lines of code just for a basic (not full) implementation of Okino animation
export?" The answer to this question/remark is that the Okino Animation System
implements and supports all major variations of animation keyframe methods
and this file exposes such variations to your exporter. In addition, it features
3 major methods to export animation: 1) raw, original keyframes, (2) raw original
keyframes that have been resampled to the format desired by the export file
format (this is the form you should try to use most of the time), and (3) the
same as #2 except that the 3-separate animation channels of common controllers
(such as translation, Euler rotation and scale) have been merged temporarily
into a single channel, tripled-key-value animation channel (ie: X, Y and Z
channels are resampled then merged into one single XYZ animation channel; this
is a special case for animation packages such as 3DS MAX and Lightwave which
can handle data in this form).

If you decide to just use resampled 3-channel keyframe lists (method #2 just
described) then you should be able to remove 1/3rd of the code contained in
this file, and thus most of the complexity. This is the safest and most common
export method used by all Okino export converters with animation.

You will basically want to follow these initial steps:

1) Determine which animation controllers are supported by your destination 3D
   file format or application program. If you are not intimately familiar with
   them now then you will be by the time you get animation export working in
   this custom exporter. There is no second guessing with animation export;
   either it is done 100% correct or it will not work - again, writing animation
   export code is by far the most time consuming aspect of writing most converters,
   not due to the coding itself (that will go fairly quickly) but trying to
   determine (in most cases) why Euler or Quaternion rotations refuse to play
   back properly; in the end it is usually a matter of axis flipping, RHCS to
   LHCS conversion, or Euler rotation order/gimbal lock problems.

2) Review and completely understand (at least) the object animation export code.
   The code basically works like this:

	if (want to output raw, unsampled, original keyframe lists) {
		- Query & copy all raw keyframe lists into local data structures
		- Create and assign appropriate animation controllers in the
		  destination file format or scene graph.
		- Dump the raw keyframes to the destination file format or scene graph.
	}
	else
	if (want to output resampled keyframe data) {
		// This variation takes the original keyframe lists and resamples
		// then into the clean, generic format desired & understood by the
		// destination file format or scene graph.

		if (want to output as a single channel, 3-component keyframe lists) {
			- Resample & merge 3 channel keyframe lists to a single channel, 3-component value (done magically by the toolkit)
			- Create and assign appropriate animation controllers in the
			  destination file format or scene graph, one channel with XYZ per key.
			- Dump the resampled keyframes to the destination file format or scene graph.
		}

		if (want to output as 3-channel, single components keyframe lists) {
			- Resample each single-valued animation channel into a temp. animation channel (done magically by the toolkit)
			- Create and assign appropriate animation controllers in the
			  destination file format or scene graph, one channel per single-valued key.
			- Dump the resampled keyframes to the destination file format or scene graph.
		}

		- Delete temporary resampled keyframe lists
	}

3) You need to add code to the following empty dummy routines to create animation controllers
   in your destination file format or scene graph. The first two are used for raw keyframe
   output mode and the latter 6 are used for resampled keyframe output mode.

	NI_Exporter_CreateQuatRotationControllerBasedOnExplicitInterpolationMethod();
	NI_Exporter_CreateFloatControllerBasedOnExplicitInterpolationMethod();
	NI_Exporter_CreateADefaultPositionController();
	NI_Exporter_CreateADefaultScaleRotationController();
	NI_Exporter_CreateADefaulEulerRotationController();
	NI_Exporter_CreateADefaultQuatRotationController();
	NI_Exporter_CreateADefaultFloatController();
	NI_Exporter_CreateADefaultRGBColorRotationController();

4) Modify the following routines to accept the raw or resampled key data and output it
   to your destination file format or scene graph.

	NI_Exporter_QueryAndOutputResampledAnimationKeyframeDataFromOkinoToolkit()
	NI_Exporter_OutputRawNuGrafToolkitKeyFrameData()

5) And from direct Okino experience of writing animation conversion software, you will
   eventually have to go over every line of this file and somehow massage, restructure
   or rewrite the code for your export animation file format or scene graph. While this
   .cpp file outlines complete code to query animation data using 3 different
   methods, it is most likely not going to be structured in the same manner as which
   your destination file format or scene graph can accept animation data. Almost every
   animation package is different, and has its own quirks. For example, in 3DS MAX,
   adding a Euler controller automatically creates 3 sub-channels using Bezier controllers;
   thus the code shown in this file would have to be modified so that only one Euler
   parent animation controller is created instead of 3 explicit sub-child controllers.
#endif

// ----------------------->>>>  Definitions  <<<<---------------------------

// Some flags used to help process the animation data being exported in this file
#define Nc_FLAG__NEEDS_FLIPPING			1	// Set when the Y channel in the destination file format (Z from NuGraf) needs to be flipped
#define Nc_FLAG__THIS_IS_A_QUAT_CHANNEL		2	// Set when a quaternion is being output
#define	Nc_FLAG__CONVERT_TO_RADIANS		4	// Set when a ROLL/FOV channels is being output as degrees
#define Nc_FLAGS__NEED_NORM_INVERTING		8	// Do a (1.0-value) on the value to be output. Needed when converting from opacity to transparency keys

// -------------------->>>>  Local Variables  <<<<--------------------------

// This is raw queried object animation keyframe data from the toolkit
static Export_IO_Anim_Controller_Info 	x_trans_kf, y_trans_kf, z_trans_kf;
static Export_IO_Anim_Controller_Info 	x_scale_kf, y_scale_kf, z_scale_kf;
static Export_IO_Anim_Controller_Info 	x_euler_kf, y_euler_kf, z_euler_kf;
static Export_IO_Anim_Controller_Info 	quat_kf;
// This is the order being used by the current Euler rotation controller queried from NuGraf
static Nd_EulerRotationOrder		curr_euler_rotation_controller_order;

// This is raw queried camera animation data from the toolkit
static Export_IO_Anim_Controller_Info x_lookat_kf, y_lookat_kf, z_lookat_kf;
static Export_IO_Anim_Controller_Info fov_kf, roll_kf;

// This is raw queried light animation data from the toolkit
static Export_IO_Anim_Controller_Info x_translate_kf, y_translate_kf, z_translate_kf;
static Export_IO_Anim_Controller_Info x_shineat_kf, y_shineat_kf, z_shineat_kf;
static Export_IO_Anim_Controller_Info shinedir_quat_kf;
static Export_IO_Anim_Controller_Info x_shinedir_eulerrot_kf, y_shinedir_eulerrot_kf, z_shinedir_eulerrot_kf;
static Export_IO_Anim_Controller_Info r_color_kf, g_color_kf, b_color_kf;

// This is raw queried material animation data from the toolkit
static Export_IO_Anim_Controller_Info r_diffuse_color_kf, g_diffuse_color_kf, b_diffuse_color_kf;
static Export_IO_Anim_Controller_Info r_specular_color_kf, g_specular_color_kf, b_specular_color_kf;
static Export_IO_Anim_Controller_Info r_ambient_color_kf, g_ambient_color_kf, b_ambient_color_kf;
static Export_IO_Anim_Controller_Info r_luminous_color_kf, g_luminous_color_kf, b_luminous_color_kf;
static Export_IO_Anim_Controller_Info r_filter_color_kf, g_filter_color_kf, b_filter_color_kf;
static Export_IO_Anim_Controller_Info ambientShadingCoefficient_kf, diffuseShadingCoefficient_kf;
static Export_IO_Anim_Controller_Info specularShadingCoefficient_kf, luminousShadingCoefficient_kf;
static Export_IO_Anim_Controller_Info reflectShadingCoefficient_kf;
static Export_IO_Anim_Controller_Info faceOpacity_kf, highlightPhongSize_kf;

static Nd_Token	NI_Exporter_Get_Users_Desired_Target_Animation_Controller_Type();

// This is a 'handle' pointer to the internally resampled animation data held temporarily inside the Okino toolkit.
static	Nd_Void	 *Nv_FunctionInstanceLocalDataPtr = NULL;

// ------------------>>>>  Function Prototypes  <<<<------------------------

static void	NI_Exporter_QueryAndOutputResampledAnimationKeyframeDataFromOkinoToolkit(FILE *ofp, Nd_Token animation_channel_type, void *exportFileFormatsAnimChanController, short *animation_was_output, short key_flags);
static short	NI_Exporter_OutputRawNuGrafToolkitKeyFrameData(FILE *ofp, Nd_Token animation_channel_type, Export_IO_Anim_Controller_Info *cont_ptr, void *exportFileFormatsAnimChanController, short flags);
static short	NI_Exporter_DoesGeometryInstanceHaveAQuaternionController(char *instance_name);
static short	NI_Exporter_DoesLightUseAQuaternionControllerForDirection(char *light_name);
static void	NI_Exporter_ChangeEulerRotationControllerOrder( FILE *ofp, void *exportFileFormatsAnimChanController, Nd_Int Nv_CurrentOkinoEulerRotationOrder);
static void	NI_Exporter_SetControllerOutofRangeType(FILE *ofp, Nd_Token mode, Nd_Token interpolation_method);
static void	NI_Exporter_Output3RelatedRawAnimationKeyFrameLists(FILE *ofp, short animation_channels_are_Eulers, Nd_Token kflist1_anim_channel_type, Nd_Token kflist2_anim_channel_type, Nd_Token kflist3_anim_channel_type, Export_IO_Anim_Controller_Info *kf_list1, Export_IO_Anim_Controller_Info *kf_list2, Export_IO_Anim_Controller_Info *kf_list3);
static void	NI_Exporter_Output3RelatedResampledAnimationKeyFrameLists(FILE *ofp, short animation_channels_are_Eulers, Nd_Token kflist1_anim_channel_type, Nd_Token kflist2_anim_channel_type, Nd_Token kflist3_anim_channel_type, short *animation_was_output);

static void 	*NI_Exporter_CreateQuatRotationControllerBasedOnExplicitInterpolationMethod(Nd_Token interpolation_method);
static void 	*NI_Exporter_CreateFloatControllerBasedOnExplicitInterpolationMethod(Nd_Token interpolation_method);
static void	*NI_Exporter_CreateADefaultPositionController();
static void	*NI_Exporter_CreateADefaultScaleRotationController();
static void 	*NI_Exporter_CreateADefaulEulerRotationController();
static void 	*NI_Exporter_CreateADefaultQuatRotationController();
static void 	*NI_Exporter_CreateADefaultFloatController();
static void 	*NI_Exporter_CreateADefaultRGBColorController();

// -------------->>>>  General Animation Info Output Code  <<<<-----------------

// This routine queries the toolkit for global information relating to the current
// scene, such as the current time, start/end play-back times, frame rate, etc.

	void
NI_Exporter_QueryAndOutputGeneralAnimationInformation(FILE *ofp)
{
	Nd_Time 	Nv_Curr_Anim_Length;
	Nd_Time 	Nv_Curr_Start_Time;
	Nd_Time 	Nv_Curr_End_Time;
	Nd_Time 	Nv_Curr_Time;
	Nd_Int  	Nv_Curr_Frame_Rate;
	char		out_string[64];
	float		seconds;
	int		int_seconds;
	Nd_AnimationStats anim_stats;

	/* Grab a snapshot of the scene's current animation contents */
	Ni_Inquire_Animation(Nt_STATISTICS, (Nd_AnimationStats *) &anim_stats, Nt_CMDEND);

	// Inquire about basic animation information
	Ni_Inquire_Animation(
		Nt_LENGTH, (Nd_Time *) &Nv_Curr_Anim_Length, Nt_CMDSEP, 
		Nt_STARTTIME, (Nd_Time *) &Nv_Curr_Start_Time, Nt_CMDSEP, 
		Nt_ENDTIME, (Nd_Time *) &Nv_Curr_End_Time, Nt_CMDSEP, 
		Nt_CURRTIME, (Nd_Time *) &Nv_Curr_Time, Nt_CMDSEP, 
		Nt_FRAMERATE, (Nd_Int *) &Nv_Curr_Frame_Rate, Nt_CMDSEP, 
		Nt_CMDEND
	);

	OPTIONAL_FPRINTF(ofp, "Global scene-related Animation information:\n");

	// Format the time string
	seconds = Nm_AnimTicksToSec(Nv_Curr_Time);
	int_seconds = (long) seconds;
	if (!abs(int_seconds/3600) && !abs(int_seconds/60))
		sprintf(out_string, "%.2gs", seconds);
	else 
		sprintf(out_string, "%dm,%.2gs", abs(int_seconds/60), seconds-abs(int_seconds/60)*60.0);
	OPTIONAL_FPRINTF(ofp, "\tCurrent time = %s, ticks = %ld\n", out_string, Nv_Curr_Time);
	OPTIONAL_FPRINTF(ofp, "\tStart playback time = %ld (ticks)\n", Nv_Curr_Start_Time);
	OPTIONAL_FPRINTF(ofp, "\tEnd playback time = %ld (ticks)\n", Nv_Curr_End_Time);
	OPTIONAL_FPRINTF(ofp, "\tFrame rate = %ld (frames per second)\n\n", Nv_Curr_Frame_Rate);

	OPTIONAL_FPRINTF(ofp, "Global scene-related Animation statistics:\n");

	/* This is the time of the very first valid keyframe. Note that this */
	/* value is not affected by the existence of noise controllers since */
	/* they have no keyframes. Set to 0x0fffffff if range is invalid. */
	OPTIONAL_FPRINTF(ofp, "\tTime of first valid key = %d (ticks)\n", anim_stats.Nv_FirstKeyFrameTime);

	/* This is the time of the very last valid keyframe. Note that this */
	/* value is not affected by the existence of noise controllers since */
	/* they have no keyframes. Set to -1 if the range is invalid. */
	OPTIONAL_FPRINTF(ofp, "\tTime of last valid key = %d (ticks)\n", anim_stats.Nv_LastKeyFrameTime);

	/* Total number of keyframes currently defined */
	OPTIONAL_FPRINTF(ofp, "\tTotal number of keys defined = %d\n", anim_stats.Nv_TotalNumKeyFrames);
	/* Total number of instance (mesh) keyframes found */
	OPTIONAL_FPRINTF(ofp, "\tTotal number of instance keys defined = %d\n", anim_stats.Nv_TotalNumInstanceKeyFrames);
	/* Total number of surface (material) keyframes found */
	OPTIONAL_FPRINTF(ofp, "\tTotal number of material keys defined = %d\n", anim_stats.Nv_TotalNumSurfaceKeyFrames);
	/* Total number of surface layer keyframes found */
	OPTIONAL_FPRINTF(ofp, "\tTotal number of surface-texture-layer keys defined = %d\n", anim_stats.Nv_TotalNumSurfaceLayerKeyFrames);
	/* Total number of texture layer keyframes found */
	OPTIONAL_FPRINTF(ofp, "\tTotal number of texture definition keys defined = %d\n", anim_stats.Nv_TotalNumTextureKeyFrames);
	/* Total number of camera keyframes found */
	OPTIONAL_FPRINTF(ofp, "\tTotal number of camera keys defined = %d\n", anim_stats.Nv_TotalNumCameraKeyFrames);
	/* Total number of light keyframes found */
	OPTIONAL_FPRINTF(ofp, "\tTotal number of light keys defined = %d\n\n", anim_stats.Nv_TotalNumLightKeyFrames);
}

// -------------->>>>  Object Animation Query & Output Code  <<<<-----------------

// This routine must first be called to see if there is any animation data
// associated with an instance.	If resampled keyframes are desired then it
// goes and asks the toolkit to resample a specific number of necessary and
// requested keyframe lists. These lists will need to be cleaned up/deleted
// later via the Ni_AnimResampleAndEnumerateKeyframeList_End() function call.

// Returns TRUE if valid animation data is ready to go out, else FALSE
// if no animation data should be output.

	short
NI_Exporter_QueryAndSetup_Object_Animation_Keyframe_Data( FILE *ofp, char *instance_name)
{
	Nd_Token	x_rot_token, y_rot_token, z_rot_token;
	short           instance_uses_quaternion_controller;
	Nd_Int		Nv_Return_Status;

	if (!export_options->ena_object_animation)
		return(Nc_FALSE);

	// Resampled animation data will be created, held, queried and then deleted via this handle pointer.
	Nv_FunctionInstanceLocalDataPtr = NULL;

	// If the export conveter wishes to attempt raw keyframe output mode then query for the existence
	// of raw animation keyframe lists...
	if (export_options->export_raw_unsampled_anim_data_if_possible) {
		// Try to export the animation data using raw keyframes from the NuGraf toolkit
		// instead of the resampled data. The data will have to come in via independent controllers.
		AnimationPtrs_Info anim_ptrs;

		// Use this cover function to query pointers to the various animation channels and controllers associated with this instance
		if (!Export_IO_RetrieveAllAnimationChannelPtrs(instance_name, &anim_ptrs, Nc_FALSE, NULL))
			return(Nc_FALSE);

		// Globalize the Euler rotation order which was optionally queried from the NuGraf Euler rotation controller (if not Quat rot controller).
		// An export converter developer may need this info later on during the export process.
		curr_euler_rotation_controller_order = (Nd_EulerRotationOrder) Export_IO_GetInstanceAnimEulerRotationOrder(instance_name);

		// Go retrieve all the raw keyframes and related information for the controllers we need
		Export_IO_Anim_RetrieveControllerinfo(NULL, anim_ptrs.x_translate_ctrlr_ptr, &x_trans_kf, Nc_TRUE);
		Export_IO_Anim_RetrieveControllerinfo(NULL, anim_ptrs.y_translate_ctrlr_ptr, &y_trans_kf, Nc_TRUE);
		Export_IO_Anim_RetrieveControllerinfo(NULL, anim_ptrs.z_translate_ctrlr_ptr, &z_trans_kf, Nc_TRUE);
		Export_IO_Anim_RetrieveControllerinfo(NULL, anim_ptrs.x_scale_ctrlr_ptr, &x_scale_kf, Nc_TRUE);
		Export_IO_Anim_RetrieveControllerinfo(NULL, anim_ptrs.y_scale_ctrlr_ptr, &y_scale_kf, Nc_TRUE);
		Export_IO_Anim_RetrieveControllerinfo(NULL, anim_ptrs.z_scale_ctrlr_ptr, &z_scale_kf, Nc_TRUE);
		Export_IO_Anim_RetrieveControllerinfo(NULL, anim_ptrs.x_rotate_ctrlr_ptr, &x_euler_kf, Nc_TRUE);
		Export_IO_Anim_RetrieveControllerinfo(NULL, anim_ptrs.y_rotate_ctrlr_ptr, &y_euler_kf, Nc_TRUE);
		Export_IO_Anim_RetrieveControllerinfo(NULL, anim_ptrs.z_rotate_ctrlr_ptr, &z_euler_kf, Nc_TRUE);
		Export_IO_Anim_RetrieveControllerinfo(NULL, anim_ptrs.quaternion_rotate_ctrlr_ptr, &quat_kf, Nc_TRUE);

		// Let's see if we got any valid animation data
		if (x_trans_kf.kf_list || y_trans_kf.kf_list || z_trans_kf.kf_list ||
			x_scale_kf.kf_list || y_scale_kf.kf_list || z_scale_kf.kf_list ||
			x_euler_kf.kf_list || y_euler_kf.kf_list || z_euler_kf.kf_list ||
			quat_kf.kf_list) {

			Export_IO_UpdateStatusDisplay("instance", instance_name, "Exporting animation data.");
			return(Nc_TRUE);
		} else
			return(Nc_FALSE);	// No valid animation data
	} else {
		Nd_Int	chosen_euler_order;

		//
		// Else, the following code reverts to creating temporary resampled keyframe lists internally
		// inside the toolkit which conform to the animation methods the exporter requires.
		//

        	// Determine if the instance uses a quaternion controller or a Euler controller
        	instance_uses_quaternion_controller = NI_Exporter_DoesGeometryInstanceHaveAQuaternionController(instance_name);

		// Set the rotation order for Eulers resampled and queried from the toolkit. We'll use
		// the method specified on the export options "Animation" panel.
		//
		// !! Normally an export converter developer will hard code this parameter right here and
		//    not ask the user for the rotation method. This is because most export file formats
		//    use a fixed Euler rotation order. For example, set it to 'Nc_EulerRotationOrder_XYZ'
		//    (the various orders are listed in the ni4_aux.h header file)
		//
		curr_euler_rotation_controller_order = (Nd_EulerRotationOrder) export_options->ena_object_anim_resampled_rot_order;

        	if (export_options->rotation_controller_type_to_use_during_export == Nc_NRS_ROTATION_CONTROLLER_AS_PER_NRS_TOOLKIT && instance_uses_quaternion_controller)
	        	// If dialog box is set to "as-per-toolkit rotation export method" then set the resampled
        		// rotation method (Quat or Euler) based on what type of rotation controller is used inside NuGraf.
        		x_rot_token = y_rot_token = z_rot_token = Nt_QUATROTATE;
        	else if (export_options->rotation_controller_type_to_use_during_export == Nc_NRS_ROTATION_CONTROLLER_QUATERNION)
        		// Else, the user wishes to override the toolkit rotation method with Quaternion export
        	 	x_rot_token = y_rot_token = z_rot_token = Nt_QUATROTATE;
		else {
			// Else, query the rotations an separate X, Y and Z Euler rotations. If the exporter
			// wants to query the Eulers as a single XYZ Euler keyframe list then the code further
			// below changes all these 3 tokens to 'Nt_XYZROTATE' so that the resampled Eulers get
			// merged together into a single Euler rotation keyframe list deep inside the Okino toolkit.
			x_rot_token = Nt_XROTATE;
			y_rot_token = Nt_YROTATE;
			z_rot_token = Nt_ZROTATE;
		}

		Export_IO_UpdateStatusDisplay("instance", instance_name, "Exporting animation data.");

        	/* Go and resample all the instance-related keyframe lists */
        	if (export_options->use_independent_controllers_for_tripled_anim_channels) {
        		// Resample the animation to independent X, Y and Z curves. Each curve could have
        		// different number of keys and different keyframe times
        		Ni_AnimResampleAndEnumerateKeyframeList_Start(
        			/* Return status: FALSE = no error, otherwise, see error list */
        			&Nv_Return_Status,

        			/* Pointer to the toolkit's local data is returned here for safe keeping. */
        			&Nv_FunctionInstanceLocalDataPtr,

        			/* Handle type and name of the Comm_Type to be enumerated (ie: Nt_CMAERA, "camera1") */
        			Nt_INSTANCE, instance_name,

        			/* This is the order to extract any new Euler rotation angles (see ni3.h for list) */
        			/* This is only needed when Euler rotation angles are being asked for. */
        			/* Lightwave, for example, uses 'Nc_EulerRotationOrder_YXZ'. */
                		/* If you want to use the Nt_ALLOWEDEULERORDERS functionality below, the following value must be set to -1. */
        			curr_euler_rotation_controller_order,

        			/* This is only needed when Euler rotation angles are being asked for. */
        			/* If TRUE, then the camera orientation matrix will have its Z value */
        			/* negated (RHCS -> LHCS) which is useful for Lightwave, but not Max. */
        			Nc_FALSE,

				// If the source Euler rotation curve flips back and forth on each
				// side of -180/180 degrees (like runs of -179, followed by 179.0)
				// then this routine will enable resampling on those rotation curves 
				// and then make sure the rotation values don't flip back and forth any more.
				Nt_FIXEULERROTATIONANGLEFLIPS, Nt_ENABLED, (export_options->flip_Euler_180flips_via_resampler ? Nt_ON : Nt_OFF), Nt_CMDSEP,

				// Specify the animation interpolation controller for the resampled output fcurves. 
				// (One of: Nt_ORIGINAL, Nt_KFCONSTANT, Nt_KFLINEAR, Nt_KFBEZIER, Nt_KFTCB)
				// If we set it to Nt_ORIGINAL then the resampler will use the same 
				// interplation type as the source animation curves that are being
				// fed to the resampler, unless it is not one of the allowable 
				// controller types set with the  Nt_ALLOWEDCONTROLLERTYPES option.
				Nt_TARGETCONTROLLERTYPE, NI_Exporter_Get_Users_Desired_Target_Animation_Controller_Type(), Nt_CMDSEP,

				// This specifies the keyframe controller types which are supported by the our exporter.
				// If the 'Nt_TARGETCONTROLLERTYPE' is set to Nt_ORIGINAL, then each
				// controller will be compared against this list of allowed controller types.
				// If the original curve's type matches, then that type will be used. If it
				// does not match, then the first item from this list will be used.
				Nt_ALLOWEDCONTROLLERTYPES, Nt_KFBEZIER, Nt_KFTCB, Nt_KFCONSTANT, Nt_KFLINEAR, Nt_CMDSEP,

				// When the 'Nt_TARGETCONTROLLERTYPE' fcurve interpolation method is set to Nt_KFBEZIER, the 
				// following list determines which Bezier tangent types are supported by this exporter 
				// as a way to create and return the new Bezier tangents. 
				Nt_ALLOWEDBEZIERTANGENTTYPES, Nt_CUSTOM, Nt_CONSTANT, Nt_LINEAR, Nt_SMOOTH, Nt_FAST, Nt_SLOW, Nt_CLAMPED, Nt_PLATEAU, Nt_AUTOFLAT, Nt_BROKENTANGENTS, Nt_CONSTANTVELOCITY, Nt_CMDSEP,

				// When the 'curr_euler_rotation_controller_order' parameter above is set to -1, the following 
		                //  specifiest he list of Euler orders which are acceptable to this exporter. Any
		                //  one of the provided  Euler orders might be used for the exported
                		//  rotation data, and the selection of Euler order will be made to
		                //  minimize any resampling. If resampling cannot be avoided, the
                		//  first Euler order in this list will be given precedence.
		                Nt_ALLOWEDEULERORDERS, Nt_EULERORDER_XYZ, 
					Nt_EULERORDER_XZY, Nt_EULERORDER_YZX, Nt_EULERORDER_YXZ, 
					Nt_EULERORDER_ZXY, Nt_EULERORDER_ZYX, Nt_EULERORDER_XYX, 
					Nt_EULERORDER_XZX, Nt_EULERORDER_YZY, Nt_EULERORDER_YXY, 
					Nt_EULERORDER_ZXZ, Nt_EULERORDER_ZYZ, Nt_CMDSEP,

		                // When using the Nt_ALLOWEDEULERORDERS option above, it is necessary
                		// for the exporter to be informed about which Euler order was
		                // actually chosen for this instance/camera/light. The Euler order
                		// chosen by the animation resampler is returned into the user-provided pointer.
		                Nt_INQUIRE_CHOSENEULERORDER, (Nd_Int*) &chosen_euler_order, Nt_CMDSEP,

#if 00	// Valid, but not used in this example exporter
				// When resampling any rotation channels, this function will pre-multiply the
				//   given (quaternion) rotation into the evaluated rotation matrix.
				// The fundamental purpose of this feature is to allow lights to be "repointed"
				//   down an axis more appropriate for the target animation system.
				// However, it can also be used to tweak the resampled rotation values from
				//   any ordinary euler-quat, euler-euler, or quat-quat resampling as well.
				Nt_ROTATIONTWEAK, (Nd_Quat *) Nv_Rotation_Tweak_Quaternion, Nt_CMDSEP,
#endif

        			/* This is the list of channels to resample/inquire */
        			Nt_ANIMCHAN,
        				Nt_XTRANSLATE, Nt_YTRANSLATE, Nt_ZTRANSLATE,
        				x_rot_token, y_rot_token, z_rot_token,	// Convert to Eulers or Quats
        				Nt_XSCALE, Nt_YSCALE, Nt_ZSCALE,
        				Nt_CMDEND);
        	} else {
        		// Else, let's utilize the special feature in the animation resampler routine to
			// merge 3 distinct XYZ animation channels into a single XYZ animation channel
			// for those export file formats which can handle a single XYZ value at each key.

			// For this exporter, let's assume it has a single value XYZ Euler rotation controller
			// so in that case let's ask the resampler to return to us the rotations as a
			// single keyframe list of XYZ Euler rotations
			if (x_rot_token == Nt_XROTATE)		// We are not doing quats
				x_rot_token = Nt_XYZROTATE;	// So, let's ask for merged XYZ Euler keys instead

        		Ni_AnimResampleAndEnumerateKeyframeList_Start(
        			&Nv_Return_Status,
        			&Nv_FunctionInstanceLocalDataPtr,
        			Nt_INSTANCE, instance_name,
        			curr_euler_rotation_controller_order,
        			Nc_FALSE,
				Nt_FIXEULERROTATIONANGLEFLIPS, Nt_ENABLED, (export_options->flip_Euler_180flips_via_resampler ? Nt_ON : Nt_OFF), Nt_CMDSEP,
				Nt_TARGETCONTROLLERTYPE, NI_Exporter_Get_Users_Desired_Target_Animation_Controller_Type(), Nt_CMDSEP,
				Nt_ALLOWEDCONTROLLERTYPES, Nt_KFBEZIER, Nt_KFTCB, Nt_KFCONSTANT, Nt_KFLINEAR, Nt_CMDSEP,
				Nt_ALLOWEDBEZIERTANGENTTYPES, Nt_CUSTOM, Nt_CONSTANT, Nt_LINEAR, Nt_SMOOTH, Nt_FAST, Nt_SLOW, Nt_CLAMPED, Nt_PLATEAU, Nt_AUTOFLAT, Nt_BROKENTANGENTS, Nt_CONSTANTVELOCITY, Nt_CMDSEP,
        			Nt_ANIMCHAN,
        				// This will cause each set of XYZ channels to be modified to that they
        				// share the same set of key times. They will still be queried by their
        				// single respective channel however.
        				Nt_XYZTRANSLATE,
        				x_rot_token, 	// Convert to Eulers or Quats
        				Nt_XYZSCALE,
        				Nt_CMDEND);
        	}

		if (Nv_Return_Status == Nc_RESAMPLEDKFLIST_STATUS_NO_NEW_KF_LISTS_CREATED)
			// Valid animation data exists but no resampling was performed internally on the source keyframe list
			return(Nc_TRUE);
        	else if (Nv_Return_Status || (Nv_Return_Status == Nc_RESAMPLEDKFLIST_STATUS_ALL_KEYS_AT_SAME_TIME)) {
        		/* Either an error was encontered, or no valid animation data */
        		/* exists, or all the keys exist at 1 point in time in which case */
        		/* we'll just output the ctm and no anim. info. for this instance. */
        		Ni_AnimResampleAndEnumerateKeyframeList_End(&Nv_Return_Status, Nv_FunctionInstanceLocalDataPtr);
        		return(Nc_FALSE);
        	} else
        		// Valid instance animation data exists
        		return(Nc_TRUE);
	}
}

// Once the instance geometry data has been output to the file then this routine should be
// called to dump out the raw or resampled keyframe data associated with the instance.
// You need to first call "NI_Exporter_Setup_Resampled_Object_Animation_Keyframe_Lists()"
// to have the raw or resampled keyframe lists queried and readied from the Okino toolkit.

	short
NI_Exporter_Output_Object_Animation_Data(FILE *ofp, char *instance_name)
{
	Nd_Int	Nv_Return_Status;
	short	animation_was_output;
	short   instance_uses_quaternion_controller;
	void	*exportFileFormatsAnimChanController;	// Replace this with your own animation controller pointer type

	if (!export_options->ena_object_animation)
		return(Nc_FALSE);

	animation_was_output = Nc_FALSE;

	// If the export conveter wishes to attempt raw keyframe output mode then query for the existence
	// of raw animation keyframe lists...
	if (export_options->export_raw_unsampled_anim_data_if_possible) {
		//
		// !! ADD CODE !! At this line you would create an animation controller in your
		//                export file scene graph to hold the instance's scale, rotation and translation keys.
		//		  The rotation controller would be created either as Euler or Quaternion, as determined below.
		//

		// Since we've checked that there is valid keyframe data above, we'll assume we are outputting valid animation
		animation_was_output = Nc_TRUE;

                OPTIONAL_FPRINTF(ofp, "\n\tInstance animation (raw keyframe) data:\n");

 		// Send the raw scale keyframes (as 3 separate animation keyframe lists)
		if (x_scale_kf.kf_list || y_scale_kf.kf_list || z_scale_kf.kf_list)
			NI_Exporter_Output3RelatedRawAnimationKeyFrameLists(ofp, Nc_FALSE,
				Nt_XSCALE, Nt_YSCALE, Nt_ZSCALE,
				&x_scale_kf, &y_scale_kf, &z_scale_kf);

		// Send the raw Euler keyframes (as 3 separate animation keyframe lists) or Quaternion rotation keyframes
		// !! NOTE: Somewhere you need to call 'NI_Exporter_ChangeEulerRotationControllerOrder()' to set up the Euler rotation order properly on this controller.
		if (x_euler_kf.kf_list || y_euler_kf.kf_list || z_euler_kf.kf_list) {
			NI_Exporter_Output3RelatedRawAnimationKeyFrameLists(ofp, Nc_TRUE,
				Nt_XROTATE, Nt_YROTATE, Nt_ZROTATE,
				&x_euler_kf, &y_euler_kf, &z_euler_kf);

		} else if (quat_kf.kf_list) {
			// Create an animation controller based on the type of this NuGraf animation controller
			exportFileFormatsAnimChanController = NI_Exporter_CreateQuatRotationControllerBasedOnExplicitInterpolationMethod(quat_kf.type);

			// Send the raw Quaternion or Angle/Axis rotation keyframes
			NI_Exporter_OutputRawNuGrafToolkitKeyFrameData(ofp, Nt_QUATROTATE, &quat_kf, exportFileFormatsAnimChanController, Nc_FLAG__THIS_IS_A_QUAT_CHANNEL);
		}

		// Send the raw translation keyframes (as 3 separate animation keyframe lists)
		if (x_trans_kf.kf_list || y_trans_kf.kf_list || z_trans_kf.kf_list)
			// !! ADD CODE !! At this line you would create an animation controller in your
			//                export file scene graph to hold the camera look-from translation keyframe data (3 separate channels)
			NI_Exporter_Output3RelatedRawAnimationKeyFrameLists(ofp, Nc_FALSE,
				Nt_XTRANSLATE, Nt_YTRANSLATE, Nt_ZTRANSLATE,
				&x_trans_kf, &y_trans_kf, &z_trans_kf);

		// And free up the keyframe lists we allocated in MAX_Setup_Resampled_Object_Animation_Keyframe_Lists()
		if (x_trans_kf.kf_list) 	Ni_Free_Memory((char *) x_trans_kf.kf_list);
		if (y_trans_kf.kf_list)		Ni_Free_Memory((char *) y_trans_kf.kf_list);
		if (z_trans_kf.kf_list)		Ni_Free_Memory((char *) z_trans_kf.kf_list);
		if (x_scale_kf.kf_list)		Ni_Free_Memory((char *) x_scale_kf.kf_list);
		if (y_scale_kf.kf_list)		Ni_Free_Memory((char *) y_scale_kf.kf_list);
		if (z_scale_kf.kf_list)		Ni_Free_Memory((char *) z_scale_kf.kf_list);
		if (x_euler_kf.kf_list)		Ni_Free_Memory((char *) x_euler_kf.kf_list);
		if (y_euler_kf.kf_list)		Ni_Free_Memory((char *) y_euler_kf.kf_list);
		if (z_euler_kf.kf_list)		Ni_Free_Memory((char *) z_euler_kf.kf_list);
		if (quat_kf.kf_list)		Ni_Free_Memory((char *) quat_kf.kf_list);
	} else {
		//
		// Else, the following code reverts to creating temporary resampled keyframe lists internally
		// inside the toolkit which conform to the animation methods the exporter requires.
		//

		// Determine if the instance uses a quaternion controller or a Euler controller
		instance_uses_quaternion_controller = NI_Exporter_DoesGeometryInstanceHaveAQuaternionController(instance_name);

		if (!export_options->use_independent_controllers_for_tripled_anim_channels) {
        		// Export the animation by converting 3 channel keyframe lists to single channel (triple-value)
        		// keyframe lists inside the toolkit, then outputting them as a single controller.

			// !! ADD CODE !! At this line you would create animation controllers in your
			//                export file scene graph to hold the camera look-from location (single channel, XYZ),
			//		  look-at location (single channel, XYZ), ROLL value and FOV value.

	                OPTIONAL_FPRINTF(ofp, "\n\tInstance animation (resampled to single channel, 3-component) data:\n");

        		// Send the scale channels from the NuGraf toolkit to the exporter as a single XYZ animation controller channel
			exportFileFormatsAnimChanController = NI_Exporter_CreateADefaultScaleRotationController();
        		NI_Exporter_QueryAndOutputResampledAnimationKeyframeDataFromOkinoToolkit(ofp, Nt_XYZSCALE, exportFileFormatsAnimChanController, &animation_was_output, 0);

			// Send the Euler or Quaternion rotation channels
			if (export_options->rotation_controller_type_to_use_during_export == Nc_NRS_ROTATION_CONTROLLER_QUATERNION
					|| (export_options->rotation_controller_type_to_use_during_export == Nc_NRS_ROTATION_CONTROLLER_AS_PER_NRS_TOOLKIT && instance_uses_quaternion_controller)) {

				// The toolkit fcurve uses quats, so we'll do the same
				exportFileFormatsAnimChanController = NI_Exporter_CreateADefaultQuatRotationController();
	        		NI_Exporter_QueryAndOutputResampledAnimationKeyframeDataFromOkinoToolkit(ofp, Nt_QUATROTATE, exportFileFormatsAnimChanController, &animation_was_output, Nc_FLAG__THIS_IS_A_QUAT_CHANNEL);
			} else {
				// Otherwise, create a single channel, 3 component Euler controller
				exportFileFormatsAnimChanController = NI_Exporter_CreateADefaulEulerRotationController();

				// Change the Euler rotation order of the Euler controller to match that of NuGraf
				NI_Exporter_ChangeEulerRotationControllerOrder(ofp, exportFileFormatsAnimChanController, curr_euler_rotation_controller_order);

				// Send the single channel, 3 component XYZ Euler keys to the animation controller
	        		NI_Exporter_QueryAndOutputResampledAnimationKeyframeDataFromOkinoToolkit(ofp, Nt_XYZROTATE, exportFileFormatsAnimChanController, &animation_was_output, 0);
			}

        		// Send the translation channels from the NuGraf toolkit to the exporter as a single XYZ animation controller channel
			exportFileFormatsAnimChanController = NI_Exporter_CreateADefaultPositionController();
	        	NI_Exporter_QueryAndOutputResampledAnimationKeyframeDataFromOkinoToolkit(ofp, Nt_XYZTRANSLATE, exportFileFormatsAnimChanController, &animation_was_output, 0);
        	} else {
        		// Else, export the 3 channel keyframe lists as distinct controllers of their own.

			// !! ADD CODE !! At this line you would create animation controllers in your
			//                export file scene graph to hold the camera look-from location (3-channel),
			//		  look-at location (3-channel), ROLL value and FOV value.

	                OPTIONAL_FPRINTF(ofp, "\n\tInstance animation (resampled, single channels) data:\n");

        		// Send the scale channels from the NuGraf toolkit as 3 separate animation channels
			NI_Exporter_Output3RelatedResampledAnimationKeyFrameLists(ofp, Nc_FALSE, Nt_XSCALE, Nt_YSCALE, Nt_ZSCALE, &animation_was_output);

			// Send the Euler or Quaternion rotation channels
			if (export_options->rotation_controller_type_to_use_during_export == Nc_NRS_ROTATION_CONTROLLER_QUATERNION
					|| (export_options->rotation_controller_type_to_use_during_export == Nc_NRS_ROTATION_CONTROLLER_AS_PER_NRS_TOOLKIT && instance_uses_quaternion_controller)) {

				// The toolkit fcurve uses quats, so we'll do the same
				exportFileFormatsAnimChanController = NI_Exporter_CreateADefaultQuatRotationController();
	        		NI_Exporter_QueryAndOutputResampledAnimationKeyframeDataFromOkinoToolkit(ofp, Nt_QUATROTATE, exportFileFormatsAnimChanController, &animation_was_output, Nc_FLAG__THIS_IS_A_QUAT_CHANNEL);
			} else {
				// Otherwise, create a Euler controller and output the explicit separate X, Y and Z Euler channels.

				//
				// !! You will most probably have to rework this Euler section a lot depending
				//    and how, and if, your export file format or scene graph handles Euler controllers.
				//    At this point here in the code you are sending 3 distinct and separate animation
				//    channels to your export file format as X, Y and Z Euler rotation channels. At the
				//    least you need to modify the "NI_Exporter_ChangeEulerRotationControllerOrder()"
				//    routine to somehow change the Euler rotation order on your superior Euler controller;
				//    in the code shown below we try to change the order on every channel, but this would
				//    really not be the case with most animation systems (instead, you would set the Euler
			        //    order on the parent controller which owns the individual X, Y and Z Euler channels).
				//

				// Send the X Euler rotation channel
				NI_Exporter_Output3RelatedResampledAnimationKeyFrameLists(ofp, Nc_TRUE, Nt_XROTATE, Nt_YROTATE, Nt_ZROTATE, &animation_was_output);
			}

        		// Send the translation channels from the NuGraf toolkit as 3 separate animation channels
			NI_Exporter_Output3RelatedResampledAnimationKeyFrameLists(ofp, Nc_FALSE, Nt_XTRANSLATE, Nt_YTRANSLATE, Nt_ZTRANSLATE, &animation_was_output);
        	}
	}

	/* Free up the resampled keyframe lists */
	if (Nv_FunctionInstanceLocalDataPtr) {
		Ni_AnimResampleAndEnumerateKeyframeList_End(&Nv_Return_Status, Nv_FunctionInstanceLocalDataPtr);
		Nv_FunctionInstanceLocalDataPtr = NULL;
	}

	return(animation_was_output);
}

// Decide which interpolation type the resampler should resample to.
// If we set it to "Original" then the resampler will use the same 
// interplation type as the source animation curves that are being
// fed to the resampler, unless it is not one of the allowable 
// controller types set with the  Nt_ALLOWEDCONTROLLERTYPES option.

	static Nd_Token
NI_Exporter_Get_Users_Desired_Target_Animation_Controller_Type()
{
	Nd_Token target_controller_type;

	switch (export_options->interpolation_type_to_use_during_export) {
		case Nc_NRS_ANIM_INTERPOLATION_TYPE_ORIGINAL:
			// Cause the resampler to use the same interpolation type as the input fcurves
			target_controller_type = Nt_ORIGINAL;
			break;
		case Nc_NRS_ANIM_INTERPOLATION_TYPE_LINEAR:
			// Force the resampled curves to use linear interpolation
			target_controller_type = Nt_KFLINEAR;
			break;
		case Nc_NRS_ANIM_INTERPOLATION_TYPE_TCB:
			// Force the resampled curves to use TCB interpolation
			target_controller_type = Nt_KFTCB;
			break;
		case Nc_NRS_ANIM_INTERPOLATION_TYPE_CONSTANT:
			// Force the resampled curves to use constant interpolation
			target_controller_type = Nt_KFCONSTANT;
			break;
		case Nc_NRS_ANIM_INTERPOLATION_TYPE_BEZIER:
		default:
			// Force the resampled curves to use Bezier interpolation
			target_controller_type = Nt_KFBEZIER;
			break;
	}

	return(target_controller_type);
}

// -------------->>>>  Camera Animation Query & Output Code  <<<<-----------------

// Call this single routine to output any possible animation data associated with
// a camera definition.

// Returns TRUE if valid animation data is ready to go out, else FALSE
// if no animation data should be output.

	Nd_Void
NI_Exporter_Output_Camera_Animation_Data( FILE *ofp, char *camera_name)
{
	short	animation_was_output;
	Nd_Int	Nv_Return_Status;
	void	*exportFileFormatsAnimChanController;	// Replace this with your own animation controller pointer type

	if (!export_options->ena_camera_animation)
		return;

	// Resampled animation data will be created, held, queried and then deleted via this handle pointer.
	Nv_FunctionInstanceLocalDataPtr = NULL;

	// If the export conveter wishes to attempt raw keyframe output mode then query for the existence
	// of raw animation keyframe lists...
	if (export_options->export_raw_unsampled_anim_data_if_possible) {
		// Try to export the animation data using raw keyframes from the NuGraf toolkit
		// instead of the resampled data. The data will have to come in via independent controllers.
		CameraAnimationPtrs_Info anim_ptrs;

		// Get pointers to the various animation channels and controllers associated with this instance
		if (!Export_IO_RetrieveAllCameraAnimationChannelPtrs(camera_name, &anim_ptrs, Nc_FALSE, NULL))
			goto exit;

		// Go retrieve all the keyframes and related information for the controllers we need
		Export_IO_Anim_RetrieveControllerinfo(NULL, anim_ptrs.x_translate_ctrlr_ptr, &x_trans_kf, Nc_TRUE);
		Export_IO_Anim_RetrieveControllerinfo(NULL, anim_ptrs.y_translate_ctrlr_ptr, &y_trans_kf, Nc_TRUE);
		Export_IO_Anim_RetrieveControllerinfo(NULL, anim_ptrs.z_translate_ctrlr_ptr, &z_trans_kf, Nc_TRUE);
		Export_IO_Anim_RetrieveControllerinfo(NULL, anim_ptrs.x_lookat_ctrlr_ptr, &x_lookat_kf, Nc_TRUE);
		Export_IO_Anim_RetrieveControllerinfo(NULL, anim_ptrs.y_lookat_ctrlr_ptr, &y_lookat_kf, Nc_TRUE);
		Export_IO_Anim_RetrieveControllerinfo(NULL, anim_ptrs.z_lookat_ctrlr_ptr, &z_lookat_kf, Nc_TRUE);
		Export_IO_Anim_RetrieveControllerinfo(NULL, anim_ptrs.fov_ctrlr_ptr, &fov_kf, Nc_TRUE);
		Export_IO_Anim_RetrieveControllerinfo(NULL, anim_ptrs.roll_ctrlr_ptr, &roll_kf, Nc_TRUE);

		// Let's see if we got any valid animation data
		if (!(x_trans_kf.kf_list || y_trans_kf.kf_list || z_trans_kf.kf_list ||
				x_lookat_kf.kf_list || y_lookat_kf.kf_list || z_lookat_kf.kf_list ||
				fov_kf.kf_list || roll_kf.kf_list))
			goto exit;	// No valid animation data

                OPTIONAL_FPRINTF(ofp, "\n\tCamera animation (raw keyframe) data:\n");

		Export_IO_UpdateStatusDisplay("camera", camera_name, "Exporting animation data.");

		// Since we've checked that there is valid keyframe data above, we'll assume we are outputting valid animation
		animation_was_output = Nc_TRUE;

		// Send the raw look-from XYZ translation keyframes (as 3 separate animation keyframe lists)
		if (x_trans_kf.kf_list || y_trans_kf.kf_list || z_trans_kf.kf_list)
			// !! ADD CODE !! At this line you would create an animation controller in your
			//                export file scene graph to hold the camera look-from translation keyframe data (3 separate channels)
			NI_Exporter_Output3RelatedRawAnimationKeyFrameLists(ofp, Nc_FALSE,
				Nt_XTRANSLATE, Nt_YTRANSLATE, Nt_ZTRANSLATE,
				&x_trans_kf, &y_trans_kf, &z_trans_kf);

		// Send the raw look-at translation keyframes (as 3 separate animation keyframe lists)
		if (x_lookat_kf.kf_list || y_lookat_kf.kf_list || z_lookat_kf.kf_list)
			// !! ADD CODE !! At this line you would create an animation controller in your
			//                export file scene graph to hold the camera look-at translation keyframe data (3 separate channels)
			NI_Exporter_Output3RelatedRawAnimationKeyFrameLists(ofp, Nc_FALSE,
				Nt_XLOOKAT, Nt_YLOOKAT, Nt_ZLOOKAT,
				&x_lookat_kf, &y_lookat_kf, &z_lookat_kf);

		// Send the roll keyframe data
		if (roll_kf.kf_list) {
			// !! ADD CODE !! At this line you would create an animation controller in your
			//                export file scene graph to hold the camera ROLL keyframe data
			exportFileFormatsAnimChanController = NI_Exporter_CreateFloatControllerBasedOnExplicitInterpolationMethod(roll_kf.type);
			NI_Exporter_OutputRawNuGrafToolkitKeyFrameData(ofp, Nt_ROLL, &roll_kf, exportFileFormatsAnimChanController, Nc_FLAG__CONVERT_TO_RADIANS);
		}

		// Send the FOV (field of view) keyframe data
		if (fov_kf.kf_list) {
			// !! ADD CODE !! At this line you would create an animation controller in your
			//                export file scene graph to hold the camera FOV keyframe data
			exportFileFormatsAnimChanController = NI_Exporter_CreateFloatControllerBasedOnExplicitInterpolationMethod(fov_kf.type);
			NI_Exporter_OutputRawNuGrafToolkitKeyFrameData(ofp, Nt_FOV, &fov_kf, exportFileFormatsAnimChanController, Nc_FLAG__CONVERT_TO_RADIANS);
		}

		// And free up the keyframe lists we allocated in MAX_Setup_Resampled_Object_Animation_Keyframe_Lists()
		if (x_trans_kf.kf_list) 	Ni_Free_Memory((char *) x_trans_kf.kf_list);
		if (y_trans_kf.kf_list)		Ni_Free_Memory((char *) y_trans_kf.kf_list);
		if (z_trans_kf.kf_list)		Ni_Free_Memory((char *) z_trans_kf.kf_list);
		if (x_lookat_kf.kf_list)	Ni_Free_Memory((char *) x_lookat_kf.kf_list);
		if (y_lookat_kf.kf_list)	Ni_Free_Memory((char *) y_lookat_kf.kf_list);
		if (z_lookat_kf.kf_list)	Ni_Free_Memory((char *) z_lookat_kf.kf_list);
		if (fov_kf.kf_list)		Ni_Free_Memory((char *) fov_kf.kf_list);
		if (roll_kf.kf_list)		Ni_Free_Memory((char *) roll_kf.kf_list);
	} else {
		//
		// Else, the following code reverts to creating temporary resampled keyframe lists internally
		// inside the toolkit which conform to the animation methods the exporter requires.
		//

		Export_IO_UpdateStatusDisplay("camera", camera_name, "Exporting animation data.");

        	/* Go and resample all the camera-related keyframe lists */
        	if (export_options->use_independent_controllers_for_tripled_anim_channels) {
        		// Resample the animation to independent X, Y and Z curves. Each curve could have
        		// different number of keys and different keyframe times
        		Ni_AnimResampleAndEnumerateKeyframeList_Start(
        			/* Return status: FALSE = no error, otherwise, see error list */
        			&Nv_Return_Status,
        			/* Pointer to the toolkit's local data is returned here for safe keeping. */
        			&Nv_FunctionInstanceLocalDataPtr,
        			/* Handle type and name of the Comm_Type to be enumerated (ie: Nt_CMAERA, "camera1") */
        			Nt_CAMERA, camera_name,
        			/* This is the order to extract any new Euler rotation angles (see ni3.h for list) */
        			/* This is only needed when Euler rotation angles are being asked for. */
        			/* Lightwave, for example, uses 'Nc_EulerRotationOrder_YXZ'. */
        			Nc_EulerRotationOrder_XYZ,
        			/* This is only needed when Euler rotation angles are being asked for. */
        			/* If TRUE, then the camera orientation matrix will have its Z value */
        			/* negated (RHCS -> LHCS) which is useful for Lightwave, but not Max. */
        			Nc_FALSE,

				// If the source Euler rotation curve flips back and forth on each
				// side of -180/180 degrees (like runs of -179, followed by 179.0)
				// then this routine will enable resampling on those rotation curves 
				// and then make sure the rotation values don't flip back and forth any more.
				Nt_FIXEULERROTATIONANGLEFLIPS, Nt_ENABLED, (export_options->flip_Euler_180flips_via_resampler ? Nt_ON : Nt_OFF), Nt_CMDSEP,

				// Specify the animation interpolation controller for the resampled output fcurves. 
				// (One of: Nt_ORIGINAL, Nt_KFCONSTANT, Nt_KFLINEAR, Nt_KFBEZIER, Nt_KFTCB)
				// If we set it to Nt_ORIGINAL then the resampler will use the same 
				// interplation type as the source animation curves that are being
				// fed to the resampler, unless it is not one of the allowable 
				// controller types set with the  Nt_ALLOWEDCONTROLLERTYPES option.
				Nt_TARGETCONTROLLERTYPE, NI_Exporter_Get_Users_Desired_Target_Animation_Controller_Type(), Nt_CMDSEP,

				// This specifies the keyframe controller types which are supported by the our exporter.
				// If the 'Nt_TARGETCONTROLLERTYPE' is set to Nt_ORIGINAL, then each
				// controller will be compared against this list of allowed controller types.
				// If the original curve's type matches, then that type will be used. If it
				// does not match, then the first item from this list will be used.
				Nt_ALLOWEDCONTROLLERTYPES, Nt_KFBEZIER, Nt_KFTCB, Nt_KFCONSTANT, Nt_KFLINEAR, Nt_CMDSEP,

				// When the 'Nt_TARGETCONTROLLERTYPE' fcurve interpolation method is set to Nt_KFBEZIER, the 
				// following list determines which Bezier tangent types are supported by this exporter 
				// as a way to create and return the new Bezier tangents. 
				Nt_ALLOWEDBEZIERTANGENTTYPES, Nt_CUSTOM, Nt_CONSTANT, Nt_LINEAR, Nt_SMOOTH, Nt_FAST, Nt_SLOW, Nt_CLAMPED, Nt_PLATEAU, Nt_AUTOFLAT, Nt_BROKENTANGENTS, Nt_CONSTANTVELOCITY, Nt_CMDSEP,

#if 00	// Valid, but not used in this example exporter
				// When resampling any rotation channels, this function will pre-multiply the
				//   given (quaternion) rotation into the evaluated rotation matrix.
				// The fundamental purpose of this feature is to allow lights to be "repointed"
				//   down an axis more appropriate for the target animation system.
				// However, it can also be used to tweak the resampled rotation values from
				//   any ordinary euler-quat, euler-euler, or quat-quat resampling as well.
				Nt_ROTATIONTWEAK, (Nd_Quat *) Nv_Rotation_Tweak_Quaternion, Nt_CMDSEP,
#endif

        			/* This is the list of channels to resample/inquire */
				Nt_ANIMCHAN,
					Nt_XTRANSLATE, Nt_YTRANSLATE, Nt_ZTRANSLATE,
					Nt_XLOOKAT, Nt_YLOOKAT, Nt_ZLOOKAT,
					Nt_ROLL, Nt_FOV,
        				Nt_CMDEND);
        	} else {
        		// Else, let's utilize the special feature in the animation resampler routine to
			// merge 3 distinct XYZ animation channels into a single XYZ animation channel
			// for those export file formats which can handle a single XYZ value at each key.
        		Ni_AnimResampleAndEnumerateKeyframeList_Start(
        			&Nv_Return_Status,
        			&Nv_FunctionInstanceLocalDataPtr,
        			Nt_CAMERA, camera_name,
        			Nc_EulerRotationOrder_XYZ,
        			Nc_FALSE,
				Nt_FIXEULERROTATIONANGLEFLIPS, Nt_ENABLED, (export_options->flip_Euler_180flips_via_resampler ? Nt_ON : Nt_OFF), Nt_CMDSEP,
				Nt_TARGETCONTROLLERTYPE, NI_Exporter_Get_Users_Desired_Target_Animation_Controller_Type(), Nt_CMDSEP,
				Nt_ALLOWEDCONTROLLERTYPES, Nt_KFBEZIER, Nt_KFTCB, Nt_KFCONSTANT, Nt_KFLINEAR, Nt_CMDSEP,
				Nt_ALLOWEDBEZIERTANGENTTYPES, Nt_CUSTOM, Nt_CONSTANT, Nt_LINEAR, Nt_SMOOTH, Nt_FAST, Nt_SLOW, Nt_CLAMPED, Nt_PLATEAU, Nt_AUTOFLAT, Nt_BROKENTANGENTS, Nt_CONSTANTVELOCITY, Nt_CMDSEP,
        			Nt_ANIMCHAN,
        				// This will cause each set of XYZ channels to be modified to that they
        				// share the same set of key times. They will still be queried by their
        				// single respective channel however.
					Nt_XYZTRANSLATE,
					Nt_XYZLOOKAT,
					Nt_ROLL, Nt_FOV,
        				Nt_CMDEND);
        	}

        	if (Nv_Return_Status != Nc_RESAMPLEDKFLIST_STATUS_NO_NEW_KF_LISTS_CREATED && (Nv_Return_Status || Nv_Return_Status == Nc_RESAMPLEDKFLIST_STATUS_ALL_KEYS_AT_SAME_TIME))
        		/* Either an error was encontered, or no valid animation data */
        		/* exists, or all the keys exist at 1 point in time in which case */
        		/* we'll just output the ctm and no anim. info. for this instance. */
        		goto exit;

		if (!export_options->use_independent_controllers_for_tripled_anim_channels) {
        		// Export the animation by converting 3 channel keyframe lists to single channel (triple-value)
        		// keyframe lists inside the toolkit, then outputting them as a single controller.

			// !! ADD CODE !! At this line you would create animation controllers in your
			//                export file scene graph to hold the camera look-from location (single channel, XYZ),
			//		  look-at location (single channel, XYZ), ROLL value and FOV value.

	                OPTIONAL_FPRINTF(ofp, "\n\tCamera animation (resampled to single channel, 3-component) data:\n");

        		// Send the look-from channels from the NuGraf toolkit to the exporter as a single XYZ animation controller channel
			exportFileFormatsAnimChanController = NI_Exporter_CreateADefaultPositionController();
        		NI_Exporter_QueryAndOutputResampledAnimationKeyframeDataFromOkinoToolkit(ofp, Nt_XYZTRANSLATE, exportFileFormatsAnimChanController, &animation_was_output, 0);

        		// Send the look-at channels from the NuGraf toolkit as a single XYZ animation controller channel
			exportFileFormatsAnimChanController = NI_Exporter_CreateADefaultPositionController();
        		NI_Exporter_QueryAndOutputResampledAnimationKeyframeDataFromOkinoToolkit(ofp, Nt_XYZLOOKAT, exportFileFormatsAnimChanController, &animation_was_output, 0);

			// Send the roll channels from the NuGraf toolkit
			exportFileFormatsAnimChanController = NI_Exporter_CreateADefaultFloatController();
        		NI_Exporter_QueryAndOutputResampledAnimationKeyframeDataFromOkinoToolkit(ofp, Nt_ROLL, exportFileFormatsAnimChanController, &animation_was_output, Nc_FLAG__CONVERT_TO_RADIANS);

			// Send the FOV channels from the NuGraf toolkit
			exportFileFormatsAnimChanController = NI_Exporter_CreateADefaultFloatController();
        		NI_Exporter_QueryAndOutputResampledAnimationKeyframeDataFromOkinoToolkit(ofp, Nt_FOV, exportFileFormatsAnimChanController, &animation_was_output, Nc_FLAG__CONVERT_TO_RADIANS);
        	} else {
        		// Else, export the 3 channel keyframe lists as distinct controllers of their own.

			// !! ADD CODE !! At this line you would create animation controllers in your
			//                export file scene graph to hold the camera look-from location (3-channel),
			//		  look-at location (3-channel), ROLL value and FOV value.

	                OPTIONAL_FPRINTF(ofp, "\n\tCamera animation (resampled, single channels) data:\n");

        		// Send the look-from translation channels from the NuGraf toolkit as 3 separate animation channels
			NI_Exporter_Output3RelatedResampledAnimationKeyFrameLists(ofp, Nc_FALSE, Nt_XTRANSLATE, Nt_YTRANSLATE, Nt_ZTRANSLATE, &animation_was_output);

        		// Send the look-at translation channels from the NuGraf toolkit as 3 separate animation channels
			NI_Exporter_Output3RelatedResampledAnimationKeyFrameLists(ofp, Nc_FALSE, Nt_XLOOKAT, Nt_YLOOKAT, Nt_ZLOOKAT, &animation_was_output);

			// Send the roll channels from the NuGraf toolkit
			exportFileFormatsAnimChanController = NI_Exporter_CreateADefaultFloatController();
        		NI_Exporter_QueryAndOutputResampledAnimationKeyframeDataFromOkinoToolkit(ofp, Nt_ROLL, exportFileFormatsAnimChanController, &animation_was_output, Nc_FLAG__CONVERT_TO_RADIANS);

			// Send the FOV channels from the NuGraf toolkit
			exportFileFormatsAnimChanController = NI_Exporter_CreateADefaultFloatController();
        		NI_Exporter_QueryAndOutputResampledAnimationKeyframeDataFromOkinoToolkit(ofp, Nt_FOV, exportFileFormatsAnimChanController, &animation_was_output, Nc_FLAG__CONVERT_TO_RADIANS);
        	}
	}

	/* Free up the resampled keyframe lists */
exit:	if (Nv_FunctionInstanceLocalDataPtr) {
		Ni_AnimResampleAndEnumerateKeyframeList_End(&Nv_Return_Status, Nv_FunctionInstanceLocalDataPtr);
		Nv_FunctionInstanceLocalDataPtr = NULL;
	}
}

// -------------->>>>  Light Animation Query & Output Code  <<<<-----------------

// Call this single routine to output any possible animation data associated with
// a light definition.

// Returns TRUE if valid animation data is ready to go out, else FALSE
// if no animation data should be output.

	Nd_Void
NI_Exporter_Output_Light_Animation_Data( FILE *ofp, char *light_name)
{
	short		animation_was_output, light_uses_quaternion_controller;
	Nd_Token	Nv_Light_Type, token1, token2, token3;
	Nd_Int		Nv_Return_Status;
	void		*exportFileFormatsAnimChanController;	// Replace this with your own animation controller pointer type

	if (!export_options->ena_light_animation)
		return;

	// Resampled animation data will be created, held, queried and then deleted via this handle pointer.
	Nv_FunctionInstanceLocalDataPtr = NULL;

	// Get the type of this light source
	Ni_Inquire_Light(light_name, Nt_MODEL, (Nd_Token *) &Nv_Light_Type, Nt_CMDEND);

	// If the export conveter wishes to attempt raw keyframe output mode then query for the existence
	// of raw animation keyframe lists...
	if (export_options->export_raw_unsampled_anim_data_if_possible) {
		// Try to export the animation data using raw keyframes from the NuGraf toolkit
		// instead of the resampled data. The data will have to come in via independent controllers.
		LightAnimationPtrs_Info anim_ptrs;

		// Get pointers to the various animation channels and controllers associated with this instance
		if (!Export_IO_RetrieveAllLightAnimationChannelPtrs(light_name, &anim_ptrs, Nc_FALSE, NULL))
			goto exit;

		// Go retrieve all the keyframes and related information for the controllers we need
		Export_IO_Anim_RetrieveControllerinfo(NULL, anim_ptrs.x_translate_ctrlr_ptr, &x_translate_kf, Nc_TRUE);
		Export_IO_Anim_RetrieveControllerinfo(NULL, anim_ptrs.y_translate_ctrlr_ptr, &y_translate_kf, Nc_TRUE);
		Export_IO_Anim_RetrieveControllerinfo(NULL, anim_ptrs.z_translate_ctrlr_ptr, &z_translate_kf, Nc_TRUE);
		Export_IO_Anim_RetrieveControllerinfo(NULL, anim_ptrs.x_shineat_ctrlr_ptr, &x_shineat_kf, Nc_TRUE);
		Export_IO_Anim_RetrieveControllerinfo(NULL, anim_ptrs.y_shineat_ctrlr_ptr, &y_shineat_kf, Nc_TRUE);
		Export_IO_Anim_RetrieveControllerinfo(NULL, anim_ptrs.z_shineat_ctrlr_ptr, &z_shineat_kf, Nc_TRUE);
		Export_IO_Anim_RetrieveControllerinfo(NULL, anim_ptrs.shinedir_quat_ctrlr_ptr, &shinedir_quat_kf, Nc_TRUE);
		Export_IO_Anim_RetrieveControllerinfo(NULL, anim_ptrs.x_shinedir_eulerrot_ctrlr_ptr, &x_shinedir_eulerrot_kf, Nc_TRUE);
		Export_IO_Anim_RetrieveControllerinfo(NULL, anim_ptrs.y_shinedir_eulerrot_ctrlr_ptr, &y_shinedir_eulerrot_kf, Nc_TRUE);
		Export_IO_Anim_RetrieveControllerinfo(NULL, anim_ptrs.z_shinedir_eulerrot_ctrlr_ptr, &z_shinedir_eulerrot_kf, Nc_TRUE);
		Export_IO_Anim_RetrieveControllerinfo(NULL, anim_ptrs.r_color_ctrlr_ptr, &r_color_kf, Nc_TRUE);
		Export_IO_Anim_RetrieveControllerinfo(NULL, anim_ptrs.g_color_ctrlr_ptr, &g_color_kf, Nc_TRUE);
		Export_IO_Anim_RetrieveControllerinfo(NULL, anim_ptrs.b_color_ctrlr_ptr, &b_color_kf, Nc_TRUE);

		// Let's see if we got any valid animation data
		if (!(x_translate_kf.kf_list || y_translate_kf.kf_list || z_translate_kf.kf_list ||
				x_shineat_kf.kf_list || y_shineat_kf.kf_list || z_shineat_kf.kf_list ||
				shinedir_quat_kf.kf_list ||
				x_shinedir_eulerrot_kf.kf_list || y_shinedir_eulerrot_kf.kf_list || z_shinedir_eulerrot_kf.kf_list ||
				r_color_kf.kf_list || g_color_kf.kf_list || b_color_kf.kf_list))
			goto exit;	// No valid animation data

                OPTIONAL_FPRINTF(ofp, "\n\tLight animation (raw keyframe) data:\n");

		Export_IO_UpdateStatusDisplay("light", light_name, "Exporting animation data.");

		// Since we've checked that there is valid keyframe data above, we'll assume we are outputting valid animation
		animation_was_output = Nc_TRUE;

		// Send the raw shine-from XYZ translation keyframes (as 3 separate animation keyframe lists)
		// This is only used by point & spot light sources, not directional lights.
		if ((Nv_Light_Type == Nt_SPOT || Nv_Light_Type == Nt_POINT) && (x_translate_kf.kf_list || y_translate_kf.kf_list || z_translate_kf.kf_list))
			// !! ADD CODE !! At this line you would create an animation controller in your
			//                export file scene graph to hold the light look-from translation keyframe data (3 separate channels)
			NI_Exporter_Output3RelatedRawAnimationKeyFrameLists(ofp, Nc_FALSE,
				Nt_XTRANSLATE, Nt_YTRANSLATE, Nt_ZTRANSLATE,
				&x_translate_kf, &y_translate_kf, &z_translate_kf);

		// Send the raw shine-at translation keyframes (as 3 separate animation keyframe lists)
		// This is used only for spot light sources.
		if (Nv_Light_Type == Nt_SPOT && (x_shineat_kf.kf_list || y_shineat_kf.kf_list || z_shineat_kf.kf_list))
			// !! ADD CODE !! At this line you would create an animation controller in your
			//                export file scene graph to hold the light look-at translation keyframe data (3 separate channels)
			NI_Exporter_Output3RelatedRawAnimationKeyFrameLists(ofp, Nc_FALSE,
				Nt_XSHINEAT, Nt_YSHINEAT, Nt_ZSHINEAT,
				&x_shineat_kf, &y_shineat_kf, &z_shineat_kf);
		// Send the raw shine-direction Euler rotation keyframes (as 3 separate Euler animation keyframe lists)
		// This is used only for directional light sources (no location, just direction), whose rotation has been set using Euler rotation angles.
		else if (Nv_Light_Type == Nt_DIRECTION && (x_shinedir_eulerrot_kf.kf_list || y_shinedir_eulerrot_kf.kf_list || z_shinedir_eulerrot_kf.kf_list))
			// !! ADD CODE !! At this line you would create an animation controller in your
			//                export file scene graph to hold the light look-at translation keyframe data (3 separate channels)
			// !! NOTE: Somewhere you need to call 'NI_Exporter_ChangeEulerRotationControllerOrder()' to set up the Euler rotation order properly on this controller.
			NI_Exporter_Output3RelatedRawAnimationKeyFrameLists(ofp, Nc_TRUE,
				Nt_XSHINEDIR, Nt_YSHINEDIR, Nt_ZSHINEDIR,
				&x_shinedir_eulerrot_kf, &y_shinedir_eulerrot_kf, &z_shinedir_eulerrot_kf);
		// Send the raw shine-direction Quaternion rotation keyframes (as one Quaternion animation keyframe list)
		// This is used only for directional light sources (no location, just direction), whose rotation has been set by a quaternion
		else if (Nv_Light_Type == Nt_DIRECTION && shinedir_quat_kf.kf_list) {
			// Create an animation controller based on the type of this NuGraf animation controller
			exportFileFormatsAnimChanController = NI_Exporter_CreateQuatRotationControllerBasedOnExplicitInterpolationMethod(shinedir_quat_kf.type);

			// Send the raw Quaternion or Angle/Axis rotation keyframes
			NI_Exporter_OutputRawNuGrafToolkitKeyFrameData(ofp, Nt_QUATSHINEDIR, &shinedir_quat_kf, exportFileFormatsAnimChanController, Nc_FLAG__THIS_IS_A_QUAT_CHANNEL);
		}

		// Send the raw light RGB color keyframes (as 3 separate animation keyframe lists)
		if (r_color_kf.kf_list || g_color_kf.kf_list || b_color_kf.kf_list) {
			// !! ADD CODE !! At this line you would create an animation controller in your
			//                export file scene graph to hold the light color keyframe data (3 separate channels)
			NI_Exporter_Output3RelatedRawAnimationKeyFrameLists(ofp, Nc_FALSE,
    				Nt_REDCOLOR, Nt_GREENCOLOR, Nt_BLUECOLOR,
				&r_color_kf, &g_color_kf, &b_color_kf);
		}

		// And free up the keyframe lists we allocated in MAX_Setup_Resampled_Object_Animation_Keyframe_Lists()
		if (x_translate_kf.kf_list) 		Ni_Free_Memory((char *) x_translate_kf.kf_list);
		if (y_translate_kf.kf_list) 		Ni_Free_Memory((char *) y_translate_kf.kf_list);
		if (z_translate_kf.kf_list) 		Ni_Free_Memory((char *) z_translate_kf.kf_list);
		if (x_shineat_kf.kf_list) 		Ni_Free_Memory((char *) x_shineat_kf.kf_list);
		if (y_shineat_kf.kf_list) 		Ni_Free_Memory((char *) y_shineat_kf.kf_list);
		if (z_shineat_kf.kf_list) 		Ni_Free_Memory((char *) z_shineat_kf.kf_list);
		if (shinedir_quat_kf.kf_list) 		Ni_Free_Memory((char *) shinedir_quat_kf.kf_list);
		if (x_shinedir_eulerrot_kf.kf_list) 	Ni_Free_Memory((char *) x_shinedir_eulerrot_kf.kf_list);
		if (y_shinedir_eulerrot_kf.kf_list) 	Ni_Free_Memory((char *) y_shinedir_eulerrot_kf.kf_list);
		if (z_shinedir_eulerrot_kf.kf_list) 	Ni_Free_Memory((char *) z_shinedir_eulerrot_kf.kf_list);
		if (r_color_kf.kf_list) 		Ni_Free_Memory((char *) r_color_kf.kf_list);
		if (g_color_kf.kf_list) 		Ni_Free_Memory((char *) g_color_kf.kf_list);
		if (b_color_kf.kf_list) 		Ni_Free_Memory((char *) b_color_kf.kf_list);
	} else {
		//
		// Else, the following code reverts to creating temporary resampled keyframe lists internally
		// inside the toolkit which conform to the animation methods the exporter requires.
		//

        	// Determine if the light uses a quaternion controller or a Euler controller for its direction (directional lights only)
		light_uses_quaternion_controller = NI_Exporter_DoesLightUseAQuaternionControllerForDirection(light_name);

		// Set the rotation order for Eulers resampled and queried from the toolkit. We'll use
		// the method specified on the export options "Animation" panel.
		//
		// !! Normally an export converter developer will hard code this parameter right here and
		//    not ask the user for the rotation method. This is because most export file formats
		//    use a fixed Euler rotation order. For example, set it to 'Nc_EulerRotationOrder_XYZ'
		//    (the various orders are listed in the ni4_aux.h header file)
		//
		curr_euler_rotation_controller_order = (Nd_EulerRotationOrder) export_options->ena_object_anim_resampled_rot_order;

        	if (Nv_Light_Type == Nt_DIRECTION && export_options->rotation_controller_type_to_use_during_export == Nc_NRS_ROTATION_CONTROLLER_AS_PER_NRS_TOOLKIT && light_uses_quaternion_controller)
	        	// If dialog box is set to "as-per-toolkit rotation export method" then set the resampled
        		// rotation method (Quat or Euler) based on what type of rotation controller is used inside NuGraf.
			// This will be used to retrieve & resample a directional light's direction quaternion rotation animation channel
        		token1 = token2 = token3 = Nt_QUATSHINEDIR;
        	else if (Nv_Light_Type == Nt_DIRECTION && export_options->rotation_controller_type_to_use_during_export == Nc_NRS_ROTATION_CONTROLLER_QUATERNION)
        		// Else, the user wishes to override the toolkit rotation method with Quaternion export
			// This will be used to retrieve & resample a directional light's direction quaternion rotation animation channel
        	 	token1 = token2 = token3 = Nt_QUATSHINEDIR;
		else if (Nv_Light_Type == Nt_DIRECTION) {
			// Else, query the rotations an separate X, Y and Z Euler rotations. If the exporter
			// wants to query the Eulers as a single XYZ Euler keyframe list then the code further
			// below changes all these 3 tokens to 'Nt_XYZROTATE' so that the resampled Eulers get
			// merged together into a single Euler rotation keyframe list deep inside the Okino toolkit.
			// This will be used to retrieve & resample a directional light's direction Euler rotation animation channel
			token1 = Nt_XSHINEDIR;
			token2 = Nt_YSHINEDIR;
			token3 = Nt_ZSHINEDIR;
		} else {
			// Else, this is not a directional light, so go resample the shine-at animation (really, only used for spot lights)
			token1 = Nt_XSHINEAT;
			token2 = Nt_YSHINEAT;
			token3 = Nt_ZSHINEAT;
		}

		Export_IO_UpdateStatusDisplay("light", light_name, "Exporting animation data.");

        	/* Go and resample all the light-related keyframe lists */
        	if (export_options->use_independent_controllers_for_tripled_anim_channels) {
        		// Resample the animation to independent X, Y and Z curves. Each curve could have
        		// different number of keys and different keyframe times
        		Ni_AnimResampleAndEnumerateKeyframeList_Start(
        			/* Return status: FALSE = no error, otherwise, see error list */
        			&Nv_Return_Status,
        			/* Pointer to the toolkit's local data is returned here for safe keeping. */
        			&Nv_FunctionInstanceLocalDataPtr,
        			/* Handle type and name of the Comm_Type to be enumerated (ie: Nt_CMAERA, "camera1") */
        			Nt_LIGHT, light_name,
        			/* This is the order to extract any new Euler rotation angles (see ni3.h for list) */
        			/* This is only needed when Euler rotation angles are being asked for. */
        			/* Lightwave, for example, uses 'Nc_EulerRotationOrder_YXZ'. */
        			curr_euler_rotation_controller_order,
        			/* This is only needed when Euler rotation angles are being asked for. */
        			/* If TRUE, then the camera orientation matrix will have its Z value */
        			/* negated (RHCS -> LHCS) which is useful for Lightwave, but not Max. */
        			Nc_FALSE,

				// If the source Euler rotation curve flips back and forth on each
				// side of -180/180 degrees (like runs of -179, followed by 179.0)
				// then this routine will enable resampling on those rotation curves 
				// and then make sure the rotation values don't flip back and forth any more.
				Nt_FIXEULERROTATIONANGLEFLIPS, Nt_ENABLED, (export_options->flip_Euler_180flips_via_resampler ? Nt_ON : Nt_OFF), Nt_CMDSEP,

				// Specify the animation interpolation controller for the resampled output fcurves. 
				// (One of: Nt_ORIGINAL, Nt_KFCONSTANT, Nt_KFLINEAR, Nt_KFBEZIER, Nt_KFTCB)
				// If we set it to Nt_ORIGINAL then the resampler will use the same 
				// interplation type as the source animation curves that are being
				// fed to the resampler, unless it is not one of the allowable 
				// controller types set with the  Nt_ALLOWEDCONTROLLERTYPES option.
				Nt_TARGETCONTROLLERTYPE, NI_Exporter_Get_Users_Desired_Target_Animation_Controller_Type(), Nt_CMDSEP,

				// This specifies the keyframe controller types which are supported by the our exporter.
				// If the 'Nt_TARGETCONTROLLERTYPE' is set to Nt_ORIGINAL, then each
				// controller will be compared against this list of allowed controller types.
				// If the original curve's type matches, then that type will be used. If it
				// does not match, then the first item from this list will be used.
				Nt_ALLOWEDCONTROLLERTYPES, Nt_KFBEZIER, Nt_KFTCB, Nt_KFCONSTANT, Nt_KFLINEAR, Nt_CMDSEP,

				// When the 'Nt_TARGETCONTROLLERTYPE' fcurve interpolation method is set to Nt_KFBEZIER, the 
				// following list determines which Bezier tangent types are supported by this exporter 
				// as a way to create and return the new Bezier tangents. 
				Nt_ALLOWEDBEZIERTANGENTTYPES, Nt_CUSTOM, Nt_CONSTANT, Nt_LINEAR, Nt_SMOOTH, Nt_FAST, Nt_SLOW, Nt_CLAMPED, Nt_PLATEAU, Nt_AUTOFLAT, Nt_BROKENTANGENTS, Nt_CONSTANTVELOCITY, Nt_CMDSEP,

#if 00	// Valid, but not used in this example exporter
				// When resampling any rotation channels, this function will pre-multiply the
				//   given (quaternion) rotation into the evaluated rotation matrix.
				// The fundamental purpose of this feature is to allow lights to be "repointed"
				//   down an axis more appropriate for the target animation system.
				// However, it can also be used to tweak the resampled rotation values from
				//   any ordinary euler-quat, euler-euler, or quat-quat resampling as well.
				Nt_ROTATIONTWEAK, (Nd_Quat *) Nv_Rotation_Tweak_Quaternion, Nt_CMDSEP,
#endif
        			/* This is the list of channels to resample/inquire */
				Nt_ANIMCHAN,
					Nt_XTRANSLATE, Nt_YTRANSLATE, Nt_ZTRANSLATE,
					token1, token2, token3,
					Nt_REDCOLOR, Nt_GREENCOLOR, Nt_BLUECOLOR,
        				Nt_CMDEND);
        	} else {
        		// Else, let's utilize the special feature in the animation resampler routine to
			// merge 3 distinct XYZ animation channels into a single XYZ animation channel
			// for those export file formats which can handle a single XYZ value at each key.

			// Decide if we want to query for XYZ Euler rotations or possible XYZ shine-at animation
			if (token1 == Nt_XSHINEDIR)		// We are not doing quats
				token1 = Nt_XYZSHINEDIR;	// So, let's ask for merged XYZ Euler keys instead
			else if (token1 == Nt_XSHINEAT)		// We are not doing quats
				token1 = Nt_XYZSHINEAT;		// So, let's ask for merged XYZ Euler keys instead

        		Ni_AnimResampleAndEnumerateKeyframeList_Start(
        			&Nv_Return_Status,
        			&Nv_FunctionInstanceLocalDataPtr,
        			Nt_LIGHT, light_name,
        			curr_euler_rotation_controller_order,
        			Nc_FALSE,
				Nt_FIXEULERROTATIONANGLEFLIPS, Nt_ENABLED, (export_options->flip_Euler_180flips_via_resampler ? Nt_ON : Nt_OFF), Nt_CMDSEP,
				Nt_TARGETCONTROLLERTYPE, NI_Exporter_Get_Users_Desired_Target_Animation_Controller_Type(), Nt_CMDSEP,
				Nt_ALLOWEDCONTROLLERTYPES, Nt_KFBEZIER, Nt_KFTCB, Nt_KFCONSTANT, Nt_KFLINEAR, Nt_CMDSEP,
				Nt_ALLOWEDBEZIERTANGENTTYPES, Nt_CUSTOM, Nt_CONSTANT, Nt_LINEAR, Nt_SMOOTH, Nt_FAST, Nt_SLOW, Nt_CLAMPED, Nt_PLATEAU, Nt_AUTOFLAT, Nt_BROKENTANGENTS, Nt_CONSTANTVELOCITY, Nt_CMDSEP,
        			Nt_ANIMCHAN,
        				// This will cause each set of XYZ channels to be modified to that they
        				// share the same set of key times. They will still be queried by their
        				// single respective channel however.
					Nt_XYZTRANSLATE,
					token1,
					Nt_RGBCOLOR,
        				Nt_CMDEND);
        	}

        	if (Nv_Return_Status != Nc_RESAMPLEDKFLIST_STATUS_NO_NEW_KF_LISTS_CREATED && (Nv_Return_Status || Nv_Return_Status == Nc_RESAMPLEDKFLIST_STATUS_ALL_KEYS_AT_SAME_TIME))
        		/* Either an error was encontered, or no valid animation data */
        		/* exists, or all the keys exist at 1 point in time in which case */
        		/* we'll just output the ctm and no anim. info. for this instance. */
        		goto exit;

		if (!export_options->use_independent_controllers_for_tripled_anim_channels) {
        		// Export the animation by converting 3 channel keyframe lists to single channel (triple-value)
        		// keyframe lists inside the toolkit, then outputting them as a single controller.

			// !! ADD CODE !! At this line you would create animation controllers in your
			//                export file scene graph to hold the light look-from location (single channel, XYZ),
			//		  look-at location (single channel, XYZ), ROLL value and FOV value.

	                OPTIONAL_FPRINTF(ofp, "\n\tLight animation (resampled to single channel, 3-component) data:\n");

        		// Send the look-from channels from the NuGraf toolkit to the exporter as a single XYZ animation controller channel
			// This is only used by point & spot light sources, not directional lights.
			if (Nv_Light_Type == Nt_SPOT || Nv_Light_Type == Nt_POINT) {
				exportFileFormatsAnimChanController = NI_Exporter_CreateADefaultPositionController();
        			NI_Exporter_QueryAndOutputResampledAnimationKeyframeDataFromOkinoToolkit(ofp, Nt_XYZTRANSLATE, exportFileFormatsAnimChanController, &animation_was_output, 0);
			}

			if (Nv_Light_Type == Nt_SPOT) {
	        		// Send the shine-at channels from the NuGraf toolkit as a single XYZ animation controller channel
				// This is used only for spot light sources.
				exportFileFormatsAnimChanController = NI_Exporter_CreateADefaultPositionController();
        			NI_Exporter_QueryAndOutputResampledAnimationKeyframeDataFromOkinoToolkit(ofp, Nt_XYZSHINEAT, exportFileFormatsAnimChanController, &animation_was_output, 0);
			} else if (Nv_Light_Type == Nt_DIRECTION) {
				if (export_options->rotation_controller_type_to_use_during_export == Nc_NRS_ROTATION_CONTROLLER_QUATERNION
						|| (export_options->rotation_controller_type_to_use_during_export == Nc_NRS_ROTATION_CONTROLLER_AS_PER_NRS_TOOLKIT && light_uses_quaternion_controller)) {
		        		// Send the shine-direction Quaternion rotation channels from the NuGraf toolkit as a single quaternion animation controller channel
					// This is used only for directional light sources (no location, just direction), whose rotation has been set by a quaternion
					exportFileFormatsAnimChanController = NI_Exporter_CreateADefaultQuatRotationController();
        				NI_Exporter_QueryAndOutputResampledAnimationKeyframeDataFromOkinoToolkit(ofp, Nt_QUATSHINEDIR, exportFileFormatsAnimChanController, &animation_was_output, 0);
				} else {
		        		// Send the shine-direction Euler rotation channels from the NuGraf toolkit as a single Euler XYZ animation controller channel
					// This is used only for directional light sources (no location, just direction), whose rotation has been set using Euler rotation angles.
					exportFileFormatsAnimChanController = NI_Exporter_CreateADefaulEulerRotationController();

					// Change the Euler rotation order of the Euler controller to match that of NuGraf
					NI_Exporter_ChangeEulerRotationControllerOrder(ofp, exportFileFormatsAnimChanController, curr_euler_rotation_controller_order);

					// Send the single channel, 3 component XYZ Euler light shine direction keys to the animation controller
        				NI_Exporter_QueryAndOutputResampledAnimationKeyframeDataFromOkinoToolkit(ofp, Nt_XYZSHINEDIR, exportFileFormatsAnimChanController, &animation_was_output, 0);
				}
			}

        		// Send the light's RGB color channels from the NuGraf toolkit to the exporter as a single RGB animation controller channel
			exportFileFormatsAnimChanController = NI_Exporter_CreateADefaultRGBColorController();
        		NI_Exporter_QueryAndOutputResampledAnimationKeyframeDataFromOkinoToolkit(ofp, Nt_RGBCOLOR, exportFileFormatsAnimChanController, &animation_was_output, 0);
        	} else {
        		// Else, export the 3 channel keyframe lists as distinct controllers of their own.

			// !! ADD CODE !! At this line you would create animation controllers in your
			//                export file scene graph to hold the light look-from location (3-channel),
			//		  look-at location (3-channel), ROLL value and FOV value.

	                OPTIONAL_FPRINTF(ofp, "\n\tLight animation (resampled, single channels) data:\n");

        		// Send the look-from translation channels from the NuGraf toolkit as 3 separate animation channels
			// This is only used by point & spot light sources, not directional lights.
			if (Nv_Light_Type == Nt_SPOT || Nv_Light_Type == Nt_POINT)
				NI_Exporter_Output3RelatedResampledAnimationKeyFrameLists(ofp, Nc_FALSE, Nt_XTRANSLATE, Nt_YTRANSLATE, Nt_ZTRANSLATE, &animation_was_output);

			if (Nv_Light_Type == Nt_SPOT)
	        		// Send the shine-at translation channels from the NuGraf toolkit as 3 separate animation channels
				// This is used only for spot light sources.
				NI_Exporter_Output3RelatedResampledAnimationKeyFrameLists(ofp, Nc_FALSE, Nt_XSHINEAT, Nt_YSHINEAT, Nt_ZSHINEAT, &animation_was_output);
			else if (Nv_Light_Type == Nt_DIRECTION) {
				if (export_options->rotation_controller_type_to_use_during_export == Nc_NRS_ROTATION_CONTROLLER_QUATERNION
						|| (export_options->rotation_controller_type_to_use_during_export == Nc_NRS_ROTATION_CONTROLLER_AS_PER_NRS_TOOLKIT && light_uses_quaternion_controller)) {
		        		// Send the shine-direction Quaternion rotation channels from the NuGraf toolkit as a single quaternion animation controller channel
					// This is used only for directional light sources (no location, just direction), whose rotation has been set by a quaternion
					exportFileFormatsAnimChanController = NI_Exporter_CreateADefaultQuatRotationController();
        				NI_Exporter_QueryAndOutputResampledAnimationKeyframeDataFromOkinoToolkit(ofp, Nt_QUATSHINEDIR, exportFileFormatsAnimChanController, &animation_was_output, 0);
				} else {
		        		// Send the shine-direction Euler rotation channels from the NuGraf toolkit as a single Euler XYZ animation controller channel
					// This is used only for directional light sources (no location, just direction), whose rotation has been set using Euler rotation angles.
					NI_Exporter_Output3RelatedResampledAnimationKeyFrameLists(ofp, Nc_TRUE, Nt_XSHINEDIR, Nt_YSHINEDIR, Nt_ZSHINEDIR, &animation_was_output);
				}
			}

        		// Send the light's RGB color channels from the NuGraf toolkit to the exporter as as 3 separate animation channels
			NI_Exporter_Output3RelatedResampledAnimationKeyFrameLists(ofp, Nc_FALSE, Nt_REDCOLOR, Nt_GREENCOLOR, Nt_BLUECOLOR, &animation_was_output);
        	}
	}

	/* Free up the resampled keyframe lists */
exit:	if (Nv_FunctionInstanceLocalDataPtr) {
		Ni_AnimResampleAndEnumerateKeyframeList_End(&Nv_Return_Status, Nv_FunctionInstanceLocalDataPtr);
		Nv_FunctionInstanceLocalDataPtr = NULL;
	}
}

// -------------->>>>  Material Animation Query & Output Code  <<<<-----------------

// Call this single routine to output any select animation data associated with
// a material definition.

// Returns TRUE if valid animation data is ready to go out, else FALSE
// if no animation data should be output.

	Nd_Void
NI_Exporter_Output_Material_Animation_Data( FILE *ofp, char *surface_name)
{
	short	animation_was_output;
	Nd_Int	Nv_Return_Status;
	void	*exportFileFormatsAnimChanController;	// Replace this with your own animation controller pointer type

	if (!export_options->ena_material_animation)
		return;

	// Resampled animation data will be created, held, queried and then deleted via this handle pointer.
	Nv_FunctionInstanceLocalDataPtr = NULL;

	// If the export conveter wishes to attempt raw keyframe output mode then query for the existence
	// of raw animation keyframe lists...
	if (export_options->export_raw_unsampled_anim_data_if_possible) {
		// Try to export the animation data using raw keyframes from the NuGraf toolkit
		// instead of the resampled data. The data will have to come in via independent controllers.
		SurfaceAnimationPtrs_Info anim_ptrs;

		// Get pointers to the various animation channels and controllers associated with this instance
		if (!Export_IO_RetrieveAllSurfaceAnimationChannelPtrs(surface_name, &anim_ptrs, Nc_FALSE, NULL))
			goto exit;

		// Go retrieve all the keyframes and related information for the controllers we are interested in looking at
		// (more are available. See the file 'importio.h' for a full list of the channels)
		Export_IO_Anim_RetrieveControllerinfo(NULL, anim_ptrs.r_diffuse_color_ctrlr_ptr, &r_diffuse_color_kf, Nc_TRUE);
		Export_IO_Anim_RetrieveControllerinfo(NULL, anim_ptrs.g_diffuse_color_ctrlr_ptr, &g_diffuse_color_kf, Nc_TRUE);
		Export_IO_Anim_RetrieveControllerinfo(NULL, anim_ptrs.b_diffuse_color_ctrlr_ptr, &b_diffuse_color_kf, Nc_TRUE);
		Export_IO_Anim_RetrieveControllerinfo(NULL, anim_ptrs.r_specular_color_ctrlr_ptr, &r_specular_color_kf, Nc_TRUE);
		Export_IO_Anim_RetrieveControllerinfo(NULL, anim_ptrs.g_specular_color_ctrlr_ptr, &g_specular_color_kf, Nc_TRUE);
		Export_IO_Anim_RetrieveControllerinfo(NULL, anim_ptrs.b_specular_color_ctrlr_ptr, &b_specular_color_kf, Nc_TRUE);
		Export_IO_Anim_RetrieveControllerinfo(NULL, anim_ptrs.r_ambient_color_ctrlr_ptr, &r_ambient_color_kf, Nc_TRUE);
		Export_IO_Anim_RetrieveControllerinfo(NULL, anim_ptrs.g_ambient_color_ctrlr_ptr, &g_ambient_color_kf, Nc_TRUE);
		Export_IO_Anim_RetrieveControllerinfo(NULL, anim_ptrs.b_ambient_color_ctrlr_ptr, &b_ambient_color_kf, Nc_TRUE);
		Export_IO_Anim_RetrieveControllerinfo(NULL, anim_ptrs.r_luminous_color_ctrlr_ptr, &r_luminous_color_kf, Nc_TRUE);
		Export_IO_Anim_RetrieveControllerinfo(NULL, anim_ptrs.g_luminous_color_ctrlr_ptr, &g_luminous_color_kf, Nc_TRUE);
		Export_IO_Anim_RetrieveControllerinfo(NULL, anim_ptrs.b_luminous_color_ctrlr_ptr, &b_luminous_color_kf, Nc_TRUE);
		Export_IO_Anim_RetrieveControllerinfo(NULL, anim_ptrs.r_filter_color_ctrlr_ptr, &r_filter_color_kf, Nc_TRUE);
		Export_IO_Anim_RetrieveControllerinfo(NULL, anim_ptrs.g_filter_color_ctrlr_ptr, &g_filter_color_kf, Nc_TRUE);
		Export_IO_Anim_RetrieveControllerinfo(NULL, anim_ptrs.b_filter_color_ctrlr_ptr, &b_filter_color_kf, Nc_TRUE);
		Export_IO_Anim_RetrieveControllerinfo(NULL, anim_ptrs.ambientShadingCoefficient_ctrlr_ptr, &ambientShadingCoefficient_kf, Nc_TRUE);
		Export_IO_Anim_RetrieveControllerinfo(NULL, anim_ptrs.diffuseShadingCoefficient_ctrlr_ptr, &diffuseShadingCoefficient_kf, Nc_TRUE);
		Export_IO_Anim_RetrieveControllerinfo(NULL, anim_ptrs.specularShadingCoefficient_ctrlr_ptr, &specularShadingCoefficient_kf, Nc_TRUE);
		Export_IO_Anim_RetrieveControllerinfo(NULL, anim_ptrs.luminousShadingCoefficient_ctrlr_ptr, &luminousShadingCoefficient_kf, Nc_TRUE);
		Export_IO_Anim_RetrieveControllerinfo(NULL, anim_ptrs.reflectShadingCoefficient_ctrlr_ptr, &reflectShadingCoefficient_kf, Nc_TRUE);
		Export_IO_Anim_RetrieveControllerinfo(NULL, anim_ptrs.highlightPhongSize_ctrlr_ptr, &highlightPhongSize_kf, Nc_TRUE);
		Export_IO_Anim_RetrieveControllerinfo(NULL, anim_ptrs.faceOpacity_ctrlr_ptr, &faceOpacity_kf, Nc_TRUE);

		// Let's see if we got any valid animation data
		if (!(r_diffuse_color_kf.kf_list || g_diffuse_color_kf.kf_list || b_diffuse_color_kf.kf_list ||
			r_specular_color_kf.kf_list || g_specular_color_kf.kf_list || b_specular_color_kf.kf_list ||
			r_ambient_color_kf.kf_list || g_ambient_color_kf.kf_list || b_ambient_color_kf.kf_list ||
			r_luminous_color_kf.kf_list || g_luminous_color_kf.kf_list || b_luminous_color_kf.kf_list ||
			r_filter_color_kf.kf_list || g_filter_color_kf.kf_list || b_filter_color_kf.kf_list ||
			ambientShadingCoefficient_kf.kf_list || diffuseShadingCoefficient_kf.kf_list ||
			specularShadingCoefficient_kf.kf_list || luminousShadingCoefficient_kf.kf_list ||
			reflectShadingCoefficient_kf.kf_list || faceOpacity_kf.kf_list ||
			highlightPhongSize_kf.kf_list))
				goto exit;	// No valid animation data

                OPTIONAL_FPRINTF(ofp, "\n\tSurface (material) animation (raw keyframe) data:\n");

		Export_IO_UpdateStatusDisplay("surface", surface_name, "Exporting animation data.");

		// Since we've checked that there is valid keyframe data above, we'll assume we are outputting valid animation
		animation_was_output = Nc_TRUE;

		// Send the raw diffuse RGB color keyframes (as 3 separate animation keyframe lists)
		if (r_diffuse_color_kf.kf_list || g_diffuse_color_kf.kf_list || b_diffuse_color_kf.kf_list) {
			// !! ADD CODE !! At this line you would create an animation controller in your
			//                export file scene graph to hold the diffuse color translation keyframe data (3 separate channels)
			NI_Exporter_Output3RelatedRawAnimationKeyFrameLists(ofp, Nc_FALSE,
				"diffuseColorR", "diffuseColorG", "diffuseColorB",
				&r_diffuse_color_kf, &g_diffuse_color_kf, &b_diffuse_color_kf);
		}

		// Send the raw specular RGB color keyframes (as 3 separate animation keyframe lists)
		if (r_specular_color_kf.kf_list || g_specular_color_kf.kf_list || b_specular_color_kf.kf_list) {
			// !! ADD CODE !! At this line you would create an animation controller in your
			//                export file scene graph to hold the specular color translation keyframe data (3 separate channels)
			NI_Exporter_Output3RelatedRawAnimationKeyFrameLists(ofp, Nc_FALSE,
				"specularColorR", "specularColorG", "specularColorB",
				&r_specular_color_kf, &g_specular_color_kf, &b_specular_color_kf);
		}

		// Send the raw ambient RGB color keyframes (as 3 separate animation keyframe lists)
		if (r_ambient_color_kf.kf_list || g_ambient_color_kf.kf_list || b_ambient_color_kf.kf_list) {
			// !! ADD CODE !! At this line you would create an animation controller in your
			//                export file scene graph to hold the ambient color translation keyframe data (3 separate channels)
			NI_Exporter_Output3RelatedRawAnimationKeyFrameLists(ofp, Nc_FALSE,
				"ambientColorR", "ambientColorG", "ambientColorB",
				&r_ambient_color_kf, &g_ambient_color_kf, &b_ambient_color_kf);
		}

		// Send the raw luminous RGB color keyframes (as 3 separate animation keyframe lists)
		if (r_luminous_color_kf.kf_list || g_luminous_color_kf.kf_list || b_luminous_color_kf.kf_list) {
			// !! ADD CODE !! At this line you would create an animation controller in your
			//                export file scene graph to hold the luminous color translation keyframe data (3 separate channels)
			NI_Exporter_Output3RelatedRawAnimationKeyFrameLists(ofp, Nc_FALSE,
				"luminousColorR", "luminousColorG", "luminousColorB",
				&r_luminous_color_kf, &g_luminous_color_kf, &b_luminous_color_kf);
		}

		// Send the raw filter RGB color keyframes (as 3 separate animation keyframe lists)
		if (r_filter_color_kf.kf_list || g_filter_color_kf.kf_list || b_filter_color_kf.kf_list) {
			// !! ADD CODE !! At this line you would create an animation controller in your
			//                export file scene graph to hold the filter color translation keyframe data (3 separate channels)
			NI_Exporter_Output3RelatedRawAnimationKeyFrameLists(ofp, Nc_FALSE,
				"filterColorR", "filterColorG", "filterColorB",
				&r_filter_color_kf, &g_filter_color_kf, &b_filter_color_kf);
		}

		// Send the ambient shading coefficient (0.0 to 1.0) keyframe data. This is a specialized
		// animation channel that isn't supported by too many converters. You can best leave it out.
		if (ambientShadingCoefficient_kf.kf_list) {
			// !! ADD CODE !! At this line you would create an animation controller in your
			//                export file scene graph to hold the ambient shading coefficient keyframe data
			exportFileFormatsAnimChanController = NI_Exporter_CreateFloatControllerBasedOnExplicitInterpolationMethod(ambientShadingCoefficient_kf.type);
			NI_Exporter_OutputRawNuGrafToolkitKeyFrameData(ofp, "ambientShadingCoefficient", &ambientShadingCoefficient_kf, exportFileFormatsAnimChanController, 0);
		}

		// Send the diffuse shading coefficient (0.0 to 1.0) keyframe data. This is a specialized
		// animation channel that isn't supported by too many converters. You can best leave it out.
		if (diffuseShadingCoefficient_kf.kf_list) {
			// !! ADD CODE !! At this line you would create an animation controller in your
			//                export file scene graph to hold the diffuse shading coefficient keyframe data
			exportFileFormatsAnimChanController = NI_Exporter_CreateFloatControllerBasedOnExplicitInterpolationMethod(diffuseShadingCoefficient_kf.type);
			NI_Exporter_OutputRawNuGrafToolkitKeyFrameData(ofp, "diffuseShadingCoefficient", &diffuseShadingCoefficient_kf, exportFileFormatsAnimChanController, 0);
		}

		// Send the specular shading coefficient (0.0 to 1.0) keyframe data. This is a specialized
		// animation channel that isn't supported by too many converters. You can best leave it out.
		if (specularShadingCoefficient_kf.kf_list) {
			// !! ADD CODE !! At this line you would create an animation controller in your
			//                export file scene graph to hold the specular shading coefficient keyframe data
			exportFileFormatsAnimChanController = NI_Exporter_CreateFloatControllerBasedOnExplicitInterpolationMethod(specularShadingCoefficient_kf.type);
			NI_Exporter_OutputRawNuGrafToolkitKeyFrameData(ofp, "specularShadingCoefficient", &specularShadingCoefficient_kf, exportFileFormatsAnimChanController, 0);
		}

		// Send the luminous shading coefficient (0.0 to 1.0) keyframe data. This is a specialized
		// animation channel that isn't supported by too many converters. You can best leave it out.
		if (luminousShadingCoefficient_kf.kf_list) {
			// !! ADD CODE !! At this line you would create an animation controller in your
			//                export file scene graph to hold the luminous shading coefficient keyframe data
			exportFileFormatsAnimChanController = NI_Exporter_CreateFloatControllerBasedOnExplicitInterpolationMethod(luminousShadingCoefficient_kf.type);
			NI_Exporter_OutputRawNuGrafToolkitKeyFrameData(ofp, "luminousShadingCoefficient", &luminousShadingCoefficient_kf, exportFileFormatsAnimChanController, 0);
		}

		// Send the reflect shading coefficient (0.0 to 1.0) keyframe data. This is a specialized
		// animation channel that isn't supported by too many converters. You can best leave it out.
		if (reflectShadingCoefficient_kf.kf_list) {
			// !! ADD CODE !! At this line you would create an animation controller in your
			//                export file scene graph to hold the reflect shading coefficient keyframe data
			exportFileFormatsAnimChanController = NI_Exporter_CreateFloatControllerBasedOnExplicitInterpolationMethod(reflectShadingCoefficient_kf.type);
			NI_Exporter_OutputRawNuGrafToolkitKeyFrameData(ofp, "reflectShadingCoefficient", &reflectShadingCoefficient_kf, exportFileFormatsAnimChanController, 0);
		}

		// Send the Phong highlight size (1 to 200, integer) keyframe data
		if (highlightPhongSize_kf.kf_list) {
			// !! ADD CODE !! At this line you would create an animation controller in your
			//                export file scene graph to hold the Phong highlight keyframe data
			exportFileFormatsAnimChanController = NI_Exporter_CreateFloatControllerBasedOnExplicitInterpolationMethod(highlightPhongSize_kf.type);
			NI_Exporter_OutputRawNuGrafToolkitKeyFrameData(ofp, "highlightPhongSize", &highlightPhongSize_kf, exportFileFormatsAnimChanController, 0);
		}

		// Send the face opacity keyframe data. If your animation system needs to animate transparency
		// (transparency = 1.0 - opacity) then specify the 'Nc_FLAGS__NEED_NORM_INVERTING' argument as the
		// last value to the NI_Exporter_OutputRawNuGrafToolkitKeyFrameData() routine below.
		if (faceOpacity_kf.kf_list) {
			// !! ADD CODE !! At this line you would create an animation controller in your
			//                export file scene graph to hold the face opacity keyframe data
			exportFileFormatsAnimChanController = NI_Exporter_CreateFloatControllerBasedOnExplicitInterpolationMethod(faceOpacity_kf.type);
			NI_Exporter_OutputRawNuGrafToolkitKeyFrameData(ofp, "faceOpacity", &faceOpacity_kf, exportFileFormatsAnimChanController, 0 /* Nc_FLAGS__NEED_NORM_INVERTING */ );
		}

		// And free up the keyframe lists we allocated in MAX_Setup_Resampled_Object_Animation_Keyframe_Lists()
		if (r_diffuse_color_kf.kf_list)		Ni_Free_Memory((char *) r_diffuse_color_kf.kf_list);
		if (g_diffuse_color_kf.kf_list)		Ni_Free_Memory((char *) g_diffuse_color_kf.kf_list);
		if (b_diffuse_color_kf.kf_list)		Ni_Free_Memory((char *) b_diffuse_color_kf.kf_list);

		if (r_specular_color_kf.kf_list)	Ni_Free_Memory((char *) r_specular_color_kf.kf_list);
		if (g_specular_color_kf.kf_list)	Ni_Free_Memory((char *) g_specular_color_kf.kf_list);
		if (b_specular_color_kf.kf_list)	Ni_Free_Memory((char *) b_specular_color_kf.kf_list);

		if (r_ambient_color_kf.kf_list)		Ni_Free_Memory((char *) r_ambient_color_kf.kf_list);
		if (g_ambient_color_kf.kf_list)		Ni_Free_Memory((char *) g_ambient_color_kf.kf_list);
		if (b_ambient_color_kf.kf_list)		Ni_Free_Memory((char *) b_ambient_color_kf.kf_list);

		if (r_luminous_color_kf.kf_list)	Ni_Free_Memory((char *) r_luminous_color_kf.kf_list);
		if (g_luminous_color_kf.kf_list)	Ni_Free_Memory((char *) g_luminous_color_kf.kf_list);
		if (b_luminous_color_kf.kf_list)	Ni_Free_Memory((char *) b_luminous_color_kf.kf_list);

		if (r_filter_color_kf.kf_list)		Ni_Free_Memory((char *) r_filter_color_kf.kf_list);
		if (g_filter_color_kf.kf_list)		Ni_Free_Memory((char *) g_filter_color_kf.kf_list);
		if (b_filter_color_kf.kf_list)		Ni_Free_Memory((char *) b_filter_color_kf.kf_list);

		if (ambientShadingCoefficient_kf.kf_list)	Ni_Free_Memory((char *) ambientShadingCoefficient_kf.kf_list);
		if (diffuseShadingCoefficient_kf.kf_list)	Ni_Free_Memory((char *) diffuseShadingCoefficient_kf.kf_list);
		if (specularShadingCoefficient_kf.kf_list)	Ni_Free_Memory((char *) specularShadingCoefficient_kf.kf_list);
		if (luminousShadingCoefficient_kf.kf_list)	Ni_Free_Memory((char *) luminousShadingCoefficient_kf.kf_list);
		if (reflectShadingCoefficient_kf.kf_list)	Ni_Free_Memory((char *) reflectShadingCoefficient_kf.kf_list);

		if (faceOpacity_kf.kf_list)		Ni_Free_Memory((char *) faceOpacity_kf.kf_list);
		if (highlightPhongSize_kf.kf_list)	Ni_Free_Memory((char *) highlightPhongSize_kf.kf_list);
	} else {
		//
		// Else, the following code reverts to creating temporary resampled keyframe lists internally
		// inside the toolkit which conform to the animation methods the exporter requires.
		//

		Export_IO_UpdateStatusDisplay("surface", surface_name, "Exporting animation data.");

        	/* Go and resample all the material related keyframe lists */
        	if (export_options->use_independent_controllers_for_tripled_anim_channels) {
        		// Resample the animation to independent R, G and B curves. Each curve could have
        		// different number of keys and different keyframe times
        		Ni_AnimResampleAndEnumerateKeyframeList_Start(
        			/* Return status: FALSE = no error, otherwise, see error list */
        			&Nv_Return_Status,
        			/* Pointer to the toolkit's local data is returned here for safe keeping. */
        			&Nv_FunctionInstanceLocalDataPtr,
        			/* Handle type and name of the Comm_Type to be enumerated (ie: Nt_CMAERA, "camera1") */
        			Nt_SURFACE, surface_name,
        			/* This is the order to extract any new Euler rotation angles (see ni3.h for list) */
        			/* This is only needed when Euler rotation angles are being asked for. */
        			/* Lightwave, for example, uses 'Nc_EulerRotationOrder_YXZ'. */
        			Nc_EulerRotationOrder_XYZ,
        			/* This is only needed when Euler rotation angles are being asked for. */
        			/* If TRUE, then the camera orientation matrix will have its Z value */
        			/* negated (RHCS -> LHCS) which is useful for Lightwave, but not Max. */
        			Nc_FALSE,

				// If the source Euler rotation curve flips back and forth on each
				// side of -180/180 degrees (like runs of -179, followed by 179.0)
				// then this routine will enable resampling on those rotation curves 
				// and then make sure the rotation values don't flip back and forth any more.
				Nt_FIXEULERROTATIONANGLEFLIPS, Nt_ENABLED, (export_options->flip_Euler_180flips_via_resampler ? Nt_ON : Nt_OFF), Nt_CMDSEP,

				// Specify the animation interpolation controller for the resampled output fcurves. 
				// (One of: Nt_ORIGINAL, Nt_KFCONSTANT, Nt_KFLINEAR, Nt_KFBEZIER, Nt_KFTCB)
				// If we set it to Nt_ORIGINAL then the resampler will use the same 
				// interplation type as the source animation curves that are being
				// fed to the resampler, unless it is not one of the allowable 
				// controller types set with the  Nt_ALLOWEDCONTROLLERTYPES option.
				Nt_TARGETCONTROLLERTYPE, NI_Exporter_Get_Users_Desired_Target_Animation_Controller_Type(), Nt_CMDSEP,

				// This specifies the keyframe controller types which are supported by the our exporter.
				// If the 'Nt_TARGETCONTROLLERTYPE' is set to Nt_ORIGINAL, then each
				// controller will be compared against this list of allowed controller types.
				// If the original curve's type matches, then that type will be used. If it
				// does not match, then the first item from this list will be used.
				Nt_ALLOWEDCONTROLLERTYPES, Nt_KFBEZIER, Nt_KFTCB, Nt_KFCONSTANT, Nt_KFLINEAR, Nt_CMDSEP,

				// When the 'Nt_TARGETCONTROLLERTYPE' fcurve interpolation method is set to Nt_KFBEZIER, the 
				// following list determines which Bezier tangent types are supported by this exporter 
				// as a way to create and return the new Bezier tangents. 
				Nt_ALLOWEDBEZIERTANGENTTYPES, Nt_CUSTOM, Nt_CONSTANT, Nt_LINEAR, Nt_SMOOTH, Nt_FAST, Nt_SLOW, Nt_CLAMPED, Nt_PLATEAU, Nt_AUTOFLAT, Nt_BROKENTANGENTS, Nt_CONSTANTVELOCITY, Nt_CMDSEP,

#if 00	// Valid, but not used in this example exporter
				// When resampling any rotation channels, this function will pre-multiply the
				//   given (quaternion) rotation into the evaluated rotation matrix.
				// The fundamental purpose of this feature is to allow lights to be "repointed"
				//   down an axis more appropriate for the target animation system.
				// However, it can also be used to tweak the resampled rotation values from
				//   any ordinary euler-quat, euler-euler, or quat-quat resampling as well.
				Nt_ROTATIONTWEAK, (Nd_Quat *) Nv_Rotation_Tweak_Quaternion, Nt_CMDSEP,
#endif
        			/* This is the list of channels to resample/inquire */
				Nt_ANIMCHAN,
					"diffuseColorR", "diffuseColorG", "diffuseColorB",
					"specularColorR", "specularColorG", "specularColorB",
					"ambientColorR", "ambientColorG", "ambientColorB",
					"luminousColorR", "luminousColorG", "luminousColorB",
					"filterColorR", "filterColorG", "filterColorB",
					"highlightPhongSize",
					"faceOpacity",
        				Nt_CMDEND);
        	} else {
        		// Else, let's utilize the special feature in the animation resampler routine to
			// merge 3 distinct RGB animation channels into a single RGB animation channel
			// for those export file formats which can handle a single RGB value at each key.
        		Ni_AnimResampleAndEnumerateKeyframeList_Start(
        			&Nv_Return_Status,
        			&Nv_FunctionInstanceLocalDataPtr,
        			Nt_SURFACE, surface_name,
        			Nc_EulerRotationOrder_XYZ,
        			Nc_FALSE,
				Nt_FIXEULERROTATIONANGLEFLIPS, Nt_ENABLED, (export_options->flip_Euler_180flips_via_resampler ? Nt_ON : Nt_OFF), Nt_CMDSEP,
				Nt_TARGETCONTROLLERTYPE, NI_Exporter_Get_Users_Desired_Target_Animation_Controller_Type(), Nt_CMDSEP,
				Nt_ALLOWEDCONTROLLERTYPES, Nt_KFBEZIER, Nt_KFTCB, Nt_KFCONSTANT, Nt_KFLINEAR, Nt_CMDSEP,
				Nt_ALLOWEDBEZIERTANGENTTYPES, Nt_CUSTOM, Nt_CONSTANT, Nt_LINEAR, Nt_SMOOTH, Nt_FAST, Nt_SLOW, Nt_CLAMPED, Nt_PLATEAU, Nt_AUTOFLAT, Nt_BROKENTANGENTS, Nt_CONSTANTVELOCITY, Nt_CMDSEP,
        			Nt_ANIMCHAN,
        				// This will cause each set of XYZ channels to be modified to that they
        				// share the same set of key times. They will still be queried by their
        				// single respective channel however.
					"RGBdiffuseColor", "RGBspecularColor", "RGBambientColor",
					"RGBluminousColor", "RGBfilterColor", "highlightPhongSize",
					"faceOpacity",
        				Nt_CMDEND);
        	}

        	if (Nv_Return_Status != Nc_RESAMPLEDKFLIST_STATUS_NO_NEW_KF_LISTS_CREATED && (Nv_Return_Status || Nv_Return_Status == Nc_RESAMPLEDKFLIST_STATUS_ALL_KEYS_AT_SAME_TIME))
        		/* Either an error was encontered, or no valid animation data */
        		/* exists, or all the keys exist at 1 point in time in which case */
        		/* we'll just output the ctm and no anim. info. for this instance. */
        		goto exit;

		if (!export_options->use_independent_controllers_for_tripled_anim_channels) {
        		// Export the animation by converting 3 channel keyframe lists to single channel (triple-value)
        		// keyframe lists inside the toolkit, then outputting them as a single controller.

			// !! ADD CODE !! At this line you would create animation controllers in your
			//                export file scene graph to hold the material RGB colors controllers
			//		  and the highlight + faceOpacity controllers

	                OPTIONAL_FPRINTF(ofp, "\n\tMaterial animation (resampled to single channel, 3-component) data:\n");

					"RGBdiffuseColor", "RGBspecularColor", "RGBambientColor",
					"RGBluminousColor", "RGBfilterColor", "highlightPhongSize",
					"faceOpacity",

        		// Send the RGB diffuse color channels from the NuGraf toolkit to the exporter as a single RGB animation controller channel
			exportFileFormatsAnimChanController = NI_Exporter_CreateADefaultRGBColorController();
        		NI_Exporter_QueryAndOutputResampledAnimationKeyframeDataFromOkinoToolkit(ofp, "RGBdiffuseColor", exportFileFormatsAnimChanController, &animation_was_output, 0);

        		// Send the RGB specular color channels from the NuGraf toolkit to the exporter as a single RGB animation controller channel
			exportFileFormatsAnimChanController = NI_Exporter_CreateADefaultRGBColorController();
        		NI_Exporter_QueryAndOutputResampledAnimationKeyframeDataFromOkinoToolkit(ofp, "RGBspecularColor", exportFileFormatsAnimChanController, &animation_was_output, 0);

        		// Send the RGB ambient color channels from the NuGraf toolkit to the exporter as a single RGB animation controller channel
			exportFileFormatsAnimChanController = NI_Exporter_CreateADefaultRGBColorController();
        		NI_Exporter_QueryAndOutputResampledAnimationKeyframeDataFromOkinoToolkit(ofp, "RGBambientColor", exportFileFormatsAnimChanController, &animation_was_output, 0);

        		// Send the RGB luminous color channels from the NuGraf toolkit to the exporter as a single RGB animation controller channel
			exportFileFormatsAnimChanController = NI_Exporter_CreateADefaultRGBColorController();
        		NI_Exporter_QueryAndOutputResampledAnimationKeyframeDataFromOkinoToolkit(ofp, "RGBluminousColor", exportFileFormatsAnimChanController, &animation_was_output, 0);

        		// Send the RGB filter color channels from the NuGraf toolkit to the exporter as a single RGB animation controller channel
			exportFileFormatsAnimChanController = NI_Exporter_CreateADefaultRGBColorController();
        		NI_Exporter_QueryAndOutputResampledAnimationKeyframeDataFromOkinoToolkit(ofp, "RGBfilterColor", exportFileFormatsAnimChanController, &animation_was_output, 0);
        	} else {
        		// Else, export the 3 channel keyframe lists as distinct controllers of their own.

			// !! ADD CODE !! At this line you would create animation controllers in your
			//                export file scene graph to hold the material RGB colors controllers
			//		  and the highlight + faceOpacity controllers.

	                OPTIONAL_FPRINTF(ofp, "\n\tMaterial animation (resampled, single channels) data:\n");

        		// Send the RGB diffuse color channels from the NuGraf toolkit to the exporter as as 3 separate animation channels
			NI_Exporter_Output3RelatedResampledAnimationKeyFrameLists(ofp, Nc_FALSE, "diffuseColorR", "diffuseColorG", "diffuseColorB", &animation_was_output);

        		// Send the RGB specular color channels from the NuGraf toolkit to the exporter as as 3 separate animation channels
			NI_Exporter_Output3RelatedResampledAnimationKeyFrameLists(ofp, Nc_FALSE, "specularColorR", "specularColorG", "specularColorB", &animation_was_output);

        		// Send the RGB ambient color channels from the NuGraf toolkit to the exporter as as 3 separate animation channels
			NI_Exporter_Output3RelatedResampledAnimationKeyFrameLists(ofp, Nc_FALSE, "ambientColorR", "ambientColorG", "ambientColorB", &animation_was_output);

        		// Send the RGB luminous color channels from the NuGraf toolkit to the exporter as as 3 separate animation channels
			NI_Exporter_Output3RelatedResampledAnimationKeyFrameLists(ofp, Nc_FALSE, "luminousColorR", "luminousColorG", "luminousColorB", &animation_was_output);

        		// Send the RGB filter color channels from the NuGraf toolkit to the exporter as as 3 separate animation channels
			NI_Exporter_Output3RelatedResampledAnimationKeyFrameLists(ofp, Nc_FALSE, "filterColorR", "filterColorG", "filterColorB", &animation_was_output);
        	}

		// Send the Phong highlight keyframe data as a single-value float controller
		exportFileFormatsAnimChanController = NI_Exporter_CreateADefaultFloatController();
       		NI_Exporter_QueryAndOutputResampledAnimationKeyframeDataFromOkinoToolkit(ofp, "highlightPhongSize", exportFileFormatsAnimChanController, &animation_was_output, 0 );

		// Send the face opacity keyframe data. If your animation system needs to animate transparency
		// (transparency = 1.0 - opacity) then specify the 'Nc_FLAGS__NEED_NORM_INVERTING' argument as the
		// last value to the NI_Exporter_OutputRawNuGrafToolkitKeyFrameData() routine below.
		exportFileFormatsAnimChanController = NI_Exporter_CreateADefaultFloatController();
       		NI_Exporter_QueryAndOutputResampledAnimationKeyframeDataFromOkinoToolkit(ofp, "faceOpacity", exportFileFormatsAnimChanController, &animation_was_output, 0 /* Nc_FLAGS__NEED_NORM_INVERTING */);
	}

	/* Free up the resampled keyframe lists */
exit:	if (Nv_FunctionInstanceLocalDataPtr) {
		Ni_AnimResampleAndEnumerateKeyframeList_End(&Nv_Return_Status, Nv_FunctionInstanceLocalDataPtr);
		Nv_FunctionInstanceLocalDataPtr = NULL;
	}
}

// ----------------->>>>  Resampled Keyframe Data Output Routines  <<<<-----------------

// This is the first of the 2 main routines in this file to output a resampled keyframe list. This version
// outputs the temporary resampled animation keyframe list currently stored deep inside the
// Okino NuGraf toolkit. This temporary list was created via the Ni_AnimResampleAndEnumerateKeyframeList_Start()
// function call and will be deleted via the Ni_AnimResampleAndEnumerateKeyframeList_End() function call (see above).

// The export converter developer will replace the dummy 'exportFileFormatsAnimChanController' pointer
// with a pointer to their own corresponding animation channel controller which is to receive the
// resampled keyframe data.

// This routine queries each and every key from the keyframe list defined by the 'animation_channel_type'
// argument, such as Nt_XTRANSLATE.

	static void
NI_Exporter_QueryAndOutputResampledAnimationKeyframeDataFromOkinoToolkit(FILE *ofp,
	Nd_Token animation_channel_type, void *exportFileFormatsAnimChanController,
	short *animation_was_output, short key_flags)
{
	Nd_Token	arguments[5];
	Nd_Int		num_returned_arguments;
	Nd_Vector	vec;
	void		*return_args[10];
	Nd_Int		Nv_Return_Status, num_arguments;
	short		first_time;

	// Inform the Okino toolkit which keyframe list we want to make inquiries from.
	// This assumes that the toolkit has been informed ahead of time via the
	// "Ni_AnimResampleAndEnumerateKeyframeList_Start()" function about which animation
	// channels needed to be (first) internally resampled to temporary animation channels.
	// All the code below will query these temporary internal animation channel lists
	// instead of the original raw, unsampled animation channels. The resampling and
	// processing by the toolkit ensures that we get the data in the format best utilized
	// by the export file format.
	arguments[0] = animation_channel_type;		// Tell the toolkit which channel we want to query
	num_arguments = 1;
	Ni_AnimResampleAndEnumerateKeyframeList_MakeRequest(
		&Nv_Return_Status,
		Nv_FunctionInstanceLocalDataPtr,
		Nc_RESAMPLEDKFLIST_REQUEST_SPECIFY_KF_LIST,
		num_arguments, (void **) arguments,
		&num_returned_arguments, return_args);	/* Data is returned here */

	/* Now inquire about each resampled key of the keyframe list */
	/* Note: no tangent or TCB info can be returned since it is not computed during resampling (the tangency info is embedded into the resampled curves) */
	if (!Nv_Return_Status) {
		OPTIONAL_FPRINTF(ofp, "\t\t'%s' animation channel resampled keyframes:\n", animation_channel_type);

		first_time = Nc_TRUE;
		while (Nc_TRUE) {
			Nd_Resampled_Keyframe_Info   kf_info;	/* The resampled keyframe info is returned here */

			/* Get data about the next keyframe */
			arguments[0] = (char *) &kf_info;     // Have the toolkit send back data to us via this struct.
			num_arguments = 1;

			/* Set up the "Nd_Resampled_Keyframe_Info" data structure by */
			/* initializing it to 0 and setting the structure version number */
			/* to 1. This is because this structure might change size in the future. */
			memset((char *) &kf_info, 0, sizeof(Nd_Resampled_Keyframe_Info));
			kf_info.Nv_Struct_Version_Num = 1;

			Ni_AnimResampleAndEnumerateKeyframeList_MakeRequest(
				&Nv_Return_Status,
				Nv_FunctionInstanceLocalDataPtr,
				Nc_RESAMPLEDKFLIST_REQUEST_GET_NEXT_KEYFRAME_DATA,
				num_arguments, (void **) arguments,	/* Not used for this call */
				&num_returned_arguments, return_args);	/* Data is returned here */

			if (Nv_Return_Status == Nc_RESAMPLEDKFLIST_STATUS_NO_MORE_KEYS_IN_LIST) {
				/* No more keys in this list */
				break;
			} else if (!Nv_Return_Status) {
				if (kf_info.Value.Nv_Num_Components == 1) {
					// This is a single channel single float value. Most keys in Okino, except for
					// rotations, are single floats.
					vec[0] = kf_info.Value.Nv_Vec[0];

					// Flip the Y channel for translations and rotates
					if (key_flags & Nc_FLAG__NEEDS_FLIPPING)
						vec[0] *= -1.0;

					// If this is the camera FOV or ROLL value then convert from degrees to radians
					if (key_flags & Nc_FLAG__CONVERT_TO_RADIANS)
						vec[0] = Nm_DEGREES2RADIAN(vec[0]);

					// Invert the value, which is needed if opacity animation values must be turned into transparency animation values
					if (key_flags & Nc_FLAGS__NEED_NORM_INVERTING)
						vec[0] = 1.0 - vec[0];

					OPTIONAL_FPRINTF(ofp, "\t\t\tResampled single component key, time = %d ticks, value = %g\n", kf_info.Nv_Time, vec[0]);
				} else if (kf_info.Value.Nv_Num_Components == 3) {
					// Vector key. We return resampled XYZ single controller values here.
					vec[0] = kf_info.Value.Nv_Vec[0];
					vec[1] = kf_info.Value.Nv_Vec[1];
					vec[2] = kf_info.Value.Nv_Vec[2];

					// Flip the Y channel for translations and rotates
					if (key_flags & Nc_FLAG__NEEDS_FLIPPING)
						vec[1] *= -1.0;

					if (key_flags & Nc_FLAG__CONVERT_TO_RADIANS) {
						vec[0] = Nm_DEGREES2RADIAN(vec[0]);
						vec[1] = Nm_DEGREES2RADIAN(vec[1]);
						vec[2] = Nm_DEGREES2RADIAN(vec[2]);
					}

					OPTIONAL_FPRINTF(ofp, "\t\t\tResampled 3-component vector key, time = %d ticks, value = (%g, %g, %g)\n", kf_info.Nv_Time, vec[0], vec[1], vec[2]);
				} else if ((key_flags & Nc_FLAG__THIS_IS_A_QUAT_CHANNEL) || kf_info.Value.Nv_Num_Components == 4) {
					// This is a quaternion rotation. Rotations are confined to 0-180 degrees.
					// Okino raw quaternions, can be queried as both relative and absolute.
					Nd_Quat rot_angle, quatrot, new_rot;

					if (first_time) {
						first_time = Nc_FALSE;
						Ni_QSet(&rot_angle, 0.0, 0.0, 0.0, 1.0);
					}

					quatrot = kf_info.Value.Quat.Nv_Relative;

					// Optionally flip the axes on the relative quaternion, using Angle/Axis form (safer)
					if (!((quatrot.w > 0.999998) &&
					    (fabs(quatrot.x) <= 1e-5) &&
					    (fabs(quatrot.y) <= 1e-5) &&
					    (fabs(quatrot.z) <= 1e-5))) {
						Nd_Vector	axis;
						double		angle;

						// Optionally swap the axes
						Ni_Q2AngleAxis(&quatrot, axis, &angle);

						// Uncomment this code if you need to swap from Y-Up to Z-Up
						// temp = axis[2];
						// axis[2] = axis[1];
						// axis[1] = temp;

						if (key_flags & Nc_FLAG__NEEDS_FLIPPING)
							axis[1] *= -1.0;

						Ni_AngleAxis2Q(axis, angle, &quatrot, Nc_FALSE);
					}

					// Convert the Okino relative quaternion into an absolute quaternion
					Ni_QMultiply(&quatrot, &rot_angle, &rot_angle);

					//
					// !! Note: you, the export converter developer, will most probably have to
					//    play around with the exported quaternions before they work correctly
					//    in your package. This most probably will entail negating the 'w' component
					//    or the x,y,z values.
					//

					new_rot.x = rot_angle.x;
					new_rot.y = rot_angle.y;
					new_rot.z = rot_angle.z;
					new_rot.w = -rot_angle.w;

					OPTIONAL_FPRINTF(ofp, "\t\t\tResampled quaternion rotation key, time = %d ticks, absolute value = (%g, %g, %g, %g), relative value = (%g, %g, %g, %g)\n",
						kf_info.Nv_Time,
						new_rot.x, new_rot.y, new_rot.z, new_rot.w,
						quatrot.x, quatrot.y, quatrot.z, quatrot.w);
				}

				*animation_was_output = Nc_TRUE;
			}
		}
	}
}

// ------------------->>>>  Raw (original) Keyframe Data Output Routines  <<<<-----------------

// This is the second of the 2 main routines in this file to output a raw keyframe list. This version
// outputs the exact raw animation key values that are stored inside the Okino toolkit, without any
// form of resampling applied to them. In general it is much safer to output resampled keyframes
// instead (see routine above) rather than raw keyframe data. This is because most export file formats
// cannot accomodate all of the animation controller and interpolation types supported by the Okino
// Animation System (however, 3DS MAX is one of the few programs that is a mirror twin to most of the
// capabilities of the Okino system).

// This routine queries each and every key from the keyframe list defined by the animation controller
// pointer 'cont_ptr'.

// The export converter developer will replace the dummy 'exportFileFormatsAnimChanController' pointer
// with a pointer to their own corresponding animation channel controller which is to receive the
// resampled keyframe data.

	static short
NI_Exporter_OutputRawNuGrafToolkitKeyFrameData(FILE *ofp, Nd_Token animation_channel_type,
	Export_IO_Anim_Controller_Info *cont_ptr, void *exportFileFormatsAnimChanController, short flags)
{
	Nd_AngleAxis 	ang;
	Nd_Vector 	vec;
	float		val;
	long 		i, num_keyframes = cont_ptr->num_keyframes;

	// No keyframes associated with this controller
	if (!cont_ptr->num_keyframes)
		return(Nc_FALSE);

	OPTIONAL_FPRINTF(ofp, "\t\t'%s' animation channel raw keyframes:\n", animation_channel_type);
	OPTIONAL_FPRINTF(ofp, "\t\t\tController type = '%s'\n", cont_ptr->type);

	// Set the pre- and post- extrapolation method on this controller
	NI_Exporter_SetControllerOutofRangeType(ofp, Nt_PREEXTRAPTYPE, cont_ptr->pre_extrap_type);
	NI_Exporter_SetControllerOutofRangeType(ofp, Nt_POSTEXTRAPTYPE, cont_ptr->post_extrap_type);

	// Loop over every keyframe in the list
	for (i=0; i < num_keyframes; ++i) {
		// Access the keys via our cover functions which queried all the keys from the core
		// toolkit to higher level, easier-to-access structures
		Export_IO_Anim_Keyframe_Info *kf = &cont_ptr->kf_list[i];

		if (kf->value.type == Nt_ANGLEAXISVALUE) {
			// This is an Angle/Axis rotation key. The angle allows multiple rotations (ie: 720 degrees)

			ang.axis[0] = -kf->value.angle_axis.axis[0];
			ang.axis[1] = -kf->value.angle_axis.axis[1];
			ang.axis[2] = -kf->value.angle_axis.axis[2];
			ang.angle   = kf->value.angle_axis.angle;

			if (flags & Nc_FLAG__NEEDS_FLIPPING)
				ang.axis[1] *= -1.0;

			OPTIONAL_FPRINTF(ofp, "\t\t\tRaw Angle/axis (relative) rotation key, time = %d ticks, axis = (%g, %g, %g), angle = %g\n",
				kf->time,
				ang.axis[0], ang.axis[1], ang.axis[2], ang.angle);
		} else if (flags & Nc_FLAG__THIS_IS_A_QUAT_CHANNEL || kf->value.type == Nt_QUATVALUE) {
			// This is a quaternion rotation. Rotations are confined to 0-180 degrees.
			// Okino raw quaternions, can be queried as both relative and absolute.

			Nd_Quat		quatrot, final_quatrot;

			// Pick up the absolute raw keyframe quaternion
			quatrot = kf->value.abs_quat_value;

			// Optionally flip the axes on the absolute quaternion, using Angle/Axis form (safer)
			if (!((quatrot.w > 0.999998) &&
			    (fabs(quatrot.x) <= 1e-5) &&
			    (fabs(quatrot.y) <= 1e-5) &&
			    (fabs(quatrot.z) <= 1e-5))) {
				Nd_Vector	axis;
				double		angle;

				// Optionally swap the axes
				Ni_Q2AngleAxis(&quatrot, axis, &angle);

				// Uncomment this code if you need to swap from Y-Up to Z-Up
				// temp = axis[2];
				// axis[2] = axis[1];
				// axis[1] = temp;

				if (flags & Nc_FLAG__NEEDS_FLIPPING)
					axis[1] *= -1.0;

				Ni_AngleAxis2Q(axis, angle, &quatrot, Nc_FALSE);
			}

			//
			// !! Note: you, the export converter developer, will most probably have to
			//    play around with the exported quaternions before they work correctly
			//    in your package. This most probably will entail negating the 'w' component
			//    or the x,y,z values.
			//
			final_quatrot.x = quatrot.x;
			final_quatrot.y = quatrot.y;
			final_quatrot.z = quatrot.z;
			final_quatrot.w = -quatrot.w;

			OPTIONAL_FPRINTF(ofp, "\t\t\tRaw quaternion rotation key, time = %d ticks, absolute value = (%g, %g, %g, %g)\n",
				kf->time,
				final_quatrot.x, final_quatrot.y, final_quatrot.z, final_quatrot.w);
		} else if (kf->value.type == Nt_VECTORVALUE) {
			// This is a vector value. At the moment no /raw/ Okino keys use vector values
			// so in general you won't get keys as XYZ unless you use the resampled keys
			// instead (and thus the Nt_VECTORVALUE is available via "NI_Exporter_QueryAndOutputResampledAnimationKeyframeDataFromOkinoToolkit()"

			vec[0] = kf->value.vector_value[0];
			vec[1] = kf->value.vector_value[1];
			vec[2] = kf->value.vector_value[2];

			if (flags & Nc_FLAG__NEEDS_FLIPPING)
				vec[1] *= -1.0;

			if (flags & Nc_FLAG__CONVERT_TO_RADIANS) {
				vec[0] = Nm_DEGREES2RADIAN(vec[0]);
				vec[1] = Nm_DEGREES2RADIAN(vec[1]);
				vec[2] = Nm_DEGREES2RADIAN(vec[2]);
			}

			OPTIONAL_FPRINTF(ofp, "\t\t\tRaw 3-component vector key, time = %d ticks, value = (%g, %g, %g)\n", kf->time, vec[0], vec[1], vec[2]);
		} else {
			// This is a single channel single float value. Most keys in Okino, except for
			// rotations, are single floats.
			val = kf->value.float_value;

			if (flags & Nc_FLAG__NEEDS_FLIPPING)
				val *= -1.0;

			// If this is the camera FOV or ROLL value then convert from degrees to radians
			if (flags & Nc_FLAG__CONVERT_TO_RADIANS)
				val = Nm_DEGREES2RADIAN(val);

			// Invert the value, which is needed if opacity animation values must be turned into transparency animation values
			if (flags & Nc_FLAGS__NEED_NORM_INVERTING)
				val = 1.0 - val;

			OPTIONAL_FPRINTF(ofp, "\t\t\tRaw single component key, time = %d ticks, value = %g\n", kf->time, val);
		}

		// ------------->>>  Bezier & TCB tangent key output


		// Now go and force the appropriate Besier or TCB tangent information onto the key which we just created in this loop
		if (cont_ptr->type == Nt_KFBEZIER) {
			//
			// At this point in the code you would feed this in/out tangent value onto your
			// Bezier key. If the key is a single value float then you only need to use
			// the 'x' value, otherwise would you would the (x,y,z,w) values for quaternion keys.
			//
			// NOTES:
			//	1) Okino tangents use the units of "dY/dTicks". To convert to "dY/dSeconds"
			//	   multiply them by 'Nc_ANIM_TICKS_PER_SECOND'.
			//	2) The tangent either uses the 'x' component or the (x,y,z,w), the latter for quats only.
			//	3) If the key represents the X, Y or Z single-float value of a Euler
			//	   rotation controller then the 'x' tangent value must be specified as
			//	   dAngle / dTicks, where 'dAngle' is in radians.
			//	4) These in/out tangent values will only be used by the controller if the
			//         'tangent_in_type' or 'tangent_out_type' is set to Nt_CUSTOM.

			OPTIONAL_FPRINTF(ofp, "\t\t\t\tBezier in-tangent value = (%g, %g, %g, %g), out-tangent value = (%g, %g, %g, %g)\n",
				// In-tangent value. This is the tangent pointing to the left of the key
				kf->tangents.tangent_in_value.x,
				kf->tangents.tangent_in_value.y,
				kf->tangents.tangent_in_value.z,
				kf->tangents.tangent_in_value.w,
				// Out-tangent value. This is the tangent pointing to the right of the key
				kf->tangents.tangent_out_value.x,
				kf->tangents.tangent_out_value.y,
				kf->tangents.tangent_out_value.z,
				kf->tangents.tangent_out_value.w);

			OPTIONAL_FPRINTF(ofp, "\t\t\t\tBezier in-tangent type = %s, out-tangent type = %s\n",
				kf->tangents.tangent_in_type, kf->tangents.tangent_out_type);

			//
			// The following in/out tangent types tell the internal controller whether
			// the tangents are to be automatically computed, or whether developer-provided
			// tangent values are to be used.
			//

			// Set the in tangent type. This defines how the animation controller should
			// compute the in-coming tangent at the key.
			if (kf->tangents.tangent_in_type == Nt_CUSTOM) {
				// Set the key's Bezier "in tangent" type to 'Custom'
				// In this case the in-tangent is explicitly defined by the "kf->tangents.tangent_in_value"
			} else if (kf->tangents.tangent_in_type == Nt_CONSTANT) {
				// Set the key's Bezier "in tangent" type to 'Constant'.
				// The tangent will always be flat.
			} else if (kf->tangents.tangent_in_type == Nt_LINEAR) {
				// Set the key's Bezier "in tangent" type to 'Linear'
				// The tangent will point directly to the next key.
				// For true linear interpolation the previous and/or next key must use linear as well
			} else if (kf->tangents.tangent_in_type == Nt_SMOOTH) {
				// Set the key's Bezier "in tangent" type to 'Smooth'
                                // Hermite interpolation: The tangent will be computed as a smooth function of the previous and next tangents.
			} else if (kf->tangents.tangent_in_type == Nt_FAST) {
				// Set the key's Bezier "in tangent" type to 'Fast'.
				// The interpolated rate of change speeds up near the key.
			} else if (kf->tangents.tangent_in_type == Nt_SLOW) {
				// Set the key's Bezier "in tangent" type to 'Slow'
				// The interpolated rate of change slows down near the key (the slope is 0).
			} else if (kf->tangents.tangent_in_type == Nt_CLAMPED) {
				// Set the key's Bezier "in tangent" type to 'Clamped'
				// The slope will effectively be 0 if the current and next values are identical (see Maya).
			} else if (kf->tangents.tangent_in_type == Nt_PLATEAU) {
				// Set the key's Bezier "in tangent" type to 'Plateau'. See Softimage-3D.
			} else if (kf->tangents.tangent_in_type == Nt_AUTOFLAT) {
				// Set the key's Bezier "in tangent" type to 'Auto Flat'.
				// This is to be compatible with 3DS MAX v5 or newer. The tangents on the
				// min/max extrema of the interpolated curve will always be made to be flat.
			}

			// Set the out tangent type. This defines how the animation controller should
			// compute the in-coming tangent at the key.
			if (kf->tangents.tangent_out_type == Nt_CUSTOM) {
				// Set the key's Bezier "out tangent" type to Custom
				// In this case the out-tangent is explicitly defined by the "kf->tangents.tangent_out_value"
			} else if (kf->tangents.tangent_out_type == Nt_CONSTANT) {
				// Set the key's Bezier "out tangent" type to 'Constant'
				// The tangent will always be flat.
			} else if (kf->tangents.tangent_out_type == Nt_LINEAR) {
				// Set the key's Bezier "out tangent" type to 'Linear'
				// The tangent will point directly to the next key.
				// For true linear interpolation the previous and/or next key must use linear as well
			} else if (kf->tangents.tangent_out_type == Nt_SMOOTH) {
				// Set the key's Bezier "out tangent" type to 'Smooth'
                                // Hermite interpolation: The tangent will be computed as a smooth function of the previous and next tangents.
			} else if (kf->tangents.tangent_out_type == Nt_FAST) {
				// Set the key's Bezier "out tangent" type to 'Fast'
				// The interpolated rate of change speeds up near the key.
			} else if (kf->tangents.tangent_out_type == Nt_SLOW) {
				// Set the key's Bezier "out tangent" type to 'Slow'
				// The interpolated rate of change slows down near the key (the slope is 0).
			} else if (kf->tangents.tangent_out_type == Nt_CLAMPED) {
				// Set the key's Bezier "out tangent" type to 'Clamped'
				// The slope will effectively be 0 if the current and next values are identical (see Maya).
			} else if (kf->tangents.tangent_out_type == Nt_PLATEAU) {
				// Set the key's Bezier "out tangent" type to 'Plateau'. See Softimage-3D.
			} else if (kf->tangents.tangent_out_type == Nt_AUTOFLAT) {
				// Set the key's Bezier "out tangent" type to 'Auto Flat'.
				// This is to be compatible with 3DS MAX v5 or newer. The tangents on the
				// min/max extrema of the interpolated curve will always be made to be flat.
			}
		} else if (cont_ptr->type == Nt_KFTCB) {
			//
			// At this point in the code you would feed these TCB parameters values onto your
			// TCB key.
			//
			OPTIONAL_FPRINTF(ofp, "\t\t\t\tTCB key tangent values: Tension = %g, continuity = %g, bias = %g, ease-from = %g, ease-to = %g\n",
				kf->tcb.tension,	// Range from -1.0 to 1.0
				kf->tcb.continuity,	// Range from -1.0 to 1.0
				kf->tcb.bias,		// Range from -1.0 to 1.0
				kf->tcb.easefrom,	// Range from 0.0 to 1.0
				kf->tcb.easeto);	// Range from 0.0 to 1.0
		}
	}

	return(Nc_TRUE);
}

// ---------------->>>> Animation controller creation for *RAW* keyframe output mode  <<<<-------------------

// The following routines are utility "virtual" functions which you, the export converter developer,
// needs to write. They create animation controllers needed to dump specific keyframe data to the
// export file format. The routines are differentiated by the animation controller type needed, and
// each routine creates a controller based on the necessary interpolation method (linear, Bezier or TCB).


// Create a quaternion animation controller in the export file format's scene graph.
// This is used when we are exporting *raw keyframe data* from the NuGraf Toolkit.

	static void *
NI_Exporter_CreateQuatRotationControllerBasedOnExplicitInterpolationMethod(Nd_Token interpolation_method)
{
	if (interpolation_method == Nt_KFCONSTANT)
		return (void *) NULL;	// Create linear quaternion controller here
	else if (interpolation_method == Nt_KFLINEAR)
		return (void *) NULL;	// Create keyframe quaternion controller here
	else if (interpolation_method == Nt_KFBEZIER)
		return (void *) NULL;	// Create Bezier keyframe quaternion controller here
	else if (interpolation_method == Nt_KFTCB)
		return (void *) NULL;	// Create TCB keyframe quaternion controller here

	return(NULL);	// This statement should never be reached
}

// Create a single channel float animation controller in the export file format's scene graph.
// This is used when we are exporting *raw keyframe data* from the NuGraf Toolkit.

	static void *
NI_Exporter_CreateFloatControllerBasedOnExplicitInterpolationMethod(Nd_Token interpolation_method)
{
	if (interpolation_method == Nt_KFCONSTANT || interpolation_method == Nt_KFLINEAR)
		return (void *) NULL;	// Create linear keyframe controller here
	else if (interpolation_method == Nt_KFTCB)
		return (void *) NULL;	// Create TCB keyframe controller here
	else if (interpolation_method == Nt_KFBEZIER)
		return (void *) NULL;	// Create Bezier keyframe controller here

	return(NULL); 	// This statement should never be reached
}


// ------------>>>> Animation controller creation for *Resampled* keyframe output mode  <<<<-------------

// The following routines are utility "virtual" functions which you, the export converter developer,
// needs to write. They create animation controllers needed to dump specific keyframe data to the
// export file format. The routines are differentiated by the animation controller type needed, and
// each routine creates a controller based on the necessary interpolation method (linear, Bezier or TCB).


// Create a single 3-component translation/position animation controller in the export file format's scene graph. Some programs,
// such as Lightwave 5.6 and 3DSMAX, allow for a single controller with XYZ values at each key. We create
// this special controller in cases where we can export a XYZ position point to a single key of one controller.
// If your file format cannot accommodate XYZ (3 component) keys then simply don't use the tripled animation
// output mode (the Okino animation system is naturally geared towards single-channel, single-value output
// and not 3-components-per-key mode, which is a special sub-mode of resampled keyframe output).

// This is used when we are exporting *resampled* keyframe data from the NuGraf Toolkit.

	static void *
NI_Exporter_CreateADefaultPositionController()
{
	switch (export_options->interpolation_type_to_use_during_export) {
		case Nc_NRS_ANIM_INTERPOLATION_TYPE_LINEAR:
			return (void *) NULL;	// Create linear keyframe controller here
		case Nc_NRS_ANIM_INTERPOLATION_TYPE_TCB:
			return (void *) NULL;	// Create TCB keyframe controller here
		case Nc_NRS_ANIM_INTERPOLATION_TYPE_BEZIER:
			return (void *) NULL;	// Create Bezier keyframe controller here
	}

	return(NULL);	// This statement should never be reached
}

// Create a single 3-component scale animation controller in the export file format's scene graph. Some programs,
// such as Lightwave 5.6 and 3DSMAX, allow for a single controller with XYZ values at each key. We create
// this special controller in cases where we can export a XYZ scale value to a single key of one controller.
// This is used when we are exporting *resampled* keyframe data from the NuGraf Toolkit.

	static void *
NI_Exporter_CreateADefaultScaleRotationController()
{
	switch (export_options->interpolation_type_to_use_during_export) {
		case Nc_NRS_ANIM_INTERPOLATION_TYPE_LINEAR:
			return (void *) NULL;	// Create linear keyframe controller here
		case Nc_NRS_ANIM_INTERPOLATION_TYPE_TCB:
			return (void *) NULL;	// Create TCB keyframe controller here
		case Nc_NRS_ANIM_INTERPOLATION_TYPE_BEZIER:
			return (void *) NULL;	// Create Bezier keyframe controller here
	}
	return(NULL);	// This statement should never be reached
}

// Create a quaternion rotation animation controller in the export file format's scene graph.
// This is used when we are exporting *resampled* keyframe data from the NuGraf Toolkit.

	static void *
NI_Exporter_CreateADefaultQuatRotationController()
{
	switch (export_options->interpolation_type_to_use_during_export) {
		case Nc_NRS_ANIM_INTERPOLATION_TYPE_LINEAR:
			return (void *) NULL;	// Create linear keyframe controller here
		case Nc_NRS_ANIM_INTERPOLATION_TYPE_TCB:
			return (void *) NULL;	// Create TCB keyframe controller here
		case Nc_NRS_ANIM_INTERPOLATION_TYPE_BEZIER:
			return (void *) NULL;	// Create Bezier keyframe controller here
	}

	return(NULL);	// This statement should never be reached
}

// Create a Euler rotation animation controller in the export file format's scene graph.
// This should be a single channel, 3 component (XYZ) Euler controller. In some cases, like
// in 3DS MAX, there is no such thing as a single channel Euler controller but instead the
// Euler controller uses 3 sub-channels of X, Y and Z. In that case modify this file (anim.cpp)
// to output Eulers as 3 separate channels and not one channel of XYZ keys.
// This is used when we are exporting *resampled* keyframe data from the NuGraf Toolkit.

	static void *
NI_Exporter_CreateADefaulEulerRotationController()
{
	switch (export_options->interpolation_type_to_use_during_export) {
		case Nc_NRS_ANIM_INTERPOLATION_TYPE_LINEAR:
			return (void *) NULL;	// Create linear keyframe controller here
		case Nc_NRS_ANIM_INTERPOLATION_TYPE_TCB:
			return (void *) NULL;	// Create TCB keyframe controller here
		case Nc_NRS_ANIM_INTERPOLATION_TYPE_BEZIER:
			return (void *) NULL;	// Create Bezier keyframe controller here
	}

	return(NULL);	// This statement should never be reached
}

// Create a float rotation animation controller in the export file format's scene graph.
// This is used when we are exporting *resampled* keyframe data from the NuGraf Toolkit.

	static void *
NI_Exporter_CreateADefaultFloatController()
{
	switch (export_options->interpolation_type_to_use_during_export) {
		case Nc_NRS_ANIM_INTERPOLATION_TYPE_LINEAR:
			return (void *) NULL;	// Create linear keyframe controller here
		case Nc_NRS_ANIM_INTERPOLATION_TYPE_TCB:
			return (void *) NULL;	// Create TCB keyframe controller here
		case Nc_NRS_ANIM_INTERPOLATION_TYPE_BEZIER:
			return (void *) NULL;	// Create Bezier keyframe controller here
	}

	return(NULL);	// This statement should never be reached
}

// Create a single 3-component RGB color animation controller in the export file format's scene graph. Some programs,
// such as Lightwave 5.6 and 3DSMAX, allow for a single controller with RGB values at each key. We create
// this special controller in cases where we can export a RGB color value to a single key of one controller.
// If your file format cannot accommodate RGB (3 component) keys then simply don't use the tripled animation
// output mode (the Okino animation system is naturally geared towards single-channel, single-value output
// and not 3-components-per-key mode, which is a special sub-mode of resampled keyframe output).

// This is used when we are exporting *resampled* keyframe data from the NuGraf Toolkit.

	static void *
NI_Exporter_CreateADefaultRGBColorController()
{
	switch (export_options->interpolation_type_to_use_during_export) {
		case Nc_NRS_ANIM_INTERPOLATION_TYPE_LINEAR:
			return (void *) NULL;	// Create linear keyframe controller here
		case Nc_NRS_ANIM_INTERPOLATION_TYPE_TCB:
			return (void *) NULL;	// Create TCB keyframe controller here
		case Nc_NRS_ANIM_INTERPOLATION_TYPE_BEZIER:
			return (void *) NULL;	// Create Bezier keyframe controller here
	}

	return(NULL);	// This statement should never be reached
}

// -------------->>>>  Animation Export Utility Functions  <<<<-----------------

// This is a helper cover function to output 3 related (raw) but separate keyframe lists
// such as the X, Y and Z camera look-from location or the R, G and B diffuse material color.
// The interpolation type for each newly output animation channel is taken from the
// interpolation of the animation controllers provided to this function.

	static void
NI_Exporter_Output3RelatedRawAnimationKeyFrameLists(
	FILE				*ofp,
	short				animation_channels_are_Eulers,	// Set to Nc_TRUE if the 3 channels represent Euler rotation triples
	Nd_Token			kflist1_anim_channel_type,
	Nd_Token			kflist2_anim_channel_type,
	Nd_Token			kflist3_anim_channel_type,
	Export_IO_Anim_Controller_Info	*kf_list1,
	Export_IO_Anim_Controller_Info	*kf_list2,
	Export_IO_Anim_Controller_Info	*kf_list3)
{
	void	*exportFileFormatsAnimChanController;	// Replace this with your own animation controller pointer type

//
// PROGRAMMER TO DO: If these are Euler channels then somehow you have to call "NI_Exporter_ChangeEulerRotationControllerOrder()"
//	             to set the master Euler controller's rotation order.
//
//	if (animation_channels_are_Eulers)
//		// Change the Euler rotation order of the Euler controller to match that of NuGraf
//		NI_Exporter_ChangeEulerRotationControllerOrder(ofp, exportFileFormatsAnimChanController, curr_euler_rotation_controller_order);

	if (kf_list1) {
		// Create an animation controller based on the type of this NuGraf animation controller
		exportFileFormatsAnimChanController = NI_Exporter_CreateFloatControllerBasedOnExplicitInterpolationMethod(kf_list1->type);
		// Send the raw NuGraf animation data to MAX
		NI_Exporter_OutputRawNuGrafToolkitKeyFrameData(ofp, kflist1_anim_channel_type, kf_list1, exportFileFormatsAnimChanController, 0);
	}
	if (kf_list2) {
		// Create an animation controller based on the type of this NuGraf animation controller
		exportFileFormatsAnimChanController = NI_Exporter_CreateFloatControllerBasedOnExplicitInterpolationMethod(kf_list2->type);
		NI_Exporter_OutputRawNuGrafToolkitKeyFrameData(ofp, kflist2_anim_channel_type, kf_list2, exportFileFormatsAnimChanController, 0);
	}
	if (kf_list3) {
		// Create an animation controller based on the type of this NuGraf animation controller
		exportFileFormatsAnimChanController = NI_Exporter_CreateFloatControllerBasedOnExplicitInterpolationMethod(kf_list3->type);
		NI_Exporter_OutputRawNuGrafToolkitKeyFrameData(ofp, kflist3_anim_channel_type, kf_list3, exportFileFormatsAnimChanController, 0);
	}
}


// This is a helper cover function to output 3 related (resampled) but separate keyframe lists
// such as the X, Y and Z camera look-from location or the R, G and B diffuse material color.

	static void
NI_Exporter_Output3RelatedResampledAnimationKeyFrameLists(
	FILE		*ofp,
	short		animation_channels_are_Eulers,	// Set to Nc_TRUE if the 3 channels represent Euler rotation triples
	Nd_Token	kflist1_anim_channel_type,
	Nd_Token	kflist2_anim_channel_type,
	Nd_Token	kflist3_anim_channel_type,
	short		*animation_was_output)
{
	void	*exportFileFormatsAnimChanController;	// Replace this with your own animation controller pointer type

//
// PROGRAMMER TO DO: If these are Euler channels then somehow you have to call "NI_Exporter_ChangeEulerRotationControllerOrder()"
//	             to set the master Euler controller's rotation order.
//
//	if (animation_channels_are_Eulers)
//		// Change the Euler rotation order of the Euler controller to match that of NuGraf
//		NI_Exporter_ChangeEulerRotationControllerOrder(

	exportFileFormatsAnimChanController = NI_Exporter_CreateADefaultFloatController();
	NI_Exporter_QueryAndOutputResampledAnimationKeyframeDataFromOkinoToolkit(ofp, kflist1_anim_channel_type, exportFileFormatsAnimChanController, animation_was_output, 0);

	exportFileFormatsAnimChanController = NI_Exporter_CreateADefaultFloatController();
	NI_Exporter_QueryAndOutputResampledAnimationKeyframeDataFromOkinoToolkit(ofp, kflist2_anim_channel_type, exportFileFormatsAnimChanController, animation_was_output, 0);

	exportFileFormatsAnimChanController = NI_Exporter_CreateADefaultFloatController();
	NI_Exporter_QueryAndOutputResampledAnimationKeyframeDataFromOkinoToolkit(ofp, kflist3_anim_channel_type, exportFileFormatsAnimChanController, animation_was_output, 0);
}

// This routine	is used to set an export file format's Euler rotation controller's
// order of XYZ channel evaluation. For example, if the Okino animation controller
// is set to 'Nc_EulerRotationOrder_XZY' then the destination program should evaluate
// and concatenate the Euler rotation controllers in the order X, then Z and then Y.

// The export converter developer will replace the dummy 'exportFileFormatsAnimChanController' pointer
// with a pointer to their own corresponding animation channel controller which is to receive the
// resampled keyframe data.

// See the explanation about "Gimbal lock can occur when..." which shows the importance
// of being able to set the Euler rotation order in the destination program.

	static void
NI_Exporter_ChangeEulerRotationControllerOrder( FILE *ofp, void *exportFileFormatsAnimChanController,
	Nd_Int Nv_CurrentOkinoEulerRotationOrder )
{
	OPTIONAL_FPRINTF(ofp, "\t\t\tEuler rotation order = ");

	switch( Nv_CurrentOkinoEulerRotationOrder ) {
		case Nc_EulerRotationOrder_XYZ:
			OPTIONAL_FPRINTF(ofp, "xyz\n");
			break;
		case Nc_EulerRotationOrder_XZY:
			OPTIONAL_FPRINTF(ofp, "xzy\n");
			break;
		case Nc_EulerRotationOrder_YZX:
			OPTIONAL_FPRINTF(ofp, "yzx\n");
			break;
		case Nc_EulerRotationOrder_YXZ:
			OPTIONAL_FPRINTF(ofp, "yxz\n");
			break;
		case Nc_EulerRotationOrder_ZXY:
			OPTIONAL_FPRINTF(ofp, "zxy\n");
			break;
		case Nc_EulerRotationOrder_ZYX:
			OPTIONAL_FPRINTF(ofp, "zyx\n");
			break;

		// These are really not used by any Okino animation controller
		case Nc_EulerRotationOrder_ZYZ:
			OPTIONAL_FPRINTF(ofp, "zyz\n");
			break;
		case Nc_EulerRotationOrder_YXY:
			OPTIONAL_FPRINTF(ofp, "yxy\n");
			break;
		case Nc_EulerRotationOrder_XZX:
			OPTIONAL_FPRINTF(ofp, "xzx\n");
			break;
		case Nc_EulerRotationOrder_XYX:
			OPTIONAL_FPRINTF(ofp, "xyx\n");
			break;
		case Nc_EulerRotationOrder_YZY:
			OPTIONAL_FPRINTF(ofp, "yzy\n");
			break;
		case Nc_EulerRotationOrder_ZXZ:
			OPTIONAL_FPRINTF(ofp, "zxz\n");
			break;
	}
}

// Set an animation controller's pre- and post- extrpolation types.
// Mode is either 'Nt_PREEXTRAPTYPE' or 'Nt_POSTEXTRAPTYPE'

	static void
NI_Exporter_SetControllerOutofRangeType(FILE *ofp, Nd_Token mode, Nd_Token interpolation_method)
{

	OPTIONAL_FPRINTF(ofp, "\t\t\t%s type = '%s'\n", mode, interpolation_method);

	if (interpolation_method == Nt_CONSTANT)
		// Value remains constant before/after keyframe list extents
		;
	else if (interpolation_method == Nt_LOOP)
		// Loops from the end back to the front. Performs a
		// smooth transition from the last value to the first value.
		;
	else if (interpolation_method == Nt_CYCLE)
		// Loops from the end back to the front. If values don't match then an abrupt change will occur.
		;
	else if (interpolation_method == Nt_PINGPONG)
		// Alternate back and forth
		;
	else if (interpolation_method == Nt_LINEAR)
		// Continues the function curve along a linear tangent line.
		;
	else if (interpolation_method == Nt_GRADIENT)
		// (Relative repeat). Repeats the animation, but offset relative to the last value (escalates from loop to loop)
		;
	else if (interpolation_method == Nt_RESET)
		// Reset controller output to 0 (for Lightwave end behaviour)
		;
}

// This returns TRUE if the instance is associated with a quaternion controller instead of a Euler controller

	static short
NI_Exporter_DoesGeometryInstanceHaveAQuaternionController(char *instance_name)
{
	void 		*x_rotate_chan_ptr, *y_rotate_chan_ptr, *z_rotate_chan_ptr;
	void		*quaternion_rotate_chan_ptr;

	// Let's see whether this instance uses Euler or Quaternion rotation
	x_rotate_chan_ptr = y_rotate_chan_ptr = z_rotate_chan_ptr = quaternion_rotate_chan_ptr = NULL;
	// Query the Euler and Quaternion animation channels from the instance
	Ni_Inquire_Instance(instance_name,
		Nt_ANIMCHAN, 
			/* Euler X/Y/Z explicit rotation channels */
			Nt_GETCHANNELID, Nt_XROTATE, (void *) &x_rotate_chan_ptr,
			Nt_GETCHANNELID, Nt_YROTATE, (void *) &y_rotate_chan_ptr,
			Nt_GETCHANNELID, Nt_ZROTATE, (void *) &z_rotate_chan_ptr,
			/* Quaternion rotation channel */
			Nt_GETCHANNELID, Nt_QUATROTATE, (void *) &quaternion_rotate_chan_ptr,
			Nt_CMDSEP,
		Nt_CMDEND);

	return (quaternion_rotate_chan_ptr != NULL);
}

// This returns TRUE if the light uses a quaternion controller for its (directional light) direction
// instead of a Euler controller

	static short
NI_Exporter_DoesLightUseAQuaternionControllerForDirection(char *light_name)
{
	void 		*x_rotate_chan_ptr, *y_rotate_chan_ptr, *z_rotate_chan_ptr;
	void		*quaternion_rotate_chan_ptr;

	// Let's see whether this instance uses Euler or Quaternion rotation
	x_rotate_chan_ptr = y_rotate_chan_ptr = z_rotate_chan_ptr = quaternion_rotate_chan_ptr = NULL;
	// Query the Euler and Quaternion animation channels from the light
	Ni_Inquire_Light(light_name,
		Nt_ANIMCHAN, 
			/* Euler X/Y/Z explicit rotation channels */
			Nt_GETCHANNELID, Nt_XROTATE, (void *) &x_rotate_chan_ptr,
			Nt_GETCHANNELID, Nt_YROTATE, (void *) &y_rotate_chan_ptr,
			Nt_GETCHANNELID, Nt_ZROTATE, (void *) &z_rotate_chan_ptr,
			/* Quaternion rotation channel */
			Nt_GETCHANNELID, Nt_QUATROTATE, (void *) &quaternion_rotate_chan_ptr,
			Nt_CMDSEP,
		Nt_CMDEND);

	return (quaternion_rotate_chan_ptr != NULL);
}

