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

#include "OutPutPin.h"


CDVBNetworkProviderPin::CDVBNetworkProviderPin(LPUNKNOWN pUnk, CBaseFilter *pFilter, CCritSec* pLock,HRESULT *phr) :
CBaseOutputPin(NAME("CDVBNetworkProviderPin"), pFilter, pLock, phr, L"Output"),
m_pCritSection(pLock),
logger(Logger::getInstance(L"NetworkProvider"))
{
	LOG4CPLUS_DEBUG(logger,"CVBNetworkProviderPin::ctor");
}

CDVBNetworkProviderPin::~CDVBNetworkProviderPin()
{
	LOG4CPLUS_DEBUG(logger,"CVBNetworkProviderPin::dtor");
}

HRESULT CDVBNetworkProviderPin::CheckMediaType (const CMediaType* pmt){
	LOG4CPLUS_INFO(logger,"CVBNetworkProviderPin::CheckMediaType");

	CheckPointer(pmt, E_POINTER);

	LOG4CPLUS_INFO(logger,"CVBNetworkProviderPin::CheckMediaType - Check GUID");
	if(IsEqualGUID(pmt->majortype,KSDATAFORMAT_TYPE_BDA_ANTENNA)){
		LOG4CPLUS_INFO(logger,"CVBNetworkProviderPin::CheckMediaType - Finished");
		return S_OK;
	}
	return VFW_E_INVALIDMEDIATYPE;
}


HRESULT CDVBNetworkProviderPin::CompleteConnect(IPin* receivePin){
	LOG4CPLUS_INFO(logger,"CVBNetworkProviderPin::CompleteConnect");
	CheckPointer(receivePin, E_POINTER);
	int hr = CreateOutputPin(receivePin);

	if(hr==S_OK){
		LOG4CPLUS_INFO(logger,"CVBNetworkProviderPin::CompleteConnect - Finish");
		return CBaseOutputPin::CompleteConnect(receivePin);
	}else{
		return hr;
	}
}

HRESULT CDVBNetworkProviderPin::Run(REFERENCE_TIME start){

	LOG4CPLUS_INFO(logger,"CVBNetworkProviderPin::Run");
	IPin* pin = GetConnected();
	CheckPointer(pin,E_FAIL);
	int hr = CreateOutputPin(pin);

	if(hr==S_OK){
		LOG4CPLUS_INFO(logger,"CVBNetworkProviderPin::Run - Finish");
		return CBaseOutputPin::Run(start);
	}else{
		return hr;
	}
}

HRESULT CDVBNetworkProviderPin::DecideBufferSize(IMemAllocator *pAlloc, ALLOCATOR_PROPERTIES *pRequest)
{
	LOG4CPLUS_DEBUG(logger,"CVBNetworkProviderPin::DecideBufferSize");
	HRESULT hr;
	CheckPointer(pAlloc, E_POINTER);
	CheckPointer(pRequest, E_POINTER);

	if (pRequest->cBuffers == 0)
	{
		pRequest->cBuffers = 30;
	}

	pRequest->cbBuffer = 256000;

	ALLOCATOR_PROPERTIES Actual;
	hr = pAlloc->SetProperties(pRequest, &Actual);
	if (FAILED(hr))
	{
		return hr;
	}

	if (Actual.cbBuffer < pRequest->cbBuffer)
	{
		return E_FAIL;
	}
	LOG4CPLUS_DEBUG(logger,"CVBNetworkProviderPin::DecideBufferSize - Finish");
	return S_OK;
}


HRESULT CDVBNetworkProviderPin::CreateOutputPin(IPin* pin){
	PIN_INFO pin_info;
	ULONG pinIn;
	ULONG pinOut;
	ULONG mappings[25];
	ULONG num_maps;
	IBDA_Topology* topology;
	IEnumPins* enum_pins;
	IPin* tempPin;
	HRESULT result;
	LOG4CPLUS_INFO(logger,"CDVBNetworkProviderPin::CreateOutputPin");
	result = pin->QueryPinInfo(&pin_info);
	LOG4CPLUS_DEBUG(logger,"CDVBNetworkProviderPin::CreateOutputPin - QueryPinInfo");

	pinIn = 0;
	pinOut = 1;
	topology = NULL;
	LOG4CPLUS_DEBUG(logger,"CDVBNetworkProviderPin::CreateOutputPin - Setting pinIn to 0 and pinOut to 1");

	if(result==S_OK){
		if(pin_info.pFilter != NULL){
			result = pin_info.pFilter->QueryInterface(_uuidof(IBDA_Topology), (void**) &topology);
			LOG4CPLUS_DEBUG(logger,"CDVBNetworkProviderPin::CreateOutputPin - QueryInterface for IBDA_Topology");
			if(result==S_OK){
				result = topology->GetPinTypes(&num_maps, 25, mappings);
				if(result==S_OK){
					if (num_maps > 1){
						pinIn = mappings[0];
						pinOut = mappings[1];
					}
					result = pin_info.pFilter->EnumPins(&enum_pins);
					LOG4CPLUS_DEBUG(logger,"CDVBNetworkProviderPin::CreateOutputPin - EnumPins");
					if(result==S_OK){
						result = enum_pins->Next(1, &tempPin, NULL);
						LOG4CPLUS_DEBUG(logger,"CDVBNetworkProviderPin::CreateOutputPin - next");
						if(result!=S_OK){
							result = topology->CreatePin(mappings[0], &pinIn);
							LOG4CPLUS_DEBUG(logger,"CDVBNetworkProviderPin::CreateOutputPin - CreatePin with ID of "<<pinIn);
						}
            tempPin->Release();
						tempPin = NULL;
						if (result == S_OK ){
							result = enum_pins->Next(1, &tempPin, NULL);
							LOG4CPLUS_DEBUG(logger,"CDVBNetworkProviderPin::CreateOutputPin - next");
							if(result!=S_OK){
								result = topology->CreatePin(mappings[1], &pinOut);
								LOG4CPLUS_DEBUG(logger,"CDVBNetworkProviderPin::CreateOutputPin - CreatePin with ID of "<<pinOut);
							}
              tempPin->Release();
							tempPin = NULL;
						}
					}
				}
			}
		}
	}

	if(topology!=NULL && result == S_OK){
		result = topology->CreateTopology(pinIn, pinOut);
		LOG4CPLUS_DEBUG(logger,"CDVBNetworkProviderPin::CreateOutputPin - CreateTopology");
	}else{
		result = E_FAIL;
		LOG4CPLUS_ERROR(logger,"CDVBNetworkProviderPin::CreateOutputPin - FAIL");
	}
	return result;
}

void CDVBNetworkProviderPin::ReconfigureLogging(wchar_t* appenderName){
  logger = Logger::getInstance(appenderName);
}

