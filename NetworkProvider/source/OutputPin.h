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

#include <winsock2.h>
#include <ws2tcpip.h>
#include <streams.h>
#include <KS.h>
#include <KSMEDIA.h>
#include <BDATYPES.h>
#include <BDAMedia.h>
#include <bdaiface.h>
#include <log4cplus/logger.h>
#include <log4cplus/configurator.h>
#include <log4cplus/loggingmacros.h>

using namespace log4cplus;


class CDVBNetworkProviderPin : public CBaseOutputPin{
	CCritSec* const	m_pCritSection;		    // Sample critical section

public:
	CDVBNetworkProviderPin(LPUNKNOWN pUnk, CBaseFilter *pFilter, CCritSec* pLock, HRESULT *phr);
	~CDVBNetworkProviderPin();

	HRESULT CheckMediaType(const CMediaType* pmt);
	HRESULT CompleteConnect(IPin* receivePin);
	HRESULT Run(REFERENCE_TIME start);
	HRESULT DecideBufferSize(IMemAllocator *pAlloc, ALLOCATOR_PROPERTIES *pRequest);

	HRESULT CreateOutputPin(IPin* pin);
  void ReconfigureLogging(wchar_t* appenderName);
private:
	Logger logger;
};