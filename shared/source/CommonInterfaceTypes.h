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

typedef enum SlotStatus {
  Slot_Empty= 0x0,
  Slot_Module_Inserted = 0x1,
  Slot_Module_OK = 0x2,
  Slot_Ca_OK = 0x3,
  Slot_Debug_Message = 0x4,
  Slot_Unknown_State = 0xFF
} SlotStatus;

