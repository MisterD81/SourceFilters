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
#include "CTTPremiumSourceStream.h"

//
// CTTPremiumSourceStream constructor
//
CTTPremiumSourceStream::CTTPremiumSourceStream(CSource* pTTPremiumSource, HRESULT *phr) :
m_threadStarted(false),
CSourceStream(NAME("TTPremium Source Output"), phr, pTTPremiumSource, L"OUTPUT"),
logger(Logger::getInstance("TTPremiumSource"))
{
	ASSERT(pTTPremiumSource);

	m_bufferSize = 1024 * 512; // 512 KB
}

//
// CTTPremiumSourceStream destructor
//
CTTPremiumSourceStream::~CTTPremiumSourceStream()
{
	LOG4CPLUS_DEBUG(logger,"CTTPremiumSourceStream dtor");
	ClearFilterList();
	LOG4CPLUS_DEBUG(logger,"CTTPremiumSourceStream dtor - finished");
}

//
// DecideBufferSize
//
// This has to be present to override the PURE virtual class base function
//
HRESULT CTTPremiumSourceStream::DecideBufferSize(IMemAllocator *pMemAllocator, ALLOCATOR_PROPERTIES * pRequest)
{
	HRESULT hr = S_OK;
	LOG4CPLUS_DEBUG(logger, "CTTPremiumSourceStream::DecideBufferSize() - request, cBuffers: "<<pRequest->cBuffers<<", cbBuffer: "
		<<pRequest->cbBuffer<<", cbAlign: "<< pRequest->cbAlign<<", cbPrefix: "<<pRequest->cbPrefix);
	if (m_bufferSize)
	{
		pRequest->cBuffers = 1;  
		pRequest->cbBuffer = m_bufferSize;
		pRequest->cbAlign = 1 ;  
		pRequest->cbPrefix = 0 ;  
		LOG4CPLUS_DEBUG(logger, "CTTPremiumSourceStream::DecideBufferSize() - setting new sizes, cBuffers: "<<pRequest->cBuffers<<", cbBuffer: "
			<<pRequest->cbBuffer<<", cbAlign: "<< pRequest->cbAlign<<", cbPrefix: "<<pRequest->cbPrefix);
		ALLOCATOR_PROPERTIES Actual;  
		hr = pMemAllocator->SetProperties(pRequest, &Actual);  
		if (FAILED(hr))
		{
			LOG4CPLUS_DEBUG(logger, "CTTPremiumSourceStream::DecideBufferSize() - failed to SetProperties - hr: "<<hr);
			return hr;
		}

		// Is this allocator unsuitable?
		if (Actual.cbBuffer < pRequest->cbBuffer) 
		{
			LOG4CPLUS_DEBUG(logger, "CTTPremiumSourceStream::DecideBufferSize() - unsuitable buffer ");
			return E_FAIL;
		}

	}
	LOG4CPLUS_DEBUG(logger, "CTTPremiumSourceStream::DecideBufferSize() - Result: "<<hr);
	return hr;
}

//
// Fill the sample buffer
//
HRESULT CTTPremiumSourceStream::FillBuffer(IMediaSample *pSample)
{
	// Don't do anything here as we actually asynchronously 
	// push data to the connected pin
	Sleep(1000);
	return NOERROR;
}

HRESULT CTTPremiumSourceStream::GetMediaType(__inout CMediaType *pMediaType) 
{
	LOG4CPLUS_DEBUG(logger, "CTTPremiumSourceStream::GetMediaType()");

	pMediaType->majortype = MEDIATYPE_Stream;
	pMediaType->subtype = MEDIASUBTYPE_MPEG2_TRANSPORT;

	return S_OK;
}


//
//
HRESULT CTTPremiumSourceStream::OnThreadCreate()
{
	m_threadStarted = true;
	LOG4CPLUS_DEBUG(logger, "CTTPremiumSourceStream::Run ");
	map<ULONG, TSDataFilter*>::const_iterator itr;
	TSDataFilter* filter;
	for(itr = m_PIDToDataFilter.begin(); itr != m_PIDToDataFilter.end(); ++itr){
		filter = (*itr).second;
		filter->Start();
	}
	HRESULT hr = CSourceStream::OnThreadCreate();
	LOG4CPLUS_DEBUG(logger, "CTTPremiumSourceStream::Run-Result "<<hr);
	return hr;
}


HRESULT CTTPremiumSourceStream::OnThreadDestroy()
{
	m_threadStarted = false;
	LOG4CPLUS_DEBUG(logger, "CTTPremiumSourceStream::Stop ");
	map<ULONG, TSDataFilter*>::const_iterator itr;
	TSDataFilter* filter;
	for(itr = m_PIDToDataFilter.begin(); itr != m_PIDToDataFilter.end(); ++itr){
		filter = (*itr).second;
		filter->Stop();
	}
	HRESULT hr = CSourceStream::OnThreadDestroy();
	LOG4CPLUS_DEBUG(logger, "CTTPremiumSourceStream::Stop-Result "<<hr);
	return hr;
}


void CTTPremiumSourceStream::ReconfigureLogging(char* identifier){
	logger = Logger::getInstance(identifier);
	map<ULONG, TSDataFilter*>::const_iterator itr;
	TSDataFilter* filter;
	for(itr = m_PIDToDataFilter.begin(); itr != m_PIDToDataFilter.end(); ++itr){
		filter = (*itr).second;
		filter->ReconfigureLogging(identifier);
	}
}

void CTTPremiumSourceStream::ClearFilterList(){
	LOG4CPLUS_DEBUG(logger,"Clear Filter List started");
	TSDataFilter* filter;
	for (map<ULONG, TSDataFilter*>::iterator it = m_PIDToDataFilter.begin(); it != m_PIDToDataFilter.end(); ++it)
	{
		filter = (*it).second;
		if(m_threadStarted == true){
			filter->Stop();
		}
		delete filter;
	}
	m_PIDToDataFilter.clear();
	LOG4CPLUS_DEBUG(logger,"Clear Filter List completed");
}

STDMETHODIMP CTTPremiumSourceStream::SetHwPids(THIS_ ULONG pidCount, ULONG* pids, ULONG* pidTypes){
	LOG4CPLUS_DEBUG(logger,"Setting new HW Pids");
	ClearFilterList();
	DVB_ERROR error = DVB_ERR_NONE;
	ULONG currentPid;
	ULONG pidType;
	TSDataFilter* filter;
	for(ULONG i=0;i<pidCount;i++){
		currentPid = pids[i];
		pidType = pidTypes[i];
		filter = new TSDataFilter(m_bufferSize,this,currentPid,pidType,logger.getName().c_str());
		if(m_threadStarted == true){
			filter->Start();
		}
		m_PIDToDataFilter.insert(make_pair(currentPid,filter));
		LOG4CPLUS_DEBUG(logger,"Added new Pid: "<<currentPid);
	}
	LOG4CPLUS_DEBUG(logger,"Setting new HW Pids done");
	return S_OK;
}
