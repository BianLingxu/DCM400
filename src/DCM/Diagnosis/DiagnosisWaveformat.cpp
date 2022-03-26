#include "DiagnosisWaveformat.h"
#include "..\HDModule.h"
#ifdef _DEBUG
#include "..\Pattern.h"
#endif // _DEBUG

using namespace std;

#define TEST_FORMAT_COUNT 3
#define DRIVE_RATE 100
#define COMPARE_RATE (DRIVE_RATE / 2)

CDiagnosisWaveformat::CDiagnosisWaveformat()
{
	m_bCurQueueLoadVector = FALSE;
	m_nTestFormat = WAVE_FORMAT::NRZ;
}


CDiagnosisWaveformat::~CDiagnosisWaveformat()
{
}

const char* CDiagnosisWaveformat::GetSubItemName() const
{
	return GetFormat();
}

int CDiagnosisWaveformat::SetEnabled(int enable)
{
	return CMutualTest::SetEnabled(enable);
}

int CDiagnosisWaveformat::IsEnabled() const
{
	return CMutualTest::IsEnabled();
}

int CDiagnosisWaveformat::QueryInstance(const char* name, void** ptr)
{
	return 0;
}

void CDiagnosisWaveformat::Release()
{
}

const char* CDiagnosisWaveformat::Name() const
{
	return "Wave Format Diagnosis";
}

int CDiagnosisWaveformat::GetChildren(STSVector<IHDDoctorItem*>& children) const
{
	return 0;
}

int CDiagnosisWaveformat::Doctor(IHDReportDevice* pReportDevice)
{
	StartTimer();
	m_pReportDevice = pReportDevice;
	double dTimeConsume = 0;
	char lpszTimeUnits[4] = { 0 };

	const char* lpszBaseIndent = IndentFormat();
	string strFirstIndent = lpszBaseIndent + IndentChar();
	const char* lpszNextIndent = strFirstIndent.c_str();
	m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<WaveFormatDiagnosis>\n", lpszBaseIndent);
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

	m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s</WaveFormatDiagnosis>\n", lpszBaseIndent);

	return nRetVal;
}

bool CDiagnosisWaveformat::IsUserCheck() const
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

int CDiagnosisWaveformat::GetTestController(int nTestIndex, std::vector<UINT>& vecTestController, BOOL bSaveLog)
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
	for (int nFormatIndex = 0; nFormatIndex < TEST_FORMAT_COUNT; ++nFormatIndex)
	{
		if (nFormatIndex * nItemTestTimes + nItemTestTimes <= nTestIndex)
		{
			continue;
		}
		WAVE_FORMAT LatestFormat = m_nTestFormat;
		switch (nFormatIndex)
		{
		case 0:
			m_nTestFormat = WAVE_FORMAT::NRZ;
			ChannelStatus = CHANNEL_OUTPUT_STATUS::HIGH;
			break;
		case 1:
			m_nTestFormat = WAVE_FORMAT::RZ;
			ChannelStatus = CHANNEL_OUTPUT_STATUS::LOW;
			break;
		case 2:
			m_nTestFormat = WAVE_FORMAT::RO;
			ChannelStatus = CHANNEL_OUTPUT_STATUS::HIGH;
			break;
		default:
			return -1;
			break;
		}
		if (LatestFormat != m_nTestFormat)
		{
			m_mapBRAMFailLineNo.clear();
		}
		if (0 == nTestIndex % nItemTestTimes)
		{
			bSetChannelStatus = TRUE;
			SaveResult();
			StartTimer();
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<Format value='%s'>\n", lpszFirstIndent, GetFormat());
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
			if (bSetChannelStatus)
			{
				auto iterTestInfo = m_mapTestInfo.begin();
				while (m_mapTestInfo.end() != iterTestInfo)
				{
					if (iterTestInfo->second.m_bDriveMode)
					{
						SetChannelStatus(iterTestInfo->first, ChannelStatus);
					}
					++iterTestInfo;
				}
			}

			return 0;
		}
	}
	SaveResult();
	return -1;
}

int CDiagnosisWaveformat::GetMutualTestVector(UINT uControllerID, int uLineIndex, char* lpszVector, int nVectorBuffSize, char* lpszCMD, int nCMDBuffSize, BYTE* pbyTimeset, ULONG* pulOperand, BOOL* pbBRAMLine, BOOL* pbNextLineOtherMemory)
{
	auto iterController = m_mapTestInfo.find(uControllerID);
	if (m_mapTestInfo.end() == iterController)
	{
		return -1;
	}
	if (nullptr != pbyTimeset)
	{
		*pbyTimeset = 0;
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
	if (0 != GetVectorLineInfo(uLineIndex, iterController->second.m_bDriveMode, lpszVector, nVectorBuffSize))
	{
		return 1;
	}
	return 0;
}


int CDiagnosisWaveformat::GetSameVectorControllerType()
{
	return 2;
}

int CDiagnosisWaveformat::GetSameVectorController(int nTypeIndex, std::vector<UINT>& vecSameController)
{
	vecSameController.clear();
	if (2 <= nTypeIndex)
	{
		return -1;
	}
	BOOL bDriver = (0 == nTypeIndex) ? TRUE : FALSE;

	for (auto& TestInfo : m_mapTestInfo)
	{
		if (bDriver == TestInfo.second.m_bDriveMode)
		{
			vecSameController.push_back(TestInfo.first);
		}
	}
	return 0;
}

BOOL CDiagnosisWaveformat::IsReloadVector()
{
	return !m_bCurQueueLoadVector;
}

inline const char* CDiagnosisWaveformat::GetFormat() const
{
	switch (m_nTestFormat)
	{
	case WAVE_FORMAT::NRZ:
		return "NRZ";
		break;
		break;
	case WAVE_FORMAT::RZ:
		return "RZ";
		break;
	case WAVE_FORMAT::RO:
		return "RO";
		break;
	default:
		return "NRZ";
		break;
	}
}

int CDiagnosisWaveformat::GetVectorLineInfo(UINT uLineIndex, BOOL bDriveMode, char* lpszPattern, int nBuffSize)
{
	int nDrivePeriodCount = 200;
	int nComparePeriodCount = nDrivePeriodCount * DRIVE_RATE / COMPARE_RATE;
	USHORT usPassData = 0x0000;
	USHORT usFailData = 0xFFFF;
	if (bDriveMode && uLineIndex >= nDrivePeriodCount)
	{
		return 1;
	}
	else if(!bDriveMode && uLineIndex >= nComparePeriodCount)
	{
		return 1;
	}

	if (bDriveMode)
	{
		for (int nIndex = 0; nIndex < nDrivePeriodCount; ++nIndex)
		{
			if (nIndex == uLineIndex)
			{
				if (nullptr != lpszPattern)
				{
					if (0 == uLineIndex % 2)
					{
						strcpy_s(lpszPattern, nBuffSize, "0000000000000000");
					}
					else
					{
						strcpy_s(lpszPattern, nBuffSize, "1111111111111111");
					}
				}
				break;
			}
		}
		return 0;
	}
	for (int nIndex = 0; nIndex < nComparePeriodCount; ++nIndex)
	{
		if (nIndex == uLineIndex)
		{
			if (nullptr != lpszPattern)
			{
				if (0 == ((nIndex / 4) % 2))
				{
					strcpy_s(lpszPattern, nBuffSize, "HHHHHHHHHHHHHHHH");
				}
				else
				{
					strcpy_s(lpszPattern, nBuffSize, "LLLLLLLLLLLLLLLL");
				}
			}

			if (m_mapBRAMFailLineNo.end() == m_mapBRAMFailLineNo.find(uLineIndex))
			{
				 USHORT usCaptureData = 0;
				switch (m_nTestFormat)
				{
				case WAVE_FORMAT::NRZ:
				{
					switch (nIndex % 8)
					{
					case 0:
						usCaptureData = usFailData;
						break;
					case 1:
						usCaptureData = usFailData;
						break;
					case  2:
						usCaptureData = usPassData;
						break;
					case 3:
						usCaptureData = usPassData;
						break;
					case 4:
						usCaptureData = usPassData;
						break;
					case 5:
						usCaptureData = usPassData;
						break;
					case 6:
						usCaptureData = usFailData;
						break;
					case 7:
						usCaptureData = usFailData;
						break;
					default:
						break;
					}
				}
				break;
				case WAVE_FORMAT::RZ:
				{
					switch (nIndex % 8)
					{
					case 0:
						usCaptureData = usFailData;
						break;
					case 1:
						usCaptureData = usFailData;
						break;
					case  2:
						usCaptureData = usPassData;
						break;
					case 3:
						usCaptureData = usFailData;
						break;
					case 4:
						usCaptureData = usPassData;
						break;
					case 5:
						usCaptureData = usPassData;
						break;
					case 6:
						usCaptureData = usFailData;
						break;
					case 7:
						usCaptureData = usPassData;
						break;
					default:
						break;
					}
				}
				break;
				case WAVE_FORMAT::RO:
				{
					switch (nIndex % 8)
					{
					case 0:
						usCaptureData = usFailData;
						break;
					case 1:
						usCaptureData = usPassData;
						break;
					case  2:
						usCaptureData = usPassData;
						break;
					case 3:
						usCaptureData = usPassData;
						break;
					case 4:
						usCaptureData = usPassData;
						break;
					case 5:
						usCaptureData = usFailData;
						break;
					case 6:
						usCaptureData = usFailData;
						break;
					case 7:
						usCaptureData = usFailData;
						break;
					default:
						break;
					}
				}
				break;
				default:
					break;
				}
				if (usCaptureData != usPassData)
				{
					m_mapBRAMFailLineNo.insert(make_pair(uLineIndex, usCaptureData));
				}
			}
		}
	}
	return 0;
}

void CDiagnosisWaveformat::SaveResult()
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

	auto iterController = m_mapTestResult.begin();
	BOOL bAllPass = TRUE;
	while (m_mapTestResult.end() != iterController)
	{
		if (0 != iterController->second)
		{
			bAllPass = FALSE;
			break;
		}
		++iterController;
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
	m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s</Format>\n", lpszFirstIndent);
	m_mapTestResult.clear();
}

BOOL CDiagnosisWaveformat::Stop()
{
	return FALSE;
}

int CDiagnosisWaveformat::CheckResult(UINT uControllerID, const std::map<int, USHORT>& mapBRAMFailLineNo, const std::map<int, USHORT>& mapDRAMFailLineNo, int nFailCount)
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
	BYTE bySlotNo = 0;
	BYTE byBoardControllerIndex = 0;

	string strFirstIndent = IndentFormat() + IndentChar() + IndentChar();
	string strSecondIndent = strFirstIndent + IndentChar();
	const char* lpszFirstIndent = strFirstIndent.c_str();
	const char* lpszSecondIndent = strSecondIndent.c_str();

	UINT uTestControllerID = iterController->second.m_uRelateControllerID;
	bySlotNo = HDModule::Instance()->ID2Board(uTestControllerID, byBoardControllerIndex);

	if (nullptr != m_pReportDevice)
	{
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<controller value='%d, slot value = %d'>\n", lpszFirstIndent, byBoardControllerIndex, bySlotNo);
	}


#ifdef _DEBUG
// 	BYTE byDriverSlotNo = 0;
// 	BYTE byDriverBoardControllerIndex = 0;
// 	UINT uDriverControllerID = iterController->second.m_uRelateControllerID;
// 	byDriverSlotNo = HDModule::Instance()->ID2Board(uDriverControllerID, byDriverBoardControllerIndex);
// 	CHardwareFunction DriverHardware(byDriverSlotNo);
// 	DriverHardware.SetControllerIndex(byDriverBoardControllerIndex);
// 	CPattern DriverPattern(DriverHardware);
// 	char lpszDriverPattern[500][17] = { 0 };
// 	DriverPattern.ReadPattern(TRUE, 0, 500, lpszDriverPattern);
// 
// 	USHORT usDriverTimeset[500] = { 0 };
// 	DriverHardware.ReadDataMemory(MEM_TYPE::BRAM, DATA_TYPE::CMD, 0, 500, usDriverTimeset);
// 	for (int nIndex = 0; nIndex < 500; ++nIndex)
// 	{
// 		usDriverTimeset[nIndex] = usDriverTimeset[nIndex] >> 5 & 0xFF;
// 	}
// 
// 
// 	BYTE byCompareSlotNo = 0;
// 	BYTE byCompareBoardControllerIndex = 0;
// 	byCompareSlotNo = HDModule::Instance()->ID2Board(uControllerID, byCompareBoardControllerIndex);
// 	CHardwareFunction CompareHardware(byCompareSlotNo);
// 	CompareHardware.SetControllerIndex(byCompareBoardControllerIndex);
// 	CPattern ComparePattern(CompareHardware);
// 	char lpszComparePattern[500][17] = { 0 };
// 	ComparePattern.ReadPattern(TRUE, 0, 500, lpszComparePattern);
// 	USHORT usCompareTimeset[500] = { 0 };
// 	CompareHardware.ReadDataMemory(MEM_TYPE::BRAM, DATA_TYPE::CMD, 0, 500, usCompareTimeset);
// 	for (int nIndex = 0; nIndex < 500; ++nIndex)
// 	{
// 		usCompareTimeset[nIndex] = usCompareTimeset[nIndex] >> 5 & 0xFF;
// 	}
#endif // _DEBUG



	StartTimer();
	double dTimeConsume = 0;
	char lpszTimeUnits[4] = { 0 };

	int nRetVal = 0;
	int nFailItemCount = 0;
	if (mapBRAMFailLineNo != m_mapBRAMFailLineNo)
	{
		auto iterRealFailLineNo = mapBRAMFailLineNo.begin();
		auto iterExpectFailLineNo = m_mapBRAMFailLineNo.begin();
		while (mapBRAMFailLineNo.end() != iterRealFailLineNo && m_mapBRAMFailLineNo.end() != iterExpectFailLineNo)
		{
			if (iterRealFailLineNo->first != iterExpectFailLineNo->first || iterRealFailLineNo->second != iterExpectFailLineNo->second)
			{
				nRetVal = -1;
				char lpszValue[128] = { 0 };
				sprintf_s(lpszValue, sizeof(lpszValue), "value='expect addr=0x%X Real addr=0x%X Expect=0x%X Real=0x%X'", iterExpectFailLineNo->first, iterRealFailLineNo->first, iterExpectFailLineNo->second, iterRealFailLineNo->second);

				m_pReportDevice->PrintfToDataLog(IHDReportDevice::Error, "%s<data retValue='false' %s/>\n", lpszSecondIndent, lpszValue);
				++nFailItemCount;

				if (ERROR_PRINT <= nFailItemCount)
				{
					break;
				}
			}
			++iterRealFailLineNo;
			++iterExpectFailLineNo;
		}
		if (mapBRAMFailLineNo.size() != m_mapBRAMFailLineNo.size())
		{
			nRetVal = -1;
			char lpszValue[128] = { 0 };
			while (mapBRAMFailLineNo.end() != iterRealFailLineNo)
			{
				sprintf_s(lpszValue, sizeof(lpszValue), "value='expect addr= - Real addr=0x%X Expect= - Real=0x%X'",
					iterRealFailLineNo->first, iterRealFailLineNo->second);
				++iterRealFailLineNo;
			}
			while (m_mapBRAMFailLineNo.end() != iterExpectFailLineNo)
			{
				sprintf_s(lpszValue, sizeof(lpszValue), "value=' expect addr=0x%X Real addr= - Expect=0x%X Real= -'",
					iterExpectFailLineNo->first, iterExpectFailLineNo->second);
				++iterExpectFailLineNo;
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

	// Ensure only print one information per controller.
	if (nullptr != m_pReportDevice)
	{
		auto iterPrint = m_mapTestResult.find(uTestControllerID);
		if (m_mapTestResult.end() == iterPrint)
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
			bySlotNo = HDModule::Instance()->ID2Board(uTestControllerID, byBoardControllerIndex);
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

void CDiagnosisWaveformat::GetCheckDataController(std::vector<UINT>& vecCheckController)
{
	vecCheckController.clear();

	auto iterController = m_mapTestInfo.begin();
	while (m_mapTestInfo.end() != iterController)
	{
		if (!iterController->second.m_bDriveMode)
		{
			vecCheckController.push_back(iterController->first);
		}
		++iterController;
	}
}

int CDiagnosisWaveformat::GetTimesetSetting(UINT uControllerID, std::map<BYTE, TIMESET_VALUE>& mapEdgeValue)
{
	mapEdgeValue.clear();
	auto iterController = m_mapTestInfo.find(uControllerID);
	if (m_mapTestInfo.end() == iterController)
	{
		return -1;
	}
	TIMESET_VALUE TimesetValue;
	if (iterController->second.m_bDriveMode)
	{
		TimesetValue.m_dPeriod = DRIVE_RATE;
		TimesetValue.m_dEgde[0] = 5;
		TimesetValue.m_dEgde[1] = DRIVE_RATE / 2 + 5;
		TimesetValue.m_dEgde[2] = 5;
		TimesetValue.m_dEgde[3] = DRIVE_RATE / 2 + 5;
		TimesetValue.m_dEgde[4] = DRIVE_RATE * 0.6;
		TimesetValue.m_dEgde[5] = DRIVE_RATE * 0.8;
		switch (m_nTestFormat)
		{
		case WAVE_FORMAT::NRZ:
			//NRZ
			TimesetValue.m_WaveFormat = WAVE_FORMAT::NRZ;
			break;
		case WAVE_FORMAT::RZ:
			//RZ
			TimesetValue.m_WaveFormat = WAVE_FORMAT::RZ;
			break;
		case  WAVE_FORMAT::RO:
			//RO
			TimesetValue.m_WaveFormat = WAVE_FORMAT::RO;
			break;
		default:
			//NRZ
			TimesetValue.m_WaveFormat = WAVE_FORMAT::NRZ;
			break;
		}

	}
	else
	{
		TimesetValue.m_dPeriod = COMPARE_RATE;
		TimesetValue.m_dEgde[0] = 0;
		TimesetValue.m_dEgde[1] = COMPARE_RATE / 2;
		TimesetValue.m_dEgde[2] = 0;
		TimesetValue.m_dEgde[3] = COMPARE_RATE / 2;
		TimesetValue.m_dEgde[4] = COMPARE_RATE  * 0.6;
		TimesetValue.m_dEgde[5] = COMPARE_RATE * 0.8;
		TimesetValue.m_WaveFormat = WAVE_FORMAT::NRZ;
	}
	for (BYTE byTimesetIndex = 0; byTimesetIndex < HDModule::SplitCount; ++byTimesetIndex)
	{
		mapEdgeValue.insert(make_pair(byTimesetIndex, TimesetValue));
	}
	return 0;
}

int CDiagnosisWaveformat::GetPinLevel(UINT uControllerID, double* pdVIH, double* pdVIL, double* pdVOH, double* pdVOL)
{
	if (m_mapTestInfo.end() == m_mapTestInfo.find(uControllerID))
	{
		return -1;
	}
	if (nullptr == pdVIH || nullptr == pdVIL || nullptr == pdVOH || nullptr == pdVOL)
	{
		return -2;
	}
	*pdVIH = 3;
	*pdVIL = 0;
	*pdVOH = 1.5;
	*pdVOL = 0.8;
	return 0;
}

void CDiagnosisWaveformat::GetVectorStartLine(UINT* puBRAMStartLine, UINT* puDRAMStartLine)
{
	if (nullptr == puBRAMStartLine || nullptr == puDRAMStartLine)
	{
		return;
	}
	*puBRAMStartLine = 0;
	*puDRAMStartLine = 0;
}
