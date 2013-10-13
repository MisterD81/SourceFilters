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

#include <streams.h>
#include <initguid.h>
#include "tongshidvbsource.h"

#define g_wszPushSource     L"MediaPortal Tongshi DVB Source Filter"
#define FILTER_PAUSED 2
#define FILTER_STOPPED 0
#define FILTER_RUNNING 1

// Filter setup data
const AMOVIESETUP_MEDIATYPE dvbSourceOpPinTypes =
{
  &MEDIATYPE_Stream,              // Major type
  &MEDIASUBTYPE_MPEG2_TRANSPORT   // Minor type
};

const AMOVIESETUP_PIN dvbSourceOutputPin = 
{
  L"Output",      // Obsolete, not used.
  FALSE,          // Is this pin rendered?
  TRUE,           // Is it an output pin?
  FALSE,          // Can the filter create zero instances?
  FALSE,          // Does the filter create multiple instances?
  &CLSID_NULL,    // Obsolete.
  NULL,           // Obsolete.
  1,              // Number of media types.
  &dvbSourceOpPinTypes // Pointer to media types.
};

const AMOVIESETUP_FILTER iptvPushSource =
{
  &CLSID_TongshiDVBSource,    // Filter CLSID
  g_wszPushSource,        // String name
  MERIT_DO_NOT_USE,       // Filter merit
  1,                      // Number pins
  &dvbSourceOutputPin          // Pin details
};

// List of class IDs and creator functions for the class factory. This
// provides the link between the OLE entry point in the DLL and an object
// being created. The class factory will call the static CreateInstance.
// We provide a set of filters in this one DLL.

CFactoryTemplate g_Templates[1] = 
{
  { 
    g_wszPushSource,                    // Name
      &CLSID_TongshiDVBSource,                // CLSID
      CTongshiDVBSource::CreateInstance,      // Method to create an instance of MyComponent
      NULL,                               // Initialization function
      &iptvPushSource                     // Set-up information (for filters)
  }
};

int g_cTemplates = sizeof(g_Templates) / sizeof(g_Templates[0]);    



////////////////////////////////////////////////////////////////////////
//
// Exported entry points for registration and unregistration 
// (in this case they only call through to default implementations).
//
////////////////////////////////////////////////////////////////////////

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

BOOL APIENTRY DllMain(HANDLE hModule, 
                      DWORD  dwReason, 
                      LPVOID lpReserved)
{
  return DllEntryPoint((HINSTANCE)(hModule), dwReason, lpReserved);
}

CTongshiDVBSourceStream::CTongshiDVBSourceStream(HRESULT *phr, CSource *pFilter)
: CSourceStream(NAME("MediaPortal Tongshi DVB Stream"), phr, pFilter, L"Output"),
m_seqNumber(0),
m_buffsize(0),
m_filterState(FILTER_STOPPED),
m_dataPort(NULL),
logger(Logger::getInstance(L"TongshiDVB"))
{

}

CTongshiDVBSourceStream::~CTongshiDVBSourceStream()
{
  LOG4CPLUS_DEBUG(logger, "CTongshiDVBSourceStream::dtor()");

  Clear();
}

void CTongshiDVBSourceStream::Clear() 
{
  LOG4CPLUS_DEBUG(logger, "CTongshiDVBSourceStream::Clear()");

  if(CAMThread::ThreadExists())
  {
    CAMThread::CallWorker(CMD_EXIT);
    CAMThread::Close();
  }
  if(m_dataPort!=NULL){
    TSDVB_DeleteDataPort(m_dataPort);
    m_dataPort = NULL;
  }
  LOG4CPLUS_DEBUG(logger, "CTongshiDVBSourceStream::Clear() - finished");
}

HRESULT CTongshiDVBSourceStream::FillBuffer(IMediaSample *pSamp)
{
  LOG4CPLUS_DEBUG(logger, "CTongshiDVBSourceStream::FillBuffer()");

  BYTE *pData;
  long cbData;
  DWORD dwByteRead = 0;
  CheckPointer(pSamp, E_POINTER);

  CAutoLock cAutoLock(m_pFilter->pStateLock());

  // Access the sample's data buffer
  pSamp->GetPointer(&pData);
  cbData = pSamp->GetSize();

  if(m_dataPort!=NULL){
    TSDVB_ReadDataPort(m_dataPort,pData,cbData,&dwByteRead);
  }

  REFERENCE_TIME rtStart = m_seqNumber;
  REFERENCE_TIME rtStop  = rtStart + 1;

  pSamp->SetTime(&rtStart, &rtStop);
  pSamp->SetActualDataLength(dwByteRead);

  m_seqNumber++;
  pSamp->SetSyncPoint(TRUE);
  LOG4CPLUS_INFO(logger, "CTongshiDVBSourceStream::FillBuffer() - finished");

  return S_OK;
}

HRESULT CTongshiDVBSourceStream::GetMediaType(__inout CMediaType *pMediaType) 
{
  LOG4CPLUS_DEBUG(logger, "CTongshiDVBSourceStream::GetMediaType()");

  pMediaType->majortype = MEDIATYPE_Stream;
  pMediaType->subtype = MEDIASUBTYPE_MPEG2_TRANSPORT;

  return S_OK;
}

HRESULT CTongshiDVBSourceStream::DecideBufferSize(IMemAllocator *pAlloc, ALLOCATOR_PROPERTIES *pRequest) 
{
  LOG4CPLUS_DEBUG(logger, "CTongshiDVBSourceStream::DecideBufferSize()");
  HRESULT hr;
  CAutoLock cAutoLock(m_pFilter->pStateLock());

  CheckPointer(pAlloc, E_POINTER);
  CheckPointer(pRequest, E_POINTER);

  // If the bitmap file was not loaded, just fail here.

  // Ensure a minimum number of buffers
  if (pRequest->cBuffers == 0)
  {
    pRequest->cBuffers = 1;
  }
  pRequest->cbBuffer = MAX_BUFFER_SIZE;

  ALLOCATOR_PROPERTIES Actual;
  hr = pAlloc->SetProperties(pRequest, &Actual);
  if (FAILED(hr)) 
  {
    return hr;
  }

  // Is this allocator unsuitable?
  if (Actual.cbBuffer < pRequest->cbBuffer) 
  {
    return E_FAIL;
  }
  LOG4CPLUS_DEBUG(logger, "CTongshiDVBSourceStream::DecideBufferSize() - finished");

  return S_OK;
}

HRESULT CTongshiDVBSourceStream::Run(){
  LOG4CPLUS_INFO(logger, "CTongshiDVBSourceStream::Run()");
  if(m_dataPort=NULL){
    m_dataPort = TSDVB_CreateRawDataPort(m_sourceFilter->m_cardHandle);
    if(m_dataPort=NULL){
      LOG4CPLUS_ERROR(logger, "CTongshiDVBSourceStream::Run() - Data port creation failed");
      return S_FALSE;
    }
    if(TSDVB_OpenAllRawPID(m_dataPort)){
      LOG4CPLUS_INFO(logger, "CTongshiDVBSourceStream::Run() - Data port for all pids created");
    }else{
      LOG4CPLUS_ERROR(logger, "CTongshiDVBSourceStream::Run() - Data port creation failed");
      TSDVB_DeleteDataPort(m_dataPort);
      m_dataPort = NULL;
      return S_FALSE;
    }
  }
  if(m_filterState==FILTER_PAUSED){
    TSDVB_CleanDataPort(m_dataPort);
  }
  m_filterState=FILTER_RUNNING;
  LOG4CPLUS_INFO(logger, "CTongshiDVBSourceStream::Run() - finished");
  return CSourceStream::Run();
}

HRESULT CTongshiDVBSourceStream::Stop(){
  LOG4CPLUS_INFO(logger, "CTongshiDVBSourceStream::Stop()");
  if(m_dataPort=NULL){
    TSDVB_DeleteDataPort(m_dataPort);
    m_dataPort = NULL;
  }
  m_filterState=FILTER_STOPPED;
  LOG4CPLUS_INFO(logger, "CTongshiDVBSourceStream::Stop() - finished");

  return CSourceStream::Stop();
}
HRESULT CTongshiDVBSourceStream::Pause(){
  LOG4CPLUS_INFO(logger, "CTongshiDVBSourceStream::Pause()");
  if(m_filterState==FILTER_RUNNING){
    m_filterState=FILTER_PAUSED;
    LOG4CPLUS_INFO(logger, "CTongshiDVBSourceStream::Pause() - Set to pause");
  }
  if(m_filterState==FILTER_PAUSED){
    m_filterState=FILTER_RUNNING;
    TSDVB_CleanDataPort(m_dataPort);
    LOG4CPLUS_INFO(logger, "CTongshiDVBSourceStream::Pause() - Set to play --> Clean data port");
  }
  LOG4CPLUS_INFO(logger, "CTongshiDVBSourceStream::Pause() - finished");
  return CSourceStream::Pause ();
}

void CTongshiDVBSourceStream::ReconfigureLogging(wchar_t* appenderName){
  logger = Logger::getInstance(appenderName);
}


CTongshiDVBSource::CTongshiDVBSource(IUnknown *pUnk, HRESULT *phr)
: CSource(NAME("MediaPortal Tongshi DVB Source Filter"), pUnk, CLSID_TongshiDVBSource),
logger(Logger::getInstance(L"TongshiDVB"))
{
  // The pin magically adds itself to our pin array.
  m_stream = new CTongshiDVBSourceStream(phr, this);

  if (phr)
  {
    if (m_stream == NULL)
      *phr = E_OUTOFMEMORY;
    else
      *phr = S_OK;
  } 
  GetNumberOfInstalledCards();
}


CTongshiDVBSource::~CTongshiDVBSource()
{
  LOG4CPLUS_INFO(logger, "CTongshiDVBSource::dtor()");

  LOG4CPLUS_INFO(logger, "CTongshiDVBSource::dtor() completed");
}

STDMETHODIMP CTongshiDVBSource::NonDelegatingQueryInterface(REFIID riid, void** ppv)
{
  CheckPointer(ppv, E_POINTER);
  // Do we have this interface
  if (riid == IID_IGTvLibLogging)
  {
    return GetInterface((IGTvLibLogging*)this, ppv);
  }else if (riid == IID_IGTvLibSource)
  {
    return GetInterface((IGTvLibSource*)this, ppv);
  }else if (riid == IID_IGTvLibTuning)
  {
    return GetInterface((IGTvLibTuning*)this, ppv);
  }

  return CSource::NonDelegatingQueryInterface(riid, ppv);

}

CUnknown * WINAPI CTongshiDVBSource::CreateInstance(IUnknown *pUnk, HRESULT *phr)
{
  CTongshiDVBSource *pNewFilter = new CTongshiDVBSource(pUnk, phr );

  if (phr)
  {
    if (pNewFilter == NULL) 
      *phr = E_OUTOFMEMORY;
    else
      *phr = S_OK;
  }

  return pNewFilter;
}



STDMETHODIMP CTongshiDVBSource::TuneDVBT (FrequencySettings fSettings){
  CAutoLock lock(&m_Lock);
  LOG4CPLUS_INFO(logger,"TuneDVBT - Start");
  int result = ERROR_SUCCESS;
  try{
    if(m_cardHandle==NULL){
      return S_FALSE;
    }
    if(m_modType!=DVB_T){
      return S_FALSE;
    }
    TSDVB_TUNERPARAM  tuneParam;
    memset( &tuneParam, 0, sizeof(tuneParam) );
    tuneParam.m_nModulationType = DVB_T;
    tuneParam.m_dwFrequency = fSettings.Frequency*fSettings.Multiplier/1000;
    tuneParam.m_dwBandWidth=fSettings.Bandwidth*fSettings.Multiplier/1000;
    result =  TSDVB_Tune(m_cardHandle,&tuneParam);
  }catch(...){
    return S_FALSE;
  }
  LOG4CPLUS_INFO(logger,"TuneDVBT - Finish");
  if(result == ERROR_SUCCESS){
    return S_OK;
  }
  return S_FALSE;
}

STDMETHODIMP CTongshiDVBSource::TuneDVBS (FrequencySettings fSettings, DigitalDemodulator2Settings dSettings, LnbInfoSettings lSettings, DiseqcSatelliteSettings sSettings){
  CAutoLock lock(&m_Lock);
  LOG4CPLUS_INFO(logger,"TuneDVBS - Start");
  int result = ERROR_SUCCESS;
  try{
    if(m_cardHandle==NULL){
      return S_FALSE;
    }
    if(m_modType!=DVB_S){
      return S_FALSE;
    }
    TSDVB_TUNERPARAM  tuneParam;
    memset( &tuneParam, 0, sizeof(tuneParam) );
    tuneParam.m_nModulationType = DVB_S;
    tuneParam.m_dwFrequency = fSettings.Frequency*fSettings.Multiplier/1000;
    tuneParam.m_dwSymbalRate=dSettings.SymbolRate;
    tuneParam.m_DVB_S.m_dwLNBFreq1 = lSettings.LowOscillator*fSettings.Multiplier/1000;
    tuneParam.m_DVB_S.m_dwLNBFreq2 = lSettings.HighOscillator*fSettings.Multiplier/1000;
    tuneParam.m_DVB_S.m_dwUniversalSwichFreq = lSettings.LnbSwitchFrequency*fSettings.Multiplier/1000;
    tuneParam.m_DVB_S.m_nPolar = fSettings.Polarity==BDA_POLARISATION_LINEAR_H ? POLAR_HORIZONTAL : POLAR_VERTICAL;
    tuneParam.m_DVB_S.m_nTunerType = DVBS_TUNER_TYPE_UNIVERSAL;

    if(fSettings.Frequency*fSettings.Multiplier/1000 > lSettings.LnbSwitchFrequency*fSettings.Multiplier/1000){
      tuneParam.m_DVB_S.m_dwFlags = tuneParam.m_DVB_S.m_dwFlags & TSDVBS_FREQ22KHZ_ON;
    }
    if(sSettings.Enabled!=1){
      tuneParam.m_DVB_S.m_dwFlags = 0;
    }else{
      if(sSettings.ToneBurstEnabled==1){
        tuneParam.m_DVB_S.m_dwFlags = tuneParam.m_DVB_S.m_dwFlags & TSDVBS_TONE_BURSE_ON;
      }
    }
    result =  TSDVB_Tune(m_cardHandle,&tuneParam);
    if(result == ERROR_SUCCESS){
      if(sSettings.Enabled==1){
        BYTE par[4];
        ULONG responseId;
        if(sSettings.Diseq10Selection != BDA_LNB_SOURCE_NOT_SET){
          par[0] = 0xE0; par[1] = 0x10; par[2] = 0x38;
          par[3] = DISEQC_HIGH_NIBLE;
          par[3] |= (fSettings.Frequency*fSettings.Multiplier/1000>=lSettings.LnbSwitchFrequency) ? DISEQC_HIGH_BAND : DISEQC_LOW_BAND;
          par[3] |= (fSettings.Polarity==BDA_POLARISATION_LINEAR_V) ? DISEQC_VERTICAL : DISEQC_HORIZONTAL;
          par[3] |= (sSettings.Diseq10Selection==BDA_LNB_SOURCE_B) ? DISEQC_POSITION_B : ((sSettings.Diseq10Selection==BDA_LNB_SOURCE_D) ? DISEQC_POSITION_B : DISEQC_POSITION_A);
          par[3] |= (sSettings.Diseq10Selection==BDA_LNB_SOURCE_D) ? DISEQC_OPTION_B : ((sSettings.Diseq10Selection==BDA_LNB_SOURCE_C) ? DISEQC_POSITION_B : DISEQC_OPTION_A);
          result = SendDiseqCMessage(DiseqCMessageType_MessageOnly,4,par,&responseId);
          if (result != ERROR_SUCCESS)
          { 
            LOG4CPLUS_DEBUG(logger,"Error while sending DiseqC message - Multi case");
            return S_FALSE;
          }
        }
        if(sSettings.Diseq11Selection != DiseqC11Switch_NOT_SET){
          par[0] = 0xE0; // Framming byte = Command from Master no reply required, first transmission
          par[1] = 0x10; // Address byte = Any LNB switcher or SMATV
          par[2] = 0x39; // Command byte = Write port group 1
          par[3] = DISEQC_HIGH_NIBLE | sSettings.Diseq11Selection; // Data byte = Clear + Switch 1
          result = SendDiseqCMessage(DiseqCMessageType_MessageOnly,4,par,&responseId);
          if (result != ERROR_SUCCESS)
          { 
            LOG4CPLUS_DEBUG(logger,"Error while sending DiseqC message - Multi case");
            return S_FALSE;
          }
        }
      }
    }
  }catch(...){
    return S_FALSE;
  }
  LOG4CPLUS_INFO(logger,"TuneDVBS - Finish");
  if(result == ERROR_SUCCESS){
    return S_OK;
  }
  return S_FALSE;
}
STDMETHODIMP CTongshiDVBSource::TuneDVBC (FrequencySettings fSettings, DigitalDemodulatorSettings dSettings){
  CAutoLock lock(&m_Lock);
  LOG4CPLUS_INFO(logger,"TuneDVBC - Start");
  int result = ERROR_SUCCESS;
  try{
    if(m_cardHandle==NULL){
      return S_FALSE;
    }
    if(m_modType!=DVB_C){
      return S_FALSE;
    }
    TSDVB_TUNERPARAM  tuneParam;
    memset( &tuneParam, 0, sizeof(tuneParam) );
    tuneParam.m_nModulationType = DVB_C;
    tuneParam.m_dwFrequency = fSettings.Frequency*fSettings.Multiplier/1000;
    tuneParam.m_dwSymbalRate=dSettings.SymbolRate;
    ModulationType modul = dSettings.Modulation;
    switch(modul){
      case BDA_MOD_16QAM:
        tuneParam.m_DVB_C.m_nQAMType = QAM_16;
        break;
      case BDA_MOD_32QAM:
        tuneParam.m_DVB_C.m_nQAMType = QAM_32;
        break;
      case BDA_MOD_64QAM:
        tuneParam.m_DVB_C.m_nQAMType = QAM_64;
        break;
      case BDA_MOD_128QAM:
        tuneParam.m_DVB_C.m_nQAMType = QAM_128;
        break;
      case BDA_MOD_256QAM:
        tuneParam.m_DVB_C.m_nQAMType = QAM_256;
        break;
      default:
        tuneParam.m_DVB_C.m_nQAMType = QAM_AUTO_DETECT;
        break;
    }
    result =  TSDVB_Tune(m_cardHandle,&tuneParam);

  }catch(...){
    return S_FALSE;
  }
  LOG4CPLUS_INFO(logger,"TuneDVBC - Finish");
  if(result == ERROR_SUCCESS){
    return S_OK;
  }
  return S_FALSE;
}
STDMETHODIMP CTongshiDVBSource::TuneATSC (ULONG channelNumber, FrequencySettings fSettings, DigitalDemodulatorSettings dSettings){
  return S_FALSE;
}

STDMETHODIMP CTongshiDVBSource::GetAvailableTuningTypes (ULONG* tuningTypes){
  *tuningTypes = m_tuningType;
  LOG4CPLUS_DEBUG(logger,"GetAvailalbeTuningTypes "<<m_tuningType);
  return S_OK;
}

STDMETHODIMP CTongshiDVBSource::GetSignalStats(ULONG* tunerLocked, ULONG* signalPresent, LONG* signalQuality, LONG* signalLevel){
  CAutoLock lock(&m_Lock);
  LOG4CPLUS_INFO(logger,"Send DiseqC command - Start");
  try{
    if(m_cardHandle==NULL){
      return S_FALSE;
    }
    TSDVB_TUNER_SIGNAL_STATUS signalStatus;
    memset(&signalStatus,0,sizeof(signalStatus));
    BOOL b_tunerLocked = TSDVB_GetSignal(m_cardHandle,&signalStatus);
    *tunerLocked = b_tunerLocked ? 1 : 0;
    *signalQuality = signalStatus.m_nQualityPercent_x100;
    *signalLevel = signalStatus.m_nLevelPercent_x100;
    if (*signalQuality > 0 && *signalLevel > 0)
    {
      *signalPresent = 1;
    }
    else
    {
      *signalPresent = 0;
    }
  }catch(...){
    return S_FALSE;
  }
  LOG4CPLUS_INFO(logger,"Send DiseqC command - Finish");
  return S_OK;
}

STDMETHODIMP CTongshiDVBSource::SendDiseqCMessage(ULONG mode, ULONG len,byte* message,ULONG* responseId){
  CAutoLock lock(&m_Lock);
  if(mode==DiseqCMessageType_Response){
    return S_FALSE;
  }
  LOG4CPLUS_INFO(logger,"Send DiseqC command - Start");
  int result = ERROR_SUCCESS;
  try{
    if(m_cardHandle==NULL){
      return S_FALSE;
    }
    if(m_modType!=DVB_S){
      return S_FALSE;
    }
    result = TSDVB_SendDiSEqCCmdData(m_cardHandle,message,len);
  }catch(...){
    return S_FALSE;
  }
  LOG4CPLUS_INFO(logger,"Send DiseqC command - Finish");
  if(result == ERROR_SUCCESS){
    return S_OK;
  }
  return S_FALSE;
}

STDMETHODIMP CTongshiDVBSource::ReadDiseqCMessage(ULONG requestId, ULONG* len, byte* command){
  return S_FALSE;
}

STDMETHODIMP CTongshiDVBSource::SetDeviceIndex(ULONG deviceIndex)
{
  if(deviceIndex>=m_numberOfDevices){
    return S_FALSE;
  }
  if(m_bInit){
    LOG4CPLUS_DEBUG(logger,"Instance already initiated. Closing it");
    DeactivateCard();
  }
  return ActivateCard(deviceIndex);
}
STDMETHODIMP CTongshiDVBSource::ActivateCard (ULONG cardId) {
  CAutoLock lock(&m_Lock);
  LOG4CPLUS_DEBUG(logger,"CTongshiDVBSource::ActivateCard: "<<cardId);
  PMYDEV_ADAPTER handle;
  try{
    handle = TSDVB_OpenDevice(cardId);
    if(handle==NULL){
      LOG4CPLUS_ERROR(logger,"CTongshiDVBSource::ActivateCard failed");
      return S_FALSE;
    }
    if(TSDVB_RegisterAsMaster(handle,TSDVB_REGISTER_EXCLUSIVE)!=ERROR_SUCCESS){
      LOG4CPLUS_ERROR(logger,"CTongshiDVBSource::ActivateCard aborted, because can not register as master");
      TSDVB_CloseDevice(handle);
      handle = NULL;
      return S_FALSE;
    }
    m_modType = TSDVB_GetModulationType(handle);
    m_cardHandle = handle;
    m_bInit = true;
    LOG4CPLUS_DEBUG(logger,"Initiating device: "<<cardId<<" finished");
    m_iDeviceIndex = cardId;
    MODULATIONTYPE modType = TSDVB_GetModulationType(m_cardHandle);
    if(modType==DVB_C){
      m_tuningType = TuningType_DVBC;
    }else if(modType==DVB_S){
      m_tuningType = TuningType_DVBS;
    }else if(modType==DVB_T){
      m_tuningType = TuningType_DVBT;
    }

  }catch(...){
    if(handle!=NULL){
      TSDVB_CloseDevice(handle);
    }
    handle=NULL;
    return S_FALSE;
  }
  return S_OK;
}

STDMETHODIMP CTongshiDVBSource::DeactivateCard () {
  CAutoLock lock(&m_Lock);
  LOG4CPLUS_DEBUG(logger,"CTongshiDVBSource::DeactivateCard");
  try{
    if(m_cardHandle==NULL){
      LOG4CPLUS_DEBUG(logger,"CTongshiDVBSource::DeactivateCard - no card handle available");
      return S_FALSE;
    }
    TSDVB_CloseDevice(m_cardHandle);
    m_cardHandle = NULL;
  }catch(...){
    m_cardHandle = NULL;
  }
  LOG4CPLUS_DEBUG(logger,"CTongshiDVBSource::DeactivateCard - finish");
  return S_OK;
}

void CTongshiDVBSource::GetNumberOfInstalledCards() {
  CAutoLock lock(&m_Lock);
  m_numberOfDevices = 0;
  for (int i=0;i<TSDVB_MAX_ADAPTER_NUMBER;i++){
    int nOutLen = 0;
    char szBuf[100];
    if(TSDVB_GetAdapterName(i,szBuf,sizeof(szBuf),&nOutLen)){
      continue;
    }
    m_numberOfDevices++;
  }
}

STDMETHODIMP CTongshiDVBSource::GetNumberOfDevices (ULONG* numberOfDevices){
  LOG4CPLUS_DEBUG(logger,"Number of devices: "<<m_numberOfDevices);
  *numberOfDevices = m_numberOfDevices;
  return S_OK;
}

STDMETHODIMP CTongshiDVBSource::GetDevicePath (wchar_t** devicePath, wchar_t** deviceName) {
  CAutoLock lock(&m_Lock);
  LOG4CPLUS_DEBUG(logger,"CTongshiDVBSource::GetDevicePath");
  try{
    if(m_cardHandle==NULL){
      return S_FALSE;
    }
    int nOutLen = 0;
    char szBuf[100];
    if(TSDVB_GetAdapterName(m_iDeviceIndex,szBuf,sizeof(szBuf),&nOutLen)){
      return S_FALSE;
    }
    MultiByteToWideChar(CP_ACP,0,szBuf,nOutLen,*devicePath,nOutLen+1);
    MultiByteToWideChar(CP_ACP,0,szBuf,nOutLen,*deviceName,nOutLen+1);
  }catch(...){
    return S_FALSE;
  }
  LOG4CPLUS_DEBUG(logger,"CTongshiDVBSource::GetDevicePath - finished");
  return S_OK;
}

STDMETHODIMP CTongshiDVBSource::ConfigureLogging(wchar_t* logFile, wchar_t* identifier, LogLevelOption logLevel){
  logger = Logger::getInstance(identifier);
  if(logger.getAppender(identifier)==NULL){

    SharedAppenderPtr append_1(
      new SpecialDailyRollingFileAppender(logFile,DAILY,true,10));
    std::auto_ptr<Layout> myLayout = std::auto_ptr<Layout>(new
      log4cplus::PatternLayout(L"%d{%d.%m.%Y %H:%M:%S,%q} %-5p [%t]: %m%n"));

    append_1->setName(identifier);
    append_1->setLayout( myLayout );
    append_1->setThreshold(logLevel);
    logger.addAppender(append_1);
  }
  m_stream->ReconfigureLogging(identifier);
  return true;
}

STDMETHODIMP CTongshiDVBSource::IsDiseqSupported (ULONG* supported){
  if(m_tuningType != TuningType_DVBS){
    *supported = DiseqCMode_None;
  }else{
    *supported = DiseqCMode_ToneBurst | DiseqCMode_Simple | DiseqCMode_WriteAllCmd ;
  }
  return true;
}
