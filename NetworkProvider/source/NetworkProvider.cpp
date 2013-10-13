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

#include <winsock2.h>
#include <ws2tcpip.h>
#include <initguid.h>
#include "networkprovider.h"

const AMOVIESETUP_MEDIATYPE outputPinTypes =
{
	&MEDIATYPE_NULL, &MEDIASUBTYPE_NULL
};

const AMOVIESETUP_PIN outputPins[] =
{
	{
		L"Output",					 // Pin string name
			FALSE,                   // Is it rendered
			TRUE,                    // Is it an output
			FALSE,                   // Allowed none
			FALSE,                   // Likewise many
			&CLSID_NULL,             // Connects to filter
			NULL,					 // Connects to pin
			1,                       // Number of types
			&outputPinTypes			 // Pin information
	}
};

const AMOVIESETUP_FILTER FilterInfo =
{
	&CLSID_GenTvLib_NetworkProvider,				// Filter CLSID
	L"Generic TvLibrary DVB Network Provider",    // String name
	MERIT_DO_NOT_USE,						// Filter merit
	1,										// Number pins
	outputPins								// Pin details
};


CFactoryTemplate g_Templates[1] = 
{
	{ 
		L"Generic TvLibrary DVB Network Provider",     // Name
			&CLSID_GenTvLib_NetworkProvider ,            // CLSID
			CNetworkProvider::CreateInstance,    // Method to create an instance of MyComponent
			NULL,                       // Initialization function
			&FilterInfo                 // Set-up information (for filters)
	}
};

int g_cTemplates = sizeof(g_Templates) / sizeof(g_Templates[0]);


CNetworkProviderFilter::CNetworkProviderFilter(CNetworkProvider *pNetworkProvider,
											   LPUNKNOWN pUnk,
											   CCritSec *pLock,
											   HRESULT *phr) :
CBaseFilter(NAME("Network Provider"), pUnk, pLock, CLSID_NetworkProvider),
m_pNetworkProvider(pNetworkProvider),
logger(Logger::getInstance(L"NetworkProvider"))
{
}


//
// GetPin
//
CBasePin * CNetworkProviderFilter::GetPin(int n)
{
	if (n == 0) {
		return m_pNetworkProvider->m_pNetworkProviderPin;
	}
	return NULL;
}


//
// GetPinCount
//
int CNetworkProviderFilter::GetPinCount()
{
	return 1;
}


//
// Stop
//
// Overriden to close the dump file
//
STDMETHODIMP CNetworkProviderFilter::Stop()
{
	CAutoLock cObjectLock(m_pLock);
	LOG4CPLUS_DEBUG(logger,"CNetworkProviderFilter::Stop()");
	HRESULT result =  CBaseFilter::Stop();
	LOG4CPLUS_DEBUG(logger,"CNetworkProviderFilter::Stop() completed");
	return result;
}


//
// Pause
//
// Overriden to open the dump file
//
STDMETHODIMP CNetworkProviderFilter::Pause()
{
	LOG4CPLUS_DEBUG(logger,"CNetworkProviderFilter::Pause()");
	CAutoLock cObjectLock(m_pLock);
	if (m_pNetworkProvider != NULL){
	}
	LOG4CPLUS_DEBUG(logger,"CNetworkProviderFilter::Pause() finished");
	return CBaseFilter::Pause();
}


//
// Run
//
// Overriden to open the dump file
//
STDMETHODIMP CNetworkProviderFilter::Run(REFERENCE_TIME tStart)
{
	LOG4CPLUS_DEBUG(logger,"CNetworkProviderFilter::Run()");
	CAutoLock cObjectLock(m_pLock);
	if(m_pNetworkProvider!=NULL){
		m_pNetworkProvider->Retune();
	}
	return CBaseFilter::Run(tStart);
}

CNetworkProvider::CNetworkProvider(LPUNKNOWN pUnk, HRESULT *phr) :
CUnknown(NAME("CNetwork Provider"), pUnk),
m_pFilter(NULL),
m_pNetworkProviderPin(NULL),
m_fid(0),
m_tuningType(TuningType_NOT_SET),
logger(Logger::getInstance(L"NetworkProvider"))
{
	m_pFilter = new CNetworkProviderFilter(this, GetOwner(), &m_Lock, phr);
	if (m_pFilter == NULL) 
	{
		if (phr)
			*phr = E_OUTOFMEMORY;
		return;
	}
	m_pNetworkProviderPin = new CDVBNetworkProviderPin(GetOwner(),
		m_pFilter,
		&m_Lock,
		phr);
	if (m_pNetworkProviderPin == NULL) {
		if (phr)
			*phr = E_OUTOFMEMORY;
		return;
	}
	m_pFrequencyFilter = new CFrequencyFilter();
	m_pDigitalDemodulatorFilter = new CDigitalDemodulatorFilter();
	m_pDigitalDemodulator2Filter = new CDigitalDemodulator2Filter();
	m_pLnbInfoFilter = new CLnbInfoFilter();
	m_pDiseqCommand = new CDiseqCommand();
	m_currentTuningType = TuningType_NOT_SET;
}

CNetworkProvider::~CNetworkProvider()
{
	LOG4CPLUS_INFO(logger,"CNetworkProvider::dtor()");
	delete m_pFilter;
	m_pFilter = NULL;
	delete m_pNetworkProviderPin;
	m_pNetworkProviderPin = NULL;
	delete m_pFrequencyFilter;
	delete m_pDigitalDemodulatorFilter;
	delete m_pDigitalDemodulator2Filter;
	delete m_pLnbInfoFilter;
	delete m_pDiseqCommand;
	LOG4CPLUS_INFO(logger,"CNetworkProviderFilter::dtor() completed");
}

CUnknown * WINAPI CNetworkProvider::CreateInstance(LPUNKNOWN punk, HRESULT *phr)
{
	ASSERT(phr);
	CNetworkProvider *pNewObject = new CNetworkProvider(punk, phr);
	if (pNewObject == NULL) {
		if (phr)
			*phr = E_OUTOFMEMORY;
	}
	return pNewObject;
}

STDMETHODIMP CNetworkProvider::NonDelegatingQueryInterface(REFIID riid, void ** ppv)
{
	CheckPointer(ppv,E_POINTER);
	CAutoLock lock(&m_Lock);

	if(riid == IID_IGTvLibLogging){
		return GetInterface((IGTvLibLogging*)this,ppv);
	}else if(riid == IID_IGTvLibTuning){
		return GetInterface((IGTvLibTuning*)this,ppv);
	}else if(riid == _uuidof(IBDA_NetworkProvider)){
		return GetInterface((IBDA_NetworkProvider*)this,ppv);
	}
	else if (riid == IID_IBaseFilter || riid == IID_IMediaFilter || riid == IID_IPersist) {
		return m_pFilter->NonDelegatingQueryInterface(riid, ppv);
	} 

	return CUnknown::NonDelegatingQueryInterface(riid, ppv);
} 

STDAPI DllRegisterServer()
{
	return AMovieDllRegisterServer2( TRUE );
} 

STDAPI DllUnregisterServer()
{
	return AMovieDllRegisterServer2( FALSE );
} 

extern "C" BOOL WINAPI DllEntryPoint(HINSTANCE, ULONG, LPVOID);

BOOL APIENTRY DllMain(HANDLE hModule, 
					  DWORD  dwReason, 
					  LPVOID lpReserved)
{
	return DllEntryPoint((HINSTANCE)(hModule), dwReason, lpReserved);
}


HRESULT CNetworkProvider::PutSignalSource(ULONG ulSignalSource) {
	LOG4CPLUS_DEBUG(logger,"CNETWORKPROVIDER::PUTSIGNALSOURCE "<<ulSignalSource);
	return S_OK;
}
HRESULT CNetworkProvider::GetSignalSource(ULONG* pulSignalSource) {
	LOG4CPLUS_DEBUG(logger,"CNETWORKPROVIDER::GETSIGNALSOURCE ");
	pulSignalSource = 0;
	return S_OK;
}
HRESULT CNetworkProvider::GetNetworkType(GUID* pguidNetworkType) {
	LOG4CPLUS_DEBUG(logger,"CNETWORKPROVIDER::GETNETWORKTYPE");
	pguidNetworkType = new GUID(GUID_NULL);
	return S_OK;
}
HRESULT CNetworkProvider::PutTuningSpace(REFGUID guidTuningSpace) {
	LOG4CPLUS_DEBUG(logger,"CNETWORKPROVIDER::PUTTUNINGSPACE");
	return S_OK;
}
HRESULT CNetworkProvider::GetTuningSpace(GUID* pguidTuingSpace) {
	LOG4CPLUS_DEBUG(logger,"CNETWORKPROVIDER::GETTUNINGSPACE");
	pguidTuingSpace = new GUID(GUID_NULL);
	return S_OK;
}
HRESULT CNetworkProvider::RegisterDeviceFilter(IUnknown* pUnkFilterControl,ULONG* ppvRegisitrationContext) {
	RegisterDevice* device;
	FILTER_INFO filterinfo;
	HRESULT hr;
	IBaseFilter* filter;
	LOG4CPLUS_INFO(logger,"RegisterDeviceFilter start");
	IMpeg2Demultiplexer* demux;
	if (m_pNetworkProviderPin->GetConnected() != NULL){
		hr = m_pNetworkProviderPin->CreateOutputPin(m_pNetworkProviderPin->GetConnected());
	}
	CAutoLock lock(&m_Lock);
	if (pUnkFilterControl == NULL){
		return E_POINTER;
	}
	if (&ppvRegisitrationContext == NULL ){
		return E_POINTER;
	}
	hr = pUnkFilterControl->QueryInterface(IID_IBaseFilter, (void**) &filter);
	if (hr != S_OK ){
		return S_FALSE;
	}
	hr = filter->QueryFilterInfo(&filterinfo);
	filterinfo.pGraph = NULL;
	if (hr != S_OK ){
		return S_FALSE;
	}
	LOG4CPLUS_DEBUG(logger,"RegisterDeviceFilter checks finished");
	m_fid = m_fid + 1;
	device = new RegisterDevice();
	device->id = m_fid;
	device->control = filter;
	wcstombs( device->name, filterinfo.achName, sizeof(device->name)); 
	ppvRegisitrationContext = &m_fid;
	hr = pUnkFilterControl->QueryInterface(IID_IMpeg2Demultiplexer, (void**) &demux);
	if (hr == S_OK ){
		LOG4CPLUS_DEBUG(logger,"RegisterDeviceFilter device prepared for registration - Name: "<<device->name<<" - "<<m_fid<<" - "<<&m_fid);
		device->control = NULL;
		delete device;
		LOG4CPLUS_INFO(logger,"RegisterDeviceFilter demux skipped");
		return S_FALSE;
	}else{
		LOG4CPLUS_INFO(logger,"RegisterDeviceFilter device prepared for registration - Name: "<<device->name<<" - "<<m_fid<<" - "<<&m_fid);
		IBDA_DeviceControl* ctrl;
		if( filter->QueryInterface(_uuidof(IBDA_DeviceControl), (void**) &ctrl)==S_OK){
			device->deviceControl=ctrl;
		}
		m_vecDevices.push_back(device);
		CheckCapabilities(filter);
	}
	LOG4CPLUS_DEBUG(logger,"RegisterDeviceFilter finished");
	return S_OK;
}

HRESULT CNetworkProvider::UnRegisterDeviceFilter(ULONG pvRegistrationContext) {
	CAutoLock lock(&m_Lock);
	LOG4CPLUS_INFO(logger,"UnregisterDeviceFlter "<<pvRegistrationContext);
	if (RemoveWithID(pvRegistrationContext)){ 
		LOG4CPLUS_INFO(logger,"UnregisterDeviceFlter "<<pvRegistrationContext<<" completed");
		return S_OK;
	}
	LOG4CPLUS_ERROR(logger,"UnregisterDeviceFlter "<<pvRegistrationContext<<" failed");
	return S_FALSE;
}


bool CNetworkProvider::RemoveWithID(ULONG id){
	RegisterDevice* device;
	LOG4CPLUS_INFO(logger,"RemoveWithID "<<id<<" requested");
	bool result = false;
	iVecDevices it = m_vecDevices.begin();
	while (it != m_vecDevices.end())
	{
		device = (*it);
		if(id==device->id){
			LOG4CPLUS_DEBUG(logger,"RemoveWithID "<<id<<" - "<<device->name<<" found --> removing");
			device->control = NULL;
			device->deviceControl = NULL;
			delete device;
			m_vecDevices.erase(it);
			result = true;
			break;
		}
		++it;
	}
	LOG4CPLUS_INFO(logger,"RemoveWithID "<<id<<" request finished");
	return result;
}

bool CNetworkProvider::ClearList(){
	bool result = true;
	try{
		RegisterDevice* device;
		LOG4CPLUS_DEBUG(logger,"Clear List");
		for (int i=0; i < (int)m_vecDevices.size();++i)
		{
			device = m_vecDevices[i];
			device->control = NULL;
			device->deviceControl = NULL;
			delete device;
		}
		m_vecDevices.clear();
	}catch(...) {
		result = false;
		LOG4CPLUS_DEBUG(logger,"Clear list failed");
	}
	LOG4CPLUS_DEBUG(logger,"Clear List finished");
	return result;

}

bool CNetworkProvider::CheckCapabilities(IBaseFilter* filter){
	IBDA_Topology* topology;
	LOG4CPLUS_DEBUG(logger,"CheckCapabilities - START");
	if(filter->QueryInterface(_uuidof(IBDA_Topology),  (void**) &topology) == S_OK){
		// Check For interfaces
		CheckInterfaces(topology);
		// Check for tuning Types
		CheckTuningType(topology);
	}
	LOG4CPLUS_DEBUG(logger,"CheckCapabilities - FINISHED");
	return S_OK;
}

bool CNetworkProvider::CheckInterfaces(IBDA_Topology* topology){
	LOG4CPLUS_DEBUG(logger,"CheckInterfaces - START");
	bool result = true;
	ULONG nodetypes;
	ULONG nodetypelist[10];
	ULONG nodeinterfaces;
	GUID nodeinterfaceslist[10];
	IUnknown* bda;
	try{
		if(topology->GetNodeTypes(&nodetypes,10,nodetypelist) == S_OK){
			if(nodetypes > 0){
				for(UINT i=0;i<nodetypes;++i){
					if(topology->GetNodeInterfaces(nodetypelist[i], &nodeinterfaces,10,nodeinterfaceslist) == S_OK){
						if(nodeinterfaces > 0 ){
							for(UINT c=0;c<nodeinterfaces;++c){
								GUID guid = nodeinterfaceslist[c];

								LOG4CPLUS_DEBUG(logger,"Descriptor: "<< guid.Data1<<"-"<< guid.Data2<<"-"<< guid.Data3<<"-"<<
									guid.Data4[0]<<"-"<< guid.Data4[1]<<"-"<< guid.Data4[2]<<"-"<<
									guid.Data4[3]<<"-"<< guid.Data4[4]<<"-"<< guid.Data4[5]<<"-"<<
									guid.Data4[6]<<"-"<< guid.Data4[7]);
								if(CheckSpecificInterface(_uuidof(IBDA_FrequencyFilter),topology,nodeinterfaceslist[c],nodetypelist[i],&bda)){
									LOG4CPLUS_DEBUG(logger,"CheckInterfaces found FREQUENCYFILTER");
									m_pFrequencyFilter->AddFilter(bda);
									continue;
								}
								if(CheckSpecificInterface(_uuidof(IBDA_DigitalDemodulator),topology,nodeinterfaceslist[c],nodetypelist[i],&bda)){
									LOG4CPLUS_DEBUG(logger,"CheckInterfaces found DIGITALDEMODULATOR");
									if(CheckSpecificInterface(_uuidof(IBDA_DigitalDemodulator2),topology,nodeinterfaceslist[c],nodetypelist[i],&bda)){
										LOG4CPLUS_DEBUG(logger,"CheckInterfaces found DIGITALDEMODULATOR2");
										m_pDigitalDemodulator2Filter->AddFilter(bda);
									}else{
										m_pDigitalDemodulatorFilter->AddFilter(bda);
									}
									continue;
								}
								if(CheckSpecificInterface(_uuidof(IBDA_LNBInfo),topology,nodeinterfaceslist[c],nodetypelist[i],&bda)){
									LOG4CPLUS_DEBUG(logger,"CheckInterfaces found LNBINFO");
									m_pLnbInfoFilter->AddFilter(bda);
									continue;
								}
								if(CheckSpecificInterface(_uuidof(IBDA_DiseqCommand),topology,nodeinterfaceslist[c],nodetypelist[i],&bda)){
									LOG4CPLUS_DEBUG(logger,"CheckInterfaces found DISEQCOMMAND");
									m_pDiseqCommand->AddFilter(bda);
									continue;
								}
								if(CheckSpecificInterface(_uuidof(IBDA_SignalStatistics),topology,nodeinterfaceslist[c],nodetypelist[i],&bda)){
									LOG4CPLUS_DEBUG(logger,"CheckInterfaces found SIGNALSTATISTICS");
									m_signalStatistics.push_back(bda);
									continue;
								}
							}
						}
					}
				}
			}
		}
	}catch(...){
		result = false;
	}
	LOG4CPLUS_DEBUG(logger,"CheckInterfaces - FINISHED");
	return result;
}

bool CNetworkProvider::CheckSpecificInterface(GUID requiredGUID, IBDA_Topology* topology, GUID testGUID,ULONG elementList,IUnknown** element){
	IUnknown* unk;
	try{
		if(IsEqualGUID(testGUID,requiredGUID)){
			if(topology->GetControlNode(0,1,elementList,&unk) == S_OK){
				if(unk->QueryInterface(requiredGUID,(void**) element) == S_OK){
					LOG4CPLUS_DEBUG(logger,"CheckSpecificInterface found element");
					return true;
				}
			}
		}
	}catch(...){
		*element = NULL;
	}
	unk = NULL;
	return false;
}

bool CNetworkProvider::CheckTuningType(IBDA_Topology* topology){
	LOG4CPLUS_DEBUG(logger,"CheckTuningType - START");
	bool result = true;
	try{
		ULONG nodeDescriptor;
		BDANODE_DESCRIPTOR nodeDescriptorList[10];
		if(topology->GetNodeDescriptors ( &nodeDescriptor,10,nodeDescriptorList) == S_OK){
			for(UINT c=0;c<nodeDescriptor;++c){
				GUID guid = nodeDescriptorList[c].guidFunction;

				LOG4CPLUS_DEBUG(logger,"Descriptor: "<< guid.Data1<<"-"<< guid.Data2<<"-"<< guid.Data3<<"-"<<
					guid.Data4[0]<<"-"<< guid.Data4[1]<<"-"<< guid.Data4[2]<<"-"<<
					guid.Data4[3]<<"-"<< guid.Data4[4]<<"-"<< guid.Data4[5]<<"-"<<
					guid.Data4[6]<<"-"<< guid.Data4[7]);

				if(IsEqualGUID(KSNODE_BDA_RF_TUNER,nodeDescriptorList[c].guidFunction)){
					LOG4CPLUS_DEBUG(logger,"CheckTuningType found Analog (RF_TUNER)");
					m_tuningType += TuningType_AnalogRfTuner;
				}
				if(IsEqualGUID(KSNODE_BDA_ANALOG_DEMODULATOR,nodeDescriptorList[c].guidFunction)){
					LOG4CPLUS_DEBUG(logger,"CheckTuningType found Analog");
					m_tuningType += TuningType_AnalogTv;
				}
				if(IsEqualGUID(KSNODE_BDA_8VSB_DEMODULATOR,nodeDescriptorList[c].guidFunction)){
					LOG4CPLUS_DEBUG(logger,"CheckTuningType found ATSC");
					m_tuningType += TuningType_ATSC;
					if((m_tuningType & TuningType_DVBC)==TuningType_DVBC){
						LOG4CPLUS_DEBUG(logger,"CheckTuningType found ATSC (QAM) - Removing DVBC");
						m_tuningType -= TuningType_DVBC;
					}
				}
				if(IsEqualGUID(KSNODE_BDA_COFDM_DEMODULATOR,nodeDescriptorList[c].guidFunction)){
					LOG4CPLUS_DEBUG(logger,"CheckTuningType found DVB-T");
					m_tuningType += TuningType_DVBT;
				}
				if(IsEqualGUID(KSNODE_BDA_QPSK_DEMODULATOR,nodeDescriptorList[c].guidFunction)){
					LOG4CPLUS_DEBUG(logger,"CheckTuningType found DVB-S");
					m_tuningType += TuningType_DVBS;
				}
				if(IsEqualGUID(KSNODE_BDA_8PSK_DEMODULATOR,nodeDescriptorList[c].guidFunction)){
					LOG4CPLUS_DEBUG(logger,"CheckTuningType found DVB-S2");
					m_tuningType += TuningType_DVBS2;
				}
				if(IsEqualGUID(KSNODE_BDA_QAM_DEMODULATOR,nodeDescriptorList[c].guidFunction)){
					if(( m_tuningType & TuningType_ATSC)!= TuningType_ATSC){
						LOG4CPLUS_DEBUG(logger,"CheckTuningType found DVB-C");
						m_tuningType += TuningType_DVBC;
					}else{
						LOG4CPLUS_DEBUG(logger,"CheckTuningType found ATSC (QAM)");
					}
				}
				if(IsEqualGUID(KSNODE_BDA_ISDB_T_DEMODULATOR,nodeDescriptorList[c].guidFunction)){
					LOG4CPLUS_DEBUG(logger,"CheckTuningType found ISDB-S");
					m_tuningType += TuningType_ISDBS;
				}
				if(IsEqualGUID(KSNODE_BDA_ISDB_T_DEMODULATOR,nodeDescriptorList[c].guidFunction)){
					LOG4CPLUS_DEBUG(logger,"CheckTuningType found ISDB-T");
					m_tuningType += TuningType_ISDBS;
				}
			}
		}
	}catch(...){
		result = false;
	}
	LOG4CPLUS_DEBUG(logger,"CheckTuningType - FINISED");
	return result;
}


bool CNetworkProvider::StartChanges(){
	HRESULT hr = S_FALSE;
	try{	
		IBDA_DeviceControl* ctrl;
		LOG4CPLUS_DEBUG(logger,"StartChanges searching for IBDA_DeviceControl");
		for (int i=0; i < (int)m_vecDevices.size();++i)
		{
			ctrl = m_vecDevices[i]->deviceControl;
			if(ctrl!=NULL){
				hr = ctrl->StartChanges();
				LOG4CPLUS_DEBUG(logger,"StartChanges done - Result "<<hr);
			}
		}
	}catch(...){
		LOG4CPLUS_ERROR(logger,"StartChanges failed");
		hr = S_FALSE;
	}
	LOG4CPLUS_DEBUG(logger,"StartChanges finished");
	return hr==S_OK;
}

bool CNetworkProvider::CommitChanges(){
	HRESULT hr = S_FALSE;
	try{
		IBDA_DeviceControl* ctrl;
		LOG4CPLUS_DEBUG(logger,"CommitChanges searching for IBDA_DeviceControl");
		for (int i=0; i < (int)m_vecDevices.size();++i)
		{
			ctrl = m_vecDevices[i]->deviceControl;
			if(ctrl!=NULL){
				hr = ctrl->CheckChanges();
				LOG4CPLUS_DEBUG(logger,"CommitChanges CheckResult "<<hr);
				hr = ctrl->CommitChanges();
				LOG4CPLUS_DEBUG(logger,"CommitChanges done - Result "<<hr);
			}
		}
	}catch(...){
		LOG4CPLUS_ERROR(logger,"CommitChanges failed");
		hr = S_FALSE;
	}
	LOG4CPLUS_DEBUG(logger,"CommitChanges finished");
	return hr == S_OK;
}


STDMETHODIMP CNetworkProvider::TuneDVBT (FrequencySettings settings){
	bool result;
	CAutoLock lock(&m_Lock);
	LOG4CPLUS_INFO(logger,"TuneDVBT - Start");
	if((m_tuningType & TuningType_DVBT) != TuningType_DVBT){
		return S_FALSE;
	}
	m_currentTuningType = TuningType_DVBT;
	m_pFrequencyFilter->SetTuningInformation(settings);
	result = StartChanges();
	if(result){
		result = m_pFrequencyFilter->Tune();
		result = CommitChanges();
	}
	LOG4CPLUS_INFO(logger,"TuneDVBT - Finish");
	if(result || m_pFilter->IsStopped()){
		return S_OK;
	}
	return S_FALSE;
}


STDMETHODIMP CNetworkProvider::TuneDVBS (FrequencySettings fSettings, DigitalDemodulator2Settings dSettings, LnbInfoSettings lSettings, DiseqcSatelliteSettings sSettings){
	bool result;
	CAutoLock lock(&m_Lock);
	LOG4CPLUS_INFO(logger,"TuneDVBS - Start");
	if((m_tuningType & TuningType_DVBS) != TuningType_DVBS){
		return S_FALSE;
	}
	m_currentTuningType = TuningType_DVBS;
	m_pFrequencyFilter->SetTuningInformation(fSettings);
	m_pDigitalDemodulatorFilter->SetTuningInformation(dSettings);
	m_pDigitalDemodulator2Filter->SetTuningInformation(dSettings);
	m_pLnbInfoFilter->SetTuningInformation(lSettings);
	m_pDiseqCommand->SetTuningInformation(sSettings);
	result = StartChanges();
	if(result){
		result = m_pFrequencyFilter->Tune();
		if(result){
			result = m_pDigitalDemodulatorFilter->Tune();
		}
		if(result){
			result = m_pDigitalDemodulator2Filter->Tune();
		}
		if(result){
			result = m_pLnbInfoFilter->Tune();
		}
		if(result){
			result = m_pDiseqCommand->Tune();
		}
		result = CommitChanges();
	}
	LOG4CPLUS_INFO(logger,"TuneDVBS - Finish");
	if(result || m_pFilter->IsStopped()){
		return S_OK;
	}
	return S_FALSE;
}

STDMETHODIMP CNetworkProvider::TuneDVBC (FrequencySettings fSettings, DigitalDemodulatorSettings dSettings){
	bool result;
	CAutoLock lock(&m_Lock);
	LOG4CPLUS_INFO(logger,"TuneDVBC - Start");
	if((m_tuningType & TuningType_DVBC) != TuningType_DVBC){
		return S_FALSE;
	}
	m_currentTuningType = TuningType_DVBC;
	m_pFrequencyFilter->SetTuningInformation(fSettings);
	m_pDigitalDemodulatorFilter->SetTuningInformation(dSettings);
	result = StartChanges();
	if(result){
		result = m_pFrequencyFilter->Tune();
		if(result){
			result = m_pDigitalDemodulatorFilter->Tune();
		}

		result = 	CommitChanges();
	}
	LOG4CPLUS_INFO(logger,"TuneDVBC - Finish");
	if(result || m_pFilter->IsStopped()){
		return S_OK;
	}
	return S_FALSE;
}

STDMETHODIMP CNetworkProvider::TuneATSC (ULONG channelNumber, FrequencySettings fSettings, DigitalDemodulatorSettings dSettings){
	bool result;
	CAutoLock lock(&m_Lock);
	LOG4CPLUS_INFO(logger,"TuneATSC - Start");
	if((m_tuningType & TuningType_ATSC) != TuningType_ATSC){
		return S_FALSE;
	}
	m_currentTuningType = TuningType_ATSC;
	ULONG frequency = fSettings.Frequency;
	ULONG multiplier = fSettings.Multiplier;
	if(channelNumber!=0 && frequency == -1 && multiplier == -1){
		// Convert channel number into frequency
		if(channelNumber==1){
			frequency = 47;
		}else if(channelNumber >=2 && channelNumber<=6){
			frequency = 57 + ((channelNumber-2)* 6);
		}else if (channelNumber >= 7 && channelNumber <=13){
			frequency = 177 + ((channelNumber-7)*6);
		}else if(channelNumber >=14){
			frequency = 473 + ((channelNumber - 14) * 6);
		}
		multiplier = 1000000;
	}
	fSettings.Frequency = frequency;
	fSettings.Multiplier = multiplier;
	m_pFrequencyFilter->SetTuningInformation(fSettings);
	m_pDigitalDemodulatorFilter->SetTuningInformation(dSettings);
	result = StartChanges();
	if(result){
		result = m_pFrequencyFilter->Tune();
		if(result){
			result = m_pDigitalDemodulatorFilter->Tune();
		}
		result = CommitChanges();
	}
	LOG4CPLUS_INFO(logger,"TuneATSC - Finish");
	if(result || m_pFilter->IsStopped()){
		return S_OK;
	}
	return S_FALSE;
}



bool CNetworkProvider::Retune(){
	if(m_currentTuningType == TuningType_NOT_SET){
		LOG4CPLUS_INFO(logger,"RETUNING SKIPPED AS NO TUNING TYPE AVAILABLE");
		return true;
	}
	LOG4CPLUS_INFO(logger,"PERFORM TUNING ON START - START");
	bool result = StartChanges();
	if(result){
		m_pFrequencyFilter->Tune();
		if(m_currentTuningType != TuningType_DVBT){
			m_pDigitalDemodulatorFilter->Tune();
		}
		if(m_currentTuningType == TuningType_DVBS){
			m_pDigitalDemodulator2Filter->Tune();
			m_pLnbInfoFilter->Tune();
			m_pDiseqCommand->Tune();
		}
		result = CommitChanges();
		LOG4CPLUS_INFO(logger,"PERFORM TUNING ON START - FINISHED");
	}
	return result;
}

STDMETHODIMP CNetworkProvider::GetAvailableTuningTypes (ULONG* tuningTypes){
	*tuningTypes = m_tuningType;
	LOG4CPLUS_DEBUG(logger,"GetAvailalbeTuningTypes "<<m_tuningType);
	return S_OK;
}

STDMETHODIMP CNetworkProvider::GetSignalStats (ULONG* tunerLocked, ULONG* signalPresent, LONG* signalQuality, LONG* signalLevel){
	HRESULT hr = S_OK;
	try{
		IBDA_SignalStatistics* signalStatistcsFilter;
		if (m_signalStatistics.size() > 0) {
			for (UINT i = 0; i< m_signalStatistics.size() ; ++i){
				if(m_signalStatistics[i]->QueryInterface(_uuidof(IBDA_SignalStatistics),(void**) &signalStatistcsFilter) == S_OK){
					BOOLEAN tmp;
					hr = signalStatistcsFilter->get_SignalPresent(&tmp);
					if(tmp==TRUE){
						*signalPresent=1;
					}else{
						*signalPresent=0;
					}

					hr = signalStatistcsFilter->get_SignalLocked(&tmp);
					if(tmp==TRUE){
						*tunerLocked=1;
					}else{
						*tunerLocked=0;
					}
					hr = signalStatistcsFilter->get_SignalQuality(signalQuality);
					hr = signalStatistcsFilter->get_SignalStrength(signalLevel);

				}
			}
		}	
	}catch(...){
		hr = S_FALSE;
		LOG4CPLUS_ERROR(logger,"GetSignalStats failed");
	}
	return hr;
}

STDMETHODIMP CNetworkProvider::IsDiseqSupported (ULONG* supported){
  if(m_tuningType != TuningType_DVBS){
    *supported = DiseqCMode_None;
  }else{
    if(m_pDiseqCommand->IsDiseqSupported()){
      *supported = DiseqCMode_ToneBurst | DiseqCMode_Simple | DiseqCMode_WriteAllCmd | DiseqCMode_ReadAllCmd;
    }else{
      *supported = DiseqCMode_ToneBurst | DiseqCMode_Simple;
    }
  }

  return S_OK;
}

STDMETHODIMP CNetworkProvider::ConfigureLogging(wchar_t* logFile, wchar_t* identifier, LogLevelOption logLevel){
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
	m_pDigitalDemodulator2Filter->ReconfigureLogging(identifier);
	m_pDigitalDemodulatorFilter->ReconfigureLogging(identifier);
	m_pDiseqCommand->ReconfigureLogging(identifier);
	m_pFrequencyFilter->ReconfigureLogging(identifier);
	m_pLnbInfoFilter->ReconfigureLogging(identifier);
	m_pNetworkProviderPin->ReconfigureLogging(identifier);
	return S_OK;
}

STDMETHODIMP CNetworkProvider::SendDiseqCMessage(ULONG mode, ULONG len,byte* message,ULONG* responseId){
	if(m_currentTuningType != TuningType_DVBS && m_currentTuningType != TuningType_DVBS2){
		LOG4CPLUS_INFO(logger,"SENDING DISEQC MESSAGE NOT AVAILABLE SINCE NO DVB-S/S2 TUNING TYPE");
		return false;
	}
	return m_pDiseqCommand->SendDiseqCMessage(mode,len,message,responseId);
}

STDMETHODIMP CNetworkProvider::ReadDiseqCMessage(ULONG requestId, ULONG* len, byte* command){
	if(m_currentTuningType != TuningType_DVBS && m_currentTuningType != TuningType_DVBS2){
		LOG4CPLUS_INFO(logger,"READING DISEQC MESSAGE NOT AVAILABLE SINCE NO DVB-S/S2 TUNING TYPE");
		return false;
	}
	return m_pDiseqCommand->ReadDiseqCMessage(requestId, len, command);

}