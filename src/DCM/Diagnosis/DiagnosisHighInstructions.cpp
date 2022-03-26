#include "DiagnosisHighInstructions.h"
#include <vector>
#include "IHDReportDevice.h""
#include "..\HDModule.h"
#include "..\Pattern.h"
using namespace std;

CDiagnosisHighInstructions::CDiagnosisHighInstructions()
{}

CDiagnosisHighInstructions::~CDiagnosisHighInstructions()
{}

int CDiagnosisHighInstructions::QueryInstance(const char * name, void ** ptr)
{
    return -1;
}

void CDiagnosisHighInstructions::Release()
{}

const char * CDiagnosisHighInstructions::Name() const
{
    return "Instruction Diagnosis";
}

int CDiagnosisHighInstructions::GetChildren(STSVector<IHDDoctorItem*>& vecChildren) const
{
	return 0;
}

int CDiagnosisHighInstructions::Doctor(IHDReportDevice * pReportDevice)
{
	m_pReportDevice = pReportDevice;
	StartTimer();
	double dTimeConsume = 0;
	char lpszTimeUnits[4] = { 0 };
	const char* lpszBaseIndent = IndentFormat();
    m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<InstructionDiagnosis>\n", lpszBaseIndent);
	std::string strFirstIndent = lpszBaseIndent + IndentChar();
	const char* lpszFirstIndent = strFirstIndent.c_str();

    int nFailCount = 0;
    do 
	{
		m_pReportDevice->PrintfToUi(" INC\n");
        if (0 != CheckINC(lpszFirstIndent))
		{
            ++nFailCount;
			if (1 != m_pReportDevice->IsFailContinue())
			{
				break;
			}
        }
        if (1 == m_pReportDevice->IsStop())
		{
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<UserStop value='NextINS=REPEAT'/>\n", lpszFirstIndent);
            break;
		}

		m_pReportDevice->PrintfToUi(" REPEAT\n");
        if (0 != CheckRepeat(lpszFirstIndent))
		{
            ++nFailCount;
			if (1 != m_pReportDevice->IsFailContinue())
			{
				break;
			}
        }
		if ( 1 == m_pReportDevice->IsStop())
		{
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<UserStop value='NextINS=CALL-RETURN'/>\n", lpszFirstIndent);
			break;
		}

		m_pReportDevice->PrintfToUi(" CALL-RETURN\n");
        if (0 != CheckCallReturn(lpszFirstIndent))
		{
            ++nFailCount;
			if (1 != m_pReportDevice->IsFailContinue())
			{
				break;
			}
        }
		if (1 == m_pReportDevice->IsStop())
		{
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<UserStop value='NextINS=LOOP'/>\n", lpszFirstIndent);
			break;
		}

		m_pReportDevice->PrintfToUi(" LOOP\n");
        if (0 != CheckLoop(lpszFirstIndent))
		{
            ++nFailCount;
			if (1 != m_pReportDevice->IsFailContinue())
			{
				break;
			}
        }

		if (m_pReportDevice->IsStop())
		{
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<UserStop/>\n", lpszFirstIndent);
			break;
		}
    } while (false);

    int nRetVal = -1;	

    if (0 == nFailCount)
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

    m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s</Op-InstructionDiagnosis>\n", lpszBaseIndent);
    return nRetVal;
}

int CDiagnosisHighInstructions::CheckINC(const char* lpszBaseIndent)
{
	if (0 == m_vecEnableController.size())
	{
		return -1;
	}

	StartTimer();
	double dTimeConsume = 0;
	char lpszTimeUnits[4] = { 0 };

	std::string strNextIndent = lpszBaseIndent + IndentChar();
	const char* lpszNextIndent = strNextIndent.c_str();
	m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<INCTest>\n", lpszBaseIndent);
	int nRetVal = -1;

	vector<UINT> vecLineOrder;

	Bind(m_vecEnableController, m_vecEnableController[0]);
	CHardwareFunction* pHardware = GetHardware(m_vecEnableController[0]);
	CPattern Pattern(*pHardware);

	const int nPatternLineCount = 4094;
	const int nINCLineCount = nPatternLineCount - 1;
	UINT uLineIndex = 0;
	for (uLineIndex = 0; uLineIndex < nINCLineCount; ++uLineIndex)
	{
		Pattern.AddPattern(uLineIndex, TRUE, "001100LLL00HHH00", 0, "INC", "", 0, FALSE, FALSE);
		vecLineOrder.push_back(uLineIndex);
	}
	Pattern.AddPattern(uLineIndex, TRUE, "0000000000000000", 0, "HLT", "", 0, FALSE, FALSE);//ָֹͣ��
	vecLineOrder.push_back(uLineIndex);

	Pattern.Load();
	ClearBind();

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
	vecPeriod.push_back(500);
	vecPeriod.push_back(1e3);
	vecPeriod.push_back(5e3);
	vecPeriod.push_back(10e3);
	vecPeriod.push_back(15e3);
	vecPeriod.push_back(50e3);
	vecPeriod.push_back(500e3);
	vecPeriod.push_back(1e6);
	vecPeriod.push_back(4e6);
	int nFailCount = 0;
	for (auto dPeriod : vecPeriod)
	{
		double dShowPeriod = GetPeriodUnits(dPeriod, lpszTimeUnits, sizeof(lpszTimeUnits));
		if (1 == m_pReportDevice->IsStop())
		{
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<UserStop value='NextPeriod=%.1f%s'/>\n", lpszNextIndent, dShowPeriod, lpszTimeUnits);
			break;
		}
		int nResult = CheckPeriod(lpszNextIndent, dPeriod, nPatternLineCount, vecLineOrder);
		if (0 != nResult)
		{
			++nFailCount;
			if (1 != m_pReportDevice->IsFailContinue())
			{
				break;
			}
		}
	}
	if (0 == nFailCount)
	{
		nRetVal = 0;
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='true'/>\n", lpszNextIndent);
	}
	else
	{
		nRetVal = -1;
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='false'/>\n", lpszNextIndent);
	}

	ShowUIResult();

	dTimeConsume = StopTimer(lpszTimeUnits, sizeof(lpszTimeUnits));
	m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<Time value='%.3f%s'/>\n", lpszNextIndent, dTimeConsume, lpszTimeUnits);
	m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s</INCTest>\n", lpszBaseIndent);
	return nRetVal;
}

int CDiagnosisHighInstructions::CheckRepeat(const char* lpszBaseIndent)
{
	if (0 == m_vecEnableController.size())
	{
		return -1;
	}

	StartTimer();
	m_setFailController.clear();
	double dTimeConsume = 0;
	char lpszTimeUnits[4] = { 0 };

	std::string strNextIndent = lpszBaseIndent + IndentChar();
	const char* lpszNextIndent = strNextIndent.c_str();
	m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<RepeatTest>\n", lpszBaseIndent);
	int nRetVal = -1;

	//3 д������
	vector<UINT> vecLineOrder;
	const int nLineCount = 4094;

	UINT uBindControllerID = m_vecEnableController[0];
	BYTE byBoardControllerIndex = 0;
	BYTE bySlotNo = HDModule::Instance()->ID2Board(uBindControllerID, byBoardControllerIndex);

	Bind(m_vecEnableController, uBindControllerID);
	CHardwareFunction* pHardware = GetHardware(uBindControllerID);
	CPattern Pattern(*pHardware);

	int nCurLineIndex = 3;
	for (int nLineIndex = 0; nLineIndex < nCurLineIndex;++nLineIndex)
	{
		vecLineOrder.push_back(nLineIndex);
	}
	int nRepeatCount = 0;
	BOOL bOverRange = FALSE;
	for (UINT uLineIndex = 0; uLineIndex < nLineCount; ++uLineIndex) //4096
	{
		if (nCurLineIndex == uLineIndex)
		{
			nRepeatCount = nCurLineIndex;
			if (vecLineOrder.size() + nRepeatCount + 2 >= nLineCount - 1)
			{
				nRepeatCount = nLineCount - 3 - vecLineOrder.size();
				bOverRange = TRUE;
			}

			Pattern.AddPattern(uLineIndex, TRUE, "0000000000000000", 0, "REPEAT", "", nRepeatCount, FALSE, FALSE);
			++uLineIndex; //������ת����
			for (int nIndex = 0; nIndex <= nRepeatCount; ++nIndex)
			{
				vecLineOrder.push_back(nCurLineIndex);
			}

			nCurLineIndex = nCurLineIndex * 2 + 1;
			if (bOverRange)
			{
				nCurLineIndex = nLineCount;
			}
			Pattern.AddPattern(uLineIndex, TRUE, "0000000000000000", 0, "JUMP", "", nCurLineIndex, FALSE, FALSE);
			vecLineOrder.push_back(uLineIndex);
		}
		else
		{
			Pattern.AddPattern(uLineIndex, TRUE, "0000000000000000", 0, "INC", "", 0, FALSE, FALSE);
		}
	}
	Pattern.AddPattern(nCurLineIndex,TRUE,  "0000000000000000", 0, "HLT", "", 0, FALSE, FALSE);
	vecLineOrder.push_back(nCurLineIndex);

	Pattern.Load();
	ClearBind();
	do
	{
		std::vector<double> vecPeriod;
		vecPeriod.push_back(12);
		vecPeriod.push_back(16);
		vecPeriod.push_back(25);
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
		vecPeriod.push_back(200);
		vecPeriod.push_back(2e3);
		vecPeriod.push_back(8e3);
		vecPeriod.push_back(12e3);
		vecPeriod.push_back(17e3);
		vecPeriod.push_back(100e3);
		vecPeriod.push_back(200e3);
		vecPeriod.push_back(1e6);
		vecPeriod.push_back(4e6);
		int nFailCount = 0;
		int nPeriodCount = vecPeriod.size();
		
		for (auto dPeriod : vecPeriod)
		{
			double dShowPeriod = GetPeriodUnits(dPeriod, lpszTimeUnits, sizeof(lpszTimeUnits));
			if (1 == m_pReportDevice->IsStop())
			{
				m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<UserStop value='NextPeriod=%.1f%s'/>\n", lpszNextIndent, dShowPeriod, lpszTimeUnits);
				break;
			}
			int nResult = CheckPeriod(lpszNextIndent, dPeriod, nLineCount, vecLineOrder);
			if (0 != nResult)
			{
				++nFailCount;
				if (1 != m_pReportDevice->IsFailContinue())
				{
					break;
				}
			}
		}
		if (0 == nFailCount)
		{
			nRetVal = 0;
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='true'/>\n", lpszNextIndent);
		}
		else
		{
			nRetVal = -1;
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='false'/>\n", lpszNextIndent);
		}
	} while (false);

	ShowUIResult();

	dTimeConsume = StopTimer(lpszTimeUnits, sizeof(lpszTimeUnits));
	m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<Time value='%.3f%s'/>\n", lpszNextIndent, dTimeConsume, lpszTimeUnits);
	m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s</RepeatTest>\n", lpszBaseIndent);
	return nRetVal;
}

int CDiagnosisHighInstructions::CheckCallReturn(const char* lpszBaseIndent)
{
	if (0 >= m_vecEnableController.size())
	{
		return -1;
	}

	StartTimer();

	m_setFailController.clear();

	double dTimeConsume = 0;
	char lpszTimeUnits[4] = { 0 };

	std::string strNextIndent = lpszBaseIndent + IndentChar();
	const char* lpszNextIndent = strNextIndent.c_str();
	m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<CallReturnTest>\n", lpszBaseIndent);
	int nRetVal = -1;

	Bind(m_vecEnableController, m_vecEnableController[0]);
	CHardwareFunction* pHardware = GetHardware(m_vecEnableController[0]);
	CPattern Pattern(*pHardware);
	const int nMaxCount = 8192;

	//3 д������
	UINT uLineIndex = 0;
	for (uLineIndex = 0; uLineIndex < nMaxCount; ++uLineIndex)
	{
		if (uLineIndex == 2)
		{
			Pattern.AddPattern(uLineIndex, TRUE, "0000000000000000", 0, "INC", "", 0, FALSE, FALSE); //!< ��2��
			++uLineIndex;
			Pattern.AddPattern(uLineIndex, TRUE, "0000000000000000", 0, "INC", "", 0, FALSE, FALSE);  //!< ��3��
			++uLineIndex;
			Pattern.AddPattern(uLineIndex, TRUE, "0000000000000000", 0, "CALL", "", 256, FALSE, FALSE);//!< ��4��
			continue;
		}
		if (uLineIndex == 7)
		{
			Pattern.AddPattern( uLineIndex,TRUE, "0000000000000000", 0, "INC", "", 0, FALSE, FALSE);//!< ��7��
			++uLineIndex;
			Pattern.AddPattern(uLineIndex, TRUE, "0000000000000000", 0, "JUMP", "", 512, FALSE, FALSE); //!< ��8��
			++uLineIndex;
			Pattern.AddPattern( uLineIndex,TRUE, "0000000000000000", 0, "HLT", "", 0, FALSE, FALSE);//!< ��9��
			continue;
		}
		if (uLineIndex == 256)
		{
			Pattern.AddPattern(uLineIndex, TRUE, "0000000000000000", 0, "INC", "", 0, FALSE, FALSE); //!< ��256��
			++uLineIndex;
			Pattern.AddPattern(uLineIndex, TRUE, "0000000000000000", 0, "RETURN", "", 0, FALSE, FALSE);//!< ��257��
			continue;
		}
		if (uLineIndex == 512)
		{
			Pattern.AddPattern(uLineIndex, TRUE, "0000000000000000", 0, "INC", "", 0, FALSE, FALSE); //!<  ��512��
			++uLineIndex;
			Pattern.AddPattern(uLineIndex, TRUE, "0000000000000000", 0, "INC", "", 0, FALSE, FALSE);//!<  ��513��
			++uLineIndex;
			Pattern.AddPattern(uLineIndex, TRUE, "0000000000000000", 0, "INC", "", 0, FALSE, FALSE); //!<  ��514�� 
			++uLineIndex;
			Pattern.AddPattern(uLineIndex, TRUE, "0000000000000000", 0, "INC", "", 0, FALSE, FALSE);//!< ��515��
			++uLineIndex;
			Pattern.AddPattern(uLineIndex, TRUE, "0000000000000000", 0, "JUMP", "", 1024, FALSE, FALSE); //!< ��516��
			continue;
		}
		if (uLineIndex == 1024)
		{
			Pattern.AddPattern(uLineIndex, TRUE, "0000000000000000", 0, "INC", "", 0, FALSE, FALSE); //!<  ��1024�� 
			++uLineIndex;
			Pattern.AddPattern(uLineIndex, TRUE, "0000000000000000", 0, "INC", "", 0, FALSE, FALSE); //!<  ��1025��
			++uLineIndex;
			Pattern.AddPattern(uLineIndex, TRUE, "0000000000000000", 0, "HLT", "", 0, FALSE, FALSE);//ָֹͣ��
			++uLineIndex;
			continue;
		}
		Pattern.AddPattern( uLineIndex,TRUE, "0000000000000000", 0, "INC", "", 0,FALSE, FALSE);
	}

	std::vector<UINT> vecLineOrder;
	vecLineOrder.push_back(0);
	vecLineOrder.push_back(1);
	vecLineOrder.push_back(2);
	vecLineOrder.push_back(3);
	vecLineOrder.push_back(4);
	vecLineOrder.push_back(256);
	vecLineOrder.push_back(257);
	vecLineOrder.push_back(5);
	vecLineOrder.push_back(6);
	vecLineOrder.push_back(7);
	vecLineOrder.push_back(8);
	vecLineOrder.push_back(512);
	vecLineOrder.push_back(513);
	vecLineOrder.push_back(514);
	vecLineOrder.push_back(515);
	vecLineOrder.push_back(516);
	vecLineOrder.push_back(1024);
	vecLineOrder.push_back(1025);
	vecLineOrder.push_back(1026);
	//4 ��������
	Pattern.Load();
	ClearBind();

	do
	{
		std::vector<double> vecPeriod;
		vecPeriod.push_back(12);
		vecPeriod.push_back(16);
		vecPeriod.push_back(25);
		vecPeriod.push_back(27);
		vecPeriod.push_back(30); 
		vecPeriod.push_back(32);
		vecPeriod.push_back(35); 
		vecPeriod.push_back(37);
		vecPeriod.push_back(40); 
		vecPeriod.push_back(45);
		vecPeriod.push_back(50);
		vecPeriod.push_back(60);
		vecPeriod.push_back(70);
		vecPeriod.push_back(80);
		vecPeriod.push_back(90);
		vecPeriod.push_back(100);
		vecPeriod.push_back(300);
		vecPeriod.push_back(3e3);
		vecPeriod.push_back(9e3);
		vecPeriod.push_back(14e3);
		vecPeriod.push_back(19e3);
		vecPeriod.push_back(70e3);
		vecPeriod.push_back(100e3);
		vecPeriod.push_back(300e3);
		vecPeriod.push_back(1e6);
		vecPeriod.push_back(4e6);
		int nFailCount = 0;
		int nPeriodCount = vecPeriod.size();
		for (auto dPeriod : vecPeriod)
		{
			if (1 == m_pReportDevice->IsStop())
			{
				double dShowPeriod = GetPeriodUnits(dPeriod, lpszTimeUnits, sizeof(lpszTimeUnits));
				m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<UserStop value='NextPeriod=%.1f%s'/>\n", lpszNextIndent, dShowPeriod, lpszTimeUnits);
				break;
			}
			int nResult = CheckPeriod(lpszNextIndent, dPeriod, uLineIndex + 1, vecLineOrder);
			if (0 != nResult)
			{
				++nFailCount;
				if (1 != m_pReportDevice->IsFailContinue())
				{
					break;
				}
			}
		}
		if (0 == nFailCount)
		{
			nRetVal = 0;
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='true'/>\n", lpszNextIndent);
		}
		else
		{
			nRetVal = -1;
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='false'/>\n", lpszNextIndent);
		}
	} while (false);

	ShowUIResult();

	dTimeConsume = StopTimer(lpszTimeUnits, sizeof(lpszTimeUnits));
	m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<Time value='%.3f%s'/>\n", lpszNextIndent, dTimeConsume, lpszTimeUnits);

	m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s</CallReturnTest>\n", lpszBaseIndent);
	return nRetVal;
}

int CDiagnosisHighInstructions::CheckLoop(const char* lpszBaseIndent)
{
	if (0 >= m_vecEnableController.size())
	{
		return 0;
	}

	StartTimer();
	m_setFailController.clear();
	double dTimeConsume = 0;
	char lpszTimeUnits[4] = { 0 };

	string strNextIndent = lpszBaseIndent + IndentChar();
	const char* lpszNextIndent = strNextIndent.c_str();
	m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<LoopingTest>\n", lpszBaseIndent);
	int nRetVal = -1;
	
	std::vector<UINT> vecLineOrder;
	Bind(m_vecEnableController, m_vecEnableController[0]);
	CHardwareFunction* pHardware = GetHardware(m_vecEnableController[0]);
	CPattern Pattern(*pHardware);

	do
	{
		size_t uLineIndex = 0;
		const int nLineCount = 4094;
		for (uLineIndex = 0; uLineIndex < nLineCount; ++uLineIndex)
		{
			if (2 == uLineIndex)
			{
				///<LOOP_C
				for (int nIndex = 0; nIndex < 2;++nIndex)
				{
					vecLineOrder.push_back(nIndex);
				}

				int nLoopCount = 32;
				Pattern.AddPattern(uLineIndex, TRUE, "0000000000000000", 0, "SET_LOOPC", "", nLoopCount, FALSE, FALSE);//!< ��2��
				++uLineIndex;
				Pattern.AddPattern(uLineIndex, TRUE, "0000000000000000", 0, "INC", "", 0, FALSE, FALSE);//!< ��3��
				++uLineIndex;
				Pattern.AddPattern(uLineIndex, TRUE, "0000000000000000", 0, "INC", "", 0, FALSE, FALSE);//!< ��4��
				++uLineIndex;
				Pattern.AddPattern(uLineIndex, TRUE, "0000000000000000", 0, "INC", "", 0, FALSE, FALSE);//!< ��5��
				++uLineIndex;
				Pattern.AddPattern(uLineIndex, TRUE, "0000000000000000", 0, "END_LOOPC", "", 3, FALSE, FALSE);//!< ��6��
				++uLineIndex;

				vecLineOrder.push_back(2);
				for (int nLoopIndex = 0; nLoopIndex <= nLoopCount; ++nLoopIndex)
				{
					vecLineOrder.push_back(3);
					vecLineOrder.push_back(4);
					vecLineOrder.push_back(5);
					vecLineOrder.push_back(6);
				}
				Pattern.AddPattern(uLineIndex, TRUE, "0000000000000000", 0, "JUMP", "", 500, FALSE, FALSE);//!< ��6��
				vecLineOrder.push_back(uLineIndex);

				continue;
			}
			if (500 == uLineIndex)
			{
				///<LOOP_B
				int nLoopCount = 32;

				Pattern.AddPattern(uLineIndex, TRUE, "0000000000000000", 0, "SET_LOOPB", "", nLoopCount, FALSE, FALSE);//!< ��500��
				++uLineIndex;
				Pattern.AddPattern(uLineIndex, TRUE, "0000000000000000", 0, "INC", "", 0, FALSE, FALSE);//!< ��501��
				++uLineIndex;
				Pattern.AddPattern(uLineIndex, TRUE, "0000000000000000", 0, "INC", "", 0, FALSE, FALSE);//!< ��502��
				++uLineIndex;
				Pattern.AddPattern(uLineIndex, TRUE, "0000000000000000", 0, "INC", "", 0, FALSE, FALSE);//!< ��503��
				++uLineIndex;
				Pattern.AddPattern(uLineIndex, TRUE, "0000000000000000", 0, "END_LOOPB", "", 501, FALSE, FALSE);//!< ��504��
				++uLineIndex;

				vecLineOrder.push_back(500);
				for (int nLoopIndex = 0; nLoopIndex <= nLoopCount; ++nLoopIndex)
				{
					vecLineOrder.push_back(501);
					vecLineOrder.push_back(502);
					vecLineOrder.push_back(503);
					vecLineOrder.push_back(504);
				}
				Pattern.AddPattern(uLineIndex, TRUE, "0000000000000000", 0, "JUMP", "", 2000, FALSE, FALSE);//!< ��6��
				vecLineOrder.push_back(505);
				continue;
			}
			if (2000 == uLineIndex)
			{
				///<LOOP_A
				int nBaseLine = 2000;
				int nFirstLoopCount = 4;
				int nSecondLoopCount = 4;
				int nThirdLoopCount = 4;
				int nForthLoopCount = 4;
				Pattern.AddPattern(uLineIndex, TRUE, "0000000000000000", 0, "SET_LOOPA", "", nFirstLoopCount - 1, FALSE, FALSE);//!< ��2000��
				++uLineIndex;
				Pattern.AddPattern(uLineIndex, TRUE, "0000000000000000", 0, "INC", "", 0, FALSE, FALSE);//!< ��2001��
				++uLineIndex;
				Pattern.AddPattern(uLineIndex, TRUE, "0000000000000000", 0, "SET_LOOPA", "", nSecondLoopCount - 1, FALSE, FALSE);//!< ��2002��
				++uLineIndex;
				Pattern.AddPattern(uLineIndex, TRUE, "0000000000000000", 0, "INC", "", 0, FALSE, FALSE);//!< ��2003��
				++uLineIndex;
				Pattern.AddPattern(uLineIndex, TRUE, "0000000000000000", 0, "SET_LOOPA", "", nThirdLoopCount - 1, FALSE, FALSE);//!< ��2004��
				++uLineIndex;
				Pattern.AddPattern(uLineIndex, TRUE, "0000000000000000", 0, "INC", "", 0, FALSE, FALSE);//!< ��2005��
				++uLineIndex;
				Pattern.AddPattern(uLineIndex, TRUE, "0000000000000000", 0, "SET_LOOPA", "", nForthLoopCount - 1, FALSE, FALSE);//!< ��2006��
				++uLineIndex;
				Pattern.AddPattern(uLineIndex, TRUE, "0000000000000000", 0, "INC", "", 0, FALSE, FALSE);//!< ��2007��
				++uLineIndex;
				Pattern.AddPattern(uLineIndex, TRUE, "0000000000000000", 0, "END_LOOPA", "", 2007, FALSE, FALSE);//!< ��2008��
				++uLineIndex;
				Pattern.AddPattern(uLineIndex, TRUE, "0000000000000000", 0, "INC", "", 0, FALSE, FALSE);//!< ��2009��
				++uLineIndex;
				Pattern.AddPattern(uLineIndex, TRUE, "0000000000000000", 0, "END_LOOPA", "", 2005, FALSE, FALSE);//!< ��2010��
				++uLineIndex;
				Pattern.AddPattern(uLineIndex, TRUE, "0000000000000000", 0, "INC", "", 0, FALSE, FALSE);//!< ��2011��
				++uLineIndex;
				Pattern.AddPattern(uLineIndex, TRUE, "0000000000000000", 0, "END_LOOPA", "", 2003, FALSE, FALSE);//!< ��2012��
				++uLineIndex;
				Pattern.AddPattern(uLineIndex, TRUE, "0000000000000000", 0, "INC", "", 0, FALSE, FALSE);//!< ��2013��
				++uLineIndex;
				Pattern.AddPattern(uLineIndex, TRUE, "0000000000000000", 0, "END_LOOPA", "", 2001, FALSE, FALSE);//!< ��2014��
				++uLineIndex;
				
				int nBaseLineCount = vecLineOrder.size();
				vecLineOrder.push_back(2000);
				for (int nFirstLoopIndex = 0; nFirstLoopIndex < nFirstLoopCount;++nFirstLoopIndex)
				{

					vecLineOrder.push_back(2001);
					vecLineOrder.push_back(2002);

					for (int nSecondLoopIndex = 0; nSecondLoopIndex < nSecondLoopCount; ++nSecondLoopIndex)
					{
						vecLineOrder.push_back(2003);
						vecLineOrder.push_back(2004);

						for (int nThirdLoopIndex = 0; nThirdLoopIndex < nThirdLoopCount; ++nThirdLoopIndex)
						{
							vecLineOrder.push_back(2005);
							vecLineOrder.push_back(2006);

							for (int nForthLoopIndex = 0; nForthLoopIndex < nForthLoopCount; ++nForthLoopIndex)
							{
								vecLineOrder.push_back(2007);
								vecLineOrder.push_back(2008);
							}
							vecLineOrder.push_back(2009);
							vecLineOrder.push_back(2010);
						}
						vecLineOrder.push_back(2011);
						vecLineOrder.push_back(2012);
					}
					vecLineOrder.push_back(2013);
					vecLineOrder.push_back(2014);
				}

				Pattern.AddPattern(uLineIndex, TRUE, "0000000000000000", 0, "JUMP", "", 3500, FALSE, FALSE);//!< ��6��
				vecLineOrder.push_back(uLineIndex);
				continue;
			}
			if (3500 == uLineIndex)
			{
				int nLoopLineCount = 0;
				int nLoopStart = 0;
				Pattern.AddPattern(uLineIndex, TRUE, "0000000000000000", 1, "SET_MCNT", "", 100, FALSE, FALSE); //!< ��3500��
				vecLineOrder.push_back(uLineIndex);
				++uLineIndex;
				Pattern.AddPattern(uLineIndex, TRUE, "0000000000000000", 1, "INC", "", 0, FALSE, FALSE);//!< ��3501��
				vecLineOrder.push_back(uLineIndex);
				++uLineIndex;
				Pattern.AddPattern(uLineIndex, TRUE, "0000000000000000", 1, "SET_FAIL", "", 10, FALSE, FALSE);//!< ��3502��
				vecLineOrder.push_back(uLineIndex);
				++uLineIndex;
				Pattern.AddPattern(uLineIndex, TRUE, "0000000000000000", 1, "INC", "", 0, FALSE, FALSE); //!< ��3503��
				vecLineOrder.push_back(uLineIndex);
				++uLineIndex;
				Pattern.AddPattern(uLineIndex, TRUE, "0000000000000000", 1, "MASKF", "", 0, FALSE, FALSE); //!< ��3504��
				++nLoopLineCount;
				nLoopStart = uLineIndex;
				++uLineIndex;
				Pattern.AddPattern(uLineIndex, TRUE, "0000000000000000", 1, "INC", "", 0, FALSE, FALSE); //!< ��3505��
				++nLoopLineCount;
				++uLineIndex;
				Pattern.AddPattern(uLineIndex, TRUE, "0000000000000000", 1, "MATCH", "", 0, FALSE, FALSE);//!< ��3506��
				++nLoopLineCount;
 				++uLineIndex;

				for (int nIndex = 0; nIndex < 4;++nIndex)
				{
					for (int nLineIndex = 0; nLineIndex < nLoopLineCount;++nLineIndex)
					{
						vecLineOrder.push_back(nLoopStart + nLineIndex);
					}
				}
				nLoopLineCount = 0;
				nLoopStart = 0;
				Pattern.AddPattern(uLineIndex, TRUE, "0000000000000000", 1, "SET_MCNT", "", 2, FALSE, FALSE); //!< ��3507��
				vecLineOrder.push_back(uLineIndex);
				++uLineIndex;
				Pattern.AddPattern(uLineIndex, TRUE, "0000000000000000", 1, "SET_GLO", "", 4093, FALSE, FALSE);//!< ��3508��
				vecLineOrder.push_back(uLineIndex);
				++uLineIndex;
				Pattern.AddPattern(uLineIndex, TRUE, "0000000000000000", 1, "SET_FAIL", "", 9, FALSE, FALSE);//!< ��3509��
				vecLineOrder.push_back(uLineIndex);
				++uLineIndex;
				Pattern.AddPattern(uLineIndex, TRUE, "0000000000000000", 1, "INC", "", 0, FALSE, FALSE); //!< ��3510��
				vecLineOrder.push_back(uLineIndex);
				++uLineIndex;
				Pattern.AddPattern(uLineIndex, TRUE, "0000000000000000", 1, "MASKF", "", 0, FALSE, FALSE); //!< ��3511��
				nLoopStart = uLineIndex;
				++nLoopLineCount;
				++uLineIndex;
				Pattern.AddPattern(uLineIndex, TRUE, "0000000000000000", 1, "INC", "", 0, FALSE, FALSE); //!< ��3512��
				++nLoopLineCount;
				++uLineIndex;
				Pattern.AddPattern(uLineIndex, TRUE, "0000000000000000", 1, "MATCH", "", 0, FALSE, FALSE); //!< ��3513��
				++nLoopLineCount;

				for (int nIndex = 0; nIndex < 3; ++nIndex)
				{
					for (int nLineIndex = 0; nLineIndex < nLoopLineCount; ++nLineIndex)
					{
						vecLineOrder.push_back(nLoopStart + nLineIndex);
					}
				}
				continue;
			}
			if (nLineCount == uLineIndex + 1)
			{
				Pattern.AddPattern(uLineIndex, TRUE, "0000000000000000", 0, "HLT", "", 0, FALSE, FALSE); //!< ��4000��
				vecLineOrder.push_back(uLineIndex);
				continue;
			}		

			Pattern.AddPattern(uLineIndex, TRUE, "0000000000000000", 0, "INC", "", 0, FALSE, FALSE);
		}

		Pattern.Load();
		pHardware->SetPeriod(1, 1000);
		ClearBind();

		std::vector<double> vecPeriod;
		vecPeriod.push_back(12);
		vecPeriod.push_back(16);
		vecPeriod.push_back(25);
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
		vecPeriod.push_back(400); 
		vecPeriod.push_back(4e3);
		vecPeriod.push_back(6e3);
		vecPeriod.push_back(15e3); 
		vecPeriod.push_back(20e3);
		vecPeriod.push_back(90e3);
		vecPeriod.push_back(100e3);
		vecPeriod.push_back(400e3);
		vecPeriod.push_back(1e6);
		vecPeriod.push_back(4e6);

		int nFailCount = 0;
		int nPeriodCount = vecPeriod.size();
		double dPeriod = 0;
		for (auto dPeriod : vecPeriod)
		{
			if (1 == m_pReportDevice->IsStop())
			{
				double dShowPeriod = GetPeriodUnits(dPeriod, lpszTimeUnits, sizeof(lpszTimeUnits));
				m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<UserStop value='NextPeriod=%.1f%s'/>\n", lpszNextIndent, dShowPeriod, lpszTimeUnits);
				break;
			}
			int nResult = CheckPeriod(lpszNextIndent, dPeriod, nLineCount, vecLineOrder, FALSE);
			if (0 != nResult)
			{
				++nFailCount;
				if (1 != m_pReportDevice->IsFailContinue())
				{
					break;
				}
			}
		}
		if (0 == nFailCount)
		{
			nRetVal = 0;
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='true'/>\n", lpszNextIndent);
		}
		else
		{
			nRetVal = -1;
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='false'/>\n", lpszNextIndent);
		}
	} while (false);

	ShowUIResult();

	dTimeConsume = StopTimer(lpszTimeUnits, sizeof(lpszTimeUnits));
	m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<Time value='%.3f%s'/>\n", lpszNextIndent, dTimeConsume, lpszTimeUnits);

	m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s</LoopingTest>\n", lpszBaseIndent);
	return nRetVal;
}

int CDiagnosisHighInstructions::CheckPeriod(const char* lpszBaseIndent, double dPeriod, UINT uLineCount, const std::vector<UINT>& vecExpectLineOrder, BOOL bSynRun)
{
	if (0 > m_vecEnableController.size())
	{
		return -1;
	}

	StartTimer();
	m_setFailController.clear();
	double dTimeConsume = 0;
	char lpszTimeUnits[4] = { 0 };

	std::string strFirstIndent = lpszBaseIndent + IndentChar();
	std::string strSecondIndent = strFirstIndent + IndentChar();
	const char* lpszFirstIndent = strFirstIndent.c_str();
	const char* lpszSecondIndent = strSecondIndent.c_str();

	double dShowPeriod = GetPeriodUnits(dPeriod, lpszTimeUnits, sizeof(lpszTimeUnits));

	m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<Period value='%.1f%s'>\n", lpszBaseIndent, dShowPeriod, lpszTimeUnits);
	int nRetVal = -1;

	Bind(m_vecEnableController, m_vecEnableController[0]);

	CHardwareFunction* pHardare = GetHardware(m_vecEnableController[0]);
	//1 ��������

	double dEdge[6] = { 5, 10, 0, 10, 10, 15 };
	vector<USHORT> vecChannel;
	//2 ����ʱ����
	for (USHORT usChannel = 0; usChannel < HDModule::ChannelCountPerControl; ++usChannel)
	{
		vecChannel.push_back(usChannel);
	}
	pHardare->SetPeriod(0, dPeriod);
	pHardare->SetEdge(vecChannel, 0, dEdge, WAVE_FORMAT::NRZ, IO_FORMAT::NRZ, COMPARE_MODE::EDGE);
	pHardare->SetRunParameter(0, uLineCount - 1);
	pHardare->SetPatternMode(TRUE, DATA_TYPE::FM, FALSE, FALSE);

	ClearBind();

	BYTE byBoardControllerIndex = 0;
	BYTE bySlotNo = 0;

	if (bSynRun)
	{
		pHardare->SynRun();
		WaitStop();
	}
	else
	{
		for (auto uControllerID : m_vecEnableController)
		{
			bySlotNo = HDModule::Instance()->ID2Board(uControllerID, byBoardControllerIndex);
			
			pHardare = GetHardware(uControllerID);
			
			pHardare->Run();
			WaitStop(uControllerID);
		}
	}

	BOOL bHaveFail = FALSE;

	int nTotalWaitStopTimes = 100000;
	BOOL bControllerPass = TRUE;
	BOOL bAllPass = TRUE;
	for (auto uControllerID : m_vecEnableController)
	{
		bySlotNo = HDModule::Instance()->ID2Board(uControllerID, byBoardControllerIndex);

		pHardare = GetHardware(uControllerID);
				
		StartTimer();

		bControllerPass = TRUE;

		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<controller value='%d, slot value = %d'>\n", lpszFirstIndent, byBoardControllerIndex, bySlotNo);
		//6 ��ȡ����
		vector<UINT> vecLineOrder;
		pHardare->GetLineRanOrder(vecLineOrder);
		//7 �ж����ݶԴ�
		int nFailCount = 0;
		int nLineIndex = 0;
		int nLineOrderCount = vecLineOrder.size() > vecExpectLineOrder.size() ? vecExpectLineOrder.size() : vecLineOrder.size();
		for (nLineIndex = 0; nLineIndex < nLineOrderCount; ++nLineIndex)
		{
			if (vecLineOrder[nLineIndex] != vecExpectLineOrder[nLineIndex])
			{
				m_pReportDevice->PrintfToDataLog(IHDReportDevice::Error, "%s<data retValue='false' value='addr=0x%X, expect=0x%X, real=0x%X'/>\n", lpszSecondIndent, nLineIndex, vecExpectLineOrder[nLineIndex], vecLineOrder[nLineIndex]);
				bControllerPass = FALSE;
				bAllPass = FALSE;
				++nFailCount;
				bHaveFail = TRUE;
				if (ERROR_PRINT < nFailCount)
				{
					break;
				}
			}
		}
		
		if (bControllerPass && nLineIndex != nLineOrderCount)
		{
			bControllerPass = FALSE;
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Error, "%s<data retValue='false' value='Expect=%d line, Real=%d line'/>\n", lpszSecondIndent, vecExpectLineOrder.size(), vecLineOrder.size());
		}
		if (bControllerPass)
		{
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='true'/>\n", lpszSecondIndent);
		}
		else
		{
			if (m_setFailController.end() == m_setFailController.find(uControllerID))
			{
				m_setFailController.insert(uControllerID);
			}
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='false'/>\n", lpszSecondIndent);
		}
		dTimeConsume = StopTimer(lpszTimeUnits, sizeof(lpszTimeUnits));
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<Time value='%.3f%s'/>\n", lpszSecondIndent, dTimeConsume, lpszTimeUnits);

		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s</controller>\n", lpszFirstIndent);
	}

	nRetVal = 0;
	if (bAllPass)
	{
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='true'/>\n", lpszFirstIndent);
	}
	else
	{
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='false'/>\n", lpszFirstIndent);
		nRetVal = -1;
	}

	dTimeConsume = StopTimer(lpszTimeUnits, sizeof(lpszTimeUnits));
	m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<Time value='%.3f%s'/>\n", lpszFirstIndent, dTimeConsume, lpszTimeUnits);

	m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s</Period>\n", lpszBaseIndent);

	return nRetVal;
}

void CDiagnosisHighInstructions::ShowUIResult()
{
	BYTE bySlotNo = 0;
	BYTE byBoardController = 0;
	for (UINT uControllerID : m_vecEnableController)
	{
		bySlotNo = HDModule::Instance()->ID2Board(uControllerID, byBoardController);
		if (m_setFailController.end() != m_setFailController.find(uControllerID))
		{
			m_pReportDevice->PrintfToUi(IHDReportDevice::Fail);
		}
		else
		{
			m_pReportDevice->PrintfToUi(IHDReportDevice::Pass);
		}
		m_pReportDevice->PrintfToUi("\t Slot %d, Controller %d\n", bySlotNo, byBoardController);
	}
	m_setFailController.clear();
}
