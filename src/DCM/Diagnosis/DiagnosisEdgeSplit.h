#pragma once
/**
 * @file DiagnosisEdgeSplilt.h
 * @brief The diagnosis of edge split
 * @author DiagnosisEdgeSplit.h
 * @date 2020/09/12
 * @copyright AccoTEST Business Unit of Beijing Huafeng Test & Control Technology Co., Ltd.
*/
#include "MutualTest.h"
/**
 * @class CDiagnosisEdgeSplit
 * @brief The diagnosis item of Edge split
*/
class CDiagnosisEdgeSplit :
	public CMutualTest
{
public:
	/**
	 * @brief Constructor
	*/
	CDiagnosisEdgeSplit();
	/**
	 * @brief Destructor
	*/
	~CDiagnosisEdgeSplit();
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
	 * @brief Start the diagnosis item
	 * @param[in] pReportDevice The point of the report device
	 * @return The diagnosis result
	 * - 0 Diagnosis result is PASS
	 * - -1 Diagnosis result is FAIL
	*/
	virtual int Doctor(IHDReportDevice* pReportDevice);
	/**
	 * @brief Whether the user select to diagnose this item
	 * @return Whether the user select to diagnose this item
	 * - true User select to diagnose this item
	 * - false User not select to diagnose this item
	*/
	virtual bool IsUserCheck() const;
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
	 * @brief Get the current sub diagnosis item name
	 * @return The item name
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
	 * @param[out] pbSRAMLine Whether the line is saved in SRAM
	 * @param[out] pbNextLineOtherMemory Whether the next line is saved in other memory
	 * @return Execute result
	 * - 0 Get the vector line successfully
	 * - 1 The line index is over range
	*/
	int GetMutualTestVector(UINT uControllerID, int nLineIndex, char* lpszVector, int nVectorBuffSize, char* lpszCMD, int nCMDBuffSize, BYTE* pbyTimeset, ULONG* pulOperand, BOOL* pbSRAMLine, BOOL* pbNextLineOtherMemory);
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
	 * - 1 The controller is not tested
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
	 * @brief Get the start line of BRAM and DRAM
	 * @param[out] puBRAMStartLine The start line in BRAM
	 * @param[out] puDRAMStartLine The start line in DRAM
	*/
	void GetVectorStartLine(UINT* puBRAMStartLine, UINT* puDRAMStartLine);
	/**
	 * @brief Check whether reload vector
	 * @return whether reload vector
	 * - TRUE Reload vector
	 * - FALSE Not need to reload vector
	*/
	BOOL IsReloadVector();
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
	 * @brief Get vector line information
	 * @param[in] nLineIndex The line index
	 * @param[in] bDriveMode Whether the vector is drive mode
	 * @param[out] pbBRAM Whether the vector current line is saved in BRAM
	 * @param[out] lpszPattern The pattern of current line
	 * @param[out] nBuffSize The buff size of the pattern
	 * @param[out] pbyTimeset The timeset of current pattern
	 * @param[out] pbSwitch Whether current line will switch to DRAM
	 * @return Execute result
	 * - 0 Get the pattern information successfully
	 * - >0 The pattern count if the line index is -1
	 * - -1 The line index is over range
	*/
	int GetVectorLineInfo(int nLineIndex, BOOL bDriveMode = FALSE, BOOL* pbBRAM = nullptr, char* lpszPattern = nullptr, int nBuffSize = 0, BYTE * pbyTimeset = nullptr, BOOL* pbSwitch = nullptr);
	/**
	 * @brief Get the pattern information of STBR test
	 * @param[in] nLineIndex The line index, -1 is get the pattern count
	 * @param[in] bDriveMode The controller type, whether the controller using for driver
	 * @param[out] pbBRAM Whether current pattern is in SRAM
	 * @param[out] lpszPattern The pattern information
	 * @param[in] nBuffSize The buff size of pattern point
	 * @param[out] pbyTimeset The timeset of current pattern
	 * @param[out] pbSwitch Whether the pattern switch to the other memory
	 * @return Execute result
	 * - 0 Get the pattern information successfully
	 * - >0 The pattern count if the line index is -1
	 * - -1 The line index is over range
	*/
	int GetSTBRPatternInfo(int nLineIndex, BOOL bDriveMode = FALSE, BOOL* pbBRAM = nullptr, char* lpszPattern = nullptr, int nBuffSize = 0, BYTE* pbyTimeset = nullptr, BOOL* pbSwitch = nullptr);
	/**
	 * @brief Get the pattern information of T1R test
	 * @param[in] nLineIndex The line index, -1 is get the pattern count
	 * @param[in] bDriveMode The controller type, whether the controller using for driver
	 * @param[out] pbBRAM Whether current pattern is in SRAM
	 * @param[out] lpszPattern The pattern information
	 * @param[in] nBuffSize The buff size of pattern point
	 * @param[out] pbyTimeset The timeset of current pattern
	 * @param[out] pbSwitch Whether the pattern switch to the other memory
	 * @return Execute result
	 * - 0 Get the pattern information successfully
	 * - >0 The pattern count if the line index is -1
	 * - -1 The line index is over range
	*/
	int GetT1RPatternInfo(int nLineIndex, BOOL bDriveMode = FALSE, BOOL* pbBRAM = nullptr, char* lpszPattern = nullptr, int nBuffSize = 0, BYTE* pbyTimeset = nullptr, BOOL* pbSwitch = nullptr);
	/**
	 * @brief Check the test result
	 * @param[in] uControllerID The controller ID
	 * @param[in] mapBRAMFailLineNo The capture data in SRAM
	 * @param[in] mapDRAMFailLineNo The capture data in DRAM
	 * @param[in] nFailCount The fail count of the controller
	 * @return Check result
	 * - 0 Check pass
	 * - -1 Check fail
	 * - -2 The controller is not existed
	 * - -3 The controller is not the compared controller
	*/
	virtual int CheckResult(UINT uControllerID, const std::map<int, USHORT>& mapBRAMFailLineNo, const std::map<int, USHORT>& mapDRAMFailLineNo, int nFailCount);
	/**
	 * @brief Save the test result
	*/
	void SaveResult();
	/**
	 * @brief Stop test
	 * @return Execute result
	 * - TRUE Stop test successfully
	 * - FALSE Not allow to stop
	*/
	BOOL Stop();
private:
	std::map<UINT, TEST_INFO> m_mapTestInfo;///<The test information of each controller, key is controller ID and value is its test information
	std::map<UINT, BOOL> m_mapTestResult;///<The test result of the each controller, key is controller ID and value is its test result
	BOOL m_bTestSTBR;///<Whether current test is STBR
	BOOL m_bTestPass;///<Whether test result is pass
	UINT m_uDriverLineCount;///<The line count of drive mode
	UINT m_uCompareLineCount;///<The line count of compare mode
};

