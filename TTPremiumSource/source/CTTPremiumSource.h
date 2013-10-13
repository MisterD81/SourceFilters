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
#include "IGTvLibLogging.h"
#include "TTPremiumControl.h"
#include "CommonInterfaceControl.h"
#include <log4cplus/logger.h>
#include <log4cplus/SpecialFileAppender.h>
#include <log4cplus/layout.h>
#include <log4cplus/configurator.h>
#include <log4cplus/loggingmacros.h>

using namespace log4cplus;
using namespace log4cplus::helpers;

// Filter GUID
// {1A5D6CE1-D385-4efd-8F9F-BD10E63FB36C}
DEFINE_GUID(CLSID_TTPremiumSource, 
0x1a5d6ce1, 0xd385, 0x4efd, 0x8f, 0x9f, 0xbd, 0x10, 0xe6, 0x3f, 0xb3, 0x6c);

DEFINE_GUID( MEDIATYPE_MPEG2_SECTIONS,
    0x455f176c, 0x4b06, 0x47ce, 0x9a, 0xef, 0x8c, 0xae, 0xf7, 0x3d, 0xf7, 0xb5);

// Class for the TTPremiumSource filter
class CTTPremiumSource : public CSource, public IGTvLibLogging
{
private:
	// Constructor is private because you have to use CreateInstance
  CTTPremiumSource(IUnknown *pUnk, HRESULT *phr);
  virtual ~CTTPremiumSource();
public:
  // Overriden to say what interfaces we support where
  STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void ** ppv);
  // Function needed for the class factory
	static CUnknown * WINAPI CreateInstance(IUnknown *pUnk, HRESULT *phr);  

  DECLARE_IUNKNOWN

  STDMETHOD (ConfigureLogging)(wchar_t* logFile, wchar_t* identifier, LogLevelOption logLevel) ;


private:

  Logger logger;
  TTPremiumControl* m_cardControl;
  CommonInterfaceControl* m_commonInterfaceControl;
  CTTPremiumSourceStream* m_outputPin;
};

