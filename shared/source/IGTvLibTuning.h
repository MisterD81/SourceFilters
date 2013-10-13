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

#include "TuningTypes.h"

// {C6724E33-BD4F-45be-AE37-B309CF3EE80C}
DEFINE_GUID(IID_IGTvLibTuning, 
0xc6724e33, 0xbd4f, 0x45be, 0xae, 0x37, 0xb3, 0x9, 0xcf, 0x3e, 0xe8, 0xc);


DECLARE_INTERFACE_(IGTvLibTuning, IUnknown)
{
  STDMETHOD (TuneDVBT) (FrequencySettings fSettings) PURE;
  STDMETHOD (TuneDVBS) (FrequencySettings fSettings, DigitalDemodulator2Settings dSettings, LnbInfoSettings lSettings, DiseqcSatelliteSettings sSettings) PURE;
  STDMETHOD (TuneDVBC) (FrequencySettings fSettings, DigitalDemodulatorSettings dSettings) PURE;
  STDMETHOD (TuneATSC) (ULONG channelNumber, FrequencySettings fSettings, DigitalDemodulatorSettings dSettings) PURE;
  STDMETHOD (GetAvailableTuningTypes) (THIS_ ULONG* tuningTypes) PURE;
  STDMETHOD (GetSignalStats)(THIS_ ULONG* tunerLocked, ULONG* signalPresent, LONG* signalQuality, LONG* signalLevel) PURE;
  STDMETHOD (IsDiseqSupported)(THIS_ ULONG* diseqcType) PURE;
};

