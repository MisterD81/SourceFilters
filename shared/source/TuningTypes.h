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
#include <bdatypes.h>

typedef enum TuningType {
  TuningType_NOT_SET = 0,
  TuningType_AnalogTv = 0x1,
  TuningType_AnalogRadio = 0x2,
  TuningType_AnalogRfTuner = 0x4,
  TuningType_DVBT = 0x8,
  TuningType_DVBS = 0x10,
  TuningType_DVBC = 0x20,
  TuningType_ATSC = 0x40,
  TuningType_DvbIp = 0x80,
  TuningType_DVBS2 = 0x100,
  TuningType_ISDBT = 0x200,
  TuningType_ISDBS = 0x400,
  TuningType_DAB = 0x800,
  TuningType_DAB_PLUS = 0x1000
} TuningType;

struct LnbInfoSettings{
  ULONG LnbSwitchFrequency;
  ULONG LowOscillator;
  ULONG HighOscillator;
};

struct FrequencySettings{
  ULONG Multiplier;
  ULONG Frequency;
  ULONG Bandwidth;
  Polarisation Polarity;
  ULONG Range;
};

typedef enum DiseqC11Switches{
  DiseqC11Switch_NOT_SET = -1,
  DiseqC11Switch_0 = 0x0,
  DiseqC11Switch_1 = 0x1,
  DiseqC11Switch_2 = 0x2,
  DiseqC11Switch_3 = 0x3,
  DiseqC11Switch_4 = 0x4,
  DiseqC11Switch_5 = 0x5,
  DiseqC11Switch_6 = 0x6,
  DiseqC11Switch_7 = 0x7,
  DiseqC11Switch_8 = 0x8,
  DiseqC11Switch_9 = 0x9,
  DiseqC11Switch_10 = 0xa,
  DiseqC11Switch_11 = 0xb,
  DiseqC11Switch_12 = 0xc,
  DiseqC11Switch_13 = 0xd,
  DiseqC11Switch_14 = 0xe,
  DiseqC11Switch_15 = 0xf,
} DiseqC11Switches;

struct DiseqcSatelliteSettings{
  ULONG Enabled;
  ULONG ToneBurstEnabled;
  LNB_Source Diseq10Selection;
  DiseqC11Switches Diseq11Selection;
};

struct DigitalDemodulatorSettings{
  FECMethod InnerFECMethod;
  BinaryConvolutionCodeRate InnerFECRate;
  ModulationType Modulation;
  FECMethod OuterFECMethod;
  BinaryConvolutionCodeRate OuterFECRate;
  SpectralInversion SpectralInversion;
  ULONG SymbolRate;
};

struct DigitalDemodulator2Settings : DigitalDemodulatorSettings{
  GuardInterval GuardInterval;
  Pilot Pilot;
  RollOff RollOff;
  TransmissionMode TransmissionMode;
};

