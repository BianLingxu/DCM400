#pragma once
/**
 * @file Board.h
 * @brief Include the class of the board
 * @detail All function of board can be realize through this class
 * @author Guangyun Wang
 * @date 2020/05/31
 * @copyright AccoTEST Business Unit of Beijing Huafeng Test & Control Technology Co., Ltd.
*/
#include "Controller.h"
#include "Site.h"
#include "DriverAlarm.h"
#include "CalibrationInfo.h"
#include "ChannelClassify.h"
#include <set>
/**
 * @class CBoard
 * @brief The class of board
 */
class CBoard
{
public:
	/**
	 * @brief Constructor
	 * @param[in] bySlotNo The slot number in which the board inserted
	 * @param[in] pAlarm The point of the class CAlarm
	*/
	CBoard(BYTE bySlotNo, CDriverAlarm* pAlarm);
	/**
	 * @brief Destructor
	*/
	~CBoard();
	/**
	 * @brief Get FPGA version
	 * @param[in] byControllerIndex Get controller index
	 * @return The FPGA revision
	*/
	USHORT GetFPGARevision(BYTE byControllerIndex);
	/**
	 * @brief Get the FPGA revision of the board
	 * @return The FPGA revision
	*/
	USHORT GetFPGARevision();
	/**
	 * @brief Check whether the board is existed
	 * @return Whether the board is existed
	 * - TRUE The board is existed
	 * - FALSE The board is not existed
	 */
	BOOL IsExisted() const;
	/**
	 * @brief Get the controller count existed
	 * @return The controller count
	 */
	int GetControllerCount() const;
	/**
	 * @brief Get the existed controller number
	 * @param[out] vecController The controller number
	 */
	void GetExistController(std::vector<BYTE>& vecController) const;
	/**
	 * @brief Check whether the channel is existed
	 * @param[in] usChannel The channel number
	 * @return Whether channel is existed
	 * - 0 The channel is not existed
	 * - 1 The channel is existed
	 * - -1 The channel is over range
	*/
	int IsChannelExisted(USHORT usChannel);
	/**
	 * @brief Set the timeset delay
	 * @param[in] dTimesetDelay
	*/
	int SetTimesetDelay(BYTE byControllerIndex, double dTimesetDelay);
	/**
	 * @brief Get the timeset delay
	 * @return The timeset delay
	*/
	double GetTimesetDelay(BYTE byControllerIndex);
	/**
	 * @brief Set the total start delay
	 * @param[in] dDelay The total start delay
	 * @return Execute result
	 * - 0 Set the total start delay
	 * - -2 The total start delay is over range
	*/
	int SetTotalStartDelay(BYTE byControllerIndex, double dDelay);
	/**
	 * @brief Get the total start delay
	 * @return The total start delay
	*/
	double GetTotalStartDelay(BYTE byControllerIndex);
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
	 * - -2 The channel number is not existed
	 * - -3 The delay is over range
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
	 * @brief Save the delay information of each channel to flash
	 * @return Execute result
	 * - 0 Save the delay successfully
	 * - -1 Write to flash error
	 * - -2 No valid controller existed
	*/
	int SaveDelay();
	/**
	 * @brief Set the period of specific timeset
	 * @param[in] byTimesetIndex The timeset index whose period will be set
	 * @param[in] dPeriod The period of timeset
	 * @return Execute result
	 * - 0 Set period successfully
	 * - -1 The period is over range
	*/
	int SetPeriod(BYTE byTimesetIndex, double dPeriod);
	/**
	 * @brief Set the period of spcific timeset
	 * @param[in] vecChannel The channel whose timeset period will be set
	 * @param[in] byTimesetIndex The timeset index
	 * @param[in] dPeriod The period of timeset
	 * @return Execute result
	 * - 0 Set period successfully
	 * - -1 The period is over range
	*/
	int SetPeriod(const std::vector<USHORT>& vecChannel, BYTE byTimesetIndex, double dPeriod);
	/**
	 * @brief Get the period of specific timeset
	 * @param[in] byControllerIndex The controller index
	 * @param[in] byTimesetIndex The timeset index
	 * @return The period of specific timeset
	 * - >0 The timeset period
	 * - -1 The controller is over range
	 * - -2 The controller is not exited
	 * - -3 The timeset is over range
	*/
	double GetPeriod(BYTE byControllerIndex, BYTE byTimesetIndex);
	/**
	 * @brief Set the edge of timeset
	 * @param[in] vecChannel The channel whose edge will be set
	 * @param[in] byTimeset The timeset index
	 * @param[in] pdEdge The point of the Edge
	 * @param[in] WaveFormat The wave format type
	 * @param[in] IOFormat The IO format
	 * @param[in] CompareMode The compare mode
	 * @return Execute result
	 * - 0 Set edge successfully
	* - -1 The timeset index is over range
	* - -2 The format is error
	* - -3 The point of edge value is nullptr
	* - -4 The value is over range
	* - -5 No valid channel existed
	*/
	int SetEdge(std::vector<USHORT>& vecChannel, BYTE byTimeset, double* pdEdge, WAVE_FORMAT WaveFormat, IO_FORMAT IOFormat, COMPARE_MODE CompareMode);
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
	 * - -2 The channel is not existed
	 * - -3 The timeset is over range
	 * - -4 The point of edge is nullptr
	*/
	int GetEdge(USHORT usChannel, BYTE byTimesetIndex, double* pdEdge, WAVE_FORMAT& WaveFormat, IO_FORMAT& IOFormat, COMPARE_MODE& CompareMode);
	/**
	 * @brief Get the serial number of board
	 * @param[out] stsrSN The serial number
	 */
	void GetSN(std::string& strSN);
	/**
	* @brief Add channel pattern
	* @param[in] usChannel The channel number of the controller
	* @param[in] bBRAM The memory which to save the pattern
	* @param[in] uPatternLineIndex The line index in the memory
	* @param[in] cPattern The pattern sign
	* @param[in] byTimeset The timeset of current pattern line
	* @param[in] lpszCMD The command of pattern
	* @param[in] lpszParallelCMD The parallel command
	* @param[in] ulOperand The operand
	* @param[in] bCapture Whether the line is capture
	* @param[in] bSwitch Whether the pattern line is the last line in current memory,next line will switch to other memory
	* @return Execute result
	* - 0 Set pattern successfully
	* - -1 The channel is over range
	* - -2 The controller index is over range
	* - -3 The controller is not existed
	* - -4 The pattern index is over range
	* - -5 The pattern sign is not supported
	* - -6 The timeset is over range
	* - -7 The point of command is nullptr
	* - -8 The command is not supported
	* - -9 The operand is over range
	*/
	int SetVector(USHORT uChannel, BOOL bBRAM, UINT uPatternLine, char cPatternSign, BYTE byTimeset, const char* lpszCMD, const char* lpszParallelCMD, 
		USHORT usOperand, BOOL bCapture, BOOL bSwitch);
	/**
	 * @brief Load the vector to memory
	 * @return Execute result
	 * - 0 Load vector successfully
	 * - -1 Allocate memory fail
	 */
	int LoadVector();
	/**
	 * @brief Save the vector validity to board
	 * @param[in] Whether the vector is valid
	 */
	void SetVectorValid(BOOL bValid);
	/**
	 * @brief Check whether the vector is valid
	 * @return Whether the vector is valid
	 * - TRUE The vector is valid
	 * - FALSE The vector is invalid 
	 */
	BOOL IsVectorValid();
	 /**
	 * @brief Set the operand of the vector line
	 * @param[in] vecChannel The channel whose operand will be set
	 * @param[in] uBRAMLineNo The line number in BRAM
	 * @param[in] usOperand The operand of the value
	 * @param[in] bCheckRange Whether check the range of the operand
	 * @return Execute result
	 * - 0 Set operand successfully
	 * - -1 The line number is over range
	 * - -2 The operand is over range
	 * - -3 No valid channel
	 */
	int SetOperand(const std::vector<USHORT>& vecChannel, UINT uBRAMLineNo, USHORT usOperand, BOOL bCheckRange);
	/**
	 * @brief Set the instruction
	 * @param[in] vecChannel The channel whose instruction will be set
	 * @param[in] uBRAMLineNo The line number in BRAM
	 * @param[in] lpszInstruction The instruction
	 * @param[in] usOperand The operand
	 * @return Execute result
	 * - 0 Set the instruction successfully
	 * - -1 The BRAM line number is over range
	 * - -2 The instruction is nullptr
	 * - -3 The instruction is not supported
	 * - -4 The operand is over range
	 * - -5 No channel existed
	*/
	int SetInstruction(const std::vector<USHORT>& vecChannel, UINT uBRAMLineNo, const char* lpszInstruction, USHORT usOperand);
	/**
	 * @brief Set the start or stop line number of select fail
	 * @param[in] vecChannel The channel whose parallel instruction will be set
	 * @param[in] uRAMLineNo The RAM line number of BRAM or DRAM
	 * @param[in] bStartSave Whether start saving
	 * @param[in] bBRAM Whether the line number is BRAM
	 * @param[in] bClose Whether bDelete start or stop fail
	 * @return Execute result
	 * - 0 Set the saving select fail successfully
	 * - -1 The line number is over range
	 * - -2 No channel existed
	*/
	int SetSaveSelectFail(const std::vector<USHORT>& vecChannel, UINT uRAMLineNo, BOOL bStartSave, BOOL bBRAM, BOOL bDelete = FALSE);
	/**
	 * @brief Get the instruction of the line
	 * @param[in] byController The controller whose instruction will be gotten
	 * @param[in] uBRAMLineNo The line number of BRAM
	 * @param[out] lpszInstruction The buff for save instruction
	 * @param[in] nBuffSize The buff size
	 * @return Execute result
	 * - 0 Get the instruction successfully
	 * - -1 The controller index is over range
	 * - -2 The controller is not existed
	 * - -3 The line number is over range
	 * - -4 The point of the buff is nullptr
	 * - -5 The buff is too small
	*/
	int GetInstruction(BYTE byController, UINT uBRAMLineNo, char* lpszInstruction, int nBuffSize);
	/**
	 * @brief Get the operand of the line
	 * @param[in] byController The controller whose operand will be gotten
	 * @param[in] uBRAMLineNo The BRAM line number
	 * @return The operand 
	 * - >=0 The operand
	 * - -1 The controller index is over range
	 * - -2 The controller is not existed
	 * - -3 The line number is over range
	*/
	int GetOperand(BYTE byController, UINT uBRAMLineNo);
	/**
	 * @brief Connect or disconnect the function relay
	 * @param[in] vecChannel The channel whose relay will be connect or disconnect
	 * @param[in] bConnect Whether connect function relay
	 * @return Execute result
	 * - 0 Connect or disconnect function relay successfully
	 * - -1 The board is not existed
	 * - -2 The channel is over range
	*/
	int Connect(const std::vector<USHORT>& vecChannel, BOOL bConnect);
	/**
	* @brief Connect or disconnect the calibration relay
	* @param[in] usChannel The channel whose relay will be connect or disconnect
	* @param[in] bConnect Whether connect relay
	* @return Execute result
	* - 0 Connect or disconnect relay successfully
	* - -1 The channel is over range
	* - -2 The channel is not existed
	*/
	int SetCalibrationRelay(USHORT usChannel, BOOL bConnect = FALSE);
	/**
	 * @brief Get the connect channel
	 * @param[out] vecChannel The connected channel
	 * @return Execute result
	 * - 0 Get the connect channel successfully
	 * - -1 The relay type is error
	*/
	int GetConnectChannel(std::vector<USHORT>& vecChannel, RELAY_TYPE RelayType = RELAY_TYPE::FUNC_RELAY);
	/**
	 * @brief Initialize MCU
	 * @param[in] vecChannel The channel whose MCU will be initialized
	 * @return Execute result
	 * - 0 Initialize MCU successfully
	*/
	int InitMCU(const std::vector<USHORT>& vecChannel);
	/**
	 * @brief Initialize PMU function
	 *param[in] vecChannel The channel whose PMU will be initialized
	 * @return Execute result
	 * - 0 Initialize PMU successfully
	*/
	int InitPMU(const std::vector<USHORT>& vecChannel);
	/**
	 * @brief Set pin level
	 * @param[in] vecChannel The channel whose pin level will be set
	 * @param[in] dVIH The input voltage of logic high
	 * @param[in] dVIL The input voltage of logic low
	 * @param[in] dVOH The output voltage of logic high
	 * @param[in] dVOL The output voltage of logic low
	 * @return Execute result
	 * - 0 Set pin level successfully
	 * - -1 The pin level is over range
	*/
	int SetPinLevel(std::vector<USHORT>& vecChannel, double dVIH, double dVIL, double dVOH, double dVOL);
	/**
	* @brief Set the channel status
	* @param[in] vecChannel The channel number
	* @param[in] ChannelStatus The pin status
	* @return Execute result
	* - 0 Set channel status successfully
	* - -1 The channel status is not supported
	* - -2 No valid channel existed
	*/
	int SetChannelStatus(const std::vector<USHORT>& vecChannel, CHANNEL_OUTPUT_STATUS ChannelStatus = CHANNEL_OUTPUT_STATUS::HIGH_IMPEDANCE);
	/**
	 * @brief Get channel mode
	 * @param[in] usChannel The channel number
	 * @return The channel mode
	 * - 0 The channel is in MCU mode
	 * - 1 The channel is in PMU mode
	 * - 2 The channel is Neither MCU mode, nor PMU mode
	 * - -1 The channel number is over range
	 * - -2 The channel is not existed
	*/
	int GetChannelMode(USHORT usChannel);
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
	 * - -2 The channel is not existed
	*/
	int GetChannelStatus(USHORT usChannel);
	/**
	 * @brief Set vector run parameter
	 * @param[in] vecChannel The channel which will run vector
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
	int SetRunParam(std::vector<USHORT>& vecChannel, UINT uStartLineNumber, UINT uStopLineNumber, BOOL bWithDRAM = FALSE, UINT uDRAMStartLine = 0, BOOL bEnableStart = TRUE);
	/**
	 * @brief Run vector
	 * @return Execute result
	 * - 0 Run synchronously successfully
	 * - -1 The board is not existed
	*/
	int SynRun(USHORT usChannel);
	/**
	 * @brief Stop vector
	 * @param[in] vecChannel The channel whose vector will be stopped
	 * @return Execute result
	 * - 0 Initialize MCU successfully
	 * - -1 No channel existed
	*/
	int StopVector(std::vector<USHORT>& vecChannel);
	/**
	 * @brief Enable receive start signal
	 * @param[in] bEnable Whether enable start
	*/
	void EnableStart(std::vector<USHORT>& vecChannel, BOOL bEnable);
	/**
	 * @brief Wait the vector stop running
	 * @param[in] vecChannel The channel in the board
	 * @param[in] uTimeout The wait timeout is ms
	 * @return Execute result
	 * - 0 Vector stop
	 * - -1 Timeout
	*/
	int WaitStop(std::vector<USHORT>& vecChannel, UINT uTimeout = 160e3);
	/**
	 * @brief Get the MCU result
	 * @param[in] vecChannel The channel in current board
	 * @return MCU run result
	 * - 0 MCU run PASS
	 * - 1 MCU run FAIL
	 * - -1 Not ran vector before
	 * - -2 Vector running
	 * - -3 No valid channel
	*/
	int GetMCUResult(const std::vector<USHORT>& vecChannel);
	/**
	 * @brief Get MCU result of specific channel
	 * @param[in] usChannel The channel number
	 * @return MCU run result
	 * - 0 MCU run PASS
	 * - 1 MCU run FAIL
	 * - -1 The channel is over range
	 * - -2 The channel is not existed
	 * - -3 Not ran vector
	 * - -4 Vector running
	*/
	int GetChannelResult(USHORT usChannel);
	/**
	 * @brief Get the running status
	 * @param[in] usChannel The channel number
	 * @return Running status
	 * - 0 Running
	 * - 1 Stop running
	 * - 2 Not ran
	 * - -1 The channel is over range
	 * - -2 The channel is not existed
	*/
	int GetRunningStatus(USHORT usChannel);
	/**
	 * @brief Get fail count of the channel
	 * @param[in] usChannel The channel number
	 * @return Fail count
	 * - >=0 The fail count of the channel
	 * - -1 The channel is over range
	 * - -2 The channel is not existed
	 * - -3 Not ran before
	 * - -4 Vector running
	*/
	int GetFailCount(USHORT usChannel);
	/**
	 * @brief Get the fail count of the controller
	 * @param usChannel The channel number whose controller fail count will be gotten
	 * @return The fail count of current controller
	 * - >=0 The fail count of the channel
	 * - -1 The channel is over range
	 * - -2 The channel is not existed
	 * - -3 Not ran before
	 * - -4 Vector running
	*/
	int GetControllerFailCount(USHORT usChannel);
	/**
	* @brief Get the fail line number of last running
	* @param[in] usChannel The channel number
	* @param[in] uGetMaxFailCount The maximum fail count will be gotten
	* @param[out] vecLineNo The fail line number of last running
	* @param[in] bForceRefresh Whether force refresh fail line number
	* @return Execute result
	* - 0 Get the fail line number successfully
	 * - -1 The channel is over range
	 * - -2 The channel is not existed
	 * - -3 Not ran before
	 * - -4 Vector running
	*/
	int GetFailLineNo(USHORT usChannel, UINT uGetMaxFailCount, std::vector<int>& vecBRAMLineNo, std::vector<int>& vecDRAMLineNo, BOOL bForceRefresh = FALSE);
	/**
	 * @brief Check whether the controller's fail line number is handled
	 * @param[0] usChannel The channel number
	 * @return Whether fail line handled
	 * - 0 The fail line count is handled
	 * - -1 The channel number is over range
	 * - -2 The channel is not existed
	 * - -3 Not handle fail line
	*/
	int IsFailLineHandled(USHORT usChannel);
	/**
	 * @brief Get the MCU fail line number
	 * @param[in] usChannel The channel number
	 * @param[in] vecBRAMLineNo The BRAM fail line number of latest running
	 * @param[in] vecDRAMLineNo The DRAM fail line number of latest running
	 * @param[in] bForceRefresh Whether force refresh fail line number
	 * @return Execute result
	* - 0 Get the fail line number successfully
	 * - -1 The channel is over range
	 * - -2 The controller is not existed
	 * - -3 Not ran before
	 * - -4 Vector running
	*/
	int GetMCUFailLineNo(USHORT usChannel, std::vector<int>& vecBRAMLineNo, std::vector<int>& vecDRAMLineNo, BOOL bForceRefresh = FALSE);
	/**
	 * @brief Delete the fail line in controller
	 * @param[in] usChannel The channel whose controller's fail will be deleted
	 * @param[in] nBRAMDeleteCount The fail count will be deleted in BRAM
	 * @param[in] nDRAMDeleteCount The fail count will be deleted in DRAM
	 * @return Execute result
	 * - 0 Delete fail line number successfully
	 * - -1 The channel is over range
	 * - -2 The controller is not existed
	*/
	int DeleteControllerFailLine(USHORT usChannel, int nBRAMDeleteCount, int nDRAMDeleteCount);
	/**
	 * @brief Get the line number of the last certail result in latest ran
	 * @param[in] usChannel The channel number
	 * @param[out] nBRAMLineNo The last line number certail in BRAM
	 * @param[out] bBRAMLineFail The result of last certained line number in BRAM
	 * @param[out] nDRAMLineNo The last line number certained in DRAM
	 * @param[out] bDRAMLineFail The result of last certained line number in BRAM
	 * @return execute result
	 * - 0 Get the certain line successfully
	 * - -1 The channel number is over range
	 * - -2 The controller is not existed
	*/
	int GetLastCertainResultLineNo(USHORT usChannel, int& nBRAMLineNo, BOOL& bBRAMLineFail, int& nDRAMLineNo, BOOL& bDRAMLineFail);
	/**
	 * @brief Preload the fail count of the channel
	 * @param[in] vecChannel The channel number
	 * @param[in] uGetMaxFailCount The maximum fail count
	 * @return Execute result
	 * - 0 Preload the fail line number successfully
	 * - -1 Not ran before
	 * - -2 Vector running
	 * - -3 No valid board existed
	*/
	int PreloadFailLineNo(const std::vector<USHORT>& vecChannel, UINT uGetMaxFailCount);
	/**
	* @brief Get the capture of channel
	* @param[out] mapBRAM The capture data in BRAM
	* @param[out] mapDRAM The capture data in DRAM
	* @return Execute result
	* - 0 Get the capture successfully
	* - -1 The channel number is over range
	* - -2 The channel number is not existed
	* - -3 Vector not ran
	* - -4 Vector running
	*/
	int GetCapture(USHORT usChannel, std::vector<LINE_DATA>& vecBRAM, std::vector<LINE_DATA>& vecDRAM);
	/**
	 * @brief Get the stop line number
	 * @param[in] usChannel The channel number
	 * @return The stop line number
	 * - >=0 The stop line number
	 * - -1 Channel over range
	 * - -2 The channel is not existed
	 * - -3 Not ran before
	 * - -4 Vector running
	*/
	int GetStopLineNo(USHORT usChannel);
	/**
	 * @brief Get the running line count
	 * @param[out] ulLineCount The line count in latest ran
	 * @return Execute result
	 * - 0 Get the line count successfully
	 * - -1 The channel is over range
	 * - -2 The channel is not existed
	*/
	int GetRunLineCount(USHORT usChannel, ULONG& ulLineCount);
	/**
	 * @brief Set the calibration information
	 * @param[in] byControllerIndex The controller index of DCM
	 * @param[in] pCalInfo The calibration information of each channel
	 * @param[in] pbyChannelStatus Whether update the calibration information of each channel
	 * @param[in] nElementCount The element count of each array
	 * @return Execute result
	 * - 0 Set the calibration information successfully
	 * - -1 The controller index is over range
	 * - -2 The controller is not existed
	 * - -3 The point of array is nullptr
	 * - -4 The element count is less than the channel of controller
	 * - -5 Set the calibration information fail
	*/
	int SetCalibrationInfo(BYTE byControllerIndex, STS_CALINFO* pCalInfo, BYTE* pbyChannelStatus, int nElementCount);
	/**
	 * @brief Get the calibration information
	 * @param[in] byControllerIndex The controller index of DCM
	 * @param[out] pCalInfo The calibration information of each channel
	 * @param[in] nElementCount The element count of each array
	 * @return Execute result
	 * - -1 The controller index is over range
	 * - -2 The controller is not existed
	 * - -3 The point of array is nullptr
	 * - -4 The element count is less than the channel of controller
	 * - -5 Get the calibration information fail
	*/
	int GetCalibrationInfo(BYTE byControllerIndex, STS_CALINFO* pCalInfo, int nElementCount);
	/**
	 * @brief Get the calibration information of the channel
	 * @param[in] usChannel The channel number
	 * @param[out] CalibrationInfo The calibration information of the channel
	 * @return Execute result
	 * - 0 Get the calibration information successfully
	 * - -1 The channel is over range
	 * - -2 The channel is not existed
	 * - -3 Get the calibration information fail
	*/
	int GetCalibrationInfo(USHORT usChannel, STS_CALINFO& CalibrationInfo);
	/**
	 * @brief Set the calibration data of each channel
	 * @param[in] pCalibrationData The calibration data of each channel
	 * @param[in] nElement The element count of the array
	 * @return Execute result
	 * - 0  Get calibration data successfully
	 * - -1 The controller is not over range
	 * - -2 The controller is not existed
	 * - -3 The point of calibration data is nullptr
	 * - -4 The element count is not enough
	 * - -5 Flash status error
	 * - -6 Write to flash fail
	*/
	int SetCalibrationData(BYTE byControllerIndex, CAL_DATA* pCalibrationData, int nElementCount);
	/**
	 * @brief Reset the calibration data of all channels in controller
	 * @param[in] byControllerIndex The controller index
	 * @return Execute result
	 * - 0 Reset calibration data successfully
	 * - -1 The controller index is over range
	 * - -2 The controller is not existed
	 * - -3 Not set the memory of calibration data
	*/
	int ResetCalibrationData(BYTE byControllerIndex);
	/**
	 * @brief Get the calibration data
	 * @param[in] byControllerIndex The controller index
	 * @param[out] pCalibrationData The calibration data of each channel
	 * @param[in] nElementCount The element count of array
	 * @return Execute result
	 * - 0  Get calibration data successfully
	 * - -1 The controller is not over range
	 * - -2 The controller is not existed
	 * - -3 The point of calibration data is nullptr
	 * - -4 The element count is not enough
	 * - -5 Flash status error
	 * - -6 Data in flash is error
	*/
	int GetCalibrationData(BYTE byControllerIndex, CAL_DATA* pCalibrationData, int nElementCount);
	/**
	 * @brief Get the calibration data from flash
	 * @param[in] byControllerIndex The controller index
	 * @return Execute result
	 * - 0  Read calibration data successfully
	 * - -1 The controller is not over range
	 * - -2 The controller is not existed
	 * - -3 The flash status is error
	 * - -4 Allocate memory fail
	 * - -5 Data in flash is error
	*/
	int ReadCalibrationData(BYTE byControllerIndex);
	/**
	 * @brief Set the hardware information
	 * @param[in] pHardInfo The hardware information of the board
	 * @param[in] nModuleCount The module count of hardware information
	 * @return Execute result
	 * - 0 Set hardware information successfully
	 * - -1 The board is not existed
	 * - -2 The point of hardware information is nullptr
	 * - -3 Set hardware information fail
	*/
	int SetHardInfo(STS_HARDINFO* pHardInfo, int nModuleCount);
	/**
	 * @brief Get the hardware function
	 * @param[in] pHardInfo The hardware function
	 * @param[in] nElementCount The element count of array
	 * @return The module count in board
	 * - >=0 The module count of the board
	 * - -1 The board is not existed
	*/
	int GetHardInfo(STS_HARDINFO* pHardInfo, int nElementCount);
	/**
	 * @brief Get the channel count of board
	 * @param[in] bForceRead Whether force read channel count from flash
	 * @return The channel count
	 * - >0 The channel count
	 * - -1 The board is not existed
	 * - -2 Flash error
	 * - -3 Allocate memory fail
	 * - -4 The data in flash is error
	*/
	int GetChannelCount(BOOL bForceRead);
	/**
	 * @brief Set channel count of board
	 * @param[in] usChannelCount The channel count
	 * @return Execute result
	 * - 0 Set channel count successfully
	 * - -1 The board is not existed
	 * - -2 The channel count is over range
	 * - -3 Flash error
	 * - -4 Write channel count fail
	*/
	int SetChannelCount(USHORT usChannelCount);
	/**
	* @brief Set dynamic load mode of specific channel
	* @param[in] usChannel The channel index
	* @param[in] DynamicLoadMode The dynamic load mode
	* @param[in] dIOH The IOH value
	* @param[in] dIOL The IOL value
	* @param[in] dVTValue The VT voltage
	* @param[in] dClmapHigh The high clamp voltage
	* @param[in] dClampLow The low clamp voltage
	* @return Execute result
	* - 0 Set dynamic load mode successfully
	* - -1 No valid board existed
	* - -2 The output current is over range
	* - -3 The VT is over range
	* - -4 The Clamp is over range
	*/
	int SetDynamicLoad(const std::vector<USHORT>& vecChannel, BOOL bEnable, double dIOH, double dIOL, double dVTValue, double dClmapHigh, double dClampLow);
	/**
	* @brief Get the dynamic load
	* @param[in] usChannel The channel number
	* @param[out] DynamicLoadMode The dynamic load mode
	* @param[out] dIOH The IOH value
	* @param[out] dIOH The IOL value
	* @return Execute result
	* - 0 Get dynamic load successfully
	* - -1 The channel index is over range
	* - -2 The channel is not existed
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
	* - -1 No board existed
	*/
	int SetPMUMode(const std::vector<USHORT>& vecChannel, PMU_MODE PMUMode, PMU_IRANGE Range, double dSetValue, double dClmapHigh, double dClampLow);
	/**
	 * @brief Get the channel whose PMU is clamp
	 * @param[in] vecChannel The channels which will be check clamp status
	 * @param[out] mapClampChannel The clamp channel and its clamp type, key is channel and value is clamp type(0 is voltage clamp and 1 is current clamp)
	*/
	void GetPMUClampChannel(const std::vector<USHORT>& vecChannel, std::map<USHORT, UCHAR>& mapClampChannel);
	/**
	 * @brief Enable PMU clamp flag
	 * @param[in] usChannel The channel number
	 * @param[in] bEnable Whether enable clamp
	 * @return Execute result
	 * - 0 Set clamp successfully
	 * - -1 The channel number is over range
	 * - -2 The channel is not existed
	*/
	int EnablePMUClampFlag(USHORT usChannel, BOOL bEnable);
	/**
	 * @brief Enable all channels' PMU clamp flag
	*/
	void EnableAllPMUClampFlag();
	/**
	* @brief Start PMU multi-measure
	* @param[in] vecChannel The channel index
	* @param[in] nSampleTimes The sample times
	* @param[in] dSamplePeriod The sample period
	* @return Execute result
	* - 0 Measure successfully
	* - -1 No board existed
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
	 * - -1 No measurement result needed to wait
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
	* @brief Set VT operation mode of specific channel
	* @param[in] vecChannel The channel index
	* @param[in] dVTVoltValue The VT voltage value
	* @param[in] VTMode The VT operation mode
	* @return Execute result
	* - 0 Set VT mode successfully
	* - -1 The VT value is over range
	* - -2 The VT mode is error or not supported
	* - -3 No valid board existed
	*/
	int SetVTMode(const std::vector<USHORT>& vecChannel, double dVTVoltValue, VT_MODE VTMode = VT_MODE::CLOSE);
	/**
	* @brief Get the mode of specific channel
	* @param[in] usChannel The fail line number of last running
	* @param[out] VTMode The VT mode of the channel
	* @return Execute result
	* - 0 Get VT mode successfully
	* - -1 The channel is over range
	* - -2 The channel is not existed
	*/
	int GetVTMode(USHORT usChannel, VT_MODE& VTMode);
	/**
	 * @brief Set the preread vector line
	 * @param[in] vecChannel The channel number
	 * @param[in] uStartLine The start line of vector
	 * @param[in] The line count
	 * @param[in] The memory type of the line
	 * @return Execute result
	 * - 0 Set the preread line successfully
	 * - -1 The preread line count have reached to maximum limited
	 * - -2 The start line is over range
	 * - -3 The line count is over range
	 * - -5 Allocate memory fail
	 * - -6 No valid controller existed
	 */
	int SetPrereadLine(const std::vector<USHORT>& vecChannel, UINT uStartLine, UINT uLineCount, MEM_TYPE MemType);
	/**
	 * @brief Set the data line information whose wave data will be written in future
	 * @param[in] vecChannel The channel number
	 * @param[in] uStartLine The start line number
	 * @param[in] uLineCount The line number count
	 * @param[in] MemType The memory type
	 * @return Execute result
	 * - 0 Set the channel data information successfully
	 * - -1 The start line number is over range
	 * - -2 The line count is over range
	 * - -3 Allocate memory fail
	 * - -4 No valid controller existed
	*/
	int SetLineInfo(const std::vector<USHORT>& vecChannel, UINT uStartLine, UINT uLineCount, MEM_TYPE MemType);
	/**
	 * @brief Set the channel data
	 * @param[in] vecChannel The channel number
	 * @param[in] pbyData The wave data data
	 * @return Execute result
	 * - 0 Set the channel data successfully
	 * - -1 Not set the channel line information
	 * - -2 The point of data is nullptr
	 * - -3 No valid controller existed
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
	 * @brief Get the pattern of controller
	 * @param[in] byControllerIndex The controller index
	 * @param[in] bBRAM The memory type in which the vector saved, TRUE is BRAM
	 * @param[in] uStartLine The start line number
	 * @param[in] uLineCount The line count will be gotten
	 * @param[out] lpszPattern The pattern read
	 * @return Execute result
	 * - 0 Get the vector successfully
	 * - -1 The controller index is over range
	 * - -2 The controller is not existed
	 * - -3 The start line is over range
	 * - -4 The line count is over range
	 * - -5 The point of pattern is nullptr
	 * - -6 Allocate memory fail
	*/
	int GetVector(BYTE byControllerIndex, BOOL bBRAM, UINT uStartLine, UINT uLineCount, char(*lpszPattern)[17]);
	/**
	 * @brief Read the data in memory
	 * @param[in] byControllerIndex The controller index
	 * @param[in] bBRAM The memory type
	 * @param[in] DataType The data type will be read
	 * @param[in] uStartLine The base pattern start line of data read
	 * @param[in] uLineCount The pattern data line count read
	 * @param[out] pusData The data buff which will save the data read
	 * @return Execute result
	* - 0 Read memory successfully
	* - -1 The controller index is over range
	* - -2 The controller is not existed
	* - -3 The data type is not supported
	* - -4 The start line is over range;
	* - -5 The data count is over range
	* - -6 The line count read is 0 or the data buff is nullptr
	*/
	int GetMemory(BYTE byControllerIndex, BOOL bBRAM, DATA_TYPE DataType, UINT uStartLine, UINT uLineCount, USHORT* pusData);
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
	 * - -2 The channel is not existed
	 * - -3 The data type is not supported
	 * - -4 Allocate memory fail
	 * - -5 The start line is over range
	 * - -6 The line count is over range
	 * - -7 The line count is 0
	 * - -8 The point is data is nullptr
	*/
	int SetMemory(USHORT usChannel, BOOL bRAM, DATA_TYPE DataType, UINT uStartLine, UINT uLineCount, BYTE* pbyData);
	/**
	 * @brief Get the pattern of specific channel
	 * @param[in] usChannel The channel number
	 * @param[in] bBRAM Whether the pattern saved in BRAM
	 * @param[in] uStartLine The start line number
	 * @param[in] uLineCount The line count
	 * @param[in] lpszPattern The pattern gotten
	 * @return Execute result
	 * - 0 Get the pattern successfully
	 * - -1 The channel number is over range
	 * - -2 The channel is not existed
	 * - -3 Allocate memory fail
	 * - -4 The start line is over range
	 * - -5 The line count is over range
	 * - -7 The point of pattern is nullptr
	*/
	int GetVector(USHORT usChannel, BOOL bBRAM, UINT uStartLine, UINT uLineCount, char*lpszPattern);
	/**
	* @brief Get the pin level of the specific channel
	* @param[in] usChannel The channel number whose pin level will be gotten
	* @param[in] LevelType The pin level type
	* @return Execute result
	*- 0 Get the pin level successfully
	* - 1e16 The channel is over range or The level type is error or Get pin level fail
	*/
	double GetPinLevel(USHORT usChannel, LEVEL_TYPE LevelType);
	/**
     * @brief Set the channel which connect to the TMU unit
     * @param[in] vecChannel The channel number
     * @param[in] byUnitIndex The unit index
     * @return Execute result
     * - 0 Set the channel successfully
     * - -1 More than one channel connect to TMU unit
	 * - -2 The unit index is over range
     * - -4 No valid board existed
    */
	int SetTMUUnitChannel(std::vector<USHORT> vecChannel, BYTE byUnitIndex);
	/**
	 * @brief Get the unit channel
	 * @param[in] usChannel The channel number
	 * @return The TMU unit number connected to the channel
	 * - >=0 The TMU unit number connected to the channel
	 * - -1 The channel number is over range
	 * - -2 The channel is not existed
	 * - -3 The channel is not connected to any unit
	*/
	int GetTMUConnectUnit(USHORT usChannel);
	/**
	 * @brief Set the TMU measurement parameter
	 * @param[in] vecChannel The channel number
	 * @param[in] bRaiseTriggerEdge Whether the trigger edge is raise
	 * @param[in] uHoldOffTime The hold off time, unit is ns
	 * @param[in] uHolfOffNum The hold off number
	 * @param[in] bySpecifedUnit Specified unit, -1 is not specifiled
	 * @return Execute result
	 * - 0 Set the parameter successfully
	 * - -1 The unit specified is over range
	 * - -2 The channel count is over range
	 * - -3 The channel is not connect to the TMU unit
	 * - -4 No valid board existed
	*/
	int SetTMUParam(const std::vector<USHORT>& vecChannel, BOOL bRaiseTriggerEdge, UINT uHoldOffTime, UINT uHolfOffNum, BYTE bySpecifiedUnit = -1);
	/**
	 * @brief Get the TMU parameter
	 * @param[in] byControllerIndex The controller index
	 * @param[in] byTMUUnitIndex The TMU unit index
	 * @param[out] bRaiseTriggerEdge Whether the trigger edge is raise
	 * @param[out] usHoldOffTime The hold off time, unit is ns
	 * @param[out] usHoldOffNum The hold off number
	 * @return Execute result
	 * - 0 Set the parameter successfully
	 * - -1 The controller index is over range
	 * - -2 The controller is not existed
	 * - -3 The TMU unit is over range
	*/
	int GetTMUUnitParameter(BYTE byControllerIndex, BYTE byTMUUnitIndex, BOOL& bRaiseTriggerEdge, USHORT& usHoldOffTime, USHORT& usHoldOffNum);
	/**
	 * @brief Get the TMU parameter
	 * @param[in] usChannel The channel number
	 * @param[out] bRaiseTriggerEdge Whether the trigger edge is raise
	 * @param[out] usHoldOffTime The hold off time, unit is ns
	 * @param[out] usHoldOffNum The hold off number
	 * @return Execute result
	 * - 0 Set the parameter successfully
	 * - -1 The channel is over range
	 * - -2 The channel is not existed
	 * - -3 The channel is not connect to the TMU unit
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
	 * - -1 The channel is not connect to the TMU unit
	 * - -2 The measurement mode is not supported
	 * - -3 No valid board existed
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
	 * - -2 The channel is not existed
	 * - -3 The channel is not connect to the TMU unit
	*/
	int GetTMUMeasure(USHORT usChannel, TMU_MEAS_MODE& MeasMode, UINT& uSampleNum, double& dTimeout);
	/**
	 * @brief Get the measure type
	 * @param[in] usChannel The channel number
	 * @param[in] MeasType The measurement type
	 * @param[out] nErrorCode The TMU measasure error
	 * - 0 No error occur
	 * - -1 The channel is over range
	 * - -2 The channel is not existed
	 * - -3 The channel is not connect to any TMU unit
	 * - -4 The measurement type is not supported
	 * - -5 The measurement type is not measured before
	 * - -6 The measurement is not stop in timeout
	 * - -7 The TMU measurement is timeout
	 * - -8 The bind unit of measurement is not stop in timeout
	 * - -9 The bind unit is timeout
	 * - -10 The edge measurement error
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
	 * - -2 The controller is not existed
	*/
	int SetTriggerOut(USHORT usChannel);
	/**
	 * @brief Set the fail synchronized channel
	 * @param vecFailSynChannel The channel need to be synchronized
	 * @return Execute result
	 * - 0 Set fail synchronized successfully
	 * - -1 Not supported
	 * - -2 The fail synchronous is conflict
	*/
	int SetFailSyn(const std::vector<CChannelGroup>& vecFailSynChannel, int& nConfictIndex);
	/**
	 * @brief Check whether the fail memory is filled
	 * @param[out] bBRAMFilled Whether the fail memory of BRAM is filled
	 * @param[out] bDRAMFilled Whether the fail memory of DRAM is filled
	 * @return Execute result
	 * - 0 Get the fail memory filled successfully
	 * - -1 The channel number is over range
	 * - -2 The channel is not existed
	 * - -3 Not ran before
	 * - -4 Vector running
	*/
	int GetFailMemoryFilled(USHORT usChannel, BOOL& bBRAMFilled, BOOL& bDRAMFilled);
	/**
	 * @brief Delete unused controller
	 * @param[in] vecChannelLeft The channels whose controller will not be deleted
	*/
	void DeleteController(const std::vector<USHORT>& vecChannelLeft);
	/**
	 * @brief Clear the preread vector
	*/
	void ClearPreread();
	/**
	 * @brief Enable only save the fail information of selected line
	 * @param[in] bEnable Enable save select fail
	 * @return Execute result
	 * - 0 Enable save selected fail successfully
	 * - -1 No board existed
	*/
	int EnableSaveSelectedFail(const std::vector<USHORT>& vecChannel, BOOL bEnable);
	/**
	 * @brief Check whether saving selected fail
	 * @param usChannel The channel number
	 * @return Whether saving selected fail
	 * - 0 Not saving selected fail
	 * - 1 Saving selected fail
	 * - -1 The channel is over range
	 * - -2 The channel is not existed
	*/
	BOOL IsSavingSelectedFail(USHORT usChannel) const;
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
private:
	/**
	 * @brief Initialize controller
	*/
	void InitController();
	/**
	 * @brief Set the delay information of each channel to the data in flash
	*/
	void SetDelay();
	/**
	 * @brief Wait us
	*/
	void Wait(UINT uUs);
	/**
	 * @brief Delete memory of map parameter whose value in heap
	 * @param[in] mapParam The map parameter
	*/
	template <typename Key, typename Value>
	void DelteMemory(std::map<Key, Value>& mapParam);
	/**
	 * @brief Get the integer number of double in round
	 * @param[in] dValue The double value
	 * @return The integer number
	*/
	inline int GetNumber(double dValue);
private:
	BYTE m_bySlotNo;///<The slot number
	BOOL m_bExisted;///<Whether the board is existed
	USHORT m_usChannelCount;///<The channel count of the board
	std::map<BYTE, CController*> m_mapController;///<The controller class, key is Controller index, value is the point of the class
	CDriverAlarm* m_pAlarm;///<The point to alarm class
	CHardwareFunction* m_pHardwareFunction;///<The point of the CHardwareFunction
	CBoardChannelClassify m_ChannelClassifiy;///<The channel classify
	std::vector<USHORT> m_vecModifiedChannel;///<The modify channel
	std::set<BYTE> m_setFailLineHandled;///<The controller whose MCU result have be handled	
	std::set<BYTE> m_setWaitPMUStartControlller;///<The controller whose PMU needed to be start
	std::set<BYTE> m_setFailSavingSelected;///<Whether the controller saving selected fail
	std::set<BYTE> m_setLatestFailSavingSelected;///<Whether the controller saving selected fail in latest ran
};
