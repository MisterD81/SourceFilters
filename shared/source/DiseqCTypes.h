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

typedef enum DiseqCMessageType {
  DiseqCMessageType_MessageOnly = 0,
  DiseqCMessageType_Response = 0x1
} DiseqCMessageType;

typedef enum DiseqCMode {
  DiseqCMode_None = 0,
  DiseqCMode_ToneBurst = 0x1,
  DiseqCMode_Simple = 0x2,
  DiseqCMode_WriteAllCmd = 0x4,
  DiseqCMode_ReadAllCmd = 0x8
} DiseqCMode ;

#define DiseqCMaxRequestId 1000
#define DISEQC_HIGH_NIBLE	0xF0
#define DISEQC_LOW_BAND	0x00
#define DISEQC_HIGH_BAND	0x01
#define DISEQC_VERTICAL		0x00
#define DISEQC_HORIZONTAL	0x02
#define DISEQC_POSITION_A	0x00
#define DISEQC_POSITION_B	0x04
#define DISEQC_OPTION_A		0x00
#define DISEQC_OPTION_B		0x08

