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
#include <bdaiface.h>
#include "TuningTypes.h"
#include "DigitalDemodulatorFilter.h"
#include <log4cplus/logger.h>
#include <log4cplus/configurator.h>
#include <log4cplus/loggingmacros.h>

using namespace std;
using namespace log4cplus;
using namespace log4cplus::helpers;

class CDigitalDemodulator2Filter : public CDigitalDemodulatorFilter{
  
public:
  CDigitalDemodulator2Filter();
  ~CDigitalDemodulator2Filter();
  bool AddFilter(IUnknown* filter);
  bool Tune();
  void SetTuningInformation(DigitalDemodulator2Settings settings);
  void ReconfigureLogging(wchar_t* appenderName);
private:
  HRESULT TuneDigitalDemodulator2(IBDA_DigitalDemodulator2* digitalDemodulatorFilter);
  vector<IBDA_DigitalDemodulator2*> m_digitalDemodulator2List;

  DigitalDemodulator2Settings m_settings2;

};
