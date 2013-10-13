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

#pragma once

#include "IGTvLibLogging.h"
#include "IGtvLibTuning.h"
#include "IGtvLibSource.h"
#include "IGtvLibDiseqC.h"

#include <log4cplus/logger.h>
#include <log4cplus/SpecialFileAppender.h>
#include <log4cplus/layout.h>
#include <log4cplus/configurator.h>
#include <log4cplus/loggingmacros.h>

#include "..\include\TSDVBAPI.h"

using namespace std;
using namespace log4cplus;
using namespace log4cplus::helpers;

class CTongshiDVBSource;
class CTongshiDVBSourceStream;

// {A5520DFE-4275-4306-A1E6-6BB36648C4F3}
DEFINE_GUID(CLSID_TongshiDVBSource, 
			0xa5520dfe, 0x4275, 0x4306, 0xa1, 0xe6, 0x6b, 0xb3, 0x66, 0x48, 0xc4, 0xf3);

#define MAX_BUFFER_SIZE 47000

class CTongshiDVBSourceStream : public CSourceStream
{

protected:
	CTongshiDVBSource* m_sourceFilter;

	DWORD m_seqNumber;
	char m_buffer[MAX_BUFFER_SIZE];
	int m_buffsize;

	HRESULT FillBuffer(IMediaSample *pSamp);
	HRESULT GetMediaType(__inout CMediaType *pMediaType);
	HRESULT DecideBufferSize(IMemAllocator *pAlloc, ALLOCATOR_PROPERTIES *pRequest);
	void Clear();

public:

	CTongshiDVBSourceStream(HRESULT *phr, CSource *pFilter);
	~CTongshiDVBSourceStream();

	HRESULT Run();
	HRESULT Stop();
	HRESULT Pause();
	void ReconfigureLogging(wchar_t* appenderName);

private:
	Logger logger;
	PMYDEV_DATA_PORT m_dataPort;
	int m_filterState;


};

class CTongshiDVBSource : public CSource, public IGTvLibSource, public IGTvLibTuning, public IGTvLibLogging, public IGTvLibDiseqC
{
	friend class CTongshiDVBSourceStream;
private:
	// Constructor is private because you have to use CreateInstance
	CTongshiDVBSource(IUnknown *pUnk, HRESULT *phr);
	~CTongshiDVBSource();

	CTongshiDVBSourceStream *m_stream;
	CCritSec 		m_Lock;								// Main renderer critical section

public:
	DECLARE_IUNKNOWN

	STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void** ppv);
	static CUnknown * WINAPI CreateInstance(IUnknown *pUnk, HRESULT *phr);  

	STDMETHOD(GetNumberOfDevices) (ULONG* numberOfDevices);
	STDMETHOD(SetDeviceIndex) (ULONG deviceIndex) ;
	STDMETHOD(GetDevicePath)(THIS_ wchar_t** devicePath, wchar_t** deviceName);

	STDMETHOD (TuneDVBT) (FrequencySettings fSettings) ;
	STDMETHOD (TuneDVBS) (FrequencySettings fSettings, DigitalDemodulator2Settings dSettings, LnbInfoSettings lSettings, DiseqcSatelliteSettings sSettings) ;
	STDMETHOD (TuneDVBC) (FrequencySettings fSettings, DigitalDemodulatorSettings dSettings) ;
	STDMETHOD (TuneATSC) (ULONG channelNumber, FrequencySettings fSettings, DigitalDemodulatorSettings dSettings);
	STDMETHOD (GetAvailableTuningTypes) (ULONG* tuningTypes) ;
	STDMETHOD (GetSignalStats)(ULONG* tunerLocked, ULONG* signalPresent, LONG* signalQuality, LONG* signalLevel) ;
  STDMETHOD (IsDiseqSupported)(ULONG* supported);

	STDMETHOD (ConfigureLogging)(wchar_t* logFile, wchar_t* identifier, LogLevelOption logLevel) ;

	STDMETHOD (SendDiseqCMessage) (ULONG mode, ULONG len,byte* message,ULONG* responseId);
	STDMETHOD (ReadDiseqCMessage) (ULONG requestId, ULONG* len, byte* command);

private:
	STDMETHODIMP ActivateCard (ULONG cardId) ;
	STDMETHODIMP DeactivateCard () ;
	void GetNumberOfInstalledCards();
	Logger logger;
	PMYDEV_ADAPTER m_cardHandle;
	MODULATIONTYPE m_modType;
	ULONG m_tuningType;
	ULONG m_numberOfDevices;
	BOOLEAN m_bInit;
	ULONG m_iDeviceIndex;

};

