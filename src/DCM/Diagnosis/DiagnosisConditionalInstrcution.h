#pragma once
/**
 * @file DiagnosisConditionalInstruction.h
 * @brief The diagnosis item of conditional instruction
 * @author Guangyun Wang
 * @date 2020/09/06
 * @copyright AccoTEST Business Unit of Beijing Huafeng Test & Control Technology Co., Ltd.
*/
#include "MutualTest.h"
/**
 * @class CDiagnosisConditionalInstrcution
 * @brief The diagnosis item of conditional instruction
*/
class CDiagnosisConditionalInstrcution :
    public CMutualTest
{
public:
	/**
	 * @brief Constructor
	*/
	CDiagnosisConditionalInstrcution();
	/**
	 * @brief Destructor
	*/
	~CDiagnosisConditionalInstrcution();
	/**
	 * @brief Get the instance of th item
	 * @param[in] lpszName The item name
	 * @param[in] ptr The point of the item
	 * @return Execute result
	 * - -1 Not supported
	*/
	virtual int QueryInstance(const char* lpszName, void** ptr);
	/**
	 * @brief Release the item
	*/
	virtual void Release();
	/**
	 * @brief Get the diagnosis item name
	 * @return The name of the diagnosis
	*/
	virtual const char* Name() const;
	/**
	 * @brief Get the children item of current diagnosis item
	 * @param[in] vecChildren The children item
	 * @return The children item count
	*/
	virtual int GetChildren(STSVector<IHDDoctorItem*>& vecChildren) const;
	/**
	 * @brief Start diagnosis
	 * @param[in] pReportDevice The point of the report device
	 * @return The diagnosis result
	 * - 0 Diagnosis pass
	 * - -1 Diagnosis fail
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
	int GetTestController(int nTestIndex, std::vector<UINT>& vecTestController, BOOL bPrintLog);
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
	int GetMutualTestVector(UINT uControllerID, int nLineIndex, char* lpszVector, int nVectorBuffSize, char* lpszCMD, int nCMDBuffSize, 
		BYTE* pbyTimeset, ULONG* pulOperand, BOOL* pbBRAMLine, BOOL* pbNextLineOtherMemory);
	/**
	 * @brief Get the start line of BRAM and DRAM
	 * @param[out] puBRAMStartLine The start line in BRAM
	 * @param[out] puDRAMStartLine The start line in DRAM
	*/
	void GetVectorStartLine(UINT* puBRAMStartLine, UINT* puDRAMStartLine);
	/**
	 * @brief Get the controller whose compare data will be gotten
	 * @param[out] vecCheckController The controller
	*/
	void GetCheckDataController(std::vector<UINT>& vecCheckController);
	/**
	 * @brief Get the check data type
	 * @param[in] uControllerID The controller ID
	 * @return Check data type
	 * - 0 Check compare result
	 * - 1 Check line ran order
	*/
	int GetCheckDataType(UINT uControllerID);
	/**
	 * @brief Get the check data type
	 * @return Check data type
	 * - 0 The check data type is compare result
	 * - 1 The check data type is line order
	*/
	int GetCheckDataType();
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
	 * - -2 The point of the pin level is nullptr
	*/
	int GetPinLevel(UINT uControllerID, double* pdVIH, double* pdVIL, double* pdVOH, double* pdVOL);
	/**
	 * @brief Check the test result
	 * @param[in] uControllerID The controller ID
	 * @param[in] mapBRAMFailLineNo The capture data in BRAM
	 * @param[out] mapDRAMFailLineNo The capture data in DRAM
	 * @return Check result
	 * - 0 Check pass
	 * - -1 Check fail
	 * - -2 The controller is not existed
	 * - -3 The controller is not the compared controller
	*/
	int CheckResult(UINT uControllerID, const std::map<int, USHORT>& mapBRAMFailLineNo, const std::map<int, USHORT>& mapDRAMFailLineNo, int nFailCount);
	/**
	 * @brief Check line order of latest ran
	 * @param[in] uControllerID The controller ID
	 * @param[in] vecLineOrder The line oder
	 * @return Execute result
	 * - 0 Check pass
	 * - -1 Check fail
	 * - -2 Not supported
	 * - -3 The controller is not existed
	 * - -4 The controller is not the compared controller
	*/
	int CheckLineOrder(UINT uControllerID, const std::vector<UINT>& vecLineOrder);
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
	 * @brief Check whether stop vector
	 * @return Whether  stop test
	 * - TRUE Stop test
	 * - FALSE Continue test
	*/
	BOOL Stop();
	/**
	 * @brief save the test result
	*/
	void SaveResult();
	/**
	 * @brief Get the pattern of MJUMP
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
	 * - -1 The line index is over range
	 * - -2 The controller ID is over range
	*/
	int GetMJUMPPattern(UINT uControllerID, int nLineIndex, char* lpszVector, int nVectorBuffSize, char* lpszCMD, int nCMDBuffSize,
		BYTE* pbyTimeset, ULONG* pulOperand, BOOL* pbBRAMLine, BOOL* pbNextLineOtherMemory);
	/**
	 * @brief Get the pattern of FJUMP
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
	 * - -1 The line index is over range
	 * - -2 The controller ID is over range
	*/
	int GetFJUMPPattern(UINT uControllerID, int nLineIndex, char* lpszVector, int nVectorBuffSize, char* lpszCMD, int nCMDBuffSize,
		BYTE* pbyTimeset, ULONG* pulOperand, BOOL* pbBRAMLine, BOOL* pbNextLineOtherMemory);
	/**
	 * @brief Get the pattern of match
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
	 * - -1 The line index is over range
	 * - -2 The controller ID is over range
	*/
	int GetMATCHPattern(UINT uControllerID, int nLineIndex, char* lpszVector, int nVectorBuffSize, char* lpszCMD, int nCMDBuffSize,
		BYTE* pbyTimeset, ULONG* pulOperand, BOOL* pbBRAMLine, BOOL* pbNextLineOtherMemory);

private:
	/**
	 * @struct TEST_INFO
	 * @brief The test information of the controller
	*/
	struct TEST_INFO
	{
		BOOL m_bDriveMode;///<Whether the controller is drive mode
		UINT m_uRelateControllerID;///<The relate controller ID
		TEST_INFO()
		{
			m_bDriveMode = TRUE;
			m_uRelateControllerID = 0;
		}
	};
	std::map<UINT, TEST_INFO> m_mapTestInfo;///<The current test information
	UINT m_uSubItemIndex;///<The sub item index
	std::vector<std::string> m_vecSubItem;///<The sub item in current diagnosis item
	std::map<UINT, int> m_mapTestResult;///<The controller result, the key is controller ID and the value is test result
	std::vector<UINT> m_vecLineOrder;///<The expected line oder of latest ran
};

