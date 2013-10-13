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

#include "LoggingTypes.h"

// {0E3E231A-57ED-4942-84A6-8BC8AA9CB500}
DEFINE_GUID(IID_IGTvLibLogging, 
0xe3e231a, 0x57ed, 0x4942, 0x84, 0xa6, 0x8b, 0xc8, 0xaa, 0x9c, 0xb5, 0x0);


DECLARE_INTERFACE_(IGTvLibLogging, IUnknown)
{
  STDMETHOD (ConfigureLogging)(THIS_ wchar_t* logFile, wchar_t* identifier, LogLevelOption logLevel) PURE;
};

