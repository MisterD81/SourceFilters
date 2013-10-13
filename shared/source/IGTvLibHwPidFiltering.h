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

// {F0AD8897-B8AA-4f33-BAF5-837FCF86293D}
DEFINE_GUID(IID_IGTvLibHwPidFiltering, 
0xf0ad8897, 0xb8aa, 0x4f33, 0xba, 0xf5, 0x83, 0x7f, 0xcf, 0x86, 0x29, 0x3d);

DECLARE_INTERFACE_(IGTvLibHwPidFiltering, IUnknown)
{
  STDMETHOD (SetHwPids)(THIS_ ULONG pidCount, ULONG* pids, ULONG* pidTypes) PURE;
};

