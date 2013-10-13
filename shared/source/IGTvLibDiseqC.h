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

#include "DiseqCTypes.h"

// {82F71C06-9675-42ac-AB7C-DBC17B4BAB3B}
DEFINE_GUID(IID_IGTvLibDiseqC, 
0x82f71c06, 0x9675, 0x42ac, 0xab, 0x7c, 0xdb, 0xc1, 0x7b, 0x4b, 0xab, 0x3b);

DECLARE_INTERFACE_(IGTvLibDiseqC, IUnknown)
{
  STDMETHOD (SendDiseqCMessage) (ULONG mode, ULONG len,byte* message,ULONG* responseId) PURE;
  STDMETHOD (ReadDiseqCMessage) (ULONG requestId, ULONG* len, byte* command) PURE;
};

