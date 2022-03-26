#include "DiagnosisConditionalInstrcution.h"
#include "..\HDModule.h"
using namespace std;
CDiagnosisConditionalInstrcution::CDiagnosisConditionalInstrcution()
	: m_uSubItemIndex(0)
{
	m_vecSubItem.push_back("FJUMP");
	m_vecSubItem.push_back("MATCH");
	m_vecSubItem.push_back("MJUMP");
}

CDiagnosisConditionalInstrcution::~CDiagnosisConditionalInstrcution()
{
}

int CDiagnosisConditionalInstrcution::QueryInstance(const char* name, void** ptr)
{
	return -1;
}

void CDiagnosisConditionalInstrcution::Release()
{
}

const char* CDiagnosisConditionalInstrcution::Name() const
{
	return "Conditional Instruction Diagnosis";
}

int CDiagnosisConditionalInstrcution::GetChildren(STSVector<IHDDoctorItem*>& vecChildren) const
{
	return 0;
}

int CDiagnosisConditionalInstrcution::Doctor(IHDReportDevice* pReportDevice)
{
	m_pReportDevice = pReportDevice;
	StartTimer();
	double dTimeConsume = 0;
	char lpszTimeUnits[4] = { 0 };
	const char* lpszBaseIndnet = IndentFormat();
	string strNextIndent = lpszBaseIndnet + IndentChar();
	const char* lpszNextIndent = strNextIndent.c_str();
	m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<ConditionalInstrustion>\n", lpszBaseIndnet);
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

	m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s</ConditionalInstrustion>\n", lpszBaseIndnet);
	return nRetVal;
}

bool CDiagnosisConditionalInstrcution::IsUserCheck() const
{
#ifdef _DEBUG
	return true;
#endif // _DEBUG

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

const char* CDiagnosisConditionalInstrcution::GetSubItemName() const
{
	if (m_uSubItemIndex >= m_vecSubItem.size())
	{
		return nullptr;
	}
	return m_vecSubItem[m_uSubItemIndex].c_str();
}

int CDiagnosisConditionalInstrcution::GetTestController(int nTestIndex, std::vector<UINT>& vecTestController, BOOL bPrintLog)
{
	string strFirstIndent = IndentFormat() + IndentChar();
	string strSecondIndent = strFirstIndent + IndentChar();
	const char* lpszFirstIndent = strFirstIndent.c_str();
	const char* lpszSecondIndent = strSecondIndent.c_str();

	double dTimeConsume = 0;
	char lpszTimeUnits[4] = { 0 };
	vecTestController.clear();

	m_mapTestInfo.clear();
	const int nItemTestTimes = 2;
	int nTestControllerCount = m_vecEnableController.size();

	BOOL nRetVal = 0;
	BOOL bSetChannelStatus = FALSE;
	CHANNEL_OUTPUT_STATUS ChannelStatus = CHANNEL_OUTPUT_STATUS::LOW;
	int nCurTestIndex = 0;
	int nSubItemCount = m_vecSubItem.size();
	for (int nSubItemIndex = 0; nSubItemIndex < nSubItemCount; ++nSubItemIndex)
	{
		if (nSubItemIndex * nItemTestTimes + nItemTestTimes <= nTestIndex)
		{
			continue;
		}
		m_uSubItemIndex = nSubItemIndex;
		if (nSubItemIndex != m_uSubItemIndex)
		{
			m_mapBRAMFailLineNo.clear();
		}
		if (0 == nTestIndex % nItemTestTimes)
		{
			SaveResult();
			StartTimer();
			m_vecLineOrder.clear();
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<%s>\n", lpszFirstIndent, m_vecSubItem[nSubItemIndex].c_str());
		}

		int nStartControllerIndex = 0;
		BYTE byDriveControllerIndex = 0;

		for (int nIndex = nTestIndex % nItemTestTimes; nIndex < nItemTestTimes; ++nIndex)
		{
			switch (nIndex)
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
			return 0;
		}
	}
	SaveResult();
	return -1;
}

int CDiagnosisConditionalInstrcution::GetMutualTestVector(UINT uControllerID, int nLineIndex, char* lpszVector, int nVectorBuffSize, char* lpszCMD, int nCMDBuffSize, BYTE* pbyTimeset, ULONG* pulOperand, BOOL* pbBRAMLine, BOOL* pbNextLineOtherMemory)
{
	if (m_uSubItemIndex >= m_vecSubItem.size())
	{
		return -1;
	}
	string strCurSubItem = m_vecSubItem[m_uSubItemIndex];
	if (0 == strCurSubItem.compare("FJUMP"))
	{
		return GetFJUMPPattern(uControllerID, nLineIndex, lpszVector, nVectorBuffSize, lpszCMD, nCMDBuffSize, pbyTimeset, pulOperand, pbBRAMLine, pbNextLineOtherMemory);
	}
	else if (0 == strCurSubItem.compare("MJUMP"))
	{
		return GetMJUMPPattern(uControllerID, nLineIndex, lpszVector, nVectorBuffSize, lpszCMD, nCMDBuffSize, pbyTimeset, pulOperand, pbBRAMLine, pbNextLineOtherMemory);
	}
	else if (0 == strCurSubItem.compare("MATCH"))
	{
		return GetMATCHPattern(uControllerID, nLineIndex, lpszVector, nVectorBuffSize, lpszCMD, nCMDBuffSize, pbyTimeset, pulOperand, pbBRAMLine, pbNextLineOtherMemory);
	}
	return -1;
}

void CDiagnosisConditionalInstrcution::GetVectorStartLine(UINT* puBRAMStartLine, UINT* puDRAMStartLine)
{
	if (nullptr != puBRAMStartLine)
	{
		*puBRAMStartLine = 0;
	}
	if (nullptr != puDRAMStartLine)
	{
		*puDRAMStartLine = 0;
	}
}

void CDiagnosisConditionalInstrcution::GetCheckDataController(std::vector<UINT>& vecCheckController)
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

int CDiagnosisConditionalInstrcution::GetCheckDataType(UINT uControllerID)
{
	return 1;
}

int CDiagnosisConditionalInstrcution::GetCheckDataType()
{
	return 1;
}

int CDiagnosisConditionalInstrcution::GetTimesetSetting(UINT uControllerID, std::map<BYTE, TIMESET_VALUE>& mapEdgeValue)
{
	mapEdgeValue.clear();
	TIMESET_VALUE Timeset;
	Timeset.m_WaveFormat = WAVE_FORMAT::NRZ;
	Timeset.m_dPeriod = 1000;
	Timeset.m_dEgde[0] = 10;
	Timeset.m_dEgde[1] = Timeset.m_dPeriod / 2 + Timeset.m_dEgde[0];
	Timeset.m_dEgde[2] = Timeset.m_dEgde[0];
	Timeset.m_dEgde[3] = Timeset.m_dEgde[1];
	Timeset.m_dEgde[4] = Timeset.m_dPeriod /2;
	Timeset.m_dEgde[5] = Timeset.m_dPeriod * 3 / 4;
	mapEdgeValue.insert(make_pair(0, Timeset));
	return 0;
}

int CDiagnosisConditionalInstrcution::GetPinLevel(UINT uControllerID, double* pdVIH, double* pdVIL, double* pdVOH, double* pdVOL)
{
	if (nullptr == pdVIH || nullptr == pdVIL || nullptr == pdVOH || nullptr  == pdVOL)
	{
		return -2;
	}
	*pdVIH = 3;
	*pdVIL = 0;
	*pdVOH = 1.5;
	*pdVOL = 0.8;
	return 0;
}

int CDiagnosisConditionalInstrcution::CheckResult(UINT uControllerID, const std::map<int, USHORT>& mapBRAMFailLineNo, const std::map<int, USHORT>& mapDRAMFailLineNo, int nFailCount)
{
	return -1;
}

int CDiagnosisConditionalInstrcution::CheckLineOrder(UINT uControllerID, const std::vector<UINT>& vecLineOrder)
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
	BYTE bySlotNo = 0;
	BYTE byBoardControllerIndex = 0;

	string strFirstIndent = IndentFormat() + IndentChar() + IndentChar();
	string strSecondIndent = strFirstIndent + IndentChar();
	const char* lpszFirstIndent = strFirstIndent.c_str();
	const char* lpszSecondIndent = strSecondIndent.c_str();

	bySlotNo = HDModule::Instance()->ID2Board(uControllerID, byBoardControllerIndex);

	if (nullptr != m_pReportDevice)
	{
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<controller value='%d, slot value = %d'>\n", lpszFirstIndent, byBoardControllerIndex, bySlotNo);
	}

	int nRetVal = 0;
	if (m_vecLineOrder != vecLineOrder)
	{
		nRetVal = -1;
		int nExpectOrderCount = m_vecLineOrder.size();
		int nRealOrderCount = vecLineOrder.size();
		int nMinOrder = nExpectOrderCount > nRealOrderCount ? nRealOrderCount : nExpectOrderCount;
		for (int nOrderIndex = 0; nOrderIndex < nMinOrder;++nOrderIndex)
		{
			IHDReportDevice::DataLogLevel LogLevel = IHDReportDevice::Error;
			if (vecLineOrder[nOrderIndex] == m_vecLineOrder[nOrderIndex])
			{
				LogLevel = IHDReportDevice::Correct;
			}
			m_pReportDevice->PrintfToDataLog(LogLevel, "%s<data retValue='false' value = 'Expect=%d Real=%d' />\n", lpszSecondIndent,
				m_vecLineOrder[nOrderIndex] ,vecLineOrder[nOrderIndex]);
		}
		if (nRealOrderCount > nMinOrder)
		{
			for (int nOrderIndex = nMinOrder; nOrderIndex < nRealOrderCount; ++nOrderIndex)
			{
				IHDReportDevice::DataLogLevel LogLevel = IHDReportDevice::Error;
				m_pReportDevice->PrintfToDataLog(LogLevel, "%s<data retValue='false' value = 'Expect=- Real=%d' />\n", lpszSecondIndent,
					vecLineOrder[nOrderIndex]);
			}
		}
		if (nExpectOrderCount > nMinOrder)
		{
			for (int nOrderIndex = nExpectOrderCount; nOrderIndex < nRealOrderCount; ++nOrderIndex)
			{
				IHDReportDevice::DataLogLevel LogLevel = IHDReportDevice::Error;
				m_pReportDevice->PrintfToDataLog(LogLevel, "%s<data retValue='false' value = 'Expect=%d Real=-' />\n", lpszSecondIndent,
					m_vecLineOrder[nOrderIndex]);
			}
		}

	}
	// Ensure only print one information per controller.
	if (nullptr != m_pReportDevice)
	{
		if (m_mapTestResult.end() == m_mapTestResult.find(uControllerID))
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
			bySlotNo = HDModule::Instance()->ID2Board(uControllerID, byBoardControllerIndex);
			m_pReportDevice->PrintfToUi("\t Slot %d, Controller %d\n", bySlotNo, byBoardControllerIndex);
			m_mapTestResult.insert(make_pair(uControllerID, nRetVal));
		}
	}

	if (nullptr != m_pReportDevice)
	{
		double dTimeConsume = 0;
		char lpszTimeUnits[32] = { 0 };
		dTimeConsume = StopTimer(lpszTimeUnits, sizeof(lpszTimeUnits));
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<Time value='%.3f%s'/>\n", lpszSecondIndent, dTimeConsume, lpszTimeUnits);
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s</controller>\n", lpszFirstIndent);
	}
	return nRetVal;
}

int CDiagnosisConditionalInstrcution::GetSameVectorControllerType()
{
	return 2;
}

int CDiagnosisConditionalInstrcution::GetSameVectorController(int nTypeIndex, std::vector<UINT>& vecSameController)
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

BOOL CDiagnosisConditionalInstrcution::IsReloadVector()
{
	return TRUE;
}

BOOL CDiagnosisConditionalInstrcution::Stop()
{
	return FALSE;
}

void CDiagnosisConditionalInstrcution::SaveResult()
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
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<data retValue='true'/>\n", lpszSecondIndent);
	}
	else
	{
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<data retValue='false'/>\n", lpszSecondIndent);
	}

	dTimeConsume = StopTimer(lpszTimeUnits, sizeof(lpszTimeUnits));
	m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<Time value='%.3f%s'/>\n", lpszSecondIndent, dTimeConsume, lpszTimeUnits);
	m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s</%s>\n", lpszFirstIndent, m_vecSubItem[m_uSubItemIndex].c_str());
	m_mapTestResult.clear();
}

int CDiagnosisConditionalInstrcution::GetMJUMPPattern(UINT uControllerID, int nLineIndex, char* lpszVector, int nVectorBuffSize, char* lpszCMD, int nCMDBuffSize, BYTE* pbyTimeset, ULONG* pulOperand, BOOL* pbBRAMLine, BOOL* pbNextLineOtherMemory)
{
	auto iterController = m_mapTestInfo.find(uControllerID);
	if (m_mapTestInfo.end() == iterController)
	{
		return -2;
	}
	if (nullptr != lpszCMD)
	{
		strcpy_s(lpszCMD, nCMDBuffSize, "INC");
	}
	if (nullptr != pbyTimeset)
	{
		*pbyTimeset = 0;
	}
	if (nullptr != pulOperand)
	{
		*pulOperand = 0;
	}
	if (nullptr == pbBRAMLine)
	{
		*pbBRAMLine = TRUE;
	}
	if (nullptr != pbNextLineOtherMemory)
	{
		*pbNextLineOtherMemory = FALSE;
	}

	if (iterController->second.m_bDriveMode)
	{
		if (40 <= nLineIndex)
		{
			return -1;
		}
		if ((6 <= nLineIndex && 8 >= nLineIndex) || (13 <= nLineIndex && 17 >= nLineIndex))
		{
			if (nullptr != lpszVector)
			{
				strcpy_s(lpszVector, nVectorBuffSize, "1111111111111111");
			}
		}
		else
		{
			if (nullptr != lpszVector)
			{
				strcpy_s(lpszVector, nVectorBuffSize, "0000000000000000");
			}
		}
		return 0;
	}

	///<The instruction controller
	if (nullptr != lpszVector)
	{
		strcpy_s(lpszVector, nVectorBuffSize, "LLLLLLLLLLLLLLLL");
	}


	if (30 <= nLineIndex)
	{
		return -1;
	}
	int nMachCount = 4;
	int nMatchLoopCount = nMachCount + 1;
	const int nMatchLine = 6;
	const int nSecondMatchLine = 10;
	switch (nLineIndex)
	{
	case 3:
	{
		if (nullptr != lpszCMD)
		{
			strcpy_s(lpszCMD, nCMDBuffSize, "SET_MCNT");
		}
		if (nullptr != pulOperand)
		{
			*pulOperand = nMachCount;
		}
	}
	break;
	case nMatchLine:
	{
		if (nullptr != lpszCMD)
		{
			strcpy_s(lpszCMD, nCMDBuffSize, "MJUMP");
		}
		if (nullptr != pulOperand)
		{
			*pulOperand = 0;
		}
	}
	break;
	case nSecondMatchLine - 2:
	{
		if (nullptr != lpszCMD)
		{
			strcpy_s(lpszCMD, nCMDBuffSize, "SET_MCNT");
		}
		if (nullptr != pulOperand)
		{
			*pulOperand = nMachCount;
		}
	}
	break;
	case nSecondMatchLine:
	{
		if (nullptr != lpszCMD)
		{
			strcpy_s(lpszCMD, nCMDBuffSize, "MJUMP");
		}
	}
	break;
	default:
		break;
	}
	if (0 == m_vecLineOrder.size())
	{
		for (int nIndex = 0; nIndex < nMatchLine; ++nIndex)
		{
			m_vecLineOrder.push_back(nIndex);
		}
		for (int nIndex = 0; nIndex < nMachCount;++nIndex)
		{
			m_vecLineOrder.push_back(nMatchLine);
		}
		for (int nIndex = nMatchLine + 1; nIndex < nSecondMatchLine; ++nIndex)
		{
			m_vecLineOrder.push_back(nIndex);
		}
		for (int nIndex = 0; nIndex < nMatchLoopCount; ++nIndex)
		{
			m_vecLineOrder.push_back(10);
		}
	}
	return 0;
}

int CDiagnosisConditionalInstrcution::GetFJUMPPattern(UINT uControllerID, int nLineIndex, char* lpszVector, int nVectorBuffSize, char* lpszCMD, int nCMDBuffSize, BYTE* pbyTimeset, ULONG* pulOperand, BOOL* pbBRAMLine, BOOL* pbNextLineOtherMemory)
{
	auto iterController = m_mapTestInfo.find(uControllerID);
	if (m_mapTestInfo.end() == iterController)
	{
		return -2;
	}
	if (nullptr != lpszCMD)
	{
		strcpy_s(lpszCMD, nCMDBuffSize, "INC");
	}
	if (nullptr != pbyTimeset)
	{
		*pbyTimeset = 0;
	}
	if (nullptr != pulOperand)
	{
		*pulOperand = 0;
	}
	if (nullptr == pbBRAMLine)
	{
		*pbBRAMLine = TRUE;
	}
	if (nullptr != pbNextLineOtherMemory)
	{
		*pbNextLineOtherMemory = FALSE;
	}

	if (iterController->second.m_bDriveMode)
	{
		if (100 <= nLineIndex)
		{
			return -1;
		}

		if ((6 <= nLineIndex && 10 > nLineIndex) || (11 == nLineIndex))
		{
			if (nullptr != lpszVector)
			{
				strcpy_s(lpszVector, nVectorBuffSize, "1111111111111111");
			}
		}
		else
		{
			if (nullptr != lpszVector)
			{
				strcpy_s(lpszVector, nVectorBuffSize, "0000000000000000");
			}
		}
		return 0;
	}

	///<The instruction controller
	if (nullptr != lpszVector)
	{
		strcpy_s(lpszVector, nVectorBuffSize, "LLLLLLLLLLLLLLLL");
	}

	if (30 <= nLineIndex)
	{
		return -1;
	}
	else if (5 == nLineIndex)
	{
		if (nullptr != lpszCMD)
		{
			strcpy_s(lpszCMD, nCMDBuffSize, "FJUMP");
		}
		if (nullptr != pulOperand)
		{
			*pulOperand = 10;
		}
	}
	else if (11 == nLineIndex)
	{
		if (nullptr != lpszCMD)
		{
			strcpy_s(lpszCMD, nCMDBuffSize, "FJUMP");
		}
		if (nullptr != pulOperand)
		{
			*pulOperand = 20;
		}
	}
	if (0 == m_vecLineOrder.size())
	{
		for (int nIndex = 0; nIndex < 30;++nIndex)
		{
			m_vecLineOrder.push_back(nIndex);
			if (11 == nIndex)
			{
				nIndex = 19;
			}
		}
	}

	return 0;
}

int CDiagnosisConditionalInstrcution::GetMATCHPattern(UINT uControllerID, int nLineIndex, char* lpszVector, int nVectorBuffSize, char* lpszCMD, int nCMDBuffSize, BYTE* pbyTimeset, ULONG* pulOperand, BOOL* pbBRAMLine, BOOL* pbNextLineOtherMemory)
{
	auto iterController = m_mapTestInfo.find(uControllerID);
	if (m_mapTestInfo.end() == iterController)
	{
		return -2;
	}
	if (nullptr != lpszCMD)
	{
		strcpy_s(lpszCMD, nCMDBuffSize, "INC");
	}
	if (nullptr != pbyTimeset)
	{
		*pbyTimeset = 0;
	}
	if (nullptr != pulOperand)
	{
		*pulOperand = 0;
	}
	if (nullptr == pbBRAMLine)
	{
		*pbBRAMLine = TRUE;
	}
	if (nullptr != pbNextLineOtherMemory)
	{
		*pbNextLineOtherMemory = FALSE;
	}

	if (iterController->second.m_bDriveMode)
	{
		if (100 <= nLineIndex)
		{
			return -1;
		}
		if (nullptr != lpszVector)
		{
			strcpy_s(lpszVector, nVectorBuffSize, "0000000000000000");
		}
		if ((10 <= nLineIndex && 26 >= nLineIndex) || (31 <= nLineIndex && 47 >= nLineIndex))
		{
			if (nullptr != lpszVector)
			{
				strcpy_s(lpszVector, nVectorBuffSize, "1111111111111111");
			}
		}
		
		return 0;
	}

	///<The instruction controller
	if (nullptr != lpszVector)
	{
		strcpy_s(lpszVector, nVectorBuffSize, "LLLLLLLLLLLLLLLL");
	}

	int nCMDLineCount = 25;
	if (nCMDLineCount <= nLineIndex)
	{
		return -1;
	}
	int nMatchCount = 4;
	int nMatchLoopCount = nMatchCount + 1;
	const int nFirstMatchStartLine = 6;
	const int nFirstMatchStopLine = 10;
	const int nFirstMatchToLine = 12;
	const int nSecondMatchStartLine = 14;
	const int nSecondMatchStopLine = 18;
	const int nSecondMatchToLine = 20;
	switch (nLineIndex)
	{
	case 3:
	{
		if (nullptr != lpszCMD)
		{
			strcpy_s(lpszCMD, nCMDBuffSize, "SET_MCNT");
		}
		if (nullptr != pulOperand)
		{
			*pulOperand = nMatchCount;
		}
	}
	break;
	case 5:
	{
		if (nullptr != lpszCMD)
		{
			strcpy_s(lpszCMD, nCMDBuffSize, "SET_GLO");
		}
		if (nullptr != pulOperand)
		{
			*pulOperand = nFirstMatchToLine;
		}
	}
	break;
	case nFirstMatchStartLine:
	{
		if (nullptr != lpszCMD)
		{
			strcpy_s(lpszCMD, nCMDBuffSize, "MASKF");
		}
		if (nullptr != pulOperand)
		{
			*pulOperand = 0;
		}
	}
	break;
	case nFirstMatchStopLine:
	{
		if (nullptr != lpszCMD)
		{
			strcpy_s(lpszCMD, nCMDBuffSize, "MATCH");
		}
	}
	break;
	case nFirstMatchToLine:
	{
		if (nullptr != lpszCMD)
		{
			strcpy_s(lpszCMD, nCMDBuffSize, "SET_MCNT");
		}
		if (nullptr != pulOperand)
		{
			*pulOperand = nMatchCount;
		}
	}
	break;
	case nFirstMatchToLine + 1:
	{
		if (nullptr != lpszCMD)
		{
			strcpy_s(lpszCMD, nCMDBuffSize, "SET_GLO");
		}
		if (nullptr != pulOperand)
		{
			*pulOperand = nSecondMatchToLine;
		}
	}
	break;
	case nSecondMatchStartLine:
	{
		if (nullptr != lpszCMD)
		{
			strcpy_s(lpszCMD, nCMDBuffSize, "MASKF");
		}
		if (nullptr != pulOperand)
		{
			*pulOperand = 0;
		}
	}
	break;
	case nSecondMatchStopLine:
	{
		if (nullptr != lpszCMD)
		{
			strcpy_s(lpszCMD, nCMDBuffSize, "MATCH");
		}
	}
	break;
	default:
		break;
	}

	if (0 == m_vecLineOrder.size())
	{
		for (int nIndex = 0; nIndex < 6; ++nIndex)
		{
			m_vecLineOrder.push_back(nIndex);
		}
		for (int nIndex = 0; nIndex < nMatchLoopCount; ++nIndex)
		{
			for (int nMatchLine = nFirstMatchStartLine; nMatchLine <= nFirstMatchStopLine; ++nMatchLine)
			{
				m_vecLineOrder.push_back(nMatchLine);
			}
		}

		for (int nIndex = nFirstMatchToLine; nIndex < nSecondMatchStartLine; ++nIndex)
		{
			m_vecLineOrder.push_back(nIndex);
		}

		for (int nIndex = 0; nIndex < nMatchCount; ++nIndex)
		{
			for (int nMatchLine = nSecondMatchStartLine; nMatchLine <= nSecondMatchStopLine; ++nMatchLine)
			{
				m_vecLineOrder.push_back(nMatchLine);
			}
		}
		for (int nIndex = nSecondMatchStopLine + 1; nIndex < nCMDLineCount; ++nIndex)
		{
			m_vecLineOrder.push_back(nIndex);
		}
	}
	return 0;
}
