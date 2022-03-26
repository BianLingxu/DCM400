#pragma once
/**
 * @file MutualTest.h
 * @brief The base class of mutual test in diagnosis
 * @author Guangyun Wang
 * @date 2020/09/03
 * @copyright AccoTEST Business Unit of Beijing Huafeng Test & Control Technology Co., Ltd.
*/
#include "DiagnosisItem.h"
#include "IHDReportDevice.h"
#include "HardwareInfo.h"
#include <set>
class CHardwareFunction;
class CPattern;
/**
 * @class CMutualTest
 * @brief The base class of mutual test
*/
class CMutualTest
	: public CDiagnosisItem
{
public:
	/**
	 * @brief Constructor
	*/
	CMutualTest();
	/**
	 * @brief Destructor
	*/
	~CMutualTest();
	/**
	 * @brief Start the diagnosis item
	 * @param[in] pReportDevice The point of the report device
	 * @return The diagnosis result
	 * - 0 Diagnosis result is PASS
	 * - -1 Diagnosis result is FAIL
	*/
	int Doctor(IHDReportDevice* pReportDevice);
protected:
	/**
	 * @struct TIMESET_VALUE
	 * @brief The timeset information
	*/
	struct TIMESET_VALUE
	{
		double m_dPeriod;///<The period of current timeset
		double m_dEgde[EDGE_COUNT];///<The edge value of current time
		WAVE_FORMAT m_WaveFormat;///<The wave format
		TIMESET_VALUE()
		{
			m_dPeriod = 12;
			memset(m_dEgde, 0, sizeof(m_dEgde));
			m_WaveFormat = WAVE_FORMAT::NRZ;
		}
	};
	/**
	 * @brief Get the current sub diagnosis item name
	 * @return The item name
	*/
	virtual const char* GetSubItemName() const = 0;
	/**
	 * @brief Get the test controller
	 * @param[in] nTestIndex The test index
	 * @param[in] vecTestController The controller will be tested
	 * @param[in] bPrintLog Whether print log
	 * @return Execute result
	 * - 0 Get the test controller successfully
	 * - -1 The test index is over range
	*/
	virtual int GetTestController(int nTestIndex,std::vector<UINT>& vecTestController, BOOL bPrintLog) = 0;
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
	virtual int GetMutualTestVector(UINT uControllerID, int nLineIndex, char* lpszVector, int nVectorBuffSize, char* lpszCMD, int nCMDBuffSize, BYTE* pbyTimeset, ULONG* pulOperand, BOOL* pbSRAMLine, BOOL* pbNextLineOtherMemory) = 0;
	/**
	 * @brief Get the controller whose compare data will be gotten
	 * @param[out] vecCheckController The controller
	*/
	virtual void GetCheckDataController(std::vector<UINT>& vecCheckController) = 0;
	/**
	 * @brief Get the check data type
	 * @param uControllerID The controller ID
	 * @return Check data type
	 * - 0 Check compare result
	 * - -1 Check line ran order
	*/
	virtual int GetCheckDataType(UINT uControllerID);
	/**
	 * @brief Get the timeset setting
	 * @param[in] uControllerID The controller ID
	 * @param[out] mapEdgeValue The edge value
	 * @return Execute Result
	 * - 0 Get timeset setting successfully
	 * - 1 The controller is not tested
	*/
	virtual int GetTimesetSetting(UINT uControllerID, std::map<BYTE, TIMESET_VALUE>& mapEdgeValue) = 0;
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
	virtual int GetPinLevel(UINT uControllerID, double* pdVIH, double* pdVIL, double* pdVOH, double* pdVOL) = 0;
	/**
	 * @brief Get the start line of BRAM and DRAM
	 * @param[out] puBRAMStartLine The start line in BRAM
	 * @param[out] puDRAMStartLine The start line in DRAM
	*/
	virtual void GetVectorStartLine(UINT* puBRAMStartLine, UINT* puDRAMStartLine) = 0;
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
	virtual int CheckResult(UINT uControllerID, const std::map<int, USHORT>& mapBRAMFailLineNo, const std::map<int, USHORT>& mapDRAMFailLineNo, int nFailCount) = 0;
	/**
	 * @brief Check line order of latest ran
	 * @param uControllerID The controller ID
	 * @param vecLineOrder The line oder
	 * @return Execute result
	 * - 0 Check pass
	 * - -1 Check fail
	 * - -2 Not supported
	 * - -3 The controller is not existed
	 * - -4 The controller is not the compared controller
	*/
	virtual int CheckLineOrder(UINT uControllerID, const std::vector<UINT>& vecLineOrder);
	/**
	 * @brief Get the same controller type
	 * @return The same vector controller count
	*/
	virtual int GetSameVectorControllerType() = 0;
	/**
	 * @brief Get controller which use same vector
	 * @param[in] nTypeIndex The type index
	 * @param[out] vecSameController The same controller
	 * @return Execute result
	 * - 0 Get controller successfully
	 * - -1 The type index is over range
	*/
	virtual int GetSameVectorController(int nTypeIndex, std::vector<UINT>& vecSameController) = 0;
	/**
	 * @brief Check whether reload vector
	 * @return whether reload vector
	 * - TRUE Reload vector
	 * - FALSE Not need to reload vector
	*/
	virtual BOOL IsReloadVector() = 0;
	/**
	 * @brief Check whether stop vector
	 * @return Whether  stop test
	 * - TRUE Stop test
	 * - FALSE Continue test
	*/
	virtual BOOL Stop() = 0;
private:
	/**
	 * @struct RUN_INFO
	 * @brief The running information
	*/
	struct RUN_INFO
	{
		UINT m_uStartLine;///<The start line of vector in SRAM
		UINT m_uStopLine;///<The stop line of vector in SRAM
		BOOL m_bUseDRAM;///<Whether use DRAM
		UINT m_uDRAMStartLine;///<The start line in DRAM
		RUN_INFO()
		{
			m_uStartLine = 0;
			m_uStopLine = 0;
			m_bUseDRAM = FALSE;
			m_uDRAMStartLine = 0;
		}
	};
	/**
	 * @brief Start diagnosis
	 * @return Diagnosis result
	 * - 0 Diagnosis pass
	 * - -1 Diagnosis fail
	*/
	int Diagnosis();
	/**
	 * @brief Diagnosis item
	 * @return Execute result
	 * - 0 Diagnosis pass
	 * - -1 Diagnosis fail
	*/
	int DiagnosisItem();
	/**
	 * @brief Connect all channels' relay
	 * @param bConnect Whether connect relay
	*/
	void Connect(BOOL bConnect);
	/**
	 * @brief Run vector
	 * @param mapRunInfo The running information, key is controller ID and value is running information 
	 * @return Execute result
	 * - 0 Finish ran
	 * - -1 No controller to run
	*/
	int Run(std::map<UINT, RUN_INFO> &mapRunInfo);
	/**
	 * @brief Load vector of controller diagnosed
	*/
	void LoadVector();
	/**
	 * @brief Bind controller
	 * @param[in] vecController The controller ID
	 * @param[in] uTargetControllerID The target controller ID
	*/
	void Bind(const std::vector<UINT>& vecController, UINT uTargetControllerID);
	/**
	 * @brief Clear bind
	 * @return Execute result
	 * - 0 Clear bind successfully
	 * - -1 Not bind
	*/
	int ClearBind();
	/**
	 * @brief Wait us
	*/
	void Wait(UINT uUs);
	/**
	 * @brief Check test result
	 * @return Execute result
	 * - 1 Check pass
	 * - 0 Check Fail
	*/
	int CheckResult();
	/**
	 * @brief Clear map data
	 * @tparam Key The key value type
	 * @tparam Value The value type
	 * @param[out] mapData The map needed clear
	*/
	template <typename Key, typename Value>
	inline void ClearMap(std::map<Key, Value*>& mapData);
	/**
	 * @brief Download pattern
	 * @param[in] mapBindController The bind controller information
	*/
	inline void DownloadPattern(const std::map<UINT, std::vector<UINT>>& mapBindController);
	/**
	 * @brief Load the pattern of current test controllers
	*/
	void DownloadPattern();
protected:
	/**
	* @struct SWITCH_INFO
	* @brief The switch information
	*/
	struct SWITCH_INFO
	{
		UINT m_uGlobalLineIndex;///<The global line number of the switch line
		BOOL m_bSwitchOut;///<Whether current line will switch to out memory DRAM
		UINT m_uSwitchLineNo;///<The target line number will be switch to
		UINT m_uSwitchLineCount;///<The line count of next block
		SWITCH_INFO()
		{
			m_uGlobalLineIndex = 0;
			m_bSwitchOut = TRUE;
			m_uSwitchLineNo = 0;
			m_uSwitchLineCount = 0;
		}
	};
	/**
	* @brief Get the switch information of vector line
	* @param[in] uTotalLineCount The total line count of vector
	* @return Execute result
	* - 0 Get the switch information successfully
	* - -1 The total line count is over range
	* - -2 The last fail line number is over total line count
	* - -3 The vector in BRAM is over range
	* - -4 The vector in DRAM is over rang
	*/
	int GetVectorSwitchInfo(UINT uTotalLineCount);
	/**
	* @brief Get the line information
	* @param[in] uGlobalLineIndex The total line index
	* @param[out] pbSRAM Whether the line is saved in SRAM
	* @return The line index in the memory
	* - >=0 The line index in memory
	* - -1 The line is over range
	*/
	int GetLineInfo(UINT uGlobalLineIndex, BOOL* pbSRAM, BOOL* pbNextLineOtherMemory);
	/**
	 * @brief Set the channel status
	 * @param[in] uControllerID The controller ID
	*/
	void SetChannelStatus(UINT uControllerID, CHANNEL_OUTPUT_STATUS ChannelStatus = CHANNEL_OUTPUT_STATUS::LOW);
protected:
	std::map<int, USHORT> m_mapFailLineNo;///<All fail line, key is the global fail line number and value is the fail channel
	std::map<int, USHORT> m_mapTimeset;///<All fail line, key is the global fail line number and value is the fail channel
	std::map<int, USHORT> m_mapBRAMFailLineNo;///<The fail line number in BRAM
	std::map<int, USHORT> m_mapBRAMTimeset;///<The test timeset line number in BRAM
	std::map<int, USHORT> m_mapDRAMFailLineNo;///<The fail line number in DRAM
	std::map<int, USHORT> m_mapDRAMTimeset;///<The test timeset line number in DRAM
	std::set<int> m_setCaptureLine;///<The line using capture
	std::vector<SWITCH_INFO> m_vecSwitch;///<The vector switch information between BRAM and DRAM
	BOOL m_bUseCapture;///<Whether use capture
private:
	std::map<UINT, RUN_INFO>	m_mapRunInfo;///<The running information of controller, the key is controller ID and the value is running information
	std::set<UINT> m_setCurTestController;///<The controller of current diagnosis
	BOOL m_bConnect;///<Whether the relay is connected
	int m_nCurItemTestIndex;///<The current test item index
	std::map<UINT, CPattern*> m_mapPattern;///<The class of pattern, key is the controller ID and value is the point of class
	std::map<UINT, UINT> m_mapUndiagnosableController;///<The undiagnosable controller, key is the controller ID and value is the related controller ID
	UINT m_uBindControllerID;///<The controller ID of latest bind
};