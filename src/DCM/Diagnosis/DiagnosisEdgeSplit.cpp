#include "DiagnosisEdgeSplit.h"
#include "..\HDModule.h"
using namespace std;
#ifdef _DEBUG
#include "..\Pattern.h"
#endif // _DEBUG

//#define _TEST_T1R 1

#define STBR_LOW_RATE 512
#define T1R_LOW_RATE 544
#define	HIGH_RATE 16


CDiagnosisEdgeSplit::CDiagnosisEdgeSplit()
{
	m_bTestSTBR = TRUE;
	m_bTestPass = TRUE;
	m_uCompareLineCount = 0;
	m_uDriverLineCount = 0;
}


CDiagnosisEdgeSplit::~CDiagnosisEdgeSplit()
{
	m_mapTestInfo.clear();
	m_mapTestResult.clear();
}

const char* CDiagnosisEdgeSplit::GetSubItemName() const
{
	if (m_bTestSTBR)
	{
		return "STBR";
	}
	else
	{
		return "T1R/T1F";
	}
}

int CDiagnosisEdgeSplit::SetEnabled(int enable)
{
	return CMutualTest::SetEnabled(enable);
}

int CDiagnosisEdgeSplit::IsEnabled() const
{
	return CMutualTest::IsEnabled();
}

int CDiagnosisEdgeSplit::QueryInstance(const char* name, void** ptr)
{
	return -1;
}

void CDiagnosisEdgeSplit::Release()
{

}

const char* CDiagnosisEdgeSplit::Name() const
{
	return "Edge Split Diagnosis";
}

int CDiagnosisEdgeSplit::GetChildren(STSVector<IHDDoctorItem*>& children) const
{
	return 0;
}

int CDiagnosisEdgeSplit::Doctor(IHDReportDevice* pReportDevice)
{
	m_pReportDevice = pReportDevice;
	StartTimer();
	m_bTestSTBR = TRUE;
	m_bTestPass = TRUE;
	m_mapTestResult.clear();
	double dTimeConsume = 0;
	char lpszTimeUnits[4] = { 0 };
	const char* lpszBaseIndent = IndentFormat();
	string strFirstIndent = lpszBaseIndent + IndentChar();
	const char* lpszNextIndent = strFirstIndent.c_str();
	m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<EdgeSplit>\n", lpszBaseIndent);
	int nRetVal = CMutualTest::Doctor(m_pReportDevice);
	if (0 == nRetVal)
	{
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='true'/>\n", lpszNextIndent);
	}
	else
	{
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='true'/>\n", lpszNextIndent);
	}

	dTimeConsume = StopTimer(lpszTimeUnits, sizeof(lpszTimeUnits));
	m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<Time value='%.3f%s'/>\n", lpszNextIndent, dTimeConsume, lpszTimeUnits);

	m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s</EdgeSplit>\n", lpszBaseIndent);

	return nRetVal;
}

bool CDiagnosisEdgeSplit::IsUserCheck() const
{
	switch (m_UserRole)
	{
	case CDiagnosisItem::PROCUCTION:
		return true;
		break;
	case CDiagnosisItem::DEVELOPER:
		return true;
		break;
	case CDiagnosisItem::USER:
		return false;
		break;
	default:
		return false;
		break;
	}
	return false;
}


int CDiagnosisEdgeSplit::GetTestController(int nTestIndex, std::vector<UINT>& vecTestController, BOOL bSaveLog)
{
	vecTestController.clear();
	double dTimeConsume = 0;
	char lpszTimeUnits[4] = { 0 };

	m_mapTestInfo.clear();
	int nTestControllerCount = m_vecEnableController.size();
	string strBaseIndent = IndentFormat() + IndentChar();
	const char* lpszBaseIndent = strBaseIndent.c_str();
	BOOL nRetVal = 0;

	const int nControllerTestTimes = 2;
	int nTestItemCount = nControllerTestTimes * 2;
	
#ifdef _TEST_T1R

	nTestIndex += 2;

#endif // _TEST_T1R

	BOOL bFirstT1REdge = FALSE;
	for (int nIndex = nTestIndex;nIndex < nTestItemCount;++nIndex)
	{
		if (0 == nIndex % nControllerTestTimes)
		{
			SaveResult();
			if (0 == nIndex / nControllerTestTimes)
			{
				m_bTestSTBR = TRUE;
			}
			else
			{
				m_bTestSTBR = FALSE;
			}
			m_uDriverLineCount = GetVectorLineInfo(-1, TRUE);
			m_uCompareLineCount = GetVectorLineInfo(-1, FALSE);
			if (!m_bTestSTBR)
			{
				GetVectorSwitchInfo(m_uCompareLineCount);
			}
			StartTimer();
			if (m_bTestSTBR)
			{
				m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<STBR>\n", lpszBaseIndent);
			}
			else
			{
				m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<T1R_T1F>\n", lpszBaseIndent);
				bFirstT1REdge = TRUE;
			}
		}

		int nStartControllerIndex = 0;
		BYTE byDriveControllerIndex = 0;
		switch (nIndex % 2)
		{
		case 0:
			byDriveControllerIndex = 0;
			nStartControllerIndex = 0;
			break;
		case 1:
			byDriveControllerIndex = 1;
			nStartControllerIndex = 0;
			break;
		default:
			byDriveControllerIndex = 0;
			nStartControllerIndex = 0;
			break;
		}

		for (USHORT usControllerIndex = nStartControllerIndex; usControllerIndex < nTestControllerCount; usControllerIndex++)
		{
			TEST_INFO TestInfo;
			if (byDriveControllerIndex == usControllerIndex % 2)
			{
				TestInfo.m_bDriveMode = TRUE;
				if (0 == byDriveControllerIndex)
				{
					TestInfo.m_uRelateControllerID = m_vecEnableController[usControllerIndex + 1];
				}
				else
				{
					TestInfo.m_uRelateControllerID = m_vecEnableController[usControllerIndex - 1];
				}
			}
			else
			{
				TestInfo.m_bDriveMode = FALSE;
				if (0 == byDriveControllerIndex)
				{
					TestInfo.m_uRelateControllerID = m_vecEnableController[usControllerIndex - 1];
				}
				else
				{
					TestInfo.m_uRelateControllerID = m_vecEnableController[usControllerIndex + 1];
				}
			}

			vecTestController.push_back(m_vecEnableController[usControllerIndex]);
			m_mapTestInfo.insert(make_pair(m_vecEnableController[usControllerIndex], TestInfo));
		}
		if (bFirstT1REdge)
		{
			///<Set the channel status of driver controller to low, in order to ensure the first pattern is start from low,
			///<because the wave format of driver controller is RZ and its wave status before T1R will follow the channel status before start
			auto iterTestInfo = m_mapTestInfo.begin();
			while (m_mapTestInfo.end() != iterTestInfo)
			{
				if (iterTestInfo->second.m_bDriveMode)
				{
					SetChannelStatus(iterTestInfo->first, CHANNEL_OUTPUT_STATUS::LOW);
				}
				++iterTestInfo;
			}
		}
		return 0;
	}

	SaveResult();

	return -1;
}

int CDiagnosisEdgeSplit::GetMutualTestVector(UINT uControllerID, int nLineIndex, char* lpszVector, int nVectorBuffSize, char* lpszCMD, int nCMDBuffSize, BYTE* pbyTimeset, ULONG* pulOperand, BOOL* pbSRAMLine, BOOL* pbNextLineOtherMemory)
{
	auto iterController = m_mapTestInfo.find(uControllerID);
	if (m_mapTestInfo.end() == iterController)
	{
		return -1;
	}
	if (HDModule::ChannelCountPerControl >= nVectorBuffSize)
	{
		return -2;
	}
	if (nullptr != pbyTimeset)
	{
		*pbyTimeset = 0;
	}
	if (nullptr != pulOperand)
	{
		*pulOperand = 0;
	}
	if (nullptr != lpszCMD)
	{
		strcpy_s(lpszCMD, nCMDBuffSize, "INC");
	}
	*pulOperand = 0;
	if (0 != GetVectorLineInfo(nLineIndex, iterController->second.m_bDriveMode, pbSRAMLine, lpszVector, nVectorBuffSize,pbyTimeset, pbNextLineOtherMemory))
	{
		return 1;
	}

	BOOL bFirstLine = FALSE;
	if (0 == nLineIndex)
	{
		bFirstLine = TRUE;
	}
#ifdef GET_VECTOR
	SaveVectorLine(lpszVector, *pbyTimeset, iterController->second.m_bDriveMode, bFirstLine);
#endif // GET_VECTOR

	return 0;
}

void CDiagnosisEdgeSplit::GetCheckDataController(std::vector<UINT>& vecCheckController)
{
	vecCheckController.clear();
	for (auto& TestInfo : m_mapTestInfo)
	{
		if (!TestInfo.second.m_bDriveMode)
		{
			vecCheckController.push_back(TestInfo.first);
		}
	}
}

int CDiagnosisEdgeSplit::GetTimesetSetting(UINT uControllerID, std::map<BYTE, TIMESET_VALUE>& mapEdgeValue)
{
	mapEdgeValue.clear();
	auto iterController = m_mapTestInfo.find(uControllerID);
	if (m_mapTestInfo.end() == iterController)
	{
		return -1;
	}
	TIMESET_VALUE EdgeValue;
	BOOL bFinish = FALSE;
	for (BYTE byTimesetIndex = 0; byTimesetIndex < HDModule::SplitCount; ++byTimesetIndex)
	{
		if (m_bTestSTBR)
		{
			if (iterController->second.m_bDriveMode)
			{
				EdgeValue.m_dPeriod = HIGH_RATE;
				EdgeValue.m_dEgde[0] = 0;
				EdgeValue.m_dEgde[1] = 5;

				EdgeValue.m_dEgde[2] = EdgeValue.m_dEgde[0];
				EdgeValue.m_dEgde[3] = EdgeValue.m_dEgde[1];
				EdgeValue.m_dEgde[4] = 10;
				EdgeValue.m_dEgde[5] = 12;
				EdgeValue.m_WaveFormat = WAVE_FORMAT::NRZ;

				bFinish = TRUE;
			}
			else
			{
				EdgeValue.m_dPeriod = STBR_LOW_RATE;
				EdgeValue.m_dEgde[0] = 0;
				EdgeValue.m_dEgde[1] = 2 > byTimesetIndex ? 5 : 200;
				EdgeValue.m_dEgde[2] = EdgeValue.m_dEgde[0];
				EdgeValue.m_dEgde[3] = EdgeValue.m_dEgde[1];
				EdgeValue.m_dEgde[4] = byTimesetIndex * HIGH_RATE + 8;
				EdgeValue.m_dEgde[5] = EdgeValue.m_dEgde[4] + 8;
				EdgeValue.m_WaveFormat = WAVE_FORMAT::NRZ;
			}
		}
		else
		{
			if (iterController->second.m_bDriveMode)
			{
				EdgeValue.m_dPeriod = T1R_LOW_RATE;

				EdgeValue.m_dEgde[0] = byTimesetIndex * HIGH_RATE + HIGH_RATE;
				EdgeValue.m_dEgde[1] = EdgeValue.m_dEgde[0] + HIGH_RATE;
				EdgeValue.m_dEgde[2] = EdgeValue.m_dEgde[0];
				EdgeValue.m_dEgde[3] = EdgeValue.m_dEgde[1];
				EdgeValue.m_dEgde[4] = 8;
				EdgeValue.m_dEgde[5] = 10;
				EdgeValue.m_WaveFormat = WAVE_FORMAT::RZ;
			}
			else
			{
				EdgeValue.m_dPeriod = HIGH_RATE;
				EdgeValue.m_dEgde[0] = 0;
				EdgeValue.m_dEgde[1] = 5;
				EdgeValue.m_dEgde[4] = 10;
				EdgeValue.m_dEgde[5] = EdgeValue.m_dEgde[4] + 2;

				EdgeValue.m_dEgde[2] = EdgeValue.m_dEgde[0];
				EdgeValue.m_dEgde[3] = EdgeValue.m_dEgde[1];
				EdgeValue.m_WaveFormat = WAVE_FORMAT::NRZ;
				bFinish = TRUE;
			}
		}
		mapEdgeValue.insert(make_pair(byTimesetIndex, EdgeValue));
		if (bFinish)
		{
			break;
		}

	}
	return 0;
}

int CDiagnosisEdgeSplit::GetPinLevel(UINT uControllerID, double* pdVIH, double* pdVIL, double* pdVOH, double* pdVOL)
{
	auto iterController = m_mapTestInfo.find(uControllerID);
	if (m_mapTestInfo.end() == iterController)
	{
		return -1;
	}
	if (nullptr == pdVIH || nullptr == pdVIL || nullptr == pdVOH || nullptr == pdVOL)
	{
		return -2;
	}
	*pdVIH = 3;
	*pdVIL = 0;
	*pdVOH = 2;
	*pdVOL = 1;
	return 0;
}

void CDiagnosisEdgeSplit::GetVectorStartLine(UINT* puBRAMStartLine, UINT* puDRAMStartLine)
{
	if (nullptr == puBRAMStartLine || nullptr == puDRAMStartLine)
	{
		return;
	}
	*puBRAMStartLine = 0;
	*puDRAMStartLine = 0;
}

BOOL CDiagnosisEdgeSplit::IsReloadVector()
{
	return TRUE;
}

int CDiagnosisEdgeSplit::GetSameVectorControllerType()
{
	return 2;
}

int CDiagnosisEdgeSplit::GetSameVectorController(int nTypeIndex, std::vector<UINT>& vecSameController)
{
	vecSameController.clear();
	BOOL bGetDriveMode = FALSE;
	switch (nTypeIndex)
	{
	case 0:
		bGetDriveMode = TRUE;
		break;
	case 1:
		bGetDriveMode = FALSE;
		break;
	default:
		return -1;
		break;
	}

	auto iterController = m_mapTestInfo.begin();
	while (m_mapTestInfo.end() != iterController)
	{
		if (bGetDriveMode == iterController->second.m_bDriveMode)
		{
			vecSameController.push_back(iterController->first);
		}
		++iterController;
	}
	return 0;
}

int CDiagnosisEdgeSplit::GetVectorLineInfo(int nLineIndex, BOOL bDriveMode, BOOL* pbBRAM, char* lpszPattern, int nBuffSize, BYTE* pbyTimeset, BOOL* pbSwitch)
{
	int nDriveLineCount = 0;
	int nCompareLineCount = 0;
	BYTE byLineTimesetIndex = 0;
	
	int nDriverSwitchOut = m_uDriverLineCount % 2 + 1;
	int nDriverSwitchIn = m_uDriverLineCount  - 3;

	int nCountPerTimeset = 0;
	if (m_bTestSTBR)
	{
		return GetSTBRPatternInfo(nLineIndex, bDriveMode, pbBRAM, lpszPattern, nBuffSize, pbyTimeset, pbSwitch);
	}
	else
	{
		return GetT1RPatternInfo(nLineIndex, bDriveMode, pbBRAM, lpszPattern, nBuffSize, pbyTimeset, pbSwitch);		
	}
}

#include <fstream>
int CDiagnosisEdgeSplit::GetSTBRPatternInfo(int nLineIndex, BOOL bDriveMode, BOOL* pbBRAM, char* lpszPattern, int nBuffSize, BYTE* pbyTimeset, BOOL* pbSwitch)
{
	int nDriveLineCount = 0;
	int nCompareLineCount = 0;
	BYTE byLineTimesetIndex = 0;

	int nDriverSwitchOut = m_uDriverLineCount % 2 + 1;
	int nDriverSwitchIn = m_uDriverLineCount - 3;

	int nDriverCountPerTimeset = STBR_LOW_RATE / HIGH_RATE;
	if (bDriveMode && nullptr != pbSwitch && nullptr != pbBRAM)
	{
		*pbBRAM = TRUE;
		if (nDriverSwitchOut < nLineIndex && nDriverSwitchIn >= nLineIndex)
		{
			*pbBRAM = FALSE;
		}
		*pbSwitch = FALSE;
		if (nDriverSwitchOut == nLineIndex || nDriverSwitchIn == nLineIndex)
		{
			*pbSwitch = TRUE;
		}
	}

	for (BYTE byTimesetIndex = 0; byTimesetIndex < HDModule::SplitCount; ++byTimesetIndex)
	{
		for (BYTE byCurTimesetIndex = 0; byCurTimesetIndex < HDModule::SplitCount; ++byCurTimesetIndex)
		{
			if (byTimesetIndex == byCurTimesetIndex)
			{
				continue;
			}

			for (int nIndex = 0; nIndex < 2; ++nIndex)
			{
				if (0 == nIndex)
				{
					byLineTimesetIndex = byCurTimesetIndex;
				}
				else
				{
					byLineTimesetIndex = byTimesetIndex;
				}
				if (!bDriveMode)
				{
					if (nCompareLineCount == nLineIndex)
					{
						if (nullptr != pbyTimeset)
						{
							*pbyTimeset = byLineTimesetIndex;
						}
						if (nullptr != lpszPattern)
						{
							strcpy_s(lpszPattern, nBuffSize, "LLLLLLLLLLLLLLLL");
						}
						if (0 != byLineTimesetIndex % 2 && m_mapBRAMFailLineNo.end() == m_mapBRAMFailLineNo.find(nCompareLineCount))
						{
							m_mapBRAMFailLineNo.insert(make_pair(nCompareLineCount, 0xFFFF));
							m_mapBRAMTimeset.insert(make_pair(nCompareLineCount, byLineTimesetIndex));
						}
						return 0;
					}
					++nCompareLineCount;
				}
				else
				{
					for (int nDriveIndex = 0; nDriveIndex < nDriverCountPerTimeset; ++nDriveIndex)
					{
						if (nDriveLineCount == nLineIndex)
						{
							if (nullptr != pbyTimeset)
							{
								*pbyTimeset = 0;
							}
							if (nullptr != lpszPattern)
							{
								if (1 == nDriveLineCount % 2)
								{
									strcpy_s(lpszPattern, nBuffSize, "1111111111111111");
								}
								else
								{
									strcpy_s(lpszPattern, nBuffSize, "0000000000000000");
								}
							}
							return 0;
						}
						++nDriveLineCount;
					}
				}
			}
		}
	}
	if (-1 == nLineIndex)
	{
		if (bDriveMode)
		{
			return nDriveLineCount;
		}
		return nCompareLineCount;
	}
	return -1;
}
int CDiagnosisEdgeSplit::GetT1RPatternInfo(int nLineIndex, BOOL bDriveMode, BOOL* pbSRAM, char* lpszPattern, int nBuffSize, BYTE* pbyTimeset, BOOL* pbSwitch)
{
	int nDriveLineCount = 0;
	int nCompareLineCount = 0;
	BYTE byLineTimesetIndex = 0;

	int nCountPerTimeset = T1R_LOW_RATE / HIGH_RATE;

	for (BYTE byTimesetIndex = 0; byTimesetIndex < HDModule::SplitCount; ++byTimesetIndex)
	{
		for (BYTE byCurTimesetIndex = 0; byCurTimesetIndex < HDModule::SplitCount; ++byCurTimesetIndex)
		{
			if (byTimesetIndex == byCurTimesetIndex)
			{
				continue;
			}
			for (int nIndex = 0; nIndex < 2; ++nIndex)
			{
				if (0 == nIndex)
				{
					byLineTimesetIndex = byCurTimesetIndex;
				}
				else
				{
					byLineTimesetIndex = byTimesetIndex;
				}

				if (bDriveMode)
				{
					if (nullptr != pbSRAM)
					{
						*pbSRAM = TRUE;
					}
					if (nDriveLineCount == nLineIndex)
					{
						if (nullptr != pbyTimeset)
						{
							*pbyTimeset = byLineTimesetIndex;
						}
						if (nullptr != lpszPattern)
						{
							strcpy_s(lpszPattern, nBuffSize, "1111111111111111");
						}
						return 0;
					}
					++nDriveLineCount;
				}
				else
				{
					for (int nCompareIndex = 0; nCompareIndex < nCountPerTimeset; ++nCompareIndex)
					{
						int nCurLineIndex = nLineIndex;
						if (-1 == nCurLineIndex)
						{
							nCurLineIndex = nCompareLineCount;
						}
						if (nCompareLineCount == nCurLineIndex)
						{
							if (nullptr != pbyTimeset)
							{
								*pbyTimeset = 0;
							}
							GetLineInfo(nCurLineIndex, pbSRAM, pbSwitch);

							if (nullptr != lpszPattern)
							{
								strcpy_s(lpszPattern, nBuffSize, "LLLLLLLLLLLLLLLL");
							}

							if (byLineTimesetIndex + 1 == nCompareIndex && m_mapFailLineNo.end() == m_mapFailLineNo.find(nCurLineIndex))
							{
								m_mapTimeset.insert(make_pair(nCurLineIndex, byLineTimesetIndex));
								m_mapFailLineNo.insert(make_pair(nCurLineIndex, 0xFFFF));
							}

							if (-1 != nLineIndex)
							{
								return 0;
							}
						}
						++nCompareLineCount;
					}
				}
			}
		}
	}
	if (-1 == nLineIndex)
	{
		if (bDriveMode)
		{
			return nDriveLineCount;
		}
		return nCompareLineCount;
	}
	return -1;
}
int CDiagnosisEdgeSplit::CheckResult(UINT uControllerID, const std::map<int, USHORT>& mapBRAMFailLineNo, const std::map<int, USHORT>& mapDRAMFailLineNo, int nFailCount)
{
	auto iterController = m_mapTestInfo.find(uControllerID);
	if (m_mapTestInfo.end() == iterController)
	{
		return -2;
	}
	if (iterController->second.m_bDriveMode)
	{
		return -3;
	}

	StartTimer();
	double dTimeConsume = 0;
	char lpszTimeUnits[4] = { 0 };

	string strFirstIndent = IndentFormat() + IndentChar() + IndentChar();
	string strSecondIndent = strFirstIndent + IndentChar();
	const char* lpszFirstIndent = strFirstIndent.c_str();
	const char* lpszSecondIndent = strSecondIndent.c_str();

	UINT uTestControllerID = uControllerID;
	if (!m_bTestSTBR)
	{
		uTestControllerID = iterController->second.m_uRelateControllerID;
	}
	int nRetVal = 0;
	BYTE bySlotNo = 0;
	BYTE byBoardControllerIndex = 0;
	bySlotNo = HDModule::Instance()->ID2Board(uTestControllerID, byBoardControllerIndex);
	if (nullptr != m_pReportDevice)
	{
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<controller value='%d, slot value = %d'>\n", lpszFirstIndent, byBoardControllerIndex, bySlotNo);
	}

	int nFailItemCount = 0;
	if (mapBRAMFailLineNo != m_mapBRAMFailLineNo)
	{
		auto iterTimest = m_mapBRAMTimeset.begin();
		auto iterRealFailLineNo = mapBRAMFailLineNo.begin();
		auto iterExpectFailLineNo = m_mapBRAMFailLineNo.begin();
		while (mapBRAMFailLineNo.end() != iterRealFailLineNo && m_mapBRAMFailLineNo.end() != iterExpectFailLineNo)
		{
			if (iterRealFailLineNo->first != iterExpectFailLineNo->first || iterRealFailLineNo->second != iterExpectFailLineNo->second)
			{
				nRetVal = -1;
				char lpszValue[128] = { 0 };
				sprintf_s(lpszValue, sizeof(lpszValue), "value='timeset=%d expect addr=0x%X Real addr=0x%X Expect=0x%X Real=0x%X'", iterTimest->second, iterExpectFailLineNo->first, iterRealFailLineNo->first, iterExpectFailLineNo->second, iterRealFailLineNo->second);

				m_pReportDevice->PrintfToDataLog(IHDReportDevice::Error, "%s<data retValue='false' %s/>\n", lpszSecondIndent, lpszValue);

				if (ERROR_PRINT < nFailCount++)
				{
					break;
				}
			}
			++iterTimest;
			++iterRealFailLineNo;
			++iterExpectFailLineNo;
		}
		if (mapBRAMFailLineNo.size() != m_mapBRAMFailLineNo.size())
		{
			nRetVal = -1;
			char lpszValue[128] = { 0 };
			nFailCount = 0;
			while (mapBRAMFailLineNo.end() != iterRealFailLineNo)
			{
				sprintf_s(lpszValue, sizeof(lpszValue), "value='expect addr= - Real addr=0x%X Expect= - Real=0x%X count=%d'",
					iterRealFailLineNo->first, iterRealFailLineNo->second, mapBRAMFailLineNo.size());
				++iterRealFailLineNo;
				if (ERROR_PRINT < nFailCount++)
				{
					break;
				}
			}
			nFailCount = 0;
			while (m_mapBRAMFailLineNo.end() != iterExpectFailLineNo)
			{
				sprintf_s(lpszValue, sizeof(lpszValue), "value=' expect addr=0x%X Real addr= - Expect=0x%X Real= - count=%d'",
					iterExpectFailLineNo->first, iterExpectFailLineNo->second, mapBRAMFailLineNo.size());
				++iterExpectFailLineNo;
				if (ERROR_PRINT < nFailCount++)
				{
					break;
				}
			}

			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Error, "%s<data retValue='false' %s/>\n", lpszSecondIndent, lpszValue);
		}
	}
	if (0 != mapDRAMFailLineNo.size())
	{
		nRetVal = -1;
		char lpszValue[128] = { 0 };
		sprintf_s(lpszValue, sizeof(lpszValue), "value='DRAMCaptureCount Expect=0x%X Real=0x%X'", 0, mapDRAMFailLineNo.size());

		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Error, "%s<data retValue='false' %s/>\n", lpszSecondIndent, lpszValue);
	}
	if (mapBRAMFailLineNo.size() != nFailCount)
	{
		nRetVal = -1;
		char lpszValue[128] = { 0 };
		sprintf_s(lpszValue, sizeof(lpszValue), "value='FailCount Expect=%d Real=%d'", nFailCount, mapBRAMFailLineNo.size());

		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Error, "%s<data retValue='false' %s/>\n", lpszSecondIndent, lpszValue);
	}

	//Ensure only print one information per controller.
	if (nullptr != m_pReportDevice)
	{
		auto iterController = m_mapTestResult.find(uTestControllerID);
		if (m_mapTestResult.end() == iterController)
		{
			if (0 != nRetVal)
			{
				//Test fail
				m_pReportDevice->PrintfToUi(IHDReportDevice::Fail);
				m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='false'/>\n", lpszSecondIndent);
			}
			else
			{
				m_pReportDevice->PrintfToUi(IHDReportDevice::Pass);
				m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='true'/>\n", lpszSecondIndent);
			}
			m_pReportDevice->PrintfToUi("\t Slot %d, Controller %d\n", bySlotNo, byBoardControllerIndex);
			m_mapTestResult.insert(make_pair(uTestControllerID, nRetVal));
		}
	}

	if (nullptr != m_pReportDevice)
	{
		dTimeConsume = StopTimer(lpszTimeUnits, sizeof(lpszTimeUnits));
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<Time value='%.3f%s'/>\n", lpszSecondIndent, dTimeConsume, lpszTimeUnits);
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s</controller>\n", lpszFirstIndent);
	}

	return nRetVal;
}

void CDiagnosisEdgeSplit::SaveResult()
{
	if (0 == m_mapTestResult.size())
	{
		return;
	}
	string strFirstIndent = IndentFormat() + IndentChar();
	string strSecondIndent = strFirstIndent + IndentChar();
	const char* lpszFirstIndent = strFirstIndent.c_str();
	const char* lpszSecondIndent = strSecondIndent.c_str();

	double dTimeConsume = 0;
	char lpszTimeUnits[8] = { 0 };

	BOOL bAllPass = TRUE;
	for (auto& TestResult : m_mapTestResult)
	{
		if (0 != TestResult.second)
		{
			m_bTestPass = FALSE;
			break;
		}
	}
	if (m_bTestPass)
	{
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='true'/>\n", lpszSecondIndent);
	}
	else
	{
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='false'/>\n", lpszSecondIndent);
	}

	dTimeConsume = StopTimer(lpszTimeUnits, sizeof(lpszTimeUnits));
	m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<Time value='%.3f%s'/>\n", lpszSecondIndent, dTimeConsume, lpszTimeUnits);
	if (m_bTestSTBR)
	{
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s</STBR>\n", lpszFirstIndent);
	}
	else
	{
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s</T1R_T1F>\n", lpszFirstIndent);
	}
	m_bTestPass = TRUE;
	m_mapTestResult.clear();
}

BOOL CDiagnosisEdgeSplit::Stop()
{
	return FALSE;
}