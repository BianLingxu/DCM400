#include "DCM.h"
#include <fstream>
#include "AccoTESTGlobal.h"
#include "Sts8100.h"
#include "ACVFailMapHeaderInfo.h"
#include "RunAuthorization.h"
#include "ConfigFile.h"
#ifdef RECORD_TIME
#include "Timer.h"
#endif // RECORD_TIME

using namespace std;

#define BOARD_CHANNEL_BEGIN(vecChannel) \
{\
set<BYTE> setBoard;\
m_ClassifyBoard.GetBoard(setBoard);\
for(auto Slot : setBoard){\
	auto iterBoard = m_mapBoard.find(Slot);\
	if (m_mapBoard.end() != iterBoard){\
	m_ClassifyBoard.GetBoardChannel(Slot, vecChannel);

#define BOARD_CHANNEL_END }\
}\
}

CDCM::CDCM(CDriverAlarm* pDriverAlarm)
	: m_bVectorShared(FALSE)
	, m_bDeleteChannelUnused(FALSE)
	, m_bAllowAddPin(FALSE)
{
	m_bLoadVector = FALSE;
	m_nBRAMLeftStartLine = 0;
	m_nDRAMLeftStartLine = 0;
	m_nLatestStartLine = 0;
	m_nLatestStopLine = 0;
	m_usDataSiteNo = ALL_SITE;
	memset(m_nLineCount, 0, sizeof(m_nLineCount));
	m_nCaptureStartOffset = -1;
	m_nCaptureLineCount = 0;
	m_bWaitRun = FALSE;
	m_pAlarm = pDriverAlarm;
	m_pProgressStep = nullptr;
	m_pProgressInfo = nullptr;
	m_bVectorBind = FALSE;
}

CDCM::~CDCM()
{
	Reset();
}

CDriverAlarm* CDCM::GetAlarm()
{
	return m_pAlarm;
}

int CDCM::Connect(const char* lpszPinGroup, BOOL bConenct)
{
	if (!m_bLoadVector)
	{
		m_pAlarm->VectorNotLoadedAlarm();
		return -1;
	}

	int nRetVal = GetChannel(lpszPinGroup);
	if (0 != nRetVal)
	{
		switch (nRetVal)
		{
		case -2:
			//The pin group is nullptr
			nRetVal = -2;
			break;
		case -3:
			//The pin group is not defined before
			nRetVal = - 3;
			break;
		case -5:
			///<No valid site
			nRetVal = -4;
			break;
		case -6:
			///<The pin is not belongs to
			nRetVal = -5;
			break;
		default:
			///<Not will happen
			break;
		}
		return nRetVal;
	}
	vector<USHORT> vecChannel;
	BOOL bNoBoard = TRUE;

	BOARD_CHANNEL_BEGIN(vecChannel)
	{
		iterBoard->second->Connect(vecChannel, bConenct);
		bNoBoard = FALSE;
	}
	BOARD_CHANNEL_END

	if (bNoBoard)
	{
		m_pAlarm->SetNoBoardAlarm(lpszPinGroup, FALSE);
		nRetVal = -6;
	}

	return nRetVal;
}

int CDCM::GetConnectChannel(BYTE bySlotNo, std::vector<USHORT>& vecChannel, RELAY_TYPE RelayType)
{
	auto iterBoard = m_mapBoard.find(bySlotNo);
	if (m_mapBoard.end() == iterBoard || nullptr == iterBoard->second)
	{
		return -1;
	}
	int nRetVal = iterBoard->second->GetConnectChannel(vecChannel);
	if (0 != nRetVal)
	{
		return -2;
	}
	return 0;
}

int CDCM::IntializeFunctionRelay(BYTE bySlotNo)
{
	auto iterBoard = m_mapBoard.find(bySlotNo);
	if (m_mapBoard.end() == iterBoard || nullptr == iterBoard->second)
	{
		return -1;
	}
	vector<USHORT> vecChannel;
	for (USHORT usChannel = 0; usChannel < DCM_MAX_CHANNELS_PER_BOARD; ++usChannel)
	{
		vecChannel.push_back(usChannel);
	}
	iterBoard->second->Connect(vecChannel, FALSE);
	return 0;
}

int CDCM::AddBoard(BYTE bySlotNo, BOOL bOnlyAddBoard)
{
	auto iterSlot = m_mapBoard.find(bySlotNo);
	if (m_mapBoard.end() != iterSlot)
	{
		return -1;
	}
	CBoard* pBoard = new CBoard(bySlotNo, m_pAlarm);
	m_mapBoard.insert(make_pair(bySlotNo, pBoard));
	if (!bOnlyAddBoard)
	{
		pBoard->EnableAllPMUClampFlag();
	}
	return 0;
}

int CDCM::GetBoardcount()
{
	return m_mapBoard.size();
}

USHORT CDCM::GetFPGARevision(BYTE bySlotNo, BYTE byControllerIndex)
{
	auto iterBoard = m_mapBoard.find(bySlotNo);
	if (m_mapBoard.end() == iterBoard || nullptr == iterBoard->second)
	{
		return 0xFFFF;
	}
	return iterBoard->second->GetFPGARevision(byControllerIndex);
}

USHORT CDCM::GetFPGARevision(BYTE bySlotNo)
{
	auto iterBoard = m_mapBoard.find(bySlotNo);
	if (m_mapBoard.end() == iterBoard || nullptr == iterBoard->second)
	{
		return 0xFFFF;
	}

	return iterBoard->second->GetFPGARevision();
}

int CDCM::SetTimesetDelay(BYTE bySlotNo, BYTE byControllerIndex, double dDelay)
{
	auto iterBoard = m_mapBoard.find(bySlotNo);
	if (m_mapBoard.end() == iterBoard || nullptr == iterBoard->second)
	{
		return -1;
	}
	int nRetVal = iterBoard->second->SetTimesetDelay(byControllerIndex, dDelay);
	if (0 != nRetVal)
	{
		switch (nRetVal)
		{
		case -1:
			///<The controller index is over range
			nRetVal = -2;
			break;
		case -2:
			///<The controller is not existed
			nRetVal = -3;
			break;
		case -3:
			///<The delay value is over range
			nRetVal = -4;
			break;
		default:
			break;
		}
	}
	return nRetVal;
}

double CDCM::GetTimesetDelay(BYTE bySlotNo, BYTE byControllerIndex)
{
	auto iterBoard = m_mapBoard.find(bySlotNo);
	if (m_mapBoard.end() == iterBoard || nullptr == iterBoard->second)
	{
		return 0x7FFFFFFF;
	}
	return iterBoard->second->GetTimesetDelay(byControllerIndex);
}

int CDCM::SetTotalStartDelay(BYTE bySlotNo, BYTE byControllerIndex, double dDelay)
{
	auto iterBoard = m_mapBoard.find(bySlotNo);
	if (m_mapBoard.end() == iterBoard || nullptr == iterBoard->second)
	{
		return -1;
	}
	int nRetVal = iterBoard->second->SetTotalStartDelay(byControllerIndex, dDelay);
	if (0 != nRetVal)
	{
		switch (nRetVal)
		{
		case -1:
			///<The controller index is over range
			nRetVal = -2;
			break;
		case -2:
			///<The controller is not existed
			nRetVal = -3;
		case -3:
			///<The delay value is over range
			nRetVal = -4;
			break;
		default:
			break;
		}
	}
	return nRetVal;
}

double CDCM::GetTotalStartDelay(BYTE bySlotNo, BYTE byControllerIndex)
{
	auto iterBoard = m_mapBoard.find(bySlotNo);
	if (m_mapBoard.end() == iterBoard || nullptr == iterBoard->second)
	{
		return 0x7FFFFFFF;
	}
	return iterBoard->second->GetTotalStartDelay(byControllerIndex);
}

int CDCM::SetIODelay(BYTE bySlotNo, USHORT usChannel, double dData, double dDataEn, double dHigh, double dLow)
{
	auto iterBoard = m_mapBoard.find(bySlotNo);
	if (m_mapBoard.end() == iterBoard)
	{
		return -1;
	}
	int nRetVal = iterBoard->second->SetIODelay(usChannel, dData, dDataEn, dHigh, dLow);
	if (0 != nRetVal)
	{
		switch (nRetVal)
		{
		case -1:
			///<The channel number is over range
			nRetVal = -2;
			break;
		case -2:
			///<The channel is not existed
			nRetVal = -3;
			break;
		case -3:
			///<The delay is over range
			nRetVal = -4;
			break;
		default:
			break;
		}
	}
	return nRetVal;
}

int CDCM::GetIODelay(BYTE bySlotNo, USHORT usChannel, double* pdData, double* pdDataEn, double* pdHigh, double* pdLow)
{
	auto iterBoard = m_mapBoard.find(bySlotNo);
	if (m_mapBoard.end() == iterBoard || nullptr == iterBoard->second)
	{
		return -1;
	}
	int nRetVal = iterBoard->second->GetIODelay(usChannel, pdData, pdDataEn, pdHigh, pdLow);
	if (0 != nRetVal)
	{
		switch (nRetVal)
		{
		case -1:
			///<The channel number is over range
			nRetVal = -2;
			break;
		case -2:
			///<The channel is not existed
			nRetVal = -3;
			break;
		case -3:
			///<The point of the data is nullptr
			nRetVal = -4;
			break;
		default:
			break;
		}
	}
	return nRetVal;
}

int CDCM::SaveDelay(BYTE bySlotNo)
{
	auto iterBoard = m_mapBoard.find(bySlotNo);
	if (m_mapBoard.end() == iterBoard || nullptr == iterBoard->second)
	{
		return -1;
	}
	int nRetVal = 0;
	nRetVal = iterBoard->second->SaveDelay();
	if (0 != nRetVal)
	{
		switch (nRetVal)
		{
		case -1:
			///<Flash error
			nRetVal = -2;
			break;
		case -2:
			///<No valid controller existed
			nRetVal = -3;
			break;
		default:
			break;
		}
	}
	return nRetVal;
}


int CDCM::GetChannelCount(BYTE bySlotNo, BOOL bForceRrefresh)
{
	auto iterBoard = m_mapBoard.find(bySlotNo);
	if (m_mapBoard.end() == iterBoard || nullptr == iterBoard->second)
	{
		return -1;
	}
	int nRetVal = iterBoard->second->GetChannelCount(bForceRrefresh);
	if (0 > nRetVal)
	{
		switch (nRetVal)
		{
		case -1:
			///<The board is not existed
			nRetVal = -1;
			break;
		case -2:
			///<The flash error
			nRetVal = -2;
			break;
		case -3:
			///<Allocate memory fail
			nRetVal = -3;
			break;
		case -4:
			///<The data in flash is error
			nRetVal = -4;
			break;
		default:
			break;
		}
	}
	return nRetVal;
}

int CDCM::SetChannelCount(BYTE bySlotNo, USHORT usChannelCount)
{
	auto iterBoard = m_mapBoard.find(bySlotNo);
	if (m_mapBoard.end() == iterBoard || nullptr == iterBoard->second)
	{
		return -1;
	}
	int nRetVal = iterBoard->second->SetChannelCount(usChannelCount);
	if (0 != nRetVal)
	{
		switch (nRetVal)
		{
		case -1:
			nRetVal = -2;
			break;
		case -2:
			nRetVal = -3;
			break;
		case -3:
			nRetVal = -4;
		default:
			break;
		}
	}
	return nRetVal;
}

int CDCM::SetCalibrationData(BYTE bySlotNo, BYTE byControllerIndex, CAL_DATA* pCalData, BYTE byElementCount)
{
	auto iterController = m_mapBoard.find(bySlotNo);
	if (m_mapBoard.end() == iterController || nullptr == iterController->second)
	{
		return -1;
	}
	int nRetVal = iterController->second->SetCalibrationData(byControllerIndex, pCalData, byElementCount);
	if (0 != nRetVal)
	{
		switch (nRetVal)
		{
		case -1:
			nRetVal = -2;
			break;
		case -2:
			nRetVal = -3;
			break;
		case -3:
			nRetVal = -4;
			break;
		case -4:
			nRetVal = -5;
			break;
		case -5:
			nRetVal = -6;
			break;
		default:
			break;
		}
	}
	return nRetVal;
}

int CDCM::ResetCalibrationData(BYTE bySlotNo, BYTE byControllerIndex)
{
	auto iterController = m_mapBoard.find(bySlotNo);
	if (m_mapBoard.end() == iterController || nullptr == iterController->second)
	{
		return -1;
	}
	int nRetVal = iterController->second->ResetCalibrationData(byControllerIndex);
	if (0 != nRetVal)
	{
		switch (nRetVal)
		{
		case -1:
			///<The controller index is over range
			nRetVal = -2;
			break;
		case -2:
			///<The controller is not existed
			nRetVal = -3;
			break;
		case -3:
			///<Not set memory of calibration of the controller before
			nRetVal = -4;
			break;
		default:
			break;
		}
	}
	return nRetVal;
}

int CDCM::GetCalibrationData(BYTE bySlotNo, BYTE byControllerIndex, CAL_DATA* pCalData, BYTE byElementCount)
{
	auto iterController = m_mapBoard.find(bySlotNo);
	if (m_mapBoard.end() == iterController || nullptr == iterController->second)
	{
		return -1;
	}
	int nRetVal = iterController->second->GetCalibrationData(byControllerIndex, pCalData, byElementCount);
	if (0 != nRetVal)
	{
		switch (nRetVal)
		{
		case -1:
			///<Controller index is over range
			nRetVal = -2;
			break;
		case -2:
			///<The controller is not existed
			nRetVal = -3;
			break;
		case -3:
			///<The point of calibration data is nullptr
			nRetVal = -4;
			break;
		case -4:
			///<The element is not enough
			nRetVal = -5;
			break;
		case -5:
			///<The flash is error
			nRetVal = -6;
			break;
		case -6:
			///<The data in flash is error
			nRetVal = -7;
			break;
		default:
			break;
		}
	}
	return nRetVal;
}

int CDCM::ReadCalibrationData(BYTE bySlotNo, BYTE byControllerIndex)
{
	auto iterController = m_mapBoard.find(bySlotNo);
	if (m_mapBoard.end() == iterController || nullptr == iterController->second)
	{
		return -1;
	}
	int nRetVal = iterController->second->ReadCalibrationData(byControllerIndex);
	if (0 != nRetVal)
	{
		switch (nRetVal)
		{
		case -1:
			///<Controller index is over range
			nRetVal = -2;
			break;
		case -2:
			///<The controller is not existed
			nRetVal = -3;
			break;
		case -3:
			///<The flash is error
			nRetVal = -4;
			break;
		case -4:
			///<Allocate memory fail
			nRetVal = -5;
			break;
		case -5:
			///<The data in flash is error
			nRetVal = -6;
			break;
		default:
			break;
		}
	}
	return nRetVal;
}

int CDCM::SetHardInfo(BYTE bySlotNo, STS_HARDINFO* pHardInfo, int nModuleCount)
{
	auto iterBoard = m_mapBoard.find(bySlotNo);
	if (m_mapBoard.end() == iterBoard || nullptr == iterBoard->second)
	{
		return -1;
	}
	int nRetVal = iterBoard->second->SetHardInfo(pHardInfo, nModuleCount);
	if (0 != nRetVal)
	{
		switch (nRetVal)
		{
		case -1:
			return -1;
			break;
		case -2:
			///<The point of hardware information is nullptr
			nRetVal = -2;
			break;
		case -3:
			nRetVal = -3;
			break;
		default:
			break;
		}
	}
	return nRetVal;
}

int CDCM::GetHardInfo(BYTE bySlotNo, STS_HARDINFO* pHardInfo, int nElementCount)
{
	auto iterBoard = m_mapBoard.find(bySlotNo);
	if (m_mapBoard.end() == iterBoard || nullptr == iterBoard->second)
	{
		return -1;
	}
	return iterBoard->second->GetHardInfo(pHardInfo, nElementCount);
}

int CDCM::GetModuleInfo(const char* lpszPinName, USHORT usSiteNo, char* lpszInfo, int nInfoSize, MODULE_INFO SelInfo, STS_BOARD_MODULE Module)
{
	CHANNEL_INFO Channel;
	int nRetVal = 0;
	nRetVal = GetBoardChannel(lpszPinName, usSiteNo, Channel);
	if (0 != nRetVal)
	{
		return nRetVal;
	}
	auto iterBoard = m_mapBoard.find(Channel.m_bySlotNo);
	if (m_mapBoard.end() == iterBoard || nullptr == iterBoard->second || DCM_MAX_CHANNELS_PER_BOARD <= Channel.m_usChannel)
	{
		m_pAlarm->SetNoBoardAlarm(lpszPinName, TRUE, usSiteNo);
		return -7;
	}
	if (nullptr == lpszInfo || 0 >= nInfoSize)
	{
		m_pAlarm->ParameternullptrAlarm("lpszInfo", usSiteNo, lpszPinName);
		return -8;
	}
	STS_HARDINFO HardInfo;

	GetHardInfo(Channel.m_bySlotNo, &HardInfo, 1);
	const char* lpszTempInfo = nullptr;
	int nTempInfoSize = 0;
	switch (SelInfo)
	{
	case CDCM::MODULE_INFO::MODULE_NAME:
		nTempInfoSize = HardInfo.moduleInfo.nameSize;
		lpszTempInfo = HardInfo.moduleInfo.moduleName;
		break;
	case CDCM::MODULE_INFO::MODULE_SN:
		nTempInfoSize = HardInfo.moduleInfo.snSize;
		lpszTempInfo = HardInfo.moduleInfo.moduleSN;
		break;
	case CDCM::MODULE_INFO::MODULE_HDREV:
		nTempInfoSize = HardInfo.moduleInfo.revSize;
		lpszTempInfo = HardInfo.moduleInfo.moduleHardRev;
		break;
	default:
		nTempInfoSize = HardInfo.moduleInfo.nameSize;
		lpszTempInfo = HardInfo.moduleInfo.moduleName;
		break;
	}
	if (0 < nTempInfoSize)
	{
		if (nInfoSize > nTempInfoSize || 1 > nInfoSize)
		{
			nInfoSize = nTempInfoSize;
		}

		memcpy_s(lpszInfo, nInfoSize - 1, lpszTempInfo, nInfoSize - 1);
		lpszInfo[nInfoSize - 1] = 0;
	}
	return 0;
}

//#define _CHECK_VECTOR 1

#ifdef _CHECK_VECTOR
inline UINT GetChannelID(BYTE bySlotNo, USHORT usChannel)
{
	return (bySlotNo << 24) | usChannel;
}

inline USHORT ID2Channel(UINT uID, BYTE& bySlotNo)
{
	bySlotNo = (uID >> 24) & 0x0FF;
	return uID & 0xFFFF;
}
#endif // _CHECK_VECTOR

int CDCM::GetModuleInfo(BYTE bySlotNo, char* lpszInfo, int nInfoSize, MODULE_INFO SelInfo, STS_BOARD_MODULE Module)
{
	int nRetVal = 0;
	auto iterBoard = m_mapBoard.find(bySlotNo);
	if (m_mapBoard.end() == iterBoard || nullptr == iterBoard->second)
	{
		m_pAlarm->SetNoBoardAlarm(nullptr);
		return -1;
	}
	if (nullptr == lpszInfo || 0 >= nInfoSize)
	{
		m_pAlarm->ParameternullptrAlarm("lpszInfo", -1, nullptr);
		return -2;
	}
	STS_HARDINFO HardInfo;
	GetHardInfo(bySlotNo, &HardInfo, 1);
	const char* lpszTempInfo = nullptr;
	int nTempInfoSize = 0;
	switch (SelInfo)
	{
	case CDCM::MODULE_INFO::MODULE_NAME:
		nTempInfoSize = HardInfo.moduleInfo.nameSize;
		lpszTempInfo = HardInfo.moduleInfo.moduleName;
		break;
	case CDCM::MODULE_INFO::MODULE_SN:
		nTempInfoSize = HardInfo.moduleInfo.snSize;
		lpszTempInfo = HardInfo.moduleInfo.moduleSN;
		break;
	case CDCM::MODULE_INFO::MODULE_HDREV:
		nTempInfoSize = HardInfo.moduleInfo.revSize;
		lpszTempInfo = HardInfo.moduleInfo.moduleHardRev;
		break;
	default:
		nTempInfoSize = HardInfo.moduleInfo.nameSize;
		lpszTempInfo = HardInfo.moduleInfo.moduleName;
		break;
	}
	if (0 < nTempInfoSize)
	{
		if (nInfoSize > nTempInfoSize || 1 > nInfoSize)
		{
			nInfoSize = nTempInfoSize;
		}
		memcpy_s(lpszInfo, nInfoSize - 1, lpszTempInfo, nInfoSize - 1);
		lpszInfo[nInfoSize - 1] = 0;
	}

	return 0;
}

void CDCM::SetProgressFunc(STS_PROGRESS_INFO_FUN& pInfo, STS_PROGRESS_FUN& pSetpFun)
{
	m_pProgressInfo = pInfo;
	m_pProgressStep = pSetpFun;
}
int CDCM::LoadVectorFile(const char* lpszFileName, BOOL bReload)
{
	///<Delete the pin group saved in file
	m_bVectorShared = FALSE;
	m_bAllowAddPin = FALSE;///<Not allow added pin
	if (0 == m_mapBoard.size())
	{
		//No board exist;
		m_pAlarm->SetAlarmLevel(CDriverAlarm::ALARM_LEVEL::AlarmError);
		m_pAlarm->SetAlarmMsg("No DCM board existed.");
		m_pAlarm->SetAlarmID(ALARM_ID::ALARM_BOARD_NOT_EXISTED);
		return -1;
	}
	if (nullptr == lpszFileName)
	{
		m_pAlarm->ParameternullptrAlarm("lpszVectorFile", -1, nullptr);
		return -2;
	}

	set<string> setFailSynPin;
	ClearVector();
	///<Clear the fail synchronous
	SetFailSyn(setFailSynPin);
	map<BYTE, CTimeset*> mapTimeset;
	int nRetVal = m_VectorInfo.OpenFile(lpszFileName, m_mapPin, mapTimeset);
	if (0 != nRetVal)
	{
		switch (nRetVal)
		{
		case -2:
			//VectorEditor can't open the file.
			m_pAlarm->SetParamName("lpszVectorFile");
			m_pAlarm->SetAlarmMsg("The vector file(%s) is not existed.", lpszFileName);
			m_pAlarm->SetAlarmID(ALARM_ID::ALARM_VECTOR_NOT_LOAD);
			m_pAlarm->SetAlarmType(CDriverAlarm::ALARM_TYPE::PARAMETER_OCCURALARM);
			nRetVal = -3;
			break;
		case -3:
			//Can't find VectorEditor module.
			m_pAlarm->SetAlarmMsg("Can't find the module of ACVectorModel.dll");
			m_pAlarm->SetAlarmID(ALARM_ID::ALARM_VECTOR_NOT_LOAD);
			nRetVal = -4;
			break;
		case -4:
			//The format of vector file is wrong.
			m_pAlarm->SetParamName("lpszVectorFile");
			m_pAlarm->SetAlarmMsg("The format of vector file(%s) is wrong.", lpszFileName);
			m_pAlarm->SetAlarmID(ALARM_ID::ALARM_VECTOR_NOT_LOAD);
			nRetVal = -5;
			break;
		case -5:
			//Allocate memory fail.
			m_pAlarm->AllocateMemoryError();
			nRetVal = -6;
			break;
		default:
			//Unknown error.
			m_pAlarm->UnknownError();
			nRetVal = 0x80000000;
			break;
		}
		return nRetVal;
	}

	if (0 == m_mapPin.size())
	{
		///<No pin
		DeleteMemory(mapTimeset);
		m_pAlarm->SetParamName("lpszVectorFile");
		m_pAlarm->SetAlarmMsg("No pin defined in vector file.");
		m_pAlarm->SetAlarmID(ALARM_ID::ALARM_VECTOR_NOT_LOAD);
		return -7;
	}

	for (auto& Timeset : mapTimeset)
	{
		m_mapTimeset.insert(make_pair(Timeset.second->Name(), Timeset.second->ID()));
	}

	nRetVal = InitSite();
	if (0 != nRetVal)
	{
		DeleteMemory(mapTimeset);
		return -8;
	}

	vector<BYTE> vecUseSlot;
	m_Site.GetUseBoard(vecUseSlot);
	USHORT usUserSlot = vecUseSlot.size();

	set<string> setUsePin;
	for (auto& Pin : m_mapPin)
	{
		setUsePin.insert(Pin.first);
	}
	vector<USHORT> vecSite;
	BYTE byBindSlot = 0;
	BYTE byBindController = 0;
	UINT uBindSite = 0;
	set<USHORT> setBindChannel;
	USHORT usSiteCount = m_Site.GetSiteCount();
	for (USHORT usSiteNo = 0; usSiteNo < usSiteCount; ++usSiteNo)
	{
		setBindChannel.insert(usSiteNo);
		vecSite.push_back(usSiteNo);
	}

	m_bVectorBind = FALSE;
	nRetVal = Bind(setUsePin, setBindChannel);
	if (0 <= nRetVal)
	{
		uBindSite = nRetVal;
		vecSite.clear();
		vecSite.push_back(uBindSite);
		m_bVectorBind = TRUE;
	}

	///<Stop vector and disable receive total start
	map<BYTE, vector<USHORT>> mapChannel;
	auto iterBoard = m_mapBoard.begin();
	vector<string> vecPin;
	copy(setUsePin.begin(), setUsePin.end(), back_inserter(vecPin));
	GetBoardChannel(vecPin, vecSite, mapChannel);

	for (auto& Channel : mapChannel)
	{
		iterBoard = m_mapBoard.find(Channel.first);
		if (m_mapBoard.end() != iterBoard && nullptr != iterBoard->second)
		{
			iterBoard->second->StopVector(Channel.second);
			iterBoard->second->EnableStart(Channel.second, FALSE);
		}
	}
	mapChannel.clear();

	nRetVal = LoadVectorFileTimeset(vecSite, mapTimeset);
	if (0 != nRetVal)
	{
		if (m_bVectorBind)
		{
			ClearBind();
		}
		DeleteMemory(mapTimeset);
		return -9;
	}

	auto ClearPinGroupSection = [&]()
	{
		string strFile;
		string strSection;
		GetPinGroupInfoFile(strFile);
		GetPinGroupSection(strSection);
		WritePrivateProfileString(strSection.c_str(), nullptr, nullptr, strFile.c_str());
	};

	BOOL bVectorValid = FALSE;
	if (!bReload)
	{
		bVectorValid = IsVectorValid(lpszFileName, vecUseSlot);

		if (bVectorValid)
		{
			if (m_bVectorBind)
			{
				ClearBind();
			}
			DeleteMemory(mapTimeset);
			m_bLoadVector = TRUE;
			m_strFileName = lpszFileName;
			ClearPinGroupSection();
			SetFailSyn(setFailSynPin);
			return 0;
		}
	}
	DeleteMemory(mapTimeset);

	int nBRAMLineCount = m_VectorInfo.GetBRAMLineCount();
	int nDRAMLineCount = m_VectorInfo.GetDRAMLineCount();

	if (DCM_BRAM_PATTERN_LINE_COUNT < nBRAMLineCount || DCM_DRAM_PATTERN_LINE_COUNT < nDRAMLineCount)
	{
		if (DCM_BRAM_PATTERN_LINE_COUNT < nBRAMLineCount)
		{
			m_pAlarm->SetAlarmID(ALARM_ID::ALARM_VECTOR_NOT_LOAD);
			m_pAlarm->SetParamName("lpszVectorFile");
			m_pAlarm->SetAlarmMsg("The vector line count(%d) in BRAM is over range[0, %d].", nBRAMLineCount, DCM_BRAM_PATTERN_LINE_COUNT);
		}
		else
		{
			m_pAlarm->SetAlarmID(ALARM_ID::ALARM_VECTOR_NOT_LOAD);
			m_pAlarm->SetParamName("lpszVectorFile");
			m_pAlarm->SetAlarmMsg("The vector line count(%d) in DRAM is over range[0, %d].", nDRAMLineCount, DCM_DRAM_PATTERN_LINE_COUNT);
		}
		return -1;
	}

	int nLineCount = nBRAMLineCount + nDRAMLineCount;


	int nLeftLine = nLineCount;
	if (m_VectorInfo.IsDebugMode())
	{
		if (m_bVectorBind)
		{
			ClearBind();
		}
		nLeftLine = 0;
	}
	int nCurReadLine = 0;
	BOOL bLastLineCurMemory = FALSE;//The last line of current read is jump to BRAM or Jump to DRAM.
	BOOL bSaveBRAM = TRUE;
	BOOL bLoadFail = FALSE;
	int nDRAMOffset = 0;
	int nBRAMOffset = 0;
	pPinPatternInfo pPinPattern = nullptr;
	int nParallelInsCount = 0;
	USHORT usOparand = 0;
	const char* lpszCMD = nullptr;
	const char* lpszOperand = nullptr;
	const char* lpszLineLabel = nullptr;
	const char* lpszParallelCMD = nullptr;
	BYTE byTimeset = 0;
	CHANNEL_INFO ChannelInfo;
	int nCurPatternLine = 0;
	set<BYTE> setLoadBoard;
	BOOL bFirstLoad = TRUE;
	BOOL bCapture = FALSE;
	int nLineOffset = 0;

#ifdef _CHECK_VECTOR
	map<ULONG, char> mapVector;
#endif // _CHECK_VECTOR
	BOOL bNoBoard = TRUE;

	if (nullptr != m_pProgressInfo && nullptr != m_pProgressStep)
	{
		m_pProgressInfo("Load vector", nLineCount);
	}
	int nMemLineCount = 0;

	while (0 < nLeftLine)
	{
		bLastLineCurMemory = FALSE;
		nCurReadLine = 2048 < nLeftLine ? 2048 : nLeftLine;
		nRetVal = m_VectorInfo.ReadLine(nLineOffset, nCurReadLine, bSaveBRAM, &bLastLineCurMemory);
		if (0 != nRetVal)
		{
			bLoadFail = TRUE;
			break;
		}

		for (int uPatternIndex = 0; uPatternIndex < nCurReadLine; ++uPatternIndex)
		{
			BOOL bFailSyn = FALSE;
			int nTMUOperand = -1;
			m_VectorInfo.GetReadLine(uPatternIndex, pPinPattern, byTimeset, lpszCMD, lpszParallelCMD, lpszOperand, lpszLineLabel, bCapture);

			if (!bFailSyn && 0 != strlen(lpszCMD))
			{
				int nInstructionType = m_mapBoard.begin()->second->GetInstructionType(lpszCMD);
				bFailSyn = 1 == nInstructionType;
			}
			if (bSaveBRAM)
			{
				nCurPatternLine = nBRAMOffset;
			}
			else
			{
				nCurPatternLine = nDRAMOffset;
			}
			nCurPatternLine += uPatternIndex;

			if (nullptr == lpszCMD)
			{
				bLoadFail = TRUE;
				int nGlobalLineNo = 0;
				m_pAlarm->SetParamName("lpszVectorFile");
				nGlobalLineNo = m_VectorInfo.GetGlobalLineNo(nCurPatternLine, bSaveBRAM);
				m_pAlarm->SetAlarmID(ALARM_ID::ALARM_CMD_NOT_SUPPORTED);
				m_pAlarm->SetAlarmMsg("The command in line %d is not supported.", nGlobalLineNo + 1);
				break;
			}

			if ('0' <= lpszOperand[0] && '9' >= lpszOperand[0])
			{
				usOparand = atoi(lpszOperand);
			}

			BOOL bSwitch = FALSE;
			if (bLastLineCurMemory && uPatternIndex == nCurReadLine - 1)
			{
				bSwitch = TRUE;
			}
			for (auto& Pin : m_mapPin)
			{
				if (bFailSyn)
				{
					setFailSynPin.insert(Pin.first);
				}
				for (USHORT usSiteNo = 0; usSiteNo < usSiteCount; ++usSiteNo)
				{
					if (m_bVectorBind)
					{
						usSiteNo = uBindSite;
					}
					int nID = Pin.second->GetID();
					for (int nSerialIndex = 0; nSerialIndex < pPinPattern[nID].ucSerial; ++nSerialIndex)
					{
						Pin.second->GetChannel(usSiteNo, ChannelInfo);
						iterBoard = m_mapBoard.find(ChannelInfo.m_bySlotNo);
						if (m_mapBoard.end() == iterBoard)
						{
							continue;
						}
						bNoBoard = FALSE;
 						nRetVal = iterBoard->second->SetVector(ChannelInfo.m_usChannel + nSerialIndex, bSaveBRAM, nCurPatternLine, pPinPattern[Pin.second->GetID()].lpszPattern[nSerialIndex],
 							byTimeset, lpszCMD, lpszParallelCMD, usOparand, bCapture, bSwitch);
						if (0 != nRetVal)
						{
							if (-3 == nRetVal)
							{
								///<The controller is not existed
								m_pAlarm->SetAlarmLevel(CDriverAlarm::ALARM_LEVEL::AlarmWarning);
								m_pAlarm->SetAlarmID(ALARM_ID::ALARM_CHANNEL_NOT_EXISTED);
								m_pAlarm->SetAlarmMsg("The board(S%d) is existed, but the channel(S%d_%d) is not existed.", iterBoard->first, ChannelInfo.m_bySlotNo, ChannelInfo.m_usChannel);
								m_pAlarm->Output(FALSE);
								continue;
							}
							bLoadFail = TRUE;

							switch (nRetVal)
							{
							case -1:
								break;
							case -2:
								break;
							case -3:
								break;
							default:
								break;
							}
							break;
						}
						if (bFirstLoad)
						{
							setLoadBoard.insert(iterBoard->first);
							iterBoard->second->SetVectorValid(FALSE);
							auto iterChannel = mapChannel.find(iterBoard->first);
							if (mapChannel.end() == iterChannel)
							{
								vector<USHORT> vecChannel;
								mapChannel.insert(make_pair(iterBoard->first, vecChannel));
								iterChannel = mapChannel.find(iterBoard->first);
							}
							iterChannel->second.push_back(ChannelInfo.m_usChannel);
						}
#ifdef _CHECK_VECTOR
						UINT uID = GetChannelID(ChannelInfo.m_bySlotNo, ChannelInfo.m_usChannel);
						auto iterVector = mapVector.find(uID);
						if (mapVector.end() == iterVector)
						{
							mapVector.insert(make_pair(uID, 'X'));
							iterVector = mapVector.find(uID);
						}
						iterVector->second = pPinPattern[iterPin->second->GetID()].lpszPattern[nSerialIndex];
#endif // _CHECK_VECTOR

					}

					if (bLoadFail)
					{
						break;
					}
					if (m_bVectorBind)
					{
						break;
					}
				}

				if (bLoadFail)
				{
					break;
				}
			}
			bFirstLoad = FALSE;
			if (bLoadFail)
			{
				break;
			}

#ifdef _CHECK_VECTOR
			fstream VectorSave;
			string strPath = "D:\\";
			if (bSaveBRAM)
			{
				strPath += "VectorBRAM.csv";
			}
			else
			{
				strPath += "VectorDRAM.csv";
			}

			VectorSave.open(strPath.c_str(), ios::in);
			if (!VectorSave.is_open())
			{
				VectorSave.open(strPath.c_str(), ios::out | ios::app);
				VectorSave << "RAM Index,";
				for (auto& Vector : mapVector)
				{
					BYTE bySlotNo = 0;
					USHORT usChannel = ID2Channel(Vector.first, bySlotNo);
					char lpszMsg[32] = { 0 };
					sprintf_s(lpszMsg, sizeof(lpszMsg), "S%d_%d,", bySlotNo, usChannel);
					VectorSave << lpszMsg;
				}
				VectorSave << endl;
			}
			else
			{
				VectorSave.close();
				VectorSave.open(strPath.c_str(), ios::out | ios::app);
			}

			VectorSave << nCurPatternLine << ",";
			char lpszDigit[16] = { 0 };
			_itoa_s(nCurPatternLine, lpszDigit, sizeof(lpszDigit), 10);
			for (auto& Vector : mapVector)
			{
				VectorSave << Vector.second << ",";
				++iterVector;
			}
			VectorSave << endl;
			VectorSave.close();
#endif // _CHECK_VECTOR

		}
		if (bNoBoard)
		{
			m_pAlarm->SetAlarmID(ALARM_ID::ALARM_BOARD_NOT_EXISTED);
			m_pAlarm->SetAlarmLevel(CDriverAlarm::ALARM_LEVEL::AlarmError);
			m_pAlarm->SetParamName("lpszVectorFileName");
			m_pAlarm->SetAlarmMsg("No valid board used in vector file is existed.");
		}
		if (bLoadFail)
		{
			break;
		}

		if (bSaveBRAM)
		{
			nBRAMOffset += nCurReadLine;
		}
		else
		{
			nDRAMOffset += nCurReadLine;
		}

		nLineOffset += nCurReadLine;
		nLeftLine -= nCurReadLine;
		nMemLineCount += nCurReadLine;
		if (2048 <= nMemLineCount || 0 == nLeftLine)
		{
			for (auto LoadBoard : setLoadBoard)
			{
				iterBoard = m_mapBoard.find(LoadBoard);
				if (m_mapBoard.end() != iterBoard && nullptr != iterBoard->second)
				{
					nRetVal = iterBoard->second->LoadVector();
					if (0 != nRetVal)
					{
						bLoadFail = TRUE;
						break;
					}
				}
			}
			nMemLineCount = 0;
		}

		if (bLoadFail)
		{
			break;
		}

		if (nullptr != m_pProgressInfo && nullptr != m_pProgressStep)
		{
			m_pProgressStep(nLineOffset);
		}
	}

	int nInsLineNo = 0;
	ULONG ulInsOperand = 0;
	int nInsLabelLineNo = 0;
	int nInsWithLabelCount = m_VectorInfo.GetInsWithLabelCount();
	for (int nInsIndex = 0; nInsIndex < nInsWithLabelCount; ++nInsIndex)
	{
		nInsLineNo = m_VectorInfo.GetInsWithLabelLineNo(nInsIndex, nInsLabelLineNo);
		for (auto& Pin : mapChannel)
		{
			iterBoard = m_mapBoard.find(Pin.first);
			if (m_mapBoard.end()!= iterBoard)
			{
				iterBoard->second->SetOperand(Pin.second, nInsLineNo, nInsLabelLineNo, FALSE);
			}
		}
	}

	for (auto UserBoard : setLoadBoard)
	{
		iterBoard = m_mapBoard.find(UserBoard);
		if (m_mapBoard.end() != iterBoard && nullptr != iterBoard->second)
		{
			iterBoard->second->SetVectorValid(!bLoadFail);
		}
	}
	
	m_VectorInfo.CloseFile();

	if (m_bVectorBind)
	{
		ClearBind();
	}
	DeleteMemory(mapTimeset);
	if (bLoadFail)
	{
		//The vector file is load fail.
		m_bLoadVector = FALSE;

		if (nullptr != m_pProgressInfo && nullptr != m_pProgressStep)
		{
			m_pProgressStep(nLineCount);
		}

		return -10;
	}
	m_bLoadVector = TRUE;
	m_nBRAMLeftStartLine = m_VectorInfo.GetBRAMLineCount();
	m_nDRAMLeftStartLine = m_VectorInfo.GetDRAMLineCount();

	m_strFileName = lpszFileName;
	SaveVectorInformation(lpszFileName, setFailSynPin);

	///<Set the fail synchronous
	SetFailSyn(setFailSynPin);

#ifdef _CHECK_VECTOR
	UINT uMaxLineCountPerRead = 1024;
	char** lpszPattern = nullptr;

	fstream VectorSave;
	string strPath[2];
	strPath[0] = "D:\\BoardBRAM.csv";
	strPath[1] = "D:\\BoardDRAM.csv";

	try
	{
		USHORT usChannelCount = mapVector.size();
		lpszPattern = new char* [usChannelCount];
		for (USHORT usChannel = 0; usChannel < usChannelCount; ++usChannel)
		{
			lpszPattern[usChannel] = new char[uMaxLineCountPerRead];
			memset(lpszPattern[usChannel], 0, uMaxLineCountPerRead * sizeof(char));
		}
	}
	catch (const std::exception&)
	{
		return 0;
	}
	string* pstrPath = &strPath[0];
	USHORT usChannel = 0;
	BYTE bySlotNo = 0;
	BOOL bBRAM = TRUE;
	UINT* puLineCount = &m_nBRAMLeftStartLine;
	for (int nMemIndex = 0; nMemIndex < 2; ++nMemIndex)
	{
		if (0 == nMemIndex)
		{
			puLineCount = &m_nBRAMLeftStartLine;
			pstrPath = &strPath[0];
			bBRAM = TRUE;
		}
		else
		{
			puLineCount = &m_nDRAMLeftStartLine;
			pstrPath = &strPath[1];
			bBRAM = FALSE;
		}
		VectorSave.open(pstrPath->c_str(), ios::out | ios::out);
		VectorSave << "RAM Index,";
		for (auto& Vector : mapVector)
		{
			BYTE bySlotNo = 0;
			USHORT usChannel = ID2Channel(Vector.first, bySlotNo);
			char lpszMsg[32] = { 0 };
			sprintf_s(lpszMsg, sizeof(lpszMsg), "S%d_%d,", bySlotNo, usChannel);
			VectorSave << lpszMsg;
			++iterVector;
		}
		VectorSave << endl;

		UINT uCurReadLineCount = 0;
		UINT uVectorLefLine = *puLineCount;
		UINT uOffset = 0;
		while (0 < uVectorLefLine)
		{
			uCurReadLineCount = uVectorLefLine > uMaxLineCountPerRead ? uMaxLineCountPerRead : uVectorLefLine;

			int nIndex = 0;
			for (auto Vector : mapVector)
			{
				usChannel = ID2Channel(Vector.first, bySlotNo);
				iterBoard = m_mapBoard.find(bySlotNo);
				if (m_mapBoard.end() == iterBoard || nullptr == iterBoard->second)
				{
					lpszPattern[nIndex][uOffset] = '-';
					memset(lpszPattern[nIndex++], '-', uCurReadLineCount * sizeof(char));
					continue;
				}
				iterBoard->second->GetVector(usChannel, bBRAM, uOffset, uCurReadLineCount, lpszPattern[nIndex++]);
			}

			for (UINT uLineIndex = 0; uLineIndex < uCurReadLineCount; ++uLineIndex)
			{
				char lpszDigit[8] = { 0 };
				_itoa_s(uOffset + uLineIndex, lpszDigit, sizeof(lpszDigit), 10);
				VectorSave << lpszDigit << ",";
				nIndex = 0;
				iterVector = mapVector.begin();
				for (auto& Vector : mapVector)
				{
					usChannel = ID2Channel(Vector.first, bySlotNo);
					VectorSave << lpszPattern[nIndex][uLineIndex] << ",";
					++nIndex;
				}
				VectorSave << endl;
			}
			uOffset += uCurReadLineCount;
			uVectorLefLine -= uCurReadLineCount;
		}
		VectorSave.close();
	}
#endif // _CHECK_VECTOR

	if (nullptr != m_pProgressInfo && nullptr != m_pProgressStep)
	{
		m_pProgressStep(nLineCount);
	}
	ClearPinGroupSection();
	return 0;
}

void CDCM::DelteVectorInfoFile(const char* lpszFileName)
{
	string strInfoFile;
	GetVectorInfoFile(strInfoFile, lpszFileName);
	DeleteFile(strInfoFile.c_str());
}

void CDCM::CopyVectorInfo(const CDCM& DCM)
{
	m_bVectorShared = FALSE;
	DeleteMemory(m_mapPinGroup);
	DeleteMemory(m_mapPin);
	m_bLoadVector = DCM.m_bLoadVector;
	m_strFileName = DCM.m_strFileName;
	for (auto& Pin : DCM.m_mapPin)
	{
		CPin* pPin = new CPin(*Pin.second);
		m_mapPin.insert(make_pair(Pin.first, pPin));
	}
	m_Site = DCM.m_Site;
	m_mapTimeset = DCM.m_mapTimeset;
	m_mapLineInfo = DCM.m_mapLineInfo;
	m_VectorInfo = DCM.m_VectorInfo;

	m_nBRAMLeftStartLine = DCM.m_nBRAMLeftStartLine;
	m_nDRAMLeftStartLine = DCM.m_nDRAMLeftStartLine;
}

BOOL CDCM::IsVectorShared()
{
	return m_bVectorShared;
}

int CDCM::LoadVectorInfo(const char* lpszFileName)
{
	string strVectorInfoFile;
	GetVectorInfoFile(strVectorInfoFile, lpszFileName);
	int nRetVal = 0;
	set<string> setFailSynPin;
	nRetVal = LoadVectorInformation(nullptr, strVectorInfoFile, setFailSynPin, FALSE, &m_strFileName);
	if (0 != nRetVal)
	{
		return nRetVal;
	}
	map<BYTE, CTimeset*> mapTimeset;
	nRetVal = m_VectorInfo.OpenFile(m_strFileName.c_str(), m_mapPin, mapTimeset);
	if (0 != nRetVal)
	{
		m_strFileName.clear();
		return -2;
	}
	m_VectorInfo.CloseFile();
	InitSite(FALSE);
	vector<BYTE> vecSlot;
	m_Site.GetUseBoard(vecSlot);
	BOOL bVectorValid = TRUE;
	for (auto Slot : vecSlot)
	{
		auto iterBoard = m_mapBoard.find(Slot);
		if (m_mapBoard.end() == iterBoard || nullptr == iterBoard->second)
		{
			continue;
		}
		if (!iterBoard->second->IsVectorValid())
		{
			bVectorValid = FALSE;
			break;
		}
	}
	if (!bVectorValid)
	{
		m_Site.Reset();
		m_VectorInfo.Reset();
		m_bLoadVector = FALSE;
		return -3;
	}

	m_bLoadVector = TRUE;
	LoadPinGroupInfo();
	SetFailSyn(setFailSynPin);
	return 0;
}

int CDCM::SetPeriod(const char* lpszTimeset, double dPeriod)
{
	if (!m_bLoadVector)
	{
		m_pAlarm->VectorNotLoadedAlarm();
		return -1;
	}
	if (0 == m_mapBoard.size())
	{
		m_pAlarm->SetNoBoardAlarm(nullptr, FALSE);
		return -2;
	}
	if (nullptr == lpszTimeset)
	{
		m_pAlarm->ParameternullptrAlarm("lpszTimesetName", -1, nullptr);
		return -3;
	}

	auto iterTimeset = m_mapTimeset.find(lpszTimeset);
	if (m_mapTimeset.end() == iterTimeset)
	{
		m_pAlarm->TimesetError(lpszTimeset);
		return -4;
	}
		
	int nRetVal = 0;
	vector<CHANNEL_INFO> vecValidSiteChannel;
	m_Site.GetValidSiteChannel(vecValidSiteChannel);
	m_ClassifyBoard.SetChannel(vecValidSiteChannel);
	vector<USHORT> vecChannel;
	BOOL bNoBoard = TRUE;

	BOARD_CHANNEL_BEGIN(vecChannel)
	{
		bNoBoard = FALSE;
		nRetVal = iterBoard->second->SetPeriod(vecChannel, iterTimeset->second, dPeriod);
		if (0 != nRetVal)
		{
			m_pAlarm->SetParamName("dPeriod");
			nRetVal = -5;
			break;
		}
	}
	BOARD_CHANNEL_END

	return nRetVal;
}

double CDCM::GetPeriod(BYTE bySlotNo, BYTE byControllerIndex, BYTE byTimesetIndex)
{
	auto iterBoard = m_mapBoard.find(bySlotNo);
	if (m_mapBoard.end() == iterBoard || nullptr == iterBoard->second)
	{
		return -1;
	}
	double dPeriod = iterBoard->second->GetPeriod(byControllerIndex, byTimesetIndex);
	if (0 > dPeriod)
	{
		dPeriod -= EQUAL_ERROR;
		if (-2 < dPeriod)
		{
			///<Controller is over range
			dPeriod = -2;
		}
		else if (-3 < dPeriod)
		{
			///<Controller is not existed
			dPeriod = -3;
		}
		else
		{
			///<The timeset is over range
			dPeriod = -4;
		}
	}
	return dPeriod;
}

double CDCM::GetPeriod(BYTE bySlotNo, BYTE byControllerIndex, const char* lpszTimesetName)
{
	auto iterTimeset = m_mapTimeset.find(lpszTimesetName);
	if (m_mapTimeset.end() == iterTimeset)
	{
		return -1;
	}
	double dPeriod = GetPeriod(bySlotNo, byControllerIndex, iterTimeset->second);
	if (0 > dPeriod)
	{
		dPeriod -= EQUAL_ERROR;
		if (-2 < dPeriod)
		{
			dPeriod = -2;
		}
		else if (-3 < dPeriod)
		{
			///<Controller is over range
			dPeriod = -3;
		}
		else if(-4 < dPeriod)
		{
			///<Controller is not existed
			dPeriod = -4;
		}
		else
		{
			///<The timeset is over range
			dPeriod = -1;
		}
	}

	return dPeriod;
}


int CDCM::SetCalibartionRelay(BYTE bySlotNo, USHORT usChannel, BOOL bConnect)
{
	auto iterBoard = m_mapBoard.find(bySlotNo);
	if (m_mapBoard.end() == iterBoard || nullptr == iterBoard->second)
	{
		return -1;
	}
	int nRetVal = iterBoard->second->SetCalibrationRelay(usChannel, bConnect);
	if (0 != nRetVal)
	{
		switch (nRetVal)
		{
		case -1:
			///<The channel is over range
			nRetVal = -2;
			break;
		case -2:
			///<The channel is not existed
			nRetVal = -3;
			break;
		default:
			break;
		}
	}
	return 0;
}

int CDCM::SetPinGroup(const char* lpszPinGroupName, const char* lpszPinNameList)
{
// #ifdef RECORD_TIME
// 	CTimer::Instance()->Start("SetPinGroup_%s", lpszPinGroupName);
// 	CTimer::Instance()->Start("CheckParameter");
// #endif // RECORD_TIME
	if (!m_bLoadVector)
	{
		m_pAlarm->VectorNotLoadedAlarm();
		return -1;
	}
	if (0 == m_mapBoard.size())
	{
		m_pAlarm->SetNoBoardAlarm(lpszPinGroupName, FALSE);
		return -2;
	}
	if (nullptr == lpszPinGroupName)
	{
		m_pAlarm->ParameternullptrAlarm("lpszPinGroupName", -1, nullptr);
		return -3;
	}
	string strPinGroup = lpszPinGroupName;
	strPinGroup.erase(0, strPinGroup.find_first_not_of(' '));
	strPinGroup.erase(strPinGroup.find_last_not_of(' ') + 1);
	if (0 == strPinGroup.size())
	{
		m_pAlarm->SetAlarmID(ALARM_ID::ALARM_PARAM_BLANK);
		m_pAlarm->SetParamName("lpszPinGroup");
		m_pAlarm->SetAlarmMsg("The pin group name is blank.");
		return -4;
	}

// #ifdef RECORD_TIME
// 	CTimer::Instance()->Stop();
// 	CTimer::Instance()->Start("ExtractPin");
// #endif // RECORD_TIME

	set<string> setPin;
	int nRetVal = ExtractPinName(lpszPinNameList, setPin);
	if (0 != nRetVal)
	{
		m_pAlarm->SetParamName("lpszPinNameList");
		switch (nRetVal)
		{
		case -1:
			///<The point is nullptr, checked before
			m_pAlarm->ParameternullptrAlarm("lpszPinNameList", -1, nullptr);
			return -3;
			break;
		case -2:
			///<No pin in pin name string
			nRetVal = -5;
			break;
		case -3:
			///<The format is wrong
			nRetVal = -6;
			break;
		case -4:
			///<Some pin is not defined in vector
			nRetVal = -7;
			break;
		case -5:
			nRetVal = -8;
			break;
		}
		return nRetVal;
	}

// #ifdef RECORD_TIME
// 	CTimer::Instance()->Stop();
// 	CTimer::Instance()->Start("CheckPinGroup");
// #endif // RECORD_TIME

	auto iterPinGroup = m_mapPinGroup.find(lpszPinGroupName);
	if (m_mapPinGroup.end() != iterPinGroup)
	{
		if (!iterPinGroup->second->IsSamePinGroup(setPin))
		{
			m_pAlarm->SetParamName("lpszPinGroup");
			m_pAlarm->SetAlarmID(ALARM_ID::ALARM_PIN_GRUUOP_CONFLICT);
			m_pAlarm->SetAlarmMsg("The pin group name(%s) is conflict.", lpszPinGroupName);
			return -9;
		}
// #ifdef RECORD_TIME
// 		CTimer::Instance()->Stop();
// 		CTimer::Instance()->Stop();
// 		CTimer::Instance()->Print("D:\\SetPinGroup.csv");
// #endif // RECORD_TIME

		return 0;
	}
	else
	{
		CPinGroup* pPinGroup = new CPinGroup(lpszPinGroupName);
		m_mapPinGroup.insert(make_pair(lpszPinGroupName, pPinGroup));
		iterPinGroup = m_mapPinGroup.find(lpszPinGroupName);
	}

// #ifdef RECORD_TIME
// 	CTimer::Instance()->Stop();
// 	CTimer::Instance()->Start("AddPinGroup");
// #endif // RECORD_TIME

	iterPinGroup->second->SetPinName(setPin);

// #ifdef RECORD_TIME
// 	CTimer::Instance()->Stop();
// 	CTimer::Instance()->Start("SavePinGroupInfo");
// #endif // RECORD_TIME

	SavePinGroupInfo(lpszPinGroupName);

// #ifdef RECORD_TIME
// 	CTimer::Instance()->Stop();
// 	CTimer::Instance()->Stop();
// 	CTimer::Instance()->Print("D:\\SetPinGroup.csv");
// #endif // RECORD_TIME

	return nRetVal;
}

int CDCM::InitMCU(const char* lpszPinGroup)
{
#ifdef RECORD_TIME
	CTimer::Instance()->Reset();
	CTimer::Instance()->Start("InitMCU");
	if (nullptr != lpszPinGroup)
	{
		CTimer::Instance()->Start("GetChannel_PinGroup_%s", lpszPinGroup);
	}
#endif // RECORD_TIME

	if (!m_bLoadVector)
	{
		m_pAlarm->VectorNotLoadedAlarm();
		return -1;
	}

	int nRetVal = GetChannel(lpszPinGroup);
	if (0 != nRetVal)
	{
		switch (nRetVal)
		{
		case -2:
			//The pin group is nullptr
			nRetVal = -2;
			break;
		case -3:
			//The pin group is not defined before
			nRetVal = -3;
			break;
		case -5:
			///<No valid site
			nRetVal = -4;
			break;
		case -6:
			///<The pin is not belongs to
			nRetVal = -5;
			break;
		default:
			///<Not will happen
			break;
		}
		return nRetVal;
	}

#ifdef RECORD_TIME
	CTimer::Instance()->Stop();
	CTimer::Instance()->Start("InitBoardMCU");
#endif // RECORD_TIME


	vector<USHORT> vecChannel;
	BOOL bNoBoard = TRUE;

	BOARD_CHANNEL_BEGIN(vecChannel)
	{
		iterBoard->second->InitMCU(vecChannel);
		bNoBoard = FALSE;
	}
	BOARD_CHANNEL_END

	if (bNoBoard)
	{
		m_pAlarm->SetNoBoardAlarm(lpszPinGroup, FALSE);
		nRetVal = -6;
	}

#ifdef RECORD_TIME
	CTimer::Instance()->Stop();
	CTimer::Instance()->Stop();
	CTimer::Instance()->Print("D:\\InitMCU.csv");
#endif // RECORD_TIME


	return nRetVal;
}

int CDCM::InitPMU(const char* lpszPinGroup)
{
	if (!m_bLoadVector)
	{
		m_pAlarm->VectorNotLoadedAlarm();
		return -1;
	}

	int nRetVal = GetChannel(lpszPinGroup);
	if (0 != nRetVal)
	{
		switch (nRetVal)
		{
		case -2:
			//The pin group is nullptr
			nRetVal = -2;
			break;
		case -3:
			//The pin group is not defined before
			nRetVal = -3;
			break;
		case -5:
			///<No valid site
			nRetVal = -4;
			break;
		case -6:
			///<The pin is not belongs to
			nRetVal = -5;
			break;
		default:
			///<Not will happen
			break;
		}
		return nRetVal;
	}
	vector<USHORT> vecChannel;
	BOOL bNoBoard = TRUE;

	BOARD_CHANNEL_BEGIN(vecChannel)
	{
		iterBoard->second->InitPMU(vecChannel);
		bNoBoard = FALSE;
	}
	BOARD_CHANNEL_END
			
	if (bNoBoard)
	{
		m_pAlarm->SetNoBoardAlarm(lpszPinGroup, FALSE);
		nRetVal = -6;
	}
	return nRetVal;
}

int CDCM::SetPinLevel(const char* lpszPinGroup, double dVIH, double dVIL, double dVOH, double dVOL)
{
	if (!m_bLoadVector)
	{
		m_pAlarm->VectorNotLoadedAlarm();
		return -1;
	}

	int nRetVal = GetChannel(lpszPinGroup);
	if (0 != nRetVal)
	{
		switch (nRetVal)
		{
		case -2:
			//The pin group is nullptr
			nRetVal = -2;
			break;
		case -3:
			//The pin group is not defined before
			nRetVal = -3;
			break;
		case -5:
			///<No valid site
			nRetVal = -4;
			break;
		case -6:
			///<The pin is not belongs to
			nRetVal = -5;
			break;
		default:
			///<Not will happen
			break;
		}
		return nRetVal;
	}

	vector<USHORT> vecChannel;
	BOOL bNoBoard = TRUE;

	BOARD_CHANNEL_BEGIN(vecChannel)
	{
		bNoBoard = FALSE;
		nRetVal = iterBoard->second->SetPinLevel(vecChannel, dVIH, dVIL, dVOH, dVOL);
		if (0 != nRetVal)
		{
			m_pAlarm->SetPinString(lpszPinGroup, FALSE);
			nRetVal = -6;
			break;
		}
	}
	BOARD_CHANNEL_END

	if (bNoBoard)
	{
		m_pAlarm->SetNoBoardAlarm(lpszPinGroup, FALSE);
		return -7;
	}
	return nRetVal;
}

int CDCM::SetEdge(const char* lpszPinGroup, const char* lpszTimeset, WAVE_FORMAT WaveFormat, IO_FORMAT IOFormat, double* pdEdgeValue, COMPARE_MODE CompareMode)
{
	if (!m_bLoadVector)
	{
		m_pAlarm->VectorNotLoadedAlarm();
		return -1;
	}
	int nRetVal = GetChannel(lpszPinGroup);
	if (0 != nRetVal)
	{
		switch (nRetVal)
		{
		case -2:
			//The pin group is nullptr
			nRetVal = -2;
			break;
		case -3:
			//The pin group is not defined before
			nRetVal = -3;
			break;
		case -5:
			///<No valid site
			nRetVal = -4;
			break;
		case -6:
			///<The pin is not belongs to
			nRetVal = -5;
			break;
		default:
			///<Not will happen
			break;
		}
		return nRetVal;
	}
	if (nullptr == lpszTimeset)
	{
		m_pAlarm->ParameternullptrAlarm("lpszTimeset", -1, lpszPinGroup, FALSE);
		return -6;
	}

	auto iterTimeset = m_mapTimeset.find(lpszTimeset);
	if (m_mapTimeset.end() == iterTimeset)
	{
		m_pAlarm->TimesetError(lpszTimeset);
		return -7;
	}

	switch (WaveFormat)
	{
	case WAVE_FORMAT::NRZ:
		break;
	case WAVE_FORMAT::RZ:
		break;
	case WAVE_FORMAT::RO:
		break;
	case WAVE_FORMAT::SBH:
		break;
	case WAVE_FORMAT::SBL:
		break;
	case WAVE_FORMAT::SBC:
		break;
	default:
		return -8;
		break;
	}
	switch (IOFormat)
	{
	case IO_FORMAT::NRZ:
		break;
	case IO_FORMAT::RO:
		break;
	default:
		return -9;
		break;
	}
	switch (CompareMode)
	{
	case COMPARE_MODE::EDGE:
		break;
	case COMPARE_MODE::WINDOW:
		break;
	default:
		return -10;
		break;
	}
		
	vector<USHORT> vecChannel;
	BOOL bNoBoard = TRUE;

	BOARD_CHANNEL_BEGIN(vecChannel)
	{
		bNoBoard = FALSE;
		nRetVal = iterBoard->second->SetEdge(vecChannel, iterTimeset->second, pdEdgeValue, WaveFormat, IOFormat, CompareMode);
		if (0 != nRetVal)
		{
			m_pAlarm->SetPinString(lpszPinGroup, FALSE);
			switch (nRetVal)
			{
			case -3:
				///<The point of edge is nullptr
				nRetVal = -11;
			case -4:
				//The edge is over range
				nRetVal = -12;
				break;
			default:
				break;
			}
			break;
		}
	}
	BOARD_CHANNEL_END

	if (bNoBoard)
	{
		m_pAlarm->SetNoBoardAlarm(lpszPinGroup, FALSE);
		nRetVal = -13;
	}
	return nRetVal;
}

int CDCM::SetEdge(BYTE bySlotNo, USHORT usChannel, BYTE byTimeset, WAVE_FORMAT WaveFormat, IO_FORMAT IOFormat, double* pdEdgeValue, COMPARE_MODE CompareMode)
{
	auto iterBoard = m_mapBoard.find(bySlotNo);
	if (m_mapBoard.end() == iterBoard || nullptr == iterBoard->second)
	{
		return -1;
	}
	vector<USHORT> vecChannel;
	vecChannel.push_back(usChannel);
	int nRetVal = iterBoard->second->SetEdge(vecChannel, byTimeset, pdEdgeValue, WaveFormat, IOFormat, CompareMode);
	if (0 != nRetVal)
	{
		switch (nRetVal)
		{
		case -1:
			///<The timeset index is over range
			nRetVal = -2;
			break;
		case -2:
			///<The format is error
			nRetVal = -3;
		case -3:
			///<The point of the edge value is nullptr
			nRetVal = -4;
			break;
		case -4:
			///<The edge value is over range
			nRetVal = -5;
			break;
		case -5:
			///<The channel is not existed
			nRetVal = -6;
			break;
		default:
			break;
		}
	}
	return nRetVal;
}

int CDCM::GetEdge(BYTE bySlotNo, USHORT usChannel, BYTE byTimesetIndex, double* pdEdge, WAVE_FORMAT& WaveFormat, IO_FORMAT& IOFormat, COMPARE_MODE& CompareMode)
{
	auto iterBoard = m_mapBoard.find(bySlotNo);
	if (m_mapBoard.end() == iterBoard || nullptr == iterBoard->second)
	{
		return -1;
	}
	int nRetVal = iterBoard->second->GetEdge(usChannel, byTimesetIndex, pdEdge, WaveFormat, IOFormat, CompareMode);
	if (0 != nRetVal)
	{
		switch (nRetVal)
		{
		case -1:
			///<The channel is over range
			nRetVal = -2;
			break;
		case -2:
			///<The channel is not existed
			nRetVal = -3;
			break;
		case -3:
			///<The timeset is over range
			nRetVal = -4;
			break;
		case -4:
			///<The point of parameter is nullptr
			nRetVal = -5;
			break;
		default:
			break;
		}
	}
	return nRetVal;
}

int CDCM::GetStringType(const char* lpszString)
{
	if (!m_bLoadVector)
	{
		m_pAlarm->VectorNotLoadedAlarm();
		return -1;
	}
	if (nullptr == lpszString)
	{
		m_pAlarm->ParameternullptrAlarm("lpszString", -1, nullptr);
		return -2;
	}

	auto iterPin = m_mapPin.find(lpszString);
	if (m_mapPin.end() != iterPin)
	{
		///<The string is pin
		return 0;
	}

	auto iterPinGroup = m_mapPinGroup.find(lpszString);
	if (m_mapPinGroup.end() != iterPinGroup)
	{
		///<The string  is pin group
		return 1;
	}
	int nLabelLineNum = m_VectorInfo.GetLabelLineNo(lpszString);
	if (0 <= nLabelLineNum)
	{
		///<The string is label
		return 2;
	}

	return -3;
}

int CDCM::RunVector(const char* lpszPinGroup, const char* lpszStartLabel, const char* lpszStopLabel, BOOL bWaitFinish)
{
	if (!m_bLoadVector)
	{
		m_pAlarm->VectorNotLoadedAlarm();
		return -1;
	}

	int nRetVal = GetChannel(lpszPinGroup);
	if (0 != nRetVal)
	{
		switch (nRetVal)
		{
		case -2:
			//The pin group is nullptr
			nRetVal = -2;
			break;
		case -3:
			//The pin group is not defined before
			nRetVal = -3;
			break;
		case -5:
			///<All site invalid
			nRetVal = -4;
		case -6:
			///<The pin is not belongs to
			nRetVal = -5;
			break;
		default:
			///<Not will happen
			break;
		}
		return nRetVal;
	}

	int nStartLine = 0;
	int nStopLine = 0;
	if (nullptr == lpszStartLabel)
	{
		m_pAlarm->SetPinString(lpszPinGroup, FALSE);
		m_pAlarm->SetAlarmID(ALARM_ID::ALARM_START_LABEL_ERROR);
		m_pAlarm->SetParamName("lpszStartLabel");
		m_pAlarm->SetAlarmMsg("The start label is nullptr.");
		m_pAlarm->SetAlarmLevel(CDriverAlarm::ALARM_LEVEL::AlarmError);
		return -6;
	}
	else if (0 != strlen(lpszStartLabel))
	{
		nStartLine = m_VectorInfo.GetLabelLineNo(lpszStartLabel, FALSE);
		if (0 > nStartLine)
		{
			m_pAlarm->SetPinString(lpszPinGroup, FALSE);
			m_pAlarm->SetAlarmID(ALARM_ID::ALARM_START_LABEL_ERROR);
			m_pAlarm->SetAlarmType(CDriverAlarm::ALARM_TYPE::PARAMETER_NOT_DEFINED);
			m_pAlarm->SetParamName("lpszStartLabel");
			m_pAlarm->SetAlarmMsg("The start label(%s) is not defined in vector file.", lpszStartLabel);
			m_pAlarm->SetAlarmLevel(CDriverAlarm::ALARM_LEVEL::AlarmError);
			return -7;
		}
	}
	if (nullptr == lpszStopLabel)
	{
		m_pAlarm->SetPinString(lpszPinGroup, FALSE);
		m_pAlarm->SetAlarmID(ALARM_ID::ALARM_START_LABEL_ERROR);
		m_pAlarm->SetParamName("lpszStopLabel");
		m_pAlarm->SetAlarmMsg("The stop label is nullptr.");
		m_pAlarm->SetAlarmLevel(CDriverAlarm::ALARM_LEVEL::AlarmError);
		return -8;
	}
	else if (0 != strlen(lpszStopLabel))
	{
		nStopLine = m_VectorInfo.GetLabelLineNo(lpszStopLabel, FALSE);
		if (0 > nStopLine)
		{
			m_pAlarm->SetPinString(lpszPinGroup, FALSE);
			m_pAlarm->SetAlarmID(ALARM_ID::ALARM_STOP_LABEL_ERROR);
			m_pAlarm->SetAlarmType(CDriverAlarm::ALARM_TYPE::PARAMETER_NOT_DEFINED);
			m_pAlarm->SetParamName("lpszStopLabel");
			m_pAlarm->SetAlarmMsg("The stop label(%s) is not defined in vector file.", lpszStopLabel);
			m_pAlarm->SetAlarmLevel(CDriverAlarm::ALARM_LEVEL::AlarmError);
			return -9;
		}
	}
	else
	{
		nStopLine = DCM_BRAM_PATTERN_LINE_COUNT - 1;
	}
	if (nStartLine >= nStopLine)
	{
		m_pAlarm->SetPinString(lpszPinGroup, FALSE);
		m_pAlarm->SetAlarmID(ALARM_ID::ALARM_START_AFTER_STOP_LABEL);
		m_pAlarm->SetParamName("lpszStartLabel");
		m_pAlarm->SetAlarmMsg("The start label(%s) must before stop label(%s).", lpszStartLabel, lpszStopLabel);
		m_pAlarm->SetAlarmLevel(CDriverAlarm::ALARM_LEVEL::AlarmError);
		return -10;
	}

	UINT uDRAMStartLine = 0;
	int nSplitIndex = 0;

	BOOL bHaveDRAM = m_VectorInfo.GetDRAMRunStartLine(nStartLine, nStopLine, uDRAMStartLine, nSplitIndex);

	vector<USHORT> vecChannel;
	BOOL bNoBoard = TRUE;
	USHORT usFirstChannel = 0;
	CBoard* pBoard = nullptr;
	BOARD_CHANNEL_BEGIN(vecChannel)
	{
		bNoBoard = FALSE;
		pBoard = iterBoard->second;
		usFirstChannel = vecChannel[0];
		nRetVal = iterBoard->second->SetRunParam(vecChannel, nStartLine, nStopLine, bHaveDRAM, uDRAMStartLine, FALSE);
	}
	BOARD_CHANNEL_END

	if (bNoBoard)
	{
		m_pAlarm->SetNoBoardAlarm(lpszPinGroup, FALSE);
		return -11;
	}

	CRunAuthorization* pRunAuthorization = CRunAuthorization::Instance();
	pRunAuthorization->Apply();

	BOARD_CHANNEL_BEGIN(vecChannel)
	{
		bNoBoard = FALSE;
		iterBoard->second->EnableStart(vecChannel, TRUE);
	}
	BOARD_CHANNEL_END

	if (nullptr != pBoard)
	{
		pBoard->SynRun(usFirstChannel);
	}

	m_bWaitRun = FALSE;
	BOARD_CHANNEL_BEGIN(vecChannel)
	{
		bNoBoard = FALSE;
		iterBoard->second->EnableStart(vecChannel, FALSE);
	}
	BOARD_CHANNEL_END

	pRunAuthorization->Release();
	if (bWaitFinish)
	{
		BOARD_CHANNEL_BEGIN(vecChannel)
		{
			nRetVal = iterBoard->second->WaitStop(vecChannel);
			if (0 != nRetVal)
			{
				break;
			}
		}
		BOARD_CHANNEL_END
	}
	m_nLatestStartLine = nStartLine;
	m_nLatestStopLine = nStopLine;
	m_strLatestRanPinGroup = lpszPinGroup;
	return 0;
}

int CDCM::SetRunParam(const char* lpszPinGroup, const char* lpszStartLabel, const char* lpszStopLabel)
{
	if (!m_bLoadVector)
	{
		m_pAlarm->VectorNotLoadedAlarm();
		return -1;
	}

	int nRetVal = GetChannel(lpszPinGroup);
	if (0 != nRetVal)
	{
		switch (nRetVal)
		{
		case -2:
			//The pin group is nullptr
			nRetVal = -2;
			break;
		case -3:
			//The pin group is not defined before
			nRetVal = -3;
			break;
		case -5:
			///<No valid site
			nRetVal = -4;
		case -6:
			///<The pin is not belongs to
			nRetVal = -5;
			break;
		default:
			///<Not will happen
			break;
		}
		return nRetVal;
	}


	int nStartLine = 0;
	int nStopLine = 0;
	if (nullptr == lpszStartLabel)
	{
		m_pAlarm->SetAlarmID(ALARM_ID::ALARM_PARAM_NULLPTR);
		m_pAlarm->SetParamName("lpszStartLabel");
		m_pAlarm->SetAlarmMsg("The start label is nullptr.");
		return -6;
	}
	else if (0 != strlen(lpszStartLabel))
	{
		nStartLine = m_VectorInfo.GetLabelLineNo(lpszStartLabel, FALSE);
		if (0 > nStartLine)
		{
			m_pAlarm->SetAlarmID(ALARM_ID::ALARM_START_LABEL_ERROR);
			m_pAlarm->SetAlarmType(CDriverAlarm::ALARM_TYPE::PARAMETER_NOT_DEFINED);
			m_pAlarm->SetParamName("lpszStartLabel");
			m_pAlarm->SetAlarmMsg("The start label(%s) is not defined in vector file.", lpszStartLabel);
			m_pAlarm->SetAlarmLevel(CDriverAlarm::ALARM_LEVEL::AlarmError);
			return -7;
		}
	}
	if (nullptr == lpszStopLabel)
	{
		m_pAlarm->SetAlarmID(ALARM_ID::ALARM_PARAM_NULLPTR);
		m_pAlarm->SetParamName("lpszStopLabel");
		m_pAlarm->SetAlarmMsg("The stop label is nullptr.");
		return -8;
	}
	else if (0 != strlen(lpszStopLabel))
	{
		nStopLine = m_VectorInfo.GetLabelLineNo(lpszStopLabel,  FALSE);
		if (0 > nStopLine)
		{
			m_pAlarm->SetAlarmID(ALARM_ID::ALARM_STOP_LABEL_ERROR);
			m_pAlarm->SetAlarmType(CDriverAlarm::ALARM_TYPE::PARAMETER_NOT_DEFINED);
			m_pAlarm->SetParamName("lpszStopLabel");
			m_pAlarm->SetAlarmMsg("The stop label(%s) is not defined in vector file.", lpszStopLabel);
			m_pAlarm->SetAlarmLevel(CDriverAlarm::ALARM_LEVEL::AlarmError);
			return -9;
		}
	}
	else
	{
		nStopLine = DCM_BRAM_PATTERN_LINE_COUNT - 1;
	}
	if (nStartLine >= nStopLine)
	{
		m_pAlarm->SetAlarmID(ALARM_ID::ALARM_START_AFTER_STOP_LABEL);
		m_pAlarm->SetParamName("lpszStopLabel");
		m_pAlarm->SetAlarmMsg("The start label(%s) must before stop label(%s).", lpszStartLabel, lpszStopLabel);
		m_pAlarm->SetAlarmLevel(CDriverAlarm::ALARM_LEVEL::AlarmError);
		return -10;
	}

	UINT uDRAMStartLine = 0;
	int nSplitIndex = 0;
	BOOL bHaveDRAM = m_VectorInfo.GetDRAMRunStartLine(nStartLine, nStopLine, uDRAMStartLine, nSplitIndex);
	vector<USHORT> vecChannel;
	BOOL bNoBoard = TRUE;
	BYTE byFirstSlot = 0;

	BOARD_CHANNEL_BEGIN(vecChannel)
	{
		bNoBoard = FALSE;
		byFirstSlot = iterBoard->first;
		nRetVal = iterBoard->second->SetRunParam(vecChannel, nStartLine, nStopLine, bHaveDRAM, uDRAMStartLine, FALSE);
	}
	BOARD_CHANNEL_END

	if (bNoBoard)
	{
		m_pAlarm->SetNoBoardAlarm(lpszPinGroup, FALSE);
		return -11;
	}

	m_bWaitRun = TRUE;
	m_strLatestRanPinGroup = lpszPinGroup;
	m_nLatestStartLine = nStartLine;
	m_nLatestStopLine = nStopLine;

	return 0;
}

int CDCM::SetChannleStatus(const char* lpszPinGroup, USHORT usSiteNo, CHANNEL_OUTPUT_STATUS ChannelStatus)
{
	if (!m_bLoadVector)
	{
		m_pAlarm->VectorNotLoadedAlarm();
		return -1;
	}

	int nRetVal = GetChannel(lpszPinGroup, usSiteNo);
	if (0 != nRetVal)
	{
		switch (nRetVal)
		{
		case -2:
			//The pin group is nullptr
			nRetVal = -2;
			break;
		case -3:
			//The pin group is not defined before
			nRetVal = -3;
			break;
		case -4:
			///<Site over range
			nRetVal = -4;
			break;
		case -5:
			///<Site invalid
			nRetVal = -5;
			break;
		case -6:
			///<The pin is not belongs to
			nRetVal = -6;
			break;
		default:
			///<Not will happen
			break;
		}
		return nRetVal;
	}
	vector<USHORT> vecChannel;
	BOOL bNoBoard = TRUE;

	BOARD_CHANNEL_BEGIN(vecChannel)
	{
		bNoBoard = FALSE;
		nRetVal = iterBoard->second->SetChannelStatus(vecChannel, ChannelStatus);
		if (0 != nRetVal)
		{
			m_pAlarm->SetPinString(lpszPinGroup, FALSE);
			m_pAlarm->SetParamName("lpszPinGroup");
			m_pAlarm->SetSite(usSiteNo);
			switch (nRetVal)
			{
			case -1:
				m_pAlarm->SetAlarmMsg("The channel status(%d) is not supported.", (int)ChannelStatus);
				m_pAlarm->SetAlarmID(ALARM_ID::ALARM_MODE_ERROR);
				nRetVal = -7;
				break;
			case -2:
				m_pAlarm->SetAlarmID(ALARM_ID::ALARM_CHANNEL_NOT_EXISTED);
				m_pAlarm->SetAlarmMsg("No valid channel in %s of SITE_%d.", lpszPinGroup, usSiteNo + 1);
				nRetVal = -8;
			default:
				break;
			}
			break;
		}
	}
	BOARD_CHANNEL_END
	if (bNoBoard)
	{
		nRetVal = -9;
	}
	return nRetVal;
}

int CDCM::GetChannelStatus(BYTE bySlotNo, USHORT usChannel)
{
	int nRetVal = 0;
	auto iterBoard = m_mapBoard.find(bySlotNo);
	if (m_mapBoard.end() == iterBoard || nullptr == iterBoard->second)
	{
		return -1;
	}
	nRetVal = iterBoard->second->GetChannelStatus(usChannel);
	if (0 > nRetVal)
	{
		switch (nRetVal)
		{
		case -1:
			///<The channel number is over range
			nRetVal = -2;
			break;
		case -2:
			///<The channel is not existed
			nRetVal = -3;
		default:
			break;
		}
	}
	return nRetVal;
}

int CDCM::GetChannelMode(BYTE bySlotNo, USHORT usChannel)
{
	auto iterSlot = m_mapBoard.find(bySlotNo);
	if (m_mapBoard.end() == iterSlot || nullptr == iterSlot->second)
	{
		return -1;
	}
	int nRetVal = iterSlot->second->GetChannelMode(usChannel);
	if (0 != nRetVal)
	{
		switch (nRetVal)
		{
		case -1:
			///<The channel is over range
			nRetVal = -2;
			break;
		case -2:
			///<The channel is not existed
			nRetVal = -3;
		default:
			break;
		}
	}
	return nRetVal;
}

int CDCM::IsWaitRun()
{
	return m_bWaitRun;
}

int CDCM::Run()
{
	if (!m_bWaitRun)
	{
		return -1;
	}
	GetChannel(m_strLatestRanPinGroup.c_str());

	CRunAuthorization* pRunAuthorization = CRunAuthorization::Instance();
	pRunAuthorization->Apply();


	vector<USHORT> vecChannel;
	m_bWaitRun = FALSE;
	BOOL bRun = FALSE;

	BOARD_CHANNEL_BEGIN(vecChannel)
	{
		iterBoard->second->EnableStart(vecChannel, TRUE);
	}
	BOARD_CHANNEL_END


	BOARD_CHANNEL_BEGIN(vecChannel)
	{
		if (!bRun)
		{
			iterBoard->second->SynRun(vecChannel[0]);
			bRun = TRUE;
		}
		iterBoard->second->EnableStart(vecChannel, FALSE);
	}
	BOARD_CHANNEL_END

	pRunAuthorization->Release();
	return 0;
}

int CDCM::WaitStop()
{
	vector<USHORT> vecChannel;
	GetChannel(m_strLatestRanPinGroup.c_str());

	BOARD_CHANNEL_BEGIN(vecChannel)
	{
		int nRetVal = iterBoard->second->WaitStop(vecChannel);
		if (0 != nRetVal)
		{
			break;
		}
	}
	BOARD_CHANNEL_END

	return 0;
}

int CDCM::Stop(const char* lpszPinGroup)
{
	if (!m_bLoadVector)
	{
		m_pAlarm->VectorNotLoadedAlarm();
		return -1;
	}

	int nRetVal = GetChannel(lpszPinGroup);
	if (0 != nRetVal)
	{
		switch (nRetVal)
		{
		case -2:
			//The pin group is nullptr
			nRetVal = -2;
			break;
		case -3:
			//The pin group is not defined before
			nRetVal = -3;
			break;
		case -5:
			///<No valid site
			nRetVal = -4;
		case -6:
			///<The pin is not belongs to
			nRetVal = -5;
			break;
		default:
			///<Not will happen
			break;
		}
		return nRetVal;
	}
	vector<USHORT> vecChannel;
	BOOL bNoBoard = TRUE;

	BOARD_CHANNEL_BEGIN(vecChannel)
	{
		nRetVal = iterBoard->second->StopVector(vecChannel);
		if (0 != nRetVal)
		{
			m_pAlarm->SetAlarmLevel(CDriverAlarm::ALARM_LEVEL::AlarmWarning);
			m_pAlarm->SetAlarmID(ALARM_ID::ALARM_CHANNEL_NOT_EXISTED);
			m_pAlarm->SetAlarmMsg("The channels of board(S%d) used in pin group(%s) is not existed.", iterBoard->first, lpszPinGroup);
			m_pAlarm->Output(FALSE);
		}
		else
		{
			bNoBoard = FALSE;
		}
	}
	BOARD_CHANNEL_END

	if (bNoBoard)
	{
		m_pAlarm->SetNoBoardAlarm(lpszPinGroup, FALSE);
		return -6;
	}
	return 0;
}

int CDCM::GetCaptureData(const char* lpszPinName, USHORT usSiteNo, const char* lpszStartLabel, ULONG ulOffset, int nLineCount, ULONG& ulData)
{
#ifdef RECORD_TIME
	CTimer::Instance()->Start("GetCaptureData");
	CTimer::Instance()->Start("CheckParameter");
#endif // RECORD_TIME

	ulData = -1;
	int nRetVal = 0;
	CHANNEL_INFO Channel;
	nRetVal = GetBoardChannel(lpszPinName, usSiteNo, Channel);
	if (0 != nRetVal)
	{
		return nRetVal;
	}
	auto iterBoard = m_mapBoard.find(Channel.m_bySlotNo);
	if (m_mapBoard.end() == iterBoard || nullptr == iterBoard->second || DCM_MAX_CHANNELS_PER_BOARD <= Channel.m_usChannel)
	{
		m_pAlarm->SetNoBoardAlarm(lpszPinName, TRUE, usSiteNo);
		return -7;
	}

	if (nullptr == lpszStartLabel)
	{
		m_pAlarm->ParameternullptrAlarm("lpszStartLabel", usSiteNo, lpszPinName);
		return -8;
	}
	int nStartLineNo = m_VectorInfo.GetLabelLineNo(lpszStartLabel, TRUE);
	if (0 > nStartLineNo)
	{
		m_pAlarm->SetAlarmID(ALARM_ID::ALARM_START_LABEL_ERROR);
		m_pAlarm->SetAlarmType(CDriverAlarm::ALARM_TYPE::PARAMETER_NOT_DEFINED);
		m_pAlarm->SetParamName("lpszStartLabel");
		m_pAlarm->SetAlarmMsg("The start label(%s) is not defined in vector file.", lpszStartLabel);
		m_pAlarm->SetAlarmLevel(CDriverAlarm::ALARM_LEVEL::AlarmError);
		return -9;
	}
	UINT uTotalLineCount = m_VectorInfo.GetBRAMLineCount() + m_VectorInfo.GetDRAMLineCount();

	if (uTotalLineCount <= nStartLineNo + ulOffset)
	{
		m_pAlarm->SetAlarmID(ALARM_ID::ALARM_OFFSET_OVER_RANGE);
		m_pAlarm->SetAlarmType(CDriverAlarm::ALARM_TYPE::PARAMETER_OVERRANGE);
		m_pAlarm->SetParamName("lpszStartLabel");
		m_pAlarm->SetAlarmMsg("The offset(%d) is over range[%d,%d).", ulOffset, 0, uTotalLineCount - nStartLineNo);
		m_pAlarm->SetAlarmLevel(CDriverAlarm::ALARM_LEVEL::AlarmError);
		return -10;
	}
	nStartLineNo += ulOffset;

	if (32 < nLineCount || uTotalLineCount < nStartLineNo + nLineCount)
	{
		m_pAlarm->SetPinString(lpszPinName, TRUE);
		m_pAlarm->SetSite(usSiteNo);
		m_pAlarm->SetParamName("uLineCount");
		m_pAlarm->SetAlarmID(ALARM_ID::ALARM_LINE_COUNT_OVER_RANGE);
		m_pAlarm->SetAlarmType(CDriverAlarm::ALARM_TYPE::PARAMETER_OVERRANGE);
		int nMaxLineCount = 32 < nLineCount ? 32 : (uTotalLineCount - nStartLineNo);
		m_pAlarm->SetAlarmMsg("The uLineCount(%d) is over range[%d,%d].", ulOffset, 1, nMaxLineCount);
		m_pAlarm->SetAlarmLevel(CDriverAlarm::ALARM_LEVEL::AlarmWarning);
		m_pAlarm->Output(FALSE);

		nLineCount = nMaxLineCount;
	}

#ifdef RECORD_TIME
	CTimer::Instance()->Stop();
	CTimer::Instance()->Start("GetLineNo");
#endif // RECORD_TIME

	int nStartRanLine = m_VectorInfo.GetGlobalLineNo(m_nLatestStartLine);
	int nStopRanLine = m_VectorInfo.GetGlobalLineNo(m_nLatestStopLine);

	if (nStartRanLine > nStartLineNo || nStopRanLine < nStartLineNo + nLineCount - 1)
	{
		m_pAlarm->SetPinString(lpszPinName, TRUE);
		m_pAlarm->SetSite(usSiteNo);
		m_pAlarm->SetParamName("uLineCount");
		m_pAlarm->SetAlarmID(ALARM_ID::ALARM_LINE_COUNT_OVER_RANGE);
		m_pAlarm->SetAlarmType(CDriverAlarm::ALARM_TYPE::PARAMETER_OVERRANGE);
		int nMaxLineCount = 32 < nLineCount ? 32 : (uTotalLineCount - nStartLineNo - nLineCount);
		m_pAlarm->SetAlarmMsg("The capture line range(%d to %d) is not included in latest ran vector(%d to %d).", nStartLineNo + 1, nStartLineNo + nLineCount, nStartRanLine + 1, nStopRanLine + 1);
		m_pAlarm->SetAlarmLevel(CDriverAlarm::ALARM_LEVEL::AlarmError);
		return -11;
	}
	vector<USHORT> vecChannel;
	vecChannel.push_back(Channel.m_usChannel);

#ifdef RECORD_TIME
	CTimer::Instance()->Stop();
	CTimer::Instance()->Start("ReadFailLineNo");
#endif // RECORD_TIME

	vector<UINT> vecFailLine;
	nRetVal = iterBoard->second->PreloadFailLineNo(vecChannel, -1);
	if (0 != nRetVal)
	{
		m_pAlarm->SetPinString(lpszPinName, TRUE);
		m_pAlarm->SetSite(usSiteNo);
		BOOL bAddAlarm = !m_pAlarm->IsSetMsg();
		switch (nRetVal)
		{
		case -1:
			///<Vector not ran
			if (bAddAlarm)
			{
				m_pAlarm->SetAlarmID(ALARM_ID::ALARM_NOT_RAN_VECTOR);
				m_pAlarm->SetAlarmMsg("The vector of channel(S%d_%d) of pin(%s) in SITE_%d is not ran.", Channel.m_bySlotNo, Channel.m_usChannel, lpszPinName, usSiteNo + 1);
			}
			nRetVal = -12;
			break;
		case -2:
			///<Vector running
			if (bAddAlarm)
			{
				m_pAlarm->SetAlarmID(ALARM_ID::ALARM_VECTOR_RUNNING);
				m_pAlarm->SetAlarmMsg("The vector of channel(S%d_%d) of pin(%s) in SITE_%d is running.", Channel.m_bySlotNo, Channel.m_usChannel, lpszPinName, usSiteNo + 1);
			}
			nRetVal = -13;
			break;
		default:
			break;
		}
		return nRetVal;
	}
#ifdef RECORD_TIME
	CTimer::Instance()->Stop();
	CTimer::Instance()->Start("GetPinCaptureData");
#endif // RECORD_TIME

	m_nCaptureStartOffset = nStartLineNo - nStartRanLine;
	m_nCaptureLineCount = nLineCount;
	nRetVal = GetCaptureData(lpszPinName, usSiteNo, ulData);
	if (0 != nRetVal)
	{
		switch (nRetVal)
		{
		case -1:
		case -2:
		case -3:
		case -4:
		case -5:
		case -6:
			///<Checked before
			break;
		case -7:
			///<The channel is over range
			nRetVal = -14;
			break;
		case -8:
			///<The channel is not existed
			nRetVal = -15;
			break;
		case -9:
			///<Not ran
			nRetVal = -12;
			break;
		case -10:
			///<Running
			nRetVal = -13;
			break;
		case -11:
			///<Not set capture line, not happen
			break;
		case -13:
			///<Fail line not saving
			nRetVal = -16;
			break;
		case -14:
			///<Not supported
			nRetVal = -17;
			break;
		default:
			break;
		}
	}
#ifdef RECORD_TIME
	CTimer::Instance()->Stop();
	CTimer::Instance()->Stop();
	CTimer::Instance()->Print("D:\\GetCaptureData.csv");
#endif // RECORD_TIME

	return nRetVal;
}

int CDCM::SetCaptureLine(const char* lpszPinGroup, USHORT usSiteNo, const char* lpszStartLabel, ULONG ulOffset, int nLineCount)
{
	m_nCaptureStartOffset = -1;
	if (!m_bLoadVector)
	{
		m_pAlarm->VectorNotLoadedAlarm();
		return -1;
	}

	int nRetVal = GetChannel(lpszPinGroup, usSiteNo);
	if (0 != nRetVal)
	{
		switch (nRetVal)
		{
		case -2:
			//The pin group is nullptr
			nRetVal = -2;
			break;
		case -3:
			//The pin group is not defined before
			nRetVal = -3;
			break;
		case -4:
			///<Site over range
			nRetVal = -4;
			break;
		case -5:
			///<Site invalid
			nRetVal = -5;
			break;
		case -6:
			///<The pin is not belongs to
			nRetVal = -6;
			break;
		default:
			///<Not will happen
			break;
		}
		return nRetVal;
	}

	if(nullptr == lpszStartLabel)
	{
		m_pAlarm->ParameternullptrAlarm("lpszStartLabel", usSiteNo, lpszPinGroup, FALSE);
		return -7;
	}
	int nStartLineNo = m_VectorInfo.GetLabelLineNo(lpszStartLabel, TRUE);
	if (0 > nStartLineNo)
	{
		m_pAlarm->SetParamName("lpszStartLabel");
		m_pAlarm->SetAlarmID(ALARM_ID::ALARM_START_LABEL_ERROR);
		m_pAlarm->SetAlarmType(CDriverAlarm::ALARM_TYPE::PARAMETER_NOT_DEFINED);
		m_pAlarm->SetAlarmMsg("The start label(%s) is not defined in vector file.", lpszStartLabel);
		m_pAlarm->SetAlarmLevel(CDriverAlarm::ALARM_LEVEL::AlarmError);
		return -8;
	}
	int nTotalLineCount = m_VectorInfo.GetBRAMLineCount() + m_VectorInfo.GetDRAMLineCount();

	if (nTotalLineCount <= nStartLineNo + ulOffset)
	{
		m_pAlarm->SetParamName("lpszStartLabel");
		m_pAlarm->SetAlarmID(ALARM_ID::ALARM_OFFSET_OVER_RANGE);
		m_pAlarm->SetAlarmType(CDriverAlarm::ALARM_TYPE::PARAMETER_OVERRANGE);
		m_pAlarm->SetAlarmMsg("The offset(%d) is over range[%d,%d).", ulOffset, 0, nTotalLineCount - nStartLineNo);
		m_pAlarm->SetAlarmLevel(CDriverAlarm::ALARM_LEVEL::AlarmError);
		return -9;
	}
	nStartLineNo += ulOffset;

	if (32 < nLineCount || nTotalLineCount < nStartLineNo + nLineCount)
	{
		m_pAlarm->SetPinString(lpszPinGroup, FALSE);
		m_pAlarm->SetParamName("uLineCount");
		m_pAlarm->SetAlarmID(ALARM_ID::ALARM_LINE_COUNT_OVER_RANGE);
		m_pAlarm->SetAlarmType(CDriverAlarm::ALARM_TYPE::PARAMETER_OVERRANGE);
		int nMaxLineCount = 32 < nLineCount ? 32 : (nTotalLineCount - nStartLineNo);
		m_pAlarm->SetAlarmMsg("The uLineCount(%d) is over range[%d,%d].", ulOffset, 1, nMaxLineCount);
		m_pAlarm->SetAlarmLevel(CDriverAlarm::ALARM_LEVEL::AlarmWarning);
		m_pAlarm->Output(FALSE);

		nLineCount = nMaxLineCount;
	}

	int nStartRanLine = m_VectorInfo.GetGlobalLineNo(m_nLatestStartLine);

	int nStopRanLine =  m_VectorInfo.GetGlobalLineNo(m_nLatestStopLine);
	if (nStartRanLine > nStartLineNo || nStopRanLine < nStartLineNo + nLineCount - 1)
	{
		m_pAlarm->SetPinString(lpszPinGroup, FALSE);
		m_pAlarm->SetParamName("uLineCount");
		m_pAlarm->SetAlarmID(ALARM_ID::ALARM_LINE_COUNT_OVER_RANGE);
		m_pAlarm->SetAlarmType(CDriverAlarm::ALARM_TYPE::PARAMETER_OVERRANGE);
		int nMaxLineCount = 32 < nLineCount ? 32 : (nTotalLineCount - nStartLineNo - nLineCount);
		m_pAlarm->SetAlarmMsg("The capture line range(%d to %d) is not included in latest ran vector(%d to %d).", nStartLineNo + 1, nStartLineNo + nLineCount, nStartRanLine + 1, nStopRanLine + 1);
		m_pAlarm->SetAlarmLevel(CDriverAlarm::ALARM_LEVEL::AlarmError);
		return -10;
	}

	vector<USHORT> vecChannel;
	BOOL bNoBoard = TRUE;
	BOARD_CHANNEL_BEGIN(vecChannel)
	{
		bNoBoard = FALSE;
		nRetVal = iterBoard->second->PreloadFailLineNo(vecChannel, nLineCount);
		if (0 != nRetVal)
		{
			BOOL bExit = TRUE;
			m_pAlarm->SetPinString(lpszPinGroup, FALSE);
			BOOL bAddAlarm = !m_pAlarm->IsSetMsg();
			switch (nRetVal)
			{
			case -1:
				///<Vector not ran
				if (bAddAlarm)
				{
					m_pAlarm->SetAlarmID(ALARM_ID::ALARM_NOT_RAN_VECTOR);
					m_pAlarm->SetAlarmMsg("The vector in board(%d) used in pin group(%s) is not ran.", iterBoard->first, lpszPinGroup);
				}
				nRetVal = -11;
				break;
			case -2:
				///<Vector running
				if (bAddAlarm)
				{
					m_pAlarm->SetAlarmID(ALARM_ID::ALARM_VECTOR_RUNNING);
					m_pAlarm->SetAlarmMsg("The vector in board(%d) used in pin group(%s) is running.", iterBoard->first, lpszPinGroup);
				}
				nRetVal = -12;
				break;
			case -3:
				///<Board not existed
				if (bAddAlarm)
				{
					m_pAlarm->SetAlarmLevel(CDriverAlarm::ALARM_LEVEL::AlarmWarning);
					m_pAlarm->SetAlarmID(ALARM_ID::ALARM_CHANNEL_NOT_EXISTED);
					m_pAlarm->SetAlarmMsg("The board(S%d) used in pin group(%s) is not existed.", iterBoard->first, lpszPinGroup);
					m_pAlarm->Output(FALSE);
				}
				nRetVal = -13;
				bExit = FALSE;
				break;
			default:
				break;
			}
			if (bExit)
			{
				break;
			}
		}
	}
	BOARD_CHANNEL_END
	if (bNoBoard)
	{
		m_pAlarm->SetNoBoardAlarm(nullptr);
		nRetVal = -14;
	}
	if (0 == nRetVal)
	{
		m_nCaptureStartOffset = nStartLineNo - nStartRanLine;
		m_nCaptureLineCount = nLineCount;
	}
	return nRetVal;
}

int CDCM::GetCaptureData(const char* lpszPinName, USHORT usSiteNo, ULONG& ulCaptureData)
{
	ulCaptureData = -1;
	int nRetVal = 0;
	CHANNEL_INFO Channel;
	nRetVal = GetBoardChannel(lpszPinName, usSiteNo, Channel);
	if (0 != nRetVal)
	{
		return nRetVal;
	}
	auto iterBoard = m_mapBoard.find(Channel.m_bySlotNo);
	if (m_mapBoard.end() == iterBoard || nullptr == iterBoard->second || DCM_MAX_CHANNELS_PER_BOARD <= Channel.m_usChannel)
	{
		m_pAlarm->SetNoBoardAlarm(lpszPinName, TRUE, usSiteNo);
		return -7;
	}
	nRetVal = iterBoard->second->IsSavingSelectedFail(Channel.m_usChannel);
	if (1 == nRetVal)
	{
		m_pAlarm->SetParamName("lpszPinName");
		m_pAlarm->SetAlarmLevel(CDriverAlarm::ALARM_LEVEL::AlarmError);
		m_pAlarm->SetAlarmID(ALARM_ID::ALARM_CPAUTE_NOT_SUPPORT);
		m_pAlarm->SetPinString(lpszPinName, TRUE);
		m_pAlarm->SetSite(usSiteNo);
		m_pAlarm->SetAlarmMsg("The channel(S%d_%d) of pin(%s) in SITE_%d is using saving fail selected, not support GetCaptureData.",
			Channel.m_bySlotNo, Channel.m_usChannel, lpszPinName, usSiteNo + 1);
		return -14;
	}

	BOOL bResultReliable = TRUE;///<Whether the result is reliable
	vector<int> vecFailLine;
	int nCertailPassLine = -1;
	nRetVal = GetFailLineNo(Channel.m_bySlotNo, Channel.m_usChannel, -1, vecFailLine, &nCertailPassLine);
	if (0 > nRetVal)
	{
		m_pAlarm->SetParamName("lpszPinName");
		m_pAlarm->SetPinString(lpszPinName, TRUE);
		m_pAlarm->SetSite(usSiteNo);
		BOOL bAddAlarm = !m_pAlarm->IsSetMsg();
		switch (nRetVal)
		{
		case -2:
			///<Channel is over range
			if (bAddAlarm)
			{
				m_pAlarm->SetAlarmID(ALARM_ID::ALARM_CHANNEL_OVER_RANGE);
				m_pAlarm->SetAlarmType(CDriverAlarm::ALARM_TYPE::PARAMETER_OVERRANGE);
				m_pAlarm->SetAlarmMsg("The channel(S%d_%d) of pin(%s) in SITE_%d is over range[0, %d].", Channel.m_bySlotNo, Channel.m_usChannel, lpszPinName, usSiteNo + 1, DCM_MAX_CHANNELS_PER_BOARD);
			}
			nRetVal = -8;
			break;
		case -3:
			///<Channel is not existed
			if (bAddAlarm)
			{
				m_pAlarm->SetAlarmID(ALARM_ID::ALARM_CHANNEL_NOT_EXISTED);
				m_pAlarm->SetAlarmMsg("The channel(S%d_%d) of pin(%s) in SITE_%d is not existed.", Channel.m_bySlotNo, Channel.m_usChannel, lpszPinName, usSiteNo + 1);
				m_pAlarm->SetAlarmLevel(CDriverAlarm::ALARM_LEVEL::AlarmWarning);
				m_pAlarm->Output(FALSE);
			}
			nRetVal = -9;
			break;
		case -4:
			///<Vector not ran
			if (bAddAlarm)
			{
				m_pAlarm->SetAlarmID(ALARM_ID::ALARM_NOT_RAN_VECTOR);
				m_pAlarm->SetAlarmMsg("The vector of channel(S%d_%d) of pin(%s) in SITE_%d is not ran.", Channel.m_bySlotNo, Channel.m_usChannel, lpszPinName, usSiteNo + 1);
			}
			nRetVal = -10;
			break;
		case -5:
			///<Vector running
			if (bAddAlarm)
			{
				m_pAlarm->SetAlarmID(ALARM_ID::ALARM_VECTOR_RUNNING);
				m_pAlarm->SetAlarmMsg("The vector of channel(S%d_%d) of pin(%s) in SITE_%d is running.", Channel.m_bySlotNo, Channel.m_usChannel, lpszPinName, usSiteNo + 1);
			}
			nRetVal = -11;
			break;
		default:
			break;
		}
		return nRetVal;
	}
	else if(0 != nRetVal)
	{
		///<The fail line number later is not certain
		bResultReliable = FALSE;
	}

	if (-1 == m_nCaptureStartOffset)
	{
		m_pAlarm->SetAlarmID(ALARM_ID::ALARM_FUNCTION_USE_ERROR);
		m_pAlarm->SetAlarmMsg("Not set the capture line base line through SetCapture.");
		return -12;
	}

	BOOL bFailDataEnough = FALSE;
	ulCaptureData = 0;
	UINT uStopLine = m_nCaptureStartOffset + m_nCaptureLineCount - 1;
	int nShiftOffset = m_nCaptureLineCount + m_nCaptureStartOffset - 1;
	for (auto LineNo : vecFailLine)
	{
		if (uStopLine <= LineNo)
		{
			if (uStopLine == LineNo)
			{
				///<Current fail line number is the stop line of capture gotten
				ulCaptureData |= 1 << (nShiftOffset - LineNo);
			}
			bFailDataEnough = TRUE;
			break;
		}
		if (m_nCaptureStartOffset <= LineNo)
		{
			ulCaptureData |= 1 << (nShiftOffset - LineNo);
		}
	}
	if (!bFailDataEnough && 0 < nCertailPassLine && nCertailPassLine >= uStopLine)
	{
		///<The last line of the fail memory for current channel is pass, and the line is larger than the stop line
		bFailDataEnough = TRUE;
	}
	if (!bFailDataEnough && !bResultReliable)
	{
		ulCaptureData = -1;
		m_pAlarm->SetAlarmID(ALARM_ID::ALARM_FAIL_LINE_NOT_SAVE);
		if (0 != vecFailLine.size())
		{
			m_pAlarm->SetAlarmMsg("The fail line number of channel(S%d_%d) of pin(%s) in SITE_%d after line %d are not saving.",
				Channel.m_bySlotNo, Channel.m_usChannel, lpszPinName, usSiteNo + 1, vecFailLine[vecFailLine.size() - 1] + 1);
		}
		else
		{
			m_pAlarm->SetAlarmMsg("The fail line number of channel(S%d_%d) of pin(%s) in SITE_%d are not saving.",
				Channel.m_bySlotNo, Channel.m_usChannel, lpszPinName, usSiteNo + 1);
		}
		return -13;
	}
	return 0;
}

int CDCM::GetHardwareCapture(const char* lpszPinName, USHORT usSiteNo, BYTE* pbyCaptureData, int nBuffSize)
{
	if (nullptr != pbyCaptureData || 0 >= nBuffSize)
	{
		memset(pbyCaptureData, -1, nBuffSize * sizeof(BYTE));
	}
	int nRetVal = 0;
	CHANNEL_INFO Channel;
	nRetVal = GetBoardChannel(lpszPinName, usSiteNo, Channel);
	if (0 != nRetVal)
	{
		return nRetVal;
	}
	auto iterBoard = m_mapBoard.find(Channel.m_bySlotNo);
	if (m_mapBoard.end() == iterBoard || nullptr == iterBoard->second || DCM_MAX_CHANNELS_PER_BOARD <= Channel.m_usChannel)
	{
		m_pAlarm->SetNoBoardAlarm(lpszPinName, TRUE, usSiteNo);
		return -7;
	}
	vector<LINE_DATA> vecBRAMCapture;
	vector<LINE_DATA> vecDRAMCapture;
	nRetVal = iterBoard->second->GetCapture(Channel.m_usChannel, vecBRAMCapture, vecDRAMCapture);
	if (0 != nRetVal)
	{
		m_pAlarm->SetParamName("lpszPinName");
		m_pAlarm->SetPinString(lpszPinName, TRUE);
		m_pAlarm->SetSite(usSiteNo);
		BOOL bAddAlarm = !m_pAlarm->IsSetMsg();
		switch (nRetVal)
		{
		case -1:
			///<Channel is over range
			if (bAddAlarm)
			{
				m_pAlarm->SetAlarmID(ALARM_ID::ALARM_CHANNEL_OVER_RANGE);
				m_pAlarm->SetAlarmType(CDriverAlarm::ALARM_TYPE::PARAMETER_OVERRANGE);
				m_pAlarm->SetAlarmMsg("The channel(S%d_%d) of pin(%s) in SITE_%d is over range[0, %d].", Channel.m_bySlotNo, Channel.m_usChannel, lpszPinName, usSiteNo + 1, DCM_MAX_CHANNELS_PER_BOARD);
			}
			nRetVal = -8;
			break;
		case -2:
			///<Channel is not existed
			if (bAddAlarm)
			{
				m_pAlarm->SetAlarmID(ALARM_ID::ALARM_CHANNEL_NOT_EXISTED);
				m_pAlarm->SetAlarmMsg("The channel(S%d_%d) of pin(%s) in SITE_%d is not existed.", Channel.m_bySlotNo, Channel.m_usChannel, lpszPinName, usSiteNo + 1);
			}
			nRetVal = -9;
			break;
		case -3:
			///<Vector not ran
			if (bAddAlarm)
			{
				m_pAlarm->SetAlarmID(ALARM_ID::ALARM_NOT_RAN_VECTOR);
				m_pAlarm->SetAlarmMsg("The vector of channel(S%d_%d) of pin(%s) in SITE_%d is not ran.", Channel.m_bySlotNo, Channel.m_usChannel, lpszPinName, usSiteNo + 1);
			}
			nRetVal = -10;
			break;
		case -4:
			///<Vector running
			if (bAddAlarm)
			{
				m_pAlarm->SetAlarmID(ALARM_ID::ALARM_VECTOR_RUNNING);
				m_pAlarm->SetAlarmMsg("The vector of channel(S%d_%d) of pin(%s) in SITE_%d is running.", Channel.m_bySlotNo, Channel.m_usChannel, lpszPinName, usSiteNo + 1);
			}
			nRetVal = -11;
			break;
		default:
			break;
		}
		return nRetVal;
	}
	if (0 == vecBRAMCapture.size() && 0 == vecDRAMCapture.size())
	{
		m_pAlarm->SetPinString(lpszPinName, TRUE);
		m_pAlarm->SetSite(usSiteNo);
		BOOL bAddAlarm = !m_pAlarm->IsSetMsg();
		if (bAddAlarm)
		{
			m_pAlarm->SetAlarmID(ALARM_ID::ALARM_NO_CAPTURE);
			m_pAlarm->SetAlarmMsg("The vector latest ran of pin(%s) in SITE_%d have no hardware capture.", lpszPinName, usSiteNo + 1);
		}
		return 0;
	}

	if (MAX_CAPTURE_COUNT_OPEN < vecBRAMCapture.size() + vecDRAMCapture.size())
	{
		///<The capture line number is not enough for saving all capture
		m_pAlarm->SetAlarmID(ALARM_ID::ALARM_CAPTURE_NOT_SAVING);
		m_pAlarm->SetAlarmMsg("No all capture of channel(S%d_%d) of pin(%s) in SITE_%d saved for the capture memory limited.",
			Channel.m_bySlotNo, Channel.m_usChannel, lpszPinName, usSiteNo + 1);

		if (nullptr != pbyCaptureData)
		{
			memset(pbyCaptureData, -1, nBuffSize);
		}
		return -12;
	}
	m_VectorInfo.CombineLine(m_nLatestStartLine, m_nLatestStopLine, vecBRAMCapture, vecDRAMCapture);

	int nLineCount = vecBRAMCapture.size();
	if (nullptr == pbyCaptureData || (nLineCount + 7) / 8 > nBuffSize)
	{
		return nLineCount;
	}

	memset(pbyCaptureData, 0, nBuffSize);
	int nLineIndex = 0;
	for (auto& BRAMCapture : vecBRAMCapture)
	{
		BYTE byData = BRAMCapture.m_byData << (7 - nLineIndex % 8);
		pbyCaptureData[nLineIndex / 8] |= byData;
		++nLineIndex;
	}

	///<The last byte is not all valid data, move the dat to low bit.
	BYTE byLastByteBitCount = nLineIndex % 8;
	if (0 != byLastByteBitCount)
	{
		BYTE byRightShift = 8 - byLastByteBitCount;
		pbyCaptureData[nLineIndex / 8] >>= byRightShift;
	}

	return nLineCount;
}

int CDCM::GetMCUResult(USHORT usSiteNo)
{
	if (!m_bLoadVector)
	{
		m_pAlarm->VectorNotLoadedAlarm();
		return -1;
	}
	if (0 == m_strLatestRanPinGroup.size())
	{
		///<Not ran before
		m_pAlarm->SetAlarmID(ALARM_ID::ALARM_NOT_RAN_VECTOR);
		m_pAlarm->SetAlarmMsg("Not ran vector before.");
		return -2;
	}
	int nRetVal = GetChannel(m_strLatestRanPinGroup.c_str(), usSiteNo);
	if (0 != nRetVal)
	{
		switch (nRetVal)
		{
		case -2:
			//The pin group is nullptr, not will happen
			break;
		case -3:
			//The pin group is not defined before, not will happen
			break;
		case -4:
			///<Site over range
			m_pAlarm->SiteOverScaleAlarm(nullptr, usSiteNo, m_Site.GetSiteCount(), FALSE);
			nRetVal = -3;
			break;
		case -5:
			///<Site invalid
			nRetVal = -4;
			break;
		default:
			///<Not will happen
			break;
		}
		return nRetVal;

	}
	vector<USHORT> vecChannel;
	BOOL bNoBoard = TRUE;
	BOARD_CHANNEL_BEGIN(vecChannel)
	{
		bNoBoard = FALSE;
		nRetVal = iterBoard->second->GetMCUResult(vecChannel);
		if (0 != nRetVal)
		{
			if (1 == nRetVal)
			{
				///<Fail
				break;
			}
			BOOL bExit = TRUE;
			m_pAlarm->SetSite(usSiteNo);
			switch (nRetVal)
			{
			case -1:
				///<Not ran vector before
				m_pAlarm->SetAlarmID(ALARM_ID::ALARM_NOT_RAN_VECTOR);
				m_pAlarm->SetAlarmMsg("The vector of SITE_%d is not ran before.", iterBoard->first, usSiteNo + 1);
				m_pAlarm->SetAlarmLevel(CDriverAlarm::ALARM_LEVEL::AlarmError);
				nRetVal = -2;
				break;
			case -2:
				///<Vector running
				m_pAlarm->SetAlarmID(ALARM_ID::ALARM_VECTOR_RUNNING);
				m_pAlarm->SetAlarmMsg("The vector of SITE_%d is running.", iterBoard->first, usSiteNo + 1);
				m_pAlarm->SetAlarmLevel(CDriverAlarm::ALARM_LEVEL::AlarmError);
				nRetVal = -5;
				break;
			case -3:
				///<No valid board
				m_pAlarm->SetAlarmID(ALARM_ID::ALARM_CHANNEL_NOT_EXISTED);
				m_pAlarm->SetAlarmMsg("Some channels in board(S%d) of SITE_%d are not existed.", iterBoard->first, usSiteNo + 1);
				m_pAlarm->SetAlarmLevel(CDriverAlarm::ALARM_LEVEL::AlarmError);
				nRetVal = -6;
				break;
			default:
				break;
			}
			if (bExit)
			{
				break;
			}
		}
		if (0 != nRetVal)
		{
			break;
		}
	}
	BOARD_CHANNEL_END

	if (bNoBoard)
	{
		m_pAlarm->SetNoBoardAlarm(nullptr);
		nRetVal = -7;
	}

	return nRetVal;
}

int CDCM::GetPinMCUResult(const char* lpszPinName, USHORT usSiteNo)
{
	int nRetVal = 0;
	CHANNEL_INFO Channel;
	nRetVal = GetBoardChannel(lpszPinName, usSiteNo, Channel);
	if (0 != nRetVal)
	{
		return nRetVal;
	}
	auto iterBoard = m_mapBoard.find(Channel.m_bySlotNo);

	if (m_mapBoard.end() == iterBoard || nullptr == iterBoard->second)
	{
		m_pAlarm->SetNoBoardAlarm(lpszPinName, TRUE, usSiteNo);
		return -7;
	}
	nRetVal = iterBoard->second->GetChannelResult(Channel.m_usChannel);
	if (0 > nRetVal)
	{
		m_pAlarm->SetParamName("lpszPinName");
		m_pAlarm->SetSite(usSiteNo);
		m_pAlarm->SetPinString(lpszPinName, TRUE);
		switch (nRetVal)
		{
		case -1:
		case -2:
			m_pAlarm->SetAlarmID(ALARM_ID::ALARM_CHANNEL_NOT_EXISTED);
			m_pAlarm->SetAlarmMsg("The channel(S%d_%d) of pin(%s) in SITE_%d is not existed.", Channel.m_bySlotNo, Channel.m_usChannel, lpszPinName, usSiteNo + 1);
			m_pAlarm->SetAlarmLevel(CDriverAlarm::ALARM_LEVEL::AlarmWarning);
			m_pAlarm->Output(FALSE);
			nRetVal = -8;
			break;
		case -3:
			m_pAlarm->SetAlarmID(ALARM_ID::ALARM_NOT_RAN_VECTOR);
			m_pAlarm->SetAlarmMsg("The pin(%s) of SITE_%d is not ran vector before.", lpszPinName, usSiteNo + 1);
			m_pAlarm->SetAlarmLevel(CDriverAlarm::ALARM_LEVEL::AlarmError);
			nRetVal = -9;
			break;
		case -4:
			m_pAlarm->SetAlarmID(ALARM_ID::ALARM_VECTOR_RUNNING);
			m_pAlarm->SetAlarmMsg("The channel in pin(%s) of SITE_%d is running now.", lpszPinName, usSiteNo + 1);
			m_pAlarm->SetAlarmLevel(CDriverAlarm::ALARM_LEVEL::AlarmError);
			nRetVal = -10;
			break;
		default:
			break;
		}
	}
	return nRetVal;
}

int CDCM::GetRunningStatus(const char* lpszPinName, USHORT usSiteNo)
{
	int nRetVal = 0;
	CHANNEL_INFO Channel;
	nRetVal = GetBoardChannel(lpszPinName, usSiteNo, Channel);
	if (0 != nRetVal)
	{
		return nRetVal;
	}
	auto iterBoard = m_mapBoard.find(Channel.m_bySlotNo);
	if (m_mapBoard.end() == iterBoard || nullptr == iterBoard->second || DCM_MAX_CHANNELS_PER_BOARD <= Channel.m_usChannel)
	{
		m_pAlarm->SetNoBoardAlarm(lpszPinName, TRUE, usSiteNo);
		return -7;
	}

	nRetVal = iterBoard->second->GetRunningStatus(Channel.m_usChannel);

	if (0 > nRetVal)
	{
		m_pAlarm->SetParamName("lpszPinName");
		m_pAlarm->SetSite(usSiteNo);
		m_pAlarm->SetPinString(lpszPinName, TRUE);
		switch (nRetVal)
		{
		case -1:
		case -2:
			m_pAlarm->SetAlarmID(ALARM_ID::ALARM_CHANNEL_NOT_EXISTED);
			m_pAlarm->SetAlarmMsg("The channel(S%d_%d) of pin(%s) in SITE_%d is not existed.", Channel.m_bySlotNo, Channel.m_usChannel, lpszPinName, usSiteNo + 1);
			m_pAlarm->SetAlarmLevel(CDriverAlarm::ALARM_LEVEL::AlarmWarning);
			m_pAlarm->Output(FALSE);
			nRetVal = -8;
			break;
		default:
			break;
		}
	}
	return nRetVal;
}

int CDCM::GetFailCount(const char* lpszPinName, USHORT usSiteNo)
{
	int nRetVal = 0;
	CHANNEL_INFO Channel;
	nRetVal = GetBoardChannel(lpszPinName, usSiteNo, Channel);
	if (0 != nRetVal)
	{
		return nRetVal;
	}
	auto iterBoard = m_mapBoard.find(Channel.m_bySlotNo);
	if (m_mapBoard.end() == iterBoard || nullptr == iterBoard->second || DCM_MAX_CHANNELS_PER_BOARD <= Channel.m_usChannel)
	{
		m_pAlarm->SetNoBoardAlarm(lpszPinName, TRUE, usSiteNo);
		return -7;
	}
	nRetVal = iterBoard->second->GetFailCount(Channel.m_usChannel);
	if (0 > nRetVal)
	{
		m_pAlarm->SetParamName("lpszPinName");
		m_pAlarm->SetSite(usSiteNo);
		m_pAlarm->SetPinString(lpszPinName, TRUE);
		switch (nRetVal)
		{
		case -1:
		case -2:
			m_pAlarm->SetAlarmID(ALARM_ID::ALARM_CHANNEL_NOT_EXISTED);
			m_pAlarm->SetAlarmMsg("The channel(S%d_%d) of pin(%s) in SITE_%d is not existed.", Channel.m_bySlotNo, Channel.m_usChannel, lpszPinName, usSiteNo + 1);
			m_pAlarm->SetAlarmLevel(CDriverAlarm::ALARM_LEVEL::AlarmWarning);
			m_pAlarm->Output(FALSE);
			nRetVal = -8;
			break;
		case -3:
			m_pAlarm->SetAlarmID(ALARM_ID::ALARM_NOT_RAN_VECTOR);
			m_pAlarm->SetAlarmMsg("The pin(%s) of SITE_%d is not ran vector before.", lpszPinName, usSiteNo + 1);
			m_pAlarm->SetAlarmLevel(CDriverAlarm::ALARM_LEVEL::AlarmError);
			nRetVal = -9;
			break;
		case -4:
			m_pAlarm->SetAlarmID(ALARM_ID::ALARM_VECTOR_RUNNING);
			m_pAlarm->SetAlarmMsg("The channel in pin(%s) of SITE_%d is running now.", lpszPinName, usSiteNo + 1);
			m_pAlarm->SetAlarmLevel(CDriverAlarm::ALARM_LEVEL::AlarmError);
			nRetVal = -10;
			break;
		default:
			break;
		}
	}
	return nRetVal;
}

int CDCM::GetFirstFailLineNo(const char* lpszPinName, USHORT usSiteNo)
{
	int nRetVal = 0;
	CHANNEL_INFO Channel;
	nRetVal = GetBoardChannel(lpszPinName, usSiteNo, Channel);
	if (0 != nRetVal)
	{
		return nRetVal;
	}
	auto iterBoard = m_mapBoard.find(Channel.m_bySlotNo);
	if (m_mapBoard.end() == iterBoard || nullptr == iterBoard->second || DCM_MAX_CHANNELS_PER_BOARD <= Channel.m_usChannel)
	{
		m_pAlarm->SetNoBoardAlarm(lpszPinName, TRUE, usSiteNo);
		return -7;
	}

	nRetVal = iterBoard->second->GetFailCount(Channel.m_usChannel);
	if (0 > nRetVal)
	{
		m_pAlarm->SetParamName("lpszPinName");
		m_pAlarm->SetSite(usSiteNo);
		m_pAlarm->SetPinString(lpszPinName, TRUE);
		switch (nRetVal)
		{
		case -1:
			m_pAlarm->SetAlarmID(ALARM_ID::ALARM_CHANNEL_NOT_EXISTED);
			m_pAlarm->SetAlarmMsg("The channel(S%d_%d) of pin(%s) in SITE_%d is over range.", Channel.m_bySlotNo, Channel.m_usChannel, lpszPinName, usSiteNo + 1);
			m_pAlarm->SetAlarmLevel(CDriverAlarm::ALARM_LEVEL::AlarmWarning);
			m_pAlarm->Output(FALSE);
			nRetVal = -8;
			break;
		case -2:
			m_pAlarm->SetAlarmID(ALARM_ID::ALARM_CHANNEL_NOT_EXISTED);
			m_pAlarm->SetAlarmMsg("The channel(S%d_%d) of pin(%s) in SITE_%d is not existed.", Channel.m_bySlotNo, Channel.m_usChannel, lpszPinName, usSiteNo + 1);
			m_pAlarm->SetAlarmLevel(CDriverAlarm::ALARM_LEVEL::AlarmWarning);
			m_pAlarm->Output(FALSE);
			nRetVal = -9;
			break;
		case -3:
			m_pAlarm->SetAlarmID(ALARM_ID::ALARM_NOT_RAN_VECTOR);
			m_pAlarm->SetAlarmMsg("The pin(%s) of SITE_%d is not ran vector before.", lpszPinName, usSiteNo + 1);
			m_pAlarm->SetAlarmLevel(CDriverAlarm::ALARM_LEVEL::AlarmError);
			nRetVal = -10;
		case -4:
			m_pAlarm->SetAlarmID(ALARM_ID::ALARM_VECTOR_RUNNING);
			m_pAlarm->SetAlarmMsg("The channel in pin(%s) of SITE_%d is running now.", lpszPinName, usSiteNo + 1);
			m_pAlarm->SetAlarmLevel(CDriverAlarm::ALARM_LEVEL::AlarmError);
			nRetVal = -11;
			break;
			break;
		default:
			break;
		}
	}
	else if (0 == nRetVal)
	{
		///<No fail line number
		nRetVal = -12;
	}
	else
	{
		int nFailCount = nRetVal;
		vector<int> vecLineNo;
		nRetVal = GetFailLineNo(Channel.m_bySlotNo, Channel.m_usChannel, 1, vecLineNo);
		if (0 > nRetVal)
		{
			///<Not will happen
		}
		else
		{
			if (0 != nRetVal)
			{
				m_pAlarm->SetPinString(lpszPinName, TRUE);
				m_pAlarm->SetAlarmLevel(CDriverAlarm::ALARM_LEVEL::AlarmWarning);
				m_pAlarm->SetAlarmID(ALARM_ID::ALARM_FAIL_LINE_NOT_SAVE);
				m_pAlarm->SetAlarmMsg("The fail line memory of the channel(S%d_%d) in pin(%s) at SITE_%d is filled, some fail line number may not be saved",
					Channel.m_bySlotNo, Channel.m_usChannel, lpszPinName, usSiteNo + 1);
			}
			if (0 != vecLineNo.size())
			{
				nRetVal = vecLineNo[0];
			}
			else
			{
				m_pAlarm->SetPinString(lpszPinName, TRUE);
				m_pAlarm->SetAlarmLevel(CDriverAlarm::ALARM_LEVEL::AlarmError);
				m_pAlarm->SetAlarmID(ALARM_ID::ALARM_FAIL_LINE_NOT_SAVE);
				m_pAlarm->SetAlarmMsg("The channel(S%d_%d) of pin(%s) in SITE_%d have %d line fail, but the line number can't be gotten,\
                       for the fail memory are fully occupied by other channels' fail line",
					Channel.m_bySlotNo, Channel.m_usChannel, lpszPinName, usSiteNo + 1, nFailCount);
				nRetVal = -13;
			}
		}
	}
	return nRetVal;
}

int CDCM::GetFailLineNo(const char* lpszPinName, USHORT usSiteNo, UINT uGetMaxFailCount, std::vector<int>& vecLineNo)
{
	vecLineNo.clear();
	int nRetVal = 0;
	CHANNEL_INFO Channel;
	nRetVal = GetBoardChannel(lpszPinName, usSiteNo, Channel);
	if (0 != nRetVal)
	{
		return nRetVal;
	}
	auto iterBoard = m_mapBoard.find(Channel.m_bySlotNo);
	if (m_mapBoard.end() == iterBoard || nullptr == iterBoard->second || DCM_MAX_CHANNELS_PER_BOARD <= Channel.m_usChannel)
	{
		m_pAlarm->SetNoBoardAlarm(lpszPinName, TRUE, usSiteNo);
		return -7;
	}

	nRetVal = GetFailLineNo(Channel.m_bySlotNo, Channel.m_usChannel, uGetMaxFailCount, vecLineNo);
	if (0 > nRetVal)
	{
		vecLineNo.clear();
		m_pAlarm->SetParamName("lpszPinName");
		m_pAlarm->SetPinString(lpszPinName, TRUE);
		m_pAlarm->SetSite(usSiteNo);
		BOOL bAddAlarm = !m_pAlarm->IsSetMsg();
		switch (nRetVal)
		{
		case -2:
			///<Channel is over range
			if (bAddAlarm)
			{
				m_pAlarm->SetAlarmID(ALARM_ID::ALARM_CHANNEL_OVER_RANGE);
				m_pAlarm->SetAlarmType(CDriverAlarm::ALARM_TYPE::PARAMETER_OVERRANGE);
				m_pAlarm->SetAlarmMsg("The channel(S%d_%d) of pin(%s) in SITE_%d is over range[0, %d].", 
					Channel.m_bySlotNo, Channel.m_usChannel, lpszPinName, usSiteNo + 1, DCM_MAX_CHANNELS_PER_BOARD);
			}
			nRetVal = -8;
			break;
		case -3:
			///<Channel is not existed
			if (bAddAlarm)
			{
				m_pAlarm->SetAlarmID(ALARM_ID::ALARM_CHANNEL_NOT_EXISTED);
				m_pAlarm->SetAlarmMsg("The channel(S%d_%d) of pin(%s) in SITE_%d is not existed.",
					Channel.m_bySlotNo, Channel.m_usChannel, lpszPinName, usSiteNo + 1);
				m_pAlarm->SetAlarmLevel(CDriverAlarm::ALARM_LEVEL::AlarmWarning);
				m_pAlarm->Output(FALSE);
			}
			nRetVal = -9;
			break;
		case -4:
			///<Vector not ran
			if (bAddAlarm)
			{
				m_pAlarm->SetAlarmID(ALARM_ID::ALARM_NOT_RAN_VECTOR);
				m_pAlarm->SetAlarmMsg("The vector of channel(S%d_%d) of pin(%s) in SITE_%d is not ran.", 
					Channel.m_bySlotNo, Channel.m_usChannel, lpszPinName, usSiteNo + 1);
			}
			nRetVal = -10;
			break;
		case -5:
			///<Vector running
			if (bAddAlarm)
			{
				m_pAlarm->SetAlarmID(ALARM_ID::ALARM_VECTOR_RUNNING);
				m_pAlarm->SetAlarmMsg("The vector of channel(S%d_%d) of pin(%s) in SITE_%d is running.",
					Channel.m_bySlotNo, Channel.m_usChannel, lpszPinName, usSiteNo + 1);
			}
			nRetVal = -11;
			break;
		default:
			break;
		}
		return nRetVal;
	}
	if (0 != nRetVal)
	{
		m_pAlarm->SetSite(usSiteNo);
		m_pAlarm->SetPinString(lpszPinName, TRUE);
		m_pAlarm->SetAlarmLevel(CDriverAlarm::ALARM_LEVEL::AlarmError);
		m_pAlarm->SetAlarmID(ALARM_ID::ALARM_FAIL_LINE_NOT_SAVE);
		m_pAlarm->SetAlarmMsg("The fail line memory of the channel(S%d_%d) in pin(%s) at SITE_%d is filled, some fail line number may not be saved.",
			Channel.m_bySlotNo, Channel.m_usChannel, lpszPinName, usSiteNo + 1);
		nRetVal = -12;
	}
	return nRetVal;
}

int CDCM::SaveFailMap(UINT uMaxFailLine)
{
	if (!m_bLoadVector)
	{
		m_pAlarm->VectorNotLoadedAlarm();
		return -1;
	}
	if (0 == m_strLatestRanPinGroup.size())
	{
		///<Not ran before
		m_pAlarm->SetAlarmID(ALARM_ID::ALARM_NOT_RAN_VECTOR);
		m_pAlarm->SetAlarmMsg("Not ran vector before.");
		return -2;
	}

	GetChannel(m_strLatestRanPinGroup.c_str());

	if (0 == uMaxFailLine)
	{
		uMaxFailLine = -1;
	}

	map<USHORT, std::vector<int>> mapFail;
	map<USHORT, int> mapCheckFailIndex;
	vector<USHORT> vecChannelID;
	vector<USHORT> vecChannel;
	vector<int> vecFailLine;
	UINT uMinFailLineNo = 0x7FFFFFFF;
	UINT uMaxFailLineNo = 0;
	int nRetVal = 0;
	BOOL bCurBoardInvalid = FALSE;
	set<BYTE> setBoard;
	m_ClassifyBoard.GetBoard(setBoard);
	for (auto Board : setBoard)
	{
		auto iterBoard = m_mapBoard.find(Board);
		if (m_mapBoard.end() == iterBoard)
		{
			continue;
		}
		m_ClassifyBoard.GetBoardChannel(Board, vecChannel, &vecChannelID);

		USHORT usChannelCount = vecChannel.size();
		for (USHORT usChannel = 0; usChannel < usChannelCount; ++usChannel)
		{
			nRetVal = GetFailLineNo(Board, vecChannel[usChannel], uMaxFailLine, vecFailLine);
			if (0 > nRetVal)
			{
				switch (nRetVal)
				{
				case -1:
					///<Board not existed
					bCurBoardInvalid = TRUE;
					break;
				case -2:
					///<Channel not existed
					continue;
				case -3:
					///<Not ran vector
					continue;
				case -4:
					///<Vector running
					m_pAlarm->SetAlarmID(ALARM_ID::ALARM_VECTOR_RUNNING);
					m_pAlarm->SetAlarmMsg("The channel(S%d_%d) is running now.", Board, vecChannel[usChannel]);
					return -3;
					break;
				case -5:
					///<Not ran
					continue;
				default:
					break;
				}
				if (bCurBoardInvalid)
				{
					break;
				}
			}
			if (0 == vecFailLine.size())
			{
				continue;
			}
			mapFail.insert(make_pair(vecChannelID[usChannel], vecFailLine));
			mapCheckFailIndex.insert(make_pair(vecChannelID[usChannel], 0));
			if (uMinFailLineNo > vecFailLine[0])
			{
				uMinFailLineNo = vecFailLine[0];
			}
			UINT uFailCount = vecFailLine.size();
			if (uMaxFailLineNo < vecFailLine[uFailCount - 1])
			{
				uMaxFailLineNo = vecFailLine[uFailCount - 1];
			}
		}
	}

	string strFailmapPath;
	{
		char lpszFile[MAX_PATH] = { 0 };
		GetModuleFileName(nullptr, lpszFile, sizeof(lpszFile));
		strFailmapPath += lpszFile;
		int nPos = strFailmapPath.rfind("\\");
		if (-1 != nPos)
		{
			strFailmapPath.erase(nPos + 1);
		}
	}
	strFailmapPath += "fail.map";

	STSFile FailMSGFile;
	bool bOk = FailMSGFile.Open(strFailmapPath.c_str(), STSIODevice::WriteOnly);
	if (!bOk)
	{
	}
	STSDataStream DataStream(&FailMSGFile);
	ACVFailMapHeaderInfo FailHead;
	FailHead.fileName = m_VectorInfo.GetVectorFileName();
	string strLabelName;
	m_VectorInfo.GetLabelName(m_nLatestStartLine, strLabelName);
	FailHead.labelMark = strLabelName.c_str();

	char lpszBuff[256] = { 0 };
	m_VectorInfo.GetID(lpszBuff, sizeof(lpszBuff));
	FailHead.fileID = lpszBuff;
	m_VectorInfo.GetSaveMark(lpszBuff, sizeof(lpszBuff));
	FailHead.saveMark = lpszBuff;

	for (auto& FailChannel : mapFail)
	{
		FailHead.channels.Append(FailChannel.first);
	}
	FailHead.Save(DataStream);
	if (0 == mapFail.size())
	{
		FailHead.EndSave(DataStream, 0);
		FailMSGFile.Close();
		return 0;
	}

	char* pcFailMsg = nullptr;
	int nFailMsgSize = (mapFail.size() + 7) / 8;
	try
	{
		pcFailMsg = new char[nFailMsgSize];
		memset(pcFailMsg, 0, nFailMsgSize * sizeof(char));
	}
	catch (const std::exception&)
	{
		m_pAlarm->AllocateMemoryError();
		return -4;
	}
	UINT uFailCount = 0;
	BOOL bHaveFail = FALSE;
	USHORT usChannelIndex = 0;
	auto iterCheckFailIndex = mapCheckFailIndex.begin();
	for (UINT uFailIndex = uMinFailLineNo; uFailIndex <= uMaxFailLineNo; ++uFailIndex)
	{
		bHaveFail = FALSE;
		memset(pcFailMsg, 0, nFailMsgSize * sizeof(char));
		usChannelIndex = 0;
		for (auto& FailChannel : mapFail)
		{
			iterCheckFailIndex = mapCheckFailIndex.find(FailChannel.first);
			if (mapCheckFailIndex.end() == iterCheckFailIndex)
			{
				++usChannelIndex;
				continue;
			}
			auto pvecFailLine = &FailChannel.second;
			if (pvecFailLine->size() <= iterCheckFailIndex->second || uFailIndex != pvecFailLine->at(iterCheckFailIndex->second))
			{
				++usChannelIndex;
				continue;
			}
			bHaveFail = TRUE;
			pcFailMsg[usChannelIndex / 8] |= 1 << (usChannelIndex % 8);
			++iterCheckFailIndex->second;
			++usChannelIndex;
		}
		if (!bHaveFail)
		{
			continue;
		}

		++uFailCount;
		FailHead.SetLabelOffsetRow(uFailIndex);
		for (int nIndex = 0; nIndex < nFailMsgSize; ++nIndex)
		{
			FailHead.SetChannelValidState(pcFailMsg[nIndex]);
		}
		FailHead.SaveData(DataStream);
	}
	if (0 == uFailCount)
	{
		m_pAlarm->SetAlarmLevel(CDriverAlarm::ALARM_LEVEL::AlarmError);
		m_pAlarm->SetAlarmID(ALARM_ID::ALARM_FAIL_LINE_NOT_SAVE);
		m_pAlarm->SetAlarmMsg("Fail line existed in valid site. But the fail memory are fully occupied by invalid sites'");
		nRetVal = -4;
	}
	FailHead.EndSave(DataStream, uFailCount);
	FailMSGFile.Close();
	if (nullptr != pcFailMsg)
	{
		delete[] pcFailMsg;
		pcFailMsg = nullptr;
	}

	return 0;
}

int CDCM::GetStopLineNo(USHORT usSiteNo)
{
	if (!m_bLoadVector)
	{
		m_pAlarm->VectorNotLoadedAlarm();
		return -1;
	}
	if (m_Site.GetSiteCount() <= usSiteNo)
	{
		m_pAlarm->SiteOverScaleAlarm(nullptr, usSiteNo, m_Site.GetSiteCount() - 1, FALSE);
		return -2;
	}
	if (!m_Site.IsSiteValid(usSiteNo))
	{
		//m_pAlarm->SiteInvalidAlarm(nullptr, usSiteNo);
		return -3;
	}
	vector<CHANNEL_INFO> vecBoardChannel;
	int nRetVal = m_Site.GetChannel(usSiteNo, vecBoardChannel);
	ClassifyChannel(vecBoardChannel);
	vector<USHORT> vecChannel;

	BOOL bNoBoard = TRUE;
	int nStopLineNo = 0;

	BOARD_CHANNEL_BEGIN(vecChannel)
	{
		bNoBoard = FALSE;
		for (auto usChannel : vecChannel)
		{
			nRetVal = iterBoard->second->GetStopLineNo(usChannel);
			if (0 > nRetVal)
			{
				BOOL bExit = TRUE;
				m_pAlarm->SetParamName("usSiteNo");
				m_pAlarm->SetSite(usSiteNo);
				string strPinName;
				CHANNEL_INFO Channel;
				Channel.m_bySlotNo = iterBoard->first;
				Channel.m_usChannel = usChannel;
				m_Site.GetPinName(usSiteNo, Channel, strPinName);
				switch (nRetVal)
				{
				case -1:
					///<The channel is over range
					m_pAlarm->SetAlarmID(ALARM_ID::ALARM_CHANNEL_OVER_RANGE);
					m_pAlarm->SetAlarmMsg("The channel(S%d_%d) of %s in SITE_%d is over range.",
						iterBoard->first, usChannel, strPinName.c_str(), usSiteNo + 1);
					nRetVal = -4;
					break;
				case -2:
					///<The channel is not existed
					m_pAlarm->SetAlarmID(ALARM_ID::ALARM_CHANNEL_NOT_EXISTED);
					m_pAlarm->SetAlarmLevel(CDriverAlarm::ALARM_LEVEL::AlarmWarning);
					m_pAlarm->SetAlarmMsg("The channel(S%d_%d) in SITE_%d is not existed.",
						iterBoard->first, usChannel, usSiteNo + 1);
					m_pAlarm->Output(FALSE);
					bExit = FALSE;
					nRetVal = -4;
					break;
				case -3:
					nRetVal = -5;
					continue;
					break;
				case -4:
					m_pAlarm->SetAlarmID(ALARM_ID::ALARM_VECTOR_RUNNING);
					m_pAlarm->SetAlarmMsg("The channel in pin(%s) of SITE_%d is running now.",
						strPinName.c_str(), usSiteNo + 1);
					m_pAlarm->SetAlarmLevel(CDriverAlarm::ALARM_LEVEL::AlarmError);
					nRetVal = -6;
					break;
				default:
					break;
				}
				if (bExit)
				{
					break;
				}
			}
			else
			{
				break;
			}
		}
		if (0 < nRetVal)
		{
			break;
		}
	
	}
	BOARD_CHANNEL_END

	if (bNoBoard)
	{
		m_pAlarm->SetNoBoardAlarm(nullptr);
		nRetVal = -7;
	}
	if (0 <= nRetVal)
	{
		int nStopLineNo = m_VectorInfo.GetGlobalLineNo(nRetVal);
		nRetVal = nStopLineNo;
	}
	else if(-5 == nRetVal)
	{
		m_pAlarm->SetAlarmID(ALARM_ID::ALARM_NOT_RAN_VECTOR);
		m_pAlarm->SetParamName("usSiteNo");
		m_pAlarm->SetAlarmMsg("Not ran vector in SITE_%d.", usSiteNo + 1);
		m_pAlarm->SetAlarmLevel(CDriverAlarm::ALARM_LEVEL::AlarmError);
	}

	return nRetVal;
}

int CDCM::GetRunLineCount(const char* lpszPinName, USHORT usSiteNo, ULONG& ulLineCount)
{
	ulLineCount = -1;
	int nRetVal = 0;
	CHANNEL_INFO Channel;
	nRetVal = GetBoardChannel(lpszPinName, usSiteNo, Channel);
	if (0 != nRetVal)
	{
		return nRetVal;
	}
	auto iterBoard = m_mapBoard.find(Channel.m_bySlotNo);
	if (m_mapBoard.end() == iterBoard || nullptr == iterBoard->second)
	{
		m_pAlarm->SetNoBoardAlarm(lpszPinName, TRUE, usSiteNo);
		return -7;
	}
	nRetVal = iterBoard->second->GetRunLineCount(Channel.m_usChannel, ulLineCount);
	if (0 != nRetVal)
	{
		m_pAlarm->SetParamName("lpszPinName");
		m_pAlarm->SetPinString(lpszPinName, TRUE);
		m_pAlarm->SetSite(usSiteNo);
		BOOL bAddAlarm = !m_pAlarm->IsSetMsg();
		switch (nRetVal)
		{
		case -1:
			if (bAddAlarm)
			{
				m_pAlarm->SetAlarmID(ALARM_ID::ALARM_CHANNEL_OVER_RANGE);
				m_pAlarm->SetAlarmType(CDriverAlarm::ALARM_TYPE::PARAMETER_OVERRANGE);
				m_pAlarm->SetParamName("lpszPinName");
				m_pAlarm->SetAlarmMsg("The channel(S%d_%d) in pin(%s) of SITE_%d is over range[%d,%d].",
					Channel.m_bySlotNo, Channel.m_usChannel, lpszPinName, usSiteNo + 1, 0, DCM_MAX_CHANNELS_PER_BOARD - 1);
			}
			nRetVal = -8;
			break;
		case -2:
			if (bAddAlarm)
			{
				m_pAlarm->SetAlarmID(ALARM_ID::ALARM_CHANNEL_NOT_EXISTED);
				m_pAlarm->SetAlarmType(CDriverAlarm::ALARM_TYPE::PARAMETER_OVERRANGE);
				m_pAlarm->SetAlarmLevel(CDriverAlarm::ALARM_LEVEL::AlarmWarning);
				m_pAlarm->SetParamName("lpszPinName");
				m_pAlarm->SetAlarmMsg("The channel(S%d_%d) in pin(%s) of SITE_%d is not existed.",
					Channel.m_bySlotNo, Channel.m_usChannel, lpszPinName, usSiteNo + 1);
				m_pAlarm->Output(FALSE);
			}
			nRetVal = -9;
			break;
		}
	}
	return nRetVal;
}

int CDCM::SetPinDynamicLoad(const char* lpszPinName, USHORT usSiteNo, BOOL bEnable, double dIOH, double dIOL,
	double dVTVoltValue, double dClampHighVoltValue, double dClampLowVoltValue)
{
	int nRetVal = 0;
	CHANNEL_INFO Channel;
	nRetVal = GetBoardChannel(lpszPinName, usSiteNo, Channel);
	if (0 != nRetVal)
	{
		return nRetVal;
	}
	auto iterBoard = m_mapBoard.find(Channel.m_bySlotNo);
	if (m_mapBoard.end() == iterBoard || nullptr == iterBoard->second)
	{
		m_pAlarm->SetNoBoardAlarm(lpszPinName, TRUE, usSiteNo);
		return -7;
	}
	vector<USHORT> vecChannel;
	vecChannel.push_back(Channel.m_usChannel);
	nRetVal = iterBoard->second->SetDynamicLoad(vecChannel, bEnable, dIOH, dIOL, dVTVoltValue, dClampHighVoltValue, dClampLowVoltValue);
	if (0 != nRetVal)
	{
		m_pAlarm->SetParamName("lpszPinName");
		m_pAlarm->SetPinString(lpszPinName, TRUE);
		m_pAlarm->SetSite(usSiteNo);
		BOOL bAddAlarm = !m_pAlarm->IsSetMsg();
		switch (nRetVal)
		{
		case -1:
			if (bAddAlarm)
			{
				m_pAlarm->SetAlarmID(ALARM_ID::ALARM_CHANNEL_NOT_EXISTED);
				m_pAlarm->SetAlarmType(CDriverAlarm::ALARM_TYPE::PARAMETER_OVERRANGE);
				m_pAlarm->SetAlarmLevel(CDriverAlarm::ALARM_LEVEL::AlarmWarning);
				m_pAlarm->SetParamName("lpszPinName");
				m_pAlarm->SetAlarmMsg("The channel(S%d_%d) in pin(%s) of SITE_%d is not existed.", 
					Channel.m_bySlotNo, Channel.m_usChannel, lpszPinName, usSiteNo + 1);
				m_pAlarm->Output(FALSE);
			}
			nRetVal = -8;
			break;
		case -2:
			if (bAddAlarm)
			{
				m_pAlarm->SetAlarmID(ALARM_ID::ALARM_CURRENT_ERROR);
				m_pAlarm->SetParamName("dIOH");
				m_pAlarm->SetAlarmMsg("The output current of High(%.1f) or LOW(%.1f) is over range.", dIOH, dIOL);
			}
			nRetVal = -9;
			break;
		case -3:
			if (bAddAlarm)
			{
				m_pAlarm->SetAlarmID(ALARM_ID::ALARM_CURRENT_ERROR);
				m_pAlarm->SetParamName("dVTVoltValue");
				m_pAlarm->SetAlarmMsg("The VT(%5.1f) is over range.", dVTVoltValue);
			}
			nRetVal = -10;
			break;
		case -4:
			if (bAddAlarm)
			{
				m_pAlarm->SetAlarmID(ALARM_ID::ALARM_CLAMP_VALUE_OVER_RANGE);
				m_pAlarm->SetParamName("dClampHighVoltValue");
				m_pAlarm->SetAlarmMsg("The clamp high(%5.1f) or clamp low(%.1f) is over range.", dClampHighVoltValue, dClampLowVoltValue);
			}
			nRetVal = -11;
			break;
		default:
			break;
		}
	}
	return nRetVal;
}

int CDCM::SetDynamicLoad(const char* lpszPinGroup, BOOL bEnable, double dIOH, double dIOL, double dVTVoltValue, double dClampHighVoltValue, double dClampLowVoltValue)
{
	if (!m_bLoadVector)
	{
		return -1;
	}
	int nRetVal = GetChannel(lpszPinGroup);
	if (0 != nRetVal)
	{
		switch (nRetVal)
		{
		case -2:
			//The pin group is nullptr
			nRetVal = -2;
			break;
		case -3:
			//The pin group is not defined before
			nRetVal = -3;
			break;
		case -5:
			///<No valid site
			nRetVal = -4;
			break;
		case -6:
			///<The pin is not belongs to
			nRetVal = -5;
			break;
		default:
			///<Not will happen
			break;
		}
		return nRetVal;
	}

	vector<USHORT> vecChannel;
	BOOL bNoBoard = TRUE;
	BOARD_CHANNEL_BEGIN(vecChannel)
	{
		bNoBoard = FALSE;
		nRetVal = iterBoard->second->SetDynamicLoad(vecChannel, bEnable, dIOH, dIOL, dVTVoltValue, dClampHighVoltValue, dClampLowVoltValue);
		if (0 != nRetVal)
		{
			BOOL bExit = TRUE;
			m_pAlarm->SetPinString(lpszPinGroup, TRUE);
			BOOL bAddAlarm = !m_pAlarm->IsSetMsg();
			switch (nRetVal)
			{
			case -1:
				if (bAddAlarm)
				{
					m_pAlarm->SetAlarmLevel(CDriverAlarm::ALARM_LEVEL::AlarmWarning);
					m_pAlarm->SetAlarmID(ALARM_ID::ALARM_CHANNEL_NOT_EXISTED);
					m_pAlarm->SetAlarmMsg("The channel of board(S%d) used in pin group(%s) is not existed.", iterBoard->first, lpszPinGroup);
					m_pAlarm->Output(FALSE);
				}
				bExit = FALSE;
				nRetVal = -6;
				break;
			case -2:
				if (bAddAlarm)
				{
					m_pAlarm->SetAlarmID(ALARM_ID::ALARM_CURRENT_ERROR);
					m_pAlarm->SetParamName("dIOH");
					m_pAlarm->SetAlarmMsg("The output current of High(%.1f) or LOW(%.1f) is over range.", dIOH, dIOL);
				}
				nRetVal = -7;
				break;
			case -3:
				if (bAddAlarm)
				{
					m_pAlarm->SetAlarmID(ALARM_ID::ALARM_CURRENT_ERROR);
					m_pAlarm->SetParamName("dVTVoltValue");
					m_pAlarm->SetAlarmMsg("The VT(%5.1f) is over range.", dVTVoltValue);
				}
				nRetVal = -8;
				break;
			case -4:
				if (bAddAlarm)
				{
					m_pAlarm->SetAlarmID(ALARM_ID::ALARM_CLAMP_VALUE_OVER_RANGE);
					m_pAlarm->SetParamName("dClampHighVoltValue");
					m_pAlarm->SetAlarmMsg("The clamp high(%5.1f) or clamp low(%.1f) is over range.", dClampHighVoltValue, dClampLowVoltValue);
				}
				nRetVal = -9;
				break;
			default:
				break;
			}
			if (bExit)
			{
				break;
			}
		}
	}
	BOARD_CHANNEL_END
	if (bNoBoard)
	{
		m_pAlarm->SetNoBoardAlarm(lpszPinGroup, FALSE);
		nRetVal = -10;
	}
	return nRetVal;
}

int CDCM::GetDynamicLoad(BYTE bySlotNo, USHORT usChannel, BOOL& bEnable, double& dIOH, double& dIOL)
{
	auto iterBoard = m_mapBoard.find(bySlotNo);
	if (m_mapBoard.end() == iterBoard || nullptr == iterBoard->second)
	{
		///<The board is not existed
		return -1;
	}
	int nRetVal = 0;
	nRetVal = iterBoard->second->GetDynamicLoad(usChannel, bEnable, dIOH, dIOL);
	if (0 != nRetVal)
	{
		switch (nRetVal)
		{
		case -1:
			///<The channel is over range
			nRetVal = -2;
			break;
		case -2:
			///<The channel is not existed
			nRetVal = -3;
			break;
		default:
			break;
		}
	}
	return nRetVal;
}

int CDCM::SetPMUMode(const char* lpszPinGroup, USHORT usSiteNo, PMU_MODE PMUMode, PMU_IRANGE Range, double dSetValue, double dClmapHigh, double dClampLow)
{
	if (!m_bLoadVector)
	{
		m_pAlarm->VectorNotLoadedAlarm();
		return -1;
	}

	int nRetVal = GetChannel(lpszPinGroup, usSiteNo);
	if (0 != nRetVal)
	{
		switch (nRetVal)
		{
		case -2:
			//The pin group is nullptr
			nRetVal = -2;
			break;
		case -3:
			//The pin group is not defined before
			nRetVal = -3;
			break;
		case -4:
			///<The site number is over range
			nRetVal = -4;
			break;
		case -5:
			///<The site is invalid
			nRetVal = -5;
			break;
		case -6:
			///<The pin is not belongs to
			nRetVal = -6;
			break;
		default:
			///<Not will happen
			break;
		}
		return nRetVal;
	}

	vector<USHORT> vecChannel;
	BOOL bNoBoard = TRUE;

	map<BYTE, set<USHORT>> mapShieldChannel;
	auto iterShieldChannel = mapShieldChannel.begin();
	GetShieldChannel("SetPMUMode", mapShieldChannel);
	
	BOARD_CHANNEL_BEGIN(vecChannel)
	{
		bNoBoard = FALSE;
		nRetVal = iterBoard->second->SetPMUMode(vecChannel, PMUMode, Range, dSetValue, dClmapHigh, dClampLow);
		if (0 != nRetVal)
		{
			BOOL bExit = TRUE;
			m_pAlarm->SetParamName("lpszPinGroup");
			m_pAlarm->SetPinString(lpszPinGroup, FALSE);
			BOOL bAddAlarm = !m_pAlarm->IsSetMsg();
			switch (nRetVal)
			{
			case -1:
				if (bAddAlarm)
				{
					m_pAlarm->SetAlarmLevel(CDriverAlarm::ALARM_LEVEL::AlarmWarning);
					m_pAlarm->SetAlarmID(ALARM_ID::ALARM_CHANNEL_NOT_EXISTED);
					m_pAlarm->SetAlarmMsg("The channels of board(S%d) used in pin group(%s) is not existed", iterBoard->first, lpszPinGroup);
					m_pAlarm->Output(FALSE);
				}
				bExit = FALSE;
				nRetVal = -7;
				break;
			default:
				break;
			}
			if (bExit)
			{
				break;
			}
		}
		set<USHORT>* psetShieldChannel = nullptr;
		iterShieldChannel = mapShieldChannel.find(iterBoard->first);
		if (mapShieldChannel.end() != iterShieldChannel)
		{
			psetShieldChannel = &iterShieldChannel->second;
		}
		CheckPMUClampStatus(iterBoard->first, vecChannel, psetShieldChannel);

	}
	BOARD_CHANNEL_END

	if (bNoBoard)
	{
		m_pAlarm->SetNoBoardAlarm(lpszPinGroup, FALSE);
		nRetVal = -8;
	}

	return nRetVal;
}

int CDCM::PMUMeasure(const char* lpszPinGroup, int nSampleTimes, double dSamplePeriod)
{
#ifdef RECORD_TIME
	CTimer::Instance()->Start("PMUMeasure_%s", lpszPinGroup);
#endif // RECORD_TIME

	if (!m_bLoadVector)
	{
		m_pAlarm->VectorNotLoadedAlarm();
		return -1;
	}

#ifdef RECORD_TIME
	CTimer::Instance()->Start("GetChannel");
#endif // RECORD_TIME
	set<string> setPinName;
	int nRetVal = GetChannel(lpszPinGroup, ALL_SITE, TRUE, &setPinName);
	if (0 != nRetVal)
	{
		switch (nRetVal)
		{
		case -2:
			//The pin group is nullptr
			nRetVal = -2;
			break;
		case -3:
			//The pin group is not defined before
			nRetVal = -3;
			break;
		case -6:
			///<The pin is not belongs to
			nRetVal = -4;
			break;
		default:
			///<Not will happen
			break;
		}
		return nRetVal;
	}

#ifdef RECORD_TIME
	CTimer::Instance()->Stop();
	CTimer::Instance()->Start("Bind");
#endif // RECORD_TIME

	set<USHORT> setSite;
	GetSiteInfo(ALL_SITE, setSite);
	int nBind = Bind(setPinName, setSite, TRUE);
	BOOL bBind = 0 <= nBind ? TRUE : FALSE;

	vector<USHORT> vecChannel;
	BOOL bNoBoard = TRUE;

	map<BYTE, set<USHORT>> mapShieldChannel;
	GetShieldChannel("PMUMeasure", mapShieldChannel);

#ifdef RECORD_TIME
	CTimer::Instance()->Stop();
	CTimer::Instance()->Start("Measure");
#endif // RECORD_TIME

	auto CheckClamp = [&](map<BYTE,CBoard*>::iterator iterBoard)
	{
		set<USHORT>* psetShieldChannel = nullptr;
		if (bBind)
		{
			set<BYTE> setSlot;
			set<BYTE> setController;
			BYTE byTargetSlot = 0;
			byTargetSlot = CBindInfo::Instance()->GetBindInfo(setSlot, setController);
			CBindInfo::Instance()->ClearBind();
			for (auto BindSlot : setSlot)
			{
				auto iterSlot = mapShieldChannel.find(BindSlot);
				if (mapShieldChannel.end() != iterSlot)
				{
					psetShieldChannel = &iterSlot->second;
				}
				else
				{
					psetShieldChannel = nullptr;
				}
				CheckPMUClampStatus(BindSlot, vecChannel, psetShieldChannel);
			}
			CBindInfo::Instance()->Bind(setSlot, setController, byTargetSlot);
		}
		else
		{
			auto iterSlot = mapShieldChannel.find(iterBoard->first);
			if (mapShieldChannel.end() != iterSlot)
			{
				psetShieldChannel = &iterSlot->second;
			}
			else
			{
				psetShieldChannel = nullptr;
			}
			CheckPMUClampStatus(iterBoard->first, vecChannel, psetShieldChannel);
		}
	};

	BOOL bWaitPMUStart = FALSE;
	BOOL bFail = FALSE;
	BOARD_CHANNEL_BEGIN(vecChannel)
	{
#ifdef RECORD_TIME
		CTimer::Instance()->Start("Measure_%d", iterBoard->first);
#endif // RECORD_TIME
		bNoBoard = FALSE;
		nRetVal = iterBoard->second->PMUMeasure(vecChannel, nSampleTimes, dSamplePeriod);

		if (0 != nRetVal && -3 != nRetVal)
		{
			bFail = TRUE;
			m_pAlarm->SetParamName("lpszPinGroup");
			m_pAlarm->SetPinString(lpszPinGroup, FALSE);
			BOOL bAddAlarm = !m_pAlarm->IsSetMsg();
			switch (nRetVal)
			{
			case -1:
				if (bAddAlarm)
				{
					m_pAlarm->SetParamName("lpszPinGroup");
					m_pAlarm->SetAlarmID(ALARM_ID::ALARM_BOARD_NOT_EXISTED);
					m_pAlarm->SetAlarmMsg("The board(S%d) is not existed.", iterBoard->first);
				}
				nRetVal = -5;
				bFail = FALSE;
				break;
			case -2:
				if (bAddAlarm)
				{
					m_pAlarm->SetPinString(lpszPinGroup, FALSE);
					m_pAlarm->SetAlarmID(ALARM_ID::ALARM_PMU_MEAS_ERROR);
					m_pAlarm->SetAlarmMsg("PMU measurement error.");
				}
				nRetVal = -6;
				break;
			default:
				break;
			}
			if (bFail)
			{
#ifdef RECORD_TIME
				CTimer::Instance()->Stop();
				CTimer::Instance()->Stop();
				CTimer::Instance()->Stop();
				CTimer::Instance()->Print("D:\\PMUMeasure.csv");
#endif // RECORD_TIME
				return nRetVal;
			}
		}
		else if (-3 == nRetVal)
		{
			bWaitPMUStart = TRUE;
		}

#ifdef RECORD_TIME
		CTimer::Instance()->Stop();
#endif // RECORD_TIME

	}
	BOARD_CHANNEL_END

#ifdef RECORD_TIME
		CTimer::Instance()->Stop();
		CTimer::Instance()->Start("Wait PMU");
#endif // RECORD_TIME

	if (bWaitPMUStart)
	{
		nRetVal = 0;
		do
		{
			BOARD_CHANNEL_BEGIN(vecChannel)
			{
#ifdef RECORD_TIME
				CTimer::Instance()->Start("Wait PMU Finish S%d", Slot);
#endif // RECORD_TIME
				nRetVal = iterBoard->second->WaitPMUFinish();
#ifdef RECORD_TIME
				CTimer::Instance()->Stop();
				CTimer::Instance()->Start("Check PMU Clamp");
#endif // RECORD_TIME

				CheckClamp(iterBoard);
#ifdef RECORD_TIME
				CTimer::Instance()->Stop();
#endif // RECORD_TIME
			}
			BOARD_CHANNEL_END
			if (0 != nRetVal)
			{
				///<No PMU data needed to be saved
				break;
			}
			BOARD_CHANNEL_BEGIN(vecChannel)
			{
#ifdef RECORD_TIME
				CTimer::Instance()->Start("Start PMU S%d", Slot);
#endif // RECORD_TIME
				nRetVal = iterBoard->second->StartPMU();
#ifdef RECORD_TIME
				CTimer::Instance()->Stop();
#endif // RECORD_TIME
				if ( 0 != nRetVal)
				{
					break;
				}
			}
			BOARD_CHANNEL_END
		} while (0 == nRetVal);
	}
	nRetVal = 0;

#ifdef RECORD_TIME
	CTimer::Instance()->Stop();
	CTimer::Instance()->Start("ClearBind");
#endif // RECORD_TIME

	if (bBind)
	{
		ClearBind();
	}
	///<Check clamp status


	if (bNoBoard)
	{
		m_pAlarm->SetNoBoardAlarm(lpszPinGroup, FALSE);
		nRetVal = -7;
	}


#ifdef RECORD_TIME
	CTimer::Instance()->Stop();
	CTimer::Instance()->Stop();
	CTimer::Instance()->Print("D:\\PMUMeasure.csv");
#endif // RECORD_TIME

	return nRetVal;
}

double CDCM::GetPMUMeasureResult(const char* lpszPinName, USHORT usSiteNo, int nSampleTimes)
{
	CHANNEL_INFO Channel;
	int nRetVal = GetBoardChannel(lpszPinName, usSiteNo, Channel);
	if (0 != nRetVal)
	{
		return MAX_MEASURE_VALUE;
	}
	auto iterBoard = m_mapBoard.find(Channel.m_bySlotNo);
	if (m_mapBoard.end() == iterBoard || nullptr == iterBoard->second)
	{
		m_pAlarm->SetNoBoardAlarm(lpszPinName, TRUE, usSiteNo);
		return MAX_MEASURE_VALUE;
	}
	double dValue = iterBoard->second->GetPMUMeasureResult(Channel.m_usChannel, nSampleTimes);
	if (m_pAlarm->IsSetMsg())
	{
		m_pAlarm->SetParamName("lpszPinName");
		m_pAlarm->SetSite(usSiteNo);
		m_pAlarm->SetPinString(lpszPinName, TRUE);
	}
	return dValue;
}

const char* CDCM::GetStopLineLabel(USHORT usSiteNo)
{
	m_strStopLabel.clear();
	int nRetVal = GetStopLineNo(usSiteNo);
	if (0 <= nRetVal)
	{
		UINT uLineNo = 0;
		m_VectorInfo.GetLineNo(nRetVal, uLineNo);
		if (0 != m_VectorInfo.GetLabelName(uLineNo, m_strStopLabel))
		{
			m_pAlarm->SetAlarmLevel(CDriverAlarm::ALARM_LEVEL::AlarmError);
			m_pAlarm->SetAlarmID(ALARM_ID::ALARM_GET_STOP_LINE_LABEL_ERROR);
			m_pAlarm->SetSite(usSiteNo);
			m_pAlarm->SetAlarmMsg("The stop line(%d) is without label.", uLineNo);
			m_strStopLabel.clear();
		}
	}
	return m_strStopLabel.c_str();
}

int CDCM::GetLabelLineNo(const char* lpszLabelName, BOOL bBRAMLine)
{
	if (!m_bLoadVector)
	{
		return -1;
	}
	if (nullptr == lpszLabelName)
	{
		return -2;
	}
	int nLabelLineNumber = m_VectorInfo.GetLabelLineNo(lpszLabelName, !bBRAMLine);
	if (0 > nLabelLineNumber)
	{
		return -3;
	}
	return nLabelLineNumber;
}

int CDCM::SetOperand(const char* lpszPinGroup, const char* lpszStartLabel, ULONG ulOffset, const char* lpszOperand)
{
	if (!m_bLoadVector)
	{
		m_pAlarm->VectorNotLoadedAlarm();
		return -1;
	}

	int nRetVal = GetChannel(lpszPinGroup);
	if (0 != nRetVal)
	{
		switch (nRetVal)
		{
		case -2:
			//The pin group is nullptr
			nRetVal = -2;
			break;
		case -3:
			//The pin group is not defined before
			nRetVal = -3;
			break;
		case -4:
			///<Site over range, not will happened
			break;
		case -5:
			///<Site invalid, not will happen
			nRetVal = -4;
			break;
		case -6:
			///<The pin is not belongs to
			nRetVal = -5;
			break;
		default:
			///<Not will happen
			break;
		}
		return nRetVal;
	}
	int nStartLine = m_VectorInfo.GetLabelLineNo(lpszStartLabel, TRUE);
	if (0 > nStartLine)
	{
		nRetVal = nStartLine;
		switch (nRetVal)
		{
		case -1:
			///<The start label is nullptr
		{
			nRetVal = -6;
			m_pAlarm->SetAlarmID(ALARM_ID::ALARM_START_LABEL_ERROR);
			m_pAlarm->SetPinString(lpszPinGroup, FALSE);
			m_pAlarm->SetParamName("lpszStartLabel");
			m_pAlarm->SetAlarmMsg("The start label is nullptr.");
			m_pAlarm->SetAlarmLevel(CDriverAlarm::ALARM_LEVEL::AlarmError);
		}
		break;
		case -2:
		{
			nRetVal = -7;
			m_pAlarm->SetAlarmID(ALARM_ID::ALARM_START_LABEL_ERROR);
			m_pAlarm->SetAlarmType(CDriverAlarm::ALARM_TYPE::PARAMETER_NOT_DEFINED);
			m_pAlarm->SetPinString(lpszPinGroup, FALSE);
			m_pAlarm->SetParamName("lpszStartLabel");
			m_pAlarm->SetAlarmMsg("The start label(%s) is not defined in vector file.", lpszStartLabel);
			m_pAlarm->SetAlarmLevel(CDriverAlarm::ALARM_LEVEL::AlarmError);
		}
		break;
		default:
			break;
		}
		return nRetVal;
	}
	UINT uLineNo = 0;
	BOOL bBRAM = m_VectorInfo.GetLineNo(nStartLine + ulOffset, uLineNo);
	if (0 > bBRAM)
	{
		m_pAlarm->SetPinString(lpszPinGroup, FALSE);
		m_pAlarm->SetAlarmID(ALARM_ID::ALARM_OFFSET_OVER_RANGE);
		m_pAlarm->SetAlarmType(CDriverAlarm::ALARM_TYPE::PARAMETER_OVERRANGE);
		m_pAlarm->SetParamName("ulOffset");
		m_pAlarm->SetPinString("lpszPinGroup", FALSE);
		UINT uVectorLineCount = m_VectorInfo.GetBRAMLineCount() + m_VectorInfo.GetDRAMLineCount();
		m_pAlarm->SetAlarmMsg("The offset(%d) is over range[%d,%d].", ulOffset, 0, uVectorLineCount - nStartLine);
		return -8;
	}
	else if (!bBRAM)
	{
		m_pAlarm->SetPinString(lpszPinGroup, FALSE);
		m_pAlarm->SetAlarmID(ALARM_ID::ALARM_LINE_NO_OPERAND);
		m_pAlarm->SetAlarmType(CDriverAlarm::ALARM_TYPE::PARAMETER_OVERRANGE);
		m_pAlarm->SetParamName("ulOffset");
		m_pAlarm->SetAlarmMsg("No operand in line %d.", nStartLine + ulOffset);
		return -9;
	}

	if (nullptr == lpszOperand)
	{
		m_pAlarm->SetPinString(lpszPinGroup, FALSE);
		m_pAlarm->SetAlarmID(ALARM_ID::ALARM_PARAM_NULLPTR);
		m_pAlarm->SetParamName("lpszOperand");
		m_pAlarm->SetAlarmMsg("The operand is nullptr.");
		m_pAlarm->SetAlarmLevel(CDriverAlarm::ALARM_LEVEL::AlarmError);

		return -10;
	}

	int nOperand = 0;

	if ('0' > lpszOperand[0] || '9' < lpszOperand[0])
	{
		nOperand = m_VectorInfo.GetLabelLineNo(lpszOperand, FALSE);
		if (0 > nOperand)
		{
			nRetVal = nOperand;
			switch (nRetVal)
			{
			case -1:
				///<The label is nullptr, not will happen
				break;
			case -2:
				///<The label is not existed
				nRetVal = -11;
				m_pAlarm->SetAlarmID(ALARM_ID::ALARM_OPERAND_ERROR);
				m_pAlarm->SetParamName("lpszOperand");
				m_pAlarm->SetPinString(lpszPinGroup, FALSE);
				m_pAlarm->SetAlarmMsg("The label operand(%s) is not existed.", lpszOperand);
				break;
			default:
				break;
			}
			return nRetVal;
		}
	}
	else
	{
		nOperand = atoi(lpszOperand);
		if (0xFFFF < nOperand)
		{
			///<The operand is over range
			m_pAlarm->SetPinString(lpszPinGroup, FALSE);
			m_pAlarm->SetParamName("lpszOperand");
			m_pAlarm->SetAlarmID(ALARM_ID::ALARM_OPERAND_ERROR);
			m_pAlarm->SetAlarmMsg("The operand(%s) is over range[1, %d].", lpszOperand, 0xFFFF);
			return -12;
		}
		else if (0 == nOperand && '0' != lpszOperand[0])
		{
			///<The operand is over range
			m_pAlarm->SetPinString(lpszPinGroup, FALSE);
			m_pAlarm->SetParamName("lpszOperand");
			m_pAlarm->SetAlarmID(ALARM_ID::ALARM_OPERAND_ERROR);
			m_pAlarm->SetAlarmMsg("The operand(%s) is not a number.");
			return -12;
		}
	}

	BOOL bNoBoard = TRUE;
	vector<USHORT> vecChannel;

	BOARD_CHANNEL_BEGIN(vecChannel)
	{
		nRetVal = iterBoard->second->SetOperand(vecChannel, uLineNo, nOperand, TRUE);
		if (0 != nRetVal)
		{
			BOOL bExit = TRUE;
			BOOL bAddAlarm = !m_pAlarm->IsSetMsg();
			m_pAlarm->SetPinString(lpszPinGroup, FALSE);
			switch (nRetVal)
			{
			case -1:
				///<Line is over range, will not happened
				break;
			case -2:
				///<The operand is over range
				m_pAlarm->SetParamName("usOperand");
				if (bAddAlarm)
				{
					m_pAlarm->SetAlarmID(ALARM_ID::ALARM_OPERAND_ERROR);
					m_pAlarm->SetAlarmMsg("The operand(%d) is over range.", nOperand);
				}
				nRetVal = -12;
				break;
			case -3:
				///<No valid channel existed
				m_pAlarm->SetAlarmID(ALARM_ID::ALARM_CHANNEL_NOT_EXISTED);
				m_pAlarm->SetAlarmMsg("The channels of board(S%d) used in pin group(%s) are not existed.", iterBoard->first, lpszPinGroup);
				m_pAlarm->SetAlarmLevel(CDriverAlarm::ALARM_LEVEL::AlarmWarning);
				m_pAlarm->Output(FALSE);
				nRetVal = -13;
				bExit = FALSE;
				break;
			default:
				break;
			}
			if (bExit)
			{
				break;
			}
		}
		bNoBoard = FALSE;
	}
	BOARD_CHANNEL_END

	if (bNoBoard)
	{
		m_pAlarm->SetNoBoardAlarm(lpszPinGroup, FALSE);
		nRetVal = -14;
	}
	return nRetVal;
}

int CDCM::SetInstruction(const char* lpszPinGroup, const char* lpszStartLabel, int nOffset, const char* lpszInstruction, const char* lpszOperand)
{
	if (!m_bLoadVector)
	{
		m_pAlarm->VectorNotLoadedAlarm();
		return -1;
	}

	int nRetVal = GetChannel(lpszPinGroup);
	if (0 != nRetVal)
	{
		switch (nRetVal)
		{
		case -2:
			//The pin group is nullptr
			nRetVal = -2;
			break;
		case -3:
			//The pin group is not defined before
			nRetVal = -3;
			break;
		case -4:
			///<Site over range, not will happened
			break;
		case -5:
			///<Site invalid, not will happen
			nRetVal = -4;
			break;
		case -6:
			///<The pin is not belongs to
			nRetVal = -5;
			break;
		default:
			///<Not will happen
			break;
		}
		return nRetVal;
	}
	int nStartLine = m_VectorInfo.GetLabelLineNo(lpszStartLabel, TRUE);
	if (0 > nStartLine)
	{
		nRetVal = nStartLine;
		switch (nRetVal)
		{
		case -1:
			///<The start label is nullptr
		{
			nRetVal = -6;
			m_pAlarm->SetPinString(lpszPinGroup, FALSE);
			m_pAlarm->SetAlarmID(ALARM_ID::ALARM_START_LABEL_ERROR);
			m_pAlarm->SetParamName("lpszStartLabel");
			m_pAlarm->SetAlarmMsg("The start label is nullptr.");
			m_pAlarm->SetAlarmLevel(CDriverAlarm::ALARM_LEVEL::AlarmError);
		}
		break;
		case -2:
		{
			nRetVal = -7;
			m_pAlarm->SetPinString(lpszPinGroup, FALSE);
			m_pAlarm->SetAlarmID(ALARM_ID::ALARM_START_LABEL_ERROR);
			m_pAlarm->SetAlarmType(CDriverAlarm::ALARM_TYPE::PARAMETER_NOT_DEFINED);
			m_pAlarm->SetParamName("lpszStartLabel");
			m_pAlarm->SetAlarmMsg("The start label(%s) is not defined in vector file.", lpszStartLabel);
			m_pAlarm->SetAlarmLevel(CDriverAlarm::ALARM_LEVEL::AlarmError);
		}
		break;
		default:
			break;
		}
		return nRetVal;
	}
	UINT uBRAMLineNo = 0;
	BOOL bBRAM = m_VectorInfo.GetLineNo(nStartLine + nOffset, uBRAMLineNo);
	if (0 > bBRAM)
	{
		m_pAlarm->SetPinString(lpszPinGroup, FALSE);
		m_pAlarm->SetAlarmID(ALARM_ID::ALARM_OFFSET_OVER_RANGE);
		m_pAlarm->SetAlarmType(CDriverAlarm::ALARM_TYPE::PARAMETER_OVERRANGE);
		m_pAlarm->SetParamName("ulOffset");
		m_pAlarm->SetPinString("lpszPinGroup", FALSE);
		UINT uVectorLineCount = m_VectorInfo.GetBRAMLineCount() + m_VectorInfo.GetDRAMLineCount();
		m_pAlarm->SetAlarmMsg("The offset(%d) is over range[%d,%d].", nOffset, 0, uVectorLineCount - nStartLine);
		return -8;
	}
	else if (!bBRAM)
	{
		m_pAlarm->SetPinString(lpszPinGroup, FALSE);
		m_pAlarm->SetAlarmID(ALARM_ID::ALARM_LINE_NO_OPERAND);
		m_pAlarm->SetAlarmType(CDriverAlarm::ALARM_TYPE::PARAMETER_OVERRANGE);
		m_pAlarm->SetParamName("lpszOperand");
		m_pAlarm->SetAlarmMsg("No operand in line %d.", nStartLine + nOffset);
		return -9;
	}
	if (nullptr == lpszOperand)
	{
		m_pAlarm->SetPinString(lpszPinGroup, FALSE);
		m_pAlarm->SetAlarmID(ALARM_ID::ALARM_PARAM_NULLPTR);
		m_pAlarm->SetParamName("lpszOperand");
		m_pAlarm->SetAlarmMsg("The operand is nullptr.");
		m_pAlarm->SetAlarmLevel(CDriverAlarm::ALARM_LEVEL::AlarmError);

		return -10;
	}

	int nOperand = 0;

	if ('0' > lpszOperand[0] || '9' < lpszOperand[0])
	{
		nOperand = m_VectorInfo.GetLabelLineNo(lpszOperand, FALSE);
		if (0 > nOperand)
		{
			nRetVal = nOperand;
			switch (nRetVal)
			{
			case -1:
				///<The label is nullptr, not will happen
				break;
			case -2:
				///<The label is not existed
				nRetVal = -11;
				m_pAlarm->SetAlarmID(ALARM_ID::ALARM_OPERAND_ERROR);
				m_pAlarm->SetParamName("lpszOperand");
				m_pAlarm->SetPinString(lpszPinGroup, FALSE);
				m_pAlarm->SetAlarmMsg("The label operand(%s) is not existed.", lpszOperand);
				break;
			default:
				break;
			}
			return nRetVal;
		}
	}
	else
	{
		nOperand = atoi(lpszOperand);
		if (0xFFFF < nOperand)
		{///<The operand is over range
			m_pAlarm->SetPinString(lpszPinGroup, FALSE);
			m_pAlarm->SetParamName("usOperand");
			m_pAlarm->SetAlarmID(ALARM_ID::ALARM_OPERAND_ERROR);
			m_pAlarm->SetAlarmMsg("The operand(%s) is over range[1, %d].", lpszOperand, 0xFFFF);
			return -12;
		}
	}

	BOOL bNoBoard = TRUE;
	vector<USHORT> vecChannel;

	BOARD_CHANNEL_BEGIN(vecChannel)
	{
		bNoBoard = FALSE;
		nRetVal = iterBoard->second->SetInstruction(vecChannel, uBRAMLineNo, lpszInstruction, nOperand);

		if (0 != nRetVal)
		{
			BOOL bExit = TRUE;
			BOOL bAddAlarm = !m_pAlarm->IsSetMsg();
			m_pAlarm->SetPinString(lpszPinGroup, FALSE);
			switch (nRetVal)
			{
			case -1:
				///<Line is over range, will not happened
				break;
			case -2:
				///<The instruction is nullptr
				m_pAlarm->SetParamName("lpszInstruction");
				if (bAddAlarm)
				{
					m_pAlarm->SetAlarmID(ALARM_ID::ALARM_PARAM_NULLPTR);
					m_pAlarm->SetAlarmMsg("The instruction is nullptr.");
				}
				nRetVal = -13;
				break;
			case -3:
				///<The instruction is not supported
				m_pAlarm->SetParamName("lpszInstruction");
				if (bAddAlarm)
				{
					m_pAlarm->SetAlarmMsg("The instruction(%s) is not supported.", lpszInstruction);
					m_pAlarm->SetAlarmID(ALARM_ID::ALARM_CMD_NOT_SUPPORTED);
				}
				nRetVal = -14;
				break;
			case -4:
				///<The operand is over range
				m_pAlarm->SetParamName("usOperand");
				if (bAddAlarm)
				{
					m_pAlarm->SetAlarmID(ALARM_ID::ALARM_OPERAND_ERROR);
					m_pAlarm->SetAlarmMsg("The operand(%d) is over range.", nOperand);
				}
				nRetVal = -12;
				break;
			case -5:
				///<No valid channel existed
				m_pAlarm->SetAlarmID(ALARM_ID::ALARM_CHANNEL_NOT_EXISTED);
				m_pAlarm->SetAlarmMsg("The channels of board(S%d) used in pin group(%s) are not existed.", iterBoard->first, lpszPinGroup);
				m_pAlarm->SetAlarmLevel(CDriverAlarm::ALARM_LEVEL::AlarmWarning);
				m_pAlarm->Output(FALSE);
				bExit = FALSE;
				nRetVal = -15;
				break;
			default:
				break;
			}
			if (bExit)
			{
				break;
			}
		}
	}
	BOARD_CHANNEL_END

	if (bNoBoard)
	{
		m_pAlarm->SetNoBoardAlarm(lpszPinGroup, FALSE);
		nRetVal = -16;
	}
	return nRetVal;
}

int CDCM::SetSaveSelectFail(const char* lpszPinGroup, const char* lpszStartLabel, int nOffset, BOOL bStartSave, BOOL bDelete)
{
	if (!m_bLoadVector)
	{
		m_pAlarm->VectorNotLoadedAlarm();
		return -1;
	}

	int nRetVal = GetChannel(lpszPinGroup);
	if (0 != nRetVal)
	{
		switch (nRetVal)
		{
		case -2:
			//The pin group is nullptr
			nRetVal = -2;
			break;
		case -3:
			//The pin group is not defined before
			nRetVal = -3;
			break;
		case -4:
			///<Site over range, not will happened
			break;
		case -5:
			///<No site valid
			return -4;
			break;
		default:
			///<Not will happen
			break;
		}
		return nRetVal;
	}
	int nStartLine = m_VectorInfo.GetLabelLineNo(lpszStartLabel, TRUE);
	if (0 > nStartLine)
	{
		nRetVal = nStartLine;
		switch (nRetVal)
		{
		case -1:
			///<The start label is nullptr
		{
			nRetVal = -5;
			m_pAlarm->SetPinString(lpszPinGroup, FALSE);
			m_pAlarm->SetAlarmID(ALARM_ID::ALARM_START_LABEL_ERROR);
			m_pAlarm->SetParamName("lpszStartLabel");
			m_pAlarm->SetAlarmMsg("The start label is nullptr.");
			m_pAlarm->SetAlarmLevel(CDriverAlarm::ALARM_LEVEL::AlarmError);
		}
		break;
		case -2:
		{
			nRetVal = -6;
			m_pAlarm->SetPinString(lpszPinGroup, FALSE);
			m_pAlarm->SetAlarmID(ALARM_ID::ALARM_START_LABEL_ERROR);
			m_pAlarm->SetAlarmType(CDriverAlarm::ALARM_TYPE::PARAMETER_NOT_DEFINED);
			m_pAlarm->SetParamName("lpszStartLabel");
			m_pAlarm->SetAlarmMsg("The start label(%s) is not defined in vector file.", lpszStartLabel);
			m_pAlarm->SetAlarmLevel(CDriverAlarm::ALARM_LEVEL::AlarmError);
		}
		break;
		default:
			break;
		}
		return nRetVal;
	}
	UINT uRAMLineNo = 0;
	BOOL bBRAM = m_VectorInfo.GetLineNo(nStartLine + nOffset, uRAMLineNo);
	if (0 > bBRAM)
	{
		m_pAlarm->SetPinString(lpszPinGroup, FALSE);
		m_pAlarm->SetAlarmID(ALARM_ID::ALARM_OFFSET_OVER_RANGE);
		m_pAlarm->SetAlarmType(CDriverAlarm::ALARM_TYPE::PARAMETER_OVERRANGE);
		m_pAlarm->SetParamName("ulOffset");
		m_pAlarm->SetPinString("lpszPinGroup", FALSE);
		UINT uVectorLineCount = m_VectorInfo.GetBRAMLineCount() + m_VectorInfo.GetDRAMLineCount();
		m_pAlarm->SetAlarmMsg("The offset(%d) is over range[%d,%d].", nOffset, 0, uVectorLineCount - nStartLine);
		return -7;
	}
	bBRAM = !bBRAM;

	BOOL bNoBoard = TRUE;
	vector<USHORT> vecChannel;

	BOARD_CHANNEL_BEGIN(vecChannel)
	{
		bNoBoard = FALSE;
		nRetVal = iterBoard->second->SetSaveSelectFail(vecChannel, uRAMLineNo, bStartSave, bBRAM, bDelete);

		if (0 != nRetVal)
		{
			BOOL bExit = TRUE;
			BOOL bAddAlarm = !m_pAlarm->IsSetMsg();
			m_pAlarm->SetPinString(lpszPinGroup, FALSE);
			switch (nRetVal)
			{
			case -1:
				///<Line is over range, will not happened
				break;
			case -2:
				///<No valid channel existed
				m_pAlarm->SetAlarmID(ALARM_ID::ALARM_CHANNEL_NOT_EXISTED);
				m_pAlarm->SetAlarmMsg("The channels of board(S%d) used in pin group(%s) are not existed.", iterBoard->first, lpszPinGroup);
				m_pAlarm->SetAlarmLevel(CDriverAlarm::ALARM_LEVEL::AlarmWarning);
				m_pAlarm->Output(FALSE);
				bExit = FALSE;
				nRetVal = -8;
				break;
			default:
				break;
			}
			if (bExit)
			{
				break;
			}
		}
	}
	BOARD_CHANNEL_END;

	if (bNoBoard)
	{
		m_pAlarm->SetNoBoardAlarm(lpszPinGroup, FALSE);
		nRetVal = -9;
	}
	return nRetVal;
}

int CDCM::GetInstruction(BYTE bySlotNo, BYTE byController, UINT uBRAMLineNo, char* lpszInstruction, int nBuffSize)
{
	auto iterBoard = m_mapBoard.find(bySlotNo);
	if (m_mapBoard.end() == iterBoard)
	{
		return -1;
	}
	int nRetVal = iterBoard->second->GetInstruction(byController, uBRAMLineNo, lpszInstruction, nBuffSize);
	if (0 != nRetVal)
	{
		switch (nRetVal)
		{
		case -1:
			///<The controller is over range
			nRetVal = -2;
			break;
		case -2:
			///<The controller is not existed
			nRetVal = -3;
			break;
		case -3:
			///<The line number is over range
			nRetVal = -4;
			break;
		case -4:
			///<The parameter for saving instruction is nullptr
			nRetVal = -5;
			break;
		case -5:
			///<The buff is too small
			nRetVal = -6;
		default:
			break;
		}
	}
	return nRetVal;
}

int CDCM::GetOperand(BYTE bySlotNo, BYTE byController, UINT uBRAMLineNo)
{
	auto iterBoard = m_mapBoard.find(bySlotNo);
	if (m_mapBoard.end() == iterBoard)
	{
		return -1;
	}
	int nRetVal = iterBoard->second->GetOperand(byController, uBRAMLineNo);
	if (0 != nRetVal)
	{
		switch (nRetVal)
		{
		case -1:
			///<The controller is over range
			nRetVal = -2;
			break;
		case -2:
			///<The controller is not existed
			nRetVal = -3;
			break;
		case -3:
			///<line number is over range
			nRetVal = -4;
			break;
		default:
			break;
		}
	}
	return nRetVal;
}

int CDCM::GetChannel(USHORT usPinNo, USHORT usSiteNo, CHANNEL_INFO& Channel)
{
	if (!m_bLoadVector)
	{
		return -1;
	}
	auto iterPin = m_mapPin.begin();
	while (m_mapPin.end() != iterPin)
	{
		if (usPinNo == iterPin->second->GetID())
		{
			break;
		}
		++iterPin;
	}
	if (m_mapPin.end() == iterPin)
	{
		return -2;
	}

	int nRetVal = 0;
	nRetVal = iterPin->second->GetChannel(usSiteNo, Channel);
	if (0 != nRetVal)
	{
		return -3;
	}
	return 0;
}

int CDCM::GetPinName(USHORT usPinNo, std::string& strPinName)
{
	if (!m_bLoadVector)
	{
		return -1;
	}
	for (auto& Pin : m_mapPin)
	{
		if (usPinNo == Pin.second->GetID())
		{
			strPinName = Pin.first;
			return 0;
		}
	}

	return -2;
}

void CDCM::Reset()
{
	m_bLoadVector = FALSE;
	m_strFileName.clear();
	DeleteMemory(m_mapBoard);
	DeleteMemory(m_mapPinGroup);
	DeleteMemory(m_mapPin);
}

template <typename Key, typename Value>
inline void CDCM::DeleteMemory(std::map<Key, Value>& mapParam)
{
	for (auto& Item : mapParam)
	{
		if (nullptr != Item.second)
		{
			delete Item.second;
			Item.second = nullptr;
		}
	}
	mapParam.clear();
}

int CDCM::SetCalibrationInfo(BYTE bySlotNo, BYTE byControllerIndex, STS_CALINFO* pCalInfo, BYTE* pbyChannelStatus, int nElementCount)
{
	auto iterBoard = m_mapBoard.find(bySlotNo);
	if (m_mapBoard.end() == iterBoard || nullptr == iterBoard->second)
	{
		return -1;
	}
	int nRetVal = iterBoard->second->SetCalibrationInfo(byControllerIndex, pCalInfo, pbyChannelStatus, nElementCount);
	if (0 != nRetVal)
	{
		nRetVal -= 1;
	}
	return nRetVal;
}

int CDCM::GetCalibrationInfo(BYTE bySlotNo, BYTE byControllerIndex, STS_CALINFO* pCalInfo, int nElementCount)
{
	auto iterBoard = m_mapBoard.find(bySlotNo);
	if (m_mapBoard.end() == iterBoard || nullptr == iterBoard->second)
	{
		return -1;
	}
	int nRetVal = iterBoard->second->GetCalibrationInfo(byControllerIndex, pCalInfo, nElementCount);
	if (0 != nRetVal)
	{
		nRetVal -= 1;
	}
	return nRetVal;
}

int CDCM::GetCalibrationInfo(const char* lpszPinName, USHORT usSiteNo, STS_CALINFO& CalibrationInfo)
{
	int nRetVal = 0;
	CHANNEL_INFO Channel;
	nRetVal = GetBoardChannel(lpszPinName, usSiteNo, Channel);
	if (0 != nRetVal)
	{
		return nRetVal;
	}
	auto iterBoard = m_mapBoard.find(Channel.m_bySlotNo);
	if (m_mapBoard.end() == iterBoard || nullptr == iterBoard->second || DCM_MAX_CHANNELS_PER_BOARD <= Channel.m_usChannel)
	{
		m_pAlarm->SetNoBoardAlarm(lpszPinName, TRUE, usSiteNo);
		return -7;
	}
	nRetVal = iterBoard->second->GetCalibrationInfo(Channel.m_usChannel, CalibrationInfo);
	if (0 != nRetVal)
	{
		m_pAlarm->SetSite(usSiteNo);
		m_pAlarm->SetPinString(lpszPinName, TRUE);
		m_pAlarm->SetParamName("lpszPinName");
		switch (nRetVal)
		{
		case -1:
			m_pAlarm->SetAlarmID(ALARM_ID::ALARM_CHANNEL_OVER_RANGE);
			m_pAlarm->SetAlarmMsg("The channel(S%d_%d) of pin(%s) in SITE_%d is not over range[%d,%d].", Channel.m_bySlotNo, Channel.m_usChannel, lpszPinName, usSiteNo + 1, 0, DCM_MAX_CHANNELS_PER_BOARD);
			m_pAlarm->SetAlarmLevel(CDriverAlarm::ALARM_LEVEL::AlarmWarning);
			m_pAlarm->Output(FALSE);
			nRetVal = -8;
			break;
		case -2:
			m_pAlarm->SetAlarmID(ALARM_ID::ALARM_CHANNEL_NOT_EXISTED);
			m_pAlarm->SetAlarmMsg("The channel(S%d_%d) of pin(%s) in SITE_%d is not existed.", Channel.m_bySlotNo, Channel.m_usChannel, lpszPinName, usSiteNo + 1);
			m_pAlarm->SetAlarmLevel(CDriverAlarm::ALARM_LEVEL::AlarmWarning);
			m_pAlarm->Output(FALSE);
			nRetVal = -9;
			break;
		case -3:
			m_pAlarm->SetAlarmID(ALARM_ID::ALARM_CALIBRATION_ERRPR);
			m_pAlarm->SetAlarmMsg("Get the calibraiton of channel(S%d_%d) of pin(%s) in SITE_%d fail.", Channel.m_bySlotNo, Channel.m_usChannel, lpszPinName, usSiteNo + 1);
			m_pAlarm->SetAlarmLevel(CDriverAlarm::ALARM_LEVEL::AlarmWarning);
			nRetVal = -10;
		default:
			break;
		}
	}
	return nRetVal;
}

int CDCM::GetCalibrationInfo(BYTE bySlotNo, USHORT usChannel, STS_CALINFO& CalibrationInfo)
{
	auto iterBoard = m_mapBoard.find(bySlotNo);
	if (m_mapBoard.end() == iterBoard || nullptr == iterBoard->second)
	{
		return -1;
	}
	int nRetVal = iterBoard->second->GetCalibrationInfo(usChannel, CalibrationInfo);
	if (0 != nRetVal)
	{
		switch (nRetVal)
		{
		case -1:
			nRetVal = -2;
			break;
		case -2:
			nRetVal = -3;
			break;
		case -3:
			nRetVal = -4;
			break;
		}
	}
	return nRetVal;
}

int CDCM::SetVT(const char* lpszPinGroup, double dVTVoltValue, VT_MODE VTMode)
{
	if (!m_bLoadVector)
	{
		m_pAlarm->VectorNotLoadedAlarm();
		return -1;
	}

	int nRetVal = GetChannel(lpszPinGroup);
	if (0 != nRetVal)
	{
		switch (nRetVal)
		{
		case -2:
			//The pin group is nullptr
			nRetVal = -2;
			break;
		case -3:
			//The pin group is not defined before
			nRetVal = -3;
			break;
		case -5:
			///<No valid site
			nRetVal = -4;
			break;
		case -6:
			///<The pin is not belongs to
			nRetVal = -5;
			break;
		default:
			///<Not will happen
			break;
		}
		return nRetVal;
	}


	vector<USHORT> vecChannel;
	BOOL bNoBoard = TRUE;

	BOARD_CHANNEL_BEGIN(vecChannel)
	{
		bNoBoard = FALSE;
		nRetVal = iterBoard->second->SetVTMode(vecChannel, dVTVoltValue, VTMode);
		if (0 != nRetVal)
		{
			m_pAlarm->SetPinString(lpszPinGroup, FALSE);
			BOOL bAddAlarm = !m_pAlarm->IsSetMsg();
			BOOL bExit = TRUE;
			switch (nRetVal)
			{
			case -1:
				if (bAddAlarm)
				{
					m_pAlarm->SetParamName("dVTVoltValue");
					m_pAlarm->SetAlarmID(ALARM_ID::ALARM_VT_VALUE_ERROR);
					m_pAlarm->SetAlarmMsg("The VT value(%.1f) is over range.", dVTVoltValue);
				}
				nRetVal = -6;
				break;
			case -2:
				if (bAddAlarm)
				{
					m_pAlarm->SetParamName("Mode");
					m_pAlarm->SetAlarmID(ALARM_ID::ALARM_MODE_ERROR);
					m_pAlarm->SetAlarmMsg("The VT mode is error.");
				}
				nRetVal = -7;
				break;
			case -3:
				if (bAddAlarm)
				{
					m_pAlarm->SetParamName("lpszPinGroup");
					m_pAlarm->SetAlarmID(ALARM_ID::ALARM_CHANNEL_NOT_EXISTED);
					m_pAlarm->SetAlarmMsg("The channels of board(S%d) used in %s are not existed.", iterBoard->first, lpszPinGroup);
					m_pAlarm->SetAlarmLevel(CDriverAlarm::ALARM_LEVEL::AlarmWarning);
					m_pAlarm->Output(FALSE);
				}
				nRetVal = -8;
				bExit = FALSE;
				break;
			default:
				break;
			}
			if (bExit)
			{
				break;
			}
		}
	}
	BOARD_CHANNEL_END

	if (bNoBoard)
	{
		m_pAlarm->SetNoBoardAlarm(lpszPinGroup, FALSE);
		nRetVal = -9;
	}
	return nRetVal;
}

int CDCM::GetVTMode(BYTE bySlotNo, USHORT usChannel, VT_MODE& VTMode)
{
	auto iterBoard = m_mapBoard.find(bySlotNo);
	if (m_mapBoard.end() == iterBoard || nullptr == iterBoard->second)
	{
		///<The board is not existed
		return -1;
	}
	int nRetVal = 0;
	nRetVal = iterBoard->second->GetVTMode(usChannel, VTMode);
	if (0 != nRetVal)
	{
		switch (nRetVal)
		{
		case -1:
			///<The channel is over range
			nRetVal = -2;
			break;
		case -2:
			///<The channel is not existed
			nRetVal = -3;
			break;
		default:
			break;
		}
	}
	return nRetVal;
}

int CDCM::SetPrereadVector(const char* lpszStartLabel, const char* lpszStopLabel)
{
	if (!m_bLoadVector)
	{
		m_pAlarm->VectorNotLoadedAlarm();
		return -1;
	}

	int nRetVal = 0;
	int nStartLabelLineNo = m_VectorInfo.GetLabelLineNo(lpszStartLabel, TRUE);
	if (0 > nStartLabelLineNo)
	{
		nRetVal = nStartLabelLineNo;
		m_pAlarm->SetParamName("lpszStartLabel");
		switch (nRetVal)
		{
		case -1:
			m_pAlarm->SetAlarmID(ALARM_ID::ALARM_PARAM_NULLPTR);
			m_pAlarm->SetAlarmMsg("The point of label is nullptr.");
			nRetVal = -2;
			break;
		case -2:
			m_pAlarm->SetAlarmID(ALARM_ID::ALARM_START_LABEL_ERROR);
			m_pAlarm->SetAlarmType(CDriverAlarm::ALARM_TYPE::PARAMETER_NOT_DEFINED);
			m_pAlarm->SetAlarmMsg("The label(%s) is not existed in vector file.", lpszStartLabel);
			nRetVal = -3;
			break;
		default:
			break;
		}
		return nRetVal;
	}
	int nStopLabelLineNo = 0;

	nStopLabelLineNo = m_VectorInfo.GetLabelLineNo(lpszStopLabel, TRUE);
	if (0 > nStopLabelLineNo)
	{
		nRetVal = nStopLabelLineNo;
		m_pAlarm->SetParamName("lpszStopLabel");
		switch (nRetVal)
		{
		case -1:
			m_pAlarm->SetAlarmID(ALARM_ID::ALARM_PARAM_NULLPTR);
			m_pAlarm->SetAlarmMsg("The point of label is nullptr.");
			nRetVal = -2;
			break;
		case -2:
			m_pAlarm->SetAlarmID(ALARM_ID::ALARM_STOP_LABEL_ERROR);
			m_pAlarm->SetAlarmType(CDriverAlarm::ALARM_TYPE::PARAMETER_NOT_DEFINED);
			m_pAlarm->SetAlarmMsg("The label(%s) is not existed in vector file.", lpszStopLabel);
			nRetVal = -4;
			break;
		default:
			break;
		}
		return nRetVal;
	}
	if (nStartLabelLineNo >= nStopLabelLineNo)
	{
		m_pAlarm->SetAlarmID(ALARM_ID::ALARM_START_LABEL_ERROR);
		m_pAlarm->SetParamName("lpszStartLabel");
		m_pAlarm->SetAlarmMsg("The start label(\"%s\") must before the stop label(\"%s\")!", lpszStartLabel, lpszStopLabel);
		return -5;

	}

	int uStartLine[2] = { 0 };///<The start line of vector in BRAM and DRAM
	int uLineCount[2] = { 0 };///<The start line of vector in BRAM and DRAM
	m_VectorInfo.GetLineInfo(nStartLabelLineNo, nStopLabelLineNo, uStartLine, uLineCount, m_mapLineInfo, FALSE);
	
	BOOL bNoBoard = TRUE;
	vector<USHORT> vecChannel;
	
	GetAllBoardChannel();

	BOARD_CHANNEL_BEGIN(vecChannel)
	{
		bNoBoard = FALSE;
		if (0 != uLineCount[0])
		{
			nRetVal = iterBoard->second->SetPrereadLine(vecChannel, uStartLine[0], uLineCount[0], MEM_TYPE::BRAM);
		}
		if (0 != uLineCount[1])
		{
			nRetVal = iterBoard->second->SetPrereadLine(vecChannel, uStartLine[1], uLineCount[1], MEM_TYPE::DRAM);
		}
		if (0 != nRetVal)
		{
			BOOL bAddAlarm = !m_pAlarm->IsSetMsg();
			switch (nRetVal)
			{
			case -1:
				///<The preread line count have reached to maximum limited
				if (bAddAlarm)
				{
					m_pAlarm->SetAlarmMsg("The preread line count have reach to maximum limited.");
				}
				nRetVal = -6;
				break;
			case -4:

				///<Allocate memory fail
				if (bAddAlarm)
				{
					m_pAlarm->AllocateMemoryError();
				}
				nRetVal = -7;
				break;
			default:
				break;
			}
			break;
		}
	}
	BOARD_CHANNEL_END

	if (bNoBoard)
	{
		m_pAlarm->SetAlarmID(ALARM_ID::ALARM_BOARD_NOT_EXISTED);
		m_pAlarm->SetAlarmMsg("No valid board existed.");
		nRetVal = -8;
	}
	return nRetVal;
}

int CDCM::SetLineInfo(const char* lpszPinGroup, USHORT usSiteNo, const char* lpszStartLabel, ULONG ulOffset, int nWriteVectorLineCount, BOOL bDataSame)
{
	if (!m_bLoadVector)
	{
		m_pAlarm->VectorNotLoadedAlarm();
		return -1;
	}
	set<string> setPinName;
	int nRetVal = GetChannel(lpszPinGroup, usSiteNo, TRUE, &setPinName);
	if (0 != nRetVal)
	{
		switch (nRetVal)
		{
		case -2:
			//The pin group is nullptr
			nRetVal = -2;
			break;
		case -3:
			//The pin group is not defined before
			nRetVal = -3;
			break;
		case -4:
			///<The site is over range
			nRetVal = -4;
			break;
		case -5:
			///<The site is invalid
			nRetVal = -5;
			break;
		case -6:
			///<The pin is not belongs to
			nRetVal = -6;
			break;
		default:
			///<Not will happen
			break;
		}
		return nRetVal;
	}
	m_usDataSiteNo = usSiteNo;

	if (bDataSame)
	{
		set<USHORT> setSite;
		GetSiteInfo(usSiteNo, setSite);
		int nBind = Bind(setPinName, setSite, TRUE);
		if (0 <= nBind)
		{
			m_usDataSiteNo = nBind;
		}
	}

	int nLabelLineNo = m_VectorInfo.GetLabelLineNo(lpszStartLabel, TRUE);
	if (0 > nLabelLineNo)
	{
		nRetVal = nLabelLineNo;
		m_pAlarm->SetPinString(lpszPinGroup, TRUE);
		m_pAlarm->SetSite(usSiteNo);
		m_pAlarm->SetParamName("lpszStartLabel");
		switch (nRetVal)
		{
		case -1:
			m_pAlarm->SetAlarmID(ALARM_ID::ALARM_PARAM_NULLPTR);
			m_pAlarm->SetAlarmMsg("The point of label is nullptr.");
			nRetVal = -7;
			break;
		case -2:
			m_pAlarm->SetAlarmID(ALARM_ID::ALARM_START_LABEL_ERROR);
			m_pAlarm->SetAlarmType(CDriverAlarm::ALARM_TYPE::PARAMETER_NOT_DEFINED);
			m_pAlarm->SetAlarmMsg("The label(%s) is not existed in vector file.", lpszStartLabel);
			nRetVal = -8;
			break;
		default:
			break;
		}
		return nRetVal;
	}
	UINT uGlobalStartLine = nLabelLineNo + ulOffset;

	int nVecctorLineCount = m_VectorInfo.GetBRAMLineCount() + m_VectorInfo.GetDRAMLineCount();

	if (nVecctorLineCount <= uGlobalStartLine)
	{
		m_pAlarm->SetAlarmID(ALARM_ID::ALARM_OFFSET_OVER_RANGE);
		m_pAlarm->SetAlarmType(CDriverAlarm::ALARM_TYPE::PARAMETER_OVERRANGE);
		m_pAlarm->SetParamName("ulOffset");
		m_pAlarm->SetAlarmMsg("The offset(%d) is over range[%d, %d].", ulOffset, 0, nVecctorLineCount - nLabelLineNo - 1);
		return -9;
	}

	UINT uGlobalStopLine = uGlobalStartLine + nWriteVectorLineCount - 1;

	int nStartLine[2] = { 0 };///<The start line of vector in BRAM and DRAM
	nRetVal = m_VectorInfo.GetLineInfo(uGlobalStartLine, uGlobalStopLine, nStartLine, m_nLineCount, m_mapLineInfo, FALSE);
	if (0 > nRetVal)
	{
		m_pAlarm->SetPinString(lpszPinGroup, TRUE);
		m_pAlarm->SetSite(usSiteNo);
		switch (nRetVal)
		{
		case -2:
			///<The line count is over range
			m_pAlarm->SetAlarmID(ALARM_ID::ALARM_LINE_COUNT_OVER_RANGE);
			m_pAlarm->SetParamName("nWriteVectorLineCount");
			m_pAlarm->SetAlarmMsg("The line count(%d) is over range.", nWriteVectorLineCount);
			nRetVal = -10;
			break;

		default:
			break;
		}
		return nRetVal;
	}
	BOOL bNoBoard = TRUE;
	vector<USHORT> vecChannel;

	BOARD_CHANNEL_BEGIN(vecChannel)
	{
		bNoBoard = FALSE;
		UINT uCurLineCount = 0;
		UINT uCurStartLine = 0;
		MEM_TYPE MemType = MEM_TYPE::BRAM;

		for (int nMemIndex = 0; nMemIndex < 2; ++nMemIndex)
		{
			uCurStartLine = nStartLine[nMemIndex];
			uCurLineCount = m_nLineCount[nMemIndex];
			if (0 == uCurLineCount)
			{
				continue;
			}
			if (0 == nMemIndex)
			{
				MemType = MEM_TYPE::BRAM;
			}
			else
			{
				MemType = MEM_TYPE::DRAM;
			}
			nRetVal = iterBoard->second->SetLineInfo(vecChannel, uCurStartLine, uCurLineCount, MemType);
			if (0 != nRetVal)
			{
				BOOL bExit = TRUE;
				m_pAlarm->SetPinString(lpszPinGroup, TRUE);
				m_pAlarm->SetSite(usSiteNo);
				BOOL bAddAlarm = !m_pAlarm->IsSetMsg();
				switch (nRetVal)
				{
				case -1:
				case -2:
					m_pAlarm->SetAlarmID(ALARM_ID::ALARM_LINE_COUNT_OVER_RANGE);
					m_pAlarm->SetParamName("nWriteVectorLineCount");
					if (bAddAlarm)
					{
						m_pAlarm->SetAlarmType(CDriverAlarm::ALARM_TYPE::PARAMETER_OVERRANGE);
						m_pAlarm->SetAlarmMsg("The line count(%d) is over range.", nWriteVectorLineCount);
					}
					nRetVal = -10;
					break;
				case -3:
					if (bAddAlarm)
					{
						m_pAlarm->SetAlarmID(ALARM_ID::ALARM_ALLOCTE_MEMORY_ERROR);
						m_pAlarm->SetAlarmMsg("Allocate memory fail.");
					}
					nRetVal = -11;
					break;
				case -4:
					m_pAlarm->SetAlarmID(ALARM_ID::ALARM_CHANNEL_NOT_EXISTED);
					m_pAlarm->SetAlarmLevel(CDriverAlarm::ALARM_LEVEL::AlarmWarning);
					m_pAlarm->SetParamName("lpszPinGroup");
					m_pAlarm->SetAlarmMsg("The channels in board(%d) used in pin group(%s) are not existed.", iterBoard->first, lpszPinGroup);
					m_pAlarm->Output(FALSE);
					nRetVal = -12;
					bExit = FALSE;
					break;
				default:
					break;
				}
				if (bExit)
				{
					break;
				}
			}

		}
		if (0 != nRetVal)
		{
			break;
		}
	}
	BOARD_CHANNEL_END


	if (bNoBoard)
	{
		m_pAlarm->SetNoBoardAlarm(lpszPinGroup, FALSE, usSiteNo);
		nRetVal = -13;
	}
	if (0 == nRetVal)
	{
		m_strWriteGetPinGroup = lpszPinGroup;
	}

	return nRetVal;
}

int CDCM::SetSiteWaveData(USHORT usSiteNo, const BYTE* pbyWaveData)
{
	if (!m_bLoadVector)
	{
		m_pAlarm->VectorNotLoadedAlarm();
		return -1;
	}
	if (0 == m_mapLineInfo.size())
	{
		m_pAlarm->SetAlarmID(ALARM_ID::ALARM_FUNCTION_USE_ERROR);
		m_pAlarm->SetSite(usSiteNo);
		m_pAlarm->SetAlarmMsg("Not use function SetWaveDataParam before.");
		return -2;
	}
	if (CBindInfo::Instance()->IsBind())
	{
		if (m_bSetSiteData)
		{
			return 0;
		}
		m_bSetSiteData = TRUE;
		usSiteNo = m_usDataSiteNo;
	}

	int nRetVal = 0;
	nRetVal = GetChannel(m_strWriteGetPinGroup.c_str(), usSiteNo);
	if (0 != nRetVal)
	{
		BOOL bExit = TRUE;
		switch (nRetVal)
		{
		case -4:
			m_pAlarm->SetAlarmID(ALARM_ID::ALARM_SITE_OVER_RANGE);
			m_pAlarm->SetAlarmType(CDriverAlarm::ALARM_TYPE::PARAMETER_OVERRANGE);
			m_pAlarm->SetAlarmMsg("The site number(%d) is over range[%d, %d].", usSiteNo, 0, m_Site.GetSiteCount() - 1);
			nRetVal = -3;
			break;
		case -5:
			///<Site invalid
			bExit = TRUE;
			nRetVal = -4;
			break;
		default:
			break;
		}
		if (bExit)
		{
			return nRetVal;
		}
	}
	if (nullptr == pbyWaveData)
	{
		m_pAlarm->SetAlarmID(ALARM_ID::ALARM_PARAM_NULLPTR);
		m_pAlarm->SetSite(usSiteNo);
		m_pAlarm->SetParamName("pbyWaveData");
		m_pAlarm->SetAlarmMsg("The point of wave data is nullptr.");
		return -5;
	}

	BYTE* pbyBRAMData = nullptr;
	BYTE* pbyDRAMData = nullptr;
	try
	{
		pbyBRAMData = new BYTE[m_nLineCount[0]];
		pbyDRAMData = new BYTE[m_nLineCount[1]];
		memset(pbyBRAMData, 0, m_nLineCount[0] * sizeof(BYTE));;
		memset(pbyDRAMData, 0, m_nLineCount[1] * sizeof(BYTE));;
	}
	catch (const std::exception&)
	{
		return -6;
	}
	m_VectorInfo.SplitData(m_mapLineInfo, pbyWaveData, m_nLineCount[0] + m_nLineCount[1], pbyBRAMData, m_nLineCount[0], pbyDRAMData, m_nLineCount[1]);

	BOOL bNoBoard = TRUE;
	vector<USHORT> vecChannel;

	BOARD_CHANNEL_BEGIN(vecChannel)
	{
		bNoBoard = FALSE;
		if (0 != m_nLineCount[0])
		{
			nRetVal = iterBoard->second->SetWaveData(vecChannel, MEM_TYPE::BRAM, pbyBRAMData);
		}
		if (0 != m_nLineCount[1])
		{
			nRetVal = iterBoard->second->SetWaveData(vecChannel, MEM_TYPE::DRAM, pbyDRAMData);
		}
	}
	BOARD_CHANNEL_END


	if (nullptr != pbyBRAMData)
	{
		delete[] pbyBRAMData;
		pbyBRAMData = nullptr;
	}
	if (nullptr != pbyDRAMData)
	{
		delete[] pbyDRAMData;
		pbyDRAMData = nullptr;
	}
	if (bNoBoard)
	{
		m_pAlarm->SetNoBoardAlarm(m_strWriteGetPinGroup.c_str(), FALSE, usSiteNo);
		nRetVal = -7;
	}

	return nRetVal;
}

int CDCM::WriteData()
{
	m_bSetSiteData = FALSE;
	if (!m_bLoadVector)
	{
		m_pAlarm->VectorNotLoadedAlarm();
		return -1;
	}
	if (0 == m_mapLineInfo.size())
	{
		m_pAlarm->SetAlarmID(ALARM_ID::ALARM_FUNCTION_USE_ERROR);
		m_pAlarm->SetSite(m_usDataSiteNo);
		m_pAlarm->SetAlarmMsg("Not use function SetWaveDataParam before.");
		CBindInfo::Instance()->ClearBind();
		return -2;
	}
	int nRetVal = 0;
	nRetVal = GetChannel(m_strWriteGetPinGroup.c_str(), m_usDataSiteNo);
	if (0 != nRetVal)
	{
		switch (nRetVal)
		{
		case -4:
			m_pAlarm->SetAlarmID(ALARM_ID::ALARM_SITE_OVER_RANGE);
			m_pAlarm->SetAlarmType(CDriverAlarm::ALARM_TYPE::PARAMETER_OVERRANGE);
			m_pAlarm->SetAlarmMsg("The site number(%d) is over range[%d, %d].", m_usDataSiteNo, 0, m_Site.GetSiteCount() - 1);
			nRetVal = -3;
			break;
		case -5:
			///<Site invalid
			nRetVal = -4;
			break;
		default:
			break;
		}
		CBindInfo::Instance()->ClearBind();
		return nRetVal;
	}
	BOOL bNoBoard = TRUE;
	vector<USHORT> vecChannel;

	BOARD_CHANNEL_BEGIN(vecChannel)
	{
		bNoBoard = FALSE;

		iterBoard->second->WriteData();
	}
	BOARD_CHANNEL_END

	if (bNoBoard)
	{
		nRetVal = -5;
	}
	CBindInfo::Instance()->ClearBind();
	return nRetVal;
}

void CDCM::GetValidBoard(std::vector<BYTE>& vecBoard)
{
	vecBoard.clear();
	for (auto& Board : m_mapBoard)
	{
		vecBoard.push_back(Board.first);
	}
}

int CDCM::GetVectorBoardCount(USHORT* pusSiteCount)
{
	if (!m_bLoadVector)
	{
		return -1;
	}
	vector<BYTE> vecBoard;
	m_Site.GetUseBoard(vecBoard);
	if (nullptr!= pusSiteCount)
	{
		*pusSiteCount = m_Site.GetSiteCount();
	}
	return vecBoard.size();
}

int CDCM::GetChannel(const char* lpszPinName, USHORT usSiteNo, CHANNEL_INFO& Channel)
{
	return GetBoardChannel(lpszPinName, usSiteNo, Channel);
}

int CDCM::GetChannel(const char* lpszPinGroup, USHORT usSiteNo, std::vector<CHANNEL_INFO>& vecChannel, BOOL bOnlyValidSite)
{
	vecChannel.clear();	
	set<string> setPinName;
	int nRetVal = 0;
	nRetVal = GetPinName(lpszPinGroup, setPinName);
	if (0 != nRetVal)
	{
		return nRetVal;
	}

	USHORT usStartSite = 0;
	USHORT usStopSite = m_Site.GetSiteCount() - 1;
	if ((USHORT) -1 != usSiteNo)
	{
		if (bOnlyValidSite && !m_Site.IsSiteValid(usSiteNo))
		{
			return -4;
		}
		usStartSite = usSiteNo;
		usStopSite = usSiteNo + 1;
	}
	CHANNEL_INFO Channel;

	for (auto strPinName : setPinName)
	{
		auto iterPin = m_mapPin.find(strPinName);
		if (m_mapPin.end() == iterPin || nullptr == iterPin->second)
		{
			continue;
		}
		for (USHORT usSiteNo = usStartSite; usSiteNo < usStopSite; ++usSiteNo)
		{
			if (bOnlyValidSite && !m_Site.IsSiteValid(usSiteNo))
			{
				continue;
			}
			nRetVal = iterPin->second->GetChannel(usSiteNo, Channel);
			if (0 != nRetVal)
			{
				continue;
			}
			vecChannel.push_back(Channel);
		}
	}
	
	return 0;
}

int CDCM::GetPinNo(const char* lpszPinGroup, std::vector<USHORT>& vecPinNo)
{
	vecPinNo.clear();
	set<string> setPinName;
	int nRetVal = GetPinName(lpszPinGroup, setPinName);
	if (0 != nRetVal)
	{
		return nRetVal;
	}
	auto iterPin = m_mapPin.begin();
	for (auto& PinName : setPinName)
	{
		iterPin = m_mapPin.find(PinName);
		if (m_mapPin.end() != iterPin && nullptr != iterPin->second)
		{
			vecPinNo.push_back(iterPin->second->GetID());
		}
	}

	return 0;
}

void CDCM::GetBoardSite(BYTE bySlotNo, std::vector<USHORT>& vecSite) const
{
	m_Site.GetBoardSite(bySlotNo, vecSite);
}

int CDCM::GetPinGroupChannel(const char* lpszPinGroup, USHORT usSiteNo, std::vector<CHANNEL_INFO>& vecChannel)
{
	vecChannel.clear();
	if (!m_bLoadVector)
	{
		return -1;
	}
	if (nullptr == lpszPinGroup)
	{
		return -2;
	}
	BOOL bPinGroup = TRUE;
	auto iterPin = m_mapPin.begin();
	auto iterPinGroup = m_mapPinGroup.find(lpszPinGroup);
	if (m_mapPinGroup.end() == iterPinGroup)
	{
		iterPin = m_mapPin.find(lpszPinGroup);
		if (m_mapPin.end() == iterPin || nullptr == iterPin->second)
		{
			if (m_setPinUnowned.end() == m_setPinUnowned.find(lpszPinGroup))
			{
				return -3;
			}
			return -5;
		}
		bPinGroup = FALSE;
	}
	else if (nullptr == iterPinGroup->second)
	{
		return -3;
	}
	if (0xFFFF != usSiteNo && m_Site.GetSiteCount() <= usSiteNo)
	{
		return -4;
	}

	int nChannelIndex = 0;
	CHANNEL_INFO Channel;
	if (bPinGroup)
	{
		set<string> setPin;
		iterPinGroup->second->GetPinName(setPin);
		for (auto& strPin : setPin)
		{
			auto iterPin = m_mapPin.find(strPin);
			if (m_mapPin.end() == iterPin || nullptr == iterPin->second)
			{
				continue;
			}
			if (0xFFFF != usSiteNo)
			{
				iterPin->second->GetChannel(usSiteNo, Channel);
				vecChannel.push_back(Channel);
			}
			else
			{
				iterPin->second->GetAllChannel(vecChannel, TRUE);
			}
		}
	}
	else
	{
		if (0xFFFF != usSiteNo)
		{
			iterPin->second->GetChannel(usSiteNo, Channel);
			vecChannel.push_back(Channel);
		}
		else
		{
			iterPin->second->GetAllChannel(vecChannel, TRUE);
		}

	}
	return 0;
}

int CDCM::GetAllPinGroupChannel(std::vector<CHANNEL_INFO>& vecChannel)
{
	vecChannel.clear();
	if (!m_bLoadVector)
	{
		return -1;
	}
	set<string> setAllPinName;
	set<string> setPinName;
	for (auto& PinGroup : m_mapPinGroup)
	{
		PinGroup.second->GetPinName(setPinName);
		for (auto& PinName : setPinName)
		{
			setAllPinName.insert(PinName);
		}
	}
	for (auto& PinName : setAllPinName)
	{
		auto iterPinName = m_mapPin.find(PinName);
		if (m_mapPin.end() == iterPinName)
		{
			continue;
		}
		iterPinName->second->GetAllChannel(vecChannel, TRUE);
	}
	return 0;
}

int CDCM::GetLineCount(const char* lpszStartLabel, const char* lpszStopLabel)
{
	if (!m_bLoadVector)
	{
		return -1;
	}
	int nStartLine = 0;
	int uStopLine = 0;
	if (nullptr == lpszStartLabel || 0 == strlen(lpszStartLabel))
	{
		nStartLine = 0;
	}
	else
	{
		nStartLine = m_VectorInfo.GetLabelLineNo(lpszStartLabel, TRUE);
		if (0 > nStartLine)
		{
			return -2;
		}
	}
	if (nullptr == lpszStopLabel || 0 == strlen(lpszStopLabel))
	{
		uStopLine = m_VectorInfo.GetBRAMLineCount() + m_VectorInfo.GetDRAMLineCount() - 1;
	}
	else
	{
		uStopLine = m_VectorInfo.GetLabelLineNo(lpszStopLabel, TRUE);
		if (0 > uStopLine)
		{
			return -3;
		}
	}

	return uStopLine - nStartLine + 1;
}

int CDCM::GetPattern(BYTE bySlotNo, BYTE byControllerIndex, BOOL bBRAM, UINT uStartLine, UINT uLineCount, char(*lpszPattern)[17])
{
	auto iterBoard = m_mapBoard.find(bySlotNo);
	if (m_mapBoard.end() == iterBoard || nullptr == iterBoard->second)
	{
		return -1;
	}
	int nRetVal = 0;
	nRetVal = iterBoard->second->GetVector(byControllerIndex, bBRAM, uStartLine, uLineCount, lpszPattern);
	if (0 != nRetVal)
	{
		switch (nRetVal)
		{
		case -1:
			///<The controller is over range
			nRetVal = -2;
			break;
		case -2:
			///<The controller is not existed
			nRetVal = -3;
			break;
		case -3:
			///<Start line over range
			nRetVal = -4;
		case -4:
			///<The line count is over range
			nRetVal = -5;
			break;
		case -5:
			///<The point of pattern is nullptr
			nRetVal = -6;
			break;
		case -6:
			///<Allocate memory fail
			nRetVal = -7;
			break;
		default:
			break;
		}
	}
	return nRetVal;
}

int CDCM::GetMemory(BYTE bySlotNo, BYTE byControllerIndex, BOOL bBRAM, DATA_TYPE DataType, UINT uStartLine, UINT uLineCount, USHORT* pusData)
{
	auto iterBoard = m_mapBoard.find(bySlotNo);
	if (m_mapBoard.end() == iterBoard || nullptr == iterBoard->second)
	{
		return -1;
	}
	int nRetVal = iterBoard->second->GetMemory(byControllerIndex, bBRAM, DataType, uStartLine, uLineCount, pusData);
	if (0 != nRetVal)
	{
		switch (nRetVal)
		{
		case -1:
			///<Controller index is over range
			nRetVal = -2;
			break;
		case -2:
			///<The controller is not existed
			nRetVal = -3;
			break;
		case -3:
			///<The data type is not supported
			nRetVal = -4;
			break;
		case -4:
			///<The start line is over range
			nRetVal = -5;
			break;
		case -5:
			///<The data count is over range
			nRetVal = -6;
			break;
		case -6:
			///<The point of data is nullptr or the read data count is 0
			nRetVal = -7;
			break;
		default:
			break;
		}
	}
	return nRetVal;
}

int CDCM::SetMemory(BYTE bySlotNo, USHORT usChannel, BOOL bRAM, DATA_TYPE DataType, UINT uStartLine, UINT uLineCount, BYTE* pbyData)
{
	auto iterBoard = m_mapBoard.find(bySlotNo);
	if (m_mapBoard.end() == iterBoard || nullptr == iterBoard->second)
	{
		return -1;
	}
	int nRetVal = iterBoard->second->SetMemory(usChannel, bRAM, DataType, uStartLine, uLineCount, pbyData);
	if (0 != nRetVal)
	{
		switch (nRetVal)
		{
		case -1:
			///<The channel is over range
			nRetVal = -2;
			break;
		case -2:
			///<The channel is not existed
			nRetVal = -3;
		case -3:
			///<The data type is not supported
			nRetVal = -4;
			break;
		case -4:
			///<Allocate memory fail
			nRetVal = -5;
			break;
		case -5:
			///<The start line is over range
			nRetVal = -6;
			break;
		case -6:
			///<The line count is over range
			nRetVal = -7;
			break;
		case -7:
			///<The line count is 0
			nRetVal = -8;
			break;
		case -8:
			///<The point of data is nullptr
			nRetVal = -9;
			break;
		default:
			break;
		}
	}
	return nRetVal;
}

int CDCM::ShieldAlarm(const char* lpszPinName, USHORT usSiteNo, BOOL bMask)
{
	if (!m_bLoadVector)
	{
		m_pAlarm->VectorNotLoadedAlarm();
		return -1;
	}
	CHANNEL_INFO Channel;
	int nRetVal = GetBoardChannel(lpszPinName, usSiteNo, Channel);
	if (0 != nRetVal)
	{
		return nRetVal;
	}
	m_pAlarm->ShieldPin(lpszPinName, usSiteNo, Channel, bMask);
	return 0;
}

int CDCM::GetShieldStatus(const char* lpszPinName, USHORT usSiteNo)
{
	if (!m_bLoadVector)
	{
		m_pAlarm->VectorNotLoadedAlarm();
		return -1;
	}
	CHANNEL_INFO Channel;
	int nRetVal = GetBoardChannel(lpszPinName, usSiteNo, Channel);
	if (0 != nRetVal)
	{
		return nRetVal;
	}	
	return m_pAlarm->GetShieldStatus(lpszPinName, usSiteNo);
}

int CDCM::ShieldFunctionAlarm(const char* lpszPinName, USHORT usSiteNo, const char* lpszFunctionName, BOOL bShield, ALARM_ID AlarmID)
{
	int nRetVal = 0;
	CHANNEL_INFO Channel;
	nRetVal = GetBoardChannel(lpszPinName, usSiteNo, Channel);
	if (0 != nRetVal)
	{
		return nRetVal;
	}
	auto iterBoard = m_mapBoard.find(Channel.m_bySlotNo);
	if(m_mapBoard.end() == iterBoard || nullptr == iterBoard->second)
	{
		m_pAlarm->SetNoBoardAlarm(lpszPinName, TRUE, usSiteNo);
		return -7;
	}
	if (0 == iterBoard->second->IsChannelExisted(Channel.m_usChannel))
	{
		m_pAlarm->SetSite(usSiteNo);
		m_pAlarm->SetPinString(lpszPinName, TRUE);
		m_pAlarm->SetParamName("lpszPinName");
		m_pAlarm->SetAlarmID(ALARM_ID::ALARM_CHANNEL_NOT_EXISTED);
		m_pAlarm->SetAlarmMsg("The channel(S%d_%d) of pin(%s) in SITE_%d is not existed.",
			Channel.m_bySlotNo, Channel.m_usChannel, lpszPinName, usSiteNo + 1);
		return -8;
	}
	string strFuncName = lpszFunctionName;
	if (-1 != strFuncName.find("PMU"))
	{
		iterBoard->second->EnablePMUClampFlag(Channel.m_usChannel, !bShield);
	}
	m_pAlarm->ShieldAlarm(lpszPinName, usSiteNo, Channel, lpszFunctionName, bShield, AlarmID);
	return 0;
}

int CDCM::GetShieldFunctionAlarm(const char* lpszPinName, USHORT usSiteNo, const char* lpszFunctionName, ALARM_ID AlarmID)
{
	int nRetVal = 0;
	CHANNEL_INFO Channel;
	nRetVal = GetBoardChannel(lpszPinName, usSiteNo, Channel);
	if (0 != nRetVal)
	{
		return nRetVal;
	}
	auto iterBoard = m_mapBoard.find(Channel.m_bySlotNo);
	if (m_mapBoard.end() == iterBoard || nullptr == iterBoard->second)
	{
		m_pAlarm->SetNoBoardAlarm(lpszPinName, TRUE, usSiteNo);
		return -7;
	}
	if (0 == iterBoard->second->IsChannelExisted(Channel.m_usChannel))
	{
		m_pAlarm->SetSite(usSiteNo);
		m_pAlarm->SetPinString(lpszPinName, TRUE);
		m_pAlarm->SetParamName("lpszPinName");
		m_pAlarm->SetAlarmID(ALARM_ID::ALARM_CHANNEL_NOT_EXISTED);
		m_pAlarm->SetAlarmMsg("The channel(S%d_%d) of pin(%s) in SITE_%d is not existed.", Channel.m_bySlotNo, Channel.m_usChannel, lpszPinName, usSiteNo + 1);
		return -8;
	}
	return m_pAlarm->GetShieldAlarm(lpszPinName, usSiteNo, lpszFunctionName, AlarmID);
}

int CDCM::GetSiteBoard(USHORT usSiteNo, std::map<BYTE, USHORT>& mapBoard)
{
	if (!m_bLoadVector)
	{
		return -1;
	}
	int nRetVal = m_Site.GetSiteBoard(usSiteNo, mapBoard);
	if (0 != nRetVal)
	{
		return -2;
	}
	return 0;
}

int CDCM::GetSiteChannel(USHORT usSiteNo, std::vector<CHANNEL_INFO>& vecChannel)
{
	vecChannel.clear();
	if (!m_bLoadVector)
	{
		return -1;
	}

	int nRetVal = m_Site.GetChannel(usSiteNo, vecChannel);
	if (0 != nRetVal)
	{
		nRetVal = -2;
	}
	return nRetVal;
}

int CDCM::GetSiteCount()
{
	if (!m_bLoadVector)
	{
		return -1;
	}
	return m_Site.GetSiteCount();
}

int CDCM::GetPinCount()
{
	return m_mapPin.size() + m_setPinUnowned.size();
}

double CDCM::GetPinLevel(BYTE bySlotNo, USHORT usChannel, LEVEL_TYPE LevelType)
{
	auto iterBoard = m_mapBoard.find(bySlotNo);
	if (m_mapBoard.end() == iterBoard || nullptr == iterBoard->second)
	{
		return 1e16;
	}
	return iterBoard->second->GetPinLevel(usChannel, LevelType);
}

double CDCM::GetPMUSettings(BYTE bySlotNo, USHORT usChannel, PMU_MODE& PMUMode, PMU_IRANGE& IRange)
{
	auto iterBoard = m_mapBoard.find(bySlotNo);
	if (m_mapBoard.end() == iterBoard || nullptr == iterBoard->second)
	{
		return MAX_MEASURE_VALUE;
	}
	return iterBoard->second->GetPMUMode(usChannel, PMUMode, IRange);
}

int CDCM::GetFailLineNo(BYTE bySlotNo, USHORT usChannel, BOOL bBRAM, UINT uGetMaxFailCount, std::vector<int>& vecLineNo)
{
	vecLineNo.clear();
	int nRetVal = 0;
	auto iterBoard = m_mapBoard.find(bySlotNo);
	if (m_mapBoard.end() == iterBoard || nullptr == iterBoard->second)
	{
		return -1;
	}
	vector<int> vecBRAMLine;
	vector<int> vecDRAMLine;

	vector<int> vecNotGetLine;
	vector<int>* pvecBRAMLine = &vecNotGetLine;
	vector<int>* pvecDRAMLine = &vecNotGetLine;
	if (bBRAM)
	{
		pvecBRAMLine = &vecLineNo;
	}
	else
	{
		pvecDRAMLine = &vecLineNo;
	}

	nRetVal = iterBoard->second->GetFailLineNo(usChannel, uGetMaxFailCount, *pvecBRAMLine, *pvecDRAMLine, TRUE);

	if (0 > nRetVal)
	{
		switch (nRetVal)
		{
		case -1:
			///<Channel is over range
			nRetVal = -2;
			break;
		case -2:
			///<Channel is not existed
			nRetVal = -3;
			break;
		case -3:
			///<Vector not ran
			nRetVal = -4;
			break;
		case -4:
			///<Vector running
			nRetVal = -5;
			break;
		default:
			break;
		}
	}
	return nRetVal;
}

int CDCM::GetChannelFailCount(BYTE bySlotNo, USHORT usChannel)
{
	auto iterBoard = m_mapBoard.find(bySlotNo);
	if (m_mapBoard.end() == iterBoard || nullptr == iterBoard->second)
	{
		return -1;
	}
	int nRetVal = iterBoard->second->GetFailCount(usChannel);
	if (0 > nRetVal)
	{
		switch (nRetVal)
		{
		case -1:
			///<The channel number is over range
			nRetVal = -2;
			break;
		case -2:
			///<The channel is not existed
			nRetVal = -3;
			break;
		case -3:
			///<Not ran
			nRetVal = -4;
			break;
		case -4:
			///<Running
			nRetVal = -5;
			break;
		default:
			break;
		}
	}
	return nRetVal;
}

int CDCM::Bind(const set<std::string>& setBindPin, const set<USHORT>& setBindSite, BOOL bSaveChannel)
{
	if (0 == setBindPin.size() || 0 == setBindSite.size())
	{
		return -1;
	}
	if (1 == setBindSite.size())
	{
		return -2;
	}

	int nBindSite = -1;
	int nRetVal = 0;
	int nChannel = 0;
	CHANNEL_INFO Channel;
	auto iterPin = m_mapPin.begin();

	BYTE byTargetSlot = 0;
	set<BYTE> setBindController;
	set<BYTE> setBindSlot;
	map<UINT, USHORT> mapControllerSite;
	vector<CHANNEL_INFO> vecChannel;///<Save the channel used after bind

	set<BYTE> setControllerPerSite;

	for (auto& PinName : setBindPin)
	{
		iterPin = m_mapPin.find(PinName);
		if (m_mapPin.end() == iterPin || nullptr == iterPin->second)
		{
			continue;
		}
		///<The pin is existed
		int nPreSiteChannel = -1;
		for (auto SiteNo : setBindSite)
		{
			iterPin->second->GetChannel(SiteNo, Channel);
			nChannel = Channel.m_bySlotNo * DCM_MAX_CHANNELS_PER_BOARD + Channel.m_usChannel;

			auto iterBoard = m_mapBoard.find(Channel.m_bySlotNo);
			if (m_mapBoard.end() == iterBoard || nullptr == iterBoard->second)
			{
				///<The channel is not existed
				continue;
			}
			UINT uController = Channel.m_usChannel / DCM_CHANNELS_PER_CONTROL;
			setBindSlot.insert(Channel.m_bySlotNo);
			setBindController.insert(uController);
			if (-1 == nBindSite)
			{
				nBindSite = SiteNo;
				byTargetSlot = Channel.m_bySlotNo;
			}

			if (bSaveChannel && nBindSite == SiteNo)
			{
				vecChannel.push_back(Channel);
			}
			if (m_bVectorBind)
			{
				///<The vector is loaded in parallel, no need to check bind condition
				continue;
			}

			if (nBindSite == SiteNo)
			{
				BYTE byCurController = Channel.m_usChannel / DCM_CHANNELS_PER_CONTROL;
				if (setControllerPerSite.end() == setControllerPerSite.find(byCurController))
				{
					setControllerPerSite.insert(byCurController);
				}
			}

			uController += Channel.m_bySlotNo * DCM_MAX_CONTROLLERS_PRE_BOARD;
			auto iterController = mapControllerSite.find(uController);
			if (mapControllerSite.end() != iterController && iterController->second != SiteNo)
			{
				///<The controller has been used by more than one site.
				nRetVal = -4;
				break;
			}
			else if (mapControllerSite.end() == iterController)
			{
				mapControllerSite.insert(make_pair(uController, SiteNo));
			}
			if (-1 == nPreSiteChannel)
			{
				nPreSiteChannel = nChannel;
			}

			if (0 != (nChannel - nPreSiteChannel) % DCM_CHANNELS_PER_CONTROL)
			{
				///<The channel offset to SITE_1 is not the multiples 16
				nRetVal = -5;
				break;
			}
			nPreSiteChannel = nChannel;
		}
		if (0 != nRetVal)
		{
			break;
		}
	}
	if (1 != setControllerPerSite.size())
	{
		nRetVal = -6;
	}

	if (0 != nRetVal)
	{
		return nRetVal;
	}
	if (bSaveChannel)
	{
		m_ClassifyBoard.SetChannel(vecChannel);
	}

	CBindInfo::Instance()->Bind(setBindSlot, setBindController, byTargetSlot);

	return nBindSite;
}

inline void CDCM::ClearBind()
{
	CBindInfo::Instance()->ClearBind();
}

void CDCM::ClearVector()
{
	m_bLoadVector = FALSE;
	m_Site.Reset();
	DeleteMemory(m_mapPin);
	DeleteMemory(m_mapPinGroup);
	m_mapTimeset.clear();
	m_nBRAMLeftStartLine = 0;
	m_nDRAMLeftStartLine = 0;
	m_mapLineInfo.clear();
	m_strLatestRanPinGroup.clear();
	m_VectorInfo.Reset();
	m_bWaitRun = FALSE;
	for (auto& Board : m_mapBoard)
	{
		if (nullptr == Board.second)
		{
			continue;
		}
		Board.second->ClearPreread();
	}
}

inline void CDCM::GetVectorInfoFile(std::string& strVectorInfoFile, const char* lpszVectorFile)
{
	HMODULE hModule = GetModuleHandle("DCM.dll");
	char lpszFileName[MAX_PATH] = { 0 };
	GetModuleFileName(hModule, lpszFileName, sizeof(lpszFileName));
	strVectorInfoFile = lpszFileName;
	int nPos = strVectorInfoFile.rfind("\\");
	if (-1 != nPos)
	{
		strVectorInfoFile.erase(nPos + 1);
	}
	strVectorInfoFile += "DCM\\";
	CreateDirectory(strVectorInfoFile.c_str(), nullptr);
	string strFileName = m_strFileName;
	if (nullptr != lpszVectorFile)
	{
		strFileName = lpszVectorFile;
	}
	nPos = strFileName.rfind("\\");
	if (-1 != nPos)
	{
		strFileName.erase(0, nPos + 1);
		nPos = strFileName.rfind(".");
		if (-1 != nPos)
		{
			strFileName.erase(nPos);
		}
	}
	strVectorInfoFile += strFileName;
	strVectorInfoFile += ".ini";
}

void CDCM::GetPinGroupInfoFile(std::string& strFile)
{
	HMODULE hModule = GetModuleHandle("DCM.dll");
	char lpszFileName[MAX_PATH] = { 0 };
	GetModuleFileName(hModule, lpszFileName, sizeof(lpszFileName));
	strFile = lpszFileName;
	int nPos = strFile.rfind("\\");
	if (-1 != nPos)
	{
		strFile.erase(nPos + 1);
	}
	strFile += "DCM\\";
	CreateDirectory(strFile.c_str(), nullptr);
	string strFileName = m_strFileName;
	
	nPos = strFileName.rfind("\\");
	if (-1 != nPos)
	{
		strFileName.erase(0, nPos + 1);
		nPos = strFileName.rfind(".");
		if (-1 != nPos)
		{
			strFileName.erase(nPos);
		}
	}
	strFile += strFileName;
	strFile += "_PinGroup.ini";
}

void CDCM::SaveVectorInformation(const char* lpszVectorFile, const std::set<std::string>& setFailSynPin)
{
	if (nullptr == lpszVectorFile)
	{
		return;
	}
	string strVectorInfoFile;
	GetVectorInfoFile(strVectorInfoFile);
	fstream File(strVectorInfoFile.c_str(), ios::out);
	//Get the modified time of the vector file.
	WIN32_FIND_DATA FileAttrib;
	HANDLE hFile = FindFirstFile(lpszVectorFile, &FileAttrib);
	SYSTEMTIME SystemTime;
	FileTimeToSystemTime(&(FileAttrib.ftLastWriteTime), &SystemTime);
	CConfigFile ConfigFile(strVectorInfoFile.c_str());

	ConfigFile.SetValue("Vector", "File", lpszVectorFile);
	ConfigFile.SetValue("Vector", "Time", "%4d%02d%02d%02d%02d%02d%03d", SystemTime.wYear, SystemTime.wMonth, SystemTime.wDay,
		SystemTime.wHour, SystemTime.wMinute, SystemTime.wSecond, SystemTime.wMilliseconds);

	char lpszOneValue[32] = { 0 };
	auto NumString = [&](UINT uNum)
	{
		sprintf_s(lpszOneValue, "%d", uNum);
		return lpszOneValue;
	};
	ConfigFile.ClearSection("LineInfo");///<Delete all item under app LineInfo
	ConfigFile.SetValue("LineInfo", "LineCount", "%d,%d", m_nBRAMLeftStartLine, m_nDRAMLeftStartLine);

	UINT uStartLine = 0;
	UINT uGlobalLine = 0;
	UINT uBlockLength = 0;
	int nDRAMBlockCount = m_VectorInfo.GetDRAMBlockCount();
	int nBlockCount = nDRAMBlockCount * 2 + 1;
	ConfigFile.SetValue("LineInfo", "Count", "%d", nBlockCount);

	char lpszKeyName[32] = { 0 };
	int nBlockIndex = 0;
	BOOL bBlockExisted = TRUE;
	int nRetVal = 0;
	do 
	{
		bBlockExisted = m_VectorInfo.GetBRAMBlock(nBlockIndex, uStartLine, uGlobalLine, uBlockLength);
		if (!bBlockExisted)
		{
			break;
		}
		ConfigFile.SetValue("LineInfo", NumString(nBlockIndex * 2), "%d,%d", uStartLine, uBlockLength);
		bBlockExisted = m_VectorInfo.GetDRAMBlock(nBlockIndex, uStartLine, uGlobalLine, uBlockLength);
		if (!bBlockExisted)
		{
			++nBlockIndex;
			continue;
		}
		ConfigFile.SetValue("LineInfo", NumString(nBlockIndex * 2 + 1), "%d,%d", uStartLine, uBlockLength);
		++nBlockIndex;
	} while (TRUE);

	int nLabelCount = m_VectorInfo.GetLabelCount();

	ConfigFile.ClearSection("Label");///<Delete all item under app LineInfo
	ConfigFile.SetValue("Label", "Count", "%d", nLabelCount);

	string strLabelName;
	int nLabelLineNo = 0;;
	for (int nIndex = 0; nIndex < nLabelCount; ++nIndex)
	{
		m_VectorInfo.GetLabelNameWithLabelIndex(nIndex, strLabelName);
		nLabelLineNo = m_VectorInfo.GetLabelLineNo(strLabelName.c_str());
		ConfigFile.SetValue("Label", NumString(nIndex), "%s,%d", strLabelName.c_str(), nLabelLineNo);
	}
	if (0 != setFailSynPin.size())
	{
		string strPinList;
		for (const auto& Pin : setFailSynPin)
		{
			strPinList += Pin;
			strPinList += ",";
		}
		ConfigFile.SetValue("FailSyn", "Pin", strPinList.c_str());
	}

	ConfigFile.Save();
}

int CDCM::LoadVectorInformation(const char* lpszFileName, const string& strVectInfoFilePath, std::set<std::string>& setFailSynPin, BOOL bCheckFile, std::string* pstrFileName)
{
	setFailSynPin.clear();
	if (bCheckFile && nullptr == lpszFileName)
	{
		return -1;
	}

	CConfigFile ConfigFile(strVectInfoFilePath.c_str());
	const char* lpszValue = ConfigFile.GetValue("Vector", "File");
	if (nullptr == lpszValue)
	{
		return -2;
	}
	if (bCheckFile)
	{
		if (0 != strcmp(lpszFileName, lpszValue))
		{
			///<File in board is not the vector file will be loaded
			return -2;
		}
		char lpszTime[64] = { 0 };
		WIN32_FIND_DATA FileAttrib;
		HANDLE hFile = FindFirstFile(lpszFileName, &FileAttrib);
		if (INVALID_HANDLE_VALUE != hFile)
		{
			SYSTEMTIME SystemTime;
			FileTimeToSystemTime(&(FileAttrib.ftLastWriteTime), &SystemTime);
			sprintf_s(lpszTime, sizeof(lpszTime), "%4d%02d%02d%02d%02d%02d%03d", SystemTime.wYear, SystemTime.wMonth, SystemTime.wDay,
				SystemTime.wHour, SystemTime.wMinute, SystemTime.wSecond, SystemTime.wMilliseconds);
		}

		lpszValue = ConfigFile.GetValue("Vector", "Time");
		if (nullptr == lpszValue || 0 != strcmp(lpszValue, lpszTime))
		{
			///<Vector file had been modified after latest load
			return -3;
		}
	}
	else if (nullptr != pstrFileName)
	{
		*pstrFileName = lpszValue;
	}

	string strData;
	auto GetNumber=[&](int* pnNumTwo)->int
	{
		strData = lpszValue;
		int nPos = strData.find(",");
		if (-1 == nPos)
		{
			if (nullptr != pnNumTwo)
			{
				*pnNumTwo = -1;
			}
			return atoi(strData.c_str());
		}
		if (nullptr != pnNumTwo)
		{
			*pnNumTwo = atoi(strData.substr(nPos + 1).c_str());
		}
		return atoi(strData.substr(0, nPos).c_str());
	};
	char lpszKey[32] = { 0 };
	auto GetNumKey = [&](int nNum)
	{
		sprintf_s(lpszKey, sizeof(lpszKey), "%d", nNum);
		return lpszKey;
	};
	int nRetVal = 0;

	auto GetNum = [&]()->double
	{
		if (nullptr == lpszValue)
		{
			return -1;
		}
		return atoi(lpszValue);
	};
	int nPos = 0;
	do
	{
		lpszValue = ConfigFile.GetValue("LineInfo", "LineCount");
		m_nBRAMLeftStartLine = GetNumber(&m_nDRAMLeftStartLine);

		lpszValue = ConfigFile.GetValue("LineInfo", "Count");
		
		int nBlockCount = GetNum();
		///<Load vector block
		int nGlobalLine = 0;
		int nStartLine = 0;
		int nBlockLength = 0;
		for (int nBlockIndex = 0; nBlockIndex < nBlockCount; ++nBlockIndex)
		{
			lpszValue = ConfigFile.GetValue("LineInfo", GetNumKey(nBlockIndex));
			nStartLine = GetNumber(&nBlockLength);
			if (-1 == nBlockLength)
			{
				nRetVal = -4;
				break;
			}
			if (0 == nBlockIndex % 2)
			{
				m_VectorInfo.AddBRAMBlock(nStartLine, nGlobalLine, nBlockLength);
			}
			else
			{
				m_VectorInfo.AddDRAMBlock(nStartLine, nGlobalLine, nBlockLength);
			}
			nGlobalLine += nBlockLength;
		}
		if (0 != nRetVal)
		{
			break;
		}
		///<Load label information
		int nLineNum = 0;
		lpszValue = ConfigFile.GetValue("Label", "Count");
		int nLabelCount = GetNum();
		for (int nIndex = 0; nIndex < nLabelCount; ++nIndex)
		{
			lpszValue = ConfigFile.GetValue("Label", GetNumKey(nIndex));
			strData = lpszValue;
			nPos = strData.find(",");
			if (-1 == nPos)
			{
				nRetVal = -5;
				break;
			}
			nLineNum = atoi(strData.substr(nPos + 1).c_str());
			m_VectorInfo.AddLabel(strData.substr(0, nPos), nLineNum);
		}
		const char* lpszPinList = ConfigFile.GetValue("FailSyn", "Pin");
		if (nullptr == lpszPinList || 0 == strlen(lpszPinList))
		{
			///<No fail sychronous
			break;
		}
		string strPinList = lpszPinList;
		nPos = 0;
		while (-1 != nPos)
		{
			nPos = strPinList.find(",");
			if (-1 == nPos)
			{
				if (0 == strPinList.size())
				{
					continue;
				}
				setFailSynPin.insert(strPinList);
			}
			setFailSynPin.insert(strPinList.substr(0, nPos));
			strPinList.erase(0, nPos + 1);
		}
	} while (false);
	if (0 != nRetVal)
	{
		ConfigFile.ClearSection("Vector");
		ConfigFile.ClearSection("LineInfo");
		ConfigFile.ClearSection("Label");
		ConfigFile.Save();
	}

	return nRetVal;
}

int CDCM::LoadVectorFileTimeset(const std::vector<USHORT>& vecSite, std::map<BYTE, CTimeset*>& mapTimeset)
{
	int nRetVal = 0;
	map<BYTE, std::vector<USHORT>> mapChannel;
	vector<string> vecPin;
	auto iterPin = m_mapPin.begin();

	CEdge Edge;
	WAVE_FORMAT WaveFormat;
	IO_FORMAT IOFormat;
	COMPARE_MODE CompareMode;
	double dEdge[6] = { 0 };
	BOOL bFail = FALSE;
	BOOL bNoBoard = TRUE;
	auto iterBoard = m_mapBoard.begin();

	bNoBoard = FALSE;
	for (auto& Timeset : mapTimeset)
	{
		USHORT usEdgeCount = Timeset.second->GetEdgeCount();
		for (USHORT usEdgeIndex = 0; usEdgeIndex < usEdgeCount; ++usEdgeIndex)
		{
			nRetVal = Timeset.second->GetEdge(usEdgeIndex, Edge);
			if (0 != nRetVal)
			{
				nRetVal = -2;
				bFail = TRUE;
				break;
			}
			Edge.GetEdge(dEdge);

			vector<string> vecPin;
			Edge.GetPin(vecPin);

			GetBoardChannel(vecPin, vecSite, mapChannel);
			for (auto& Channel : mapChannel)
			{
				iterBoard = m_mapBoard.find(Channel.first);
				if (m_mapBoard.end() == iterBoard)
				{
					continue;
				}
				iterBoard->second->SetPeriod(Timeset.first, Timeset.second->GetPeriod());

				Edge.GetFormat(&WaveFormat, &IOFormat, &CompareMode);

				nRetVal = iterBoard->second->SetEdge(Channel.second, Timeset.first, dEdge, WaveFormat, IOFormat, CompareMode);
				if (0 != nRetVal)
				{
					if (-5 == nRetVal)
					{
						///<The channel is not existed
						nRetVal = 0;
						m_pAlarm->SetAlarmID(ALARM_ID::ALARM_CHANNEL_NOT_EXISTED);
						m_pAlarm->SetAlarmMsg("The board(S%d) is existed, but no controller existed.", iterBoard->first);
						m_pAlarm->SetAlarmLevel(CDriverAlarm::ALARM_LEVEL::AlarmWarning);
						m_pAlarm->Output(FALSE);
						break;
					}
					switch (nRetVal)
					{
					case -1:
						///<The timeset index is over range
						///<Not will happened
						break;
					case -4:
						///<The edge value is over range
						nRetVal = -3;
						break;
					default:
						///<Not will happened
						break;
					}
					bFail = TRUE;
					break;
				}
			}
			if (bFail)
			{
				break;
			}
		}
		if (bFail)
		{
			break;
		}
	}

	for (auto& Channel : mapChannel)
	{
		Channel.second.clear();
	}
	mapChannel.clear();
	if (bNoBoard)
	{
		m_pAlarm->SetAlarmID(ALARM_ID::ALARM_BOARD_NOT_EXISTED);
		m_pAlarm->SetAlarmMsg("No valid board used in vector file existed.");
		m_pAlarm->SetAlarmLevel(CDriverAlarm::ALARM_LEVEL::AlarmError);
		nRetVal = -1;
	}
	return nRetVal;
}

int CDCM::GetBoardChannel(const std::vector<std::string>& vecPin, const std::vector<USHORT>& vecSite, std::map<BYTE, std::vector<USHORT>>& mapChannel)
{
	for (auto& Channel : mapChannel)
	{
		Channel.second.clear();
	}
	mapChannel.clear();
	if (0 == m_mapPin.size())
	{
		return -1;
	}
	CHANNEL_INFO ChannelInfo;
	auto iterPin = m_mapPin.begin();
	USHORT usPinCount = vecPin.size();
	USHORT usSiteCount = vecSite.size();
	BOOL bAllPinExist = TRUE;
	for (auto& PinName : vecPin)
	{
		iterPin = m_mapPin.find(PinName);
		if (m_mapPin.end() == iterPin)
		{
			bAllPinExist = FALSE;
			break;
		}
		for (auto usSiteNo : vecSite)
		{
			iterPin->second->GetChannel(usSiteNo, ChannelInfo);
			auto iterChannel = mapChannel.find(ChannelInfo.m_bySlotNo);
			if (mapChannel.end() == iterChannel)
			{
				vector<USHORT> vecChannel;
				mapChannel.insert(make_pair(ChannelInfo.m_bySlotNo, vecChannel));
				iterChannel = mapChannel.find(ChannelInfo.m_bySlotNo);
			}
			iterChannel->second.push_back(ChannelInfo.m_usChannel);
		}
	}
	if (!bAllPinExist)
	{
		return -1;
	}
	return 0;
}

inline int CDCM::GetBoardChannel(const char* lpszPinName, USHORT usSiteNo, CHANNEL_INFO& Channel)
{
	if (!m_bLoadVector)
	{
		m_pAlarm->VectorNotLoadedAlarm();
		return -1;
	}
	if (nullptr == lpszPinName)
	{
		m_pAlarm->ParameternullptrAlarm("lpszPinName", -1, nullptr);
		return -2;
	}
	int nRetVal = 0;
	if (m_Site.GetSiteCount() <= usSiteNo)
	{
		m_pAlarm->SetParamName("usSiteNo");
		m_pAlarm->SiteOverScaleAlarm(lpszPinName, usSiteNo, m_Site.GetSiteCount() - 1, TRUE);
		return -4;
	}
	if (!m_Site.IsSiteValid(usSiteNo))
	{
		return -5;
	}
	nRetVal = m_Site.GetChannel(lpszPinName, usSiteNo, Channel);
	if (0 != nRetVal)
	{
		switch (nRetVal)
		{
		case -1:
			///<The pin is not defined
			if (m_setPinUnowned.end() == m_setPinUnowned.find(lpszPinName))
			{
				m_pAlarm->SetSite(usSiteNo);
				m_pAlarm->PinError(lpszPinName, TRUE);
				nRetVal = -3;
			}
			else
			{
				m_pAlarm->SetAlarmID(ALARM_ID::ALARM_PIN_UNOWNED);
				m_pAlarm->SetAlarmType(CDriverAlarm::ALARM_TYPE::PARAMETER_NOT_DEFINED);
				m_pAlarm->SetPinString(lpszPinName, TRUE);
				m_pAlarm->SetAlarmMsg("The pin(%s) is not belongs to current instance.", lpszPinName);
				nRetVal = -6;
			}
			break;
		case -2:
			m_pAlarm->SetParamName("usSiteNo");
			m_pAlarm->SiteOverScaleAlarm(lpszPinName, usSiteNo, m_Site.GetSiteCount() - 1, TRUE);
			nRetVal = -4;
			break;
		default:
			m_pAlarm->SetSite(usSiteNo);
			nRetVal = -3;
			break;
		}
		return nRetVal;
	}
	return nRetVal;
}

inline BOOL CDCM::IsVectorValid(const char* lpszVectorFileName, const vector<BYTE>& vecUseBoard)
{
	auto iterBoard = m_mapBoard.begin();
	for (auto bySlotNo : vecUseBoard)
	{
		iterBoard = m_mapBoard.find(bySlotNo);
		if (m_mapBoard.end() == iterBoard)
		{
			continue;
		}
		if (!iterBoard->second->IsVectorValid())
		{
			return FALSE;
		}
	}

	string strVectorInfoFile;
	GetVectorInfoFile(strVectorInfoFile, lpszVectorFileName);
	set<string> setFailSynPin;
	int nRetVal = 0;
	nRetVal = LoadVectorInformation(lpszVectorFileName, strVectorInfoFile, setFailSynPin);
	if (0 != nRetVal)
	{
		return FALSE;
	}

	return TRUE;
}

inline int CDCM::ExtractPinName(const char* lpszPinList, std::set<std::string>& setPin)
{
	setPin.clear();
	if (nullptr == lpszPinList)
	{
		return -1;
	}
	if (0 == strlen(lpszPinList))
	{
		m_pAlarm->SetAlarmID(ALARM_ID::ALARM_PIN_NAME_STRING_FORMT_WRONG);
		m_pAlarm->SetAlarmMsg("No pin in lpszPinNameString.");
		return -2;
	}

	auto iterPin = m_mapPin.begin();
	string strPinNameList = lpszPinList;
	strPinNameList.erase(strPinNameList.find_last_not_of(' ') + 1);
	strPinNameList.erase(0, strPinNameList.find_first_not_of(' '));
	if (0 == strPinNameList.size())
	{
		m_pAlarm->SetAlarmID(ALARM_ID::ALARM_PIN_NAME_STRING_FORMT_WRONG);
		m_pAlarm->SetAlarmMsg("No pin in lpszPinNameString.");
		return -2;
	}

	string strPin;
	int nPos = 0;
	int nRetVal = 0;
	while (-1 != nPos)
	{
		nPos = strPinNameList.find(",");
		if (-1 == nPos)
		{
			if (0 == strPinNameList.size())
			{
				continue;
			}
			strPin = strPinNameList;
		}
		else
		{
			strPin = strPinNameList.substr(0, nPos);
			strPinNameList.erase(0, nPos + 1);
		}
		if (0 == strPin.size())
		{
			m_pAlarm->SetAlarmID(ALARM_ID::ALARM_PIN_NAME_STRING_FORMT_WRONG);
			m_pAlarm->SetAlarmMsg("The format of lpszPinNameString(%s) is wrong.", strPin.c_str());
			nRetVal = -3;
			break;
		}

		strPin.erase(0, strPin.find_first_not_of(' '));
		strPin.erase(strPin.find_last_not_of(' ') + 1);
		iterPin = m_mapPin.find(strPin);
		if (m_mapPin.end() == iterPin)
		{
			///<Support the pin group nested
			auto iterPinGroup = m_mapPinGroup.find(strPin);
			if (m_mapPinGroup.end() == iterPinGroup)
			{
				if (m_setPinUnowned.end() == m_setPinUnowned.find(strPin))
				{
					m_pAlarm->SetAlarmID(ALARM_ID::ALARM_PIN_NAME_STRING_FORMT_WRONG);
					m_pAlarm->SetAlarmType(CDriverAlarm::ALARM_TYPE::PARAMETER_NOT_DEFINED);
					m_pAlarm->SetAlarmMsg("The pin name(%s) is not defined in vector file.", strPin.c_str());
					nRetVal = -4;
				}
				else
				{
					m_pAlarm->SetAlarmID(ALARM_ID::ALARM_PIN_UNOWNED);
					m_pAlarm->SetAlarmType(CDriverAlarm::ALARM_TYPE::PARAMETER_NOT_DEFINED);
					m_pAlarm->SetAlarmMsg("The pin(%s) is not belongs to current instance.", strPin.c_str());
					nRetVal = -5;
				}
				break;
			}

			set<string> setPinGroupPin;
			iterPinGroup->second->GetPinName(setPinGroupPin);
			for (auto& PinName : setPinGroupPin)
			{
				setPin.insert(PinName);
			}
			continue;
		}
		if (setPin.end() != setPin.find(strPin))
		{
			continue;
		}
		setPin.insert(strPin);
	}
	if (0 == setPin.size() && 0 == nRetVal)
	{
		m_pAlarm->SetAlarmID(ALARM_ID::ALARM_PIN_NAME_STRING_FORMT_WRONG);
		m_pAlarm->SetAlarmMsg("No pin in lpszPinNameString.");
		return -2;
	}
	return nRetVal;
}

inline int CDCM::GetChannel(std::set<string>& setPinName, USHORT usSiteNo, BOOL bOnlyValidSite)
{
#ifdef RECORD_TIME
// 	CTimer::Instance()->Start("GetPinChannel");
// 	CTimer::Instance()->Start("GetSiteInfo");
#endif // RECORD_TIME

	set<USHORT> setSite;
	int nRetVal = GetSiteInfo(usSiteNo, setSite, bOnlyValidSite);
	if (0 != nRetVal)
	{
		if (-1 == nRetVal)
		{
			///<The site number is over range
			return -1;
		}
		else
		{
			///<The site is invalid
			return -2;
		}
	}

#ifdef RECORD_TIME
// 	CTimer::Instance()->Stop();
// 	CTimer::Instance()->Start("GetSiteChannel");
#endif // RECORD_TIME

	vector<CHANNEL_INFO> vecChannel;
	for (auto Site : setSite)
	{
		for (auto& PinName : setPinName)
		{
			auto iterPin = m_mapPin.find(PinName);
			if (m_mapPin.end() == iterPin || nullptr == iterPin->second)
			{
				continue;
			}
			CHANNEL_INFO Channel;
			iterPin->second->GetChannel(Site, Channel);
			
			if (m_mapBoard.end() != m_mapBoard.find(Channel.m_bySlotNo))
			{
				vecChannel.push_back(Channel);
			}
		}
	}
#ifdef RECORD_TIME
// 	CTimer::Instance()->Stop();
// 	CTimer::Instance()->Start("SetChannel");
#endif // RECORD_TIME

	m_ClassifyBoard.SetChannel(vecChannel);

// #ifdef RECORD_TIME
// 	CTimer::Instance()->Stop();
// 	CTimer::Instance()->Stop();
// #endif // RECORD_TIME

	return 0;
}

inline int CDCM::GetChannel(const char* lpszPinGroup, USHORT usSiteNo, BOOL bOnlyValidSite, std::set<std::string>* psetPin)
{
	set<string> setPinName;
	int nRetVal = GetPinName(lpszPinGroup, setPinName);
	if (0 != nRetVal)
	{
		switch (nRetVal)
		{
		case -1:
			///<Vector not loaded
			return -1;
		case -2:
			///<The pin group is nullptr
			return -2;
			break;
		case -3:
			///<The pin group is not defined
			m_pAlarm->SetAlarmID(ALARM_ID::ALARM_PIN_GROUP_ERROR);
			m_pAlarm->SetAlarmType(CDriverAlarm::ALARM_TYPE::PARAMETER_NOT_DEFINED);
			m_pAlarm->SetAlarmMsg("The pin or pin group(%s) is not defined.", lpszPinGroup);
			m_pAlarm->SetParamName("lpszPinGroup");
			return -3;
		case -4:
			///<The pin is not owned
			m_pAlarm->SetAlarmID(ALARM_ID::ALARM_PIN_UNOWNED);
			m_pAlarm->SetAlarmType(CDriverAlarm::ALARM_TYPE::PARAMETER_NOT_DEFINED);
			m_pAlarm->SetPinString(lpszPinGroup, TRUE);
			m_pAlarm->SetAlarmMsg("The pin(%s) is not belongs to current instance.", lpszPinGroup);
			return -6;
		default:
			break;
		}
	}
	if (nullptr != psetPin)
	{
		*psetPin = setPinName;
	}

	nRetVal = GetChannel(setPinName, usSiteNo, bOnlyValidSite);

	if (0 != nRetVal)
	{
		USHORT usSiteCount = m_Site.GetSiteCount();
		switch (nRetVal)
		{
		case -1:
			m_pAlarm->SiteOverScaleAlarm(lpszPinGroup, usSiteNo, usSiteCount - 1, FALSE);
			nRetVal = -4;
			break;
		case -2:
			//m_pAlarm->SiteInvalidAlarm(lpszPinGroup, usSiteNo, FALSE);
			nRetVal = -5;
			break;
		default:
			break;
		}
	}

	return nRetVal;
}

inline int CDCM::ClassifyChannel(const std::vector<CHANNEL_INFO>& vecChannel)
{
	m_ClassifyBoard.SetChannel(vecChannel);
	return 0;
}

int CDCM::GetFailLineNo(BYTE bySlotNo, USHORT usChannel, UINT uGetMaxFailCount, std::vector<int>& vecLineNo, int* pnLastCertainPassNo)
{
	vecLineNo.clear();
	int nRetVal = 0;
	auto iterBoard = m_mapBoard.find(bySlotNo);
	if (m_mapBoard.end() == iterBoard || nullptr == iterBoard->second)
	{
		return -1;
	}
	vector<int> vecBRAMLine;
	vector<int> vecDRAMLine;
	///<Check whether the fail line of controller has been handled, because only first 1023 lines in BRAM or DRAM each controller be saved
	///<only first 1023 fail lines in vector order should be reserved
	nRetVal = iterBoard->second->IsFailLineHandled(usChannel);
	if (0 > nRetVal)
	{
		///<The fail line number not be handled
		if (-3 != nRetVal)
		{
			switch (nRetVal)
			{
			case -1:
				///<Channel is over range
				nRetVal = -2;
				break;
			case -2:
				///<Channel is not existed
				nRetVal = -3;
				break;
			default:
				break;
			}
			return nRetVal;
		}
		///<The all fail lines in BRAM and DRAM
		int nBRAMDeleteCount = 0;
		int nDRAMDeleteCount = 0;
		nRetVal = iterBoard->second->GetMCUFailLineNo(usChannel, vecBRAMLine, vecDRAMLine);
		if (0 != nRetVal)
		{
			switch (nRetVal)
			{
			case -1:
				///<Channel is over range
				nRetVal = -2;
				break;
			case -2:
				///<Channel is not existed
				nRetVal = -3;
				break;
			case -3:
				///<Vector not ran
				nRetVal = -4;
				break;
			case -4:
				///<Vector running
				nRetVal = -5;
				break;
			default:
				break;
			}
			return nRetVal;
		}

		if (MAX_FAIL_LINE_COUNT_OPEN < vecBRAMLine.size() + vecDRAMLine.size())
		{
			///<The fail line count total saved is over maximum fail line count(0x03FF)

			///<Combine the fail line in vector line order
			vector<CVectorInfo::LINE_INFO> vecLineNo;
			m_VectorInfo.CombineLine(m_nLatestStartLine, m_nLatestStopLine, vecBRAMLine, vecDRAMLine, vecLineNo);
			vecBRAMLine.clear();
			vecDRAMLine.clear();

			///<Calculate the count of fail line will be deleted
			for (int nLineIndex = vecLineNo.size() - 1; nLineIndex >= MAX_FAIL_LINE_COUNT_OPEN; --nLineIndex)
			{
				if (vecLineNo[nLineIndex].m_bBRAM)
				{
					++nBRAMDeleteCount;
				}
				else
				{
					++nDRAMDeleteCount;
				}
			}
		}
		else
		{
			///<The fail line number is not over maximum fail line count open
			vecBRAMLine.clear();
			vecDRAMLine.clear();
		}
		iterBoard->second->DeleteControllerFailLine(usChannel, nBRAMDeleteCount, nDRAMDeleteCount);
	}

	nRetVal = iterBoard->second->GetFailLineNo(usChannel, uGetMaxFailCount, vecBRAMLine, vecDRAMLine);
	
	if (0 != nRetVal)
	{
		switch (nRetVal)
		{
		case -1:
			///<Channel is over range
			nRetVal = -2;
			break;
		case -2:
			///<Channel is not existed
			nRetVal = -3;
			break;
		case -3:
			///<Vector not ran
			nRetVal = -4;
			break;
		case -4:
			///<Vector running
			nRetVal = -5;
			break;
		default:
			break;
		}
		return nRetVal;
	}

	BOOL bFiletered = FALSE;
	int nFailLineCountGotten = vecBRAMLine.size() + vecDRAMLine.size();
	if (uGetMaxFailCount != nFailLineCountGotten)
	{
		///<The line count gotten is not equal to the line count expected, the fail line may be not all saved
		///<Noted that the fail count is not equal to channel's, this maybe cause by only the fail information of line selected be saved
		int nFailCount = iterBoard->second->GetControllerFailCount(usChannel);
		if (MAX_FAIL_LINE_COUNT_OPEN <= nFailCount)
		{
			///<The fail line count of current controller is full
			nFailCount = iterBoard->second->GetFailCount(usChannel);
			if (nFailCount > nFailLineCountGotten)
			{
				///<The fail line are not all saved
				///<The alarm will be outputed is the upper function without pin name or pin
				bFiletered = TRUE;
				m_pAlarm->SetAlarmID(ALARM_ID::ALARM_FAIL_LINE_NOT_SAVE);
				m_pAlarm->SetAlarmMsg("The fail line saving memory of channel(S%d_%d) is filled, some fail lines may not be saved.",
					bySlotNo, usChannel);
			}
		}
	}
	m_VectorInfo.CombineLine(m_nLatestStartLine, m_nLatestStopLine, vecBRAMLine, vecDRAMLine, vecLineNo);

	if (bFiletered && nullptr != pnLastCertainPassNo)
	{
		///<Check whehter the fail line number gotten of MCU is certain
		int nBRAMLineNo = 0;
		int nDRAMLineNoOffset = 0;
		BOOL bBRAMLineFail = FALSE;
		BOOL bDRAMLineFail = FALSE;
		iterBoard->second->GetLastCertainResultLineNo(usChannel, nBRAMLineNo, bBRAMLineFail, nDRAMLineNoOffset, bDRAMLineFail);
		if (-1 != nBRAMLineNo || -1 != nDRAMLineNoOffset)
		{
			int uStartLineNo = m_VectorInfo.GetGlobalLineNo(m_nLatestStartLine);
			int nBRAMOffset = DCM_BRAM_PATTERN_LINE_COUNT + DCM_DRAM_PATTERN_LINE_COUNT;
			if (-1 != nBRAMLineNo)
			{
				nBRAMOffset = m_VectorInfo.GetGlobalLineNo(nBRAMLineNo) - uStartLineNo;
			}

			int nDRAMOffset = DCM_BRAM_PATTERN_LINE_COUNT + DCM_DRAM_PATTERN_LINE_COUNT;
			if (-1 != nDRAMLineNoOffset)
			{
				nDRAMOffset = m_VectorInfo.GetDRAMOffsetGlobalLineNo(m_nLatestStartLine, m_nLatestStopLine, nDRAMLineNoOffset) - uStartLineNo;
			}
			BOOL bBRAMFailMemoryFilled = TRUE;
			BOOL bDRAMFailMemoryFilled = TRUE;
			iterBoard->second->GetFailMemoryFilled(usChannel, bBRAMFailMemoryFilled, bDRAMFailMemoryFilled);

			int nCertailLineNo = 0;
			BOOL bCurChannelFailInLastFailSaved = FALSE;

			if (!bBRAMFailMemoryFilled || !bDRAMFailMemoryFilled)
			{
				nCertailLineNo = bBRAMFailMemoryFilled ? nBRAMOffset : nDRAMOffset;
				bCurChannelFailInLastFailSaved = bBRAMFailMemoryFilled ? bBRAMLineFail : bDRAMLineFail;
			}
			else
			{
				nCertailLineNo = nBRAMOffset > nDRAMOffset ? nDRAMOffset : nBRAMOffset;
				bCurChannelFailInLastFailSaved = nBRAMOffset > nDRAMOffset ? bDRAMLineFail : bBRAMLineFail;
			}

			*pnLastCertainPassNo = -1;
			if (!bCurChannelFailInLastFailSaved)
			{
				*pnLastCertainPassNo = nCertailLineNo;
			}
		}
	}

	return bFiletered;
}

inline void CDCM::GetAllBoardChannel()
{
	set<string> setPin;
	auto iterPin = m_mapPin.begin();
	while (m_mapPin.end() != iterPin)
	{
		if (nullptr != iterPin->second)
		{
			setPin.insert(iterPin->first);
		}
		++iterPin;
	}
	GetChannel(setPin, ALL_SITE, FALSE);
}

inline int CDCM::GetSiteInfo(USHORT usSiteNo, std::set<USHORT>& setSite, BOOL bOnlyValidSite)
{
	setSite.clear();
	BOOL bHaveValidSite = FALSE;
	USHORT usStartSite = usSiteNo;
	USHORT usStopSite = usSiteNo;
	if (ALL_SITE == usStartSite)
	{
		usStartSite = 0;
		USHORT usSiteCount = m_Site.GetSiteCount();
		if (0 == usSiteCount)
		{
			return -3;
		}
		usStopSite = usSiteCount - 1;
	}
	else
	{
		USHORT usSiteCount = m_Site.GetSiteCount();
		if (m_Site.GetSiteCount() <= usSiteNo)
		{
			return -1;
		}
	}
	for (USHORT usCurSiteNo = usStartSite; usCurSiteNo <= usStopSite; ++usCurSiteNo)
	{
		if (bOnlyValidSite && !m_Site.IsSiteValid(usCurSiteNo))
		{
			continue;
		}
		bHaveValidSite = TRUE;
		setSite.insert(usCurSiteNo);
	}
	if (!bHaveValidSite)
	{
		return -2;
	}
	return 0;
}

int CDCM::GetTMUConnectUnit(BYTE bySlotNo, USHORT usChannel)
{
	auto iterBoard = m_mapBoard.find(bySlotNo);
	if (m_mapBoard.end() == iterBoard || nullptr == iterBoard->second)
	{
		///<The board is not existed
		return -1;
	}
	int nRetVal = iterBoard->second->GetTMUConnectUnit(usChannel);
	if (0 > nRetVal)
	{
		switch (nRetVal)
		{
		case -1:
			///<The channel number is over range
			nRetVal = -2;
			break;
		case -2:
			///<The channel is not existed
			nRetVal = -3;
			break;
		case -3:
			///<The channel is not connected to any unit
			nRetVal = -4;
			break;
		default:
			break;
		}
	}
	return nRetVal;
}

int CDCM::GetTMUParameter(BYTE bySlotNo, USHORT usChannel, BOOL& bRaiseTriggerEdge, USHORT& usHoldOffTime, USHORT& usHoldOffNum)
{
	auto iterBoard = m_mapBoard.find(bySlotNo);
	if (m_mapBoard.end() == iterBoard || nullptr == iterBoard->second)
	{
		///<The board is not existed
		return -1;
	}
	int nRetVal = iterBoard->second->GetTMUParameter(usChannel, bRaiseTriggerEdge, usHoldOffTime, usHoldOffNum);
	if (0 > nRetVal)
	{
		switch (nRetVal)
		{
		case -1:
			///<The channel is over range
			nRetVal = -2;
			break;
		case -2:
			///<The controller is not existed
			nRetVal = -3;
			break;
		case -3:
			///<The channel is not connected to any TMU unit
			nRetVal = -4;
			break;
		default:
			break;
		}
	}
	return nRetVal;
}

int CDCM::GetTMUUnitParameter(BYTE bySlotNo, BYTE byControllerIndex, BYTE byTMUUnitIndex, BOOL& bRaiseTriggerEdge, USHORT& usHoldOffTime, USHORT& usHoldOffNum)
{
	auto iterBoard = m_mapBoard.find(bySlotNo);
	if (m_mapBoard.end() == iterBoard || nullptr == iterBoard->second)
	{
		///<The board is not existed
		return -1;
	}
	int nRetVal = iterBoard->second->GetTMUUnitParameter(byControllerIndex, byTMUUnitIndex, bRaiseTriggerEdge, usHoldOffTime, usHoldOffNum);
	if (0 > nRetVal)
	{
		switch (nRetVal)
		{
		case -1:
			///<The controller index is over range
			nRetVal = -2;
			break;
		case -2:
			///<The controller is not existed
			nRetVal = -3;
			break;
		case -3:
			///<The TMU unit index is over range
			nRetVal = -4;
			break;
		default:
			break;
		}
	}
	return nRetVal;
}

int CDCM::SetTMUUnitChannel(const char* lpszPinGroup, USHORT usSiteNo, BYTE byUnitIndex)
{
	if (!m_bLoadVector)
	{
		m_pAlarm->VectorNotLoadedAlarm();
		return -1;
	}
	int nRetVal = GetChannel(lpszPinGroup, usSiteNo);
	if (0 != nRetVal)
	{
		switch (nRetVal)
		{
		case -2:
			//The pin group is nullptr
			nRetVal = -2;
			break;
		case -3:
			//The pin group is not defined before
			nRetVal = -3;
			break;
		case -4:
			///<The site is over range
			nRetVal = -4;
			break;
		case -5:
			///<The site is invalid
			nRetVal = -5;
			break;
		case -6:
			///<The pin is not belongs to
			nRetVal = -6;
			break;
		default:
			///<Not will happen
			break;
		}
		return nRetVal;
	}
	vector<USHORT> vecChannel;
	BOOL bNoBoard = TRUE;

	BOARD_CHANNEL_BEGIN(vecChannel)
	{
		bNoBoard = FALSE;
		nRetVal = iterBoard->second->SetTMUUnitChannel(vecChannel, byUnitIndex);
		if (0 != nRetVal)
		{
			BOOL bExit = TRUE;
			BOOL bAddAlarm = !m_pAlarm->IsSetMsg();
			m_pAlarm->SetPinString(lpszPinGroup, FALSE);
			if (ALL_SITE != usSiteNo)
			{
				m_pAlarm->SetSite(usSiteNo);
			}
			switch (nRetVal)
			{
			case -1:
				///<More than one channel bind to one TMU unit
				if (bAddAlarm)
				{
					m_pAlarm->SetAlarmID(ALARM_ID::ALARM_TMU_UNIT_CONNECT_CHANNEL_OVER_RANGE);
					m_pAlarm->SetAlarmMsg("Only one channel can be connect to TMU unit %d.", byUnitIndex);
				}
				nRetVal = -7;
				break;
			case -2:
				///<The unit is over range
				m_pAlarm->SetParamName("TMUUnit");
				if (bAddAlarm)
				{
					m_pAlarm->SetAlarmID(ALARM_ID::ALARM_TMU_UNIT_INDEX_OVER_RANGE);
					m_pAlarm->SetAlarmMsg("The TMU unit(%d) is over range.", byUnitIndex);
				}
				nRetVal = -8;
				break;
			case -3:
				///<No valid channel in the board
				m_pAlarm->SetAlarmID(ALARM_ID::ALARM_CHANNEL_NOT_EXISTED);
				m_pAlarm->SetAlarmMsg("The channels of board(S%d) are not existed", iterBoard->first);
				m_pAlarm->SetAlarmLevel(CDriverAlarm::ALARM_LEVEL::AlarmWarning);
				m_pAlarm->Output(FALSE);
				nRetVal = 0;
				bExit = FALSE;
				break;
			default:
				break;
			}
			if (bExit)
			{
				break;
			}
		}
	}
	BOARD_CHANNEL_END

	if (bNoBoard)
	{
		m_pAlarm->SetNoBoardAlarm(lpszPinGroup, FALSE);
		nRetVal = -9;
	}
	return nRetVal;
}

int CDCM::SetTMUParam(const char* lpszPinGroup, USHORT usSiteNo, BOOL bRaiseTriggerEdge, UINT uHoldOffTime, UINT uHolfOffNum, BYTE bySpecifiedUnit)
{
	if (!m_bLoadVector)
	{
		m_pAlarm->VectorNotLoadedAlarm();
		return -1;
	}
#ifdef RECORD_TIME
	CTimer::Instance()->Start("CDCM::SetTMUParam");
	CTimer::Instance()->Start("GetChannel");
#endif // RECORD_TIME

	int nRetVal = GetChannel(lpszPinGroup, usSiteNo);
	if (0 != nRetVal)
	{
		switch (nRetVal)
		{
		case -2:
			//The pin group is nullptr
			nRetVal = -2;
			break;
		case -3:
			//The pin group is not defined before
			nRetVal = -3;
			break;
		case -4:
			///<The site is over range
			nRetVal = -4;
			break;
		case -5:
			///<The site is invalid
			nRetVal = -5;
			break;
		case -6:
			///<The pin is not belongs to
			nRetVal = -6;
			break;
		default:
			///<Not will happen
			break;
		}
		return nRetVal;
	}
	vector<USHORT> vecChannel;
	BOOL bNoBoard = TRUE;

#ifdef RECORD_TIME
	CTimer::Instance()->Stop();
	CTimer::Instance()->Start("Set Parameter");
#endif // RECORD_TIME

	BOARD_CHANNEL_BEGIN(vecChannel)
	{
#ifdef RECORD_TIME
		CTimer::Instance()->Start("Slot %d", iterBoard->first);
#endif // RECORD_TIME
		nRetVal = iterBoard->second->SetTMUParam(vecChannel, bRaiseTriggerEdge, uHoldOffTime, uHolfOffNum, bySpecifiedUnit);
		if (0 != nRetVal)
		{
			BOOL bExit = TRUE;
			m_pAlarm->SetPinString(lpszPinGroup, FALSE);
			m_pAlarm->SetSite(usSiteNo);
			BOOL bAddAlarm = !m_pAlarm->IsSetMsg();
			switch (nRetVal)
			{
			case -1:
				///<The unit specified is ove range
				nRetVal = -7;
				break;
			case -2:
				///<The channels count is over range
				if (bAddAlarm)
				{
					m_pAlarm->SetAlarmID(ALARM_ID::ALARM_TMU_UNIT_CONNECT_CHANNEL_OVER_RANGE);
					m_pAlarm->SetAlarmMsg("The channel count in the controller is over range.");
				}
				nRetVal = -8;
				break;
			case -3:
				///<Channel is not connected to the unit
				if (bAddAlarm)
				{
					m_pAlarm->SetAlarmID(ALARM_ID::ALARM_CHANNEL_NOT_CONNECT_TMU);
					m_pAlarm->SetAlarmMsg("Some channel in pin group(%s) is not connected to TMU unit.", lpszPinGroup);
				}
				nRetVal = -9;
				break;
			case -4:
				///<No board existed
				bExit = FALSE;
				break;
			default:
				break;
			}
			if (bExit)
			{
				bNoBoard = FALSE;
				break;
			}
		}
		else
		{
			bNoBoard = FALSE;
		}

#ifdef RECORD_TIME
		CTimer::Instance()->Stop();
#endif // RECORD_TIME
	}
	BOARD_CHANNEL_END

	if (bNoBoard)
	{
		m_pAlarm->SetNoBoardAlarm(lpszPinGroup, FALSE, usSiteNo);
		nRetVal = -10;
	}

#ifdef RECORD_TIME
	CTimer::Instance()->Stop();
	CTimer::Instance()->Stop();
	CTimer::Instance()->Print("D:\\DRAM_Multi.txt");
#endif // RECORD_TIME
	return nRetVal;
}

int CDCM::TMUMeasure(const char* lpszPinGroup, TMU_MEAS_MODE MeasMode, UINT uSampleNum, double dTimeout)
{
	if (!m_bLoadVector)
	{
		m_pAlarm->VectorNotLoadedAlarm();
		return -1;
	}
	int nRetVal = GetChannel(lpszPinGroup);
	if (0 != nRetVal)
	{
		switch (nRetVal)
		{
		case -2:
			//The pin group is nullptr
			nRetVal = -2;
			break;
		case -3:
			//The pin group is not defined before
			nRetVal = -3;
			break;
		case -5:
			///<No valid site
			nRetVal = -4;
		case -6:
			///<The pin is not belongs to
			nRetVal = -5;
			break;
		default:
			///<Not will happen
			break;
		}
		return nRetVal;
	}
	vector<USHORT> vecChannel;
	BOOL bNoBoard = TRUE;


	m_pAlarm->SetPinString(lpszPinGroup, FALSE);

	BOARD_CHANNEL_BEGIN(vecChannel)
	{
		nRetVal = iterBoard->second->TMUMeasure(vecChannel, MeasMode, uSampleNum, dTimeout);
		if (0 != nRetVal)
		{
			BOOL bExit = TRUE;
			BOOL bAddAlarm = !m_pAlarm->IsSetMsg();
			switch (nRetVal)
			{
			case -1:
				///<Channel is not connected to the unit
				if (bAddAlarm)
				{
					m_pAlarm->SetAlarmID(ALARM_ID::ALARM_CHANNEL_NOT_CONNECT_TMU);
					m_pAlarm->SetAlarmMsg("Some channel in pin group(%s) is not connected to TMU unit.", lpszPinGroup);
				}
				nRetVal = -6;
				break;
			case -2:
				///<The measurement mode is not supported
				if (bAddAlarm)
				{
					m_pAlarm->SetAlarmID(ALARM_ID::ALARM_TMU_MEAS_MODE_ERROR);
					m_pAlarm->SetAlarmMsg("The measurement mode(%d) is not supported", (BYTE)MeasMode);
				}
				nRetVal = -7;
				break;
			case -3:
				///<No board existed
				bExit = FALSE;
				break;
			default:
				break;
			}
			if (bExit)
			{
				bNoBoard = FALSE;
				break;
			}
		}
		else
		{
			bNoBoard = FALSE;
		}
	}
	BOARD_CHANNEL_END

	if (bNoBoard)
	{
		m_pAlarm->SetNoBoardAlarm(lpszPinGroup, FALSE);
		nRetVal = -8;
	}
	return nRetVal;
}

int CDCM::GetTMUMeasure(BYTE bySlotNo, USHORT usChannel, TMU_MEAS_MODE& MeasMode, UINT& uSampleNum, double& dTimeout)
{
	auto iterBoard = m_mapBoard.find(bySlotNo);
	if (m_mapBoard.end() == iterBoard || nullptr == iterBoard->second)
	{
		return -1;
	}
	int nRetVal = iterBoard->second->GetTMUMeasure(usChannel, MeasMode, uSampleNum, dTimeout);
	if (0 != nRetVal)
	{
		switch (nRetVal)
		{
		case -1:
			///<The channel is over range
			nRetVal = -2;
			break;
		case -2:
			///<The channel is not existed
			nRetVal = -3;
			break;
		case -3:
			///<The channel is not connected to any TMU unit
			nRetVal = -4;
			break;
		default:
			break;
		}
	}
	return nRetVal;
}

double CDCM::GetTMUMeasureResult(const char* lpszPinName, USHORT usSiteNo, TMU_MEAS_TYPE MeasType, int& nErrorCode)
{
	int nRetVal = 0;
	CHANNEL_INFO Channel;
	nRetVal = GetBoardChannel(lpszPinName, usSiteNo, Channel);
	if (0 != nRetVal)
	{
		nErrorCode = nRetVal;
		return TMU_ERROR;
	}

	auto iterBoard = m_mapBoard.find(Channel.m_bySlotNo);
	if (m_mapBoard.end() == iterBoard || nullptr == iterBoard->second)
	{
		m_pAlarm->SetNoBoardAlarm(lpszPinName, TRUE, usSiteNo);
		nErrorCode = -7;
		return TMU_ERROR;
	}

	double dResult = iterBoard->second->GetTMUMeasureResult(Channel.m_usChannel, MeasType, nErrorCode);
	if (0 != nErrorCode)
	{
		m_pAlarm->SetPinString(lpszPinName, TRUE);
		m_pAlarm->SetSite(usSiteNo);
		BOOL bAddAlarm = !m_pAlarm->IsSetMsg();
		switch (nErrorCode)
		{
		case -1:
			///<The channel is over range, not will happen
			break;
		case -2:
			///<The channel is not existed
			nErrorCode = -8;
			m_pAlarm->SetAlarmID(ALARM_ID::ALARM_CHANNEL_NOT_EXISTED);
			m_pAlarm->SetAlarmMsg("The channel(S%d_%d) of pin name(%s) in SITE_%d is not existed.", Channel.m_bySlotNo, Channel.m_usChannel, lpszPinName, usSiteNo + 1);
			m_pAlarm->SetAlarmLevel(CDriverAlarm::ALARM_LEVEL::AlarmWarning);
			m_pAlarm->Output(FALSE);
			break;
		case -3:
			///<The channel is not connect to any TMU unit
			nErrorCode = -9;
			m_pAlarm->SetAlarmMsg("The channel(S%d_%d) of pin name(%s) in SITE_%d is not connected to TMU unit.", Channel.m_bySlotNo, Channel.m_usChannel, lpszPinName, usSiteNo + 1);
			m_pAlarm->SetAlarmID(ALARM_ID::ALARM_CHANNEL_NOT_CONNECT_TMU);
			break;
		case -4:
			///<The measurement type is not supported
			if (bAddAlarm)
			{
				m_pAlarm->SetAlarmID(ALARM_ID::ALARM_TMU_MEASURE_RESULT_ERROR);
				m_pAlarm->SetAlarmMsg("The type of measurement result(%d) is not supported.", (BYTE)MeasType);
			}
			nErrorCode = -10;
			break;
		case -5:
			///<The measurement type is not measured before
			if (bAddAlarm)
			{
				m_pAlarm->SetParamName("TMUMeasType");
				m_pAlarm->SetAlarmID(ALARM_ID::ALARM_TMU_NOT_MEASURE_ERROR);
				m_pAlarm->SetAlarmMsg("The type of measurement result(%d) is not measured before.", (BYTE)MeasType);
			}
			nErrorCode = -11;
			break;
		case -6:
			///<The measurement is not stop in timeout
// 			if (bAddAlarm)
// 			{
// 				m_pAlarm->SetAlarmID(ALARM_ID::ALARM_TMU_NOT_STOP);
// 				m_pAlarm->SetAlarmMsg("The TMU unit is not stop.");
// 			}
			nErrorCode = -12;
			break;
		case -7:
			///<The TMU measurement is timeout
// 			if (bAddAlarm)
// 			{
// 				m_pAlarm->SetAlarmID(ALARM_ID::ALARM_TMU_TIMEOUT);
// 				m_pAlarm->SetAlarmMsg("The TMU unit is timeout.");
// 			}
			nErrorCode = -13;
			break;
		case -8:
			///<The bind unit of measurement is not stop in timeout
// 			if (bAddAlarm)
// 			{
// 				m_pAlarm->SetAlarmID(ALARM_ID::ALARM_TMU_NOT_STOP);
// 				m_pAlarm->SetAlarmMsg("The TMU unit is not stop.");
// 			}
			nErrorCode = -14;
			break;
		case -9:
			///<The bind unit is timeout
// 			if (bAddAlarm)
// 			{
// 				m_pAlarm->SetAlarmID(ALARM_ID::ALARM_TMU_TIMEOUT);
// 				m_pAlarm->SetAlarmMsg("The TMU unit is timeout.");
// 			}
			nErrorCode = -15;
			break;
		case -10:
			///<The edge measurement error
			nErrorCode = -16;
			break;
		default:
			break;
		}
	}

	return dResult;
}

int CDCM::SetTriggerOut(const char* lpszPinName, USHORT usSiteNo)
{
	CHANNEL_INFO Channel;
	int nRetVal = GetBoardChannel(lpszPinName, usSiteNo, Channel);
	if (0 != nRetVal)
	{
		return nRetVal;
	}
	auto iterBoard = m_mapBoard.find(Channel.m_bySlotNo);
	if (m_mapBoard.end() == iterBoard || nullptr == iterBoard->second || DCM_MAX_CHANNELS_PER_BOARD <= Channel.m_usChannel)
	{
		m_pAlarm->SetNoBoardAlarm(lpszPinName, TRUE, usSiteNo);
		return -7;
	}

	nRetVal = iterBoard->second->SetTriggerOut(Channel.m_usChannel);
	if (0 != nRetVal)
	{
		m_pAlarm->SetSite(usSiteNo);
		m_pAlarm->SetPinString(lpszPinName, TRUE);
		m_pAlarm->SetParamName("lpszPinName");
		switch (nRetVal)
		{
		case -1:
		case -2:
			m_pAlarm->SetAlarmID(ALARM_ID::ALARM_CHANNEL_NOT_EXISTED);
			m_pAlarm->SetAlarmMsg("The channel(S%d_%d) of pin(%s) in SITE_%d is not existed.", Channel.m_bySlotNo, Channel.m_usChannel, lpszPinName, usSiteNo + 1);
			m_pAlarm->SetAlarmLevel(CDriverAlarm::ALARM_LEVEL::AlarmWarning);
			m_pAlarm->Output(FALSE);
			nRetVal = -8;
			break;
		default:
			break;
		}
	}
	return nRetVal;
}

int CDCM::UpdateChannelMode(BYTE bySlotNo)
{
	auto iterSlot = m_mapBoard.find(bySlotNo);
	if (m_mapBoard.end() == iterSlot || nullptr == iterSlot->second)
	{
		return -1;
	}
	iterSlot->second->UpdateChannelMode();
	return 0;
}

int CDCM::EnableSaveSelectedFail(const char* lpszPinGroup, BOOL bEnable)
{
	if (!m_bLoadVector)
	{
		return -1;
	}
	int nRetVal = GetChannel(lpszPinGroup);
	if (0 != nRetVal)
	{
		switch (nRetVal)
		{
		case -1:
			///<No load vector
			nRetVal = -1;
			break;
		case -2:
			///<The pin group is nullptr
			nRetVal = -2;
			break;
		case -3:
			///<Pin group is not defined
			nRetVal = -3;
			break;
		case -4:
			///<The site is ove range, not will happen
			break;
		case -5:
			///<The site are all invalid
			nRetVal = -4;
			break;
		default:
			break;
		}
		return nRetVal;
	}

	vector<USHORT> vecChannel;
	BOOL bNoBoard = TRUE;
	BOARD_CHANNEL_BEGIN(vecChannel)
	{
		bNoBoard = FALSE;
		nRetVal = iterBoard->second->EnableSaveSelectedFail(vecChannel, bEnable);
		if (0 != nRetVal)
		{
			bNoBoard = TRUE;
		}
	}
	BOARD_CHANNEL_END

	if (bNoBoard)
	{
		return -5;
	}
	return 0;
}

BOOL CDCM::IsLoadVector(std::string& strVectorFile)
{
	strVectorFile.clear();
	if (!m_bLoadVector)
	{
		return FALSE;
	}
	strVectorFile = m_strFileName;
	return TRUE;
}

void CDCM::GetVectorMemoryLeft(UINT& uBRAMStart, UINT& uDRAMStart)
{
	uBRAMStart = m_nBRAMLeftStartLine;
	uDRAMStart = m_nDRAMLeftStartLine;
}

int CDCM::GetPinGroup(std::vector<std::string>& vecPinGroup)
{
	vecPinGroup.clear();
	if (!m_bLoadVector)
	{
		return -1;
	}
	for (auto& PinGroup : m_mapPinGroup)
	{
		vecPinGroup.push_back(PinGroup.first);
	}
	return 0;
}

BOOL CDCM::IsPinExisted(const char* lpszPin)
{
	if (nullptr == lpszPin)
	{
		return FALSE;
	}
	auto iterPin = m_mapPin.find(lpszPin);
	if (m_mapPin.end() != iterPin)
	{
		return TRUE;
	}
	return FALSE;
}

int CDCM::DeleteUnusedBoard()
{
	if (!m_bLoadVector)
	{
		return -1;
	}
	if (m_bDeleteChannelUnused)
	{
		return 0;
	}
	vector<CHANNEL_INFO> vecChannel;
	for (auto& Pin : m_mapPin)
	{
		Pin.second->GetAllChannel(vecChannel, TRUE);
	}
	m_ClassifyBoard.SetChannel(vecChannel);
	vector<USHORT> vecBoardChannel;
	set<BYTE> setUsedBoard;
	BOARD_CHANNEL_BEGIN(vecBoardChannel)
	{
		iterBoard->second->DeleteController(vecBoardChannel);
		setUsedBoard.insert(iterBoard->first);
	}
	BOARD_CHANNEL_END

	vector<BYTE> vecDeleteBoard;
	for (auto& Slot : m_mapBoard)
	{
		if (setUsedBoard.end() == setUsedBoard.find(Slot.first))
		{
			vecDeleteBoard.push_back(Slot.first);
		}
	}
	auto iterBoard = m_mapBoard.begin();
	for (auto Slot : vecDeleteBoard)
	{
		iterBoard = m_mapBoard.find(Slot);
		if (m_mapBoard.end() != iterBoard)
		{
			if (nullptr != iterBoard->second)
			{
				delete iterBoard->second;
				iterBoard->second = nullptr;
			}
			m_mapBoard.erase(iterBoard);
		}
	}

	m_bDeleteChannelUnused = TRUE;
	return 0;
}

int CDCM::SetValidPin(const char* lpszPinName)
{
	if (!m_bLoadVector)
	{
		m_pAlarm->VectorNotLoadedAlarm();
		return -1;
	}
	m_setPinUnowned.clear();
	set<string> setPin;
	int nRetVal = ExtractPinName(lpszPinName, setPin);
	if (0 != nRetVal)
	{
		m_pAlarm->SetParamName("lpszPinNameList");
		switch (nRetVal)
		{
		case -1:
			///<The point is nullptr, checked before
			m_pAlarm->ParameternullptrAlarm("lpszPinNameList", -1, nullptr);
			return -2;
			break;
		case -2:
			///<No pin in pin name string
			nRetVal = -3;
			break;
		case -3:
			///<The format is wrong
			nRetVal = -4;
			break;
		case -4:
			///<Some pin is not defined in vector
			nRetVal = -5;
			break;
		}
		return nRetVal;
	}

	for (auto& PinName : m_mapPin)
	{
		if (setPin.end() != setPin.find(PinName.first))
		{
			continue;
		}
		m_setPinUnowned.insert(PinName.first);
	}
	for (auto& PinName : m_setPinUnowned)
	{
		auto iterPin = m_mapPin.find(PinName);
		if (m_mapPin.end() != iterPin)
		{
			if (nullptr != iterPin->second)
			{
				delete iterPin->second;
				iterPin->second = nullptr;
			}
		}
		m_mapPin.erase(iterPin);
		m_Site.DeletePin(PinName);
	}
	return 0;
}

int CDCM::EnableAddPin(BOOL bEnable, BOOL bClearVector)
{
	m_bAllowAddPin = bEnable;
	if (!m_bAllowAddPin)
	{
		return 0;
	}
	if (bClearVector)
	{
		m_VectorInfo.Reset();
	}
	DeleteMemory(m_mapPin);
	DeleteMemory(m_mapPinGroup);
	m_Site.Reset();
	m_bLoadVector = TRUE;
	return 0;
}

int CDCM::AddPin(const char* lpszPinName, USHORT usPinNo, const char* lpszChannel)
{
	if (!m_bAllowAddPin)
	{
		return -1;
	}
	if (nullptr == lpszPinName || nullptr == lpszChannel)
	{
		return -2;
	}
	string strPinName = lpszPinName;
	string strChannel = lpszChannel;
	strPinName.erase(strPinName.find_last_not_of(" ") + 1);
	strChannel.erase(strChannel.find_last_not_of(" ") + 1);
	if (0 == strPinName.size() || 0 == strChannel.size())
	{
		return -3;
	}
	if (m_mapPin.end() != m_mapPin.find(strPinName))
	{
		return -4;
	}
	int nPos = 0;
	BOOL bFormatError = FALSE;
	string strCurChannel;
	vector<CHANNEL_INFO> vecChannel;
	while (-1 != nPos)
	{
		nPos = strChannel.find(",");
		if (-1 == nPos)
		{
			if (1 > strChannel.size())
			{
				continue;
			}
			strCurChannel = strChannel;
			strChannel.clear();
		}
		else
		{
			strCurChannel = strChannel.substr(0, nPos);
			strChannel.erase(0, nPos + 1);
		}
		nPos = strCurChannel.find("S");
		if (-1 == nPos)
		{
			bFormatError = TRUE;
			break;
		}
		strCurChannel.erase(0, nPos + 1);
		nPos = strCurChannel.find("_");
		if (-1 == nPos)
		{
			bFormatError = TRUE;
			break;
		}
		int nSlotNo = atoi(strCurChannel.substr(0, nPos).c_str());
		if (0 >= nSlotNo)
		{
			bFormatError = TRUE;
			break;
		}
		strCurChannel.erase(0, nPos + 1);
		int nChannel = atoi(strCurChannel.c_str());
		if (0 == nChannel && '0' != strCurChannel[0])
		{
			bFormatError = TRUE;
			break;
		}
		if (DCM_MAX_CHANNELS_PER_BOARD <= nChannel || 0 > nChannel)
		{
			bFormatError = TRUE;
			break;
		}
		vecChannel.push_back(CHANNEL_INFO(nSlotNo, nChannel));
	}
	if (bFormatError)
	{
		return -4;
	}
	CPin* pPin = new CPin(strPinName.c_str(), usPinNo);
	USHORT usSiteNo = 0;
	for (auto& Channel : vecChannel)
	{
		m_Site.AddChannel(strPinName, usSiteNo++, Channel);
		pPin->AddChannel(Channel);
	}
	m_mapPin.insert(make_pair(strPinName, pPin));
	return 0;
}

void CDCM::ClearPreread()
{
	for (auto& Board : m_mapBoard)
	{
		if (nullptr == Board.second)
		{
			continue;
		}
		Board.second->ClearPreread();
	}
}

inline int CDCM::InitSite(BOOL bAddAlarm)
{	
	//Create site class.
	auto iterPin = m_mapPin.begin();
	USHORT uSiteCount = iterPin->second->GetSiteCount();
	for (USHORT usSiteNo = 0; usSiteNo < uSiteCount; ++usSiteNo)
	{
		CHANNEL_INFO ChannelInfo;
		for (auto& Pin : m_mapPin)
		{
			Pin.second->GetChannel(usSiteNo, ChannelInfo);
			if (ALL_SITE == ChannelInfo.m_usChannel)
			{
				if (bAddAlarm)
				{
					m_pAlarm->SetAlarmID(ALARM_ID::ALARM_CHANNEL_OVER_RANGE);
					m_pAlarm->SetAlarmType(CDriverAlarm::ALARM_TYPE::PARAMETER_OVERRANGE);
					m_pAlarm->SetAlarmMsg("The channel(S%d_%d) of pin(%s) in SITE_%d is over range.",
						ChannelInfo.m_bySlotNo, ChannelInfo.m_usChannel, Pin.first.c_str(), usSiteNo + 1);
					m_pAlarm->SetAlarmLevel(CDriverAlarm::ALARM_LEVEL::AlarmError);
				}
				return -1;
				break;
			}
			m_Site.AddChannel(Pin.first, usSiteNo, ChannelInfo);
		}
	}
	return 0;
}

inline int CDCM::SavePinGroupInfo(const char* lpszPinGroupName)
{
// #ifdef RECORD_TIME
// 	CTimer::Instance()->Start("SavePinGroupInfo_%s", lpszPinGroupName);
// 	CTimer::Instance()->Start("GetPinGroup");
// #endif // RECORD_TIME

	if (nullptr == lpszPinGroupName)
	{
		return -1;
	}
	auto iterPinGroup = m_mapPinGroup.find(lpszPinGroupName);
	if (m_mapPinGroup.end() == iterPinGroup)
	{
		return -2;
	}

// #ifdef RECORD_TIME
// 	CTimer::Instance()->Stop();
// 	CTimer::Instance()->Start("GetFileName");
// #endif // RECORD_TIME

	string strFile;
	GetPinGroupInfoFile(strFile);
	CConfigFile ConfigFile(strFile.c_str());
	string strSection;
	GetPinGroupSection(strSection);
	int nPinGroupCount = 0;
	const char* lpszValue = ConfigFile.GetValue(strSection.c_str(), "Count");
	if (nullptr == lpszValue)
	{
		nPinGroupCount = 1;
	}
	else
	{
		nPinGroupCount = atoi(lpszValue) + 1;
	}

	ConfigFile.SetValue(strSection.c_str(), "Count", "%d", nPinGroupCount);

// #ifdef RECORD_TIME
// 	CTimer::Instance()->Stop();
// 	CTimer::Instance()->Start("GetPinName");
// #endif // RECORD_TIME

	string strPinGroup = lpszPinGroupName;
	strPinGroup += ":";
	set<string> setPinName;
	iterPinGroup->second->GetPinName(setPinName);
// #ifdef RECORD_TIME
// 	CTimer::Instance()->Stop();
// 	CTimer::Instance()->Start("SavePinName");
// 
// #endif // RECORD_TIME
	for (auto& PinName : setPinName)
	{
		strPinGroup += PinName + " ";
	}

	char lpszData[4] = { 0 };
	_itoa_s(nPinGroupCount - 1, lpszData, sizeof(lpszData), 10);

	ConfigFile.SetValue(strSection.c_str(), lpszData, strPinGroup.c_str());
	
	ConfigFile.Save();

// #ifdef RECORD_TIME
// 	CTimer::Instance()->Stop();
// 	CTimer::Instance()->Stop();
// 	CTimer::Instance()->Print("D:\\SavePinGroupInfo.csv");
// #endif // RECORD_TIME

	return 0;
}

inline void CDCM::LoadPinGroupInfo()
{
	string strFile;
	GetPinGroupInfoFile(strFile);
	const char* lpszConfig = strFile.c_str();
	char lpszAPP[32] = "PinGroup";
	if (m_bVectorShared)
	{
		strcpy_s(lpszAPP, sizeof(lpszAPP), "PinGroup_Shared");
	}

	CConfigFile ConfigFile(strFile.c_str());
	const char* lpszValue = ConfigFile.GetValue(lpszAPP, "Count");
	if (nullptr == lpszValue)
	{
		return;
	}

	int nPinGroupCount =atoi(lpszValue);
	char lpszKey[32] = { 0 };
	string strData;
	string strPinGroupName;
	set<string> setPinName;
	string strPinName;
	for (int nIndex = 0; nIndex < nPinGroupCount; ++nIndex)
	{
		_itoa_s(nIndex, lpszKey, sizeof(lpszKey), 10);
		lpszValue = ConfigFile.GetValue(lpszAPP, lpszKey);
		if (nullptr == lpszValue)
		{
			continue;
		}
		strData = lpszValue;
		int nPos = strData.find(":");
		if (-1 == nPos)
		{
			continue;
		}
		strPinGroupName = strData.substr(0, nPos);
		if (m_mapPinGroup.end() != m_mapPinGroup.find(strPinGroupName))
		{
			continue;
		}
		strData.erase(0, nPos + 1);
		setPinName.clear();
		while (-1 != nPos)
		{
			nPos = strData.find(' ');
			if (-1 == nPos)
			{
				if (0 == strData.size())
				{
					break;
				}
				strPinName = strData;
				strData.clear();
			}
			else
			{
				strPinName = strData.substr(0, nPos);
				strData.erase(0, nPos + 1);
			}
			if (m_mapPin.end() == m_mapPin.find(strPinName))
			{
				setPinName.clear();
				break;
			}
			setPinName.insert(strPinName);
		}
		if (0 == setPinName.size())
		{
			continue;
		}
		CPinGroup* pPinGroup = new CPinGroup(strPinGroupName.c_str());
		pPinGroup->SetPinName(setPinName);
		m_mapPinGroup.insert(make_pair(strPinGroupName, pPinGroup));
	}
}

inline int CDCM::GetPinName(const char* lpszPinGroup, std::set<std::string>& setPinName)
{
	setPinName.clear();
	if (!m_bLoadVector)
	{
		m_pAlarm->VectorNotLoadedAlarm();
		return -1;
	}
	if (nullptr == lpszPinGroup)
	{
		m_pAlarm->ParameternullptrAlarm("lpszPinGroup", ALL_SITE, nullptr);
		return -2;
	}
	auto iterPinGroup = m_mapPinGroup.find(lpszPinGroup);
	if (m_mapPinGroup.end() != iterPinGroup && nullptr != iterPinGroup->second)
	{
		iterPinGroup->second->GetPinName(setPinName);
	}
	else
	{
		auto iterPin = m_mapPin.find(lpszPinGroup);
		if (m_mapPin.end() == iterPin || nullptr == iterPin->second)
		{
			if (m_setPinUnowned.end() != m_setPinUnowned.find(lpszPinGroup))
			{
				return -4;
			}
			return -3;
		}
		setPinName.insert(lpszPinGroup);
	}
	return 0;
}

inline int CDCM::GetChannelPin(const CHANNEL_INFO& Channel, std::string& strPinName)
{
	int nRetVal = 0;
	for (auto& Pin : m_mapPin)
	{
		nRetVal = Pin.second->GetSiteNo(Channel);
		if (0 <= nRetVal)
		{
			strPinName = Pin.first;
			return nRetVal;
		}
	}
	return -1;
}

inline void CDCM::GetShieldChannel(const char* lpszFunction, std::map<BYTE, std::set<USHORT>>& mapShieldChannel)
{
	vector<CHANNEL_INFO> vecShieldChannel;
	m_pAlarm->GetShieldChannel(lpszFunction, vecShieldChannel);
	USHORT usShieldChannelCount = vecShieldChannel.size();
	for (auto& Channel : vecShieldChannel)
	{		
		auto iterShieldChannel = mapShieldChannel.find(Channel.m_bySlotNo);
		if (mapShieldChannel.end() == iterShieldChannel)
		{
			set<USHORT> setChannel;
			mapShieldChannel.insert(make_pair(Channel.m_bySlotNo, setChannel));
			iterShieldChannel = mapShieldChannel.find(Channel.m_bySlotNo);
		}
		iterShieldChannel->second.insert(Channel.m_usChannel);
	}
}

inline void CDCM::CheckPMUClampStatus(BYTE bySlotNo, const std::vector<USHORT>& vecChannel, const std::set<USHORT>* psetShieldChannel)
{
	if (!m_pAlarm->IsAlarmOpen())
	{
		///<Alarm is close
		return;
	}
	USHORT usChannelCount = vecChannel.size();
	if (0 >= usChannelCount)
	{
		return;
	}

	auto iterBoard = m_mapBoard.find(bySlotNo);
	if (m_mapBoard.end() == iterBoard || nullptr == iterBoard->second)
	{
		return;
	}

	map<USHORT, UCHAR> mapClampChannel;
	iterBoard->second->GetPMUClampChannel(vecChannel, mapClampChannel);

	if (0 == mapClampChannel.size())
	{
		return;
	}

	for (auto& ClampChannel : mapClampChannel)
	{
		if (nullptr != psetShieldChannel && psetShieldChannel->end() != psetShieldChannel->find(ClampChannel.first))
		{
			continue;
		}
		string strPinName;
		USHORT usSiteCount = 0;
		CHANNEL_INFO Channel;
		Channel.m_bySlotNo = bySlotNo;
		Channel.m_usChannel = ClampChannel.first;
		int nRetVal = GetChannelPin(Channel, strPinName);
		if (0 > nRetVal)
		{
			continue;
		}
		USHORT usSiteNo = nRetVal;
		m_pAlarm->SetPinString(strPinName.c_str(), TRUE);
		m_pAlarm->SetAlarmID(ALARM_ID::ALARM_PMU_CLAMP);
		m_pAlarm->SetSite(usSiteNo);
		if (0 != ClampChannel.second)
		{
			m_pAlarm->SetAlarmType(CDriverAlarm::ALARM_TYPE::CURRENT_CLAMP);
			m_pAlarm->SetAlarmMsg("The channel(S%d_%d) of pin(%s) in SITE_%d occurs current clamp.", bySlotNo, Channel.m_usChannel, strPinName.c_str(), usSiteNo + 1);
		}
		else
		{
			m_pAlarm->SetAlarmType(CDriverAlarm::ALARM_TYPE::VOLTAGE_CLAMP);
			m_pAlarm->SetAlarmMsg("The channel(S%d_%d) of pin(%s) in SITE_%d occurs voltage clamp.", bySlotNo, Channel.m_usChannel, strPinName.c_str(), usSiteNo + 1);
		}
		m_pAlarm->Output(FALSE);
	}
}

void CDCM::SetFailSyn(const std::set<std::string>& setFailSynPin)
{
	USHORT usSiteCount = m_Site.GetSiteCount();
	map<BYTE, vector<CChannelGroup>> mapFailSynChannel;

	CHANNEL_INFO Channel;
	for (const auto& Pin : setFailSynPin)
	{
		auto iterPin = m_mapPin.find(Pin);
		if (m_mapPin.end() == iterPin || nullptr == iterPin->second)
		{
			continue;
		}
		for (USHORT usSiteNo = 0; usSiteNo < usSiteCount; ++usSiteNo)
		{
			iterPin->second->GetChannel(usSiteNo, Channel);
			if (m_mapBoard.end() == m_mapBoard.find(Channel.m_bySlotNo))
			{
				continue;
			}
			auto iterChannel = mapFailSynChannel.find(Channel.m_bySlotNo);
			if (mapFailSynChannel.end() == iterChannel)
			{
				vector<CChannelGroup> vecChannel;
				for (USHORT usCurSite = 0; usCurSite <= usSiteNo; ++usCurSite)
				{
					vecChannel.push_back(CChannelGroup());
				}
				mapFailSynChannel.insert(make_pair(Channel.m_bySlotNo, vecChannel));
				iterChannel = mapFailSynChannel.find(Channel.m_bySlotNo);
			}
			else
			{
				USHORT usSiteCount = iterChannel->second.size();

				for (USHORT usCurSiteNo = usSiteCount; usCurSiteNo <= usSiteNo; ++usCurSiteNo)
				{
					iterChannel->second.push_back(CChannelGroup());
				}
			}
			iterChannel->second[usSiteNo].AddChannel(Channel.m_usChannel);
		}
	}
	int nRetVal = 0;
	int nConfilctSite = 0;
	for (auto& FailSynChannel : mapFailSynChannel)
	{
		auto iterBoard = m_mapBoard.find(FailSynChannel.first);
		if (m_mapBoard.end() == iterBoard || nullptr == iterBoard->second)
		{
			continue;
		}
		nRetVal = iterBoard->second->SetFailSyn(FailSynChannel.second, nConfilctSite);
		if (0 != nRetVal)
		{
			nRetVal = -1;
			break;
		}

	}
	if (0 == nRetVal)
	{
		return;
	}

	m_pAlarm->SetAlarmMsg("The vector using conditional instruction, but the channels in SITE_%d are using same controller with other site,	the instruction may not operation same as expected", nConfilctSite + 1);
	m_pAlarm->SetAlarmLevel(CDriverAlarm::ALARM_LEVEL::AlarmError);
	m_pAlarm->SetAlarmID(ALARM_ID::ALARM_CONDITIONAL_INSTR_ERROR);
}

void CDCM::GetPinGroupSection(std::string& strSection)
{
	strSection = "PinGroup";
	if (m_bVectorShared)
	{
		strSection = "PinGroup_Shared";
	}
}
