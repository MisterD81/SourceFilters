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

#include "stdafx.h"
#include "CommonInterfaceControl.h"

CommonInterfaceControl::CommonInterfaceControl(LPUNKNOWN pUnk):CDVBComnIF(),
CUnknown(NAME("CommonInterface"),pUnk),
logger(Logger::getInstance("TTPremiumSource"))
{
  m_nSlotStatus[0]=Slot_Empty;
  m_nSlotStatus[1]=Slot_Empty;
  m_ciDisplayString[0]="";
  m_ciDisplayString[1]="";
  m_ciStatus = 0;
  m_caErrorCount = 0;
  m_bInit = false;

}

CommonInterfaceControl::~CommonInterfaceControl()
{
  if(m_bInit){
    Close();
  }
  ciCallback  = NULL;
}

HRESULT CommonInterfaceControl::Init()
{
  if(m_bInit){
    Close();
  }
  LOG4CPLUS_DEBUG(logger,"CI init started");
  DVB_ERROR error = Open();

  if(error==DVB_ERR_NONE){
    m_bInit = true;
    LOG4CPLUS_DEBUG(logger,"CI init finished");
    CString version,date;
    CDVBComnIF::GetCIVersion(version,date);
    LOG4CPLUS_DEBUG(logger,"CI library version " << version.GetBuffer() << "  " << date.GetBuffer());

  }else{
    LOG4CPLUS_ERROR(logger,"CI init failed");
  }
  return  error == DVB_ERR_NONE ? S_OK : S_FALSE;;
}

STDMETHODIMP CommonInterfaceControl::GetNumberOfSlots(ULONG* numberOfSlots){
  *numberOfSlots = 2;
  return S_OK;
}

STDMETHODIMP CommonInterfaceControl::GetSlotStatus(ULONG slotNumber, SlotStatus* sStatus){
  *sStatus = m_nSlotStatus[slotNumber];
  return S_OK;
}

STDMETHODIMP CommonInterfaceControl::EnterCiMenu(ULONG slotNumber){
  LOG4CPLUS_DEBUG(logger,"Entering CI Menu for slot: "<<slotNumber);
  DVB_ERROR error = EnterModuleMenu((BYTE)slotNumber);
  LOG4CPLUS_INFO(logger,"Entering CI Menu for slot: "<<slotNumber <<" result: "<<error);
  return error == DVB_ERR_NONE ? S_OK : S_FALSE;
}

STDMETHODIMP CommonInterfaceControl::SelectMenu(ULONG slotNumber,byte* choice){
  LOG4CPLUS_DEBUG(logger,"Select menu of slot: "<<slotNumber<<" choice: "<<(int)*choice);
  DVB_ERROR error = MenuAnswer((BYTE)slotNumber,*choice);
  LOG4CPLUS_INFO(logger,"Select menu of slot: "<<slotNumber<<" result: "<<error);
  return error == DVB_ERR_NONE ? S_OK : S_FALSE;
}

STDMETHODIMP CommonInterfaceControl::SendMenuAnswer(ULONG slotNumber, ULONG cancel, wchar_t* answer, ULONG length) {
  CStringW temp = answer;
  CString answerA = answer;
  DVB_ERROR error = Answer((BYTE) slotNumber,answerA.GetBuffer(),(BYTE)length);
  return error == DVB_ERR_NONE ? S_OK : S_FALSE;
}

STDMETHODIMP CommonInterfaceControl::SetCallBack(IGTvLibCommonInterfaceCallback* callback){
  ciCallback = callback;
  return S_OK;
}

void CommonInterfaceControl::ReconfigureLogging(char* identifier){
  logger = Logger::getInstance(identifier);
}

void CommonInterfaceControl::OnSlotStatus(BYTE nSlot, BYTE nStatus, typ_SlotInfo* csInfo)
{
  WORD n;
  LOG4CPLUS_INFO(logger,"Slot status: "<<(int)nStatus<<" for slot: "<<(int)nSlot);
  switch(nStatus)
  {
  case CI_SLOT_EMPTY:
  case CI_SLOT_UNKNOWN_STATE:
    m_nSlotStatus[nSlot]=Slot_Empty;
    break;

  case CI_SLOT_MODULE_INSERTED:
    m_nSlotStatus[nSlot]=Slot_Module_Inserted;
    break;

  case CI_SLOT_MODULE_OK:
  case CI_SLOT_CA_OK:
    m_nSlotStatus[nSlot]=Slot_Module_OK;

    for(n=0; n < csInfo->wNoOfCaSystemIDs; n++)
    {
      LOG4CPLUS_DEBUG(logger,"supported CA ID: "<<csInfo->pCaSystemIDs[n]);
    }
    LOG4CPLUS_INFO(logger,"MenuTitleString: "<<csInfo->pMenuTitleString);
    break;
  case CI_SLOT_DBG_MSG:
    m_nSlotStatus[nSlot] = Slot_Debug_Message;
    LOG4CPLUS_DEBUG(logger,"Recieved debug message for slot: "<<nSlot);
    break;

  default:
    m_nSlotStatus[nSlot] = Slot_Unknown_State;
    LOG4CPLUS_DEBUG(logger,"Recieved unknown slot state for slot: "<<nSlot);
  }
}

void CommonInterfaceControl::OnCAStatus(BYTE nSlot, BYTE nReplyTag, WORD wStatus)
{
  LOG4CPLUS_DEBUG(logger,"$ OnCaChange slot:"<<(int)nSlot<<" reply:"<<(int)nReplyTag<<" status:"<<(int)wStatus);
  switch (nReplyTag)
  {
  case CI_PSI_COMPLETE:
    LOG4CPLUS_DEBUG(logger,"$ CI: ### Number of programs : "<< wStatus);
    break;

  case CI_MODULE_READY:
    LOG4CPLUS_DEBUG(logger,"$ CI: CI_MODULE_READY in OnCAStatus not supported");
    break;
  case CI_SWITCH_PRG_REPLY:
    {
      switch (wStatus)
      {
      case ERR_INVALID_DATA:
        LOG4CPLUS_DEBUG(logger,"$ CI: ERROR::SetProgram failed !!! (invalid PNR)");
        break;
      case ERR_NO_CA_RESOURCE:
        LOG4CPLUS_DEBUG(logger,"$ CI: ERROR::SetProgram failed !!! (no CA resource available)");
        m_ciStatus = -1; // not ready
        m_caErrorCount++; // count the errors to allow reset
        break;
      case ERR_NONE:
        LOG4CPLUS_DEBUG(logger,"$ CI:    SetProgram OK");
        m_ciStatus = 1;
        m_caErrorCount = 0; // reset counter
        break;
      default:
        break;
      }
    }
    break;
  default:
    break;
  }
  if (m_ciStatus == -1 && m_caErrorCount >= 1)
  {
    LOG4CPLUS_INFO(logger,"SetProgram failed "<<m_caErrorCount<<" times because of no CA resource. Resetting CI now.");
    Close();
    Init();
  }
}

void CommonInterfaceControl::OnDisplayString(BYTE nSlot,char* pString,WORD wLength)
{
  CString value;
  value.Format("%s",pString);
  CStringW temp = value;
  m_ciDisplayString[nSlot]=temp;
}

void CommonInterfaceControl::OnDisplayMenu(BYTE nSlot, WORD wItems, char* pStringArray, WORD wLength)
{
  HandleDisplayMenuList(nSlot,wItems,pStringArray,wLength,FALSE);
}

void CommonInterfaceControl::OnDisplayList(BYTE nSlot, WORD wItems, char* pStringArray, WORD wLength)
{
  HandleDisplayMenuList(nSlot,wItems,pStringArray,wLength,FALSE);
}

void CommonInterfaceControl::HandleDisplayMenuList(BYTE nSlot, WORD wItems, char* pStringArray, WORD wLength, BOOL bMenu){

  if(ciCallback == NULL){
    return;
  }

  char* ptr = pStringArray;;
  int offset = 0, index = 0;
  CString s, s2;
  CStringW title, subTitle, temp;
  while(offset < (wLength))
  {
    s.Format("%s", ptr + offset);
    offset += s.GetLength() + 1;

    do
    {
      s2 = s;
      s = s.Right(s.GetLength() - ConvertCharBuf(s2, s2.GetLength()));
      ++index;

      switch(index)
      {
      case 1:
        title = s2;
        break;
      case 2:
        subTitle = s2;
        break;
      case 3:
        temp = s2;
        LOG4CPLUS_INFO(logger,"Menu with title: "<<title.GetBuffer()<<" - Subtitle: "<<subTitle.GetBuffer()<<" - Bottom: "<<temp.GetBuffer());
        ciCallback->OnCiMenu(title.GetBuffer(),subTitle.GetBuffer(),temp.GetBuffer(),wItems-3);
        break;
      default:
        if(bMenu==TRUE){
          temp = s2;
          LOG4CPLUS_INFO(logger,"Menu choice with index: "<<index-5<<" value: "<<temp.GetBuffer());
          //ciCallback->OnCiMenuChoice(index-5,s2.GetBuffer());
        }else{
          temp = s2;
          LOG4CPLUS_INFO(logger,"Menu (LIST) choice with index: "<<index-4<<" value: "<<temp.GetBuffer());
          ciCallback->OnCiMenuChoice(index-4,temp.GetBuffer());
        }
        break;
      }

    } while(s.GetLength() != 0);
  }
}
void CommonInterfaceControl::OnSwitchOsdOff(BYTE nSlot)
{
  if(ciCallback != NULL){
    ciCallback->OnCiCloseDisplay(0);
  }
}

void CommonInterfaceControl::OnInputRequest(BYTE nSlot,BOOL bBlindAnswer,BYTE nExpectedLength,DWORD dwKeyMask)
{
  if(ciCallback != NULL){
    ciCallback->OnCiRequest(bBlindAnswer,nExpectedLength,m_ciDisplayString[nSlot].GetBuffer());
  }
}
