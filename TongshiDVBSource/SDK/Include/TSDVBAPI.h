//////////////////////////////////////////////////////////////////////////	
///
///     Xi'an Tongshi LTD.
///     Date: 2007-3-3
///
///     http://www.tongshi.com
///
//////////////////////////////////////////////////////////////////////////	


#ifndef __TONGSHI_DVB_INTERFACE_H_20070303__
#define __TONGSHI_DVB_INTERFACE_H_20070303__

//////////////////////////////////////////////////////////////////////////
// Data structure and const definition

const DWORD TSDVB_MAX_ADAPTER_NUMBER = 8;			// ���Ľ�������������

typedef struct tagMYDEV_ADAPTER *	  PMYDEV_ADAPTER;
typedef struct tagMYDEV_DATA_PORT *	  PMYDEV_DATA_PORT;

typedef enum
{
	TSDVB_REGISTER_DEREGISTER = 0,			// ע��
	TSDVB_REGISTER_SHARED = 1,				// ����ʽע��
	TSDVB_REGISTER_EXCLUSIVE,				// ��ռ��ʽע�ᣬ��ʱ�ܾ��������󴴽����ݶ˿ڵȲ���
}TSDVB_REGISTER_MODE;

enum
{
	TSDVB_ERROR_CODE_NO_SIGNAL = 100000,	// û���ź�
	TSDVB_ERROR_CODE_NO_TUNER,				// û���ҵ����õĸ�Ƶͷ
	TSDVB_ERROR_CODE_UNKNWON_TUNER,			// δ֪��Ƶͷ����
	TSDVB_ERROR_CODE_NO_INTERNAL,			// �ڲ�����
	TSDVB_ERROR_CODE_NO_CAM,				// û�� CAM ��
};

//////////////////////////////////////////////////////////////////////////
// const definition
typedef enum 
{					// DVB ��������
	DVB_UNKNOW = 0,
	DVB_C,
	DVB_S,
	DVB_T,	
}MODULATIONTYPE;

typedef enum 
{					// DVB-S�ļ�������
	POLAR_HORIZONTAL = 0,
	POLAR_VERTICAL
}POLARIZATION;

typedef enum 
{					// DVB-C�ĵ��Ʒ�ʽ	
	QPSK = 0,
	QAM_4 = 0,
	QAM_16,
	QAM_32,
	QAM_64,
	QAM_128,
	QAM_256,
	QAM_AUTO_DETECT = -1,	// �Զ���� QAM ��ʽ
}MODULATIONQAM;

//----------------------------------------------------
// DVB-S ��Ƶͷ����
typedef enum
{
	DVBS_TUNER_TYPE_VOL_CTRL = 0,		// ��ѹ���ƣ�18v ˮƽ��14v ��ֱ
	DVBS_TUNER_TYPE_DOUBLE_BAND,		// ˫���Σ���һ����ˮƽ���ڶ�����ֱ��
	DVBS_TUNER_TYPE_UNIVERSAL,			// ͨ����ѹ��22KHz��Ƶ��ʵ��4��ת����
}TSDVBS_TUNER_TYPE;

//////////////////////////////////////////////////////////////////////////
// ˵����
//	��Ƶͷ��̨������
//		����ƵͷΪ��ѹ����ʱ���� m_nPolar ���ڿ���ˮƽ����ֱ��m_dwLNBFreq1��Ч���� m_dwLNBFreq2 ������
//		����ƵͷΪ˫����ʱ������TSDVBS_USE_SECOND_LNB_FREQѡ��ʹ��m_dwLNBFreq1��m_dwLNBFreq2�� m_nPolar������
//		����ƵͷΪ Universal ʱ��m_nPolar ��Ч
//+------------+--------------+--------------+--------------+-------------+------------------+------------------------+
//| ��Ƶͷ���� |    m_nPolar  | m_dwLNBFreq1 | m_dwLNBFreq2 |  22KHz ��Ƶ | ToneBurse,DiSEqC | m_dwUniversalSwichFreq |
//+------------+--------------+--------------+--------------+-------------+------------------+------------------------+
//|  ��ѹ����  |     ��Ч     |    ��Ч      |    ����      |  ����ʹ��   |    ����ʹ��      |		   ��  ��		  |
//+------------+--------------+--------------+--------------+-------------+------------------+------------------------+
//|   ˫����   |     ����     |    ��Ч      |    ��Ч      |  ����ʹ��   |    ����ʹ��      |         ��  ��         |
//+------------+--------------+--------------+--------------+-------------+------------------+------------------------+
//| Universal  |     ��Ч     |    ��Ч      |    ��Ч      |     ����    |    ����ʹ��      |         ��  Ч         |
//+------------+--------------+--------------+----------------------------+------------------+------------------------+

//
// �������ѹ
#define TSDVBS_CTRL_NO_POWEROUT		1
// 22KHz ��Ƶ�źŴ�
// ����ƵΪ��ѹ���ơ�������˫����ʱ���ñ��ؿ��Զ���ʹ��
#define TSDVBS_FREQ22KHZ_ON			2

// ������������λ������ͬʱΪ0����ֻ����һ��Ϊ 1
#define TSDVBS_TONE_BURSE_ON		4		// Tone Burse ��
#define TSDVBS_DiSEqC_ON			8		// DiSEqC ��

//��˫��������ʱ��Ч��ʹ�õڶ�����Ƶ��
#define TSDVBS_USE_SECOND_LNB_FREQ	0x10

#pragma pack(push,1)
//////////////////////////////////////////////////////////////////////////
typedef struct tagTSDVB_TUNERPARAM
{											// ����Tuner�����ṹ��
	MODULATIONTYPE	m_nModulationType;		// ���Ʒ�ʽ
	DWORD	m_dwFrequency;					// ����Ƶ��	��λ: KHz
	union
	{	
		DWORD	m_dwSymbalRate;				// DVB-C, DVB-S��������	��λ: KHz
		DWORD	m_dwBandWidth;				// DVB-T
	};
	union
	{
		struct 
		{									// DVB-S ���Ӳ���			
			TSDVBS_TUNER_TYPE m_nTunerType;
			DWORD			m_dwFlags;		// ���Ʋ���
			
			POLARIZATION	m_nPolar;		// �������򣬵�ѹ���ƣ�18v ˮƽ��14v ��ֱ
			DWORD			m_dwLNBFreq1;	// ��һ���񣬵�λ��Khz
			DWORD			m_dwLNBFreq2;	// �ڶ����񣬵�λ��KHz

			DWORD			m_dwUniversalSwichFreq;	// �ڶ����񣬵�λ��KHz


			BYTE			m_byDiSEqCChannel;	// DiSEqC ͨ�����ҵ� TSDVBS_DiSEqC_ON ʱ����Ч
		}m_DVB_S;

		struct 
		{
			MODULATIONQAM	m_nQAMType;		// ȱʡΪ�Զ�	
		}m_DVB_C;

		struct
		{
			BYTE			m_byHPLP;		// һ��ȱʡΪ HP
		}m_DVB_T;
	};
}TSDVB_TUNERPARAM, *PTSDVB_TUNERPARAM;


//////////////////////////////////////////////////////////////////////////
// ��Ƶͷ�ź�״̬
#define TUNER_SIGNAL_STATUS_ERROR_RATE_ON	1		// Error rate ��Ч

typedef struct tagTSDVB_TUNER_SIGNAL_STATUS
{
	int m_nQualityPercent_x100;			// �ź��������ٷֱȣ���һ��Ϊ 0 �� 100����100���ź����
	int m_nLevelPercent_x100;			// �źŵ�ƽ���ٷֱȣ���һ��Ϊ 0 �� 100��

	DWORD	m_dwOriginalQuality;		// ��һ��ǰ���ź�����
	DWORD	m_dwOriginalLevel;			// ��һ��ǰ���źŵ�ƽ

	DWORD	m_dwValueMask;
	DWORD	m_dwErrRate;				// �����ʣ�DVB-C ����
}TSDVB_TUNER_SIGNAL_STATUS,*PTSDVB_TUNER_SIGNAL_STATUS;

//////////////////////////////////////////////////////////////////////////
// version info
typedef struct tagTSDVB_VERSION_INFO
{
	WORD	m_wIFDrvVersion;			// �ӿ������汾
	WORD	m_wIFDrvBuildNo;			// �ӿ����������

	DWORD	m_dwFirmwareVersion;		// Ӳ���汾	

	WORD	m_wDriverVersion;			// �����汾
	WORD	m_wDriverBuildNo;			// ���������
	
	BYTE	m_abyIDCode[8];				// ID ��
	BYTE	m_abyMACAddr[6];			// MAC ��ַ

	WORD	m_wAPIVersion;
	WORD	m_wAPIBuildNo;
}TSDVB_VERSION_INFO,*PTSDVB_VERSION_INFO;

#pragma pack(pop)

//----------------------------------------------------------------------
//
//////////////////////////////////////////////////////////////////////////
// API definition
#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

//-------------------------------------------------------------------------
int		WINAPI	TSDVB_GetAdapterName( DWORD dwAdapterNo, char * pszBuf, int nLen, int * pnOutLen = NULL);
PMYDEV_ADAPTER WINAPI	TSDVB_OpenDevice( DWORD dwAdapterNo );
PMYDEV_ADAPTER WINAPI	TSDVB_OpenDeviceDialog( int * pnOutDevNo = NULL );
void	WINAPI	TSDVB_CloseDevice( PMYDEV_ADAPTER pDevObj );

int		WINAPI	TSDVB_RegisterAsMaster( PMYDEV_ADAPTER pDevObj, TSDVB_REGISTER_MODE nMode=TSDVB_REGISTER_SHARED);
int		WINAPI	TSDVB_ResetAdapter( PMYDEV_ADAPTER pDevObj );
int		WINAPI	TSDVB_GetVersionInfo( PMYDEV_ADAPTER pDevObj, PTSDVB_VERSION_INFO pVersionInfo );


//-------------------------------------------------------------------------
// DataPort
PMYDEV_DATA_PORT WINAPI	TSDVB_CreateRawDataPort( PMYDEV_ADAPTER pDevObj );
PMYDEV_DATA_PORT WINAPI	TSDVB_CreateUDPDataPort( PMYDEV_ADAPTER pDevObj, bool bWin32DataPort=false, DWORD dwDstIP=0, WORD wPort=0 );
int		WINAPI	TSDVB_DeleteDataPort( PMYDEV_DATA_PORT pDataPort );
void	WINAPI	TSDVB_CleanDataPort( PMYDEV_DATA_PORT pDataPort );
int		WINAPI	TSDVB_ReadDataPort( PMYDEV_DATA_PORT pDataPort, PBYTE pBuf, int nLen, PDWORD pdwByteTransferred, OVERLAPPED * pOverlapped = NULL );
int		WINAPI	TSDVB_GetDataPortResult( PMYDEV_DATA_PORT pDataPort, OVERLAPPED * pOverlapped, PDWORD pBytesTransferred, DWORD dwTimeOut );
int		WINAPI	TSDVB_AllowDummyTSPacket( PMYDEV_DATA_PORT pDataPort, BOOL bAllow, BOOL * pbOldValue );
int		WINAPI	TSDVB_QueryDataPortState( PMYDEV_DATA_PORT pDataPort, PDWORD pdwTSPacketReceived, PDWORD pdwPacketLost = NULL);

//-------------------------------------------------------------------------
// PID
bool	WINAPI	TSDVB_OpenPID( PMYDEV_DATA_PORT pDataPort, WORD wPID );
bool	WINAPI	TSDVB_ClosePID( PMYDEV_DATA_PORT pDataPort, WORD wPID );
bool	WINAPI	TSDVB_OpenAllRawPID( PMYDEV_DATA_PORT pDataPort );
void	WINAPI	TSDVB_CloseAllPID( PMYDEV_DATA_PORT pDataPort );

//-------------------------------------------------------------------------
// Tune
int		WINAPI	TSDVB_Tune( PMYDEV_ADAPTER pDevObj, PTSDVB_TUNERPARAM pTuneParam );
BOOL	WINAPI	TSDVB_IsSingalLocked(PMYDEV_ADAPTER pDevObj );
BOOL	WINAPI	TSDVB_GetSignal( PMYDEV_ADAPTER pDevObj, PTSDVB_TUNER_SIGNAL_STATUS pSignalStatus );
MODULATIONTYPE WINAPI	TSDVB_GetModulationType( PMYDEV_ADAPTER pDevObj );
int		WINAPI	TSDVB_SendDiSEqCCmdData( PMYDEV_ADAPTER pDevObj, PBYTE pBuf, int nLen );

// Manual tune
const DWORD TSDVB_MANUAL_TUNE_FLAG_SAVE_MODIFY = 1;			// the modification will be save to pTuneParam when function return
const DWORD TSDVB_MANUAL_TUNE_FLAG_NOT_TUNE = 2;			// edit only, do not tune, and the tune button will be chagned to "save"
int		WINAPI	TSDVB_TuneManual( PMYDEV_ADAPTER pDevObj, PTSDVB_TUNERPARAM pTuneParam, DWORD dwFlags, LPCSTR * ppChannelList = NULL, int nChCount = 0 );

//------------------------------------------------------------------------
// IR
#define TSDVB_IR_GET_SCANCODE( dwIRKey )	(BYTE)( (dwIRKey)&0xFF )
#define TSDVB_IR_GET_ADDRESS( dwIRKey )		(BYTE)( ((dwIRKey)>>8)&0xFF )
#define TSDVB_IR_IS_REPEAT( dwIRKey )		( (dwIRKey)&0x10000 )
int		WINAPI	TSDVB_IR_GetKey(PMYDEV_ADAPTER pDevObj, DWORD * pdwIRKey );

#ifdef __cplusplus
}
#endif //__cplusplus

#endif // __TONGSHI_DVB_INTERFACE_H_20070303__

