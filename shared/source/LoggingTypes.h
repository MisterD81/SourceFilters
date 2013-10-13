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

typedef enum LogLevelOption {
  LogLevel_NOT_SET = -1,
  LogLevel_ALL = 0,
  LogLevel_TRACE = 0,
  LogLevel_DEBUG = 10000,
  LogLevel_INFO = 20000,
  LogLEvel_WARN = 30000,
  LogLevel_ERROR = 40000,
  LogLevel_FATAL = 50000,
  LogLevel_OFF = 60000
} LogLevelOption;

