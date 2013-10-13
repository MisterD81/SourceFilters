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

#include "DVBTSFilter.h"
#include <log4cplus/logger.h>
#include <log4cplus/loggingmacros.h>


using namespace log4cplus;

class CTTPremiumSourceStream;

class TSDataFilter : public CDVBTSFilter
{
public:
	TSDataFilter(int bufferSize, CTTPremiumSourceStream* outputPin, ULONG pid, ULONG pidType, const char* appenderName);
	virtual ~TSDataFilter();

	void Start();
	void Stop();

	void ReconfigureLogging(char* appenderName);

protected:
	void OnDataArrival(BYTE* Buff, int len);

private:
	HRESULT ConfigureFilter();

	bool					m_isRunning;
	int						m_bufferSize;
	ULONG m_activePid;
	ULONG m_activePidType;
	CTTPremiumSourceStream* m_outputPin;
	CCritSec 		m_Lock;
	BYTE m_packetCounter;
	Logger logger;
	byte* m_nullByte;
	bool m_sectionFilterHandling;

};