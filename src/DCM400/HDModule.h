#pragma once

#include "DriverGlobal.h"
#include <string>

#include "IHDModule.h"
#include "IHDRes8200.h"
#include "IHDRes8300.h"
#include "IHDResCal.h"
#include "IHDResSearch.h"
//#include "IHDDoctorItem.h"
class HDDebugTool;
class DataInterface;

/**
* @brief HDModule的功能说明
* HDModule的全部接口都来自于IHDModule、IHDRes8200、IHDRes8300、IHDResCal、IHDResSearch
*/
class HDModule : public IHDModule, public IHDRes8200, public IHDRes8300, public IHDResCal, public IHDResSearch
{
public:
    static HDModule * Instance();

public:
    HDModule();
    ~HDModule();

	/*************************** IZUnknown BEGIN****************************/
	virtual int QueryInterface(const char * interfaceName,
							   void **		ptr);

	virtual zint ZX_CALL query(const zuuid & uuid,
							   zptr*		 ptr);
	virtual zint ZX_CALL addRef();
	virtual zint ZX_CALL release();
	virtual zint ZX_CALL refCount() const;
	/*************************** IZUnknown END****************************/

	/*************************** IHDModule BEGIN****************************/
	virtual int QueryInstance(const char *	uuid,
							  void **		ptr);

	virtual AccoTest::HardwareType Type() const;
	virtual const char * Name() const;
	virtual const char * Alias() const;
	virtual void SetAlias(const char * alias);

	virtual int GetBoardIndexByChannel(int channel) const;

	virtual int IsSupportDoctor() const;

	virtual IHDDoctorItem * CreateDoctorItems(int	boardID,
											  int * selfCheckResult,
											  int	selfCheckResultCount) const;

	virtual int Doctor(const STSVector<IHDDoctorItem *> &	items,
					   IHDReportDevice *					reportDevice);
	virtual void PrintDoctorResult(IHDReportDevice * reportDevice);

	virtual void SetDriverPackCheck(IHDCheck * driverPackCheck);

	/**
	* @brief 配置调试工具
	* @return 调试工具接口
	*/
	virtual IHDDebugTool * DebugTool() const;
	/*************************** IHDModule END****************************/

	/*************************** IHDRes8200 BEGIN****************************/
	virtual zint ZX_CALL SetAcoResCfg(const AcoResCfg * resCfg) override;
 	virtual zint ZX_CALL GetAcoResCfg(const AcoResCfg ** resCfg) override;
 	virtual zint ZX_CALL SetDriverPack(IAcoDriverPack * pDriverPack) override;
 	virtual zint ZX_CALL HardWareType() const override;
	virtual zint ZX_CALL HardwareExist(zint boardNum,
									   bool exist) override;
	virtual zint ZX_CALL BindAddress(DWORD				moduleType,
									 tag_STS_BIND_INFO*	pinfo,
									 zint				startAddress,
									 zint				totalunit,
									 zint				totalsite,
									 SMODULE*			sModule,
									 UINT				totalModuleCnt) override;
	virtual zint ZX_CALL BeforeTest() override;
	virtual zint ZX_CALL AfterTest() override;
	virtual zint ZX_CALL UnLoadPgs() override;
	virtual zint ZX_CALL GetSrcName(zint	channel,
									char*	name,
									zint	nameMaxLength) override;
	virtual zint ZX_CALL ClearSrcName(zint station) override;
	virtual zint ZX_CALL EnableCalibration(bool enable) override;
	virtual zint ZX_CALL TestifyHDExist(zint&	chreturn,
										zint	ch0,
										va_list marker) override;
	/*
		@return	0:存在；其它：不存在
	*/
	zint TestifyHDExist(zint&	chreturn,
						zint	ch0, ...);

   virtual zint ZX_CALL getCalResultFun(GetCalResultFun * fun) override;
   virtual zint ZX_CALL getCalTimeFun(GetCalTimeFun * fun) override;

	virtual zint ZX_CALL Init() override;

	// 自检 调用的函数
	virtual zint ZX_CALL SelfCheck(char* lpszLogFile, unsigned char	unBoardNo, int* pnCheckResult, int nCheckResultSize, int& nReturnCode) override;
	virtual zint ZX_CALL ContinueSelfCheck(bool continueFlag) override;

	virtual zint ZX_CALL GetCalibrationDate(zuint1	logicChannel,
											zuint1	site,
											time_t&	t) override;
	virtual zint ZX_CALL GetCalibrationResult(zuint1	logicChannel,
											  zuint1	site,
											  zuint1&	ret) override;
	virtual zint ZX_CALL GetCalibrationInfo(zuint1	logicChannel,
											zuint1	site,
											STS_CALINFO& calInfo) override;
	virtual zint ZX_CALL GetHardwareVersion(zint	boardNo,
											char*	info,
											int	infoSize,
											int&	ret) override;
	virtual zint ZX_CALL GetModuleChannelNum() override;
    virtual zint ZX_CALL GetSerialNumber(zint	boardNo,
										 char*	info,
										 int	infoSize,
										 int&	ret) override;
	// 调试工具调用函数
	virtual zint ZX_CALL GetFunCtrlForDebugTool(IATFunRegion ** funRegion) override;
	/*************************** IHDRes8200 END****************************/

	/*************************** IHDRes8300 BEGIN****************************/
	virtual zint ZX_CALL CalibrationLoadData(zint boardID, bool enabled) override;
	virtual zint ZX_CALL RamConfig(BOARDINFO  *				pBoardInfo,
								   int						nBoardCount,
								   int						nChannelPerBoard,
								   AccoTest::HardwareType	BoardType,
								   const char *				lpszTypeName) override;
	virtual zint ZX_CALL GetLogicVersionForCom(zuint	logicChannel,
											   zint		site,
											   zuint1	&version) override;
	virtual zint ZX_CALL CheckLogicVersionForCtrl(zint boardID,
												  zint &powerOffCnt) override;
	virtual zint StsEnablePowerON(zint boardID, bool enabled) override;
	virtual zint ZX_CALL SetMultiSiteFlag(bool flag) override;
	virtual zint ZX_CALL IsMultiSiteFlag(bool & flag) override;

	virtual zint ZX_CALL GetMaxPhyChannelCount(zint & maxPhyCount) override;

	virtual zint ZX_CALL GetUserDefPhyChannelCount(zint & channlCount) override;
	virtual zint ZX_CALL GetUserDefChannelList(zint index, ZVector<zulong> & perChannelList) override;
	virtual zint ZX_CALL GetUserDefChannelErrorID(zint index, zint & errID) override;
	virtual zint ZX_CALL GetUserDefChannelErrorMsg(zint index, const char ** errMsg) override;

	virtual zint ZX_CALL SetBindAddress(const int*					phyInSite,
										int							totalUnit,
										int							maxSite,
										int**						logAndPhy,
										const DWORD*				gpChannelAddr,
										const CHANNEL_PROPERTY*		userDefChProperty,
										int							userDefLogchCnt,
										bool						singleSiteFlag = true) override;
	virtual zint ZX_CALL SynGetEnableSlot(zulong & slot) override;
	virtual zint ZX_CALL SynSetRamInfo(zulong synInterBdRamAddr) override;
	virtual zint ZX_CALL SynClearRecordInfo() override;
	virtual zint ZX_CALL BeforeLoadPgsDll() override;

	virtual zint ZX_CALL SetDefaultChannelInfo(const unsigned long *		dwChList,
											   const ZVector<const char *>	&strChName,
											   int							channelCnt) override;
	virtual zint ZX_CALL SetSelftestBoard(const ZVector<int> & checks, bool mask) override;
	/*************************** IHDRes8300 END****************************/

	/*************************** IHDResCal BEGIN****************************/
	virtual zint ZX_CALL Calibration() override;
	virtual zint ZX_CALL SetCalibrationPtr(ICalDriver* cal) override;
	/*************************** IHDResCal END****************************/

	/*************************** IHDResSearch BEGIN****************************/
	virtual zint ZX_CALL Search(const ZVector<const AcoResCfg *> &	customSearch,
								SMODULE *							sModule,
								zint &								iTotalMdCnt,
								zint &								iTotalBdCnt,
								zuint2 &							wAddress,
								str_SBDRESULT *						BDResult,
								ZVector<const AcoResCfg *> &		load) override;

	virtual zint ZX_CALL SetRes(ZVector<IHDRes8200 *> pResVec) override;
	/*************************** IHDResSearch END****************************/


private:
	const AcoResCfg*	m_pCfg;
	IAcoDriverPack*		m_pDriverPack;
	std::string			m_strAlias;
	HDDebugTool*		m_pDebugTool;
	DataInterface*		m_pDataInterface;
	ICalDriver*			m_pCalDriver;
};

