#include "DiagnosisComparisonShield.h"
#include "..\HDModule.h"
#include "..\Pattern.h"
#include <iterator>
using namespace std;
CDiagnosisComparisonShield::CDiagnosisComparisonShield()
{

}

CDiagnosisComparisonShield::~CDiagnosisComparisonShield()
{
}


int CDiagnosisComparisonShield::QueryInstance(const char* lpszName, void** ppPoint)
{
	return -1;
}

void CDiagnosisComparisonShield::Release()
{

}

const char* CDiagnosisComparisonShield::Name() const
{
	return "Comparison Shield Diagnosis";
}

int CDiagnosisComparisonShield::GetChildren(STSVector<IHDDoctorItem*>& vecChildren) const
{
	return 0;
}

int CDiagnosisComparisonShield::Doctor(IHDReportDevice* pReportDevice)
{
	m_pReportDevice = pReportDevice;
	int nRetVal = -1;

	StartTimer();
	double dTimeConsume = 0;
	char lpszTimeUnits[4] = { 0 };
	const char* lpszBaseIndent = IndentFormat();

	m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<ComparisonShield>\n", lpszBaseIndent);
	std::string strNextIndent = IndentFormat() + IndentChar();
	const char* lpszNextIndent = strNextIndent.c_str();
	CheckMutualDiagnosable();
	BYTE bySlotNo = 0;
	BYTE byBoardControllerIndex = 0;
	for (auto& Undiagnosable : m_mapUndiagnosableController)
	{
		bySlotNo = HDModule::Instance()->ID2Board(Undiagnosable.first, byBoardControllerIndex);
		m_pReportDevice->PrintfToUi(IHDReportDevice::Fail);
		m_pReportDevice->PrintfToUi("\t Slot %d, Controller %d\n", bySlotNo, byBoardControllerIndex);
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<Controller value='%d, slot value=%d'>\n", IndentFormat(), bySlotNo, byBoardControllerIndex);

		bySlotNo = HDModule::Instance()->ID2Board(Undiagnosable.second, byBoardControllerIndex);
		m_pReportDevice->PrintfToUi("\t\t Undiagnosable for slot %d controller %d is not existed.", bySlotNo, byBoardControllerIndex);

		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<Undiagnosable for slot %d controller %d is not existed./>\n", lpszNextIndent, bySlotNo, byBoardControllerIndex);
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s</Controller>\n", IndentFormat());

	}

	int nFailItemCount = 0;
	int nItemResult = 0;

	Connect();

	if (0 == m_vecEnableController.size())
	{
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<Item>No controller valid to be diagnosed</Item>\n", lpszNextIndent);
	}
	else
	{
		do
		{
			nItemResult = DiagnosisFail(lpszNextIndent);
			if (0 != nItemResult)
			{
				++nFailItemCount;
			}
			nItemResult = DiagnosisPass(lpszNextIndent);
			if (0 != nItemResult)
			{
				++nFailItemCount;
			}

		} while (false);
	}
	Disconnect();


	if (0 == nFailItemCount)
	{
		nRetVal = 0;
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='true'/>\n", lpszNextIndent);
	}
	else
	{
		nRetVal = -1;
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='false'/>\n", lpszNextIndent);
	}

	dTimeConsume = StopTimer(lpszTimeUnits, sizeof(lpszTimeUnits));
	m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<Time value='%.3f%s'/>\n", lpszNextIndent, dTimeConsume, lpszTimeUnits);

	m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s</ComparisonShield>\n", lpszBaseIndent);

	return nRetVal;
}

bool CDiagnosisComparisonShield::IsUserCheck() const
{
	switch (m_UserRole)
	{
	case CDiagnosisItem::PROCUCTION:
	case CDiagnosisItem::DEVELOPER:
		return true;
		break;
	case CDiagnosisItem::USER:
	default:
		return false;
		break;
	}
}

int CDiagnosisComparisonShield::DiagnosisFailComparison(const std::set<USHORT>& setChannel, const char* lpszBaseIndent)
{
	StartTimer();
	USHORT usChannelComparison = 0;
	for (auto Channel : setChannel)
	{
		usChannelComparison |= 1 << Channel;
	}
	set<USHORT> setShieldChannel;
	for (USHORT usChannel = 0; usChannel < DCM_CHANNELS_PER_CONTROL;++usChannel)
	{
		if (setChannel.end() == setChannel.find(usChannel))
		{
			setShieldChannel.insert(usChannel);
		}
	}
	m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<Comparsion value='0x%04X'>\n", lpszBaseIndent, usChannelComparison);
	double dTimeConsume = 0;
	char lpszTimeUnits[4] = { 0 };

	string strFirstIndent = lpszBaseIndent + IndentChar();
	const char* lpszFirstIndent = strFirstIndent.c_str();
	string strSecondIndent = strFirstIndent + IndentChar();
	const char* lpszSecondIndent = strSecondIndent.c_str();

	double dTestRate = 100;
	
	int nRetVal = -1;

	vector<CHardwareFunction::DATA_RESULT> vecBRAMFailData[2];
	vector<CHardwareFunction::DATA_RESULT> vecDRAMFailData[2];

	vector<UINT> vecCurController;
	CHardwareFunction* pHardware = nullptr;
	const char* lpszCurPattern = nullptr;
	string strCurResult;

	CHardwareFunction::DATA_RESULT DataResult;

	UINT uBRAMOffset = 0;
	UINT uDRAMOffset = 0;
	auto SaveDataResult = [&](int nComparsionControllerIndex, BOOL bBRAM)
	{
		DataResult.m_usData = 0;
		lpszCurPattern = strCurResult.c_str();
		UINT& uOffset = bBRAM ? uBRAMOffset : uDRAMOffset;
		auto& vecFailData = bBRAM ? vecBRAMFailData[nComparsionControllerIndex] : vecDRAMFailData[nComparsionControllerIndex];
		for (auto Channel : setChannel)
		{
			if ('1' == lpszCurPattern[Channel])
			{
				DataResult.m_usData |= 1 << Channel;
			}
		}

		if (0 != DataResult.m_usData)
		{
			DataResult.m_nLineNo = uOffset;
			vecFailData.push_back(DataResult);
		}
	};

	for (int nCheckIndex = 0; nCheckIndex < 2; ++nCheckIndex)
	{
		UINT uLineIndex = 0;
		uBRAMOffset = 0;
		uDRAMOffset = 0;
		vecCurController.clear();
		for (auto Controller : m_vecEnableController)
		{
			if (nCheckIndex == Controller % 2)
			{
				vecCurController.push_back(Controller);
			}
		}

		Bind(vecCurController, vecCurController[0]);
		pHardware = GetHardware(vecCurController[0]);
		CPattern Pattern(*pHardware);
		///<BRAM
		if (0 == nCheckIndex)
		{
			strCurResult = "1010101010101010";
			SaveDataResult(1, TRUE);
		}
		else
		{
			strCurResult = "LLLLLLLLLLLLLLLL";
		}
		lpszCurPattern = strCurResult.c_str();
		Pattern.AddPattern(uBRAMOffset++, TRUE, lpszCurPattern, 0, "INC",  "",  0, FALSE, FALSE);

		if (1 == nCheckIndex)
		{
			strCurResult = "1010101010101010";
			SaveDataResult(0, TRUE);
		}
		else
		{
			strCurResult = "LLLLLLLLLLLLLLLL";
		}
		lpszCurPattern = strCurResult.c_str();
		Pattern.AddPattern(uBRAMOffset++, TRUE, lpszCurPattern, 0, "INC",  "",  0, FALSE, FALSE);


		if (0 == nCheckIndex)
		{
			strCurResult = "0101010101010101";
			SaveDataResult(1, TRUE);
		}
		else
		{
			strCurResult = "LLLLLLLLLLLLLLLL";
		}
		lpszCurPattern = strCurResult.c_str();
		Pattern.AddPattern(uBRAMOffset++, TRUE, lpszCurPattern, 0, "INC",  "",  0, FALSE, FALSE);


		if (1 == nCheckIndex)
		{
			strCurResult = "0101010101010101";
			SaveDataResult(0, TRUE); 
		}
		else
		{
			strCurResult = "LLLLLLLLLLLLLLLL";
		}
		lpszCurPattern = strCurResult.c_str();
		Pattern.AddPattern(uBRAMOffset++, TRUE, lpszCurPattern, 0, "INC",  "", 0, FALSE, TRUE);

		///<DRAM
		if (0 == nCheckIndex)
		{
			strCurResult = "1010101010101010";
			SaveDataResult(1, FALSE);
		}
		else
		{
			strCurResult = "LLLLLLLLLLLLLLLL";
		}
		lpszCurPattern = strCurResult.c_str();
		Pattern.AddPattern(uDRAMOffset++, FALSE, lpszCurPattern, 0, "INC",  "", 0, FALSE, FALSE);

		if (1 == nCheckIndex)
		{
			strCurResult = "1010101010101010";
			SaveDataResult(0, FALSE);
		}
		else
		{
			strCurResult = "LLLLLLLLLLLLLLLL";
		}
		lpszCurPattern = strCurResult.c_str();
		Pattern.AddPattern(uDRAMOffset++, FALSE, lpszCurPattern, 0, "INC",  "",  0, FALSE, FALSE);

		if (0 == nCheckIndex)
		{
			strCurResult = "0101010101010101";
			SaveDataResult(1, FALSE);
		}
		else
		{
			strCurResult = "LLLLLLLLLLLLLLLL";
		}
		lpszCurPattern = strCurResult.c_str();
		Pattern.AddPattern(uDRAMOffset++, FALSE, lpszCurPattern, 0, "INC",  "",  0, FALSE, FALSE);

		if (1 == nCheckIndex)
		{
			strCurResult = "0101010101010101";
			SaveDataResult(0, FALSE);
		}
		else
		{
			strCurResult = "LLLLLLLLLLLLLLLL";
		}
		lpszCurPattern = strCurResult.c_str();
		Pattern.AddPattern(uDRAMOffset++, FALSE, lpszCurPattern, 0, "INC",  "",  0, FALSE, FALSE);


		if (0 == nCheckIndex)
		{
			strCurResult = "1111111111111111";
			SaveDataResult(1, FALSE);
		}
		else
		{
			strCurResult = "LLLLLLLLLLLLLLLL";
		}
		lpszCurPattern = strCurResult.c_str();
		Pattern.AddPattern(uDRAMOffset++, FALSE, lpszCurPattern, 0, "INC",  "",  0, FALSE, FALSE);

		if (1 == nCheckIndex)
		{
			strCurResult = "1111111111111111";
			SaveDataResult(0, FALSE);
		}
		else
		{
			strCurResult = "LLLLLLLLLLLLLLLL";
		}
		lpszCurPattern = strCurResult.c_str();
		Pattern.AddPattern(uDRAMOffset++, FALSE, lpszCurPattern, 0, "INC",  "",  0, FALSE, FALSE);


		if (0 == nCheckIndex)
		{
			strCurResult = "0000000000000000";
			SaveDataResult(1, FALSE);
		}
		else
		{
			strCurResult = "LLLLLLLLLLLLLLLL";
		}
		lpszCurPattern = strCurResult.c_str();
		Pattern.AddPattern(uDRAMOffset++, FALSE, lpszCurPattern, 0, "INC",  "",  0, FALSE, FALSE);


		if (1 == nCheckIndex)
		{
			strCurResult = "0000000000000000";
			SaveDataResult(0, FALSE);
		}
		else
		{
			strCurResult = "LLLLLLLLLLLLLLLL";
		}
		lpszCurPattern = strCurResult.c_str();
		Pattern.AddPattern(uDRAMOffset++, FALSE, lpszCurPattern, 0, "INC",  "",  0, FALSE, TRUE);

		///<BRAM
		if (0 == nCheckIndex)
		{
			strCurResult = "1111111111111111";
			SaveDataResult(1, TRUE);
		}
		else
		{
			strCurResult = "LLLLLLLLLLLLLLLL";
		}
		lpszCurPattern = strCurResult.c_str();
		Pattern.AddPattern(uBRAMOffset++, TRUE, lpszCurPattern, 0, "INC",  "",  0, FALSE, FALSE);

		if (1 == nCheckIndex)
		{
			strCurResult = "1111111111111111";
			SaveDataResult(0, TRUE);
		}
		else
		{
			strCurResult = "LLLLLLLLLLLLLLLL";
		}
		lpszCurPattern = strCurResult.c_str();
		Pattern.AddPattern(uBRAMOffset++, TRUE, lpszCurPattern, 0, "INC",  "",  0, FALSE, FALSE);


		if (0 == nCheckIndex)
		{
			strCurResult = "0000000000000000";
			SaveDataResult(1, TRUE);
		}
		else
		{
			strCurResult = "LLLLLLLLLLLLLLLL";
		}
		lpszCurPattern = strCurResult.c_str();
		Pattern.AddPattern(uBRAMOffset++, TRUE, lpszCurPattern, 0, "INC",  "",  0, FALSE, FALSE);

		if (1 == nCheckIndex)
		{
			strCurResult = "0000000000000000";
			SaveDataResult(0, TRUE);
		}
		else
		{
			strCurResult = "LLLLLLLLLLLLLLLL";
		}
		lpszCurPattern = strCurResult.c_str();
		Pattern.AddPattern(uBRAMOffset++, TRUE, lpszCurPattern, 0, "INC",  "",  0, FALSE, FALSE);
		Pattern.Load();
		ClearBind();
	}

	///<Run vector
	Bind(m_vecEnableController, m_vecEnableController[0]);
	pHardware = GetHardware(m_vecEnableController[0]);
	pHardware->SetPeriod(0, dTestRate);
	double adEdge[EDGE_COUNT] = { 0, dTestRate / 2, 0, dTestRate / 2, dTestRate * 3 / 4, dTestRate * 4 / 5 };
	vector<USHORT> vecChannel;
	for (USHORT usChannel = 0; usChannel < DCM_CHANNELS_PER_CONTROL;++usChannel)
	{
		vecChannel.push_back(usChannel);
	}
	pHardware->SetEdge(vecChannel, 0, adEdge, WAVE_FORMAT::NRZ, IO_FORMAT::NRZ, COMPARE_MODE::EDGE);
	pHardware->SetPinLevel(vecChannel, 3, 0, 1.5, 1.5, 0.8);
	pHardware->SetComparedChannel(setChannel);
	pHardware->SetRunParameter(0, uBRAMOffset - 1, TRUE, 0);
	pHardware->SynRun();
	ClearBind();

	WaitStop();

	BYTE bySlotNo = 0;
	byte byBoardControllerIndex = 0;
	BOOL bItemFail = FALSE;
	vector<CHardwareFunction::DATA_RESULT> vecRealBRAMFailData;
	vector<CHardwareFunction::DATA_RESULT> vecRealDRAMFailData;

	for (auto Controller : m_vecEnableController)
	{
		StartTimer();
		BOOL bControllerFail = FALSE;
		bySlotNo = HDModule::Instance()->ID2Board(Controller, byBoardControllerIndex);

		if (nullptr != m_pReportDevice)
		{
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<controller value='%d, slot value = %d'>\n", lpszFirstIndent, byBoardControllerIndex, bySlotNo);
		}

		pHardware = GetHardware(Controller);

		auto& vecExpectedBRAM = vecBRAMFailData[Controller % 2];
		auto& vecExpectedDRAM = vecDRAMFailData[Controller % 2];
		pHardware->GetFailData(vecRealBRAMFailData, vecRealDRAMFailData);
		if (vecRealBRAMFailData != vecExpectedBRAM || vecRealDRAMFailData != vecExpectedDRAM)
		{
			bControllerFail = TRUE;
			char lpszValue[128] = { 0 };
			///<Check BRAM
			int nErrorCount = 0;
			if (vecRealBRAMFailData != vecExpectedBRAM)
			{
				auto iterReal = vecRealBRAMFailData.begin();
				auto iterExpected = vecExpectedBRAM.begin();
				while (vecRealBRAMFailData.end() != iterReal && vecExpectedBRAM.end() != iterExpected)
				{
					if (iterReal->m_nLineNo != iterExpected->m_nLineNo || iterReal->m_usData != iterExpected->m_usData)
					{
						if (ERROR_PRINT < nErrorCount++)
						{
							break;
						}
						sprintf_s(lpszValue, sizeof(lpszValue), "value='BRAM expect addr='0x%X' Real addr=0x%X Expect='0x%X' Real=0x%X'",
							iterExpected->m_nLineNo, iterReal->m_nLineNo, iterExpected->m_usData, iterReal->m_usData);
						m_pReportDevice->PrintfToDataLog(IHDReportDevice::Error, "%s<data retValue='false' %s/>\n", lpszSecondIndent, lpszValue);
					}
					++iterReal;
					++iterExpected;
				}
				if (ERROR_PRINT >= nErrorCount && vecRealBRAMFailData.size() != vecExpectedBRAM.size())
				{
					while (vecRealBRAMFailData.end() != iterReal)
					{
						if (ERROR_PRINT < nErrorCount++)
						{
							break;
						}
						sprintf_s(lpszValue, sizeof(lpszValue), "value='BRAM expect addr='-' Real addr=0x%X Expect='-' Real=0x%X'",
							iterReal->m_nLineNo, iterReal->m_usData);
						m_pReportDevice->PrintfToDataLog(IHDReportDevice::Error, "%s<data retValue='false' %s/>\n", lpszSecondIndent, lpszValue);
						++iterReal;
					}
					while (vecExpectedBRAM.end() != iterExpected)
					{
						if (ERROR_PRINT < nErrorCount++)
						{
							break;
						}
						sprintf_s(lpszValue, sizeof(lpszValue), "value='BRAM expect addr=0x%X Real addr='-' Expect=0x%X Real='-''",
							iterExpected->m_nLineNo, iterExpected->m_usData);
						m_pReportDevice->PrintfToDataLog(IHDReportDevice::Error, "%s<data retValue='false' %s/>\n", lpszSecondIndent, lpszValue);
						++iterExpected;
					}
				}

			}
			if (vecRealDRAMFailData != vecExpectedDRAM)
			{
				nErrorCount = 0;
				auto iterReal = vecRealDRAMFailData.begin();
				auto iterExpected = vecExpectedDRAM.begin();
				while (vecRealDRAMFailData.end() != iterReal && vecExpectedDRAM.end() != iterExpected)
				{
					if (iterReal->m_nLineNo != iterExpected->m_nLineNo || iterReal->m_usData != iterExpected->m_usData)
					{
						if (ERROR_PRINT < nErrorCount++)
						{
							break;
						}
						sprintf_s(lpszValue, sizeof(lpszValue), "value='DRAM expect addr='0x%X' Real addr=0x%X Expect='0x%X' Real=0x%X'",
							iterExpected->m_nLineNo, iterReal->m_nLineNo, iterExpected->m_usData, iterReal->m_usData);
						m_pReportDevice->PrintfToDataLog(IHDReportDevice::Error, "%s<data retValue='false' %s/>\n", lpszSecondIndent, lpszValue);
					}
					++iterReal;
					++iterExpected;
				}
				if (ERROR_PRINT >= nErrorCount && vecRealDRAMFailData.size() != vecExpectedDRAM.size())
				{
					while (vecRealDRAMFailData.end() != iterReal)
					{
						if (ERROR_PRINT < nErrorCount++)
						{
							break;
						}
						sprintf_s(lpszValue, sizeof(lpszValue), "value='DRAM expect addr='-' Real addr=0x%X Expect='-' Real=0x%X'",
							iterReal->m_nLineNo, iterReal->m_usData);
						m_pReportDevice->PrintfToDataLog(IHDReportDevice::Error, "%s<data retValue='false' %s/>\n", lpszSecondIndent, lpszValue);
						++iterReal;
					}
					while (vecExpectedDRAM.end() != iterExpected)
					{
						if (ERROR_PRINT < nErrorCount++)
						{
							break;
						}
						sprintf_s(lpszValue, sizeof(lpszValue), "value='DRAM expect addr=0x%X Real addr='-' Expect=0x%X Real='-''",
							iterExpected->m_nLineNo, iterExpected->m_usData);
						m_pReportDevice->PrintfToDataLog(IHDReportDevice::Error, "%s<data retValue='false' %s/>\n", lpszSecondIndent, lpszValue);
						++iterExpected;
					}
				}

			}

			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='false'/>\n", lpszSecondIndent);
		}
		else
		{
			///<Check run result
			int nRunResult = 0;
			for (auto Channel : setShieldChannel)
			{
				nRunResult = pHardware->GetChannelResult(Channel);
				if (0 != nRunResult)
				{
					char lpszValue[128] = { 0 };
					sprintf_s(lpszValue, sizeof(lpszValue), "value='Channel %d Expect=%d Real=%d'", Channel, 0, nRunResult);
					m_pReportDevice->PrintfToDataLog(IHDReportDevice::Error, "%s<data retValue='false' %s/>\n", lpszSecondIndent, lpszValue);
					bControllerFail = TRUE;
				}
			}
		}
		if (bControllerFail)
		{
			bItemFail = TRUE;
			m_setFailController.insert(Controller);
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='false'/>\n", lpszSecondIndent);
		}
		else
		{
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='true'/>\n", lpszSecondIndent);
		}
		dTimeConsume = StopTimer(lpszTimeUnits, sizeof(lpszTimeUnits));
		if (nullptr != m_pReportDevice)
		{
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<Time value='%.3f%s'/>\n", lpszSecondIndent, dTimeConsume, lpszTimeUnits);
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s</controller>\n", lpszFirstIndent);
		}
	}

	dTimeConsume = StopTimer(lpszTimeUnits, sizeof(lpszTimeUnits));
	if (nullptr != m_pReportDevice)
	{
		if (bItemFail)
		{
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='false'/>\n", lpszFirstIndent);
		}
		else
		{
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='true'/>\n", lpszFirstIndent);
		}
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<Time value='%.3f%s'/>\n", lpszFirstIndent, dTimeConsume, lpszTimeUnits);
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s</Comparsion>\n", lpszBaseIndent);
	}

	if (bItemFail)
	{
		return -1;
	}
	return 0;
}

int CDiagnosisComparisonShield::DiagnosisPassComparison(const std::set<USHORT>& setChannel, const char* lpszBaseIndent)
{
	StartTimer();
	USHORT usChannelComparison = 0;
	for (auto Channel : setChannel)
	{
		usChannelComparison |= 1 << Channel;
	}
	set<USHORT> setShieldChannel;
	for (USHORT usChannel = 0; usChannel < DCM_CHANNELS_PER_CONTROL; ++usChannel)
	{
		if (setChannel.end() == setChannel.find(usChannel))
		{
			setShieldChannel.insert(usChannel);
		}
	}
	m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<Comparsion value='0x%04X'>\n", lpszBaseIndent, usChannelComparison);
	double dTimeConsume = 0;
	char lpszTimeUnits[4] = { 0 };

	string strFirstIndent = lpszBaseIndent + IndentChar();
	const char* lpszFirstIndent = strFirstIndent.c_str();
	string strSecondIndent = strFirstIndent + IndentChar();
	const char* lpszSecondIndent = strSecondIndent.c_str();

	double dTestRate = 100;

	int nRetVal = -1;

	vector<UINT> vecCurController;
	CHardwareFunction* pHardware = nullptr;
	const char* lpszCurPattern = nullptr;
	string strCurResult;

	UINT uBRAMOffset = 0;
	UINT uDRAMOffset = 0;
	auto GetPattern = [&](BOOL bDriver)
	{
		strCurResult.clear();
		if (!bDriver)
		{
			strCurResult = "LLLLLLLLLLLLLLLL";
			return;
		}
		for (USHORT usChannel = 0; usChannel < DCM_CHANNELS_PER_CONTROL;++usChannel)
		{
			strCurResult += setShieldChannel.end() != setShieldChannel.find(usChannel) ? '1' : '0';
		}
	};

	for (int nCheckIndex = 0; nCheckIndex < 2; ++nCheckIndex)
	{
		UINT uLineIndex = 0;
		uBRAMOffset = 0;
		uDRAMOffset = 0;
		vecCurController.clear();
		for (auto Controller : m_vecEnableController)
		{
			if (nCheckIndex == Controller % 2)
			{
				vecCurController.push_back(Controller);
			}
		}

		Bind(vecCurController, vecCurController[0]);
		pHardware = GetHardware(vecCurController[0]);
		CPattern Pattern(*pHardware);
		///<BRAM
		if (0 == nCheckIndex)
		{
			GetPattern(TRUE);
		}
		else
		{
			GetPattern(FALSE);
		}
		lpszCurPattern = strCurResult.c_str();
		Pattern.AddPattern(uBRAMOffset++, TRUE, lpszCurPattern, 0, "INC",  "",  0, FALSE, FALSE);

		if (1 == nCheckIndex)
		{
			GetPattern(TRUE);
		}
		else
		{
			GetPattern(FALSE);
		}
		lpszCurPattern = strCurResult.c_str();
		Pattern.AddPattern(uBRAMOffset++, TRUE, lpszCurPattern, 0, "INC",  "",  0, FALSE, TRUE);


		///<DRAM
		if (0 == nCheckIndex)
		{
			GetPattern(TRUE);
		}
		else
		{
			GetPattern(FALSE);
		}
		lpszCurPattern = strCurResult.c_str();
		Pattern.AddPattern(uDRAMOffset++, FALSE, lpszCurPattern, 0, "INC",  "",  0, FALSE, FALSE);

		if (1 == nCheckIndex)
		{
			GetPattern(TRUE);
		}
		else
		{
			GetPattern(FALSE);
		}
		lpszCurPattern = strCurResult.c_str();
		Pattern.AddPattern(uDRAMOffset++, FALSE, lpszCurPattern, 0, "INC",  "",  0, FALSE, FALSE);
		if (0 == nCheckIndex)
		{
			GetPattern(TRUE);
		}
		else
		{
			GetPattern(FALSE);
		}
		lpszCurPattern = strCurResult.c_str();
		Pattern.AddPattern(uDRAMOffset++, FALSE, lpszCurPattern, 0, "INC",  "",  0, FALSE, FALSE);

		if (1 == nCheckIndex)
		{
			GetPattern(TRUE);
		}
		else
		{
			GetPattern(FALSE);
		}
		lpszCurPattern = strCurResult.c_str();
		Pattern.AddPattern(uDRAMOffset++, FALSE, lpszCurPattern, 0, "INC",  "",  0, FALSE, TRUE);

		///<BRAM
		if (1 == nCheckIndex)
		{
			GetPattern(TRUE);
		}
		else
		{
			GetPattern(FALSE);
		}
		lpszCurPattern = strCurResult.c_str();
		Pattern.AddPattern(uBRAMOffset++, TRUE, lpszCurPattern, 0, "INC",  "",  0, FALSE, FALSE);

		if (0 == nCheckIndex)
		{
			GetPattern(TRUE);
		}
		else
		{
			GetPattern(FALSE);
		}
		lpszCurPattern = strCurResult.c_str();
		Pattern.AddPattern(uBRAMOffset++, TRUE, lpszCurPattern, 0, "INC",  "",  0, FALSE, FALSE);


		Pattern.Load();
		ClearBind();
	}
	
	///<Run vector
	Bind(m_vecEnableController, m_vecEnableController[0]);
	pHardware = GetHardware(m_vecEnableController[0]);
	pHardware->SetPeriod(0, dTestRate);
	double adEdge[EDGE_COUNT] = { 0, dTestRate / 2, 0, dTestRate / 2, dTestRate * 3 / 4, dTestRate * 4 / 5 };
	vector<USHORT> vecChannel;
	for (USHORT usChannel = 0; usChannel < DCM_CHANNELS_PER_CONTROL; ++usChannel)
	{
		vecChannel.push_back(usChannel);
	}
	pHardware->SetEdge(vecChannel, 0, adEdge, WAVE_FORMAT::NRZ, IO_FORMAT::NRZ, COMPARE_MODE::EDGE);
	pHardware->SetPinLevel(vecChannel, 3, 0, 1.5, 1.5, 0.8);
	pHardware->SetComparedChannel(setChannel);
	pHardware->SetRunParameter(0, uBRAMOffset - 1, TRUE, 0);
	pHardware->SynRun();
	ClearBind();

	WaitStop();

	vecChannel.clear();
	copy(setChannel.begin(), setChannel.end(), back_inserter(vecChannel));

	BYTE bySlotNo = 0;
	byte byBoardControllerIndex = 0;
	BOOL bItemFail = FALSE;
	vector<CHardwareFunction::DATA_RESULT> vecRealBRAMFailData;
	vector<CHardwareFunction::DATA_RESULT> vecRealDRAMFailData;

	for (auto Controller : m_vecEnableController)
	{
		StartTimer();
		BOOL bControllerFail = FALSE;
		bySlotNo = HDModule::Instance()->ID2Board(Controller, byBoardControllerIndex);

		if (nullptr != m_pReportDevice)
		{
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<controller value='%d, slot value = %d'>\n", lpszFirstIndent, byBoardControllerIndex, bySlotNo);
		}

		pHardware = GetHardware(Controller);
		int nResult = pHardware->GetMCUResult(vecChannel);
		if (0 != nResult)
		{
			bControllerFail = TRUE;
			char lpszValue[128] = { 0 };
			sprintf_s(lpszValue, sizeof(lpszValue), "value='Result Expect='0x%X' Real=0x%X'", 0, nResult);
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Error, "%s<data retValue='false' %s/>\n", lpszSecondIndent, lpszValue);

			int nErrorCount = 0;
			pHardware->GetFailData(vecRealBRAMFailData, vecRealDRAMFailData);
			for (auto& FailData : vecRealBRAMFailData)
			{
				sprintf_s(lpszValue, sizeof(lpszValue), "value='BRAM fail Addr='0x%X' Real=0x%04X'", FailData.m_nLineNo, FailData.m_usData);
				m_pReportDevice->PrintfToDataLog(IHDReportDevice::Error, "%s<data retValue='false' %s/>\n", lpszSecondIndent, lpszValue);
				if (ERROR_PRINT < nErrorCount++)
				{
					break;
				}
			}
			for (auto& FailData : vecRealDRAMFailData)
			{
				sprintf_s(lpszValue, sizeof(lpszValue), "value='DRAM fail Addr='0x%X' Real=0x%04X'", FailData.m_nLineNo, FailData.m_usData);
				m_pReportDevice->PrintfToDataLog(IHDReportDevice::Error, "%s<data retValue='false' %s/>\n", lpszSecondIndent, lpszValue);
				if (ERROR_PRINT < nErrorCount++)
				{
					break;
				}
			}
		}
		if (bControllerFail)
		{
			bItemFail = TRUE;
			m_setFailController.insert(Controller);
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='false'/>\n", lpszSecondIndent);
		}
		else
		{
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='true'/>\n", lpszSecondIndent);
		}
		dTimeConsume = StopTimer(lpszTimeUnits, sizeof(lpszTimeUnits));
		if (nullptr != m_pReportDevice)
		{
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<Time value='%.3f%s'/>\n", lpszSecondIndent, dTimeConsume, lpszTimeUnits);
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s</controller>\n", lpszFirstIndent);
		}
	}

	dTimeConsume = StopTimer(lpszTimeUnits, sizeof(lpszTimeUnits));
	if (nullptr != m_pReportDevice)
	{
		if (bItemFail)
		{
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='false'/>\n", lpszFirstIndent);
		}
		else
		{
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='true'/>\n", lpszFirstIndent);
		}
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<Time value='%.3f%s'/>\n", lpszFirstIndent, dTimeConsume, lpszTimeUnits);
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s</Comparsion>\n", lpszBaseIndent);
	}

	if (bItemFail)
	{
		return -1;
	}
	return 0;
}

void CDiagnosisComparisonShield::GetItem(std::map<std::string, std::set<USHORT>>& mapItem)
{
	mapItem.clear();
	set<USHORT> setComparisonChannel;
	mapItem.insert(make_pair("AllShield", setComparisonChannel));

	for (USHORT usChannel = 0; usChannel < DCM_CHANNELS_PER_CONTROL; ++usChannel)
	{
		if (4 <= usChannel)
		{
			setComparisonChannel.insert(usChannel);
		}
	}
	mapItem.insert(make_pair("ShieldFirstFour", setComparisonChannel));
	setComparisonChannel.clear();

	for (USHORT usChannel = 0; usChannel < DCM_CHANNELS_PER_CONTROL; ++usChannel)
	{
		if (DCM_CHANNELS_PER_CONTROL - 4 > usChannel)
		{
			setComparisonChannel.insert(usChannel);
		}
	}
	mapItem.insert(make_pair("ShieldLastFour", setComparisonChannel));
	setComparisonChannel.clear();

	for (USHORT usChannel = 0; usChannel < DCM_CHANNELS_PER_CONTROL; ++usChannel)
	{
		if (DCM_CHANNELS_PER_CONTROL / 4 > usChannel || DCM_CHANNELS_PER_CONTROL * 3 / 4 <= usChannel)
		{
			setComparisonChannel.insert(usChannel);
		}
	}
	mapItem.insert(make_pair("ShieldMidEight", setComparisonChannel));
	setComparisonChannel.clear();


	for (USHORT usChannel = 0; usChannel < DCM_CHANNELS_PER_CONTROL; ++usChannel)
	{
		if (0 == usChannel % 2)
		{
			setComparisonChannel.insert(usChannel);
		}
	}
	mapItem.insert(make_pair("ShieldOdd", setComparisonChannel));
	setComparisonChannel.clear();

	for (USHORT usChannel = 0; usChannel < DCM_CHANNELS_PER_CONTROL; ++usChannel)
	{
		if (1 == usChannel % 2)
		{
			setComparisonChannel.insert(usChannel);
		}
	}
	mapItem.insert(make_pair("ShieldEven", setComparisonChannel));
	setComparisonChannel.clear();

	for (USHORT usChannel = 0; usChannel < DCM_CHANNELS_PER_CONTROL / 2; ++usChannel)
	{
		setComparisonChannel.insert(usChannel);
	}
	mapItem.insert(make_pair("ShieldLastEight", setComparisonChannel));
	setComparisonChannel.clear();

	for (USHORT usChannel = DCM_CHANNELS_PER_CONTROL / 2; usChannel < DCM_CHANNELS_PER_CONTROL; ++usChannel)
	{
		setComparisonChannel.insert(usChannel);
	}
	mapItem.insert(make_pair("ShieldFirstEight", setComparisonChannel));
	setComparisonChannel.clear();

	for (USHORT usChannel = 0; usChannel < DCM_CHANNELS_PER_CONTROL; ++usChannel)
	{
		setComparisonChannel.insert(usChannel);
	}
	mapItem.insert(make_pair("NotShield", setComparisonChannel));
	setComparisonChannel.clear();
}


int CDiagnosisComparisonShield::DiagnosisPass(const char* lpszBaseIndent)
{
	StartTimer();
	double dTimeConsume = 0;
	char lpszTimeUnits[4] = { 0 };
	m_pReportDevice->PrintfToUi(" Diagnosis Pass\n");

	m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<DiagnosisPass>\n", lpszBaseIndent);
	string strFirstIndent = lpszBaseIndent + IndentChar();
	const char* lpszFirstIndent = strFirstIndent.c_str();
	string strSecond = strFirstIndent + IndentChar();
	const char* lpszSecond = strFirstIndent.c_str();
	int nItemResult = 0;
	int nFailItemCount = 0;
	map<string, set<USHORT>> mapDiagnosis;
	GetItem(mapDiagnosis);
	int nRetVal = 0;
	for (auto& Item : mapDiagnosis)
	{
		if (m_pReportDevice->IsStop())
		{
			string strItem = Item.first;
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<UserStop value='NextItem=%s'/>\n", lpszFirstIndent, strItem.c_str());
			break;
		}
		nItemResult = DiagnosisPassComparison(Item.second, lpszSecond);
		if (0 != nItemResult)
		{
			++nFailItemCount;
		}
	}
	if (0 == nFailItemCount)
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

	m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s</DiagnosisPass>\n", lpszBaseIndent);

	ShowUIResult();

	if (0 == nFailItemCount)
	{
		return 0;
	}
	return -1;
}

int CDiagnosisComparisonShield::DiagnosisFail(const char* lpszBaseIndent)
{
	StartTimer();
	double dTimeConsume = 0;
	char lpszTimeUnits[4] = { 0 };

	m_pReportDevice->PrintfToUi(" Diagnosis Fail\n");

	m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<DiagnosisFail>\n", lpszBaseIndent);
	string strFirstIndent = lpszBaseIndent + IndentChar();
	const char* lpszFirstIndent = strFirstIndent.c_str();
	string strSecond = strFirstIndent + IndentChar();
	const char* lpszSecondIndent = strFirstIndent.c_str();
	int nItemResult = 0;
	int nFailItemCount = 0;
	map<string, set<USHORT>> mapDiagnosis;
	GetItem(mapDiagnosis);
	int nRetVal = 0;
	for (auto& Item : mapDiagnosis)
	{
		if (m_pReportDevice->IsStop())
		{
			string strItem = Item.first;
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<UserStop value='NextItem=%s'/>\n", lpszFirstIndent, strItem.c_str());
			break;
		}
		nItemResult = DiagnosisFailComparison(Item.second, lpszSecondIndent);
		if (0 != nItemResult)
		{
			++nFailItemCount;
		}
	}
	if (0 == nFailItemCount)
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

	m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s</DiagnosisFail>\n", lpszBaseIndent);

	ShowUIResult();

	return nRetVal;
}

void CDiagnosisComparisonShield::ShowUIResult()
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
