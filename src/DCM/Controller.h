#pragma once
/**
 * @file Controller.h
 * @brief Include the class of CController
 * @detail The class can realize all function of controller
 * @author Guangyun Wang
 * @date 2020/05/31
 * @copyright AccoTEST Business Unit of Beijing Huafeng Test & Control Technology Co., Ltd.
*/
#include "StdAfx.h"
#include "HardwareFunction.h"
#include "Pattern.h"
#include "FlashInformation.h"
#include "ChannelData.h"
#include <array>
/**
 * @struct LINE_DATA
 * @brief The line data
*/
struct LINE_DATA
{
	int m_nLineNo;///<The line number or offset for DRAM
	BYTE m_byData;///<The data of line
	LINE_DATA()
	{
		m_nLineNo = 0;
		m_byData = 0xFF;
	}
};
/**
 * @class CController
 * @brief The class operation of controller
*/
class CController
{
public:
	/**
	 * @brief Constructor
	 * @param[in] bySlotNo The slot number of board belonged
	 * @param[in] byIndex The controller index
	*/
	CController(BYTE bySlotNo, BYTE byIndex, CDriverAlarm* pAlarm);
	/**
	 * @brief Destructor
	 */
	~CController();
	/**
	 * @brief Whether the controller is existed
	 * @return
	 * - TRUE The controller is existed
	 * - FALSE The controller is not existed
	*/
	BOOL IsExist();
	/**
	 * @brief Get the FPGA revision
	 * @return FPGA revision
	*/
	USHORT GetFPGARevision();
	/**
	* @brief Set the total start delay
	* @param[in] dDelay The delay, unit ns
	* @return Execute result
	* - 0 Set total start delay
	* - -1 The delay is over range
	*/
	int SetTotalStartDelay(double dDelay);
	/**
	 * @brief Get total start delay
	 * @return Total start delay
	*/
	double GetTotalStartDelay();
	/**
	 * @brief The delay of timeset
	 * @param[in] The delay of timeset, unit ns
	 * @return Execute result
	 * - 0 Set timeset delay successfully
	 * - -1 The delay value is over range
	*/
	int SetTimesetDelay(double dDelay);
	/**
	 * @brief Get the IO delay
	 * @return The timeset delay
	*/
	double GetTimesetDelay();
	/**
	 * @brief Set the IO delay of channel
	 * @param[in] usChannel The channel number
	 * @param[in] dData The data delay
	 * @param[in] dDataEn The data enable delay
	 * @param[in] dHigh The high delay
	 * @param[in] dLow The low delay
	 * @return Execute result
	 * - 0 Set the IO delay successfully
	 * - -1 The channel is over range
	 * - -2 The delay is over range
	*/
	int SetIODelay(USHORT usChannel, double dData, double dDataEn, double dHigh, double dLow);
	/**
	 * @brief Get the IO delay of channel
	 * @param[in] usChannel The channel number
	 * @param[out] pdData The data delay
	 * @param[out] pdDataEn The data enable delay
	 * @param[out] pdHigh The high delay
	 * @param[out] pdLow The low delay
	 * @return Execute result
	 * - 0 Get the delay successfully
	 * - -1 The channel number is over range
	 * - -2 The channel is not existed
	 * - -3 The point of delay is nullptr
	*/
	int GetIODelay(USHORT usChannel, double* pdData, double* pdDataEn, double* pdHigh, double* pdLow);
	/**
	 * @brief Update delay data
	*/
	void UpdateDelay();
	/**
	* @brief Add channel pattern
	* @param[in] usChannel The channel number of the controller
	* @param[in] bBRAM The memory which to save the pattern
	* @param[in] uPatternLineIndex The line index in the memory
	* @param[in] cPattern The pattern sign
	* @param[in] byTimeset The timeset of current pattern line
	* @param[in] lpszCMD The command of pattern
	* @param[in] ulOperand The operand
	* @param[in] bCapture Whether the line is capture
	* @param[in] bSwitch Whether the pattern line is the last line in current memory,next line will switch to other memory
	* @return Execute result
	* - 0 Set pattern successfully
	* - -1 The channel is over range
	* - -2 The pattern index is over range
	* - -3 The pattern sign is not supported
	* - -4 The timeset is over range
	* - -5 The point of command is nullptr
	* - -6 The command is not supported
	* - -7 The operand is over range
	*/
	int SetVector(USHORT uChannel, BOOL bBRAM, UINT uStartPatternLine, char cPatternSign, BYTE byTimeset,
		const char* lpszCMD, const char* lpszParallelCMD, USHORT usOperand, BOOL bCapture, BOOL bSwitch);
	/**
	 * @brief Get the pattern of all channel
	 * @param[in] bBRAM Whether the pattern is saved in memory
	 * @param[in] uStartLine The start line number
	 * @param[in] uLineCount The line count
	 * @param[out] lpszPattern The pattern read
	 * @return Execute result
	 * - 0 Get pattern successfully
	 * - -1 The start line is over range
	 * - -2 The line count is over range
	 * - -3 The point of pattern is nullptr
	*/
	int GetPattern(BOOL bBRAM, UINT uStartLine, UINT uLineCount, char(*lpszPattern)[17]);
	/**
	 * @brief Read the data in memory
	 * @param[in] bBRAM The memory type
	 * @param[in] DataType The data type will be read
	 * @param[in] uStartLine The base pattern start line of data read
	 * @param[in] uLineCount The pattern data line count read
	 * @param[out] pusData The data buff which will save the data read
	 * @return  Execute result
	* - 0 Read memory successfully
	* - -1 The data type is not supported
	* - -2 The start line is over range;
	* - -3 The data count is over range
	* - -4 The line count read is 0 or the data buff is nullptr
	*/
	int GetMemory(BOOL bBRAM, DATA_TYPE DataType, UINT uStartLine, UINT uLineCount, USHORT* pusData);
	/**
	* @brief Set the memory of channel
	* @param[in] usChannel The channel number
	* @param[in] bRAM Whether the data line in BRAM
	* @param[in] DataType The data type
	* @param[in] uStartLine The start line will be written from
	* @param[in] uLineCount The write line count
	* @param[out] pbyData The data will be written
	* @return Execute result
	* - 0 Write memory successfully
	* - -1 The channel number is over range
	* - -2 The data type is not supported
	* - -3 Allocate memory fail
	* - -4 The start line is over range
	* - -5 The line count is over range
	* - -6 The line count is 0
	* - -7 The point is data is nullptr
	*/
	int SetMemory(USHORT usChannel, BOOL bBRAM, DATA_TYPE DataType, UINT uStartLine, UINT uLineCount, BYTE* pbyData);
	/**
	 * @brief Load vector to memory
	 * @return Execute result
	 * - 0 Load vector successfully
	 * - -1 Allocate memory fail
	 */
	int LoadVector();
	/**
	 * @brief Set the operand
	 * @param[in] uBRAMLineNo The line number in BRAM
	 * @param[in] usOperand The operand
	* @param[in] bCheckRange Whether check the range of the operand
	 * @return Execute result
	 * - 0 Set the operand successfully
	 * - -1 The line number is over range
	 * - -2 The operand is over range
	*/
	int SetOperand(UINT uBRAMLineNo, USHORT usOperand, BOOL bCheckRange);
	/**
	 * @brief Set the instruction in BRAM
	 * @param[in] uBRAMLineNo The line number in BRAM
	 * @param[in] lpszInstruction The instruction
	 * @param[in] usOperand The operand
	 * @return Execute result
	 * - 0 Set instruction successfully
	 * - -1 The line number is over range
	 * - -2 The instruction is nullptr
	 * - -3 The instruction is not supported
	 * - -4 The operand is over range
	*/
	int SetInstruction(UINT uBRAMLineNo, const char* lpszInstruction, USHORT usOperand);
	/**
	 * @brief Get the instruction of the line
	 * @param[in] uBRAMLineNo The line number of BRAM
	 * @param[out] lpszInstruction The buff for save instruction
	 * @param[in] nBuffSize The buff size
	 * @return Execute result
	 * - 0 Get the instruction successfully
	 * - -1 The line number is over range
	 * - -2 The point of the buff is nullptr
	 * - -3 The buff is too small
	*/
	int GetInstruction(UINT uBRAMLineNo, char* lpszInstruction, int nBuffSize);
	/**
	 * @brief Set the start or stop line number of select fail
	 * @param[in] uRAMLineNo The RAM line number of BRAM or DRAM
	 * @param[in] bStartSave Whether start saving
	 * @param[in] bBRAM Whether the line number is BRAM
	 * @param[in] bClose Whether bDelete start or stop fail
	 * @return Execute result
	 * - 0 Set the saving select fail successfully
	 * - -1 The line number is over range
	*/
	int SetSaveSelectFail(UINT uRAMLineNo, BOOL bStartSave, BOOL bBRAM, BOOL bDelete = FALSE);
	/**
	 * @brief Get the operand of the line
	 * @param[in] uBRAMLineNo The BRAM line number
	 * @return The operand
	 * - >=0 The operand
	 * - -1 The line number is over range
	*/
	int GetOperand(UINT uBRAMLineNo);
	/**
	* @brief Set the period of specific timeset
	* @param[in] byTimesetIndex The timeset index whose period will be set
	* @param[in] dPeriod The period of timeset
	* @return Execute result
	* - 0 Set period successfully
	*/
	int SetPeriod(BYTE byTimesetIndex, double dPeriod);
	/**
	 * @brief Get the timeset period
	 * @param[in] byTimesetIndex The timeset index
	 * @return The period of the timeset
	 * - >0 The period of timeset
	 * - -1 The timeset is over range
	*/
	double GetPeriod(BYTE byTimesetIndex);
	/**
	* @brief Set edge and format of timeset
	* @param[in] vecChannel The channel number whose edge will be set
	* @param[in] byTimesetIndex The timeset index
	* @param[in] WaveFormat The waver format
	* @param[in] pdEdge The edge value
	* @return Execute result
	* - 0 Set the edge successfully
	* - -1 The timeset index is over range
	* - -2 The wave format is error
	* - -3 The point of edge value is nullptr
	* - -4 The value is over range
	*/
	int SetEdge(const std::vector<USHORT>& vecChannel, BYTE byTimesetIndex, double* pdEdge, WAVE_FORMAT WaveFormat, IO_FORMAT IOFormat, COMPARE_MODE CompareMode = COMPARE_MODE::EDGE);
	/**
	 * @brief Get the edge of channel
	 * @param[in] usChannel The channel number
	 * @param[in] byTimesetIndex The timeset index
	 * @param[out] pdEdge The edge value of timeset
	 * @param[out] WaveFormat The wave format
	 * @param[out] IOFormat The IO format
	 * @param[out] CompareMode The compare mode
	 * @return Execute result
	 * - 0 Get edge successfully
	 * - -1 The channel is over range
	 * - -2 The timeset is over range
	 * - -3 The point of edge is nullptr
	*/
	int GetEdge(USHORT usChannel, BYTE byTimesetIndex, double* pdEdge, WAVE_FORMAT& WaveFormat, IO_FORMAT& IOFormat, COMPARE_MODE& CompareMode);
	/**
	* @brief Initialize MCU
	* @param[in] vecChannel The channel
	* @return Execute result
	* - 0 Initialize MCU successfully
	* - -1 The channel is over range
	*/
	int InitMCU(const std::vector<USHORT>& vecChannel);
	/**
	 * @brief Initialize PMU
	 * @param[in] vecChannel The channel whose PMU will be initialized
	 * @return Execute result
	 * - 0 Initialize PMU successfully
	*/
	int InitPMU(const std::vector<USHORT>& vecChannel);
	/**
	 * @brief Set the MCU pin level
	 * @param[in] vecChannel The channel whose pin level will be set
	 * @param[in] dVIH The input voltage of logic high
	 * @param[in] dVIL The input voltage of logic low
	 * @param[in] dVOH The output voltage of logic high
	 * @param[in] dVOL The output voltage of logic low
	 * @return Execute result
	 * - 0 Set pin level successfully
	 * - -1 The pin level is over range
	*/
	int SetPinLevel(const std::vector<USHORT>& vecChannel, double dVIH, double dVIL, double dVOH, double dVOL);
	/**
	 * @brief Set the channel status
	 * @param[in] vecChannel The channel number
	 * @param[in] ChannelStatus The pin status
	 * @return Execute result
	 * - 0 Set channel status successfully
	 * - -1 The channel status is not supported
	 * - -2 The channel is over range
	*/
	int SetChannelStatus(const std::vector<USHORT>& vecChannel, CHANNEL_OUTPUT_STATUS ChannelStatus = CHANNEL_OUTPUT_STATUS::HIGH_IMPEDANCE);
	/**
	 * @brief Update channel mode
	*/
	void UpdateChannelMode();
	/**
	 * @brief Get the channel status
	 * @param[in] usChannel The channel number
	 * @return The channel status
	 * - 0 The level is lower than VOH and VOL
	 * - 1 The level is higher than VOL and lower than VOH
	 * - 2 The level is higher than VOH and lower than VOL
	 * - 3 The level is higher than VOL and VOH
	 * - -1 The channel is over range
	*/
	int GetChannelStatus(USHORT usChannel);
	/**
	 * @brief Set the channel mode to MCU
	 * @param[in] vecChannel The channel whose channel mode will be set
	 * @return Execute result
	 * - 0 Set the mode successfully
	 * - -1 The channel is over range
	*/
	int SetChannelMCUMode(const std::vector<USHORT>& vecChannel);
	/**
	 * @brief Get channel mode
	 * @param[in] usChannel The channel number
	 * @return The channel mode
	 * - 0 The channel is in MCU mode
	 * - 1 The channel is in PMU mode
	 * - 2 The channel is Neither MCU mode, nor PMU mode
	 * - -1 The channel number is over range
	*/
	int GetChannelMode(USHORT usChannel);
	/**
	 * @brief Set the compared channel
	 * @param[in] vecChannel The channel number
	*/
	void SetComparedChannel(const std::vector<USHORT>& vecChannel);
	/**
	 * @brief Set vector run parameter
	 * @param[in] uStartLineNumber Run start line number
	 * @param[in] uStopLineNumber Run stop line number
	 * @param[in] bWithDRAM Whether the vector has DRAM vector
	 * @param[in] uDRAMStartLine The start vector line in DRAM is the vector has DRAM line
	 * @param[in] bEnableStart Whether enable start
	 * @return Execute result
	* - 0 Run vector successfully
	* - -1 The start line number of BRAM is over range
	* - -2 The stop line number is over range
	* - -3 The stop line is not after start line
	* - -4 The start line of DRAM is over range
	* - -5 Unknown error
	*/
	int SetRunParam(UINT uStartLineNumber, UINT uStopLineNumber, BOOL bWithDRAM = FALSE, UINT uDRAMStartLine = 0, BOOL bEnableStart = TRUE);
	/**
	 * @brief Run vector syn
	*/
	void SynRun();
	/**
	 * @brief Disable receive start signal
	 * @param[in] bEnable Enable start
	*/
	void EnableStart(BOOL bEnable);
	/**
	 * @brief Stop vector
	*/
	void Stop();
	/**
	 * @brief Get the MCU run result of specific channel
	 * @return MCU result
	 * - 0 MCU run result PASS
	 * - 1 MCU run result FAIL
	 * - -1 The channel is over range
	 * - -2 No run vector before
	 * - -3 Vector running
	*/
	int GetChannelResult(USHORT usChannel);
	/**
	* @brief Get the vector running result of specific channel
	* @param[in] vecChannel The channel index
	* @return Run result
	* - 0 Running pass
	* - 1 Running fail
	* - -1 Not running before
	* - -2 Vector running
	* - -3 The channel is over range
	*/
	int GetMCUResult(const std::vector<USHORT>& vecChannel);
	/**
	 * @brief Get the status of running
	 * @return Running status
	 * - 0 Running
	 * - 1 Stop running
	 * - -1 Not ran
	*/
	int GetRunningStatus();
	/**
	 * @brief Get fail count of channel
	 * @param[in] usChannel The channel number
	 * @return Fail line count
	 * - >=0 Fail line count
	 * - -1 The channel is over range
	 * - -2 Not ran vector
	 * - -3 Vector running
	*/
	int GetFailCount(USHORT usChannel);
	/**
	 * @brief Get the fail count of current controller
	 * @return The fail count of current controller
	 * - >=0 The fail count of current controller
	 * - -1 Not ran before
	 * - -2 Vector running
	*/
	int GetFailCount();
	/**
	 * @brief Get the fail line number of last running
	 * @param[in] usChannel The channel number
	 * @param[in] uGetMaxFailCount The maximum fail count will be gotten
	 * @param[in] vecBRAMLineNo The BRAM fail line number of latest running
	 * @param[in] vecDRAMLineNo The DRAM fail line number of latest running
	 * @param[in] bForceRefresh Whether force refresh fail line number
	 * @return Execute result
	* - 0 Get the fail line number successfully
	 * - -1 The channel is over range
	 * - -2 Not ran before
	 * - -3 Vector running
	*/
	int GetFailLineNo(USHORT usChannel, UINT uGetMaxFailCount, std::vector<int>& vecBRAMLineNo, std::vector<int>& vecDRAMLineNo, BOOL bForceRefresh = FALSE);
	/**
	 * @brief Get the MCU fail line number
	 * @param[in] vecBRAMLineNo The BRAM fail line number of latest running
	 * @param[in] vecDRAMLineNo The DRAM fail line number of latest running
	 * @param[in] bForceRefresh Whether force refresh fail line number
	 * @return Execute result
	* - 0 Get the fail line number successfully
	 * - -1 Not ran before
	 * - -2 Vector running
	*/
	int GetMCUFailLineNo(std::vector<int>& vecBRAMLineNo, std::vector<int>& vecDRAMLineNo, BOOL bForceRefresh = FALSE);
	/**
	 * @brief Delte MCU fail line number
	 * @param[in] nBRAMDeleteCount The fail count will be deleted in BRAM
	 * @param[in] nDRAMDeleteCount The fail count will be deleted in DRAM
	*/
	void DeleteFailLine(int nBRAMDeleteCount, int nDRAMDeleteCount);
	/**
	 * @brief Get the line number of last certail result in latest ran
	 * @param[in] usChannel The channel number
	 * @param[out] nBRAMLineNo The last line number certail in BRAM
	 * @param[out] bBRAMLineFail The result of last certained line number in BRAM
	 * @param[out] nDRAMLineNo The last line number certained in DRAM
	 * @param[out] bDRAMLineFail The result of last certained line number in BRAM
	 * @return execute result
	 * - 0 Get the certain line successfully
	 * - -1 The channel number is over range
	*/
	int GetLastCertainResultLineNo(USHORT usChannel, int& nBRAMLineNo, BOOL& bBRAMLineFail, int& nDRAMLineNo, BOOL& bDRAMLineFail);
	/**
	 * @brief Preload the fail line number
	 * @param[in] uGetMaxFailLineCount The maximum fail count will be gotten
	 * @return Execute result
	* - 0 Preload fail line number successfully
	 * - -1 Not ran before
	 * - -2 Vector running
	*/
	int PreloadFailLineNo(UINT uGetMaxFailLineCount);
	/**
	 * @brief Clear the fail line number
	*/
	void ClearFailLineNo();
	/**
	 * @brief Get the capture of channel
	 * @param[out] vecBRAMCapture The capture data in BRAM
	 * @param[out] vecDRAMCapture The capture data in DRAM
	 * @return Execute result
	 * - 0 Get the capture successfully
	 * - -1 The channel number is over range
	 * - -2 Vector not ran
	 * - -3 Vector running
	*/
	int GetCapture(USHORT usChannel, std::vector<LINE_DATA>& vecBRAMCapture, std::vector<LINE_DATA>& vecDRAMCapture);
	/**
	 * @brief Get the stop line number
	 * @return The stop line number
	 * - >=0 The stop line number
	 * - -1 Not ran before
	 * - -2 Vector running
	*/
	int GetStopLineNo();
	/**
	 * @brief Get the running line count
	 * @return The line count in latest run
	*/
	ULONG GetRunLineCount();
	/**
	 * @brief Set the calibration information
	 * @param[in] byControllerIndex The controller index of DCM
	 * @param[in] pCalInfo The calibration information of each channel
	 * @param[in] pbyChannelStatus Whether update the calibration information of each channel
	 * @param[in] nElementCount The element count of each array
	 * @return Execute result
	 * - 0 Set the calibration information successfully
	 * - -1 The point of array is nullptr
	 * - -2 The element count is less than the channel of controller
	 * - -3 Set calibration information fail
	*/
	int SetCalibrationInfo(STS_CALINFO* pCalInfo, BYTE* pbyChannelStatus, int nElementCount);
	/**
	 * @brief Get the calibration information
	 * @param[out] pCalInfo The calibration information of each channel
	 * @param[in] nElementCount The element count of each array
	 * @return Execute result
	 * - 0 Get the calibration information successfully
	 * - -1 The point of array is nullptr
	 * - -2 The element count is less than the channel of controller
	 * - -3 Get calibration information fail
	*/
	int GetCalibrationInfo(STS_CALINFO* pCalInfo, int nElementCount);
	/**
	 * @brief Get the calibration information of the channel
	 * @param[in] usChannel The channel number
	 * @param[out] CalibrationInfo The calibration information of the channel
	 * @return Execute result
	 * - 0 Get the calibration information successfully
	 * - -1 The channel is over range
	 * - -2 Get the calibration information fail
	*/
	int GetCalibrationInfo(USHORT usChannel, STS_CALINFO& CalibrationInfo);
	/**
	 * @brief Set the calibration data of each channel
	 * @param[in] pCalibrationData The calibration data of each channel
	 * @param[in] nElement The element count of the array
	 * @return Execute result
	 * - 0 Set calibration data successfully
	 * - -1 The point of calibration data is nullptr
	 * - -2 The element count is less than channel count
	 * - -3 Flash error
	 * - -4 Write to flash fail
	*/
	int SetCalibrationData(CAL_DATA* pCalibrationData, int nElementCount);
	/**
	 * @brief Reset calibration data
	 * @return Execute result
	 * - 0 Reset calibration data successfully
	 * - -1 Not set the memory of calibration data
	*/
	int ResetCalibrationData();
	/**
	 * @brief Get the calibration data
	 * @param[out] pCalibrationData The calibration data of each channel
	 * @param[in] nElementCount The element count of array
	 * @return Execute result
	 * - 0  Get calibration data successfully
	 * - -1 The point of calibration data is nullptr
	 * - -2 The element count is not enough
	 * - -3 Flash status error
	 * - -4 Data in flash is error
	*/
	int GetCalibrationData(CAL_DATA* pCalibrationData, int nElementCount);
	/**
	 * @brief Get the calibration data from flash
	 * @param[in] pCalibrationData The calibration data of each channel
	 * @param[out] nElementCount The element count of the array
	 * @return  Execute result
	 * - 0  Read calibration data successfully
	 * - -1 The flash status is error
	 * - -2 Allocate memory fail
	 * - -3 Data in flash is error
	*/
	int ReadCalibrationData();
	/**
	 * @brief Set dynamic load mode of specific channel
	 * @param[in] vecChannel The channel number
	 * @param[in] bEnable Whether enable dynamic load
	 * @param[in] dIOH The IOH value
	 * @param[in] dIOL The IOL value
	 * @param[in] dVTValue The VT voltage
	 * @param[in] dClmapHigh The high clamp voltage
	 * @param[in] dClampLow The low clamp voltage
	 * @return Execute result
	 * - 0 Set dynamic load mode successfully
	 * - -1 The channel is over range
	 * - -2 The output current is over range
	 * - -3 The VT is over range
	 * - -4 The clamp is over range
	*/
	int SetDynamicLoad(std::vector<USHORT> vecChannel, BOOL bEnable, double dIOH, double dIOL, double dVTValue, double dClmapHigh, double dClampLow);
	/**
	* @brief Get the dynamic load
	* @param[in] usChannel The channel number
	* @param[out] DynamicLoadMode The dynamic load mode
	* @param[out] dIOH The IOH value
	* @param[out] dIOH The IOL value
	* @return Execute result
	* - 0 Get dynamic load successfully
	* - -1 The channel is over range
	*/
	int GetDynamicLoad(USHORT usChannel, BOOL& bEnable, double& dIOH, double& dIOL);
	/**
	* @brief Set PMU settings of specific channel
	* @param[in] vecChannel The channel index
	* @param[in] PMUMode The PMU mode
	* @param[in] Range The current range of PMU
	* @param[in] dSetValue The set value of voltage or current after calibration
	* @param[in] dClmapHigh The high clamp value
	* @param[in] dClampLow The low clamp value
	* @return Execute result
	* - 0 Set PMU successfully
	* - -1 The channel is over range
	* - -2 The PMU mode is error
	* - -3 The current range is error
	* - -4 The set value is over range
	* - -5 The clamp value is over range
	*/
	int SetPMUMode(const std::vector<USHORT>& vecChannel, PMU_MODE PMUMode, PMU_IRANGE Range, double dSetValue, double dClmapHigh, double dClampLow);
	/**
	 * @brief Get the channel whose PMU is clamp
	 * @param[in] vecChannel The channels which will be check clamp status
	 * @param[out] mapClampChannel The clamp channel and its clamp type, key is channel and value is clamp type(0 is voltage clamp and 1 is current clamp)
	*/
	void GetClampChannel(const std::vector<USHORT>& vecChannel, std::map<USHORT, UCHAR>& mapClampChannel);
	/**
	 * @brief Enable PMU clamp flag
	 * @param[in] vecChannel The channel number
	 * @param[in] bEnable Whether enable clamp
	 * @return Execute result
	 * - 0 Set clamp successfully
	 * - -1 The channel number is over range
	*/
	int EnablePMUClampFlag(const std::vector<USHORT>& vecChannel, BOOL bEnable);
	/**
	* @brief Start PMU multi-measure
	* @param[in] vecChannel The channel index
	* @param[in] nSampleTimes The sample times
	* @param[in] dSamplePeriod The sample period
	* @return Execute result
	* - 0 Measure successfully
	* - -1 The channel is over range
	* - -2 Measure fail
	* - -3 PMU need to be waited for stopping
	*/
	int PMUMeasure(const std::vector<USHORT>& vecChannel, int nSampleTimes, double dSamplePeriod);
	/**
	 * @brief Start PMU measurement
	 * @return Execute result
	 * - 0 Start PMU successfully
	 * - -1 No PMU needed to be started
	*/
	int StartPMU();
	/**
	 * @brief Save the PMU average result
	 * @return Execute result
	 * - 0 Save PMU average result successfully
	 * - -1 No result needed to wait
	 * - -2 Finish PMU fail
	*/
	int WaitPMUFinish();
	/**
	 * @brief Get the PMU measure result of specific sample
	 * @param[in] usChannel The channel index
	 * @param[in] nSamppleTimes The sample point, -1 is the average value
	 * @return The measure result
	 * - != 1e15 The measure result
	 * - 1e15 The channel is over range or the sample times is over range
	*/
	double GetPMUMeasureResult(USHORT usChannel, int nSampleTimes);
	/**
	* @brief Get PMU settings of specific channel
	* @param[in] usChannel The channel index
	* @param[out] PMUMode The PMU mode
	* @param[out] PMURange The current range of PMU
	* @return The setting value of current or voltage depends on the PMU mode
	* - 1e15-1 Get PMU fail
	*/
	double GetPMUMode(USHORT usChannel, PMU_MODE& PMUMode, PMU_IRANGE& PMURange);
	/**
	 * @brief Set the preread vector line
	 * @param[in] uStartLine The start line of vector
	 * @param[in] The line count
	 * @param[in] The memory type of the line
	 * @return Execute result
	 * - 0 Set the preread line successfully
	 * - -1 The prread line count have reached to maximum limited
	 * - -2 The start line is over range
	 * - -3 The line count is over range
	 * - -4 Allocate memory fail
	 */
	int SetPrereadLine(UINT uStartLine, UINT uLineCount, MEM_TYPE MemType);
	/**
	* @brief Set VT operation mode of specific channel
	* @param[in] vecChannel The channel index
	* @param[in] dVTVoltValue The VT voltage value
	* @param[in] VTMode The VT operation mode
	* @return Execute result
	* - 0 Set VT mode successfully
	* - -1 The channel is over range
	* - -2 The VT value is over range
	* - -3 The VT mode is error or not supported
	*/
	int SetVTMode(const std::vector<USHORT>& vecChannel, double dVTVoltValue, VT_MODE VTMode = VT_MODE::CLOSE);
	/**
	* @brief Get the mode of specific channel
	* @param[in] usChannel The fail line number of last running
	* @param[out] VTMode The VT mode of the channel
	* @return Execute result
	* - 0 Get VT mode successfully
	* - -1 The channel is over range
	*/
	int GetVTMode(USHORT usChannel, VT_MODE& VTMode);
	/**
	 * @brief Get the point of CHardwareFunction
	 * @return The point of CHardwareFunction
	*/
	CHardwareFunction* GetHardwareFunction();
	/**
	 * @brief Set the line information whose wave data will be written in future
	 * @param[in] uStartLine The start line number
	 * @param[in] uLineCount The line number count
	 * @param[in] MemType The memory type
	 * @return Execute result
	 * - 0 Set the channel data information successfully
	 * - -1 The start line is over range
	 * - -2 The line count is over range
	 * - -3 Allocate memory fail
	*/
	int SetLineInfo(UINT uStartLine, UINT uLineCount, MEM_TYPE MemType);
	/**
	 * @brief Set the wave data of channel
	 * @param[in] vecChannel The channel number
	 * @param[in] pbyData The wave data data
	 * @return Execute result
	 * - 0 Set the channel data successfully
	 * - -1 Not set the channel data information
	 * - -2 The channel is over range
	 * - -3 The point of data is nullptr
	*/
	int SetWaveData(const std::vector<USHORT>& vecChannel, MEM_TYPE MemType, const BYTE* pbyData);
	/**
	 * @brief Write channel data to memory
	 * @return Execute result
	 * - 0 Set the wave data successfully
	 * - -1 Not set the channel data information
	*/
	int WriteData();
	/**
	* @brief Get the pin level of the specific channel
	* @param[in] usChannel The channel number whose pin level will be gotten
	* @param[in] LevelType The pin level type
	* @return Execute result
	*- 0 Get the pin level successfully
	* - 0x80000000 The channel is over range
	* - 0x80000001 The level type is error
	* - 0x80000002 Get pin level fail
	*/
	double GetPinLevel(USHORT usChannel, LEVEL_TYPE LevelType);
	/**
	 * @brief Set the channel which connect to the TMU unit
	 * @param[in] usChannel The channel number
	 * @param[in] byUnitIndex The unit index
	 * @return Execute result
	 * - 0 Set the channel successfully
	 * - -1 The channel number is over range
	 * - -2 The unit index is over range
	*/
	int SetTMUUnitChannel(USHORT usChannel, BYTE byUnitIndex);
	/**
	 * @brief Get the unit channel
	 * @param[in] usChannel The unit index
	 * @return The channel number
	 * - >=0 The channel connected to the TMU unit
	 * - -1 The channel is over range
	 * - -2 The channel is not connected to any unit
	*/
	int GetTMUConnectUnit(USHORT usChannel);
	/**
	 * @brief Set the TMU measurement parameter
	 * @param[in] vecChannel The channel number
	 * @param[in] bRaiseTriggerEdge Whether the trigger edge is raise
	 * @param[in] uHoldOffTime The hold off time, unit is ns
	 * @param[in] uHoldOffNum The hold off number
	 * @param[in] bySpecifedUnit Specified unit, -1 is not specifiled
	 * @return Execute result
	 * - 0 Set the parameter successfully
	 * - -1 The channel is over range
	 * - -2 The channel count is over range
	 * - -3 The unit specified is over range
	 * - -4 The channel is not connect to the TMU unit
	*/
	int SetTMUParam(const std::vector<USHORT>& vecChannel, BOOL bRaiseTriggerEdge, UINT uHoldOffTime, UINT uHoldOffNum, BYTE bySpecifiedUnit = -1);
	/**
	 * @brief Get the parameter of the TMU unit
	 * @param[in] byTMUUnitIndex The unit index
	 * @param[out] bRaiseTriggerEdge Whether the trigger edge is raise
	 * @param[out] usHoldOffTime The hold off time, unit is ns
	 * @param[out] usHoldOffNum The hold off number
	 * @return Execute result
	 * - 0 Set the parameter successfully
	 * - -1 The unit index is over range
	*/
	int GetTMUUnitParameter(BYTE byTMUUnitIndex, BOOL& bRaiseTriggerEdge, USHORT& usHoldOffTime, USHORT& usHoldOffNum);
	/**
	 * @brief Get the TMU parameter
	 * @param[in] usChannel The channel number
	 * @param[out] bRaiseTriggerEdge Whether the trigger edge is raise
	 * @param[out] usHoldOffTime The hold off time, unit is ns
	 * @param[out] usHoldOffNum The hold off number
	 * @return Execute result
	 * - 0 Set the parameter successfully
	 * - -1 The channel is over range
	 * - -2 The channel is not connect to the TMU unit
	*/
	int GetTMUParameter(USHORT usChannel, BOOL& bRaiseTriggerEdge, USHORT& usHoldOffTime, USHORT& usHoldOffNum);
	/**
	 * @brief Start TMU measurement
	 * @param[in] vecChannel The channel number
	 * @param[in] MeasMode The measurement mode
	 * @param[in] uSampleNum The sample number
	 * @param[in] dTimeout The timeout, the unit is ms
	 * @return Execute result
	 * - 0 Start TMU measurement successfully
	 * - -1 The channel is over range
	 * - -2 The channel is not connect to the TMU unit
	 * - -3 The measurement mode is not supported
	*/
	int TMUMeasure(const std::vector<USHORT>& vecChannel, TMU_MEAS_MODE MeasMode, UINT uSampleNum, double dTimeout);
	/**
	 * @brief Get the TMU measurement
	 * @param[out] usChannel The channel number
	 * @param[out] MeasMode The measurement mode
	 * @param[out] uSampleNum The sample number
	 * @param[out] dTimeout The timeout
	 * @return Execute result
	 * - 0 Ge the TMU measurement successfully
	 * - -1 The channel is over range
	 * - -2 The channel is not connected to TMU unit
	*/
	int GetTMUMeasure(USHORT usChannel, TMU_MEAS_MODE& MeasMode, UINT& uSampleNum, double& dTimeout);
	/**
	 * @brief Get the measure type
	 * @param[in] usChannel The channel number
	 * @param[in] MeasType The measurement type
	 * @param[out] nError The TMU measasure error
	 * - 0 No error occur
	 * - -1 The channel is over range
	 * - -2 The channel is not connect to any TMU unit
	 * - -3 The measurement type is not supported
	 * - -4 The measurement type is not measured before
	 * - -5 The measurement is not stop in timeout
	 * - -6 The TMU measurement is timeout
	 * - -7 The bind unit of measurement is not stop in timeout
	 * - -8 The bind unit is timeout
	 * - -9 The edge measurement error
	 * @return The measurement result
	 * - != TMU_ERROR The measurement result
	*/
	double GetTMUMeasureResult(USHORT usChannel, TMU_MEAS_TYPE MeasType, int& nErrorCode);
	/**
	 * @brief Set the trigger out channel
	 * @param[in] usChannel The channel number
	 * @return Execute result
	 * - 0 Set the trigger out channel successfully
	 * - -1 The channel is over range
	*/
	int SetTriggerOut(USHORT usChannel);
	/**
	 * @brief Check whether the fail memory is filled
	 * @param[out] bBRAMFilled Whether the fail memory of BRAM is filled
	 * @param[out] bDRAMFilled Whether the fail memory of DRAM is filled
	 * @return Execute result
	 * - 0 Get the fail memory filled successfully
	 * - -1 Not ran before
	 * - -2 Vector running
	*/
	int GetFailMemoryFilled(BOOL& bBRAMFilled, BOOL& bDRAMFilled);
	/**
	 * @brief Enable only save the fail information of selected line
	 * @param[in] bEnable Enable save select fail
	*/
	void EnableSaveSelectedFail(BOOL bEnable);
	/**
	 * @brief Get the fail synchronous type
	 * @return The fail synchronous type
	 * - 0 Support synchronize fail signal of first serval controllers
	 * - 1 Support synchronize fail signal in board without limited
	*/
	int GetFailSynType();
	/**
	 * @brief Set the fail synchronous
	 * @param[in] mapFailSyn The sychronous of each controller, key is controller and value is its fail syc source
	 * @return Execute result
	 * - 0 Set the fail sychronous successfully
	 * - -1 Not supported
	 * - -2 The controller is over range
	*/
	int SetFailSyn(const std::map<BYTE, BYTE>& mapFailSyn);
	/**
	 * @brief Get the instruction type
	 * @param[in] lpszInstruction The instruction sign
	 * @return The instruction type
	 * - 0 The normal single instruction
	 * - 1 The conditional instruction
	 * - 2 The parallel instruction
	 * - -1 The instruction is nullptr
	 * - -2 The instruction is not supported
	*/
	int GetInstructionType(const char* lpszInstruction);
	/**
	 * @brief Clear the preread vector
	*/
	void ClearPreread();
private:
	BYTE m_bySlotNo;///<The slot number of the board
	BYTE m_byIndex;///<The controller index
	CHardwareFunction m_HardwareFunction;///<The hardware function class of current controller
	CPattern m_Pattern;///<The pattern class
	CDriverAlarm* m_pAlarm;///<The point of driver alarm class
	STS_CALINFO m_CalibrationInfo[DCM_CHANNELS_PER_CONTROL];///<The calibration information
	BOOL m_bGetCalibraitonInfo;///<Whether get calibration information from flash
	std::vector<CChannelData*> m_vecPrereadData;///<The preread vector data
	CChannelData* m_pChannelData;///<The point of channel data
	std::vector<CHardwareFunction::DATA_RESULT> m_vecBRAMCapture;///<The capture data in BRAM, key is line number and the value is compare data of each channel
	std::vector<CHardwareFunction::DATA_RESULT> m_vecDRAMCapture;///<The capture data in DRAM, key is line number and the value is compare data of each channel
	std::vector<CHardwareFunction::DATA_RESULT> m_vecBRAMFailLine;///<The fail information in BRAM, key is line number and the value fail information of each channel
	std::vector<CHardwareFunction::DATA_RESULT> m_vecDRAMFailLine;///<The fail information in DRAM, key is line number and the value is fail information of each channel
	std::array<CChannelData*, 2> m_arrayDataWritten;///<The vector will be written, first element is the point of CChannelData for BRAM and second is point of CChannelData for DRAM
	BOOL m_bBRAMFilter;///<Whether BRAM fail line filter
	BOOL m_bDRAMFilter;///<Whether DRAM fail line filter
};

