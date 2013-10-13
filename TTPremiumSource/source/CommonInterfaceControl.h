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

#include <DVBComnIF.h>
#include "IGTvLibCommonInterface.h"
#include <log4cplus/logger.h>
#include <log4cplus/configurator.h>
#include <log4cplus/loggingmacros.h>

using namespace log4cplus;

class CommonInterfaceControl : public CDVBComnIF, public CUnknown, public IGTvLibCommonInterface
{
public:
	CommonInterfaceControl(LPUNKNOWN pUnk);
	virtual ~CommonInterfaceControl();
	
  HRESULT Init();
  STDMETHOD (GetNumberOfSlots) (THIS_ ULONG* numberOfSlots) ;
  STDMETHOD (GetSlotStatus) (THIS_ ULONG slotNumber, SlotStatus* sStatus) ;
  STDMETHOD (EnterCiMenu)(THIS_ ULONG slotNumber) ;
  STDMETHOD (SelectMenu)(THIS_ ULONG slotNumber,byte* choice) ;
  STDMETHOD (SendMenuAnswer)(THIS_ ULONG slotNumber, ULONG cancel, wchar_t* answer, ULONG length) ;
	STDMETHOD (SetCallBack)(THIS_ IGTvLibCommonInterfaceCallback* callback);

  void ReconfigureLogging(char* identifier);

  DECLARE_IUNKNOWN

private:
	void OnSlotStatus(BYTE nSlot, BYTE nStatus, typ_SlotInfo* csInfo);
	void OnCAStatus(BYTE nSlot, BYTE nReplyTag, WORD wStatus);
	void OnDisplayString(BYTE nSlot, char* pString, WORD wLength);
	void OnDisplayMenu(BYTE nSlot, WORD wItems, char* pStringArray, WORD wLength);
	void OnDisplayList(BYTE nSlot, WORD wItems, char* pStringArray, WORD wLength);
	void OnSwitchOsdOff(BYTE nSlot);
	void OnInputRequest(BYTE nSlot, BOOL bBlindAnswer, BYTE nExpectedLength, DWORD dwKeyMask);
  void CommonInterfaceControl::HandleDisplayMenuList(BYTE nSlot, WORD wItems, char* pStringArray, WORD wLength, BOOL bMenu);


  IGTvLibCommonInterfaceCallback* ciCallback;

  ULONG m_ciStatus;
  ULONG m_caErrorCount;
  BOOLEAN m_bInit;
	SlotStatus	m_nSlotStatus[2];
	CStringW	m_ciDisplayString[2];
  Logger logger;
};


