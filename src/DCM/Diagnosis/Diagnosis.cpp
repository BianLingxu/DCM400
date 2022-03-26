#include "..\HDModule.h"
#include "Diagnosis.h"
#include "DiagnosisLow.h"
#include "DiagnosisHighInstructions.h"
#include "DiagnosisHighMemory.h"
#include "DiagnosisDriverReceiver.h"
#include "DiagnosisConnectivity.h"
#include "DiagnosisWaveformat.h"
#include "DiagnosisRateSplit.h"
#include "DiagnosisEdgeSplit.h"
#include "DiagnosisEdgeScan.h"
#include "DiagnosisTMU.h"
#include "DiagnosisConditionalInstrcution.h"
#include "DiagnosisPMU.h"
#include "DiagnosisComparisonShield.h"
#include "DiagnosisFailSelected.h"
#include "DiagnosisHardwareCapture.h"
#include "DiagnosisFailSyn.h"
#include "SM8213.h"

using namespace std;
CDiagnosis::CDiagnosis()
{
	CDiagnosisItem* pDoctorItem = nullptr;

	pDoctorItem = new CDiagnosisLow();
	m_vecDoctorItem.push_back(pDoctorItem);
 
	pDoctorItem = new CDiagnosisHighInstructions();
	m_vecDoctorItem.push_back(pDoctorItem);

	pDoctorItem = new CDiagnosisDriverReceiver();
	m_vecDoctorItem.push_back(pDoctorItem);

	pDoctorItem = new CDiagnosisHighMemory();
	m_vecDoctorItem.push_back(pDoctorItem);
 
	pDoctorItem = new CDiagnosisConnectivity();
	m_vecDoctorItem.push_back(pDoctorItem);

	pDoctorItem = new CDiagnosisComparisonShield();
	m_vecDoctorItem.push_back(pDoctorItem);

	pDoctorItem = new CDiagnosisFailSyn();
	m_vecDoctorItem.push_back(pDoctorItem);

	pDoctorItem = new CDiagnosisFailSelected();
	m_vecDoctorItem.push_back(pDoctorItem);

	pDoctorItem = new CDiagnosisHardwareCapture();
	m_vecDoctorItem.push_back(pDoctorItem);

	pDoctorItem = new CDiagnosisConditionalInstrcution();
	m_vecDoctorItem.push_back(pDoctorItem);

	pDoctorItem = new CDiagnosisPMU();
	m_vecDoctorItem.push_back(pDoctorItem);
 
	pDoctorItem = new CDiagnosisTMU();
	m_vecDoctorItem.push_back(pDoctorItem);

	pDoctorItem = new CDiagnosisWaveformat();
	m_vecDoctorItem.push_back(pDoctorItem);

	pDoctorItem = new CDiagnosisRateSplit();
	m_vecDoctorItem.push_back(pDoctorItem);
 
	pDoctorItem = new CDiagnosisEdgeSplit();
	m_vecDoctorItem.push_back(pDoctorItem);
 
	pDoctorItem = new CDiagnosisEdgeScan();
	m_vecDoctorItem.push_back(pDoctorItem);
}


CDiagnosis::~CDiagnosis()
{
	for (auto& Item : m_vecDoctorItem)
	{
		if (nullptr != Item)
		{
			delete Item;
			Item = nullptr;
		}
	}
	m_vecDoctorItem.clear();
}

int CDiagnosis::QueryInstance(const char* name, void** ptr)
{
	return -1;
}

void CDiagnosis::Release()
{
}

const char* CDiagnosis::Name() const
{
	return "Parallel Diagnosis";
}

int CDiagnosis::GetChildren(STSVector<IHDDoctorItem*>& children) const
{
	int nChildrenCount = 0;
	children.clear();
	if (HDModule::Instance()->IsShowDoctorChildren())
	{
		nChildrenCount = m_vecDoctorItem.size();
		for (auto& DoctorItem : m_vecDoctorItem)
		{
			if (nullptr == DoctorItem)
			{
				continue;
			}
			if (DoctorItem->IsUserCheck())
			{
				children.push_back(DoctorItem);
			}
		}
	}
	return nChildrenCount;
}

int CDiagnosis::SetEnabled(int enable)
{
	m_nEnableStatus = enable;
	return 0;
}

int CDiagnosis::IsEnabled() const
{
	return m_nEnableStatus;
}

void CDiagnosis::SetEnableController(const std::vector<UINT>& vecController)
{
	m_vecEnableController.clear();
	
	for (auto uController : vecController)
	{
		m_vecEnableController.push_back(uController);
	}

	for (auto& pDoctorItem : m_vecDoctorItem)
	{
		pDoctorItem->SetEnableController(m_vecEnableController);
	}
}

int CDiagnosis::Doctor(IHDReportDevice* pReportDevice)
{
	StartTimer();
	std::set<std::string> setItemUndiagnosed;///<The item unchecked by user
	m_pReportDevice = pReportDevice;
	double dTimeConsume = 0;
	char lpszTimeUnits[4] = { 0 };
	std::string  strFirstIndent = IndentFormat() + IndentChar();
	std::string  strSecondIndent = strFirstIndent + IndentChar();
	std::string  strThirdIndent = strSecondIndent + IndentChar();
	const char* lpszFirstIndent = strFirstIndent.c_str();
	const char* lpszSecondIndent = strSecondIndent.c_str();
	const char* lpszThirdIndent = strThirdIndent.c_str();

	m_pReportDevice->PrintfToUi("Diagnosis DCM\n");
	m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<DCM>\n", lpszFirstIndent);
	
	char lpszUserRole[32] = { 0 };
	switch (m_UserRole)
	{
	case CDiagnosisItem::PROCUCTION:
		sts_strcpy(lpszUserRole, sizeof(lpszUserRole), "Producer");
		break;
	case CDiagnosisItem::DEVELOPER:
		sts_strcpy(lpszUserRole, sizeof(lpszUserRole), "Developer");
		break;
	case CDiagnosisItem::USER:
		sts_strcpy(lpszUserRole, sizeof(lpszUserRole), "User");
		break;
	default:
		sts_strcpy(lpszUserRole, sizeof(lpszUserRole), "User");
		break;
	}
	m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<User>%s</User>\n", lpszSecondIndent, lpszUserRole);

	ShowBoardInfo(lpszSecondIndent);

	UINT uDignosisItemCount = 0;

	UINT uFailItemCount = 0;
	int nRetVal = 0;
	set<string> setFailItem;
	set<string>vecItemNeed;
	
	HMODULE hModule = GetModuleHandle("DCM.dll");
	char lpszFile[MAX_PATH] = { 0 };
	GetModuleFileName(hModule, lpszFile, sizeof(lpszFile));
	string strFile = lpszFile;
	int nPos = strFile.rfind("\\");
	if (-1 != nPos)
	{
		strFile.erase(nPos);
	}
	strFile += "\\DCMDiagnosis.ini";
	int nCycleCount =  GetPrivateProfileInt("Diagnosis", "Cycle", 1, strFile.c_str());

	for (int nCycleIndex = 0; nCycleIndex < nCycleCount; ++nCycleIndex)
	{
		for (auto& pDoctorItem : m_vecDoctorItem)
		{
			if (nullptr == pDoctorItem)
			{
				continue;
			}
			if (!pDoctorItem->IsUserCheck())
			{
				continue;
			}
			vecItemNeed.insert(pDoctorItem->Name());
			if (0 == pDoctorItem->IsEnabled())
			{
				if (0 != nCycleIndex)
				{
					continue;
				}
				setItemUndiagnosed.insert(pDoctorItem->Name());
				continue;
			}
			if (1 == m_pReportDevice->IsStop())
			{
				m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<UserStop value='NextItem=%s'/>\n", lpszSecondIndent, pDoctorItem->Name());
				break;
			}
			pDoctorItem->SetIndent(2);
			m_pReportDevice->PrintfToUi("\n%s\n", pDoctorItem->Name());
			++uDignosisItemCount;
			if (0 != pDoctorItem->Doctor(m_pReportDevice))
			{
				setFailItem.insert(pDoctorItem->Name());

				++uFailItemCount;
				if (0 != m_pReportDevice->IsFailContinue())
				{
					break;
				}
			}
		}
		if (0 != m_pReportDevice->IsFailContinue())
		{
			break;
		}
	}
	///<Save summary
	m_pReportDevice->PrintfToUi("\n\n-------------------------Summary-------------------------\n");
	if (0 != setItemUndiagnosed.size())
	{
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<UndiagnosedItem>\n", lpszSecondIndent);

		for (auto& Item : setItemUndiagnosed)
		{
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<Item>%s</Item>\n", lpszThirdIndent, Item.c_str());
		}
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s</UndiagnosedItem>\n", lpszSecondIndent);
	}
	for (auto& Item : vecItemNeed)
	{
		if (setItemUndiagnosed.end() != setItemUndiagnosed.find(Item))
		{
			m_pReportDevice->PrintfToUi(IHDReportDevice::UnDoctor);
		}
		else if (setFailItem.end() != setFailItem.find(Item))
		{
			m_pReportDevice->PrintfToUi(IHDReportDevice::Fail);
		}
		else
		{
			m_pReportDevice->PrintfToUi(IHDReportDevice::Pass);
		}

		m_pReportDevice->PrintfToUi("\t %s\n", Item.c_str());
	}

	if (0 == uDignosisItemCount)
	{
		///<No valid item diagnosed
		m_pReportDevice->PrintfToUi("\nNo valid item to be diagnosed\n");
	}

	if (m_pReportDevice->IsStop())
	{
		///<User stop, show to UI
		m_pReportDevice->PrintfToUi("\nUser Stop\n");
	}

	if (0 == uFailItemCount)
	{
		nRetVal = 0;
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='true'/>\n", lpszSecondIndent);
	}
	else
	{
		nRetVal = - 1;
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='false'/>\n", lpszSecondIndent);
	}

	dTimeConsume = StopTimer(lpszTimeUnits, sizeof(lpszTimeUnits));
	m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<Time value='%.3f%s'/>\n", lpszSecondIndent, dTimeConsume, lpszTimeUnits);

	m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s</DCM>\n", lpszFirstIndent);
	return nRetVal;
}

void CDiagnosis::SetUserRole(CDiagnosisItem::USER_ROLE UserRole)
{
	m_UserRole = UserRole;
	for (auto& pDoctorItem : m_vecDoctorItem)
	{
		if (nullptr != pDoctorItem)
		{
			pDoctorItem->SetUserRole(m_UserRole);
		}
	}
}

void CDiagnosis::ShowBoardInfo(const char* lpszBaseIndent)
{
	std::string  strFirstIndent = lpszBaseIndent + IndentChar();
	std::string  strSecondIndent = strFirstIndent + IndentChar();
	const char* lpszFirstIndent = strFirstIndent.c_str();
	const char* lpszSecondIndent = strSecondIndent.c_str();

	BYTE bySlotNo = 0;
	BYTE byBoardControllerIndex = 0;
	map<BYTE, CHardwareFunction*> mapSlot;
	set<UINT> setController;
	CHardwareFunction* pHardware = nullptr;
	for (auto Controller : m_vecEnableController)
	{
		setController.insert(Controller);
		bySlotNo = HDModule::Instance()->ID2Board(Controller, byBoardControllerIndex);
		if (mapSlot.end() != mapSlot.find(Controller))
		{
			continue;
		}
		pHardware = GetHardware(Controller);
		mapSlot.insert(make_pair(bySlotNo, pHardware));
	}

	m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<Board>\n", lpszBaseIndent);
	m_pReportDevice->PrintfToUi("\nBoard Diagnosed\n");

	STS_HARDINFO HardInfo[5];
	int nModuleCount = 0;
	USHORT usBoardRev = 0;
	USHORT usControllerRev = 0;
	for (auto& Slot : mapSlot)
	{
		dcm_GetHardInfo(Slot.first, HardInfo, 5, nModuleCount);
		pHardware = Slot.second;
		usBoardRev = pHardware->GetBoardLogicRevision();
		m_pReportDevice->PrintfToUi(" Slot %d\n", Slot.first);
		m_pReportDevice->PrintfToUi("  SN:       \t%s\n", HardInfo[0].moduleInfo.moduleSN);
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<Slot value='%d'>\n", lpszFirstIndent, Slot.first);

		m_pReportDevice->PrintfToUi("  Revision:    \t0x%04X\n", usBoardRev);
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<SN>%s</SN>\n", lpszSecondIndent, HardInfo[0].moduleInfo.moduleSN);
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<Revision>0x%04X</Revision>\n", lpszSecondIndent, usBoardRev);

		for (BYTE byController = 0; byController < DCM_MAX_BOARD_NUM; ++byController)
		{
			UINT uID = HDModule::Instance()->GetID(bySlotNo, byController);
			if (setController.end() == setController.find(uID))
			{
				continue;
			}
			pHardware = GetHardware(uID);
			usControllerRev = pHardware->GetControllerLogicRevision();
			m_pReportDevice->PrintfToUi("  Controller %d:  0x%04X\n", byController, usControllerRev);
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<Controller value='%d'>0x%04X</Controller>\n", lpszSecondIndent, byController, usControllerRev);
		}
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s</Slot>\n", lpszFirstIndent);
	}
	m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s</Board>\n", lpszBaseIndent);

}
