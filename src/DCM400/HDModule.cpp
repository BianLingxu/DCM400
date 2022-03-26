#include "pch.h"
#include "HDModule.h"
#include "STSCoreFx.h"
#include "AcoResCfg.h"
#include "IHDCheck.h"
#include <IAcoDriverPack.h>
#include <DriverGlobal.h>
#include "Sts8100.h"
#include "SelfCheck.h"
#include "SM8250.h"
#pragma comment(lib, "zx_base-x32.lib")
#pragma comment(lib, "aco_res_cfg-x32.lib")
#pragma comment(lib, "AT_CBB.lib")
extern int APIENTRY DCM400_SetConfigInfo(short nBoardCount, const BOARDINFO* pAllBoardInfo);


HD_EXPORT_MODULE(DCM400, HDModule);

HDModule * HDModule::Instance()
{
    return &gs_instance;
}

HDModule::HDModule()
	: m_pDebugTool(STS_NULL)
	, m_pDataInterface(znullptr)
	, m_pCfg(znullptr)
	, m_pDriverPack(znullptr)
{

}

HDModule::~HDModule()
{
	if (STS_NULL != m_pDataInterface)
	{
		delete m_pDataInterface;
		m_pDataInterface = STS_NULL;
	}
	if (STS_NULL != m_pDebugTool)
	{
		delete m_pDebugTool;
		m_pDebugTool = STS_NULL;
	}
}

/*************************** IZUnknown BEGIN****************************/
int HDModule::QueryInterface(const char *	interfaceName,
							 void **		ptr)
{
	return QueryInstance(interfaceName, ptr);
}

zint HDModule::query(const zuuid &	uuid,
					 zptr *			ptr)
{
	if (ZX_IFS_UUID(IHDRes8200) == uuid)
	{
		IHDRes8200 *p = this;
		*ptr = p;
		return 0;
	}

	if (ZX_IFS_UUID(IHDRes8300) == uuid)
	{
		IHDRes8300 *p = this;
		*ptr = p;
		return 0;
	}

	if (ZX_IFS_UUID(IHDResSearch) == uuid)
	{
		IHDResSearch *p = this;
		*ptr = p;
		return 0;
	}
	if (ZX_IFS_UUID(IHDResCal) == uuid)
	{
		IHDResCal *p = this;
		*ptr = p;
		return 0;
	}
// 	if (ZX_IFS_UUID(IHDDataInterface8200) == uuid)
// 	{
// 		if (znullptr == m_pDataInterface)
// 		{
// 			m_pDataInterface = new DataInterface(this);
// 		}
// 		IHDDataInterface8200 *p = m_pDataInterface;
// 		*ptr = p;
// 		return 0;
// 	}
	return 0;
}
zint HDModule::addRef()
{
	return 1;
}
zint HDModule::release()
{
	return 1;
}
zint HDModule::refCount() const
{
	return 1;
}
/*************************** IZUnknown END****************************/


/*************************** IHDModule BEGIN****************************/
int HDModule::QueryInstance(const char *	uuid,
							void **			ptr)
{
	std::string strUuid = uuid;
	if (strUuid == "IHDModule")
	{
		IHDModule *p = this;
		*ptr = p;
		return 0;
	}

	if (strUuid == "IHDRes8200")
	{
		IHDRes8200 *p = this;
		*ptr = p;
		return 0;
	}

	*ptr = STS_NULL;
	return -1;
}

AccoTest::HardwareType HDModule::Type() const
{
	return AccoTest::HardwareType(m_pCfg->hdtype());
}

const char * HDModule::Name() const
{
	static char * name = const_cast<char *>(m_pCfg->name());
	return name;
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
	return -1;
}

int HDModule::IsSupportDoctor() const
{
	return -1;
}

IHDDoctorItem * HDModule::CreateDoctorItems(int  boardID,
											int* selfCheckResult,
											int  selfCheckResultCount) const
{
	return STS_NULL;
}

int HDModule::Doctor(const STSVector<IHDDoctorItem *> & items,
					 IHDReportDevice *					reportDevice)
{
	return -1;
}

void HDModule::PrintDoctorResult(IHDReportDevice * reportDevice)
{
}

void HDModule::SetDriverPackCheck(IHDCheck * driverPackCheck)
{
}

IHDDebugTool * HDModule::DebugTool() const
{
// 	if (znullptr == m_pDebugTool)
// 	{
// 		HDModule *pModule = const_cast<HDModule *>(this);
// 		pModule->m_pDebugTool = new HDDebugTool(pModule);
// 	}
 //	return m_pDebugTool;
	return nullptr;
}
/*************************** IHDModule END****************************/


/*************************** IHDRes8200 BEGIN****************************/
int HDModule::SetAcoResCfg(const AcoResCfg * resCfg)
{
	m_pCfg = resCfg;
//	g_acm200_moduleType = m_pCfg->hdtype();
	return 0;
}

zint HDModule::GetAcoResCfg(const AcoResCfg ** resCfg)
{
	*resCfg = m_pCfg;
	return 0;
}

zint HDModule::SetDriverPack(IAcoDriverPack * pDriverPack)
{
	m_pDriverPack = pDriverPack;
	return 0;
}
zint HDModule::HardWareType() const
{
	return Type();
}

zint HDModule::HardwareExist(zint boardNum,
							 bool exist)
{
	return 0;
}

zint HDModule::BindAddress(DWORD				moduleType,
						   tag_STS_BIND_INFO*	pinfo,
						   zint					startAddress,
						   zint					totalunit,
						   zint					totalsite,
						   SMODULE*				sModule,
						   UINT					totalModuleCnt)
{
	return 0;
}

zint HDModule::BeforeTest()
{
//	acm200_user_test_before();
	return 0;
}

zint HDModule::AfterTest()
{
//	acm200_user_test_after();
	return 1;
}

zint HDModule::UnLoadPgs()
{
//	acm200_set_unload_pgs();
	return 0;
}

zint HDModule::GetSrcName(zint	channel,
						  char* name,
						  zint	nameMaxLength)
{
	// 8300不能双station
// 	if ((channel >= 0) && (channel < ACM200_TOTAL_CHANNEL_COUNT))
// 	{
// 		sts_strcpy(name, nameMaxLength, g_ch_ACM200_SrcName[channel]);
// 		return 0;
// 	}
	return 1;
}

zint HDModule::ClearSrcName(zint station)
{
// 	for (int chcnt = 0; chcnt < ACM200_TOTAL_CHANNEL_COUNT; chcnt++)
// 	{
// 		memset(g_ch_ACM200_SrcName[chcnt], 0, sizeof(g_ch_ACM200_SrcName[chcnt]));
// 	}
	return 0;
}

zint HDModule::EnableCalibration(bool enable)
{
// 	for (int iBd = 0; iBd < EIB_MAX_BOARD_COUNT; iBd++)
// 	{
// 		eib_calibration_load_data(iBd);
// 	}
	return 0;
}

zint HDModule::TestifyHDExist(zint		&chreturn,
							  zint		ch0,
							  va_list	marker)
{
// 	if (znullptr != m_pDriverpack)
// 	{
// 		int retExist = m_pDriverpack->TestifyHDExist(AccoTest::HardwareType(m_pCfg->hdtype()), chreturn, ch0, marker);
// 		return retExist;
// 	}
	return 1;
}

zint HDModule::TestifyHDExist(zint &chreturn,
							  zint ch0, ...)
{
	va_list marker;
	va_start(marker, ch0);
	zint rcode = TestifyHDExist(chreturn, ch0, marker);
	va_end(marker);
	return rcode;
}

zint HDModule::getCalResultFun(GetCalResultFun * fun)
{
	//*fun = (GetCalResultFun)ifun_get_cal_result;
	return 0;
}

zint HDModule::getCalTimeFun(GetCalTimeFun * fun)
{
	//*fun = (GetCalTimeFun)ifun_get_cal_time;
	return 0;
}

zint HDModule::Init()
{
	return 0;
}

zint HDModule::SelfCheck(char*			lpszLogFile,
						 unsigned char	unBoardNo,
						 int*			pnCheckResult,
						 int			nCheckResultSize,
						 int&			nReturnCode)
{

	int nSlotNo = DCM400_GetSlotNo(unBoardNo);
	CSelfCheck SelfCheck(nSlotNo);
	nReturnCode = SelfCheck.Check(lpszLogFile, *pnCheckResult);
	return 0;
}

zint HDModule::ContinueSelfCheck(bool continueFlag)
{
	return 1;
}

zint HDModule::GetCalibrationDate(zuint1	logicChannel,
								  zuint1	site,
								  time_t&	t)
{
	//t = acm200_get_calibration_date(logicChannel, site);
	return 1;
}
zint HDModule::GetCalibrationResult(zuint1	logicChannel,
									zuint1	site,
									zuint1&	ret)
{
	//ret = acm200_get_calibration_result(logicChannel, site);
	return 1;
}

zint HDModule::GetCalibrationInfo(zuint1		logicChannel,
								  zuint1		site,
								  STS_CALINFO&	calInfo)
{
// 	int phyCh = acm200_get_phy_channel_status(logicChannel, site);
// 	if (_ChannelValid(phyCh))
// 	{
// 		acm200_FLASH_get_calibrationInfo(phyCh, calInfo);
// 	}
	return 1;
}

zint HDModule::GetHardwareVersion(zint	boardNo,
								  char*	info,
								  int	infoSize,
								  int&	ret)
{
	return 1;
}

zint HDModule::GetModuleChannelNum()
{
	return 1;
}

zint HDModule::GetSerialNumber(zint		boardNO,
							   char*	info,
							   int		infoSize,
							   int&		ret)
{
// 	for (int ich = 0; ich < ACM200_MAX_CH_PER_BOARD; ich++)
// 	{
// 		int phyCh = boardNO * ACM200_MAX_CH_PER_BOARD + ich;
// 		if (_ChannelValid(phyCh))
// 		{
// 			acm200_get_module_infomation(phyCh, 0, info, infoSize, acm200::VIS_MODULE_SN, STS_MOTHER_BOARD);
// 			break;
// 		}
// 	}
	
	return 0;
}

zint HDModule::GetFunCtrlForDebugTool(IATFunRegion ** funRegion)
{
 	int nRetVal = 0;
// 	if (m_pCfg->debugTool())
// 	{
// 		*funRegion = ATFunMgr<ACM200>::Instance();
// 	}
// 	else
// 	{
// 		ret = 1;
// 	}
	return nRetVal;
}
/*************************** IHDRes8200 END****************************/


/*************************** IHDRes8300 BEGIN****************************/
zint HDModule::CalibrationLoadData(zint boardID, bool enabled)
{
// 	if (znullptr != m_pDriverpack)
// 	{
// 		int chreturn = -1;
// 		int nCh = boardID * ACM200_MAX_CH_PER_BOARD;
// 		TestifyHDExist(chreturn, nCh, nCh + 1, nCh + 2, nCh + 3, nCh + 4, nCh + 5, \
// 			nCh + 6, nCh + 7, nCh + 8, nCh + 9, nCh + 10, nCh + 11, -1);
// 
// 		BOOL bRet = m_pDriverpack->TestifyHDExist(AccoTest::HardwareType(m_pCfg->hdtype()), chreturn, nCh, nCh + 1, nCh + 2, nCh + 3, nCh + 4, nCh + 5, \
// 																									  nCh + 6, nCh + 7, nCh + 8, nCh + 9, nCh + 10, nCh + 11, -1);
// 		if (TRUE == bRet)
// 		{
// 			for (int j = 0; j < ACM200_MAX_CH_PER_BOARD; j++)
// 			{
// 				acm200_calibration_load_data(ACM200_MAX_CH_PER_BOARD * boardID + j, enabled);
// 			}
// 		}
// 	}
	return 1;
}
zint HDModule::RamConfig(BOARDINFO  *				pBoardInfo,
						 int						nBoardCount,
						 int						nChannelPerBoard,
						 AccoTest::HardwareType	BoardType,
						 const char *				lpszTypeName)
{
	if (znullptr == m_pDriverPack)
	{
		return 1;
	}

	IAcoDriverPack2 *drpack2 = znullptr;
	m_pDriverPack->query(ZX_IFS_UUID(IAcoDriverPack2), (zptr *)&drpack2);
	if (znullptr == drpack2)
	{
		return 1;
	}
	DCM400_SetConfigInfo(nBoardCount, pBoardInfo);
	
	return 0;
}
zint HDModule::GetLogicVersionForCom(zuint	logicChannel,
									 zint	site,
									 zuint1	&version)
{
	//int comRev = acm200_get_logic_version(logicChannel, site, acm200::VIS_REV_COM_MODULE);
//	version = comRev;
	return 0;
}
zint HDModule::CheckLogicVersionForCtrl(zint boardID,
										zint &powerOffCnt)
{
// 	int boardNo = boardID - 1;
// 	int nCh = boardNo * ACM200_MAX_CH_PER_BOARD;
// 	
// 	int fpgaRev[2] = { -1, -1 };
// 	fpgaRev[0] = acm200_get_logic_version(nCh, 0, acm200::VIS_REV_CTRL_MODULE);
// 	fpgaRev[1] = acm200_get_logic_version(nCh + 6, 0, acm200::VIS_REV_CTRL_MODULE);
// 	if ((fpgaRev[0] < MODULE_MIN_LOGIC_REV || fpgaRev[0] >= MODULE_MAX_LOGIC_REV) ||
// 		(fpgaRev[1] < MODULE_MIN_LOGIC_REV || fpgaRev[1] >= MODULE_MAX_LOGIC_REV) ||
// 		(fpgaRev[0] >= ACM200_B_MIN_GOLDEN_REV && fpgaRev[0] <= ACM200_B_MAX_GOLDEN_REV) ||
// 		(fpgaRev[1] >= ACM200_B_MIN_GOLDEN_REV && fpgaRev[1] <= ACM200_B_MAX_GOLDEN_REV))
// 	{
// 		powerOffCnt += 1;
// 	}	

	return 0;
}
zint HDModule::StsEnablePowerON(zint boardID, bool enabled)
{
	//acm200_control_board_power(boardID - 1, enabled);
	return 0;
}
zint HDModule::SetMultiSiteFlag(bool flag)
{
	//g_acm200_singleSiteFlag = flag;
	return 0;
}
zint HDModule::IsMultiSiteFlag(bool & flag)
{
	///<Not use routing map, regard as single site
	flag = 1;
	return 1;
}

zint HDModule::GetMaxPhyChannelCount(zint & maxPhyCount)
{
	//maxPhyCount = m_pCfg->maxChannelCount();
	return 1;
}

zint HDModule::GetUserDefPhyChannelCount(zint & channlCount)
{
	//channlCount = g_acm200_userDef_channelCnt;
	return 1;
}
zint HDModule::GetUserDefChannelList(zint index, ZVector<zulong> & perChannelList)
{
// 	for each (auto var in g_vec_acm200_userDef_ChannelList[index])
// 	{
// 		perChannelList.append(var);
// 	}
		
	return 0;
}
zint HDModule::GetUserDefChannelErrorID(zint index, zint & errID)
{
	//errID = g_acm200_errID[index];
	return 1;
}
zint HDModule::GetUserDefChannelErrorMsg(zint index, const char ** errMsg)
{
	//*errMsg = g_acm200_errMsg[index].c_str();
	return 1;
}

zint HDModule::SetBindAddress(const int*				phyInSite,
							  int						totalUnit,
							  int						maxSite,
							  int**						logAndPhy,
							  const DWORD*				gpChannelAddr,
							  const CHANNEL_PROPERTY*	userDefChProperty,
							  int						userDefLogchCnt,
							  bool						singleSiteFlag)
{
// 	if (TRUE == acm200_set_bind_address(phyInSite, totalUnit, totalUnit, logAndPhy, gpChannelAddr, userDefChProperty, userDefLogchCnt, g_acm200_singleSiteFlag))
// 	{
// 		return 0;
// 	}	
	return 0;
}
zint HDModule::SynGetEnableSlot(zulong & slot)
{
	//slot = acm200_syn_get_enable_slot();
	return 0;
}
zint HDModule::SynSetRamInfo(zulong synInterBdRamAddr)
{
	//acm200_syn_set_ramInfo(synInterBdRamAddr);
	return 0;
}
zint HDModule::SynClearRecordInfo()
{
	//acm200_syn_clear_recordInfo();
	return 0;
}
zint HDModule::BeforeLoadPgsDll()
{
// 	for (int iCh = ACM200_MAX_PHY_CHANNEL; iCh < ACM200_TOTAL_CHANNEL_COUNT; iCh++)
// 	{
// 		g_acm200_perChannel_count[iCh] = 0;
// 		for (int jCh = 0; jCh < ACM200_MAX_PHY_CHANNEL; jCh++)
// 		{
// 			g_acm200_perChannel_list[iCh][jCh] = 0xFFFFFFFF;
// 		}
// 	}
// 	g_acm200_userDef_channelCnt = 0;
// 	g_map_acm200_perLogch_phych_list.clear();

	return 0;
}

zint HDModule::SetDefaultChannelInfo(const unsigned long *			dwChList,
									 const ZVector<const char *>	&strChName,
									 int							channelCnt)
{
// 	int iCh = 0;
// 	if (channelCnt > ACM200_MAX_PHY_CHANNEL)
// 	{
// 		channelCnt = ACM200_MAX_PHY_CHANNEL;
// 	}
// 	for (iCh = 0; iCh < ACM200_MAX_PHY_CHANNEL; iCh++)
// 	{
// 		g_acm200_perChannel_count[iCh] = 0;
// 		for (int jCh = 0; jCh < ACM200_MAX_PHY_CHANNEL; jCh++)
// 		{
// 			g_acm200_perChannel_list[iCh][jCh] = 0xFFFFFFFF;
// 		}
// 	}
// 
// 	for (iCh = 0; iCh < channelCnt; iCh++)
// 	{
// 		g_acm200_perChannel_count[iCh] = 1;
// 		g_acm200_perChannel_list[iCh][0] = dwChList[iCh];
// 
// 		sts_strcpy(g_ch_ACM200_SrcName[iCh], STS_ARRAY_SIZE(g_ch_ACM200_SrcName[iCh]), strChName[iCh]);
// 		acm200_set_source_name(iCh, g_ch_ACM200_SrcName[iCh]);
// 	}

	return 0;
}
zint HDModule::SetSelftestBoard(const ZVector<int> & checks, bool mask)
{
	//acm200_set_selftest_board((int *)checks.data(), (int)checks.size()/* 可以强制转换, 板卡不会超过int的最大数目*/, mask);
	return 0;
}
/*************************** IHDRes8300 END****************************/


/*************************** IHDResCal BEGIN****************************/
zint HDModule::Calibration()
{
// 	CACM200_Calibration acm200Cal;
// 	acm200Cal.SetCalDriverPtr(m_pCalDrive);
// 	acm200Cal.CalibrationAlgorithm();
	return 0;
}

zint HDModule::SetCalibrationPtr(ICalDriver* cal)
{
	int ret = 0;
	if (m_pCfg->canCalibration())
	{
		m_pCalDriver = cal;
	}
	else
	{
		ret = 1;
	}
	return ret;
}
/*************************** IHDResCal END****************************/


/*************************** IHDResSearch BEGIN****************************/
zint HDModule::Search(const ZVector<const AcoResCfg *> &	customSearch,
					  SMODULE *								sModule,
					  zint &								iTotalMdCnt,
					  zint &								iTotalBdCnt,
					  zuint2 &								wAddress,
					  str_SBDRESULT *						BDResult,
					  ZVector<const AcoResCfg *> &			load)
{
// 	// EIB板卡的工位绑定发生在扫描阶段
// 	eib_scan_extend_module(customSearch, wAddress, iTotalBdCnt, BDResult, iTotalMdCnt, sModule, load);
	return 0;
}

zint HDModule::SetRes(ZVector<IHDRes8200 *> pResVec)
{
//     g_pResVec.clear();
// 	for each (auto var in pResVec)
// 	{
// 		g_pResVec.append(var);
// 	}
	return 0;
}
/*************************** IHDResSearch END****************************/