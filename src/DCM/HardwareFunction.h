#pragma once
/**
 * @file HardwareFunctionh
 * @brief The base function class of DCM
 * @author Guangyun Wang
 * @date 2020/02/21
 * @copyright AccoTEST Business Unit of Beijing Huafeng Test & Control Technology Co., Ltd
*/
#include "Operation.h"
#include "DriverAlarm.h"
#include "HardwareInfo.h"
#include "Relay.h"
#include "Bind.h"
/**
 * @class CHardwareFunction
 * @brief The base function of DCM
*/
class CHardwareFunction
{
public:
	/**
	 * @enum RAM_TYPE
	 * @brief RAM type
	*/
	enum class RAM_TYPE
	{
		IMM1 = 0x01,///<Command code
		IMM2 = 0x02,///<Operand
		FM = 0x08,///<Vector code of FM
		MM = 0x10,///<Vector code of MM
		IOM = 0x20,///<Vector code of IOM
		PMU_BUF = 0x40,///<PMU 
		MEM_PERIOD = 0x80,///<Period memory
		MEM_RSU_SVM1 = 0x1000,///<SVM1 in RSU memory
		MEM_RSU_LVM1 = 0x1002,///<LVM1 in RSU memory
		MEM_RSU_LVM2 = 0x1003,///<LVM2 in RSU memory
		MEM_HIS_SVM = 0x2000,///<SVM in HIS memory
		MEM_HIS_LVM1 = 0x2002,///<SVM in HIS memory
		MEM_HIS_LVM2 = 0x2003,///<SVM in HIS memory
		MEM_TIMING_FMT = 0x4000,///<FMT in Timing memory
		MEM_TIMING_T1R = 0x4001,///<T1R in Timing memory
		MEM_TIMING_T1F = 0x4002,///<T1F in Timing memory
		MEM_TIMING_IOR = 0x4003,///<IOR in Timing memory
		MEM_TIMING_IOF = 0x4004,///<IOF in Timing memory
		MEM_TIMING_STBR = 0x4005,///<STBR in Timing memory
	};
	/**
	 * @struct DATA_RESULT
	 * @brief The data result with line number
	*/
	struct DATA_RESULT
	{
		int m_nLineNo;///<The line number of BRAM or offset of DRAM
		USHORT m_usData;///<The data of controller;
		DATA_RESULT()
		{
			m_nLineNo = -1;
			m_usData = 0;
		}
	};

	/**
	* @brief The constructor
	* @param[in] bySlotNo The slot number the board inserted
	*/
	CHardwareFunction(BYTE bySlotNo, CDriverAlarm* pAlarm = nullptr);
	/**
	 * @brief Destructor
	 */
	~CHardwareFunction();
	/**
	*Set the index of the controller in board
	* @param[in] byControllerIndex The controller index
	* @return Execute result
	* - 0 Set successfully
	* - -1 The controller index is over range
	*/
	int SetControllerIndex(BYTE byControllerIndex);
	/**
	* @brief Get the controller index
	* @return The controller index
	*/
	int GetControllerIndex() const;
	/**
	 * @brief Get the slot number
	 * @return The slot number
	*/
	BYTE GetSlotNo() const;
	/**
	* @brief Whether the board is existed
	* @return
	* - TRUE Board is existed
	* - FALSE Board is not existed
	*/
	BOOL IsBoardExisted();
	/**
	* @brief Whether the controller is exist
	* @return
	* - TRUE Controller is existed
	* - FALSE Controller is not existed
	*/
	BOOL IsControllerExist();
	/**
	* @brief Get the flash ID  of the flash chip in SM8213
	* @return The Flash ID
	*/
	ULONG GetFlashID();
	/**
	 * @brief Check flash ID
	 * @return Check result
	 * - TRUE Check pass
	 * - FALSE Check fail
	*/
	BOOL CheckFlashID();
	/**
	*Read the data from flash
	* @param[in] bySectorNo The sector of flash
	* @param[in] byPageNo The page index of flash
	* @param[in] byOffset The read base index offset to first line
	* @param[in] usReadByteCount The read data bytes
	* @param[in] pbyData The data buff save the data read
	* @return Execute result
	* - 0 Write flash data successfully
	* - >0 The error byte count
	* - -1 Sector number is over range
	* - -2 The page number is over range
	* - -3 The offset is over range
	* - -4 The write data byte is over range
	* - -5 The write data byte is 0 or parameter is nullptr
	* - -6 Check flash ID fail
	*/
	int ReadFlash(BYTE bySectorNo, BYTE byPageNo, BYTE byOffset, USHORT usReadByteCount, BYTE* pbyData);
	/**
	* @brief Write the data to flash
	* @param[in] bySectorNo The sector of flash
	* @param[in] byPageNo The page index of flash
	* @param[in] byOffset The read base index offset to first line
	* @param[in] usWriteByteCount The write data bytes
	* @param[in] pbyWriteData The data buff save the data read
	* @return Execute result
	* - 0 Write flash data successfully
	* - >0 The error byte count
	* - -1 Sector number is over range
	* - -2 The page number is over range
	* - -3 The offset is over range
	* - -4 The write data byte is over range
	* - -5 The write data byte is 0 or parameter is nullptr
	* - -6 Check flash ID fail
	*/
	int WriteFlash(BYTE bySectorNo, BYTE byPageNo, BYTE byOffset, USHORT usWriteByteCount, BYTE* pbyWriteData);
	/**
	* @brief Erase the flash of specific sector
	* @param[in] bySectorNo The sector of flash
	* @return Execute result
	* - 0 Erase flash data successfully
	* - -1 Sector number is over range
	* - -2 Erase flash fail
	*/
	int EraseFlash(BYTE bySectorNo);
	/**
	* @brief Get the logic revision of SM8213
	* @return The logic revision
	*/
	USHORT GetBoardLogicRevision();
	/**
	*Get the logic revision of SE8213
	* @return The logic revision
	*/
	USHORT GetControllerLogicRevision();
	/**
	* @brief Initialize MCU
	* @param[in] vecChannel The channel
	* @return Execute result
	* - 0 Initialize MCU successfully
	* - -1 The channel is over range
	*/
	int InitMCU(const std::vector<USHORT>& vecChannel);
	/**
	* @brief Connect or disconnect the function relay
	* @param[in] vecChannel The channel whose relay will be connect or disconnect
	* @param[in] bConnect Whether connect relay
	* @return Execute result
	* 0 Connect or disconnect relay successfully
	* - -1 The channel is over range
	*/
	int SetFunctionRelay(const std::vector<USHORT>& vecChannel, BOOL bConnect = TRUE);
	/**
	* @brief Get the channel whose relay is connect
	* @param[out] vecChannel The channel number whose relay is connected
	* @param[in] RelayType The relay type
	 * @return Execute result
	 * - 0 Get the connect channel successfully
	 * - -1 The relay type is error
	*/
	int GetConnectChannel(std::vector<USHORT>& vecChannel, RELAY_TYPE RelayType = RELAY_TYPE::FUNC_RELAY);
	/**
	* @brief Set the pin level after calibration of the specific channel
	* @param[in] vecChannel The channel number whose pin level will be set
	* @param[in] dVIH The VIH value after calibration
	* @param[in] dVIL The VIL value after calibration
	* @param[in] dVT The VT value after calibration
	* @param[in] dVOH The VOH value after calibration
	* @param[in] dVOL The VOL value after calibration
	* @param[in] dClampHigh The high clamp value after calibration
	* @param[in] dClampLow The low clamp value after calibration
	* @return Execute result
	* - 0 Set pin level successfully
	* - -1 The channel is over range
	* - -2 The pin level is over range
	*/
	int SetPinLevel(const std::vector<USHORT>& vecChannel, double dVIH, double dVIL, double dVT, double dVOH, double dVOL, double dClampHigh = CLx_LEVEL_MAX, double dClampLow = CLx_LEVEL_MIN);
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
	* @brief Set the period of the specific timeset
	* @param[in] byTimesetIndex The timeset index
	* @param[in] dPeriod The period will set to timeset
	* @return Execute result
	* - 0 Set timeset period successfully
	* - -1 The timeset index is over range
	* - -2 The period is over range
	* - -3 Set period fail
	*/
	int SetPeriod(BYTE byTimesetIndex, double dPeriod);
	/**
	* @brief Get the period of the specific timeset
	* @param[in] byTimesetIndex The timeset index
	* @return The period of timeset
	* - >0 The period of timeset
	* - -1 The timeset index is over range
	*/
	double GetPeriod(BYTE byTimesetIndex);
	/**
	* @brief Set edge and format of timeset
	* @param[in] vecChannel The channel number whose edge will be set
	* @param[in] byTimesetIndex The timeset index
	* @param[in] WaveFormat The waver format
	* @param[in] pdEdge The edge value of T1R/T1F/IOR/IOF/STBR/STBF
	* @return Execute result
	* - 0 Set the edge successfully
	* - -1 The timeset index is over range
	* - -2 The format is error
	* - -3 The point of edge value is nullptr
	* - -4 The value is over range
	*/
	int SetEdge(const std::vector<USHORT>& vecChannel, BYTE byTimesetIndex, const double* pdEdge, WAVE_FORMAT WaveFormat, IO_FORMAT IOFormat, COMPARE_MODE CompareMode = COMPARE_MODE::EDGE);
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
	* @brief Read the data in memory
	* @param[in] MemoryType The memory type
	* @param[in] DataType The data type will be read
	* @param[in] uPatternStartLine The base pattern start line of data read
	* @param[in] uPatternLineCount The pattern data line count read
	* @param[out] pusDataBuff The data buff which will save the data read
	* @return Execute result
	* - 0 Read memory successfully
	* - -1 The memory type is error
	* - -2 The data type is not supported
	* - -3 The start line is over range;
	* - -4 The data count is over range
	* - -5 The line count read is 0 or the data buff is nullptr
	*/
	int ReadDataMemory(MEM_TYPE MemoryType, DATA_TYPE DataType, UINT uPatternStartLine, UINT uPatternLineCount, USHORT* pusDataBuff);
	/**
	* @brief Write the data to memory
	* @param[in] MemoryType The memory type
	* @param[in] DataType The data type will be written
	* @param[in] uPatternStartLine The base pattern start line of data written
	* @param[in] uWritePatternLineCount The data pattern line count written
	* @param[out] pusData The data which will be written to memory
	* @return Execute result
	* - 0 Write memory successfully
	* - -1 The memory type is error
	* - -2 The start line is over range
	* -  -3 The data count is over range
	* - -4 The line count written is 0 or the data buff is nullptr
	*/
	int WriteDataMemory(MEM_TYPE MemoryType, DATA_TYPE DataType, UINT uPatternStartLine, UINT uWritePatternLineCount, const USHORT* pusData);
	/**
	* @brief Write the data of specific channel to memory
	* @param[in] MemoryType The memory type
	* @param[in] DataType The data type will be written
	* @param[in] usChannel The channel number
	* @param[in] uStartLine The base start line of data written
	* @param[in] uWriteLineCount The data line count written
	* @param[out] pulData The data which will be written to memory, bit 0 is first line
	* @return Execute result
	* - 0 Write memory successfully
	* - -1 The memory type is error
	* - -2 The start line is over range
	* - -3 The data count is over range
	* - -4 The line count written is 0 or the data buff is nullptr
	*/
	int WriteChannelDataMemory(MEM_TYPE MemoryType, DATA_TYPE DataType, USHORT usChannel, UINT uStartLine, UINT uWriteLineCount, ULONG* pulData);
	/**
	 * @brief Set the channel mode to MCU
	 * @param[in] vecChannel The channel whose channel mode will be set
	 * @return Execute result
	 * - 0 Set the mode successfully
	 * - -1 The channel is over range
	*/
	int SetChannelMCUMode(const std::vector<USHORT>& vecChannel);
	/**
	 * @brief Set the compared channel
	 * @param[in] setChannel The channel number
	*/
	void SetComparedChannel(const std::set<USHORT>& setChannel);
	/**
	* @brief Set run parameter of running, must set running parameter before run vector
	* @param[in] uStartLineNo The start line number of BRAM
	* @param[in] uStopLineNo The stop line number of BRAM
	* @param[in] bWithDRAM Whether the vector between Start line and stop line has vector in DRAM
	* @param[in] uDRAMStartLineNo The first start line number in DRAM between start line and stop line in BRAM
	* @param[in] bEnableStart Whether enable start
	* @return Execute result
	* - 0 Run vector successfully
	* - -1 The start line number of BRAM is over range
	* - -2 The stop line number is over range
	* - -3 The stop line is not after start line
	* - -4 The start line of DRAM is over range
	* - -5 Unknown error
	*/
	int SetRunParameter(UINT uStartLineNo, UINT uStopLineNo, BOOL bWithDRAM = FALSE, UINT uDRAMStartLineNo = 0, BOOL bEnableStart = TRUE);
	/**
	 * @brief Set the channel status
	 * @param[in] vecChannel The channel number
	 * @param[in] ChannelStatus The pin status
	 * @return Execute result
	 * - 0 Set channel status successfully
	 * - -1 The channel status is not supported
	 * - -2 The channel is over range
	*/
	int SetChannelStatus(const std::vector<USHORT>& vecChannel, CHANNEL_OUTPUT_STATUS ChannelStatus);
	/**
	 * @brief Disable the channels
	 * @param[in] vecChannel The channel be disable
	 * @return Execute result
	 * - 0 Disable channel successfully
	 * - -1 The channel is over range
	*/
	int DisableChannel(const std::vector<USHORT>& vecChannel);
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
	 * @brief Update channel mode
	*/
	void UpdateChannelMode();
	/**
	* @brief Run vector after set running parameter
	*/
	void Run();
	/**
	* @brief Run synchronously
	*/
	void SynRun();
	/**
	 * @brief Disable receive start signal
	 * @param[in] bEnable Enable start
	*/
	void EnableStart(BOOL bEnable);
	/**
	* @brief Stop the running vector
	*/
	void StopRun();
	/**
	* @brief Get the running status
	* @return Execute result
	* - 0 Running
	* - 1 Stop
	* - -1 Not ran before
	*/
	int GetRunningStatus();
	/**
	* @brief Get the stop line number of last running
	* @return The stop line number
	* - >=0 The stop line number
	* - -1 Not ran vector
	* - -2 Vector running
	*/
	int GetStopLineNo();
	/**
	 * @brief Get the running line count
	 * @return The line count in latest run
	*/
	ULONG GetRunLineCount();
	/**
	* @brief Get the vector running result
	* @return Run result
	* - 0 Running pass
	* - 1 Running fail
	* - -1 Not ran before
	* - -2 Vector running
	* - -3 Get result fail
	*/
	int GetResult();
	/**
	* @brief Get the vector running result of specific channel
	* @param[in] usChannel The channel index
	* @return Run result
	* - 0 Running pass
	* - 1 Running fail
	* - -1 The channel is over range
	* - -2 Not running before
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
	* @brief Get the channel fail count of latest running
	* @param[in] usChannel The channel number
	* @return Fail line count
	* - >=0 The fail line count
	* - -1 The channel is over range
	* - -2 Not ran before
	* - -3 Vector running
	*/
	int GetChannelFailCount(USHORT usChannel);
	/**
	 * @brief Get the fail count of all channel
	 * @return Fail line count
	 * - -1 Not ran before
	 * - -2 Vector running
	*/
	int GetFailCount();
	/**
	 * @brief Get the fail data of all channel
	 * @param[out] vecBRAMLine The fail data in BRAM
	 * @param[out] vecDRAMLine The fail data in DRAM
	 * @return Execute result
	 * - 0 Get fail line number successfully
	 * - -1 Not ran before
	 * - -2 Vector running
	*/
	int GetFailData(std::vector<DATA_RESULT>& vecBRAMLine, std::vector<DATA_RESULT>& vecDRAMLine);
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
	* @brief Connect or disconnect the calibration relay
	* @param[in] usChannel The channel whose relay will be connect or disconnect
	* @param[in] bConnect Whether connect relay
	* @return Execute result
	* - 0 Connect or disconnect relay successfully
	* - -1 The channel is over range
	*/
	int SetCalibrationRelay(USHORT usChannel, BOOL bConnect = FALSE);
	/**
	* @brief Init the PMU mode of specific channel
	* @param[in] vecChannel The channel index
	* @return Execute result
	* - 0 Initialize PMU successfully
	* - -1 The channel is over range
	* - -2 Initialize PMU fail
	*/
	int InitPMU(const std::vector<USHORT>& vecChannel);
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
	*/
	int SetPMUMode(const std::vector<USHORT>& vecChannel, PMU_MODE PMUMode, PMU_IRANGE Range, double dSetValue, double dClmapHigh, double dClampLow);
	/**
	 * @brief Get the channel whose PMU is clamp
	 * @param[in] vecChannel The channels which will be check clamp status
	 * @param[out] mapClampChannel The clamp channel and its clamp type, key is channel and value is clamp type(0 is voltage clamp and 1 is current clamp)
	*/
	void GetClampChannel(const std::vector<USHORT>& vecChannel, std::map<USHORT, UCHAR>& mapClampChannel);
	/**
	 * @brief Enable PMU clamp
	 * @param[in] vecChannel The channel number
	 * @param[in] bEnable Whether enable clamp
	 * @return Execute result
	 * - 0 Set clamp successfully
	 * - -1 The channel number is over range
	*/
	int EnablePMUClampFlag(const std::vector<USHORT>& vecChannel, BOOL bEnable);
	/**
	* @brief Get PMU settings of specific channel
	* @param[in] usChannel The channel index
	* @param[out] PMUMode The PMU mode
	* @param[out] PMURange The current range of PMU
	* @return The setting value of current or voltage depends on the PMU mode
	* - 1e15 The channel is over range
	*/
	double GetPMUMode(USHORT usChannel, PMU_MODE& PMUMode, PMU_IRANGE& PMURange);
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
	* @brief Set high voltage out
	* @param[in] vecChannel The channel index
	* @param[in] bEnable Whether enable high voltage out
	* @param[in] dVoltage The voltage
	* @return Execute result
	* - 0 Set high voltage out successfully
	* - -1 The channel is over range
	* - -2 The voltage is over range
	*/
	int SetHighMode(const std::vector<USHORT>& vecChannel, BOOL bEnable, double dVoltage);
	/**
	 * @brief Initialize the TMU unit
	 * @return Execute result
	 * - 0 Initialize TMU successfully
	 * - -1 Initialize TMU fail
	*/
	int InitTMU();
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
	 * @brief Get the unit connected to the channel
	 * @param[in] usChannel The channel will be 
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
	 * @brief Get the TMU parameter
	 * @param[in] usChannel The channel number
	 * @param[out] bRaiseTriggerEdge Whether the trigger edge is raise
	 * @param[out] usHoldOffTime The hold off time, unit is ns
	 * @param[out] usHoldOffNum The hold off number
	 * @return Execute result
	 * - 0 Set the parameter successfully
	 * - -1 The channel is over range
	 * - -2 The The channel is not connect to the TMU unit
	*/
	int GetTMUParameter(USHORT usChannel, BOOL& bRaiseTriggerEdge, USHORT& usHoldOffTime, USHORT& usHoldOffNum);
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
	 * @brief Enable TMU unit's self check
	 * @param[in] byUnitIndex The unit index
	 * @param[in] bEnable Whether enable self check
	 * @return Execute result
	 * - 0 Enable or disable TMU self check successfully
	 * - -1 The unit index is over range
	*/
	int EnableTMUSelfCheck(BYTE byUnitIndex, BOOL bEnable);
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
	double GetTMUMeasureResult(USHORT usChannel, TMU_MEAS_TYPE MeasType, int& nError);
	/**
	 * @brief Get the measure type
	 * @param[in] usChannel The channel number
	 * @param[in] MeasType The measurement type
	 * @param[out] nError The TMU measasure error
	 * - -1 The measurement type is not supported
	 * - -2 The measurement type is not measured before
	 * - -3 The measurement is not stop in timeout
	 * - -4 The TMU measurement is timeout
	 * - -5 The bind unit of measurement is not stop in timeout
	 * - -6 The bind unit is timeout
	 * - -7 The edge measurement error
	 * @return The measurement result
	 * - != TMU_ERROR The measurement result
	*/
	double GetTMUUnitMeasureResult(BYTE byUnit, TMU_MEAS_TYPE MeasType, int& nError);
	/**
	 * @brief Set the IO delay
	 * @param[in] usChannel The channel number
	 * @param[in] dData The delay of data, unit ns
	 * @param[in] dDataEn The delay of data enable, unit ns
	 * @param[in] dHigh The delay of high, unit ns
	 * @param[in] dLow The delay of low, unit ns
	 * @return Execute result
	 * - 0 Set IO delay successfully
	 * - -1 The channel is over range
	*/
	int SetIODelay(USHORT usChannel, double dData, double dDataEn, double dHigh, double dLow);
	/**
	 * @brief Get the IO delay value
	 * @param[in] usChannel The channel number
	 * @param[out] pdData The delay of data, unit ns
	 * @param[out] pdDataEn The delay of data enable, unit ns
	 * @param[out] pdHigh The delay of high, unit ns
	 * @param[out] pdLow The delay of low, unit ns
	 * @return Execute result
	 * - 0 Get IO delay successfully
	 * - -1 The channel is over range
	 * - -2 The point is nullptr
	*/
	int GetIODelay(USHORT usChannel, double* pdData, double* pdDataEn, double* pdHigh, double* pdLow);
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
	 * @brief Update delay
	*/
	void UpdateDelay();
	/**
	 * @brief Delay us
	 * @param[in] ulUs The time to delay, unit is us
	*/
	void DelayUs(ULONG ulUs);
	/**
	 * @brief Delay ms
	 * @param[in] ulMs The time to delay, unit is ms
	*/
	void DelayMs(ULONG ulMs);
	/**
	* @brief Read BRAM function, only support MEM_PERIOD, RSU_SVM, RSU_LVM, HIS_SVM, HIS_LVM, MEM_TIMG
	* @param[in] RAMType RAM type
	* @param[in] subType Sub BRAM type
	* @param[in] uStartLine The start line of data read
	* @param[in] uLineCount The data line count read
	* @param[out] pulData The data buff which will save the data read
	* @return Execute result
	* - 0 Read memory successfully
	* - -1 The BRAM type or sub BRAM type is error
	* - -2 The start line is over range
	* - -3 The data count is over range
	* - -4 The line count read is 0 or the data buff is nullptr
	*/
	int ReadBRAMMemory(RAM_TYPE RAMType, UINT uStartLine, UINT uLineCount, ULONG* pulData);
	/**
	* @brief Write BRAM function, only support MEM_PERIOD, RSU_SVM, RSU_LVM, HIS_SVM, HIS_LVM, MEM_TIMG
	* @param[in] RAMType RAM type
	* @param[in] uStartLine The start line of data write
	* @param[in] uLineCount The data line count write
	* @param[out] pulData The data buff which saved the data write
	* @return Execute result
	* - 0 Write memory successfully
	* - -1 The BRAM type or sub BRAM type is error
	* - -2 The start line is over range
	* - -3 The data count is over range
	* - -4 The line count write is 0 or the data buff is nullptr
	*/
	int WriteBRAMMemory(RAM_TYPE RAMType, UINT uStartLine, UINT uLineCount, const ULONG* pulData);
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
	 * @brief Get the calibration data
	 * @param[out] pCalibrationData The calibration data of each channel
	 * @param[in] nElementCount The element count of array
	 * @return The channel count
	 * - >=0  The channel count
	 * - -1 The flash status is error
	 * - -2 Allocate memory fail
	 * - -3 Data in flash is error
	*/
	int GetCalibrationData(CAL_DATA* pCalibrationData, int nElementCount);
	/**
	 * @brief Reset the calibration data of all channels in controller
	 * @return Execute result
	 * - 0 Execute result
	 * - -1 The memory of the calibration data is not set before
	*/
	int ResetCalibrationData();
	/**
	 * @brief Reset the calibration data of channel
	 * @param[in] usChannel The channel number
	 * @return Execute result
	 * - 0 Reset calibration data successfully
	 * - -1 The memory of the calibration data is not set before
	 * - -2 The channel number is over range
	*/
	int ResetCalibrationData(USHORT usChannel);
	/**
	 * @brief Get the calibration data from flash
	 * @param[in] pCalibrationData The calibration data of each channel
	 * @param[out] nElementCount The element count of the array
	 * @return Execute result
	 * - 0  Read calibration data successfully
	 * - -1 The flash status is error
	 * - -2 Allocate memory fail
	 * - -3 Data in flash is error
	*/
	int ReadCalibrationData();
	/**
	 * @brief Get the capture data of all channel
	 * @param[out] mapBRAM The capture data in BRAM
	 * @param[out] mapDRAM The capture data in DRAM
	 * @return Execute result
	 * - 0 Get the capture successfully
	 * - -1 Vector not ran
	 * - -2 Vector running
	*/
	int GetCapture(std::vector<DATA_RESULT>& vecBRAMLine, std::vector<DATA_RESULT>& vecDRAMLine);
	/**
	 * @brief Set the vector valid
	 * @param[in] vecController The controller will be set
	 * @param[in] bValid Whether the vector valid
	*/
	void SetVectorValid(BOOL bValid);
	/**
	 * @brief Get the controller whose controller is valid
	 * @return Whether vector valid
	 * - TRUE The vector is valid
	 * - FALSE The vector is invalid
	*/
	BOOL IsVectorValid();
	/**
	 * @brief Set check mode
	 * @param[in] bSaveLineOrder Whether save the line ran order
	 * @param[in] CheckDataType The check data type
	 * @param[in] bSwitch Whether switch DRAM
	 * @param[in] bRead Whether read memory before write
	 * @return Execute result
	 * - 0 Set check successfully
	 * - -1 The check data type is not supported
	*/
	int SetPatternMode(BOOL bSaveLineOrder, DATA_TYPE CheckDataType = DATA_TYPE::FM, BOOL bSwitch = FALSE, BOOL bRead = FALSE);
	/**
	 * @brief Get the channel status
	 * @return The channel status
	*/
	UINT GetChannelStatus();
	/**
	 * @brief Get the line order
	 * @param[out] mapBRAM The line order
	 * @return Execute result
	 * - 0 Get line order successfully
	*/
	int GetLineRanOrder(std::vector<UINT>& vecBRAM);
	/**
	 * @brief Get the line order count
	 * @return The line order count
	*/
	int GetLineOrderCount();
	/**
	 * @brief Get pattern count in latest ran
	 * @return The pattern count
	*/
	UINT GetRanPatternCount();
	/**
	 * @brief Reset the controller
	*/
	void SoftReset();
	/**
	 * @brief Set the trigger out channel
	 * @param[in] usChannel The channel number
	 * @return Execute result
	 * - 0 Set the trigger out channel successfully
	 * - -1 The channel is over range
	*/
	int SetTriggerOut(USHORT usChannel);
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
	 * @brief Enable only save the fail information of selected line
	 * @param[in] bEnable Enable save select fail
	*/
	void EnableSaveSelectedFail(BOOL bEnable);
	/**
	 * @brief Check whether support saving fail selected
	 * @return Whether support fail selected
	 * - TRUE Support fail selected
	 * - FALSE Not support
	*/
	BOOL IsSupportSaveFailSelected();
	/**
	 * @brief Check whether support fail synchronzation
	 * @return Whether support fail synchronzation
	 * - TRUE Support fail synchronzation
	 * - FALSE Not support
	*/
	BOOL IsSupportFailSyn();
private:
	/**
	* @brief Get the flash status
	* @return The status of flash status
	*/
	BYTE GetFlashStatus();
	/**
	 * @brief Save the calibration data to flash
	 * @param[out] pCalibrationData The calibration data of each channel
	 * @param[in] nElementCount The element count of array
	 * @return Execute result
	 * - 0 Save calibration data successfully
	 * - -1 Flash status error
	 * - -2 Write flash fail
	*/
	int SaveCalibrationData();
	/**
	 * @brief Check the edge value
	 * @param[in] dSmallerEdge The smaller edge
	 * @param[in] dBiggerEdge  The bigger edge
	 * @param[in] lpszBiggerEdgeName The name of bigger edge
	 * @return Execute result
	 * - 0 Edge check pass
	 * - -1 Edge check fail
	*/
	inline int EdgeCheck(double dSmallerEdge, double dBiggerEdge, const char* lpszBiggerEdgeName);
	/**
	 * @brief Wait DRAM is ready
	 * @return Execute result
	 * - 0 DRAM is ready
	 * - -1 DRAM is timeout
	*/
	inline int WaitDRAMReady();
	/**
	 * @brief Wait TMU unit stop
	 * @param[in] byUnitIndex The TMU unit index
	 * @return Execute result
	 * - 0 TMU is stop
	 * - -1 Wait TMU timeout
	 * - -2 TMU measurement timeout
	*/
	inline int WaitTMUStop(BYTE byUnitIndex);
	/**
	 * @brief Start PMU
	 * @param[in] uSampleTime The sample times
	 * @param[in] byStartEvenChannel The start channel type, one bit one chip, 1 is even channel
	*/
	inline void StartPMU(UINT uSampleTimes, BYTE byStartEvenChannel);
	/**
	 * @brief Check whether the fail memory filled
	 * @param[in] nBRAMFailLineCountSaved The fail line saved in BRAM
	 * @param[in] nDRAMFailLineCountSaved The fail line saved in DRAM
	*/
	inline void CheckFailMemoryFilled(int nBRAMFailLineCountSaved, int nDRAMFailLineCountSaved);
private:
	struct PMU_STATUS
	{
		ULONG m_ulDACCode;
		ULONG m_ulIRangeMD;
		PMU_STATUS()
		{
			m_ulDACCode = 0;
			m_ulIRangeMD = 0;
		}
	};
	ULONG m_ulBoardVer;///<The board version
	BYTE m_bySlotNo;///<The slot number which board is inserted;
	BYTE m_byControllerIndex;///<The controller index
	BOOL m_bLatestRanWithDRAM;///<The latest ran with DRAM
	COperation m_Operation;///<The operation class access hardware
	CDriverAlarm* m_pAlarm;///<The instance of class CDriverAlarm
	PMU_STATUS m_PMUStatus[DCM_CHANNELS_PER_CONTROL];
	BOOL m_bGetCalibrationData;///<Whether get calibration data from flash
	CRelay* m_pRelay;///<The relay class
	BYTE m_byBindTMU[TMU_UNIT_COUNT_PER_CONTROLLER];///<The bind TMU unit of each unit
	BOOL m_bBRAMFailMemoryFilled;///<Whether the fail memory of BRAM is filled
	BOOL m_bDRAMFailMemoryFilled;///<Whether the fail memory of DRAM is filled
	BOOL m_byPMUStatus[2];///<The PMU start status, 0: No PMU wait start;1:PMU wait start;2:PMU result wait to gotten
	std::map<USHORT, ULONG> m_mapPMUMeasure[2];///<The PMU needed to be start
	BYTE m_byPMUMeasureChip[2];///<The PMU chip needed to be start measurement
	UINT m_uSampleTimes;///<The sample times of the PMU needed to start measure
	UINT m_byPMUMeasureChipEven[2];///<The sample times of the PMU needed to start measure
	std::set<USHORT> m_setUnComparedChannel;///<The channel will uncompared
	BOOL m_bSelectFail;///<Only save the fail information of selected line
};