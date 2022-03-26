#pragma once
/**
 * @file HDModule.h
 * @brief The hardware module of DCM, the debug tool and diagnosis base on it
 * @author Guangyun Wang
 * @date 2020/09/13
 * @copyright AccoTEST Business Unit of Beijing Huangfeng Test & Control Technology Co., Ltd.
*/
#include "IHDModule.h"
#include "IHDModule3.h"
#include "IHDModuleDiagnostics.h"
#include "ATGlobal.h"
#include "HardwareInfo.h"
#include <string>
#include <map>
#include <vector>

class HDDoctorResult;
class HDDebugTool;
class CDiagnosis;
/**
 * @class HDModule
 * @brief The hardware module of DCM, the debug tool and diagnosis tool is base on this
*/
class HDModule : public IHDModule, public IHDModule3, public IHDModuleDiagnostics
{
public:
    /**
	 * @enum BOARD
     * @brief The channels and board supported by DCM
    */
    enum BOARD
    {
        ChannelCountPerControl  = DCM_CHANNELS_PER_CONTROL,///<The channel count in each controller
        ControlCountPerBoard    =  DCM_MAX_CONTROLLERS_PRE_BOARD,///<The controller count in each board
        ChannelCountPerBoard    = ChannelCountPerControl *  ControlCountPerBoard,///<The channel count of each board
        
    };
	/**
	 * @enum MyEnum
	 * @brief The split count
	*/
	enum MyEnum
	{
		SplitCount = TIMESET_COUNT,///<The split count of board
	};
	/**
	 * @enum LINE_COUNT
	 * @brief The vector line count
	*/
	enum LINE_COUNT
	{
		BRAMLineCount = DCM_BRAM_PATTERN_LINE_COUNT,///<The vector line in BRAM
		DRAMLineCount = DCM_DRAM_PATTERN_LINE_COUNT,///<The vector line count in DRAM
	};
	
public:
    /**
     * @brief Constructor
    */
    HDModule();
	/**
	 * @brief Destructor
	*/
    ~HDModule();
	/**
	 * @brief Get the instance of the class
	 * @return The point of the instance
	*/
	static HDModule* Instance();
	/**
	 * @brief Get the interface of the module
	 * @param[in] lpszInterfaceName The interface name
	 * @param[out] ppTr The point of the instance
	 * @return Execute result
	 * - 0 Get the instance successfully
	 * - -1 The instance is not existed
	*/
	virtual int QueryInterface(const char* lpszInterfaceName, void ** ppTr);
	/**
	 * @brief Get the interface of the module
	 * @param[in] lpszUUID The interface unique ID
	 * @param[out] ppTr The point of the instance
	 * @return Execute result
	 * - 0 Get the instance successfully
	 * - -1 The instance is not existed
	*/
    virtual int QueryInstance(const char* lpszUUID, void** ptr);
    /**
     * @brief Get the type of the board type
     * @return The type of board
    */
    virtual AccoTest::HardwareType Type() const;
    /**
     * @brief Get the board name
     * @return The board name
    */
    virtual const char * Name() const;
    /**
     * @brief Get the alias of the board
     * @return The alias of board
    */
    virtual const char * Alias() const;
    /**
     * @brief Set the alias of board
     * @param[in] lpszAlias The alias
    */
    virtual void SetAlias(const char * lpszAlias);
    /**
     * @brief Get the board index the channel belongs to
     * @param[in] nChannel The channel number
     * @return The board index
    */
    virtual int GetBoardIndexByChannel(int nChannel) const;
    /**
     * @brief Check whether the board support diagnosis
     * @return Whether board support diagnosis
	 * - 0 Support diagnosis
    */
    virtual int IsSupportDoctor() const;
	/**
	 * @brief Set the self check of board
	 * @param[in] nBoardIndex The board index
	 * @param[in] nSelfCheckResult The self check result of board
	 * @param[in] nSelfCheckResultCount The count of the self check result
	 * @return Execute result
	 * - 0 Set result successfully
	*/
	virtual int SetSelftestResult(int nBoardIndex, int* nSelfCheckResult, int nSelfCheckResultCount);
	/**
	 * @brief Get the diagnosis item
	 * @return The point of diagnosis item
	*/
	virtual IHDDoctorItem* CreateDoctorItem();
	/**
	 * @brief Diagnosis the item, the function has deprecated
	 * @param[in] pHDDoctorItem The point of the diagnosis item
	 * @param[in] pReportDevice The report device
	 * @return Diagnosis result
	*/
	virtual int Doctor(IHDDoctorItem *pHDDoctorItem, IHDReportDevice* pReportDevice);
    /**
     * @brief Diagnose the diagnosis item, deprecated
     * @param[in] vecItem The diagnosis items
     * @param[in] pReportDevice The point of the report device
     * @return The diagnosis result
	 * - 0 All item is pass
	 * - -1 Have fail items
    */
    virtual int Doctor(const STSVector<IHDDoctorItem *> & vecItem, IHDReportDevice * pReportDevice);
    /**
     * @brief Print the result of diagnosis
     * @param[in] pReportDevice The point of report device
    */
    virtual void PrintDoctorResult(IHDReportDevice * pReportDevice);
    /**
     * @brief Set the check of driverpack
     * @param[in] pDriverPackCheck The point of driverpack check
    */
    virtual void SetDriverPackCheck(IHDCheck* pDriverPackCheck);
    /**
     * @brief Get the point of debug tool
     * @return The point of the debug tool
    */
    virtual IHDDebugTool * DebugTool() const;
	/**
	 * @brief The callback function of testui progress
	 * @param[in] infoFun The process information function
	 * @param[in] stepFun The process step function
	*/
	virtual void set_testui_progress_callback(STS_PROGRESS_INFO_FUN infoFun, STS_PROGRESS_FUN stepFun);
    /**
     * @brief Check whether show the children diagnosis item
     * @return Whether show the children diagnosis item
    */
    bool IsShowDoctorChildren() const
	{
        return m_bShowDoctorChildren;
    }
	/**
	 * @brief Get diagnosis item of the board, deprecated
	 * @param[in] nBoardIndex The board index
	 * @param[in] nSelfCheckResult The self check result of board
	 * @param[in] nSelfCheckResultCount The count of the self check result
	 * @return Execute result
	 * - 0 Set result successfully
	*/
	virtual IHDDoctorItem* CreateDoctorItems(int nBoardIndex, int* nSelfCheckResult, int nSelfCheckResultCount) const;
    /**
     * @brief Check whether the board is valid
     * @param[in] nChannel The channel number
     * @param[in] nBoardIndex The board index
     * @return Whether the channel is valid
	 * - true The board is valid
	 * - false The board is invalid
    */
    bool IsValid(int nChannel, int nBoardIndex = -1) const;
	/**
	 * @brief Get callback function of testui progress
	 * @param[out] infoFun The process information function
	 * @param[out] stepFun The process step function
	*/
	virtual void GetProgressFunc(STS_PROGRESS_INFO_FUN &infoFun, STS_PROGRESS_FUN &stepFun);
	/**
	 * @brief Get the board information which need diagnosis
	 * @param[out] vecBoardInfo The board information
	 * @param[out] bWithFailControoler Whether get the check fail controller
	*/
	virtual void GetDiagnosisBoardInfo(std::vector<UINT> vecBoardInfo, BOOL bWithFailControoler = TRUE);
	/**
	 * @brief Initialize the controller information
	*/
	virtual void InitControllerInfo();
	/**
	 * @brief Check whether the board support parallel diagnosis
	 * @return Whether the board support parallel diagnosis
	 * - true support parallel diagnosis
	*/
	virtual int IsParllelDoctor() const;
	/**
	 * @brief Create the diagnosis item
	 * @param[in] pCheckResult The check result of all board
	 * @param[in] nOperatorType The operator type
	 * @return The point of the diagnosis item
	 * - !=nullptr The point of diagnosis item
	 * - nullptr No valid board
	*/
	virtual IHDDoctorItem* CreateDoctorItem(const __STRCHKRESULT* pCheckResult, int nOperatorType);
	/**
	 * @brief Get the ID of controller
	 * @param[in] bySlotNo The slot number
	 * @param[in] byControllerIndex The controllerIndex
	 * @return The ID of controller
	*/
	UINT GetID(BYTE bySlotNo, BYTE byControllerIndex);
	/**
	 * @brief Get the slot number and controller index from ID
	 * @param[in] uID The ID
	 * @param[out] byControllerIndex The controller index
	 * @return The slot number
	*/
	BYTE ID2Board(UINT uID, BYTE& byControllerIndex);
private:
	/**
	 * @struct CHECK_INFO
	 * @brief The self check information
	*/
	struct CHECK_INFO 
	{
		BYTE m_bySlotNo;///<The slot number of the board
		BYTE m_byControllerIndex;///<The controller index
		BOOL m_bCheckPass;///<Whether the self check result is PASS
		CHECK_INFO()
		{
			m_bySlotNo = 0;
			m_byControllerIndex = 0;
			m_bCheckPass = TRUE;
		}
	};
    std::string m_strAlias;///<The alias of the board

    bool m_bShowDoctorChildren; ///<Whether show the children diagnosis item
    HDDoctorResult* m_pDoctorResult;///<The point of the diagnosis result
    IHDCheck* m_pDriverPackCheckRef;///<The reference of driver pack check
	HDDebugTool* m_pDebugTool;///<The point of debug tool
	STS_PROGRESS_INFO_FUN m_pProgressInfoFun;///<The function to set the process information
	STS_PROGRESS_FUN m_pProgressStepFun;///<The function to set the process step
	std::map<UINT, CHECK_INFO> m_mapControllerInfo;///<The self check information of each controller, the key is controller ID and value is self check information
	CDiagnosis* m_pDiagnosis;///<The point of the diagnosis
};
