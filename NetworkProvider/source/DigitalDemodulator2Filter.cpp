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
#include "DigitalDemodulator2Filter.h"

CDigitalDemodulator2Filter::CDigitalDemodulator2Filter():
CDigitalDemodulatorFilter()
{
};

CDigitalDemodulator2Filter::~CDigitalDemodulator2Filter(){
};

bool CDigitalDemodulator2Filter::AddFilter(IUnknown* filter){
  IBDA_DigitalDemodulator2* digitalDemodulator2Filter;
  if(filter->QueryInterface(_uuidof(IBDA_DigitalDemodulator2),(void**) &digitalDemodulator2Filter) == S_OK){

    m_digitalDemodulator2List.push_back(digitalDemodulator2Filter);
    return true;
  }
  return false;
}

bool CDigitalDemodulator2Filter::Tune(){
  HRESULT hr = S_OK;
  try{
    if (m_digitalDemodulator2List.size() > 0) {
      IBDA_DigitalDemodulator2* digitalDemodulator2Filter;
      LOG4CPLUS_INFO(logger,"CDigitalDemodulatorFilter request to change InnerFECMethod 0x"<<m_settings.InnerFECMethod<<" - InnerFECRate 0x"
        <<m_settings.InnerFECRate<<" - OuterFECMethod 0x"<<m_settings.OuterFECMethod<<" - OuterFECRate 0x"<<m_settings.OuterFECRate
        <<" - Modulation 0x"<<m_settings.Modulation<<" - SymbolRate 0x"<<m_settings.SymbolRate<<" - SpectralInversion 0x"<<m_settings.SpectralInversion);
      LOG4CPLUS_INFO(logger,"CDigitalDemodulator2Filter request to change GuardInterval 0x"<<m_settings2.GuardInterval<<" - Pilot 0x"
        <<m_settings2.Pilot<<" - RollOff 0x"<<m_settings2.RollOff<<" - TransmissionMode 0x"<<m_settings2.TransmissionMode);
      LOG4CPLUS_INFO(logger,"CDigitalDemodulator2Filter tuning start");
      for (UINT i = 0; i< m_digitalDemodulator2List.size() ; ++i){
        digitalDemodulator2Filter = m_digitalDemodulator2List[i];
        IBDA_DigitalDemodulator* digitalDemodulatorFilter = digitalDemodulator2Filter;
        hr = TuneDigitalDemodulator(digitalDemodulatorFilter);
        if(hr==S_OK){
          hr = TuneDigitalDemodulator2(digitalDemodulator2Filter);
        }
      }	
      LOG4CPLUS_INFO(logger,"CDigitalDemodulator2Filter tuning finished");
    }
  }catch(...){
    hr = S_FALSE;
    LOG4CPLUS_ERROR(logger,"CDigitalDemodulator2Filter::Tune failed");
  }
  return hr==S_OK;
};

HRESULT CDigitalDemodulator2Filter::TuneDigitalDemodulator2(IBDA_DigitalDemodulator2* digitalDemodulator2Filter){
  LOG4CPLUS_DEBUG(logger,"CDigitalDemodulatorFilter Tuning DigitalDemodulator version 2");
  HRESULT hr = S_OK;
  if(m_settings2.GuardInterval != BDA_GUARD_NOT_SET){
    hr = digitalDemodulator2Filter->put_GuardInterval(&m_settings2.GuardInterval);
    LOG4CPLUS_DEBUG(logger,"CDigitalDemodulator2Filter: put_GuardInterval Value: 0x"<<m_settings2.GuardInterval<<" - Result "<<hr);
  }
  if(hr==S_OK && m_settings2.Pilot != BDA_PILOT_NOT_SET){
    hr = digitalDemodulator2Filter->put_Pilot(&m_settings2.Pilot);
    LOG4CPLUS_DEBUG(logger,"CDigitalDemodulator2Filter: put_Pilot Value: 0x"<<m_settings2.Pilot<<" - Result "<<hr);
  }
  if(hr==S_OK && m_settings2.RollOff != BDA_ROLL_OFF_NOT_SET){
    hr = digitalDemodulator2Filter->put_RollOff(&m_settings2.RollOff);
    LOG4CPLUS_DEBUG(logger,"CDigitalDemodulator2Filter: put_RollOff Value: 0x"<<m_settings2.RollOff<<" - Result "<<hr);
  }
  if(hr==S_OK && m_settings2.TransmissionMode != BDA_XMIT_MODE_NOT_SET){
    hr = digitalDemodulator2Filter->put_TransmissionMode(&m_settings2.TransmissionMode) ;
    LOG4CPLUS_DEBUG(logger,"CDigitalDemodulator2Filter: put_TransmissionMode Value: 0x"<<m_settings2.TransmissionMode<<" - Result "<<hr);
  }
  return hr;
}

void CDigitalDemodulator2Filter::SetTuningInformation(DigitalDemodulator2Settings settings){
  try{
    if (m_digitalDemodulator2List.size() > 0) {
      m_settings.InnerFECMethod = settings.InnerFECMethod;
      m_settings.InnerFECRate = settings.InnerFECRate;
      m_settings.Modulation = settings.Modulation;
      m_settings.OuterFECMethod = settings.OuterFECMethod;
      m_settings.OuterFECRate = settings.OuterFECRate;
      m_settings.SpectralInversion = settings.SpectralInversion;
      m_settings.SymbolRate = settings.SymbolRate;
      m_settings2 = settings;
    }
  }catch(...){
    LOG4CPLUS_ERROR(logger,"CDigitalDemodulator2Filter::SetTuningInformation failed ");
  }
};

void CDigitalDemodulator2Filter::ReconfigureLogging(wchar_t* appenderName){
  logger = Logger::getInstance(appenderName);
}
