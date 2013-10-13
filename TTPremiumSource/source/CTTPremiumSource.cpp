// Copyright (C) 2009-2013 MisterD
// 
// GenericTvLibrary is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License.
// 
// This Programm is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with GenericTvLibrary. If not, see <http://www.gnu.org/licenses/>.

#include "stdafx.h"

#include <initguid.h>
#include "CTTPremiumSourceStream.h"
#include "CTTPremiumSource.h"
#include "IGTvLibSource.h"
#include "IGTvLibTuning.h"
#include "IGTvLibHwPidFiltering.h"
#include "DVBDLLInit.h"

// Using this pointer in constructor
#pragma warning(disable:4355 4127)

// Setup data

const AMOVIESETUP_MEDIATYPE sudPinTypes =
{
	&MEDIATYPE_Stream,							// Major type
	&MEDIASUBTYPE_MPEG2_TRANSPORT   // Minor type
};

const AMOVIESETUP_PIN psudPins[] =
{
	{ L"Output",            // Pin's string name
	FALSE,                // Is it rendered
	TRUE,                 // Is it an output
	FALSE,                // Allowed none
	FALSE,                // Allowed many
	&CLSID_NULL,          // Connects to filter
	L"Input",             // Connects to pin
	1,                    // Number of types
	&sudPinTypes }        // Pin information
};

const AMOVIESETUP_FILTER sudTTPremium =
{
	&CLSID_TTPremiumSource,           // CLSID of filter
	L"TTPremiumSource",    // Filter's name
	MERIT_DO_NOT_USE,       // Filter merit
	1,                      // Number of pins
	psudPins                // Pin information
};


CFactoryTemplate g_Templates [1] = 
{
	{ L"TTPremiumSource"
	, &CLSID_TTPremiumSource
	, CTTPremiumSource::CreateInstance
	, NULL
	, &sudTTPremium }
};
int g_cTemplates = sizeof(g_Templates) / sizeof(g_Templates[0]);

STDAPI DllRegisterServer()
{
	return AMovieDllRegisterServer2( TRUE );
}

STDAPI DllUnregisterServer()
{
	return AMovieDllRegisterServer2( FALSE );
}

//
// DllEntryPoint
//
extern "C" BOOL WINAPI DllEntryPoint(HINSTANCE, ULONG, LPVOID);
extern "C" BOOL WINAPI _DllMainCRTStartup(HINSTANCE, ULONG, LPVOID);

BOOL APIENTRY DllMain(HANDLE hModule, 
					  DWORD  dwReason, 
					  LPVOID lpReserved)
{
	_DllMainCRTStartup((HINSTANCE)hModule, dwReason, lpReserved);
	return DllEntryPoint((HINSTANCE)hModule, dwReason, lpReserved);
}



//
// CreateInstance
//
// Creator function for the class ID
//
CUnknown * WINAPI CTTPremiumSource::CreateInstance(IUnknown *pUnk, HRESULT *phr)
{
	InitDvbApiDll();
	CTTPremiumSource *pNewFilter = new CTTPremiumSource(pUnk, phr);
	if (phr)
	{
		if (pNewFilter == NULL) {
			*phr = E_OUTOFMEMORY;
		}else{
			*phr = S_OK;
		}
	}
	return pNewFilter;
}

//
// Constructor
//
CTTPremiumSource::CTTPremiumSource(IUnknown *pUnk, HRESULT *phr) :
	CSource(NAME("TTPremium Source Filter"), pUnk, CLSID_TTPremiumSource)
{

	logger = Logger::getInstance("TTPremiumSource");
	m_cardControl = new TTPremiumControl(pUnk);
	m_commonInterfaceControl = new CommonInterfaceControl(pUnk);
	m_outputPin = new CTTPremiumSourceStream(this, phr);
	if (phr)
	{
		if (m_outputPin == NULL){
			*phr = E_OUTOFMEMORY;
		}else{
			*phr = S_OK;
		}
	} 
}

//
// Destructor
//
CTTPremiumSource::~CTTPremiumSource()
{
	LOG4CPLUS_DEBUG(logger,"CTTPremiumSource dtor");
	delete m_outputPin;
	delete m_commonInterfaceControl;
	delete m_cardControl;
	LOG4CPLUS_DEBUG(logger,"CTTPremiumSource dtor - finished");
}

//
// Return the interface(s) that we support
//
STDMETHODIMP CTTPremiumSource::NonDelegatingQueryInterface(REFIID riid, void** ppv)
{
	CheckPointer(ppv, E_POINTER);
	// Do we have this interface
	if (riid == IID_IGTvLibLogging)
	{
		return GetInterface((IGTvLibLogging*)this, ppv);
	}else if (riid == IID_IGTvLibSource)
	{
		return GetInterface((IGTvLibSource*)m_cardControl, ppv);
	}else if (riid == IID_IGTvLibTuning)
	{
		return GetInterface((IGTvLibTuning*)m_cardControl, ppv);
	}else if (riid == IID_IGTvLibDiseqC)
	{
		return GetInterface((IGTvLibDiseqC*)m_cardControl, ppv);
	}else if (riid == IID_IGTvLibHwPidFiltering)
	{
		return GetInterface((IGTvLibHwPidFiltering*)m_outputPin, ppv);
	}else if (riid == IID_IGTvLibCommonInterface)
	{
		return GetInterface((IGTvLibCommonInterface*)m_commonInterfaceControl, ppv);
	}

	return CSource::NonDelegatingQueryInterface(riid, ppv);

}

////////////////////////////////////////////////////////////////////////
//
// Exported entry points for registration and unregistration 
// (in this case they only call through to default implementations).
//
////////////////////////////////////////////////////////////////////////

STDMETHODIMP CTTPremiumSource::ConfigureLogging(wchar_t* logFile, wchar_t* identifier, LogLevelOption logLevel){
	CStringW tempW = logFile;
	CString logFileCA = tempW;
	char* logFileA = logFileCA.GetBuffer();
	tempW = identifier;
	CString identifierCA = tempW;
	char* identifierA = identifierCA.GetBuffer();
	logger = Logger::getInstance(identifierA);
	if(logger.getAppender(identifierA)==NULL){


		SharedAppenderPtr append_1(
			new SpecialDailyRollingFileAppender(logFileA,DAILY,true,10));
		std::auto_ptr<Layout> myLayout = std::auto_ptr<Layout>(new
			log4cplus::PatternLayout("%d{%d.%m.%Y %H:%M:%S,%q} %-5p [%t]: %m%n"));

		append_1->setName(identifierA);
		append_1->setLayout( myLayout );
		append_1->setThreshold(logLevel);
		logger.addAppender(append_1);
		m_outputPin->ReconfigureLogging(identifierA);
		m_cardControl->ReconfigureLogging(identifierA);
		m_commonInterfaceControl->ReconfigureLogging(identifierA);
	}

	return true;
}


