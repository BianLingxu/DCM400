#include "DiagnosisEdgeScan.h"
#include "..\HDModule.h"
using namespace std;

#define HIGH_DEAD_ZONE 4
#define SCAN_STEP 1
#define TEST_T1R_PLUS_WIDTH 20
#define T1R_COMPARE_WIDTH 31
#define TEST_T1R_OFFSET 4
#define T1R_HIGH_MINUS T1R_COMPARE_WIDTH -  TEST_T1R_PLUS_WIDTH - TEST_T1R_OFFSET
#define TEST_VECTOR_COUNT 50
#define MIN_TEST_RATE 16

//#define ONLY_TEST_T1R 1

CDiagnosisEdgeScan::CDiagnosisEdgeScan()
{
	BOOL bSaveLog = FALSE;
	m_nCompareLineCount = 0;
	m_nDriveLineCount = 0;
	m_bTestSTBR = TRUE;
	m_bNeedLoadVector = TRUE;
	m_dPeriod = MIN_TEST_RATE;
	m_dEdgeValue = 0;
	m_bTestPass = TRUE;
	m_bHaveTest = FALSE;
	m_bSaveLogHead = FALSE;
	m_nCurItemTestIndex = 0;

	m_nSTBRTestEdgeCount = GetTestEdge(-1, TRUE);
}


CDiagnosisEdgeScan::~CDiagnosisEdgeScan()
{
}

const char* CDiagnosisEdgeScan::GetSubItemName() const
{
	if (m_bTestSTBR)
	{
		return "STBR";
	}
	else
	{
		return "T1R/T1F";
	}
	return nullptr;
}

int CDiagnosisEdgeScan::SetEnabled(int enable)
{
	return CMutualTest::SetEnabled(enable);
}

int CDiagnosisEdgeScan::IsEnabled() const
{
	return CMutualTest::IsEnabled();
}

int CDiagnosisEdgeScan::QueryInstance(const char* name, void** ptr)
{
	return 0;
}

void CDiagnosisEdgeScan::Release()
{
}

const char* CDiagnosisEdgeScan::Name() const
{
	return "Edge Scan";
}

int CDiagnosisEdgeScan::GetChildren(STSVector<IHDDoctorItem*>& children) const
{
	return 0;
}

int CDiagnosisEdgeScan::Doctor(IHDReportDevice* pReportDevice)
{
	StartTimer();
	m_pReportDevice = pReportDevice;
	double dTimeConsume = 0;
	char lpszTimeUnits[4] = { 0 };
	m_bTestSTBR = TRUE;
	const char* lpszBaseIndnet = IndentFormat();
	string strNextIndent = IndentFormat() + IndentChar();
	const char* lpszNextIndent = strNextIndent.c_str();
	m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<EdgeScan>\n", lpszBaseIndnet);
	int nRetVal = CMutualTest::Doctor(m_pReportDevice);
	if (0 == nRetVal)
	{
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='true'/>\n", lpszNextIndent);
	}
	else
	{
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='false'/>\n", lpszNextIndent);
	}

	dTimeConsume = StopTimer(lpszTimeUnits, sizeof(lpszTimeUnits));
	m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<Time value='%.3f%s'/>\n", lpszNextIndent, dTimeConsume, lpszTimeUnits);

	m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s</EdgeScan>\n", lpszBaseIndnet);
	return nRetVal;
}

bool CDiagnosisEdgeScan::IsUserCheck() const
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

int CDiagnosisEdgeScan::GetTestController(int nTestIndex, std::vector<UINT>& vecTestController, BOOL bSaveLog)
{
	vecTestController.clear();
	m_mapTestInfo.clear();

	int nTestControllerCount = m_vecEnableController.size();
	int nStartControllerIndex = 0;
	BYTE byDriverControllerIndex = 0;

	string strFirstIndent = IndentFormat() + IndentChar();
	string strSecondIndent = strFirstIndent + IndentChar();
	const char* lpszFirstIndent = strFirstIndent.c_str();
	const char* lpszSecondIndent = strSecondIndent.c_str();

	double dTimeConsume = 0;
	char lpszTimeUnits[8] = { 0 };

	int nCurTestIndex = 0;
	if (0 == nTestIndex)
	{
		m_bSaveLog = bSaveLog;
	}
	const int nControllerTestTimes = 2;
	const BYTE byItemCount = 2;
	BOOL bFirstT1REdge = FALSE;
#ifdef ONLY_TEST_T1R
	nTestIndex += m_nSTBRTestEdgeCount * nControllerTestTimes;
	bFirstT1REdge = TRUE;
#endif // ONLY_TEST_T1R


	for (int nTestItemIndex = 0; nTestItemIndex < byItemCount; ++nTestItemIndex)
	{
		double dLatestEdgeValue = m_dEdgeValue;
		do
		{
			int nTestEdgeIndex = nTestIndex / nControllerTestTimes - (m_bTestSTBR ? 0 : m_nSTBRTestEdgeCount);
			int nRetVal = GetTestEdge(nTestEdgeIndex, m_bTestSTBR);
			if (0 != nRetVal)
			{
				break;
			}
			if (0 == nTestIndex)
			{
				StartTimer();
				m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<STBR>\n", lpszFirstIndent);
				m_mapBRAMFailLineNo.clear();
				m_mapBRAMTimeset.clear();
				m_mapDRAMFailLineNo.clear();
				m_mapDRAMTimeset.clear();
			}
			int nControllerTestIndex = nTestIndex % nControllerTestTimes;
			if (0 == nControllerTestIndex)
			{
				if (0 != nTestIndex && 0 != nTestEdgeIndex)
				{
					SaveEdgeResult(dLatestEdgeValue);
				}
				m_bHaveTest = TRUE;
				StartTimer();
				m_bSaveLogHead = FALSE;
				m_strResultIndent = strSecondIndent;
			}
			switch (nTestIndex % nControllerTestTimes)
			{
			case 0:
				byDriverControllerIndex = 0;
				nStartControllerIndex = 0;
				break;
			case 1:
				byDriverControllerIndex = 1;
				nStartControllerIndex = 0;
				break;
			default:
				byDriverControllerIndex = 0;
				nStartControllerIndex = 0;
				break;
			}
			for (USHORT usControllerIndex = nStartControllerIndex; usControllerIndex < nTestControllerCount; usControllerIndex++)
			{
				TEST_INFO TestInfo;
				if (byDriverControllerIndex == usControllerIndex % 2)
				{
					TestInfo.m_bDriveMode = TRUE;
					if (0 == byDriverControllerIndex)
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
					if (0 == byDriverControllerIndex)
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
		} while (FALSE);
		if (0 != vecTestController.size())
		{
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

#ifdef ONLY_TEST_T1R
		if (m_bTestSTBR)
		{
			StartTimer();
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<T1R_T1F>\n", lpszFirstIndent);
			m_bTestSTBR = FALSE;
		}
		continue;

#endif // ONLY_TEST_T1R

		
		if (m_bTestSTBR)
		{
			//Print the test result to UI.
			SaveEdgeResult(m_dEdgeValue);
			auto iterFailController = m_mapItemTestResult.begin();
			BYTE bySlotNo = 0;
			BYTE byBoardControllerIndex = 0;
			BOOL bAllPass = TRUE;
			for (auto uControllerID : m_vecEnableController)
			{
				bySlotNo = HDModule::Instance()->ID2Board(uControllerID, byBoardControllerIndex);
				if (m_mapItemTestResult.end() == m_mapItemTestResult.find(uControllerID))
				{
					m_pReportDevice->PrintfToUi(IHDReportDevice::Pass);
				}
				else
				{
					bAllPass = FALSE;
					m_pReportDevice->PrintfToUi(IHDReportDevice::Fail);
				}
				m_pReportDevice->PrintfToUi("\t Slot %d, Controller %d\n", bySlotNo, byBoardControllerIndex);

			}
			m_mapItemTestResult.clear();

			dTimeConsume = StopTimer(lpszTimeUnits, sizeof(lpszTimeUnits));
			if(bAllPass)
			{
				m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='true'/>\n", lpszSecondIndent);
			}
			else
			{
				m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='true'/>\n", lpszSecondIndent);
			}
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<Time value='%.3f%s'/>\n", lpszSecondIndent, dTimeConsume, lpszTimeUnits);

			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s</STBR>\n", lpszFirstIndent);

			m_bTestPass = TRUE;

			StartTimer();
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<T1R_T1F>\n", lpszFirstIndent);
			m_mapBRAMFailLineNo.clear();
			m_mapBRAMTimeset.clear();
			m_mapDRAMFailLineNo.clear();
			m_mapDRAMTimeset.clear();
			m_bTestSTBR = FALSE;
			bFirstT1REdge = TRUE;
		}
	}
	if (m_bHaveTest)
	{
		SaveEdgeResult(m_dEdgeValue);
		if (m_bTestPass)
		{
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='true'/>\n", lpszSecondIndent);
		}
		else
		{
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='false'/>\n", lpszSecondIndent);
		}

		//Print the test result to UI.
		USHORT uTestControllerCount = m_vecEnableController.size();
		auto iterFailController = m_mapItemTestResult.begin();
		BYTE bySlotNo = 0;
		BYTE byBoardControllerIndex = 0;

		for (auto uControllerID : m_vecEnableController)
		{
			bySlotNo = HDModule::Instance()->ID2Board(uControllerID, byBoardControllerIndex);
			
			if (m_mapItemTestResult.end() == m_mapItemTestResult.find(uControllerID))
			{
				m_pReportDevice->PrintfToUi(IHDReportDevice::Pass);
			}
			else
			{
				m_pReportDevice->PrintfToUi(IHDReportDevice::Fail);
			}
			m_pReportDevice->PrintfToUi("\t Slot %d, Controller %d\n", bySlotNo, byBoardControllerIndex);
		}
		m_mapItemTestResult.clear();

		dTimeConsume = StopTimer( lpszTimeUnits, sizeof(lpszTimeUnits));
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<Time value='%.3f%s'/>\n", lpszSecondIndent, dTimeConsume, lpszTimeUnits);
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s</T1R_T1F>\n", lpszFirstIndent);
		m_bHaveTest = FALSE;
	}

	return -1;
}

int CDiagnosisEdgeScan::GetMutualTestVector(UINT uControllerID, int nLineIndex, char* lpszVector, int nVectorBuffSize, char* lpszCMD, int nCMDBuffSize, BYTE* pbyTimeset, ULONG* pulOperand, BOOL* pbBRAMLine, BOOL* pbNextLineOtherMemory)
{
	auto iterController = m_mapTestInfo.find(uControllerID);
	if (m_mapTestInfo.end() == iterController)
	{
		return -1;
	}

	if (nullptr != pulOperand)
	{
		*pulOperand = 0;
	}
	if (nullptr != pbBRAMLine)
	{
		*pbBRAMLine = TRUE;
	}
	if (nullptr != lpszCMD)
	{
		strcpy_s(lpszCMD, nCMDBuffSize, "INC");
	}
	if (nullptr != pbNextLineOtherMemory)
	{
		*pbNextLineOtherMemory = FALSE;
	}

	while (0 != GetVectorLineInfo(nLineIndex, iterController->second.m_bDriveMode, lpszVector, nVectorBuffSize, pbyTimeset))
	{
		return -1;
	}
	return 0;
}

void CDiagnosisEdgeScan::GetCheckDataController(std::vector<UINT>& vecCheckController)
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

int CDiagnosisEdgeScan::GetTimesetSetting(UINT uControllerID, std::map<BYTE, TIMESET_VALUE>& mapEdgeValue)
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
			EdgeValue.m_dPeriod = m_dPeriod;
			if (iterController->second.m_bDriveMode)
			{
				EdgeValue.m_dEgde[0] = 0;
				EdgeValue.m_dEgde[1] = 15;
				EdgeValue.m_dEgde[2] = EdgeValue.m_dEgde[0];
				EdgeValue.m_dEgde[3] = EdgeValue.m_dEgde[1];
				EdgeValue.m_dEgde[4] = 15;
				EdgeValue.m_dEgde[5] = 20;
				EdgeValue.m_WaveFormat = WAVE_FORMAT::NRZ;
			}
			else
			{
				EdgeValue.m_dEgde[0] = 5;
				EdgeValue.m_dEgde[1] = 15;
				EdgeValue.m_dEgde[2] = EdgeValue.m_dEgde[0];
				EdgeValue.m_dEgde[3] = EdgeValue.m_dEgde[1];
				EdgeValue.m_dEgde[4] = m_dEdgeValue;
				EdgeValue.m_dEgde[5] = EdgeValue.m_dEgde[4] + 1;
				EdgeValue.m_WaveFormat = WAVE_FORMAT::NRZ;
			}
			bFinish = TRUE;
		}
		else
		{
			EdgeValue.m_dPeriod = m_dPeriod;
			if (iterController->second.m_bDriveMode)
			{
				EdgeValue.m_dEgde[0] = m_dEdgeValue;
				EdgeValue.m_dEgde[1] = m_dEdgeValue + TEST_T1R_PLUS_WIDTH;
				EdgeValue.m_dEgde[2] = EdgeValue.m_dEgde[0];
				EdgeValue.m_dEgde[3] = EdgeValue.m_dEgde[1];
				EdgeValue.m_dEgde[4] = 15;
				EdgeValue.m_dEgde[5] = 20;
				EdgeValue.m_WaveFormat = WAVE_FORMAT::RZ;
				bFinish = TRUE;
			}
			else
			{
				EdgeValue.m_dEgde[0] = 5;
				EdgeValue.m_dEgde[1] = 15;
				EdgeValue.m_dEgde[2] = 0;
				EdgeValue.m_dEgde[3] = EdgeValue.m_dEgde[1];
				EdgeValue.m_dEgde[4] = m_dEdgeValue - 4 + byTimesetIndex;
				EdgeValue.m_dEgde[5] = EdgeValue.m_dEgde[4] + 1;
				EdgeValue.m_WaveFormat = WAVE_FORMAT::NRZ;
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

int CDiagnosisEdgeScan::GetPinLevel(UINT uControllerID, double* pdVIH, double* pdVIL, double* pdVOH, double* pdVOL)
{
	BOOL bFindController = FALSE;
	for (auto uCurControllerID : m_vecEnableController)
	{
		if (uControllerID == uCurControllerID)
		{
			bFindController = TRUE;
			break;
		}
	}
	if (!bFindController)
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

void CDiagnosisEdgeScan::GetVectorStartLine(UINT* puRAMStartLine, UINT* puDRAMStartLine)
{
	if (nullptr == puRAMStartLine || nullptr == puDRAMStartLine)
	{
		return;
	}
	*puRAMStartLine = 0;
	*puDRAMStartLine = 0;
}

int CDiagnosisEdgeScan::CheckResult(UINT uControllerID, const std::map<int, USHORT>& mapBRAMFailLineNo, const std::map<int, USHORT>& mapDRAMFailLineNo, int nFailCount)
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

	string strFirstIndent = m_strResultIndent;
	string strSecondIndent = strFirstIndent + IndentChar();
	string strThirdIndent = strSecondIndent + IndentChar();
	const char* lpszFirstIndent = strFirstIndent.c_str();
	const char* lpszSecondIndent = strSecondIndent.c_str();
	const char* lpszThirdIndent = strThirdIndent.c_str();

	UINT uTestController = uControllerID;
	if (!m_bTestSTBR)
	{
		uTestController = iterController->second.m_uRelateControllerID;
	}

	m_bHaveTest = TRUE;

	BOOL bPrintControl = FALSE;
	if (m_bSaveLog)
	{
		BYTE bySlotNo = 0;
		BYTE byBoardControllerIndex = 0;
		bySlotNo = HDModule::Instance()->ID2Board(uTestController, byBoardControllerIndex);
		if (!m_bSaveLogHead)
		{
			double dShowPeriod = GetPeriodUnits(m_dPeriod, lpszTimeUnits, sizeof(lpszTimeUnits));
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<Edge_%.0fns Period='%.1f%s'>\n", lpszFirstIndent, m_dEdgeValue, dShowPeriod, lpszTimeUnits);
			m_bSaveLogHead = TRUE;
		}
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<controller value='%d, slot value = %d'>\n", lpszSecondIndent,
			byBoardControllerIndex, bySlotNo);
		bPrintControl = TRUE;
	}

	int nRetVal = 0;
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

				m_pReportDevice->PrintfToDataLog(IHDReportDevice::Error, "%s<data retValue='false' %s/>\n", lpszThirdIndent, lpszValue);

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
				sprintf_s(lpszValue, sizeof(lpszValue), "value='expect addr= - Real addr=0x%X Expect= - Real=0x%X'",
					iterRealFailLineNo->first, iterRealFailLineNo->second);
				++iterRealFailLineNo;
				if (ERROR_PRINT < nFailCount++)
				{
					break;
				}
			}
			nFailCount = 0;
			while (m_mapBRAMFailLineNo.end() != iterExpectFailLineNo)
			{
				sprintf_s(lpszValue, sizeof(lpszValue), "value=' expect addr=0x%X Real addr= - Expect=0x%X Real= -'",
					iterExpectFailLineNo->first, iterExpectFailLineNo->second);
				++iterExpectFailLineNo;
				if (ERROR_PRINT < nFailCount++)
				{
					break;
				}
			}

			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Error, "%s<data retValue='false' %s/>\n", lpszThirdIndent, lpszValue);
		}
	}
	if (0 != mapDRAMFailLineNo.size())
	{
		nRetVal = -1;
		char lpszValue[128] = { 0 };
		sprintf_s(lpszValue, sizeof(lpszValue), "value='DRAMCaptureCount Expect=0x%X Real=0x%X'", 0, mapDRAMFailLineNo.size());

		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Error, "%s<data retValue='false' %s/>\n", lpszThirdIndent, lpszValue);
	}

	//Ensure only print one information per controller.
	dTimeConsume = StopTimer( lpszTimeUnits, sizeof(lpszTimeUnits));

	if (nullptr != m_pReportDevice)
	{
		if (bPrintControl)
		{
			if (0 != nRetVal)
			{
				//Test fail
				m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='false'/>\n", lpszThirdIndent);
			}
			else
			{
				m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='true'/>\n", lpszThirdIndent);
			}
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<Time value='%.3f%s'/>\n", lpszThirdIndent, dTimeConsume, lpszTimeUnits);
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s</controller>\n", lpszSecondIndent);
		}

	}

	if (m_mapTestResult.end() == m_mapTestResult.find(uTestController))
	{
		m_mapTestResult.insert(make_pair(uTestController, nRetVal));
	}
	if (0 != nRetVal)
	{
		if (m_mapItemTestResult.end() == m_mapItemTestResult.find(uTestController))
		{
			m_mapItemTestResult.insert(make_pair(uTestController, 0));
		}
	}

	return nRetVal;
}

int CDiagnosisEdgeScan::GetSameVectorControllerType()
{
	return 2;
}

int CDiagnosisEdgeScan::GetSameVectorController(int nTypeIndex, std::vector<UINT>& vecSameController)
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

BOOL CDiagnosisEdgeScan::IsReloadVector()
{
	return m_bNeedLoadVector;
}

int CDiagnosisEdgeScan::GetVectorLineInfo(int nLineIndex, BOOL bDriveMode, char* lpszPattern, int nBuffSize, BYTE* pbyTimeset)
{
	if (TEST_VECTOR_COUNT <= nLineIndex)
	{
		return -1;
	}
	if (m_bTestSTBR)
	{
		if (nullptr != pbyTimeset)
		{
			*pbyTimeset = 0;
		}
		if (bDriveMode)
		{
			if (nullptr != lpszPattern)
			{
				if (0 == nLineIndex % 2)
				{
					strcpy_s(lpszPattern, nBuffSize, "0000000000000000");
				}
				else
				{
					strcpy_s(lpszPattern, nBuffSize, "1111111111111111");
				}
			}
		}
		else
		{
			if (nullptr != lpszPattern)
			{
				strcpy_s(lpszPattern, nBuffSize, "HHHHHHHHHHHHHHHH");
			}
			if (0 == (nLineIndex % 2) && m_mapBRAMFailLineNo.end() == m_mapBRAMFailLineNo.find(nLineIndex))
			{
				m_mapBRAMFailLineNo.insert(make_pair(nLineIndex, 0xFFFF));
				m_mapBRAMTimeset.insert(make_pair(nLineIndex, 0));
			}
		}
	}
	else
	{
		if (bDriveMode)
		{
			if (nullptr != lpszPattern && 0 != nBuffSize)
			{
				strcpy_s(lpszPattern, nBuffSize, "1111111111111111");
			}
			if (nullptr != pbyTimeset)
			{
				*pbyTimeset = 0;
			}
		}
		else
		{
			const BYTE byTimesetCount = T1R_COMPARE_WIDTH / SCAN_STEP;
			BYTE byCurTimesetIndex = nLineIndex % byTimesetCount;
			if (nullptr != pbyTimeset)
			{
				*pbyTimeset = byCurTimesetIndex;
			}
			strcpy_s(lpszPattern, nBuffSize, "HHHHHHHHHHHHHHHH");
			USHORT usCompareData = 0;

			if (2 <= byCurTimesetIndex && TEST_T1R_PLUS_WIDTH + TEST_T1R_OFFSET + 5 >= byCurTimesetIndex)
			{
				if (0 <= 9 - byCurTimesetIndex || (TEST_T1R_OFFSET + TEST_T1R_PLUS_WIDTH - 5 <= byCurTimesetIndex && TEST_T1R_OFFSET + TEST_T1R_PLUS_WIDTH + 5 >= byCurTimesetIndex))
				{
					strcpy_s(lpszPattern, nBuffSize, "XXXXXXXXXXXXXXXX");
				}
				usCompareData = 0x00;
			}
			else
			{
				usCompareData = 0xFFFF;
			}
			if (0 != usCompareData && m_mapBRAMFailLineNo.end() == m_mapBRAMFailLineNo.find(nLineIndex))
			{
				m_mapBRAMFailLineNo.insert(make_pair(nLineIndex, usCompareData));
				m_mapBRAMTimeset.insert(make_pair(nLineIndex, byCurTimesetIndex));
			}
		}
	}
	return 0;
}

BOOL CDiagnosisEdgeScan::Stop()
{
	if (0 != m_nCurItemTestIndex % 2)
	{
		return FALSE;
	}

	auto iterController = m_mapItemTestResult.begin();
	UINT uTestControllerCount = m_vecEnableController.size();
	BOOL bAllPass = TRUE;
	BYTE bySlotNo = 0;
	BYTE byBoardControllerIndex = 0;
	for (auto uControllerID : m_vecEnableController)
	{
		bySlotNo = HDModule::Instance()->ID2Board(uControllerID, byBoardControllerIndex);
		iterController = m_mapItemTestResult.find(uControllerID);

		if (m_mapItemTestResult.end() == iterController)
		{
			m_pReportDevice->PrintfToUi(IHDReportDevice::Pass);
		}
		else
		{
			bAllPass = FALSE;
			m_pReportDevice->PrintfToUi(IHDReportDevice::Fail);
		}
		m_pReportDevice->PrintfToUi("\t Slot %d, Controller %d\n", bySlotNo, byBoardControllerIndex);
	}

	string strFirstIndent = IndentFormat() + IndentChar();
	string strSecondIndent = strFirstIndent + IndentChar();
	const char* lpszFirstIndent = strFirstIndent.c_str();
	const char* lpszSecondIndent = strSecondIndent.c_str();

	SaveEdgeResult(m_dEdgeValue);

	double dTimeConsume = 0;
	char lpszTimeUnits[32] = { 0 };

	if (m_bTestPass)
	{
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='true'/>\n", lpszSecondIndent);
	}
	else
	{
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='true'/>\n", lpszSecondIndent);
	}


	dTimeConsume = StopTimer( lpszTimeUnits, sizeof(lpszTimeUnits));
	m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<Time value='%.3f%s'/>\n", lpszSecondIndent, dTimeConsume, lpszTimeUnits);
	if (m_bTestSTBR)
	{
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s</STBR>\n", lpszFirstIndent);
	}
	else
	{
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s</T1R_T1F>\n", lpszFirstIndent);
	}

	string strType = "STBR";
	if (!m_bTestSTBR)
	{
		strType = "T1R_T1F";
	}
	m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<UserStop value='TestType=%s Period=%.0fns Edge=%.0fns'/>\n", lpszFirstIndent,strType.c_str(), m_dPeriod, m_dEdgeValue);
	return TRUE;
}

inline int CDiagnosisEdgeScan::GetTestEdge(int nTestEdgeIndex, BOOL bTestSTBR)
{
	if (bTestSTBR)
	{
		const int nTestPeriodCount = 3;
		const double dTestPeriod[nTestPeriodCount] = { 32,16e3,1.6e6 };
		const double dTestStart[nTestPeriodCount] = { 8, dTestPeriod[1] / 2, dTestPeriod[2] / 2 };
		const double dTestStep[nTestPeriodCount] = { 1, 100, 1000 };
		int nTotalTestCount = 0;
		int nTestPeriodIndex = 0;
		for (nTestPeriodIndex = 0; nTestPeriodIndex < nTestPeriodCount; nTestPeriodIndex++)
		{
			nTotalTestCount += (int)(dTestPeriod[nTestPeriodIndex] - dTestStart[nTestPeriodIndex] - HIGH_DEAD_ZONE + 1) / dTestStep[nTestPeriodIndex];
		}
		
		if (-1 == nTestEdgeIndex)
		{
			return nTotalTestCount;
		}
		if (nTotalTestCount <= nTestEdgeIndex)
		{
			return -1;
		}


		int nBaseTestOffset = 0;
		nTestPeriodIndex = 0;
		for (nTestPeriodIndex = 0; nTestPeriodIndex < nTestPeriodCount; nTestPeriodIndex++)
		{
			int nTestCount = (dTestPeriod[nTestPeriodIndex] - dTestStart[nTestPeriodIndex] - HIGH_DEAD_ZONE + 1) / dTestStep[nTestPeriodIndex];
			if (nTestEdgeIndex >= nBaseTestOffset + nTestCount)
			{
				nBaseTestOffset += nTestCount;
				continue;
			}
			break;
		}
		m_dPeriod = dTestPeriod[nTestPeriodIndex];
		m_dEdgeValue = dTestStart[nTestPeriodIndex] + dTestStep[nTestPeriodIndex] * (nTestEdgeIndex - nBaseTestOffset);
		
	}
	else
	{
		const int nTestPeriodCount = 3;
		const double dTestPeriod[nTestPeriodCount] = { 100, 10e3,1e6 };
		const double dTestStart[nTestPeriodCount] = { TEST_T1R_OFFSET, 4480 - T1R_HIGH_MINUS, 999980 - T1R_HIGH_MINUS };
		const double dTestTimes[nTestPeriodCount] = { 1, 1, 1 };
		
		int nTotalEdgeCount = 0;
		for (int nTestPeriodIndex = 0; nTestPeriodIndex < nTestPeriodCount; nTestPeriodIndex++)
		{
			nTotalEdgeCount += (int)(dTestTimes[nTestPeriodIndex] / SCAN_STEP);
		}
		if (-1 == nTestEdgeIndex)
		{
			return nTotalEdgeCount;
		}

		if (nTotalEdgeCount <= nTestEdgeIndex)
		{
			return -1;
		}

		int nBaseTestOffset = 0;
		int nTestPeriodIndex = 0;
		for (nTestPeriodIndex = 0; nTestPeriodIndex < nTestPeriodCount; nTestPeriodIndex++)
		{
			int nTestCount = dTestTimes[nTestPeriodIndex] / SCAN_STEP;
			if (nTestEdgeIndex >= nBaseTestOffset + nTestCount)
			{
				nBaseTestOffset += nTestCount;
				continue;
			}
			break;
		}
		m_dPeriod = dTestPeriod[nTestPeriodIndex];
		m_dEdgeValue = dTestStart[nTestPeriodIndex] + SCAN_STEP * (nTestEdgeIndex - nBaseTestOffset);
	}
	return 0;
}

inline void CDiagnosisEdgeScan::SaveEdgeResult(double dEdgeValue)
{
	string strFirstIndent = IndentFormat() + IndentChar();
	string strSecondIndent = strFirstIndent + IndentChar();
	string strThirdIndent = strSecondIndent + IndentChar();
	const char* lpszFirstIndent = strFirstIndent.c_str();
	const char* lpszSecondIndent = strSecondIndent.c_str();
	const char* lpszThirdIndent = strThirdIndent.c_str();

	BOOL bAllPass = TRUE;
	for (auto& TestResult : m_mapTestResult)
	{
		if (0 != TestResult.second)
		{
			bAllPass = FALSE;
			break;
		}
	}
	char lpszTimeUnits[32] = { 0 };
	double dTimeConsume = StopTimer( lpszTimeUnits, sizeof(lpszTimeUnits));
	if (m_bSaveLog || !bAllPass)
	{
		if (!bAllPass)
		{
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='false'/>\n", lpszThirdIndent);
		}
		else
		{
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='true'/>\n", lpszThirdIndent);
		}
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<Time value='%.3f%s'/>\n", lpszThirdIndent, dTimeConsume, lpszTimeUnits);

		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s</Edge_%.0fns>\n", lpszSecondIndent, dEdgeValue);
	}
	m_bSaveLogHead = FALSE;
	m_mapTestResult.clear();
}
