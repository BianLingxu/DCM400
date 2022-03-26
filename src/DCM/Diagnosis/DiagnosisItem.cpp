#include "DiagnosisItem.h"
#include "..\HDModule.h"
#include "..\HardwareFunction.h"
#include <iterator>
using namespace std;
extern bool operator==(const CHardwareFunction::DATA_RESULT& SourceResult, const CHardwareFunction::DATA_RESULT& TargetResult)
{
	if (SourceResult.m_nLineNo != TargetResult.m_nLineNo || SourceResult.m_usData != TargetResult.m_usData)
	{
		return false;
	}
	return true;
};
CDiagnosisItem::CDiagnosisItem()
	: m_nEnableStatus(2)
	, m_nIndent(0)
	, m_strIndent()
	, m_strIndentChar("  ")
	, m_UserRole(USER)
	, m_pReportDevice(nullptr)
{
}

CDiagnosisItem::~CDiagnosisItem()
{
	ClearMap(m_mapHardware);
}

void CDiagnosisItem::SetIndent(int nIndent)
{
    m_nIndent = nIndent;
    m_strIndent = std::string();
    for (int i = 0; i < nIndent; ++i)
    {
        m_strIndent += IndentChar();
    }
}

void CDiagnosisItem::SetEnableController(const std::vector<UINT>& vecController)
{
	m_vecEnableController.clear();
	for (auto uControllerID : vecController)
	{
		m_vecEnableController.push_back(uControllerID);
	}
}

int CDiagnosisItem::GetEnableController(std::vector<UINT>& vecController)
{
	vecController.clear();
	for (auto uControllerID : m_vecEnableController)
	{
		vecController.push_back(uControllerID);
	}
	return 0;
}

void CDiagnosisItem::Bind(const std::vector<UINT>& vecController, UINT uTargetController)
{
	BYTE byBoardControllerIndex = 0;
	BYTE byTargetSlot = HDModule::Instance()->ID2Board(uTargetController, byBoardControllerIndex);
	set<BYTE> setController;
	set<BYTE> setBoard;
	BYTE bySlotNo = 0;
	for (auto& uControllerID : vecController)
	{
		bySlotNo = HDModule::Instance()->ID2Board(uControllerID, byBoardControllerIndex);
		if (setBoard.end() == setBoard.find(bySlotNo))
		{
			setBoard.insert(bySlotNo);
		}
		if (setController.end() == setController.find(byBoardControllerIndex))
		{
			setController.insert(byBoardControllerIndex);
		}
	}
	if (0 == setController.size())
	{
		return;
	}

	CBindInfo::Instance()->Bind(setBoard, setController, byTargetSlot);
}

void CDiagnosisItem::ClearBind()
{
	CBindInfo::Instance()->ClearBind();
}

int CDiagnosisItem::SetEnabled(int nEnable)
{
	m_nEnableStatus = nEnable;
	return 0;
}

int CDiagnosisItem::IsEnabled() const
{
	return m_nEnableStatus;
}

int CDiagnosisItem::IsCheckAble() const
{
/*#ifdef _DEBUG*/
    return 1;
// #endif
// 	return 0;
}

void CDiagnosisItem::StartTimer()
{
	LARGE_INTEGER StartTic;
	QueryPerformanceCounter(&StartTic);
	m_stackStartTick.push(StartTic);
}

double CDiagnosisItem::StopTimer(char* lpszTimeUnits, int nBuffSize)
{
	if (m_stackStartTick.empty())
	{
		return -1;
	}
	LARGE_INTEGER StopTic, Freq;
	QueryPerformanceCounter(&StopTic);
	QueryPerformanceFrequency(&Freq);

	double dTimeCousume = (double)(StopTic.QuadPart - m_stackStartTick.top().QuadPart) / Freq.QuadPart;
	m_stackStartTick.pop();

	if (nullptr != lpszTimeUnits && 2 < nBuffSize)
	{
		if (1e-3 > dTimeCousume)
		{
			dTimeCousume *= 1e6;
			strcpy_s(lpszTimeUnits, nBuffSize, "us");
		}
		else if (1 > dTimeCousume)
		{
			dTimeCousume *= 1e3;
			strcpy_s(lpszTimeUnits, nBuffSize, "ms");
		}
		else if (60 > dTimeCousume)
		{
			strcpy_s(lpszTimeUnits, nBuffSize, "s");
		}
		else if (3600 > dTimeCousume)
		{
			dTimeCousume /= 60;
			strcpy_s(lpszTimeUnits, nBuffSize, "min");
		}
		else if(86400 > dTimeCousume)
		{
			dTimeCousume /= 3600;
			strcpy_s(lpszTimeUnits, nBuffSize, "h");
		}
		else
		{
			dTimeCousume /= 86400;
			strcpy_s(lpszTimeUnits, nBuffSize, "d");
		}
	}
	return dTimeCousume;
}

double CDiagnosisItem::GetTimeUnits(double dTime, char* lpszTimeUnits, int nBuffSize)
{
	if (nullptr != lpszTimeUnits && 2 < nBuffSize)
	{
		if (1e-3 > dTime)
		{
			dTime *= 1e6;
			strcpy_s(lpszTimeUnits, nBuffSize, "us");
		}
		else if (1 > dTime)
		{
			dTime *= 1e3;
			strcpy_s(lpszTimeUnits, nBuffSize, "ms");
		}
		else if (60 > dTime)
		{
			strcpy_s(lpszTimeUnits, nBuffSize, "s");
		}
		else if (3600 > dTime)
		{
			dTime /= 60;
			strcpy_s(lpszTimeUnits, nBuffSize, "min");
		}
		else if (86400 > dTime)
		{
			dTime /= 3600;
			strcpy_s(lpszTimeUnits, nBuffSize, "h");
		}
		else
		{
			dTime /= 86400;
			strcpy_s(lpszTimeUnits, nBuffSize, "d");
		}
	}
	return dTime;
}

double CDiagnosisItem::GetPeriodUnits(double dPeriod, char* lpszPeriodUnits, int nBuffSize)
{
	if (nullptr != lpszPeriodUnits && 2 < nBuffSize)
	{
		if (1e3 > dPeriod)
		{
			strcpy_s(lpszPeriodUnits, nBuffSize, "ns");
		}
		else if (1e6 > dPeriod)
		{
			dPeriod *= 1e-3;
			strcpy_s(lpszPeriodUnits, nBuffSize, "us");
		}
		else if (1e9 > dPeriod)
		{
			dPeriod *= 1e-6;
			strcpy_s(lpszPeriodUnits, nBuffSize, "ms");
		}
		else
		{
			dPeriod *= 1e-9;
			strcpy_s(lpszPeriodUnits, nBuffSize, "s");
		}
	}
	return dPeriod;
}

void CDiagnosisItem::Wait(UINT uUs)
{
	LARGE_INTEGER TimeCur, TimeStop, TimeFreq;
	QueryPerformanceFrequency(&TimeFreq);
	QueryPerformanceCounter(&TimeCur);
	TimeStop.QuadPart = TimeCur.QuadPart + uUs * TimeFreq.QuadPart * 1e-6;
	while (TimeStop.QuadPart > TimeCur.QuadPart)
	{
		QueryPerformanceCounter(&TimeCur);
	}
}

int CDiagnosisItem::WaitStop()
{
	int nRetVal = 0;
	for (auto uControllerID : m_vecEnableController)
	{
		nRetVal = WaitStop(uControllerID);
		if (-1 == nRetVal)
		{
			///<Not ran vector before
			return -1;
		}
		else if (-2 == nRetVal)
		{
			///<Not stop before timeout
			return -2;
		}
	}
	return 0;
}

int CDiagnosisItem::WaitStop(UINT uControllerID)
{
	UINT uTotalWaitTimes = (UINT)16e6;
	auto iterCurHardware = m_mapHardware.find(uControllerID);
	if (m_mapHardware.end() == iterCurHardware)
	{
		BYTE byCurSlotNo = 0;
		BYTE byCurBoardControllerIndex = 0;
		byCurSlotNo = HDModule::Instance()->ID2Board(uControllerID, byCurBoardControllerIndex);
		CHardwareFunction* pHardware = new CHardwareFunction(byCurSlotNo);
		pHardware->SetControllerIndex(byCurBoardControllerIndex);
		m_mapHardware.insert(make_pair(uControllerID, pHardware));
		iterCurHardware = m_mapHardware.find(uControllerID);
	}

	UINT uWaitTimes = 0;
	int nRetVal = 0;
	do
	{
		if (0 != uWaitTimes++)
		{
			iterCurHardware->second->DelayUs(10);
		}
		nRetVal = iterCurHardware->second->GetRunningStatus();
	} while (0 == nRetVal && uWaitTimes < uTotalWaitTimes);
	if (-1 == nRetVal)
	{
		///<Not ran vector before
		return -1;
	}
	if (uTotalWaitTimes <= uWaitTimes)
	{
		return -2;
	}
	return 0;
}

CHardwareFunction* CDiagnosisItem::GetHardware(UINT uControllerID)
{
	auto iterController = m_mapHardware.find(uControllerID);
	if (m_mapHardware.end() == iterController || nullptr == iterController->second)
	{
		BYTE bySlotNo = 0;
		BYTE byControllerIndex = 0;
		bySlotNo = HDModule::Instance()->ID2Board(uControllerID, byControllerIndex);
		CHardwareFunction* pHardware = new CHardwareFunction(bySlotNo);
		pHardware->SetControllerIndex(byControllerIndex);
		if (m_mapHardware.end() != iterController)
		{
			iterController->second = pHardware;
		}
		else
		{
			m_mapHardware.insert(make_pair(uControllerID, pHardware));
			iterController = m_mapHardware.find(uControllerID);
		}
	}
	return iterController->second;
}

void CDiagnosisItem::Connect()
{
	vector<USHORT> vecBoardChannel;
	for (USHORT usChannel = 0; usChannel < DCM_MAX_CHANNELS_PER_BOARD; ++usChannel)
	{
		vecBoardChannel.push_back(usChannel);
	}
	set<BYTE> setSlot;
	BYTE bySlotNo = 0;
	BYTE byController = 0;
	for (auto uControllerID : m_vecEnableController)
	{
		bySlotNo = HDModule::Instance()->ID2Board(uControllerID, byController);
		if (setSlot.end() == setSlot.find(bySlotNo))
		{
			CHardwareFunction* pHardware = GetHardware(uControllerID);
			pHardware->SetFunctionRelay(vecBoardChannel);
			setSlot.insert(bySlotNo);
		}
	}
}

void CDiagnosisItem::Disconnect()
{
	vector<USHORT> vecBoardChannel;
	for (USHORT usChannel = 0; usChannel < DCM_MAX_CHANNELS_PER_BOARD; ++usChannel)
	{
		vecBoardChannel.push_back(usChannel);
	}
	set<BYTE> setSlot;
	BYTE bySlotNo = 0;
	BYTE byController = 0;
	for (auto uControllerID : m_vecEnableController)
	{
		bySlotNo = HDModule::Instance()->ID2Board(uControllerID, byController);
		if (setSlot.end() == setSlot.find(bySlotNo))
		{
			CHardwareFunction* pHardware = GetHardware(uControllerID);
			pHardware->SetFunctionRelay(vecBoardChannel, FALSE);
			setSlot.insert(bySlotNo);
		}
	}
}

void CDiagnosisItem::CheckMutualDiagnosable()
{
	if (0 == m_vecEnableController.size() && 0 == m_mapUndiagnosableController.size())
	{
		return;
	}

	set<UINT> setController;
	for (auto uControllerID : m_vecEnableController)
	{
		setController.insert(uControllerID);
	}
	m_vecEnableController.clear();

	for (auto& Undiagnosable : m_mapUndiagnosableController)
	{
		setController.insert(Undiagnosable.first);
	}
	m_mapUndiagnosableController.clear();


	BYTE bySlotNo = 0;
	BYTE byBoardControllerIndex = 0;
	BYTE byRelatedController = 0;
	UINT uRelatedControllerID = 0;
	for (auto uControllerID : setController)
	{
		bySlotNo = HDModule::Instance()->ID2Board(uControllerID, byBoardControllerIndex);
		if (0 == byBoardControllerIndex % 2)
		{
			byRelatedController = byBoardControllerIndex + 1;
		}
		else
		{
			byRelatedController = byBoardControllerIndex - 1;
		}
		uRelatedControllerID = HDModule::Instance()->GetID(bySlotNo, byRelatedController);
		if (setController.end() == setController.find(uRelatedControllerID))
		{
			m_mapUndiagnosableController.insert(make_pair(uRelatedControllerID, uControllerID));
		}
	}

	for (auto& undiagnosable : m_mapUndiagnosableController)
	{
		auto iterController = setController.find(undiagnosable.first);
		if (setController.end() != iterController)
		{
			setController.erase(iterController);
		}
	}

	copy(setController.begin(), setController.end(), back_inserter(m_vecEnableController));
}

void CDiagnosisItem::SetUserRole(USER_ROLE UserRole)
{
	m_UserRole = UserRole;

#ifdef _DEBUG
	m_UserRole = DEVELOPER;
#endif // _DEBUG
}

bool CDiagnosisItem::IsUserCheck() const
{
	return true;
}

template<typename Key, typename Value>
inline void CDiagnosisItem::ClearMap(std::map<Key, Value*>& mapData)
{
	for (auto& Param : mapData)
	{
		if (nullptr != Param.second)
		{
			delete Param.second;
			Param.second = nullptr;
		}
	}
	mapData.clear();
}