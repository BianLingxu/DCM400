#include "HDModule.h"
#include "Diagnosis\Diagnosis.h"
#include "STSCoreFx.h"
#include "SM8213.h"
#include "IHDReportDevice.h"
#include "HDDoctorResult.h"
#include "IHDCheck.h"
#include "DebugTool/HDDebugTool.h" 
using namespace std;

#include "HardwareFunction.h"

HD_EXPORT_MODULE(DCM, HDModule);

HDModule * HDModule::Instance()
{
    return &gs_instance;
}

HDModule::HDModule()
    : m_bShowDoctorChildren(false)
	, m_pDoctorResult(STS_NULL)
	, m_pDriverPackCheckRef(STS_NULL)
	, m_pDiagnosis(STS_NULL)
{
	m_pDebugTool = new HDDebugTool(this);

	m_pProgressInfoFun = nullptr;
	m_pProgressStepFun = nullptr;

	if (0 != m_mapControllerInfo.size())
	{
		m_mapControllerInfo.clear();
	}
}

HDModule::~HDModule()
{
    if (STS_NULL != m_pDoctorResult)
    {
        delete m_pDoctorResult;
        m_pDoctorResult = STS_NULL;
    }
    m_pDriverPackCheckRef = STS_NULL;

	if (STS_NULL != m_pDebugTool)
	{
		delete m_pDebugTool;
		m_pDebugTool = STS_NULL;
	}
	if (STS_NULL != m_pDiagnosis)
	{
		delete m_pDiagnosis;
		m_pDiagnosis = STS_NULL;
	}
}

int HDModule::QueryInstance(const char * uuid, void ** ptr)
{
    return QueryInterface(uuid, ptr);
}

int HDModule::QueryInterface(const char * interfaceName, void ** ptr)
{
	if (STS_NULL != ptr)
	{
		if (0 == strcmp(interfaceName, "IHDModule"))
		{
			IHDModule *p = this;
			*ptr = p;
			return 0;
		}
		if (0 == strcmp(interfaceName, "IHDModule3"))
		{
			IHDModule3 *p = this;
			*ptr = p;
			return 0;
		}
		if (0 == strcmp(interfaceName, "IHDModuleDiagnostics"))
		{
			IHDModuleDiagnostics* p = this;
			*ptr = p;
			return 0;
		}
	}
	return -1;
}

AccoTest::HardwareType HDModule::Type() const
{
    return AccoTest::HD_DCM;
}

const char * HDModule::Name() const
{
    const static char * lpszName = "DCM";
    return lpszName;
}

const char * HDModule::Alias() const
{
    return m_strAlias.c_str();
}

void HDModule::SetAlias(const char * alias)
{
    m_strAlias = alias;
}

int HDModule::GetBoardIndexByChannel(int channel) const
{
    int nRet = -1;
    nRet = channel / ChannelCountPerBoard;
    return nRet;
}

int HDModule::IsSupportDoctor() const
{
    return 0;
}

enum SelfCheckState
{
    /* 来自check模块check.h文件 */
    // 模块所有通道的自检结果，0失效，1成功，2未自检, 3, not exist
    Fail = 0,
    Pass = 1,
    UnCheck = 2,
    NOExit = 3
};


int HDModule::SetSelftestResult(int boardID, int* selfCheckResult, int selfCheckResultCount)
{
	do
	{
		int nSlot = dcm_get_board_slot(boardID);
		if (0 >= nSlot)
		{
			break;
		}
		int i = 0;
		for (i = 0; i < ControlCountPerBoard; ++i)
		{
			if (0 == m_mapControllerInfo.size())
			{
				InitControllerInfo();
			}
			UINT uID = GetID(nSlot, i);
			auto iterTestInfo = m_mapControllerInfo.find(uID);
			if (m_mapControllerInfo.end() != iterTestInfo)
			{
				iterTestInfo->second.m_bCheckPass = selfCheckResult[i * ChannelCountPerControl];
			}
		}
	} while (false);
	return 0;
}

IHDDoctorItem* HDModule::CreateDoctorItem()
{
	if (STS_NULL == m_pDiagnosis)
	{
		m_pDiagnosis = new CDiagnosis();
	}
	
	return m_pDiagnosis;
}
int HDModule::Doctor(IHDDoctorItem* pHDDoctorItem, IHDReportDevice* pReportDevice)
{
	return 0;
}

int HDModule::Doctor(const STSVector<IHDDoctorItem *> & vecItems, IHDReportDevice * pReportDevice)
{
    int nRetVal = -1;
    do 
    {
        if (nullptr != m_pDoctorResult)
        {
            delete m_pDoctorResult;
			m_pDoctorResult = nullptr;
        }
        m_pDoctorResult = new HDDoctorResult;
        if (nullptr == pReportDevice)
        {
            break;
        }
		nRetVal = 0;
		int nCurResult = 0;
		for (auto& Item : vecItems)
		{
			nCurResult = ((CDiagnosisItem*)Item)->Doctor(pReportDevice);
			if (0 != nCurResult)
			{
				nRetVal = -1;
			}
		}        
    } while (false);
		
    return nRetVal;
}

void HDModule::PrintDoctorResult(IHDReportDevice * pReportDevice)
{
    do 
    {
        if (STS_NULL == m_pDoctorResult)
        {
            break;
        }
        const std::vector<HDDoctorResultBoard> &vecBoards = m_pDoctorResult->boardResult;
        for (auto Board : vecBoards)
        {
            const std::vector<HDDoctorResultContrl> &vecControls = Board.controlResult;
            for (size_t j = 0; j < vecControls.size(); ++j)
            {
                const HDDoctorResultContrl &contrl = vecControls.at(j);
                if (1 == contrl.pass)
                {
                    pReportDevice->PrintfToUi(IHDReportDevice::Pass);
                    
                }
                else
                {
                    pReportDevice->PrintfToUi(IHDReportDevice::Fail);
                }
                pReportDevice->PrintfToUi("\tDCM slot %d, controller %d \n", Board.slotIndex, contrl.contrlIndex);
            }

        }
        
    } while (false);
}

void HDModule::SetDriverPackCheck(IHDCheck * driverPackCheck)
{
    m_pDriverPackCheckRef = driverPackCheck;
}

IHDDebugTool * HDModule::DebugTool() const
{
	return m_pDebugTool;
}

void HDModule::set_testui_progress_callback(STS_PROGRESS_INFO_FUN infoFun, STS_PROGRESS_FUN stepFun)
{
	m_pProgressInfoFun = infoFun;
	m_pProgressStepFun = stepFun;;
}

IHDDoctorItem* HDModule::CreateDoctorItems(int boardID, int* selfCheckResult, int selfCheckResultCount) const
{
	return nullptr;
}

bool HDModule::IsValid(int channel, int board) const
{
    if (STS_NULL != m_pDriverPackCheckRef)
    {
        return m_pDriverPackCheckRef->IsValid(channel, board);
    }
    return false;
}

void HDModule::GetProgressFunc(STS_PROGRESS_INFO_FUN &infoFun, STS_PROGRESS_FUN &stepFun)
{
	infoFun = m_pProgressInfoFun;
	stepFun = m_pProgressStepFun;
}

void HDModule::GetDiagnosisBoardInfo(std::vector<UINT> vecBoardInfo, BOOL bWithFailControoler)
{
	vecBoardInfo.clear();
	if (0 == m_mapControllerInfo.size())
	{
		InitControllerInfo();
	}
	for (auto& Board : m_mapControllerInfo)
	{
		if (bWithFailControoler || Board.second.m_bCheckPass)
		{
			vecBoardInfo.push_back(Board.first);
		}
	}
}

void HDModule::InitControllerInfo()
{
	m_mapControllerInfo.clear();
	map<BYTE, BYTE> mapBoardInfo;
	for (int nBoardIndex = 0; nBoardIndex < DCM_MAX_BOARD_NUM; ++nBoardIndex)
	{
		int nSlot = dcm_get_board_slot(nBoardIndex);
		if (0 >= nSlot)
		{
			break;
		}
		CHardwareFunction HardwareFunction(nSlot);
		BYTE byControllerExited = 0;
		for (BYTE byController = 0; byController < DCM_MAX_CONTROLLERS_PRE_BOARD;++byController)
		{
			HardwareFunction.SetControllerIndex(byController);
			if (HardwareFunction.IsControllerExist())
			{
				byControllerExited |= 1 << byController;
			}
		}
		mapBoardInfo.insert(make_pair(nSlot, byControllerExited));
	}


	int nBoardCount = mapBoardInfo.size();
	if (0 == nBoardCount)
	{
		return;
	}

	for (auto& Board : mapBoardInfo)
	{
		CHECK_INFO CheckInfo;
		CheckInfo.m_bySlotNo = Board.first;

		for (BYTE byControllerIndex = 0; byControllerIndex < DCM_MAX_CONTROLLERS_PRE_BOARD; byControllerIndex++)
		{
			if (0 != (Board.second >> byControllerIndex & 0x01))
			{
				CheckInfo.m_byControllerIndex = byControllerIndex;
				m_mapControllerInfo.insert(make_pair(GetID(Board.first, byControllerIndex), CheckInfo));
			}
		}
	}
}

int HDModule::IsParllelDoctor() const
{
	return TRUE;
}

IHDDoctorItem* HDModule::CreateDoctorItem(const __STRCHKRESULT* checkresult, int nUserType)
{
	if (0 == m_mapControllerInfo.size())
	{
		InitControllerInfo();
	}
	if (0 == m_mapControllerInfo.size())
	{
		return nullptr;
	}

	auto iterController = m_mapControllerInfo.begin();
	if (nullptr != checkresult)
	{
		int nBoardCount = checkresult->ChannelCnt / checkresult->ChPerBoard;
		for (int nBoardIndex = 0; nBoardIndex < nBoardCount; ++nBoardIndex)
		{
			for (USHORT usControllerIndex = 0; usControllerIndex < ControlCountPerBoard; ++usControllerIndex)
			{
				if (Pass != checkresult->ChkResult[usControllerIndex * ChannelCountPerControl])
				{
					iterController = m_mapControllerInfo.find(usControllerIndex + nBoardIndex * ControlCountPerBoard);
					if (m_mapControllerInfo.end() != iterController)
					{
						iterController->second.m_bCheckPass = FALSE;
					}
				}
			}
		}

	}
	if (STS_NULL == m_pDiagnosis)
	{
		m_pDiagnosis = new CDiagnosis();
	}
	if (STS_NULL == m_pDiagnosis)
	{
		return STS_NULL;
	}

	vector<UINT> vecEnableController;
	for (auto& Controller : m_mapControllerInfo)
	{
		vecEnableController.push_back(Controller.first);
	}
	m_pDiagnosis->SetEnableController(vecEnableController);

	m_bShowDoctorChildren = false;
#ifdef _DEBUG
	m_bShowDoctorChildren = true;
#endif // _DEBUG
	switch (nUserType)
	{
	case 0:
		m_pDiagnosis->SetUserRole(CDiagnosisItem::PROCUCTION);
		break;
	case 1:
		m_bShowDoctorChildren = true;
		m_pDiagnosis->SetUserRole(CDiagnosisItem::DEVELOPER);
		break;
	case 2:
		m_pDiagnosis->SetUserRole(CDiagnosisItem::USER);
	default:
		m_pDiagnosis->SetUserRole(CDiagnosisItem::USER);
		break;
	}

	return m_pDiagnosis;
}

UINT HDModule::GetID(BYTE bySlotNo, BYTE byControllerIndex)
{
	return (bySlotNo << 24) | byControllerIndex;
}

BYTE HDModule::ID2Board(UINT uID, BYTE& byControllerIndex)
{
	byControllerIndex = uID & 0xFF;
	return uID >> 24;
}
