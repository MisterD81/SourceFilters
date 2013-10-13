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
#include "TSDataFilter.h"

#define PREMIUM_PACKET_LENGTH 0xB8
#define TS_PACKET_LENGTH 0xBC
#define TS_HEADER_LENGTH 0x4
#define TS_HEADER_SYNC_BYTE 0x47
#define TS_HEADER_PAYLOAD_START 0x40
#define TS_HEADER_TRANSPORT_PRIO 0x20
#define TS_HEADER_PAYLOAD_ONLY 0x10
#define TS_HEADER_CONTINUITY_MAX_VALUE 0x10

#define FILTER_TYPE_SECTION 0x0
#define FILTER_TYPE_STREAMING 0x1

typedef struct _TS_HEADER{
	BYTE Byte_1;
	BYTE Byte_2;
	BYTE Byte_3;
	BYTE Byte_4;
} TS_HEADER;

//
//
TSDataFilter::TSDataFilter(int bufferSize, CTTPremiumSourceStream* outputPin, ULONG pid,ULONG pidType,const char* appenderName) : 
CDVBTSFilter(bufferSize),
m_isRunning(false), 
m_bufferSize(bufferSize),
logger(Logger::getInstance(appenderName)),
m_outputPin(outputPin),
m_activePid(pid),
m_activePidType(pidType),
m_sectionFilterHandling(pidType==FILTER_TYPE_SECTION)
{
	LOG4CPLUS_DEBUG(logger,"TSDataFilter ctor: "<<pid);
	LOG4CPLUS_DEBUG(logger,"TSDataFilter appenderName: "<<appenderName);
	LOG4CPLUS_DEBUG(logger,"TSDataFilter ctor finished");
	m_nullByte = new byte[TS_PACKET_LENGTH];
	for(int i=0;i<TS_PACKET_LENGTH;i++){
		m_nullByte[i] = 0x0;
	}

}

//
//
TSDataFilter::~TSDataFilter()
{
	LOG4CPLUS_DEBUG(logger,"TSDataFilter dtor");
	delete m_nullByte;
	LOG4CPLUS_DEBUG(logger,"TSDataFilter dtor finished");
}

//
//
void TSDataFilter::Start()
{
	LOG4CPLUS_DEBUG(logger,"TSFILTER: Start initiated");
	if(m_isRunning == false){
		m_isRunning = true;
		ConfigureFilter();
	}
	LOG4CPLUS_DEBUG(logger,"TSFILTER: Start completed");
}

//
//
void TSDataFilter::Stop()
{
	LOG4CPLUS_DEBUG(logger,"TSFILTER: Stop initiated");
	if (m_isRunning)
	{
		LOG4CPLUS_DEBUG(logger,"TSFILTER: Filter resetted");

		CDVBTSFilter::ResetFilter();
	}
	m_isRunning = false;
	LOG4CPLUS_DEBUG(logger,"TSFILTER: Stop completed");
}

//
//
void TSDataFilter::OnDataArrival(BYTE* Buff, int len)
{
	if (m_isRunning)
	{
		CAutoLock lock(&m_Lock);
		IMediaSample *pSample;
		HRESULT hr = m_outputPin->GetDeliveryBuffer(&pSample,NULL,NULL,0);
		if (FAILED(hr)) 
		{
			return;
		}

		int numberOfPackets = len / PREMIUM_PACKET_LENGTH;
		if(m_activePidType==FILTER_TYPE_SECTION){
			numberOfPackets = (len+1) / PREMIUM_PACKET_LENGTH;
		}
		if(numberOfPackets*PREMIUM_PACKET_LENGTH<len){
			numberOfPackets++;
		}
		int tsPacketLength = numberOfPackets*TS_PACKET_LENGTH;
		BYTE *pBuffer = NULL;
		pSample->GetPointer(&pBuffer);  
		int sampleLen = pSample->GetSize();  
		if (sampleLen < tsPacketLength)
		{
			LOG4CPLUS_DEBUG(logger,"SKIPPING DATA - BUFFER TOO SMALL");
			return ;
		}
		pSample->SetActualDataLength(tsPacketLength);

		LOG4CPLUS_DEBUG(logger,"TS PACKETS: "<<numberOfPackets<<" for pid: "<<m_activePid<<" and type: "<<m_activePidType);

		int m_bufferPos = 0;
		int m_resultBufferPos = 0;
		BYTE firstPidPart = (m_activePid & (0x1F00)) >>8;
		BYTE secondPidPart = m_activePid & (0xFF);
		BYTE pointerField = 0;
		BYTE pesHeader12 = 0x00;
		BYTE pesHeader3 = 0x01;
		TS_HEADER header;
		header.Byte_1 = TS_HEADER_SYNC_BYTE;
		if(m_sectionFilterHandling){
			header.Byte_2 = TS_HEADER_PAYLOAD_START+TS_HEADER_TRANSPORT_PRIO+firstPidPart;
		}else{
			if(Buff[0]==pesHeader12 && Buff[1] == pesHeader12 && Buff[2]==pesHeader3){
				LOG4CPLUS_DEBUG(logger,"TS PACKETS: Start of payload detected");
				header.Byte_2 = TS_HEADER_PAYLOAD_START+TS_HEADER_TRANSPORT_PRIO+firstPidPart;
			}else{
				header.Byte_2 = TS_HEADER_TRANSPORT_PRIO+firstPidPart;
			}
		}
		header.Byte_3 = secondPidPart;
		bool firstPacket = m_sectionFilterHandling;
		int packetLength = PREMIUM_PACKET_LENGTH;
		while(m_bufferPos<len){
			header.Byte_4 = TS_HEADER_PAYLOAD_ONLY;
			header.Byte_4 += m_packetCounter;
			memcpy(&pBuffer[m_resultBufferPos],&header,TS_HEADER_LENGTH);

			if(firstPacket){
				memcpy(&pBuffer[m_resultBufferPos+TS_HEADER_LENGTH],&pointerField,1);
				m_resultBufferPos++;
				packetLength--;
			}
			if(m_bufferPos+packetLength<=len){
				memcpy(&pBuffer[m_resultBufferPos+TS_HEADER_LENGTH],&Buff[m_bufferPos],packetLength);
			}else{
				int existingBytes = len-m_bufferPos;
				memcpy(&pBuffer[m_resultBufferPos+TS_HEADER_LENGTH],&Buff[m_bufferPos],existingBytes);
				memcpy(&pBuffer[m_resultBufferPos+TS_HEADER_LENGTH+existingBytes],&m_nullByte[0],packetLength-existingBytes);
			}
			m_packetCounter++;
			m_packetCounter = m_packetCounter%TS_HEADER_CONTINUITY_MAX_VALUE;
			m_bufferPos += PREMIUM_PACKET_LENGTH;
			m_resultBufferPos += TS_PACKET_LENGTH;

			if(firstPacket){
				m_resultBufferPos--;
				packetLength = PREMIUM_PACKET_LENGTH;
			}
			if(m_sectionFilterHandling){
				header.Byte_2 = TS_HEADER_TRANSPORT_PRIO+firstPidPart;
			}else{
				if(Buff[m_bufferPos]==pesHeader12 && Buff[m_bufferPos+1] == pesHeader12 && Buff[m_bufferPos+2]==pesHeader3){
					LOG4CPLUS_DEBUG(logger,"TS PACKETS: Start of payload detected");
					header.Byte_2 = TS_HEADER_PAYLOAD_START+TS_HEADER_TRANSPORT_PRIO+firstPidPart;
				}else{
					header.Byte_2 = TS_HEADER_TRANSPORT_PRIO+firstPidPart;
				}
			}
			firstPacket = false;
		}
		hr = m_outputPin->Deliver(pSample);
		pSample->Release();

		// downstream filter returns S_FALSE if it wants us to
		// stop or an error if it's reporting an error.
		if (hr != S_OK)
		{
			LOG4CPLUS_DEBUG(logger,"Deliver() returned "<<hr<<"; stopping");
			return ;
		}
	}
}

void TSDataFilter::ReconfigureLogging(char* appenderName){
	logger = Logger::getInstance(appenderName);
}

HRESULT TSDataFilter::ConfigureFilter(){
	LOG4CPLUS_DEBUG(logger,"TSFILTER: Configuration of filter for pid: "<<m_activePid<<" and type "<<m_activePidType<<" started");
	CDVBTSFilter::FILTERTYPE type = CDVBTSFilter::PIPING_FILTER;
	BYTE *filterData = NULL;
	BYTE *filterMask = NULL;
	BYTE filterDataSize = 0;
	DVB_ERROR error;
	if(m_sectionFilterHandling){
		type = CDVBTSFilter::SECTION_FILTER;
		filterData = new BYTE[1];
		filterMask = new BYTE[1];
		filterData[0] = 0x00;
		filterMask[0] = 0x80;
		filterDataSize = 1;
		error = SetFilter(type, (WORD)m_activePid, filterData, filterMask, filterDataSize);
	}else{
    error = SetFilter(type, (WORD)m_activePid, NULL, NULL, PIPE_SIZE_32K);
	}
	m_packetCounter = 0;
	if (error != DVB_ERR_NONE)
	{
		LOG4CPLUS_DEBUG(logger,"TSFILTER: Configuration of filter for pid: "<<m_activePid<<" failed");
		return S_FALSE;
	}
	LOG4CPLUS_DEBUG(logger,"TSFILTER: Configuration of filter for pid: "<<m_activePid<<" completed");
	return S_OK;

}
