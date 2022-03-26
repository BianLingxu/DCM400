#pragma once
/**
 * @file DCM.h
 * @brief Include the function class of CDCM
 * @author Guangyun Wang
 * @date 2020/05/12
 * @copyright AccoTEST Business Unit of Beijing Huafeng Test & Control Technology Co., Ltd.
*/
#include "Board.h"
#include "VectorInfo.h"
#include "DriverAlarm.h"
#include "PinGroup.h"
#include "AccoTESTGlobal.h"
#include "ChannelClassify.h"
#include <set>
#define ALL_SITE 0xFFFF
/**
 * @class CDCM
 * @brief This class provide all function of DCM, include digit function/PMU and TMU
*/
class CDCM
{
public:
	/**
	 * @enum MODULE_INFO
	 * @brief The module information type
	*/
	enum class MODULE_INFO
	{
		MODULE_NAME = 0,///<The module name
		MODULE_SN,///<The module SN
		MODULE_HDREV,///<The hardware revision
	};
	/**
	 * @brief Constructor
	 * @param[in] pDriverAlaram The point of the driver alarm
	*/
	CDCM(CDriverAlarm* pDriverAlarm);
	/**
	 * @brief Destructor
	*/
	~CDCM();
	/**
	 * @brief Get alarm class
	 * @return The point of the alarm
	*/
	CDriverAlarm* GetAlarm();
	/**
	 * @brief Add DCM board
	 * @param[in] bySlotNo The slot number of new board
	 * @return Execute result
	 * - 0 Add board successfully
	 * - -1 The board have been added before
	*/
	int AddBoard(BYTE bySlotNo, BOOL bOnlyAddBoard);
	/**
	 * @brief Get the board count
	 * @return The board count
	*/
	int GetBoardcount();
	/**
	 * @brief Get FPGA revision of controller
	 * @param[in] bySlotNo The slot number of the board
	 * @param[in] byControllerIndex The controller index
	 * @return FPGA revision
	 * - != 0xFFFFFFFF FPGA version
	 * - 0xFFFFFFFF The board is not existed or the controller index is over range
	*/
	USHORT GetFPGARevision(BYTE bySlotNo, BYTE byControllerIndex);
	/**
	 * @brief Get FPGA revision of board
	 * @param[in] bySlotNo The slot number of the board
	 * @return FPGA revision
	 * - != 0xFFFF FPGA version
	 * - 0xFFFF The board is not existed or the controller index is over range
	*/
	USHORT GetFPGARevision(BYTE bySlotNo);
	/**
	 * @brief Set the timeset delay
	 * @param[in] bySlotNo The slot number of the board
	 * @param[in] byControllerIndex The controller index
	 * @param[in] dDelay The timeset delay
	 * @return Execute result
	 * - 0 Set the total start delay
	 * - -1 The board is not existed
	 * - -2 The controller index is over range
	 * - -3 The controller is not existed
	 * - -4 The timeset delay is over range
	*/
	int SetTimesetDelay(BYTE bySlotNo, BYTE byControllerIndex, double dDelay);
	/**
	 * @brief Get the timeset delay
	 * @param[in] bySlotNo The slot number of the board
	 * @param[in] byControllerIndex The controller index
	 * @return The timeset delay
	*/
	double GetTimesetDelay(BYTE bySlotNo, BYTE byControllerIndex);
	/**
	 * @brief Set the total start delay
	 * @param[in] bySlotNo The slot number of the board
	 * @param[in] byControllerIndex The controller index
	 * @param[in] dDelay The total start delay
	 * @return Execute result
	 * - 0 Set the total start delay
	 * - -1 The board is not existed
	 * - -2 The controller index is over range
	 * - -3 The controller is not existed
	 * - -4 The timeset delay is over range
	*/
	int SetTotalStartDelay(BYTE bySlotNo, BYTE byControllerIndex, double dDelay);
	/**
	 * @brief Get the total start delay
	 * @param[in] bySlotNo The slot number of the board
	 * @return The total start delay
	*/
	double GetTotalStartDelay(BYTE bySlotNo, BYTE byControllerIndex);
	/**
	 * @brief Set the IO delay of channel
	 * @param[in] bySlotNo The slot number of the board
	 * @param[in] usChannel The channel number
	 * @param[in] dData The data delay
	 * @param[in] dDataEn The data enable delay
	 * @param[in] dHigh The high delay
	 * @param[in] dLow The low delay
	 * @return Execute result
	 * - 0 Set the IO delay successfully
	 * - -1 The board is not existed
	 * - -2 The channel is over range
	 * - -3 The channel number is not existed
	 * - -4 The delay is over range
	*/
	int SetIODelay(BYTE bySlotNo, USHORT usChannel, double dData, double dDataEn, double dHigh, double dLow);
	/**
	 * @brief Get the IO delay of channel
	 * @param[in] bySlotNo The slot number of the board
	 * @param[in] usChannel The channel number
	 * @param[out] pdData The data delay
	 * @param[out] pdDataEn The data enable delay
	 * @param[out] pdHigh The high delay
	 * @param[out] pdLow The low delay
	 * @return Execute result
	 * - 0 Get the delay successfully
	 * - -1 The board is not existed
	 * - -2 The channel number is over range
	 * - -3 The channel is not existed
	 * - -4 The point of delay is nullptr
	*/
	int GetIODelay(BYTE bySlotNo, USHORT usChannel, double* pdData, double* pdDataEn, double* pdHigh, double* pdLow);
	/**
	 * @brief Save the delay into flash
	 * @param[in] bySlotNo The slot number to which the board inserted
	 * @return Execute result
	 * - 0 Save the delay information successfully
	 * - -1 The board is not existed
	*/
	int SaveDelay(BYTE bySlotNo);
	/**
	 * @brief Get the channel count in board
	 * @param[in] bySlotNo The slot number of board
	 * @param[in] bForceRrefresh Whether refresh channel count
	 * @return Channel count
	 * - >=0 The channel count
	 * - -1 The board is not existed
	 * - -2 Flash error
	 * - -3 Allocate memory fail
	 * - -4 The data in flash is error
	*/
	int GetChannelCount(BYTE bySlotNo, BOOL bForceRrefresh);
	/**
	 * @brief Set the channel count of board
	 * @param[in] bySlotNo The slot number of board
	 * @param[in] usChannelCount The channel count
	 * @return Execute result
	 * - 0 Set channel count successfully
	 * - -1 The board is not existed
	 * - -2 The channel count is over range
	 * - -3 The flash is error
	 * - -4 Write channel count fail
	*/
	int SetChannelCount(BYTE bySlotNo, USHORT usChannelCount);
	/**
	 * @brief Set the calibration data to board
	 * @param[in] bySlotNo The slot number of board
	 * @param[in] byControllerIndex The controller index
	 * @param[in] pCalData The calibration data of each channel
	 * @param[in] byElementCount The element count of array
	 * @return Execute result
	 * - 0 Set calibration data successfully
	 * - -1 The board is not existed
	 * - -2 The controller index is over range
	 * - -3 The controller index is not existed
	 * - -4 The point of calibration data is nullptr
	 * - -5 The element count is less than channel count
	 * - -6 Set calibration fail
	*/
	int SetCalibrationData(BYTE bySlotNo, BYTE byControllerIndex, CAL_DATA* pCalData, BYTE byElementCount);
	/**
	 * @brief Reset the calibration data of all channels in controller
	 * @param[in] bySlotNo The slot number
	 * @param[in] byControllerIndex The controller index
	 * @return Execute result
	 * - 0 Reset the calibration data successfully
	 * - -1 The board is not existed
	 * - -2 The controller index is over range
	 * - -3 The controller is not existed
	 * - -4 Not set calibration memory
	*/
	int ResetCalibrationData(BYTE bySlotNo, BYTE byControllerIndex);
	/**
	 * @brief Get calibration data
	 * @param[in] bySlotNo The slot number of board
	 * @param[in] byControllerIndex The controller index
	 * @param[out] pCalData The calibration data of each channel
	 * @param[in] byElementCount The element count of array
	 * @return Execute result
	 * - 0 Get calibration data successfully
	 * - -1 The board is not existed
	 * - -2 The controller index is over range
	 * - -3 The controller is not existed
	 * - -4 The point of calibration data is nullptr
	 * - -5 The element count is not enough
	 * - -6 Flash status error
	 * - -7 Data in flash is error
	*/
	int GetCalibrationData(BYTE bySlotNo, BYTE byControllerIndex, CAL_DATA* pCalData, BYTE byElementCount);
	/**
	 * @brief Read calibration data from controller
	 * @param[in] bySlotNo The slot number of the board
	 * @param[in] byControllIndex The controller index
	 * @return Execute result
	 * - 0 Get calibration data successfully
	 * - -1 The board is not existed
	 * - -2 The controller index is over range
	 * - -3 The controller is not existed
	 * - -4 Flash status error
	 * - -5 Allocate memory fail
	 * - -6 Data in flash is error
	*/
	int ReadCalibrationData(BYTE bySlotNo, BYTE byControllIndex);
	/**
	* @brief Set the hardware information
	* @param[in] bySlotNo The slot number of board
	* @param[in] pHardInfo The hardware information of the board
	* @param[in] nModuleCount The module count of hardware information
	* @return Execute result
	* - 0 Set hardware information successfully
	* - -1 The board is not existed
	* - -2 The point of hardware information is nullptr
	* - -3 Set hardware information fail
	*/
	int SetHardInfo(BYTE bySlotNo, STS_HARDINFO* pHardInfo, int nModuleCount);
	/**
	 * @brief Get the hardware function
	 * @param[in] pHardInfo The hardware function
	 * @param[in] nElementCount The element count of array
	 * @return The module count in board
	 * - >= 0 The module count in board
	 * - -1 The board is not existed
	*/
	int GetHardInfo(BYTE bySlotNo, STS_HARDINFO* pHardInfo, int nElementCount);
	/**
	 * @brief Get the module information
	 * @param[in] lpszPinName The pin name
	 * @param[in] usSiteNo The site number
	 * @param[out] lpszInfo The module information
	 * @param[in] nInfoSize The size of lpszInfo
	 * @param[in] Module The module type
	 * @return Execute result
	 * - 0 Get the module information successfully
	 * - -1 Not load vector before
	 * - -2 The point of pin name is nullptr
	 * - -3 The pin is not defined
	 * - -4 The site is over range
	 * - -5 The site is invalid
	 * - -6 The pin is not belongs to
	 * - -7 Board not existed
	 * - -8 The point of information is nullptr
	*/
	int GetModuleInfo(const char* lpszPinName, USHORT usSiteNo, char* lpszInfo, int nInfoSize, MODULE_INFO SelInfo, STS_BOARD_MODULE Module = STS_MOTHER_BOARD);
	/**
	 * @brief Get the module information
	 * @param[in] bySlotNo The slot number in which the board inserted
	 * @param[out] lpszInfo The module information
	 * @param[in] nInfoSize The size of lpszInfo
	 * @param[in] Module The module type
	 * @return Execute result
	 * - 0 Get the module information successfully
	 * - -1 The board is not existed
	 * - -2 The point of information is nullptr
	*/
	int GetModuleInfo(BYTE bySlotNo, char* lpszInfo, int nInfoSize, MODULE_INFO SelInfo, STS_BOARD_MODULE Module = STS_MOTHER_BOARD);
	/**
	 * @brief Set the progress function
	*/
	void SetProgressFunc(STS_PROGRESS_INFO_FUN& pInfo, STS_PROGRESS_FUN& pSetpFun);
	/**
	 * @brief Load vector from file
	 * @param[in] lpszFileName The full file name of vector file
	 * @param[in] bReload Whether reload vector
	 * @return Execute result
	 * - 0 Load vector successfully
	 * - -1 No board existed
	 * - -2 The point of vector file path is nullptr
	 * - -3 Can't find vector editor module
	 * - -4 The vector file is not existed
	 * - -5 The format of the vector file is wrong
	 * - -6 Allocate memory fail
	 * - -7 Load vector fail
	 * - -8 No valid channel information
	 * - -9 The timeset is error
	 * - -10 The vector line is error
	*/
	int LoadVectorFile(const char* lpszFileName, BOOL bReload = TRUE);
	/**
	 * @brief Delete vector information file
	 * @param[in] lpszFileName The vector file name
	*/
	void DelteVectorInfoFile(const char* lpszFileName);
	/**
	 * @brief Copy vector information to other instance
	 * @param[in] DCM The target instance
	*/
	void CopyVectorInfo(const CDCM& DCM);
	/**
	 * @brief Get pin group information from file
	*/
	void LoadPinGroupInfo();
	/**
	 * @brief Get whether the vector used is shared
	 * @return Whether the vector shared
	*/
	BOOL IsVectorShared();
	/**
	 * @brief Load the vector information
	 * @param[in] lpszFileName The vector file name without path and extension
	 * @return Execute result
	 * - 0 Load vector information successfully
	 * - -1 The format of vector information file is error
	 * - -2 The vector file is error
	 * - -3 The vector in board is invalid
	*/
	int LoadVectorInfo(const char* lpszFileName);
	/**
	 * @brief Add pin group
	 * @param[in] lpszPinGroupName The name of pin group
	 * @param[in] lpszPinNameString The pin name used in pin group
	 * @return Execute result
	 * - 0 Add pin group successfully
	 * - -1 Not load vector before
	 * - -2 No board inserted
	 * - -3 The point of pin group name or pin name string is nullptr
	 * - -4 The pin group name is blank
	 * - -5 No pin in pin name string
	 * - -6 The format of pin name string is wrong
	 * - -7 Some pin in pin name string is not define in vector
	 * - -9 Some pin are not belongs to
	 * - -9 The pin group name has been used before
	*/
	int SetPinGroup(const char* lpszPinGroupName, const char* lpszPinNameList);
	/**
	 * @brief Set period
	 * @param[in] lpszTimeset The timeset name whose period will be set
	 * @param[in] dPeriod The period, unit is ns
	 * @return Execute result
	 * - 0 Set period successfully
	 * - -1 Not load vector before
	 * - -2 No board existed
	 * - -3 The point of timeset name is nullptr
	 * - -4 The timeset name is not defined in vector
	 * - -5 The period is over range
	*/
	int SetPeriod(const char* lpszTimeset, double dPeriod);
	/**
	 * @brief Get the period
	 * @param[in] bySlotNo The slot number
	 * @param[in] byControllerIndex The controller index
	 * @param[in] byTimesetIndex The byTimeset index
	 * @return The period of timeset
	 * - >0 The period of timeset
	 * - -1 The board is not existed
	 * - -2 The controller is over range
	 * - -3 The controller is not existed
	 * - -4 The timeset is over range
	*/
	double GetPeriod(BYTE bySlotNo, BYTE byControllerIndex, BYTE byTimesetIndex);
	/**
	 * @brief Get the period of timeset
	 * @param[in] bySlotNo The slot number
	 * @param[in] byControllerIndex The controller index
	 * @param[in] lpszTimesetName The timsetName
	 * @return The period of timeset
	 * - >0 The period of timeset
	 * - -1 The timeset name is not existed
	 * - -2 The board is not existed
	 * - -3 The controller is over range
	 * - -4 The controller is not existed
	*/
	double GetPeriod(BYTE bySlotNo, BYTE byControllerIndex, const char* lpszTimesetName);
	/**
	 * @brief Set the calibration relay
	 * @param[in] bySlotNo The slot number
	 * @param[in] usChannel The channel number
	 * @param[in] bConnect Whether connect the relay
	 * @return Execute result
	 * - 0 Set the relay successfully
	 * - -1 The board is not existed
	 * - -2 The channel is over range
	 * - -3 The channel is not existed
	*/
	int SetCalibartionRelay(BYTE bySlotNo, USHORT usChannel, BOOL bConnect);
	/**
	 * @brief Connect or disconnect the function relay
	 * @param[in] lpszPinGroup The pin group name in which the channel relay will be connect or disconnect
	 * @param[in] bConnect The connect type, TRUE is connect,False is disconnect
	 * @return Execute result
	 * - 0 The function relay has been connect or disconnect successfully
	 * - -1 Not load vector before
	 * - -2 The point of pin group is nullptr
	 * - -3 The pin group is not defined before
	 * - -4 No valid site
	 * - -5 The pin name is not belongs to
	 * - -6 No valid board existed
	*/
	int Connect(const char* lpszPinGroup, BOOL bConenct);
	/**
	 * @brief Get the connect channel
	 * @param[out] vecChannel The connected channel 
	 * @param[in] RelayType The relay type
	 * @return Execute result
	 * - 0 Get the connect channel
	 * - -1 The board is not existed
	 * - -2 The relay type is error
	*/
	int GetConnectChannel(BYTE bySlotNo, std::vector<USHORT>& vecChannel, RELAY_TYPE RelayType = RELAY_TYPE::FUNC_RELAY);	
	/**
	 * @brief Disconnect all function relay
	 * @param[in] bySlotNo The slot number
	 * @return Execute result
	 * - 0 Disconnect function relay successfully
	 * - -1 The board is not existed
	*/
	int IntializeFunctionRelay(BYTE bySlotNo);
	/**
	 * @brief Initialize MCU
	 * @param[in] lpszPinGroup The pin group name in which the channels' MCU will be initialized
	 * @return Execute result
	 * - 0 Initialize MCU successfully
	 * - -1 Not load vector before
	 * - -2 The point of pin group is nullptr
	 * - -3 The pin group is not defined before
	 * - -4 No valid site
	 * - -5 The pin name is not belongs to
	 * - -6 No valid board inserted
	*/
	int InitMCU(const char* lpszPinGroup);
	/**
	 * @brief Initialize PMU
	 * @param[in] lpszPinGroup The pin group name in which the channels' PMU will be initialized
	 * @return Execute result
	 * - 0 Initialize PMU successfully
	 * - -1 Not load vector before
	 * - -2 The point of pin group is nullptr
	 * - -3 The pin group is not defined before
	 * - -4 No valid site
	 * - -5 The pin name is not belongs to
	 * - -6 No valid board inserted
	*/
	int InitPMU(const char* lpszPinGroup);
	/**
     * @brief Set the pin voltage using in MCU
     * @param[in] lpszPinGroup The pin group whose pin level will be set
     * @param[in] dVIH The input voltage of logic high
     * @param[in] dVIL The input voltage of logic low
     * @param[in] dVOH The output voltage of logic high
     * @param[in] dVOL The output voltage of logic low
     * @return Execute result
     * - 0 Set pin voltage successfully
     * - -1 Not load vector before
	 * - -2 The point of pin group is nullptr
     * - -3 The pin group is not defined before
	 * - -4 No valid site
	 * - -5 The pin name is not belongs to
	 * - -6 The pin level is over range
	 * - -7 No board existed
    */
	int SetPinLevel(const char* lpszPinGroup, double dVIH, double dVIL, double dVOH, double dVOL);
	/**
	 * @brief Set the timeset edge
	 * @param[in] lpszPinGroup The name of pin group
	 * @param[in] lpszTimeset The name of the timeset
	 * @param[in] WaveFormat The wave format
	 * @param[in] IOFormat The IO format
	 * @param[in] pdEdgeValue The edge value
	 * @param[in] CompareMode The compare mode
	 * @return Execute result
	 * - 0 Set edge successfully
	 * - -1 Not load vector before
	 * - -2 The point of pin group is nullptr
	 * - -3 The pin group is not defined before
	 * - -4 No valid site
	 * - -5 The pin name is not belongs to
	 * - -6 The point of timeset name is nullptr
	 * - -7 The timeset name is not defined in vector
	 * - -8 The wave format is not supported
	 * - -9 The IO format is not supported
	 * - -10 The compare mode is not supported
	 * - -11 The point of edge is nullptr
	 * - -12 The edge is over range
	 * - -13 No valid board inserted
	*/
	int SetEdge(const char* lpszPinGroup, const char* lpszTimeset, WAVE_FORMAT WaveFormat, IO_FORMAT IOFormat, double* pdEdgeValue, COMPARE_MODE CompareMode);
	/**
	 * @brief Set the timeset edge
	 * @param[in] bySlotNo The slot number
	 * @param[in] usChannel The channel number
	 * @param[in] byTimeset The timeset index
	 * @param[in] WaveFormat The wave format
	 * @param[in] IOFormat The IO format
	 * @param[in] pdEdgeValue The edge value
	 * @param[in] CompareMode The compare mode
	 * @return Execute result
	 * - 0 Set edge successfully
	 * - -1 The board is not exited
	 * - -2 The timeset is over range
	 * - -3 The format is error
	 * - -4 The point of edge is nullptr
	 * - -5 The edge value is over range
	 * - -6 The channel is not existed
	*/
	int SetEdge(BYTE bySlotNo, USHORT usChannel, BYTE byTimeset, WAVE_FORMAT WaveFormat, IO_FORMAT IOFormat, double* pdEdgeValue, COMPARE_MODE CompareMode);
	/**
	 * @brief Get the edge of channel
	 * @param[in] bySlotNo The slot number
	 * @param[in] usChannel The channel number
	 * @param[in] byTimesetIndex The timeset index
	 * @param[out] pdEdge The edge value of timeset
	 * @param[out] WaveFormat The wave format
	 * @param[out] IOFormat The IO format
	 * @param[out] CompareMode The compare mode
	 * @return Execute result
	 * - 0 Get edge successfully
	 * - -1 The board is not existed
	 * - -2 The channel is over range
	 * - -3 The channel is not existed
	 * - -4 The timeset is over range
	 * - -5 The point of edge is nullptr
	*/
	int GetEdge(BYTE bySlotNo, USHORT usChannel, BYTE byTimesetIndex, double* pdEdge, WAVE_FORMAT& WaveFormat, IO_FORMAT& IOFormat, COMPARE_MODE& CompareMode);
	/**
	 * @brief Get pin type
	 * @param[in] lpszString
	 * @return Pin type
	 * - 0 The string is pin
	 * - 1 The string is pin group
	 * - 2 The string label
	 * - -1 Not load vector before
	 * - -2 The point of string is nullptr
	 * - -3 Not find
	*/
	int GetStringType(const char* lpszString);
	/**
	 * @brief Run vector
	 * @param[in] lpszPinGroup The pin group name
	 * @param[in] lpszStartLabel The start label name
	 * @param[in] lpszStopLabel The stop label name
	 * @param[in] bWaitFinish Whether wait the vector stop
	 * @return Execute result
	 * - 0 Run vector successfully
	 * - -1 Not load vector before
	 * - -2 The point of pin group is nullptr
	 * - -3 The pin group is not defined before
	 * - -4 No valid site
	 * - -5 The pin is not belongs to
	 * - -6 The start label is nullptr
	 * - -7 The start label is not defined in vector file
	 * - -8 The stop label is nullptr
	 * - -9 The stop label is not defined in vector file
	 * - -10 The start label is after stop label
	 * - -11 No valid board existed
	*/
	int RunVector(const char* lpszPinGroup, const char* lpszStartLabel, const char* lpszStopLabel, BOOL bWaitFinish);
	/**
	 * @brief Set the run parameter
	 * @param[in] lpszPinGroup The pin group name
	 * @param[in] lpszStartLabel The start label name
	 * @param[in] lpszStopLabel The stop label name
	 * @return Execute result
	 * - 0 Run vector successfully
	 * - -1 Not load vector before
	 * - -2 The point of pin group is nullptr
	 * - -3 The pin group is not defined before
	 * - -4 No valid site
	 * - -5 The pin name is not belongs to
	 * - -6 The start label is nullptr
	 * - -7 The start label is not defined in vector file
	 * - -8 The stop label is nullptr
	 * - -9 The stop label is not defined in vector file
	 * - -10 The start label is after stop label
	 * - -11 No valid board existed
	*/
	int SetRunParam(const char* lpszPinGroup, const char* lpszStartLabel, const char* lpszStopLabel);
	/**
	 * @brief Set channel status
	 * @param[in] lpszPinGroup The pin group name
	 * @param[in] usSiteNo The site number
	 * @param[in] ChannelStatus The channel status
	 * @return Execute result
	 * - 0 Set capture lien successfully
	 * - -1 Not load vector before
	 * - -2 The point of pin group is nullptr
	 * - -3 The pin group is not defined
	 * - -4 The site is over range
	 * - -5 The site is invalid
	 * - -6 The pin is not belongs to
	 * - -7 The channel status is not supported
	 * - -8 The channel is not existed
	 * - -9 No valid channel existed
	*/
	int SetChannleStatus(const char* lpszPinGroup, USHORT usSiteNo, CHANNEL_OUTPUT_STATUS ChannelStatus);
	/**
	 * @brief Get the channel status
	 * @param[in] bySlotNo The slot number
	 * @param[in] usChannel The channel number
	 * @return The channel status
	 * - 0 The level is lower than VOH and VOL
	 * - 1 The level is higher than VOL and lower than VOH
	 * - 2 The level is higher than VOH and lower than VOL
	 * - 3 The level is higher than VOL and VOH
	 * - -1 The board is not existed
	 * - -2 The channel is over range
	 * - -3 The channel is not existed
	*/
	int GetChannelStatus(BYTE bySlotNo, USHORT usChannel);
	/**
	 * @brief Get channel mode
	 * @param[in] bySlotNo The slot number board inserted
	 * @param[in] usChannel The channel number in board
	 * @return The channel mode	 *
	 * - 0 The channel is in MCU mode
	 * - 1 The channel is in PMU mode
	 * - 2 The channel is Neither MCU mode, nor PMU mode
	 * - -1 The board is not exsited
	 * - -2 The channel number is over range
	 * - -3 The channel is not existed
	*/
	int GetChannelMode(BYTE bySlotNo, USHORT usChannel);
	/**
	 * @brief Check whether wait run command
	 * @return Whether wait run command
	 * - TRUE Wait run command
	 * - FALSE Not wait run command
	*/
	int IsWaitRun();
	/**
	 * @brief Run vector
	 * @return Execute result
	 * - 0 Run vector successfully
	 * - -1 No vector wait for running
	*/
	int Run();
	/**
	 * @brief Wait the vector stop
	 * @return 
	*/
	int WaitStop();
	/**
	 * @brief Stop vector
	 * @param[in] lpszPinGroup The pin group name
	 * @return Execute result
	 * - 0 Stop vector successfully
	 * - -1 Not load vector before
	 * - -2 The point of pin group is nullptr
	 * - -3 The pin group is not defined before
	 * - -4 No valid site
	 * - -5 The pin is not belongs to
	 * - -6 No valid board existed
	*/
	int Stop(const char* lpszPinGroup);
	/**
	 * @brief Get the data of latest ran
	 * @param[in] lpszPinName The pin number
	 * @param[in] usSiteNo The site number
	 * @param[in] lpszStartLabel The start label name
	 * @param[in] ulOffset The start line offset to the label
	 * @param[in] nLineCount The line count to be read
	 * @param[out] ulData The data result
	 * @return Execute result
	 * - 0 Get the capture data successfully
	 * - -1 Not load vector before
	 * - -2 The point of pin name is nullptr
	 * - -3 The pin is not defined
	 * - -4 The site is over range
	 * - -5 The site is invalid
	 * - -6 The pin is not belongs to
	 * - -7 The board is not existed
	 * - -8 The point of start label is nullptr
	 * - -9 The start label is not defined in vector
	 * - -10 The offset is over range
	 * - -11 The capture line is not in latest ran vector
	 * - -12 Not ran vector in channel
	 * - -13 The vector is running
	 * - -14 The channel is over range
	 * - -15 The channel is not existed
	 * - -16 Fail line number is not saving
	 * - -17 The channel used save selected line fail, not support capture data
	*/
	int GetCaptureData(const char* lpszPinName, USHORT usSiteNo, const char* lpszStartLabel, ULONG ulOffset, int nLineCount, ULONG& ulData);
	/**
	 * @brief Set the capture line
	 * @param[in] lpszPinGroup The pin group name
	 * @param[in] usSiteNo The site number
	 * @param[in] lpszStartLabel The start label name
	 * @param[in] ulOffset The start line offset to the label
	 * @param[in] nLineCount The line count to be read
	 * @return Execute result
	 * - 0 Set capture lien successfully
	 * - -1 Not load vector before
	 * - -2 The point of pin group is nullptr
	 * - -3 The pin group is not defined
	 * - -4 The site is over range
	 * - -5 The site is invalid
	 * - -6 The pin is not belongs to
	 * - -7 The point of start label is nullptr
	 * - -8 The start label is not defined in vector
	 * - -9 The offset is over range
	 * - -10 The capture line is not in latest ran vector
	 * - -11 Not ran vector in channel
	 * - -12 The vector is running
	 * - -13 The channel is not existed
	 * - -14 The board is not existed
	*/
	int SetCaptureLine(const char* lpszPinGroup, USHORT usSiteNo, const char* lpszStartLabel, ULONG ulOffset, int nLineCount);
	/**
	 * @brief Get the capture data of channel
	 * @param[in] lpszPinName The pin name
	 * @param[in] usSiteNo The site number
	 * @param[out] ulCaptureData The capture data
	 * @return Execute result
	 * - 0 Get the capture data successfully
	 * - -1 Not load vector before
	 * - -2 The point of pin name is nullptr
	 * - -3 The pin is not defined
	 * - -4 The site is over range
	 * - -5 The site is invalid
	 * - -6 The pin is not belongs to
	 * - -7 The board is not existed
	 * - -8 The channel is over range
	 * - -9 The channel is not existed
	 * - -10 Not ran vector in channel
	 * - -11 The vector is running
	 * - -12 Not seting the capture line
	 * - -13 Fail line number is not saving
	 * - -14 The channel used save selected line fail, not support capture data
	*/
	int GetCaptureData(const char* lpszPinName, USHORT usSiteNo, ULONG& ulCaptureData);
	/**
	 * @brief Get the capture
	 * @param[in] lpszPinName The pin number
	 * @param[in] usSiteNo The site number
	 * @param[out] pbyCaptureData The capture data, the element of the array must have enough memory to save the capture data
	 * @param[in] nBuffSize The capture line count 
	 * @return The capture line count
	 * - >=0 The capture line count
	 * - -1 Not load vector before
	 * - -2 The point of pin name is nullptr
	 * - -3 The pin is not defined
	 * - -4 The site is over range
	 * - -5 The site is invalid
	 * - -6 The pin is not belongs to
	 * - -7 The board is not existed
	 * - -8 The channel is over range
	 * - -9 The channel is not existed
	 * - -10 Not ran vector
	 * - -11 Vector running
	 * - -12 The cpature line count in latest ran are ove range
	*/
	int GetHardwareCapture(const char* lpszPinName, USHORT usSiteNo, BYTE* pbyCaptureData, int nBuffSize);
	/**
	 * @brief Get the MCU run result of specific site
	 * @param[in] usSiteNo The specific site
	 * @return MCU run result
	 * - 0 MCU run PASS
	 * - 1 MCU run FAIL
	 * - -1 No load vector before
	 * - -2 Not ran vector before
	 * - -3 The site number is over range
	 * - -4 The site is invalid
	 * - -5 Vector running
	 * - -6 Some channels are not existed
	 * - -7 No board existed
	*/
	int GetMCUResult(USHORT usSiteNo);
	/**
	 * @brief Get MCU result of specific channel
	 * @param[in] lpszPinName The pin number
	 * @param[in] usSiteNo The specific site
	 * @return MCU run result
	 * - 0 MCU run PASS
	 * - 1 MCU run FAIL
	 * - -1 Not load vector before
	 * - -2 The point of pin name is nullptr
	 * - -3 The pin is not defined
	 * - -4 The site is over range
	 * - -5 The site is invalid
	 * - -6 The pin is not belongs to
	 * - -7 The board is not existed
	 * - -8 The channel is not existed
	 * - -9 Not ran vector before
	 * - -10 The vector is running 
	*/
	int GetPinMCUResult(const char* lpszPinName, USHORT usSiteNo);
	/**
	 * @brief Get the running status
	 * @param[in] lpszPinName The pin number
	 * @param[in] usSiteNo The site number
	 * @return Running status
	 * - 0 Running
	 * - 1 Stop running
	 * - 2 Not ran
	 * - -1 Not load vector before
	 * - -2 The point of pin name is nullptr
	 * - -3 The pin is not defined
	 * - -4 The site is over range
	 * - -5 The site is invalid
	 * - -6 The pin is not belongs to
	 * - -7 The board is not existed
	 * - -8 The channel is not existed
	*/
	int GetRunningStatus(const char* lpszPinName, USHORT usSiteNo);
	/**
	 * @brief Get fail line count of pin
	 * @param[in] lpszPinName The pin number
	 * @param[in] usSiteNo The site number
	 * @return Fail line count
	 * - >0 The fail line count
	 * - -1 Not load vector before
	 * - -2 The point of pin name is nullptr
	 * - -3 The pin is not defined
	 * - -4 The site is over range
	 * - -5 The site is invalid
	 * - -6 The pin is not belongs to
	 * - -7 The board is not existed
	 * - -8 The channel is not existed
	 * - -9 Not ran vector before
	 * - -10 Vector running
	*/
	int GetFailCount(const char* lpszPinName, USHORT usSiteNo);
	/**
	 * @brief Get first fail line number
	 * @param[in] lpszPinName The pin number
	 * @param[in] usSiteNo The site number
	 * @return The fist fail line number
	 * - >=0 First fail line number
	 * - -1 Not load vector before
	 * - -2 The point of pin name is nullptr
	 * - -3 The pin is not defined
	 * - -4 The site is over range
	 * - -5 The site is invalid
	 * - -6 The pin is not belongs to
	 * - -7 The board is not existed
	 * - -8 The channel is over range
	 * - -9 The channel is not existed
	 * - -10 Not ran before
	 * - -11 Vector running
	 * - -12 No fail line in latest ran
	 * - -13 The fail line number can't be gotten for the fail memory full occupied by other channels'
	 */
	int GetFirstFailLineNo(const char* lpszPinName, USHORT usSiteNo);
	/**
	* @brief Get the fail line number of last running
	* @param[in] lpszPinName The pin number
	* @param[in] usSiteNo The site number
	* @param[in] uGetMaxFailCount The maximum fail count will be gotten
	* @param[out] vecLineNo The fail line number of last running
	* @return Execute result
	 * - 0 Get the fail line number successfully
	 * - 1 Not all fail line being saved
	 * - -1 Not load vector before
	 * - -2 The point of pin name is nullptr
	 * - -3 The pin is not defined
	 * - -4 The site is over range
	 * - -5 The site is invalid
	 * - -6 The pin is not belongs to
	 * - -7 The board is not existed
	 * - -8 The channel is over range
	 * - -9 The channel is not existed
	 * - -10 Not ran vector
	 * - -11 Vector running
	 * - -12 The fail line number can't be gotten for the fail memory full occupied by other channels'
	*/
	int GetFailLineNo(const char* lpszPinName, USHORT usSiteNo, UINT uGetMaxFailCount, std::vector<int>& vecLineNo);
	/**
	 * @brief Save the fail line number to failmap
	 * @param[in] uMaxFailLine The maximum fail line count
	 * @return Execute result
	 * - 0 Save fail map successfully
	 * - -1 Not load vector before
	 * - -2 Not ran vector before
	 * - -3 The vector is running
	 * - -4 Allocate memory fail
	 * - -5 The fail memory are fully occupied by invalid sites'
	*/
	int SaveFailMap(UINT uMaxFailLine);
	/**
	 * @brief Get the stop line number
	 * @param[in] usSiteNo The site number
	 * @return Stop line number
	 * - >= 0 The stop line number
	 * - -1 Not load vector before
	 * - -2 The site number is over range
	 * - -3 The site is invalid
	 * - -4 Channel is not existed
	 * - -5 Not ran before
	 * - -6 Vector running
	 * - -7 Board is not existed
	*/
	int GetStopLineNo(USHORT usSiteNo);
	/**
	 * @brief Get the line count of running
	 * @param[in] lpszPinName The pin name
	 * @param[in] usSiteNo The site count
	 * @param[out] ulLineCount The line count of the vector running
	 * @return Execute result
	 * - 0 Get the line count successfully 
	 * - -1 Not load vector before
	 * - -2 The point of pin name is nullptr
	 * - -3 The pin is not defined
	 * - -4 The site is over range
	 * - -5 The site is invalid
	 * - -6 The pin is not belongs to
	 * - -7 The board is not existed
	 * - -8 The channel is over range
	 * - -9 The channel is not existed
	*/
	int GetRunLineCount(const char* lpszPinName, USHORT usSiteNo, ULONG& ulLineCount);
	/**
	 * @brief Set the dynamic load
	 * @param[in] lpszPinName The pin name
	 * @param[in] usSiteNo The site number
	 * @param[in] Mode The dynamic load mode
	 * @param[in] dIOH The IOH value
	 * @param[in] dIOL The IOL value
	 * @param[in] dVTValue The VT voltage
	 * @param[in] dClmapHigh The high clamp voltage
	 * @param[in] dClampLow The low clamp voltage
	 * @return Execute result
	 * - 0 Set dynamic load successfully
	 * - -1 Not load vector before
	 * - -2 The point of pin name is nullptr
	 * - -3 The pin is not defined
	 * - -4 The site is over range
	 * - -5 The site is invalid
	 * - -6 The pin is not belongs to
	 * - -7 The board is not existed
	 * - -8 The channel is not existed	
	 * - -9 The output current is over range
	 * - -10 The VT is over range
	 * - -11 The clamp is over range
	*/
	int SetPinDynamicLoad(const char* lpszPinName, USHORT usSiteNo, BOOL bEnable, double dIOH, double dIOL, double dVTVoltValue, double dClampHighVoltValue = 7.5, double dClampLowVoltValue = -2.5);
	/**
	 * @brief Set the dynamic load
	 * @param[in] lpszPinGroup The pin group
	 * @param[in] Mode The dynamic load mode
	 * @param[in] bEnable Whether enable dynamic load
	 * @param[in] dIOH The IOH value
	 * @param[in] dIOL The IOL value
	 * @param[in] dVTValue The VT voltage
	 * @param[in] dClmapHigh The high clamp voltage
	 * @param[in] dClampLow The low clamp voltage
	 * @return Execute result
	 * - 0 Set capture lien successfully
	 * - -1 Not load vector before
	 * - -2 The point of pin group is nullptr
	 * - -3 The pin group is not defined
	 * - -4 No valid site
	 * - -5 The pin is not belongs to
	 * - -6 The channel used is not existed
	 * - -7 The output current is over range
	 * - -8 The VT is over range
	 * - -9 The clamp is over range
	 * - -10 No board used is existed
	*/
	int SetDynamicLoad(const char* lpszPinGroup, BOOL bEnable, double dIOH, double dIOL, double dVTVoltValue, double dClampHighVoltValue = 7.5, double dClampLowVoltValue = -2.5);
	/**
	* @brief Get the dynamic load
	* @param[in] bySlotNo The slot number
	* @param[in] usChannel The channel number
	* @param[out] DynamicLoadMode The dynamic load mode
	* @param[out] dIOH The IOH value
	* @param[out] dIOH The IOL value
	* @return Execute result
	* - 0 Get dynamic load successfully
	* - -1 The board is not existed
	* - -2 The channel index is over range
	* - -3 The channel is not existed
	*/
	int GetDynamicLoad(BYTE bySlotNo, USHORT usChannel, BOOL& bEnable, double& dIOH, double& dIOL);
	/**
	* @brief Set PMU settings of specific channel
	* @param[in] lpszPinGroup The pin group
	* @param[in] usSiteNo The site number, 0xFFFF is all site
	* @param[in] PMUMode The PMU mode
	* @param[in] Range The current range of PMU
	* @param[in] dSetValue The set value of voltage or current after calibration
	* @param[in] dClmapHigh The high clamp value
	* @param[in] dClampLow The low clamp value
	* @return Execute result
	* - 0 Set PMU successfully
	* - -1 Not load vector before
	* - -2 The point of pin group is nullptr
	* - -3 The pin group is not defined before
	* - -4 The site number is over range
	* - -5 The site is invalid
	* - -6 The pin is not belongs to
	* - -7 The board in pin group is not existed
	* - -8 No valid board existed
	*/
	int SetPMUMode(const char* lpszPinGroup, USHORT usSiteNo, PMU_MODE PMUMode, PMU_IRANGE Range, double dSetValue, double dClmapHigh = 7.5, double dClampLow = -2.5);
	/**
	* @brief Start PMU multi-measure
	* @param[in] lpszPinGroup The name of pin group
	* @param[in] nSampleTimes The sample times
	* @param[in] dSamplePeriod The sample period
	* @return Execute result
	* - 0 Measure successfully
	* - -1 Not load vector before
	* - -2 The point of pin group is nullptr
	* - -3 The pin group is not defined before
	* - -4 The pin is not belongs to
	* - -5 The channel are all not existed
	* - -6 PMU measurement error
	* - -7 No valid board existed in pin group
	*/
	int PMUMeasure(const char* lpszPinGroup, int nSampleTimes, double dSamplePeriod);
	/**
	 * @brief Get the PMU measure result of specific sample
	 * @param[in] lpszPinName The pin number
	 * @param[in] usSiteNo The site number
	 * @param[in] nSamppleTimes The sample point, -1 is the average value
	 * @return The measure result
	 * - != 1e15 The measure result
	 * - 1e15 The channel is over range or the sample times is over range
	*/
	double GetPMUMeasureResult(const char* lpszPinName, USHORT usSiteNo, int nSampleTimes);
	/**
	 * @brief Get the label name of stop line
	 * @param[in] usSiteNo The site number
	 * @return The stop label
	 * - != nullptr The stop line number
	 * - nullptr Not load vector before;The site nunber is over range;The site is invalid;4 Not ran before;-5 Vector running
	*/
	const char* GetStopLineLabel(USHORT usSiteNo);
	/**
	 * @brief Get the label line number
	 * @param[in] lpszLabelName The label name
	 * @param[in] bBRAMLine Whether only get the BRAM line number, not global line
	 * @return Label line number
	 * - >=0 The label line number
	 * - -1 Not load vector before
	 * - -2 The point of label is nullptr
	 * - -3 The label is not existed
	*/
	int GetLabelLineNo(const char* lpszLabelName, BOOL bBRAMLine);
	/**
	 * @brief Set the operand of line
	 * @param[in] lpszPinGroup The pin group name
	 * @param[in] lpszStartLabel The start label
	 * @param[in] ulOffset The line number offset to the label
	 * @param[in] lpszOperand The operand
	 * @return Execute result
	 * - 0 Set the operand successfully
	 * - -1 Not load vector before
	 * - -2 The point of pin group is nullptr
	 * - -3 The pin group is not defined before
	 * - -4 No valid site
	 * - -5 The pin is belongs to
	 * - -6 The start label is nullptr
	 * - -7 The start label is not existed
	 * - -8 The offset is over range
	 * - -9 The line is not in BRAM
	 * - -10 The operand is nullptr
	 * - -11 The label of the operand is not existed
	 * - -12 The operand is over range
	 * - -13 The channels are all not existd
	 * - -14 No valid board existed
	*/
	int SetOperand(const char* lpszPinGroup, const char* lpszStartLabel, ULONG ulOffset, const char* lpszOperand);
	/**
	 * @brief Set the instruction
	 * @param[in] lpszPinGroup The pin group name
	 * @param[in] lpszLabel The start label
	 * @param[in] nOffset The line number offset to the label
	 * @param[in] lpszInstruction The instruction set
	 * @param[in] lpszOperand The operand of the instruction
	 * @return Execute result
	 * - -1 Not load vector before
	 * - -2 The pin group is nullptr
	 * - -3 The pin group is not defined
	 * - -4 No valid site
	 * - -5 The pin is not belongs to
	 * - -6 The start label is nullptr
	 * - -7 The start label is not existed
	 * - -8 The offset is over range
	 * - -9 The line is not in BRAM
	 * - -10 The operand is nullptr
	 * - -11 The label of the operand is not existed
	 * - -12 The operand is over range
	 * - -13 The instruction is nullptr
	 * - -14 The instruction is not supported
	 * - -15 The channels used are all not existed
	 * - -16 No board existed
	*/
	int SetInstruction(const char* lpszPinGroup, const char* lpszStartLabel, int nOffset, const char* lpszInstruction, const char* lpszOperand);
	/**
	 * @brief Set the start or stop line number of select fail
	 * @param[in] lpszPinGroup The pin group name
	 * @param[in] lpszLabel The start label
	 * @param[in] nOffset The line number offset to the label
	 * @param[in] bStartSave Whether start saving
	 * @param[in] bBRAM Whether the line number is BRAM
	 * @param[in] bClose Whether bDelete start or stop fail
	 * @return Execute result
	 * - 0 Set the saving select fail successfully
	 * - -1 Not load vector before
	 * - -2 The pin group is nullptr
	 * - -3 The pin group is not defined
	 * - -4 No valid site 
	 * - -5 The start label is nullptr
	 * - -6 The start label is not existed
	 * - -7 The offset is over range
	 * - -8 The channel is not existed
	 * - -9 No board existed
	*/
	int SetSaveSelectFail(const char* lpszPinGroup, const char* lpszLabel, int nOffset, BOOL bStartSave, BOOL bDelete = FALSE);
	/**
	 * @brief Get the instruction of the line
	 * @param[in] bySlotNo The slot number of the board
	 * @param[in] byController The controller whose instruction will be gotten
	 * @param[in] uBRAMLineNo The line number of BRAM
	 * @param[out] lpszInstruction The buff for save instruction
	 * @param[in] nBuffSize The buff size
	 * @return Execute result
	 * - 0 Get the instruction successfully
	 * - -1 The board is not existed
	 * - -2 The controller index is over range
	 * - -3 The controller is not existed
	 * - -4 The line number is over range
	 * - -5 The point of the buff is nullptr
	 * - -6 The buff is too small
	*/
	int GetInstruction(BYTE bySlotNo, BYTE byController, UINT uBRAMLineNo, char* lpszInstruction, int nBuffSize);
	/**
	 * @brief Get the operand of the line
	 * @param[in] bySlotNo The slot number of the board
	 * @param[in] byController The controller whose operand will be gotten
	 * @param[in] uBRAMLineNo The BRAM line number
	 * @return The operand
	 * - >=0 The operand
	 * - -1 The board is not existed
	 * - -2 The controller index is over range
	 * - -3 The controller is not existed
	 * - -4 The line number is over range
	*/
	int GetOperand(BYTE bySlotNo, BYTE byController, UINT uBRAMLineNo);
	/**
	 * @brief Get the channel information
	 * @param[in] usPinNo The pin number
	 * @param[in] usSiteNo The site number
	 * @param[out] Channel The channel information
	 * @return Execute result
	 * - 0 Get the channel information successfully
	 * - -1 The pin number is not existed
	 * - -2 The site number is over range
	*/
	int GetChannel(USHORT usPinNo, USHORT usSiteNo, CHANNEL_INFO& Channel);
	/**
	 * @brief Get the pin name
	 * @param[in] usPinNo The pin number
	 * @param[out] strPinName The pin name
	 * @return Execute result
	 * - 0 Get pin name
	 * - -1 Not load vector file
	 * - -2 The pin number is not existed
	*/
	int GetPinName(USHORT usPinNo, std::string& strPinName);
	/**
	 * @brief Reset all board information including board
	*/
	void Reset();
	/**
	 * @brief Set the calibration information
	 * @param[in] bySlotNo The slot number of the board
	 * @param[in] byControllerIndex The controller index of board
	 * @param[in] pCalInfo The calibration information of each channel
	 * @param[in] pbyChannelStatus Whether update the calibration information of each channel
	 * @param[in] nElementCount The element count of each array
	 * @return Execute result
	 * - 0 Set the calibration information successfully
	 * - -1 The board is not existed
	 * - -2 The controller index is over range
	 * - -3 The controller is not existed
	 * - -4 The point of array is nullptr
	 * - -5 The element count is less than the channel of controller
	 * - -6 Set the calibration information fail
	*/
	int SetCalibrationInfo(BYTE bySlotNo, BYTE byControllerIndex, STS_CALINFO* pCalInfo, BYTE* pbyChannelStatus, int nElementCount);
	/**
	 * @brief Get the calibration information
	 * @param[in] bySlotNo The slot number of the board
	 * @param[in] byControllerIndex The controller index of DCM
	 * @param[out] pCalInfo The calibration information of each channel
	 * @param[in] nElementCount The element count of each array
	 * @return Execute result
	 * - -1 The board is not existed
	 * - -2 The controller index is over range
	 * - -3 The controller is not existed
	 * - -4 The point of array is nullptr
	 * - -5 The element count is less than the channel of controller
	 * - -6 Get the calibration information fail
	*/
	int GetCalibrationInfo(BYTE bySlotNo, BYTE byControllerIndex, STS_CALINFO* pCalInfo, int nElementCount);
	/**
	 * @brief Get the calibration information of the pin
	 * @param[in] lpszPinName The pin name
	 * @param[in] usSiteNo The site number
	 * @param[out] CalibrationInfo The calibration information
	 * @return Execute result
	 * - 0 Get the calibration result successfully
	 * - -1 Not load vector before
	 * - -2 The pin name is nullptr
	 * - -3 The pin name is not defined
	 * - -4 The Site number is over range
	 * - -5 The site is invalid
	 * - -6 The pin is not belongs to
	 * - -7 The board is not existed
	 * - -8 The channel is over range
	 * - -9 The channel is not existed
	 * - -10 Get the calibration information fail
	*/
	int GetCalibrationInfo(const char* lpszPinName, USHORT usSiteNo, STS_CALINFO& CalibrationInfo);
	/**
	 * @brief Get the calibration information of the channel
	 * @param[in] bySlotNo The slot number of the board
	 * @param[in] usChannel The channel number
	 * @param[out] CalibrationInfo The calibration information
	 * @return  Execute result
	 * - 0 Get the calibration result successfully
	 * - -1 The board is not existed
	 * - -2 The channel is over range
	 * - -3 The channel is not existed
	 * - -4 Get the calibration information fail
	*/
	int GetCalibrationInfo(BYTE bySlotNo, USHORT usChannel, STS_CALINFO& CalibrationInfo);
	/**
	 * @brief Set the VT
	 * @param[in] lpszPinGroup The pin group
	 * @param[in] dVTVoltValue The VT voltage value
	 * @param[in] VTMode The VT mode
	 * @return Execute result
	* - 0 Set VT successfully
	* - -1 Not load vector before
	* - -2 The point of pin group is nullptr
	* - -3 The pin group is not defined before
	* - -4 No valid site
	* - -5 The pin is not belongs to
	* - -6 The VT value is over range
	* - -7 The mode is error
	* - -8 The chanenls are all not existed
	* - -9 The board is not existed
	*/
	int SetVT(const char* lpszPinGroup, double dVTVoltValue, VT_MODE VTMode = VT_MODE::CLOSE);
	/**
	* @brief Get the mode of specific channel
	* @param[in] bySlotNo The slot number
	* @param[in] usChannel The fail line number of last running
	* @param[out] VTMode The VT mode of the channel
	* @return Execute result
	* - 0 Get VT mode successfully
	* - -1 The board is not existed
	* - -2 The channel is over range
	* - -3 The channel is not existed
	*/
	int GetVTMode(BYTE bySlotNo, USHORT usChannel, VT_MODE& VTMode);
	/**
	 * @brief Set preread vector
	 * @param[in] lpszStartLabel The start label name
	 * @param[in] lpszStopLabel The stop label name
	 * @return Execute result
	 * - 0 Set the preread line successfully
	 * - -1 Not load vector before
	 * - -2 The point of start label or stop label is nullptr
	 * - -3 The start label is not existed
	 * - -4 The stop label is not existed
	 * - -5 The start label is not before stop label
	 * - -6 The preread line count have reached to maximum limited
	 * - -7 Allocate memory fail
	 * - -8 The board is not existed
	 */
	int SetPrereadVector(const char* lpszStartLabel, const char* lpszStopLabel);
	/**
	 * @brief Set the line information used in future
	 * @param[in] lpszPinGroup The pin group name
	 * @param[in] usSiteNo The site number, 0xFFFF is all site
	 * @param[in] lpszStartLabel The label name
	 * @param[in] ulOffset The start line offset to label
	 * @param[in] nWriteVectorLineCount The line count
	 * @return Execute result
	 * - 0 Set line info successfully
	 * - -1 Not load vector before
	 * - -2 The point of pin group is nullptr
	 * - -3 The pin group is not defined
	 * - -4 The site is over range
	 * - -5 The site is invalid
	 * - -6 The pin is not belongs to
	 * - -7 The point of start label is nullptr
	 * - -8 The label is not existed
	 * - -9 The offset is over range
	 * - -10 The vector line count is over range
	 * - -11 Allocate memory fail
	 * - -12 The channels used in pin group are not existed
	 * - -13 No valid board existed
	*/
	int SetLineInfo(const char* lpszPinGroup, USHORT usSiteNo, const char* lpszStartLabel, ULONG ulOffset, int nWriteVectorLineCount, BOOL bDataSame = FALSE);
	/**
	 * @brief Set the wave data of the site
	 * @param[in] usSiteNo The site number
	 * @param[in] pbyWaveData The wave data of the channel in specific site
	 * @return Execute result
	 * - 0 Execute result
	 * - -1 No load vector file
	 * - -2 Not set the line information
	 * - -3 The site number is over range
	 * - -4 The site is invalid
	 * - -5 The point of wave data is nullptr
	 * - -6 Allocate memory fail
	 * - -7 No valid board existed
	*/
	int SetSiteWaveData(USHORT usSiteNo, const BYTE* pbyWaveData);
	/**
	 * @brief Write wave data to board
	 * @return Execute result
	 * - -1 Not load vector file
	 * - -2 Not set line information
	 * - -3 The site number is over range
	 * - -4 The site is invalid
	 * - -5 No valid board existed
	*/
	int WriteData();
	/**
	 * @brief Get the valid board
	 * @param[out] vecBoard The slot number of all valid board
	*/	
	void GetValidBoard(std::vector<BYTE>& vecBoard);
	/**
	 * @brief Get the board count and site count in vector
	 * @param[out] pusSiteCount The site count in vector
	 * @return Board count used in vector file
	 * - >=0 The board count used in vector file
	 * - -1 Not load vector before
	*/
	int GetVectorBoardCount(USHORT* pusSiteCount);
	/**
	 * @brief Get the channel of pin
	 * @param[in] lpszPinName The pin name
	 * @param[in] usSiteNo The site number
	 * @param[out] Channel Channel information
	 * @return Execute result
	 * - 0 Get the channel successfully
	 * - -1 Not load vector before
	 * - -2 The point of pin name is nullptr
	 * - -3 The pin name is not defined
	 * - -4 The site is over range
	 * - -5 The site is invalid
	 * - -6 The pin is belongs to
	*/
	int GetChannel(const char* lpszPinName, USHORT usSiteNo, CHANNEL_INFO& Channel);
	/**
	 * @brief Get the channel information of pin group
	 * @param[in] lpszPinGroup The pin group name
	 * @param[in] usSiteNo The site number
	 * @param[in] vecChannel The channel information
	 * @param[in] bOnlyValidSite Whether only get valid site
	 * @return Execute result
	 * - 0 Get channel successfully
	 * - -1 Not load vector file successfully
	 * - -2 The point of pin group is nullptr
	 * - -3 The pin group is not defined
	 * - -4 The site is invalid
	*/
	int GetChannel(const char* lpszPinGroup, USHORT usSiteNo, std::vector<CHANNEL_INFO>& vecChannel, BOOL bOnlyValidSite = TRUE);
	/**
	 * @brief Get the pin number in pin group
	 * @param[in] lpszPinGroup The pin group name
	 * @param[out] vecPinNo The pin number in pin group
	 * @return Execute result
	 * - 0 Get the pin number successfully
	 * - -1 Not load vector before
	 * - -2 The point of pin group is nullptr
	 * - -3 The pin group is not defined before
	 * - -4 The pin is not belongs to the instance
	*/
	int GetPinNo(const char* lpszPinGroup, std::vector<USHORT>& vecPinNo);
	/**
	 * @brief Get the site number in the board
	 * @param[in] bySlotNo The slot number
	 * @param[out] vecSite The site number
	*/
	void GetBoardSite(BYTE bySlotNo, std::vector<USHORT>& vecSite) const;
	/**
	 * @brief Get the channel information of pin group
	 * @param[in] lpszPinGroup The pin group name
	 * @param[in] usSiteNo The site number
	 * @param[out] vecChannel The channel information
	 * @return Execute result
	 * - 0 Get the channel successfully
	 * - -1 Not load vector before
	 * - -2 The point of pin group is nullptr
	 * - -3 The pin group is not defined before
	 * - -4 The site number is over range
	 * - -5 The pin is not belongs to
	*/
	int GetPinGroupChannel(const char* lpszPinGroup, USHORT usSiteNo, std::vector<CHANNEL_INFO>& vecChannel);
	/**
	 * @brief Get channel of all pin group
	 * @param[out] vecChannel The channel used in pin group
	 * @return Execute result
	 * - 0 Get pin group channel successfully
	 * - -1 Not load vector before
	*/
	int GetAllPinGroupChannel(std::vector<CHANNEL_INFO>& vecChannel);
	/**
	 * @brief Get the line count between label included label line
	 * @param[in] lpszStartLabel The name of start label,"" is mean from the first line of vector
	 * @param[in] lpszStopLabel The name of stop label, "" is mean stop at the last line of vector
	 * @param[out] uLineCount The count of vector line between start label and stop label
	 * @return The line count
	 * - >=0 The line count between two label
	 * - -1 Not load vector before
	 * - -2 The start label is not defined
	 * - -3 The stop label is not defined
	*/
	int GetLineCount(const char* lpszStartLabel, const char* lpszStopLabel);
	/**
	 * @brief Get the pattern of controller
	 * @param[in] bySlotNo The slot number
	 * @param[in] byControllerIndex The controller index
	 * @param[in] bBRAM Whether get the pattern saved in BRAM
	 * @param[in] uStartLine The start line number
	 * @param[in] uLineCount The line count will be read
	 * @param[out] lpszPattern The pattern
	 * @return Execute result
	 * - 0 Get the pattern successfully
	 * - -1 The board is not existed
	 * - -2 The controller is over range
	 * - -3 The controller is not existed
	 * - -4 The start line is over range
	 * - -5 The line count is over range
	 * - -6 The point of pattern is nullptr
	 * - -7 Allocate memory fail
	*/
	int GetPattern(BYTE bySlotNo, BYTE byControllerIndex, BOOL bBRAM, UINT uStartLine, UINT uLineCount, char(*lpszPattern)[17]);
	/**
	 * @brief Get the memory of controller
	 * @param[in] bySlotNo The slot number
	 * @param[in] byControllerIndex
	 * @param[in] bBRAM Whether the memory is in BRAM
	 * @param[in] DataType The data type
	 * @param[in] uStartLine The start line number
	 * @param[in] uLineCount The line count
	 * @param[out] pusData The data in memory
	 * @return Execute result 
	 * - 0 Read memory successfully
	 * - -1 The board is not existed
	 * - -2 The controller index is over range
	 * - -3 The controller is not existed
	 * - -4 The data type is not supported
	 * - -5 The start line is over range;
	 * - -6 The data count is over range
	 * - -7 The line count read is 0 or the data buff is nullptr
	*/
	int GetMemory(BYTE bySlotNo, BYTE byControllerIndex, BOOL bBRAM, DATA_TYPE DataType, UINT uStartLine, UINT uLineCount, USHORT* pusData);
	/**
	 * @brief Set the memory of channel
	 * @param[in] bySlotNo The slot number
	 * @param[in] usChannel The channel number
	 * @param[in] bRAM Whether the data line in BRAM
	 * @param[in] DataType The data type
	 * @param[in] uStartLine The start line will be written from
	 * @param[in] uLineCount The write line count
	 * @param[out] pbyData The data will be written
	 * @return Execute result
	 * - 0 Write memory successfully
	 * - -1 The board is not existed
	 * - -2 The channel number is over range
	 * - -3 The channel is not existed
	 * - -4 The data type is not supported
	 * - -5 Allocate memory fail
	 * - -6 The start line is over range
	 * - -7 The line count is over range
	 * - -8 The line count is 0
	 * - -9 The point is data is nullptr
	*/
	int SetMemory(BYTE bySlotNo, USHORT usChannel, BOOL bRAM, DATA_TYPE DataType, UINT uStartLine, UINT uLineCount, BYTE* pbyData);
	/**
	 * @brief Mask the alarm of pin
	 * @param[in] lpszPinName The pin Name
	 * @param[in] usSiteNo The site number
	 * @param[in] bMask Whether mask alarm
	 * @return Execute result
	 * - 0 Mask alarm successfully
	 * - -1 Not load vector before
	 * - -2 The point of pin name is nullptr
	 * - -3 The pin is not defined
	 * - -4 The site is over range
	 * - -5 The site is invalid
	 * - -6 The pin is not belongs to
	*/
	int ShieldAlarm(const char* lpszPinName, USHORT usSiteNo, BOOL bMask);
	/**
	 * @brief Get the mask status
	 * @param[in] lpszPinName The pin name
	 * @param[in] usSiteNo The site number
	 * @return Mask status
	 * - 0 Not mask
	 * - 1 Mask
	 * - -1 Not load vector before
	 * - -2 The point of pin name is nullptr
	 * - -3 The pin is not defined
	 * - -4 The site is over range
	 * - -5 The site is invalid
	 * - -6 The pin is not belongs to
	*/
	int GetShieldStatus(const char* lpszPinName, USHORT usSiteNo);
	/**
	 * @brief Shield the specific alarm ID of specific function
	 * @param[in] lpszPinName The pin name
	 * @param[in] usSiteNo The site number
	 * @param[in] lpszFunctionName The function name
	 * @param[in] bShield Whether shield alarm
	 * @param[in] AlarmID The alarm ID
	 * @return Execute result
	 * - 0 Shield alarm successfully
	 * - -1 Not load vector before
	 * - -2 The point of pin name is nullptr
	 * - -3 The pin is not defined
	 * - -4 The site is over range
	 * - -5 The site is invalid
	 * - -6 The pin is not belongs to
	 * - -7 The board is not existed
	 * - -8 The channel is not existed
	*/
	int ShieldFunctionAlarm(const char* lpszPinName, USHORT usSiteNo, const char* lpszFunctionName, BOOL bShield, ALARM_ID AlarmID);
	/**
	 * @brief Get the shield status of the function
	 * @param[in] lpszPinName The pin name
	 * @param[in] usSiteNo The site number
	 * @param[in] lpszFunctionName The function name
	 * @param[in] AlarmID The alarm ID
	 * @return Execute result
	 * - 0 Not shield
	 * - 1 Shield
	 * - -1 Not load vector before
	 * - -2 The point of pin name is nullptr
	 * - -3 The pin is not defined
	 * - -4 The site is over range
	 * - -5 The site is invalid
	 * - -6 The pin is not belongs to
	 * - -7 The board is not existed
	 * - -8 The channel is not existed
	*/
	int GetShieldFunctionAlarm(const char* lpszPinName, USHORT usSiteNo, const char* lpszFunctionName, ALARM_ID AlarmID);
	/**
	 * @brief Get the board and its minimum channel count needed
	 * @param[in] usSiteNo The site number
	 * @param[out] mapBoard The board and its minimum channel count needed
	 * @return Execute result
	 * - 0 Get the site board successfully
	 * - -1 Not load vector before
	 * - -2 The site is over range
	*/
	int GetSiteBoard(USHORT usSiteNo, std::map<BYTE, USHORT>& mapBoard);
	/**
	 * @brief Get the channel information in the site
	 * @param[in] usSiteNo The site number
	 * @param[out] vecChannel The channel information of site
	 * @return Execute result
	 * - 0 Get channel successfully
	 * - -1 Not load vector before
	 * - -2 The site number is over range
	*/
	int GetSiteChannel(USHORT usSiteNo, std::vector<CHANNEL_INFO>& vecChannel);
	/**
	 * @brief Get the site count in vector file
	 * @return The site count
	 * - >=0 The site count
	 * - -1 Not load vector before
	*/
	int GetSiteCount();
	/**
	 * @brief Get the pin count in vector file
	 * @return The pin count
	 * - >=0 The pin count
	*/
	int GetPinCount();
	/**
	* @brief Get the pin level of the specific channel
	* @param[in] bySlotNo The slot number
	* @param[in] usChannel The channel number whose pin level will be gotten
	* @param[in] LevelType The pin level type
	* @return Execute result
	*- 0 Get the pin level successfully
	* - 1e16 The board is not existed, The channel is over range or The level type is error or Get pin level fail
	*/
	double GetPinLevel(BYTE bySlotNo, USHORT usChannel, LEVEL_TYPE LevelType);
	/**
	 * @brief Get the PMU setting
	 * @param[in] bySlotNo The slot number
	 * @param[in] usChannel The channel number
	 * @param[out] PMUMode The PMU mode
	 * @param[out] IRange The current range
	 * @return PMU set value
	 * - The set value of PMU
	 * - 1e15-1 The board is not existed
	 * - 1e15-1 The channel is over range
	 * - 1e15-1 The channel is not existed
	*/
	double GetPMUSettings(BYTE bySlotNo, USHORT usChannel, PMU_MODE& PMUMode, PMU_IRANGE& IRange);
	/**
	 * @brief Clear the vector information
	*/
	void ClearVector();
	/**
	 * @brief Get the fail line number
	 * @param[in] bySlotNo The slot number
	 * @param[in] usChannel The channel number of board
	 * @param[in] bBRAM Whether get the fail line number in BRAM
	 * @param[in] uGetMaxFailCount The maximum fail count will be gotten
	 * @param[out] vecLineNo The fail line number of last running
	 * @return Execute result
	 * - 0 Get line number successfully
	 * - -1 The board is not existed
	 * - -2 The channel is over range
	 * - -3 The channel is not existed
	 * - -4 Not ran vector before
	 * - -5 Vector running
	*/
	int GetFailLineNo(BYTE bySlotNo, USHORT usChannel, BOOL bBRAM, UINT uGetMaxFailCount, std::vector<int>& vecLineNo);
	/**
	 * @brief Get the fail count of channel
	 * @param[in] bySlotNo The slot number
	 * @param[in] usChannel The channel number
	 * @return Fail count
	 * >= The fail count of channel
	 * - -1 The board is not existed
	 * - -2 The channel is over range
	 * - -3 The channel is not existed
	 * - -4 Not ran before
	 * - -5 Vector running
	*/
	int APIENTRY GetChannelFailCount(BYTE bySlotNo, USHORT usChannel);
	/**
	 * @brief Get site in site number
	 * @param[in] usSiteNo The site number, 0xFFFF is all site
	 * @param[out] setSite The site in site number
	 * @param[in] bOnlyValidSite Only get the valid site
	 * @return Execute result
	 * - 0 Get site information successfully
	 * - -1 The site is over range
	 * - -2 The site is invalid
	*/
	inline int GetSiteInfo(USHORT usSiteNo, std::set<USHORT>& setSite, BOOL bOnlyValidSite = TRUE);	
	/**
	 * @brief Set the TMU unit channel
	 * @param[in] lpszPinGroup The pin group name
	 * @param[in] usSiteNo The site number
	 * @param[in] byUnitIndex The unit index
	 * @return Execute result
	 * - 0 Set the channel successfully
	 * - -1 Not load vector before
	 * - -2 The point of pin group is nullptr
	 * - -3 The pin group is not defined before
	 * - -4 The site number is over range
	 * - -5 The site is invalid
	 * - -6 The pin is not belongs to
	 * - -7 The pin group have more than one channel in a controller
	 * - -8 The unit index is over range
	 * - -9 No valid board existed
	*/
	int SetTMUUnitChannel(const char* lpszPinGroup, USHORT usSiteNo, BYTE byUnitIndex);
	/**
	 * @brief Get the unit channel
	 * @param[in] bySlotNo The slot number
	 * @param[in] usChannel The channel number
	 * @return The TMU unit connected to the channel
	 * - >=0 The TMU unit connected to the channel
	 * - -1 The board is not existed
	 * - -2 The channel is over range
	 * - -3 The channel is not existed
	 * - -4 The channel is not connected to any unit
	*/
	int GetTMUConnectUnit(BYTE bySlotNo, USHORT usChannel);
	/**
	 * @brief Set the TMU measurement parameter
	 * @param[in] lpszPinGroup The pin group name
	 * @param[in] usSiteNo The site number
	 * @param[in] bRaiseTriggerEdge Whether the trigger edge is raise
	 * @param[in] uHoldOffTime The hold off time, unit is ns
	 * @param[in] uHolfOffNum The hold off number
	 * @param[in] bySpecifedUnit Specified unit, -1 is not specifiled
	 * @return Execute result
	 * - 0 Set the parameter successfully
	 * - -1 Not load vector before
	 * - -2 The point of pin group is nullptr
	 * - -3 The pin group is not defined before
	 * - -4 The site number is over range
	 * - -5 The site is invalid
	 * - -6 The pin name is not belongs to
	 * - -7 The unit specified is over range
	 * - -8 Over 1 channels when specified unit
	 * - -9 The channel is not connect to the TMU unit
	 * - -10 No valid board existed
	*/
	int SetTMUParam(const char* lpszPinGroup, USHORT usSiteNo, BOOL bRaiseTriggerEdge, UINT uHoldOffTime, UINT uHolfOffNum, BYTE bySpecifiedUnit = -1);
	/**
	 * @brief Get the TMU parameter
	 * @param[in] bySlotNo The slot number
	 * @param[in] usChannel The channel number
	 * @param[out] bRaiseTriggerEdge Whether the trigger edge is raise
	 * @param[out] usHoldOffTime The hold off time, unit is ns
	 * @param[out] usHoldOffNum The hold off number
	 * @return Execute result
	 * - 0 Set the parameter successfully
	 * - -1 The board is not existed
	 * - -2 The channel is over range
	 * - -3 The channel is not existed
	 * - -4 The channel is not connect to the TMU unit
	*/
	int GetTMUParameter(BYTE bySlotNo, USHORT usChannel, BOOL& bRaiseTriggerEdge, USHORT& usHoldOffTime, USHORT& usHoldOffNum);
	/**
	 * @brief Get the TMU parameter
	 * @param[in] bySlotNo The slot number
	 * @param[in] byControllerIndex The controller index
	 * @param[in] byTMUUnitIndex The TMU unit index
	 * @param[out] bRaiseTriggerEdge Whether the trigger edge is raise
	 * @param[out] usHoldOffTime The hold off time, unit is ns
	 * @param[out] usHoldOffNum The hold off number
	 * @return Execute result
	 * - 0 Set the parameter successfully
	 * - -1 The board is not existed
	 * - -2 The controller index is over range
	 * - -3 The controller index is not existed
	 * - -4 The TMU unit index is over range
	*/
	int GetTMUUnitParameter(BYTE bySlotNo, BYTE byControllerIndex, BYTE byTMUUnitIndex, BOOL& bRaiseTriggerEdge, USHORT& usHoldOffTime, USHORT& usHoldOffNum);
	/**
	 * @brief Start TMU measurement
	 * @param[in] vecChannel The channel number
	 * @param[in] MeasMode The measurement mode
	 * @param[in] uSampleNum The sample number
	 * @param[in] dTimeout The timeout, the unit is ms
	 * @return Execute result
	 * - 0 Start TMU measurement successfully
	 * - -1 Not load vector before
	 * - -2 The point of pin group is nullptr
	 * - -3 The pin group is not defined before
	 * - -4 No valid site
	 * - -5 The pin name is not belongs to
	 * - -6 The channel is not connect to the TMU unit
	 * - -7 The measurement mode is not supported
	 * - -8 No valid board existed
	*/
	int TMUMeasure(const char* lpszPinGroup, TMU_MEAS_MODE MeasMode, UINT uSampleNum, double dTimeout);
	/**
	 * @brief Get the TMU measurement
	 * @param[in] bySlotNo The slot number
	 * @param[out] usChannel The channel number
	 * @param[out] MeasMode The measurement mode
	 * @param[out] uSampleNum The sample number
	 * @param[out] dTimeout The timeout
	 * @return Execute result
	 * - 0 Ge the TMU measurement successfully
	 * - -1 The board is not existed
	 * - -2 The channel is over range
	 * - -3 The channel is not existed
	 * - -4 The channel is not connect to the TMU unit
	*/
	int GetTMUMeasure(BYTE bySlotNo, USHORT usChannel, TMU_MEAS_MODE& MeasMode, UINT& uSampleNum, double& dTimeout);
	/**
	 * @brief Get the measure type
	 * @param[in] lpszPinName The pin name
	 * @param[in] MeasType The measurement type
	 * @param[in] nErrorCode The error code
	 * - 0 No error occur
	 * - -1 Not load vector before
	 * - -2 The point of pin name is nullptr
	 * - -3 The pin name is not defined
	 * - -4 The site is over range
	 * - -5 The site is invalid
	 * - -6 The pin is not belongs to
	 * - -7 The board is not existed
	 * - -8 The channel is not existed
	 * - -9 The channel is not connect to any TMU unit
	 * - -10 The measurement type is not supported
	 * - -11 The measurement type is not measured before
	 * - -12 The measurement is not stop in timeout
	 * - -16 The TMU measurement is timeout
	 * - -14 The bind unit of measurement is not stop in timeout
	 * - -15 The bind unit is timeout
	 * - -16 The edge measurement error
	 * @return The measurement result
	 * - != TMU_ERROR The measurement result
	*/
	double GetTMUMeasureResult(const char* lpszPinName, USHORT usSiteNo, TMU_MEAS_TYPE MeasType, int& nErrorCode);
	/**
	 * @brief Set the trigger out channel
	 * @param[in] lpszPinName The pin name
	 * @param[in] usSiteNo The site number
	 * @return Execute result
	 * - 0 Set the trigger out channel successfully
	 * - -1 Not load vector before
	 * - -2 The point of pin name is nullptr
	 * - -3 The pin name is not defined
	 * - -4 The site is over range
	 * - -5 The site is invalid
	 * - -6 The pin name is not belongs to
	 * - -7 The board is not existed
	 * - -8 The channel is not existed
	*/
	int SetTriggerOut(const char* lpszPinName, USHORT usSiteNo);
	/**
	 * @brief Update the mode of the board
	 * @param[in] bySlotNo The slot number of the board
	 * @return Execute result
	 * - 0 Update channel mode successfully
	 * - -1 The board is not existed
	*/
	int UpdateChannelMode(BYTE bySlotNo);
	/**
	 * @brief Whether load vector
	 * @param[in] strVectorFile The vector file name
	*/
	int IsLoadVector(std::string& strVectorFile);
	/**
	 * @brief Get the vector memory left
	 * @param[out] uBRAMStart The start line in BRAM
	 * @param[out] uDRAMStart The start line in DRAM
	*/
	void GetVectorMemoryLeft(UINT& uBRAMStart, UINT& uDRAMStart);
	/**
	 * @brief Get the pin group setted
	 * @param[out] vecPinGroup The pin group
	 * @return Execute result
	 * - 0 Get pin group successfully
	 * - -1 Not load vector before
	*/
	int GetPinGroup(std::vector<std::string>& vecPinGroup);
	/**
	 * @brief Is pin existed
	 * @param[in] lpszPin The pin name
	 * @return Whether the pin is existed
	*/
	BOOL IsPinExisted(const char* lpszPin);
	/**
	 * @brief Delete the board and controller unused
	 * @return Execute result
	 * - 0 Delte unused board successfully
	 * - -1 Not load vector file before
	*/
	int DeleteUnusedBoard();
	/**
	 * @brief Set the pin owned
	 * @param[in] lpszPinName The pin name
	 * @return Execute result
	 * - -1 Not load vector
	 * - -2 Pin name string is nullptr
	 * - -3 No pin in pin string
	 * - -4 The format is wrong
	 * - -5 Pin name is not defined
	*/
	int SetValidPin(const char* lpszPinName);
	/**
	 * @brief Enable add pin by function
	 * @param[in] bEnable Whether enable add pin by function
	 * @param[in] bClearVector Whether clear the vector information
	 * @return Execute result
	 * - 0 Enable or disable function successfully
	 * - -1 Load vector before
	*/
	int EnableAddPin(BOOL bEnable, BOOL bClearVector);
	/**
	 * @brief Add pin
	 * @param[in] lpszPinName The pin name
	 * @param[in] usPinNo The pin number
	 * @param[in] lpszChannel The channel of the pin
	 * @return Execute result
	 * - 0 Add pin successfully
	 * - -1 Not allow
	 * - -2 The pin name is nullptr
	 * - -3 The channel is nullptr
	 * - -4 The channel format is wrong
	 * - -5 The pin name is conflict
	*/
	int AddPin(const char* lpszPinName, USHORT usPinNo, const char* lpszChannel);
	/**
	 * @brief Clear preread vector
	*/
	void ClearPreread();
	/**
	 * @brief Enable saving fail selected
	 * @param lpszPinGroup The pin group name
	 * @return Execute result
	 * - 0 Enable or disable saving fail selected successfully
	 * - -1 Not load vector before
	 * - -2 The pin group point is nullptr
	 * - -3 The pin group is not defined
	 * - -4 No valid site
	 * - -5 No board existed
	*/
	int EnableSaveSelectedFail(const char* lpszPinGroup, BOOL bEnable);
private:
	/**
	 * @brief Bind the pin
	 * @param[in] setBindPin The pin need to be binded
	 * @param[in] setBindSite The site need to be binded
	 * @param[in] bSaveChannel Whether save the channel to member variable
	 * @return The target site will be followed
	 * - >=0 The target site
	 * - -1 No bind site or no bind pin
	 * - -2 The bind site count is 1, no need to bind
	 * - -3 Some controller used by more than one site
	 * - -4 The controller has been used by more than one site
	 * - -5 The channel offset to SITE_1 is not the multiples 16
	 * - -6 The site using more than one controller
	*/
	int Bind(const std::set<std::string>& setBindPin, const std::set<USHORT>& setBindSite, BOOL bSaveChannel = FALSE);
	/**
	 * @brief Clear bind information
	*/
	inline void ClearBind();
	/**
	 * @brief Delete memory of map parameter whose value in heap
	 * @param[in] mapParam The map parameter
	*/
	/**/
	template <typename Key, typename Value>
	void DeleteMemory(std::map<Key, Value>& mapParam);
	/**
	 * @brief Get the file name of vector information
	 * @param[out] strVectorInfoFile The vector file name
	*/
	inline void GetVectorInfoFile(std::string& strVectorInfoFile, const char* lpszVectorFile = nullptr);
	/**
	 * @brief Get the pin group information file
	 * @param[out] strFile The file with whole path
	*/
	inline void GetPinGroupInfoFile(std::string& strFile);
	/**
	 * @brief Save the information to file
	 * @param[in] lpszVectorFile The vector file name
	*/
	void SaveVectorInformation(const char* lpszVectorFile, const std::set<std::string>& vecFailSynPin);
	/**
	 * @brief Load vector information in vector file
	 * @param[in] lpszFileName The vector file name
	 * @param[in] strVectInfoFilePath The vector information of latest loaded
	 * @return Execute result
	 * - 0 Load vector information successfully
	 * - -1 The point of file name is nullptr
	 * - -2 The vector information file is not existed
	 * - -3 The vector information in vector information file is not same as the vector file
	*/
	int LoadVectorInformation(const char* lpszFileName, const std::string& strVectInfoFilePath, std::set<std::string>& vecFailSynPin, BOOL bCheckFile = TRUE, std::string* pstrFileName = nullptr);
	/**
	 * @brief Load the timeset in vector file
	 * @param[in] vecSite The site information 
	 * @param[in] mapTimeset The timeset information
	 * @return Execute result
	 * - 0 Load timeset successfully
	 * - -1 No board existed
	 * - -2 Get the timeset edge fail
	 * - -3 Set tiemset edge fail
	*/
	int LoadVectorFileTimeset(const std::vector<USHORT>& vecSite, std::map<BYTE, CTimeset*>& mapTimeset);
	/**
	 * @brief Classify the channel to board
	 * @param[in] vecPin The pin name
	 * @param[in] vecSite The site number
	 * @param[out] mapBoard The board channel information
	 * @return Execute result
	 * - 0 Get the board channel successfully
	 * - -1 No pin
	*/
	int GetBoardChannel(const std::vector<std::string>& vecPin, const std::vector<USHORT>& vecSite, std::map<BYTE, std::vector<USHORT>>& mapBoard);
	/**
	 * @brief Get the channel information of the pin in specific site
	 * @param[in] lpszPinName The pin name
	 * @param[in] usSiteNo The site number
	 * @param[out] Channel The channel information of the pin
	 * @return Execute result
	 * - 0 Get the channel information successfully
	 * - -1 Not load vector before
	 * - -2 The point of pin name is nullptr
	 * - -3 The pin name is not defined
	 * - -4 The site is over range
	 * - -5 The site is invalid
	 * - -6 The pin is not belongs to
	*/
	inline int GetBoardChannel(const char* lpszPinName, USHORT usSiteNo, CHANNEL_INFO& Channel);
	/**
	 * @brief Check whether the valid loaded in board is valid
	 * @param[in] lpszVectorFileName The vector file name
	 * @param[in] mapUseSlot The slot number of used board
	 * @return Check result
	 * - TRUE The vector is valid
	 * - FALSE The vector is invalid
	*/
	inline BOOL IsVectorValid(const char* lpszVectorFileName, const std::vector<BYTE>& mapUseSlot);
	/**
	 * @brief Extract pin name from pin name string
	 * @param[in] lpszPinNameList Pin name string
	 * @param[out] setPin The pin name in pin name string
	 * @return Execute result
	 * - 0 Extract pin name successfully
	 * - -1 The pin name string is nullptr
	 * - -2 No pin in pin name string
	 * - -3 The format of pin name string is wrong
	 * - -4 Some pin is not defined in vector 
	 * - -5 Some pin is not belongs to
	*/
	inline int ExtractPinName(const char* lpszPinNameList, std::set<std::string>& setPin);
	/**
	 * @brief Get the channel of pin
	 * @param[in] setPinName The pin name
	 * @param[in] usSiteNo The site number, 0xFFFF is all site
	 * @param[in] bOnlyValidSite Whether only get channel of valid site
	 * @return Execute result
	 * - 0 Get the channel successfully
	 * - -1 The site number is over range
	 * - -2 The site is invalid
	*/
	inline int GetChannel(std::set<std::string>& setPinName, USHORT usSiteNo = 0xFFFF, BOOL bOnlyValidSite = TRUE);
	/**
	 * @brief Get the channel in the Pin Group
	 * @param[in] lpszPinGroup The pin group name
	 * @param[in] usSiteNo The site number, 0xFFFF is all site
	 * @param[in] bOnlyValidSite Whether only get channel of valid site
	 * @return Execute result
	 * - 0 Get the channel successfully
	 * - -1 Not load vector before
	 * - -2 The pin group point is nullptr
	 * - -3 The pin group is not defined before
	 * - -4 The site number is over range
	 * - -5 The site is invalid
	 * - -6 The pin name is not belongs to
	*/
	inline int GetChannel(const char* lpszPinGroup, USHORT usSiteNo = 0xFFFF, BOOL bOnlyValidSite = TRUE, std::set<std::string>* psetPin = nullptr);
	/**
	 * @brief Classify the channel to board channel
	 *param[in] vecChannel The channel information with slot
	 * @return Execute result
	 * - 0 Classify channel successfully
	*/
	inline int ClassifyChannel(const std::vector<CHANNEL_INFO>& vecChannel);
	/**
	 * @brief Get the fail line number
	 * @param[in] bySlotNo The slot number
	 * @param[in] usChannel The channel number of board
	 * @param[in] uGetMaxFailCount The maximum fail count will be gotten
	 * @param[out] vecLineNo The fail line number of last running
	 * @param[out] pnLastCertainPassNo The point of the variable for getting the line number of the certail pass in latest ran
	 * @return Execute result
	 * - 0 Get line number successfully
	 * - -1 The board is not existed
	 * - -2 The channel is not existed
	 * - -3 Not ran vector before
	 * - -4 Vector running
	 * - -5 The vector is not ran by user
	*/
	int GetFailLineNo(BYTE bySlotNo, USHORT usChannel, UINT uGetMaxFailCount, std::vector<int>& vecLineNo, int* pnLastCertainPassNo = nullptr);
	/**
	 * @brief Get the all board channel in vector file
	 */
	inline void GetAllBoardChannel();
	/**
	 * @brief Initialize the site information from vector file
	 * @param[in] bAddAlarm Whether add alarm
	 * @return Execute result
	 * - 0 Initialize site successfully
	 * - -1 The channel number is over range
	*/
	inline int InitSite(BOOL bAddAlarm = TRUE);
	/**
	 * @brief Save the pin group information to file
	 * @param[in] The pin group name
	 * @return Execute result
	 * - 0 Save pin group information successfully
	 * - -1 The point of pin group name is nullptr
	 * - -2 The pin group is not defined before
	 * - -3 Open file fail
	*/
	inline int SavePinGroupInfo(const char* lpszPinGroupName);
	/**
	 * @brief Get the pin name in the pin group
	 * @param[in] lpszPinGroup The pin group
	 * @param[out] setPinName The pin name in the pin group
	 * @return Execute result
	 * - 0 Get the pin name successfully
	 * - -1 Not load vector file before
	 * - -2 The point of the pin group is nullptr
	 * - -3 The pin group is not defined
	 * - -4 The pin name is not owned by the class
	*/
	inline int GetPinName(const char* lpszPinGroup, std::set<std::string>& setPinName);
	/**
	 * @brief Get the pin name and site number of the channel
	 * @param[in] Channel The channel number
	 * @param[out] strPinName The pin name
	 * @return The site number
	 * - >=0 The site number
	 * - -1 Can't find the channel
	*/
	inline int GetChannelPin(const CHANNEL_INFO& Channel, std::string& strPinName);
	/**
	 * @brief Get the shield channel
	 * @param[in] lpszFunction The function be shield
	 * @param[out] mapShiledChannel The channel be shield
	*/
	inline void GetShieldChannel(const char* lpszFunction, std::map<BYTE, std::set<USHORT>>& mapShieldChannel);
	/**
	 * @brief Check the PMU clamp status
	 * @param[in] bySlotNo The slot number
	 * @param[in] vecChannel The channel will be checked
	 * @param[in] psetShieldChannel The shield channel
	*/
	inline void CheckPMUClampStatus(BYTE bySlotNo, const std::vector<USHORT>& vecChannel, const std::set<USHORT>* psetShieldChannel);
	/**
	 * @brief Set the fail synchronous
	 * @param[in] bFailSyn Whether set fail synchronous
	 * - TRUE The fail synchronized according to the channel distribution
	 * - FALSE Fail signal not synchronized
	*/
	void SetFailSyn(const std::set<std::string>& setFailSynPin);/**
	 * @brief Get pin group section in configuration fil
	 * @param[out] strSection The section name
	*/
	inline void GetPinGroupSection(std::string& strSection);
private:
	BOOL m_bLoadVector;///<Whether load vector before
	BOOL m_bDeleteChannelUnused;///<Whether delete channel unused
	BOOL m_bVectorShared;///<Whether using vector shared
	int m_nBRAMLeftStartLine;///<The start line in BRAM left
	int m_nDRAMLeftStartLine;///<The start line in DRAM left
	std::map<BYTE, CBoard*> m_mapBoard;///<The board information, The key is slot number, the value is point of the class
	CDriverAlarm* m_pAlarm;///<The alarm class
	CVectorInfo m_VectorInfo;///<The vector information
	CSite m_Site;///<The site information
	std::map<std::string, CPin*> m_mapPin;///<The pin information, key is pin name, and the value is point of key
	std::map<std::string, BYTE> m_mapTimeset;///<The timeset information, key is timeset name, value is timeset index
	std::map<std::string, CPinGroup*> m_mapPinGroup;///<The pin group information, key is pin group name, the value is point of CPinGroup class
	CClassifyBoard m_ClassifyBoard;///<The channel in the board of latest query, like query pin group or pin name
	std::string m_strStopLabel;///<The stop line label of latest stop
	int m_nLatestStartLine;///<The start line number of latest ran
	int m_nLatestStopLine;///<The stop line number of latest ran
	std::string m_strWriteGetPinGroup;///<The pin name used in write wave data or get capture data
	USHORT m_usDataSiteNo;///<The site number needed to write data
	BOOL m_bSetSiteData;///<Whether has set wave in bind
	int m_nLineCount[2];///<The line count of vector in BRAM and DRAM
	std::map<int, CVectorInfo::LINE_BLOCK> m_mapLineInfo;///<The line information of line data
	std::string m_strLatestRanPinGroup;///<The pin group in latest ran
	int m_nCaptureStartOffset;///<The capture start line offset to latest ran
	int m_nCaptureLineCount;///<The capture stop line count
	BOOL m_bWaitRun;///<Whether wait run command
	BOOL m_bVectorBind;///<Whether the vector support binded
	STS_PROGRESS_INFO_FUN m_pProgressInfo;///<The point of testui's progress information
	STS_PROGRESS_FUN m_pProgressStep;///<The point of testui's progress step information
	std::string m_strFileName;///<The vector file name
	std::set<std::string> m_setPinUnowned;///<The pin name owned
	BOOL m_bAllowAddPin;///<Whether allow adding pin by function
};