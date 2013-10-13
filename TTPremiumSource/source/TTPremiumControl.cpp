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
#include "TTPremiumControl.h"
#include "DVBDLLInit.h"

//
// Constructor
//
TTPremiumControl::TTPremiumControl(LPUNKNOWN pUnk) 
:CUnknown( NAME ("TTPremiumControl"), pUnk),
logger(Logger::getInstance("TTPremiumSource"))
{
	m_bInit = false;
	//InitDvbApiDll();

	m_numberOfDevices = CDVBCommon::GetNumberOfDevices();
}

//
// Destructor
//
TTPremiumControl::~TTPremiumControl()
{
	LOG4CPLUS_DEBUG(logger,"TTPremiumControl dtor");
	if(m_bInit){
		Close();
	}
	LOG4CPLUS_DEBUG(logger,"TTPremiumControl dtor - finished");
}

STDMETHODIMP TTPremiumControl::GetNumberOfDevices (ULONG* numberOfDevices){
	LOG4CPLUS_DEBUG(logger,"Number of devices: "<<m_numberOfDevices);
	*numberOfDevices = m_numberOfDevices;
	return S_OK;
}
//
// Initialized the filter
//
STDMETHODIMP TTPremiumControl::SetDeviceIndex(ULONG deviceIndex)
{
	if(deviceIndex>=m_numberOfDevices){
		return S_FALSE;
	}
	if(m_bInit){
		LOG4CPLUS_DEBUG(logger,"Instance already initiated. Closing it");
		Close();
	}
	LOG4CPLUS_DEBUG(logger,"Initiating device: "<<deviceIndex);
	if (!CDVBCommon::OpenDevice(0, "TTPremiumSource"))
	{
		if (!CDVBCommon::OpenDevice(0, "TTPremiumSource", TRUE))
		{
			LOG4CPLUS_DEBUG(logger,"OpenDevice failed");
			return S_FALSE;
		}
	}
	LOG4CPLUS_DEBUG(logger,"OpenDevice finished");

	// Boot the device
	LOG4CPLUS_DEBUG(logger,"Preparing boot");
	CString path = "E:\\TTPremiumBoot\\24\\";
	DVB_ERROR error = m_boardControl.BootARM(&path);
	if (error != DVB_ERR_NONE)
	{
		LOG4CPLUS_DEBUG(logger,"Boot failed");
		return S_FALSE;
	}
	LOG4CPLUS_DEBUG(logger,"Boot finished");
	LOG4CPLUS_DEBUG(logger,"Get BE Version");

	CDVBBoardControl::BE_VERSION version;
	error = m_boardControl.GetBEVersion(version);
	if (error != DVB_ERR_NONE)
	{
		LOG4CPLUS_DEBUG(logger,"BE version failed");
		return S_FALSE;
	}
	else
	{
		LOG4CPLUS_DEBUG(logger,"Check Boot Version 2.1 ");

		// 2.1er TI
		if ((version.Firmware >> 16) == 0xF021)
		{
			LOG4CPLUS_DEBUG(logger,"Boot Version 2.1 ");
			path = "E:\\TTPremiumBoot\\21\\";
			error = m_boardControl.BootARM(&path);
			if (error != DVB_ERR_NONE)
			{
				LOG4CPLUS_DEBUG(logger,"Boot Version 2.1 failed");
				return S_FALSE;
			}
		}
	}
	LOG4CPLUS_DEBUG(logger,"Frontend Init start");

	// Init all the interfaces
	error = m_Frontend.Init();
	if (error != DVB_ERR_NONE)
	{
		LOG4CPLUS_DEBUG(logger,"Frontend Init failed");
		return S_FALSE;
	}
	LOG4CPLUS_DEBUG(logger,"Frontend Init finished");
	LOG4CPLUS_DEBUG(logger,"Enable Data DMA");

	error = m_boardControl.EnableDataDMA(TRUE);
	if (error != DVB_ERR_NONE)
	{
		LOG4CPLUS_DEBUG(logger,"Enable Data DMA failed");
		return S_FALSE;
	}
	LOG4CPLUS_DEBUG(logger,"Enable Data DMA successful");
	LOG4CPLUS_DEBUG(logger,"AV Control init");

	error = m_AVControl.Init();
	if (error != DVB_ERR_NONE)
	{
		LOG4CPLUS_DEBUG(logger,"AV Control failed");
		return S_FALSE;
	}
	LOG4CPLUS_DEBUG(logger,"AV Control finished");
	LOG4CPLUS_DEBUG(logger,"AV Control - Check for analog ");
	if((m_AVControl.GetCapabilities() && HAS_ANALOG) == TRUE){
		LOG4CPLUS_DEBUG(logger,"AV Control - Analog detected switching to DVB");
		error = m_AVControl.SetADSwitch(CDVBAVControl::INPUT_XPORT_DVB);
		if (error != DVB_ERR_NONE)
		{
			LOG4CPLUS_DEBUG(logger,"AV Control - Switchting to analog failed");
			return S_FALSE;
		}
		LOG4CPLUS_DEBUG(logger,"AV Control - Successfully switched to DVB");
	}else{
		LOG4CPLUS_DEBUG(logger,"AV Control - Analog not detected");
	}
	m_bInit = true;
	LOG4CPLUS_DEBUG(logger,"Initiating device: "<<deviceIndex<<" finished");
	m_iDeviceIndex = deviceIndex;
	return S_OK;
}


//
// De-initialize the instance
//
HRESULT TTPremiumControl::Close()
{
	m_boardControl.EnableDataDMA(FALSE);
	CDVBCommon::CloseDevice();
	m_bInit = false;
	LOG4CPLUS_DEBUG(logger,"Device closed");
	return S_OK;
}

//
// Get the network type
//
STDMETHODIMP TTPremiumControl::GetAvailableTuningTypes(ULONG* tuningTypes)
{
	CDVBFrontend::FRONTEND_TYPE ft  = m_Frontend.GetType();
	if(ft==CDVBFrontend::FE_SATELLITE){
		*tuningTypes = TuningType_DVBS;
	}else if(ft==CDVBFrontend::FE_CABLE){
		*tuningTypes = TuningType_DVBC;
	}else if(ft==CDVBFrontend::FE_TERRESTRIAL){
		*tuningTypes = TuningType_DVBT;
	}
	LOG4CPLUS_DEBUG(logger,"GetAvailalbeTuningTypes "<<*tuningTypes);
	return S_OK;
}


STDMETHODIMP TTPremiumControl::TuneDVBT(FrequencySettings fSettings){
	CDVBFrontend::FRONTEND_TYPE ft  = m_Frontend.GetType();
	if(ft!=CDVBFrontend::FE_TERRESTRIAL){
		return S_FALSE;
	}
	LOG4CPLUS_DEBUG(logger,"Starting DVBT tune");
	LOG4CPLUS_DEBUG(logger,"Setting Frequency: "<<fSettings.Frequency);
	LOG4CPLUS_DEBUG(logger,"Setting Multiplier: "<<fSettings.Multiplier);
	LOG4CPLUS_DEBUG(logger,"Setting Bandwidth: "<<fSettings.Bandwidth);
	DVB_ERROR error = DVB_ERR_NONE;
	CDVBFrontend::CHANNEL_TYPE ct;

	ct.dvb_t.dwFreq = fSettings.Frequency*fSettings.Multiplier/1000;
	if (m_Frontend.GetCapabilities() & HAS_SI_AUTO)
	{
		ct.dvb_t.Inversion = CDVBFrontend::SI_AUTO;
	}
	else
	{
		ct.dvb_t.Inversion = CDVBFrontend::SI_OFF;
	}

	if (m_Frontend.GetCapabilities() & HAS_BW_AUTO)
	{
		ct.dvb_t.BandWidth = CDVBFrontend::BW_AUTO;
	}
	else
	{
		CDVBFrontend::BANDWITH_TYPE bwMin, bwMax;
		m_Frontend.GetBandwidthRange(bwMin, bwMax);
		if (bwMin == bwMax)
		{
			ct.dvb_t.BandWidth = bwMin;
		}
		else{
			if(fSettings.Bandwidth==8000){
				ct.dvb_t.BandWidth = CDVBFrontend::BW_8MHz;
			}else if(fSettings.Bandwidth==7000){
				ct.dvb_t.BandWidth = CDVBFrontend::BW_7MHz;
			}else if(fSettings.Bandwidth==6000){
				ct.dvb_t.BandWidth = CDVBFrontend::BW_6MHz;
			}else{
				ct.dvb_t.BandWidth = CDVBFrontend::BW_NONE;
			}
		}
	}
	ct.dvb_t.bScan = FALSE;
	error = m_Frontend.SetChannel(ct);
	if (error != DVB_ERR_NONE)
	{
		LOG4CPLUS_DEBUG(logger,"Error when trying to tune");
		return S_FALSE;
	}
	LOG4CPLUS_DEBUG(logger,"Tuning DVBT finished");

	return S_OK;
}

STDMETHODIMP TTPremiumControl::TuneDVBC(FrequencySettings fSettings, DigitalDemodulatorSettings dSettings){
	CDVBFrontend::FRONTEND_TYPE ft  = m_Frontend.GetType();
	if(ft!=CDVBFrontend::FE_CABLE){
		return S_FALSE;
	}
	LOG4CPLUS_DEBUG(logger,"Starting DVBC tune");
	DVB_ERROR error = DVB_ERR_NONE;
	CDVBFrontend::CHANNEL_TYPE ct;
	LOG4CPLUS_DEBUG(logger,"Setting Frequency: "<<fSettings.Frequency);
	LOG4CPLUS_DEBUG(logger,"Setting Multiplier: "<<fSettings.Multiplier);
	LOG4CPLUS_DEBUG(logger,"Setting SymbolRate: "<<dSettings.SymbolRate);
	LOG4CPLUS_DEBUG(logger,"Setting Modulation: "<<dSettings.Modulation);
	ct.dvb_c.dwFreq = fSettings.Frequency*fSettings.Multiplier/1000;
	ct.dvb_c.dwSymbRate = dSettings.SymbolRate*1000;
	if(m_Frontend.GetCapabilities() & HAS_SI_AUTO)
	{
		ct.dvb_c.Inversion = CDVBFrontend::SI_AUTO;
	}
	else
	{
		ct.dvb_c.Inversion = CDVBFrontend::SI_OFF;
	}
	ModulationType modul = dSettings.Modulation;
	switch(modul){
	case BDA_MOD_16QAM:
		ct.dvb_c.Qam = CDVBFrontend::QAM_16;
		break;
	case BDA_MOD_32QAM:
		ct.dvb_c.Qam = CDVBFrontend::QAM_32;
		break;
	case BDA_MOD_64QAM:
		ct.dvb_c.Qam = CDVBFrontend::QAM_64;
		break;
	case BDA_MOD_128QAM:
		ct.dvb_c.Qam = CDVBFrontend::QAM_128;
		break;
	case BDA_MOD_256QAM:
		ct.dvb_c.Qam = CDVBFrontend::QAM_256;
		break;
	}

	if(m_Frontend.GetCapabilities() & HAS_BW_AUTO)
	{
		ct.dvb_c.BandWidth = CDVBFrontend::BW_AUTO;
	}
	else
	{
		CDVBFrontend::BANDWITH_TYPE bwMin, bwMax;
		m_Frontend.GetBandwidthRange(bwMin, bwMax);
		if (bwMin == bwMax)
		{
			ct.dvb_c.BandWidth = bwMin;
		}
		else
		{
			ct.dvb_c.BandWidth = CDVBFrontend::BW_7MHz;
		}
	}
	error = m_Frontend.SetChannel(ct);
	if (error != DVB_ERR_NONE)
	{
		LOG4CPLUS_DEBUG(logger,"Error when trying to tune: "<<error);
		return S_OK;
	}
	LOG4CPLUS_DEBUG(logger,"Tuning DVBC finished");
	LOG4CPLUS_DEBUG(logger,"AV Control - Check for analog ");
	if((m_AVControl.GetCapabilities() && HAS_ANALOG) == TRUE){
		LOG4CPLUS_DEBUG(logger,"AV Control - Analog detected switching to DVB");
		error = m_AVControl.SetADSwitch(CDVBAVControl::INPUT_XPORT_DVB);
		if (error != DVB_ERR_NONE)
		{
			LOG4CPLUS_DEBUG(logger,"AV Control - Switchting to analog failed");
			return S_FALSE;
		}
		LOG4CPLUS_DEBUG(logger,"AV Control - Successfully switched to DVB");
	}else{
		LOG4CPLUS_DEBUG(logger,"AV Control - Analog not detected");
	}
	m_AVControl.SetPIDs((WORD)1, (WORD) 2, (WORD)3);
	return S_OK;
};

STDMETHODIMP TTPremiumControl::TuneDVBS(FrequencySettings fSettings, DigitalDemodulator2Settings dSettings, LnbInfoSettings lSettings, DiseqcSatelliteSettings sSettings){
	CDVBFrontend::FRONTEND_TYPE ft  = m_Frontend.GetType();
	if(ft!=CDVBFrontend::FE_SATELLITE){
		return S_FALSE;
	}
	LOG4CPLUS_DEBUG(logger,"Starting DVBS tune");
	LOG4CPLUS_DEBUG(logger,"Setting Frequency: "<<fSettings.Frequency);
	LOG4CPLUS_DEBUG(logger,"Setting Multiplier: "<<fSettings.Multiplier);
	LOG4CPLUS_DEBUG(logger,"Setting SymbolRate: "<<dSettings.SymbolRate);
	LOG4CPLUS_DEBUG(logger,"Setting Modulation: "<<dSettings.Modulation);
	LOG4CPLUS_DEBUG(logger,"Setting Polarity: "<<fSettings.Polarity);
	LOG4CPLUS_DEBUG(logger,"Setting SwitchFrequency: "<<lSettings.LnbSwitchFrequency);
	LOG4CPLUS_DEBUG(logger,"Setting LowOscillator: "<<lSettings.LowOscillator);
	LOG4CPLUS_DEBUG(logger,"Setting HighOscillator: "<<lSettings.HighOscillator);
	LOG4CPLUS_DEBUG(logger,"Setting SpectralInversion: "<<dSettings.SpectralInversion);
	DVB_ERROR error = DVB_ERR_NONE;
	CDVBFrontend::CHANNEL_TYPE ct;


	BYTE par[4];
	if(sSettings.Enabled!=1){
		par[0] = 0xE0; par[1] = 0x00; par[2] = 0x00;
		error = m_Frontend.SendDiSEqCMsg(par, 3, 0xff);
		if (error != DVB_ERR_NONE)
		{
			LOG4CPLUS_DEBUG(logger,"Error while sending DiseqC message - None case");
			return S_FALSE;
		}
	}else{
		par[0] = 0xE0; par[1] = 0x00; par[2] = 0x00;
		error = m_Frontend.SendDiSEqCMsg(par, 0, (sSettings.ToneBurstEnabled==1) ? 0 : 1);
		if (error != DVB_ERR_NONE)
		{
			LOG4CPLUS_DEBUG(logger,"Error while sending DiseqC message - Simple case");
			return S_FALSE;
		}
		Sleep(120);
		if(sSettings.Diseq10Selection != BDA_LNB_SOURCE_NOT_SET){
			par[0] = 0xE0; par[1] = 0x10; par[2] = 0x38;
			par[3] = DISEQC_HIGH_NIBLE;
			par[3] |= (fSettings.Frequency*fSettings.Multiplier/1000>=lSettings.LnbSwitchFrequency) ? DISEQC_HIGH_BAND : DISEQC_LOW_BAND;
			par[3] |= (fSettings.Polarity==BDA_POLARISATION_LINEAR_V) ? DISEQC_VERTICAL : DISEQC_HORIZONTAL;
			par[3] |= (sSettings.Diseq10Selection==BDA_LNB_SOURCE_B) ? DISEQC_POSITION_B : ((sSettings.Diseq10Selection==BDA_LNB_SOURCE_D) ? DISEQC_POSITION_B : DISEQC_POSITION_A);
			par[3] |= (sSettings.Diseq10Selection==BDA_LNB_SOURCE_D) ? DISEQC_OPTION_B : ((sSettings.Diseq10Selection==BDA_LNB_SOURCE_C) ? DISEQC_POSITION_B : DISEQC_OPTION_A);
			for (int i = 0; i < 3; i++)
			{
				error = m_Frontend.SendDiSEqCMsg(par, 4, 0xff);
				if (error != DVB_ERR_NONE)
				{ 
					LOG4CPLUS_DEBUG(logger,"Error while sending DiseqC message - Multi case");
					return S_FALSE;
				}
				par[0] = 0xE1;
				Sleep(120);
			}
		}
		if(sSettings.Diseq11Selection != DiseqC11Switch_NOT_SET){
			par[0] = 0xE0; // Framming byte = Command from Master no reply required, first transmission
			par[1] = 0x10; // Address byte = Any LNB switcher or SMATV
			par[2] = 0x39; // Command byte = Write port group 1
			par[3] = DISEQC_HIGH_NIBLE | sSettings.Diseq11Selection; // Data byte = Clear + Switch 1
			for (int i = 0; i < 3; i++)
			{
				error = m_Frontend.SendDiSEqCMsg(par, 4, 0xff);
				if (error != DVB_ERR_NONE)
				{ 
					LOG4CPLUS_DEBUG(logger,"Error while sending DiseqC message - Multi case");
					return S_FALSE;
				}
				par[0] = 0xE1;
				Sleep(120);
			}
		}

	}
	ct.dvb_s.dwFreq = fSettings.Frequency*fSettings.Multiplier/1000;
	ct.dvb_s.dwSymbRate = dSettings.SymbolRate*1000;
	ct.dvb_s.Viterbi = CDVBFrontend::VR_AUTO;
	ct.dvb_s.LNB_Power = (fSettings.Polarity==BDA_POLARISATION_LINEAR_H) ? CDVBFrontend::POL_HORZ : CDVBFrontend::POL_VERT;
	ct.dvb_s.bF22kHz = (fSettings.Frequency*fSettings.Multiplier/1000>=lSettings.LnbSwitchFrequency*fSettings.Multiplier/1000) ? TRUE : FALSE;
	ct.dvb_s.dwLOF = (fSettings.Frequency*fSettings.Multiplier/1000>=lSettings.LnbSwitchFrequency*fSettings.Multiplier/1000) ? lSettings.HighOscillator : lSettings.LowOscillator;
	if (m_Frontend.GetCapabilities() & HAS_SI_AUTO)
	{
		ct.dvb_s.Inversion = CDVBFrontend::SI_AUTO;
	}
	else
	{
		ct.dvb_s.Inversion = (dSettings.SpectralInversion==BDA_SPECTRAL_INVERSION_NORMAL) ? CDVBFrontend::SI_ON : CDVBFrontend::SI_OFF;
	}



	error = m_Frontend.SetChannel(ct);
	if (error != DVB_ERR_NONE)
	{
		LOG4CPLUS_DEBUG(logger,"Error when trying to tune");
		return S_FALSE;
	}
	LOG4CPLUS_DEBUG(logger,"Tuning DVBS finished");

	return S_OK;
};

STDMETHODIMP TTPremiumControl::TuneATSC (ULONG channelNumber, FrequencySettings fSettings, DigitalDemodulatorSettings dSettings){
	return S_FALSE;
};

STDMETHODIMP TTPremiumControl::GetSignalStats(ULONG* tunerLocked, ULONG* signalPresent, LONG* signalQuality, LONG* signalLevel)
{
	CDVBFrontend::SIGNAL_TYPE st;
	CDVBFrontend::LOCK_STATE ls;
	BOOL locked;
	DVB_ERROR error = m_Frontend.GetState(locked, &st, &ls);
	if(locked){
		*tunerLocked = 1;
	}else{
		*tunerLocked = 0;
	}
	if (error != DVB_ERR_NONE)
	{
		return S_FALSE;
	}

	if(st.BER < 10e-4){
		*signalQuality = 75; // 75%
	}else if(st.BER < 10e-3){
		*signalQuality  = 50; // 50%
	}else if(st.BER < 10e-2){
		*signalQuality  = 25; // 25%
	}else{ // (Signal.BER >= 10e-2)
		*signalQuality  = 0; // 0%
	}
	if(locked){
		*signalQuality  += 25;
	}else{
		*signalQuality  = 0;
	}

	*signalLevel = st.SNR100;

	if (*signalQuality > 0 && *signalLevel > 0)
	{
		*signalPresent = 1;
	}
	else
	{
		*signalPresent = 0;
	}
	return S_OK;
}


void TTPremiumControl::ReconfigureLogging(char* identifier){
	logger = Logger::getInstance(identifier);
}

STDMETHODIMP TTPremiumControl::GetDevicePath(wchar_t** devicePath, wchar_t** deviceName){
	CString name = "";
	CString dummy1 = "";
	CString dummy2 = "";
	CDVBCommon::GetDeviceInfo((WORD)m_iDeviceIndex,  name,dummy2,dummy1);
	CString path = "";
	CDVBCommon::GetDeviceRegistryPath(false,path);
	CStringW nameW = name;
	CStringW pathW = path;
	*deviceName = nameW.GetBuffer();
	*devicePath = pathW.GetBuffer();
	LOG4CPLUS_DEBUG(logger,"DeviceName: "<<name.GetBuffer());
	LOG4CPLUS_DEBUG(logger,"DevicePath: "<<path.GetBuffer());
	return S_OK;
}

STDMETHODIMP TTPremiumControl::SendDiseqCMessage(ULONG mode, ULONG len,byte* message,ULONG* responseId){
	if(mode==DiseqCMessageType_Response){
		return S_FALSE;
	}
	DVB_ERROR error = DVB_ERR_NONE;
	error = m_Frontend.SendDiSEqCMsg(message, (byte) len, 0xff);
	if (error != DVB_ERR_NONE)
	{
		LOG4CPLUS_DEBUG(logger,"Error while sending DiseqC message - None case");
		return S_FALSE;
	}
	*responseId = 0;
	return S_OK;
}

STDMETHODIMP TTPremiumControl::ReadDiseqCMessage(ULONG requestId, ULONG* len, byte* command){
	return S_FALSE;
}

STDMETHODIMP TTPremiumControl::IsDiseqSupported (ULONG* supported){
	CDVBFrontend::FRONTEND_TYPE ft  = m_Frontend.GetType();
  if(ft!=CDVBFrontend::FE_SATELLITE){
    *supported = DiseqCMode_None;
  }else{
    *supported = DiseqCMode_ToneBurst | DiseqCMode_Simple | DiseqCMode_WriteAllCmd ;
  }
  return true;
}
