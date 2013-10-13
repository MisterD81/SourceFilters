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
#include "FrequencyFilter.h"

CFrequencyFilter::CFrequencyFilter():
logger(Logger::getInstance(L"NetworkProvider")){
};

CFrequencyFilter::~CFrequencyFilter(){
};

bool CFrequencyFilter::AddFilter(IUnknown* filter){
  IBDA_FrequencyFilter* frequencyFilter;
  if(filter->QueryInterface(_uuidof(IBDA_FrequencyFilter),(void**) &frequencyFilter) == S_OK){

    m_frequencyList.push_back(frequencyFilter);
    return true;
  }
  return false;
}

bool CFrequencyFilter::Tune(){
  HRESULT hr = S_FALSE;
  try{
    IBDA_FrequencyFilter* frequencyFilter;
    LOG4CPLUS_INFO(logger,"CFrequencyFilter::Tune request to change Multiplier "<<m_settings.Multiplier<<" - Frequency to 0x"<<m_settings.Frequency<<" - Bandwith 0x"<<m_settings.Bandwidth<<" - Polarisation 0x"<<m_settings.Polarity<<" - Range 0x"<<m_settings.Range);
    if (m_frequencyList.size() > 0) {
      LOG4CPLUS_INFO(logger,"CFrequencyFilter tuning start");
      for (UINT i = 0; i< m_frequencyList.size() ; ++i){
        frequencyFilter = m_frequencyList[i];
        hr = S_OK;
        if(m_settings.Multiplier != -1){
          if (m_settings.Multiplier == 0){
            LOG4CPLUS_DEBUG(logger,"CFrequencyFilter: Frequency Multiplier = 0 !! Setting Frequency Multiplier to guessed value of 1000");
            m_settings.Multiplier = 1000;
          }
          hr = frequencyFilter->put_FrequencyMultiplier(m_settings.Multiplier);
          LOG4CPLUS_DEBUG(logger,"CFrequencyFilter: put_FrequencyMultiplier Value: "<<m_settings.Multiplier<<" - Result "<<hr);
        }
        if(hr == S_OK && m_settings.Frequency != -1){
          hr = frequencyFilter->put_Frequency(m_settings.Frequency);
          LOG4CPLUS_DEBUG(logger,"CFrequencyFilter: put_Frequency Value: 0x"<<m_settings.Frequency<<" - Result "<<hr);
        }
        if(hr == S_OK && m_settings.Bandwidth != -1){
          hr = frequencyFilter->put_Bandwidth(m_settings.Bandwidth);
          LOG4CPLUS_DEBUG(logger,"CFrequencyFilter: put_Bandwidth Value: 0x"<<m_settings.Bandwidth<<" - Result "<<hr);
        }
        if(hr == S_OK && m_settings.Polarity != BDA_POLARISATION_NOT_SET){
          hr = frequencyFilter->put_Polarity(m_settings.Polarity);
          LOG4CPLUS_DEBUG(logger,"CFrequencyFilter: put_Polarity Value: 0x"<<m_settings.Polarity<<" - Result "<<hr);
        }
        if(hr == S_OK && m_settings.Range != -1){
          hr = frequencyFilter->put_Range(m_settings.Range);
          LOG4CPLUS_DEBUG(logger,"CFrequencyFilter: put_Range Value: 0x"<<m_settings.Range<<" - Result "<<hr);
        }
      }
      LOG4CPLUS_INFO(logger,"CFrequencyFilter tuning finished");
    }	
  }catch(...){
    hr = S_FALSE;
    LOG4CPLUS_ERROR(logger,"CFrequencyFilter::Tune failed");
  }
  return hr == S_OK;
};

void CFrequencyFilter::SetTuningInformation(FrequencySettings settings){
  try{
    m_settings = settings;
  }catch(...){
    LOG4CPLUS_ERROR(logger,"CFrequencyFilter::SetTuningInformation failed");
  }
};

void CFrequencyFilter::ReconfigureLogging(wchar_t* appenderName){
  logger = Logger::getInstance(appenderName);
}
