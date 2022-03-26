#include "DiagnosisFailSyn.h"
#include "..\HDModule.h"
#include "..\Pattern.h"
#include <array>
using namespace std;
CDiagnosisFailSyn::CDiagnosisFailSyn()
{

}

CDiagnosisFailSyn::~CDiagnosisFailSyn()
{

}

int CDiagnosisFailSyn::QueryInstance(const char* lpszName, void** ppPoint)
{
	return -1;
}

void CDiagnosisFailSyn::Release()
{

}

const char* CDiagnosisFailSyn::Name() const
{
	return "Fail Sychronization Diagnosis";
}

int CDiagnosisFailSyn::GetChildren(STSVector<IHDDoctorItem*>& vecChildren) const
{
	return 0;
}

int CDiagnosisFailSyn::Doctor(IHDReportDevice* pReportDevice)
{
	m_pReportDevice = pReportDevice;
	int nRetVal = -1;

	StartTimer();
	double dTimeConsume = 0;
	char lpszTimeUnits[4] = { 0 };
	const char* lpszBaseIndent = IndentFormat();
	string strFirstIndent = lpszBaseIndent + IndentChar();
	const char* lpszFirstIndent = strFirstIndent.c_str();

	m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<FailSynchronizationDiagnosis>\n", lpszBaseIndent);
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

		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<Info>Undiagnosable for slot %d controller %d is not existed.</Info>\n", lpszNextIndent, bySlotNo, byBoardControllerIndex);
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s</Controller>\n", IndentFormat());
	}


	///<Check supported
	set<UINT> setUnsupportedController;
	CHardwareFunction* pHardware = nullptr;
	BYTE byMutualContrller = 0;
	UINT uRelatedControllerID = 0;
	for (auto Controller : m_vecEnableController)
	{
		bySlotNo = HDModule::Instance()->ID2Board(Controller, byBoardControllerIndex);
		pHardware = GetHardware(Controller);
		if (pHardware->IsSupportSaveFailSelected())
		{
			///<Check whether the controller support saving the fail information of selected line
			if (0 == byBoardControllerIndex % 2)
			{
				byMutualContrller = byBoardControllerIndex + 1;
			}
			else
			{
				byMutualContrller = byBoardControllerIndex - 1;
			}
			uRelatedControllerID = HDModule::Instance()->GetID(bySlotNo, byMutualContrller);
			pHardware = GetHardware(uRelatedControllerID);
			if (pHardware->IsSupportFailSyn())
			{
				continue;
			}
			m_pReportDevice->PrintfToUi(IHDReportDevice::Fail);
			m_pReportDevice->PrintfToUi("\t Slot %d, Controller %d\n", bySlotNo, byBoardControllerIndex);
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<Controller value='%d, slot value=%d'>\n", IndentFormat(), bySlotNo, byBoardControllerIndex);

			m_pReportDevice->PrintfToUi("\t\t Undiagnosable for slot %d controller %d is not supported.", bySlotNo, byMutualContrller);
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<Undiagnosable for slot %d controller %d is not supported./>\n", lpszNextIndent, bySlotNo, byBoardControllerIndex);
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s</Controller>\n", IndentFormat());
			continue;
		}
		setUnsupportedController.insert(Controller);
	}

	if (0 != setUnsupportedController.size())
	{
		vector<UINT> vecEnable = m_vecEnableController;
		m_vecEnableController.clear();
		for (auto Controller : vecEnable)
		{
			if (setUnsupportedController.end() != setUnsupportedController.find(Controller))
			{
				continue;
			}
			m_vecEnableController.push_back(Controller);
		}
	}

	if (0 == m_vecEnableController.size())
	{
		m_pReportDevice->PrintfToUi("\t All controllers are not supported\n", bySlotNo, byBoardControllerIndex);
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<Item>All controllers are not supported</Item>\n", lpszNextIndent);
		dTimeConsume = StopTimer(lpszTimeUnits, sizeof(lpszTimeUnits));
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<Time value='%.3f%s'/>\n", lpszNextIndent, dTimeConsume, lpszTimeUnits);

		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s</FailSynchronizationDiagnosis>\n", lpszBaseIndent);
		return 0;
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
			nItemResult = DiagnosisFJUMP(lpszFirstIndent);
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

	m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s</FailSynchronizationDiagnosis>\n", lpszBaseIndent);

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


	return nRetVal;
}

bool CDiagnosisFailSyn::IsUserCheck() const
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

int CDiagnosisFailSyn::DiagnosisFJUMP(const char* lpszBaseIndent)
{
	StartTimer();
	double dTimeConsume = 0;
	char lpszTimeUnits[4] = { 0 };

	string strFirstIndent = lpszBaseIndent + IndentChar();
	string strSecondIndent = lpszBaseIndent + IndentChar();
	const char* lpszFirstIndent = strFirstIndent.c_str();
	const char* lpszSecondIndent = strSecondIndent.c_str();

	int nRetVal = 0;
	int nFailItemCount = 0;
	double dPeriod = 1000;
	m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<FJUMP, value='Period=%.0f'>\n", lpszBaseIndent, dPeriod);

	CHardwareFunction* pHardware = nullptr;
	vector<BYTE> vecFailSynController;

	vector<USHORT> vecChannel;
	for (USHORT usChannel = 0;usChannel < DCM_CHANNELS_PER_CONTROL;++usChannel)
	{
		vecChannel.push_back(usChannel);
	}

	double dEdge[EDGE_COUNT] = { dPeriod / 4, dPeriod / 2, dPeriod / 4, dPeriod / 2,dPeriod / 2, dPeriod * 3 / 4 };

	Bind(m_vecEnableController, m_vecEnableController[0]);
	pHardware = GetHardware(m_vecEnableController[0]);
	pHardware->SetPeriod(0, dPeriod);
	pHardware->SetEdge(vecChannel, 0, dEdge, WAVE_FORMAT::NRZ, IO_FORMAT::NRZ, COMPARE_MODE::EDGE);
	pHardware->SetPinLevel(vecChannel, 3, 0, 1.5, 2, 0.8);
	ClearBind();


	auto CheckResult=[&](const char* lpszItemName)->int
	{
		StartTimer();
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<%s>\n", lpszFirstIndent, lpszItemName);
		int nPatternCount = LoadPattern(vecFailSynController);
		if (0 > nPatternCount)
		{
			return 1;
		}
		

		///<Run vector
		Bind(m_vecEnableController, m_vecEnableController[0]);
		pHardware = GetHardware(m_vecEnableController[0]);
		pHardware->SetRunParameter(0, nPatternCount - 1, FALSE, 0);
		pHardware->SynRun();
		ClearBind();

		WaitStop();

		int nRetVal = ResultAnalyze(lpszSecondIndent);
		if (0 != nRetVal)
		{
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='false'/>\n", lpszSecondIndent);
			++nFailItemCount;
		}
		else
		{
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='true'/>\n", lpszSecondIndent);
		}
		dTimeConsume = StopTimer(lpszTimeUnits, sizeof(lpszTimeUnits));
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<Time value='%.3f%s'/>\n", lpszSecondIndent, dTimeConsume, lpszTimeUnits);
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s</%s>\n", lpszFirstIndent, lpszItemName);
		return 0;
	};

	do 
	{
		///<One controller fail sychronization
		vecFailSynController.clear();
		vecFailSynController.push_back(0x01);
		vecFailSynController.push_back(0x02);
		vecFailSynController.push_back(0x04);
		vecFailSynController.push_back(0x08);
		CheckResult("EachControllerSyn");
		if (1 == m_pReportDevice->IsStop())
		{
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<UserStop value='Next=FirstTwoSyn'/>\n", lpszFirstIndent);
			break;
		}

		///<0/1 2 3
		vecFailSynController.clear();
		vecFailSynController.push_back(0x03);
		vecFailSynController.push_back(0x04);
		vecFailSynController.push_back(0x08);
		CheckResult("FirstTwoSyn");
		if (1 == m_pReportDevice->IsStop())
		{
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<UserStop value='Next=LastTwoSyn'/>\n", lpszFirstIndent);
			break;
		}

		///<0 1 2/3
		vecFailSynController.clear();
		vecFailSynController.push_back(0x01);
		vecFailSynController.push_back(0x02);
		vecFailSynController.push_back(0x0C);
		CheckResult("LastTwoSyn");
		if (1 == m_pReportDevice->IsStop())
		{
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<UserStop value='Next=FirstIntervalSyn'/>\n", lpszFirstIndent);
			break;
		}

		///<0/2 1 3
		vecFailSynController.clear();
		vecFailSynController.push_back(0x05);
		vecFailSynController.push_back(0x02);
		vecFailSynController.push_back(0x08);
		CheckResult("FirstIntervalSyn");
		if (1 == m_pReportDevice->IsStop())
		{
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<UserStop value='Next=LastIntervalSyn'/>\n", lpszFirstIndent);
			break;
		}

		///<0 2 1/3
		vecFailSynController.clear();
		vecFailSynController.push_back(0x01);
		vecFailSynController.push_back(0x04);
		vecFailSynController.push_back(0x0A);
		CheckResult("LastIntervalSyn");
		if (1 == m_pReportDevice->IsStop())
		{
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<UserStop value='Next=TwoCoupleSyn'/>\n", lpszFirstIndent);
			break;
		}

		///<0/1 2/3
		vecFailSynController.clear();
		vecFailSynController.push_back(0x03);
		vecFailSynController.push_back(0x0C);
		CheckResult("TwoCoupleSyn");
		if (1 == m_pReportDevice->IsStop())
		{
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<UserStop value='Next=TwoIntervalCoupleSyn'/>\n", lpszFirstIndent);
			break;
		}

		///<0/3 1/2
		vecFailSynController.clear();
		vecFailSynController.push_back(0x0A);
		vecFailSynController.push_back(0x05);
		CheckResult("TwoIntervalCoupleSyn");
		if (1 == m_pReportDevice->IsStop())
		{
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<UserStop value='Next=FirstIndependent'/>\n", lpszFirstIndent);
			break;
		}

		///<0 1/2/3
		vecFailSynController.clear();
		vecFailSynController.push_back(0x01);
		vecFailSynController.push_back(0x0E);
		CheckResult("FirstIndependent");
		if (1 == m_pReportDevice->IsStop())
		{
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<UserStop value='Next=SecondIndependent'/>\n", lpszFirstIndent);
			break;
		}

		///<1 0/2/3
		vecFailSynController.clear();
		vecFailSynController.push_back(0x02);
		vecFailSynController.push_back(0x0D);
		CheckResult("SecondIndependent");
		if (1 == m_pReportDevice->IsStop())
		{
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<UserStop value='Next=ThirdIndependent'/>\n", lpszFirstIndent);
			break;
		}

		///<2 0/1/3
		vecFailSynController.clear();
		vecFailSynController.push_back(0x04);
		vecFailSynController.push_back(0x0B);
		CheckResult("ThirdIndependent");
		if (1 == m_pReportDevice->IsStop())
		{
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<UserStop value='Next=ForthIndependent'/>\n", lpszFirstIndent);
			break;
		}

		///<3 0/1/2
		vecFailSynController.clear();
		vecFailSynController.push_back(0x08);
		vecFailSynController.push_back(0x07);
		CheckResult("ForthIndependent");
		if (1 == m_pReportDevice->IsStop())
		{
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<UserStop value='Next=FourSyn'/>\n", lpszFirstIndent);
			break;
		}

		///<0/1/2/3
		vecFailSynController.clear();
		vecFailSynController.push_back(0x0F);
		CheckResult("AllSyn");
	} while (FALSE);

	dTimeConsume = StopTimer(lpszTimeUnits, sizeof(lpszTimeUnits));
	if (nullptr != m_pReportDevice)
	{
		if (0 != nFailItemCount)
		{
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='false'/>\n", lpszFirstIndent);
		}
		else
		{
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='true'/>\n", lpszFirstIndent);
		}
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<Time value='%.3f%s'/>\n", lpszFirstIndent, dTimeConsume, lpszTimeUnits);
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s</SectionDiagnosis>\n", lpszBaseIndent);
	}

	if (0 != nFailItemCount)
	{
		return -1;
	}
	return 0;
}

int CDiagnosisFailSyn::LoadPattern(std::vector<BYTE>& vecFailSynController)
{
	for (auto& FailInfo : m_avecBRAMFailExpected)
	{
		FailInfo.clear();
	}
	CHardwareFunction* pHardware = nullptr;
	const int nPatternCount = 25;
	///<Load pattern
	vector<UINT> vecCurController;
	const char lpszInstruction[2][8] = { "INC","FJUMP" };
	const char* lpszCurInstruction = lpszInstruction[0];
	array<int, DCM_MAX_CONTROLLERS_PRE_BOARD> arrayJumpBaseLine = { 7, 16, 11, 2 };
	const int nJumpOver = 2;
	map<int, USHORT> mapFJUMP;
	for (auto BaseLine : arrayJumpBaseLine)
	{
		mapFJUMP.insert(make_pair(BaseLine, BaseLine + nJumpOver + 1));
	}
	auto iterFJUMP = mapFJUMP.begin();
	USHORT usOperand = 0;
	for (int nControllerType = 0; nControllerType < 2; ++nControllerType)
	{
		vecCurController.clear();
		for (auto Controller : m_vecEnableController)
		{
			if (nControllerType == Controller % 2)
			{
				vecCurController.push_back(Controller);
			}
		}

		Bind(vecCurController, vecCurController[0]);
		pHardware = GetHardware(vecCurController[0]);
		CPattern Pattern(*pHardware);
			
		for (int nPatternIndex = 0; nPatternIndex < nPatternCount; ++nPatternIndex)
		{
			usOperand = 0;
			lpszCurInstruction = lpszInstruction[0];
			iterFJUMP = mapFJUMP.find(nPatternIndex);
			if (mapFJUMP.end() != iterFJUMP)
			{
				usOperand = iterFJUMP->second;
				lpszCurInstruction = lpszInstruction[1];
			}
			if ((0 == nControllerType && 0 == nPatternIndex % 2) || (1 == nControllerType && 1 == nPatternIndex % 2))
			{
				Pattern.AddPattern(nPatternIndex, TRUE, "1010101010101010", 0, lpszCurInstruction, "", usOperand, FALSE, FALSE);
			}
			else
			{
				if (0 != usOperand)
				{
					Pattern.AddPattern(nPatternIndex, TRUE, "HLHLHLHLHLHLHLHL", 0, lpszCurInstruction, "", usOperand, FALSE, FALSE);
				}
				else
				{
					Pattern.AddPattern(nPatternIndex, TRUE, "LLLLLLLLLLLLLLLLLL", 0, lpszCurInstruction, "", usOperand, FALSE, FALSE);
				}
			}
		}
		Pattern.Load();
		ClearBind();
	}

	int nControllerFail[DCM_MAX_CONTROLLERS_PRE_BOARD] = { 7,16, 11, 2 };
	int nLineIndex = 0;
	///<Set the conditional instruction
	for (auto Controller : m_vecEnableController)
	{
		pHardware = GetHardware(Controller);
		CPattern Pattern(*pHardware);
		BYTE byControllerIndex = 0;
		BYTE bySlotNo = HDModule::Instance()->ID2Board(Controller, byControllerIndex);
		nLineIndex = nControllerFail[byControllerIndex];
		iterFJUMP = mapFJUMP.find(nLineIndex);
		if (mapFJUMP.end() != iterFJUMP)
		{
			usOperand = iterFJUMP->second;
			lpszCurInstruction = lpszInstruction[1];
		}
		else
		{
			lpszCurInstruction = lpszInstruction[0];
			usOperand = 0;
		}
		Pattern.AddPattern(nLineIndex, TRUE, "LLLLLLLLLLLLLLLL", 0, lpszCurInstruction, "", usOperand, FALSE, FALSE);
		Pattern.Load();
	}

	pHardware = GetHardware(m_vecEnableController[0]);
	CPattern Pattern(*pHardware);
	char lpszPattern[32][17] = { 0 };
	Pattern.ReadPattern(TRUE, 0, 25, lpszPattern);

	pHardware = GetHardware(m_vecEnableController[1]);
	CPattern Pattern1(*pHardware);
	char lpszPattern1[32][17] = { 0 };
	Pattern1.ReadPattern(TRUE, 0, 25, lpszPattern1);


	///<Get the expected fail line information
	set<BYTE> setSynController;
	vector<BYTE> vecCurSynController;
	std::map<BYTE, BYTE> mapFailSyn;
	for (auto FailSyn : vecFailSynController)
	{
		vecCurSynController.clear();
		set<BYTE> setBaseJumpLine;
		for (BYTE byControllerIndex = 0; byControllerIndex < DCM_MAX_CONTROLLERS_PRE_BOARD; ++byControllerIndex)
		{
			if (FailSyn >> byControllerIndex & 0x01)
			{
				setBaseJumpLine.insert(arrayJumpBaseLine[byControllerIndex]);
				if (setSynController.end() != setSynController.find(byControllerIndex))
				{
					///<Only support synchronize fail informatini to one sychronization type for each controller
					return -1;
				}
				setSynController.insert(byControllerIndex);
				vecCurSynController.push_back(byControllerIndex);
			}
			for (auto Controller : vecCurSynController)
			{
				mapFailSyn.insert(make_pair(Controller, FailSyn));
			}
		}
		for (auto Controller : vecCurSynController)
		{
			for (int nPatternIndex = 0; nPatternIndex < nPatternCount; ++nPatternIndex)
			{
				if (mapFJUMP.end() == mapFJUMP.find(nPatternIndex) || nControllerFail[Controller] == nPatternIndex)
				{
					if (nPatternIndex % 2 != Controller % 2)
					{
						CHardwareFunction::DATA_RESULT DataResult;
						DataResult.m_nLineNo = nPatternIndex;
						DataResult.m_usData = 0x5555;
						m_avecBRAMFailExpected[Controller].push_back(DataResult);
					}
				}
				if (setBaseJumpLine.end() != setBaseJumpLine.find(nPatternIndex))
				{
					///<The pattern line is jump over
					nPatternIndex += nJumpOver;
				}
			}
		}
	}

	std::map<BYTE, BYTE> mapDisablePatternCount;
	for (auto& Controller : mapFailSyn)
	{
		BYTE byMutualController = 0 == Controller.first % 2 ? Controller.first + 1 : Controller.first - 1;
		if (0 != (Controller.second  >> byMutualController & 0x01))
		{
			continue;
		}
		auto iterMutual = mapFailSyn.find(byMutualController);
		if (mapFailSyn.end() == iterMutual)
		{
			continue;
		}
		else if(iterMutual->second == (1 << byMutualController))
		{
			continue;
		}
		if (iterMutual->second != (1 <<byMutualController))
		{
			mapDisablePatternCount.insert(make_pair(Controller.first, 2));
		}
		else
		{
			mapDisablePatternCount.insert(make_pair(Controller.first, 4));
		}
	}

	///<Disable the pattern to X
	for (auto& Controller : mapDisablePatternCount)
	{
		vector<UINT> vecController;
		BYTE byControllerIndex = 0;
		BYTE bySlotNo = 0;
		for (auto ControllerID : m_vecEnableController)
		{
			BYTE bySlotNo = HDModule::Instance()->ID2Board(ControllerID, byControllerIndex);
			if (Controller.first == byControllerIndex)
			{
				vecController.push_back(ControllerID);
			}
		}
		Bind(vecController, vecController[0]);
		pHardware = GetHardware(vecController[0]);
		CPattern Pattern(*pHardware);
		int nPatternIndex = nPatternCount - 1;

		auto& avecFailExpected = m_avecBRAMFailExpected[Controller.first];

		for (int nIndex = 0; nIndex < Controller.second;++nIndex, --nPatternIndex)
		{
			Pattern.AddPattern(nPatternIndex, TRUE, "XXXXXXXXXXXXXXXX", 0, "INC", "", 0, FALSE, FALSE);
			if (nPatternIndex == avecFailExpected[avecFailExpected.size() - 1].m_nLineNo)
			{
				avecFailExpected.pop_back();
			}
		}
		Pattern.Load();
		ClearBind();
	}


	///<Set the fail synchronization to board
	Bind(m_vecEnableController, m_vecEnableController[0]);
	pHardware = GetHardware(m_vecEnableController[0]);
	pHardware->SetFailSyn(mapFailSyn);
	ClearBind();

	return nPatternCount;
}

int CDiagnosisFailSyn::ResultAnalyze(const char* lpszBaseIndent)
{
	double dTimeConsume = 0;
	char lpszTimeUnits[4] = { 0 };

	string strFirstIndent = lpszBaseIndent + IndentChar();
	const char* lpszFirstIndent = strFirstIndent.c_str();
	string strSecondIndent = strFirstIndent + IndentChar();
	const char* lpszSecondIndent = strSecondIndent.c_str();
	BYTE bySlotNo = 0;
	byte byBoardControllerIndex = 0;
	CHardwareFunction* pHardware = nullptr;
	vector<CHardwareFunction::DATA_RESULT> vecBRAMFailData;
	vector<CHardwareFunction::DATA_RESULT> vecDRAMFailData;
	BOOL bItemFail = FALSE;
	vector<CHardwareFunction::DATA_RESULT> vecDRAMExpected;
	for (auto Controller : m_vecEnableController)
	{
		StartTimer();
		BOOL bControllerFail = FALSE;
		pHardware = GetHardware(Controller);
		pHardware->GetFailData(vecBRAMFailData, vecDRAMFailData);

		bySlotNo = HDModule::Instance()->ID2Board(Controller, byBoardControllerIndex);
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<controller value='%d, slot value = %d'>\n", lpszFirstIndent, byBoardControllerIndex, bySlotNo);

		char lpszValue[128] = { 0 };
		///<Check BRAM
		int nErrorCount = 0;
		const char* lpszRAMType[2] = { "BRAM", "DRAM" };
		for (int nRAMType = 0; nRAMType < 2; ++nRAMType)
		{
			const auto& vecExpected = 0 == nRAMType ? m_avecBRAMFailExpected[byBoardControllerIndex] : vecDRAMExpected;
			auto& vecReal = 0 == nRAMType ? vecBRAMFailData : vecDRAMFailData;
			if (vecExpected == vecReal)
			{
				continue;
			}
			nErrorCount = 0;
			bControllerFail = TRUE;
			auto iterReal = vecReal.begin();
			auto iterExpected = vecExpected.begin();
			while (vecReal.end() != iterReal && vecExpected.end() != iterExpected)
			{
				if (iterReal->m_nLineNo != iterExpected->m_nLineNo || iterReal->m_usData != iterExpected->m_usData)
				{
					if (ERROR_PRINT < nErrorCount++)
					{
						break;
					}
					sprintf_s(lpszValue, sizeof(lpszValue), "value='%s expect addr='0x%X' Real addr=0x%X Expect='0x%X' Real=0x%X'",
						lpszRAMType[nRAMType], iterExpected->m_nLineNo, iterReal->m_nLineNo, iterExpected->m_usData, iterReal->m_usData);
					m_pReportDevice->PrintfToDataLog(IHDReportDevice::Error, "%s<data retValue='false' %s/>\n", lpszSecondIndent, lpszValue);
				}
				++iterReal;
				++iterExpected;
			}
			if (ERROR_PRINT >= nErrorCount && vecReal.size() != vecExpected.size())
			{
				while (vecReal.end() != iterReal)
				{
					if (ERROR_PRINT < nErrorCount++)
					{
						break;
					}
					sprintf_s(lpszValue, sizeof(lpszValue), "value='%s expect addr=- Real addr=0x%X Expect=- Real=0x%X'",
						lpszRAMType[nRAMType], iterReal->m_nLineNo, iterReal->m_usData);
					m_pReportDevice->PrintfToDataLog(IHDReportDevice::Error, "%s<data retValue='false' %s/>\n", lpszSecondIndent, lpszValue);
					++iterReal;
				}
				while (vecExpected.end() != iterExpected)
				{
					if (ERROR_PRINT < nErrorCount++)
					{
						break;
					}
					sprintf_s(lpszValue, sizeof(lpszValue), "value='%s expect addr=0x%X Real addr=- Expect=0x%X Real=-'",
						lpszRAMType[nRAMType], iterExpected->m_nLineNo, iterExpected->m_usData);
					m_pReportDevice->PrintfToDataLog(IHDReportDevice::Error, "%s<data retValue='false' %s/>\n", lpszSecondIndent, lpszValue);
					++iterExpected;
				}
			}
		}
		if (bControllerFail)
		{
			m_setFailController.insert(Controller);
			bItemFail = TRUE;
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
	if (bItemFail)
	{
		return -1;
	}
	return 0;
}
