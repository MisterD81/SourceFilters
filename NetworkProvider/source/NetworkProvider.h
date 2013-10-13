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
#include "IGTvLibTuning.h"
#include "IGTvLibDiseqC.h"
#include "OutputPin.h"
#include "FrequencyFilter.h"
#include "DigitalDemodulatorFilter.h"
#include "DigitalDemodulator2Filter.h"
#include "LnbInfoFilter.h"
#include "DiseqCommand.h"
#include <log4cplus/SpecialFileAppender.h>
#include <log4cplus/layout.h>
#include <log4cplus/configurator.h>
#include <log4cplus/loggingmacros.h>

using namespace std;
using namespace log4cplus;
using namespace log4cplus::helpers;


class CNetworkProviderFilter;
class CNetworkProvider;

typedef struct stRegisterDevice
{
  int     id;
  CHAR	name[128];
  IBaseFilter* control;
  IBDA_DeviceControl* deviceControl;
}RegisterDevice;


// {B8B0CB0B-17D3-4ef5-9545-8DD83F49D088}
DEFINE_GUID(CLSID_GenTvLib_NetworkProvider, 
0xb8b0cb0b, 0x17d3, 0x4ef5, 0x95, 0x45, 0x8d, 0xd8, 0x3f, 0x49, 0xd0, 0x88);


class CNetworkProviderFilter : public CBaseFilter 
{
  CNetworkProvider* const m_pNetworkProvider;

public:

  // Constructor
  CNetworkProviderFilter(CNetworkProvider *m_pNetworkProvider,
    LPUNKNOWN pUnk,
    CCritSec *pLock,
    HRESULT *phr);

  // Pin enumeration
  CBasePin* GetPin(int n);
  int GetPinCount();

  // Open and close the file as necessary
  STDMETHODIMP Run(REFERENCE_TIME tStart);
  STDMETHODIMP Pause();
  STDMETHODIMP Stop();
private:
  Logger logger;
};

class CNetworkProvider : public CUnknown, public IBDA_NetworkProvider, public IGTvLibTuning, public IGTvLibLogging, public IGTvLibDiseqC
{
  friend class CNetworkProviderFilter;
  friend class CDVBNetworkProviderPin;
  CNetworkProviderFilter*	m_pFilter;						// Methods for filter interfaces
  CDVBNetworkProviderPin*	m_pNetworkProviderPin;			// The output pin
  CCritSec 		m_Lock;								// Main renderer critical section
  CCritSec 		m_ReceiveLock;						// Sublock for received samples
public:
  DECLARE_IUNKNOWN

  CNetworkProvider(LPUNKNOWN pUnk, HRESULT *phr);
  ~CNetworkProvider();

  static CUnknown * WINAPI CreateInstance(LPUNKNOWN punk, HRESULT *phr);

  // IGTvLibTuning, IGTvLibLogging, IGTvLibDiseqC
  STDMETHODIMP TuneDVBT (FrequencySettings fSettings);
  STDMETHODIMP TuneDVBS (FrequencySettings fSettings, DigitalDemodulator2Settings dSettings, LnbInfoSettings lSettings, DiseqcSatelliteSettings sSettings);
  STDMETHODIMP TuneDVBC (FrequencySettings fSettings, DigitalDemodulatorSettings dSettings);
  STDMETHODIMP TuneATSC (ULONG channelNumber, FrequencySettings fSettings, DigitalDemodulatorSettings dSettings);
  STDMETHODIMP GetAvailableTuningTypes (ULONG* tuningTypes);
  STDMETHODIMP GetSignalStats (ULONG* tunerLocked, ULONG* signalPresent, LONG* signalQuality, LONG* signalLevel);
  STDMETHODIMP IsDiseqSupported(ULONG* supported);
  STDMETHODIMP ConfigureLogging (wchar_t* logFile, wchar_t* identifier, LogLevelOption logLevel);
  STDMETHODIMP SendDiseqCMessage (ULONG mode, ULONG len,byte* message,ULONG* responseId);
  STDMETHODIMP ReadDiseqCMessage (ULONG requestId, ULONG* len, byte* command);

  // IBDA_Network_Provider
  HRESULT STDMETHODCALLTYPE  PutSignalSource(ULONG ulSignalSource) ;
  HRESULT STDMETHODCALLTYPE  GetSignalSource(ULONG* pulSignalSource) ;
  HRESULT STDMETHODCALLTYPE  GetNetworkType(GUID* pguidNetworkType) ;
  HRESULT STDMETHODCALLTYPE  PutTuningSpace(REFGUID guidTuningSpace) ;
  HRESULT STDMETHODCALLTYPE  GetTuningSpace(GUID* pguidTuingSpace) ;
  HRESULT STDMETHODCALLTYPE  RegisterDeviceFilter(IUnknown* pUnkFilterControl,ULONG* ppvRegisitrationContext) ;
  HRESULT STDMETHODCALLTYPE  UnRegisterDeviceFilter(ULONG pvRegistrationContext) ;

private:

  // Overriden to say what interfaces we support where
  STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void ** ppv);
  bool Retune ();
  bool ClearList();
  bool RemoveWithID(ULONG id);
  bool StartChanges();
  bool CommitChanges();
  bool CheckCapabilities(IBaseFilter* filter);
  bool CheckInterfaces(IBDA_Topology* topology);
  bool CheckSpecificInterface(GUID requiredGUID, IBDA_Topology* topology, GUID testGUID,ULONG elementList,IUnknown** element);
  bool CheckTuningType(IBDA_Topology* topology);
  vector<RegisterDevice*> m_vecDevices;
  typedef vector<RegisterDevice*>::iterator iVecDevices;
  CFrequencyFilter* m_pFrequencyFilter;
  CDigitalDemodulatorFilter* m_pDigitalDemodulatorFilter;
  CDigitalDemodulator2Filter* m_pDigitalDemodulator2Filter;
  CLnbInfoFilter* m_pLnbInfoFilter;
  CDiseqCommand* m_pDiseqCommand;
  vector<IUnknown*> m_signalStatistics;
  ULONG m_fid; 
  ULONG m_tuningType;
  TuningType m_currentTuningType;
  Logger logger;
};
