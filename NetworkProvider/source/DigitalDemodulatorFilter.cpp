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
#include "DigitalDemodulatorFilter.h"

CDigitalDemodulatorFilter::CDigitalDemodulatorFilter():
logger(Logger::getInstance(L"NetworkProvider")){
};

CDigitalDemodulatorFilter::~CDigitalDemodulatorFilter(){
};

bool CDigitalDemodulatorFilter::AddFilter(IUnknown* filter){
  IBDA_DigitalDemodulator* digitalDemodulatorFilter;
  if(filter->QueryInterface(_uuidof(IBDA_DigitalDemodulator),(void**) &digitalDemodulatorFilter) == S_OK){

    m_digitalDemodulatorList.push_back(digitalDemodulatorFilter);
    return true;
  }
  return false;
}

bool CDigitalDemodulatorFilter::Tune(){
  HRESULT hr = S_FALSE;
  try{
    LOG4CPLUS_INFO(logger,"CDigitalDemodulatorFilter request to change InnerFECMethod 0x"<<m_settings.InnerFECMethod<<" - InnerFECRate 0x"
      <<m_settings.InnerFECRate<<" - OuterFECMethod 0x"<<m_settings.OuterFECMethod<<" - OuterFECRate 0x"<<m_settings.OuterFECRate
      <<" - Modulation 0x"<<m_settings.Modulation<<" - SymbolRate 0x"<<m_settings.SymbolRate<<" - SpectralInversion 0x"<<m_settings.SpectralInversion);
    if (m_digitalDemodulatorList.size() > 0) {
      LOG4CPLUS_INFO(logger,"CDigitalDemodulatorFilter tuning start");
      for (UINT i = 0; i< m_digitalDemodulatorList.size() ; ++i){
        hr = TuneDigitalDemodulator(m_digitalDemodulatorList[i]);
      }
      LOG4CPLUS_INFO(logger,"CDigitalDemodulatorFilter tuning finished");
    }	
  }catch(...){
    hr = S_FALSE;
    LOG4CPLUS_ERROR(logger,"CDigitalDemodulatorFilter::Tune failed");
  }
  return hr==S_OK;
};

HRESULT CDigitalDemodulatorFilter::TuneDigitalDemodulator(IBDA_DigitalDemodulator* digitalDemodulatorFilter){
  LOG4CPLUS_DEBUG(logger,"CDigitalDemodulatorFilter Tuning DigitalDemodulator version 1");
  HRESULT hr = S_OK;
  if(m_settings.InnerFECMethod != BDA_FEC_METHOD_NOT_SET){
    hr = digitalDemodulatorFilter->put_InnerFECMethod(&m_settings.InnerFECMethod);
    LOG4CPLUS_DEBUG(logger,"CDigitalDemodulatorFilter: put_InnerFECMethod Value: 0x"<<m_settings.InnerFECMethod<<" - Result "<<hr);
  }
  if(hr==S_OK && m_settings.InnerFECRate != BDA_BCC_RATE_NOT_SET){
    hr = digitalDemodulatorFilter->put_InnerFECRate(&m_settings.InnerFECRate);
    LOG4CPLUS_DEBUG(logger,"CDigitalDemodulatorFilter: put_InnerFECRate Value: 0x"<<m_settings.InnerFECRate<<" - Result "<<hr);
  }
  if(hr==S_OK && m_settings.OuterFECMethod != BDA_FEC_METHOD_NOT_SET){
    hr = digitalDemodulatorFilter->put_OuterFECMethod(&m_settings.OuterFECMethod);
    LOG4CPLUS_DEBUG(logger,"CDigitalDemodulatorFilter: put_OuterFECMethod Value: 0x"<<m_settings.OuterFECMethod<<" - Result "<<hr);
  }
  if(hr==S_OK && m_settings.OuterFECRate != BDA_BCC_RATE_NOT_SET){
    hr = digitalDemodulatorFilter->put_OuterFECRate(&m_settings.OuterFECRate);
    LOG4CPLUS_DEBUG(logger,"CDigitalDemodulatorFilter: put_OuterFECRate Value: 0x"<<m_settings.OuterFECRate<<" - Result "<<hr);
  }
  if(hr==S_OK && m_settings.Modulation != BDA_MOD_NOT_SET){
    hr = digitalDemodulatorFilter->put_ModulationType(&m_settings.Modulation);
    LOG4CPLUS_DEBUG(logger,"CDigitalDemodulatorFilter: put_ModulationType Value: 0x"<<m_settings.Modulation<<" - Result "<<hr);
  }
  if(hr==S_OK && m_settings.SymbolRate != -1){
    hr = digitalDemodulatorFilter->put_SymbolRate(&m_settings.SymbolRate);
    LOG4CPLUS_DEBUG(logger,"CDigitalDemodulatorFilter: put_SymbolRate Value: 0x"<<m_settings.SymbolRate<<" - Result "<<hr);
  }
  if(hr==S_OK && m_settings.SpectralInversion != BDA_SPECTRAL_INVERSION_NOT_SET){
    hr = digitalDemodulatorFilter->put_SpectralInversion(&m_settings.SpectralInversion);
    LOG4CPLUS_DEBUG(logger,"CDigitalDemodulatorFilter: put_SpectralInversion Value: 0x"<<m_settings.SpectralInversion<<" - Result "<<hr);
  }
  return hr;
};

void CDigitalDemodulatorFilter::SetTuningInformation(DigitalDemodulatorSettings settings){
  try{
    m_settings = settings;
  }catch(...){
    LOG4CPLUS_ERROR(logger,"CDigitalDemodulatorFilter::SetTuningInformation failed");
  }
};

void CDigitalDemodulatorFilter::ReconfigureLogging(wchar_t* appenderName){
  logger = Logger::getInstance(appenderName);
}
