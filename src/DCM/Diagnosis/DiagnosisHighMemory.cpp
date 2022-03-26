#include "DiagnosisHighMemory.h"
#include "IHDReportDevice.h"
#include "..\HDModule.h"
#include "..\HardwareFunction.h"
#include "..\Pattern.h"
using namespace std;

//#define _UNBIND 1

CDiagnosisHighMemory::CDiagnosisHighMemory()
{}

CDiagnosisHighMemory::~CDiagnosisHighMemory()
{}

int CDiagnosisHighMemory::QueryInstance(const char * name, void ** ptr)
{
    return -1;
}

void CDiagnosisHighMemory::Release()
{}

const char * CDiagnosisHighMemory::Name() const
{
    return "High Speed Memory Diagnosis";
}

int CDiagnosisHighMemory::GetChildren(STSVector<IHDDoctorItem *> & children) const
{
    return 0;
}

int CDiagnosisHighMemory::Doctor(IHDReportDevice* pReportDevice)
{
	StartTimer();
	m_pReportDevice = pReportDevice;
	double dTimeConsume = 0;
	char lpszTimeUnits[4] = { 0 };

	const char* lpszBaseIndent = IndentFormat();
    std::string strIndentFirstFormat = lpszBaseIndent + IndentChar();
	const char* lpszNextBaseIndent = strIndentFirstFormat.c_str();
	m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<MemoryHighSpeedDiagnosis>\n", lpszBaseIndent);
    int nFailCount = 0;
	do
	{
		m_pReportDevice->PrintfToUi(" BRAM\n");
		if (0 != BRAMDiagnosis(lpszNextBaseIndent))
		{
			++nFailCount;
			if (1 != m_pReportDevice->IsFailContinue())
			{
				break;
			}
		}

		if (1 == m_pReportDevice->IsStop())
		{
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<UserStop value='Next=DRAM Diagnosis'/>\n", lpszNextBaseIndent);
			break;
  		}
		m_pReportDevice->PrintfToUi(" DRAM\n");
		if (0 != DRAMDiagnosis(lpszNextBaseIndent))
		{
			++nFailCount;
			if (1 != m_pReportDevice->IsFailContinue())
			{
				break;
			}
		}
	} while (false);
    int nRetVal = -1;
    if (0 == nFailCount)
    {
        nRetVal = 0;
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='true'/>\n", lpszNextBaseIndent);
    }
    else
    {
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='false'/>\n", lpszNextBaseIndent);
    }

	dTimeConsume = StopTimer( lpszTimeUnits, sizeof(lpszTimeUnits));
	m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<Time value='%.3f%s'/>\n", lpszNextBaseIndent, dTimeConsume, lpszTimeUnits);

	m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s</MemoryHighSpeedDiagnosis>\n", lpszBaseIndent);
    return nRetVal;
}

#define GET_DATA(x) ((x) | ((x) << 10))

int CDiagnosisHighMemory::BRAMDiagnosis(const char* lpszBaseIndent)
{
	StartTimer();
	double dTimeConsume = 0;
	char lpszTimeUnits[4] = { 0 };

	std::string strFirstIndent = lpszBaseIndent + IndentChar();
	std::string strSecondIndent = strFirstIndent + IndentChar();
	std::string strThirdIndent = strSecondIndent + IndentChar();
	const char* lpszFirstIndent = strFirstIndent.c_str();
	const char* lpszSecondIndent = strSecondIndent.c_str();
	const char* lpszThirdIndent = strThirdIndent.c_str();

	m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<BRAMHighSpeedTest>\n", lpszBaseIndent);

	BOOL bAllPass = TRUE;
	int nRetVal = -1;

	USHORT* pusExpectData = nullptr;
	map<int, USHORT> mapBRAMExpectFailLine;
	set<UINT> setFailController;
	auto iterController = setFailController.begin();
	UINT uControllerID = 0;
	BYTE byBoardControllerIndex = 0;
	BYTE bySlotNo = 0;
	USHORT usTestControllerCount = m_vecEnableController.size();
	do
	{
		if (0 >= usTestControllerCount)
		{
			nRetVal = 0;
			break;
		}

		try
		{
			UINT uCurLineIndex = 0;
			pusExpectData = new USHORT[HDModule::BRAMLineCount];
			memset(pusExpectData, 0, HDModule::BRAMLineCount * sizeof(USHORT));
			for (UINT uLineIndex = 0; uLineIndex < HDModule::BRAMLineCount; uLineIndex += 0x800)
			{
				uCurLineIndex = uLineIndex;
				for (UINT uFailIndex = 0; uFailIndex < BRAM_MAX_SAVE_FAIL_LINE_COUNT / 0x800; ++uFailIndex)
				{
					if (BRAM_MAX_SAVE_FAIL_LINE_COUNT <= mapBRAMExpectFailLine.size() )
					{
						break;
					}
					pusExpectData[uCurLineIndex] = GET_DATA(uFailIndex + 1);
					mapBRAMExpectFailLine.insert(make_pair(uCurLineIndex, pusExpectData[uCurLineIndex]));
					++uCurLineIndex;
				}
			}
		}
		catch (const std::exception&)
		{
			break;
		}

		std::vector<double> vecPeriod;
		vecPeriod.push_back(12);
		vecPeriod.push_back(16);
		vecPeriod.push_back(27);
		vecPeriod.push_back(30);
		vecPeriod.push_back(32);
		vecPeriod.push_back(35);
		vecPeriod.push_back(37);
		vecPeriod.push_back(40);
		vecPeriod.push_back(50);
		vecPeriod.push_back(60);
		vecPeriod.push_back(70);
		vecPeriod.push_back(80);
		vecPeriod.push_back(90);
		vecPeriod.push_back(100);
		vecPeriod.push_back(300); 
		vecPeriod.push_back(1300); 
		vecPeriod.push_back(6e3);
		vecPeriod.push_back(13e3);
		vecPeriod.push_back(25e3);
		vecPeriod.push_back(59e3);
		vecPeriod.push_back(600e3);
		vecPeriod.push_back(4e6);
		BOOL bMemPas = TRUE;
		BOOL bPeriodPass = TRUE;
		int nPeriodCount = vecPeriod.size();
		double dPeriod = 0;
		const BYTE byMemCount = 3;
		const char* lpszMemType[byMemCount] = { "FM","MM","IOM" };
		DATA_TYPE DataType[byMemCount] = { DATA_TYPE::FM, DATA_TYPE::MM, DATA_TYPE::IOM };
		int nErrorCount = 0;
		for (auto dPeriod : vecPeriod)
		{
			double dShowPeriod = GetPeriodUnits(dPeriod, lpszTimeUnits, sizeof(lpszTimeUnits));
			if (m_pReportDevice->IsStop())
			{
				m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<UserStop value='NextPeriod=%.1f%s'/>\n", lpszFirstIndent, dShowPeriod, lpszTimeUnits);
				break;
			}

			StartTimer();
			bPeriodPass = TRUE;

			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<Period value='%.1f%s'>\n", lpszFirstIndent, dShowPeriod, lpszTimeUnits);

			for (BYTE byMemTypeIndex = 0; byMemTypeIndex < byMemCount; ++byMemTypeIndex)
			{
				if (m_pReportDevice->IsStop())
				{
					m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<UserStop value='NextMemory=%s'/>\n", lpszSecondIndent, lpszMemType[byMemTypeIndex]);
					break;
				}

				StartTimer();

				m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<MemType value='%s'>\n", lpszSecondIndent, lpszMemType[byMemTypeIndex]);

				BRAMHighSpeedCheck(dPeriod, DataType[byMemTypeIndex], 0, HDModule::BRAMLineCount, pusExpectData);

				nRetVal = BRAMResultCheck(lpszThirdIndent, mapBRAMExpectFailLine);
				
				if (0 == nRetVal)
				{
					m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='true'/>\n", lpszThirdIndent);
				}
				else
				{
					m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='false'/>\n", lpszThirdIndent);
				}
				dTimeConsume = StopTimer( lpszTimeUnits, sizeof(lpszTimeUnits));
				m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<Time value='%.3f%s'/>\n", lpszThirdIndent, dTimeConsume, lpszTimeUnits);

				m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s</MemType>\n", lpszSecondIndent);
			}
			if (bPeriodPass)
			{
				m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='true'/>\n", lpszSecondIndent);
			}
			else
			{
				bAllPass = FALSE;
				m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='false'/>\n", lpszSecondIndent);
			}
			dTimeConsume = StopTimer( lpszTimeUnits, sizeof(lpszTimeUnits));
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<Time value='%.3f%s'/>\n", lpszSecondIndent, dTimeConsume, lpszTimeUnits);

			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s</Period>\n", lpszFirstIndent);
		}
	} while (false);

	if (nullptr != pusExpectData)
	{
		delete[] pusExpectData;
		pusExpectData = nullptr;
	}

	ShowUIResult();

	if (bAllPass)
	{
		nRetVal = 0;
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='true'/>\n", lpszFirstIndent);
	}
	else
	{
		nRetVal = -1;
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='false'/>\n", lpszFirstIndent);
	}
	dTimeConsume = StopTimer( lpszTimeUnits, sizeof(lpszTimeUnits));
	m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<Time value='%.3f%s'/>\n", lpszFirstIndent, dTimeConsume, lpszTimeUnits);

	m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s</BRAMHighSpeedTest>\n", lpszBaseIndent);
	return nRetVal;
}

int CDiagnosisHighMemory::DRAMDiagnosis(const char* lpszBaseIndnet)
{
	StartTimer();
	double dTimeConsume = 0;
	char lpszTimeUnits[4] = { 0 };

	std::string strNextIndent = lpszBaseIndnet + IndentChar();
	const char* lpszNextIndent = strNextIndent.c_str();
	m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<DRAMHighSpeedTest>\n", lpszBaseIndnet);
	int nRetVal = -1;

	BOOL bAllPass = TRUE;
	do
	{
		nRetVal = DRAMMultiSwitchDiagnosis(lpszNextIndent);
		if (0 != nRetVal)
		{
			if (1 != m_pReportDevice->IsFailContinue())
			{
				break;
			}
			bAllPass = FALSE;
		}
		if (1 == m_pReportDevice->IsStop())
		{
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<UserStop value='Next=DRAMStability'/>\n", lpszNextIndent);
			break;
		}

		nRetVal = DRAMStabilityDiagnosis(lpszNextIndent);
		if (0 != nRetVal)
		{
			if (1 != m_pReportDevice->IsFailContinue())
			{
				break;
			}
			bAllPass = FALSE;
		}

		if (1 == m_pReportDevice->IsStop())
		{
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<UserStop value='Next=DRAMMultiLenght'/>\n", lpszNextIndent);
			break;
		}
		nRetVal = DRAMMultiLengthDiagnosis(lpszNextIndent);
		if (0 != nRetVal)
		{
			if (1 != m_pReportDevice->IsFailContinue())
			{
				break;
			}
			bAllPass = FALSE;
		}

		if (1 == m_pReportDevice->IsStop())
		{
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<UserStop value='Next=DRAMMultiRate'/>\n", lpszNextIndent);
			break;
		}

		nRetVal = DRAMMultiRateDiagnosis(lpszNextIndent);
		if (0 != nRetVal)
		{
			bAllPass = FALSE;
			if (1 != m_pReportDevice->IsFailContinue())
			{
				break;
			}
		}

		if (1 == m_pReportDevice->IsStop())
		{
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<UserStop value='Next=DRAMFast'/>\n", lpszNextIndent);
			break;
		}
		nRetVal = DRAMFastDiagnosis(lpszNextIndent);
		if (0 != nRetVal)
		{
			if (1 != m_pReportDevice->IsFailContinue())
			{
				break;
			}
			bAllPass = FALSE;
		}

		if (1 == m_pReportDevice->IsStop())
		{
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<UserStop value='Next=DRAMAllPage'/>\n", lpszNextIndent);
			break;
		}
		nRetVal = DRAMAllPageDiagnosis(lpszNextIndent);
		if (0 != nRetVal)
		{
			bAllPass = FALSE;
			if (1 != m_pReportDevice->IsFailContinue())
			{
				break;
			}
		}
		if (1 == m_pReportDevice->IsStop())
		{
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<UserStop value='Next=AllPageRan'/>\n", lpszNextIndent);
			break;
		}
		nRetVal = DRAMAllPageRanDiagnosis(lpszNextIndent);
		if (0 != nRetVal)
		{
			bAllPass = FALSE;
			if (1 != m_pReportDevice->IsFailContinue())
			{
				break;
			}
		}
	} while (false);
	
	ShowUIResult();

	if (bAllPass)
	{
		nRetVal = 0;
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='true'/>\n", lpszNextIndent);
	}
	else
	{
		nRetVal = -1;
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='false'/>\n", lpszNextIndent);
	}

	dTimeConsume = StopTimer( lpszTimeUnits, sizeof(lpszTimeUnits));
	m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<Time value='%.3f%s'/>\n", lpszNextIndent, dTimeConsume, lpszTimeUnits);
	m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s</DRAMHighSpeedTest>\n", lpszBaseIndnet);
	return nRetVal;
}

int CDiagnosisHighMemory::BRAMHighSpeedCheck(double dPeriod, DATA_TYPE DataType, int nBRAMStartAddr, int nBRAMDepth, const USHORT* const pusBRAMData)
{
	USHORT usTestControllerCount = m_vecEnableController.size();
	if (0 >= usTestControllerCount)
	{
		return -1;
	}
	if (HDModule::BRAMLineCount < nBRAMStartAddr + nBRAMDepth)
	{
		return -2;
	}
	else if (nullptr == pusBRAMData)
	{
		return -3;
	}
	UINT uControllerID = m_vecEnableController[0];

	USHORT* pusCommand = nullptr;
	try
	{
		pusCommand = new USHORT[nBRAMDepth];
		memset(pusCommand, 0, nBRAMDepth * sizeof(USHORT));
	}
	catch (const std::exception&)
	{
		return -4;
	}

	Bind(m_vecEnableController, uControllerID);

	vector<USHORT> vecChannel;
	for (USHORT usChannel = 0; usChannel < HDModule::ChannelCountPerControl; ++usChannel)
	{
		vecChannel.push_back(usChannel);
	}

	double dEdge[EDGE_COUNT] = { dPeriod / 4,dPeriod * 3 / 4, dPeriod / 4, dPeriod * 3 / 4, dPeriod / 2, dPeriod * 3 / 4 };

	CHardwareFunction* pHardware = GetHardware(uControllerID);

	pHardware->WriteDataMemory(MEM_TYPE::BRAM, DataType, 0, nBRAMDepth, pusBRAMData);
	pHardware->WriteDataMemory(MEM_TYPE::BRAM, DATA_TYPE::CMD, 0, nBRAMDepth, pusCommand);

	pHardware->SetPeriod(0, dPeriod);
	pHardware->SetEdge(vecChannel, 0, dEdge, WAVE_FORMAT::NRZ, IO_FORMAT::NRZ, COMPARE_MODE::EDGE);
	pHardware->SetRunParameter(0, nBRAMDepth - 1);
	pHardware->SetPatternMode(FALSE, DataType, FALSE, FALSE);

	ClearBind();

	pHardware->SynRun();


	WaitStop();

	if (nullptr != pusCommand)
	{
		delete[] pusCommand;
		pusCommand = nullptr;
	}

	return 0;
}

int CDiagnosisHighMemory::BRAMResultCheck(const char* lpszBaseIndent, const map<int, USHORT>& mapExpectFailLine)
{
	double dTimeConsume = 0;
	char lpszTimeUnits[8] = { 0 };
	string strNextIndent = lpszBaseIndent + IndentChar();
	const char* lpszNextIndent = strNextIndent.c_str();

	USHORT usControllerCount = m_vecEnableController.size();
	BYTE bySlotNo = 0;
	BYTE byBoardControllerIndex = 0;
	BOOL bAllPass = TRUE;
	map<int, USHORT> mapBRAMFailLine;
	map<int, USHORT> mapDRAMFailLine;

	vector<CHardwareFunction::DATA_RESULT> vecBRAMFaiLLineNo;
	vector<CHardwareFunction::DATA_RESULT> vecDRAMFaiLLineNo;
	auto CopyMap = [&](BOOL bBRAM)
	{
		auto& vecFail = bBRAM ? vecBRAMFaiLLineNo : vecDRAMFaiLLineNo;
		auto& mapFail = bBRAM ? mapBRAMFailLine : mapDRAMFailLine;
		for (auto& Data : vecFail)
		{
			mapFail.insert(make_pair(Data.m_nLineNo, Data.m_usData));
		}
	};

	for (auto uControllerID : m_vecEnableController)
	{
		StartTimer();
		int nErrorCount = 0;
		BOOL bControllerPass = TRUE;
		bySlotNo = HDModule::Instance()->ID2Board(uControllerID, byBoardControllerIndex);

		CHardwareFunction* pHardware = GetHardware(uControllerID);
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<controller value='%d, slot value = %d'>\n", lpszBaseIndent, byBoardControllerIndex, bySlotNo);

		pHardware->GetFailData(vecBRAMFaiLLineNo, vecDRAMFaiLLineNo);

		CopyMap(TRUE);
		CopyMap(FALSE);

		if (mapBRAMFailLine != mapExpectFailLine)
		{
			bAllPass = FALSE;
			bControllerPass = FALSE;
			auto iterRealFailLine = mapBRAMFailLine.begin();
			auto iterExpectFailLine = mapExpectFailLine.begin();
			while (mapBRAMFailLine.end() != iterRealFailLine && mapExpectFailLine.end() != iterExpectFailLine)
			{
				if (iterRealFailLine->first != iterExpectFailLine->first || iterRealFailLine->second != iterExpectFailLine->second)
				{
					m_pReportDevice->PrintfToDataLog(IHDReportDevice::Error, "%s<data retValue='false' value='ExpectAddr=0x%X, ReadlAddr=0x%X, cmp_data=0x%X, rdata=0x%X'/>\n",
						lpszNextIndent, iterExpectFailLine->first, iterRealFailLine->first, iterExpectFailLine->second, iterRealFailLine->second);
					
					if (ERROR_PRINT < nErrorCount++)
					{
						break;
					}
				}
				++iterRealFailLine;
				++iterExpectFailLine;
			}
			nErrorCount = 0;

			if (mapBRAMFailLine.size() != mapExpectFailLine.size())
			{
				m_pReportDevice->PrintfToDataLog(IHDReportDevice::Error, "%s<data retValue='false' value='ExpectFailCount=0x%X, RealFailCount=0x%X'/>\n", lpszNextIndent,
					mapBRAMFailLine.size(), mapExpectFailLine.size());

				if (ERROR_PRINT < nErrorCount++)
				{
					break;
				}
			}
		}

		if (bControllerPass)
		{
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='true'/>\n", lpszNextIndent);

		}
		else
		{
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='false'/>\n", lpszNextIndent);

			SaveFailController(uControllerID);
		}
		dTimeConsume = StopTimer(lpszTimeUnits, sizeof(lpszTimeUnits));
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<Time value='%.3f%s'/>\n", lpszNextIndent, dTimeConsume, lpszTimeUnits);

		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s</controller>\n", lpszBaseIndent);
	}
	if (bAllPass)
	{
		return 0;
	}
	else
	{
		return -1;
	}
}

int CDiagnosisHighMemory::DRAMHighSpeedCheck(double dPeriod, DATA_TYPE DataType, int nSwitchTimes, const UINT* const puBRAMSwitchLineOffset, 
	const UINT* const puDRAMSwitchLineOffset, UINT uBRAMDepth, const USHORT* const pusBRAMData, UINT uDRAMStartAddr, UINT uDRAMDepth, const USHORT* const pusDRAMData)
{
	USHORT uTestControllersCount = m_vecEnableController.size();
	if (0 >= uTestControllersCount)
	{
		return -1;
	}
	else if (nullptr == pusBRAMData || 4 > uBRAMDepth)
	{
		return -2;
	}
	else if (nullptr == pusDRAMData || 0 == uDRAMDepth)
	{
		return -3;
	}
	else if (nullptr == puBRAMSwitchLineOffset || nullptr == puDRAMSwitchLineOffset || 0 == nSwitchTimes)
	{
		return -4;
	}
	map<UINT, BYTE> mapBRAMSwitchLine;
	map<UINT, BYTE> mapDRAMSwitchLine;
	for (int nLineIndex = 0; nLineIndex < nSwitchTimes; ++nLineIndex)
	{
		if (puBRAMSwitchLineOffset[nLineIndex] >= uBRAMDepth || puDRAMSwitchLineOffset[nLineIndex] > uDRAMDepth)
		{
			return  -5;
		}
		else if (nLineIndex + 1 == nSwitchTimes && puDRAMSwitchLineOffset[nLineIndex] != uDRAMDepth - 1)
		{
			return -5;
		}
		mapBRAMSwitchLine.insert(make_pair(puBRAMSwitchLineOffset[nLineIndex], 0));
		mapDRAMSwitchLine.insert(make_pair(puDRAMSwitchLineOffset[nLineIndex], 0));
	}
	if (mapBRAMSwitchLine.size() != nSwitchTimes || mapDRAMSwitchLine.size() != nSwitchTimes)
	{
		return -6;
	}
	UINT uControllerID = m_vecEnableController[0];

	CHardwareFunction* pHardware = GetHardware(uControllerID);

	CPattern Pattern(*pHardware);

	USHORT* pusBRAMCommand = nullptr;
	USHORT* pusDRAMCommand = nullptr;

	try
	{
		pusBRAMCommand = new USHORT[uBRAMDepth];
		pusDRAMCommand = new USHORT[uDRAMDepth];
		memset(pusBRAMCommand, 0, uBRAMDepth * sizeof(USHORT));
		memset(pusDRAMCommand, 0, uDRAMDepth * sizeof(USHORT));
	}
	catch (const std::exception&)
	{
		return -7;
	}

	USHORT* pusCommand = nullptr;
	UINT uPatternLineCount = 0;
	map<UINT, BYTE>* pmapSwitchLine = nullptr;
	BOOL bSwitch = TRUE;
	BOOL bBRAM = TRUE;
	for (int nPatternMemIndex = 0; nPatternMemIndex < 2; ++nPatternMemIndex)
	{
		if (0 == nPatternMemIndex)
		{
			pusCommand = pusBRAMCommand;
			uPatternLineCount = uBRAMDepth;
			pmapSwitchLine = &mapBRAMSwitchLine;
			bBRAM = TRUE;
		}
		else
		{
			pusCommand = pusDRAMCommand;
			uPatternLineCount = uDRAMDepth;
			pmapSwitchLine = &mapDRAMSwitchLine;
			bBRAM = FALSE;
		}
		for (UINT uIndex = 0; uIndex < uPatternLineCount; ++uIndex)
		{
			if (pmapSwitchLine->end() != pmapSwitchLine->find(uIndex))
			{
				bSwitch = TRUE;
			}
			else
			{
				bSwitch = FALSE;
			}
			pusCommand[uIndex] = Pattern.GetCommand(bBRAM, 0, FALSE, bSwitch);
		}
	}

	double dEdge[6] = { dPeriod / 4,dPeriod * 3 / 4, dPeriod / 4, dPeriod * 3 / 4, dPeriod / 2, dPeriod * 3 / 4 };

	if (100 < dPeriod)
	{
		dEdge[4] = dPeriod / 2;
		dEdge[5] = dPeriod / 2 + 20;
	}

	vector<USHORT> vecChannel;
	for (USHORT usChannel = 0; usChannel < HDModule::ChannelCountPerControl; ++usChannel)
	{
		vecChannel.push_back(usChannel);
	}

	Bind(m_vecEnableController, uControllerID);

	pHardware = GetHardware(uControllerID);
	pHardware->SetPeriod(0, dPeriod);	//设速率，只写第“0”分离级

	pHardware->SetEdge(vecChannel, 0, dEdge, WAVE_FORMAT::NRZ, IO_FORMAT::NRZ, COMPARE_MODE::EDGE);

	pHardware->WriteDataMemory(MEM_TYPE::BRAM, DataType, 0, uBRAMDepth, pusBRAMData);
	pHardware->WriteDataMemory(MEM_TYPE::BRAM, DATA_TYPE::CMD, 0, uBRAMDepth, pusBRAMCommand);
	pHardware->WriteDataMemory(MEM_TYPE::DRAM, DataType, uDRAMStartAddr, uDRAMDepth, pusDRAMData);
	pHardware->WriteDataMemory(MEM_TYPE::DRAM, DATA_TYPE::CMD, uDRAMStartAddr, uDRAMDepth, pusDRAMCommand);

	pHardware->SetRunParameter(0, uBRAMDepth - 1, TRUE, uDRAMStartAddr);
	pHardware->SetPatternMode(FALSE, DataType, TRUE, FALSE);

	ClearBind();
	pHardware->SynRun();

	WaitStop();

	if (nullptr != pusBRAMCommand)
	{
		delete[] pusBRAMCommand;
		pusBRAMCommand = nullptr;
	}
	if (nullptr != pusDRAMCommand)
	{
		delete[] pusDRAMCommand;
		pusDRAMCommand = nullptr;
	}

	return 0;
}

int CDiagnosisHighMemory::DRAMResultCheck(const char* lpszBaseIndent, DATA_TYPE TestDataType, UINT uDRAMStartAddress, const std::map<int, USHORT>& mapBRAMExpectFailLine, 
	const std::map<int, USHORT>& mapDRAMExpectFailLine, BOOL& bSaveLog)
{
	string strFirstIndent = lpszBaseIndent + IndentChar();
	string strSecondIndent = strFirstIndent + IndentChar();
	const char* lpszFirstIndent = strFirstIndent.c_str();
	const char* lpszSecondIndent = strSecondIndent.c_str();

	BYTE bySlotNo = 0;
	BYTE byBoardControllerIndex = 0;
	map<int, USHORT> mapBRAMFailLine;
	map<int, USHORT> mapDRAMFailLine; 
	vector<CHardwareFunction::DATA_RESULT> vecBRAMFaiLLineNo;
	vector<CHardwareFunction::DATA_RESULT> vecDRAMFaiLLineNo;
	auto CopyMap = [&](BOOL bBRAM)
	{
		auto& vecFail = bBRAM ? vecBRAMFaiLLineNo : vecDRAMFaiLLineNo;
		auto& mapFail = bBRAM ? mapBRAMFailLine : mapDRAMFailLine;
		for (auto& Data : vecFail)
		{
			mapFail.insert(make_pair(Data.m_nLineNo, Data.m_usData));
		}
	};
	BOOL bPrintDataTypeLog = FALSE;
	BOOL bAllPass = TRUE;
	const BYTE byMemCount = 2;
	char lpszMemType[byMemCount][8] = {"BRAM", "DRAM"};
	for (auto uControllerID : m_vecEnableController)
	{
		BOOL bControllerPass = TRUE;
		int nErrorCount = 0;

		StartTimer();
		BOOL bPrintControlLog = FALSE;

		bySlotNo = HDModule::Instance()->ID2Board(uControllerID, byBoardControllerIndex);
		if (bSaveLog)
		{
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<controller value='%d, slot value = %d'>\n", lpszBaseIndent, byBoardControllerIndex, bySlotNo);
			bPrintControlLog = TRUE;
		}
		CHardwareFunction* pHardware = GetHardware(uControllerID);

		pHardware->GetFailData(vecBRAMFaiLLineNo, vecDRAMFaiLLineNo);
		CopyMap(TRUE);
		CopyMap(FALSE);

		BOOL bPrintMemType = FALSE;
		int nCurFailCount = 0;
		for (BYTE byMemIndex = 0; byMemIndex < byMemCount; ++byMemIndex)
		{
			auto iterRealFailLine = mapBRAMFailLine.begin();
			auto iterExpectFailLine = mapBRAMExpectFailLine.begin();

			auto pmapFailLine = &mapBRAMFailLine;
			auto pmapExpectFailLine = &mapBRAMExpectFailLine;

			nCurFailCount = 0;
			if (0 != byMemIndex)
			{
				pmapFailLine = &mapDRAMFailLine;
				pmapExpectFailLine = &mapDRAMExpectFailLine;
			}
			bPrintMemType = FALSE;
			if (*pmapFailLine != *pmapExpectFailLine)
			{
				bAllPass = FALSE;
				while (pmapFailLine->end() != iterRealFailLine && pmapExpectFailLine->end() != iterExpectFailLine)
				{
					if (iterRealFailLine->first != iterExpectFailLine->first || iterRealFailLine->second != iterExpectFailLine->second)
					{
						if (!bSaveLog && !bPrintDataTypeLog)
						{
							SaveDRAMAddressLog(uDRAMStartAddress);
							SaveDRAMDataTypeLog(TestDataType);
							bPrintDataTypeLog = TRUE;
						}
						if (!bPrintControlLog)
						{
							m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<controller value='%d, slot value = %d'>\n", lpszBaseIndent, byBoardControllerIndex, bySlotNo);
							bPrintControlLog = TRUE;
						}

						if (!bPrintMemType)
						{
							m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<MemType value='%s'>\n", lpszFirstIndent, lpszMemType[byMemIndex]);
							bPrintMemType = TRUE;
						}
						m_pReportDevice->PrintfToDataLog(IHDReportDevice::Error, "%s<data retValue='false' value='ExpectAddr=0x%X, ReadlAddr=0x%X, cmp_data=0x%X, rdata=0x%X'/>\n", 
							lpszSecondIndent, iterExpectFailLine->first, iterRealFailLine->first, iterExpectFailLine->second, iterRealFailLine->second);
						++nErrorCount;
						if (ERROR_PRINT < nCurFailCount++)
						{
							break;
						}
					}
					++iterRealFailLine;
					++iterExpectFailLine;
				}
				if (pmapFailLine->size() != pmapExpectFailLine->size())
				{
					if (!bSaveLog && !bPrintDataTypeLog)
					{
						SaveDRAMAddressLog(uDRAMStartAddress);
						SaveDRAMDataTypeLog(TestDataType);

						bPrintDataTypeLog = TRUE;
					}
					if (!bPrintControlLog)
					{
						m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<controller value='%d, slot value = %d'>\n", lpszBaseIndent, byBoardControllerIndex, bySlotNo);
						bPrintControlLog = TRUE;
					}
					if (!bPrintMemType)
					{
						m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<MemType value='%s'>\n", lpszFirstIndent, lpszMemType[byMemIndex]);
						bPrintMemType = TRUE;
					}

					m_pReportDevice->PrintfToDataLog(IHDReportDevice::Error, "%s<data retValue='false' value='ExpectFailCount=0x%X, RealFailCount=0x%X'/>\n", lpszSecondIndent,
						pmapExpectFailLine->size(), pmapFailLine->size());
					
					nCurFailCount = 0;
					while (pmapFailLine->end() != iterRealFailLine)
					{
						m_pReportDevice->PrintfToDataLog(IHDReportDevice::Error, "%s<data retValue='false' value='ExpectAddr= 0x00, ReadlAddr=0x%X, cmp_data=0x00, rdata=0x%X'/>\n", 
							lpszSecondIndent, iterRealFailLine->first, iterRealFailLine->second);
						++iterRealFailLine;
						if (ERROR_PRINT < nCurFailCount++)
						{
							break;
						}
					}
					while (pmapExpectFailLine->end() != iterExpectFailLine)
					{
						m_pReportDevice->PrintfToDataLog(IHDReportDevice::Error, "%s<data retValue='false' value='ExpectAddr=0x%X, ReadlAddr=-, cmp_data=0x%X, rdata=0x00'/>\n",
							lpszSecondIndent, iterExpectFailLine->first, iterExpectFailLine->second);
						++iterExpectFailLine;
						if (ERROR_PRINT < nCurFailCount++)
						{
							break;
						}
					}

					++nErrorCount;
				}
				if (bPrintMemType)
				{
					m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s</MemType>\n", lpszFirstIndent);
				}
			}
		}
		if (0 != nErrorCount)
		{
			bAllPass = FALSE;
			bControllerPass = FALSE;
		}

		if (bControllerPass)
		{
			if (bPrintControlLog)
			{
				m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='true'/>\n", lpszFirstIndent);
			}
		}
		else
		{
			SaveFailController(uControllerID);
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='false'/>\n", lpszFirstIndent);
		}
		char lpszTimeUnits[8] = { 0 };
		double dTimeConsume = StopTimer(lpszTimeUnits, sizeof(lpszTimeUnits));
		
		if (bPrintControlLog)
		{
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<Time value='%.3f%s'/>\n", lpszFirstIndent, dTimeConsume, lpszTimeUnits);

			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s</controller>\n", lpszBaseIndent);
		}
	}
	if (bAllPass)
	{
		return 0;
	}
	return -1;
}

int CDiagnosisHighMemory::DRAMSwitchTest(const char* lpszBaseIndent, double dPeriod, UINT uDRAMStartAddr, 
	int nSwitchTimes, UINT uDRAMDataLength, BOOL bSaveLog, BOOL bSaveAddressLog)
{
	USHORT uTestControllersCount = m_vecEnableController.size();
	if (0 >= uTestControllersCount)
	{
		return -1;
	}
	if (nullptr == m_pReportDevice || nullptr == lpszBaseIndent)
	{
		return -2;
	}
	string strFirstIndent = lpszBaseIndent + IndentChar();
	if (!bSaveAddressLog)
	{
		strFirstIndent = lpszBaseIndent;
	}
	string strSecondIndent = strFirstIndent + IndentChar();
	const char* lpszFirstIndent = strFirstIndent.c_str();
	const char* lpszSecondIndent = strSecondIndent.c_str();

	m_strDRAMAddrIndent = lpszBaseIndent;
	m_strDRAMDataTypeIndent = strFirstIndent;
	StartTimer();
	if (bSaveLog && bSaveAddressLog)
	{
		SaveDRAMAddressLog(uDRAMStartAddr);
	}

	const int nLineAfterSwitch = 2;
	int nBRAMDataCount = nSwitchTimes * 4 + nLineAfterSwitch;
	
	map<int, USHORT> mapBRAMExpectFailLine;
	map<int, USHORT> mapDRAMExpectFailLine;

	USHORT* pusDRAMData = nullptr;
	BOOL* pbBRAMCapture = nullptr;
	USHORT* pusBRAMData = nullptr;
	UINT* puBRAMSwitchLine = nullptr;
	UINT* puDRAMSwitchLineOffset = nullptr;
	try
	{
		pusBRAMData = new USHORT[nBRAMDataCount];
		for (int nIndex = 0; nIndex < nBRAMDataCount; ++nIndex)
		{
			pusBRAMData[nIndex] = GET_DATA(nIndex + 1);
			if (0 != pusBRAMData[nIndex])
			{
				mapBRAMExpectFailLine.insert(make_pair(nIndex, pusBRAMData[nIndex]));
			}
		}
		puBRAMSwitchLine = new UINT[nSwitchTimes];
		puDRAMSwitchLineOffset = new UINT[nSwitchTimes];
		pusDRAMData = new USHORT[uDRAMDataLength];

		memset(pusDRAMData, 0, uDRAMDataLength * sizeof(USHORT));
		
		UINT uFailInterval = uDRAMDataLength / 0x400;
		if (0 == uFailInterval)
		{
			uFailInterval = 1;
		}
		
		UINT uFailIndex = 0;
		const int nFailCount = 0x400 - 2;
		for (UINT uLineIndex = 0; uLineIndex < uDRAMDataLength; uLineIndex += uFailInterval)
		{
			if (nFailCount == uFailIndex)
			{
				break;
			}
			pusDRAMData[uLineIndex] = GET_DATA(uFailIndex + 1);
			if (0 != pusDRAMData[uLineIndex])
			{
				mapDRAMExpectFailLine.insert(make_pair(uLineIndex, pusDRAMData[uLineIndex]));
				++uFailIndex;
			}
		}
	}
	catch (const std::exception&)
	{
		return -3;
	}


	double dTimeConsume = 0;
	char lpszTimeUnits[4] = { 0 };
	
	int nBRAMSwithStep = (nBRAMDataCount - nLineAfterSwitch) / nSwitchTimes;

	for (int nIndex = 0; nIndex < nSwitchTimes; ++nIndex)
	{
		puBRAMSwitchLine[nIndex] = (nBRAMSwithStep - 1) + nIndex * nBRAMSwithStep;
		puDRAMSwitchLineOffset[nIndex] = (nIndex / 2) * 1024 + (nIndex % 2) * 10 + 50;
	}

	if (puDRAMSwitchLineOffset[nSwitchTimes - 1] != uDRAMDataLength - 1)
	{
		puDRAMSwitchLineOffset[nSwitchTimes - 1] = uDRAMDataLength - 1;
	}

	BOOL bPrintPrevious = FALSE;
	BOOL bAllPass = TRUE;

	auto iterController = m_setFailController.begin();

	const BYTE byDataTypeCount = 3;
	const char* lpszDataType[byDataTypeCount] = { "FM", "MM", "IOM"};
	DATA_TYPE DataType[byDataTypeCount] = { DATA_TYPE::FM, DATA_TYPE::MM, DATA_TYPE::IOM};
	const BYTE byMemType = 2;
	const char* lpszMemType[byMemType] = { "BRAM","DRAM" };
	int nRetVal = 0;
	int nErrorCount = 0;
	for (BYTE byDataTypeIndex = 0; byDataTypeIndex < byDataTypeCount; ++byDataTypeIndex)
	{
		StartTimer();

		BOOL bPrintDataTypeLog = FALSE;
		if (bSaveLog)
		{
			SaveDRAMDataTypeLog(DataType[byDataTypeIndex]);
			bPrintDataTypeLog = TRUE;
		}

		DRAMHighSpeedCheck(dPeriod, DataType[byDataTypeIndex], nSwitchTimes, puBRAMSwitchLine, puDRAMSwitchLineOffset, nBRAMDataCount,
			pusBRAMData, uDRAMStartAddr, uDRAMDataLength, pusDRAMData);

		 nRetVal = DRAMResultCheck(lpszSecondIndent, DataType[byDataTypeIndex], uDRAMStartAddr, mapBRAMExpectFailLine, mapDRAMExpectFailLine, bPrintDataTypeLog);
		
		if (0 == nRetVal)
		{
			if (bPrintDataTypeLog)
			{
				m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='true'/>\n", lpszSecondIndent);
			}
		}
		else
		{
			bAllPass = FALSE;
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='false'/>\n", lpszSecondIndent);
		}
		dTimeConsume = StopTimer( lpszTimeUnits, sizeof(lpszTimeUnits));
		if (bPrintDataTypeLog)
		{
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<Time value='%.3f%s'/>\n", lpszSecondIndent, dTimeConsume, lpszTimeUnits);
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s</DataType>\n", lpszFirstIndent);
		}
	}
	if (bAllPass)
	{
		nRetVal = 0;
	}
	else
	{
		nRetVal = -1;
	}
	if (bSaveAddressLog)
	{
		if (bAllPass)
		{
			if (bSaveLog)
			{
				m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='true'/>\n", lpszFirstIndent);
			}
		}
		else
		{
			nRetVal = -1;
			bSaveLog = TRUE;
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='false'/>\n", lpszFirstIndent);
		}
		dTimeConsume = StopTimer(lpszTimeUnits, sizeof(lpszTimeUnits));
		if (bSaveLog)
		{
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<Time value='%.3f%s'/>\n", lpszFirstIndent, dTimeConsume, lpszTimeUnits);
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s</DRAMStartAddr>\n", lpszBaseIndent);
		}
	}

	if (nullptr != pusDRAMData)
	{
		delete[] pusDRAMData;
		pusDRAMData = nullptr;
	}
	if (nullptr != pusDRAMData)
	{
		delete[] pusDRAMData;
		pusDRAMData = nullptr;
	}
	if (nullptr != pusBRAMData)
	{
		delete[] pusBRAMData;
		pusBRAMData = nullptr;
	}
	if (nullptr != pusBRAMData)
	{
		delete[] pusBRAMData;
		pusBRAMData = nullptr;
	}
	if (nullptr != puBRAMSwitchLine)
	{
		delete[] puBRAMSwitchLine;
		puBRAMSwitchLine = nullptr;
	}
	if (nullptr != puDRAMSwitchLineOffset)
	{
		delete[] puDRAMSwitchLineOffset;
		puDRAMSwitchLineOffset = nullptr;
	}

	return nRetVal;
}

int CDiagnosisHighMemory::DRAMMultiSwitchDiagnosis(const char* lpszBaseIndent)
{
	StartTimer();
	double dTimeConsume = 0;
	char lpszTimeUnits[4] = { 0 };

	double dTestRate = 12;
	m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<DRAMMultiSwitchTest value='period=%.0fns'>\n", lpszBaseIndent, dTestRate);

	int nRetVal = 0;
	string strFirstIndent = lpszBaseIndent + IndentChar();
	const char* lpszFirstIndent = strFirstIndent.c_str();

	const int nAddrTestCount = 3;
	UINT uDRAMStartAddr[nAddrTestCount] = { 0,1000,2044 };
	BOOL bAllPass = TRUE;
	for (int nAddrIndex = 0; nAddrIndex < nAddrTestCount; ++nAddrIndex)
	{
		UINT uStartAddr = uDRAMStartAddr[nAddrIndex];
		if (m_pReportDevice->IsStop())
		{
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<UserStop value='NextAddr=0x%X'/>\n", lpszFirstIndent, uStartAddr);
			break;
		}
		nRetVal = DRAMSwitchTest(lpszFirstIndent, dTestRate, uStartAddr, 4, 4 * 0x400, TRUE);
		if (0 != nRetVal)
		{
			bAllPass = FALSE;
		}
	}
	if (bAllPass)
	{
		nRetVal = 0;
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='true'/>\n", lpszFirstIndent);
	}
	else
	{
		nRetVal = 1;
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='false'/>\n", lpszFirstIndent);
	}
	dTimeConsume = StopTimer( lpszTimeUnits, sizeof(lpszTimeUnits));
	m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<Time value='%.3f%s'/>\n", lpszFirstIndent, dTimeConsume, lpszTimeUnits);
	m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s</DRAMMultiSwitchTest>\n", lpszBaseIndent);
	return nRetVal;
}

int CDiagnosisHighMemory::DRAMStabilityDiagnosis(const char* lpszBaseIndent)
{
	StartTimer();
	double dTimeConsume = 0;
	char lpszTimeUnits[4] = { 0 };

	double dTestRate = 12;
	m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<DRAMStabilityTest value='period=%.0fns'>\n", lpszBaseIndent, dTestRate);
	const int nTestAddrCount = 2;
	ULONG ulAddrType[nTestAddrCount] = { 1000,2046 };
	ULONG uDRAMStartAddr = 0;
	string strFirstIndent = lpszBaseIndent + IndentChar();
	string strSecondIndent = strFirstIndent + IndentChar();
	string strThirdIndent = strSecondIndent + IndentChar();
	const char* lpszFirstIndent = strFirstIndent.c_str();
	const char* lpszSecondIndent = strSecondIndent.c_str();
	const char* lpszThirdIndent = strThirdIndent.c_str();
	
	BOOL bAllPass = TRUE;
	int nRetVal = 0;
	int nFailTimes = 0;
	for (int nAddrIndex = 0; nAddrIndex < nTestAddrCount;++nAddrIndex)
	{
		StartTimer();
		uDRAMStartAddr = ulAddrType[nAddrIndex];
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<DRAMStartAddr value='%d'>\n", lpszFirstIndent, uDRAMStartAddr);
		for (int nTestIndex = 0; nTestIndex < 50; ++nTestIndex)
		{
			if (m_pReportDevice->IsStop())
			{
				m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<UserStop value='NextTimes=0x%X'/>\n", lpszSecondIndent, nTestIndex + 1);
				break;
			}
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<TestTimes value='%d'>\n", lpszSecondIndent, nTestIndex + 1);

			StartTimer();
			nRetVal = DRAMSwitchTest(lpszThirdIndent, dTestRate, uDRAMStartAddr, 4, 4 * 0x400, TRUE, FALSE);
			if (0 == nRetVal)
			{
				m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='true'/>\n", lpszThirdIndent);
			}
			else
			{
				++nFailTimes;
				bAllPass = FALSE;
				m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='false'/>\n", lpszThirdIndent);
			}
			dTimeConsume = StopTimer( lpszTimeUnits, sizeof(lpszTimeUnits));
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<Time value='%.3f%s'/>\n", lpszThirdIndent, dTimeConsume, lpszTimeUnits);
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s</TestTimes>\n", lpszSecondIndent);
			
		}

		if (0 == nFailTimes)
		{
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='true'/>\n", lpszSecondIndent);
		}
		else
		{
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='false'/>\n", lpszSecondIndent);
		}
		dTimeConsume = StopTimer( lpszTimeUnits, sizeof(lpszTimeUnits));
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<Time value='%.3f%s'/>\n", lpszSecondIndent, dTimeConsume, lpszTimeUnits);

		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s</DRAMStartAddr, failTime='%d'>\n", lpszFirstIndent, nFailTimes);
	}

	if (bAllPass)
	{
		nRetVal = 0;
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='true'/>\n", lpszFirstIndent);
	}
	else
	{
		nRetVal = 1;
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='false'/>\n", lpszFirstIndent);
	}
	dTimeConsume = StopTimer( lpszTimeUnits, sizeof(lpszTimeUnits));
	m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<Time value='%.3f%s'/>\n", lpszFirstIndent, dTimeConsume, lpszTimeUnits);

	m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s</DRAMStabilityTest>\n", lpszBaseIndent);
	return nRetVal;
}

int CDiagnosisHighMemory::DRAMMultiRateDiagnosis(const char* lpszBaseIndent)
{
	StartTimer();
	double dTimeConsume = 0;
	char lpszTimeUnits[4] = { 0 };

	ULONG ulDRAMStartAddr = 0;
	m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<DRAMMultiRateDiagnosis, DRAMStartAddr='0x%04X'>\n", lpszBaseIndent, ulDRAMStartAddr);
	
	//Test Rate
	vector<double> vecPeriod;
	vecPeriod.push_back(12);
	vecPeriod.push_back(16);
	vecPeriod.push_back(17);
	vecPeriod.push_back(18);
	vecPeriod.push_back(19);
	vecPeriod.push_back(20);
	vecPeriod.push_back(21);
	vecPeriod.push_back(22);
	vecPeriod.push_back(23);
	vecPeriod.push_back(24);
	vecPeriod.push_back(25);
	vecPeriod.push_back(30);
	vecPeriod.push_back(35);
	vecPeriod.push_back(40);
	vecPeriod.push_back(45);
	vecPeriod.push_back(50);
	vecPeriod.push_back(55);
	vecPeriod.push_back(60);
	vecPeriod.push_back(65);
	vecPeriod.push_back(70);
	vecPeriod.push_back(75);
	vecPeriod.push_back(80);
	vecPeriod.push_back(85);
 	vecPeriod.push_back(100);
 	vecPeriod.push_back(400);
 	vecPeriod.push_back(2e3);
 	vecPeriod.push_back(15e3);
	vecPeriod.push_back(400e3);

	string strFirstIndent = lpszBaseIndent + IndentChar();
	string strSecondIndent = strFirstIndent + IndentChar();
	const char* lpszFirstIndent = strFirstIndent.c_str();
	const char* lpszSecondIndent = strSecondIndent.c_str();

	int nRetVal = 0;
	BOOL bAllPass = TRUE;
	int nTestRateCount = vecPeriod.size();
	for (auto dPeriod : vecPeriod)
	{
		double dShowPeriod = GetPeriodUnits(dPeriod, lpszTimeUnits, sizeof(lpszTimeUnits));
		if (m_pReportDevice->IsStop())
		{
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<UserStop value='NextPeriod=%.1f%s'/>\n", lpszFirstIndent, dShowPeriod, lpszTimeUnits);
			break;
		}
		StartTimer();

		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<Period value='%.1f%s'>\n", lpszFirstIndent, dShowPeriod, lpszTimeUnits);
		nRetVal = DRAMSwitchTest(lpszSecondIndent, dPeriod, ulDRAMStartAddr, 1, 4 * 0x400, TRUE);
		if (0 == nRetVal)
		{
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='true'/>\n", lpszSecondIndent);
		}
		else
		{
			bAllPass = FALSE;
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='false'/>\n", lpszSecondIndent);
		}
		dTimeConsume = StopTimer( lpszTimeUnits, sizeof(lpszTimeUnits));
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<Time value='%.3f%s'/>\n", lpszSecondIndent, dTimeConsume, lpszTimeUnits);
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s</Period>\n", lpszFirstIndent);
	}

	if (bAllPass)
	{
		nRetVal = 0;
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='true'/>\n", lpszFirstIndent);
	}
	else
	{
		nRetVal = 1;
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='false'/>\n", lpszFirstIndent);
	}
	dTimeConsume = StopTimer( lpszTimeUnits, sizeof(lpszTimeUnits));
	m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<Time value='%.3f%s'/>\n", lpszFirstIndent, dTimeConsume, lpszTimeUnits);
	m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s</DRAMMultiRateDiagnosis>\n", lpszBaseIndent);
	return nRetVal;
}

int CDiagnosisHighMemory::DRAMMultiLengthDiagnosis(const char* lpszBaseIndent)
{
	StartTimer();
	double dTimeConsume = 0;
	char lpszTimeUnits[4] = { 0 };

	double dRate = 12;
	m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<DRAMMultiLenghtDiagnosis Period='%.0fns'>\n", lpszBaseIndent, dRate);

	BOOL bAllPass = TRUE;
	int nRetVal = 0;

	string strFirstIndent = lpszBaseIndent + IndentChar();
	string strSecondIndent = strFirstIndent + IndentChar();
	string strThirdIndent = strSecondIndent + IndentChar();
	const char* lpszFirstIndent = strFirstIndent.c_str();
	const char* lpszSecondIndent = strSecondIndent.c_str();
	const char* lpszThirdIndent = strThirdIndent.c_str();

	const int nTestAddrCount = 4;
	ULONG uAddrType[nTestAddrCount] = { 0, 1000, 1024, 2099196 };
	const int nTestLengthCount = 4;
	int nLengthType[nTestLengthCount] = { 1 , 2, 4, 20 };

	UINT uCurDRAMStartAddr = 0;

	for (int nLengthIndex = 0; nLengthIndex < nTestLengthCount; ++nLengthIndex)
	{
		BOOL bLengthPass = TRUE;
		int nCurLength = nLengthType[nLengthIndex];
		if (m_pReportDevice->IsStop())
		{
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<UserStop value='NextLength=%d'/>\n", lpszFirstIndent, nCurLength);
			break;
		}
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<DRAMLength value='%d'>\n", lpszFirstIndent, nCurLength);

		StartTimer();

		for (int nAddrIndex = 0; nAddrIndex < nTestAddrCount; ++nAddrIndex)
		{
			uCurDRAMStartAddr = uAddrType[nAddrIndex];
			if (m_pReportDevice->IsStop())
			{
				m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<UserStop value='NextAddr=0x%X'/>\n", lpszFirstIndent, uCurDRAMStartAddr);
				break;
			}
			nRetVal = DRAMSwitchTest(lpszSecondIndent, dRate, uCurDRAMStartAddr, 1, nCurLength, TRUE);
			if (0 != nRetVal)
			{
				bAllPass = FALSE;
				bLengthPass = FALSE;
			}
		}
		if (bLengthPass)
		{
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='true'/>\n", lpszSecondIndent);
		}
		else
		{
			bAllPass = FALSE;
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='false'/>\n", lpszSecondIndent);
		}
		dTimeConsume = StopTimer(lpszTimeUnits, sizeof(lpszTimeUnits));
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<Time value='%.3f%s'/>\n", lpszSecondIndent, dTimeConsume, lpszTimeUnits);
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s</DRAMLength>\n", lpszFirstIndent);
	}

	if (bAllPass)
	{
		nRetVal = 0;
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='true'/>\n", lpszFirstIndent);
	}
	else
	{
		nRetVal = -1;
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='false'/>\n", lpszFirstIndent);
	}
	dTimeConsume = StopTimer(lpszTimeUnits, sizeof(lpszTimeUnits));
	m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<Time value='%.3f%s'/>\n", lpszFirstIndent, dTimeConsume, lpszTimeUnits);

	m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s</DRAMMultiLenghtDiagnosis>\n", lpszBaseIndent);
	return nRetVal;
}

int CDiagnosisHighMemory::DRAMAllPageDiagnosis(const char* lpszBaseIndent)
{
	if (USER == m_UserRole)
	{
		return 0;
	}

	StartTimer();
	double dTimeConsume = 0;
	char lpszTimeUnits[4] = { 0 };

	double dPeriod = 12;
	const int nDRAMAddrStep = 0x1000;///<4K;
	const int nLogSaveInterval = 0x100000 / nDRAMAddrStep;///<Save log each 1M
	m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<DRAMAllPageDiagnosis value='period=%.0fns, TestAddrStep=0x%04X, LogSaveInterval=%d'>\n", lpszBaseIndent, dPeriod, nDRAMAddrStep, nLogSaveInterval);

	string strFirstIndent = lpszBaseIndent + IndentChar();
	const char* lpszFirstIndent = strFirstIndent.c_str();

	int nRetVal = 0;
	BOOL bAllPass = TRUE;
	BOOL bSavePassLog = FALSE;
	char lpszMsg[128] = { 0 };
	int nTestIndex = 0;
	for (ULONG ulDRAMAddr = 0, nTestIndex = 0; ulDRAMAddr < HDModule::DRAMLineCount; ulDRAMAddr += nDRAMAddrStep, ++ nTestIndex)
	{
		if (m_pReportDevice->IsStop())
		{
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<UserStop value='NextAddr=0x%X'/>\n", lpszFirstIndent, ulDRAMAddr);
			break;
		}
		if (0 == nTestIndex % nLogSaveInterval)
		{
			bSavePassLog = TRUE;
		}
		else
		{
			bSavePassLog = FALSE;
		}

		nRetVal = DRAMSwitchTest(lpszFirstIndent, dPeriod, ulDRAMAddr, 1, nDRAMAddrStep, bSavePassLog);
		
		if (0 != nRetVal)
		{
			bAllPass = FALSE;
		}
	}
	
	if (bAllPass)
	{
		nRetVal = 0;
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='true'/>\n", lpszFirstIndent);
	}
	else
	{
		nRetVal = -1;
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='false'/>\n", lpszFirstIndent);
	}
	dTimeConsume = StopTimer( lpszTimeUnits, sizeof(lpszTimeUnits));
	m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<Time value='%.3f%s'/>\n", lpszFirstIndent, dTimeConsume, lpszTimeUnits);
	m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s</DRAMAllPageDiagnosis>\n", lpszBaseIndent);

	return nRetVal;
}

int CDiagnosisHighMemory::DRAMAllPageRanDiagnosis(const char* lpszBaseIndent)
{
	if (USER == m_UserRole)
	{
		return 0;
	}

	StartTimer();
	double dTimeConsume = 0;
	char lpszTimeUnits[4] = { 0 };

	UINT uBRAMInterval = 2;
	UINT uSwitchTimes = 64;
	m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<DRAMAllPageRanDiagnosis, value='SwitchTimes=%d'>\n", lpszBaseIndent, uSwitchTimes);

	string strFirstIndent = lpszBaseIndent + IndentChar();
	string strSecondIndent = strFirstIndent + IndentChar();
	string strThirdIndent = strSecondIndent + IndentChar();
	const char* lpszFirstIndent = strFirstIndent.c_str();
	const char* lpszSecondIndent = strSecondIndent.c_str();
	const char* lpszThirdIndent = strThirdIndent.c_str();

	int nRetVal = 0;
	BOOL bAllPass = TRUE;
	char lpszMsg[128] = { 0 };
	int nTestIndex = 0;

	Bind(m_vecEnableController, m_vecEnableController[0]);
	CHardwareFunction* pHardware = GetHardware(m_vecEnableController[0]);

	CPattern Pattern(*pHardware);

	UINT uBRAMLineCount = uBRAMInterval * (uSwitchTimes + 1);
	USHORT* pusBRAMData = nullptr;
	USHORT* pusDRAMData = nullptr;
	USHORT* pusBRAMCommand = nullptr;
	USHORT* pusDRAMCommand = nullptr;

	UINT uDRAMLineCountPerWrite = 0x400;

	try
	{
		pusBRAMData = new USHORT[uBRAMLineCount];
		memset(pusBRAMData, 0, uBRAMLineCount * sizeof(USHORT));
		pusBRAMCommand = new USHORT[uBRAMLineCount];
		memset(pusBRAMCommand, 0, uBRAMLineCount * sizeof(USHORT));
		pusDRAMData = new USHORT[uDRAMLineCountPerWrite];
		memset(pusDRAMData, 0, uDRAMLineCountPerWrite * sizeof(USHORT));
		pusDRAMCommand = new USHORT[uDRAMLineCountPerWrite];
		memset(pusDRAMCommand, 0, uDRAMLineCountPerWrite * sizeof(USHORT));
	}
	catch (const std::exception&)
	{
		StopTimer();
		return -1;
	}
	int nSwitchTimes = 0;
	map<int, USHORT> mapBRAMExpectFailLine;
	map<int, USHORT> mapDRAMExpectFailLine;
	BOOL bSwitch = TRUE;
	do
	{
		for (UINT uLineIndex = 0; uLineIndex < uBRAMLineCount; ++uLineIndex)
		{
			pusBRAMData[uLineIndex] = GET_DATA(uLineIndex + 1);
			mapBRAMExpectFailLine.insert(make_pair(uLineIndex, pusBRAMData[uLineIndex]));
			if (uBRAMInterval - 1 == uLineIndex % uBRAMInterval && uSwitchTimes > nSwitchTimes)
			{
				++nSwitchTimes;
				bSwitch = TRUE;
			}
			else
			{
				bSwitch = FALSE;
			}

			pusBRAMCommand[uLineIndex] = Pattern.GetCommand(TRUE, 0, FALSE, bSwitch);
		}

		pHardware->WriteDataMemory(MEM_TYPE::BRAM, DATA_TYPE::FM, 0, uBRAMLineCount, pusBRAMData);
		pHardware->WriteDataMemory(MEM_TYPE::BRAM, DATA_TYPE::MM, 0, uBRAMLineCount, pusBRAMData);
		pHardware->WriteDataMemory(MEM_TYPE::BRAM, DATA_TYPE::IOM, 0, uBRAMLineCount, pusBRAMData);
		pHardware->WriteDataMemory(MEM_TYPE::BRAM, DATA_TYPE::CMD, 0, uBRAMLineCount, pusBRAMCommand);

		UINT uCurLineNo = 0;
		UINT uDRAMPerBlockLineCount = 0x100000;
		UINT uBlockCount = HDModule::DRAMLineCount / uDRAMPerBlockLineCount;
		UINT uCycleCount = uDRAMPerBlockLineCount / uDRAMLineCountPerWrite;
		UINT uFailIndex = 0;
		BOOL bCurLineFail = FALSE;
		UINT uDRAMFailCount = 1021;
		UINT uStartAddr = 0;
		for (UINT uBlockIndex = 0; uBlockIndex < uBlockCount; ++uBlockIndex)
		{
			if (m_pReportDevice->IsStop())
			{
				break;
			}
			for (UINT nCycleIndex = 0; nCycleIndex < uCycleCount; ++nCycleIndex, uCurLineNo += uDRAMLineCountPerWrite, uStartAddr += uDRAMLineCountPerWrite)
			{
				if (m_pReportDevice->IsStop())
				{
					break;
				}
				bCurLineFail = FALSE;
				memset(pusDRAMData, 0, uDRAMLineCountPerWrite * sizeof(USHORT));
				memset(pusDRAMCommand, 0, uDRAMLineCountPerWrite * sizeof(USHORT));
				if (nCycleIndex + 1 == uCycleCount)
				{
					pusDRAMCommand[uDRAMLineCountPerWrite - 1] = Pattern.GetCommand(FALSE, 0, FALSE, TRUE);
				}

				if (nCycleIndex == (uFailIndex % 16) && uDRAMFailCount > uFailIndex)
				{
					UINT uLineOffset = 0;
					USHORT usFailData = GET_DATA(uFailIndex);
					if (0 == usFailData)
					{
						usFailData = GET_DATA(uFailIndex + 1);
					}
					if (0 == nCycleIndex)
					{
						pusDRAMData[0] = usFailData;
					}
					else if (nCycleIndex + 1 == uCycleCount)
					{
						uFailIndex = uDRAMLineCountPerWrite - 1;
						pusDRAMData[uFailIndex] = usFailData;

					}
					else
					{
						pusDRAMData[uFailIndex] = usFailData;
						uLineOffset = uFailIndex;
					}
					mapDRAMExpectFailLine.insert(make_pair(uCurLineNo + uLineOffset, usFailData));
					++uFailIndex;
				}

				pHardware->WriteDataMemory(MEM_TYPE::DRAM, DATA_TYPE::FM, uStartAddr, uDRAMLineCountPerWrite, pusDRAMData);
				pHardware->WriteDataMemory(MEM_TYPE::DRAM, DATA_TYPE::MM, uStartAddr, uDRAMLineCountPerWrite, pusDRAMData);
				pHardware->WriteDataMemory(MEM_TYPE::DRAM, DATA_TYPE::IOM, uStartAddr, uDRAMLineCountPerWrite, pusDRAMData);
				pHardware->WriteDataMemory(MEM_TYPE::DRAM, DATA_TYPE::CMD, uStartAddr, uDRAMLineCountPerWrite, pusDRAMCommand);
			}
		}
	} while (FALSE);
	ClearBind();

	if (nullptr == pusBRAMData)
	{
		delete[] pusBRAMData;
		pusBRAMData = nullptr;
	}
	if (nullptr == pusDRAMData)
	{
		delete[] pusDRAMData;
		pusDRAMData = nullptr;
	}
	if (nullptr == pusBRAMCommand)
	{
		delete[] pusBRAMCommand;
		pusBRAMCommand = nullptr;
	}
	if (nullptr == pusDRAMCommand)
	{
		delete[] pusDRAMCommand;
		pusDRAMCommand = nullptr;
	}

	if (m_pReportDevice->IsStop())
	{
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<UserStop/>\n", lpszFirstIndent);
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='true'/>\n", lpszFirstIndent);
		
		dTimeConsume = StopTimer(lpszTimeUnits, sizeof(lpszTimeUnits));
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<Time value='%.3f%s'/>\n", lpszFirstIndent, dTimeConsume, lpszTimeUnits);
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s</DRAMAllPageRanDiagnosis>\n", lpszBaseIndent, uSwitchTimes);
		return 0;
	}

	const BYTE byDataTypeCount = 3;
	const char* lpszDataType[byDataTypeCount] = { "FM","MM","IOM" };
	DATA_TYPE DataType[byDataTypeCount] = { DATA_TYPE::FM,DATA_TYPE::MM ,DATA_TYPE::IOM };

	vector<double> vecPeriod;
 	vecPeriod.push_back(12);
	vecPeriod.push_back(17);
	vecPeriod.push_back(100);

	double dEdge[EDGE_COUNT] = { 0 };
	vector<USHORT> vecChannel;
	for (USHORT usChannel = 0; usChannel < HDModule::ChannelCountPerControl; ++usChannel)
	{
		vecChannel.push_back(usChannel);
	}
	BOOL bSavePassLog = FALSE;
	double dPeriod = 0;
	int nPeriodCount = vecPeriod.size();
	for (auto dPeriod : vecPeriod)
	{
		BOOL bPeriodPass = TRUE;
		double dShowPeriod = GetPeriodUnits(dPeriod, lpszTimeUnits, sizeof(lpszTimeUnits));
		if (m_pReportDevice->IsStop())
		{
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<UserStop value='NextPeriod=%.1f%s'/>\n", lpszFirstIndent, dShowPeriod, lpszTimeUnits);
			break;
		}
		StartTimer();
		dEdge[0] = 0;
		dEdge[1] = dPeriod / 2;
		dEdge[2] = 0;
		dEdge[3] = dPeriod / 2;
		dEdge[4] = dPeriod / 2;
		dEdge[5] = dPeriod * 3 / 4;

		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<Period value='%.1f%s'>\n", lpszFirstIndent, dShowPeriod, lpszTimeUnits);
		for (BYTE byDataTypeIndex = 0; byDataTypeIndex < byDataTypeCount; ++byDataTypeIndex)
		{
			if (m_pReportDevice->IsStop())
			{
				m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<UserStop value='NextDataType=%s'/>\n", lpszSecondIndent, lpszDataType[byDataTypeIndex]);
				break;
			}
			StartTimer();

			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<DataType value='%s'>\n", lpszSecondIndent, lpszDataType[byDataTypeIndex]);

			///<Set all parameter of running vector
			Bind(m_vecEnableController, m_vecEnableController[0]);
			pHardware = GetHardware(m_vecEnableController[0]);
			pHardware->SetPeriod(0, dPeriod);
			pHardware->SetEdge(vecChannel, 0, dEdge, WAVE_FORMAT::NRZ, IO_FORMAT::NRZ, COMPARE_MODE::EDGE);
			pHardware->SetRunParameter(0, uBRAMLineCount - 1, TRUE, 0);
			pHardware->SetPatternMode(FALSE, DataType[byDataTypeIndex], TRUE, FALSE);
			ClearBind();

			pHardware->SynRun();
			WaitStop();

			nRetVal = DRAMResultCheck(lpszThirdIndent, DataType[byDataTypeIndex], 0, mapBRAMExpectFailLine, mapDRAMExpectFailLine, bSavePassLog);

			if (0 != nRetVal)
			{
				bPeriodPass = FALSE;
				m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='false'/>\n", lpszThirdIndent);
			}
			else
			{
				m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='true'/>\n", lpszThirdIndent);
			}
			dTimeConsume = StopTimer(lpszTimeUnits, sizeof(lpszTimeUnits));
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<Time value='%.3f%s'/>\n", lpszThirdIndent, dTimeConsume, lpszTimeUnits);
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s</DataType>\n", lpszSecondIndent);
		}
		if (bPeriodPass)
		{
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='true'/>\n", lpszSecondIndent);
		}
		else
		{
			bAllPass = FALSE;
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='false'/>\n", lpszSecondIndent);
		}

		dTimeConsume = StopTimer(lpszTimeUnits, sizeof(lpszTimeUnits));

		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<Time value='%.3f%s'/>\n", lpszSecondIndent, dTimeConsume, lpszTimeUnits);
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s</Period>\n", lpszFirstIndent);
	}
	if (bAllPass)
	{
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='true'/>\n", lpszFirstIndent);
	}
	else
	{
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='false'/>\n", lpszFirstIndent);
	}
	dTimeConsume = StopTimer(lpszTimeUnits, sizeof(lpszTimeUnits));
	m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<Time value='%.3f%s'/>\n", lpszFirstIndent, dTimeConsume, lpszTimeUnits);
	m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s</DRAMAllPageRanDiagnosis>\n", lpszBaseIndent, uSwitchTimes);
	
	if (bAllPass)
	{
		return 0;
	}
	return -1;
}

int CDiagnosisHighMemory::DRAMFastDiagnosis(const char* lpszBaseIndent)
{
	if (USER != m_UserRole)
	{
		return 0;
	}
	StartTimer();


	double dTimeConsume = 0;
	char lpszTimeUnits[4] = { 0 };

	string strFirstIndent = lpszBaseIndent + IndentChar();
	const char* lpszFirstIndent = strFirstIndent.c_str();

	double dPeriod = 12;
	const int nTestLineCount = 0x400;
	const int nDRAMAddrStep = 0x100400;
	m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<DRAMFastDiagnosis Period='%.0fns'>\n", lpszBaseIndent, dPeriod);
	m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<LineCountPerTest>%d</LineCountPerTest>\n", lpszFirstIndent, nTestLineCount);
	m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<AddressCount>0x%X</AddressCount>\n", lpszFirstIndent, 0x100400);


	int nRetVal = 0;
	BOOL bAllPass = TRUE;
	for (UINT uDRAMAddr = 0; uDRAMAddr < HDModule::DRAMLineCount; uDRAMAddr += nDRAMAddrStep)
	{
		if (m_pReportDevice->IsStop())
		{
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<UserStop value='NextAddr=0x%X'/>\n", lpszFirstIndent, uDRAMAddr);
			break;
		}
	
		nRetVal = DRAMSwitchTest(lpszFirstIndent, dPeriod, uDRAMAddr, 1, nTestLineCount, TRUE);

		if (0 != nRetVal)
		{
			bAllPass = FALSE;
		}
	}

	if (bAllPass)
	{
		nRetVal = 0;
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='true'/>\n", lpszFirstIndent);
	}
	else
	{
		nRetVal = -1;
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='false'/>\n", lpszFirstIndent);
	}
	dTimeConsume = StopTimer( lpszTimeUnits, sizeof(lpszTimeUnits));
	m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<Time value='%.3f%s'/>\n", lpszFirstIndent, dTimeConsume, lpszTimeUnits);
	m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s</DRAMFastDiagnosis>\n", lpszBaseIndent);
	return nRetVal;
}

inline void CDiagnosisHighMemory::SaveFailController(UINT uControllerID)
{
	if (m_setFailController.end() == m_setFailController.find(uControllerID))
	{
		m_setFailController.insert(uControllerID);
	}
}

inline void CDiagnosisHighMemory::ShowUIResult()
{
	BYTE bySlotNo = 0;
	BYTE byBoardControllerIndex = 0;
	for (auto uControllerID : m_vecEnableController)
	{
		bySlotNo = HDModule::Instance()->ID2Board(uControllerID, byBoardControllerIndex);
		if (m_setFailController.end() == m_setFailController.find(uControllerID))
		{
			m_pReportDevice->PrintfToUi(IHDReportDevice::Pass);
		}
		else
		{
			m_pReportDevice->PrintfToUi(IHDReportDevice::Fail);
		}

		m_pReportDevice->PrintfToUi("\t Slot %d, controller %d\n", bySlotNo, byBoardControllerIndex);
	}
	m_setFailController.clear();
}

inline void CDiagnosisHighMemory::SaveDRAMAddressLog(UINT uDRAMAddr)
{
	m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<DRAMStartAddr value='0x%04X'>\n", m_strDRAMAddrIndent.c_str(), uDRAMAddr);
}

inline void CDiagnosisHighMemory::SaveDRAMDataTypeLog(DATA_TYPE DataType)
{
	string strDataTypeName;
	switch (DataType)
	{
	case DATA_TYPE::FM:
		strDataTypeName = "FM";
		break;
	case DATA_TYPE::MM:
		strDataTypeName = "MM";
		break;
	case DATA_TYPE::IOM:
		strDataTypeName = "IOM";
		break;
	case DATA_TYPE::CMD:
	case DATA_TYPE::OPERAND:
	default:
		return;
		break;
	}
	m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<DataType value='%s'>\n", m_strDRAMDataTypeIndent.c_str(), strDataTypeName.c_str());
}
