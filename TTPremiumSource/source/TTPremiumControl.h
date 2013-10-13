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

#include "IGtvLibTuning.h"
#include "IGtvLibSource.h"
#include "IGtvLibDiseqC.h"

#include "DVBFrontend.h"
#include "DVBBoardControl.h"
#include "DVBAVCtrl.h"
#include <log4cplus/logger.h>
#include <log4cplus/loggingmacros.h>

using namespace log4cplus;

class TTPremiumControl: public CUnknown, public IGTvLibSource, public IGTvLibTuning, public IGTvLibDiseqC
{

public:
  TTPremiumControl(LPUNKNOWN pUnk);
  ~TTPremiumControl(void);

  DECLARE_IUNKNOWN

  STDMETHOD(GetNumberOfDevices) (ULONG* numberOfDevices);
  STDMETHOD(SetDeviceIndex) (ULONG deviceIndex) ;
  STDMETHOD(GetDevicePath)(THIS_ wchar_t** devicePath, wchar_t** deviceName);
  STDMETHOD(TuneDVBT) (FrequencySettings fSettings) ;
  STDMETHOD(TuneDVBS) (FrequencySettings fSettings, DigitalDemodulator2Settings dSettings, LnbInfoSettings lSettings, DiseqcSatelliteSettings sSettings) ;
  STDMETHOD(TuneDVBC) (FrequencySettings fSettings, DigitalDemodulatorSettings dSettings) ;
  STDMETHOD(TuneATSC) (ULONG channelNumber, FrequencySettings fSettings, DigitalDemodulatorSettings dSettings);
  STDMETHOD(GetAvailableTuningTypes) (ULONG* tuningTypes) ;
  STDMETHOD(GetSignalStats)(ULONG* tunerLocked, ULONG* signalPresent, LONG* signalQuality, LONG* signalLevel) ;
  STDMETHOD(IsDiseqSupported)(ULONG* supported);
  STDMETHOD(SendDiseqCMessage) (ULONG mode, ULONG len,byte* message,ULONG* responseId);
  STDMETHOD(ReadDiseqCMessage) (ULONG requestId, ULONG* len, byte* command);

  void ReconfigureLogging(char* identifier);


private:
  HRESULT Close();

  Logger logger;
  ULONG m_numberOfDevices;
  BOOLEAN m_bInit;
  ULONG m_iDeviceIndex;
  CDVBBoardControl m_boardControl;
	CDVBFrontend m_Frontend;
  CDVBAVControl m_AVControl;
};

