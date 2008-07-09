/*****************************************************************************

	This is the common source code that can be used to create
	Okino COM client applications. See the "Okino Plug-Ins SDK",
	 COM Interface section, for explanations about this file.

         Use this source code as a basis to form your own COM client
		 to the PolyTrans/NuGraf Software. It's Simple!

  Copyright (c) 1988, 2002 Okino Computer Graphics, Inc. All Rights Reserved.

This file is proprietary source code of Okino Computer Graphics, Inc. and it 
is not to be disclosed to third parties, published, adopted, distributed,
copied or duplicated in any form, in whole or in part without the prior 
authorization of Okino Computer Graphics, Inc. This file may, however, be
modified and recompiled solely for use with the NuGraf Rendering Toolkit; all
changes and modifications made to this file remain the property and copyright
of Okino Computer Graphics, Inc. 

		U.S. GOVERNMENT RESTRICTED RIGHTS NOTICE

This Source Code and Technical Material are provided with RESTRICTED RIGHTS. 
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

	Last change:  RCL  12 Nov 2001    5:18 am
*****************************************************************************/

/* ----------------------->>>>  Includes  <<<<--------------------------- */

// The main include file for this .cpp file which you are reading
#include 	"COMHelper/shared_okino_com_src.h"
#include 	"COMHelper/shared_okino_com_src_resources.h"

// ATL related functions
#include 	<atlbase.h>
#include 	<atlcom.h>
#include 	<atlimpl.cpp>

#if defined(_DEBUG) && defined (_AFXDLL)
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/* ------------------->>>>  Global Variables  <<<<--------------------- */

// Our global interface pointers
INuGrafIO	*pNuGrafIO;	// Generic functions
IConvertIO	*pConvertIO;	// Import/export converter functions
IRenderIO	*pRenderIO;	// Rendering related functions

char last_geometry_filename_selected[300];	// Used for end-user file selection
short		updating_edit_control;	// We need this to prevent the edit controls from being updated as we modify their values
short		abort_flag;		// A flag used to signify if the user pressed the CANCEL button on the import or export status dialog box
HINSTANCE 	gInstance;		// Global instance for this module

// This is the current main dialog box window being shown. Anyone can use
// this to connect to the dialog box as a child window
HWND	parent_hwnd = NULL;

// This is a handle to one of the listboxes on the dialog box panels which
// serve to be a dumping ground from error and status messages passed to us
// via an event sink from the COM server. Whenever you change a panel and
// wish to have messages shown on that panel, set this HWND to a listbox 
// shown on that panel. 
HWND	curr_Error_Msg_Listbox_hwnd = NULL;

// This is a handle to one of the text lines on the dialog box panels which
// serve to receive the information status messages passes to us 
// via an event sink from the COM server. Whenever you change a panel and
// wish to have status messages shown on that panel, set this HWND to a text 
// line shown on that panel. 
HWND	curr_Status_Msg_Text_Line_hwnd = NULL;

/* ----------------->>>>  Object and Message Maps  <<<<-------------------- */

// This is the main instance of the COM module, as defined by ATL
CComModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
END_OBJECT_MAP()

/* ---------------->>>>  ATL Connection Point Class  <<<<------------------- */

// Function definitions for the event sink override callbacks
typedef int	(*def_OnGeometryImportProgress_OverrideCallback)(char *mode, char *info_text, long curr_file_offset, long file_length, long total_objects, long total_polygons );
typedef int 	(*def_OnGeometryExportProgress_OverrideCallback)(char *mode, long Nv_Scanline_Num, long Nv_Num_Scanlines);
typedef int 	(*def_OnErrorMessage_OverrideCallback)(char *Nv_Error_Level, char *Nv_Error_Label, char *Nv_Formatted_Message );
typedef int  	(*def_OnMessageWindowTextOutput_OverrideCallback)( char *Nv_Formatted_Message, int Nv_String_Extents);
typedef int 	(*def_OnBatchJobDone_OverrideCallback)(char *job_guid, char *status_text, char *time_string, char *windows_temp_dir);

// And these are the override callbacks which the programmer's COM client 
// program can specify so that they can hook into the event sink pipeline
typedef struct AllEventSinkOverrideInfo {
	def_OnGeometryImportProgress_OverrideCallback	OnGeometryImportProgress_Override;
	def_OnGeometryExportProgress_OverrideCallback	OnGeometryExportProgress_Override;
	def_OnErrorMessage_OverrideCallback		OnErrorMessage_Override;
	def_OnMessageWindowTextOutput_OverrideCallback 	OnMessageWindowTextOutput_Override;
	def_OnBatchJobDone_OverrideCallback 		OnBatchJobDone_Override;
} AllEventSinkOverrideInfo;
static AllEventSinkOverrideInfo event_sinks_override_fns;

// This class defines the connection point events (event sinks) used by this
// COM client and which are fired by the COM server (via NuGrafIOEvents in the .idl file)
class CTestClientSinkEvents: public IDispatchImpl<_INuGrafIOEvents, &IID__INuGrafIOEvents, &LIBID_COMSRVLib>, public CComObjectRoot 
{
	public:
		CTestClientSinkEvents() {};

	BEGIN_COM_MAP(CTestClientSinkEvents)
		COM_INTERFACE_ENTRY(IDispatch)
		COM_INTERFACE_ENTRY(_INuGrafIOEvents)
	END_COM_MAP()

	//-- Local event sinks contained entirely within this file
	virtual STDMETHODIMP    OnGeometryImportProgress(BSTR bstr_mode, BSTR bstr_info_text, long curr_file_offset, long file_length, long total_objects, long total_polygons);
	virtual STDMETHODIMP 	OnGeometryExportProgress( BSTR bstr_mode, long Nv_Scanline_Num, long Nv_Num_Scanlines);
	virtual STDMETHODIMP 	OnErrorMessage(BSTR Nv_Error_Level, BSTR Nv_Error_Label, BSTR Nv_Formatted_Message ) ;
	virtual STDMETHODIMP 	OnMessageWindowTextOutput( BSTR Nv_Formatted_Message );
	virtual STDMETHODIMP 	OnBatchJobDone( BSTR job_guid, BSTR status_text, BSTR time_string, BSTR windows_temp_dir ) ;
};

/* ------------------->>>>  Function Prototypes  <<<<--------------------- */

BOOL WINAPI OkinoCommonComSource___GeometryImportStatusDlgProc(HWND hDlg, WORD wMsg, WORD wParam, LONG lParam);
BOOL WINAPI OkinoCommonComSource___GeometryExportStatusDlgProc(HWND hDlg, WORD wMsg, WORD wParam, LONG lParam);

static BOOL WINAPI OkinoCommonComSource___RemoteHostDlgProc(HWND hwnd, UINT msg, UINT wparam, LONG lparam);
static BOOL WINAPI OkinoCommonComSource___ServerFaultDlgProc(HWND hDlg, WORD wMsg, WORD wParam, LONG lParam);

//-----------------------------------------------------------------------------

	void
OkinoCommonComSource___Initialization()
{
	// Pointers to our three interfaces exported from the COM server
	pNuGrafIO 	= NULL;
	pConvertIO	= NULL;
	pRenderIO	= NULL;

	// Init all the programmer-specified event sink overrides to NULL.
	memset((char *) &event_sinks_override_fns, 0, sizeof(AllEventSinkOverrideInfo)); 

	updating_edit_control = FALSE;
	abort_flag = FALSE;
	last_geometry_filename_selected[0] = '\0';
}

// Attach to the Okino COM server and instantiate the 2 main interface pointers.
// Returns TRUE if the COM interfaces were acquired, else FALSE if failure. 

	BOOL
OkinoCommonComSource___AttachToCOMInterface(HWND parent_hwnd, 
	HINSTANCE hInstance)
{
	short	status;

	// Initialize COM/OLE
	HRESULT hr = CoInitialize(NULL);
	if (!SUCCEEDED(hr)) {
		MessageBox(parent_hwnd, "Could not initialize COM and OLE interface.", "Fatal Error", MB_OK | MB_APPLMODAL);
		return FALSE;
	}

	// Initialize the COM client module, as defined and used by ATL
	_Module.Init(ObjectMap, hInstance);
	gInstance = hInstance;		// Make available to our event sink callbacks
	
	// Make sure Okino's PolyTrans/NuGraf type-lib has been registered.
	// This is TRUE if either of these programs have been run and their
	// COM server's have been registered (which is done automatically)
	if (!OkinoCommonComSource___IsTypeLibRegistered()) {
		//This is commented out because it a user does not have polytrans installed
        //the dialog appears every time the osgPoltrans plugin is loaded which 
        //can be annoying. If there is a way to tell at runtime other than this 
        //method if polytrans is installed we could uncomment this dialog code.
        //MessageBox(parent_hwnd, "The NuGraf or PolyTrans host programs have not been registered as COM/DCOM servers yet.\n\nPlease execute nugraf32.exe or pt32.exe at least once before executing this program.", "Fatal Error", MB_OK | MB_APPLMODAL);
		CoUninitialize();
		return FALSE;
	}

	// Create a local instance of the NuGraf interface
	status = OkinoCommonComSource___StartComClassInterfacesAndEventSink();
	if (!status)
		MessageBox(parent_hwnd, "Could not obtain handles to the NuGrafIO or ConvertIO class interfaces, or could not register the event sinks.", "Fatal Error", MB_OK | MB_APPLMODAL);

	return(status);
}

// Once we are done with the COM interfaces, call this routine to detach
// the COM client from the COM server. 

// This will also cause the server (nugraf32.exe or pt32.exe) to exit if 
// the pNuGrafIO->ExitServerProgramWhenLastCOMClientDetaches()
// function has been called with a TRUE argument. 

	void
OkinoCommonComSource___DetachFromCOMInterface()
{
	// Stop the event sinks
	OkinoCommonComSource___StopEventSink();

	// Release pointers to the interfaces. 
	if (pConvertIO)
		pConvertIO->Release();
	if (pNuGrafIO)
		pNuGrafIO->Release();
	if (pRenderIO)
		pRenderIO->Release();

	// Terminate ATL
	_Module.Term();

	// Shutdown COM/OLE
	CoUninitialize();
}

// Ensure that the COM server's GUI has been made visible. Normally it will
// be hidden when the COM server is started from a COM client. 

	short
OkinoCommonComSource___MakeCOMServerGUIVisible()
{
	HRESULT	hresult;

	try {
		short		com_error_result;

loop:		hresult = pNuGrafIO->MakeGUIVisible();
		// If fatal, ask the user what to do: retry, restart or abort. If restart, try restarting the server,
		com_error_result = OkinoCommonComSource___Check_COM_HRESULT_Error_CODE(hresult, "Error while asking COM server to make its graphical user interface visible. COM server has either crashed or is waiting for a user response.");
		if (com_error_result == IDC_SERVERFAULT_RETRY_OPERATION)
			goto loop;
		else if (com_error_result == IDC_SERVERFAULT_CANCEL_OPERATION)
			return(FALSE);
	} catch (_com_error &e) {
		OkinoCommonComSource___Display_COM_Error(e, "Error while asking COM server to make its graphical user interface visible. COM server has either crashed or is waiting for a user response.");
		return(FALSE);
	}

	return(TRUE);
}

// -------------------->>>  COM Server Instantiation  <<<<------------------------

// Create an instance of a specific exported interface class from a local 
// or remote COM server (as selected by the user).

	bool
OkinoCommonComSource___CreateObjectInstance(REFCLSID rclsid, REFIID riid, LPVOID *ppv) 
{
	HRESULT hr;
	short	use_local_server = TRUE;	// Must always be true

	if (use_local_server) {
		// Start up the local COM server (if it is not already running),
		// instantiate a class of the exported interface, and return a 
		// pointer to the class.
		hr = CoCreateInstance(rclsid, NULL, CLSCTX_LOCAL_SERVER, riid, ppv);

		if (FAILED(hr)) {
			MessageBox(NULL, "Could not connect to the NuGraf or PolyTrans COM server.\n\nIf NuGraf (nugraf32.exe) or PolyTrans (pt32.exe) are not currently running, then execute them now and try again.", "Fatal Error", MB_OK | MB_APPLMODAL);
			return FALSE;
		}
	} 
#if 0
	// Remote instantiation is not supported by the modern version of the COM server
	else {
		// Set up remote computer COM info (DNS name and security clearance)

		COSERVERINFO server;
		memset(&server, 0, sizeof(COSERVERINFO));
		CString serv_name = server_name;	// Create a CString
		server.pwszName = serv_name.AllocSysString();

		MULTI_QI mq;
		memset(&mq, 0, sizeof(MULTI_QI));
		mq.pIID = &riid;

		// Start up the remote COM server (if it is not already running),
		// instantiate a class of the exported interface, and return a 
		// pointer to the class.
		hr = CoCreateInstanceEx(rclsid, NULL, CLSCTX_REMOTE_SERVER | CLSCTX_LOCAL_SERVER, &server, 1, &mq);

		if (FAILED(hr) || FAILED(mq.hr)) {
			MessageBox(NULL, "Could not connect to the remote NuGraf or PolyTrans DCOM server.\n\nIf NuGraf (nugraf32.exe) or PolyTrans (pt32.exe) are not currently running, then execute them now and try again.", "Fatal Error", MB_OK | MB_APPLMODAL);
			return false;
		} else
			*ppv = mq.pItf;
	}
#endif

	return true;
}

// Create the local instances of the NuGraf & converter I/O interfaces, then start the event sink.
// If the COM server dies then we call into this routine to restart everything.
// Returns FALSE if failure.

	short
OkinoCommonComSource___StartComClassInterfacesAndEventSink()
{
	// Create a local/remote instance of the NuGraf interface
	if (OkinoCommonComSource___CreateObjectInstance(CLSID_NuGrafIO, IID_INuGrafIO, (void **) &pNuGrafIO)) {
		// Create a local/remote instance of the converter I/O interface
		if (OkinoCommonComSource___CreateObjectInstance(CLSID_ConvertIO, IID_IConvertIO, (void **) &pConvertIO)) {
			// If TRUE, have the server program (nugraf32.exe or pt32.exe) exit
			// once this client COM program has finished, else set to FALSE if
			// the server should remain open and active after all the clients
			// have finished and detached.
			pNuGrafIO->ExitServerProgramWhenLastCOMClientDetaches(TRUE);

			// Start up the event sinks which will allow the host COM server
			// to send to us messages and feedback information.
			if (!OkinoCommonComSource___StartEventSink()) {
				pConvertIO->Release();
				pNuGrafIO->Release();
				return(FALSE);
			}
		} else {
			pNuGrafIO->Release();
			return(FALSE);
		}
	} else
		return(FALSE);

	return(TRUE);
}

// Make sure Okino's PolyTrans/NuGraf type-lib has been registered.
// This is TRUE if either of these programs have been run and their
// COM server's have been registered (which is done automatically)

	bool 
OkinoCommonComSource___IsTypeLibRegistered( ) 
{
	HKEY hKey;

	// Let's see if the Okino PolyTrans/NuGraf type-lib has been registered
	if (RegOpenKeyEx(HKEY_CLASSES_ROOT, "TypeLib\\{35A72FD1-70D6-11d3-AEB7-0080C8E677B8}", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
		RegCloseKey(hKey);
		return TRUE;
	} else
		return FALSE;
}

// Determine if the host COM server program (pt32.exe or nugraf32.exe) has been properly
// licensed with serial numbers. This will also be FALSE if the host program is running in demo mode.

// Only version 4.1.8 or newer

	bool
OkinoCommonComSource___IsCOMServerLicensed()
{
	long	state;

	// Get the program flags
	state = OkinoCommonComSource___get_ProgramFlags();

	if (state & Nc_NRS_IO_FLAGS1_COMPILED_AS_DEMO_VERSION)
		return FALSE;

	if (state & Nc_NRS_IO_FLAGS1_DEMO_MODE)
		return FALSE;

	return TRUE;	// Host program is licensed properly
}

// Show the serial number registration dialog box in the host COM server program

// If 'ok_to_ask_user_to_register_online' is Nc_TRUE then the user will be prompted with "Do you want to
// also register your product online" once they enter valid serial numbers. Else Nc_FALSE will suppress
// this request message box.

// Only version 4.1.8 or newer

	void
OkinoCommonComSource___ShowCOMServerRegistrationDialogBox(HWND hwndParent, BOOL ok_to_ask_user_to_register_online)
{
	HRESULT 	hresult;
	_bstr_t 	dialog_box_name_bstr = "RegistrationDialogBox";
	long		state;

	// Get the program flags
	state = OkinoCommonComSource___get_ProgramFlags();

	// If the COM server is compiled as demo mode then return now since the hard coded demo
	// mode can't show the serial numbers dialog box
	if (state & Nc_NRS_IO_FLAGS1_COMPILED_AS_DEMO_VERSION)
		return;

	try {
		short		com_error_result;

loop:		hresult = pNuGrafIO->ShowInternalDialogBox(dialog_box_name_bstr, (long) hwndParent, NULL, NULL, ok_to_ask_user_to_register_online, 0);
		// E_FAIL means that S_FALSE is being returned from the dialog box
		if (hresult == E_FAIL)
			hresult = S_FALSE;

		// If fatal, ask the user what to do: retry, restart or abort. If restart, try restarting the server,
		com_error_result = OkinoCommonComSource___Check_COM_HRESULT_Error_CODE(hresult, "Error while asking COM server to show the serial numbers registration dialog box. COM server has either crashed or is waiting for a user response.");
		if (com_error_result == IDC_SERVERFAULT_RETRY_OPERATION)
			goto loop;
		else if (com_error_result == IDC_SERVERFAULT_CANCEL_OPERATION)
			return;
	} catch (_com_error &e) {
		OkinoCommonComSource___Display_COM_Error(e, "Error while asking COM server to the serial numbers registration dialog box. COM server has either crashed or is waiting for a user response.");
		return;
	}
}

// Client side cover function to the "get_ProgramFlags(&state)" COM method

	unsigned long
OkinoCommonComSource___get_ProgramFlags()
{
	HRESULT 	hresult;
	long		state;

	// Get the program flags
	try {
		short		com_error_result;

loop:		hresult = pNuGrafIO->get_ProgramFlags(&state);
		// If fatal, ask the user what to do: retry, restart or abort. If restart, try restarting the server,
		com_error_result = OkinoCommonComSource___Check_COM_HRESULT_Error_CODE(hresult, "Error while asking COM server to get the program flags. COM server has either crashed or is waiting for a user response.");
		if (com_error_result == IDC_SERVERFAULT_RETRY_OPERATION)
			goto loop;
		else if (com_error_result == IDC_SERVERFAULT_CANCEL_OPERATION)
			return 0L;
	} catch (_com_error &e) {
		OkinoCommonComSource___Display_COM_Error(e, "Error while asking COM server to get the program flags. COM server has either crashed or is waiting for a user response.");
		return 0L;
	}

	return state;
}

// Client side cover function to get the version numbers of the main COM server and host program.
// Returns FALSE if the version number could not be obtained. The original COM API was compliant with v2.2.30

	BOOL
OkinoCommonComSource___get_ProgramVersionInfo(short *major_version, short *minor_version, short *sub_minor_version)
{
	SAFEARRAY 	*out_params;		// Resulting output parameters
	HRESULT 	hresult;
	CComVariant 	val;
	CComVariant 	str_var, short_var;	// These hold our variant types (string or short)
	long		index;

	*major_version = *minor_version = *sub_minor_version = 0;

	VARIANT out_params_variant;
	VariantInit(&out_params_variant);

	try {
		short		com_error_result;

loop:		hresult = pNuGrafIO->get_ProgramVersionInfo(&out_params_variant);
		// If fatal, ask the user what to do: retry, restart or abort. If restart, try restarting the server,
		com_error_result = OkinoCommonComSource___Check_COM_HRESULT_Error_CODE(hresult, "Error while asking COM server to get the program version info. COM server has either crashed or is waiting for a user response.");
		if (com_error_result == IDC_SERVERFAULT_RETRY_OPERATION)
			goto loop;
		else if (com_error_result == IDC_SERVERFAULT_CANCEL_OPERATION)
			return 0L;
	} catch (_com_error &e) {
		OkinoCommonComSource___Display_COM_Error(e, "Error while asking COM server to get the program version info. COM server has either crashed or is waiting for a user response.");
		return 0L;
	}

	out_params = V_ARRAY(&out_params_variant);	// Get the safe array out of the variant
	index = 0;
	if (out_params) {
		// Short return arguments

		SafeArrayGetElement(out_params, &index, (void *) &val); ++index;
		*major_version = val.iVal;

		SafeArrayGetElement(out_params, &index, (void *) &val); ++index;
		*minor_version = val.iVal;

		SafeArrayGetElement(out_params, &index, (void *) &val); ++index;
		*sub_minor_version = val.iVal;

		SafeArrayDestroy(out_params);
	}

{ char buf[128];

sprintf(buf, "Version = %d %d %d", *major_version, *minor_version, *sub_minor_version);

MessageBox(NULL, buf, "Error", MB_OK);
}


	return TRUE;
}

// Determine if the version of the host COM server is greater than or equal to this version specified

	BOOL
OkinoCommonComSource___IsHostProgramVersionGreaterThanOrEqualTo(short major_version, short minor_version, short sub_minor_version)
{
	short major, minor, sub_minor;

	if (!OkinoCommonComSource___get_ProgramVersionInfo(&major, &minor, &sub_minor))
		return FALSE;

	if (major_version > major || (major_version == major && minor_version > minor) || (major_version == major && minor_version == minor && sub_minor_version >= sub_minor))
		return TRUE;
	else
		return FALSE;
}

// ---------------------->>>  Event Sinks  <<<<------------------------------

	void
OkinoCommonComSource___SetEventSinkOverrideCallbackFuntion(char *event_sink_name,
	Nd_Func2 callback)
{
	if (!strcmp(event_sink_name, "OnGeometryImportProgress")) {
		event_sinks_override_fns.OnGeometryImportProgress_Override = (def_OnGeometryImportProgress_OverrideCallback) callback;
	} else if (!strcmp(event_sink_name, "OnGeometryExportProgress")) {
		event_sinks_override_fns.OnGeometryExportProgress_Override = (def_OnGeometryExportProgress_OverrideCallback) callback;
	} else if (!strcmp(event_sink_name, "OnErrorMessage")) {
		event_sinks_override_fns.OnErrorMessage_Override = (def_OnErrorMessage_OverrideCallback) callback;
	} else if (!strcmp(event_sink_name, "OnMessageWindowTextOutput")) {
		event_sinks_override_fns.OnMessageWindowTextOutput_Override = (def_OnMessageWindowTextOutput_OverrideCallback) callback;
	} else if (!strcmp(event_sink_name, "OnBatchJobDone")) {
		event_sinks_override_fns.OnBatchJobDone_Override = (def_OnBatchJobDone_OverrideCallback) callback;
	}
}

// This starts up the event sinks. Event sinks allow the host COM server
// to convey messages and information back to this client, such as runtime
// error messages. 

static DWORD 					event_sink_cookie = 0;
static CComPtr<IUnknown>			ptrTestClientSinkEventsUnk = NULL;
static CComObject<CTestClientSinkEvents>	*pTestClientSinkEvents = NULL;

	short
OkinoCommonComSource___StartEventSink() 
{
	if (!pTestClientSinkEvents) {
		pTestClientSinkEvents = new CComObject<CTestClientSinkEvents>;
		ptrTestClientSinkEventsUnk = pTestClientSinkEvents;

		HRESULT hr = AtlAdvise(pNuGrafIO, ptrTestClientSinkEventsUnk, IID__INuGrafIOEvents, &event_sink_cookie);
		if (FAILED(hr)) {
			pTestClientSinkEvents = 0;
			OkinoCommonComSource___StopEventSink();
			return(FALSE);
		} 
	}
	return(TRUE);
}

// This stops and terminates the event sinks

	void 
OkinoCommonComSource___StopEventSink() 
{
	if (pTestClientSinkEvents) {
		AtlUnadvise(pNuGrafIO, IID__INuGrafIOEvents, event_sink_cookie);
		pTestClientSinkEvents = 0;
	}
	if (ptrTestClientSinkEventsUnk)
		ptrTestClientSinkEventsUnk = 0;
}

// This event is fired while a geometry import process is under way.

// This is the method by which the import converter mechanism can pass
// status information to the COM clients (to display on their import status
// dialog box) and to ask the COM clients if the abort button has been pressed.
//
// 'mode' is one of:
//	"initialize"
//	"shutdown"
//	"updatestatusandcheckforabort", use third argument to pass update info
//		- Must return E_FAIL if aborted, else S_OK if not aborted.
//	"showinfotext", uses second argment to display info text

	STDMETHODIMP
CTestClientSinkEvents::OnGeometryImportProgress(
	BSTR bstr_mode,			// Which mode we are working in
	BSTR bstr_info_text,		// Info text for the "showinfotext" mode
	long curr_file_offset,		/* Current offset into the file (-1 to make dialog box field blank) */
	long file_length,		/* Length of the input file */
	long total_objects,		/* Total number of objects created to date (-1 to make dialog box field blank) */
	long total_polygons )		/* Total number of polygons created to date (-1 to make dialog box field blank) */
{
	USES_CONVERSION;
	static	HWND	hDlgGeomImportStatus;	/* Handle to the geometry import status dialog */
	LPTSTR mode = OLE2T(bstr_mode);
	LPTSTR info_text = OLE2T(bstr_info_text);

	// Give the programmer's COM client code a chance to see this event
	// message and handle it inside their own local code. 
	if (event_sinks_override_fns.OnGeometryImportProgress_Override) {
		// If the override returns TRUE, then return immediately
		if ((*event_sinks_override_fns.OnGeometryImportProgress_Override)(mode, info_text, curr_file_offset, file_length, total_objects, total_polygons))
			return S_OK;
	}

	if (!strcmp(mode, "initialize")) {
		/* Create the modeless geometry import status dialog box. */
		if (parent_hwnd == NULL)
			// Warn the programmer that the global 'parent_hwnd' has not been set up yet. 
			MessageBox(parent_hwnd, "CTestClientSinkEvents::OnGeometryImportProgress(), the global 'parent_hwnd' has not been initialized yet. Please initialize this global variable in your code to the parent window of the geometry import status dialog box.", "Programming Error", MB_OK | MB_APPLMODAL);
		hDlgGeomImportStatus = CreateDialog(gInstance, "GeometryImportStatusDialog", parent_hwnd, (DLGPROC) OkinoCommonComSource___GeometryImportStatusDlgProc);
		if (!hDlgGeomImportStatus)
		{
			// Warn the programmer that the import status dialog box could not be created. 
			// Either the .rc file was not included or maybe the hInstance handle is wrong.
			MessageBox(parent_hwnd, "CTestClientSinkEvents::OnGeometryImportProgress(), could not create the import status dialog box.", "Programming Error", MB_OK | MB_APPLMODAL);
		}
		SetDlgItemText(hDlgGeomImportStatus, IDD_GEOMIMPORT_LINE_NUM, "0");
		SetDlgItemText(hDlgGeomImportStatus, IDD_GEOMIMPORT_TOTAL_OBJECTS, "0");
		SetDlgItemText(hDlgGeomImportStatus, IDD_GEOMIMPORT_TOTAL_POLYS, "0");
		SetDlgItemText(hDlgGeomImportStatus, IDD_GEOMIMPORT_TEXT, "");
		OkinoCommonComSource___SimulateAMeter(hDlgGeomImportStatus, IDD_METER_CONTROL, 0, 100);
		ShowWindow(hDlgGeomImportStatus, SW_SHOW);
		UpdateWindow(hDlgGeomImportStatus);
		abort_flag = FALSE;
	} else if (!strcmp(mode, "shutdown")) {
		/* Destroy the geometry import status dialog box */
		if (hDlgGeomImportStatus)
			DestroyWindow(hDlgGeomImportStatus);
		hDlgGeomImportStatus = NULL;
	} else if (!strcmp(mode, "updatestatusandcheckforabort")) {
		short	state;
		char	buf[10];

		if (curr_file_offset > 0) {
			sprintf(buf, "%ld", curr_file_offset);
			SetDlgItemText(hDlgGeomImportStatus, IDD_GEOMIMPORT_LINE_NUM, buf);
			state = TRUE;
			OkinoCommonComSource___SimulateAMeter(hDlgGeomImportStatus, IDD_METER_CONTROL, curr_file_offset, file_length);
		} else
			state = FALSE;
		OkinoCommonComSource___Set_Control_Enable(hDlgGeomImportStatus, IDD_GEOMIMPORT_LINE_NUM, state);
		OkinoCommonComSource___Set_Control_Enable(hDlgGeomImportStatus, IDD_GEOMIMPORT_LINE_NUM_TEXT, state);

		if (total_objects > 0) {
			sprintf(buf, "%ld", total_objects);
			SetDlgItemText(hDlgGeomImportStatus, IDD_GEOMIMPORT_TOTAL_OBJECTS, buf);
			state = TRUE;
		} else
			state = FALSE;
		OkinoCommonComSource___Set_Control_Enable(hDlgGeomImportStatus, IDD_GEOMIMPORT_TOTAL_OBJECTS, state);
		OkinoCommonComSource___Set_Control_Enable(hDlgGeomImportStatus, IDD_GEOMIMPORT_TOTAL_OBJECTS_TEXT, state);

		if (total_polygons > 0) {
			sprintf(buf, "%ld", total_polygons);
			SetDlgItemText(hDlgGeomImportStatus, IDD_GEOMIMPORT_TOTAL_POLYS, buf);
			state = TRUE;
		} else
			state = FALSE;
		OkinoCommonComSource___Set_Control_Enable(hDlgGeomImportStatus, IDD_GEOMIMPORT_TOTAL_POLYS, state);
		OkinoCommonComSource___Set_Control_Enable(hDlgGeomImportStatus, IDD_GEOMIMPORT_TOTAL_POLYS_TEXT, state);

		/* If the user has pressed the "CANCEL" button then abort */
		OkinoCommonComSource___CheckAndProcessWindowsMessageQueue();
		if (abort_flag) {
			abort_flag = FALSE;	/* So that we can restart after an abort */
			return(E_FAIL);	// Aborting
		}

		return(S_OK);
	} else if (!strcmp(mode, "showinfotext")) {
		if (hDlgGeomImportStatus)
			SetDlgItemText(hDlgGeomImportStatus, IDD_GEOMIMPORT_TEXT, info_text);
	}

	return S_OK;	// No import abort yet
}

// This is the dialog box handler for the geometry import status dialog

	BOOL WINAPI
OkinoCommonComSource___GeometryImportStatusDlgProc(HWND hDlg, WORD wMsg, WORD wParam, LONG lParam)
{
	lParam = lParam;

	switch (wMsg) {
		case WM_INITDIALOG:
			/* Center the dialog */
			OkinoCommonComSource___Center_Window(GetParent(hDlg), hDlg);
			break;
		case WM_SHOWWINDOW:
			if (!wParam)
				break;
			EnableWindow(GetDlgItem(hDlg, IDD_GEOMIMPORT_CANCEL), TRUE);
			break;
		case WM_COMMAND:
			switch (wParam) {
				case IDOK:
					/* User presses ENTER. Do the CANCEL case. */
				case IDD_GEOMIMPORT_CANCEL:
					/* Set the global abort flag to tell the renderer to stop */
					abort_flag = TRUE;
					break;
			}
			break;
		default:
			break;
	}
	return(FALSE);
}

// This event is fired while a geometry export process is under way. 

// This is the method by which the export converter mechanism can pass
// status information to the COM clients (to display on their export status
// dialog box) and to ask the COM clients if the abort button has been pressed.
//
// 'mode' is one of:
//	"initialize"
//	"shutdown"
//	"proceed" 
//		- Must return E_FAIL if aborted, else S_OK if not aborted.

static HWND	hDlgExportAbortDialog = NULL;

	STDMETHODIMP 
CTestClientSinkEvents::OnGeometryExportProgress( 
	BSTR 	bstr_mode,			// Which mode we are working in
	long	Nv_Scanline_Num,
	long	Nv_Num_Scanlines)
{
	USES_CONVERSION;
	LPTSTR mode = OLE2T(bstr_mode);
	char	buf[128];

	// Give the programmer's COM client code a chance to see this event
	// message and handle it inside their own local code. 
	if (event_sinks_override_fns.OnGeometryExportProgress_Override) {
		// If the override returns TRUE, then return immediately
		if ((*event_sinks_override_fns.OnGeometryExportProgress_Override)(mode, Nv_Scanline_Num, Nv_Num_Scanlines))
			return S_OK;
	}

	if (!strcmp(mode, "initialize")) {
		abort_flag = FALSE;
		if (hDlgExportAbortDialog == NULL) {
			if (parent_hwnd == NULL)
				// Warn the programmer that the global 'parent_hwnd' has not been set up yet. 
				MessageBox(parent_hwnd, "CTestClientSinkEvents::OnGeometryExportProgress(), the global 'parent_hwnd' has not been initialized yet. Please initialize this global variable in your code to the parent window of the geometry export status dialog box.", "Programming Error", MB_OK | MB_APPLMODAL);

			hDlgExportAbortDialog = CreateDialog(gInstance, "ExportAbortDialog", parent_hwnd, (DLGPROC) OkinoCommonComSource___GeometryExportStatusDlgProc);

			// Warn the programmer that the export status dialog box could not be created. 
			// Either the .rc file was not included or maybe the hInstance handle is wrong.
			if (!hDlgExportAbortDialog)
				MessageBox(parent_hwnd, "CTestClientSinkEvents::OnGeometryExportProgress(), could not create the export status dialog box.", "Programming Error", MB_OK | MB_APPLMODAL);
		}
		strcpy(buf, "0");
		SetDlgItemText(hDlgExportAbortDialog, IDD_GEOMEXPORT_HANDLETYPE, "");
		SetDlgItemText(hDlgExportAbortDialog, IDD_GEOMEXPORT_HANDLENAME, "");
		/* Make the meter status dialog box visible */
		ShowWindow(hDlgExportAbortDialog, SW_SHOW);
		UpdateWindow(hDlgExportAbortDialog);
		OkinoCommonComSource___SimulateAMeter(hDlgExportAbortDialog, IDD_METER_CONTROL, 0, Nv_Num_Scanlines);
		SendMessage(hDlgExportAbortDialog, WM_SETTEXT, 0, (LPARAM) "Geometry Export Status");
	} else if (!strcmp(mode, "shutdown")) {
		/* Destroy the status dialog box */
		if (hDlgExportAbortDialog)
			DestroyWindow(hDlgExportAbortDialog);
		hDlgExportAbortDialog = NULL;
	} else if (!strcmp(mode, "proceed")) {
		OkinoCommonComSource___CheckAndProcessWindowsMessageQueue();
		if (Nv_Num_Scanlines >= 0)
			sprintf(buf, "%ld", Nv_Scanline_Num);
		else
			strcpy(buf, "0");
		OkinoCommonComSource___SimulateAMeter(hDlgExportAbortDialog, IDD_METER_CONTROL, Nv_Scanline_Num, Nv_Num_Scanlines);

		OkinoCommonComSource___CheckAndProcessWindowsMessageQueue();
		if (abort_flag) {
			abort_flag = FALSE;
			return(E_FAIL);	// Aborting
		}

		return(S_OK);
	}

	return S_OK;		// No abort yet
}

// This is the dialog box handler for the geometry export status dialog

	BOOL WINAPI
OkinoCommonComSource___GeometryExportStatusDlgProc(HWND hDlg, WORD wMsg, WORD wParam, LONG lParam)
{
	short	cmd_id;

	lParam = lParam;
	switch (wMsg) {
		case WM_INITDIALOG:
			/* Center the dialog */
			OkinoCommonComSource___Center_Window(GetParent(hDlg), hDlg);
			break;
		case WM_SHOWWINDOW:
			if (!LOWORD(wParam))
				break;
			EnableWindow(GetDlgItem(hDlg, IDD_METER_CANCEL), TRUE);
			break;
		case WM_COMMAND:
			cmd_id = LOWORD(wParam);
			switch (cmd_id) {
				case IDOK:
					/* User presses ENTER. Do the CANCEL case. */
				case IDD_METER_CANCEL:
					/* Set the global abort flag to tell the renderer to stop */
					abort_flag = TRUE;
					break;
			}
			break;
		default:
			break;
	}
	return(FALSE);
}

/* This routine prints out text on the geometry export abort dialog box */
/* (normally the info strings for the status bar are routed here instead). */

	void
OkinoCommonComSource___OutputInfoTextToGeomExportAbortStatusDlg(char *text)
{
	char	buf2[256], buf3[64];
	short	count, count2;

	if (!hDlgExportAbortDialog)
		return;

	if (!text) return;

	if (*text == ' ')
		++text;
	buf2[0] = buf3[0] = '\0';
	if (*text == '\"') {
		if (!strncmp(text, "\"instance\"", 10)) {
			text += 10;
			strcpy(buf3, "Processing instance:");
		} else if (!strncmp(text, "\"light\"", 7)) {
			text += 7;
			strcpy(buf3, "Processing light:");
		} else if (!strncmp(text, "\"camera\"", 8)) {
			text +=8;
			strcpy(buf3, "Processing camera:");
		} else if (!strncmp(text, "\"surface\"", 9)) {
			text += 9;
			strcpy(buf3, "Processing surface:");
		} else if (!strncmp(text, "\"texture\"", 9)) {
			text += 9;
			strcpy(buf3, "Processing texture:");
		}

		++text;
		/* The first part of the string is the handle name which we */
		/* are currently processing. Strip it off and display on the */
		/* dialog box separately. */
		count = strlen(text);
		count2 = '\0';
		while (count && *text != '\"') {
			buf2[count2++] = *(text++);
			--count;
			buf2[count2] = '\0';
		}
		if (*text == '\"')
			++text;
	}
	if (buf3[0] != '\0')
		/* Set the "Processing..." text */
		SetDlgItemText(hDlgExportAbortDialog, IDD_GEOMEXPORT_PROCESSING, buf3);
	if (buf2[0] != '\0') {
		/* Set the current handle name */
		SetDlgItemText(hDlgExportAbortDialog, IDD_GEOMEXPORT_HANDLETYPE, buf2);
		SendDlgItemMessage(hDlgExportAbortDialog, IDD_GEOMEXPORT_HANDLETYPE, EM_SETSEL, -1, 32767);
		if (buf3[0] == '\0')
			/* Default the "Processing..." text */
			SetDlgItemText(hDlgExportAbortDialog, IDD_GEOMEXPORT_PROCESSING, "Processing:");
	}
	/* Set the current descriptive text */
	SetDlgItemText(hDlgExportAbortDialog, IDD_GEOMEXPORT_HANDLENAME, text);
	/* Set the focus to the static text control so that the caret goes away */
	SetFocus(GetDlgItem(hDlgExportAbortDialog, IDD_GEOMEXPORT_PROCESSING));
	OkinoCommonComSource___CheckAndProcessWindowsMessageQueue();
}

// This event is fired when the host program has a NuGraf Toolkit error, 
// warning or info message to display. 

// 'Nv_Error_Level' is one of the following strings:
//	"info"		- This is just an informational message
//	"warning"	- This is a warning message
//	"error"		- This is a recoverable message
//	"fatal"		- This is a fatal error. Non-recoverable

// 'Nv_Error_Label' is the label associated with this error messages. This
// label comes from the left side of each text message in the nugraf1.msg file.

// 'Nv_Formatted_Message' is the actual text of the error message. 

static long	max_error_msg_listbox_extent = 0;

	STDMETHODIMP 
CTestClientSinkEvents::OnErrorMessage(BSTR Nv_Error_Level, 
	BSTR Nv_Error_Label, BSTR Nv_Formatted_Message ) 
{
	USES_CONVERSION;
	LPTSTR error_level = OLE2T(Nv_Error_Level);
	LPTSTR error_label = OLE2T(Nv_Error_Label);
	LPTSTR formatted_message = OLE2T(Nv_Formatted_Message);

	// Give the programmer's COM client code a chance to see this event
	// message and handle it inside their own local code. 
	if (event_sinks_override_fns.OnErrorMessage_Override) {
		// If the override returns TRUE, then return immediately
		if ((*event_sinks_override_fns.OnErrorMessage_Override)(error_level, error_label, formatted_message))
			return S_OK;
	}

	// !! For this text client, we'll only handle "info" status messages
	//    in here. For all other error messages we'll assume that they
	//    are output to the text client via the OnMessageWindowTextOutput() 
	//    routine below which gets all output text, including these errrors.

	if (!strcmp(error_level, "info")) {
		// If this is purely an informational message, then go and
		// place it on the status line and not in the message window
		if (formatted_message && strlen(formatted_message)) {
			/* If the geometry export dialog box is open then */
			/* send all info messages to it for display. */
			if (hDlgExportAbortDialog && strcmp(error_label, "INFO_EXPORT_STATS"))
				OkinoCommonComSource___OutputInfoTextToGeomExportAbortStatusDlg(formatted_message);
			else
				SendMessage(curr_Status_Msg_Text_Line_hwnd, WM_SETTEXT, 0, (LPARAM) formatted_message);
		}
	} 

	return S_OK;
}

// This event is fired when the host program is about to display *any* text
// to the host message/error output window. This also includes all text output
// to the OnErrorMessage() routine above, except for "info" messages which
// are only handled by the OnErrorMessage().

	STDMETHODIMP 
CTestClientSinkEvents::OnMessageWindowTextOutput(BSTR Nv_Formatted_Message )
{
	USES_CONVERSION;
	LPTSTR formatted_message = OLE2T(Nv_Formatted_Message);
	char	final_string[2048];
	short	extent;

	final_string[0] = '\0';
	extent = 0;
	if (formatted_message && strlen(formatted_message)) {
		short	i, j, count;

		// Ignore script comment lines
		if (formatted_message[0] == '#')
			return(S_OK);

		// Ignore script number prompt lines
		if (formatted_message[1] == '>' || formatted_message[2] == '>' || formatted_message[3] == '>' || !strncmp(formatted_message, "listscript", 10))
			return(S_OK);

		// Turn any tab characters into spaces
		count = 0;
		for (i=0; i < (short) strlen(formatted_message); ++i) {
			if (formatted_message[i] == 0x9) {
				for(j=0; j < 8; ++j)
					final_string[count++] = ' ';
			} else if (formatted_message[i] == 0xa || formatted_message[i] == 0xd) {
				;
			} else
				final_string[count++] = formatted_message[i];
			final_string[count] = '\0';
		}

		/* Set the horizontal extent of this string */
		extent = OkinoCommonComSource___WGetListboxStringExtent(curr_Error_Msg_Listbox_hwnd, final_string);
		if (extent > max_error_msg_listbox_extent) {
			max_error_msg_listbox_extent = extent;
			SendMessage(curr_Error_Msg_Listbox_hwnd, LB_SETHORIZONTALEXTENT, extent, 0L);
		}
	} 

	// Give the programmer's COM client code a chance to see this event
	// message and handle it inside their own local code. 
	if (event_sinks_override_fns.OnMessageWindowTextOutput_Override) {
		// If the override returns TRUE, then return immediately
		if ((*event_sinks_override_fns.OnMessageWindowTextOutput_Override)(final_string, extent))
			return S_OK;
	}

	if (curr_Error_Msg_Listbox_hwnd && strlen(final_string)) {
		/* Send the string to the list box */
		if (SendMessage(curr_Error_Msg_Listbox_hwnd, LB_ADDSTRING, NULL, (LONG)(LPSTR) final_string) == LB_ERRSPACE) {
			SendMessage(curr_Error_Msg_Listbox_hwnd, LB_RESETCONTENT, 0, 0);
			SendMessage(curr_Error_Msg_Listbox_hwnd, LB_ADDSTRING, NULL, (LONG)(LPSTR) final_string);
		}
	} else if (curr_Error_Msg_Listbox_hwnd && !strlen(final_string))
		SendMessage(curr_Error_Msg_Listbox_hwnd, LB_ADDSTRING, NULL, (LONG)(LPSTR) "");

	return S_OK;
}

// This event is fired when the batch converter has completed one of the
// batch jobs which this client program had queued up in the past. 

// 'job_guid'
//	- This is the GUID assigned to the job originally by the COM server
// 'status_text'
//	- "status = crashed", "status = completed" or "status = aborted"
// 'time_string'
//	- "Running_Time = HH:MM:SS"
// 'windows_temp_dir'
//	- This is the directory where the info file about the completed job
//	  is contained. When a running job is completed, it's filename is
// 	  renamed to the 'job_guid' name and placed in this directory. This
//	  file contains all information related to that running job. 

	STDMETHODIMP 
CTestClientSinkEvents::OnBatchJobDone( 
	BSTR job_guid_bstr,		// The Job ID
	BSTR status_text_bstr, 		// Signifies if the job completed, crashed or aborted
	BSTR time_string_bstr, 		// Time completion string
	BSTR windows_temp_dir_bstr)	// Windows TEMP directory we are using to store the queue files
{
	//USES_CONVERSION;
	//LPTSTR job_guid = OLE2T(job_guid_bstr);
	//LPTSTR status_text = OLE2T(status_text_bstr);
	//LPTSTR time_string = OLE2T(time_string_bstr);
	//LPTSTR windows_temp_dir = OLE2T(windows_temp_dir_bstr);

	//// Go remove the job from the job list
	//OkinoCommonComSource___HandleJobCompletedNotificationFromCOMServer(job_guid, status_text, time_string, windows_temp_dir);

	//// Give the programmer's COM client code a chance to see this event
	//// message and handle it inside their own local code. 
	//if (event_sinks_override_fns.OnBatchJobDone_Override) {
	//	// If the override returns TRUE, then return immediately
	//	if ((*event_sinks_override_fns.OnBatchJobDone_Override)(job_guid, status_text, time_string, windows_temp_dir))
	//		return S_OK;
	//}

	return S_OK;
}

// -------------->>>> Create Convenienet List of Known Exporters <<<<------------

// This routine creates a convenient list of all known export converters and
// their associated attributes, as queried from the host COM server. After
// you are done with this list use the FreeListofExportersAndTheirAttributesObtainedFromCOMServer()
// routine to free it up. The number of export converters in the list is stored
// in the first element of the list, 'list[0].num_exporters_in_list'.

	Nd_Export_Converter_Info_List *
OkinoCommonComSource___GetListofExportersAndTheirAttributesFromCOMServer()
{
	USES_CONVERSION;			// Since we are using OLE2T() macro
	Nd_Export_Converter_Info_List	*list;
	char		*str_ptr;
	short		i, num_converters;
	SAFEARRAY 	*psa;			// The parameters are passed in via a SAFEARRAY

	if (!pConvertIO)
		return(NULL);

	// NOTE: If the export converter is an internal exporter and not 
	//	 one of the newfangled plug-in exporters, then the following
	//	 variables will have these values:
	//
	//	*dll_filepath = NULL
	//	*plugin_directory = NULL
	//	plugin_version = -1;
	//	has_options_dialog_box = TRUE
	//	has_about_dialog_box = TRUE
	//	*plugin_type_description = NULL
	//	*plugin_descriptive_name = NULL
	//	*menu_description = NULL
	//	*fileopen_filter_spec = NULL
	//	show_file_selector_before_exporting = TRUE

	// Find out how many export converters there are
	try {
		HRESULT		hresult;
		short		com_error_result;

		// Find out how many import converters there are
loop:		hresult = pConvertIO->get_NumExportConverters(&num_converters);

		// If fatal, ask the user what to do: retry, restart or abort. If restart, try restarting the server,
		com_error_result = OkinoCommonComSource___Check_COM_HRESULT_Error_CODE(hresult, "Error while asking COM server for the number of export converters. COM server has either crashed or is waiting for a user response.");
		if (com_error_result == IDC_SERVERFAULT_RETRY_OPERATION)
			goto loop;
		else if (com_error_result == IDC_SERVERFAULT_CANCEL_OPERATION)
			return(NULL);
	} catch (_com_error &e) {
		OkinoCommonComSource___Display_COM_Error(e, "Error while asking COM server for the number of export converters. COM server has either crashed or is waiting for a user response.");
		return(NULL);
	}

	if (!num_converters)
		return(NULL);

	// Allocate memory for all of the exporters
	list = (Nd_Export_Converter_Info_List *) malloc(num_converters * sizeof(Nd_Export_Converter_Info_List));
	memset((char *) list, 0, num_converters * sizeof(Nd_Export_Converter_Info_List));
	// The first element in the list will contain the number of exporters in the entire list
	list[0].num_exporters_in_list = num_converters;

	for (i=0; i < num_converters; ++i) {
		CComVariant val;
		long  index = 0;

		VARIANT out_options_variant;
		VariantInit(&out_options_variant);

		// Ask the COM server to go and ask the Windows UI for all
		// major parameters to an export converter and stuff in a 
		// 1D SAFEARRAY with type VT_VARIANT
		try {
			HRESULT		hresult;
			short		com_error_result;

			// Find out how many import converters there are
loop2:			hresult = pConvertIO->get_SpecificExportConverterInfo(i, 1, &out_options_variant);

			// If fatal, ask the user what to do: retry, restart or abort. If restart, try restarting the server,
			com_error_result = OkinoCommonComSource___Check_COM_HRESULT_Error_CODE(hresult, "Error while asking COM server for export converter info. COM server has either crashed or is waiting for a user response.");
			if (com_error_result == IDC_SERVERFAULT_RETRY_OPERATION)
				goto loop2;
			else if (com_error_result == IDC_SERVERFAULT_CANCEL_OPERATION)
				return(NULL);
		} catch (_com_error &e) {
			OkinoCommonComSource___Display_COM_Error(e, "Error while asking COM server for export converter info. COM server has either crashed or is waiting for a user response.");
			return(NULL);
		}


		psa = V_ARRAY(&out_options_variant);	// Get the safe array out of the variant
		if (!psa)
			continue;

		SafeArrayGetElement(psa, &index, (void *) &val); ++index;
		str_ptr = OLE2T(val.bstrVal);
		if (str_ptr)
			OkinoCommonComSource___Copy_String(&list[i].plugin_type_description, str_ptr);

		SafeArrayGetElement(psa, &index, (void *) &val); ++index;
		str_ptr = OLE2T(val.bstrVal);
		if (str_ptr)
			OkinoCommonComSource___Copy_String(&list[i].plugin_descriptive_name, str_ptr);

		SafeArrayGetElement(psa, &index, (void *) &val); ++index;
		str_ptr = OLE2T(val.bstrVal);
		if (str_ptr)
			OkinoCommonComSource___Copy_String(&list[i].menu_description, str_ptr);

		SafeArrayGetElement(psa, &index, (void *) &val); ++index;
		str_ptr = OLE2T(val.bstrVal);
		if (str_ptr)
			OkinoCommonComSource___Copy_String(&list[i].fileopen_filter_spec, str_ptr);

		SafeArrayGetElement(psa, &index, (void *) &val); ++index;
		str_ptr = OLE2T(val.bstrVal);
		if (str_ptr)
			OkinoCommonComSource___Copy_String(&list[i].file_type_desc, str_ptr);

		SafeArrayGetElement(psa, &index, (void *) &val); ++index;
		str_ptr = OLE2T(val.bstrVal);
		if (str_ptr)
			OkinoCommonComSource___Copy_String(&list[i].file_extensions, str_ptr);

		SafeArrayGetElement(psa, &index, (void *) &val); ++index;
		str_ptr = OLE2T(val.bstrVal);
		if (str_ptr)
			OkinoCommonComSource___Copy_String(&list[i].help_ref, str_ptr);

		SafeArrayGetElement(psa, &index, (void *) &val); ++index;
		str_ptr = OLE2T(val.bstrVal);
		if (str_ptr)
			OkinoCommonComSource___Copy_String(&list[i].guid, str_ptr);

		SafeArrayGetElement(psa, &index, (void *) &val); ++index;
		str_ptr = OLE2T(val.bstrVal);
		if (str_ptr)
			OkinoCommonComSource___Copy_String(&list[i].dll_filepath, str_ptr);

		SafeArrayGetElement(psa, &index, (void *) &val); ++index;
		str_ptr = OLE2T(val.bstrVal);
		if (str_ptr)
			OkinoCommonComSource___Copy_String(&list[i].plugin_directory, str_ptr);
		
		// Short variables
		SafeArrayGetElement(psa, &index, (void *) &val); ++index;
		list[i].menu_id = val.iVal;

		SafeArrayGetElement(psa, &index, (void *) &val); ++index;
		list[i].about_menu_id = val.iVal;

		SafeArrayGetElement(psa, &index, (void *) &val); ++index;
		list[i].geom_token = val.iVal;

		SafeArrayGetElement(psa, &index, (void *) &val); ++index;
		list[i].plugin_version = val.iVal;

		SafeArrayGetElement(psa, &index, (void *) &val); ++index;
		list[i].has_options_dialog_box = val.iVal;

		SafeArrayGetElement(psa, &index, (void *) &val); ++index;
		list[i].has_about_dialog_box = val.iVal;

		SafeArrayGetElement(psa, &index, (void *) &val); ++index;
		list[i].show_file_selector_before_exporting = val.iVal;

		SafeArrayDestroy(psa);
	}

	return(list);
}

	void
OkinoCommonComSource___FreeListofExportersAndTheirAttributesObtainedFromCOMServer(Nd_Export_Converter_Info_List *list)
{
	short	i;

	if (list == NULL)
		return;

	for (i=0; i < list[0].num_exporters_in_list; ++i) {
		if (list[i].file_type_desc != NULL)
			free((char *) list[i].file_type_desc);
		if (list[i].file_extensions != NULL)
			free((char *) list[i].file_extensions);
		if (list[i].help_ref != NULL)
			free((char *) list[i].help_ref);
		if (list[i].guid != NULL)
			free((char *) list[i].guid);
		if (list[i].dll_filepath != NULL)
			free((char *) list[i].dll_filepath);
		if (list[i].plugin_directory != NULL)
			free((char *) list[i].plugin_directory);
		if (list[i].plugin_type_description != NULL)
			free((char *) list[i].plugin_type_description);
		if (list[i].plugin_descriptive_name != NULL)
			free((char *) list[i].plugin_descriptive_name);
		if (list[i].menu_description != NULL)
			free((char *) list[i].menu_description);
		if (list[i].fileopen_filter_spec != NULL)
			free((char *) list[i].fileopen_filter_spec);
	}

	free((char *) list);
}

// -------------->>>> Create Convenienet List of Known Importers <<<<------------

// This routine creates a convenient list of all known import converters and
// their associated attributes, as queried from the host COM server. After
// you are done with this list use the FreeListofImportersAndTheirAttributesObtainedFromCOMServer()
// routine to free it up. The number of import converters in the list is stored
// in the first element of the list, 'list[0].num_importers_in_list'.

	Nd_Import_Converter_Info_List *
OkinoCommonComSource___GetListofImportersAndTheirAttributesFromCOMServer()
{
	USES_CONVERSION;			// Since we are using OLE2T() macro
	Nd_Import_Converter_Info_List	*list;
	char		*str_ptr;
	short		i, num_converters;
	SAFEARRAY 	*psa;			// The parameters are passed in via a SAFEARRAY

	if (!pConvertIO)
		return(NULL);

	// Find out how many import converters there are
	try {
		HRESULT		hresult;
		short		com_error_result;

		// Find out how many import converters there are
loop:		hresult = pConvertIO->get_NumImportConverters(&num_converters);

		// If fatal, ask the user what to do: retry, restart or abort. If restart, try restarting the server,
		com_error_result = OkinoCommonComSource___Check_COM_HRESULT_Error_CODE(hresult, "Error while asking COM server for the number of import converters. COM server has either crashed or is waiting for a user response.");
		if (com_error_result == IDC_SERVERFAULT_RETRY_OPERATION)
			goto loop;
		else if (com_error_result == IDC_SERVERFAULT_CANCEL_OPERATION)
			return(NULL);
	} catch (_com_error &e) {
		OkinoCommonComSource___Display_COM_Error(e, "Error while asking COM server for the number of import converters. COM server has either crashed or is waiting for a user response.");
		return(NULL);
	}

	if (!num_converters)
		return(NULL);

	// Allocate memory for all of the importers
	list = (Nd_Import_Converter_Info_List *) malloc(num_converters * sizeof(Nd_Import_Converter_Info_List));
	memset((char *) list, 0, num_converters * sizeof(Nd_Import_Converter_Info_List));
	// The first element in the list will contain the number of importers in the entire list
	list[0].num_importers_in_list = num_converters;

	for (i=0; i < num_converters; ++i) {
		CComVariant val;
		long  index = 0;

		VARIANT out_options_variant;
		VariantInit(&out_options_variant);

		// Ask the COM server to go and ask the Windows UI for all
		// major parameters to an import converter and stuff in a 
		// 1D SAFEARRAY with type VT_VARIANT
		try {
			HRESULT		hresult;
			short		com_error_result;

			// Find out how many import converters there are
loop2:			hresult = pConvertIO->get_SpecificImportConverterInfo(i, 1, &out_options_variant);

			// If fatal, ask the user what to do: retry, restart or abort. If restart, try restarting the server,
			com_error_result = OkinoCommonComSource___Check_COM_HRESULT_Error_CODE(hresult, "Error while asking COM server for import converter info. COM server has either crashed or is waiting for a user response.");
			if (com_error_result == IDC_SERVERFAULT_RETRY_OPERATION)
				goto loop2;
			else if (com_error_result == IDC_SERVERFAULT_CANCEL_OPERATION)
				return(NULL);
		} catch (_com_error &e) {
			OkinoCommonComSource___Display_COM_Error(e, "Error while asking COM server for import converter info. COM server has either crashed or is waiting for a user response.");
			return(NULL);
		}

		psa = V_ARRAY(&out_options_variant);	// Get the safe array out of the variant
		if (!psa)
			continue;

		SafeArrayGetElement(psa, &index, (void *) &val); ++index;
		str_ptr = OLE2T(val.bstrVal);
		if (str_ptr)
			OkinoCommonComSource___Copy_String(&list[i].plugin_type_description, str_ptr);

		SafeArrayGetElement(psa, &index, (void *) &val); ++index;
		str_ptr = OLE2T(val.bstrVal);
		if (str_ptr)
			OkinoCommonComSource___Copy_String(&list[i].plugin_descriptive_name, str_ptr);

		SafeArrayGetElement(psa, &index, (void *) &val); ++index;
		str_ptr = OLE2T(val.bstrVal);
		if (str_ptr)
			OkinoCommonComSource___Copy_String(&list[i].menu_description, str_ptr);

		SafeArrayGetElement(psa, &index, (void *) &val); ++index;
		str_ptr = OLE2T(val.bstrVal);
		if (str_ptr)
			OkinoCommonComSource___Copy_String(&list[i].fileopen_filter_spec, str_ptr);

		SafeArrayGetElement(psa, &index, (void *) &val); ++index;
		str_ptr = OLE2T(val.bstrVal);
		if (str_ptr)
			OkinoCommonComSource___Copy_String(&list[i].file_extensions, str_ptr);

		SafeArrayGetElement(psa, &index, (void *) &val); ++index;
		str_ptr = OLE2T(val.bstrVal);
		if (str_ptr)
			OkinoCommonComSource___Copy_String(&list[i].guid, str_ptr);

		SafeArrayGetElement(psa, &index, (void *) &val); ++index;
		str_ptr = OLE2T(val.bstrVal);
		if (str_ptr)
			OkinoCommonComSource___Copy_String(&list[i].dll_filepath, str_ptr);

		SafeArrayGetElement(psa, &index, (void *) &val); ++index;
		str_ptr = OLE2T(val.bstrVal);
		if (str_ptr)
			OkinoCommonComSource___Copy_String(&list[i].plugin_directory, str_ptr);
		
		SafeArrayGetElement(psa, &index, (void *) &val); ++index;
		list[i].plugin_version = val.iVal;

		SafeArrayGetElement(psa, &index, (void *) &val); ++index;
		list[i].has_options_dialog_box = val.iVal;

		SafeArrayGetElement(psa, &index, (void *) &val); ++index;
		list[i].has_about_dialog_box = val.iVal;

		SafeArrayGetElement(psa, &index, (void *) &val); ++index;
		list[i].show_file_selector_before_importing = val.iVal;

		SafeArrayDestroy(psa);
	}

	return(list);
}

	void
OkinoCommonComSource___FreeListofImportersAndTheirAttributesObtainedFromCOMServer(Nd_Import_Converter_Info_List *list)
{
	short	i;

	if (list == NULL)
		return;

	for (i=0; i < list[0].num_importers_in_list; ++i) {
		if (list[i].file_extensions != NULL)
			free((char *) list[i].file_extensions);
		if (list[i].guid != NULL)
			free((char *) list[i].guid);
		if (list[i].dll_filepath != NULL)
			free((char *) list[i].dll_filepath);
		if (list[i].plugin_directory != NULL)
			free((char *) list[i].plugin_directory);
		if (list[i].plugin_type_description != NULL)
			free((char *) list[i].plugin_type_description);
		if (list[i].plugin_descriptive_name != NULL)
			free((char *) list[i].plugin_descriptive_name);
		if (list[i].menu_description != NULL)
			free((char *) list[i].menu_description);
		if (list[i].fileopen_filter_spec != NULL)
			free((char *) list[i].fileopen_filter_spec);
	}

	free((char *) list);
}

// -------->>> Import 3D Model to COM Server Cover Function  <<<------------

// This is a powerful cover function which handles the entire process of importing a 3D model into
// the stand-alone PolyTrans or NuGraf user interface software via the COM server API interface.
// Some of the tedious process handled by this code include:
//
//	1) If the user chose a "Auto detect" menu item or drop-down combo box option then this
//	   routine will ask the COM server to determine an appropriate import converter based on
//	   the file extension of the input filename,
//	2) Else, if the user provided a specific import filename with proper file format extension
//	   then this routine will call the COM server and ask if the filename + extension is
//	   a supported 3D file format and (possibly) if the input file is itself in the specified
//	   3D file format (this is because there are some 3D file formats with the same extension
//	   such as .3dm for Quickdraw-3D and Rhino's OpenNURBS format).
//	3) The import converter's options dialog box is optionally shown.
//	4) And then the 3D model is asked to be loaded into the stand-alone PolyTrans or NuGraf
//	   user interface software via the COM server API

// NOTE: Don't use this function to load in Okino .bdf files. Instead you
//       will have to use another dedicated COM interface method. See the
//	 source code in Test_Function_LoadBDFFile().

// Returns TRUE if the import process proceeded to completion, else FALSE if an error

// Once this function has been called then you should call OkinoCommonComSource___HandleCompleteFileExportProces()
// to immediately export the 3D database from the stand-alone PolyTrans or NuGraf user interface to
// a specific export file format.

	short
OkinoCommonComSource___HandleCompleteFileImportProcess(
	HWND	parent_window_handle,			// Window handle to the parent of any dialog boxes to be shown
	char	*input_filename_and_path,		// Import filename and path
	char	*Importer_GUID_char,			// Import converter's GUID string
	short	show_import_converter_option_dialog_box, // TRUE if the import converter's options dialog box is allowed to be shown.
	// These are the options which affect the import process
	short	reset_program_before_import,		// TRUE if main program should be reset before importing (should be set to TRUE so that the database is empty)
	short   resize_model_to_viewport_upon_import,	// TRUE if the model should be resized to the main camera's viewing area afer import
	short	add_default_lights_to_scene_if_none,	// TRUE if 2 lights should be added to the scene, after importing, if none are available
	short	ask_user_for_fileformat_if_unknown)	// If the importer plugin mechanism could not determine which import converter to use, then setting this variable to TRUE will cause a dialog box to be presented to the user so that they can select the proper import converter to use.
{
	_bstr_t geom_name = input_filename_and_path;
	_bstr_t Importer_GUID = Importer_GUID_char;
	_bstr_t dialog_box_name_bstr;
	HRESULT	hresult, status;	// S_OK if no error, S_FALSE if error

	// Reset the error/message window so that it is empty
	if (curr_Error_Msg_Listbox_hwnd)
		SendMessage(curr_Error_Msg_Listbox_hwnd, LB_RESETCONTENT, 0, 0L);

	// The last entry was chosen, "Auto convert", in which case we have to
	// ask the COM server which importer is best suited for the file chosen.
	if (Importer_GUID_char == NULL) {
		SAFEARRAY 	*in_options;		// Input parameters
		SAFEARRAY 	*out_options;		// Resulting output parameters
		CComVariant 	val;
		CComVariant 	str_var, short_var;
		long  		index = 0;
		BSTR		return_importer_guid;
		short 		return_header_verified = FALSE;
		short 		return_file_extension_verified = FALSE;
		short 		return_nugraf_file_specified = FALSE;

		// Initialize the bounds for the 1D array
		SAFEARRAYBOUND safeBound[1];

		// Set up the bounds of the 1D array
		safeBound[0].cElements = 6;	// 6 arguments
		safeBound[0].lLbound = 0;	// Lower bound is 0

		// The array type is VARIANT so that we can stuff any type into each element
		// Storage will accomodate a BSTR and a short
		in_options = SafeArrayCreate(VT_VARIANT, 1, &safeBound[0]);
		if (in_options == NULL)
			return(FALSE);		// Failed

		index = 0;

		// The full path + filename of the input filename, including file extension
		str_var = input_filename_and_path;
		SafeArrayPutElement(in_options, &index, &str_var); index++;

		// If a dialog box has to be displayed, then this is the parent window handle (HWND cast to long. This is supposed to be ok.)
		short_var= (long) parent_window_handle;
		SafeArrayPutElement(in_options, &index, &short_var);index++;

		// Set to TRUE to cause an importer to be executed in order to verify it's header
		short_var= TRUE;
		SafeArrayPutElement(in_options, &index, &short_var);index++;

		// If the file header check failed then go assume a valid match if only the file extension matches an existing importer. If FALSE, then we can't assume the match has been made if only the extension matches
		short_var= TRUE;
		SafeArrayPutElement(in_options, &index, &short_var);index++;

		// Set to TRUE to cause a dialog box to be displayed as each import converter is loaded up during a long header detection phase
		short_var= TRUE;
		SafeArrayPutElement(in_options, &index, &short_var);index++;

		// Set to TRUE if the user should be asked for the file format via a list box if it could not be determined
		short_var= ask_user_for_fileformat_if_unknown;
		SafeArrayPutElement(in_options, &index, &short_var);index++;

		VARIANT in_options_variant;
		VariantInit(&in_options_variant);
		in_options_variant.vt = VT_VARIANT | VT_ARRAY;
		V_ARRAY(&in_options_variant) = in_options;

		VARIANT out_options_variant;
		VariantInit(&out_options_variant);

		// The last entry was chosen, "Auto convert", in which case we have to
		// ask the COM server which importer is best suited for the file chosen.
		try {
			short		com_error_result;

loop:			status = pConvertIO->Autodetect3DFileFormatImportConverter(
				1,				// Interface version for ingoing and outgoing parameters
				&in_options_variant,		// Input parameters
				// Output variables
				&out_options_variant);		// Return arguments

			// If fatal, ask the user what to do: retry, restart or abort. If restart, try restarting the server,
			com_error_result = OkinoCommonComSource___Check_COM_HRESULT_Error_CODE(status, "Error while asking COM server to auto-detect a file format. COM server has either crashed or is waiting for a user response.");
			if (com_error_result == IDC_SERVERFAULT_RETRY_OPERATION)
				goto loop;
			else if (com_error_result == IDC_SERVERFAULT_CANCEL_OPERATION) {
				SafeArrayDestroy(in_options);
				return(FALSE);
			}
		} catch (_com_error &e) {
			OkinoCommonComSource___Display_COM_Error(e, "Error while asking COM server to auto-detect a file format. COM server has either crashed or is waiting for a user response.");
			SafeArrayDestroy(in_options);
			return(FALSE);
		}

		SafeArrayDestroy(in_options);	// Clean up after we are done with the input parameter safe array

		out_options = V_ARRAY(&out_options_variant);	// Get the safe array out of the variant
		if (out_options) {
			index = 0;
			// Short return arguments

			// The file header was verified properly
			SafeArrayGetElement(out_options, &index, (void *) &val); ++index;
			return_header_verified = val.iVal;

			// The file extension was verified properly
			SafeArrayGetElement(out_options, &index, (void *) &val); ++index;
			return_file_extension_verified = val.iVal;

			// The file was verified as a NuGraf .bdf file
			SafeArrayGetElement(out_options, &index, (void *) &val); ++index;
			return_nugraf_file_specified = val.iVal;

			// String return arguments

			// The qualified importer guid is returned here. Enter with it pointing to NULL.
			SafeArrayGetElement(out_options, &index, (void *) &val); ++index;
			CComBSTR bstr = val.bstrVal;
			return_importer_guid = (BSTR) bstr.Detach();

			SafeArrayDestroy(out_options);
		}

		if (status != S_OK || (!return_file_extension_verified && !return_header_verified)) {
			MessageBox(parent_window_handle, "The input file format could not be validated.\n\nPlease select an explicit file format from the drop-down combo box or from the menus which corresponds to the file format chosen and try again.", "Error", MB_OK | MB_APPLMODAL);
			return(FALSE);
		} else
			Importer_GUID = return_importer_guid;
	} else {
		// Else, the user chose an explicit import converter in the
		// import combo box or from the menus

		SAFEARRAY	*out_options;		// Resulting output parameters
		CComVariant 	val, str_var, short_var;
		long  index;
		short return_header_verified = FALSE;			// The file header was verified properly
		short return_file_extension_verified = FALSE;		// The file extension was verified properly
		short return_nugraf_file_specified = FALSE;		// The file was verified as a NuGraf .bdf file
		short return_multiple_importers_match_file_xtn = FALSE; // This is always set TRUE/FALSE. It is set TRUE if multiple import converters match the file extension of the input filename

		VARIANT out_options_variant;
		VariantInit(&out_options_variant);

		// And while we are at it, let's ask the COM server to verify if the
		// chosen import convert can actually handle the selected file.
		try {
			short		com_error_result;

loop2:			status = pConvertIO->VerifyIfCorrectImportConverterForSpecifiedFilename(
				// Input variables
				geom_name,		// The full path + filename of the input filename, including file extension
				Importer_GUID,		// This is the importer plugin selected by the user, or guessed at by some other routine
				// Output variables
				1,			// Interface version for outgoing parameters
				&out_options_variant);	// Return arguments

			// If fatal, ask the user what to do: retry, restart or abort. If restart, try restarting the server,
			com_error_result = OkinoCommonComSource___Check_COM_HRESULT_Error_CODE(status, "Error while asking COM server to verify an import file format. COM server has either crashed or is waiting for a user response.");
			if (com_error_result == IDC_SERVERFAULT_RETRY_OPERATION)
				goto loop2;
			else if (com_error_result == IDC_SERVERFAULT_CANCEL_OPERATION)
				return(FALSE);
		} catch (_com_error &e) {
			OkinoCommonComSource___Display_COM_Error(e, "Error while asking COM server to verify an import file format. COM server has either crashed or is waiting for a user response.");
			return(FALSE);
		}

		out_options = V_ARRAY(&out_options_variant);	// Get the safe array out of the variant
		if (out_options) {
			index = 0;
			// Short return arguments

			// The file header was verified properly
			SafeArrayGetElement(out_options, &index, (void *) &val); ++index;
			return_header_verified = val.iVal;
			// The file extension was verified properly
			SafeArrayGetElement(out_options, &index, (void *) &val); ++index;
			return_file_extension_verified = val.iVal;
			// The file was verified as a NuGraf .bdf file
			SafeArrayGetElement(out_options, &index, (void *) &val); ++index;
			return_nugraf_file_specified = val.iVal;
			// This is always set TRUE/FALSE. It is set TRUE if multiple import converters match the file extension of the input filename
			SafeArrayGetElement(out_options, &index, (void *) &val); ++index;
			return_multiple_importers_match_file_xtn = val.iVal;

			SafeArrayDestroy(out_options);
		}

		if (status != S_OK || (!return_header_verified && !return_file_extension_verified)) {
			MessageBox(parent_window_handle, "The import converter you have selected from the drop-down box or menus cannot handle the input file selected.\n\nPlease select another explicit file format from the drop-down combo box or menus which corresponds to the file format chosen and try again.", "Error", MB_OK | MB_APPLMODAL);
			return(FALSE);
		}
	}

	// Optionally, go show the import converter options dialog box. The box will be displayed from the COM server, but the dialog boxes's parent will be this test client's main window
	if (show_import_converter_option_dialog_box) {
		dialog_box_name_bstr = "ImportConverterOptionsDialogBox";
		try {

			short	com_error_result;

//
// RCL. Added 'geom_name' to fourth argument of this function call, July 8 2004
// and modified the host COM server to accept the geometry import filename as the
// fourth argument (v4.1.8). The geometry filename is needed by the DXF/DWG importer
// for the "auto parse" command and also for the Inventor/Granite/SolidEdge/SolidWorks
// import options dialog box so that that the user is not presented wit the panel to
// select the source file (this won't affect the conversion process with any earlier
// version, but it may confuse the user if they select a new filename on the wizard 
// panel yet find that the COM client's filename (geom_name) overrides their selection
// (which is still correct).
//
loop3:			hresult = pNuGrafIO->ShowInternalDialogBox(dialog_box_name_bstr, (long) parent_window_handle, Importer_GUID, geom_name, 0, 0);
			// E_FAIL means that S_FALSE is being returned from the dialog box
			if (hresult == E_FAIL) {
				hresult = S_FALSE;
// Added July 8 2004
				return FALSE;
			}

			// If fatal, ask the user what to do: retry, restart or abort. If restart, try restarting the server,
			com_error_result = OkinoCommonComSource___Check_COM_HRESULT_Error_CODE(hresult, "Error while asking COM server to show an import converter options dialog box. COM server has either crashed or is waiting for a user response.");
			if (com_error_result == IDC_SERVERFAULT_RETRY_OPERATION)
				goto loop3;
			else if (com_error_result == IDC_SERVERFAULT_CANCEL_OPERATION)
				return(FALSE);
		} catch (_com_error &e) {
			OkinoCommonComSource___Display_COM_Error(e, "Error while asking COM server to show an import converter options dialog box. COM server has either crashed or is waiting for a user response.");
			return(FALSE);
		}
	}

	// Load up the model into the PolyTrans or NuGraf user interface via the COM server API
	{
		SAFEARRAY	*in_options;		// Input parameters
		CComVariant 	val, str_var, short_var;
		long  		index;

		// Initialize the bounds for the 1D array
		SAFEARRAYBOUND safeBound[1];

		// Set upthe bounds of the 1D array
		safeBound[0].cElements = 4;	// 4 arguments
		safeBound[0].lLbound = 0;	// Lower bound is 0

		// The array type is VARIANT so that we can stuff any type into each element
		// Storage will accomodate a BSTR and a short
		in_options = SafeArrayCreate(VT_VARIANT, 1, &safeBound[0]);
		if (in_options == NULL)
			return(FALSE);		// Failed

		index = 0;

		// Short input arguments

		// reset_program_before_import
		short_var= TRUE;
		SafeArrayPutElement(in_options, &index, &short_var);index++;
		// resize_model_to_viewport_upon_import
		short_var= resize_model_to_viewport_upon_import;
		SafeArrayPutElement(in_options, &index, &short_var);index++;
		// add_default_lights_to_scene_if_none
		short_var= add_default_lights_to_scene_if_none;
		SafeArrayPutElement(in_options, &index, &short_var);index++;
		// ask_user_for_fileformat_if_unknown
		short_var= ask_user_for_fileformat_if_unknown;
		SafeArrayPutElement(in_options, &index, &short_var);index++;

		VARIANT in_options_variant;
		VariantInit(&in_options_variant);
		in_options_variant.vt = VT_VARIANT | VT_ARRAY;
		V_ARRAY(&in_options_variant) = in_options;

		try {
			short	com_error_result;

loop3b:                 status = pConvertIO->Import3DModel(
				geom_name,
				Importer_GUID,
				1,		// Interface version
				&in_options_variant);

			// If fatal, ask the user what to do: retry, restart or abort. If restart, try restarting the server,
			com_error_result = OkinoCommonComSource___Check_COM_HRESULT_Error_CODE(status, "Error while asking COM server to import a 3d model. COM server has either crashed or is waiting for a user response.");
			if (com_error_result == IDC_SERVERFAULT_RETRY_OPERATION)
				goto loop3b;
			else if (com_error_result == IDC_SERVERFAULT_CANCEL_OPERATION) {
				SafeArrayDestroy(in_options);
				return(FALSE);
			}
		} catch (_com_error &e) {
			OkinoCommonComSource___Display_COM_Error(e, "Error while asking COM server to import a 3D model. COM server has either crashed or is waiting for a user response.");
			SafeArrayDestroy(in_options);
			return(FALSE);
		}

		SafeArrayDestroy(in_options);	// Clean up after we are done with the input parameter safe array
	}

	return (TRUE);
}

// ---------->>> Export 3D Model to COM Server Cover Function  <<<------------

// This is a cover function which handles the process of requesting the the stand-alone PolyTrans
// or NuGraf user interface software to export its scene database into a specific 3D file format.
// This routine optionally also allows the export converter's options dialog box to be shown.

// NOTE: Don't use this function to save out an Okino .bdf files. Instead you
//       will have to use another dedicated COM interface method. See the
//	 source code in Test_Function_SaveOutBDFFile().

// Returns TRUE if the export process proceeded to completion, else FALSE if an error

	short
OkinoCommonComSource___HandleCompleteFileExportProcess(
	HWND	parent_window_handle,			// Window handle to the parent of any dialog boxes to be shown
	char	*export_filename_and_path,		// Export filename and path
	char	*Exporter_GUID_char,			// Export converter's GUID string
	short	show_export_converter_option_dialog_box, // TRUE if the export converter's options dialog box is allowed to be shown.
	// These are the options which affect the export process
	short	create_backup_file)			// TRUE if the filename already exists on disk, in which case it will first be renamed to .bak
{
	CComVariant 	short_var;
	_bstr_t 	Exporter_GUID = Exporter_GUID_char;
	_bstr_t 	geom_name = export_filename_and_path;
	SAFEARRAY 	*in_options2;		// Input parameters
	HRESULT		hresult, status;
	long		index;

	// Optionally, go show the export converter options dialog box. The box will be displayed from the COM server, but the dialog boxes's parent will be this test client's main window
	if (show_export_converter_option_dialog_box) {
		_bstr_t dialog_box_name_bstr;

		dialog_box_name_bstr = "ExportConverterOptionsDialogBox";
		try {
			short		com_error_result;

//
// RCL. Added 'geom_name' to fourth argument of this function call, July 8 2004
// and modified the host COM server to accept the exometry import filename as the
// fourth argument (v4.1.8). Some internal export converters may need to know the
// filename in order to modify the contents of their options dialog box (such as 
// trueSpace .cob vs. scn export)
//

loop4:			hresult = pNuGrafIO->ShowInternalDialogBox(dialog_box_name_bstr, (long) parent_window_handle, Exporter_GUID, geom_name, 0, 0);
			// E_FAIL means that S_FALSE is being returned from the dialog box
			if (hresult == E_FAIL) {
				hresult = S_FALSE;

// Added July 8 2004
				return FALSE;
			}

			// If fatal, ask the user what to do: retry, restart or abort. If restart, try restarting the server,
			com_error_result = OkinoCommonComSource___Check_COM_HRESULT_Error_CODE(hresult, "Error while asking COM server to show an export converter's options dialog box. COM server has either crashed or is waiting for a user response.");
			if (com_error_result == IDC_SERVERFAULT_RETRY_OPERATION)
				goto loop4;
			else if (com_error_result == IDC_SERVERFAULT_CANCEL_OPERATION) {
				return(FALSE);
			}
		} catch (_com_error &e) {
			OkinoCommonComSource___Display_COM_Error(e, "Error while asking COM server to show an export converter's options dialog box. COM server has either crashed or is waiting for a user response.");
			return(FALSE);
		}
	}

	// Initialize the bounds for the 1D array
	SAFEARRAYBOUND safeBound[1];

	// Set up the bounds of the 1D array
	safeBound[0].cElements = 1;	// 1 argument
	safeBound[0].lLbound = 0;	// Lower bound is 0

	// The array type is VARIANT so that we can stuff any type into each element
	// Storage will accomodate a BSTR and a short
	in_options2 = SafeArrayCreate(VT_VARIANT, 1, &safeBound[0]);
	if (in_options2 == NULL)
		return(FALSE);		// Failed

	index = 0;

	// Short input arguments

	// create_backup_file
	short_var = create_backup_file;
	SafeArrayPutElement(in_options2, &index, &short_var); index++;

	VARIANT in_options_variant;
	VariantInit(&in_options_variant);
	in_options_variant.vt = VT_VARIANT | VT_ARRAY;
	V_ARRAY(&in_options_variant) = in_options2;

	try {
		short		com_error_result;

loop5:      	status = pConvertIO->Export3DModel(
			geom_name,
			Exporter_GUID,
			1,		// Interface version
			&in_options_variant);

		if (status == E_FAIL)
			// Export process aborted, or was in error
			status = S_FALSE;

		// If fatal, ask the user what to do: retry, restart or abort. If restart, try restarting the server,
		com_error_result = OkinoCommonComSource___Check_COM_HRESULT_Error_CODE(status, "Error while asking COM server to export a 3d model. COM server has either crashed or is waiting for a user response.");
		if (com_error_result == IDC_SERVERFAULT_RETRY_OPERATION)
			goto loop5;
		else if (com_error_result == IDC_SERVERFAULT_CANCEL_OPERATION) {
			SafeArrayDestroy(in_options2);
			return(FALSE);
		}
	} catch (_com_error &e) {
		OkinoCommonComSource___Display_COM_Error(e, "Error while asking COM server to export a 3D model. COM server has either crashed or is waiting for a user response.");
		SafeArrayDestroy(in_options2);
		return(FALSE);
	}

	SafeArrayDestroy(in_options2);	// Clean up after we are done with the input parameter safe array

	return(TRUE);
}

// ----------->>> Okino .bdf File Load and Save COM Functions <<<-----------

// Load an Okino .bdf file into the current database, invoked via COM interface
// Returns TRUE if file was loaded ok, else FALSE if an error.

	short
OkinoCommonComSource___LoadOkinoBDFFile(char *filename_and_path, short confirm_reset)
{
	HRESULT	hresult;
	_bstr_t bdf_name = filename_and_path;

	try {
		short		com_error_result;

loop:		hresult = pNuGrafIO->LoadBDFFile(bdf_name, confirm_reset);
		if (hresult == E_FAIL)
			return(FALSE);	// Error
		// If fatal, ask the user what to do: retry, restart or abort. If restart, try restarting the server,
		com_error_result = OkinoCommonComSource___Check_COM_HRESULT_Error_CODE(hresult, "Error while asking COM server to load a .bdf file. COM server has either crashed or is waiting for a user response.");
		if (com_error_result == IDC_SERVERFAULT_RETRY_OPERATION)
			goto loop;
		else if (com_error_result == IDC_SERVERFAULT_CANCEL_OPERATION)
			return(FALSE);
	} catch (_com_error &e) {
		OkinoCommonComSource___Display_COM_Error(e, "Error while asking COM server to load a .bdf file. COM server has either crashed or is waiting for a user response.");
		return(FALSE);
	}

	return(TRUE);
}

// Save the current database out to an Okino .bdf file, invoked via COM interface
// Returns TRUE if file was saved ok, else FALSE if an error.

	short
OkinoCommonComSource___SaveOkinoBDFFile(char *filename_and_path)
{
	short	choose_new_name = FALSE, save_selective_components = FALSE;
	HRESULT	hresult;

	_bstr_t bdf_name = filename_and_path;
	try {
		short		com_error_result;

loop:		hresult = pNuGrafIO->SaveBDFFile(bdf_name, choose_new_name, save_selective_components);
		if (hresult == E_FAIL)
			return(FALSE);	// Error
		// If fatal, ask the user what to do: retry, restart or abort. If restart, try restarting the server,
		com_error_result = OkinoCommonComSource___Check_COM_HRESULT_Error_CODE(hresult, "Error while asking COM server to save a .bdf file. COM server has either crashed or is waiting for a user response.");
		if (com_error_result == IDC_SERVERFAULT_RETRY_OPERATION)
			goto loop;
		else if (com_error_result == IDC_SERVERFAULT_CANCEL_OPERATION)
			return(FALSE);
	} catch (_com_error &e) {
		OkinoCommonComSource___Display_COM_Error(e, "Error while asking COM server to save a .bdf file. COM server has either crashed or is waiting for a user response.");
		return(FALSE);
	}
	return(TRUE);
}

// ---------------->>> Show A Dialog Box from COM Server <<<-----------------

// This shows a dialog box from the main PolyTrans/NuGraf host program via COM.

	void
OkinoCommonComSource___ShowInternalDialogBox(HWND hDlg, char *dialog_box_name)
{
	_bstr_t dialog_box_name_bstr = dialog_box_name;

	try {
		HRESULT		hresult;
		short		com_error_result;

		// Find out how many import converters there are
loop:		hresult = pNuGrafIO->ShowInternalDialogBox(dialog_box_name_bstr, (long) hDlg, NULL, NULL, 0, 0);
		// E_FAIL means that S_FALSE is being returned from the dialog box
		if (hresult == E_FAIL)
			hresult = S_FALSE;

		// If fatal, ask the user what to do: retry, restart or abort. If restart, try restarting the server,
		com_error_result = OkinoCommonComSource___Check_COM_HRESULT_Error_CODE(hresult, "Error while asking COM server to show an internal dialog box. COM server has either crashed or is waiting for a user response.");
		if (com_error_result == IDC_SERVERFAULT_RETRY_OPERATION)
			goto loop;
		else if (com_error_result == IDC_SERVERFAULT_CANCEL_OPERATION)
			return;
	} catch (_com_error &e) {
		OkinoCommonComSource___Display_COM_Error(e, "Error while asking COM server to show an internal dialog box. COM server has either crashed or is waiting for a user response.");
		return;
	}
}

// -------------------------------------------------------------------------

// This utility function determines the importer's GUID for the input
// filename specified. It asks the COM server to determine which importer
// to use for this filename, based on its file extension and an auto-detect
// of the file itself. If the COM server cannot determine the format then 
// it will ask the user to select a format using a locally based dialog box.

// NOTE: If the file format is in .bdf then don't call this routine since
// .bdf file format does not have a GUID. 

// Return FALSE if GUID could not be determined, else TRUE if it is ok.

	short
OkinoCommonComSource___DetermineImporterGUIDBasedOnFilenameOnly(HWND hDlg, char *input_filename,
	BSTR *return_importer_guid)
{
 	HRESULT		status;	// S_OK if no error, S_FALSE if error
	short 		return_header_verified = FALSE;
	short 		return_file_extension_verified = FALSE;
	short 		return_nugraf_file_specified = FALSE;
	SAFEARRAY 	*in_options;		// Input parameters
	SAFEARRAY 	*out_options;		// Resulting output parameters
	CComVariant 	val;
	CComVariant 	str_var, short_var;
	long  		index = 0;

	// Initialize the bounds for the 1D array
	SAFEARRAYBOUND safeBound[1]; 
	
	// Set up the bounds of the 1D array
	safeBound[0].cElements = 6;	// 6 arguments
	safeBound[0].lLbound = 0;	// Lower bound is 0

	// The array type is VARIANT so that we can stuff any type into each element
	// Storage will accomodate a BSTR and a short
	in_options = SafeArrayCreate(VT_VARIANT, 1, &safeBound[0]);
	if (in_options == NULL)
		return(FALSE);		// Failed

	index = 0;

	// The full path + filename of the input filename, including file extension
	str_var = input_filename;
	SafeArrayPutElement(in_options, &index, &str_var); index++;

	// If a dialog box has to be displayed, then this is the parent window handle (HWND cast to long. This is supposed to be ok.)
	short_var = (long) hDlg;
	SafeArrayPutElement(in_options, &index, &short_var);index++;

	// Set to TRUE to cause an importer to be executed in order to verify it's header
	short_var = TRUE;
	SafeArrayPutElement(in_options, &index, &short_var);index++;

	// If the file header check failed then go assume a valid match if only the file extension matches an existing importer. If FALSE, then we can't assume the match has been made if only the extension matches
	short_var = TRUE;
	SafeArrayPutElement(in_options, &index, &short_var);index++;

	// Set to TRUE to cause a dialog box to be displayed as each import converter is loaded up during a long header detection phase
	short_var = TRUE;
	SafeArrayPutElement(in_options, &index, &short_var);index++;

	// Set to TRUE if the user should be asked for the file format via a list box if it could not be determined
	short_var = TRUE;
	SafeArrayPutElement(in_options, &index, &short_var);index++;

	VARIANT in_options_variant;
	VariantInit(&in_options_variant);
	in_options_variant.vt = VT_VARIANT | VT_ARRAY;
	V_ARRAY(&in_options_variant) = in_options;

	VARIANT out_options_variant;
	VariantInit(&out_options_variant);

	// Ask the COM server which importer is best suited for the input file chosen.
	try {
		short		com_error_result;

loop:		status = pConvertIO->Autodetect3DFileFormatImportConverter(
			1,				// Interface version for ingoing and outgoing parameters
			&in_options_variant,		// Input parameters
			// Output variables
			&out_options_variant);		// Return arguments

		// If fatal, ask the user what to do: retry, restart or abort. If restart, try restarting the server,
		com_error_result = OkinoCommonComSource___Check_COM_HRESULT_Error_CODE(status, "Error while asking COM server to auto-detect a file format. COM server has either crashed or is waiting for a user response.");
		if (com_error_result == IDC_SERVERFAULT_RETRY_OPERATION)
			goto loop;
		else if (com_error_result == IDC_SERVERFAULT_CANCEL_OPERATION) {
			SafeArrayDestroy(in_options);
			return(FALSE);
		}
	} catch (_com_error &e) {
		OkinoCommonComSource___Display_COM_Error(e, "Error while asking COM server to auto-detect a file format. COM server has either crashed or is waiting for a user response.");
		SafeArrayDestroy(in_options);
		return(FALSE);
	}

	SafeArrayDestroy(in_options);	// Clean up after we are done with the input parameter safe array

	out_options = V_ARRAY(&out_options_variant);	// Get the safe array out of the variant
	if (out_options) {
		index = 0;
		// Short return arguments

		// The file header was verified properly
		SafeArrayGetElement(out_options, &index, (void *) &val); ++index;
		return_header_verified = val.iVal;

		// The file extension was verified properly
		SafeArrayGetElement(out_options, &index, (void *) &val); ++index;
		return_file_extension_verified = val.iVal;

		// The file was verified as a NuGraf .bdf file
		SafeArrayGetElement(out_options, &index, (void *) &val); ++index;
		return_nugraf_file_specified = val.iVal;

		// String return arguments

		// The qualified importer guid is returned here. Enter with it pointing to NULL.
		SafeArrayGetElement(out_options, &index, (void *) &val); ++index;
		CComBSTR bstr = val.bstrVal;
		*return_importer_guid = (BSTR) bstr.Detach();

		SafeArrayDestroy(out_options);
	}

	// If the COM server cannot determine the appropriate import converter, then tell the user that we can't continue
	if (status != S_OK || (!return_file_extension_verified && !return_header_verified)) {
		MessageBox(hDlg, "The input file format could not be determined by the COM server.\n\nThe input file cannot be converted.", "Error", MB_OK | MB_APPLMODAL);
		return(FALSE);
	} else	
		return(TRUE);
}

// ------------->>> COM Error Recovery Cover Functions  <<<------------------

// This should be called from a COM "catch()" function.

	void
OkinoCommonComSource___Display_COM_Error(_com_error &e, char *com_func_name)
{
	char	err_msg[512];

	if (e.ErrorInfo())
		sprintf(err_msg, "Error: %s. %s. %s", com_func_name, e.ErrorMessage(), e.Description());
	else
		sprintf(err_msg, "Error: %s. %s.", com_func_name, e.ErrorMessage());

	MessageBox(parent_hwnd, err_msg, "Fatal COM Error", MB_OK | MB_APPLMODAL);
}

// Severe COM errors are returned via the HRESULT parameter. Check for severe problems here.
// This should be called after a HRESULT, of any value, is returned from a COM function.

// Returns:
// IDC_SERVERFAULT_RETRY_OPERATION
//	- Retry the operation again (the user could have reset the server during this call)
// IDC_SERVERFAULT_CANCEL_OPERATION
//	- User wants to cancel the operation
// IDOK
//	- Everything went ok

	short
OkinoCommonComSource___Check_COM_HRESULT_Error_CODE(HRESULT hr, char *com_func_name)
{
	LPSTR 	MessageBuffer;
	DWORD 	dwBufferLength;
	short	status;
	char	err_msg[512];

	if (FAILED(hr)) {
		// We've got a severe error

		DWORD dwFormatFlags = FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_SYSTEM ;

		status = IDC_SERVERFAULT_CANCEL_OPERATION;

		if (dwBufferLength = FormatMessageA(
			dwFormatFlags,
			NULL, 		// module to get message from (NULL == system)
			hr,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // default language
			(LPSTR) &MessageBuffer, 0, NULL)) {
				sprintf(err_msg, "%s\n\nSystem level error code (%lx):\n%s", com_func_name, hr, MessageBuffer);
				// MessageBox(parent_hwnd, err_msg, "Error Accessing COM Server", MB_OK | MB_APPLMODAL);
				status = DialogBoxParam(gInstance, "ServerProblemDialogBox", parent_hwnd, (DLGPROC) OkinoCommonComSource___ServerFaultDlgProc, (LPARAM) err_msg);
				LocalFree(MessageBuffer);
		}
		return(status);
	} else
		return(IDOK);
}

// This is the dialog box handler for the geometry import status dialog

	static BOOL WINAPI
OkinoCommonComSource___ServerFaultDlgProc(HWND hDlg, WORD wMsg, WORD wParam, LONG lParam)
{
	lParam = lParam;

	switch (wMsg) {
		case WM_INITDIALOG:
			/* Center the dialog */
			OkinoCommonComSource___Center_Window(GetParent(hDlg), hDlg);

			// Set the error text
			if (lParam)
				SetDlgItemText(hDlg, IDC_SERVERFAULT_ERROR_MSG, (char *) lParam);
			else
				SetDlgItemText(hDlg, IDC_SERVERFAULT_ERROR_MSG, "");
			break;
		case WM_COMMAND:
			switch (wParam) {
				case IDC_SERVERFAULT_RETRY_OPERATION:
					EndDialog(hDlg, IDC_SERVERFAULT_RETRY_OPERATION);
					return(TRUE);
				case IDC_SERVERFAULT_RESTART_SERVER:
					if (!OkinoCommonComSource___RestartStandAloneNuGraforPolyTrans(hDlg))
						// Oops, restart of server failed
						EndDialog(hDlg, IDC_SERVERFAULT_CANCEL_OPERATION);
					else
						// Else, restart of the server was successful, so go and retry the operation again.
						EndDialog(hDlg, IDC_SERVERFAULT_RETRY_OPERATION);
					return(TRUE);
				case IDC_SERVERFAULT_CANCEL_OPERATION:
					EndDialog(hDlg, IDC_SERVERFAULT_CANCEL_OPERATION);
					return(TRUE);
				case IDC_SERVERFAULT_HELP:
					// DisplayHelpTopic(hDlg, "server fault");
					break;
			}
			break;
		default:
			break;
	}
	return(FALSE);
}

// Cause nugraf32.exe or pt32.exe to be aborted then restart their COM interfaces and the event sink.
// Returns FALSE if an error.

	short
OkinoCommonComSource___RestartStandAloneNuGraforPolyTrans(HWND hWnd)
{
	HWND		FirsthWnd;
	HANDLE		hProcess;
	DWORD		process_ID, kill_result;
	short		running_polytrans = FALSE;

	kill_result = 0;	// Failed

	if ((FirsthWnd = FindWindow("PT_MSC_Class1", NULL)) == NULL) {
		if ((FirsthWnd = FindWindow("NG_MSC_Class1", NULL)) == NULL) {
			// Neither PolyTrans or NuGraf are running 
			goto skip;
		}
	} else
		running_polytrans = TRUE;

	// Get the process ID that created the main window
	GetWindowThreadProcessId(FirsthWnd, &process_ID);
	if (!process_ID) {
		MessageBox(hWnd, "Could not get the process ID of nugraf32.exe or pt32.exe.", "Process Termination Failure", MB_OK | MB_APPLMODAL);
	}
                                
	// Open up the process object
	hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, process_ID);
	if (hProcess) {
		kill_result = TerminateProcess(hProcess, 0);
		CloseHandle(hProcess);
		if (!kill_result) {
			if (running_polytrans)
                                MessageBox(hWnd, "Could not terminate the process named 'pt32.exe'.\n\nPlease terminate this process yourself using Task Manager, then try again.", "Process Termination Failure", MB_OK | MB_APPLMODAL);
			else
                                MessageBox(hWnd, "Could not terminate the process named 'nugraf32.exe'.\n\nPlease terminate this process yourself using Task Manager, then try again.", "Process Termination Failure", MB_OK | MB_APPLMODAL);
			return(FALSE);
		}
	}

skip:	// Stop our local event sinks
	OkinoCommonComSource___StopEventSink();

	// Start up the NuGraf and PolyTrans class interfaces again, and the event sink
	if (!OkinoCommonComSource___StartComClassInterfacesAndEventSink())
		return(FALSE);
	else
		return(TRUE);
}

// ------------->>> Microsoft Windows Utility Functions  <<<------------------

// Enable or disable a dialog box control

	void 
OkinoCommonComSource___Set_Control_Enable(HWND hwndDlg, WORD id, BOOL bState)
{
	HWND    hwndDI;

	hwndDI = GetDlgItem(hwndDlg, id);
	EnableWindow(hwndDI, bState);
}

/* Send a Nd_Int value to an EDIT field in a dialog form. */

	void
OkinoCommonComSource___NdInt_To_Dialog(HWND hDlg, WORD Control_ID, long val)
{
	char	temp_buf[128];

	updating_edit_control = TRUE;
	sprintf(temp_buf, "%ld", val);
	SetDlgItemText(hDlg, Control_ID, temp_buf);
	updating_edit_control = FALSE;
}
/* Retrieve a long integer value from an EDIT field of a dialog box */
/* and store in a Nd_Int value. */

	void
OkinoCommonComSource___Dialog_To_NdInt(HWND hDlg, WORD Control_ID, long *val)
{
	char	temp_buf[30];

	GetDlgItemText(hDlg, Control_ID, temp_buf, sizeof(temp_buf));
	*val = (long) atoi(temp_buf);
}

// Returns the extent, in pixels, of a string that will be or is
// in a listbox.  The hDC of the listbox is used and an extra
// average character width is added to the extent to insure that
// a horizontal scrolling listbox that is based on this extent
// will scroll such that the end of the string is visible.

	short 
OkinoCommonComSource___WGetListboxStringExtent(HWND hList, char *psz)
{
	TEXTMETRIC  tm;
	HDC         hDC;
	HFONT       hFont;
	WORD        wExtent;
	SIZE	    SizeRect;

	/* Make sure we are using the correct font. */
	hDC = GetDC(hList);
	hFont = (HFONT) SendMessage(hList, WM_GETFONT, 0, 0L);

	if (hFont != NULL)
		SelectObject(hDC, hFont);

	GetTextMetrics(hDC, &tm);

	/* Add one average text width to insure that we see the end of the */
	/* string when scrolled horizontally. */
	GetTextExtentPoint(hDC, psz, strlen(psz), &SizeRect);
	wExtent = (WORD) (SizeRect.cx + tm.tmAveCharWidth);
	ReleaseDC(hList, hDC);

	return(wExtent);
}

/* This function simulates a meter with a '%age' text indicator in the middle */
/* hDlg is the parent window handle, Control_ID is the ID to the rectangle */

	void
OkinoCommonComSource___SimulateAMeter(HWND hDlg, short Control_ID, unsigned long curr_value, 
	unsigned long total)
{
	RECT	rcClient, rcPrcnt;
	HWND	hwnd_control;
	HDC	hdc;
	SIZE	size;
	char	percentage[10];

	if (curr_value > total)
		curr_value = total;
	if (total < curr_value)
		total = curr_value;

	sprintf(percentage, "%d%%", (short) ((100.0 * curr_value) / (float) total));

	hwnd_control = GetDlgItem(hDlg, Control_ID);
	hdc = GetDC(hwnd_control);

	/* Set-up default foreground and background text colors. */
	SetBkColor(hdc, GetSysColor(COLOR_WINDOW));
	SetTextColor(hdc, GetSysColor(COLOR_WINDOWTEXT));

	SetTextAlign(hdc, TA_CENTER | TA_TOP);

	/* Invert the foreground and background colors. */
	SetBkColor(hdc, SetTextColor(hdc, GetBkColor(hdc)));

	/* Set rectangle coordinates to include only left percentage of the window. */
	GetClientRect(hwnd_control, &rcClient);
	SetRect(&rcPrcnt, 0, 0, (short) ((rcClient.right * (float) curr_value) / (float) total), rcClient.bottom);

	/* Output the percentage value in the window. */
	/* Function also paints left part of window. */
	GetTextExtentPoint(hdc, "X", 1, (LPSIZE) &size);
	ExtTextOut(hdc, rcClient.right / 2, (rcClient.bottom - size.cy) / 2,
		ETO_OPAQUE | ETO_CLIPPED, &rcPrcnt,
		percentage, lstrlen(percentage), NULL);

	/* Adjust rectangle so that it includes the remaining  */
	/* percentage of the window. */
	rcPrcnt.left = rcPrcnt.right;
	rcPrcnt.right = rcClient.right;

	/* Invert the foreground and background colors. */
	SetBkColor(hdc, SetTextColor(hdc, GetBkColor(hdc)));

	/* Output the percentage value a second time in the window. */
	/* Function also paints right part of window. */
	ExtTextOut(hdc, rcClient.right / 2, (rcClient.bottom - size.cy) / 2,
		ETO_OPAQUE | ETO_CLIPPED, &rcPrcnt,
		percentage, lstrlen(percentage), NULL);

	ReleaseDC(hwnd_control, hdc);
}

// This routine checks and processes any messages in the main Windows message
// queue. We only use this when the import/export process is underway and the
// COM server wants us to check if our local user has pressed the CANCEL 
// button on either the import or export status dialog boxes.

	BOOL 
OkinoCommonComSource___CheckAndProcessWindowsMessageQueue()
{
	BOOL	status = TRUE;
	short	aborted;
	MSG	msg;

	while (PeekMessage(&msg, (HWND) NULL, (unsigned int) NULL, (unsigned int) NULL, PM_REMOVE)) {
		status = OkinoCommonComSource___HandleRawMessage(&msg, &aborted);
		if (aborted)
			return(TRUE);
		if (msg.message == WM_QUIT)
			status = FALSE;
	}
	return(status);
}

// This handles the message returned from PeekMessage()
// Called from CheckAndProcessWindowsMessageQueue() above. 

	BOOL
OkinoCommonComSource___HandleRawMessage(MSG *msg, short *aborted)
{
	BOOL	status = TRUE, modeless_dialog_msg;

	*aborted = FALSE;

	/* Handle the modeless dialog boxes separately... */
	modeless_dialog_msg = FALSE;

	if (msg->message == WM_QUIT)
		status = FALSE;
	else {
		;

//		if (hDlgExportAbortDialog && IsDialogMessage(hDlgExportAbortDialog, msg))
//			modeless_dialog_msg = TRUE;
	}

	// Don't translate and dispatch messages for modeless dialog boxes
	if (!modeless_dialog_msg) {
		TranslateMessage(msg);
		DispatchMessage(msg);
	}

	/* If the rendering abort flag has been pressed then return now. */
	if (abort_flag) {
		*aborted = TRUE;
		return(TRUE);
	}

	return(status);
}

	void
OkinoCommonComSource___Center_Window(HWND hParent, HWND hChild)
{
	RECT		child_rect;
	long		child_width, child_height;
	POINT		computed_offset;
	short		is_a_child_window;

	/* Get stats on the child window's full-extent window area */
	GetWindowRect(hChild, &child_rect);
	child_width  = child_rect.right - child_rect.left;
	child_height = child_rect.bottom - child_rect.top;

	/* Determine if this is a WS_CHILD window so that we will make all */
	/* computed coordinates relative to the parent window. */ 
	if (GetWindowLong(hChild, GWL_STYLE) & WS_CHILD)
		is_a_child_window = TRUE;
	else
		is_a_child_window = FALSE;

	/* Compute the centered position for the window */
	OkinoCommonComSource___ComputeCenteredWindowPosition(hParent, TRUE,
		is_a_child_window, (short) child_width, (short) child_height, &computed_offset);

	MoveWindow(hChild, computed_offset.x, computed_offset.y, 
		child_width, child_height, TRUE);
}

	void
OkinoCommonComSource___ComputeCenteredWindowPosition(HWND hParent, short center_in_screen,
	short is_a_child_window, short child_width, short child_height, 
	POINT *computed_offset)
{
	RECT		parent_rect;
	short		temp, xDisplay, yDisplay;
	POINT		center, parent_ul;

	/* Get size of the desktop */
	xDisplay = GetSystemMetrics(SM_CXSCREEN);
	yDisplay = GetSystemMetrics(SM_CYSCREEN);

	if (center_in_screen) {
		center.x = xDisplay / 2;
		center.y = yDisplay / 2;
	} else {
		GetClientRect(hParent, &parent_rect);
		center.x = (parent_rect.right - parent_rect.left) / 2;
		center.y = (parent_rect.bottom - parent_rect.top) / 2;
		ClientToScreen(hParent, &center);
	}
	/* Offset to the upper-left corner of the child window */
	center.x -= (child_width  / 2);
	center.y -= (child_height / 2);

	/* Make the coordinates relative to the parent window if this */
	/* window is a child window. */
	if (is_a_child_window) {
		GetClientRect(hParent, &parent_rect);
		parent_ul.x = parent_rect.left;
		parent_ul.y = parent_rect.top;
		ClientToScreen(hParent, &parent_ul);
		center.x -= parent_ul.x;
		center.y -= parent_ul.y;
	}

	/* Don't let child window move beyond the upper-left corner of screen */
	if (center.x < 0) center.x = 0;
	if (center.y < 0) center.y = 0;
	if ((temp = (xDisplay - child_width)) < center.x) 
		center.x = temp;
	if ((temp = (yDisplay - child_height)) < center.y) 
		center.y = temp;
	if (center.x < 0) center.x = 0;
	if (center.y < 0) center.y = 0;

	*computed_offset = center;
}

// Go and ask the user for a geometry import filename. Get the list of valid
// filespecs from the NuGraf UI via the COM callback.

// Returns NULL if the cancel button was pressed, else a pointer to the
// string buffer containing the selected filename.

	char *
OkinoCommonComSource___SelectGeometryImportFilename(HWND hDlg)
{
	USES_CONVERSION;
	BSTR 		file_open_filter_spec_out;
	LPTSTR 		filter_spec;
	HRESULT		hresult;

	// Get the filter spec from the COM server (which gets it from wsysplug.cpp)
	try {
		short		com_error_result;

loop:		hresult = pConvertIO->CreateFileOpenFilterSpecForAllImportConverters(&file_open_filter_spec_out);
		// If fatal, ask the user what to do: retry, restart or abort. If restart, try restarting the server,
		com_error_result = OkinoCommonComSource___Check_COM_HRESULT_Error_CODE(hresult, "Error while asking COM server to retrieve the file open spec. COM server has either crashed or is waiting for a user response.");
		if (com_error_result == IDC_SERVERFAULT_RETRY_OPERATION)
			goto loop;
		else if (com_error_result == IDC_SERVERFAULT_CANCEL_OPERATION)
			return(NULL);
	} catch (_com_error &e) {
		OkinoCommonComSource___Display_COM_Error(e, "Error while asking COM server to retrieve the file open spec. COM server has either crashed or is waiting for a user response.");
		return(NULL);
	}

	if (hresult == S_OK) {
		// Convert from Wide to char * 
		filter_spec = OLE2T(file_open_filter_spec_out);

		// Go get the filename from the user via the Import I/O library
		if (OkinoCommonComSource___GetLoadSaveFileName(TRUE, hDlg, gInstance, "Select Geometry File", last_geometry_filename_selected, 300, filter_spec))
			return(last_geometry_filename_selected);
		else
			return(NULL);
	} else
		return(NULL);
}

// File Selector to get a new filename for loading or saving.
//
// 	window_title				- Window title
// 	selected_filename			- In and out filename
// 	sizeof_selected_filename_buffer	- Size of above buffer
// 	FileOpen_Filter_Spec		= "Shockwave3D Files (*.w3d)|*.w3d|"
//
// Returns FALSE if the CANCEL button was pressed.

	short
OkinoCommonComSource___GetLoadSaveFileName(short loading, HWND hDlg, HINSTANCE hInstance, char *window_title, 
	char *selected_filename, short sizeof_selected_filename_buffer, char *FileOpen_Filter_Spec)
{
	OPENFILENAME 	of;
	long		Flags;

	/* Load the common controls DLL  */
	InitCommonControls();

	Flags = OFN_HIDEREADONLY | OFN_LONGNAMES | OFN_PATHMUSTEXIST;
	if (loading)
		Flags |= OFN_FILEMUSTEXIST;

	/* Add these so that the user can create a new directory by typing its name */
	/* Always use the NT version since we can't modify the Win95 dialog box very much */
	if (!OkinoCommonComSource___Setup_Select_Geometry_File(&of, loading, hDlg, NULL, FileOpen_Filter_Spec, window_title, 
			selected_filename, strlen(selected_filename), 
			Flags, NULL, NULL, hInstance, selected_filename, sizeof_selected_filename_buffer))
		return(FALSE);
	else
		return(TRUE);
}

/* Setup the common dialog "Open File" structure for a geometry filename. */
/* This is called from multiple locations. */

/* Set 'ok_to_extract_filename' to TRUE if you want this routineine to extract */
/* the path+filename from 'selected_filename', else set it to FALSE if */
/* 'selected_filename' only contains a path and no filename. */

	short
OkinoCommonComSource___Setup_Select_Geometry_File(OPENFILENAME *of, short load_save, HWND hwnd, char *sub_directory, 
	char *gszFilter, char *gszTitle, char *selected_filename,
	short ok_to_extract_filename, long Flags, LPOFNHOOKPROC lpfnHook, LPCSTR lpTemplateName, 
	HINSTANCE hInstance, char *szFile, long sizeof_szFile)
{
	char 	szFileTitle[MAXFILETITLELEN];
	char 	filename_copy[MAXFILETITLELEN];
	char	filepath[MAXFILETITLELEN];
	short	i;

	memset((char *) of, 0, sizeof(OPENFILENAME));
	strcpy(filename_copy, selected_filename);

	if (selected_filename[0] != '\0') {
		/* The output buffer contains a valid filepath and filename. */
		/* Extract the filepath and filename then put them into */
		/* the appropriate arrays. */
		if (ok_to_extract_filename) {
			i = strlen(selected_filename)-1;
			while (i >= 1 && selected_filename[i-1] != ':'
					&& selected_filename[i-1] != '/'
					&& selected_filename[i-1] != '\\')
				--i;
			/* Copy the base filename */
			strcpy(szFile, filename_copy+i);
			filename_copy[i] = '\0';
		} else {
			/* Else, 'selected_filename' only contains a path name */
			szFile[0] = '\0';
			i = strlen(selected_filename);
		}

		filepath[0] = '\0';
		if (i) {
			/* Remove any trailing slash (Windows won't accept the */
			/* filepath if we include it) */
			if (i && filename_copy[i-1] == '\\' && filename_copy[1] != ':')
				filename_copy[i-1] = '\0';
			/* Copy the filepath */
			if (filename_copy[1] == ':' || filename_copy[0] == '/' || filename_copy[0] == '\\')
				/* The path is absolute */
				strcpy(filepath, filename_copy);
			else {
				/* Prepend the CWD to the path */
				filepath[0] = '\0';
				strcat(filepath, filename_copy);
			}
		} else if (!i && sub_directory != (char *) NULL) {
			/* If the original filename did not have a filepath on */
			/* it but a sub-directory is specified then use it instead. */
			OkinoCommonComSource___CreateRelativeGeometryFilePath(sub_directory, filepath);
		}
		filename_copy[0] = '\0';
	} else {
		OkinoCommonComSource___CreateRelativeGeometryFilePath(sub_directory, filepath);
		szFile[0] = '\0';		/* No initial file */
	}
	szFileTitle[0] = '\0';		

	/* Translate the filter characters to nulls */
	for (i = 0; gszFilter[i] != '\0'; ++i)
		if (gszFilter[i] == '|')
			gszFilter[i] = '\0';

	of->lStructSize = sizeof(OPENFILENAME);
	of->hwndOwner = hwnd;
	of->hInstance = hInstance;
	of->lpstrFilter = gszFilter;
	of->nFilterIndex = 1;
	of->lpstrFile = szFile;
	of->nMaxFile = sizeof_szFile;
	of->lpstrFileTitle = szFileTitle;
	of->nMaxFileTitle = sizeof(szFileTitle);
	of->lpstrTitle = gszTitle;
	of->lpstrInitialDir = filepath;
	of->lpfnHook = lpfnHook;
	of->lpTemplateName = lpTemplateName;
	of->Flags = Flags;

	if (load_save)
		return (GetOpenFileName(of));
	else
		return (GetSaveFileName(of));
}

	void
OkinoCommonComSource___CreateRelativeGeometryFilePath(char *sub_directory, char *output_filepath)
{
	if (sub_directory != (char *) NULL)
		strcpy(output_filepath, sub_directory);
	else
		strcpy(output_filepath, "\\");
}

char *global_starting_directory;

	int CALLBACK 
OkinoCommonComSource___BrowseCtrlCallback(HWND hwnd, UINT uMsg, LPARAM /*lParam*/, LPARAM lpData)
{
	switch (uMsg)
	{
	case BFFM_INITIALIZED:
		// Set the starting directory
		if (global_starting_directory)
			::SendMessage(hwnd, BFFM_SETSELECTION, TRUE, (LPARAM)(LPCTSTR) global_starting_directory);
		break;
	
	case BFFM_SELCHANGED:
		break;
	}
	
	return FALSE;
}

// Entry point to ask user for a directory path

	BOOL 
OkinoCommonComSource___NewWindowsBrowseForFolder2(HWND pParentWnd, char **initial_directory,
	char *title, char *display_name)
{
	BROWSEINFO 	bInfo;
	LPITEMIDLIST 	pidl;
	char		buffer[MAX_PATH];
	LPMALLOC 	pMalloc;

	if (::SHGetMalloc(&pMalloc)!= NOERROR)
		return FALSE;
	
	ZeroMemory( (PVOID) &bInfo, sizeof(BROWSEINFO));
	
	OleInitialize(NULL);

	global_starting_directory = *initial_directory;
	bInfo.pidlRoot = NULL;
	
	bInfo.hwndOwner		= pParentWnd;
	bInfo.pszDisplayName	= buffer;
	bInfo.lpszTitle		= title;
	bInfo.ulFlags		= BIF_RETURNONLYFSDIRS | BIF_USENEWUI;
	bInfo.lpfn		= OkinoCommonComSource___BrowseCtrlCallback;  // address of callback function
	bInfo.lParam		= NULL;		// pass address of object to callback function
	
	if ((pidl = ::SHBrowseForFolder(&bInfo)) == NULL)
		return FALSE;
	
	if (::SHGetPathFromIDList(pidl, buffer) == FALSE) {
		pMalloc ->Free(pidl);
		pMalloc ->Release();
		return FALSE;
	}
	
	OkinoCommonComSource___Copy_String(initial_directory, buffer);
 
	pMalloc->Free(pidl);
	pMalloc->Release();
	return TRUE;
}

// ------------->>>  String & Filename Utility Functions  <<<----------------

	void
OkinoCommonComSource___Copy_String(char **string, char *new_contents)
{
	if (*string != (char *) NULL)
		free((char *) *string);
	*string = (char *) malloc(strlen(new_contents) + 1);
	strcpy(*string, new_contents);
}

/* Extract the filepath from a filename */

	void
OkinoCommonComSource___ExtractBaseFilePath(char *in_filename, char *out_path)
{
	short	i;

	if (in_filename != (char *) NULL) {
		i = strlen(in_filename)-1;
		while (i >= 0 && 
#if !defined(UNIX)
				in_filename[i] != ':' &&
#endif
	
				in_filename[i] != '/' &&
				in_filename[i] != '\\')
			--i;
		strncpy(out_path, in_filename, i+1);
		out_path[i+1] = '\0';
	} else
		out_path[0] = '\0';
}

	short
OkinoCommonComSource___ExtractBaseFileName(char *in_name, char *out_name, short root_filename_only)
{
	char	*t, *l, *ptr;
	short	count;

	/* First, find the root filename */
	for (t = l = in_name; *t != '\0'; t++) {
#if 11
		if (*t == '\\' || *t == '/')
			l = t + 1;
		else if (*(t + 1) == ':')
			/* Skip over drive specifier */
			l = t + 2;
#elif defined(MACINTOSH)
		if (*t == ':')
			l = t + 1;
#else
		if (*t == '/')
			l = t + 1;
#endif
	}

	/* Then copy it to the output filename */
	count = 0;
	t = l;
	do {
		*(out_name + count++) = *t;
	} while (*(t++) != '\0');

	/* And strip off the suffix after the last '.' in the name */
	if (root_filename_only) {
		ptr = out_name + strlen(out_name) - 1;
		while (*ptr && *ptr != '.' && ptr != out_name)
			--ptr;
		if (*ptr == '.' && ptr != in_name) 
			*ptr = '\0';
	}

	return((short) (l-in_name));
}

	static void
string_to_lower(char *in_str, char *out_str)
{
	short	i;

	for (i = 0; i < (short) strlen(in_str); i++)
		if (in_str[i] >= 'A' && in_str[i] <= 'Z')
			out_str[i] = in_str[i] + 0x20;
		else
			out_str[i] = in_str[i];
	out_str[i] = '\0';
}

/* Check to see if the filename has the specified extension (ie: ".tif") */
/* Returns TRUE if the extension is as expected, else FALSE. Nothing is modified. */

	short
OkinoCommonComSource___Check_For_File_Extension(char *in_file, char *extension)
{
	char	temp_buf[300], temp_extension[300];
	unsigned long	len2;

	if (in_file == NULL)
		return(FALSE);
	if (extension == NULL)
		return(FALSE);
	if (!strlen(in_file))
		return(FALSE);
	if (!strlen(extension))
		return(FALSE);

	strcpy(temp_extension, extension);
	string_to_lower(temp_extension, temp_extension);
	len2 = strlen(temp_extension);
	strcpy(temp_buf, in_file);
	string_to_lower(temp_buf, temp_buf);
	if (strlen(in_file) >= len2) {
		if (!strcmp(temp_buf+strlen(temp_buf)-len2, temp_extension))
			return(TRUE);
	}
	return(FALSE);
}

// ------------------------>>>  Obsolete Code  <<<-------------------------

// This dialog box asks the user if the COM server is local to this computer
// or on a remote computer. If the remote computer is selected then the
// user also has to type in a DNS name for the remote machine.

#if 0
	BOOL WINAPI 
OkinoCommonComSource___RemoteHostDlgProc(HWND hwnd, UINT msg, UINT wparam, LONG lparam)
{
	switch (msg) {
	case WM_INITDIALOG:
		OkinoCommonComSource___Center_Window(GetParent(hwnd), hwnd);

		SetDlgItemText(hwnd, IDC_REMOTE_MACHINE_NAME, "");

		use_local_server = TRUE;
		CheckRadioButton(hwnd, IDC_LOCAL_RB, IDC_RB_REMOTE, IDC_LOCAL_RB);
		OkinoCommonComSource___Set_Control_Enable(hwnd, IDC_REMOTE_MACHINE_NAME, FALSE);

		return (FALSE);
	case WM_COMMAND:
		switch(LOWORD(wparam)) {
			case IDOK:
				if (!use_local_server && !GetDlgItemText(hwnd, IDC_REMOTE_MACHINE_NAME, server_name, 300))
					return(FALSE);
				EndDialog(hwnd, LOWORD(wparam));
				return (TRUE);

			case IDCANCEL:
				EndDialog(hwnd, LOWORD(wparam));
				return (TRUE);
			case IDC_LOCAL_RB:
				use_local_server = TRUE;
				CheckRadioButton(hwnd, IDC_LOCAL_RB, IDC_RB_REMOTE, IDC_LOCAL_RB);
				OkinoCommonComSource___Set_Control_Enable(hwnd, IDC_REMOTE_MACHINE_NAME, FALSE);
				return (TRUE);
			case IDC_RB_REMOTE:
				use_local_server = FALSE;
				CheckRadioButton(hwnd, IDC_LOCAL_RB, IDC_RB_REMOTE, IDC_RB_REMOTE);
				OkinoCommonComSource___Set_Control_Enable(hwnd, IDC_REMOTE_MACHINE_NAME, TRUE);
				return (TRUE);
		}
		break;
	}
	return (FALSE);
}
#endif
