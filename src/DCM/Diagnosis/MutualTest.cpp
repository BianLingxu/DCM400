#include "MutualTest.h"
#include "..\HDModule.h"
#include "..\HardwareFunction.h"
#include "..\Relay.h"
#include "..\Pattern.h"
#include <set>
using namespace std;

//#define _UNBIND 1

CMutualTest::CMutualTest()
{
	m_nCurItemTestIndex = 0;
	m_nEnableStatus = 2;
	m_bConnect = FALSE;
}

CMutualTest::~CMutualTest()
{
	ClearMap(m_mapPattern);
}

int CMutualTest::Doctor(IHDReportDevice* pReportDevice)
{
	if (0 == m_nEnableStatus)
	{
		return 0;
	}
	m_pReportDevice = pReportDevice;

	m_mapFailLineNo.clear();
	m_mapTimeset.clear();
	m_mapBRAMFailLineNo.clear();
	m_mapBRAMTimeset.clear();
	m_mapDRAMFailLineNo.clear();
	m_mapDRAMTimeset.clear();

	CheckMutualDiagnosable();

	const char* lpszBaseIndent = IndentFormat();
	string strNextIndent = lpszBaseIndent + IndentChar();
	const char* lpszNextIndent = strNextIndent.c_str();
	BYTE bySlotNo = 0;
	BYTE byBoardControllerIndex = 0;
	for (auto& unDiagnosable : m_mapUndiagnosableController)
	{
		bySlotNo = HDModule::Instance()->ID2Board(unDiagnosable.first, byBoardControllerIndex);
		m_pReportDevice->PrintfToUi(IHDReportDevice::Fail);
		m_pReportDevice->PrintfToUi("\t Slot %d, Controller %d\n", bySlotNo, byBoardControllerIndex);
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<Controller value='%d, slot value=%d'>", lpszBaseIndent, bySlotNo, byBoardControllerIndex);

		bySlotNo = HDModule::Instance()->ID2Board(unDiagnosable.second, byBoardControllerIndex);
		m_pReportDevice->PrintfToUi("\t\t Undiagnosable for slot %d controller %d is not existed.", bySlotNo, byBoardControllerIndex);


		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<Info>Undiagnosable for slot %d controller %d is not existed.</Info>", lpszNextIndent, bySlotNo, byBoardControllerIndex);
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s</Controller>", lpszBaseIndent);
	}
	if (0 == m_vecEnableController.size())
	{
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<Item>No controller valid to be diagnosed</Item>\n", lpszBaseIndent);
	}
	return Diagnosis();
}

int CMutualTest::Diagnosis()
{
	BOOL bAllTestPass = TRUE;
	int nTestIndex = 0;
	string strItemName;
	string strCurItemName;

	const int nLogPrintInterval = 1024;
	BOOL bPrintLog = TRUE;
	vector<UINT> vecCurTestController;
	while (0 == GetTestController(nTestIndex, vecCurTestController, bPrintLog))
	{
		for (auto uControllerID : vecCurTestController)
		{
			m_setCurTestController.insert(uControllerID);
		}
		strCurItemName = GetSubItemName();

		if (m_pReportDevice->IsStop() && Stop())
		{
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<UserStop value='NextItem=%s'/>\n", IndentFormat(), strCurItemName.c_str());
			break;
		}

		if (0 == nTestIndex || 0 != strItemName.compare(strCurItemName))
		{
			strItemName = strCurItemName;
			if (nullptr != m_pReportDevice && 0 != strItemName.size())
			{
				m_pReportDevice->PrintfToUi("  %s\n", strItemName.c_str());
				if (0 != m_nCurItemTestIndex)
				{
					m_nCurItemTestIndex = -1;
				}
			}
		}

		if (0 != DiagnosisItem())
		{
			bAllTestPass = FALSE;
		}
		++nTestIndex;
		++m_nCurItemTestIndex;

		if (0 == m_nCurItemTestIndex % nLogPrintInterval)
		{
			bPrintLog = TRUE;
		}
		else
		{
			bPrintLog = FALSE;
		}
	}

	Connect(FALSE);

	if (bAllTestPass)
	{
		return 0;
	}
	else
	{
		return -1;
	}
}

int CMutualTest::DiagnosisItem()
{
	auto iterTestController = m_setCurTestController.begin();

	Connect(TRUE);

	LoadVector();

	Run(m_mapRunInfo);
	
	int nRetVal = CheckResult();

	if (0 != nRetVal)
	{
		return -1;
	}
	return 0;
	
}

void CMutualTest::Connect(BOOL bConnect)
{
	if (bConnect && m_bConnect)
	{
		return;
	}

	vector<USHORT> vecChannel;
	for (USHORT usChannel = 0; usChannel < HDModule::ChannelCountPerBoard;++usChannel)
	{
		vecChannel.push_back(usChannel);
	}
	BYTE byLastSlotNo = 0;
	BYTE bySlotNo = 0;
	BYTE byBoardControllerIndex = 0;
	auto iterHardware = m_mapHardware.begin();
	for (auto uControllerID : m_setCurTestController)
	{
		bySlotNo = HDModule::Instance()->ID2Board(uControllerID, byBoardControllerIndex);
		if (bySlotNo == byLastSlotNo)
		{
			continue;
		}
		byLastSlotNo = bySlotNo;
		
		CHardwareFunction* pHardware = GetHardware(uControllerID);

		pHardware->SetFunctionRelay(vecChannel, bConnect);
	}
	m_bConnect = bConnect;
	return;
}

int CMutualTest::Run(map<UINT, RUN_INFO>& mapRunInfo)
{
	if (0 == mapRunInfo.size())
	{
		return -1;
	}
	BYTE bySlotNo = 0;
	BOOL bOpenSyn = TRUE;
	auto iterHardware = m_mapHardware.begin();

	BOOL bUserDRAM = FALSE;
	for (auto& RunInfo : mapRunInfo)
	{
		if (RunInfo.second.m_bUseDRAM)
		{
			///<Ensure the start time is same
			bUserDRAM = TRUE;
			break;
		}
	}
	int nCheckDataType = 0;
	for (auto& RunInfo : mapRunInfo)
	{
		iterHardware = m_mapHardware.find(RunInfo.first);
		if (m_mapHardware.end() == iterHardware)
		{
			///<Not will happen
			continue;
		}
		iterHardware->second->SetRunParameter(RunInfo.second.m_uStartLine, RunInfo.second.m_uStopLine, bUserDRAM, RunInfo.second.m_uDRAMStartLine);
		nCheckDataType = GetCheckDataType(RunInfo.first);
		if (0 != nCheckDataType)
		{
			iterHardware->second->SetPatternMode(TRUE, DATA_TYPE::FM, FALSE, FALSE);
		}
	}
	
	iterHardware = m_mapHardware.begin();
	while (m_mapHardware.end() != iterHardware && nullptr == iterHardware->second)
	{
		++iterHardware;
	}

	if (m_mapHardware.end() != iterHardware)
	{
		iterHardware->second->SynRun();
	}


	//延时等待pattern运行结束
	UINT uTotalWaiteTimes = (UINT)10e6;

	for (auto& uControllerID : m_setCurTestController)
	{
		iterHardware = m_mapHardware.find(uControllerID);
		if (m_mapHardware.end() == iterHardware)
		{
			///<Not will happen
			continue;
		}

		UINT uWaitTimes = 0;
		int nRetVal = 0;
		do 
		{
			if (0 != uWaitTimes++)
			{
				Wait(10);
			}
			nRetVal = iterHardware->second->GetRunningStatus();
		} while (1 != nRetVal && uWaitTimes < uTotalWaiteTimes);
	}
	return 0;
}

void CMutualTest::Wait(UINT uUs)
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

int CMutualTest::CheckResult()
{
	vector<UINT> vecCheckController;
	GetCheckDataController(vecCheckController);
	BOOL bAllPass = TRUE;
	int nRetVal = 0;
	int nLineOrder = 0;
	map<int, USHORT> mapBRAMFailLineNo;
	map<int, USHORT> mapDRAMFailLineNo;
	vector<CHardwareFunction::DATA_RESULT> vecBRAMFaiLLineNo;
	vector<CHardwareFunction::DATA_RESULT> vecDRAMFaiLLineNo;
	auto CopyMap = [&](BOOL bBRAM)
	{
		auto& vecFail = bBRAM ? vecBRAMFaiLLineNo : vecDRAMFaiLLineNo;
		auto& mapFail = bBRAM ? mapBRAMFailLineNo : mapDRAMFailLineNo;
		for (auto& Data : vecFail)
		{
			mapFail.insert(make_pair(Data.m_nLineNo, Data.m_usData));
		}
	};
	vector<CHardwareFunction::DATA_RESULT> vecBRAMCapture;
	vector<CHardwareFunction::DATA_RESULT> vecDRAMCapture;
	vector<UINT> vecRanLineOrder;
	auto iterHardware = m_mapHardware.begin();

	for (auto uControllerID : vecCheckController)
	{
		iterHardware = m_mapHardware.find(uControllerID);
		if (m_mapHardware.end() == iterHardware || nullptr == iterHardware->second)
		{
			///<Not will happen
			continue;
		}
		nLineOrder = GetCheckDataType(iterHardware->first);
		if (0 != nLineOrder)
		{
			iterHardware->second->GetLineRanOrder(vecRanLineOrder);
			nRetVal = CheckLineOrder(iterHardware->first, vecRanLineOrder);
		}
		else
		{
			iterHardware->second->GetFailData(vecBRAMFaiLLineNo, vecDRAMFaiLLineNo);
			CopyMap(TRUE);
			CopyMap(FALSE);

			iterHardware->second->GetCapture(vecBRAMCapture, vecDRAMCapture);
			if (0 != vecBRAMCapture.size())
			{
				for (auto& Capture : vecBRAMCapture)
				{
					mapBRAMFailLineNo.insert(make_pair(Capture.m_nLineNo, Capture.m_usData));
				}
			}
			if (0 != vecDRAMCapture.size())
			{
				for (auto& Capture : vecDRAMCapture)
				{
					mapDRAMFailLineNo.insert(make_pair(Capture.m_nLineNo, Capture.m_usData));
				}
			}
			int nFailCount = iterHardware->second->GetFailCount();
			nRetVal = CheckResult(iterHardware->first, mapBRAMFailLineNo, mapDRAMFailLineNo, nFailCount);
		}

		if (0 != nRetVal)
		{
			bAllPass = FALSE;
		}
	}

	if (bAllPass)
	{
		return 0;
	}

	return -1;
}
inline void CMutualTest::DownloadPattern(const std::map<UINT, std::vector<UINT>>& mapBindController)
{
	///<Load vector
	for (auto& Pattern : m_mapPattern)
	{
		if (nullptr == Pattern.second || 0 == Pattern.second->GetPatternCount())
		{
			continue;
		}
		auto iterBind = mapBindController.find(Pattern.first);

		if (mapBindController.end() != iterBind)
		{
			Bind(iterBind->second, iterBind->first);
		}
		Pattern.second->Load();

		if (mapBindController.end() != iterBind)
		{
			ClearBind();
		}
	}
}

void CMutualTest::DownloadPattern()
{
	auto iterRunInfo = m_mapRunInfo.begin();
	map<UINT, UINT> mapSameController;
	auto pmapTestTestController = &m_setCurTestController;
	set<UINT> setTestController;
	map<UINT, vector<UINT>> mapBindInfo;

	int nSameVectorControllerType = 0;

#ifndef _UNBIND
	nSameVectorControllerType = GetSameVectorControllerType();
#else
	nSameVectorControllerType = 0;
#endif // !_UNBIND

	if (0 != nSameVectorControllerType)
	{
		vector<UINT> vecController;
		for (USHORT uIndex = 0; uIndex < nSameVectorControllerType; uIndex++)
		{
			GetSameVectorController(uIndex, vecController);
			if (0 != vecController.size())
			{
				mapBindInfo.insert(make_pair(vecController[0], vecController));
				auto iterTestController = m_setCurTestController.find(vecController[0]);
				if (m_setCurTestController.end() != iterTestController)
				{
					setTestController.insert(*iterTestController);
					int nControllerCount = vecController.size();
					for (USHORT usControllerIndex = 1; usControllerIndex < nControllerCount; ++usControllerIndex)
					{
						mapSameController.insert(make_pair(vecController[usControllerIndex], vecController[0]));
					}
				}
			}
		}
		pmapTestTestController = &setTestController;
	}

	if (IsReloadVector())
	{
		UINT uBRAMStartLine = 0;
		UINT uDRAMStartLine = 0;
		GetVectorStartLine(&uBRAMStartLine, &uDRAMStartLine);
		char lpszVector[17] = { 0 };
		char lpszCMD[16] = "INC";

		ULONG ulOperand = 0;
		BYTE byTimeSet = 0;
		BOOL bLastLine = 0;
		BOOL bSRAMLine = TRUE;
		set<BYTE> setLoadController;
		m_mapRunInfo.clear();

		for (auto& ControllerID : *pmapTestTestController)
		{
			{
				BOOL bNextLineOtherMemory = FALSE;
				UINT uBRAMOffset = 0;
				UINT uDRAMOffset = 0;
				UINT* puOffset = nullptr;
				const UINT* puStartLine = nullptr;
				RUN_INFO RunInfo;
				RunInfo.m_uStartLine = uBRAMStartLine;
				int nLineIndex = 0;
				BOOL bSwitch = FALSE;
				do
				{
					bLastLine = GetMutualTestVector(ControllerID, nLineIndex, lpszVector, sizeof(lpszVector), lpszCMD, sizeof(lpszCMD), &byTimeSet, &ulOperand, &bSRAMLine, &bNextLineOtherMemory);
					if (0 != bLastLine)
					{
						break;
					}

					auto iterPattern = m_mapPattern.find(ControllerID);
					if (m_mapPattern.end() == iterPattern)
					{
						CHardwareFunction* pHardawre = GetHardware(ControllerID);
						CPattern* pPattern = new CPattern(*pHardawre);
						m_mapPattern.insert(make_pair(ControllerID, pPattern));
						iterPattern = m_mapPattern.find(ControllerID);
					}
					if (bSRAMLine)
					{
						puStartLine = &uBRAMStartLine;
						puOffset = &uBRAMOffset;
					}
					else
					{
						puStartLine = &uDRAMStartLine;
						puOffset = &uDRAMOffset;
					}
					if (!bSwitch)
					{
						bSwitch = bNextLineOtherMemory;
					}
					BOOL bCapture = FALSE;
					int nCurLineIndex = *puStartLine + *puOffset;
					if (bSRAMLine && m_setCaptureLine.end() != m_setCaptureLine.find(nCurLineIndex))
					{
						bCapture = TRUE;
						m_bUseCapture = TRUE;
					}
					iterPattern->second->AddPattern(nCurLineIndex, bSRAMLine, lpszVector, byTimeSet, lpszCMD, "", (USHORT)ulOperand, bCapture, bNextLineOtherMemory);
					if (2048 <= iterPattern->second->GetPatternCount())
					{
						DownloadPattern(mapBindInfo);
					}

					++* puOffset;
					++nLineIndex;
				} while (TRUE);
				setLoadController.insert(ControllerID);

				///<Save the run information
				RunInfo.m_uStopLine = uBRAMStartLine + uBRAMOffset - 1;
				RunInfo.m_bUseDRAM = bSwitch;
				RunInfo.m_uDRAMStartLine = 0;
				m_mapRunInfo.insert(make_pair(ControllerID, RunInfo));
			}
		}
		DownloadPattern(mapBindInfo);
	}

	for (auto& SampeController : mapSameController)
	{
		auto iterRunInfo = m_mapRunInfo.find(SampeController.first);
		if (m_mapRunInfo.end() == iterRunInfo)
		{
			iterRunInfo = m_mapRunInfo.find(SampeController.second);
			if (m_mapRunInfo.end() != iterRunInfo)
			{
				m_mapRunInfo.insert(make_pair(SampeController.first, iterRunInfo->second));
			}
		}
		else
		{
			auto iterTargetRunInfo = m_mapRunInfo.find(SampeController.second);
			if (m_mapRunInfo.end() != iterRunInfo)
			{
				m_mapRunInfo.insert(make_pair(SampeController.first, iterTargetRunInfo->second));
			}
		}
	}
	mapSameController.clear();

	if (0 != setTestController.size())
	{
		setTestController.clear();
	}
}

int CMutualTest::GetVectorSwitchInfo(UINT uTotalLineCount)
{
	m_vecSwitch.clear();
	m_mapBRAMFailLineNo.clear();
	m_mapDRAMFailLineNo.clear();
	m_mapBRAMTimeset.clear();
	m_mapDRAMTimeset.clear();
	if (DCM_TOTAL_LINE_COUNT < uTotalLineCount)
	{
		return -1;
	}
	else if (DCM_BRAM_PATTERN_LINE_COUNT >= uTotalLineCount)
	{
		auto iterTimeset = m_mapTimeset.end();
		for (auto& FailLineNo : m_mapFailLineNo)
		{
			m_mapBRAMFailLineNo.insert(make_pair(FailLineNo.first, FailLineNo.second));
			if (BRAM_MAX_SAVE_FAIL_LINE_COUNT < m_mapBRAMFailLineNo.size())
			{
				m_setCaptureLine.insert(FailLineNo.first);
			}
			iterTimeset = m_mapTimeset.find(FailLineNo.first);
			if (m_mapTimeset.end() != iterTimeset)
			{
				m_mapBRAMTimeset.insert(make_pair(FailLineNo.first, iterTimeset->second));
			}
		}

		return 0;
	}
	UINT uLastFailLine = 0;
	auto riterFailLine = m_mapFailLineNo.rbegin();
	if (m_mapFailLineNo.rend() != riterFailLine)
	{
		uLastFailLine = riterFailLine->first;
	}
	if (uTotalLineCount <= uLastFailLine)
	{
		return -2;
	}
	SWITCH_INFO SwitchInfo;
	SWITCH_INFO* pSwitchInfo = nullptr;

	UINT uBRAMLineCount = 0;
	UINT uDRAMLineCount = 0;
	int nFailCount = m_mapFailLineNo.size();
	int nFailIndex = 0;
	for (auto& FailLineNo : m_mapFailLineNo)
	{
		do
		{
			///<Add switch in
			int nLastSwitchIndex = m_vecSwitch.size() - 1;
			if (-1 == nLastSwitchIndex)
			{
				SwitchInfo.m_bSwitchOut = TRUE;
				SwitchInfo.m_uSwitchLineNo = 0;
				SwitchInfo.m_uGlobalLineIndex = MIN_RAM_LINE_COUNT - 1;
				m_vecSwitch.push_back(SwitchInfo);
				if (SwitchInfo.m_uGlobalLineIndex > FailLineNo.first)
				{
					m_mapBRAMFailLineNo.insert(make_pair(FailLineNo.first, FailLineNo.second));
					if (BRAM_MAX_SAVE_FAIL_LINE_COUNT < m_mapBRAMFailLineNo.size())
					{
						m_setCaptureLine.insert(FailLineNo.first);
					}
					auto iterTimeset = m_mapTimeset.find(FailLineNo.first);
					if (m_mapTimeset.end() != iterTimeset)
					{
						m_mapBRAMTimeset.insert(make_pair(FailLineNo.first, iterTimeset->second));
					}
					break;

				}
				nLastSwitchIndex = 0;
			}
			pSwitchInfo = &m_vecSwitch[nLastSwitchIndex];

			int nBlockLineSize = FailLineNo.first - pSwitchInfo->m_uGlobalLineIndex - 1;

			if (pSwitchInfo->m_bSwitchOut)
			{
				BOOL bPopBack = FALSE;
				if (MIN_DRAM_LINE_COUNT > nBlockLineSize)
				{
					///<The line count in current block DRAM is too little to switch in, so not switch in
					bPopBack = TRUE;
				}
				if (!bPopBack)
				{
					int nNextMemoryMiniCount = 1;

					if (0 != nBlockLineSize % 2)
					{
						///<The line count of DRAM block must be even
						--nBlockLineSize;
						++nNextMemoryMiniCount;
					}

					if (nFailIndex + 1 == nFailCount && MIN_RAM_LINE_COUNT > nNextMemoryMiniCount)
					{
						///<Ensure the line count in last block of SRAM is satisfy the minimum limit
						nBlockLineSize = MIN_RAM_LINE_COUNT - nNextMemoryMiniCount;
						if (0 != nBlockLineSize % 2 && MIN_DRAM_LINE_COUNT >= nBlockLineSize)
						{
							///<The line count of DRAM block must be even
							--nBlockLineSize;
						}
						if (MIN_DRAM_LINE_COUNT > nBlockLineSize)
						{
							///<The line count in current block DRAM is too little to switch in, so delete the latest switch in
							bPopBack = TRUE;
						}
					}
				}
				if (bPopBack)
				{
					UINT uSRAMLineNo = FailLineNo.first;
					if (0 != nLastSwitchIndex)
					{
						uSRAMLineNo = m_vecSwitch[nLastSwitchIndex - 1].m_uSwitchLineNo + m_vecSwitch[nLastSwitchIndex - 1].m_uSwitchLineCount;
					}
					m_mapBRAMFailLineNo.insert(make_pair(uSRAMLineNo, FailLineNo.second));
					if (BRAM_MAX_SAVE_FAIL_LINE_COUNT < m_mapBRAMFailLineNo.size())
					{
						m_setCaptureLine.insert(uSRAMLineNo);
					}
					auto iterTimeset = m_mapTimeset.find(FailLineNo.first);
					if (m_mapTimeset.end() != iterTimeset)
					{
						m_mapBRAMTimeset.insert(make_pair(uSRAMLineNo, iterTimeset->second));
					}
					pSwitchInfo->m_uGlobalLineIndex = FailLineNo.first;
					break;
				}

				pSwitchInfo->m_uSwitchLineCount = nBlockLineSize;
				SwitchInfo.m_uGlobalLineIndex = pSwitchInfo->m_uGlobalLineIndex + pSwitchInfo->m_uSwitchLineCount;


				SwitchInfo.m_bSwitchOut = FALSE;
				SwitchInfo.m_uSwitchLineNo = 0 == nLastSwitchIndex ? pSwitchInfo->m_uGlobalLineIndex + 1 : m_vecSwitch[nLastSwitchIndex - 1].m_uSwitchLineNo + m_vecSwitch[nLastSwitchIndex - 1].m_uSwitchLineCount;
				SwitchInfo.m_uSwitchLineCount = FailLineNo.first - SwitchInfo.m_uGlobalLineIndex;
				m_vecSwitch.push_back(SwitchInfo);

				pSwitchInfo = &m_vecSwitch[++nLastSwitchIndex];

				UINT uSRAMLineNo = FailLineNo.first - pSwitchInfo->m_uGlobalLineIndex + pSwitchInfo->m_uSwitchLineNo - 1;
				m_mapBRAMFailLineNo.insert(make_pair(uSRAMLineNo, FailLineNo.second));
				if (BRAM_MAX_SAVE_FAIL_LINE_COUNT < m_mapBRAMFailLineNo.size())
				{
					m_setCaptureLine.insert(uSRAMLineNo);
				}
				auto iterTimeset = m_mapTimeset.find(FailLineNo.first);
				if (m_mapTimeset.end() != iterTimeset)
				{
					m_mapBRAMTimeset.insert(make_pair(uSRAMLineNo, iterTimeset->second));
				}
				break;
			}

			SwitchInfo.m_bSwitchOut = TRUE;
			pSwitchInfo->m_uSwitchLineCount = MIN_RAM_LINE_COUNT > pSwitchInfo->m_uSwitchLineCount ? MIN_RAM_LINE_COUNT : pSwitchInfo->m_uSwitchLineCount;
			SwitchInfo.m_uGlobalLineIndex = pSwitchInfo->m_uGlobalLineIndex + pSwitchInfo->m_uSwitchLineCount;
			
			pSwitchInfo = &m_vecSwitch[nLastSwitchIndex - 1];
			SwitchInfo.m_uSwitchLineNo = pSwitchInfo->m_uSwitchLineNo + pSwitchInfo->m_uSwitchLineCount;
			m_vecSwitch.push_back(SwitchInfo);
		} while (TRUE);
		++nFailIndex;
	}
	int nSwitchCount = m_vecSwitch.size();
	for (int nIndex = 1; nIndex < nSwitchCount; ++nIndex)
	{
		SWITCH_INFO* pFornt = &m_vecSwitch[nIndex - 1];
		SWITCH_INFO* pCur = &m_vecSwitch[nIndex];
		if (pCur->m_uGlobalLineIndex + 1 != pFornt->m_uSwitchLineNo + pFornt->m_uSwitchLineCount + pCur->m_uSwitchLineNo)
		{
			nIndex = nIndex;
		}
	}


	int nLastSwitchIndex = m_vecSwitch.size() - 1;
	int nCurrentSRAMLineCount = MIN_RAM_LINE_COUNT;
	if (-1 != nLastSwitchIndex)
	{
		pSwitchInfo = &m_vecSwitch[nLastSwitchIndex];
		if (pSwitchInfo->m_bSwitchOut)
		{
			if (0 == nLastSwitchIndex)
			{
				riterFailLine = m_mapFailLineNo.rbegin();
				if (m_mapFailLineNo.rend() != riterFailLine)
				{
					nCurrentSRAMLineCount = riterFailLine->first + 1;
				}
				uBRAMLineCount = nCurrentSRAMLineCount;
			}
			else
			{
				UINT uGlobalLineNo = pSwitchInfo->m_uGlobalLineIndex;
				pSwitchInfo = &m_vecSwitch[nLastSwitchIndex - 1];
				pSwitchInfo->m_uSwitchLineCount = uGlobalLineNo - pSwitchInfo->m_uGlobalLineIndex;
				nCurrentSRAMLineCount = pSwitchInfo->m_uSwitchLineNo;

				uBRAMLineCount = pSwitchInfo->m_uSwitchLineNo + pSwitchInfo->m_uSwitchLineCount;


				pSwitchInfo = &m_vecSwitch[nLastSwitchIndex - 2];
				uDRAMLineCount = pSwitchInfo->m_uSwitchLineNo + pSwitchInfo->m_uSwitchLineCount;
			}
			m_vecSwitch.pop_back();
			--nLastSwitchIndex;
		}
		else
		{
			uBRAMLineCount = pSwitchInfo->m_uSwitchLineNo + pSwitchInfo->m_uSwitchLineCount;
			pSwitchInfo = &m_vecSwitch[nLastSwitchIndex - 1];
			uDRAMLineCount = pSwitchInfo->m_uSwitchLineNo + pSwitchInfo->m_uSwitchLineCount;
		}
	}
	else
	{
		riterFailLine = m_mapFailLineNo.rbegin();
		if (m_mapFailLineNo.rend() != riterFailLine)
		{
			nCurrentSRAMLineCount = riterFailLine->first + 1;
		}
		uBRAMLineCount += uTotalLineCount;
	}

	int nLeftLineCount = uTotalLineCount - uBRAMLineCount - uDRAMLineCount;
	if (DCM_BRAM_PATTERN_LINE_COUNT < uBRAMLineCount + nLeftLineCount)
	{
		///<The line left can't be save in SRAM all
		///<Add new switch out
		if (-1 == nLastSwitchIndex)
		{
			///<Not using DRAM before
			SwitchInfo.m_bSwitchOut = TRUE;
			SwitchInfo.m_uGlobalLineIndex = nCurrentSRAMLineCount - 1;
			SwitchInfo.m_uSwitchLineNo = 0;
			m_vecSwitch.push_back(SwitchInfo);
			nLastSwitchIndex = 0;
		}
		else
		{
			pSwitchInfo = &m_vecSwitch[nLastSwitchIndex];///<The last switch
			if (!pSwitchInfo->m_bSwitchOut)
			{
				SwitchInfo.m_bSwitchOut = TRUE;
				pSwitchInfo->m_uSwitchLineCount = uBRAMLineCount;
				if (MIN_RAM_LINE_COUNT > pSwitchInfo->m_uSwitchLineCount)
				{
					pSwitchInfo->m_uSwitchLineCount = MIN_RAM_LINE_COUNT;

				}
				uBRAMLineCount = pSwitchInfo->m_uSwitchLineCount + pSwitchInfo->m_uSwitchLineNo;

				SwitchInfo.m_uGlobalLineIndex = pSwitchInfo->m_uGlobalLineIndex + pSwitchInfo->m_uSwitchLineCount - 1;

				nLeftLineCount = uTotalLineCount - uBRAMLineCount - uDRAMLineCount;
				pSwitchInfo = &m_vecSwitch[nLastSwitchIndex - 1];///<The latest switch out
				SwitchInfo.m_uSwitchLineNo = pSwitchInfo->m_uSwitchLineNo + pSwitchInfo->m_uSwitchLineCount;
				m_vecSwitch.push_back(SwitchInfo);
				++nLastSwitchIndex;
			}
		}

		///<Add switch in
		pSwitchInfo = &m_vecSwitch[nLastSwitchIndex];
		UINT uLastDRAMLineCount = nLeftLineCount - MIN_RAM_LINE_COUNT;
		uLastDRAMLineCount = 0 != uLastDRAMLineCount % 2 ? uLastDRAMLineCount - 1 : uLastDRAMLineCount;
		nLeftLineCount -= uLastDRAMLineCount;

		pSwitchInfo->m_uSwitchLineCount = uLastDRAMLineCount;
		SwitchInfo.m_bSwitchOut = FALSE;
		SwitchInfo.m_uGlobalLineIndex = pSwitchInfo->m_uGlobalLineIndex + pSwitchInfo->m_uSwitchLineCount;
		if (0 < nLastSwitchIndex)
		{
			pSwitchInfo = &m_vecSwitch[nLastSwitchIndex - 1];
			SwitchInfo.m_uSwitchLineNo = pSwitchInfo->m_uSwitchLineNo + pSwitchInfo->m_uSwitchLineCount;
		}
		else
		{
			SwitchInfo.m_uSwitchLineNo = pSwitchInfo->m_uGlobalLineIndex + 1;
		}
		SwitchInfo.m_uSwitchLineCount = nLeftLineCount;
		m_vecSwitch.push_back(SwitchInfo);
	}
	else if (-1 != nLastSwitchIndex)
	{
		m_vecSwitch[nLastSwitchIndex].m_uSwitchLineCount += nLeftLineCount;
	}

	nLastSwitchIndex = m_vecSwitch.size() - 1;
	if (-1 != nLastSwitchIndex)
	{
		pSwitchInfo = &m_vecSwitch[nLastSwitchIndex];
		uBRAMLineCount = pSwitchInfo->m_uSwitchLineNo + pSwitchInfo->m_uSwitchLineCount;
		pSwitchInfo = &m_vecSwitch[nLastSwitchIndex - 1];
		uDRAMLineCount = pSwitchInfo->m_uSwitchLineNo + pSwitchInfo->m_uSwitchLineCount;
	}

	int nRetVal = 0;
	if (DCM_BRAM_PATTERN_LINE_COUNT < uBRAMLineCount)
	{
		nRetVal = -3;
	}
	else if (DCM_DRAM_PATTERN_LINE_COUNT < uDRAMLineCount)
	{
		nRetVal = -4;
	}
	if (0 != nRetVal)
	{
		m_vecSwitch.clear();
	}
	return nRetVal;
}

int CMutualTest::GetLineInfo(UINT uGlobalLineIndex, BOOL* pbSRAM, BOOL* pbNextLineOtherMemory)
{
	int nSwitchIndex = 0;
	for (auto& SwitchInfo : m_vecSwitch)
	{
		if (uGlobalLineIndex >= SwitchInfo.m_uGlobalLineIndex && uGlobalLineIndex < SwitchInfo.m_uGlobalLineIndex + SwitchInfo.m_uSwitchLineCount)
		{
			if (uGlobalLineIndex == SwitchInfo.m_uGlobalLineIndex)
			{
				if (nullptr != pbSRAM)
				{
					*pbSRAM = SwitchInfo.m_bSwitchOut;
				}
				if (nullptr != pbNextLineOtherMemory)
				{
					*pbNextLineOtherMemory = TRUE;
				}
				UINT uCurOffset = uGlobalLineIndex;
				if (0 != nSwitchIndex)
				{
					uCurOffset = m_vecSwitch[nSwitchIndex - 1].m_uSwitchLineNo + m_vecSwitch[nSwitchIndex - 1].m_uSwitchLineNo - 1;
				}
				return uCurOffset;
			}
			else
			{
				if (nullptr != pbSRAM)
				{
					*pbSRAM = !SwitchInfo.m_bSwitchOut;
				}
				if (nullptr != pbNextLineOtherMemory)
				{
					*pbNextLineOtherMemory = FALSE;
				}
				return uGlobalLineIndex - SwitchInfo.m_uGlobalLineIndex + SwitchInfo.m_uSwitchLineNo - 1;
			}
		}
		else if (uGlobalLineIndex < SwitchInfo.m_uGlobalLineIndex)
		{
			///<First block in SRAM
			if (nullptr != pbSRAM)
			{
				*pbSRAM = TRUE;
			}
			return uGlobalLineIndex;
		}
		++nSwitchIndex;
	}
	if (0 == m_vecSwitch.size())
	{
		if (nullptr != pbSRAM)
		{
			*pbSRAM = TRUE;
		}
		if (nullptr != pbNextLineOtherMemory)
		{
			*pbNextLineOtherMemory = FALSE;
		}
		return uGlobalLineIndex;
	}
	return -1;
}

void CMutualTest::SetChannelStatus(UINT uControllerID, CHANNEL_OUTPUT_STATUS ChannelStatus)
{
	CHardwareFunction* pHardware = GetHardware(uControllerID);
	vector<USHORT> vecChannel;
	for (USHORT usChannel = 0; usChannel < DCM_CHANNELS_PER_CONTROL;++usChannel)
	{
		vecChannel.push_back(usChannel);
	}

	pHardware->SetChannelStatus(vecChannel, ChannelStatus);
}

void CMutualTest::LoadVector()
{
	DownloadPattern();
	//Set Timeset
	vector<USHORT> vecChannel;
	for (USHORT usChannel = 0; usChannel < HDModule::ChannelCountPerControl; ++usChannel)
	{
		vecChannel.push_back(usChannel);
	}
	BYTE bySlotNo = 0;
	USHORT uChannel = 0;
	double dVIH = 0;
	double dVIL = 0;
	double dVOH = 0;
	double dVOL = 0;
	map<BYTE, TIMESET_VALUE> mapTimeset;
	
	for (auto& RunInfo : m_mapRunInfo)
	{
		GetTimesetSetting(RunInfo.first, mapTimeset);
		CHardwareFunction* pHardware = GetHardware(RunInfo.first);
		GetPinLevel(RunInfo.first, &dVIH, &dVIL, &dVOH, &dVOL);
		pHardware->SetPinLevel(vecChannel, dVIH, dVIL, (dVIH + dVIL) / 2, dVOH, dVOL);
		for (auto& Timeset : mapTimeset)
		{
			pHardware->SetPeriod(Timeset.first, Timeset.second.m_dPeriod);
			pHardware->SetEdge(vecChannel, Timeset.first, Timeset.second.m_dEgde, Timeset.second.m_WaveFormat, IO_FORMAT::NRZ, COMPARE_MODE::EDGE);
		}
	}
}

void CMutualTest::Bind(const std::vector<UINT>& vecController, UINT uTargetControllerID)
{
	BYTE byBoardControllerIndex = 0;
	BYTE byTargetSlot = HDModule::Instance()->ID2Board(uTargetControllerID, byBoardControllerIndex);
	set<BYTE> setController;
	set<BYTE> setBoard;
	BYTE bySlot = 0;

	for (auto uControllerID : vecController)
	{
		bySlot = HDModule::Instance()->ID2Board(uControllerID, byBoardControllerIndex);
		if (setBoard.end() == setBoard.find(bySlot))
		{
			setBoard.insert(bySlot);
		}
		if (setController.end() == setController.find(byBoardControllerIndex))
		{
			setController.insert(byBoardControllerIndex);
		}
	}

	CBindInfo::Instance()->Bind(setBoard, setController, byTargetSlot);

	m_uBindControllerID = uTargetControllerID;
}

int CMutualTest::ClearBind()
{
	auto iterHardware = m_mapHardware.find(m_uBindControllerID);
	if (m_mapHardware.end() == iterHardware)
	{
		return -1;
	}
	CBindInfo::Instance()->ClearBind();
	return 0;
}

template<typename Key, typename Value>
inline void CMutualTest::ClearMap(std::map<Key, Value*>& mapData)
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

int CMutualTest::CheckLineOrder(UINT uControllerID, const std::vector<UINT>& vecLineOrder)
{
	return -2;
}

int CMutualTest::GetCheckDataType(UINT uControllerID)
{
	return 0;
}
