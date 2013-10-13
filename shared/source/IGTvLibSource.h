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

// {720221F8-85F7-4576-8DF8-0066CDF3D461}
DEFINE_GUID(IID_IGTvLibSource, 
0x720221f8, 0x85f7, 0x4576, 0x8d, 0xf8, 0x0, 0x66, 0xcd, 0xf3, 0xd4, 0x61);


DECLARE_INTERFACE_(IGTvLibSource, IUnknown)
{
  STDMETHOD(GetNumberOfDevices) (THIS_ ULONG* numberOfDevices) PURE;
  STDMETHOD(SetDeviceIndex) (ULONG deviceIndex) PURE;
  STDMETHOD(GetDevicePath)(THIS_ wchar_t** devicePath, wchar_t** deviceName) PURE;
};

