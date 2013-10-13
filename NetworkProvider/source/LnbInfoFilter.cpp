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
#include "LnbInfoFilter.h"

CLnbInfoFilter::CLnbInfoFilter():
logger(Logger::getInstance(L"NetworkProvider")){
};

CLnbInfoFilter::~CLnbInfoFilter(){
};

bool CLnbInfoFilter::AddFilter(IUnknown* filter){
  IBDA_LNBInfo* lnbInfoFilter;
  if(filter->QueryInterface(_uuidof(IBDA_LNBInfo),(void**) &lnbInfoFilter) == S_OK){

    m_lnbInfoList.push_back(lnbInfoFilter);
    return true;
  }
  return false;
}

bool CLnbInfoFilter::Tune(){
  HRESULT hr = S_FALSE;
  try{
    IBDA_LNBInfo* lnbInfoFilter;
    LOG4CPLUS_INFO(logger,"CLnbInfoFilter::Tune request to change LNBSwitch frequency 0x"<<m_settings.LnbSwitchFrequency<<" - LowOscilllator 0x"<<m_settings.LowOscillator<<" - HighOscillator 0x"<<m_settings.HighOscillator);
    if (m_lnbInfoList.size() > 0) {
      LOG4CPLUS_INFO(logger,"CLnbInfoFilter tuning start");
      for (UINT i = 0; i< m_lnbInfoList.size() ; ++i){
        lnbInfoFilter = m_lnbInfoList[i];
        hr = S_OK;
        if(m_settings.LnbSwitchFrequency != -1){
          hr = lnbInfoFilter->put_HighLowSwitchFrequency(m_settings.LnbSwitchFrequency);
          LOG4CPLUS_DEBUG(logger,"CLnbInfoFilter: put_HighLowSwitchFrequency Value: 0x"<<m_settings.LnbSwitchFrequency<<" - Result "<<hr);
        }
        if(hr == S_OK && m_settings.LowOscillator != -1){
          hr = lnbInfoFilter->put_LocalOscilatorFrequencyLowBand(m_settings.LowOscillator);
          LOG4CPLUS_DEBUG(logger,"CLnbInfoFilter: put_LocalOscilatorFrequencyLowBand Value: 0x"<<m_settings.LowOscillator<<" - Result "<<hr);
        }
        if(hr == S_OK && m_settings.HighOscillator != -1){
          hr = lnbInfoFilter->put_LocalOscilatorFrequencyHighBand(m_settings.HighOscillator);
          LOG4CPLUS_DEBUG(logger,"CLnbInfoFilter: put_LocalOscilatorFrequencyHighBand Value: 0x"<<m_settings.HighOscillator<<" - Result "<<hr);
        }
      }
      LOG4CPLUS_INFO(logger,"CLnbInfoFilter tuning finished");
    }	
  }catch(...){
    hr = S_FALSE;
    LOG4CPLUS_ERROR(logger,"CLnbInfoFilter::Tune failed");
  }
  return hr == S_OK;
};

void CLnbInfoFilter::SetTuningInformation(LnbInfoSettings settings){
  try{
    m_settings = settings;
  }catch(...){
    LOG4CPLUS_ERROR(logger,"CLnbInfoFilter::SetTuningInformation failed");
  }
};

void CLnbInfoFilter::ReconfigureLogging(wchar_t* appenderName){
  logger = Logger::getInstance(appenderName);
}
