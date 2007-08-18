#include "COMHelper/PolyTransCOMSinkEvents.h"
#include "COMHelper/PolyTransOutput.h"
#include <atlbase.h>
#include <string>

PolyTransCOMSinkEvents::PolyTransCOMSinkEvents()
{
}


void
PolyTransCOMSinkEvents::SetThePolyTransOutput( PolyTransOutput* aPolyTransOutput )
{
	m_ThePolyTransOutput = aPolyTransOutput;
}


STDMETHODIMP
PolyTransCOMSinkEvents::OnGeometryImportProgress(
	BSTR bstr_mode,			// Which mode we are working in
	BSTR bstr_info_text,		// Info text for the "showinfotext" mode
	long curr_file_offset,		/* Current offset into the file (-1 to make dialog box field blank) */
	long file_length,		/* Length of the input file */
	long total_objects,		/* Total number of objects created to date (-1 to make dialog box field blank) */
	long total_polygons )		/* Total number of polygons created to date (-1 to make dialog box field blank) */
{
	if ( m_ThePolyTransOutput != NULL )
	{
		USES_CONVERSION; //needed by ATL to do conversion
		LPTSTR mode = OLE2T( bstr_mode );

		char progressMessage[300];
		strcpy( progressMessage, "PolyTrans Import Progress: " );
		strcat( progressMessage, "Mode: " );
		strcat( progressMessage, mode );
		
		if ( bstr_info_text != NULL )
		{
			LPTSTR infoText = OLE2T( bstr_info_text );
			strcat( progressMessage, " Info: " );
			strcat( progressMessage, infoText );
		}

		m_ThePolyTransOutput->WriteImportProgress( progressMessage );
	}

	return S_OK;
}


STDMETHODIMP 
PolyTransCOMSinkEvents::OnGeometryExportProgress( 
	BSTR bstr_mode,			// Which mode we are working in
	long Nv_Scanline_Num,
	long Nv_Num_Scanlines)
{
	if ( m_ThePolyTransOutput != NULL )
	{
		if ( bstr_mode != NULL )
		{
			USES_CONVERSION; //needed for OLE2T
			LPTSTR mode = OLE2T(bstr_mode);

			if ( strcmp( mode, "initialize" ) == 0 )
			{
				m_ThePolyTransOutput->WriteExportMessage( mode );
			}
			else if ( strcmp( mode, "shutdown" ) == 0 )
			{
				m_ThePolyTransOutput->WriteExportMessage( mode );
			}
			else if ( strcmp( mode, "proceed" ) == 0 )
			{
				m_ThePolyTransOutput->WriteExportProgress( Nv_Scanline_Num, Nv_Num_Scanlines );
			}
			else
			{
				m_ThePolyTransOutput->WriteExportMessage( mode );
			}
		}
	}

	return S_OK;
}


STDMETHODIMP 
PolyTransCOMSinkEvents::OnErrorMessage(BSTR Nv_Error_Level, 
	BSTR Nv_Error_Label, BSTR Nv_Formatted_Message ) 
{
	if ( m_ThePolyTransOutput != NULL )
	{
		if ( ( Nv_Error_Level != NULL ) &&
			( Nv_Error_Label != NULL ) &&
			( Nv_Formatted_Message != NULL ) )
		{
			USES_CONVERSION;
			LPTSTR error_level = OLE2T(Nv_Error_Level);
			LPTSTR error_label = OLE2T(Nv_Error_Label);
			LPTSTR formatted_message = OLE2T(Nv_Formatted_Message);

			m_ThePolyTransOutput->WriteErrorMessage( error_level, error_label, formatted_message );
		}
	}

	return S_OK;
}


STDMETHODIMP 
PolyTransCOMSinkEvents::OnMessageWindowTextOutput(BSTR Nv_Formatted_Message )
{
	HRESULT hResult = S_OK;

	USES_CONVERSION;
	LPTSTR formatted_message = OLE2T(Nv_Formatted_Message);
	char	final_string[2048];
	short	extent;

	final_string[0] = '\0';
	extent = 0;
	if (formatted_message && strlen(formatted_message))
	{
		short i, j, count;

		// Ignore script comment lines
		if (formatted_message[0] == '#')
		{
			return S_OK;
		}

		// Ignore script number prompt lines
		if (formatted_message[1] == '>' || formatted_message[2] == '>' || formatted_message[3] == '>' || !strncmp(formatted_message, "listscript", 10))
		{
			return S_OK;
		}

		// Turn any tab characters into spaces
		count = 0;
		for (i=0; i < (short) strlen(formatted_message); ++i)
		{
			if (formatted_message[i] == 0x9)
			{
				for(j=0; j < 8; ++j)
				{
					final_string[count++] = ' ';
				}
			}
			else if (formatted_message[i] == 0xa || formatted_message[i] == 0xd)
			{
				;
			}
			else
			{
				final_string[count++] = formatted_message[i];
			}
			final_string[count] = '\0';
		}

		m_ThePolyTransOutput->WriteMessage( final_string );
	}

	return S_OK;
}


//Note: not sure that we will use this in the OpenSceneGraph implementation
STDMETHODIMP 
PolyTransCOMSinkEvents::OnBatchJobDone( 
	BSTR job_guid_bstr,		// The Job ID
	BSTR status_text_bstr, 		// Signifies if the job completed, crashed or aborted
	BSTR time_string_bstr, 		// Time completion string
	BSTR windows_temp_dir_bstr)	// Windows TEMP directory we are using to store the queue files
{
	if ( m_ThePolyTransOutput != NULL )
	{
		USES_CONVERSION;
		LPTSTR job_guid = OLE2T(job_guid_bstr);
		LPTSTR status_text = OLE2T(status_text_bstr);
		LPTSTR time_string = OLE2T(time_string_bstr);
		LPTSTR windows_temp_dir = OLE2T(windows_temp_dir_bstr);

		char doneMessage[300];
		strcpy( doneMessage, "PolyTrans Batch Job Done: " );
		strcat( doneMessage, "Status: " );
		strcat( doneMessage, status_text );

		m_ThePolyTransOutput->WriteMessage( doneMessage );
	}

	return S_OK;
}