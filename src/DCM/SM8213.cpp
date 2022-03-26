// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "SM8213.h"
#include "STSSP8201.h"
#include "SelfCheck.h"
#include "HDModule.h"
#include <process.h>
#include "Calibration.h"
#include "Sts8100.h"
#include "TMU.h"
#include "PMU.h"
#include "ChannelMode.h"
#include "Relay.h"
#include "I2C\I2C.h"
#include "DCM.h"
#include "ClassDistribution.cpp"
#include "ConfigFile.h"
string g_strPinName;

#pragma data_seg("SM8213")
BYTE g_byBoardCount = 0;///<The board count
BYTE g_abyBoardSlot[DCM_MAX_BOARD_NUM] = { 0 };///<The slot number of each board
CAL_DATA g_aCalibrationData[DCM_MAX_BOARD_NUM][DCM_MAX_CHANNELS_PER_BOARD] = { 0 };///<The calibration data of all board
BYTE g_abyChannelMode[DCM_MAX_BOARD_NUM][DCM_MAX_CONTROLLERS_PRE_BOARD][DCM_CHANNELS_PER_CONTROL] = { 0 };///<The channel mode of each channel, 0 is MCU, 1 is PMU, 2 is Neither

///<PMU 
unsigned char g_aucMeasIRange[DCM_MAX_BOARD_NUM * DCM_MAX_CHANNELS_PER_BOARD] = { 0, 0 };///<The PMU current range
unsigned char g_aucMeasType[DCM_MAX_BOARD_NUM * DCM_MAX_CHANNELS_PER_BOARD] = { 0, 0 };///<The PMU measurement type 0 - MV  1 - MI
unsigned char g_aucMeasForceMode[DCM_MAX_BOARD_NUM * DCM_MAX_CHANNELS_PER_BOARD] = { 0 };///<The PMU force mode, 0 is FV and 1 is FI
unsigned char g_aucLatestMeasType[DCM_MAX_BOARD_NUM * DCM_MAX_CHANNELS_PER_BOARD] = { 0, 0 };///<The PMU latest measurement type 0 - MV  1 - MI

double g_adSamplePeriod[DCM_MAX_BOARD_NUM * DCM_MAX_CHANNELS_PER_BOARD] = { 0 };///<The sample period of each channel
UINT g_auSampleTimes[DCM_MAX_BOARD_NUM * DCM_MAX_CHANNELS_PER_BOARD] = { 0 };///<The sample times
USHORT g_ausPMUSampleChannel[DCM_MAX_BOARD_NUM][DCM_MAX_CONTROLLERS_PRE_BOARD] = { 0 };///<The PMU sample chip of each controller
USHORT g_ausPMUAverageData[DCM_MAX_BOARD_NUM][DCM_MAX_CONTROLLERS_PRE_BOARD][DCM_CHANNELS_PER_CONTROL] = { 0 };///<The PMU average data of each controller

///<TMU
USHORT g_ausTMUUnitChannel[DCM_MAX_BOARD_NUM][DCM_MAX_CONTROLLERS_PRE_BOARD][TMU_UNIT_COUNT_PER_CONTROLLER] = { 0 };///<The channel of the TMU unit
BYTE g_abyTMUMode[DCM_MAX_BOARD_NUM][DCM_MAX_CONTROLLERS_PRE_BOARD][TMU_UNIT_COUNT_PER_CONTROLLER] = { 0 };///<The mode of the TMU unit
BYTE g_abyTMUTriggerEdge[DCM_MAX_BOARD_NUM][DCM_MAX_CONTROLLERS_PRE_BOARD][TMU_UNIT_COUNT_PER_CONTROLLER] = {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1 };///<The trigger edge of the TMU unit
USHORT g_ausTMUSampleNum[DCM_MAX_BOARD_NUM][DCM_MAX_CONTROLLERS_PRE_BOARD][TMU_UNIT_COUNT_PER_CONTROLLER] = { 0 };///<The sample number of the TMU unit
USHORT g_ausTMUHoldOffTime[DCM_MAX_BOARD_NUM][DCM_MAX_CONTROLLERS_PRE_BOARD][TMU_UNIT_COUNT_PER_CONTROLLER] = { 0 };///<The hold off time of the TMU unit
USHORT g_ausTMUHoldOffNum[DCM_MAX_BOARD_NUM][DCM_MAX_CONTROLLERS_PRE_BOARD][TMU_UNIT_COUNT_PER_CONTROLLER] = { 0 };///<The hold off time of the TMU unit
float g_afTMUTimeout[DCM_MAX_BOARD_NUM][DCM_MAX_CONTROLLERS_PRE_BOARD][TMU_UNIT_COUNT_PER_CONTROLLER] = { 0 };///<The timeout of the TMU unit

///<For relay status
ULONG g_aulFuncReley[DCM_MAX_BOARD_NUM][RELAY_REGISTER_COUNT] = { 0 };
ULONG g_aulDCReley[DCM_MAX_BOARD_NUM][RELAY_REGISTER_COUNT] = { 0 };

int g_nCurInstanceID = 0;///<The variable shared of current instance ID
int g_nInstanceCount = 0;///<The instance count for debug tool

USHORT g_usMaxSiteCount = 0;///<The maximum site count of all instance include I2C
BOOL g_bAllowDebugToolShow = FALSE;///<Whether allow debug tools show pin name and site count
#pragma  data_seg()
#pragma  comment(linker, "/SECTION:SM8213,RWS")
/**
 * @struct BOARD_INFO
 * @brief The board information of the board
*/
struct BOARD_INFO 
{
	BYTE m_byBoardIndex;///<The board index of the board
	USHORT m_usChannelCount;///<The channel count of the board
	BOARD_INFO()
	{
		m_byBoardIndex = 0;
		m_usChannelCount = 0;
	}
};
map<BYTE, BOARD_INFO> g_mapValidBoard;
map<int, CI2C*> g_mapI2C;///<The map for the pair of I2C and instance index, key is instance index and value is the point of the class for the instance
CClassDistribution<CDCM>* g_pDCMDistribution = CClassDistribution<CDCM>::Instance();///<The point of the class of DCM distribution
CClassDistribution<CI2C>* g_pI2CDistribution = CClassDistribution<CI2C>::Instance();///<The point of the class of CI2C distribution

map<string, int> g_mapPinGroup;///<The pin group and the instance ID belongs to, key is pin group and value is instance ID
/**
 * @class CPinInfo
 * @brief The singleton class for pin information management
*/
class CPinNoManage
{
public:
	/**
	 * @brief Get the instance of singleton class
	 * @return The point of the instance
	*/
	static CPinNoManage* Instance();
	/**
	 * @brief Set the pin count
	 * @param[in] nInstanceID The instance ID of the instance
	 * @param[in] usPinCount The pin count
	 * @param[in] bDCMWithVector Whether the instance is the DCM with vector file
	*/
	void SetPinCount(int nInstanceID, USHORT usPinCount, BOOL bDCMWithVector);
	/**
	 * @brief Get the pin number offset the current instance
	 * @param[in] usPinNo The pin number
	 * @param[out] nInstanceID The instance ID which the pin number is belongs to
	 * @param[out] bDCMWithVector Whether the instance is DCM with vector file
	 * @return The pin number
	 * - >=0 The pin number offset the instance
	 * - -1 The pin number is over range
	*/
	int GetPinNoOffset(USHORT usPinNo, int& nInstanceID, BOOL& bDCMWithVector);
	/**
	 * @brief Delete the instance
	 * @param[in] nInstanceID The instance ID
	*/
	void DeleteInstance(int nInstanceID);
	/**
	 * @brief Get the pin count of all instance
	 * @return The pin count
	*/
	USHORT GetPinCount() const;
private:
	/**
	 * @brief Constructor
	*/
	CPinNoManage();

private:
	std::map<int, USHORT> m_mapDCM;///<The pin count of DCM with vector file
	std::map<int, USHORT> m_mapI2C;///<The pin count of I2C instance
};

CPinNoManage* CPinNoManage::Instance()
{
	static CPinNoManage PinNoManage;
	return &PinNoManage;
}

void CPinNoManage::SetPinCount(int nInstanceID, USHORT usPinCount, BOOL bDCMWithVector)
{
	auto& mapPin = bDCMWithVector ? m_mapDCM : m_mapI2C;
	auto iterPin = mapPin.find(nInstanceID);
	if (mapPin.end() == iterPin)
	{
		if (0 == usPinCount)
		{
			return;
		}
		mapPin.insert(make_pair(nInstanceID, usPinCount));
	}
	else
	{
		iterPin->second = usPinCount;
	}
}

int CPinNoManage::GetPinNoOffset(USHORT usPinNo, int& nInstanceID, BOOL& bDCMWithVector)
{
	USHORT usCurPinCount = 0;

	auto CheckPin = [&](BOOL bDCM)->int
	{
		for (auto& PinCount : bDCM ? m_mapDCM : m_mapI2C)
		{
			if (usCurPinCount + PinCount.second <= usPinNo)
			{
				usCurPinCount += PinCount.second;
				continue;
			}
			nInstanceID = PinCount.first;
			bDCMWithVector = bDCM;
			return usPinNo - usCurPinCount;
		}
		return -1;
	};
	int nOffset = CheckPin(TRUE);
	if (0 <= nOffset)
	{
		return nOffset;
	}
	return CheckPin(FALSE);
}

void CPinNoManage::DeleteInstance(int nInstanceID)
{
	auto iterPin = m_mapDCM.find(nInstanceID);
	if (m_mapDCM.end() != iterPin)
	{
		m_mapDCM.erase(iterPin);
	}
	iterPin = m_mapI2C.find(nInstanceID);
	if (m_mapI2C.end() != iterPin)
	{
		m_mapI2C.erase(nInstanceID);
	}
}

USHORT CPinNoManage::GetPinCount() const
{
	USHORT usPinCount = 0;
	auto GetPinCount = [&](BOOL bDCMWithVector)
	{
		auto& mapData = bDCMWithVector ? m_mapDCM : m_mapI2C;
		for (auto& Pin : mapData)
		{
			usPinCount += Pin.second;
		}
	};
	GetPinCount(TRUE);
	GetPinCount(FALSE);
	return usPinCount;
}

CPinNoManage::CPinNoManage()
{
}

void AddBoard();

void SetMemory()
{
	for (auto& Board : g_mapValidBoard)
	{
		for (BYTE byControllerIndex = 0; byControllerIndex < DCM_MAX_CONTROLLERS_PRE_BOARD; ++byControllerIndex)
		{
			///<Channel mode
			CChannelMode::Instance()->SetMemory(Board.first, byControllerIndex, g_abyChannelMode[Board.second.m_byBoardIndex][byControllerIndex]);
			///<PMU
			USHORT usChannelStart = Board.second.m_byBoardIndex * DCM_MAX_CHANNELS_PER_BOARD + byControllerIndex * DCM_CHANNELS_PER_CONTROL;
			CPMU::Instance()->SetAverageDataMemory(Board.first, byControllerIndex, (USHORT*)(g_ausPMUAverageData[Board.second.m_byBoardIndex][byControllerIndex]));
			CPMU::Instance()->SetSampleModeMemory(Board.first, byControllerIndex, &g_auSampleTimes[usChannelStart],
				&g_adSamplePeriod[usChannelStart]);
			CPMU::Instance()->SetMeasureModeMemory(Board.first, byControllerIndex, &g_aucMeasType[usChannelStart]);
			CPMU::Instance()->SetLatestMeasureModeMemory(Board.first, byControllerIndex, &g_aucLatestMeasType[usChannelStart]);
			CPMU::Instance()->SetForceModeMemory(Board.first, byControllerIndex, &g_aucMeasForceMode[usChannelStart], &g_aucMeasIRange[usChannelStart]);

			///<TMU
			CTMU::Instance()->SetModeMemory(Board.first, byControllerIndex, g_abyTMUMode[Board.second.m_byBoardIndex][byControllerIndex]);
			CTMU::Instance()->SetUnitChannelMemory(Board.first, byControllerIndex, (USHORT*)g_ausTMUUnitChannel[Board.second.m_byBoardIndex][byControllerIndex]);
			CTMU::Instance()->SetTriggerEdgeMemory(Board.first, byControllerIndex, (BYTE*)g_abyTMUTriggerEdge[Board.second.m_byBoardIndex][byControllerIndex]);
			CTMU::Instance()->SetHoldOffMemory(Board.first, byControllerIndex, (USHORT*)g_ausTMUHoldOffTime[Board.second.m_byBoardIndex][byControllerIndex], (USHORT*)g_ausTMUHoldOffNum[Board.second.m_byBoardIndex][byControllerIndex]);
			CTMU::Instance()->SetTimeoutMemory(Board.first, byControllerIndex, (float*)g_afTMUTimeout[Board.second.m_byBoardIndex][byControllerIndex]);
			CTMU::Instance()->SetSampleNumberMemory(Board.first, byControllerIndex, (USHORT*)g_ausTMUSampleNum[Board.second.m_byBoardIndex][byControllerIndex]);
		}

		CRelayRigister::Instance()->SetFuncRelayMem(Board.first, g_aulFuncReley[Board.second.m_byBoardIndex]);
		CRelayRigister::Instance()->SetDCRelayMem(Board.first, g_aulDCReley[Board.second.m_byBoardIndex]);
	}
}

bool operator==(BOARD_INFO SourceData, BOARD_INFO TargetData)
{
	if (SourceData.m_byBoardIndex == TargetData.m_byBoardIndex && SourceData.m_usChannelCount == TargetData.m_usChannelCount)
	{
		return true;
	}
	return false;
}

BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
)
{
	vector<int> vecValidInstance;
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		g_pDCMDistribution->GetInstanceValid(vecValidInstance);
		if (0 == vecValidInstance.size())
		{
			dcm_InitializeInstance(0);
		}
		break;
	case DLL_THREAD_ATTACH:
		break;
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

inline void GetConfigDirectory(string& strDirectory)
{
	HMODULE hDCM = GetModuleHandle("DCM.dll");
	char lpszPath[MAX_PATH] = { 0 };
	GetModuleFileName(hDCM, lpszPath, sizeof(lpszPath));
	strDirectory = lpszPath;
	int nPos = strDirectory.rfind("\\");
	if (-1 != nPos)
	{
		strDirectory.erase(nPos + 1);
	}
	strDirectory += "DCM\\";
	CreateDirectory(strDirectory.c_str(), nullptr);
}

void GetConfigFile(string& strFilePath)
{
	GetConfigDirectory(strFilePath);
	strFilePath += "VectorPair.ini";
}

void SetCalibrationMemory()
{
	if (EQUAL_ERROR > g_aCalibrationData[0][0].m_fDVHGain[0])
	{
		///<The calibration data is not be initialized
		CAL_DATA InitCalData[DCM_CHANNELS_PER_CONTROL];

		for (int iCh = 0; iCh < DCM_CHANNELS_PER_CONTROL; iCh++)
		{
			InitCalData[iCh].m_fDVHGain[0] = 1.0;
			InitCalData[iCh].m_fDVHOffset[0] = 0.0;
			InitCalData[iCh].m_fDVLGain[0] = 1.0;
			InitCalData[iCh].m_fDVLOffset[0] = 0.0;
			InitCalData[iCh].m_fFVGain[0] = 1.0;
			InitCalData[iCh].m_fFVOffset[0] = 0.0;
			InitCalData[iCh].m_fMVGain[0] = 1.0;
			InitCalData[iCh].m_fMVOffset[0] = 0.0;
			for (int i = 0; i < 5; i++)
			{
				InitCalData[iCh].m_fFIGain[i] = 1.0;
				InitCalData[iCh].m_fFIOffset[i] = 0.0;
				InitCalData[iCh].m_fMIGain[i] = 1.0;
				InitCalData[iCh].m_fMIOffset[i] = 0.0;
			}
		}
		for (BYTE byBoardIndex = 0; byBoardIndex < DCM_MAX_BOARD_NUM; ++byBoardIndex)
		{
			for (BYTE byControllerIndex = 0; byControllerIndex < DCM_MAX_CONTROLLERS_PRE_BOARD; ++byControllerIndex)
			{
				memcpy_s(&g_aCalibrationData[byBoardIndex][byControllerIndex * DCM_CHANNELS_PER_CONTROL], DCM_CHANNELS_PER_CONTROL * sizeof(CAL_DATA),
					InitCalData, DCM_CHANNELS_PER_CONTROL * sizeof(CAL_DATA));
			}

		}
	}

	for (auto& Board : g_mapValidBoard)
	{
		for (BYTE byControllerIndex = 0; byControllerIndex < DCM_MAX_CONTROLLERS_PRE_BOARD; ++byControllerIndex)
		{
			CCalibration::Instance()->SetCalibrationMemory(Board.first, byControllerIndex,
				&g_aCalibrationData[Board.second.m_byBoardIndex][byControllerIndex * DCM_CHANNELS_PER_CONTROL]);
		}
	}
}

void AddBoard()
{
	CDCM* pDCM = nullptr;
	BOARD_INFO BoardInfo;
	vector<int> vecInstanceID;
	g_pDCMDistribution->GetInstanceValid(vecInstanceID);
	for (int nBoardIndex = 0; nBoardIndex < g_byBoardCount;++nBoardIndex)
	{
		pDCM = g_pDCMDistribution->GetClass();
		BYTE bySlotNo = g_abyBoardSlot[nBoardIndex];
		for (auto InstanceID : vecInstanceID)
		{
			pDCM = g_pDCMDistribution->GetClass(nullptr, InstanceID);
			pDCM->AddBoard(bySlotNo, TRUE);
			if (0 == InstanceID)
			{
				BoardInfo.m_byBoardIndex = nBoardIndex;
				BoardInfo.m_usChannelCount = pDCM->GetChannelCount(bySlotNo, FALSE);
			}
		}
		g_mapValidBoard.insert(make_pair(bySlotNo, BoardInfo));

	}
	SetCalibrationMemory();
	SetMemory();
}

void APIENTRY dcm_InitializeInstance(int nInstanceID)
{
	CDCM* pDCM = g_pDCMDistribution->Initialize(nInstanceID);
	pDCM->Reset();
	for (auto& Board : g_mapValidBoard)
	{
		pDCM->AddBoard(Board.first, FALSE);
	}
	///<Get the instance count
	vector<int> vecInstanceID;
	g_pDCMDistribution->GetInstanceValid(vecInstanceID);
	g_nInstanceCount = -1;
	for (auto InstanceID : vecInstanceID)
	{
		g_nInstanceCount = g_nInstanceCount > nInstanceID ? g_nInstanceCount : nInstanceID;
	}
	++g_nInstanceCount;
}

/**
 * @class CVectorFileRecord
 * @brief The singleton class of vector file used for each instace
*/
class CVectorFileRecord
{
public:
	/**
	 * @brief Get the intance of singleton class
	 * @return The point of instance
	*/
	static CVectorFileRecord* Instance();
	/**
	 * @brief Set the vector file of the intance
	 * @param[in] nInstanceID The instance ID
	 * @param[in] strVectorFile The vector file used by the instance
	 * @return Execute result
	 * - 0 Set vector file successfully
	 * - -1 The file length is 0
	*/
	int SetVectorFile(int nInstanceID, const string& strVectorFile);
	/**
	 * @brief Check whether the instance monopolize vector file
	 * @param[in] nInstanceID The instance ID
	 * @return Whether the instance monopolize vector file
	 * - 0 The instance monopolize vector file
	 * - -1 Not record instance
	 * - -2 Not record file
	 * - -3 Share vector file
	*/
	int IsMonopolizeFile(int nInstanceID);
	/**
	 * @brief The vector file of the instance is invalid or close
	 * @param[in] nInstanceID The instance ID
	*/
	void SetVectorInvalid(int nInstanceID);
private:
	/**
	 * @brief Constructor
	*/
	CVectorFileRecord();
private:
	map<int, string> m_mapInstance;///<The vector file name of each instance, the key is instance ID and value is file name without path
	map<string, int> m_mapFileUsedCount;///<The instance count of the vector file used, key is file name without path and value is count
};

CVectorFileRecord* CVectorFileRecord::Instance()
{
	static CVectorFileRecord VectorFile;
	return &VectorFile;
}

int CVectorFileRecord::SetVectorFile(int nInstanceID, const string& strVectorFile)
{
	if (0 == strVectorFile.size())
	{
		return -1;
	}
	string strFile = strVectorFile;
	int nPos = strFile.rfind("\\");
	if (-1 != nPos)
	{
		strFile.erase(0, nPos + 1);
	}
	string strCurVectorFile;
	BOOL bFirstSetFile = FALSE;
	auto iterInstance = m_mapInstance.find(nInstanceID);
	if (m_mapInstance.end() == iterInstance)
	{
		m_mapInstance.insert(make_pair(nInstanceID, strFile));
		iterInstance = m_mapInstance.find(nInstanceID);
		bFirstSetFile = TRUE;
	}
	else
	{
		if (iterInstance->second == strFile)
		{
			///<The same vector file
			return 0;
		}
	}
	strCurVectorFile = iterInstance->second;
	iterInstance->second = strFile;

	auto iterFile = m_mapFileUsedCount.find(strCurVectorFile);

	if (bFirstSetFile)
	{
		///<The first vector file set for current file
		if (m_mapFileUsedCount.end() == iterFile)
		{
			m_mapFileUsedCount.insert(make_pair(strFile, 1));
			return 0;
		}
		++iterFile->second;
		return 0;
	}
	///<The file count used of previous file minus 1
	if (m_mapFileUsedCount.end() != iterFile)
	{
		if (0 != iterFile->second)
		{
			--iterFile->second;
		}
	}

	///<The file count used of new file add 1
	iterFile = m_mapFileUsedCount.find(strFile);
	if (m_mapFileUsedCount.end() == iterFile)
	{
		m_mapFileUsedCount.insert(make_pair(strFile, 1));
	}
	else
	{
		++iterFile->second;
	}
	return 0;
}

int CVectorFileRecord::IsMonopolizeFile(int nInstanceID)
{
	auto iterInstance = m_mapInstance.find(nInstanceID);
	if (m_mapInstance.end() == iterInstance)
	{
		return -1;
	}
	auto iterCount = m_mapFileUsedCount.find(iterInstance->second);
	if (m_mapFileUsedCount.end() == iterCount)
	{
		return -2;
	}
	int nCount = iterCount->second;
	if (1 == nCount)
	{
		return 0;
	}
	return -3;
}

void CVectorFileRecord::SetVectorInvalid(int nInstanceID)
{
	auto iterInstance = m_mapInstance.find(nInstanceID);
	if (m_mapInstance.end() == iterInstance)
	{
		return;
	}
	auto iterVector = m_mapFileUsedCount.find(iterInstance->second);
	if (m_mapFileUsedCount.end() != iterVector)
	{
		if (1 >= iterVector->second)
		{
			m_mapFileUsedCount.erase(iterVector);
		}
		else
		{
			--iterVector->second;
		}
	}
	m_mapInstance.erase(iterInstance);
}

CVectorFileRecord::CVectorFileRecord()
{
}

void APIENTRY dcm_DeleteInstance(int nInstanceID)
{
	if (0 != nInstanceID)
	{
		g_pDCMDistribution->Initialize(nInstanceID, TRUE);
	}

	if (g_nCurInstanceID == nInstanceID)
	{
		g_nCurInstanceID = 0;
		g_pDCMDistribution->SetInstanceID(g_nCurInstanceID);
	}
	g_pI2CDistribution->Initialize(nInstanceID, TRUE);
	--g_nInstanceCount;
	CVectorFileRecord::Instance()->SetVectorInvalid(nInstanceID);
}

/**
 * @brief Get the global channel of the board channel number
 * @return The channel number in global
 * - >=0 The channel number in global
 * - -1 The board is not existed
 * - -2 The channel is over range
*/
inline int GetGlobalChannel(CHANNEL_INFO& Channel)
{
	auto iterBoard = g_mapValidBoard.find(Channel.m_bySlotNo);
	if (g_mapValidBoard.end() == iterBoard)
	{
		return -1;
	}
	int nChannelCount = iterBoard->second.m_usChannelCount;
	if (Channel.m_usChannel >= nChannelCount)
	{
		return -2;
	}
	return iterBoard->second.m_byBoardIndex * DCM_MAX_CHANNELS_PER_BOARD + Channel.m_usChannel;
}

void APIENTRY dcm_set_config_info(short boardcnt, BOARDINFO* pAllBoardInfo)
{
	g_byBoardCount = 0;
	memset(g_abyBoardSlot, 0, sizeof(g_abyBoardSlot));
	auto mapValidBoard = g_mapValidBoard;
	g_mapValidBoard.clear();
	if (nullptr == pAllBoardInfo)
	{
		return;
	}
	BOARD_INFO BoardInfo;
	for (short sBoardIndex = 0; sBoardIndex < boardcnt; ++sBoardIndex)
	{
		BYTE bySlotNo = pAllBoardInfo[sBoardIndex].slot;
		BoardInfo.m_byBoardIndex = sBoardIndex;
		BoardInfo.m_usChannelCount = 0;
		auto iterSlot = mapValidBoard.find(bySlotNo);
		if (mapValidBoard.end() != iterSlot)
		{
			BoardInfo.m_usChannelCount = iterSlot->second.m_usChannelCount;
		}
		g_mapValidBoard.insert(make_pair(bySlotNo, BoardInfo));
		g_abyBoardSlot[g_byBoardCount++] = bySlotNo;
	}

	if (g_mapValidBoard != mapValidBoard)
	{
		SetCalibrationMemory();
		SetMemory();

		vector<USHORT> vecConnectChannel;
		vector<int> vecInstanceID;
		g_pDCMDistribution->GetInstanceValid(vecInstanceID);
		CDCM* pDCM = nullptr;
		BOOL bUpdate = TRUE;
		for (auto InstanceID : vecInstanceID)
		{
			pDCM = g_pDCMDistribution->GetClass(nullptr, InstanceID);
			if (nullptr == pDCM)
			{
				continue;
			}
			pDCM->Reset();
			for (auto& Board : g_mapValidBoard)
			{
				pDCM->AddBoard(Board.first, !bUpdate);
				if (bUpdate)
				{
					pDCM->UpdateChannelMode(Board.first);
					pDCM->GetConnectChannel(Board.first, vecConnectChannel, RELAY_TYPE::FUNC_RELAY);
					pDCM->GetConnectChannel(Board.first, vecConnectChannel, RELAY_TYPE::DC_RELAY);
					Board.second.m_usChannelCount = pDCM->GetChannelCount(Board.first, TRUE);
				}
			}
			bUpdate = TRUE;
		}
		if (0 != mapValidBoard.size())
		{
			///<Reload calibration information, in case of the board is not the same board before
			dcm_calibration(TRUE, 0xFF);
		}
	}
}

//#define _SHIELD_SELFCHECK 1

BOOL APIENTRY dcm_selfcheck(char* lpszFileName, BYTE byBoardNo, int* pnCheckResult)
{
#ifdef _SHIELD_SELFCHECK

	for (USHORT usChannel = 0; usChannel < DCM_MAX_CHANNELS_PER_BOARD; ++usChannel)
	{
		pnCheckResult[usChannel] = 1;
	}
	return 1;

#endif // _SHIELD_SELFCHECK

	if (byBoardNo >= g_byBoardCount)
	{
		return 0;
	}
	
	dcm_calibration(TRUE, byBoardNo);
	BYTE bySlotNo = g_abyBoardSlot[byBoardNo];
	CSelfCheck Selftest(*g_pDCMDistribution->GetClass(0));
	return Selftest.Check(lpszFileName, bySlotNo, pnCheckResult);
}

void APIENTRY dcm_set_cal_relay(BYTE bySlotNo, BYTE byChannel, BYTE byRelayStatus)
{
	CDCM* pDCM = g_pDCMDistribution->GetClass(0);
	pDCM->SetCalibartionRelay(bySlotNo, byChannel, byRelayStatus);
}

int APIENTRY dcm_cal_result_to_gVariable(UINT uGlobalChannel, DCM_CAL_MODE CalMode, BYTE byRange, float fSlope, float fOffset)
{
	BYTE byBoardNo = uGlobalChannel / DCM_MAX_CHANNELS_PER_BOARD;
	if (byBoardNo >= g_byBoardCount)
	{
		///<Board not existed
		return -2;
	}
	BYTE bySlotNo = g_abyBoardSlot[byBoardNo];
	USHORT usChannel = uGlobalChannel % DCM_MAX_CHANNELS_PER_BOARD;
	int nRetVal = CCalibration::Instance()->SetCalibration(bySlotNo, usChannel / DCM_CHANNELS_PER_CONTROL, usChannel % DCM_CHANNELS_PER_CONTROL,
		(CCalibration::CAL_TYPE)CalMode, (PMU_IRANGE)byRange, fSlope, fOffset);
	if (0 != nRetVal)
	{
		return -1;
	}

	return 0;
}

BYTE APIENTRY dcm_get_board_slot(short sBoardNo)
{
	if (g_byBoardCount <= sBoardNo)
	{
		return 0;
	}

	return g_abyBoardSlot[sBoardNo];
}

void APIENTRY dcm_relay_init(BYTE bySlotNo)
{
	CDCM* pDCM = g_pDCMDistribution->GetClass(0);
	pDCM->IntializeFunctionRelay(bySlotNo);
}

int APIENTRY dcm_getmcuresult(USHORT usSiteNo)
{
	CDCM* pDCM = g_pDCMDistribution->GetClass();
	CDriverAlarm* pAlarm = pDCM->GetAlarm();
	pAlarm->SetDriverPackName("GetMCUResult");
	int nRetVal = pDCM->GetMCUResult(usSiteNo);
	if (0 > nRetVal)
	{
		switch (nRetVal)
		{
		case -1:
			nRetVal = VECTOR_FILE_NOT_LOADED;
			break;
		case -2:
			nRetVal = VECTOR_NOT_RAN;
			break;
		case -3:
			nRetVal = SITE_ERROR;
			break;
		case -4:
			nRetVal = SITE_INVALID;
			break;
		case -5:
			nRetVal = VECTOR_RUNING;
			break;
		case -6:
			nRetVal = CHANNEL_NOT_EXISTED;
			break;
		case -7:
			nRetVal = BOARD_NOT_INSERT_ERROR;
			break;
		default:
			break;
		}
	}
	pAlarm->Output();
	return nRetVal;
}

int APIENTRY dcm_getcapturedata(const char* lpszPinName, const char* lpszStartLabel, USHORT usSiteNo, ULONG ulOffset, int nLineCount, ULONG& ulData)
{
	CDCM* pDCM = g_pDCMDistribution->GetClass();
	CDriverAlarm* pAlarm = pDCM->GetAlarm();
	pAlarm->SetDriverPackName("GetCaptureData");
	int nRetVal = pDCM->GetCaptureData(lpszPinName, usSiteNo, lpszStartLabel, ulOffset, nLineCount, ulData);
	if (0 > nRetVal)
	{
		switch (nRetVal)
		{
		case -1:
			nRetVal = VECTOR_FILE_NOT_LOADED;
			break;
		case -2:
			///<The point of pin name is nullptr
		case -3:
			nRetVal = PIN_NAME_ERROR;
			break;
		case -4:
			nRetVal = SITE_ERROR;
			break;
		case -5:
			nRetVal = SITE_INVALID;
			break;
		case -6:
			///<The pin is not belongs to the instance
			nRetVal = PIN_NOT_BELONGS;
			break;
		case -7:
			nRetVal = BOARD_NOT_INSERT_ERROR;
			break;
		case -8:
			///<The point of start label is nullptr
		case -9:
			nRetVal = START_LABEL_ERROR;
			break;
		case -10:
			nRetVal = OFFSET_ERROR;
			break;
		case -11:
			nRetVal = VECTOR_NOT_IN_LAST_RAN;
			break;
		case -12:
			nRetVal = VECTOR_NOT_RAN;
			break;
		case -13:
			nRetVal = VECTOR_RUNING;
			break;
		case -14:
			///<The channel number is over range, not will happen
			break;
		case -15:
			nRetVal = CHANNEL_NOT_EXISTED;
			break;
		case -16:
			nRetVal = FAIL_LINE_NOT_SAVE;
			break;
		case -17:
			nRetVal = CAPTURE_NOT_SUPPORTTED;
			break;
		default:
			break;
		}
		ulData = -1;
	}
	pAlarm->Output();
	return nRetVal;
}

int APIENTRY dcm_loadvectorfile(const char* lpszFileName, BOOL bReload)
{
	int nCurInstanceID = 0;
	CDCM* pCurDCM = g_pDCMDistribution->GetClass(&nCurInstanceID);
	CDriverAlarm* pAlarm = pCurDCM->GetAlarm();
	pAlarm->SetDriverPackName("LoadVectorFile");

	auto DeletePath = [](string& strFile)
	{
		int nPos = strFile.rfind("\\");
		if (-1 != nPos)
		{
			strFile.erase(0, nPos + 1);
		}
		nPos = strFile.rfind(".");
		if (-1 != nPos)
		{
			strFile.erase(nPos);
		}
	};

	///<Check whether the vector information file is valid
	string strConfig;
	GetConfigFile(strConfig);
	CConfigFile ConfigFile(strConfig.c_str());

	char lpszKey[64] = { 0 };
	_itoa_s(nCurInstanceID, lpszKey, sizeof(lpszKey), 10);
	const char* lpszValue = ConfigFile.GetValue("Vector", lpszKey);
	BOOL bVectorFileInfoInvalid = TRUE;
	if (nullptr != lpszValue)
	{
		string strValue = lpszValue;
		if (0 != strValue.size() && nullptr != lpszFileName)
		{
			string strCurFileConfig = lpszFileName;
			DeletePath(strCurFileConfig);
			if (0 != strValue.compare(strCurFileConfig))
			{
				ConfigFile.SetValue("Vector", lpszKey, "");
			}
			else
			{
				bVectorFileInfoInvalid = FALSE;
			}
		}
	}
	if (bVectorFileInfoInvalid)
	{
		pCurDCM->DelteVectorInfoFile(lpszFileName);
	}

	int nRetVal = pCurDCM->LoadVectorFile(lpszFileName, bReload);
	if (0 != nRetVal)
	{
		nRetVal = VECTOR_FILE_NOT_LOADED;
	}
	else
	{
		USHORT usSiteCount = pCurDCM->GetSiteCount();
		if (g_usMaxSiteCount < usSiteCount)
		{
			g_usMaxSiteCount = usSiteCount;
		}
		///<Reset the I2C belongs to the instance
		UINT uBRAMStartLine = 0;
		UINT uDRAMStartLine = 0;
		CI2C* pI2C = g_pI2CDistribution->GetClass(nullptr, nCurInstanceID);
		if (nullptr != pI2C)
		{
			pI2C->Reset(FALSE);
			pCurDCM->GetVectorMemoryLeft(uBRAMStartLine, uDRAMStartLine);
			pI2C->SetBaseLine(uBRAMStartLine, uDRAMStartLine);
		}
		///<Save the pair information between vector file and instance
		CVectorFileRecord* pVectorFile = CVectorFileRecord::Instance();
		pVectorFile->SetVectorFile(nCurInstanceID, lpszFileName);

		CPinNoManage::Instance()->SetPinCount(nCurInstanceID, pCurDCM->GetPinCount(), TRUE);

		string strFileName;
		string strVectorFileName = lpszFileName;
		DeletePath(strVectorFileName);

		sprintf_s(lpszKey, "%d", nCurInstanceID);
		ConfigFile.SetValue("Vector", lpszKey, strVectorFileName.c_str());
		ConfigFile.SetValue("Shared", lpszKey, "-1");

		char lpszShared[8] = { 0 };
		_itoa_s(nCurInstanceID, lpszShared, sizeof(lpszShared), 10);

		vector<int> vecInstanceID;
		g_pDCMDistribution->GetInstanceValid(vecInstanceID);
		CDCM* pDCM = nullptr;
		for (auto InstanceID : vecInstanceID)
		{
			if (nCurInstanceID == InstanceID)
			{
				continue;
			}
			pDCM = g_pDCMDistribution->GetClass(nullptr, InstanceID);
			if (nullptr != pDCM)
			{
				if (pDCM->IsLoadVector(strFileName))
				{
					continue;
				}
				pVectorFile->SetVectorFile(InstanceID, lpszFileName);///<Record vector file
				pDCM->CopyVectorInfo(*pCurDCM);
				///<Reset the I2C belongs to the instance
				pI2C = g_pI2CDistribution->GetClass(nullptr, InstanceID);
				if (nullptr != pI2C)
				{
					pI2C->Reset(FALSE);
					pI2C->SetBaseLine(uBRAMStartLine, uDRAMStartLine);
				}
				sprintf_s(lpszKey, "%d", InstanceID);

				ConfigFile.SetValue("Vector", lpszKey, strVectorFileName.c_str());
				ConfigFile.SetValue("Shared", lpszKey, lpszShared);
			}
		}
		g_bAllowDebugToolShow = TRUE;
	}
	ConfigFile.Save();
	pAlarm->Output();
	return nRetVal;
}

int APIENTRY dcm_stopvector(const char* lpszPinGroup)
{
	CDCM* pDCM = g_pDCMDistribution->GetClass();
	CDriverAlarm* pAlarm = pDCM->GetAlarm();
	pAlarm->SetDriverPackName("StopVector");
	int nRetVal = pDCM->Stop(lpszPinGroup);
	if (0 != nRetVal)
	{
		switch (nRetVal)
		{
		case -1:
			nRetVal = VECTOR_FILE_NOT_LOADED;
			break;
		case -2:
			///<The point of pin group is nullptr
		case -3:
			nRetVal = PIN_GROUP_ERROR;
			break;
		case -4:
			///<No valid site
			nRetVal = SITE_INVALID;
			break;
		case -5:
			///<The pin is not belongs to
			nRetVal = PIN_NOT_BELONGS;
			break;
		case -6:
			nRetVal = BOARD_NOT_INSERT_ERROR;
			break;
		default:
			break;
		}
	}
	pAlarm->Output();
	return nRetVal;
}

int APIENTRY dcm_VectorBoardNum(USHORT* pusSiteNum)
{
	CDCM* pDCM = g_pDCMDistribution->GetClass();
	if (nullptr == pDCM)
	{
		return -1;
	}
	return pDCM->GetVectorBoardCount(pusSiteNum);
}

/**
 * @brief The class for check controller overlap
*/
class CInstanceCheck
{
public:
	/**
	 * @brief Contructor
	 * @param[in] vecChannel The channel will be checked
	*/
	CInstanceCheck(const std::vector<CHANNEL_INFO>& vecChannel);
	/**
	 * @brief Check the channel
	 * @param[in] vecChannelExisted The existed channel
	 * @param[out] byControllerOverlap The overlaped controller
	 * @return Check result
	 * - 0 No overlap controller
	 * - >0 The board overlaped
	*/
	int CheckChannel(const std::vector<CHANNEL_INFO>& vecChannelExisted, BYTE& byControllerOverlap);
private:
	/**
	 * @brief Classify channel to controller
	 * @param[in] vecChannel The channels will be classified
	 * @param[out] mapController The controller classified from channel
	*/
	void Classify(const vector<CHANNEL_INFO>& vecChannel, map<BYTE, set<BYTE>>& mapController);
private:
	CClassifyBoard m_BoardClassify;
	map<BYTE, set<BYTE>> m_mapBoardChecked;
};

CInstanceCheck::CInstanceCheck(const std::vector<CHANNEL_INFO>& vecChannel)
{
	Classify(vecChannel, m_mapBoardChecked);
}


int CInstanceCheck::CheckChannel(const std::vector<CHANNEL_INFO>& vecChannelExisted, BYTE& byControllerOverlap)
{
	map<BYTE, set<BYTE>> mapController;
	Classify(vecChannelExisted, mapController);
	BOOL bOverlap = FALSE;
	int nBoardOverlap = 0;
	auto iterController = mapController.begin();
	for (auto& Board : m_mapBoardChecked)
	{
		iterController = mapController.find(Board.first);
		if (mapController.end() == iterController)
		{
			continue;
		}
		
		for (auto Controller : Board.second)
		{
			if (iterController->second.end() != iterController->second.find(Controller))
			{
				bOverlap = TRUE;
				byControllerOverlap = Controller;
				break;
			}
		}
		if (bOverlap)
		{
			nBoardOverlap = Board.first;
			break;
		}
	}
	return nBoardOverlap;
}

void CInstanceCheck::Classify(const vector<CHANNEL_INFO>& vecChannel, map<BYTE, set<BYTE>>& mapController)
{
	mapController.clear();
	m_BoardClassify.SetChannel(vecChannel);
	set<BYTE> setBoard;
	set<BYTE> setController;
	m_BoardClassify.GetBoard(setBoard);
	vector<USHORT> vecBoardChannel;
	CBoardChannelClassify BoardChannel;
	for (auto Slot : setBoard)
	{
		m_BoardClassify.GetBoardChannel(Slot, vecBoardChannel);
		BoardChannel.GetController(vecBoardChannel, setController);
		mapController.insert(make_pair(Slot, setController));
	}
}

int APIENTRY dcm_SetPinGroup(const char* lpszPinGroupName, const char* lpszPinNameList)
{
	int nInstanceID = 0;
	CDCM* pDCM = g_pDCMDistribution->GetClass(&nInstanceID);
	CDriverAlarm* pAlarm = pDCM->GetAlarm();

	pAlarm->SetDriverPackName("SetPinGroup");
	int nRetVal = pDCM->SetPinGroup(lpszPinGroupName, lpszPinNameList);
	if (0 != nRetVal)
	{
		switch (nRetVal)
		{
		case -1:
			nRetVal = VECTOR_FILE_NOT_LOADED;
			break;
		case -2:
			nRetVal = BOARD_NOT_INSERT_ERROR;
			break;
		case -3:
			if (nullptr == lpszPinGroupName)
			{
				///<The point of pin group is nullptr
				pAlarm->SetParamName("lpszPinGroupName");
				nRetVal = PIN_GROUP_FORMAT_ERROR;
			}
			else
			{
				///<The point of pin name string is nullptr
				pAlarm->SetParamName("lpszPinNameList");
				return PIN_NAME_ERROR;
			}
			break;
		case -4:
			pAlarm->SetParamName("lpszPinGroupName");
			nRetVal = PIN_GROUP_FORMAT_ERROR;
			break;
		case -5:
		case -6:
		case -7:
			pAlarm->SetParamName("lpszPinNameList");
			nRetVal = PIN_NAME_ERROR;
			break;
		case -8:
			pAlarm->SetParamName("lpszPinNameList");
			nRetVal = PIN_NOT_BELONGS;
			break;
		case -9:
			pAlarm->SetParamName("lpszPinGroupName");
			nRetVal = PIN_GROUP_NAME_CONFLICT;
			break;
		default:
			pAlarm->SetParamName("lpszPinNameList");
			nRetVal = PIN_NAME_ERROR;
			break;
		}
	}
	else
	{
		if (0 != nInstanceID)
		{
			pDCM->DeleteUnusedBoard();
		}
		if (1 < g_nInstanceCount)
		{
			///<Check the controller if the instance count is not less than 1
			vector<CHANNEL_INFO> vecChannel;
			pDCM->GetPinGroupChannel(lpszPinGroupName, 0xFFFF, vecChannel);
			CInstanceCheck Check(vecChannel);

			int nBoardOverlap = 0;
			BYTE byControllerOverlap = 0;

			vector<int> vecInstance;
			g_pDCMDistribution->GetInstanceValid(vecInstance);
			for (auto Instance : vecInstance)
			{
				if (Instance == nInstanceID)
				{
					continue;
				}
				pDCM = g_pDCMDistribution->GetClass(nullptr, Instance);
				if (nullptr == pDCM)
				{
					continue;
				}
				pDCM->GetAllPinGroupChannel(vecChannel);
				nBoardOverlap = Check.CheckChannel(vecChannel, byControllerOverlap);
				if (0 == nBoardOverlap)
				{
					continue;
				}
				pAlarm->SetAlarmID(ALARM_ID::ALARM_INSTANCE_OVERLAP);
				pAlarm->SetAlarmLevel(CDriverAlarm::ALARM_LEVEL::AlarmWarning);
				pAlarm->SetPinString(lpszPinGroupName, FALSE);
				int nChannelStart = byControllerOverlap * DCM_CHANNELS_PER_CONTROL;
				pAlarm->SetAlarmMsg("The controller %d(Channel %d - %d) in board(S%d) is used in more than one instance.", byControllerOverlap, nChannelStart,
					nChannelStart + DCM_CHANNELS_PER_CONTROL, nBoardOverlap);
				break;
			}
		}
	}

	pAlarm->Output();
	return nRetVal;
}

int APIENTRY dcm_GetInstanceID(const char* lpszPinGroup)
{
	if (nullptr == lpszPinGroup)
	{
		return g_nCurInstanceID;
	}
	auto iterPinGroup = g_mapPinGroup.find(lpszPinGroup);
	if (g_mapPinGroup.end() != iterPinGroup)
	{
		return iterPinGroup->second;
	}
	CDCM* pDCM = g_pDCMDistribution->GetClass(nullptr, g_nCurInstanceID);
	if (nullptr == pDCM && pDCM->IsPinExisted(lpszPinGroup))
	{
		return g_nCurInstanceID;
	}
	vector<int> vecInstanceID;
	g_pDCMDistribution->GetInstanceValid(vecInstanceID);
	for (auto InstanceID : vecInstanceID)
	{
		pDCM = g_pDCMDistribution->GetClass(nullptr, InstanceID);
		if (nullptr == pDCM)
		{
			continue;
		}
		if (pDCM->IsPinExisted(lpszPinGroup))
		{
			return InstanceID;
		}
	}
	return -2;
}

double APIENTRY dcm_GetPPMUSetValue(BYTE bySlotNo, USHORT uChannelNo, BYTE& byPPMUMode, BYTE& byPPMUIRange)
{
	CDCM* pDCM = g_pDCMDistribution->GetClass();
	if (nullptr == pDCM)
	{
		return PMU_MEASURE_ERROR;
	}
	PMU_MODE PMUMode = PMU_MODE::FIMI;
	PMU_IRANGE PMUIRange = PMU_IRANGE::IRANGE_20UA;
	double dSetValue = 0;
	dSetValue = pDCM->GetPMUSettings(bySlotNo, uChannelNo, PMUMode, PMUIRange);
	byPPMUMode = (BYTE)PMUMode;
	byPPMUIRange = (BYTE)PMUIRange;
	return dSetValue;
}

double APIENTRY dcm_GetTimeSetPeriod(BYTE bySlotNo, BYTE byControlNo, BYTE byTimeSet)
{
	CDCM* pDCM = g_pDCMDistribution->GetClass();
	if (nullptr == pDCM)
	{
		return -1;
	}
	return pDCM->GetPeriod(bySlotNo, byControlNo, byTimeSet);
}

ULONG APIENTRY dcm_GetValidBoard()
{
	ULONG ulBoard = 0;
	for (BYTE byBoardIndex = 0; byBoardIndex < g_byBoardCount; ++byBoardIndex)
	{
		ulBoard |= 1 << byBoardIndex;
	}
	return ulBoard;
}

int dcm_GetRelayStatus(BYTE bySlotNo, USHORT usChannel, BYTE byRelayType)
{
	if (DCM_MAX_CHANNELS_PER_BOARD <= usChannel)
	{
		return -1;
	}
	vector<USHORT> vecConnectChannel;
	RELAY_TYPE RelayType = RELAY_TYPE::FUNC_RELAY;
	switch (byRelayType)
	{
	case 0:
		RelayType = RELAY_TYPE::FUNC_RELAY;
		break;
	case 1:
		RelayType = RELAY_TYPE::HIGH_VOLTAGE_RELAY;
		break;
	case 2:
		RelayType = RELAY_TYPE::DC_RELAY;
		break;
	default:
		return -2;
		break;
	}
	CDCM* pDCM = g_pDCMDistribution->GetClass(nullptr, 0);
	if (nullptr == pDCM)
	{
		return -1;
	}
	int nRetVal = pDCM->GetConnectChannel(bySlotNo, vecConnectChannel, RelayType);
	if (0 != nRetVal)
	{
		switch (nRetVal)
		{
		case -1:
			nRetVal = -3;
			break;
		case -2:
			nRetVal = -4;
			break;
		default:
			break;
		}
		return nRetVal;
	}
	if (DCM_MAX_CHANNELS_PER_BOARD <= usChannel)
	{
		return -3;
	}
	for (USHORT usConnectChannel : vecConnectChannel)
	{
		if (usChannel == usConnectChannel)
		{
			return 1;
		}
	}
	return 0;
}

int APIENTRY dcm_GetBoardInfo(BYTE* pbySlotNo, int nArrayLenth)
{
	if (nullptr == pbySlotNo || g_byBoardCount > nArrayLenth)
	{
		return g_byBoardCount;
	}
	for (int nBoardIndex = 0; nBoardIndex < g_byBoardCount;++nBoardIndex)
	{
		pbySlotNo[nBoardIndex] = g_abyBoardSlot[nBoardIndex];
	}
	return g_byBoardCount;
}

int APIENTRY dcm_GetSlotSite(BYTE bySlotNo, USHORT* pusSite, int nArrayLength)
{
	vector<USHORT> vecSite;
	CDCM* pDCM = g_pDCMDistribution->GetClass();
	if (nullptr == pDCM)
	{
		return -1;
	}
	pDCM->GetBoardSite(bySlotNo, vecSite);
	USHORT usSiteCount = vecSite.size();
	if (nullptr == pusSite || usSiteCount > nArrayLength)
	{
		return usSiteCount;
	}
	for (USHORT usSiteIndex = 0; usSiteIndex < usSiteCount;++usSiteIndex)
	{
		pusSite[usSiteIndex] = vecSite[usSiteIndex];
	}
	return usSiteCount;
}

int APIENTRY dcm_GetPinGroupChannel(const char* lpszPinGroup, USHORT usSiteNo, BYTE* pbySlotNo, USHORT* pusChannel, int nElementCount)
{
	CDCM* pDCM = g_pDCMDistribution->GetClass();
	if (nullptr == pDCM)
	{
		return -1;
	}
	vector<CHANNEL_INFO> vecChannel;
	int nRetVal = pDCM->GetPinGroupChannel(lpszPinGroup, usSiteNo, vecChannel);
	if (0 != nRetVal)
	{
		switch (nRetVal)
		{
		case -1:
			///<Vector not loaded
			nRetVal = -1;
			break;
		case -2:
			///<The point of pin group is nullptr
			nRetVal = -2;
			break;
		case -3:
			///<The pin group is not defined before
			nRetVal = -3;
			break;
		case -4:
			///<The site number is over range
			nRetVal = -4;
			break;
		case -5:
			///<The pin is not belongs to
			nRetVal = -5;
			break;
		default:
			break;
		}
		return nRetVal;
	}
	int nChannelCount = vecChannel.size();
	if (nullptr == pbySlotNo || nullptr == pusChannel || nChannelCount > nElementCount)
	{
		return nChannelCount;
	}
	for (int nChannel = 0; nChannel < nChannelCount; nChannel++)
	{
		pbySlotNo[nChannel] = vecChannel[nChannel].m_bySlotNo;
		pusChannel[nChannel] = vecChannel[nChannel].m_usChannel;
	}
	return nChannelCount;
}

int APIENTRY dcm_GetVectorSiteCount()
{
	CDCM* pDCM = g_pDCMDistribution->GetClass();
	if (nullptr == pDCM)
	{
		return -1;
	}
	return pDCM->GetSiteCount();
}

CI2C* GetI2C(CDriverAlarm* &pAlarm, const char* lpszFunctionName)
{
	int nCurInstanceID = 0;
	CI2C* pI2C = g_pI2CDistribution->GetClass(&nCurInstanceID);
	if (nullptr == pI2C)
	{
		if (nullptr != lpszFunctionName)
		{
			pAlarm = CAlarmManage::Instance()->GetAlarm(nCurInstanceID);
			pAlarm->SetDriverPackName(lpszFunctionName);
			pAlarm->SetAlarmID(ALARM_ID::ALARM_I2C_NOT_SET_SITE);
			pAlarm->SetAlarmMsg("Not set the channel information through I2CSet before.");
			pAlarm->Output();
		}
		return nullptr;
	}
	pAlarm = pI2C->GetAlarm();
	pAlarm->SetDriverPackName(lpszFunctionName);
	return pI2C;
}

void GetI2CConfig(std::string& strFilePath)
{
	HMODULE hDCM = GetModuleHandle("DCM.dll");
	char lpszPath[MAX_PATH] = { 0 };
	GetModuleFileName(hDCM, lpszPath, sizeof(lpszPath));
	strFilePath = lpszPath;
	int nPos = strFilePath.rfind("\\");
	if (-1 != nPos)
	{
		strFilePath.erase(nPos + 1);
	}
	strFilePath += "DCM\\";
	CreateDirectory(strFilePath.c_str(), nullptr);
	strFilePath += "I2C.ini";
}

int APIENTRY dcm_I2CSet(float Time, int nSiteNum, BYTE byREGAddrMode, const char* lpszSCLChannel, const char* lpszSDAChannel)
{
	int nCurInstanceID = 0;
	CI2C* pI2C = g_pI2CDistribution->GetClass(&nCurInstanceID);
	if (nullptr == pI2C)
	{
		g_pI2CDistribution->Initialize(g_nCurInstanceID, false);
		pI2C = g_pI2CDistribution->GetClass(&nCurInstanceID);
	}
	CDriverAlarm* pAlarm = pI2C->GetAlarm();
	pAlarm->SetDriverPackName("I2CSet");

	pI2C->Reset();
	UINT uBRAMStartLine = 0;
	UINT uDRAMStartLine = 0;
	CDCM* pDCM = g_pDCMDistribution->GetClass(nullptr, nCurInstanceID);
	if (nullptr != pDCM)
	{
		pDCM->GetVectorMemoryLeft(uBRAMStartLine, uDRAMStartLine);
	}
	vector<BYTE> vecValidBoard;
	for (auto& Board : g_mapValidBoard)
	{
		vecValidBoard.push_back(Board.first);
	}
	pI2C->SetExistedBoard(vecValidBoard);

	pI2C->SetBaseLine(uBRAMStartLine, uDRAMStartLine);
	int nRetVal = pI2C->Set(Time, nSiteNum, CI2C::REG_MODE(byREGAddrMode), lpszSCLChannel, lpszSDAChannel);
	if (0 != nRetVal)
	{
		switch (nRetVal)
		{
		case -1:
			///<Site count is over range
			nRetVal = -1;
			break;
		case -2:
			///<Register count is over range
			nRetVal = -2;
			break;
		case -3:
			///<The point channel information is nullptr
		case -4:
			///<The channel information is blank
			nRetVal = -3;
			break;
		case -5:
			///<The format is error
			nRetVal = -6;
			break;
		case -6:
			///<The channel is over range
			nRetVal = -5;
			break;
		case -7:
			///<The channel is not existed
			nRetVal = -4;
			break;
		case -8:
			///<Site and channel count is not match
			nRetVal = -7;
			break;
		case -9:
			///<Channel conflict
			nRetVal = -8;
			break;
		default:
			break;
		}
	}
	else
	{
		string strFilePath;
		GetI2CConfig(strFilePath);

		CConfigFile ConfigFile(strFilePath.c_str());

		char lpszSection[32] = { 0 };
		char lpszKey[32] = { 0 };

		const char* lpszValue = ConfigFile.GetValue("Instance", "Count");
		auto GetNum = [&]() -> int
		{
			if (nullptr == lpszValue)
			{
				return -1;
			}
			return atoi(lpszValue);
		};

		int nMaxInstance = GetNum();
		nMaxInstance = nMaxInstance > (nCurInstanceID + 1) ? nMaxInstance : (nCurInstanceID + 1);
		ConfigFile.SetValue("Instance", "Count", "%d", nMaxInstance);
		_itoa_s(nCurInstanceID, lpszSection, sizeof(lpszSection), 10);
		ConfigFile.ClearSection(lpszSection);
		USHORT usSiteCount = pI2C->GetSiteCount();
		if (g_usMaxSiteCount < usSiteCount)
		{
			g_usMaxSiteCount = usSiteCount;
		}

		ConfigFile.SetValue(lpszSection, "SiteCount", "%d", usSiteCount);

		ConfigFile.SetValue(lpszSection, "Period", "%.0f", pI2C->GetPeriod());
		ConfigFile.SetValue(lpszSection, "RegisterByteCount", "%d", pI2C->GetRegisterByteCount());

		CHANNEL_INFO SCLChannel;
		CHANNEL_INFO SDAChannel;
		for (USHORT usSiteNo = 0; usSiteNo < usSiteCount;++usSiteNo)
		{
			sprintf_s(lpszKey, sizeof(lpszKey), "SITE_%d", usSiteNo + 1);
			pI2C->GetChannel(usSiteNo, TRUE, SCLChannel);
			pI2C->GetChannel(usSiteNo, FALSE, SDAChannel);
			ConfigFile.SetValue(lpszSection, lpszKey, "S%d_%d,S%d_%d", SCLChannel.m_bySlotNo, SCLChannel.m_usChannel, SDAChannel.m_bySlotNo, SDAChannel.m_usChannel);
		}
		ConfigFile.Save();
		g_bAllowDebugToolShow = TRUE;
	}

	pAlarm->Output();
	return nRetVal;
}

int APIENTRY dcm_I2CSetPinLevel(double dVIH, double dVIL, double dVOH, double dVOL)
{
	CDriverAlarm* pAlarm = nullptr;
	CI2C* pI2C = GetI2C(pAlarm, "I2CSetPinLevel");
	if (nullptr == pI2C)
	{
		return -2;
	}
	int nRetVal = pI2C->SetPinLevel(dVIH, dVIL, dVOH, dVOL, 3);
	if (0 != nRetVal)
	{
		switch (nRetVal)
		{
		case -1:
			///<Not set channel information 
			nRetVal = -2;
			break;
		case -2:
			///<The pin level error
			nRetVal = -1;
			break;
		default:
			break;
		}
	}
	pAlarm->Output();
	return nRetVal;
}

int APIENTRY dcm_I2CWriteData(BYTE bySAddress, DWORD dwRegAddress, int nDataLength, DWORD dwDataArray[])
{
	CDriverAlarm* pAlarm = nullptr;
	CI2C* pI2C = GetI2C(pAlarm, "I2CWriteData");
	if (nullptr == pI2C)
	{
		return -2;
	}
	USHORT usSiteCount = pI2C->GetSiteCount();
	if (0 == usSiteCount)
	{
		pAlarm->SetAlarmID(ALARM_ID::ALARM_I2C_NOT_SET_SITE);
		pAlarm->SetAlarmMsg("Not set I2C channel before.");
		pAlarm->Output();
		return -2;
	}
	if (0 >= nDataLength || 4 < nDataLength)
	{
		pAlarm->SetParamName("nDataLength");
		pAlarm->SetAlarmMsg("The byte count(%d) of data written is over range([%d,%d]).", nDataLength, 0, 4);
		pAlarm->SetAlarmID(ALARM_ID::ALARM_I2C_DATA_LENGTH_ERROR);
		pAlarm->SetAlarmLevel(CDriverAlarm::ALARM_LEVEL::AlarmError);
		pAlarm->Output();
		return -1;
	}
	BYTE** ppbySiteData = nullptr;
	try
	{
		ppbySiteData = new BYTE * [usSiteCount];
		for (USHORT usSiteNo = 0; usSiteNo < usSiteCount;++usSiteNo)
		{
			ppbySiteData[usSiteNo] = new BYTE[nDataLength];
			for (BYTE byDataIndex = 0; byDataIndex < nDataLength;++byDataIndex)
			{
				ppbySiteData[usSiteNo][byDataIndex] = BYTE(dwDataArray[usSiteNo] >> ((nDataLength - 1 - byDataIndex) * 8));
			}
		}
	}
	catch (const std::exception&)
	{
		pAlarm->SetAlarmID(ALARM_ID::ALARM_ALLOCTE_MEMORY_ERROR);
		pAlarm->SetAlarmMsg("Allocate %d bytes memory fail.", usSiteCount * nDataLength);
		pAlarm->Output();
		return -3;
	}
	
	int nRetVal = pI2C->Write(bySAddress, dwRegAddress, nDataLength, ppbySiteData);
	if (nullptr != ppbySiteData)
	{
		for (USHORT usSiteNo = 0; usSiteNo < usSiteCount;++usSiteNo)
		{
			if (nullptr != ppbySiteData[usSiteNo])
			{
				delete[] ppbySiteData[usSiteNo];
				ppbySiteData[usSiteNo] = nullptr;
			}
		}
		delete[] ppbySiteData;
		ppbySiteData = nullptr;
	}
	if (0 != nRetVal)
	{
		switch (nRetVal)
		{
		case -2:
			//Not happen
			nRetVal = -1;
			break;
		case -3:
			///<Data point is nullptr, not will happen
			break;
		case -4:
			///<Line not enough
			nRetVal = -4;
		default:
			break;
		}
	}
	pAlarm->Output();
	return nRetVal;
}

int APIENTRY dcm_I2CWriteMultiData(BYTE bySAddress, DWORD dwRegAddress, int nDataLength, BYTE* pbyDataAddressArray[])
{
	CDriverAlarm* pAlarm = nullptr;
	CI2C* pI2C = GetI2C(pAlarm, "I2CWriteMultiData");
	if (nullptr == pI2C)
	{
		return -2;
	}
	if (I2C_WRITE_MAX_BYTE_COUNT < nDataLength || 0 >= nDataLength)
	{
		pAlarm->SetParamName("nDataLength");
		pAlarm->SetAlarmMsg("The byte count(%d) of data written is over range[1, %d].", nDataLength, I2C_WRITE_MAX_BYTE_COUNT);
		pAlarm->SetAlarmID(ALARM_ID::ALARM_I2C_DATA_LENGTH_ERROR);
		pAlarm->Output();
		return -1;
	}
	int nRetVal = pI2C->Write(bySAddress, dwRegAddress, nDataLength, pbyDataAddressArray);
	if (0 != nRetVal)
	{
		switch (nRetVal)
		{
		case -1:
			///<Not set channel information
			nRetVal = -2;
			break;
		case -2:
			///<The length is too less, Checked before
			break;
		case -3:
			///<The parameter is nullptr
			pAlarm->SetParamName("pbyDataAddressArray");
			nRetVal = -3;
			break;
		case -4:
			///<Line is not enough
			nRetVal = -4;
			break;
		default:
			break;
		}
	}
	pAlarm->Output();
	return nRetVal;
}

int APIENTRY dcm_I2CWriteSameData(BYTE bySAddress, DWORD dwRegAddress, int nDataLength, const BYTE* pbyDataWritten)
{
	CDriverAlarm* pAlarm = nullptr;
	CI2C* pI2C = GetI2C(pAlarm, "I2CWriteData");
	if (nullptr == pI2C)
	{
		return -2;
	}
	if (I2C_WRITE_MAX_BYTE_COUNT < nDataLength || 0 >= nDataLength)
	{
		pAlarm->SetParamName("nDataLength");
		pAlarm->SetAlarmMsg("The byte count(%d) of data written is over range[1, %d].", nDataLength, I2C_WRITE_MAX_BYTE_COUNT);
		pAlarm->SetAlarmID(ALARM_ID::ALARM_I2C_DATA_LENGTH_ERROR);
		pAlarm->Output();
		return -1;
	}
	int nRetVal = pI2C->Write(bySAddress, dwRegAddress, nDataLength, pbyDataWritten);
	if (0 != nRetVal)
	{
		switch (nRetVal)
		{
		case -1:
			///<Not set channel information
			nRetVal = -2;
			break;
		case -2:
			///<The length is too less, Checked before
			break;
		case -3:
			///<The parameter is nullptr
			pAlarm->SetParamName("pbyDataWritten");
			nRetVal = -3;
			break;
		case -4:
			///<Line is not enough
			nRetVal = -4;
			break;
		default:
			break;
		}
	}
	pAlarm->Output();
	return nRetVal;
}

int APIENTRY dcm_I2CReadData(BYTE bySAddress, DWORD dwRegAddress, int nDataCount)
{
	CDriverAlarm* pAlarm = nullptr;
	CI2C* pI2C = GetI2C(pAlarm, "I2CReadData");
	if (nullptr == pI2C)
	{
		return -2;
	}
	if (I2C_READ_MAX_BYTE_COUNT < nDataCount || 0 >= nDataCount)
	{
		pAlarm->SetParamName("nDataCount");
		pAlarm->SetAlarmMsg("The byte count(%d) of data read is over range[1,%d].", nDataCount, I2C_READ_MAX_BYTE_COUNT);
		pAlarm->SetAlarmID(ALARM_ID::ALARM_I2C_DATA_LENGTH_ERROR);
		pAlarm->Output();
		return -1;
	}
	int nRetVal = pI2C->Read(bySAddress, dwRegAddress, nDataCount);
	if (0 != nRetVal)
	{
		switch (nRetVal)
		{
		case -1:
			///<Not set channel information
			nRetVal = -3;
			break;
		case -2:
			///<The line is not enough
			nRetVal = -2;
			break;
		default:
			break;
		}
	}
	pAlarm->Output();
	return nRetVal;
}

int APIENTRY dcm_GetNACKIndex(USHORT usSiteNo)
{
	CDriverAlarm* pAlarm = nullptr;
	CI2C* pI2C = GetI2C(pAlarm, "I2CGetNACKIndex");
	if (nullptr == pI2C)
	{
		return -3;
	}
	int nRetVal = pI2C->GetNACKIndex(usSiteNo);
	if (0 > nRetVal)
	{
		switch (nRetVal)
		{
		case -1:
			///<Not set channel information
			nRetVal = -3;
			break;
		case -2:
			///<The site is over range
			nRetVal = -1;
			break;
		case -3:
			///<Site invalid
			pAlarm->SetParamName("usSiteNo");
			nRetVal = -2;
			break;
		case -4:
			///<Not read or write before
			nRetVal = -4;
			break;
		case -5:
			///<The board of the site is not existed
			nRetVal = -5;
			break;
		default:
			break;
		}
	}
	pAlarm->Output();
	return nRetVal;
}

int APIENTRY dcm_I2CGetReadData(USHORT usSiteNo, int nDataIndex)
{
	CDriverAlarm* pAlarm = nullptr;
	CI2C* pI2C = GetI2C(pAlarm, "I2CGetReadData");
	if (nullptr == pI2C)
	{
		return -4;
	}
	int nRetVal = pI2C->GetReadData(usSiteNo, nDataIndex);
	if (0 > nRetVal)
	{
		switch (nRetVal)
		{
		case -1:
			///<Not set channel information
			nRetVal = -4;
			break;
		case -2:
			///<Not read before
			nRetVal = -5;
			break;
		case -3:
			///<Site over range
			pAlarm->SetParamName("usSiteNo");
			nRetVal = -1;
			break;
		case -4:
			///<The site is invalid
			pAlarm->SetParamName("usSiteNo");
			nRetVal = -2;
			break;
		case -5:
			pAlarm->SetParamName("nDataIndex");
			nRetVal = -3;
			break;
		case -6:
			pAlarm->SetParamName("usSiteNo");
			nRetVal = -6;
			break;
		default:
			break;
		}
	}
	pAlarm->Output();
	return nRetVal;
}

void APIENTRY dcm_I2CDeleteMemory()
{
	CI2C* pI2C = g_pI2CDistribution->GetClass();
	if (nullptr == pI2C)
	{
		return;
	}
	pI2C->Reset();
}

ULONG APIENTRY dcm_I2CGetBitData(USHORT usSiteNo, int nStartBitIndex, int nBitCount)
{
	CDriverAlarm* pAlarm = nullptr;
	CI2C* pI2C = GetI2C(pAlarm, "I2CGetBitData");
	if (nullptr == pI2C)
	{
		return -1;
	}
	LONGLONG llData = 0;
	ULONG ulAdd = 0;
	int nRetVal = 0;
	do
	{
		USHORT usSiteCount = pI2C->GetSiteCount();
		if (0 == usSiteCount)
		{
			pAlarm->SetAlarmID(ALARM_ID::ALARM_I2C_NOT_SET_SITE);
			pAlarm->SetAlarmMsg("Not set the channel information through I2CSet before.");
			nRetVal = -1;
			break;
		}
		if (usSiteCount <= usSiteNo)
		{
			pAlarm->SetAlarmID(ALARM_ID::ALARM_SITE_OVER_RANGE);
			pAlarm->SetAlarmType(CDriverAlarm::ALARM_TYPE::PARAMETER_OVERRANGE);
			pAlarm->SetParamName("usSiteNo");
			pAlarm->SetAlarmMsg("The site number(%d) is over range[%d,%d].", usSiteNo, 0, usSiteCount - 1);
			nRetVal = -1;
			break;
		}
		else if (!pI2C->IsSiteValid(usSiteNo))
		{
			// 		m_pAlarm->SetAlarmLevel(CDriverAlarm::ALARM_LEVEL::AlarmWarning);
			// 		m_pAlarm->SetAlarmID(ALARM_ID::ALARM_SITE_INVALID);
			// 		m_pAlarm->SetAlarmType(CDriverAlarm::ALARM_TYPE::PARAMETER_SITE_INVALID);
			// 		m_pAlarm->SetAlarmMsg("The SITE_%d is invalid.", usSiteNo + 1);
			// 		m_pAlarm->SetParamName("usSiteNo");
			// 		m_pAlarm->Output(FALSE);
			nRetVal = -1;
			break;
		}

		int nDataByteCount = pI2C->GetReadByteCount();
		if (0 > nDataByteCount)
		{
			nRetVal = -1;
			break;
		}
		int nMaxBitPos = nDataByteCount * 8;
		if (0 > nStartBitIndex || nMaxBitPos < nStartBitIndex)
		{
			pAlarm->SetAlarmID(ALARM_ID::ALARM_I2C_GET_BIT_START_ERROR);
			pAlarm->SetAlarmType(CDriverAlarm::ALARM_TYPE::PARAMETER_OVERRANGE);
			pAlarm->SetParamName("nBitPos");
			pAlarm->SetAlarmMsg("The start bit(%d) is over range [%d,%d].", nStartBitIndex, 0, nMaxBitPos);
			nRetVal = -1;
			break;
		}
		if (0 >= nBitCount || nMaxBitPos - nStartBitIndex < nBitCount)
		{
			pAlarm->SetAlarmID(ALARM_ID::ALARM_LINE_COUNT_OVER_RANGE);
			pAlarm->SetAlarmType(CDriverAlarm::ALARM_TYPE::PARAMETER_OVERRANGE);
			pAlarm->SetParamName("nBitCount");
			pAlarm->SetAlarmMsg("The bit count(%d) is over range [%d,%d].", nBitCount, 0, nMaxBitPos - nStartBitIndex);
			nRetVal = -1;
			break;
		}
		int nStartByteIndex = nStartBitIndex / 8;
		int nFirstByteValidBitCount = 8 - nStartBitIndex % 8;
		int nStopByteIndex = nStartByteIndex + (nBitCount - nFirstByteValidBitCount + 7) / 8;

		int nData = 0;
		int nIndex = 0;
		for (int nDataIndex = nStartByteIndex; nDataIndex <= nStopByteIndex; ++nDataIndex)
		{
			nData = pI2C->GetReadData(usSiteNo, nDataIndex);
			if (0 > nData)
			{
				nRetVal = -1;
				break;
			}
			llData = llData << 8 | nData;
		}
		if (0 != nRetVal)
		{
			break;
		}

		int nRightShift = 0;
		if (nBitCount <= nFirstByteValidBitCount)
		{
			nRightShift = nFirstByteValidBitCount - nBitCount;
		}
		else
		{
			nRightShift = (nBitCount - nFirstByteValidBitCount) % 8;
			nRightShift = 0 == nRightShift ? nRightShift : 8 - nRightShift;
		}

		llData >>= nRightShift;

		for (int nIndex = 0; nIndex < nBitCount; nIndex++)
		{
			ulAdd |= 1 << nIndex;
		}
	} while (false);
	pAlarm->Output();
	if (0 != nRetVal)
	{
		return nRetVal;
	}
 	return llData  & ulAdd;
}

int APIENTRY dcm_I2CSetSDATime(double dT1R, double dT1F, double dIOR, double dIOF, double dSTBR, double dSTBF)
{
	CDriverAlarm* pAlarm = nullptr;
	CI2C* pI2C = GetI2C(pAlarm, "I2CSetSDAEdge");
	if (nullptr == pI2C)
	{
		return -1;
	}
	double dEdge[EDGE_COUNT] = { dT1R, dT1F, dIOR,dIOF, dSTBR,dSTBF };

	int nRetVal = pI2C->SetEdge(FALSE, dEdge);
	if (0 != nRetVal)
	{
		switch (nRetVal)
		{
		case -1:
			///<Not set I2C configuration
			nRetVal = -1;
			break;
		case -2:
			///<Point is nullptr, not happen
			break;
		case -3:
			///<The edge is over range
			nRetVal = -2;
			break;
		default:
			break;
		}
	}
	pAlarm->Output();
	return nRetVal;
}

int APIENTRY dcm_I2CSetSCLTime(double dT1R, double dT1F)
{
	CDriverAlarm* pAlarm = nullptr;
	CI2C* pI2C = GetI2C(pAlarm, "I2CSetSCLEdge");
	if (nullptr == pI2C)
	{
		return -1;
	}
	double dEdge[EDGE_COUNT] = { dT1R, dT1F, dT1R,dT1F, dT1R,dT1F };
	int nRetVal = pI2C->SetEdge(TRUE, dEdge);
	if (0 != nRetVal)
	{
		switch (nRetVal)
		{
		case -1:
			///<Not set i2c channel
			nRetVal = -1;
			break;
		case -2:
			///<The point of edge is nullptr, not happened
			break;
		case -3:
			///<The edge is over range
			nRetVal = -2;
			break;
		default:
			break;
		}
	}
	pAlarm->Output();
	return nRetVal;
}

int APIENTRY dcm_I2CGetSiteCount()
{
	int nCurInstanceID = 0;
	CI2C* pI2C = g_pI2CDistribution->GetClass(&nCurInstanceID);
	if (nullptr == pI2C)
	{
		return -1;
	}
	return pI2C->GetSiteCount();
}

int APIENTRY dcm_I2CSetPeriod(double dPeriod)
{
	CDriverAlarm* pAlarm = nullptr;
	CI2C* pI2C = GetI2C(pAlarm, "I2CSetPeriod");
	if (nullptr == pI2C)
	{
		return -1;
	}
	int nRetVal = pI2C->SetPeriod(dPeriod);
	if (0 != nRetVal)
	{
		switch (nRetVal)
		{
		case -1:
			///<Not set I2C information before
			nRetVal = -1;
			break;
		case -3:
			///<The period is over range
			nRetVal = -2;
			break;
		default:
			break;
		}
	}
	pAlarm->Output();
	return nRetVal;
}

int APIENTRY dcm_GetChannelCount(BYTE bySlotNo)
{
	CDCM* pDCM = g_pDCMDistribution->GetClass();
	if (nullptr == pDCM)
	{
		return -1;
	}
	return pDCM->GetChannelCount(bySlotNo, FALSE);
}

int APIENTRY dcm_SetChannelCount(BYTE bySlotNo, USHORT usChannelCount)
{
	CDCM* pDCM = g_pDCMDistribution->GetClass();
	if (nullptr == pDCM)
	{
		return -1;
	}
	return pDCM->SetChannelCount(bySlotNo, usChannelCount);
}

int APIENTRY dcm_RunVectorEn(const char* lpszPinGroup, const char* lpszStartLabel, const char* lpszStopLabel)
{
	CDCM* pDCM = g_pDCMDistribution->GetClass();
	CDriverAlarm* pAlarm = pDCM->GetAlarm();
	pAlarm->SetDriverPackName("RunVectorEn"); 
	int nRetVal = pDCM->SetRunParam(lpszPinGroup, lpszStartLabel, lpszStopLabel);
	if (0 != nRetVal)
	{
		switch (nRetVal)
		{
		case -1:
			nRetVal = VECTOR_FILE_NOT_LOADED;
			break;
		case -2:
			///<The point of pin group is nullptr
		case -3:
			nRetVal = PIN_GROUP_ERROR;
			break;
		case -4:
			///<No valid site
			nRetVal = SITE_INVALID;
			break;
		case -5:
			///<The pin is not belongs to
			nRetVal = PIN_NOT_BELONGS;
			break;
		case -6:
			///<The point of start label is nullptr
		case -7:
			nRetVal = START_LABEL_ERROR;
			break;
		case -8:
			///<The point of stop label is nullptr
		case -9:
			nRetVal = STOP_LABEL_ERROR;
			break;
		case -10:
			nRetVal = START_LABLE_AFTER_END;
			break;
		case -11:
			nRetVal = BOARD_NOT_INSERT_ERROR;
			break;
		default:
			break;
		}
	}
	pAlarm->Output();
	return nRetVal;
}

int APIENTRY dcm_IsSynRun()
{
	CDCM* pDCM = g_pDCMDistribution->GetClass();
	if (nullptr == pDCM)
	{
		return -1;
	}
	return pDCM->IsWaitRun();
}

int APIENTRY dcm_Run()
{
	CDCM* pDCM = g_pDCMDistribution->GetClass();
	if (nullptr == pDCM)
	{
		return -1;
	}
	return pDCM->Run();
}

int APIENTRY dcm_WaitStop()
{
	CDCM* pDCM = g_pDCMDistribution->GetClass();
	if (nullptr == pDCM)
	{
		return -1;
	}
	return pDCM->WaitStop();
}

int APIENTRY dcm_GetSiteBoard(USHORT usSiteNo, BYTE* pbySlotNo, USHORT* pusChannelCount, int nElementCount)
{
	map<BYTE, USHORT> mapBoard; 
	CDCM* pDCM = g_pDCMDistribution->GetClass();
	if (nullptr == pDCM)
	{
		return -1;
	}
	int nRetVal = pDCM->GetSiteBoard(usSiteNo, mapBoard);
	if (0 != nRetVal)
	{
		switch (nRetVal)
		{
		case -1:
			return -2;
			break;
		case -2:
			return -2;
		default:
			break;
		}
	}
	int nBoardCount = mapBoard.size();
	if (nullptr == pbySlotNo || nullptr == pusChannelCount || nBoardCount > nElementCount)
	{
		return nBoardCount;
	}
	int nBoardIndex = 0;
	auto iterBoard = mapBoard.begin();
	while (mapBoard.end() != iterBoard)
	{
		pbySlotNo[nBoardIndex] = iterBoard->first;
		pusChannelCount[nBoardIndex++] = iterBoard->second;
		++iterBoard;
	}
	return nBoardCount;
}

int APIENTRY dcm_GetHardwareCapture(const char* lpszPinName, USHORT usSiteNo, BYTE* pbyCaptureData, int nElementCount)
{
	CDCM* pDCM = g_pDCMDistribution->GetClass();
	CDriverAlarm* pAlarm = pDCM->GetAlarm();
	pAlarm->SetDriverPackName("GetHardwareCapture");
	int nRetVal = 0;
	nRetVal = pDCM->GetHardwareCapture(lpszPinName, usSiteNo, pbyCaptureData, nElementCount);
	if (0 > nRetVal)
	{
		switch (nRetVal)
		{
		case -1:
			nRetVal = VECTOR_FILE_NOT_LOADED;
			break;
		case -2:
			///<The point of pin name is nullptr
		case -3:
			nRetVal = PIN_NAME_ERROR;
			break;
		case -4:
			nRetVal = SITE_ERROR;
			break;
		case -5:
			nRetVal = SITE_INVALID;
			break;
		case -6:
			///<The pin is not belongs to the instance
			nRetVal = PIN_NOT_BELONGS;
			break;
		case -7:
			nRetVal = BOARD_NOT_INSERT_ERROR;
			break;
		case -8:
			///<The channel number is over range, not will happen
			break;
		case -9:
			nRetVal = CHANNEL_NOT_EXISTED;
			break;
		case -10:
			nRetVal = VECTOR_NOT_RAN;
			break;
		case -11:
			nRetVal = VECTOR_RUNING;
			break;
		case -12:
			nRetVal = CAPTURE_NOT_ALL_SAVE;
			break;
		default:
			break;
		}
	}
	pAlarm->Output();
	return nRetVal;
}

int APIENTRY dcm_GetPattern(BYTE bySlotNo, BYTE byControllerIndex, BOOL bBRAM, UINT uStartLine, UINT uLineCount, char(*lpszPattern)[17])
{
	CDCM* pDCM = g_pDCMDistribution->GetClass(0);
	if (nullptr == pDCM)
	{
		return -1;
	}
	return pDCM->GetPattern(bySlotNo, byControllerIndex, bBRAM, uStartLine, uLineCount, lpszPattern);
}

int APIENTRY dcm_GetMemory(BYTE bySlotNo, BYTE byControllerIndex, BOOL bBRAM, DATA_TYPE DataType, UINT uStartLine, UINT uLineCount, USHORT* pusData)
{
	switch (DataType)
	{
	case DATA_TYPE::FM:
		break;
	case DATA_TYPE::MM:
		break;
	case DATA_TYPE::IOM:
		break;
	case DATA_TYPE::CMD:
		break;
	case DATA_TYPE::OPERAND:
		break;
	default:
		return -1;
		break;
	}
	CDCM* pDCM = g_pDCMDistribution->GetClass(0);
	if (nullptr == pDCM)
	{
		return -1;
	}
	int nRetVal = pDCM->GetMemory(bySlotNo, byControllerIndex, bBRAM, DataType, uStartLine, uLineCount, pusData);
	if (0 != nRetVal)
	{
		switch (nRetVal)
		{
		case -1:
			///<Board not existed
			nRetVal = -2;
			break;
		case -2:
			///<Controller index is over range
			nRetVal = -3;
			break;
		case -3:
			///<The controller is not existed
			nRetVal = -4;
			break;
		case -4:
			///<The data type is not supported
			nRetVal = -5;
			break;
		case -5:
			///<The start line is over range
			nRetVal = -6;
			break;
		case -6:
			///<The data count is over range
			nRetVal = -1;
			break;
		case -7:
			///<The point of data is nullptr or the read data count is 0
			nRetVal = -7;
			break;
		default:
			break;
		}
	}
	return nRetVal;
}

int APIENTRY dcm_SetMemory(BYTE bySlotNo, USHORT usChannel, BOOL bRAM, DATA_TYPE DataType, UINT uStartLine, UINT uLineCount, BYTE* pbyData)
{
	CDCM* pDCM = g_pDCMDistribution->GetClass(0);
	if (nullptr == pDCM)
	{
		return -1;
	}
	return pDCM->SetMemory(bySlotNo, usChannel, bRAM, DataType, uStartLine, uLineCount, pbyData);
}

int APIENTRY dcm_GetRAMFailLineNo(BYTE bySlotNo, USHORT usChannel, BOOL bBRAM, UINT uGetMaxFailCount, UINT* puFailLineNo)
{
	vector<int> vecFailLineNo; 
	CDCM* pDCM = g_pDCMDistribution->GetClass();
	if (nullptr == pDCM)
	{
		return -1;
	}
	int nRetVal = pDCM->GetFailLineNo(bySlotNo, usChannel, bBRAM, uGetMaxFailCount, vecFailLineNo);
	if (0 != nRetVal)
	{
		switch (nRetVal)
		{
		case -1:
			///<The board is not existed
			nRetVal = -1;
			break;
		case -2:
			///<The channel is over range
			nRetVal = -2;
			break;
		case -3:
			///<The channel is not existed
			nRetVal = -3;
			break;
		case -4:
			///<Not ran before
			nRetVal = -4;
			break;
		case -5:
			///<The vector is running
			nRetVal = -5;
			break;
		default:
			break;
		}
		return nRetVal;
	}
	UINT uFailCount = vecFailLineNo.size();
	int nElementCount = uGetMaxFailCount > uFailCount ? uFailCount : uGetMaxFailCount;
	for (UINT uFailIndex = 0; uFailIndex < nElementCount;++uFailIndex)
	{
		puFailLineNo[uFailIndex] = vecFailLineNo[uFailIndex];
	}
	return uFailCount;
}

int APIENTRY dcm_GetLatestI2CMemory(UINT& uStartLine, UINT& uLineCount, BOOL& bWithDRAM, UINT& uLineCountBeforeOut, UINT& uDRAMStartLine, UINT& uDRAMLineCount)
{
	return g_pI2CDistribution->GetClass()->GetLatestMemoryInfo(uStartLine, uLineCount, bWithDRAM, uLineCountBeforeOut, uDRAMStartLine, uDRAMLineCount);
}

int APIENTRY dcm_GetChannelFailCount(BYTE bySlotNo, USHORT usChannel)
{
	CDCM* pDCM = g_pDCMDistribution->GetClass();
	if (nullptr == pDCM)
	{
		return -1;
	}
	return pDCM->GetChannelFailCount(bySlotNo, usChannel);
}

int APIENTRY dcm_SetEdgeByIndex(BYTE bySlotNo, USHORT usChannel, BYTE byTimeset, WAVE_FORMAT WaveFormat, IO_FORMAT IOFormat, double* pdEdgeValue, COMPARE_MODE CompareMode)
{
	CDCM* pDCM = g_pDCMDistribution->GetClass();
	if (nullptr == pDCM)
	{
		return -1;
	}
	return pDCM->SetEdge(bySlotNo, usChannel, byTimeset, WaveFormat, IOFormat, pdEdgeValue, CompareMode);
}

int APIENTRY dcm_GetEdge(BYTE bySlotNo, USHORT usChannel, BYTE byTimesetIndex, double* pdEdge, WAVE_FORMAT& WaveFormat, IO_FORMAT& IOFormat, COMPARE_MODE& CompareMode)
{
	CDCM* pDCM = g_pDCMDistribution->GetClass();
	if (nullptr == pDCM)
	{
		return -1;
	}
	return pDCM->GetEdge(bySlotNo, usChannel, byTimesetIndex, pdEdge, WaveFormat, IOFormat, CompareMode);
}

int APIENTRY dcm_SetChannelStatus(const char* lpszPinGroup, USHORT usSiteNo, BYTE byStatus)
{
	CDCM* pDCM = g_pDCMDistribution->GetClass();
	CDriverAlarm* pAlarm = pDCM->GetAlarm();
	pAlarm->SetDriverPackName("SetChannelStatus");
	int nRetVal = pDCM->SetChannleStatus(lpszPinGroup, usSiteNo, (CHANNEL_OUTPUT_STATUS)byStatus);
	if (0 != nRetVal)
	{
		switch (nRetVal)
		{
		case -1:
			///<Vector not loaded
			nRetVal = -1;
			break;
		case -2:
			///< The point of pin group is nullptr
		case -3:
			///<The pin group is not defined
			nRetVal = PIN_GROUP_ERROR;
			break;
		case -4:
			///<The site is over range
			nRetVal = SITE_ERROR;
			break;
		case -5:
			///<The site is invalid
			nRetVal = SITE_INVALID;
			break;
		case -6:
			///<The pin is not belongs to
			nRetVal = PIN_NOT_BELONGS;
			break;
		case -7:
			///<Channel status is not supported
			/// Not happened
			break;
		case 8:
			///<Channel not existed
			nRetVal = CHANNEL_NOT_EXISTED;
			break;
		case -9:
			///<No valid channel existed
			nRetVal = BOARD_NOT_INSERT_ERROR;
			break;
		default:
			break;
		}
	}
	pAlarm->Output();
	return nRetVal;
}

int APIENTRY dcm_GetChannelStatus(BYTE bySlotNo, USHORT usChannel)
{
	CDCM* pDCM = g_pDCMDistribution->GetClass();
	if (nullptr == pDCM)
	{
		return -1;
	}
	return pDCM->GetChannelStatus(bySlotNo, usChannel);
}

int APIENTRY dcm_GetChannelMode(BYTE bySlotNo, USHORT usChannel)
{
	CDCM* pDCM = g_pDCMDistribution->GetClass();
	if (nullptr == pDCM)
	{
		return -1;
	}
	return pDCM->GetChannelMode(bySlotNo, usChannel);
}

int APIENTRY dcm_SetTMUMatrix(const char* lpszPinGroup, USHORT usSiteNo, BYTE byTMUUnitIndex)
{
	CDCM* pDCM = g_pDCMDistribution->GetClass();
	CDriverAlarm* pAlarm = pDCM->GetAlarm();
	pAlarm->SetDriverPackName("SetTMUMatrix");
	int nRetVal = pDCM->SetTMUUnitChannel(lpszPinGroup, usSiteNo, byTMUUnitIndex);
	if (0 != nRetVal)
	{
		switch (nRetVal)
		{
		case -1:
			///<Not load vector file before
			nRetVal = VECTOR_FILE_NOT_LOADED;
			break;
		case -2:
			///<The point pin group is nullptr
		case -3:
			///<The pin group is not defined
			nRetVal = PIN_GROUP_ERROR;
			break;
		case -4:
			///<The site number is over range
			nRetVal = SITE_ERROR;
			break;
		case -5:
			///<The site is invalid
			nRetVal = SITE_INVALID;
			break;
		case -6:
			///<The pin is not belongs to
			nRetVal = PIN_NOT_BELONGS;
			break;
		case -7:
			///<The pin group have more than one channel in a controller
			nRetVal = TMU_CHANNEL_OVER_RANGE;
			break;
		case -8:
			///<The unit index is over range
			nRetVal = TMU_UNIT_OVER_RANGE;
			break;
		case -9:
			///<No valid board existed
			nRetVal = BOARD_NOT_INSERT_ERROR;
			break;
		default:
			break;
		}
	}
	pAlarm->Output();
	return nRetVal;
}

int APIENTRY dcm_ReadTMUConnectUnit(BYTE bySlotNo, USHORT usChannel)
{
	CDCM* pDCM = g_pDCMDistribution->GetClass();
	if (nullptr == pDCM)
	{
		return -1;
	}
	return pDCM->GetTMUConnectUnit(bySlotNo, usChannel);
}

int APIENTRY dcm_GetTMUConnectUnit(BYTE bySlotNo, USHORT usChannel)
{
	auto iterBoard = g_mapValidBoard.find(bySlotNo);
	if (g_mapValidBoard.end() == iterBoard)
	{
		///<The board is not existed
		return -1;
	}
	BYTE byBoardNo = iterBoard->second.m_byBoardIndex;
	USHORT usChannelCount = iterBoard->second.m_usChannelCount;
	BYTE byController = 0;
	CClassifyBoard Classify;
	int nChannel = Classify.GetControllerChannel(usChannel, byController);
	if (0 > nChannel)
	{
		///<The channel is over range
		return -2;
	}
	if (iterBoard->second.m_usChannelCount <= nChannel)
	{
		///<The channel is not existed
		return -3;
	}
	int nConnectUnit = -4;
	for (BYTE byUnitIndex = 0; byUnitIndex < TMU_UNIT_COUNT_PER_CONTROLLER; ++byUnitIndex)
	{
		if (nChannel == g_ausTMUUnitChannel[byBoardNo][byController][byUnitIndex])
		{
			nConnectUnit = byUnitIndex;
			break;
		}
	}
	return nConnectUnit;
}

int APIENTRY dcm_SetTMUParam(const char* lpszPinGroup, USHORT usSiteNo, BOOL bRaiseTriggerEdge, UINT uHoldOffTime, UINT uHoldOffNum)
{
	CDCM* pDCM = g_pDCMDistribution->GetClass();
	CDriverAlarm* pAlarm = pDCM->GetAlarm();
	pAlarm->SetDriverPackName("SetTMUParam");
	int nRetVal = pDCM->SetTMUParam(lpszPinGroup, usSiteNo, bRaiseTriggerEdge, uHoldOffTime, uHoldOffNum, -1);
	if (0 != nRetVal)
	{
		switch (nRetVal)
		{
		case -1:
			///<The vector is not load before
			nRetVal = VECTOR_FILE_NOT_LOADED;
			break;
		case -2:
			///<The point of the pin group is nullptr
		case -3:
			///<The pin group is not defined
			nRetVal = PIN_GROUP_ERROR;
			break;
		case -4:
			///<The site is over range
			nRetVal = SITE_ERROR;
			break;
		case -5:
			///<The site is invalid
			nRetVal = SITE_INVALID;
			break;
		case -6:
			///<The pin is not belongs to the instance
			nRetVal = PIN_NOT_BELONGS;
			break;
		case -7:
			///<The TMU unit is over range, not will happen
			break;
		case -8:
			///<The channel count is over range in specified channel, not will happen
			break;
		case -9:
			///<The channel is not connected to TMU unit
			nRetVal = TMU_CHANNEL_NOT_CONNECT;
			break;
		case -10:
			///<No valid board existed
			nRetVal = BOARD_NOT_INSERT_ERROR;
			break;
		default:
			break;
		}
	}
	pAlarm->Output();
	return nRetVal;
}

int APIENTRY dcm_SetTMUUnitParam(const char* lpszPinGroup, BOOL bRaiseTriggerEdge, UINT uHoldOffTime, UINT uHoldOffNum, BYTE byUnitIndex)
{
	CDCM* pDCM = g_pDCMDistribution->GetClass();
	CDriverAlarm* pAlarm = pDCM->GetAlarm();
	pAlarm->SetDriverPackName("SetTMUParam");
	int nRetVal = pDCM->SetTMUParam(lpszPinGroup, ALL_SITE, bRaiseTriggerEdge, uHoldOffTime, uHoldOffNum, byUnitIndex);
	if (0 != nRetVal)
	{
		switch (nRetVal)
		{
		case -1:
			///<The vector is not load before
			nRetVal = VECTOR_FILE_NOT_LOADED;
			break;
		case -2:
			///<The point of the pin group is nullptr
		case -3:
			///<The pin group is not defined
			nRetVal = PIN_GROUP_ERROR;
			break;
		case -4:
			///<The site is over range
			nRetVal = SITE_ERROR;
			break;
		case -5:
			///<The site is invalid
			nRetVal = SITE_INVALID;
			break;
		case -6:
			///<The pin is not belongs to the instance
			nRetVal = PIN_NOT_BELONGS;
			break;
		case -7:
			///<The TMU unit is over range, not will happen
			break;
		case -8:
			///<The channel count is over range in specified channel
			nRetVal = MORE_ONE_CHANNEL_BIND_UINT;
			break;
		case -9:
			///<The channel is not connected to TMU unit
			nRetVal = TMU_CHANNEL_NOT_CONNECT;
			break;
		case -10:
			///<No valid board existed
			nRetVal = BOARD_NOT_INSERT_ERROR;
			break;
		default:
			break;
		}
	}
	pAlarm->Output();
	return nRetVal;
}

int APIENTRY dcm_GetTMUUnitParam(BYTE bySlotNo, BYTE byControllerIndex, BYTE byTMUUnitIndex, BOOL& bRaiseTriggerEdge, USHORT& usHoldOffTime, USHORT& usHoldOffNum)
{
	auto iterSlot = g_mapValidBoard.find(bySlotNo);
	if (g_mapValidBoard.end() == iterSlot)
	{
		return -1;
	}
	BYTE byBoardNo = iterSlot->second.m_byBoardIndex;
	if (DCM_MAX_CONTROLLERS_PRE_BOARD <= byControllerIndex)
	{
		return -2;
	}
	if (TMU_UNIT_COUNT_PER_CONTROLLER <= byTMUUnitIndex)
	{
		return -3;
	}
	bRaiseTriggerEdge = g_abyTMUTriggerEdge[byBoardNo][byControllerIndex][byTMUUnitIndex];
	usHoldOffNum = g_ausTMUHoldOffNum[byBoardNo][byControllerIndex][byTMUUnitIndex];
	usHoldOffTime = g_ausTMUHoldOffTime[byBoardNo][byControllerIndex][byTMUUnitIndex];
	return 0;
}

int APIENTRY dcm_ReadTMUParam(BYTE bySlotNo, USHORT usChannel, BOOL& bRaiseTriggerEdge, USHORT& usHoldOffTime, USHORT& usHoldOffNum)
{
	CDCM* pDCM = g_pDCMDistribution->GetClass();
	if (nullptr == pDCM)
	{
		///<The board is not existed
		return -1;
	}
	return pDCM->GetTMUParameter(bySlotNo, usChannel, bRaiseTriggerEdge, usHoldOffTime, usHoldOffNum);
}

int APIENTRY dcm_ReadTMUUnitParam(BYTE bySlotNo, BYTE byControllerIndex, BYTE byTMUUnitIndex, BOOL& bRaiseTriggerEdge, USHORT& usHoldOffTime, USHORT& usHoldOffNum)
{
	CDCM* pDCM = g_pDCMDistribution->GetClass();
	if (nullptr == pDCM)
	{
		///<The board is not existed
		return -1;
	}
	return pDCM->GetTMUUnitParameter(bySlotNo, byControllerIndex, byTMUUnitIndex, bRaiseTriggerEdge, usHoldOffTime, usHoldOffNum);
}

int APIENTRY dcm_GetTMUParameter(BYTE bySlotNo, USHORT usChannel, BOOL& bRaiseTriggerEdge, USHORT& usHoldOffTime, USHORT& usHoldOffNum)
{
	auto iterBoard = g_mapValidBoard.find(bySlotNo);
	if (g_mapValidBoard.end() == iterBoard)
	{
		///<The board is not existed
		return -1;
	}
	BYTE byBoardNo = iterBoard->second.m_byBoardIndex;
	USHORT usChannelCount = iterBoard->second.m_usChannelCount;
	BYTE byController = 0;
	CClassifyBoard Classify;
	int nChannel = Classify.GetControllerChannel(usChannel, byController);
	if (0 > nChannel)
	{
		///<The channel is over range
		return -2;
	}
	if (iterBoard->second.m_usChannelCount <= nChannel)
	{
		///<The channel is not existed
		return -3;
	}
	int nConnectUnit = -1;
	for (BYTE byUnitIndex = 0; byUnitIndex < TMU_UNIT_COUNT_PER_CONTROLLER; ++byUnitIndex)
	{
		if (nChannel == g_ausTMUUnitChannel[byController][byController][byUnitIndex])
		{
			nConnectUnit = byUnitIndex;
			break;
		}
	}
	if (0 > nConnectUnit)
	{
		///<The channel is not connected to unit
		return -4;
	}
	bRaiseTriggerEdge = g_abyTMUTriggerEdge[byBoardNo][byController][nConnectUnit];
	usHoldOffNum = g_ausTMUHoldOffNum[byBoardNo][byController][nConnectUnit];
	usHoldOffTime = g_ausTMUHoldOffTime[byBoardNo][byController][nConnectUnit];
	return 0;
}

int APIENTRY dcm_TMUMeasure(const char* lpszPinGroup, BYTE byMeasMode, UINT uSampleNum, double dTimeout)
{
	CDCM* pDCM = g_pDCMDistribution->GetClass();
	CDriverAlarm* pAlarm = pDCM->GetAlarm();
	pAlarm->SetDriverPackName("TMUMeasure");
	int nRetVal = pDCM->TMUMeasure(lpszPinGroup, (TMU_MEAS_MODE)byMeasMode, uSampleNum, dTimeout);
	if (0 != nRetVal)
	{
		switch (nRetVal)
		{
		case -1:
			///<The vector file is not loaded before
			nRetVal = VECTOR_FILE_NOT_LOADED;
			break;
		case -2:
			///<The point of the pin group is nullptr
		case -3:
			///<The pin group is not defined
			nRetVal = PIN_GROUP_ERROR;
			break;
		case -4:
			///<No valid site
			nRetVal = SITE_INVALID;
			break;
		case -5:
			///<The pin is not belongs to the instance
			nRetVal = PIN_NOT_BELONGS;
			break;
		case -6:
			///<The channel is not connected to TMU unit
			nRetVal = TMU_CHANNEL_NOT_CONNECT;
			break;
		case -7:
			///<The measurement mode is not supported, not will happen
			break;
		case -8:
			///<No valid board existed
			nRetVal = BOARD_NOT_INSERT_ERROR;
			break;
		default:
			break;
		}
	}
	pAlarm->Output();
	return nRetVal;
}

int APIENTRY dcm_GetTMUMeasure(BYTE bySlotNo, USHORT usChannel, BYTE& byMeasMode, UINT& uSampleNum, double& dTimeout)
{
	auto iterBoard = g_mapValidBoard.find(bySlotNo);
	if (g_mapValidBoard.end() == iterBoard)
	{
		///<The board is not existed
		return -1;
	}
	BYTE byBoardNo = iterBoard->second.m_byBoardIndex;
	USHORT usChannelCount = iterBoard->second.m_usChannelCount;
	BYTE byController = 0;
	CClassifyBoard Classify;
	int nChannel = Classify.GetControllerChannel(usChannel, byController);
	if (0 > nChannel)
	{
		///<The channel is over range
		return -2;
	}
	if (iterBoard->second.m_usChannelCount <= nChannel)
	{
		///<The channel is not existed
		return -3;
	}
	int nConnectUnit = -1;
	for (BYTE byUnitIndex = 0; byUnitIndex < TMU_UNIT_COUNT_PER_CONTROLLER; ++byUnitIndex)
	{
		if (nChannel == g_ausTMUUnitChannel[byController][byController][byUnitIndex])
		{
			nConnectUnit = byUnitIndex;
			break;
		}
	}
	if (0 > nConnectUnit)
	{
		///<The channel is not connected to unit
		return -4;
	}
	byMeasMode = g_abyTMUMode[byBoardNo][byController][nConnectUnit];
	uSampleNum = g_ausTMUSampleNum[byBoardNo][byController][nConnectUnit];
	dTimeout = g_afTMUTimeout[byBoardNo][byController][nConnectUnit];
	return 0;
}

double APIENTRY dcm_GetTMUMeasureResult(const char* lpszPinName, USHORT usSiteNo, BYTE byMeasType)
{
	CDCM* pDCM = g_pDCMDistribution->GetClass();
	CDriverAlarm* pAlarm = pDCM->GetAlarm();
	pAlarm->SetDriverPackName("GetTMUMeasureResult");
	int nErrorCode = 0;
	double dResult = pDCM->GetTMUMeasureResult(lpszPinName, usSiteNo, (TMU_MEAS_TYPE)byMeasType, nErrorCode);
	if (0 != nErrorCode)
	{
		dResult = TMU_ERROR;
	}
	pAlarm->Output();
	return dResult;
}

int APIENTRY dcm_SetOperand(const char* lpszPinGroup, const char* lpszStartLabel, ULONG ulOffset, const char* lpszOperand)
{
	CDCM* pDCM = g_pDCMDistribution->GetClass();
	CDriverAlarm* pAlarm = pDCM->GetAlarm();
	pAlarm->SetDriverPackName("SetOperand");
	int nRetVal = pDCM->SetOperand(lpszPinGroup, lpszStartLabel, ulOffset, lpszOperand);
	if (0 != nRetVal)
	{
		switch (nRetVal)
		{
		case -1:
			///<No load vector
			nRetVal = VECTOR_FILE_NOT_LOADED;
			break;
		case -2:
			///<The point of pin group is nullptr
			nRetVal = PIN_GROUP_ERROR;
			break;
		case -3:
			///<The pin group is not defined before
			nRetVal = PIN_GROUP_ERROR;
			break;
		case -4:
			///<No valid site
			nRetVal = SITE_INVALID;
			break;
		case -5:
			///<The pin is not belongs to the instance
			nRetVal = PIN_NOT_BELONGS;
			break;
		case -6:
			///<The start label is nullptr
		case -7:
			///<The start label is not existed
			nRetVal = START_LABEL_ERROR;
			break;
		case -8:
			///<The offset is over range
		case -9:
			///<The line is not in BRAM
			nRetVal = OFFSET_ERROR;
			break;
		case -10:
			///<The operand is nullptr
		case -11:
			///<The operand label is not existed
		case -12:
			///<Operand is ove range
			nRetVal = OPERAND_ERROR;
			break;
		case -13:
			///<Channel not existed
			nRetVal = CHANNEL_NOT_EXISTED;
			break;
		case -14:
			///<No valid board existed
			nRetVal = BOARD_NOT_INSERT_ERROR;
			break;
		default:
			break;
		}
	}
	pAlarm->Output();
	return nRetVal;
}

int APIENTRY dcm_SetInstruction(const char* lpszPinGroup, const char* lpszStartLabel, ULONG ulOffset, const char* lpszInstruction, const char* lpszOpeand)
{
	CDCM* pDCM = g_pDCMDistribution->GetClass();
	CDriverAlarm* pAlarm = pDCM->GetAlarm();
	pAlarm->SetDriverPackName("SetInstruction");
	int nRetVal = pDCM->SetInstruction(lpszPinGroup, lpszStartLabel, ulOffset, lpszInstruction, lpszOpeand);
	if (0 != nRetVal)
	{
		switch (nRetVal)
		{
		case -1:
			///<Vector not loaded
			nRetVal = VECTOR_FILE_NOT_LOADED;
			break;
		case -2:
			///<The point of pin group is nullptr
			nRetVal = PIN_GROUP_ERROR;
			break;
		case -3:
			///<The pin group is not defined before
			nRetVal = PIN_GROUP_ERROR;
			break;
		case -4:
			///<No valid site
			nRetVal = SITE_INVALID;
			break;
		case -5:
			///<Pin is not belongs to
			nRetVal = PIN_NOT_BELONGS;
			break;
		case -6:
			///<The start label is nullptr
			nRetVal = START_LABEL_ERROR;
			break;
		case -7:
			///<The start label is not existed
			nRetVal = START_LABEL_ERROR;
			break;
		case -8:
			///<The offset is over range
			nRetVal = OFFSET_ERROR;
			break;
		case -9:
			///<The line is not in BRAM
			nRetVal = OFFSET_ERROR;
			break;
		case -10:
			///<The operand is nullptr
		case -11:
			///<The operand label is not existed
		case -12:
			///<The operand is over range
			nRetVal = OPERAND_ERROR;
			break;
		case -13:
			///<The instruction is nullptr
		case -14:
			///<The instruction is not supported
			nRetVal = INSTRUCTION_ERROR;
			break;
		case -15:
			///<Channel not existed
			nRetVal = CHANNEL_NOT_EXISTED;
			break;
		case -16:
			nRetVal = BOARD_NOT_INSERT_ERROR;
			break;
		default:
			break;
		}
	}
	pAlarm->Output();
	return nRetVal;
}

int APIENTRY dcm_I2CConnect()
{
	CDriverAlarm* pAlarm = nullptr;
	CI2C* pI2C = GetI2C(pAlarm, "I2CConnect");
	if (nullptr == pI2C)
	{
		return -1;
	}

	int nRetVal = pI2C->Connect(TRUE);

	pAlarm->Output();
	return nRetVal;
}

int APIENTRY dcm_I2CDisconnect()
{
	CDriverAlarm* pAlarm = nullptr;
	CI2C* pI2C = GetI2C(pAlarm, "I2CDisconnect");
	if (nullptr == pI2C)
	{
		return -1;
	}
	int nRetVal = pI2C->Connect(FALSE);
	pAlarm->Output();
	return nRetVal;
}

int APIENTRY dcm_connect(const char* lpszPinGroup)
{
	CDCM* pDCM = g_pDCMDistribution->GetClass();
	CDriverAlarm* pAlarm = pDCM->GetAlarm();
	pAlarm->SetDriverPackName("Connect");
	
	int nRetVal = pDCM->Connect(lpszPinGroup, TRUE);
	if (0 != nRetVal)
	{
		switch (nRetVal)
		{
		case -1:
			nRetVal = VECTOR_FILE_NOT_LOADED;
			break;
		case -2:
			///<The point of pin group is nullptr
		case -3:
			nRetVal = PIN_GROUP_ERROR;
			break;
		case -4:
			///<No valid site
			nRetVal = SITE_INVALID;
			break;
		case -5:
			///<The pin is not belongs to the instance
			nRetVal = PIN_NOT_BELONGS;
			break;
		case -6:
			nRetVal = BOARD_NOT_INSERT_ERROR;
			break;
		default:
			break;
		}
	}
	pAlarm->Output();
	return nRetVal;
}

int APIENTRY dcm_disconnect(const char* lpszPinGroup)
{
	CDCM* pDCM = g_pDCMDistribution->GetClass();
	CDriverAlarm* pAlarm = pDCM->GetAlarm();
	pAlarm->SetDriverPackName("Disconnect");

	int nRetVal = pDCM->Connect(lpszPinGroup, FALSE);
	if (0 != nRetVal)
	{
		switch (nRetVal)
		{
		case -1:
			nRetVal = VECTOR_FILE_NOT_LOADED;
			break;
		case -2:
			///<The point of pin group is nullptr
		case -3:
			nRetVal = PIN_GROUP_ERROR;
			break;
		case -4:
			///<No valid site
			nRetVal = SITE_INVALID;
			break;
		case -5:
			///<The pin is not belongs to the instance
			nRetVal = PIN_NOT_BELONGS;
			break;
		case -6:
			nRetVal = BOARD_NOT_INSERT_ERROR;
			break;
		default:
			break;
		}
	}

	pAlarm->Output();
	return nRetVal;
}

int APIENTRY dcm_SaveFailMap(ULONG ulMaxErrLine)
{
	CDCM* pDCM = g_pDCMDistribution->GetClass();
	CDriverAlarm* pAlarm = pDCM->GetAlarm();
	pAlarm->SetDriverPackName("SaveFailMap");
	int nRetVal = pDCM->SaveFailMap(ulMaxErrLine);
	if (0 != nRetVal)
	{
		switch (nRetVal)
		{
		case -1:
			nRetVal = VECTOR_FILE_NOT_LOADED;
			break;
		case -2:
			nRetVal = VECTOR_NOT_RAN;
			break;
		case -3:
			nRetVal = VECTOR_RUNING;
			break;
		case -4:
			nRetVal = ALLOCATE_MEMORY_FAIL;
			break;
		case -5:
			nRetVal = FAIL_LINE_NOT_SAVE;
			break;
		default:
			break;
		}
	}
	pAlarm->Output();
	return nRetVal;
}

int APIENTRY dcm_CloseFile()
{
	vector<int> vecInstanceID;
	g_pDCMDistribution->GetInstanceValid(vecInstanceID);
	CDCM* pDCM = nullptr;
	CVectorFileRecord* pVectorRecord = CVectorFileRecord::Instance();
	CPinNoManage* pPinNoInstance = CPinNoManage::Instance();
	for (auto InstanceID : vecInstanceID)
	{
		pDCM = g_pDCMDistribution->GetClass(nullptr, InstanceID);
		if (nullptr != pDCM)
		{
			pDCM->ClearVector();
			pVectorRecord->SetVectorInvalid(InstanceID);
			pPinNoInstance->SetPinCount(InstanceID, 0, TRUE);
		}
	}
	string strFilePath;
	GetI2CConfig(strFilePath);
	DeleteFile(strFilePath.c_str());
	g_bAllowDebugToolShow = FALSE;
	return 0;
}

int APIENTRY dcm_SetTimeByName(const char* lpszPinGroup, const char* lpszTimeset, BYTE byWaveFormat, double dT1R, double dT1F, double dIOR, double dIOF, double dSTBR, double dSTBF, BYTE byCompareMode)
{
	CDCM* pDCM = g_pDCMDistribution->GetClass();
	CDriverAlarm* pAlarm = pDCM->GetAlarm();
	pAlarm->SetDriverPackName("SetEdge");
	WAVE_FORMAT WaveFormat = WAVE_FORMAT::NRZ;
	COMPARE_MODE CompareMode = (COMPARE_MODE)byCompareMode;
	switch (byWaveFormat & 0x0F)
	{
	case 0:
		WaveFormat = WAVE_FORMAT::NRZ;
		break;
	case 1:
		WaveFormat = WAVE_FORMAT::RZ;
		break;
	case 2:
		WaveFormat = WAVE_FORMAT::RO;
		break;
	case 8:
		WaveFormat = WAVE_FORMAT::SBL;
		break;
	case 9:
		WaveFormat = WAVE_FORMAT::SBH;
		break;
	case 10:
		WaveFormat = WAVE_FORMAT::SBC;
		break;
	default:
		break;
	}
	IO_FORMAT IOFormat = IO_FORMAT::NRZ;
	switch (byWaveFormat >> 4)
	{
	case 0:
		IOFormat = IO_FORMAT::NRZ;
		break;
	case 1:
		IOFormat = IO_FORMAT::RO;
		break;
	default:
		break;
	}

	double dEdgeValue[EDGE_COUNT] = { dT1R, dT1F, dIOR, dIOF, dSTBR,dSTBF };
	int nRetVal = pDCM->SetEdge(lpszPinGroup, lpszTimeset, WaveFormat, IOFormat, dEdgeValue, CompareMode);
	if (0 != nRetVal)
	{
		switch (nRetVal)
		{
		case -1:
			nRetVal = VECTOR_FILE_NOT_LOADED;
			break;
		case -2:
			///<The point of pin group is nullptr
		case -3:
			nRetVal = PIN_GROUP_ERROR;
			break;
		case -4:
			///<Site invalid
			nRetVal = SITE_INVALID;
			break;
		case -5:
			///<The pin is not belongs to the instance
			nRetVal = PIN_NOT_BELONGS;
			break;
		case -6:
			///<The point of timeset is nullptr
		case -7:
			nRetVal = TIMESET_ERROR;
			break;
		case -11:
			///<Not will happened
			break;
		case -12:
			nRetVal = TIME_ERROR;
			break;
		case -13:
			nRetVal = BOARD_NOT_INSERT_ERROR;
			break;
		default:
			nRetVal = TIME_ERROR;
			break;
		}
	}
	pAlarm->Output();
	return nRetVal;
}

int APIENTRY dcm_SetEdge(const char* lpszPinGroup, const char* lpszTimeset, BYTE byWaveFormat, BYTE byIOFormat, double dT1R, double dT1F, double dIOR, double dIOF, double dSTBR, double dSTBF, BYTE byCompareMode)
{
	CDCM* pDCM = g_pDCMDistribution->GetClass();
	CDriverAlarm* pAlarm = pDCM->GetAlarm();
	pAlarm->SetDriverPackName("SetEdge");
	WAVE_FORMAT WaveFormat = WAVE_FORMAT::NRZ;
	IO_FORMAT IOFormat = (IO_FORMAT)byIOFormat;
	COMPARE_MODE CompareMode = (COMPARE_MODE)byCompareMode;
	switch (byWaveFormat)
	{
	case 0:
		WaveFormat = WAVE_FORMAT::NRZ;
		break;
	case 1:
		WaveFormat = WAVE_FORMAT::RZ;
		break;
	case 2:
		WaveFormat = WAVE_FORMAT::RO;
		break;
	case 8:
		WaveFormat = WAVE_FORMAT::SBL;
		break;
	case 9:
		WaveFormat = WAVE_FORMAT::SBH;
		break;
	case 10:
		WaveFormat = WAVE_FORMAT::SBC;
		break;
	default:
		break;
	}
	double dEdgeValue[EDGE_COUNT] = { dT1R, dT1F, dIOR, dIOF, dSTBR,dSTBF };
	int nRetVal = pDCM->SetEdge(lpszPinGroup, lpszTimeset, WaveFormat, IOFormat, dEdgeValue, CompareMode);
	if (0 != nRetVal)
	{
		switch (nRetVal)
		{
		case -1:
			nRetVal = VECTOR_FILE_NOT_LOADED;
			break;
		case -2:
			///<The point of pin group is nullptr
		case -3:
			nRetVal = PIN_GROUP_ERROR;
			break;
		case -4:
			///<No valid site
			nRetVal = SITE_INVALID;
			break;
		case -5:
			///<The pin is not belongs to the instance
			nRetVal = PIN_NOT_BELONGS;
			break;
		case -6:
			///<The point of timeset name is nullptr
		case -7:
			nRetVal = TIMESET_ERROR;
			break;
		case -10:
			///<Not will happened
			break;
		case -12:
			nRetVal = TIME_ERROR;
			break;
		case -13:
			nRetVal = BOARD_NOT_INSERT_ERROR;
		default:
			nRetVal = TIME_ERROR;
			break;
		}
	}
	pAlarm->Output();
	return nRetVal;
}

int APIENTRY dcm_SetPeriodByName(const char* lpszTimesetName, double dPeriod)
{
	CDCM* pDCM = g_pDCMDistribution->GetClass();
	CDriverAlarm* pAlarm = pDCM->GetAlarm();
	pAlarm->SetDriverPackName("SetPeriod");
	int nRetVal = pDCM->SetPeriod(lpszTimesetName, dPeriod);
	if (0 != nRetVal)
	{
		switch (nRetVal)
		{
		case -1:
			nRetVal = VECTOR_FILE_NOT_LOADED;
			break;
		case -2:
			nRetVal = BOARD_NOT_INSERT_ERROR;
			break;
		case -3:
			///<The point of timeset name is nullptr
		case -4:
			nRetVal = TIMESET_ERROR;
			break;
		case -5:
			nRetVal = RATE_ERROR;
			break;
		default:
			nRetVal = RATE_ERROR;
			break;
		}
	}
	pAlarm->Output();
	return nRetVal;
}


int APIENTRY dcm_SetPinLevel(const char* lpszPinGroup, double dVIH, double dVIL, double dVOH, double dVOL)
{
	CDCM* pDCM = g_pDCMDistribution->GetClass();
	CDriverAlarm* pAlarm = pDCM->GetAlarm();
	pAlarm->SetDriverPackName("SetPinLevel");
	int nRetVal = pDCM->SetPinLevel(lpszPinGroup, dVIH, dVIL, dVOH, dVOL);
	if (0 != nRetVal)
	{
		switch (nRetVal)
		{
		case -1:
			nRetVal = VECTOR_FILE_NOT_LOADED;
			break;
		case -2:
			///<The point of pin group is nullptr
		case -3:
			///<Pin group not defined
			nRetVal = PIN_GROUP_ERROR;
			break;
		case -4:
			///<No valid site
			nRetVal = SITE_INVALID;
			break;
		case -5:
			///<The pin is not belongs to the instance
			nRetVal = PIN_NOT_BELONGS;
			break;
		case -6:
			nRetVal = PIN_LEVEL_ERROR;
			break;
		case -7:
			nRetVal = BOARD_NOT_INSERT_ERROR;
			break;
		default:
			nRetVal = PIN_LEVEL_ERROR;
			break;
		}
	}
	pAlarm->Output();
	return nRetVal;
}

int APIENTRY dcm_setppmu(const char* lpszPinGroup, BYTE byMode, double dValue, BYTE byIRange)
{
	CDCM* pDCM = g_pDCMDistribution->GetClass();
	CDriverAlarm* pAlarm = pDCM->GetAlarm();
	pAlarm->SetDriverPackName("SetPPMU");
	int nRetVal = pDCM->SetPMUMode(lpszPinGroup, (USHORT)-1, (PMU_MODE)byMode, (PMU_IRANGE)byIRange, dValue);
	if (0 != nRetVal)
	{
		switch (nRetVal)
		{
		case -1:
			nRetVal = VECTOR_FILE_NOT_LOADED;
			break;
		case -2:
			///<The point of pin group is nullptr
		case -3:
			nRetVal = PIN_GROUP_ERROR;
			break;
		case -6:
			///<The pin is not belongs to the instance
			nRetVal = PIN_NOT_BELONGS;
			break;
		case -7:
			nRetVal = CHANNEL_NOT_EXISTED;
			break;
		case -8:
			nRetVal = BOARD_NOT_INSERT_ERROR;
			break;
		default:
			nRetVal = PIN_GROUP_ERROR;
			break;
		}
	}
	pAlarm->Output();
	return nRetVal;
}

int APIENTRY dcm_setppmu_single_site(const char* lpszPinGroup, USHORT usSiteNo, BYTE byMode, double dValue, BYTE byIRange)
{
	CDCM* pDCM = g_pDCMDistribution->GetClass();
	CDriverAlarm* pAlarm = pDCM->GetAlarm();
	pAlarm->SetDriverPackName("SetPPMUSingleSite");
	int nRetVal = pDCM->SetPMUMode(lpszPinGroup, usSiteNo, (PMU_MODE)byMode, (PMU_IRANGE)byIRange, dValue);
	if (0 != nRetVal)
	{
		switch (nRetVal)
		{
		case -1:
			nRetVal = VECTOR_FILE_NOT_LOADED;
			break;
		case -2:
			///<The point of pin group is nullptr
		case -3:
			nRetVal = PIN_GROUP_ERROR;
			break;
		case -4:
			///<Site over range
			nRetVal = SITE_ERROR;
			break;
		case -5:
			///<Site invalid
			nRetVal = SITE_INVALID;
			break;
		case -6:
			///<The pin is not belongs to the instance
			nRetVal = PIN_NOT_BELONGS;
			break;
		case -7:
			nRetVal = CHANNEL_NOT_EXISTED;
			break;
		case -8:
			nRetVal = BOARD_NOT_INSERT_ERROR;
			break;
		default:
			nRetVal = PIN_GROUP_ERROR;
			break;
		}
	}
	pAlarm->Output();
	return nRetVal;
}

int APIENTRY dcm_initmcu(const char* lpszPinGroup)
{
	CDCM* pDCM = g_pDCMDistribution->GetClass();
	CDriverAlarm* pAlarm = pDCM->GetAlarm();
	pAlarm->SetDriverPackName("InitMCU");
	int nRetVal = pDCM->InitMCU(lpszPinGroup);
	if (0 != nRetVal)
	{
		switch (nRetVal)
		{
		case -1:
			nRetVal = VECTOR_FILE_NOT_LOADED;
			break;
		case -2:
			///<The point of pin group is nullptr
		case -3:
			nRetVal = PIN_GROUP_ERROR;
			break;
		case -4:
			///<No valid site
			nRetVal = SITE_INVALID;
			break;
		case -5:
			///<The pin is not belongs to the instance
			nRetVal = PIN_NOT_BELONGS;
			break;
		case -6:
			nRetVal = BOARD_NOT_INSERT_ERROR;
			break;
		default:
			nRetVal = PIN_GROUP_ERROR;
			break;
		}
	}
	pAlarm->Output();
	return nRetVal;
}

int APIENTRY dcm_initppmu(const char* lpszPinGroup)
{
	CDCM* pDCM = g_pDCMDistribution->GetClass();
	CDriverAlarm* pAlarm = pDCM->GetAlarm();
	pAlarm->SetDriverPackName("InitPPMU");
	int nRetVal = pDCM->InitPMU(lpszPinGroup);
	if (0 != nRetVal)
	{
		switch (nRetVal)
		{
		case -1:
			nRetVal = VECTOR_FILE_NOT_LOADED;
			break;
		case -2:
			///<The point of pin group is nullptr
		case -3:
			nRetVal = PIN_GROUP_ERROR;
			break;
		case -4:
			///<No valid site
			nRetVal = SITE_INVALID;
			break;
		case -5:
			///<The pin is not belongs to the instance
			nRetVal = PIN_NOT_BELONGS;
			break;
		case -6:
			nRetVal = BOARD_NOT_INSERT_ERROR;
			break;
		default:
			nRetVal = PIN_GROUP_ERROR;
			break;
		}
	}
	pAlarm->Output();
	return nRetVal;
}

int APIENTRY dcm_GetStringType(const char* lpszString)
{
	CDCM* pDCM = g_pDCMDistribution->GetClass();
	if (nullptr == pDCM)
	{
		return -1;
	}
	int nRetVal = pDCM->GetStringType(lpszString);
	if (0 > nRetVal)
	{
		switch (nRetVal)
		{
		case -1:
			///<Not load vector file
			nRetVal = -1;
			break;
		case -2:
			///<The string is nullptr
			nRetVal = -2;
			break;
		case -3:
			///<Not find string
			nRetVal = -3;
			break;
		default:
			break;
		}
	}
	return nRetVal;
}

int APIENTRY dcm_GetLevelSettingValue(BYTE bySlotNo, USHORT usChannel, DCM_LEVEL_TYPE LevelType, double& dLevelValue)
{
	LEVEL_TYPE DCMLevelType = LEVEL_TYPE::VIH;
	switch (LevelType)
	{
	case DCM_VIH:
		DCMLevelType = LEVEL_TYPE::VIH;
		break;
	case DCM_VIL:
		DCMLevelType = LEVEL_TYPE::VIL;
		break;
	case DCM_VOH:
		DCMLevelType = LEVEL_TYPE::VOH;
		break;
	case DCM_VOL:
		DCMLevelType = LEVEL_TYPE::VOL;
		break;
	case DCM_IOH:
		DCMLevelType = LEVEL_TYPE::IOH;
		break;
	case DCM_IOL:
		DCMLevelType = LEVEL_TYPE::IOL;
		break;
	case DCM_VT:
		DCMLevelType = LEVEL_TYPE::VT;
		break;
	case DCM_CLAMP_HIGH:
		DCMLevelType = LEVEL_TYPE::CLAMP_HIGH;
		break;
	case DCM_CLAMP_LOW:
		DCMLevelType = LEVEL_TYPE::CLAMP_LOW;
		break;
	default:
		return -1;
		break;
	}
	CDCM* pDCM = g_pDCMDistribution->GetClass();
	if (nullptr == pDCM)
	{
		return -1;
	}
	dLevelValue = pDCM->GetPinLevel(bySlotNo, usChannel, DCMLevelType);
	if (1000 < dLevelValue)
	{
		return -2;
	}
	return 0;
}

int APIENTRY dcm_ConvertPinNameToChannel(const char* lpszPinName, USHORT usSiteNo, BYTE& bySlotNo, int& nChanelNo)
{
	CHANNEL_INFO Channel; 
	CDCM* pDCM = g_pDCMDistribution->GetClass();
	if (nullptr == pDCM)
	{
		return -1;
	}
	int nRetVal = pDCM->GetChannel(lpszPinName, usSiteNo, Channel);
	if (0 != nRetVal)
	{
		switch (nRetVal)
		{
		case -1:
			nRetVal = VECTOR_FILE_NOT_LOADED;
			break;
		case -2:
			///<The point of pin name is nullptr
		case -3:
			nRetVal = PIN_NAME_ERROR;
			break;
		case -4:
			///<The point of timeset name is nullptr
			nRetVal = SITE_ERROR;
			break;
		case -5:
			nRetVal = SITE_INVALID;
			break;
		case -6:
			nRetVal = PIN_NOT_BELONGS;
			break;
		}
	}
	else
	{
		bySlotNo = Channel.m_bySlotNo;
		nChanelNo = Channel.m_usChannel;
	}
	return nRetVal;
}

int APIENTRY dcm_GetDynamicLoadMode(BYTE bySlotNo, USHORT usChannelNo, int& nDynamicMode)
{
	double dIOH = 0;
	double dIOL = 0;
	CDCM* pDCM = g_pDCMDistribution->GetClass();
	if (nullptr == pDCM)
	{
		return -1;
	}
	int nRetVal = pDCM->GetDynamicLoad(bySlotNo, usChannelNo, nDynamicMode, dIOH, dIOL);
	if (0 != nRetVal)
	{
		switch (nRetVal)
		{
		case -1:
			///<Board not existed
			return -1;
			break;
		case -2:
			///<Channel is over range
			return -2;
			break;
		case -3:
			///<Channel is not existed
			return -3;
			break;
		default:
			break;
		}
		return nRetVal;
	}
	
	return 0;
}

int APIENTRY dcm_GetVTMode(BYTE bySlotNo, USHORT usChannelNo, int& nVTMode)
{
	VT_MODE VTMode = VT_MODE::CLOSE;
	CDCM* pDCM = g_pDCMDistribution->GetClass();
	if (nullptr == pDCM)
	{
		return -1;
	}
	int nRetVal = pDCM->GetVTMode(bySlotNo, usChannelNo, VTMode);
	if (0 != nRetVal)
	{
		switch (nRetVal)
		{
		case -1:
			///<Board not existed
			nRetVal = -1;
			break;
		case -2:
			///<Channel is over range
			nRetVal = -2;
			break;
		case -3:
			///<Channel is not existed
			nRetVal = -3;
			break;
		default:
			break;
		}
		return nRetVal;
	}
	nVTMode = (int)VTMode;
	return 0;
}

int APIENTRY dcm_getmcupinresult(const char* lpszPinName, USHORT usSiteNo)
{
	CDCM* pDCM = g_pDCMDistribution->GetClass();
	CDriverAlarm* pAlarm = pDCM->GetAlarm();
	pAlarm->SetDriverPackName("GetMCUPinResult");
	int nRetVal = pDCM->GetPinMCUResult(lpszPinName, usSiteNo);
	if (0 > nRetVal)
	{
		switch (nRetVal)
		{
		case -1:
			nRetVal = VECTOR_FILE_NOT_LOADED;
			break;
		case -2:
		case -3:
			nRetVal = PIN_NAME_ERROR;
			break;
		case -4:
			nRetVal = SITE_ERROR;
			break;
		case -5:
			nRetVal = SITE_INVALID;
			break;
		case -6:
			///<The pin is not belongs to the instance
			nRetVal = PIN_NOT_BELONGS;
			break;
		case -7:
			nRetVal = BOARD_NOT_INSERT_ERROR;
			break;
		case -8:
			nRetVal = CHANNEL_NOT_EXISTED;
			break;
		case -9:
			nRetVal = VECTOR_NOT_RAN;
			break;
		case -10:
			nRetVal = VECTOR_RUNING;
			break;
		default:
			break;
		}
	}
	pAlarm->Output();
	return nRetVal;
}

int APIENTRY dcm_getmcupinrunstatus(const char* lpszPinName, USHORT usSiteNo)
{
	CDCM* pDCM = g_pDCMDistribution->GetClass();
	CDriverAlarm* pAlarm = pDCM->GetAlarm();
	pAlarm->SetDriverPackName("GetMCUPinRunStatus");
	int nRetVal = pDCM->GetRunningStatus(lpszPinName, usSiteNo);
	if (0 > nRetVal)
	{
		switch (nRetVal)
		{
		case -1:
			nRetVal = VECTOR_FILE_NOT_LOADED;
			break;
		case -2:
			///<The point of pin name is nullptr
		case -3:
			nRetVal = PIN_NAME_ERROR;
			break;
		case -4:
			nRetVal = SITE_ERROR;
			break;
		case -5:
			nRetVal = SITE_INVALID;
			break;
		case -6:
			///<The pin is not belongs to the instance
			nRetVal = PIN_NOT_BELONGS;
			break;
		case -7:
			nRetVal = BOARD_NOT_INSERT_ERROR;
			break;
		case -8:
			nRetVal = CHANNEL_NOT_EXISTED;
			break;
		default:
			break;
		}
	}
	pAlarm->Output();
	return nRetVal;
}

int APIENTRY dcm_GetFailCount(const char* lpszPinName, USHORT usSiteNo, ULONG& ulFailCount)
{
	CDCM* pDCM = g_pDCMDistribution->GetClass();
	CDriverAlarm* pAlarm = pDCM->GetAlarm();
	pAlarm->SetDriverPackName("GetFailCount");
	int nRetVal = pDCM->GetFailCount(lpszPinName, usSiteNo);
	if (0 > nRetVal)
	{
		switch (nRetVal)
		{
		case -1:
			nRetVal = VECTOR_FILE_NOT_LOADED;
			break;
		case -2:
			///<The point of pin name is nullptr
		case -3:
			nRetVal = PIN_NAME_ERROR;
			break;
		case -4:
			nRetVal = SITE_ERROR;
			break;
		case -5:
			nRetVal = SITE_INVALID;
			break;
		case -6:
			///<The pin is not belongs to the instance
			nRetVal = PIN_NOT_BELONGS;
			break;
		case -7:
			nRetVal = BOARD_NOT_INSERT_ERROR;
			break;
		case -8:
			nRetVal = CHANNEL_NOT_EXISTED;
			break;
		case -9:
			nRetVal = VECTOR_NOT_RAN;
			break;
		case -10:
			nRetVal = VECTOR_RUNING;
			break;
		default:
			break;
		}
		ulFailCount = -1;
	}
	else
	{
		ulFailCount = nRetVal;
		nRetVal = 0;
	}
	pAlarm->Output();
	return nRetVal;
}

char* APIENTRY dcm_GetStopLabel(USHORT usSiteNo)
{
	CDCM* pDCM = g_pDCMDistribution->GetClass();
	CDriverAlarm* pAlarm = pDCM->GetAlarm();
	pAlarm->SetDriverPackName("GetStopLabel");
	const char* lpszLabel = pDCM->GetStopLineLabel(usSiteNo);
	pAlarm->Output();
	return (char*)lpszLabel;
}

int APIENTRY dcm_GetStopLineNo(USHORT usSiteNo, ULONG& ulStopAddress)
{
	CDCM* pDCM = g_pDCMDistribution->GetClass();
	CDriverAlarm* pAlarm = pDCM->GetAlarm();
	pAlarm->SetDriverPackName("GetStopLineNo");
	int nRetVal = pDCM->GetStopLineNo(usSiteNo);
	if (0 > nRetVal)
	{
		switch (nRetVal)
		{
		case -1:
			nRetVal = VECTOR_FILE_NOT_LOADED;
			break;
		case -2:
			nRetVal = SITE_ERROR;
			break;
		case -3:
			nRetVal = SITE_INVALID;
			break;
		case -4:
			nRetVal = CHANNEL_NOT_EXISTED;
			break;
		case -5:
			nRetVal = VECTOR_NOT_RAN;
			break;
		case -6:
			nRetVal = VECTOR_RUNING;
			break;
		case -7:
			nRetVal = BOARD_NOT_INSERT_ERROR;
			break;
		default:
			break;
		}
		ulStopAddress = -1;
	}
	else
	{
		ulStopAddress = nRetVal;
	}
	pAlarm->Output();
	return nRetVal;
}

int APIENTRY dcm_GetRunLineCount(const char* lpszPinName, USHORT usSiteNo, ULONG& ulLineCount)
{
	CDCM* pDCM = g_pDCMDistribution->GetClass();
	CDriverAlarm* pAlarm = pDCM->GetAlarm();
	pAlarm->SetDriverPackName("GetRunLineCount");
	int nRetVal = pDCM->GetRunLineCount(lpszPinName, usSiteNo, ulLineCount);
	if (0 != nRetVal)
	{
		switch (nRetVal)
		{
		case -1:
			nRetVal = VECTOR_FILE_NOT_LOADED;
			break;
		case -2:
			///<The point of pin name is nullptr
		case -3:
			nRetVal = PIN_NAME_ERROR;
			break;
		case -4:
			nRetVal = SITE_ERROR;
			break;
		case -5:
			nRetVal = SITE_INVALID;
			break;
		case -6:
			///<The pin is not belongs to the instance
			nRetVal = PIN_NOT_BELONGS;
			break;
		case -7:
			nRetVal = BOARD_NOT_INSERT_ERROR;
			break;
		case -8:
			///<The channel number is over range, not will happen
			break;
		case -9:
			nRetVal = CHANNEL_NOT_EXISTED;
			break;
		}
		ulLineCount = -1;
	}
	pAlarm->Output();
	return nRetVal;
}

int APIENTRY dcm_GetFirstFailLineNo(const char* lpszPinName, USHORT usSiteNo, ULONG& ulFirstFailLine)
{
	CDCM* pDCM = g_pDCMDistribution->GetClass();
	CDriverAlarm* pAlarm = pDCM->GetAlarm();
	pAlarm->SetDriverPackName("GetFirstFailLineNo");
	int nRetVal = pDCM->GetFirstFailLineNo(lpszPinName, usSiteNo);
	if (0 > nRetVal)
	{
		switch (nRetVal)
		{
		case -1:
			nRetVal = VECTOR_FILE_NOT_LOADED;
			break;
		case -2:
			///<The point of pin name is nullptr
		case -3:
			nRetVal = PIN_NAME_ERROR;
			break;
		case -4:
			nRetVal = SITE_ERROR;
			break;
		case -5:
			nRetVal = SITE_INVALID;
			break;
		case -6:
			///<The pin is not belongs to the instance
			nRetVal = PIN_NOT_BELONGS;
			break;
		case -7:
			nRetVal = BOARD_NOT_INSERT_ERROR;
			break;
		case -8:
			///<Channel over range
		case -9:
			nRetVal = CHANNEL_NOT_EXISTED;
			break;
		case -10:
			nRetVal = VECTOR_NOT_RAN;
			break;
		case -11:
			nRetVal = VECTOR_RUNING;
			break;
		case -12:
			nRetVal = NO_FAIL_LINE;
			break;
		case -13:
			nRetVal = FAIL_LINE_NOT_SAVE;
			break;
		default:
			break;
		}
		ulFirstFailLine = -1;
	}
	else
	{
		ulFirstFailLine = nRetVal;
		nRetVal = 0;
	}
	pAlarm->Output();
	return nRetVal;
}

int APIENTRY dcm_WriteWaveData(const char* lpszPinGroup, const char* lpszStartLabel, USHORT usSiteNo, ULONG ulOffset, int nCount, ULONG ulWaveData)
{
	CDCM* pDCM = g_pDCMDistribution->GetClass();
	CDriverAlarm* pAlarm = pDCM->GetAlarm();
	pAlarm->SetDriverPackName("WriteWaveData");
	if (0 >= nCount)
	{
		pAlarm->SetAlarmID(ALARM_ID::ALARM_LINE_COUNT_OVER_RANGE);
		pAlarm->SetParamName("nCount");
		pAlarm->SetAlarmMsg("The line count(%d) is over range[%d, %d], and will be set to %d.", nCount, 1, 32, 1);
		pAlarm->SetAlarmLevel(CDriverAlarm::ALARM_LEVEL::AlarmWarning);
		pAlarm->Output();
		nCount = 1;
	}
	else if (32 < nCount)
	{
		pAlarm->SetAlarmID(ALARM_ID::ALARM_LINE_COUNT_OVER_RANGE);
		pAlarm->SetParamName("nCount");
		pAlarm->SetAlarmMsg("The line count(%d) is over range[%d, %d], and will be set to %d.", nCount, 1, 32, 32);
		pAlarm->SetAlarmLevel(CDriverAlarm::ALARM_LEVEL::AlarmWarning);
		pAlarm->Output();
		nCount = 32;
	}
	int nRetVal = pDCM->SetLineInfo(lpszPinGroup, usSiteNo, lpszStartLabel, ulOffset, nCount, TRUE);
	if (0 != nRetVal)
	{
		switch (nRetVal)
		{
		case -1:
			nRetVal = VECTOR_FILE_NOT_LOADED;
			break;
		case -2:
			///<The point of pin group is nullptr
		case -3:
			nRetVal =  PIN_GROUP_ERROR;
			break;
		case -4:
			nRetVal = SITE_ERROR;
			break;
		case -5:
			nRetVal = SITE_INVALID;
			break;
		case -6:
			///<The pin is not belongs to the instance
			nRetVal = PIN_NAME_ERROR;
			break;
		case -7:
			///<The point of start label is nullptr
		case -8:
			nRetVal =  START_LABEL_ERROR;
			break;
		case -9:
			nRetVal = OFFSET_ERROR;
			break;
		case -10:
			nRetVal =  VECTOR_LINE_OVER_RANGE;
			break;
		case -11:
			nRetVal =  ALLOCATE_MEMORY_FAIL;
			break;
		case -12:
			nRetVal =  CHANNEL_NOT_EXISTED;
			break;
		case -13:
			nRetVal =  BOARD_NOT_INSERT_ERROR;
			break;
		default:
			break;
		}
		pAlarm->Output();
		return nRetVal;
	}
	
	int nDataByteCount = (nCount + 7) / 8;
	BYTE* pbyData = nullptr;
	try
	{
		pbyData = new BYTE[nDataByteCount];
		memset(pbyData, 0, nDataByteCount * sizeof(BYTE));
	}
	catch (const std::exception&)
	{
		return ALLOCATE_MEMORY_FAIL;
	}
	int nCurByteBitCount = 7;
	BOOL bFullByte = 0 == nCount % 8 ? TRUE : FALSE;
	for (int nDataIndex = 0; nDataIndex < nCount;++nDataIndex)
	{
		if (!bFullByte && nDataByteCount - 1 == nDataIndex / 8)
		{
			nCurByteBitCount = nCount % 8 - 1;
		}
		pbyData[nDataIndex / 8] |= ((ulWaveData >> (nCount - nDataIndex - 1)) & 0x01) << (nCurByteBitCount - (nDataIndex % 8));
	}

	nRetVal = pDCM->SetSiteWaveData(usSiteNo, pbyData);
	if (nullptr != pbyData)
	{
		delete[] pbyData;
		pbyData = nullptr;
	}
	if (0 != nRetVal)
	{
		switch (nRetVal)
		{
		case -1:
		case -2:
			///<Not will happened
		case -3:
			nRetVal = SITE_ERROR;
			break;
		case -4:
			nRetVal = SITE_INVALID;
			break;
		case -5:
			///<The point of wave data is nullptr, not will happened
		case -6:
			nRetVal = ALLOCATE_MEMORY_FAIL;
			break;
		case -7:
			nRetVal = BOARD_NOT_INSERT_ERROR;
			break;
		default:
			break;
		}
		pAlarm->Output();
	
		return nRetVal;
	}
	nRetVal = pDCM->WriteData();
	pAlarm->Output();
	return 0;
}

int APIENTRY dcm_GetFailLineNo(const char* lpszPinName, USHORT usSiteNo, int nCount, DWORD dwData[])
{
	CDCM* pDCM = g_pDCMDistribution->GetClass();
	CDriverAlarm* pAlarm = pDCM->GetAlarm();
	pAlarm->SetDriverPackName("GetFailLineNo");
	
	std::vector<int> vecFailLineNo;
	int nRetVal = pDCM->GetFailLineNo(lpszPinName, usSiteNo, nCount, vecFailLineNo);
	if (0 != nRetVal && -12 != nRetVal)
	{
		switch (nRetVal)
		{
		case -1:
			nRetVal = VECTOR_FILE_NOT_LOADED;
			break;
		case -2:
			///<The point of pin name is nullptr
		case -3:
			nRetVal = PIN_NAME_ERROR;
			break;
		case -4:
			nRetVal = SITE_ERROR;
			break;
		case -5:
			nRetVal = SITE_INVALID;
			break;
		case -6:
			///<The pin is not belongs to the instance
			nRetVal = PIN_NOT_BELONGS;
			break;
		case -7:
			nRetVal = BOARD_NOT_INSERT_ERROR;
			break;
		case -8:
			///<The channel is over range, not will happen
			break;
		case -9:
			nRetVal = CHANNEL_NOT_EXISTED;
			break;
		case -10:
			nRetVal = VECTOR_NOT_RAN;
			break;
		case -11:
			nRetVal = VECTOR_RUNING;
			break;
		}
		if (nullptr != dwData)
		{
			memset(dwData, -1, nCount * sizeof(DWORD));
		}
	}
	else
	{
		if (nullptr == dwData)
		{
			pAlarm->SetAlarmID(ALARM_ID::ALARM_PARAM_NULLPTR);
			pAlarm->SetParamName("ulFailLineNo");
			pAlarm->SetSite(usSiteNo);
			pAlarm->SetPinString(lpszPinName, TRUE);
			pAlarm->SetAlarmMsg("The parameter of ulFailLineNo is null pointer.");
		}
		else
		{
			memset(dwData, -1, nCount * sizeof(DWORD));
			int nFailLineCount = vecFailLineNo.size();
			for (int nIndex = 0; nIndex < nFailLineCount; ++nIndex)
			{
				if (nIndex >= nCount)
				{
					break;
				}
				dwData[nIndex] = vecFailLineNo[nIndex];
			}
			if (0 != nRetVal)
			{
				nRetVal = FAIL_LINE_NOT_SAVE;
			}
		}
	}
	pAlarm->Output();
	return nRetVal;
}

int APIENTRY dcm_setDriveVTorTristate(const char* lpszPinGroup, double dVTVoltValue, BYTE byMode)
{
	CDCM* pDCM = g_pDCMDistribution->GetClass();
	CDriverAlarm* pAlarm = pDCM->GetAlarm();
	pAlarm->SetDriverPackName("SetVT");
	int nRetVal = pDCM->SetVT(lpszPinGroup, dVTVoltValue, (VT_MODE)byMode);
	if (0 != nRetVal)
	{
		switch (nRetVal)
		{
		case -1:
			nRetVal = VECTOR_FILE_NOT_LOADED;
			break;
		case -2:
			///<The point of pin group is nullptr
		case -3:
			nRetVal = PIN_GROUP_ERROR;
			break;
		case -4:
			///<No valid site
			nRetVal = SITE_INVALID;
			break;
		case -5:
			///<The pin is not belongs to the instance
			nRetVal = PIN_NOT_BELONGS;
			break;
		case -6:
			nRetVal = PIN_LEVEL_ERROR;
			break;
		case -7:
			///<The mode error, will not happed
			break;
		case -8:
			///<Channel not existed
			nRetVal = CHANNEL_NOT_EXISTED;
			break;
		case -9:
			nRetVal = BOARD_NOT_INSERT_ERROR;
			break;
		default:
			break;
		}
	}
	pAlarm->Output();
	return nRetVal;
}

int APIENTRY dcm_runvectorwithgroup(const char* lpszPinGroup, const char* lpszStartLabel, const char* lpszStopLabel, BOOL bWaitFinish)
{
	CDCM* pDCM = g_pDCMDistribution->GetClass();
	CDriverAlarm* pAlarm = pDCM->GetAlarm();
	pAlarm->SetDriverPackName("RunVectorWithGroup");
	int nRetVal = pDCM->RunVector(lpszPinGroup, lpszStartLabel, lpszStopLabel, bWaitFinish);
	if (0 != nRetVal)
	{
		switch (nRetVal)
		{
		case -1:
			nRetVal = VECTOR_FILE_NOT_LOADED;
			break;
		case -2:
			///<The point of pin group is nullptr
		case -3:
			nRetVal = PIN_GROUP_ERROR;
			break;
		case -4:
			///<All site invalid
			nRetVal = SITE_INVALID;
			break;
		case -5:
			///<The pin is not belongs to the instance
			nRetVal = PIN_NOT_BELONGS;
			break;
		case -6:
			///<The point of start label is nullptr
		case -7:
			nRetVal = START_LABEL_ERROR;
			break;
		case -8:
			///<The stop label is nullptr
		case -9:
			nRetVal = STOP_LABEL_ERROR;
			break;
		case -10:
			nRetVal = START_LABLE_AFTER_END;
			break;
		case -11:
			nRetVal = BOARD_NOT_INSERT_ERROR;
			break;
		default:
			break;
		}
	}
	pAlarm->Output();
	return nRetVal;
}

int APIENTRY dcm_GetChannelFromPinGroup(const char* lpszPinGroup, USHORT uSiteID, int* nChannelNo, int nArrayLength, int& nChannelCount)
{
	vector<CHANNEL_INFO> vecChannel;
	CDCM* pDCM = g_pDCMDistribution->GetClass();
	int nRetVal = pDCM->GetPinGroupChannel(lpszPinGroup, uSiteID, vecChannel);
	if (0 != nRetVal)
	{
		switch (nRetVal)
		{
		case -1:
			return VECTOR_FILE_NOT_LOADED;
			break;
		case -2:
			///<The point of pin group is nullptr
		case -3:
			return PIN_GROUP_ERROR;
		case -4:
			return SITE_ERROR;
		case -5:
			///<The pin is not belongs to the instance
			return PIN_NOT_BELONGS;
			break;

		default:
			break;
		}
	}
	nChannelCount = vecChannel.size();
	if (nullptr == nChannelNo || nChannelCount < nArrayLength)
	{
		return ARRAY_LENGTH_NOT_ENOUGH;
	}
	for (int nChannelIndex = 0; nChannelIndex < nChannelCount; ++nChannelIndex)
	{
		nChannelNo[nChannelIndex] = GetGlobalChannel(vecChannel[nChannelIndex]);
	}
	return 0;
}

int APIENTRY dcm_GetVectorLineCount(const char* lpszStartLabel, const char* lpszStopLabel, ULONG& ulVectorLineCount)
{
	CDCM* pDCM = g_pDCMDistribution->GetClass();
	if (nullptr == pDCM)
	{
		return -1;
	}
	int nRetVal = pDCM->GetLineCount(lpszStartLabel, lpszStopLabel);
	if (0 > nRetVal)
	{
		switch (nRetVal)
		{
		case -1:
			nRetVal = VECTOR_FILE_NOT_LOADED;
			break;
		case -2:
			nRetVal = START_LABEL_ERROR;
			break;
		case -3:
			nRetVal = STOP_LABEL_ERROR;
			break;
		default:
			break;
		}
		return nRetVal;
	}
	ulVectorLineCount = nRetVal;
	return 0;
}

int APIENTRY dcm_GetLabelLineNo(const char* lpszLabelName, BOOL bBRAMLine)
{
	CDCM* pDCM = g_pDCMDistribution->GetClass();
	if (nullptr == pDCM)
	{
		return -1;
	}
	
	return pDCM->GetLabelLineNo(lpszLabelName, bBRAMLine);
}

int APIENTRY dcm_set_channel_alarm_mask(const char* lpszPinName, USHORT usSiteNo, bool bStatus)
{
	CDCM* pDCM = g_pDCMDistribution->GetClass();
	CDriverAlarm* pAlarm = pDCM->GetAlarm();
	pAlarm->SetDriverPackName("SetAlarmMask");
	int nRetVal = pDCM->ShieldAlarm(lpszPinName, usSiteNo, bStatus);
	if (0 != nRetVal)
	{
		switch (nRetVal)
		{
		case -1:
			nRetVal = VECTOR_FILE_NOT_LOADED;
			break;
		case -2:
			///<The point of pin name is nullptr
		case -3:
			nRetVal = PIN_NAME_ERROR;
			break;
		case -4:
			nRetVal = SITE_ERROR;
			break;
		case -5:
			nRetVal = SITE_INVALID;
			break;
		case -6:
			///<The pin is not belongs to the instance
			nRetVal = PIN_NOT_BELONGS;
			break;
		}
	}
	pAlarm->Output();
	return nRetVal;
}

bool APIENTRY dcm_get_channel_alarm_mask(const char* lpszPinName, USHORT usSiteNo)
{
	CDCM* pDCM = g_pDCMDistribution->GetClass();
	CDriverAlarm* pAlarm = pDCM->GetAlarm();
	pAlarm->SetDriverPackName("GetAlarmMask");
	int nRetVal = pDCM->GetShieldStatus(lpszPinName, usSiteNo);
	pAlarm->Output();
	if (0 > nRetVal)
	{
		return false;
	}
	return true;
}

int APIENTRY dcm_set_clamp_alarm_mask(const char* lpszPinName, USHORT usSiteNo, bool bMaskFlag, WORD wFuncName)
{
	CDCM* pDCM = g_pDCMDistribution->GetClass();
	CDriverAlarm* pAlarm = pDCM->GetAlarm();
	pAlarm->SetDriverPackName("SetReadClampAlarmMask");
	const char* lpszFuncName[2] = { "SetPMUMode", "PMUMeasure" };
	int nRetVal = 0;
	for (int nIndex = 0; nIndex < 2; ++nIndex)
	{
		nRetVal = pDCM->ShieldFunctionAlarm(lpszPinName, usSiteNo, lpszFuncName[nIndex], bMaskFlag, ALARM_ID::ALARM_PMU_CLAMP);
		if (0 != nRetVal)
		{
			switch (nRetVal)
			{
			case -1:
				nRetVal = VECTOR_FILE_NOT_LOADED;
				break;
			case -2:
				///<The point of pin name is nullptr
			case -3:
				nRetVal = PIN_NAME_ERROR;
				break;
			case -4:
				nRetVal = SITE_ERROR;
				break;
			case -5:
				nRetVal = SITE_INVALID;
				break;
			case -6:
				///<The pin is not belongs to the instance
				nRetVal = PIN_NOT_BELONGS;
				break;
			case -7:
				nRetVal = BOARD_NOT_INSERT_ERROR;
				break;
			case -8:
				///<The channel number is over exceed, not will happen
				break;
			case -9:
				nRetVal = CHANNEL_NOT_EXISTED;
				break;
			}
		}
	}
	pAlarm->Output();
	return nRetVal;
}

WORD APIENTRY dcm_get_clamp_alarm_mask(const char* lpszPinName, BYTE usSiteNo)
{
	CDCM* pDCM = g_pDCMDistribution->GetClass();
	CDriverAlarm* pAlarm = pDCM->GetAlarm();
	WORD wStatus = 0;
	pAlarm->SetDriverPackName("SetReadClampAlarmMask");
	const char* lpszFuncName[2] = { "SetPPMUMode", "PPMUMeasure" };
	int nRetVal = 0;
	for (int nIndex = 0; nIndex < 2; ++nIndex)
	{
		nRetVal = pDCM->GetShieldFunctionAlarm(lpszPinName, usSiteNo, lpszFuncName[nIndex], ALARM_ID::ALARM_PMU_CLAMP);
		if (0 != nRetVal)
		{
			nRetVal = 1;
			break;
		}
		wStatus |= nRetVal << nIndex;
	}
	pAlarm->Output();
	return wStatus;
}

int APIENTRY dcm_SetCapture(const char* lpszPinGroup, const char* lpszLabel, USHORT usSiteNo, ULONG ulOffset, int nCount)
{
	CDCM* pDCM = g_pDCMDistribution->GetClass();
	CDriverAlarm* pAlarm = pDCM->GetAlarm();
	pAlarm->SetDriverPackName("SetCapture");
	int nRetVal = pDCM->SetCaptureLine(lpszPinGroup, usSiteNo, lpszLabel, ulOffset, nCount);
	if (0 != nRetVal)
	{
		switch (nRetVal)
		{
		case -1:
			nRetVal = VECTOR_FILE_NOT_LOADED;
			break;
		case -2:
			///<The point of pin group is nullptr
		case -3:
			nRetVal = PIN_GROUP_ERROR;
			break;
		case -4:
			nRetVal = SITE_ERROR;
			break;
		case -5:
			nRetVal = SITE_INVALID;
			break;
		case -6:
			///<The pin is not belongs to the instance
			nRetVal = PIN_NOT_BELONGS;
			break;
		case -7:
			///<The point of start label is nullptr
		case -8:
			nRetVal = START_LABEL_ERROR;
			break;
		case -9:
			nRetVal = OFFSET_ERROR;
			break;
		case -10:
			nRetVal = VECTOR_NOT_IN_LAST_RAN;
			break;
		case -11:
			nRetVal = VECTOR_NOT_RAN;
			break;
		case -12:
			nRetVal = VECTOR_RUNING;
			break;
		case -13:
			nRetVal = CHANNEL_NOT_EXISTED;
			break;
		case -14:
			nRetVal = BOARD_NOT_INSERT_ERROR;
			break;
		default:
			break;
		}
	}
	pAlarm->Output();
	return nRetVal;
}

int APIENTRY dcm_GetCaptureData(const char* lpszPinName, USHORT usSiteNo, ULONG& ulCaptureData)
{
	CDCM* pDCM = g_pDCMDistribution->GetClass();
	CDriverAlarm* pAlarm = pDCM->GetAlarm();
	pAlarm->SetDriverPackName("GetCaptureData");
	int nRetVal = pDCM->GetCaptureData(lpszPinName, usSiteNo, ulCaptureData);
	if (0 != nRetVal)
	{
		switch (nRetVal)
		{
		case -1:
			nRetVal = VECTOR_FILE_NOT_LOADED;
			break;
		case -2:
			///<The point of pin name is nullptr
		case -3:
			nRetVal = PIN_NAME_ERROR;
			break;
		case -4:
			nRetVal = SITE_ERROR;
			break;
		case -5:
			nRetVal = SITE_INVALID;
			break;
		case -6:
			///<The pin is not belongs to the instance
			nRetVal = PIN_NOT_BELONGS;
			break;
		case -7:
			nRetVal = BOARD_NOT_INSERT_ERROR;
			break;
		case -8:
			///<The channel number is over exceed, not will happen
			break;
		case -9:
			nRetVal = CHANNEL_NOT_EXISTED;
			break;
		case -10:
			nRetVal = VECTOR_NOT_RAN;
			break;
		case -11:
			nRetVal = VECTOR_RUNING;
			break;
		case -12:
			nRetVal = FUNCTION_USE_ERROR;
			break;
		case -13:
			nRetVal = FAIL_LINE_NOT_SAVE;
			break;
		case -14:
			nRetVal = CAPTURE_NOT_SUPPORTTED;
			break;
		default:
			break;
		}
		ulCaptureData = -1;
	}
	pAlarm->Output();
	return nRetVal;
}

int APIENTRY dcm_SetPrereadVector(const char* lpszStartLabel, const char* lpszStopLabel)
{
	CDCM* pDCM = g_pDCMDistribution->GetClass();
	CDriverAlarm* pAlarm = pDCM->GetAlarm();
	pAlarm->SetDriverPackName("SetPrereadVector");
	int nRetVal = 0;
	nRetVal = pDCM->SetPrereadVector(lpszStartLabel, lpszStopLabel);
	if (0 != nRetVal)
	{
		switch (nRetVal)
		{
		case -1:
			nRetVal = VECTOR_FILE_NOT_LOADED;
			break;
		case -2:
			///<The point of start label is nullptr
		case -3:
			nRetVal = START_LABEL_ERROR;
			break;
		case -4:
			nRetVal = STOP_LABEL_ERROR;
			break;
		case -5:
			nRetVal = START_LABLE_AFTER_END;
			break;
		case -6:
			nRetVal = SECTION_NUM_EXCEED;
			break;
		case -7:
			nRetVal = ALLOCATE_MEMORY_FAIL;
			break;
		case -8:
			nRetVal = BOARD_NOT_INSERT_ERROR;
			break;
		default:
			break;
		}
	}
	pAlarm->Output();
	return nRetVal;
}

ULONG APIENTRY dcm_read_FPGA_Version(BYTE bySlotNo, BYTE ctrl)
{
	CDCM* pDCM = g_pDCMDistribution->GetClass(0);
	if (nullptr == pDCM)
	{
		return -1;
	}
	return pDCM->GetFPGARevision(bySlotNo, ctrl);
}

#define DCM_TYPE				0x8213		///< 用于软件示波器，该值需要与DataInterface工程QueryImp.h文件中eSourceType枚举定义的eDCM值一致
void SoftwareTopShown(const char* lpszPinGroup, CDCM* pDCM)
{
	vector<USHORT> vecPinNo;
	int nRetVal = pDCM->GetPinNo(lpszPinGroup, vecPinNo);
	if (0 != nRetVal)
	{
		return;
	}

	set<USHORT> setSite;
	pDCM->GetSiteInfo(ALL_SITE, setSite);

	for (auto Site : setSite)
	{
		for (int nPinNo : vecPinNo)
		{
			StsSetModuleMeas(DCM_TYPE, nPinNo, Site);
		}
	}
}

int APIENTRY dcm_ppmuMultiMeasure(const char* lpszPinGroup, UINT uSampleTimes, double dSamplePeriod)
{
	CDCM* pDCM = g_pDCMDistribution->GetClass();
	CDriverAlarm* pAlarm = pDCM->GetAlarm();
	pAlarm->SetDriverPackName("PPMUMeasure");
	int nRetVal = pDCM->PMUMeasure(lpszPinGroup, uSampleTimes, dSamplePeriod);
	if (0 != nRetVal)
	{
		switch (nRetVal)
		{
		case -1:
			nRetVal = VECTOR_FILE_NOT_LOADED;
			break;
		case -2:
			///<The point of pin group is nullptr
		case -3:
			nRetVal = PIN_GROUP_ERROR;
			break;
		case -4:
			///<The pin is not belongs to the instance
			nRetVal = PIN_NOT_BELONGS;
			break;
		case -5:
			nRetVal = CHANNEL_NOT_EXISTED;
			break;
		case -6:
			nRetVal = PMU_MEASURE_ERROR;
			break;
		case -7:
			nRetVal = BOARD_NOT_INSERT_ERROR;
			break;
		default:
			break;
		}
	}
	else
	{
		SoftwareTopShown(lpszPinGroup, pDCM);
	}

	pAlarm->Output();
	return nRetVal;
}


double APIENTRY dcm_getPpmuMultiMeasResult(const char* lpszPinName, USHORT usSiteNo, int sampleNumber)
{
	CDCM* pDCM = g_pDCMDistribution->GetClass();
	if (nullptr == pDCM)
	{
		return PMU_MEASURE_ERROR;
	}
	CDriverAlarm* pAlarm = pDCM->GetAlarm();
	pAlarm->SetDriverPackName("GetPPMUMeasResult");
	double dValue = pDCM->GetPMUMeasureResult(lpszPinName, usSiteNo, sampleNumber);
	pAlarm->Output();
	return dValue;
}

int APIENTRY dcm_write_flash_calibration_data_by_Control(BYTE bySlotNo, BYTE byControllerIndex, DCM_CAL_DATA* pCalData, BYTE byElementCount)
{
	CDCM* pDCM = g_pDCMDistribution->GetClass();
	if (nullptr == pDCM)
	{
		return -1;
	}
	int nRetVal = pDCM->SetCalibrationData(bySlotNo, byControllerIndex, (CAL_DATA*)pCalData, byElementCount);
	if (0 != nRetVal)
	{
		switch (nRetVal)
		{
		case -1:
			nRetVal = BOARD_NOT_INSERT_ERROR;
			break;
		case -2:
		case -3:
			nRetVal = BOARD_NOT_INSERT_ERROR;
			break;
		case -4:
			///<The point of calibration data is nullptr
			nRetVal = PARAM_NULLPTR;
			break;
		case -5:
			nRetVal = ARRAY_LENGTH_NOT_ENOUGH;
			break;
		case -6:
			nRetVal = FLASH_ERROR;
			break;
		default:
			break;
		}
	}
	return nRetVal;
}

DWORD APIENTRY dcm_read_flash_calibration_data_by_control(BYTE bySlotNo, BYTE byControllerIndex, DCM_CAL_DATA* pCalData, BYTE byElementCount)
{
	CDCM* pDCM = g_pDCMDistribution->GetClass();
	if (nullptr == pDCM)
	{
		return -1;
	}
	int nRetVal = pDCM->GetCalibrationData(bySlotNo, byControllerIndex, (CAL_DATA*)pCalData, byElementCount);
	if (0 != nRetVal)
	{
		switch (nRetVal)
		{
		case -1:
			nRetVal = BOARD_NOT_INSERT_ERROR;
			break;
		case -2:
			nRetVal = CONTROL_NO_EXCEED;
			break;
		case -3:
			nRetVal = BOARD_NOT_INSERT_ERROR;
			break;
		case -4:
			nRetVal = PARAM_NULLPTR;
			break;
		case -5:
			nRetVal = PARAM_NULLPTR;
			break;
		case -6:
			nRetVal = FLASH_ERROR;
			break;
		case -7:
			nRetVal = -1;
		default:
			break;
		}
	}
	return nRetVal;
}

int APIENTRY dcm_SetCalibrationInfoByCtrl(BYTE bySlotNo, BYTE byCtrlNo, STS_CALINFO* pCalInfo, BYTE* byChannelStatus, int nElementCount)
{
	CDCM* pDCM = g_pDCMDistribution->GetClass();
	if (nullptr == pDCM)
	{
		return -1;
	}
	int nRetVal = pDCM->SetCalibrationInfo(bySlotNo, byCtrlNo, pCalInfo, byChannelStatus, nElementCount);
	if (0 != nRetVal)
	{
		switch (nRetVal)
		{
		case -1:
		case -2:
		case -3:
			nRetVal = BOARD_NOT_INSERT_ERROR;
			break;
		case -4:
			nRetVal = PARAM_NULLPTR;
			break;
		case -5:
		case -6:
			nRetVal = -1;
		default:
			break;
		}
	}
	return nRetVal;
}

int APIENTRY dcm_GetCalibrationInfoByCtrl(BYTE bySlotNo, BYTE byCtrlNo, STS_CALINFO* pCalibrationInfo)
{
	CDCM* pDCM = g_pDCMDistribution->GetClass();
	if (nullptr == pDCM)
	{
		return -1;
	}
	int nRetVal = pDCM->GetCalibrationInfo(bySlotNo, byCtrlNo, pCalibrationInfo, DCM_CHANNELS_PER_CONTROL);
	if (0 != nRetVal)
	{
		switch (nRetVal)
		{
		case -1:
		case -2:
		case -3:
			nRetVal = BOARD_NOT_INSERT_ERROR;
			break;
		case -4:
			nRetVal = PARAM_NULLPTR;
			break;
		case -5:
		case -6:
			nRetVal = -1;
		default:
			break;
		}
	}
	return nRetVal;
}

int APIENTRY dcm_set_hardinfo_to_flash(BYTE byBoardNo, STS_HARDINFO* pHardInfo, int nModuleNum)
{
	if (DCM_MAX_BOARD_NUM <= byBoardNo)
	{
		return BOARD_NOT_INSERT_ERROR;
	}
	BYTE bySlotNo = g_abyBoardSlot[byBoardNo];

	CDCM* pDCM = g_pDCMDistribution->GetClass();
	if (nullptr == pDCM)
	{
		return -1;
	}
	int nRetVal = pDCM->SetHardInfo(bySlotNo, pHardInfo, nModuleNum);
	switch (nRetVal)
	{
	case -1:
		nRetVal = BOARD_NOT_INSERT_ERROR;
		break;
	case -2:
		nRetVal = PARAM_NULLPTR;
		break;
	case -3:
		nRetVal = -1;
		break;
	default:
		break;
	}
	return nRetVal;
}

int APIENTRY dcm_get_hardinfo_from_flash(BYTE byBoardNo, STS_HARDINFO* pHardInfo, int nElementCount, int& nModuleNum)
{
	nModuleNum = 0;
	if (DCM_MAX_BOARD_NUM <= byBoardNo)
	{
		return BOARD_NOT_INSERT_ERROR;
	}
	BYTE bySlotNo = g_abyBoardSlot[byBoardNo];
	CDCM* pDCM = g_pDCMDistribution->GetClass();
	if (nullptr == pDCM)
	{
		return -1;
	}
	int nRetVal = pDCM->GetHardInfo(bySlotNo, pHardInfo, nElementCount);
	if (0 > nRetVal)
	{
		nRetVal = BOARD_NOT_INSERT_ERROR;
	}
	else
	{
		nModuleNum = nRetVal;
		nRetVal = 0;
	}
	
	return nRetVal;
}

int APIENTRY dcm_GetModuleInfoByBoard(BYTE bySlotNo, char* lpszInfo, int nInfoSize, DCMINFO::MODULEINFO SelInfo, STS_BOARD_MODULE Module)
{
	CDCM* pDCM = g_pDCMDistribution->GetClass();
	CDriverAlarm* pAlarm = pDCM->GetAlarm();
	int nChannelNo = 0;
	char cDriverPaceFunc[30] = { 0 };
	switch (SelInfo)
	{
	case DCMINFO::MODULE_NAME:
		pAlarm->SetDriverPackName("GetBoardName");
		break;
	case DCMINFO::MODULE_SN:
		pAlarm->SetDriverPackName("GetBoardSN");
		break;
	case DCMINFO::MODULE_HDREV:
		pAlarm->SetDriverPackName("GetBoardHDRev");
		break;
	default:
		break;
	}
	int nRetVal = pDCM->GetModuleInfo(bySlotNo, lpszInfo, nInfoSize, (CDCM::MODULE_INFO)SelInfo, Module);
	if (0 != nRetVal)
	{
		switch (nRetVal)
		{
		case -1:
			nRetVal = BOARD_NOT_INSERT_ERROR;
			break;
		case -2:
			nRetVal = PARAM_NULLPTR;
			break;
		default:
			break;
		}
	}
	pAlarm->Output();
	return nRetVal;
}

int APIENTRY dcm_GetModuleInfoByPin(const char* lpszPinName, USHORT usSiteNo, char* lpszInfo, int nInfoSize, DCMINFO::MODULEINFO SelInfo, STS_BOARD_MODULE Module)
{
	CDCM* pDCM = g_pDCMDistribution->GetClass();
	CDriverAlarm* pAlarm = pDCM->GetAlarm();
	int nChannelNo = 0;
	char cDriverPaceFunc[30] = { 0 };
	switch (SelInfo)
	{
	case DCMINFO::MODULE_NAME:
		pAlarm->SetDriverPackName("GetBoardName");
		break;
	case DCMINFO::MODULE_SN:
		pAlarm->SetDriverPackName("GetBoardSN");
		break;
	case DCMINFO::MODULE_HDREV:
		pAlarm->SetDriverPackName("GetBoardHDRev");
		break;
	default:
		break;
	}
	int nRetVal = pDCM->GetModuleInfo(lpszPinName, usSiteNo, lpszInfo, nInfoSize, (CDCM::MODULE_INFO)SelInfo, Module);
	if (0 != nRetVal)
	{
		switch (nRetVal)
		{
		case -1:
			nRetVal = VECTOR_FILE_NOT_LOADED;
			break;
		case -2:
			///<The point of pin name is nullptr
		case -3:
			nRetVal = PIN_NAME_ERROR;
			break;
		case -4:
			nRetVal = SITE_ERROR;
			break;
		case -5:
			nRetVal = SITE_INVALID;
			break;
		case -6:
			///<The pin is not belongs to the instance
			nRetVal = PIN_NOT_BELONGS;
			break;
		case -7:
			nRetVal = BOARD_NOT_INSERT_ERROR;
			break;
		case -8:
			nRetVal = PARAM_NULLPTR;
			break;
		default:
			break;
		}
	}
	pAlarm->Output();
	return nRetVal;
}

time_t APIENTRY dcm_GetCalibrationDateByChannel(BYTE bySlotNo, int nChannelNo)
{
	STS_CALINFO CalibrationInfo;
	CDCM* pDCM = g_pDCMDistribution->GetClass();
	if (nullptr == pDCM)
	{
		return -1;
	}
	int nRetVal = pDCM->GetCalibrationInfo(bySlotNo, nChannelNo, CalibrationInfo);
	if (0 != nRetVal)
	{
		return 0;
	}
	return CalibrationInfo.calDate;
}

time_t APIENTRY dcm_GetCalibrationDateByPin(const char* lpszPinName, USHORT usSiteNo)
{
	CDCM* pDCM = g_pDCMDistribution->GetClass();
	CDriverAlarm* pAlarm = pDCM->GetAlarm();
	pAlarm->SetDriverPackName("GetCalibrationDate");
	STS_CALINFO CalibrationInfo;
	int nRetVal = pDCM->GetCalibrationInfo(lpszPinName, usSiteNo, CalibrationInfo);
	pAlarm->Output();
	if (0 != nRetVal)
	{		
		return 0;
	}
	return CalibrationInfo.calDate;
}

double APIENTRY dcm_GetCalibrationTemperatureByPin(const char* lpszPinName, USHORT usSiteNo)
{
	CDCM* pDCM = g_pDCMDistribution->GetClass();
	CDriverAlarm* pAlarm = pDCM->GetAlarm();
	pAlarm->SetDriverPackName("GetCalibrationTemperature");
	STS_CALINFO CalibrationInfo;
	int nRetVal = pDCM->GetCalibrationInfo(lpszPinName, usSiteNo, CalibrationInfo);
	if (0 != nRetVal)
	{
		switch (nRetVal)
		{
		case -1:
			nRetVal = VECTOR_FILE_NOT_LOADED;
			break;
		case -2:
			///<The point of pin name is nullptr
		case -3:
			nRetVal = PIN_NAME_ERROR;
			break;
		case -4:
		case -5:
			nRetVal = SITE_ERROR;
			break;
		case -6:
			///<The pin is not belongs to the instance
			nRetVal = PIN_NOT_BELONGS;
			break;
		case -7:
			nRetVal = BOARD_NOT_INSERT_ERROR;
			break;
		case -8:
		case -9:
			nRetVal = CHANNEL_NOT_EXISTED;
			break;
		case -10:
			///<Get calibration information fail
			nRetVal = GET_CALIBRATION_FAIL;
			break;
		default:
			break;
		}
	}
	pAlarm->Output();
	return CalibrationInfo.temperature;
}

double APIENTRY dcm_GetCalibrationHumidityByPin(const char* lpszPinName, USHORT usSiteNo)
{
	CDCM* pDCM = g_pDCMDistribution->GetClass();
	CDriverAlarm* pAlarm = pDCM->GetAlarm();
	pAlarm->SetDriverPackName("GetCalibrationHumidity");
	STS_CALINFO CalibrationInfo;
	int nRetVal = pDCM->GetCalibrationInfo(lpszPinName, usSiteNo, CalibrationInfo);
	if (0 != nRetVal)
	{
		switch (nRetVal)
		{
		case -1:
			nRetVal = VECTOR_FILE_NOT_LOADED;
			break;
		case -2:
			///<The point of pin name is nullptr
		case -3:
			nRetVal = PIN_NAME_ERROR;
			break;
		case -4:
		case -5:
			nRetVal = SITE_ERROR;
			break;
		case -6:
			///<The pin is not belongs to the instance
			nRetVal = PIN_NOT_BELONGS;
			break;
		case -7:
			nRetVal = BOARD_NOT_INSERT_ERROR;
			break;
		case -8:
		case -9:
			nRetVal = CHANNEL_NOT_EXISTED;
			break;
		case -10:
			///<Get calibration information fail
			nRetVal = GET_CALIBRATION_FAIL;
			break;
		default:
			break;
		}
	}
	pAlarm->Output();
	return CalibrationInfo.humidity;
}

BYTE APIENTRY dcm_GetCalibrationResultByChannel(BYTE bySlotNo, int nChannelNo)
{
	STS_CALINFO CalibrationInfo;
	CDCM* pDCM = g_pDCMDistribution->GetClass();
	if (nullptr == pDCM)
	{
		return -1;
	}
	int nRetVal = pDCM->GetCalibrationInfo(bySlotNo, nChannelNo, CalibrationInfo);
	if (0 != nRetVal)
	{
		return 0xFF;
	}
	return CalibrationInfo.calResult;
}

double APIENTRY dcm_GetCalibrationResultByPin(const char* lpszPinName, USHORT usSiteNo)
{
	CDCM* pDCM = g_pDCMDistribution->GetClass();
	CDriverAlarm* pAlarm = pDCM->GetAlarm();
	pAlarm->SetDriverPackName("GetCalibrationResult");
	STS_CALINFO CalibrationInfo;
	int nRetVal = pDCM->GetCalibrationInfo(lpszPinName, usSiteNo, CalibrationInfo);
	if (0 != nRetVal)
	{
		switch (nRetVal)
		{
		case -1:
			nRetVal = VECTOR_FILE_NOT_LOADED;
			break;
		case -2:
			///<The point of pin name is nullptr
		case -3:
			nRetVal = PIN_NAME_ERROR;
			break;
		case -4:
		case -5:
			nRetVal = SITE_ERROR;
			break;
		case -6:
			///<The pin is not belongs to the instance
			nRetVal = PIN_NOT_BELONGS;
			break;
		case -7:
			nRetVal = BOARD_NOT_INSERT_ERROR;
			break;
		case -8:
		case -9:
			nRetVal = CHANNEL_NOT_EXISTED;
			break;
		case -10:
			///<Get calibration information fail
			nRetVal = GET_CALIBRATION_FAIL;
			break;
		default:
			break;
		}
	}
	pAlarm->Output();
	return CalibrationInfo.calResult;
}

double APIENTRY dcm_GetCalibrationLogicRevByPin(const char* lpszPinName, USHORT usSiteNo)
{
	CDCM* pDCM = g_pDCMDistribution->GetClass();
	CDriverAlarm* pAlarm = pDCM->GetAlarm();
	pAlarm->SetDriverPackName("GetCalibrationLogicRev");
	STS_CALINFO CalibrationInfo;
	int nRetVal = pDCM->GetCalibrationInfo(lpszPinName, usSiteNo, CalibrationInfo);
	if (0 != nRetVal)
	{
		switch (nRetVal)
		{
		case -1:
			nRetVal = VECTOR_FILE_NOT_LOADED;
			break;
		case -2:
			///<The point of pin name is nullptr
		case -3:
			nRetVal = PIN_NAME_ERROR;
			break;
		case -4:
		case -5:
			nRetVal = SITE_ERROR;
			break;
		case -6:
			///<The pin is not belongs to the instance
			nRetVal = PIN_NOT_BELONGS;
			break;
		case -7:
			nRetVal = BOARD_NOT_INSERT_ERROR;
			break;
		case -8:
		case -9:
			nRetVal = CHANNEL_NOT_EXISTED;
			break;
		case -10:
			///<Get calibration information fail
			nRetVal = GET_CALIBRATION_FAIL;
			break;
		default:
			break;
		}
	}
	pAlarm->Output();
	return CalibrationInfo.logicRev;
}

double APIENTRY dcm_GetCalibrationMeterTypeByPin(const char* lpszPinName, USHORT usSiteNo)
{
	CDCM* pDCM = g_pDCMDistribution->GetClass();
	CDriverAlarm* pAlarm = pDCM->GetAlarm();
	pAlarm->SetDriverPackName("GetCalibrationMeterType");
	STS_CALINFO CalibrationInfo;
	int nRetVal = pDCM->GetCalibrationInfo(lpszPinName, usSiteNo, CalibrationInfo);
	if (0 != nRetVal)
	{
		switch (nRetVal)
		{
		case -1:
			nRetVal = VECTOR_FILE_NOT_LOADED;
			break;
		case -2:
			///<The point of pin name is nullptr
		case -3:
			nRetVal = PIN_NAME_ERROR;
			break;
		case -4:
		case -5:
			nRetVal = SITE_ERROR;
			break;
		case -6:
			///<The pin is not belongs to the instance
			nRetVal = PIN_NOT_BELONGS;
			break;
		case -7:
			nRetVal = BOARD_NOT_INSERT_ERROR;
			break;
		case -8:
		case -9:
			nRetVal = CHANNEL_NOT_EXISTED;
			break;
		case -10:
			///<Get calibration information fail
			nRetVal = GET_CALIBRATION_FAIL;
			break;
		default:
			break;
		}
	}
	pAlarm->Output();
	return CalibrationInfo.meterType;
}

double APIENTRY dcm_GetCalibrationCalboardlogicRevByPin(const char* lpszPinName, USHORT usSiteNo)
{
	CDCM* pDCM = g_pDCMDistribution->GetClass();
	CDriverAlarm* pAlarm = pDCM->GetAlarm();
	pAlarm->SetDriverPackName("GetCalibrationCalboardlogicRev");
	STS_CALINFO CalibrationInfo;
	int nRetVal = pDCM->GetCalibrationInfo(lpszPinName, usSiteNo, CalibrationInfo);
	if (0 != nRetVal)
	{
		switch (nRetVal)
		{
		case -1:
			nRetVal = VECTOR_FILE_NOT_LOADED;
			break;
		case -2:
			///<The point of pin name is nullptr
		case -3:
			nRetVal = PIN_NAME_ERROR;
			break;
		case -4:
		case -5:
			nRetVal = SITE_ERROR;
			break;
		case -6:
			///<The pin is not belongs to the instance
			nRetVal = PIN_NOT_BELONGS;
			break;
		case -7:
			nRetVal = BOARD_NOT_INSERT_ERROR;
			break;
		case -8:
		case -9:
			nRetVal = CHANNEL_NOT_EXISTED;
			break;
		case -10:
			///<Get calibration information fail
			nRetVal = GET_CALIBRATION_FAIL;
			break;
		default:
			break;
		}
	}
	pAlarm->Output();
	return CalibrationInfo.calBdLogicRev;
}

int APIENTRY dcm_GetCalibrationOptionInfoByPin(const char* lpszPinName, USHORT usSiteNo, char* lpszInfo, int nInfoSize, DCMINFO::CALINFO SelInfo)
{
	CDCM* pDCM = g_pDCMDistribution->GetClass();
	CDriverAlarm* pAlarm = pDCM->GetAlarm();
	pAlarm->SetDriverPackName("GetCalibrationOptionInfo");
	STS_CALINFO CalibrationInfo;
	int nRetVal = pDCM->GetCalibrationInfo(lpszPinName, usSiteNo, CalibrationInfo);
	if (0 != nRetVal)
	{
		switch (nRetVal)
		{
		case -1:
			nRetVal = VECTOR_FILE_NOT_LOADED;
			break;
		case -2:
			///<The point of pin name is nullptr
		case -3:
			nRetVal = PIN_NAME_ERROR;
			break;
		case -4:
		case -5:
			nRetVal = SITE_ERROR;
			break;
		case -6:
			///<The pin is not belongs to the instance
			nRetVal = PIN_NOT_BELONGS;
			break;
		case -7:
			nRetVal = BOARD_NOT_INSERT_ERROR;
			break;
		case -8:
		case -9:
			nRetVal = CHANNEL_NOT_EXISTED;
			break;
		case -10:
			///<Get calibration information fail
			nRetVal = GET_CALIBRATION_FAIL;
			break;
		default:
			break;
		}

		pAlarm->Output();
		return nRetVal;
	}
	const char* lpszPoint = nullptr;
	int nStoreSize = 0;
	switch (SelInfo)
	{
	case DCMINFO::METER_SN:
		nStoreSize = CalibrationInfo.meterSnSize;
		lpszPoint = CalibrationInfo.meterSn;
		break;
	case DCMINFO::CALBD_SN:
		nStoreSize = CalibrationInfo.calBdSnSize;
		lpszPoint = CalibrationInfo.calBdSn;
		break;
	case DCMINFO::CALBD_HD_REV:
		nStoreSize = CalibrationInfo.calBdHardRevSize;
		lpszPoint = CalibrationInfo.calBdHardRev;
		break;
	case DCMINFO::SOTEWARE_REV:
		nStoreSize = CalibrationInfo.softwareRevSize;
		lpszPoint = CalibrationInfo.softwareRev;
		break;
	case DCMINFO::ATE_SN:
		nStoreSize = CalibrationInfo.ATESnSize;
		lpszPoint = CalibrationInfo.ATESn;
		break;
	case DCMINFO::BRIDGEBD_SN:
		nStoreSize = CalibrationInfo.bridgeBdSnSize;
		lpszPoint = CalibrationInfo.bridgeBdSn;
		break;
	default:
		pAlarm->Output();
		return -3;
		break;
	}

	if (nullptr == lpszInfo || 0 >= nInfoSize)
	{
		pAlarm->ParameternullptrAlarm("lpszInfo", usSiteNo, lpszPinName, TRUE);
		nRetVal = PARAM_NULLPTR;
	}
	else if (0 < nStoreSize)
	{
		if (nInfoSize - 1 > nStoreSize)
		{
			nInfoSize = nStoreSize + 1;
		}
		memcpy_s(lpszInfo, nInfoSize - 1, lpszPoint, nInfoSize - 1);
		lpszInfo[nInfoSize - 1] = 0;
	}
	pAlarm->Output();
	return 0;
}

double APIENTRY dcm_GetCalibrationSlotIDByPin(const char* lpszPinName, USHORT usSiteNo)
{
	CDCM* pDCM = g_pDCMDistribution->GetClass();
	CDriverAlarm* pAlarm = pDCM->GetAlarm();
	pAlarm->SetDriverPackName("GetCalibrationSlotID");
	STS_CALINFO CalibrationInfo;
	int nRetVal = pDCM->GetCalibrationInfo(lpszPinName, usSiteNo, CalibrationInfo);
	if (0 != nRetVal)
	{
		switch (nRetVal)
		{
		case -1:
			nRetVal = VECTOR_FILE_NOT_LOADED;
			break;
		case -2:
			///<The point of pin name is nullptr
		case -3:
			nRetVal = PIN_NAME_ERROR;
			break;
		case -4:
		case -5:
			nRetVal = SITE_ERROR;
			break;
		case -6:
			///<The pin is not belongs to the instance
			nRetVal = PIN_NOT_BELONGS;
			break;
		case -7:
			nRetVal = BOARD_NOT_INSERT_ERROR;
			break;
		case -8:
		case -9:
			nRetVal = CHANNEL_NOT_EXISTED;
			break;
		case -10:
			///<Get calibration information fail
			nRetVal = GET_CALIBRATION_FAIL;
			break;
		default:
			break;
		}
	}
	pAlarm->Output();
	return CalibrationInfo.calSlotID;
}

BYTE APIENTRY dcm_GetLogicRevsion(BYTE bySlotNo)
{
	CDCM* pDCM = g_pDCMDistribution->GetClass();
	if (nullptr == pDCM)
	{
		return -1;
	}
	return (pDCM->GetFPGARevision(bySlotNo) & 0xFF);
}

int APIENTRY dcm_set_pinload(const char* lpszPinName, USHORT usSiteNo, BYTE byMode, double dIOH, double dIOL, double dVT, double dClampH, double dClampL)
{
	CDCM* pDCM = g_pDCMDistribution->GetClass();
	CDriverAlarm* pAlarm = pDCM->GetAlarm();
	pAlarm->SetDriverPackName("SetDynamicLoad");
	BOOL bEnable = FALSE;
	if (1 == byMode || 3 == byMode)
	{
		bEnable = TRUE;
	}
	int nRetVal = pDCM->SetPinDynamicLoad(lpszPinName, usSiteNo, bEnable, dIOH, dIOL, dVT);
	if (0 != nRetVal)
	{
		switch (nRetVal)
		{
		case -1:
			nRetVal = VECTOR_FILE_NOT_LOADED;
			break;
		case -2:
			///<The point of pin name is nullptr
		case -3:
			nRetVal = PIN_NAME_ERROR;
			break;
		case -4:
			nRetVal = SITE_ERROR;
			break;
		case -5:
			nRetVal = SITE_INVALID;
			break;
		case -6:
			///<Get the calibraiton information fail
			nRetVal = PIN_NOT_BELONGS;
			break;
		case -7:
			nRetVal = BOARD_NOT_INSERT_ERROR;
			break;
		case -8:
			///<The channel number is over range
			nRetVal = CHANNEL_NOT_EXISTED;
			break;
		case -9:
			nRetVal = PIN_CURRENT_ERROR;
			break;
		case -10:
			nRetVal = PIN_LEVEL_ERROR;
			break;
		case -11:
			nRetVal = CLAMP_VALUE_ERROR;
			break;
		default:
			break;
		}
	}
	pAlarm->Output();
	return nRetVal;
}


int APIENTRY dcm_SetDynamicLoad(const char* lpszPinGroup, BYTE bEnable, double dIOH, double dIOL, double dVTVoltValue)
{
	CDCM* pDCM = g_pDCMDistribution->GetClass();
	CDriverAlarm* pAlarm = pDCM->GetAlarm();
	pAlarm->SetDriverPackName("SetDynamicLoad");
	int nRetVal = pDCM->SetDynamicLoad(lpszPinGroup, bEnable, dIOH, dIOL, dVTVoltValue);
	if (0 != nRetVal)
	{
		switch (nRetVal)
		{
		case -1:
			nRetVal = VECTOR_FILE_NOT_LOADED;
			break;
		case -2:
			///<The point of pin name is nullptr
		case -3:
			nRetVal = PIN_GROUP_ERROR;
			break;
		case -4:
			///<No valid site
			nRetVal = SITE_INVALID;
			break;
		case -5:
			///<The pin is not belongs to the instance
			nRetVal = PIN_NOT_BELONGS;
			break;
		case -6:
			nRetVal = CHANNEL_NOT_EXISTED;
			break;
		case -7:
			nRetVal = PIN_CURRENT_ERROR;
			break;
		case -8:
			nRetVal = PIN_LEVEL_ERROR;
			break;
		case -9:
			///<Not will happen
			break;
		case -10:
			nRetVal = BOARD_NOT_INSERT_ERROR;
			break;
		default:
			break;
		}
	}
	pAlarm->Output();
	return nRetVal;
}

USHORT APIENTRY dcm_GetSiteAndPinCount(int& nPinCount)
{
	CDCM* pDCM = g_pDCMDistribution->GetClass();
	int nRetVal = pDCM->GetPinCount();
	if (0 > nRetVal)
	{
		return VECTOR_FILE_NOT_LOADED;
	}
	nPinCount = nRetVal;
	return pDCM->GetSiteCount();
}

int APIENTRY dcm_GetSiteChannel(USHORT usSiteNo, BYTE* pbySlotNo, USHORT* pusChannelNo, int nElementCount)
{
	vector<CHANNEL_INFO> vecChannel;
	CDCM* pDCM = g_pDCMDistribution->GetClass();
	if (nullptr == pDCM)
	{
		return -1;
	}
	int nRetVal = pDCM->GetSiteChannel(usSiteNo, vecChannel);
	if (0 != nRetVal)
	{
		switch (nRetVal)
		{
		case -1:
			nRetVal = VECTOR_FILE_NOT_LOADED;
			break;
		case -2:
			nRetVal = SITE_ERROR;
			break;
		default:
			break;
		}
		return nRetVal;
	}
	USHORT usChannelCount = vecChannel.size();
	if (nullptr == pbySlotNo || nullptr == pusChannelNo || usChannelCount > nElementCount)
	{
		return usChannelCount;
	}
	for (USHORT usChannel = 0; usChannel < usChannelCount;++usChannel)
	{
		pbySlotNo[usChannel] = vecChannel[usChannel].m_bySlotNo;
		pusChannelNo[usChannel] = vecChannel[usChannel].m_usChannel;
	}
	return usChannelCount;
}

int APIENTRY dcm_SetTimesetDelay(BYTE bySlotNo, BYTE byControllerIndex, double dDelay)
{
	int nRetVal = 0;
	CDCM* pDCM = g_pDCMDistribution->GetClass();
	if (nullptr == pDCM)
	{
		return -1;
	}
	nRetVal = pDCM->SetTimesetDelay(bySlotNo, byControllerIndex, dDelay);
	if (0 != nRetVal)
	{
		switch (nRetVal)
		{
		case -1:
			///<The board is not existed
			nRetVal = -1;
			break;
		case -2:
			///<The controller index is over range
			nRetVal = -2;
			break;
		case -3:
			///<The controller is not existed
			nRetVal = -3;
			break;
		case -4:
			///<The timeset delay is over range
			nRetVal = -4;
			break;
		default:
			break;
		}
	}
	return nRetVal;
}

double APIENTRY dcm_GetTimestDelay(BYTE bySlotNo, BYTE byControllerIndex)
{
	CDCM* pDCM = g_pDCMDistribution->GetClass();
	if (nullptr == pDCM)
	{
		return -1;
	}
	return pDCM->GetTimesetDelay(bySlotNo, byControllerIndex);
}

int APIENTRY dcm_SetTotalStartDelay(BYTE bySlotNo, BYTE byControllerIndex, double dDelay)
{
	int nRetVal = 0;
	CDCM* pDCM = g_pDCMDistribution->GetClass();
	if (nullptr == pDCM)
	{
		return -1;
	}
	nRetVal = pDCM->SetTotalStartDelay(bySlotNo, byControllerIndex, dDelay);
	if (0 != nRetVal)
	{
		switch (nRetVal)
		{
		case -1:
			///<Board is not existed
			nRetVal = -1;
			break;
		case -2:
			///<The controller is over range
			nRetVal = -2;
			break;
		case -3:
			///<The controller is not existed
			nRetVal = -3;
			break;
		case -4:
			///<The delay is over range
			nRetVal = -4;
			break;
		default:
			break;
		}
	}
	return nRetVal;
}

double APIENTRY dcm_GetTotalStartDelay(BYTE bySlotNo, BYTE byControllerIndex)
{
	CDCM* pDCM = g_pDCMDistribution->GetClass();
	if (nullptr == pDCM)
	{
		return -1;
	}
	return pDCM->GetTotalStartDelay(bySlotNo, byControllerIndex);
}

int APIENTRY dcm_SetIODelay(BYTE bySlotNo, USHORT usChannel, double* pdDelay, int nElementCount)
{
	if (nullptr == pdDelay || 4 > nElementCount)
	{
		return -1;
	}
	CDCM* pDCM = g_pDCMDistribution->GetClass();
	if (nullptr == pDCM)
	{
		return -1;
	}
	int nRetVal = pDCM->SetIODelay(bySlotNo, usChannel, pdDelay[0], pdDelay[1], pdDelay[2], pdDelay[3]);
	if (0 != nRetVal)
	{
		switch (nRetVal)
		{
		case -1:
			///<Board not existed
			nRetVal = -2;
			break;
		case -2:
			///<The channel is over range
			nRetVal = -3;
			break;
		case -3:
			///<The channel is not existed
			nRetVal = -4;
			break;
		case -4:
			///<The delay is over range
			nRetVal = -5;
			break;
		default:
			break;
		}
	}
	return nRetVal;
}

int APIENTRY dcm_GetIODelay(BYTE bySlotNo, USHORT usChannel, double* pdDelay, int nElementCount)
{
	if (nullptr == pdDelay || 4 > nElementCount)
	{
		return -1;
	}
	CDCM* pDCM = g_pDCMDistribution->GetClass();
	if (nullptr == pDCM)
	{
		return -1;
	}
	int nRetVal = pDCM->GetIODelay(bySlotNo, usChannel, &pdDelay[0], &pdDelay[1], &pdDelay[2], &pdDelay[3]);
	if (0 != nRetVal)
	{
		switch (nRetVal)
		{
		case -1:
			///<The board is not existed
			nRetVal = -2;
			break;
		case -2:
			///<The channel is over range
			nRetVal = -3;
			break;
		case -3:
			///<The channel is not existed
			nRetVal = -4;
			break;
		case -4:
			///<The point is nullptr
			nRetVal = -1;
			break;
		default:
			break;
		}
	}
	return nRetVal;
}


int APIENTRY dcm_SaveDelay(BYTE bySlotNo)
{
	int nRetVal = 0;
	CDCM* pDCM = g_pDCMDistribution->GetClass();
	if (nullptr == pDCM)
	{
		return -1;
	}
	nRetVal = pDCM->SaveDelay(bySlotNo);
	if (0 != nRetVal)
	{
		switch (nRetVal)
		{
		case -1:
			///<The board is not existed
			nRetVal = -1;
			break;
		case -2:
			///<Flash error
			nRetVal = -2;
			break;
		case -3:
			///<No valid controller existed
			nRetVal = -3;
		default:
			break;
		}
	}
	return nRetVal;
}

int APIENTRY dcm_calibration(BOOL bEnable, BYTE byBoardNo)
{
	if (0 >= g_byBoardCount)
	{
		return -1;
	}
	CDCM* pDCM = g_pDCMDistribution->GetClass(0);///<First instance have all board instance
	STS_PROGRESS_INFO_FUN StsSetProgressInfo = nullptr;
	STS_PROGRESS_FUN StsSetProgressStep = nullptr;
	HDModule::Instance()->GetProgressFunc(StsSetProgressInfo, StsSetProgressStep);
	pDCM->SetProgressFunc(StsSetProgressInfo, StsSetProgressStep);

	BYTE byStartBoardNo = 0;
	int nBoardIndex = 0;
	BYTE byEndBoardNo = g_byBoardCount - 1;
	if (0xFF != byBoardNo)
	{
		byStartBoardNo = byBoardNo;
		byEndBoardNo = byBoardNo;
	}

	int nStepCount = (byEndBoardNo - byStartBoardNo + 1) * DCM_MAX_CONTROLLERS_PRE_BOARD;
	int nStepIndex = 0;
	if (nullptr != StsSetProgressInfo && nullptr != StsSetProgressStep)
	{
		StsSetProgressInfo("Read DCM Calibration Data", nStepCount);
	}

	for (int nBoardIndex = byStartBoardNo; nBoardIndex <= byEndBoardNo; ++nBoardIndex)
	{
		BYTE bySlotNo = g_abyBoardSlot[nBoardIndex];
		pDCM->GetChannelCount(bySlotNo, TRUE);
		for (BYTE byControllerIndex = 0; byControllerIndex < DCM_MAX_CONTROLLERS_PRE_BOARD; ++byControllerIndex)
		{
			if (!bEnable)
			{
				pDCM->ResetCalibrationData(bySlotNo, byControllerIndex);
			}
			else
			{
				pDCM->ReadCalibrationData(bySlotNo, byControllerIndex);
			}
			if (nullptr != StsSetProgressInfo && nullptr != StsSetProgressStep)
			{
				StsSetProgressStep(++nStepIndex);
			}
		}
	}

	if (nullptr != StsSetProgressInfo && nullptr != StsSetProgressStep)
	{
		StsSetProgressStep(nStepCount);
	}

	return 0;
}

int APIENTRY dcm_GetPinNo(const char* lpszPinName)
{
	vector<USHORT> vecPinNo;
	CDCM* pDCM = g_pDCMDistribution->GetClass();
	if (nullptr == pDCM)
	{
		return -1;
	}
	int nRetVal = pDCM->GetPinNo(lpszPinName, vecPinNo);
	if (0 != nRetVal)
	{
		return nRetVal;
	}
	return vecPinNo[0];
}

int APIENTRY dcm_GetInstruction(BYTE bySlotNo, BYTE byController, UINT uBRAMLineNo, char* lpszInstruction, int nBuffSize)
{
	CDCM* pDCM = g_pDCMDistribution->GetClass();
	if (nullptr == pDCM)
	{
		return -1;
	}
	return pDCM->GetInstruction(bySlotNo, byController, uBRAMLineNo, lpszInstruction, nBuffSize);
}

int APIENTRY dcm_GetOperand(BYTE bySlotNo, BYTE byController, UINT uBRAMLineNo)
{
	CDCM* pDCM = g_pDCMDistribution->GetClass();
	if (nullptr == pDCM)
	{
		return -1;
	}
	return pDCM->GetOperand(bySlotNo, byController, uBRAMLineNo);
}

int APIENTRY dcm_SetTriggerOut(const char* lpszPinName, USHORT usSiteNo)
{
	CDCM* pDCM = g_pDCMDistribution->GetClass();
	CDriverAlarm* pAlarm = pDCM->GetAlarm();
	pAlarm->SetDriverPackName("SetTriggerOut");
	int nRetVal = pDCM->SetTriggerOut(lpszPinName, usSiteNo);
	if (0 != nRetVal)
	{
		switch (nRetVal)
		{
		case -1:
			nRetVal = VECTOR_FILE_NOT_LOADED;
			break;
		case -2:
			///<The point of pin name is nullptr
		case -3:
			nRetVal = PIN_NAME_ERROR;
			break;
		case -4:
			nRetVal = SITE_ERROR;
			break;
		case -5:
			nRetVal = SITE_INVALID;
			break;
		case -6:
			///<The pin is not belongs to the instance
			nRetVal = PIN_NOT_BELONGS;
			break;
		case -7:
			nRetVal = BOARD_NOT_INSERT_ERROR;
			break;
		case -8:
			nRetVal = CHANNEL_NOT_EXISTED;
			break;
		default:
			break;
		}
	}
	pAlarm->Output();
	return nRetVal;
}

int APIENTRY dcm_I2CSetPinPinLevel(double dVIH, double dVIL, double dVOH, double dVOL, BYTE byChannelType)
{
	CDriverAlarm* pAlarm = nullptr;
	CI2C* pI2C = GetI2C(pAlarm, "I2CSetPinLevel");
	if (nullptr == pI2C)
	{
		return -2;
	}
	int nRetVal = pI2C->SetPinLevel(dVIH, dVIL, dVOH, dVOL, byChannelType);
	if (0 != nRetVal)
	{
		switch (nRetVal)
		{
		case -1:
			///<Not set channel information
			nRetVal = -2;
			break;
		case -2:
			///<The pin level error
			nRetVal = -1;
			break;
		default:
			break;
		}
	}
	pAlarm->Output();
	return nRetVal;
}

void APIENTRY dcm_I2CSetStopStatus(BOOL bHighImpedance)
{
	CDriverAlarm* pAlarm = nullptr;
	CI2C* pI2C = GetI2C(pAlarm, "I2CSetStopStatus");
	if (nullptr == pI2C)
	{
		return;
	}
	pI2C->SetStopStatus(bHighImpedance);
	pAlarm->Output();
}

int APIENTRY dcm_I2CSetDynamicLoad(BYTE byChannelType, BYTE byEnable, double dIOH, double dIOL, double dVTVoltValue)
{
	CDriverAlarm* pAlarm = nullptr;
	CI2C* pI2C = GetI2C(pAlarm, "I2CSetDynamicLoad");
	if (nullptr == pI2C)
	{
		return -1;
	}
	int nRetVal = pI2C->SetDynamicLoad(byChannelType, byEnable, dIOH, dIOL, dVTVoltValue);
	pAlarm->Output();
	return nRetVal;
}

void APIENTRY dcm_SetInstanceID(int nInstanceID)
{
	g_nCurInstanceID = nInstanceID;
	g_pDCMDistribution->SetInstanceID(nInstanceID);
	g_pI2CDistribution->SetInstanceID(nInstanceID);
}

USHORT APIENTRY dcm_GetPinCount()
{
	return CPinNoManage::Instance()->GetPinCount();
}


void GetValidPinConfig(string& strFile)
{
	GetConfigDirectory(strFile);
	strFile += "ValidConfig.ini";
}

int APIENTRY dcm_SetValidPin(const char* lpszPinNameList)
{
	int nInstanceID = 0;
	CDCM* pDCM = g_pDCMDistribution->GetClass(&nInstanceID);
	string strVectorFile;

	if (!pDCM->IsLoadVector(strVectorFile))
	{
		return VECTOR_FILE_NOT_LOADED;
	}
	if (nullptr == lpszPinNameList)
	{
		return PIN_NAME_ERROR;
	}

	if (0 == CVectorFileRecord::Instance()->IsMonopolizeFile(nInstanceID))
	{
		///<Not set vector file owned if the instance monopolize vector file
		return 0;
	}
	CDriverAlarm* pDriverAlarm = pDCM->GetAlarm();
	pDriverAlarm->SetDriverPackName("SetPinValid");

	int nRetVal = pDCM->SetValidPin(lpszPinNameList);
	if (0 != nRetVal)
	{
		switch (nRetVal)
		{
		case -1:
			nRetVal = VECTOR_FILE_NOT_LOADED;
			break;
		case -2:
		case -3:
		case -4:
		case -5:
			pDriverAlarm->SetParamName("lpszPinNameList");
			nRetVal = PIN_NAME_ERROR;
		default:
			break;
		}
	}
	else
	{
		CPinNoManage::Instance()->SetPinCount(nInstanceID, pDCM->GetPinCount(), TRUE);
		string strConfigFile;
		GetValidPinConfig(strConfigFile);
		char lpszKey[32] = { 0 };
		_itoa_s(nInstanceID, lpszKey, sizeof(lpszKey), 10);
		WritePrivateProfileString("Config", lpszKey, lpszPinNameList, strConfigFile.c_str());
	}
	pDriverAlarm->Output();

	return nRetVal;
}

int APIENTRY dcm_EnableAddPin(BOOL bEnable, BOOL bClearVector)
{
	return g_pDCMDistribution->GetClass(nullptr, 0)->EnableAddPin(bEnable, bClearVector);
}

int APIENTRY dcm_AddPin(const char* lpszPinName, USHORT usPinNo, const char* lpszChannel)
{
	CDCM* pDCM = g_pDCMDistribution->GetClass(nullptr, 0);
	int nRetVal = pDCM->AddPin(lpszPinName, usPinNo, lpszChannel);
	if (0 != nRetVal)
	{
		return nRetVal;
	}
	CPinNoManage::Instance()->SetPinCount(0, pDCM->GetPinCount(), TRUE);

	return 0;
}

void APIENTRY dcm_ClearePreadVector()
{
	CDCM* pDCM = g_pDCMDistribution->GetClass(nullptr, 0);
	if (nullptr != pDCM)
	{
		pDCM->ClearPreread();
	}
}

int APIENTRY dcm_getPinNumAndsiteNum(USHORT& usPinCount, USHORT& usSiteCount)
{
	usPinCount = 0;
	usSiteCount = 0;
	CPinNoManage* pPinNoManage = CPinNoManage::Instance();
	if (g_bAllowDebugToolShow && 0 == pPinNoManage->GetPinCount())
	{
		///<Get the pin information of DCM with vector file
		g_mapPinGroup.clear();

		for (int nInstanceID = 0; nInstanceID < g_nInstanceCount; ++nInstanceID)
		{
			g_pDCMDistribution->Initialize(nInstanceID, FALSE);
		}

		vector<int> vecInstanceID;
		CDCM* pDCM = nullptr;

		AddBoard();
		string strConfig;
		GetConfigFile(strConfig);
		const char* lpszConfig = strConfig.c_str();
		char lpszKey[32] = { 0 };
		vector<string> vecPinGroup;
		char lpszData[128] = { 0 };
		map<int, int> mapSharedVector;
		g_pDCMDistribution->GetInstanceValid(vecInstanceID);
		for (auto InstanceID : vecInstanceID)
		{
			pDCM = g_pDCMDistribution->GetClass(nullptr, InstanceID);
			sprintf_s(lpszKey, sizeof(lpszKey), "%d", InstanceID);

			int nUseVectorShared = GetPrivateProfileInt("UseInfo", lpszKey, -1, strConfig.c_str());
			if (-1 != nUseVectorShared)
			{
				mapSharedVector.insert(make_pair(InstanceID, nUseVectorShared));
				continue;
			}
			GetPrivateProfileString("Vector", lpszKey, "", lpszData, sizeof(lpszData), strConfig.c_str());
			int nRetVal = pDCM->LoadVectorInfo(lpszData);
			if (0 != nRetVal)
			{
				return -1;
			}
			string strConfigFile;
			GetValidPinConfig(strConfigFile);
			_itoa_s(InstanceID, lpszKey, sizeof(lpszKey), 10);
			char lpszPinNameList[128] = { 0 };
			GetPrivateProfileString("Config", lpszKey, "", lpszPinNameList, sizeof(lpszPinNameList), strConfigFile.c_str());
			if (0 != strlen(lpszPinNameList))
			{
				pDCM->SetValidPin(lpszPinNameList);
			}

			pDCM->GetPinGroup(vecPinGroup);
			for (auto& PinGroup : vecPinGroup)
			{
				g_mapPinGroup.insert(make_pair(PinGroup, InstanceID));
			}

			pPinNoManage->SetPinCount(InstanceID, pDCM->GetPinCount(), TRUE);
		}
		CDCM* pSourceDCM = nullptr;
		for (auto& Instance : mapSharedVector)
		{
			pDCM = g_pDCMDistribution->GetClass(nullptr, Instance.first);
			pSourceDCM = g_pDCMDistribution->GetClass(nullptr, Instance.second);

			pDCM->CopyVectorInfo(*pSourceDCM);
			pDCM->LoadPinGroupInfo();

			pDCM->GetPinGroup(vecPinGroup);
			for (auto& PinGroup : vecPinGroup)
			{
				g_mapPinGroup.insert(make_pair(PinGroup, Instance.first));
			}
		}
		///<Get the information of I2C
		CI2C* pI2C = nullptr;
		string strI2CConfig;
		GetI2CConfig(strI2CConfig);
		lpszConfig = strI2CConfig.c_str();
		int nInstanceCount = GetPrivateProfileInt("Instance", "Count", 0, lpszConfig);
		if (0 < nInstanceCount)
		{
			char lpszSection[32] = { 0 };
			char lpszKey[32] = { 0 };
			char lpszValue[32] = { 0 };
			BOOL bInstanceFail = FALSE;
			vector<CHANNEL_INFO> vecSCLChannel;
			vector<CHANNEL_INFO> vecSDAChannel;
			CHANNEL_INFO SCLChannel;
			CHANNEL_INFO SDAChannel;
			for (int nInstance = 0; nInstance < nInstanceCount;++nInstance)
			{
				BOOL bInstanceFail = FALSE;
				_itoa_s(nInstance, lpszSection, sizeof(lpszSection), 10);
				int nSiteCount = GetPrivateProfileInt(lpszSection, "SiteCount", -1, lpszConfig);
				if (0 >= nSiteCount)
				{
					continue;
				}
				int nByteCount = GetPrivateProfileInt(lpszSection, "RegisterByteCount", 1, lpszConfig);
				double dPeriod = GetPrivateProfileInt(lpszSection, "Period", 0, lpszConfig);
				vecSDAChannel.clear();
				vecSCLChannel.clear();
				for (USHORT usSiteNo = 0; usSiteNo < nSiteCount;++usSiteNo)
				{
					sprintf_s(lpszKey, sizeof(lpszKey), "SITE_%d", usSiteNo + 1);
					GetPrivateProfileString(lpszSection, lpszKey, "", lpszValue, sizeof(lpszValue), lpszConfig);
					string strChannel = lpszValue;
					int nRetVal = sscanf_s(lpszValue, "S%d_%d,S%d_%d", &SCLChannel.m_bySlotNo, &SCLChannel.m_usChannel, &SDAChannel.m_bySlotNo, &SDAChannel.m_usChannel);
					if (4 != nRetVal)
					{
						vecSDAChannel.clear();
						vecSDAChannel.clear();
						break;
					}
					vecSCLChannel.push_back(SCLChannel);
					vecSDAChannel.push_back(SDAChannel);
				}
				if (0 != vecSDAChannel.size())
				{
					g_pI2CDistribution->Initialize(nInstance);
					pI2C = g_pI2CDistribution->GetClass(nullptr, nInstance);
					pI2C->Set(dPeriod, (CI2C::REG_MODE)nByteCount, vecSCLChannel, vecSDAChannel);

					pPinNoManage->SetPinCount(nInstance, 2, FALSE);
				}
			}
		}
	}
	usPinCount = pPinNoManage->GetPinCount();
	usSiteCount = g_usMaxSiteCount;
	return 0;
}

BOOL APIENTRY dcm_getPinChannel(USHORT usPinNo, USHORT usSiteNo, short& sChannel)
{
	int nInstanceID = 0;
	BOOL bDCMWithVector = TRUE;
	int nOffsetPinNo = CPinNoManage::Instance()->GetPinNoOffset(usPinNo, nInstanceID, bDCMWithVector);
	if (0 > nOffsetPinNo)
	{
		return PIN_NAME_ERROR;
	}
	int nRetVal = 0;
	CHANNEL_INFO Channel;
	if (bDCMWithVector)
	{
		CDCM* pDCM = g_pDCMDistribution->GetClass(nullptr, nInstanceID);
		if (nullptr == pDCM)
		{
			return -1;
		}
		nRetVal = pDCM->GetChannel(nOffsetPinNo, usSiteNo, Channel);
		if (0 != nRetVal)
		{
			switch (nRetVal)
			{
			case -1:
				nRetVal = VECTOR_FILE_NOT_LOADED;
				break;
			case -2:
				nRetVal = PIN_NAME_ERROR;
				break;
			case -3:
				nRetVal = SITE_ERROR;
				break;
			default:
				break;
			}
			return FALSE;
		}
	}
	else
	{
		CI2C* pI2C = g_pI2CDistribution->GetClass(nullptr, nInstanceID);
		if (nullptr == pI2C)
		{
			return -1;
		}
		nRetVal = pI2C->GetChannel(usSiteNo, 0 == nOffsetPinNo, Channel);
		if (0 != nRetVal)
		{
			return FALSE;
		}

	}

	nRetVal = GetGlobalChannel(Channel);
	if (0 > nRetVal)
	{
		return FALSE;
	}
	sChannel = nRetVal;

	return TRUE;
}

int APIENTRY dcm_GetPinSlotChannel(USHORT usPinNo, USHORT usSiteNo, USHORT& usChannel)
{
	int nInstanceID = 0;
	BOOL bDCMWithVector = TRUE;
	int nOffsetPinNo = CPinNoManage::Instance()->GetPinNoOffset(usPinNo, nInstanceID, bDCMWithVector);
	if (0 > nOffsetPinNo)
	{
		return PIN_NAME_ERROR;
	}
	CHANNEL_INFO Channel;
	int nRetVal = 0;
	if (bDCMWithVector)
	{
		CDCM* pDCM = g_pDCMDistribution->GetClass(nullptr, nInstanceID);
		if (nullptr == pDCM)
		{
			return -1;
		}

		nRetVal = pDCM->GetChannel(nOffsetPinNo, usSiteNo, Channel);
		if (0 != nRetVal)
		{
			return nRetVal;
		}
	}
	else
	{
		CI2C* pI2C = g_pI2CDistribution->GetClass(nullptr, nInstanceID);
		if (nullptr == pI2C)
		{
			return -1;
		}
		nRetVal = pI2C->GetChannel(usSiteNo, 0 == nOffsetPinNo, Channel);
		if (0 != nRetVal)
		{
			return SITE_ERROR;
		}
	}
	usChannel = Channel.m_usChannel;
	return Channel.m_bySlotNo;
}

int APIENTRY dcm_getSampleTimes(USHORT usPinNo, USHORT usSiteNo)
{
	int nInstanceID = 0;
	BOOL bDCMWithVector = TRUE;
	int nOffsetPinNo = CPinNoManage::Instance()->GetPinNoOffset(usPinNo, nInstanceID, bDCMWithVector);
	if (0 > nOffsetPinNo)
	{
		return PIN_NAME_ERROR;
	}
	int nRetVal = 0;
	CHANNEL_INFO Channel;
	if (!bDCMWithVector)
	{
		///<I2C function not support 
		CI2C* pI2C = g_pI2CDistribution->GetClass(nullptr, nInstanceID);
		if (nullptr == pI2C)
		{
			return -1;
		}
		CHANNEL_INFO Channel;
		pI2C->GetChannel(usSiteNo, 0 == nOffsetPinNo, Channel);
		return -1;
	}
	else
	{
		CDCM* pDCM = g_pDCMDistribution->GetClass(nullptr, nInstanceID);
		if (nullptr == pDCM)
		{
			return -1;
		}

		string strPinName;
		nRetVal = pDCM->GetPinName(nOffsetPinNo, strPinName);
		if (0 != nRetVal)
		{
			return -2;
		}

		nRetVal = pDCM->GetChannel(strPinName.c_str(), usSiteNo, Channel);
		if (0 != nRetVal)
		{
			return -1;
		}
	}
	BYTE byController = 0;
	CClassifyBoard Classify;
	nRetVal = Classify.GetControllerChannel(Channel.m_usChannel, byController);
	if (0 > nRetVal)
	{
		return -1;
	}
	UINT uSampleCount = 0;
	double dSamplePeriod = 0;
	CPMU::Instance()->GetSampleSetting(Channel.m_bySlotNo, byController, nRetVal, uSampleCount, dSamplePeriod);
	return uSampleCount;
}

int APIENTRY dcm_getSampleInterval(USHORT usPinNo, USHORT usSiteNo)
{
	int nInstanceID = 0;
	BOOL bDCMWithVector = TRUE;
	int nOffsetPinNo = CPinNoManage::Instance()->GetPinNoOffset(usPinNo, nInstanceID, bDCMWithVector);
	if (0 > nOffsetPinNo)
	{
		return PIN_NAME_ERROR;
	}
	int nRetVal = 0;
	CHANNEL_INFO Channel;
	if (!bDCMWithVector)
	{
		///<I2C function not support
		CI2C* pI2C = g_pI2CDistribution->GetClass(nullptr, nInstanceID);
		if (nullptr == pI2C)
		{
			return -1;
		}
		pI2C->GetChannel(usSiteNo, 0 == nOffsetPinNo, Channel);
	}
	else
	{

		CDCM* pDCM = g_pDCMDistribution->GetClass(nullptr, nInstanceID);
		if (nullptr == pDCM)
		{
			return -1;
		}

		string strPinName;
		nRetVal = pDCM->GetPinName(nOffsetPinNo, strPinName);
		if (0 != nRetVal)
		{
			return -2;
		}

		nRetVal = pDCM->GetChannel(strPinName.c_str(), usSiteNo, Channel);
		if (0 != nRetVal)
		{
			return -1;
		}
	}
	BYTE byController = 0;
	CClassifyBoard Classify;
	nRetVal = Classify.GetControllerChannel(Channel.m_usChannel, byController);
	if (0 > nRetVal)
	{
		return -1;
	}
	UINT uSampleCount = 0;
	double dSamplePeriod = 0;
	CPMU::Instance()->GetSampleSetting(Channel.m_bySlotNo, byController, nRetVal, uSampleCount, dSamplePeriod);
	return (int)(dSamplePeriod + EQUAL_ERROR);
}

int APIENTRY dcm_getPpmuMeasStatus(USHORT usPinNo, USHORT usSiteNo, BYTE& byIRange, BYTE& byMeasType)
{
	int nInstanceID = 0;
	BOOL bDCMWithVector = TRUE;
	int nOffsetPinNo = CPinNoManage::Instance()->GetPinNoOffset(usPinNo, nInstanceID, bDCMWithVector);
	if (0 > nOffsetPinNo)
	{
		return PIN_NAME_ERROR;
	}
	int nRetVal = 0;
	CHANNEL_INFO Channel;
	if (!bDCMWithVector)
	{
		///<I2C function not support 
		CI2C* pI2C = g_pI2CDistribution->GetClass(nullptr, nInstanceID);
		if (nullptr == pI2C)
		{
			return -1;
		}
		pI2C->GetChannel(usSiteNo, 0 == nOffsetPinNo, Channel);
	}
	else 
	{
		CDCM* pDCM = g_pDCMDistribution->GetClass(nullptr, nInstanceID);
		if (nullptr == pDCM)
		{
			return -1;
		}
		string strPinName;
		nRetVal = pDCM->GetPinName(nOffsetPinNo, strPinName);
		if (0 != nRetVal)
		{
			return -2;
		}

		nRetVal = pDCM->GetChannel(strPinName.c_str(), usSiteNo, Channel);
		if (0 != nRetVal)
		{
			return -1;
		}
	}
	
	BYTE byController = 0;
	CClassifyBoard Classify;
	nRetVal = Classify.GetControllerChannel(Channel.m_usChannel, byController);
	if (0 > nRetVal)
	{
		return -1;
	}
	UINT uSampleCount = 0;
	double dSamplePeriod = 0;
	unsigned char ucIRange = 0;
	unsigned char ucMeasType = 0;
	unsigned char ucForceMode = 0;
	CPMU::Instance()->GetMeasureMode(Channel.m_bySlotNo, byController, nRetVal, ucMeasType);
	CPMU::Instance()->GetForceMode(Channel.m_bySlotNo, byController, nRetVal, ucForceMode, ucIRange);
	byIRange = ucIRange;
	byMeasType = ucMeasType;
	return 0;
}

const char* APIENTRY dcm_getPinName(USHORT usPinNo)
{
	int nInstanceID = 0;
	BOOL bDCMWithVector = TRUE;
	int nOffsetPinNo = CPinNoManage::Instance()->GetPinNoOffset(usPinNo, nInstanceID, bDCMWithVector);
	if (0 > nOffsetPinNo)
	{
		return nullptr;
	}
	if (bDCMWithVector)
	{
		///<DCM with vector file
		CDCM* pDCM = g_pDCMDistribution->GetClass(nullptr, nInstanceID);
		if (nullptr == pDCM)
		{
			return nullptr;
		}
		int nRetVal = 0;

		nRetVal = pDCM->GetPinName(nOffsetPinNo, g_strPinName);
		if (0 != nRetVal)
		{
			return nullptr;
		}
	}
	else
	{
		///<I2C
		CI2C* pI2C = g_pI2CDistribution->GetClass(nullptr, nInstanceID);
		if (nullptr == pI2C)
		{
			return nullptr;
		}
		char lpszPinName[32] = { 0 };
		if (0 == nOffsetPinNo)
		{
			if (0 == nInstanceID)
			{
				sprintf_s(lpszPinName, sizeof(lpszPinName), "I2C_SCL");
			}
			else
			{
				sprintf_s(lpszPinName, sizeof(lpszPinName), "I2C_SCL%d", nInstanceID);
			}
		}
		else
		{
			if (0 == nInstanceID)
			{
				sprintf_s(lpszPinName, sizeof(lpszPinName), "I2C_SDA");
			}
			else
			{
				sprintf_s(lpszPinName, sizeof(lpszPinName), "I2C_SDA%d", nInstanceID);
			}
		}
		g_strPinName = lpszPinName;
	}
	return g_strPinName.c_str();
}

int APIENTRY dcm_GetPinInstanceID(USHORT usPinNo, BOOL& bDCMWithVector)
{
	int nInstanceID = 0;
	int nOffsetPinNo = CPinNoManage::Instance()->GetPinNoOffset(usPinNo, nInstanceID, bDCMWithVector);
	if (0 > nOffsetPinNo)
	{
		return -1;
	}
	return nInstanceID;
}

int dcm_SetWaveDataParam(const char* lpszPinGroup, const char* lpszStartLabel, ULONG ulOffset, int nWriteVectorLineCount)
{
	CDCM* pDCM = g_pDCMDistribution->GetClass();
	CDriverAlarm* pAlarm = pDCM->GetAlarm();
	pAlarm->SetDriverPackName("SetWaveDataParam");
	int nRetVal = 0;
	nRetVal = pDCM->SetLineInfo(lpszPinGroup, ALL_SITE, lpszStartLabel, ulOffset, nWriteVectorLineCount);
	if (0 != nRetVal)
	{
		switch (nRetVal)
		{
		case -1:
			nRetVal = VECTOR_FILE_NOT_LOADED;
			break;
		case -2:
			///<The point of pin group is nullptr
		case -3:
			nRetVal = PIN_GROUP_ERROR;
			break;
		case -4:
			///<Site error, not will happened
			break;
		case -5:
			///<Site invalid, not will happened
			return SITE_INVALID;
			break;
		case -6:
			///<The pin is not belongs to the instance
			nRetVal = PIN_NOT_BELONGS;
			break;
		case -7:
			///<The point of start label is nullptr
		case -8:
			nRetVal = START_LABEL_ERROR;
			break;
		case -9:
			nRetVal = OFFSET_ERROR;
			break;
		case -10:
			nRetVal = VECTOR_LINE_OVER_RANGE;
			break;
		case -11:
			nRetVal = ALLOCATE_MEMORY_FAIL;
			break;
		case -12:
			nRetVal = CHANNEL_NOT_EXISTED;
			break;
		case -13:
			nRetVal = BOARD_NOT_INSERT_ERROR;
			break;
		default:
			break;
		}
	}
	pAlarm->Output();
	return nRetVal;
}

int dcm_SetSiteWaveData(USHORT usSiteNo, BYTE* pbyWaveData)
{
	CDCM* pDCM = g_pDCMDistribution->GetClass();
	CDriverAlarm* pAlarm = pDCM->GetAlarm();
	pAlarm->SetDriverPackName("SetSiteWaveData");
	int nRetVal = 0;
	nRetVal = pDCM->SetSiteWaveData(usSiteNo, pbyWaveData);	
	if (0 != nRetVal)
	{
		switch (nRetVal)
		{
		case -1:
			nRetVal = VECTOR_FILE_NOT_LOADED;
			break;
		case -2:
			nRetVal = FUNCTION_USE_ERROR;
			break;
		case -3:
			nRetVal = SITE_ERROR;
			break;
		case -4:
			nRetVal = SITE_INVALID;
			break;
		case -5:
			nRetVal = PARAM_NULLPTR;
		case -6:
			nRetVal = ALLOCATE_MEMORY_FAIL;
			break;
		case -7:
			nRetVal = BOARD_NOT_INSERT_ERROR;
			break;
		default:
			break;
		}
	}
	pAlarm->Output();
	return nRetVal;
}

int dcm_StartWriteData()
{
	CDCM* pDCM = g_pDCMDistribution->GetClass();
	CDriverAlarm* pAlarm = pDCM->GetAlarm();
	pAlarm->SetDriverPackName("SetSiteWaveData");
	int nRetVal = 0;
	nRetVal = pDCM->WriteData();
	if (0 != nRetVal)
	{
		switch (nRetVal)
		{
		case -1:
			nRetVal = VECTOR_FILE_NOT_LOADED;
			break;
		case -2:
			nRetVal = FUNCTION_USE_ERROR;
			break;
		case -3:
			///<Site error, Not will happen
			nRetVal = SITE_ERROR;
			break;
		case -4:
			///<Site invalid, not will happen
			nRetVal = SITE_INVALID;
			break;
		case -5:
			nRetVal = BOARD_NOT_INSERT_ERROR;
			break;
		default:
			break;
		}
		
	}
	return nRetVal;
}

int APIENTRY dcm_GetHardInfo(BYTE bySlotNo, STS_HARDINFO* pHardInfo, int nElementCount, int& nModuleNum)
{
	CDCM* pDCM = g_pDCMDistribution->GetClass();
	CDriverAlarm* pAlarm = pDCM->GetAlarm();
	pAlarm->SetDriverPackName("GetHardInfo");
	int nRetVal = 0;
	nRetVal = pDCM->GetHardInfo(bySlotNo, pHardInfo, nElementCount);
	if (0 > nRetVal)
	{
		nRetVal = BOARD_NOT_INSERT_ERROR;
	}
	else
	{
		nModuleNum = nRetVal;
		nRetVal = 0;
	}

	return nRetVal;
}

int APIENTRY dcm_SetFailSavingType(const char* lpszPinGroup, int nSavingType)
{
	CDCM* pDCM = g_pDCMDistribution->GetClass();
	CDriverAlarm* pAlarm = pDCM->GetAlarm();
	pAlarm->SetDriverPackName("SetFailSavingType");
	int nRetVal = 0;
	nRetVal = pDCM->EnableSaveSelectedFail(lpszPinGroup, 0 == nSavingType ? FALSE : TRUE);
	if (0 != nRetVal)
	{
		switch (nRetVal)
		{
		case -1:
			///<Not load vector file
			nRetVal = VECTOR_FILE_NOT_LOADED;
			break;
		case -2:
			///<The pin group is nullptr
		case -3:
			///<The pin group is not defined
			nRetVal = PIN_GROUP_ERROR;
			break;
		case -4:
			///<No valid site
			nRetVal = SITE_INVALID;
			break;
		case -5:
			///<No board existed
			nRetVal = BOARD_NOT_INSERT_ERROR;
			break;
		default:
			break;
		}
	}
	pAlarm->Output();
	return nRetVal;
}

/**
 * @brief Get the serial number of the board
 * @param[in] bySlotNo The slot number
 * @param[out] lpszSN The serial number
 * @param[in] nBuffSize The buff size
 * @return Execute result
 * - 0 Get the serial number successfully
 * - -1 The point of serial number is nullptr
*/
int GetSN(BYTE bySlotNo, char* lpszSN, int nBuffSize)
{
	STS_HARDINFO HardInfo;
	int nRetVal = g_pDCMDistribution->GetClass(nullptr, 0)->GetHardInfo(bySlotNo, &HardInfo, 1);
	if (0 > nRetVal)
	{
		nRetVal = BOARD_NOT_INSERT_ERROR;
	}
	strcpy_s(lpszSN, nBuffSize, HardInfo.moduleInfo.moduleSN);
	return 0;
}

void APIENTRY dcm_I2CEnableUncompare(BOOL bEnable)
{
	CDriverAlarm* pAlarm = nullptr;
	CI2C* pI2C = GetI2C(pAlarm, "I2CGetReadData");
	if (nullptr == pI2C)
	{
		return;
	}
	pI2C->EnableCopmareShield(bEnable);
}
