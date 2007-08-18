#pragma once

#include "shared_okino_com_src.h"
//#include "stdafx.h"

class PolyTransOutput;

class PolyTransCOMSinkEvents : public IDispatchImpl<_INuGrafIOEvents, &IID__INuGrafIOEvents, &LIBID_COMSRVLib>, public CComObjectRoot 
{
	
public:
	
	//CONSTRUCTOR
	PolyTransCOMSinkEvents();

	void SetThePolyTransOutput( PolyTransOutput* aPolyTransOutput );


	BEGIN_COM_MAP( PolyTransCOMSinkEvents )
		COM_INTERFACE_ENTRY(IDispatch)
		COM_INTERFACE_ENTRY(_INuGrafIOEvents)
	END_COM_MAP()

	//-- Local event sinks contained entirely within this file
	virtual STDMETHODIMP    OnGeometryImportProgress(BSTR bstr_mode, BSTR bstr_info_text, long curr_file_offset, long file_length, long total_objects, long total_polygons);
	virtual STDMETHODIMP 	OnGeometryExportProgress( BSTR bstr_mode, long Nv_Scanline_Num, long Nv_Num_Scanlines);
	virtual STDMETHODIMP 	OnErrorMessage(BSTR Nv_Error_Level, BSTR Nv_Error_Label, BSTR Nv_Formatted_Message ) ;
	virtual STDMETHODIMP 	OnMessageWindowTextOutput( BSTR Nv_Formatted_Message );
	virtual STDMETHODIMP 	OnBatchJobDone( BSTR job_guid, BSTR status_text, BSTR time_string, BSTR windows_temp_dir ) ;

private:

	PolyTransOutput* m_ThePolyTransOutput;
};