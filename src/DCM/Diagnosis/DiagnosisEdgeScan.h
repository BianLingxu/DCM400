#pragma once
/**
 * @file DiagnosisEdgeSplilt.h
 * @brief The diagnosis of edge scan
 * @author DiagnosisEdgeSplit.h
 * @date 2020/09/12
 * @copyright AccoTEST Business Unit of Beijing Huafeng Test & Control Technology Co., Ltd.
*/
#include "MutualTest.h"
/**
 * @class CDiagnosisEdgeScan
 * @brief The diagnosis of edge scan
*/
class CDiagnosisEdgeScan :
	public CMutualTest
{
public:
	/**
	 * @brief Constructor
	*/
	CDiagnosisEdgeScan();
	/**
	 * @brief Destructor
	*/
	~CDiagnosisEdgeScan();
	/**
	 * @brief Get the instance
	 * @param[in] lpszName The name
	 * @param[in] ppPoint The point
	 * @return Execute result
	 * - -1 Not supported
	*/
	virtual int QueryInstance(const char* lpszName, void** ppPoint);
	/**
	 * @brief Release
	*/
	virtual void Release();
	/**
	 * @brief The diagnosis item name
	 * @return The name of current diagnosis item
	*/
	virtual const char* Name() const;
	/**
	 * @brief Get the child diagnosis item
	 * @param[in] vecChildren child The diagnosis item
	 * @return The count of children diagnosis item
	*/
	virtual int GetChildren(STSVector<IHDDoctorItem*>& vecChildren) const;
	/**
	 * @brief Enable the diagnosis item
	 * @param[in] nEnable Whether enable the diagnosis item
	 * @return Execute result
	 * - 0 Enable item successfully
	*/
	virtual int SetEnabled(int nEnable);
	/**
	 * @brief Check whether the item is enabled
	 * @return Whether the item is enabled
	*/
	virtual int IsEnabled() const;
	/**
	 * @brief Whether the user select to diagnose this item
	 * @return Whether the user select to diagnose this item
	 * - true User select to diagnose this item
	 * - false User not select to diagnose this item
	*/
	virtual bool IsUserCheck() const;
	/**
	 * @brief Start the diagnosis item
	 * @param[in] pReportDevice The point of the report device
	 * @return The diagnosis result
	 * - 0 Diagnosis result is PASS
	 * - -1 Diagnosis result is FAIL
	*/
	int Doctor(IHDReportDevice* pReportDevice);
private:
	/**
	 * @struct TEST_INFO
	 * @brief The test information of controller
	*/
	struct TEST_INFO
	{
		BOOL m_bDriveMode;///<Whether the mode is driver
		UINT m_uRelateControllerID;///<The related controller
		TEST_INFO()
		{
			m_bDriveMode = TRUE;
			m_uRelateControllerID = 0;
		}
	};
	/**
	 * @brief Get the name of the sub item
	 * @return The name of the sub item
	*/
	const char* GetSubItemName() const;
	/**
	 * @brief Get the test controller
	 * @param[in] nTestIndex The test index
	 * @param[in] vecTestController The controller will be tested
	 * @param[in] bPrintLog Whether print log
	 * @return Execute result
	 * - 0 Get the test controller successfully
	 * - -1 The test index is over range
	*/
	int GetTestController(int nTestIndex, std::vector<UINT>& vecTestController, BOOL bSaveLog);
	/**
	 * @brief Get the mutual test vector
	 * @param[in] uControllerID The controller ID
	 * @param[in] nLineIndex The vector line index
	 * @param[out] lpszVector The vector string
	 * @param[in] nVectorBuffSize The buff size vector point
	 * @param[out] lpszCMD The command string
	 * @param[in] nCMDBuffSize The size of command point
	 * @param[out] pbyTimeset The timeset index
	 * @param[out] pulOperand The operand
	 * @param[out] pbBRAMLine Whether the line is saved in BRAM
	 * @param[out] pbNextLineOtherMemory Whether the next line is saved in other memory
	 * @return Execute result
	 * - 0 Get the vector line successfully
	 * - 1 The line index is over range
	*/
	int GetMutualTestVector(UINT uControllerID, int nLineIndex, char* lpszVector, int nVectorBuffSize, char* lpszCMD, int nCMDBuffSize, BYTE* pbyTimeset, ULONG* pulOperand, BOOL* pbBRAMLine, BOOL* pbNextLineOtherMemory);
	/**
	* @brief Get the controller whose compare data will be gotten
	* @param[out] vecCheckController The controller
	*/
	void GetCheckDataController(std::vector<UINT>& vecCheckController);
	/**
	 * @brief Get the timeset setting
	 * @param[in] uControllerID The controller ID
	 * @param[out] mapEdgeValue The edge value
	 * @return Execute Result
	 * - 0 Get timeset setting successfully
	 * - -1 The controller is not tested
	*/
	int GetTimesetSetting(UINT uControllerID, std::map<BYTE, TIMESET_VALUE>& mapEdgeValue);
	/**
	 * @brief Get the pin level of controller
	 * @param[in] uControllerID The controller ID
	 * @param[out] pdVIH The high voltage of input
	 * @param[out] pdVIL The low voltage of input
	 * @param[out] pdVOH The high voltage of output
	 * @param[out] pdVOL The low voltage of output
	 * @return Execute Result
	 * - 0 Get pin level successfully
	 * - -1 The controller is not tested
	*/
	int GetPinLevel(UINT uControllerID, double* pdVIH, double* pdVIL, double* pdVOH, double* pdVOL);
	/**
	 * @brief Get the start line of BRAM and DDRAM
	 * @param[out] puBRAMStartLine The start line in BRAM
	 * @param[out] puDRAMStartLine The start line in DDRAM
	*/
	void GetVectorStartLine(UINT* puBRAMStartLine, UINT* puDRAMStartLine);
	/**
	 * @brief Check the test result
	 * @param[in] uControllerID The controller ID
	 * @param[in] mapBRAMFailLineNo The capture data in BRAM
	 * @param[out] mapDRAMFailLineNo The capture data in DRAM
	 * @param[in] nFailCount The fail count of the controller
	 * @return Check result
	 * - 0 Check pass
	 * - -1 Check fail
	 * - -2 The controller is not existed
	 * - -3 The controller is not the compared controller
	*/
	int CheckResult(UINT uControllerID, const std::map<int, USHORT>& mapBRAMFailLineNo, const std::map<int, USHORT>& mapDRAMFailLineNo, int nFailCount);
	/**
	 * @brief Get the same controller type
	 * @return The same vector controller count
	*/
	int GetSameVectorControllerType();
	/**
	 * @brief Get controller which use same vector
	 * @param[in] nTypeIndex The type index
	 * @param[out] vecSameController The same controller
	 * @return Execute result
	 * - 0 Get controller successfully
	 * - -1 The type index is over range
	*/
	int GetSameVectorController(int nTypeIndex, std::vector<UINT>& vecSameController);
	/**
	 * @brief Check whether reload vector
	 * @return whether reload vector
	 * - TRUE Reload vector
	 * - FALSE Not need to reload vector
	*/
	BOOL IsReloadVector();
	/**
	 * @brief Get the vector line information
	 * @param[in] nLineIndex The line index
	 * @param[in] bDriveMode Whether driver mode
	 * @param[out] lpszPattern The pattern
	 * @param[in] nBuffSize The buff size
	 * @param[out] pbyTimeset The timeset
	 * @return Execute result
	 * - 0 Get vector successfully
	 * - -1 The line index is over range
	*/
	int GetVectorLineInfo(int nLineIndex, BOOL bDriveMode, char* lpszPattern, int nBuffSize, BYTE *pbyTimeset);
	/**
	 * @brief Check whether stop vector
	 * @return Whether  stop test
	 * - TRUE Stop test
	 * - FALSE Continue test
	*/
	BOOL Stop();
	/**
	 * @brief Get current test edge
	 * @param[out] nTestIndex The test index
	 * @return The test rate
	*/
	inline int GetTestEdge(int nTestIndex, BOOL bTestSTBR);
	/**
	 * @brief Save the test result
	 * @param[in] dEdgeValue The test edge
	*/
	inline void SaveEdgeResult(double dEdgeValue);
private:
	int	m_nCompareLineCount;///<The vector line count of compare controller
	int	m_nDriveLineCount;///<The vector line count of drive controller
	std::map<UINT, TEST_INFO> m_mapTestInfo;///<The test information of each controller, key is controller ID and value is its test information
	std::map<UINT, BOOL> m_mapTestResult;///<The test result of the each controller, key is controller ID and value is its test result
	std::map<UINT, BYTE> m_mapItemTestResult;
	BOOL m_bTestSTBR;///<Whether test the STBR
	double m_dPeriod;///<The test period
	double m_dEdgeValue;///<The test edge value
	int m_nSTBRTestEdgeCount;///<The tested edge count of STBR
	BOOL m_bSaveLogHead;///<Whether save the head of log
	BOOL m_bNeedLoadVector;///<Whether need load vector
	BOOL m_bTestPass;///<Whether the test result of the item is pass
	BOOL m_bHaveTest;///<Whether have been tested before
	std::string	m_strResultIndent;///<The indent of the test result
	BOOL m_bSaveLog;///<Whether save log
	int m_nCurItemTestIndex;///<The item index of current test
};

