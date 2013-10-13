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
#include "IGTvLibHwPidFiltering.h"
#include <map>
#include <log4cplus/logger.h>
#include <log4cplus/loggingmacros.h>
#include "TSDataFilter.h"

using namespace std;
using namespace log4cplus;

class CTTPremiumSourceStream : public CSourceStream, public IGTvLibHwPidFiltering
{
public:
	// Constructor and destructor
	CTTPremiumSourceStream(CSource *pTTPremiumSource, HRESULT *phr);
	virtual ~CTTPremiumSourceStream();

	// Override the version that offers exactly one media type
	HRESULT GetMediaType(CMediaType *pMediaType);
	HRESULT DecideBufferSize(IMemAllocator *pAlloc, ALLOCATOR_PROPERTIES *pRequest);
	HRESULT FillBuffer(IMediaSample *pSample);
	HRESULT OnThreadCreate();
	HRESULT OnThreadDestroy();

	DECLARE_IUNKNOWN
	STDMETHOD (SetHwPids)(THIS_ ULONG pidCount, ULONG* pids, ULONG* pidTypes);

	void ReconfigureLogging(char* identifier);

private:
	void ClearFilterList();
	Logger logger;

	ULONG m_bufferSize;
	bool m_threadStarted;


	map<ULONG, TSDataFilter*> m_PIDToDataFilter;
};
