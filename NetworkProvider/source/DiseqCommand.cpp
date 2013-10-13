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
#include "DiseqCommand.h"
#include "DiseqCTypes.h"

CDiseqCommand::CDiseqCommand():
m_bCommandsEnabled(false),
m_bSupported(false),
m_iRequestId(0),
logger(Logger::getInstance(L"NetworkProvider")){
};

CDiseqCommand::~CDiseqCommand(){
};

bool CDiseqCommand::AddFilter(IUnknown* filter){
  IBDA_DiseqCommand* diseqFilter;
  if(filter->QueryInterface(_uuidof(IBDA_DiseqCommand),(void**) &diseqFilter) == S_OK){

    m_diseqCommandList.push_back(diseqFilter);
    m_bSupported = true;
    return true;
  }
  return false;
}

bool CDiseqCommand::Tune(){
  HRESULT hr = S_OK; 
  try{
    if(m_bSupported){
      LOG4CPLUS_INFO(logger,"CDiseqCommand::Tune request to change ToneBurst "<<m_satelliteSetting.ToneBurstEnabled<<" - Diseq10Selection to 0x"<<m_satelliteSetting.Diseq10Selection<<" - Diseq11Selection to 0x"<<m_satelliteSetting.Diseq11Selection<<" - Enabled to "<<m_satelliteSetting.Enabled);
      IBDA_DiseqCommand* diseqCommandFilter;
      for (UINT i = 0; i< m_diseqCommandList.size() ; ++i){
        diseqCommandFilter = m_diseqCommandList[i];
        if(m_satelliteSetting.Enabled == 1){

          if(m_satelliteSetting.ToneBurstEnabled != -1){
            BOOLEAN toneBurst = m_satelliteSetting.ToneBurstEnabled==1;
            hr = diseqCommandFilter->put_DiseqUseToneBurst(toneBurst);
            LOG4CPLUS_DEBUG(logger,"CDiseqCommand: put_DiseqUseToneBurst Value: "<<m_satelliteSetting.ToneBurstEnabled<<" - Result "<<hr);
          }
          if(hr == S_OK && m_satelliteSetting.Diseq10Selection != BDA_LNB_SOURCE_NOT_SET){
            hr = diseqCommandFilter->put_DiseqLNBSource(m_satelliteSetting.Diseq10Selection);
            LOG4CPLUS_DEBUG(logger,"CDiseqCommand: put_DiseqLNBSource Value: "<<m_satelliteSetting.Diseq10Selection<<" - Result "<<hr);
          }
          if(hr == S_OK && m_satelliteSetting.Diseq11Selection != DiseqC11Switch_NOT_SET){
            byte* commandBytes = new byte[4];
            commandBytes[0] = 0xe0; // Framming byte = Command from Master no reply required, first transmission
            commandBytes[1] = 0x10; // Address byte = Any LNB switcher or SMATV
            commandBytes[2] = 0x39; // Command byte = Write port group 1
            commandBytes[4] = 0xf0 | m_satelliteSetting.Diseq11Selection; // Data byte = Clear + Switch 1
            hr = diseqCommandFilter->put_DiseqSendCommand(0,4,commandBytes);
            delete[] commandBytes;
            LOG4CPLUS_DEBUG(logger,"CDiseqCommand: put_DiseqSendCommand - 1.1 Switches Value: "<<m_satelliteSetting.Diseq11Selection<<" - Result "<<hr);
          }
        }else if(m_satelliteSetting.Enabled == 0){
          m_satelliteSetting.Enabled = 0;
          m_satelliteSetting.ToneBurstEnabled = 0;
          m_satelliteSetting.Diseq10Selection = BDA_LNB_SOURCE_NOT_SET;
          m_satelliteSetting.Diseq11Selection = DiseqC11Switch_NOT_SET;
          hr = diseqCommandFilter->put_DiseqUseToneBurst(false);
          LOG4CPLUS_DEBUG(logger,"CDiseqCommand: put_DiseqUseToneBurst Value: false - Result "<<hr);
          if(hr==S_OK){
            hr = diseqCommandFilter->put_EnableDiseqCommands(false);
            LOG4CPLUS_DEBUG(logger,"CDiseqCommand: put_EnableDiseqCommands Value: false - Result "<<hr);
            m_bCommandsEnabled = false;
          }
        }
      }
    }else{
      hr = S_OK;
    }
  }catch(...){
    hr = S_FALSE;
    LOG4CPLUS_ERROR(logger,"CDiseqCommand::Tune failed");
  }
  return hr;
};

void CDiseqCommand::SetTuningInformation(DiseqcSatelliteSettings settings){
  try{
    if(m_bSupported){
      m_satelliteSetting = settings;
    }
  }catch(...){
    LOG4CPLUS_ERROR(logger,"CDiseqCommand::Retune failed");
  }
};
bool CDiseqCommand::ReadDiseqCMessage(ULONG requestId, ULONG* len, byte* command){
  HRESULT hr = S_OK;
  try{
    if (m_bSupported && m_bCommandsEnabled) {
      for (UINT i = 0; i< m_diseqCommandList.size() ; ++i){
        hr = m_diseqCommandList[i]->get_DiseqResponse(requestId,len,command);
      }
    }	
  }catch(...){
    hr = S_FALSE;
    LOG4CPLUS_ERROR(logger,"CDiseqCommand::Tune failed");
  }
  return hr;
};

bool CDiseqCommand::SendDiseqCMessage(ULONG mode, ULONG len,byte* message,ULONG* responseId){
  HRESULT hr = S_OK;
  try{
    if (m_bSupported) {
      for (UINT i = 0; i< m_diseqCommandList.size() ; ++i){
		if(!m_bCommandsEnabled){
			hr = m_diseqCommandList[i]->put_EnableDiseqCommands(true);
			if(hr!=S_OK){
				return false;
			}
		}
		m_bCommandsEnabled = true;
        hr = m_diseqCommandList[i]->put_DiseqSendCommand(m_iRequestId,len,message);
		*responseId = m_iRequestId;
		m_iRequestId++;
		m_iRequestId = m_iRequestId % DiseqCMaxRequestId;
      }
    }	
  }catch(...){
    hr = S_FALSE;
    LOG4CPLUS_ERROR(logger,"CDiseqCommand::Tune failed");
  }
  return hr;
};

bool CDiseqCommand::DisableCommands(){
  HRESULT hr = S_OK;
  try{
    if (m_bSupported && m_bCommandsEnabled) {
      for (UINT i = 0; i< m_diseqCommandList.size() ; ++i){
        hr = m_diseqCommandList[i]->put_EnableDiseqCommands(false);
        m_bCommandsEnabled = false;
      }
    }	
  }catch(...){
    hr = S_FALSE;
    LOG4CPLUS_ERROR(logger,"CDiseqCommand::Tune failed");
  }
  return hr;
};

void CDiseqCommand::ReconfigureLogging(wchar_t* appenderName){
  logger = Logger::getInstance(appenderName);
}

bool CDiseqCommand::IsDiseqSupported(){
  return m_bSupported;
}

