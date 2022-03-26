#include "DiagnosisRateSplit.h"
#include "..\HDModule.h"
#include "IHDReportDevice.h"
using namespace std;
#define TEST_CHANNEL 4

#define TIMESET_PER_TEST_COUNT 8

#ifdef _DEBUG
#include "..\Pattern.h"
#endif // _DEBUG


//#define GET_VECTOR 1

CDiagnosisRateSplit::CDiagnosisRateSplit()
{
	m_nTimesetStart = 0;
	m_nCompareLineCount = 0;
	m_nDriveLineCount = 0;
}


CDiagnosisRateSplit::~CDiagnosisRateSplit()
{
	m_mapTestInfo.clear();
	m_mapTestResult.clear();
}

const char* CDiagnosisRateSplit::GetSubItemName() const
{
	if (0 == m_nTimesetStart)
	{
		return "0-7";
	}
	else if (8 == m_nTimesetStart)
	{
		return "8-15";
	}
	else if (16 == m_nTimesetStart)
	{
		return "16-23";
	}
	else
	{
		return "24-31";
	}
}

int CDiagnosisRateSplit::SetEnabled(int enable)
{
	return CMutualTest::SetEnabled(enable);
}

int CDiagnosisRateSplit::IsEnabled() const
{
	return CMutualTest::IsEnabled();
}

int CDiagnosisRateSplit::QueryInstance(const char* name, void** ptr)
{
	return 0;
}

void CDiagnosisRateSplit::Release()
{
}

const char* CDiagnosisRateSplit::Name() const
{
	return "Rate Split Diagnosis";
}

int CDiagnosisRateSplit::GetChildren(STSVector<IHDDoctorItem*>& children) const
{
	return 0;
}

int CDiagnosisRateSplit::Doctor(IHDReportDevice* pReportDevice)
{
	m_pReportDevice = pReportDevice;
	StartTimer();

	m_mapTestResult.clear();

	double dTimeConsume = 0;
	char lpszTimeUnits[4] = { 0 };
	string strNextIndent = IndentFormat() + IndentChar();
	const char* lpszNextIndent = strNextIndent.c_str();
	m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<RateSplit>\n", IndentFormat());
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
	pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s</RateSplit>\n", IndentFormat());
	return nRetVal;
}

bool CDiagnosisRateSplit::IsUserCheck() const
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

int CDiagnosisRateSplit::GetTestController(int nTestIndex, std::vector<UINT>& vecTestController, BOOL bSaveLog)
{
	vecTestController.clear();
	m_mapTestInfo.clear();
	const int nControllerTestTimes = 2;
	int nTestItemCount = nControllerTestTimes * ((HDModule::SplitCount + TIMESET_PER_TEST_COUNT - 1) / TIMESET_PER_TEST_COUNT);

	int nTestControllerCount = m_vecEnableController.size();

	if (nTestItemCount <= nTestIndex)
	{
		SaveResult();
		return -1;
	}
	string strBaseIndent = IndentFormat() + IndentChar();
	const char* lpszBaseIndnet = strBaseIndent.c_str();
	int nCurTestIndex = 0;

	int nStartControllerIndex = 0;
	BYTE byDriveControllerIndex = 0;
	for (int nIndex = nTestIndex; nIndex < nTestItemCount; ++nIndex)
	{
		if (0 == nTestIndex % nControllerTestTimes)
		{
			SaveResult();
			m_nTimesetStart = nIndex / 2 * (TIMESET_PER_TEST_COUNT);

			m_mapBRAMFailLineNo.clear();
			m_mapTimeset.clear();
			GetVectorLineInfo(-1);
			StartTimer();
			if (0 == m_nTimesetStart)
			{
				m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<FirstEight>\n", lpszBaseIndnet);
			}
			else if (8 == m_nTimesetStart)
			{
				m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<SecondEight>\n", lpszBaseIndnet);
			}
			else if (16 == m_nTimesetStart)
			{
				m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<ThirdEight>\n", lpszBaseIndnet);
			}
			else
			{
				m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<LastEight>\n", lpszBaseIndnet);
			}
		}

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
			TestInfo.m_usChannel = TEST_CHANNEL;
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
		return 0;
	}
	SaveResult();
	return -1;
}

int CDiagnosisRateSplit::GetMutualTestVector(UINT uControllerID, int nLineIndex, char* lpszVector, int nVectorBuffSize, char* lpszCMD, int nCMDBuffSize, BYTE* pbyTimeset, ULONG* pulOperand, BOOL* pbBRAMLine, BOOL* pbNextLineOtherMemory)
{
	if (nullptr == pbyTimeset || nullptr == lpszVector || nullptr == pulOperand || nullptr == pbBRAMLine)
	{
		return -1;
	}
	else if (HDModule::ChannelCountPerControl >= nVectorBuffSize)
	{
		return -2;
	}
	else if (16 > nCMDBuffSize)
	{
		return -3;
	}
	auto iterControllerInfo = m_mapTestInfo.find(uControllerID);
	if (m_mapTestInfo.end() == iterControllerInfo)
	{
		return -3;
	}
	
	*pulOperand = 0;
	char cCurVector = '0';
	*pbBRAMLine = TRUE;
	*pbNextLineOtherMemory = FALSE;
	USHORT uSelectedChannel = iterControllerInfo->second.m_usChannel;
	strcpy_s(lpszVector, nVectorBuffSize, "XXXXXXXXXXXXXXXX");
	if (!iterControllerInfo->second.m_bDriveMode)
	{
		if (nLineIndex >= m_nCompareLineCount)
		{
			return 1;
		}
		lpszVector[uSelectedChannel] = 'L';
		*pbyTimeset = 0;
		return 0;
	}

	if (nLineIndex >= m_nDriveLineCount)
	{
		return 1;
	}

	lpszVector[uSelectedChannel] = nLineIndex % 2 + '0';
	

	GetVectorLineInfo(nLineIndex, TRUE, pbyTimeset);
#ifdef GET_VECTOR
	BOOL bFirstLine = FALSE;
	if (0 == nLineIndex)
	{
		bFirstLine = TRUE;
	}
	SaveVectorLine(lpszVector, *pbyTimeset, bFirstLine);
#endif // GET_VECTOR

	return 0;
}

int CDiagnosisRateSplit::CheckResult(UINT uControllerID, const std::map<int, USHORT>& mapBRAMFailLineNo, const std::map<int, USHORT>& mapDRAMFailLineNo, int nFailCount)
{
	auto iterControllerInfo = m_mapTestInfo.find(uControllerID);
	if (m_mapTestInfo.end() == iterControllerInfo)
	{
		return -2;
	}
	if (iterControllerInfo->second.m_bDriveMode)
	{
		return -3;
	}
	int nRetVal = 0;
	if (0 == m_mapBRAMFailLineNo.size())
	{
		///<Get the result
		UINT uLineIndex = 0;
		do 
		{
			nRetVal = GetVectorLineInfo(uLineIndex++, FALSE);
		} while (0 == nRetVal);
		nRetVal = 0;
	}

	nRetVal = 0;

	StartTimer();
	double dTimeConsume = 0;
	char lpszTimeUnits[4] = { 0 };
	UINT uDriveControllerID = iterControllerInfo->second.m_uRelateControllerID;
	BOOL bSaveDataLog = FALSE;
	string strFirstIndent = IndentFormat() + IndentChar() + IndentChar();
	string strSecondIndent = strFirstIndent + IndentChar();
	const char* lpszFirstIndent = strFirstIndent.c_str();
	const char* lpszSecondIndent = strSecondIndent.c_str();

	BYTE bySlotNo = 0;
	byte byBoardControllerIndex = 0;
	bySlotNo = HDModule::Instance()->ID2Board(uDriveControllerID, byBoardControllerIndex);

	if (nullptr != m_pReportDevice)
	{
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<controller value='%d, slot value = %d'>\n", lpszFirstIndent, byBoardControllerIndex, bySlotNo);
	}

	USHORT uChannel = iterControllerInfo->second.m_usChannel;

	int nFailItemCount = 0;
	if (mapBRAMFailLineNo != m_mapBRAMFailLineNo)
	{
		auto iterTimest = m_mapTimeset.begin();
		auto iterRealFailLineNo = mapBRAMFailLineNo.begin();
		auto iterExpectFailLineNo = m_mapBRAMFailLineNo.begin();
		while (mapBRAMFailLineNo.end() != iterRealFailLineNo && m_mapBRAMFailLineNo.end() != iterExpectFailLineNo)
		{
			if (iterRealFailLineNo->first != iterExpectFailLineNo->first || iterRealFailLineNo->second != iterExpectFailLineNo->second)
			{
				nRetVal = -1;
				char lpszValue[128] = { 0 };
				sprintf_s(lpszValue, sizeof(lpszValue), "value='timeset=%d expect addr=0x%X Real addr=0x%X Expect=0x%X Real=0x%X'",
					iterTimest->second, iterExpectFailLineNo->first, iterRealFailLineNo->first, iterExpectFailLineNo->second, iterRealFailLineNo->second);

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
				sprintf_s(lpszValue, sizeof(lpszValue), "value='expect addr='-' Real addr=0x%X Expect='-' Real=0x%X'", iterRealFailLineNo->first, iterRealFailLineNo->second);
				m_pReportDevice->PrintfToDataLog(IHDReportDevice::Error, "%s<data retValue='false' %s/>\n", lpszSecondIndent, lpszValue);
				++iterRealFailLineNo;
				if (ERROR_PRINT < nFailCount++)
				{
					break;
				}
			}
			nFailCount = 0;
			while (m_mapBRAMFailLineNo.end() != iterExpectFailLineNo)
			{
				sprintf_s(lpszValue, sizeof(lpszValue), "value='timeset=%d expect addr=0x%X Real addr='-' Expect=0x%X Real='-''",
					iterTimest->second, iterExpectFailLineNo->first, iterExpectFailLineNo->second);
				m_pReportDevice->PrintfToDataLog(IHDReportDevice::Error, "%s<data retValue='false' %s/>\n", lpszSecondIndent, lpszValue);
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

	//Ensure only print one information per controller.
	if (nullptr != m_pReportDevice)
	{
		auto iterPrint = m_mapTestResult.find(uDriveControllerID);
		if (m_mapTestResult.end() == iterPrint)
		{
			if (0 != nRetVal)
			{
				//Test fail
				m_pReportDevice->PrintfToUi(IHDReportDevice::Fail);
			}
			else
			{
				m_pReportDevice->PrintfToUi(IHDReportDevice::Pass);
			}
			bySlotNo = HDModule::Instance()->ID2Board(uDriveControllerID, byBoardControllerIndex);
			m_pReportDevice->PrintfToUi("\t Slot %d, Controller %d\n", bySlotNo, byBoardControllerIndex);
			m_mapTestResult.insert(make_pair(uDriveControllerID, nRetVal));
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


void CDiagnosisRateSplit::GetCheckDataController(std::vector<UINT>& vecCheckController)
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

#define _NEW 1
int CDiagnosisRateSplit::GetTimesetSetting(UINT uControllerID, map<BYTE, TIMESET_VALUE>& mapEdgeValue)
{
	mapEdgeValue.clear();
	auto iterController = m_mapTestInfo.find(uControllerID);
	if (m_mapTestInfo.end() == iterController)
	{
		return -1;
	}
#ifdef _NEW


	TIMESET_VALUE EdgeValue;
	EdgeValue.m_dPeriod = 16;
	EdgeValue.m_dEgde[0] = 0;
	EdgeValue.m_dEgde[1] = 10;
	EdgeValue.m_dEgde[2] = 0;
	EdgeValue.m_dEgde[3] = 10;
	EdgeValue.m_dEgde[4] = 8;
	EdgeValue.m_dEgde[5] = 13;
	EdgeValue.m_WaveFormat = WAVE_FORMAT::NRZ;
	if (iterController->second.m_bDriveMode)
	{
		for (BYTE byTimesetIndex = 0; byTimesetIndex < HDModule::SplitCount; ++byTimesetIndex)
		{
			EdgeValue.m_dPeriod = (byTimesetIndex % TIMESET_PER_TEST_COUNT) * 16 + 16;
			EdgeValue.m_dEgde[4] = 8;
			EdgeValue.m_dEgde[5] = 13;
			mapEdgeValue.insert(make_pair(byTimesetIndex, EdgeValue));
		}
	}
	else
	{
		mapEdgeValue.insert(make_pair(0, EdgeValue));
	}
#else
	TIMESET_VALUE EdgeValue;
	EdgeValue.m_dPeriod = 30;
	EdgeValue.m_dEgde[0] = 5;
	EdgeValue.m_dEgde[1] = 10;
	EdgeValue.m_dEgde[2] = 5;
	EdgeValue.m_dEgde[3] = 10;
	EdgeValue.m_dEgde[4] = 18;
	EdgeValue.m_dEgde[5] = 24;
	EdgeValue.m_WaveFormat = WAVE_FORMAT::NRZ;
	if (iterController->second.m_bDriveMode)
	{
		for (BYTE byTimesetIndex = 0; byTimesetIndex < HDModule::SplitCount; ++byTimesetIndex)
		{
			EdgeValue.m_dPeriod = byTimesetIndex * 30 + 30;
			EdgeValue.m_dEgde[4] = EdgeValue.m_dPeriod * 0.6;
			EdgeValue.m_dEgde[5] = EdgeValue.m_dPeriod * 0.8;
			mapEdgeValue.insert(make_pair(byTimesetIndex, EdgeValue));
		}
	}
	else
	{
		mapEdgeValue.insert(make_pair(0, EdgeValue));
	}
#endif // _NEW


	return 0;
}
int CDiagnosisRateSplit::GetPinLevel(UINT uControllerID, double* pdVIH, double* pdVIL, double* pdVOH, double* pdVOL)
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

void CDiagnosisRateSplit::GetVectorStartLine(UINT* puBRAMStartLine, UINT* puDRAMStartLine)
{
	if (nullptr == puBRAMStartLine ||nullptr == puDRAMStartLine)
	{
		return;
	}
	*puBRAMStartLine = 0;
	*puDRAMStartLine = 0;
}

int CDiagnosisRateSplit::GetSameVectorControllerType()
{
	return 2;
}

int CDiagnosisRateSplit::GetSameVectorController(int nTypeIndex, std::vector<UINT>& vecSameController)
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
	
	for (auto& TestInfo : m_mapTestInfo)
	{
		if (bGetDriveMode == TestInfo.second.m_bDriveMode)
		{
			vecSameController.push_back(TestInfo.first);
		}
	}
	return 0;
}

BOOL CDiagnosisRateSplit::IsReloadVector()
{
	return TRUE;
}


#include <fstream>
void CDiagnosisRateSplit::SaveVectorLine(const char* lpszVector, BYTE byTimeset, BOOL bFirstLine)
{
	if (nullptr == lpszVector)
	{
		return;
	}
	fstream VectorFile("D:\\VectorFile.csv", ios::app | ios::out);
	if (bFirstLine)
	{
		VectorFile << "Vector,Drive Timeset" << endl;
	}
	char lpszMsg[128] = { 0 };
	for (int nIndex = 0; nIndex <= byTimeset;++nIndex)
	{
		sprintf_s(lpszMsg, sizeof(lpszMsg), "%s,%d", lpszVector, byTimeset);
		VectorFile << lpszMsg << endl;
	}
	VectorFile.close();

}

int CDiagnosisRateSplit::GetVectorLineInfo(int nLineIndex, BOOL bDriveMode, BYTE* pbyDriveTimeset)
{
	USHORT usFailValue = 1 << TEST_CHANNEL;
	int nDriveLineCount = 0;
	int nCompareLineCount = 0;

	if (bDriveMode)
	{		
		if (-1 != nLineIndex && 0 < m_nDriveLineCount && nLineIndex >= m_nDriveLineCount)
		{
			return -1;
		}
	}
	else
	{
		if (-1 != nLineIndex && 0 < m_nCompareLineCount && nLineIndex >= m_nCompareLineCount)
		{
			return -1;
		}
	}

	int nStartTimeset = m_nTimesetStart;
	int nStopTimeset = m_nTimesetStart + TIMESET_PER_TEST_COUNT;

	for (int nSplitIndex = m_nTimesetStart; nSplitIndex < nStopTimeset; ++nSplitIndex)
	{
		for (int nCurLine = 0; nCurLine < 2; ++nCurLine)
		{
			if (bDriveMode && nDriveLineCount == nLineIndex)
			{
				if (nullptr != pbyDriveTimeset)
				{
					*pbyDriveTimeset = nSplitIndex;
					if (0 < m_nCompareLineCount)
					{
						return 0;
					}
				}
			}
			++nDriveLineCount;
			if (bDriveMode && 0 < m_nCompareLineCount)
			{
				continue;
			}
			for (int nCompareIndex = m_nTimesetStart; nCompareIndex < nSplitIndex + 1; ++nCompareIndex)
			{
				if (nCompareLineCount == nLineIndex && !bDriveMode)
				{
					if (nullptr != pbyDriveTimeset)
					{
						*pbyDriveTimeset = nSplitIndex;
					}
					if (0 != nCurLine)
					{
						if (m_mapBRAMFailLineNo.end() == m_mapBRAMFailLineNo.find(nLineIndex))
						{
							m_mapTimeset.insert(make_pair(nLineIndex, nSplitIndex));
							m_mapBRAMFailLineNo.insert(make_pair(nLineIndex, usFailValue));
						}
					}
					return 0;
				}
				++nCompareLineCount;
			}
		}
	}

	int nCycleTime = 0;
	BYTE bySplit = 0;
	for (int nSplitIndex = m_nTimesetStart; nSplitIndex < nStopTimeset; ++nSplitIndex)
	{
		for (int nCurSplit = nSplitIndex + 1; nCurSplit < nStopTimeset; ++nCurSplit)
		{
			for (int nIndex = 0; nIndex < 2; ++nIndex)
			{
				if (0 == nIndex)
				{
					bySplit = nCurSplit;
				}
				else
				{
					bySplit = nSplitIndex;
				}
				nCycleTime = bySplit + 1;
				for (int nCurLine = 0; nCurLine < 2; ++nCurLine)
				{
					if (bDriveMode && nDriveLineCount == nLineIndex)
					{
						if (nullptr != pbyDriveTimeset)
						{
							*pbyDriveTimeset = bySplit;
							return 0;
						}
					}
					++nDriveLineCount;
					if (bDriveMode && 0 < m_nCompareLineCount)
					{
						continue;
					}

					for (int nCompareIndex = m_nTimesetStart; nCompareIndex < nCycleTime; ++nCompareIndex)
					{
						if (nCompareLineCount == nLineIndex && !bDriveMode)
						{
							if (nullptr != pbyDriveTimeset)
							{
								*pbyDriveTimeset = bySplit;
							}

							if (0 != nCurLine)
							{
								if (m_mapBRAMFailLineNo.end() == m_mapBRAMFailLineNo.find(nLineIndex))
								{
									m_mapTimeset.insert(make_pair(nLineIndex, bySplit));
									m_mapBRAMFailLineNo.insert(make_pair(nLineIndex, usFailValue));
								}
							}

							return 0;
						}
						++nCompareLineCount;
					}
				}
			}
		}

		if (nStopTimeset - 1 == nSplitIndex)
		{
			for (int nIndex = 0; nIndex < 2; ++nIndex)
			{
				if (0 == nIndex)
				{
					bySplit = nSplitIndex;
					nCycleTime = nSplitIndex + 1;
				}
				else
				{
					bySplit = m_nTimesetStart;
					nCycleTime = 1;
				}

				for (int nCurLine = 0; nCurLine < 2; ++nCurLine)
				{
					if (bDriveMode && nDriveLineCount == nLineIndex)
					{
						if (nullptr != pbyDriveTimeset)
						{
							*pbyDriveTimeset = bySplit;
							return 0;
						}
					}
					++nDriveLineCount;
					if (bDriveMode && 0 < m_nCompareLineCount)
					{
						continue;
					}
					for (int nCompareIndex = m_nTimesetStart; nCompareIndex < nCycleTime; ++nCompareIndex)
					{
						if (nCompareLineCount == nLineIndex && !bDriveMode)
						{
							if (nullptr != pbyDriveTimeset)
							{
								*pbyDriveTimeset = bySplit;
							}

							if (0 != nCurLine)
							{
								if (m_mapBRAMFailLineNo.end() == m_mapBRAMFailLineNo.find(nLineIndex))
								{
									m_mapTimeset.insert(make_pair(nLineIndex, bySplit));
									m_mapBRAMFailLineNo.insert(make_pair(nLineIndex, usFailValue));
								}
							}
							return 0;
						}
						++nCompareLineCount;
					}
				}
			}
		}
	}
	m_nCompareLineCount = nCompareLineCount;

	m_nDriveLineCount = nDriveLineCount;

	if (-1 == nLineIndex)
	{
		if (bDriveMode)
		{
			return nDriveLineCount;
		}
		else
		{
			return nCompareLineCount;
		}
	}

	return -1;
}

BOOL CDiagnosisRateSplit::Stop()
{
	return FALSE;
}

void CDiagnosisRateSplit::SaveResult()
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
			bAllPass = FALSE;
			break;
		}
	}
	if (bAllPass)
	{
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='true'/>\n", lpszSecondIndent);
	}
	else
	{
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='false'/>\n", lpszSecondIndent);
	}

	dTimeConsume = StopTimer(lpszTimeUnits, sizeof(lpszTimeUnits));
	m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<Time value='%.3f%s'/>\n", lpszSecondIndent, dTimeConsume, lpszTimeUnits);
	if (0 == m_nTimesetStart)
	{
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s</FirstEight>\n", lpszFirstIndent);
	}
	else if (8 == m_nTimesetStart)
	{
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s</SecondEight>\n", lpszFirstIndent);
	}
	else if (16 == m_nTimesetStart)
	{
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s</ThirdEight>\n", lpszFirstIndent);
	}
	else
	{
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s</LastEight>\n", lpszFirstIndent);
	}
	m_mapTestResult.clear();
}
