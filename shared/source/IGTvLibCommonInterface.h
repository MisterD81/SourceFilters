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

#include "CommonInterfaceTypes.h"

// {DC994913-7297-4f33-8FDD-BA3175E454C2}
DEFINE_GUID(IID_IGTvLibCommonInterfaceCallback, 
0xdc994913, 0x7297, 0x4f33, 0x8f, 0xdd, 0xba, 0x31, 0x75, 0xe4, 0x54, 0xc2);

DECLARE_INTERFACE_(IGTvLibCommonInterfaceCallback, IUnknown)
{
	STDMETHOD(OnCiMenu)(wchar_t* lpszTitle, wchar_t* lpszSubtitle, wchar_t* lpszBottm, ULONG numberOfChoices)PURE;
	STDMETHOD(OnCiMenuChoice)(ULONG numberOfChoice, wchar_t* lpszText)PURE;
	STDMETHOD(OnCiCloseDisplay)(ULONG delaySeconds)PURE;
	STDMETHOD(OnCiRequest)(ULONG blind, ULONG answerLength, wchar_t* lpszText)PURE;
};

// {5BFDA77E-DEF2-47f3-8E73-20B9F03C9CB7}
DEFINE_GUID(IID_IGTvLibCommonInterface, 
0x5bfda77e, 0xdef2, 0x47f3, 0x8e, 0x73, 0x20, 0xb9, 0xf0, 0x3c, 0x9c, 0xb7);

DECLARE_INTERFACE_(IGTvLibCommonInterface, IUnknown)
{
  STDMETHOD (GetNumberOfSlots) (THIS_ ULONG* numberOfSlots) PURE;
  STDMETHOD (GetSlotStatus) (THIS_ ULONG slotNumber, SlotStatus* sStatus) PURE;
  STDMETHOD (EnterCiMenu)(THIS_ ULONG slotNumber) PURE;
  STDMETHOD (SelectMenu)(THIS_ ULONG slotNumber,THIS_ byte* choice) PURE;
  STDMETHOD (SendMenuAnswer)(THIS_ ULONG slotNumber, ULONG cancel, wchar_t* answer, ULONG length) PURE;
  STDMETHOD (SetCallBack)(THIS_ IGTvLibCommonInterfaceCallback* callback)PURE;
};

