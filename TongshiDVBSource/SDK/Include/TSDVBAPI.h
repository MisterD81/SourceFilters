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

const DWORD TSDVB_MAX_ADAPTER_NUMBER = 8;			// 最大的接收适配器个数

typedef struct tagMYDEV_ADAPTER *	  PMYDEV_ADAPTER;
typedef struct tagMYDEV_DATA_PORT *	  PMYDEV_DATA_PORT;

typedef enum
{
	TSDVB_REGISTER_DEREGISTER = 0,			// 注销
	TSDVB_REGISTER_SHARED = 1,				// 共享方式注册
	TSDVB_REGISTER_EXCLUSIVE,				// 独占方式注册，此时拒绝其他对象创建数据端口等操作
}TSDVB_REGISTER_MODE;

enum
{
	TSDVB_ERROR_CODE_NO_SIGNAL = 100000,	// 没有信号
	TSDVB_ERROR_CODE_NO_TUNER,				// 没有找到可用的高频头
	TSDVB_ERROR_CODE_UNKNWON_TUNER,			// 未知高频头类型
	TSDVB_ERROR_CODE_NO_INTERNAL,			// 内部错误
	TSDVB_ERROR_CODE_NO_CAM,				// 没有 CAM 卡
};

//////////////////////////////////////////////////////////////////////////
// const definition
typedef enum 
{					// DVB 传输类型
	DVB_UNKNOW = 0,
	DVB_C,
	DVB_S,
	DVB_T,	
}MODULATIONTYPE;

typedef enum 
{					// DVB-S的极化方向
	POLAR_HORIZONTAL = 0,
	POLAR_VERTICAL
}POLARIZATION;

typedef enum 
{					// DVB-C的调制方式	
	QPSK = 0,
	QAM_4 = 0,
	QAM_16,
	QAM_32,
	QAM_64,
	QAM_128,
	QAM_256,
	QAM_AUTO_DETECT = -1,	// 自动检测 QAM 方式
}MODULATIONQAM;

//----------------------------------------------------
// DVB-S 高频头类型
typedef enum
{
	DVBS_TUNER_TYPE_VOL_CTRL = 0,		// 电压控制，18v 水平，14v 垂直
	DVBS_TUNER_TYPE_DOUBLE_BAND,		// 双波段，第一本振水平，第二本振垂直；
	DVBS_TUNER_TYPE_UNIVERSAL,			// 通过电压和22KHz导频来实现4个转发器
}TSDVBS_TUNER_TYPE;

//////////////////////////////////////////////////////////////////////////
// 说明：
//	高频头调台参数：
//		当高频头为电压控制时，仅 m_nPolar 用于控制水平、垂直；m_dwLNBFreq1有效；但 m_dwLNBFreq2 被忽略
//		当高频头为双波段时，仅由TSDVBS_USE_SECOND_LNB_FREQ选择使用m_dwLNBFreq1和m_dwLNBFreq2， m_nPolar被忽略
//		当高频头为 Universal 时，m_nPolar 有效
//+------------+--------------+--------------+--------------+-------------+------------------+------------------------+
//| 高频头类型 |    m_nPolar  | m_dwLNBFreq1 | m_dwLNBFreq2 |  22KHz 导频 | ToneBurse,DiSEqC | m_dwUniversalSwichFreq |
//+------------+--------------+--------------+--------------+-------------+------------------+------------------------+
//|  电压控制  |     有效     |    有效      |    忽略      |  自由使用   |    自由使用      |		   忽  略		  |
//+------------+--------------+--------------+--------------+-------------+------------------+------------------------+
//|   双波段   |     忽略     |    有效      |    有效      |  自由使用   |    自由使用      |         忽  略         |
//+------------+--------------+--------------+--------------+-------------+------------------+------------------------+
//| Universal  |     有效     |    有效      |    有效      |     忽略    |    自由使用      |         有  效         |
//+------------+--------------+--------------+----------------------------+------------------+------------------------+

//
// 不输出电压
#define TSDVBS_CTRL_NO_POWEROUT		1
// 22KHz 导频信号打开
// 当高频为电压控制、或者是双波段时，该比特可以独立使用
#define TSDVBS_FREQ22KHZ_ON			2

// 以下两个控制位，可以同时为0，但只能有一个为 1
#define TSDVBS_TONE_BURSE_ON		4		// Tone Burse 打开
#define TSDVBS_DiSEqC_ON			8		// DiSEqC 打开

//仅双波段类型时有效，使用第二本振频率
#define TSDVBS_USE_SECOND_LNB_FREQ	0x10

#pragma pack(push,1)
//////////////////////////////////////////////////////////////////////////
typedef struct tagTSDVB_TUNERPARAM
{											// 设置Tuner参数结构体
	MODULATIONTYPE	m_nModulationType;		// 调制方式
	DWORD	m_dwFrequency;					// 传输频率	单位: KHz
	union
	{	
		DWORD	m_dwSymbalRate;				// DVB-C, DVB-S；符号率	单位: KHz
		DWORD	m_dwBandWidth;				// DVB-T
	};
	union
	{
		struct 
		{									// DVB-S 附加参数			
			TSDVBS_TUNER_TYPE m_nTunerType;
			DWORD			m_dwFlags;		// 控制参数
			
			POLARIZATION	m_nPolar;		// 极化方向，电压控制，18v 水平，14v 垂直
			DWORD			m_dwLNBFreq1;	// 第一本振，单位：Khz
			DWORD			m_dwLNBFreq2;	// 第二本振，单位：KHz

			DWORD			m_dwUniversalSwichFreq;	// 第二本振，单位：KHz


			BYTE			m_byDiSEqCChannel;	// DiSEqC 通道，且当 TSDVBS_DiSEqC_ON 时才有效
		}m_DVB_S;

		struct 
		{
			MODULATIONQAM	m_nQAMType;		// 缺省为自动	
		}m_DVB_C;

		struct
		{
			BYTE			m_byHPLP;		// 一般缺省为 HP
		}m_DVB_T;
	};
}TSDVB_TUNERPARAM, *PTSDVB_TUNERPARAM;


//////////////////////////////////////////////////////////////////////////
// 高频头信号状态
#define TUNER_SIGNAL_STATUS_ERROR_RATE_ON	1		// Error rate 有效

typedef struct tagTSDVB_TUNER_SIGNAL_STATUS
{
	int m_nQualityPercent_x100;			// 信号质量，百分比，归一化为 0 ～ 100％，100％信号最好
	int m_nLevelPercent_x100;			// 信号电平，百分比，归一化为 0 ～ 100％

	DWORD	m_dwOriginalQuality;		// 归一化前的信号质量
	DWORD	m_dwOriginalLevel;			// 归一化前的信号电平

	DWORD	m_dwValueMask;
	DWORD	m_dwErrRate;				// 误码率，DVB-C 才有
}TSDVB_TUNER_SIGNAL_STATUS,*PTSDVB_TUNER_SIGNAL_STATUS;

//////////////////////////////////////////////////////////////////////////
// version info
typedef struct tagTSDVB_VERSION_INFO
{
	WORD	m_wIFDrvVersion;			// 接口驱动版本
	WORD	m_wIFDrvBuildNo;			// 接口驱动编译号

	DWORD	m_dwFirmwareVersion;		// 硬件版本	

	WORD	m_wDriverVersion;			// 驱动版本
	WORD	m_wDriverBuildNo;			// 驱动编译号
	
	BYTE	m_abyIDCode[8];				// ID 号
	BYTE	m_abyMACAddr[6];			// MAC 地址

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

