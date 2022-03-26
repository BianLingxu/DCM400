/*  ///////////////////////////////////////////////////////////////////////
//  SM8213h
//  header file for DCMdll
//  API to access DCM driver(new version!)
//  (C)copyright? HFTC 2014
//  By LCchen
//////////////////////////////////////////////////////////////////////// */
#ifndef _SM8213_H_
#define _SM8213_H_
#include <windows.h>
#include "FlashInformation.h"
#include <vector>
#include <map>
#include <string>
#include "HardwareInfo.h"
using namespace std;

#define EXECUTE_SUCCESS			0
#define VECTOR_FILE_NOT_LOADED	-1
#define PIN_GROUP_ERROR			-2
#define TIMESET_ERROR			-3
#define RATE_ERROR				-4
#define TIME_ERROR				-5
#define START_LABEL_ERROR		-6
#define STOP_LABEL_ERROR		-7
#define INSTRUCTION_ERROR 		-8
#define PIN_NAME_ERROR			-9
#define	SITE_ERROR				-10
#define DATA_LENGTH_ERROR		-11
#define NO_FAIL_LINE			-12
#define WRITE_FILE_ERROR		-13
#define	PIN_LEVEL_ERROR			-14
#define	OFFSET_ERROR			-15
#define BOARD_NOT_INSERT_ERROR	-16
#define PMU_MEASURE_ERROR		-17
#define FLASH_ERROR				-18//Internal use only for reading flash
#define PIN_CURRENT_ERROR		-18
#define START_LABLE_AFTER_END	-19
#define ADDR_EXCEED				-20
#define SECTION_NUM_EXCEED		-21
#define PIN_GROUP_NAME_CONFLICT	-22
#define PIN_GROUP_FORMAT_ERROR	-23
#define OPERAND_ERROR			-24
#define ARRAY_LENGTH_NOT_ENOUGH	-25
#define NOT_NEW_VECTOR			-26//Internal use 
#define VECTOR_LINE_OVER_RANGE	-26
#define CONTROL_NO_EXCEED		-27//Internal use 
#define PARAM_NULLPTR			-27
#define	ALLOCATE_MEMORY_FAIL	-28
#define NO_TYPE_RELAY			-29//Internal use 
#define FUNCTION_USE_ERROR		-29
#define NO_TRIGGER_SIGNAL		-30
#define CROSS_LABEL_RUNNING		-31
#define UNKNOWN_ERROR			-888
#define SITE_INVALID            -32
#define VECTOR_NOT_RAN          -33
#define VECTOR_RUNING           -34
#define CHANNEL_NOT_EXISTED     -35
#define CLAMP_VALUE_ERROR       -36
#define VECTOR_NOT_IN_LAST_RAN  -37
#define TMU_CHANNEL_OVER_RANGE  -38
#define TMU_UNIT_OVER_RANGE     -39
#define TMU_CHANNEL_NOT_CONNECT -40
#define FAIL_LINE_NOT_SAVE      -41
#define CAPTURE_NOT_ALL_SAVE    -42
#define PIN_NOT_BELONGS         -43
#define GET_CALIBRATION_FAIL    -44
#define MORE_ONE_CHANNEL_BIND_UINT   -45
#define CAPTURE_NOT_SUPPORTTED  -46
enum DCM_IRANG
{
	DCM_2uA = 0,
	DCM_20uA,
	DCM_200uA,
	DCM_2mA,
	DCM_32mA
};
enum DCM_LEVEL_TYPE
{
	DCM_VIH,
	DCM_VIL,
	DCM_VOH,
	DCM_VOL,
	DCM_IOH,
	DCM_IOL,
	DCM_VT,
	DCM_CLAMP_HIGH,
	DCM_CLAMP_LOW

};

typedef struct dcm_calibration_data
{
	// 各档位(校准项目)的增益和零位(各功能校准数据顺序：小量程档→大量程档)
	float dcm_DVH_gain[1];
	float dcm_DVH_offset[1];
	float dcm_DVL_gain[1];
	float dcm_DVL_offset[1];
	float dcm_FV_gain[1];
	float dcm_FV_offset[1];
	float dcm_FI_gain[5];
	float dcm_FI_offset[5];
	float dcm_MV_gain[1];
	float dcm_MV_offset[1];
	float dcm_MI_gain[5];
	float dcm_MI_offset[5];

	dcm_calibration_data()
	{
		int i = 0;
		dcm_DVH_gain[0] = 1;
		dcm_DVH_offset[0] = 0;
		dcm_DVL_gain[0] = 1;
		dcm_DVL_offset[0] = 0;
		dcm_FV_gain[0] = 1;
		dcm_FV_offset[0] = 0;
		dcm_MV_gain[0] = 1;
		dcm_MV_offset[0] = 0;
		for (i = 0; i < 5;++i)
		{
			dcm_FI_gain[i] = 1;
			dcm_FI_offset[i] = 0;
			dcm_MI_gain[i] = 1;
			dcm_MI_offset[i] = 0;
		}
	}
}DCM_CAL_DATA;

namespace DCMINFO
{
	enum MODULEINFO
	{
		MODULE_NAME = 0,
		MODULE_SN,
		MODULE_HDREV,
	};
	enum CALINFO
	{
		METER_SN = 0,
		CALBD_SN,
		CALBD_HD_REV,
		SOTEWARE_REV,
		ATE_SN,
		BRIDGEBD_SN,
	};
}

enum DCM_CAL_MODE
{
	DCM_CAL_DVH = 0,
	DCM_CAL_DVL,
	DCM_CAL_FV,
	DCM_CAL_FI,
	DCM_CAL_MV,
	DCM_CAL_MI
};


#ifdef __cplusplus
extern "C"
{
#endif //__cplusplus

    //for config and check
    struct BOARDINFO;

    void APIENTRY dcm_set_config_info(short boardcnt, BOARDINFO* pAllBoardInfo);
    /**
     * @brief The self-check program of DCM
     * @param[in] lpszFileName The self-check log file
     * @param[in] byBoardNo The board number of DCM
     * @param[out] pnCheckResult The check result of each channel
     * @return Check result
     * - 0 Check fail
     * - 1 Check pass
    */
    BOOL APIENTRY dcm_selfcheck(char* lpszFileName, BYTE byBoardNo, int* pnCheckResult);
    /**
     * @brief Set the calibration relay
     * @param[in] bySlotNo The slot number
     * @param[in] byChannel The channel number
     * @param[in] byRelayStatus The relay status, 1 is connect and 0 is disconnect
    */
    void APIENTRY dcm_set_cal_relay(BYTE bySlotNo, BYTE byChannel, BYTE byRelayStatus);
	/**
	 * @brief Set the calibration data to channel without write flash
	 * @param[in] uGlobalChannel The channel number of global
	 * @param[in] CalMode The calibration mode
	 * @param[in] byRange The current mode
	 * @param[in] fSlope The slope gain
	 * @param[in] fOffset The offset
	 * @return Execute result
     * - 0 Set calibration data successfully
     * - -1 The current range is over range
	*/
	int APIENTRY dcm_cal_result_to_gVariable(UINT uGlobalChannel, DCM_CAL_MODE CalMode, BYTE byRange, float fSlope, float fOffset);
    /**
     * @brief Get the slot number of board
     * @param[in] sBoardNo The board number
     * @return Slot number of board
     * - >0 The slot number of board
     * - 0 The board is not existed
    */
    BYTE APIENTRY dcm_get_board_slot(short sBoardNo);    
    /**
     * @brief Disconnect all relay
     * @param[in] bySlotNo The slot of the board
    */
    void APIENTRY dcm_relay_init(BYTE bySlotNo);
    /**
     * @brief Enable calibration
     * @param[in] bEnable Whether enable calibration
     * @param[in] byBoardNo The board number
     * @return Execute result
     * - 0 Enable calibration successfully
     * - -1 Read flash fail
    */
    int APIENTRY dcm_calibration(BOOL bEnable, BYTE byBoardNo = -1);
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
     * - -2 The pin group is not defined before or its point is nullptr
     * - -14 The pin level is over range
	 * - -16 No valid board existed
     * - -32 No valid site
	 * - -43 The pin group include the pin which is not belongs to current instance
    */
    int APIENTRY dcm_SetPinLevel(const char* lpszPinGroup, double dVIH, double dVIL, double dVOH, double dVOL);
    /**
     * @brief Set the PPMU mode
     * @param[in] lpszPinGroup The pin group
     * @param[in] byMode PMU mode
     * @param[in] dValue The set value
     * @param[in] byIRange The current range
	 * @return Execute result
     * - 0 Set PMU successfully
	 * - -1 Not load vector before
	 * - -2 The pin group is not defined before or its point is nullptr
	 * - -16 No board existed
	 * - -35 The channels in the pin group are not existed
	 * - -43 The pin group include the pin which is not belongs to current instance
    */
    int APIENTRY dcm_setppmu(const char* lpszPinGroup, BYTE byMode, double dValue, BYTE byIRange);
    /**
     * @brief Set the PPMU mode
     * @param[in] lpszPinGroup The pin group
     * @param[in] byMode PMU mode
     * @param[in] dValue The set value
     * @param[in] byIRange The current range
     * @return Execute result
     * - 0 Set PMU successfully
     * - -1 Not load vector before
     * - -2 The pin group is not defined before or its point is nullptr
     * - -10 The site is over range
	 * - -16 No board existed
	 * - -32 The site is invalid
	 * - -35 The channels in the pin group are not existed
	 * - -43 The pin group include the pin which is not belongs to current instance
    */
    int APIENTRY dcm_setppmu_single_site(const char* lpszPinGroup, USHORT usSiteNo, BYTE byMode, double dValue, BYTE byIRange);
    /**
     * @brief Initialize MCU function
     * @param[in] lpszPinGroup The pin group whose MCU will be initialized
     * @return Execute result
     * - 0 Initialize MCU successfully
     * - -1 Not Load vector before
     * - -2 The pin group is not defined before or its point is nullptr
	 * - -16 No valid board existed
     * - -32 No valid site
	 * - -43 The pin group include the pin which is not belongs to current instance
    */
    int APIENTRY dcm_initmcu(const char* lpszPinGroup);
    /**
	 * @brief Initialize PMU function
	 * @param[in] lpszPinGroup The pin group whose PMU will be initialized
     * @return Execute result
     * - 0 Initialize PMU successfully
	 * - -1 Not Load vector before
	 * - -2 The pin group is not defined before or its point is nullptr
	 * - -16 No valid board existed
     * - -32 No valid site
	 * - -43 The pin group include the pin which is not belongs to current instance
    */
    int APIENTRY dcm_initppmu(const char* lpszPinGroup);
    /**
     * @brief Get the MCU run result of specific site
     * @param[in] usSiteNo The specific site index
     * @return MCU result
     * - 0 The MCU result is PASS
     * - 1 The MCU result is FAIL
     * - -1 Not load vector file
	 * - -10 Site is over range
     * - -16 No valid board existed
	 * - -32 The site is invalid
	 * - -33 Not ran vector before
	 * - -34 Running vector now
     * - -35 Some channels are not existed
    */
    int APIENTRY dcm_getmcuresult(USHORT usSiteNo);
    /**
	 * @brief Get the compare data of the channel
	 * @param[in] lpszPinName The pin name
	 * @param[in] lpszStartLabel The start label
	 * @param[in] usSiteNo The site number
	 * @param[in] ulOffset The start line offset to the label
     * @param[in] nLineCount The line count of the compare data
     * @param[out] ulData The compare data
     * @return Execute result
     * - 0 Get capture data successfully
     * - -1 Not load vector before
     * - -6 The start label is not existed in vector file or its point is nullptr
     * - -9 The pin name is not existed in vector file
	 * - -10 The site is over range
     * - -15 The offset is over range
	 * - -16 The board is not existed
     * - -32 The site is invalid
     * - -33 The vector is not ran before
     * - -34 The vector is running now
     * - -35 The channel is not existed
     * - -37 The vector of latest ran is not from start label
	 * - -38 The channel number is over range
	 * - -41 The fail line number not saved
	 * - -43 The pin is not belongs to current instance
    */
    int APIENTRY dcm_getcapturedata(const char* lpszPinName, const char* lpszStartLabel, USHORT usSiteNo, ULONG ulOffset, int nLineCount, ULONG& ulData);
    /**
     * @brief Load vector from file
     * @param[in] lpszFileName The vector file name
     * @param[in] bReload Whether reload vector forcibly
     * @return Execute result
     * - 0 Load vector fail successfully
     * - -1 Load vector fail
    */
    int APIENTRY dcm_loadvectorfile(const char* lpszFileName, BOOL bReload);
    /**
     * @brief Stop vector
     * @param[in] lpszPinGroup The pin group name
     * @return Execute result
     * - 0 Stop vector successfully
     * - -1 Not load vector before
     * - -2 The pin group is not defined before or its point is nullptr
	 * - -16 No valid board existed
     * - -32 No valid site
	 * - -43 The pin group include the pin which is not belongs to current instance
    */
    int APIENTRY dcm_stopvector(const char* lpszPinGroup);
    /**
     * @brief Connect function relay
     * @param[in] lpszPinGroup The pin group in which the channels' relay will be connected
     * @return Execute result
     * - 0 Connect function relay successfully
     * - -1 Not load vector before
     * - -2 The pin group is not defined before or its point is nullptr
	 * - -16 No valid board existed
     * - -32 No valid site
	 * - -43 The pin group include the pin which is not belongs to current instance
    */
    int APIENTRY dcm_connect(const char* lpszPinGroup);
    /**
	 * @brief Disconnect function relay
	 * @param[in] lpszPinGroup The pin group in which the channels' relay will be disconnected
     * @return Execute result
     * - 0 Disconnect function relay successfully
     * - -1 Not load vector before
     * - -2 The pin group is not defined before or its point is nullptr
	 * - -16 No valid board existed
     * - -32 No valid site
	 * - -43 The pin group include the pin which is not belongs to current instance
    */
    int APIENTRY dcm_disconnect(const char* lpszPinGroup);
    /**
     * @brief Save fail map
     * @param[in] ulMaxErrline The maximum error line to gotten
     * @return Execute result
	 * - 0 Save fail map fail
     * - -1 Not load vector before
	 * - -28 Allocate memory fail
     * - -33 Not ran vector before
	 * - -34 Vector running
	 * - -41 The fail line number can't be gotten for the fail memory full occupied by other channels'
    */
    int APIENTRY dcm_SaveFailMap(ULONG ulMaxErrLine);
    /**
     * @brief Close vector file and reset memory
     * @return Execute result
     * - 0 Successfully
    */
    int APIENTRY dcm_CloseFile();
    /**
     * @brief Run vector of the channel in pin group
     * @param[in] lpszPinGroup The pin group name
     * @param[in] lpszStartLabel The start label of run vector
     * @param[in] lpszStopLabel The stop label of run vector
     * @param[in] bWaitFinish Whether wait the vector stop
     * @retrurn Execute result
     * - 0 Run vector successfully
     * - -1 Not load vector successfully
     * - -2 The pin group is not defined before or its point is nullptr
     * - -6 The start label is not defined in vector file or its point is nullptr
     * - -7 The stop label is not defined in vector file or its point is nullptr
     * - -16 No valid board existed
	 * - -19 The start label is after stop label
     * - -32 No valid site
	 * - -43 The pin group include the pin which is not belongs to current instance
    */
    int APIENTRY dcm_runvectorwithgroup(const char* lpszPinGroup, const char* lpszStartLabel, const char* lpszStopLabel, BOOL bWaitFinish);
    /**
     * @brief Get the MCU run result of specific pin
     * @param[in] lpszPinName The pin name
     * @param[in] usSiteNo The site number
     * @return MCU run result
     * - 0 MCU run PASS
     * - 1 MCU run FAIL
     * - -1 Not load vector before
     * - -9 The pin number is not defined or its point is nullptr
     * - -10 The site number is over range
     * - -16 No valid board existed
	 * - -32 The site is invalid
	 * - -33 Not ran vector before
     * - -34 Vector running now
	 * - -35 The channel is not existed
	 * - -43 The pin is not belongs to current instance
    */
    int APIENTRY dcm_getmcupinresult(const char* lpszPinName, USHORT usSiteNo);
    /**
     * @brief Get the running status of the pin
     * @param[in] lpszPinName The pin name
     * @param[in] usSiteNo The site number
	 * @return Running status
	 * - 0 Running
	 * - 1 Stop running
	 * - 2 Not ran
     * - -1 Not load vector before
     * - -9 The pin name is not defined or its point is nullptr
     * - -10 The site is over range
     * - -16 The board or the channel is not existed
     * - -32 The site is invalid
	 * - -35 The channel is not existed
	 * - -43 The pin is not belongs to current instance
    */
    int APIENTRY dcm_getmcupinrunstatus(const char* lpszPinName, USHORT usSiteNo);
    /**
	 * @brief Get the fail count of pin
	 * @param[in] lpszPinName The pin name
	 * @param[in] usSiteNo The site number
     * @param[out] ulFailCount The fail count of the pin
     * @return Execute result
     * - 0 Get fail count successfully
	 * - -1 Not load vector file
	 * - -9 The pin name is not defined or its point is nullptr
	 * - -10 The site is over range
	 * - -16 The board is not existed
	 * - -32 Site is invalid
	 * - -33 Not ran vector before
     * - -34 Vector running
	 * - -35 The channel is not existed
	 * - -43 The pin is not belongs to current instance
    */
    int APIENTRY dcm_GetFailCount(const char* lpszPinName, USHORT usSiteNo, ULONG& ulFailCount);
    /**
	 * @brief Get the stop label
	 * @param[in] usSiteNo The site number
     * @return The stop label name
     * - != nullptr The stop label name
     * - nullptr Not find label
    */
    char* APIENTRY dcm_GetStopLabel(USHORT usSiteNo);
    /**
     * @brief Get the stop line number
	 * @param[in] usSiteNo The site number
	 * @param[in] ulStopAddress The stop line number
     * @return Execute result
	 * - 0 Get the stop line number successfully
	 * - -1 Not load vector file
	 * - -10 The site is over range
	 * - -12 No fail line in latest ran
	 * - -16 The board or the channel is not existed
     * - -32 The site is invalid
	 * - -33 Not ran vector before
	 * - -34 Vector running
     * - -35 The channel is not existed
    */
    int APIENTRY dcm_GetStopLineNo(USHORT usSiteNo, ULONG& ulStopAddress);
    /**
     * @brief Get the line count of running
     * @param[in] lpszPinName The pin name
     * @param[in] usSiteNo The site count
     * @param[out] ulLineCount The line count of the vector running
     * @return Execute result
	 * - 0 Get the line count successfully
	 * - -1 Not load vector before
	 * - -9 The pin is not existed in vector file or its point is nullptr
	 * - -10 The site is over range
	 * - -16 The board is not existed
	 * - -32 The site is invalid
	 * - -35 The channel is not existed
	 * - -38 The channel is over range
	 * - -43 The pin is not belongs to current instance
    */
    int APIENTRY dcm_GetRunLineCount(const char* lpszPinName, USHORT usSiteNo, ULONG& ulLineCount);
    /**
     * @brief Get the first fail line number of pin
     * @param[in] lpszPinName The pin name
     * @param[in] usSiteNo The site number
     * @param[out] ulFirstFailLine The fail count of the pin
	 * @return Execute result
	 * - 0 Get fail count successfully
	 * - -1 Not load vector file
	 * - -9 The pin name is not defined or its point is nullptr
	 * - -10 The site is over range or invalid
     * - -12 No fail line in latest ran
	 * - -16 The board or the channel is not existed
	 * - -33 Not ran vector before
	 * - -34 Vector running
	 * - -35 The channel is not existed
	 * - -41 The fail line number can't be gotten for the fail memory full occupied by other channels'
	 * - -43 The pin is not belongs to current instance
    */
    int APIENTRY dcm_GetFirstFailLineNo(const char* lpszPinName, USHORT usSiteNo, ULONG& ulFirstFailLine);
    /**
     * @brief Modify the wave data in BRAM or DRAM
     * @param[in] lpszPinGroup The pin group whose wave data will be modified
     * @param[in] startLabel The base start label which will be modified start
     * @param[in] usSiteNo The site number
     * @param[in] ulOffset The line offset to start label
     * @param[in] nCount The line number will be modified
     * @param[in] ulWaveData The wave which will be written to BRAM or DRAM
     * @return Execute result
     * - 0 Write wave data successfully
     * - -1 The vector file is not loaded
     * - -2 The pin group is not defined before or its point is nullptr
     * - -6 The start label is wrong or its point is nullptr
     * - -10 The site number is wrong
     * - -15 The offset is over range
     * - -16 No valid board existed
     * - -26 The line count is over range
     * - -28 Allocate memory fail
     * - -32 The site is invalid
	 * - -35 The channel is not existed
	 * - -43 The pin group include the pin which is not belongs to current instance
    */
    int APIENTRY dcm_WriteWaveData(const char* lpszPinGroup, const char* lpszStartLabel, USHORT usSiteNo, ULONG ulOffset, int nCount, ULONG ulWaveData);
    /**
     * @brief Get the fail line's index offset to the start line of the latest run vector
     * @param[in] lpszPinName The pin name defined in vector file
     * @param[in] usSiteNo The site no
     * @param[in] usSiteNo The site number
     * @param[out] dwData The array which will save the index of fail line
     * @return Execute result
     * - 0 Get the fail line successfully
     * - -1 The vector file is not loaded
     * - -9 The pin name is not defined
     * - -10 The site number is over range
     * - -16 The board is not existed
	 * - -32 The site is invalid
	 * - -35 The channel is not existed
	 * - -41 The fail line number can't be gotten for the fail memory full occupied by other channels'
	 * - -43 The pin is not belongs to current instance
    */
    int APIENTRY dcm_GetFailLineNo(const char* lpszPinName, USHORT usSiteNo, int nCount, DWORD dwData[]);
    /**
     * @brief Set VT mode and the voltage value
     * @param[in] lpszPinGroup The pin group whose will set VT volt and mode
     * @param[in] dVTVoltValue The volt value of VT, the range is -15~6
     * @param[in] byMode The VT mode will be set
     * @return Execute result
     * - 0 Set VT successfully
     * - -1 The vector file is not loaded
	 * - -2 The pin group is not defined or its point is nullptr
     * - -8 The VT mode is error
     * - -14 VT value is over range
	 * - -16 No valid board existed
	 * - -32 No Valid site
	 * - -43 The pin group include the pin which is not belongs to current instance
    */
    int APIENTRY dcm_setDriveVTorTristate(const char* lpszPinGroup, double dVTVoltValue, BYTE byMode);
    /**
     * @brief Write calibration data into flash by Controller
     * @param[in] bySlotNo The slot number of the board whose calibration data will be written
     * @param[in] byControllerIndex The Controller whose channel's calibration data will be written
     * @param[in] pCalData The array of the calibration data
     * @param[in] byElementCount The number of the data array
     * @return Execute result
     * - 0 Write successfully
     * - -1 Can't write calibration data into flash
     * - -16 The board is not inserted
     * - -18 The flash error
     * - -27 The point of calibration data is nullptr
    */
    int APIENTRY dcm_write_flash_calibration_data_by_Control(BYTE bySlotNo, BYTE byControllerIndex, DCM_CAL_DATA* pCalData, BYTE byElementCount);
    /**
     * @brief Read calibration data from flash by Controller
     * @param[in] bySlotNo The slot number of the board whose calibration data will be read
     * @param[in] byControllerIndex The Controller number whose channel's calibration data will be read
     * @param[out] pCalData The array of the calibration data
     * @param[in] byElementCount The number of the data array
     * @return Execute result
     * - 0 Read all channel's calibration successfully
     * - 0x01 The channel 1's has no calibration data
    */
    DWORD APIENTRY dcm_read_flash_calibration_data_by_control(BYTE bySlotNo, BYTE byControllerIndex, DCM_CAL_DATA* pCalData, BYTE byElementCount);
    /**
     * @brief Write calibration information to flash by Controller
     * @param[in] bySlotNo The board number of the board whose initialization information will be written
     * @param[in] byCtrlNo The Controller number of the board whose calibration information data will be written
     * @param[out] pCalInfo The array of the calibration information which need to be written into flash
     * @param[out] byChannelStatus The variable saved 16 channels which need save calibration information
     * @param[in] nElementCount The elements of the byChannelStatus
     * @return Execute result
     * - 0 Read all channel's calibration successfully
     * - -1 The calibration information is written fail
     * - -16 The board is not inserted
    */
    int APIENTRY dcm_SetCalibrationInfoByCtrl(BYTE bySlotNo, BYTE byCtrlNo, STS_CALINFO* pCalInfo, BYTE* byChannelStatus, int nElementCount);
    /**
     * @brief Read calibration information from flash by Controller
     * @param[in] bySlotNo The board number of the board whose calibration information will be read
     * @param[in] byCtrlNo The Controller number whose channel's calibration information will be read
     * @param[out] calData The array of the calibration information
     * @return Execute result
     * - 0 Read all channel's calibration successfully
     * - -1 Read the calibration information fail
     * - -16 The board is not inserted
    */
    int APIENTRY dcm_GetCalibrationInfoByCtrl(BYTE bySlotNo, BYTE byCtrlNo, STS_CALINFO* pCalibrationInfo);
    /**
     * @brief Write hard information to flash
     * @param[in] byBoardNo The board number of the board whose initialization information will be read
     * @param[in] pHardInfo The variable which will save the hard information written
     * @param[in] nModuleNum The number of the module information saved to flash
     * @return Execute result
     * - 0 Write successfully
     * - -1 Write fail
     * - -16 The board is not existed
    */
    int APIENTRY dcm_set_hardinfo_to_flash(BYTE byBoardNo, STS_HARDINFO* pHardInfo, int nModuleNum);
    /**
     * @brief Read hard information from flash
     * @param[in] byBoardNo The board number of the board whose initialization information will be read
     * @param[out] pHardInfo The variable which will save the hard information
     * @param[in] nElementCount The array length of the hardinfo
     * @param[in] nModuleCount The number of the module saved in flash
     * @return Execute result
     * - 0 Read successfully
     * - -1 Read fail
     * - -2 The array of hard info is too small
     * - -18 The board number exceed maximum
    */
    int APIENTRY dcm_get_hardinfo_from_flash(BYTE byBoardNo, STS_HARDINFO* pHardInfo, int nElementCount, int& nModuleCount);
    /**
     * @brief Read the module information by board No, including SN,name and hard version
     * @param[in] bySlotNo The slot number which the module belongs to
     * @param[out] lpszInfo The variable which will save the module information
     * @param[in] nInfoSize The size of the variable of info
     * @param[in] SelInfo The module information be gotten
     * @param[in] Module The module type be gotten
     * @return Execute result
     * - 0 Get the module information successfully
     * - -16 The board is not inserted
     * - -27 The point of parameter is nullptr
    */
    int APIENTRY dcm_GetModuleInfoByBoard(BYTE bySlotNo, char* lpszInfo, int nInfoSize, DCMINFO::MODULEINFO SelInfo, STS_BOARD_MODULE Module);
    /**
     * @brief Read the module information by pin Name and site number, including SN,name and hard version
     * @param[in] lpszPinName The pin name defined in vector file
     * @param[in] bySiteNo The site number
     * @param[out] lpszInfo The variable which will save the module information
     * @param[in] nInfoSize The size of the variable of info
     * @param[in] SelInfo The module information be gotten
     * @param[in] module The module type be gotten
     * @return Execute result
	 * - 0  Get the module information successfully
	 * - -1 Not load vector file
	 * - -9 The pin name is not defined or its point is nullptr
	 * - -10 The site is over range or invalid
	 * - -16 The board or the channel is not existed
     * - -27 The point of module information is nullptr
	 * - -32 The site is invalid
	 * - -43 The pin is not belongs to current instance
    */
    int APIENTRY dcm_GetModuleInfoByPin(const char* lpszPinName, USHORT usSiteNo, char* lpszInfo, int nInfoSize, DCMINFO::MODULEINFO SelInfo, STS_BOARD_MODULE Module = STS_MOTHER_BOARD);
	/**
	 * @brief Read the calibration date of specific channel by board No.and channel No
	 * @param[I] bySlotNo The slot No.which the module belongs to
	 * @param[I] nChannelNo The channel No.of this board
	 * @return Calibration date
    */
	time_t APIENTRY dcm_GetCalibrationDateByChannel(BYTE bySlotNo, int nChannelNo);
    /**
     * @brief Read the calibration date of specific channel by pin name and site number
     * @param[in] lpszPinName The pin name defined in vector file
     * @param[in] bySiteNo The site number
     * @return The calibration date of this channel
     * - >0 The calibration date of this channel
     * - -1 Not load vector before
     * - -9 The pin name is not defined or its point is nullptr
     * - -10 The site is error
	 * - -18 The board number exceed maximum
	 * - -43 The pin is not belongs to current instance
    */
    time_t APIENTRY dcm_GetCalibrationDateByPin(const char* lpszPinName, USHORT usSiteNo);
    /**
     * @brief Read the calibration temperature of specific channel by pin name and site number
     * @param[in] lpszPinName The pin name defined in vector file
     * @param[in] bySiteNo The site number
	 * @return The calibration temperature of this channel
	 * - >=0 The calibration temperature of this channel
	 * - -1 Not load vector before
	 * - -9 The pin name is not defined or its point is nullptr
	 * - -10 The site is error
	 * - -18 The board number exceed maximum
	 * - -43 The pin is not belongs to current instance
     * - -44 Get the calibraiton information fail
    */
    double APIENTRY dcm_GetCalibrationTemperatureByPin(const char* lpszPinName, USHORT usSiteNo);
    /**
     * @brief Read the calibration humidity of specific channel by pin name and site number
     * @param[in] lpszPinName The pin name defined in vector file
     * @param[in] bySiteNo The site number
     * @return The calibration humidity of this channel
	 * - >0 The calibration humudity of this channel
	 * - -1 Not load vector before
	 * - -9 The pin name is not defined or its point is nullptr
	 * - -10 The site is error
	 * - -18 The board number exceed maximum
	 * - -32 The site is invalid
	 * - -43 The pin is not belongs to current instance
	 * - -44 Get the calibraiton information fail
    */
    double APIENTRY dcm_GetCalibrationHumidityByPin(const char* lpszPinName, USHORT usSiteNo);
	/**
    * @brief Read the calibration result of specific channel by board No.and channel No
    * @param[I] bySlotNo The slot No.which the module belongs to
    * @param[I] nChannelNo The channel No.of this board
    * @return
    * - 0 Calibration is PASS
    * - 1 Calibration is fail
    */
	BYTE APIENTRY dcm_GetCalibrationResultByChannel(BYTE bySlotNo, int nChannelNo);
    /**
     * @brief Read the calibration result of specific channel by pin name and site number
     * @param[in] lpszPinName The pin name defined in vector file
     * @param[in] usSiteNo The site number
     * @return Calibration result
     * - 0 Calibration is PASS
	 * - 1 Calibration is fail
	 * - -1 Not load vector before
	 * - -9 The pin name is not defined or its point is nullptr
	 * - -10 The site is error
	 * - -18 The board number exceed maximum
	 * - -32 The site is invalid
	 * - -43 The pin is not belongs to current instance
	 * - -44 Get the calibraiton information fail
    */
    double APIENTRY dcm_GetCalibrationResultByPin(const char* lpszPinName, USHORT usSiteNo);
    /**
     * @brief Read the logic version of specific channel when calibration by pin name and site number
     * @param[in] lpszPinName The pin name defined in vector file
     * @param[in] bySiteNo The site number
     * @return Logic revision
	 * - >0 Logic revision
	 * - -1 Not load vector before
	 * - -9 The pin name is not defined or its point is nullptr
	 * - -10 The site is error
	 * - -18 The board number exceed maximum
	 * - -32 The site is invalid
	 * - -43 The pin is not belongs to current instance
	 * - -44 Get the calibraiton information fail
    */
    double APIENTRY dcm_GetCalibrationLogicRevByPin(const char* lpszPinName, USHORT usSiteNo);
    /**
     * @brief Read the meter type of specific channel when calibration by pin name and site number
     * @param[in] lpszPinName The pin name defined in vector file
     * @param[in] bySiteNo The site number
     * @return Meter type
     * - 0 Keithley2000
     * - 1 Agilent34401
	 * - 2 Agilent3458A
	 * - -1 Not load vector before
	 * - -9 The pin name is not defined or its point is nullptr
	 * - -10 The site is error
	 * - -18 The board number exceed maximum
	 * - -32 The site is invalid
	 * - -43 The pin is not belongs to current instance
	 * - -44 Get the calibraiton information fail
    */
    double APIENTRY dcm_GetCalibrationMeterTypeByPin(const char* lpszPinName, USHORT usSiteNo);
    /**
     * @brief Read the calibration board's logic version of specific channel when calibration by pin name and site number
     * @param[in] lpszPinName The pin name defined in vector file
     * @param[in] bySiteNo The site number
     * @return Logic revision
	 * - >0 The logic revision of the calibration board
	 * - -1 Not load vector before
	 * - -9 The pin name is not defined or its point is nullptr
	 * - -10 The site is error
	 * - -18 The board number exceed maximum
	 * - -32 The site is invalid
	 * - -43 The pin is not belongs to current instance
	 * - -44 Get the calibraiton information fail
    */
    double APIENTRY dcm_GetCalibrationCalboardlogicRevByPin(const char* lpszPinName, USHORT usSiteNo);
    /**
     * @brief Read the calibration information of specific channel by pin name and site number
     * @param[in] lpszPinName The pin name defined in vector file
     * @param[in] bySiteNo The site number
     * @param[out] lpszInfo The variable which will save the calibration information
     * @param[in] InfoSize The size of the variable of info
     * @param[in] SelInfo The type of calibration information be gotten
     * @return Execute result
	 * - 0 Get the module information successfully
	 * - -1 Not load vector before
	 * - -9 The pin name is not defined or its point is nullptr
	 * - -10 The site is error
	 * - -18 The board number exceed maximum
	 * - -32 The site is invalid
	 * - -43 The pin is not belongs to current instance
	 * - -44 Get the calibraiton information fail
    */
    int APIENTRY dcm_GetCalibrationOptionInfoByPin(const char* lpszPinName, USHORT usSiteNo, char* lpszInfo, int InfoSize, DCMINFO::CALINFO SelInfo);
    /**
     * @brief Read the slot id the board inserted when it be calibrated by pin name and site number
     * @param[in] lpszPinName The pin name defined in vector file
     * @param[in] bySiteNo The site number
     * @return The slot ID
	 * - >0 The slot id the board inserted
	 * - -1 Not load vector before
	 * - -9 The pin name is not defined or its point is nullptr
	 * - -10 The site is error
	 * - -18 The board number exceed maximum
	 * - -32 The site is invalid
	 * - -43 The pin is not belongs to current instance
	 * - -44 Get the calibraiton information fail
    */
    double APIENTRY dcm_GetCalibrationSlotIDByPin(const char* lpszPinName, USHORT usSiteNo);
    /**
     * @brief Get the logic revision
     * @param[in] bySlotNo The slot number whose logic revision will be read
     * @return The logic revision of current board
    */
    BYTE APIENTRY dcm_GetLogicRevsion(BYTE bySlotNo);
    /**
     * @brief Set the dynamic load
     * @param[in] lpszPinName The pin name
     * @param[in] usSiteNo The site number
     * @param[in] Mode The dynamic load mode
     * @param[in] dIOH The output high current value
     * @param[in] dIOL The output low current value
     * @param[in] dVTValue The VT voltage
     * @param[in] dClmapHigh The high clamp voltage
     * @param[in] dClampLow The low clamp voltage
     * @return Execute result
     * - 0 Set dynamic load successfully
	 * - -1 Not load vector before
	 * - -8 The dynamic mode is error or not supported
	 * - -9 The pin is not existed in vector file or its point is nullptr
	 * - -10 The site is over range
	 * - -14 The VT voltage is over range
     * - -16 The board is not existed
	 * - -18 The output current is over range
     * - -32 The site is invalid
     * - -35 The channel is not existed
     * - -36 The clamp value is over range
	 * - -38 The channel is over range
	 * - -43 The pin is not belongs to current instance
    */
    int APIENTRY dcm_set_pinload(const char* lpszPinName, USHORT usSiteNo, BYTE byMode, double dIOH, double dIOL, double dVTVoltValue, double dClampHighVoltValue, double dClampLowVoltValue);
    /**
     * @brief Set the dynamic load
	 * @param[in] lpszPinGroup The pin group
	 * @param[in] Mode The dynamic load mode
	 * @param[in] dIOH The output high current value
	 * @param[in] dIOL The output low current value
	 * @param[in] dVTValue The VT voltage
	 * @param[in] dClmapHigh The high clamp voltage
	 * @param[in] dClampLow The low clamp voltage
	 * @return Execute result
	 * - 0 Set dynamic load successfully
	 * - -1 Not load vector before
	 * - -9 The pin is not existed in vector file or its point is nullptr
	 * - -10 The site is over range
	 * - -14 The VT voltage is over range
	 * - -16 The board is not existed
	 * - -18 The output current is over range
	 * - -32 The site is invalid
	 * - -35 The channels in the pin group are all not existed
     * - -32 No valid site
	 * - -43 The pin group include the pin which is not belongs to current instance
    */
    int APIENTRY dcm_SetDynamicLoad(const char* lpszPinGroup, BYTE bEnable, double dIOH, double dIOL, double dVTVoltValue);
    /**
     * @brief Get the sum number of the site and pin
     * @param[out] nPinCount The sum number of the pin used in vector
     * @return The sum number of the site
    */
    USHORT APIENTRY dcm_GetSiteAndPinCount(int& nPinCount);
    /**
     * @brief Get the site information
     * @param[out] usSiteNo The site number
     * @param[out] pbySlotNo The slot number of each pin
     * @param[out] pusChannelNo The channel number of each pin
     * @param[in] nElementCount The element count of the array
     * @return The channel count of the site
     * - >=0 The channel count of the site
     * - -1 Not load vector before
     * - -2 The site number is over range
    */
    int APIENTRY dcm_GetSiteChannel(USHORT usSiteNo, BYTE* pbySlotNo, USHORT* pusChannelNo, int nElementCount);
    /**
     * @brief Set the timeset delay
     * @param[in] bySlotNo The slot number of board
     * @param[in] byControllerIndex The controller index of board
     * @param[in] dDelay The delay data
     * @return Execute result
     * - -1 The board is not existed
     * - -2 The controller index is over range
     * - -3 The controller is not existed
     * - -4 The delay value is over range
    */
    int APIENTRY dcm_SetTimesetDelay(BYTE bySlotNo, BYTE byControllerIndex, double dDelay);
    /**
     * @brief Get the timeset delay
     * @param[in] bySlotNo The slot number of board
     * @param[in] byControllerIndex The controller index of board
     * @return The timeset delay
     * - Timeset delay
     * - 1e15 The board is not existed
     * - 1e15 The controller index is over range
     * - 1e15 The controller is not existed
    */
    double APIENTRY dcm_GetTimestDelay(BYTE bySlotNo, BYTE byControllerIndex);
    /**
     * @brief Set the total start delay
     * @param[in] bySlotNo The slot number of board
     * @param[in] byControllerIndex The controller index of board
     * @param[in] dDelay The delay data
     * @return Execute result
     * - -1 The board is not existed
     * - -2 The controller index is over range
     * - -3 The controller is not existed
     * - -4 The delay is over range
    */
    int APIENTRY dcm_SetTotalStartDelay(BYTE bySlotNo, BYTE byControllerIndex, double dDelay);
    /**
     * @brief Get the total start delay
     * @param[in] bySlotNo The slot number of board
     * @param[in] byControllerIndex The controller index of board
     * @return The total start delay
     * - total start delay
     * - 1e15 The board is not existed
     * - 1e15 The controller index is over range
     * - 1e15 The controller is not existed
    */
    double APIENTRY dcm_GetTotalStartDelay(BYTE bySlotNo, BYTE byControllerIndex);
    /**
     * @brief Set the IO delay of channel
     * @param[in] bySlotNo The slot number of board
     * @param[in] usChannel The channel number
     * @param[in] pdDelay The delay data
     * @param[in] nElementCount The element count of delay
     * @return Execute result
     * - 0 Set the IO delay successfully
     * - -1 The element is not enough
     * - -2 The board is not existed
     * - -3 The channel is over range
     * - -4 The channel is not existed
     * - -5 The delay is over range
    */
    int APIENTRY dcm_SetIODelay(BYTE bySlotNo, USHORT usChannel, double* pdDelay, int nElementCount);
    /**
     * @brief Get the IO delay of channel
     * @param[in] bySlotNo The slot number of board
     * @param[in] usChannel The channel number
     * @param[out] pdDelay The delay data
     * @param[in] nElementCount The element count of delay
	 * @return Execute result
	 * - 0 Get the IO delay successfully
	 * - -1 The element is not enough
	 * - -2 The board is not existed
	 * - -3 The channel is over range
	 * - -4 The channel is not existed
    */
    int APIENTRY dcm_GetIODelay(BYTE bySlotNo, USHORT usChannel, double* pdDelay, int nElementCount);
    /**
	 * @brief Save the delay data to flash
	 * @param[in] bySlotNo The slot number of board
     * @return Execute result
     * - 0 Save delay successfully
     * - -1 The board is not existed
     * - -2 Write flash error
     * - -3 No valid controller existed
    */
    int APIENTRY dcm_SaveDelay(BYTE bySlotNo);
    /**
     * @brief Get the sum number of board the vector used
     * @param[out] pusSiteNum The sum number of site set by vector file
     * @return The board count the vector used
     * - >=0 The board count the vector used
     * - -1 The vector file is not opened success
    */
    int APIENTRY dcm_VectorBoardNum(USHORT* pusSiteNum);
    /**
     *Add pin group to dcm
     * @param[in] pinGroupName The name of the pin group
     * @param[in] lpszPinNameList The all pin name of the pin group
     * @return Execute result
     * - 0  Add pin group success
     * - -1 The board is not existed
     * - -9 The pin name string is error or its point is nullptr
     * - -22 The pin group name is conflict
	 * - -23 The ping group name is blank
	 * - -43 The pin name list include the pin which not belongs to current instance
    */
    int APIENTRY dcm_SetPinGroup(const char* lpszinGroupName, const char* lpszPinNameList);
    /**
     * @brief Get the instance ID through pin group name
     * @param[in] lpszPinGroup The pin group
     * @return The instance ID
     * - >=0 The instance ID
     * - -1 Pin group is not existed
    */
    int APIENTRY dcm_GetInstanceID(const char* lpszPinGroup);
    /**
     * @brief Get the ppmu set value of the channel
     * @param[in] bySlotNo The slot number
     * @param[in] usChannelNo The channel number whose set value will be gotten
     * @param[out] byPPMUMode The PPMU mode
     * @return The measurement result
     * - >=0 The set value of current channel's ppmu
     * - 1e15-1 The board is not inserted or the channel number is bigger than 63
    */
    double APIENTRY dcm_GetPPMUSetValue(BYTE bySlotNo, USHORT usChannelNo, BYTE& byPPMUMode, BYTE& byPPMUIRange);
    /**
     * @brief Get the period of timeset
     * @param[in] bySlotNo The slot number
     * @param[in] byControlIndex The board number of the board
     * @param[in] byTimeSet The timeset whose rate will be gotten
	 * @return >0 The period of the timeset
	 * - >0 The period of timeset
	 * - -1 The board is not existed
	 * - -2 The controller is over range
	 * - -3 The controller is not existed
	 * - -4 The timeset is over range
    */
    double APIENTRY dcm_GetTimeSetPeriod(BYTE bySlotNo, BYTE byControlNo, BYTE byTimeSet);
    /**
     * @brief Get the valid DCM board
     * @return The valid board inserted
    */
    ULONG APIENTRY dcm_GetValidBoard();
    /**
     * @brief Get the relay status of channel
     * @param[in] bySlotNo The slot number
     * @param[in] usChannel The channel number whose relay status will be gotten
     * @param[in] byRelayType The type of the relay, 0 is function relay
     * @return Execute result
     * - 0 The relay is off
     * - 1 The relay is on
	 * - -1 The channel number is bigger than 63
     * - -2 The relay type is error
	 * - -3 The board is not inserted
     * - -4 This channel has no this type of relay
    */
    int dcm_GetRelayStatus(BYTE bySlotNo, USHORT usChannel, BYTE byRelayType);
    /**
     * @brief Get the string type
     * @param[in] lpszString The string will be checked
     * @return String type
     * - 0 The string is pin
     * - 1 The string is pin group
     * - 2 The string is label
     * - -1 The vector is not loaded
     * - -2 The point of string is nullptr
     * - -3 Not find
    */
    int APIENTRY dcm_GetStringType(const char* lpszString);
    /**
     * @brief Get the level setting int ATE305
     * @param[in] bySlotNo The slot number
     * @param[in] usChannel The channel number whose level will be gotten
     * @param[in] LevelType The type of level want to be gotten
     * @param[out] dLevelValue The level value be gotten
     * @return Execute result
     * - 0 The level of the channel is gotten successfully
     * - -16  The board is not inserted
     * - -26 The channel number is bigger than 63
    */
    int APIENTRY dcm_GetLevelSettingValue(BYTE bySlotNo, USHORT usChannel, DCM_LEVEL_TYPE LevelType, double& dLevelValue);
    /**
     * @brief Convert pin name and site NO to channel No
     * @param[in] lpszPinName The pin name defined in vector file
     * @param[in] usSiteNo The site number
     * @param[out] bySlotNo The slot No which the channel belongs to
     * @param[out] nChanelNo The channel No converted by pin name and site number
     * @return Execute result
     * - 0 Convert pin successfully
     * - -1 Not load vector before
     * - -9 The pin name is not defined
     * - -10 The site number is exceed
     * - -27 The point of pin name is nullptr
	 * - -32 The site is invalid
	 * - -43 The pin is not belongs to current instance
    */
    int APIENTRY dcm_ConvertPinNameToChannel(const char* lpszPinName, USHORT usSiteNo, BYTE& bySlotNo, int& nChanelNo);
    /**
     * @brief Get dynamic mode of channel
     * @param[in] bySlotNo The slot number
     * @param[in] usChannelNo The channel number whose dynamic mode will be gotten
     * @param[out] iDynamicMode The dynamic mode of current channel
     * @return Execute result
     * - 0 The dynamic mode of the channel is gotten successfully
     * - -1 The board is not inserted
     * - -2 The channel number is bigger than 63
     * - -3 The channel is not existed
    */
    int APIENTRY dcm_GetDynamicLoadMode(BYTE bySlotNo, USHORT usChannelNo, int& iDynamicMode);
    /**
     * @brief Get VT mode of channel
     * @param[in] bySlotNo The slot number
     * @param[in] usChannelNo The channel number whose VT mode will be gotten
     * @param[out] nVTMode The dynamic mode of current channel
     * @return Execute result
     * - 0 The VT mode of the channel is gotten successfully
     * - -1 The board is not inserted
     * - -2 The channel number is bigger than 63
     * - -3 The channel is not existed
    */
    int APIENTRY dcm_GetVTMode(BYTE bySlotNo, USHORT usChannelNo, int& nVTMode);
    /**
     * @brief Get the channel of the pin group
     * @param[in] lpszPinGroup The name of pin group
     * @param[in] usSiteNo The site number
     * @param[out] nChannelNo The channel number in pin group
     * @param[in] nArrayLength The array length of the iChannelNo
     * @param[out] nChannelCount The count of the channel in pin group
     * @return Execute result
     * - 0 Get the channel successfully
     * - -1 Not load vector before
     * - -2  The pin group is not defined or its point is nullptr
     * - -10 The site is over range
	 * - -25 The array length is not enough to save all channel in pin group
	 * - -43 The pin group include the pin which is not belongs to current instance
    */
    int APIENTRY dcm_GetChannelFromPinGroup(const char* lpszPinGroup, USHORT usSiteNo, int* nChannelNo, int nArrayLength, int& nChannelCount);
    /**
     * @brief Get the count of vector line between two label(The label line included)
     * @param[in] lpszStartLabel The name of start label,"" is mean from the first line of vector
     * @param[in] lpszStopLabel The name of stop label, "" is mean stop at the last line of vector
     * @param[out] ulVectorLineCount The count of vector line between start label and stop label
     * @return Execute result
     * - 0 Get the count of vector line successfully
     * - -1 The vector file is not loaded
     * - -6 The start label is not defined
     * - -7 The stop label is not defined
    */
    int APIENTRY dcm_GetVectorLineCount(const char* lpszStartLabel, const char* lpszStopLabel, ULONG& ulVectorLineCount);
    /**
     * @brief Get the line number of the label
     * @param[in] lpszLabelName The name of the label
     * @param[in] bBRAMLine The line to be gotten is BRAM line, TRUE is get BRAM line number, FALSE is get global line number
     * @return >0 The label line number
     * - -1 The vector file is not loaded
     * - -2 The point of label is nullptr
     * - -3 The label is not existed
    */
    int APIENTRY dcm_GetLabelLineNo(const char* lpszLabelName, BOOL bBRAMLine = TRUE);
    /**
     * @brief Mask the alarm of channel
     * @param[in] lpszPinName The pin name
     * @param[in] usSiteNo The site number
     * @param[in] bStatus The mask status
     * @return Execute result
     * - 0 Mask alarm successfully
	 * - -1 Not load vector before
	 * - -9 The pin name is not defined or its point is nullptr
	 * - -10 The site number is exceed
	 * - -32 The site is invalid
	 * - -43 The pin is not belongs to current instance
    */
    int APIENTRY dcm_set_channel_alarm_mask(const char* lpszPinName, USHORT usSiteNo, bool bStatus);
    /**
     * @brief Get the alarm mask status of channel
     * @param[in] lpszPinName The pin name
     * @param[in] usSiteNo The site number
     * @return Mask status
     * - 0 Mask 
     * - 1 Not mask
     * - -1 Not load vector before
     * - -9 The pin name is not defined or its point is nullptr
	 * - -10 The site number is exceed
	 * - -43 The pin is not belongs to current instance
    */
    bool APIENTRY dcm_get_channel_alarm_mask(const char* lpszPinName, USHORT usSiteNo);
    /**
     * @brief Set clamp alarm mask 
     * @param[in] lpszPinName The pin name
     * @param[in] usSiteNo The site number
     * @param[in] bMaskFlag Whether mask
     * @param[in] wFuncName The mask function name
     * @return Execute result
	 * - 0 Set clamp mask successfully
	 * - -1 Not load vector before
	 * - -9 The pin is not defined or its point is nullptr
	 * - -10 The site is over range or invalid
	 * - -18 The output current is over range
	 * - -16 The board is not existed
     * - -32 The site is invalid
	 * - -34 The channel is not existed
	 * - -38 The channel is over range
	 * - -43 The pin is not belongs to current instance
    */
    int APIENTRY dcm_set_clamp_alarm_mask(const char* lpszPinName, USHORT usSiteNo, bool bMaskFlag, WORD wFuncName);
    /**
	 * @brief Get mask status of clamp alarm
	 * @param[in] lpszPinName The pin name
	 * @param[in] usSiteNo The site number
	 * @return Execute result
	 * - 0 Not mask
     * - 1 Mask
    */
    WORD APIENTRY dcm_get_clamp_alarm_mask(const char* lpszPinName, BYTE usSiteNo);
    /**
	 * @brief Set the timeset edge
	 * @param[in] lpszPinGroup The name of pin group
	 * @param[in] lpszTimeset The name of the timeset
     * @param[in] byWaveFormat The wave format
     * @param[in] dT1R The raise edge of driver wave
     * @param[in] dT1F The fall edge of driver wave
     * @param[in] dIOR The raise edge of IO
     * @param[in] dIOF The fall edge of IO
     * @param[in] dSTBR The raise edge of compare
     * @param[in] dSTBF The fall edge of compare
	 * @param[in] byCompareMode The compare mode
	 * @return Execute result
	 * - 0 Set time edge successfully
	 * - -1 Not load vector before
	 * - -2 Pin group is not defined before
	 * - -3 The timeset is not defined in vector
	 * - -5 The edge is over range
	 * - -16 No valid board inserted
     * - -32 No valid site
	 * - -43 The pin group include the pin which is not belongs to current instance
    */
    int APIENTRY dcm_SetTimeByName(const char* lpszPinGroup, const char* lpszTimeset, BYTE byWaveFormat, double dT1R, double dT1F, double dIOR, double dIOF, double dSTBR, double dSTBF, BYTE byCompareMode);
    /**
     * @brief Set the timeset edge
     * @param[in] lpszPinGroup The name of pin group
     * @param[in] lpszTimeset The name of the timeset
	 * @param[in] byWaveFormat The wave format
	 * @param[in] byIOFormat The IO format
     * @param[in] dT1R The raise edge of driver wave
     * @param[in] dT1F The fall edge of driver wave
     * @param[in] dIOR The raise edge of IO
     * @param[in] dIOF The fall edge of IO
     * @param[in] dSTBR The raise edge of compare
     * @param[in] dSTBF The fall edge of compare
     * @param[in] byCompareMode The compare mode
     * @return Execute result
     * - 0 Set time edge successfully
     * - -1 Not load vector before
     * - -2 Pin group is not defined before
     * - -3 The timeset is not defined in vector
     * - -5 The edge is over range
	 * - -16 No valid board inserted
     * - -32 No valid site
	 * - -43 The pin group include the pin which is not belongs to current instance
    */
    int APIENTRY dcm_SetEdge(const char* lpszPinGroup, const char* lpszTimeset, BYTE byWaveFormat, BYTE byIOFormat, double dT1R, double dT1F, double dIOR, double dIOF, double dSTBR, double dSTBF, BYTE byCompareMode);
    /**
     * @brief Set the period of timeset by name
     * @param[in] lpszTimesetName The name of the timeset
     * @param[in] dPeriod The period of the timeset
     * @return Execute result
     * - 0 Clear the memory successfully
     * - -1 Not load vector before
     * - -3 The name of the timeset is not defined
     * - -4 The period is over range
     * - -16 The board is not existed
    */
    int APIENTRY dcm_SetPeriodByName(const char* lpszTimesetName, double dPeriod);
    /**
     * @brief Set the wave data parameter
     * @param[in] lpszPinGroup The pin group
     * @param[in] lpszStartLabel The label name
     * @param[in] ulOffset The start line offset to the label
     * @param[in] nWriteVectorLineCount The line count of wave data
     * @return Execute result
     * - 0 Set the parameter successfully
     * - -1 Not load vector before
     * - -2 The pin group is not defined before
     * - -6 The start label is not existed in vector file
     * - -15 The offset is over range
     * - -16 No valid bard existed
     * - -27 The point of pin group or start label is nullptr
     * - -26 The line count be written is over range
     * - -28 Allocate memory fail
     * - -32 No valid site
	 * - -35 The channel is not existed
	 * - -43 The pin group include the pin which is not belongs to current instance
    */
    int dcm_SetWaveDataParam(const char* lpszPinGroup, const char* lpszStartLabel, ULONG ulOffset, int nWriteVectorLineCount);
    /**
     * @brief Set the wave data of each site
     * @param[in] usSiteNo The site number
     * @param[in] pbyWaveData The wave data of the site
     * @return Execute result
     * - 0 Set wave data successfully
     * - -1 Not load vector before
     * - -10 The site is over range
     * - -16 No valid board existed
     * - -28 Allocate memory fail
     * - -29 Not set wave data parameter before
     * - -32 The site is invalid
    */
    int dcm_SetSiteWaveData(USHORT usSiteNo, BYTE* pbyWaveData);
    /**
     * @brief Start write data to board
     * @return Execute result
     * - 0 Write successfully
	 * - -1 Not load vector before
	 * - -16 board not inserted
	 * - -29 Not set wave data parameter before
	 * - -32 No Valid site
    */
    int dcm_StartWriteData();
    /**
     * @brief Set the capture data
     * @param[in] lpszPinGroup The pin group
     * @param[in] lpszStartLabel The label
     * @param[in] usSiteNo The site number
     * @param[in] ulOffset The start line offset to the label
     * @param[in] nCount The line count to get capture
     * @return Execute result
     * - 0 Set capture successfully
     * - -1 Not load vector before
     * - -2 The pin group not defined
     * - -6 The start label is not existed in vector file
     * - -10 The site number is over range
     * - -15 The offset is over range
	 * - -16 No valid board exited
	 * - -32 No Valid site
     * - -33 Not ran vector before
     * - -34 The vector is running
     * - -35 The channels in the pin group are all not existed
	 * - -37 The latest ran vector is not ran from start label
	 * - -43 The pin group include the pin which is not belongs to current instance
    */
    int APIENTRY dcm_SetCapture(const char* lpszPinGroup, const char* lpszLabel, USHORT usSiteNo, ULONG ulOffset, int nCount);
    /**
     * @brief Get the capture data
     * @param[in] lpszPinName The pin name
     * @param[in] usSiteNo The site number
     * @param[out] ulCaptureData The capture data
     * @return Execute result
	 * - 0 Get capture data successfully
	 * - -1 Not load vector before
     * - -9 The pin name is not existed in vector file
     * - -10 The site number is over range
     * - -16 The board is not existed
	 * - -32 The site is invalid
	 * - -33 Not ran vector before
	 * - -34 The vector is running
	 * - -35 The channel is not existed
	 * - -41 The fail line number not saved
	 * - -43 The pin is not belongs to current instance
    */
    int APIENTRY dcm_GetCaptureData(const char* lpszPinName, USHORT usSiteNo, ULONG& ulCaptureData);
    /**
     * @brief Set preread data
     * @param[in] lpszStartLabel The start label
     * @param[in] lpszStart label The stop label
     * @return Execute result
     * - 0 Set preread vector successfully
     * - -6 The start label is not existed or its point is nullptr
     * - -7 The stop label is not existed or its point is nullptr
     * - -16 No valid board existed
     * - -19 The stop label is before start label
	 * - -21 The section count of preread vector is over range
     * - -28 Allocate memory fail
    */
    int APIENTRY dcm_SetPrereadVector(const char* lpszStartLabel, const char* lpszStopLabel);
    /**
     * @brief Get the FPGA Revision
     * @param[in] bySlotNo The slot number of board
     * @param[in] byControllerIndex The controller index
     * @return FPGA revision
     * - != 0xFFFFFFFF The FPGA version
     * - 0xFFFFFFFF The board is not existed or the controller index is over range or not existed
    */
    ULONG APIENTRY dcm_read_FPGA_Version(BYTE bySlotNo, BYTE byControllerIndex);
    /**
     * @brief Set PMU multi measure
     * @param[in] lpszPinGroup The pin group name
     * @param[in] uSampleTimes The sample times
     * @param[in] dSamplePeriod The sample period
     * @return Execute result
	 * - 0 Set measure successfully
     * - -1 Not load vector before
     * - -2 The pin group is not defined or its' point is nullptr
	 * - -16 No valid board existed
     * - -17 PMU measurement error
	 * - -35 The channels in the pin group are all not existed
	 * - -43 The pin group include the pin which is not belongs to current instance
    */
    int APIENTRY dcm_ppmuMultiMeasure(const char* lpszPinGroup, UINT uSampleTimes, double dSamplePeriod);
    /**
     * @brief Get the PMU measure result of specific sample
     * @param[in] lpszPinName The pin number
     * @param[in] usSiteNo The site number
     * @param[in] nSamppleTimes The sample point, -1 is the average value
     * @return The measure result
     * - != 1e15 The measure result
     * - 1e15 The channel is over range or the sample times is over range
    */
    double APIENTRY dcm_getPpmuMultiMeasResult(const char* lpszPinName, USHORT usSiteNo, int sampleNumber);
    /**
     * @brief Get the pin count and site count
     * @param[out] usPinCount The pin count in vector
     * @param[out] usSiteCount The site count
     * @return Execute result
     * - 0 Get pin count and site count successfully
     * - -1 Not load vector before
    */
    int APIENTRY dcm_getPinNumAndsiteNum(USHORT& usPinCount, USHORT& usSiteCount);
    /**
     * @brief Get the pin channel
     * @param[in] usPinNo The pin number
     * @param[in] usSiteNo The site number
     * @param[out] sChannel The channel number
     * @return Execute result
     * - TRUE Get pin channel successfully
     * - FALSE Get pin channel fail
    */
    BOOL APIENTRY dcm_getPinChannel(USHORT usPinNo, USHORT usSiteNo, short& sChannel);
    /**
     * @brief Get the slot number and channel number in board
     * @param[in] usPinNo The pin number
     * @param[in] usSiteNo The site number
     * @param[out] usChannel The channel number in board
     * @return The slot number
     * - >0 The slot number
     * - -1 The pin number is not existed
     * - -2 The site number is over range
    */
    int APIENTRY dcm_GetPinSlotChannel(USHORT usPinNo, USHORT usSiteNo, USHORT& usChannel);
    /**
     * @brief Get the sample times of PMU
     * @param[in] usPinNo The pin number
     * @param[in] usSiteNo The site number
     * @return The sample times of the channel
     * - >=0 The sample times
     * - -1 Can't find the pin number
     * - -2 The site is over range or invalid
    */
    int APIENTRY dcm_getSampleTimes(USHORT usPinNo, USHORT usSiteNo);
    /**
     * @brief Get the sampler interval of PMU
     * @param[in] usPinNo The pin number
     * @param[in] usSiteNo The site number
	 * @return The sample interval of the channel
	 * - >=0 The sample interval
	 * - -1 Can't find the pin number
	 * - -2 The site is over range or invalid
    */
    int APIENTRY dcm_getSampleInterval(USHORT usPinNo, USHORT usSiteNo);
    /**
     * @brief Get the measure status of PMU
     * @param[in] usPinNo The pin number
     * @param[in] usSiteNo The site number
     * @param[out] byIRange The current range
     * @param[out] byMeasType The measurement type
     * @return Execute result
     * - 0 Get measurement successfully
     * - -1 The board not existed
     * - -2 The pin number is not existed
    */
    int APIENTRY dcm_getPpmuMeasStatus(USHORT usPinNo, USHORT usSiteNo, BYTE& byIRange, BYTE& byMeasType);
    /**
     * @brief Get the pin name
     * @param[in] usPinNo The pin number
     * @return The pin name
     * - != nullptr The pin name
     * - nullptr The pin number is not existed
    */
    const char* APIENTRY dcm_getPinName(USHORT usPinNo);
    /**
     * @brief Get the instance ID of the pin number
     * @param[in] usPinNo The pin number
     * @param[out] bDCMWithVector Whether the instance with vector file
     * @return Execute result
     * - >=0 The instance ID
     * - -1 The pin number is not existed
    */
    int APIENTRY dcm_GetPinInstanceID(USHORT usPinNo, BOOL& bDCMWithVector);
    /**
     * @brief Get the board information
     * @param[out] pbySlotNo The slot number of board
     * @param[in] nElementCount The element count
     * @return The board count
    */
    int APIENTRY dcm_GetBoardInfo(BYTE* pbySlotNo, int nElementCount);
    /**
     * @brief Get the site number which use the board
     * @param[in] bySlotNo The slot number
     * @param[out] pusSite The site number which use the board
     * @param[in] nElementCount The element count in the array
     * @return The site count in the board
    */
    int APIENTRY dcm_GetSlotSite(BYTE bySlotNo, USHORT* pusSite, int nElementCount);
    /**
     * @brief Get the channel number used in the pin group 
     * @param[in] lpszPinGroup The pin group name
     * @param[in] usSiteNo The site number
     * @param[in] pbySlotNo The slot number of the board used in pin group
     * @param[in] pusChannel The channel number of board
     * @param[in] nElementCount The element count of the array
	 * @return The channel count in the pin group
	 * - > 0 The channel count in the pin group
	 * - -1 Not load vector before
	 * - -2 The point of pin group is nullptr
	 * - -3 The pin group is not defined before
	 * - -4 The site number is over range
     * - -5 The pin is not belongs to
    */
    int APIENTRY dcm_GetPinGroupChannel(const char* lpszPinGroup, USHORT usSiteNo, BYTE* pbySlotNo, USHORT* pusChannel, int nElementCount);
    /**
     * @brief Get the pin count in vector file
     * @return The site count
     * - >=0 The pin count
     * - -1 Not load vector before
    */
    int APIENTRY dcm_GetVectorSiteCount();
    /**
     * @brief Set I2C information
     * @param[in] fPeriod The period of I2C
     * @param[in] nSiteCount The site count of I2C
     * @param[in] byREGAddrMode The register byte count
     * @param[in] lpszSCLChannel The channel of SCL
     * @param[in] lpszSDAChannel The channel of SDA
     * @return Execute result
     * - 0 Set I2C information successfully
     * - -1 The site count is over range
     * - -2 The register address byte count is over range
     * - -3 The channel information of SCL or SDA is error
     * - -4 The channel in SCL or SDA is not existed
     * - -5 The channel in SCL or SDA is over range
     * - -6 The format of SCL or SDA is error
     * - -7 The site count and channel count is not match
     * - -8 The channel in SCL and SDA is conflict
    */
    int APIENTRY dcm_I2CSet(float fPeriod, int nSiteCount, BYTE byREGAddrMode, const char* lpszSCLChannel, const char* lpszSDAChannel);
    /**
      * @brief Set the pin level of I2C channel
      * @param[in] dVIH The input voltage of logic high
      * @param[in] dVIL The input voltage of logic low
      * @param[in] dVOH The output voltage of logic high
      * @param[in] dVOL The output voltage of logic low
      * @return Execute result
      * - 0 Set pin level successfully
      * - -1 The pin level is over range
      * - -2 No set site before
     */
    int APIENTRY dcm_I2CSetPinLevel(double dVIH, double dVIL, double dVOH, double dVOL);
    /**
     * @brief Write data through I2C
     * @param[in] bySAddress The slave address
     * @param[in] dwRegAddress The register address
     * @param[in] nDataLength The write data count
     * @param[in] dwDataArray The data written of each site
     * @return Execute result
     * - 0 Write data successfully
     * - -1 The data byte count is over range
	 * - -2 Not set I2C channel information
     * - -3 Allocate memory fail
	 * - -4 The line is not enough
    */
    int APIENTRY dcm_I2CWriteData(BYTE bySAddress, DWORD dwRegAddress, int nDataLength, DWORD dwDataArray[]);
    /**
     * @brief Write data through I2C
     * @param[in] bySlaveAddress The slave address
     * @param[in] uRegisterAddress The register address
     * @param[in] uDataByteCount The write data count
     * @param[in] pbyWriteData The data written of each site
     * @return Execute result
     * - 0 Write data successfully
     * - -1 The data cout is less than 1
     * - -2 Not set I2C information before
     * - -3 The point of data is nullptr
     * - -4 The line is not enough
    */
    int APIENTRY dcm_I2CWriteMultiData(BYTE bySAddress, DWORD dwRegAddress, int nDataLength, BYTE* pbyDataAddressArray[]);
    /**
     * @brief Write data through I2C which data of each site is same
     * @param[in] bySlaveAddress The slave address
     * @param[in] uRegisterAddress The register address
     * @param[in] uDataByteCount The write data count
	 * @param[in] pbyDataWritten The data written
	 * @return Execute result
     * - 0 Write data successfully
     * - -1 The data cout is less than 1
     * - -2 Not set I2C information before
     * - -3 The point of data is nullptr
     * - -4 The line is not enough
    */
    int APIENTRY dcm_I2CWriteSameData(BYTE bySAddress, DWORD dwRegAddress, int nDataLength, const BYTE* pbyDataWritten);
    /**
     * @brief Read data through I2C
     * @param[in] bySlaveAddress The slave address
     * @param[in] uRegisterAddress The register address
     * @param[in] uDataByteCount The read data count
     * @return Execute result
     * - 0 Read data successfully
     * - -1 The data count will be read is over range
     * - -2 The line is not enough
     * - -3 Not set I2C information before
    */
    int APIENTRY dcm_I2CReadData(BYTE bySAddress, DWORD dwRegAddress, int datacount);
    /**
     * @brief Get NACK index of latest I2C operation
     * @param[in] usSiteIndex The site index
     * @return The NACK index
     * - >0 The NACK index
     * - 0 No NACK
     * - -1 The site is over range
     * - -2 The site is invalid
     * - -3 Not set I2C information before
	 * - -4 No I2C operation before
	 * - -5 The board of the site is not existed
    */
    int APIENTRY dcm_GetNACKIndex(USHORT usSiteNo);
    /**
    * @brief Get the data read before
    * @param[in] usSiteIndex The site index
    * @param[in] nDataIndex The read data index
    * @return Data read before
    * - >=0 The data read before
    * - -1 The site index is over range
    * - -2 The site is invalid
	* - -3 The data index is over range
	* - -4 Not set I2C information before
	* - -5 Not read before
    * - -6 The data of the site is not existed
    */
    int APIENTRY dcm_I2CGetReadData(USHORT usSiteNo, int nDataIndex);
    /**
     * @brief Clear I2C memory
    */
    void APIENTRY dcm_I2CDeleteMemory();
    /**
     * @brief Connect relay
     * @return Execute result
     * - 0 Connect successfully
     * - -1 Not set I2C information before
    */
    int APIENTRY dcm_I2CConnect();
    /**
     * @brief Disconnect relay
     * @return Execute result
     * - 0 Disconnect successfully
     * - -1 Not set I2C information before
    */
    int APIENTRY dcm_I2CDisconnect();
    /**
     * @brief Get the bit data of site
     * @param[in] usSiteNo The site number
     * @param[in] nStartBitIndex The start bit index
     * @param[in] nBitCount The bit count
     * @return The bit data
    */
    ULONG APIENTRY dcm_I2CGetBitData(USHORT usSiteNo, int nStartBitIndex, int nBitCount);
    /**
     * @brief Set the edge of SDA channel
	 * @param[in] dT1R The raise edge of driver wave
	 * @param[in] dT1F The fall edge of driver wave
	 * @param[in] dIOR The raise edge of IO
	 * @param[in] dIOF The fall edge of IO
	 * @param[in] dSTBR The raise edge of compare
	 * @param[in] dSTBF The fall edge of compare
     * @return Execute result
     * - 0 Set the edge of SDA is successfully
     * - -1 Not set I2C information before
     * - -2 The edge is over range
    */
    int APIENTRY dcm_I2CSetSDATime(double dT1R, double dT1F, double dIOR, double dIOF, double dSTBR, double dSTBF);
    /**
     * @brief Set the edge of SCL channel
     * @param[in] dT1R The raise edge of driver wave
     * @param[in] dT1F The fall edge of driver wave
     * @return Execute result
     * - 0 Set the edge of SCL is successfully
     * - -1 Not set I2C site information
     * - -2 The edge is over range
    */
    int APIENTRY dcm_I2CSetSCLTime(double dT1R, double dT1F);
    /**
     * @brief Get the I2C site count
     * @return The site count of I2C
     */
    int APIENTRY dcm_I2CGetSiteCount();
    /**
     * @brief Set the period of I2C protocol
     * @param[in] dPeriod The Period
     * @return Execute result
    * - 0 Set period successfully
    * - -1 Not set I2C information before
    * - -2 The period is over range
    */
    int APIENTRY dcm_I2CSetPeriod(double dPeriod);
    /**
    * @brief Get channel count of the board
    * @param[in] bySlotNo The slot number of board
	* @return The channel count of the board
	 * - >=0 The channel count
	 * - -1 The board is not existed
	 * - -2 Flash error
	 * - -3 Allocate memory fail
	 * - -4 The data in flash is error
    */
    int APIENTRY dcm_GetChannelCount(BYTE bySlotNo);
    /**
     * @brief Get channel count of the board
     * @param[in] bySlotNo The slot number of board
     * @param[in] usChannelCount The channel count of the board
	 * @return Execute result
	 * - 0 Set channel count successfully
	 * - -1 The board is not existed
	 * - -2 The channel count is over range
	 * - -3 The flash is error
	 * - -4 Write channel count fail
    */
    int APIENTRY dcm_SetChannelCount(BYTE bySlotNo, USHORT usChannelCount);
    /**
     * @brief Run vector of the channel in pin group without ran
     * @param[in] lpszPinGroup The pin group name
     * @param[in] lpszStartLabel The start label of run vector
     * @param[in] lpszStopLabel The stop label of run vector
     * @retrurn Execute result
     * - 0 Set ran vector parameter successfully
     * - -1 Not load vector file before
     * - -2 The pin group is not defined before
     * - -6 The start label is not defined in vector file
     * - -7 The stop label is not defined in vector file
     * - -16 No valid board existed
	 * - -19 The start label is after stop label
     * - -32 No valid site
	 * - -43 The pin group include the pin which is not belongs to current instance
    */
    int APIENTRY dcm_RunVectorEn(const char* lpszPinGroup, const char* lpszStartLabel, const char* lpszStopLabel);
    /**
     * @brief Whether need run synchronously
     * @return Whether need run
     * - TRUE Need run vector synchronously
     * - FALSE No vector need ran
    */
    int APIENTRY dcm_IsSynRun();
    /**
     * @brief Run vector set before
     * @return Execute result
     * - 0 Ran vector successfully
     * - -1 No vector need to be ran
    */
    int APIENTRY dcm_Run();
    /**
     * @brief Wait the synchronous vector stop
     * @return Execute result
     * - 0 Vector stop
     * - -1 No running vector need to wait stop
    */
    int APIENTRY dcm_WaitStop();
    /**
     * @brief Get the board information of specific site
     * @param[in] usSiteNo The site number
     * @param[out] pbySlotNo The slot number of board
     * @param[out] pusChannelCount The channel count needed of the board
     * @param[in] nElementCount The array length
     * @return The board count used by this site
     * - >=0 The board count used by this site
     * - -1 No loaded vector before
     * - -2 The site number is over range
    */
    int APIENTRY dcm_GetSiteBoard(USHORT usSiteNo, BYTE* pbySlotNo, USHORT* pusChannelCount, int nElementCount);
	/**
     * @brief Get the capture
     * param[in] lpszPinName The pin number
     * param[in] usSiteNo The site number
     * param[out] pbyCaptureData The capture data, the element of the array must have enough memory to save the capture data
     * param[in] nCaptureLineCount The capture line count
     * @return The capture line count
	 * - >=0 The capture line count
	 * - -1 Not load vector before
	 * - -9 The pin name is not defined or its point is nullptr
	 * - -10 The site is over range
	 * - -16 The board is not existed
	 * - -32 The site is invalid
	 * - -33 Not ran vector
	 * - -34 Vector running
	 * - -35 The channel is not existed
     * - -42 The hardware capture of latest ran are not all saved
	 * - -43 The pin is not belongs to current instance
    */
    int APIENTRY dcm_GetHardwareCapture(const char* lpszPinName, USHORT usSiteNo, BYTE* pbyCaptureData, int nElementCount);
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
    int APIENTRY dcm_GetPattern(BYTE bySlotNo, BYTE byControllerIndex, BOOL bBRAM, UINT uStartLine, UINT uLineCount, char(*lpszPattern)[17]);
	/**
	 * @brief Get the memory of controller
	 * @param[in] bySlotNo The slot number
	 * @param[in] byControllerIndex The controller index
	 * @param[in] bBRAM Whether get the pattern saved in BRAM
	 * @param[in] DataType The data type
	 * @param[in] uStartLine The start line number
	 * @param[in] uLineCount The line count
	 * @param[out] pusData The data in memory
	 * @return Execute result
	 * - 0 Read memory successfully
     * - -1 The data type is not supported
	 * - -2 The board is not existed
	 * - -3 The controller index is over range
	 * - -4 The controller is not existed
	 * - -5 The data type is not supported
	 * - -6 The start line is over range;
	 * - -7 The line count read is 0 or the data buff is nullptr
    */
    int APIENTRY dcm_GetMemory(BYTE bySlotNo, BYTE byControllerIndex, BOOL bBRAM, DATA_TYPE DataType, UINT uStartLine, UINT uLineCount, USHORT* pusData);	
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
    int APIENTRY dcm_SetMemory(BYTE bySlotNo, USHORT usChannel, BOOL bRAM, DATA_TYPE DataType, UINT uStartLine, UINT uLineCount, BYTE* pbyData);
    /**
     * @brief Get the fail line number
     * @param[in] bySlotNo The slot number
     * @param[in] usChannel The channel number of board
     * @param[in] bBRAM Whether get the fail line number in BRAM
     * @param[in] uGetMaxFailCount The maximum fail count will be gotten
     * @param[out] vecLineNo The fail line number of last running
     * @return Execute result
     * - >=0 The fail line number
     * - -1 The board is not existed
     * - -2 The channel is over range
     * - -3 The channel is not existed
     * - -4 Not ran vector before
     * - -5 Vector running
    */
    int APIENTRY dcm_GetRAMFailLineNo(BYTE bySlotNo, USHORT usChannel, BOOL bBRAM, UINT uGetMaxFailCount,  UINT* puFailLineNo);
    /**
     * @brief Get the latest I2C memory information
     * @param[out] uStartLine The start line in BRAM
     * @param[out] uLineCount The line count in DRAM
     * @param[out] bWithDRAM Whether use BRAM
     * @param[out] uLineCountBeforeOut The line count before switch out
     * @param[out] uDRAMStartLine The start line in DRAM
     * @param[out] uDRAMLineCount The line count in DRAM
     * @return Execute result
     * - 0 Get the memory information successfully
     * - -1 No I2C before
    */
    int APIENTRY dcm_GetLatestI2CMemory(UINT& uStartLine, UINT& uLineCount, BOOL& bWithDRAM, UINT& uLineCountBeforeOut, UINT& uDRAMStartLine, UINT& uDRAMLineCount);
    /**
     * @brief Get the fail count of channel
     * @param[in] bySlotNo The slot number
     * @param[in] usChannel The channel number
     * @return The fail count of channel
     * - >=0 The fail count of the channel
     * - -1 The board is not existed
     * - -2 The channel number is over range
     * - -3 The channel is not existed
     * - -4 Not ran vector before
     * - -5 Vector running
    */
    int APIENTRY dcm_GetChannelFailCount(BYTE bySlotNo, USHORT usChannel);	
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
    int APIENTRY dcm_SetEdgeByIndex(BYTE bySlotNo, USHORT usChannel, BYTE byTimeset, WAVE_FORMAT WaveFormat, IO_FORMAT IOFormat, double* pdEdgeValue, COMPARE_MODE CompareMode);
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
    int APIENTRY dcm_GetEdge(BYTE bySlotNo, USHORT usChannel, BYTE byTimesetIndex, double* pdEdge, WAVE_FORMAT& WaveFormat, IO_FORMAT& IOFormat, COMPARE_MODE& CompareMode);
    /**
     * @brief Set the channel status
     * @param[in] lpszPinGroup The pin group
     * @param[in] usSiteNo The site number
     * @param[in] byStatus The channel status
	 * @return Execute result
	 * - 0 Set channel status successfully
	 * - -1 Not load vector file before
	 * - -2 The pin group is not defined before
     * - -10 The site number is over range
     * - -16 The board are all not existed
     * - -32 The site is invalid
	 * - -35 The channel is not existed
	 * - -43 The pin group include the pin which is not belongs to current instance
    */
    int APIENTRY dcm_SetChannelStatus(const char* lpszPinGroup, USHORT usSiteNo, BYTE byStatus);
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
    int APIENTRY dcm_GetChannelStatus(BYTE bySlotNo, USHORT usChannel);
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
    int APIENTRY dcm_GetChannelMode(BYTE bySlotNo, USHORT usChannel);
    /**
     * @brief Connect the channel to TMU unit
     * @param[in] lpszPinGroup The pin group
     * @param[in] usSiteNo The site number
     * @param[in] byTMUUnitIndex The TMU unit index
     * @return Execute result
     * - 0 Connect the TMU unit successfully
     * - -1 Not load vector file before
     * - -2 The pin group is not define or its point is nullptr
     * - -10 The site is over range
     * - -16 No valid board existed
     * - -32 The site is invalid
     * - -35 The channel in the pin group is not existed
     * - -38 The pin group have more than one channel in a controller
	 * - -39 The unit index is over range
	 * - -43 The pin group include the pin which is not belongs to current instance
    */
    int APIENTRY dcm_SetTMUMatrix(const char* lpszPinGroup, USHORT usSiteNo, BYTE byTMUUnitIndex);
    /**
     * @brief Get the unit channel
     * @param[in] bySlotNo The slot number
     * @param[in] usChannel The channel number
     * @return The unit connected to the channel
     * - >=0 The channel connected to the TMU unit
     * - -1 The Board is not existed
     * - -2 The channel number is over range
     * - -3 The channel is not existed
     * - -4 The channel is not connected any unit
    */
    int APIENTRY dcm_ReadTMUConnectUnit(BYTE bySlotNo, USHORT usChannel);
    /**
     * @brief Get the unit channel
     * @param[in] bySlotNo The slot number
     * @param[in] usChannel The channel number
     * @return The unit connected to the channel
     * - >=0 The channel connected to the TMU unit
     * - -1 The Board is not existed
     * - -2 The channel number is over range
     * - -3 The channel is not existed
     * - -4 The channel is not connected any unit
    */
    int APIENTRY dcm_GetTMUConnectUnit(BYTE bySlotNo, USHORT usChannel);
    /**
     * @brief Set TMU parameter
     * @param[in] lpszPinGroup The pin group
     * @param[in] usSiteNo The site number
     * @param[in] bRaiseTriggerEdge The rigger edge
     * @param[in] uHoldOffTime The hold off time, the unit is ns
     * @param[in] uHoldOffNum The hold off number
	 * @return Execute result
	 * - 0 Set the TMU parameter successfully
	 * - -1 Not load vector file before
	 * - -2 The pin group is not define or its point is nullptr
	 * - -10 The site is over range
	 * - -16 No valid board existed
	 * - -32 The site is invalid
	 * - -35 The channel in the pin group is not existed
	 * - -40 The channel is not connected to TMU unit
	 * - -43 The pin group include the pin which is not belongs to current instance
    */
	int APIENTRY dcm_SetTMUParam(const char* lpszPinGroup, USHORT usSiteNo, BOOL bRaiseTriggerEdge, UINT uHoldOffTime, UINT uHoldOffNum);
    /**
	 * @brief Set the TMU unit parameter
	 * @param[in] lpszPinGroup The pin group
	 * @param[in] bRaiseTriggerEdge The rigger edge
	 * @param[in] uHoldOffTime The hold off time, the unit is ns
	 * @param[in] uHoldOffNum The hold off number
     * @param[in] byTMUUnitIndex The TMU unit index
	 * @return Execute result
	 * - 0 Set the TMU parameter successfully
	 * - -1 Not load vector file before
	 * - -2 The pin group is not define or its point is nullptr
	 * - -16 No valid board existed
	 * - -32 No valid site
	 * - -35 The channel in the pin group is not existed
	 * - -40 The channel is not connected to TMU unit
	 * - -43 The pin group include the pin which is not belongs to current instance
    */
    int APIENTRY dcm_SetTMUUnitParam(const char* lpszPinGroup, BOOL bRaiseTriggerEdge, UINT uHoldOffTime, UINT uHoldOffNum, BYTE byTMUUnitIndex);
    /**
     * @brief Read the TMU parameter from board
     * @param[in] bySlotNo The slot number
	 * @param[in] usChannel The channel number of the board
	 * @param[out] bRaiseTriggerEdge Whether the trigger edge is raise
	 * @param[out] usHoldOffTime The hold off time, unit is ns
	 * @param[out] usHoldOffNum The hold off number
	 * @return Execute result
	 * - 0 Get the parameter successfully
	 * - -1 The board is not existed
	 * - -2 The channel number is over range
     * - -3 The channel number is not existed
	 * - -4 The TMU unit index is not existed
    */
	int APIENTRY dcm_ReadTMUParam(BYTE bySlotNo, USHORT usChannel, BOOL& bRaiseTriggerEdge, USHORT& usHoldOffTime, USHORT& usHoldOffNum);
    /**
     * @brief Read the unit parameter
     * @param[in] bySlotNo The slot number
     * @param[in] byControllerIndex The controller index
	 * @param[in] byTMUUnitIndex The TMU unit whose parameter will be read
	 * @param[out] bRaiseTriggerEdge Whether the trigger edge is raise
	 * @param[out] usHoldOffTime The hold off time, unit is ns
	 * @param[out] usHoldOffNum The hold off number
	 * @return Execute result
	 * - 0 Get the parameter successfully
	 * - -1 The board is not existed
	 * - -2 The controller is over range 
     * - -3 The controller is not existed
	 * - -4 The TMU unit index is over range
    */
    int APIENTRY dcm_ReadTMUUnitParam(BYTE bySlotNo, BYTE byControllerIndex, BYTE byTMUUnitIndex, BOOL& bRaiseTriggerEdge, USHORT& usHoldOffTime, USHORT& usHoldOffNum);
    /**
     * @brief Get the TMU parameter
     * @param[in] bySlotNo The slot number
     * @param[in] byControllerIndex The controller index
     * @param[out] bRaiseTriggerEdge Whether the trigger edge is raise
     * @param[out] usHoldOffTime The hold off time, unit is ns
     * @param[out] usHoldOffNum The hold off number
     * @return Execute result
     * - 0 Get the parameter successfully
     * - -1 The board is not existed
     * - -2 The controller index is over range
     * - -3 The TMU unit index is not existed
    */
    int APIENTRY dcm_GetTMUUnitParam(BYTE bySlotNo, BYTE byControllerIndex, BYTE byTMUUnitIndex, BOOL& bRaiseTriggerEdge, USHORT& usHoldOffTime, USHORT& usHoldOffNum);
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
    int APIENTRY dcm_GetTMUParameter(BYTE bySlotNo, USHORT usChannel, BOOL& bRaiseTriggerEdge, USHORT& usHoldOffTime, USHORT& usHoldOffNum);
    /**
     * @brief Start TMU measure
     * @param[in] lpszPinGroup The pin group number
     * @param[in] byMeasMode The measurement mode
     * @param[in] uSampleNum The sample number
     * @param[in] dTimeout The timeout, the unit is ms
	 * @return Execute result
	 * - 0 Set the TMU parameter successfully
	 * - -1 Not load vector file before
	 * - -2 The pin group is not define or its point is nullptr
	 * - -16 No valid board existed
	 * - -32 No Valid site
	 * - -35 The channel in the pin group is not existed
	 * - -40 The channel is not connected to TMU unit
	 * - -43 The pin group include the pin which is not belongs to current instance
    */
	int APIENTRY dcm_TMUMeasure(const char* lpszPinGroup, BYTE byMeasMode, UINT uSampleNum, double dTimeout);
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
	int APIENTRY dcm_GetTMUMeasure(BYTE bySlotNo, USHORT usChannel, BYTE& byMeasMode, UINT& uSampleNum, double& dTimeout);
    /**
     * @brief Get the measurement result
     * @param[in] lpszPinName The pin name
     * @param[in] usSiteNo The site number
     * @param[in] byMeasType The measurement type
     * @return The measurement result
	 * - !=1e15-1 The measurement result
	 * - 1e15-1 Some parameter is error or measure error
    */
    double APIENTRY dcm_GetTMUMeasureResult(const char* lpszPinName, USHORT usSiteNo, BYTE byMeasType);
    /**
     * @brief Set the operand of line
     * @param[in] lpszPinGroup The pin group name
     * @param[in] lpszStartLabel The start label
     * @param[in] ulOffset The line number offset to the label
     * @param[in] lpszOperand The operand
     * @return Execute result
	 * - 0 Set the operand successfully
	 * - -1 Not load vector before
	 * - -2 The pin group is nullptr or not defined
	 * - -6 The start label is not defined in vector file
	 * - -15 The offset is over range or the line is not in BRAM
	 * - -16 No board existed
	 * - -24 The operand is nullptr or its label is not existed or over range
     * - -32 No valid site
	 * - -43 The pin group include the pin which is not belongs to current instance
    */
    int APIENTRY dcm_SetOperand(const char* lpszPinGroup, const char* lpszStartLabel, ULONG ulOffset, const char* lpszOperand);
    /**
     * @brief Set the instruction
     * @param[in] lpszPinGroup The pin group name
     * @param[in] lpszLabel The start label
     * @param[in] nOffset The line number offset to the label
     * @param[in] lpszInstruction The instruction set
     * @param[in] lpszOperand The operand of the instruction
     * @return Execute result
     * - -1 Not load vector before
	 * - -2 The pin group is nullptr or not defined
	 * - -6 The start label is not defined in vector file
	 * - -8 The instruction is nullptr or not supported
	 * - -15 The offset is over range or the line is not in BRAM
	 * - -16 No board existed
	 * - -24 The operand is nullptr or its label is not existed or over range
     * - -32 No Valid site
	 * - -43 The pin group include the pin which is not belongs to current instance
    */
    int APIENTRY dcm_SetInstruction(const char* lpszPinGroup, const char* lpszStartLabel, ULONG ulOffset, const char* lpszInstruction, const char* lpszOpeand);
    /**
     * @brief Get the pin number of pin name
     * @param[in] lpszPinName The pin name
     * @return The pin number
     * - >=0 The pin number of the pin name
     * - -1 Not load vector before
     * - -2 The pin name is nullptr
     * - -3 The pin name is not exited
	 * - -4 The pin is not belongs to current instance
    */
    int APIENTRY dcm_GetPinNo(const char* lpszPinName);
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
    int APIENTRY dcm_GetInstruction(BYTE bySlotNo, BYTE byController, UINT uBRAMLineNo, char* lpszInstruction, int nBuffSize);
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
    int APIENTRY dcm_GetOperand(BYTE bySlotNo, BYTE byController, UINT uBRAMLineNo);
    /**
     * @brief Set the trigger out channel
     * @param[in] lpszPinName The pin name
     * @param[in] usSiteNo The site number
	 * @return Execute result
	 * - 0 Set trigger out channel successfully
     * - -1 Not load vector before
	 * - -9 The pin name is not existed in vector file
	 * - -10 The site number is over range
	 * - -16 The board is not existed
	 * - -32 The site is invalid
	 * - -35 The channel is not existed
	 * - -43 The pin is not belongs to current instance
    */
    int APIENTRY dcm_SetTriggerOut(const char* lpszPinName, USHORT usSiteNo);
    /**
     * @brief Set the pin level of I2C channel
     * @param[in] dSCL Whether set channel
     * @param[in] dVIH The input voltage of logic high
     * @param[in] dVIL The input voltage of logic low
     * @param[in] dVOH The output voltage of logic high
     * @param[in] dVOL The output voltage of logic low
     * @return Execute result
     * - 0 Set pin level successfully
     * - -1 The pin level is over range
     * - -2 No set site before
    */
    int APIENTRY dcm_I2CSetPinPinLevel(double dVIH, double dVIL, double dVOH, double dVOL, BYTE byChannelType);
    /**
     * @brief Set the stop status of I2C
     * @param[in] bHighImpedance Whether stop with high impedance
    */
    void APIENTRY dcm_I2CSetStopStatus(BOOL bHighImpedance);
    /**
     * @brief Set the dynamic load of I2C channel
	 * @param[in] byChanneltype The channel type
	 * @param[in] bEnable Whether enable dynamic load
     * @param[in] dIOH The current when pin level higher than VT
     * @param[in] dIOL The current when pin level lower than VT
     * @param[in] dVTVoltValue The VT
     * @return Execute result
     * - 0 Set dynamic load successfully
     * - -1 Not set I2C channel
     * - -2 The output current is over range
     * - -3 The VT is over range
    */
    int APIENTRY dcm_I2CSetDynamicLoad(BYTE byChannelType, BYTE byEnable, double dIOH, double dIOL, double dVTVoltValue);
    /**
     * @brief Initialize the instance
     * @param[in] nInstanceID The instance ID
    */
    void APIENTRY dcm_InitializeInstance(int nInstanceID);
    /**
     * @brief Delete instance
     * @param[in] nInstanceID The instance ID
    */
    void APIENTRY dcm_DeleteInstance(int nInstanceID);
    /**
     * @brief Set the instance ID of next operation
     * @param[in] nInstanceID  The instance ID
    */
    void APIENTRY dcm_SetInstanceID(int nInstanceID);
    /**
     * @brief Get pin count
     * @return The pin count
    */
    USHORT APIENTRY dcm_GetPinCount();
    /**
     * @brief Set the pin name owned
     * @param[in] lpszPinNameList The pin name
     * @return Execute result
     * - 0 Set pin valid successfully
     * - -1 Not load vector before
     * - -9 Pin name is not existed
    */
    int APIENTRY dcm_SetValidPin(const char* lpszPinNameList);
    /**
     * @brief Enable add pin by function
	 * @param[in] bEnable Whether enable add pin by function
	 * @param[in] bClearVector Whether clear the vector information
     * @return Execute result
     * - 0 Enable or disable function successfully
     * - -1 Load vector before
    */
    int APIENTRY dcm_EnableAddPin(BOOL bEnable, BOOL bClearVector);
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
    int APIENTRY dcm_AddPin(const char* lpszPinName, USHORT usPinNo, const char* lpszChannel);
    /**
     * @brief Clear preread vector
    */
    void APIENTRY dcm_ClearePreadVector();
    /**
     * @brief Set the dynamic load of I2C channel
	 * @param[in] byChanneltype The channel type
	 * @param[in] bEnable Whether enable dynamic load
     * @param[in] dIOH The current when pin level higher than VT
     * @param[in] dIOL The current when pin level lower than VT
     * @param[in] dVTVoltValue The VT
     * @return Execute result
     * - 0 Set dynamic load successfully
     * - -1 Not set I2C channel
     * - -2 The output current is over range
     * - -3 The VT is over range
    */
    int APIENTRY dcm_I2CSetDynamicLoad(BYTE byChannelType, BYTE byEnable, double dIOH, double dIOL, double dVTVoltValue);
    /**
     * @brief Read hard information from flash
     * @param[in] bySlotNo The slot number of the board
     * @param[out] pHardInfo The variable which will save the hard information
     * @param[in] nElementCount The array length of the hardinfo
     * @param[in] nModuleCount The number of the module saved in flash
     * @return Execute result
     * - 0 Read successfully
     * - -1 Read fail
     * - -2 The array of hard info is too small
     * - -18 The board number exceed maximum
    */
    int APIENTRY dcm_GetHardInfo(BYTE bySlotNo, STS_HARDINFO* pHardInfo, int nElementCount, int& nModuleNum);
    /**
     * @brief Set the fail saving type
     * @param[in] lpszPinGroup The pin group
     * @param[in] nSavingType The fail saving type
     * @return Execute result
     * - 0 Enable or disable successfully
	 * - -1 Not load vector file before
	 * - -2 The pin group is nullptr or not defined
	 * - -16 No valid board existed
	 * - -32 No valid site
    */
    int APIENTRY dcm_SetFailSavingType(const char* lpszPinGroup, int nSavingType);
    /**
     * @brief Enable uncompare channels not belongs to I2C
     * @param[in] bEnable Whether enable uncompare channels not belongs to I2C
    */
    void APIENTRY dcm_I2CEnableUncompare(BOOL bEnable);
#ifdef __cplusplus
}
#endif //__cplusplus
#endif //_SM8202_H_
