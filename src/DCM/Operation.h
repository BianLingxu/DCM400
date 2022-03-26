#pragma once
/**
 * @file Operation.h
 * @brief The class be used to operate memory and register of DCM
 * @author Guangyun Wang
 * @date 2020/02/20
 * @copyright AccoTEST Business Unit of Beijing Huafeng Test & Control Technology Co., Ltd.
*/
#include "StdAfx.h"
#include <map>
#include "BaseOperation.h"

class COperation
{
public:
	/**
	 * @enum BRAM_TYPE
	 * @brief BRAM type
	 */
	enum class BRAM_TYPE
	{
		IMM1 = 0x01,///<Command code
		IMM2 = 0x02,///<Operand
		IMM3 = 0x04,///<Controller bit
		FM = 0x08,///<Vector code of FM
		MM = 0x10,///<Vector code of MM
		IOM = 0x20,///<Vector code of IOM
		PMU_BUF = 0x40,///<PMU 
		MEM_PERIOD = 0x80,///<Period memory
		MEM_RSU = 0x100,///<RSU memory
		MEM_HIS = 0x200,///<HIS memory
		MEM_TIMING = 0x400,///<Timing memory
	};
	/**
	 * @enum DRAM_TYPE
	 * @brief DRAM data type
	 */
	enum class DRAM_TYPE
	{
		FM = 0x01,///<FM
		MM = 0x02,///<MM
		IOM = 0x04,///<IOM
		CMD = 0x08,///<Command
		DRAM5 = 0x10,///<not used
		DRAM6 = 0x20,///<not used
	};
	/**
	 * @enum REG_TYPE
	 * @brief Register type
	 */
	enum class REG_TYPE
	{
		SYS_REG = 0,///<System register
		FUNC_REG = 2,///<Function register
		TMU_REG = 3,///<TMU register
		CAL_REG = 4,///<Calibration register
	};
	/**
	 * @brief Constructor
	 * @param[in] bySlotNo The slot number the board inserted
	 */
	COperation(BYTE bySlotNo);
	/**
	 * @brief Destructor
	 */
	~COperation();
	/**
	 * @brief Get the slot number of board
	 * @return The slot number
	 */
	BYTE GetSlotNo() const;
	/**
	 * @brief Set the index of the controller in board
	 * @param[in] byControllerIndex The controller index
	 * @return Execute result
	 * - 0 Set successfully
	 * - -1 The controller index is over range
	 */
	int SetControllerIndex(BYTE byControllerIndex);
	/**
	 * @brief Get the controller index of the operation
	 * @return The controller index
	 */
	BYTE GetControllerIndex() const;
	/**
	 * @brief Read data from the memory in FPGA
	 * @param[in] RAMType The memory type
	 * @param[in] uRAMAddress The base memory address will be read
	 * @param[in] nReadDataCount The data count will be read
	 * @param[out] pulDataBuff The data buff which will save the data read
	 * @return Execute result
	 * - 0 Read data successfully
	 * - -1 The memory type is error
	 * - -2 The data count is 0 or the data parameter is nullptr
	 * - 0x80000000 Can't find the read method
	 */
	int ReadRAM(BRAM_TYPE RAMType, USHORT usRAMAddress, UINT uReadDataCount, ULONG* pulDataBuff);
	/**
	 * @brief Write data to the memory in FPGA
	 * @param[in] RAMType The memory type
	 * @param[in] uAddress The address which the data will be written to
	 * @param[in] nWriteDataCount The bytes count will be written
	 * @param[in] pulData The data will be written
	 * @return Execute result
	 * - 0 Write data successfully
	 * - -1 The memory type is error
	 * - -2 The data count is 0 or the data parameter is nullptr
	 * - 0x80000000 Can't find the read method
	 */
	int WriteRAM(BRAM_TYPE RAMType, USHORT usRAMAddress, UINT uWriteDataCount, const ULONG* pulData);
	/**
	 * @brief Read data from the DRAM in SE8212
	 * @param[in] DRAMType The memory type
	 * @param[in] uStartLineNo The start line number will be read from
	 * @param[in] uReadDataCount The data line count will be read
	 * @param[out] pulDataBuff The data buff which will save the data read
	 * @return Execute result
	 * - 0 Read data successfully
	 * - -1 The memory type is error
	 * - -2 The data count is 0 or the data parameter is nullptr
	 * - -3 The start line number is over range
	 * - -4 The data count read is over range
	 * - 0x80000001 Unknow error
	 * - 0x80000000 Can't find the read method
	 */
	int ReadDRAM(DRAM_TYPE DRAMType, UINT uStartLineNo, UINT uReadDataCount, ULONG* pulDataBuff);
	/**
	 * @brief Write data to the DRAM in SE8212
	 * @param[in] RAMType The memory type
	 * @param[in] uStartLineNo The start line number will be read from
	 * @param[in] nWriteDataCount The data line count will be written
	 * @param[in] pulData The data will be written
	 * @return Execute result
	 * - 0 Write data successfully
	 * - -1 The memory type is error
	 * - -2 The data count is 0 or the data parameter is nullptr
	 * - -3 The start line number is over range
	 * - -4 The data count written is over range
	 * - 0x80000001 Unknow error
	 * - 0x80000000 Can't find the read method
	 */
	int WriteDRAM(DRAM_TYPE DRAMType, UINT uStartLineNo, UINT uWriteDataCount, ULONG* pulData);
	/**
	 * @brief Check whether the DRAM is ready
	 * @return Whether DRAM is ready
	 * - 0 DRAM is not ready
	 * - 1 DRAM ready
	*/
	int IsDRAMReady();
	/**
	 * @brief Get the status of ATE305
	 * @return The data in status register of ATE305
	 * - The data in status register of ATE305
	 * - 0x80000000 Can't find the read method
	 */
	int Get305Status();	
	/**
	 * @brief Read data from ATE305
	 * @param[in] byAddress The SPI address
	 * @param[I/O] mapChannelData The channel will be read and the data read
	 * @return The data in the specific address
	 * - 0x80000000 Can't find the read method
	 */
	int Read305(BYTE byAddress, std::map<USHORT, ULONG>& mapChannelData);
	/**
	 * @brief Write data to ATE305
	 * @param[in] byAddress The SPI address
	 * @param[in] mapChannelData The channel will be write and the data written
	 * @return Execute result
	 * - 0 Write data successfully
	 * - 0x80000000 Can't find the read method
	 */
	int Write305(BYTE byAddress, std::map<USHORT, ULONG>& mapChannelData);
	/**
	 * @brief Start PMU measure
	 * @param[in] nSampleDepth The sample depth
	 * @return Execute result
	 * - 0 Start PMU successfully
	 * - 0x80000000 Can't find the read method
	*/
	int PMUStart(int nSampleDepth);
	/**
	 * @brief Write data to 7606
	 * @param[in] ulData The data written
	 * @return Execute result
	 * - 0 Write data successfully
	 * - 0x80000000 Can't find the read method
	*/
	int Write7606(ULONG ulData);
	/**
	 * @brief Read AD7606 parameter
	 * @param[I] usChannel AD7606 channel
	 * @param[I] nSampelDepth Sample depth
	 * @param[O] pulBuff Data buff
	 * @return Execute result
	 * - 0 Read AD7606 success
	 * - -1: Ret AD7606 fail
	 */
	int Read7606(USHORT usChannel, int nSampleDepth, ULONG *pulBuff);
	/*
	 *Read data from register.
	 * @param[I] RegisterType The register type.
	 * @param[I] usREGAddress The register address.
	 * @return The data read.
	 */
	ULONG ReadRegister(REG_TYPE RegisterType, USHORT usREGAddress);
	/**
	 * @brief Write data to register
	 * @param[in] RegisterType The register type
	 * @param[in] usREGAddress The register address
	 * @param[in] ulData The data will be written
	 * @return Execute result
	 * - 0 Write data successfully
	 * - -1 The register type is error
	 * - 0x80000000 Can't find the operation method
	 */
	int WriteRegister(REG_TYPE RegisterType, USHORT usREGAddress, ULONG ulData);
	/**
	 * @brief Read data from SM8213
	 * @param[in] usREGAddress The address will be read
	 * @param[in] uReadDataCount The data line count will be read
	 * @param[out] pulDataBuff The data buff which will save the data read
	 * @return Execute result
	 * - 0 Read data successfully
	 * - -1 The data count is 0 or the data parameter is nullptr
	 * - 0x80000000 Can't find the read method
	 */
	int ReadBoard(USHORT usAddress, UINT uReadDataCount, ULONG* pulDataBuff);
	/**
	 * @brief Write data to SM8213
	 * @param[in] usREGAddress The address will be written
	 * @param[in] nWriteDataCount The data line count will be written
	 * @param[in] pulData The data will be written
	 * @return Execute result
	 * - 0 Write data successfully
	 * - -1 The data count is 0 or the data parameter is nullptr
	 * - 0x80000001 Unknow error
	 * - 0x80000000 Can't find the read method
	 */
	int WriteBoard(USHORT usAddress, UINT uWriteDataCount, ULONG* pulData);

	/**
	 * @brief Read data from SM8213
	 * @param[in] usREGAddress The address will be read
	 * @param[in] uReadDataCount The data line count will be read
	 * @param[out] pulDataBuff The data buff which will save the data read
	 * @return Execute result
	 * - 0 Read data successfully
	 * - 0xFFFFFFFF Can't find the read method
	 */
	ULONG ReadBoard(USHORT usAddress);
	/**
	 * @brief Write data to SM8213
	 * @param[in] usREGAddress The address will be written
	 * @param[in] nWriteDataCount The data line count will be written
	 * @param[in] pulData The data will be written
	 * @return Execute result
	 * - 0 Write data successfully
	 * - -1 Can't find the read method
	 * - -2 Unknow error
	 */
	int WriteBoard(USHORT usAddress, ULONG ulData);
	/**
	 * @brief Wait time 
	 * @param[in] dUs The time to wait in us
	*/
	void WaitUs(double dUs);
private:
	/**
	 * @enum OPERATION_TYPE
	 * @brief The operation type
	*/
	enum OPERATION_TYPE
	{
		BRAM = 0,///<The BRAM
		DRAM,///<The DRAM
		COM_TYPE,///<The COM type
		ATE305_TYPE,///<The ATE305
		AD7606_TYPE,///<The AD7606
		SYS_REG_TYPE,///<The system register
		FUNC_REG_TYPE,///<The function register
		TMU_REG_TYPE,///<The TMU type
		CAL_REG_TYPE,///<The calibration type
		BOARD_TYPE,///<The board type
	};	
	/**
	 * @brief Get the point of the operation method class
	 * @param[in] OperationType The operation type
	 * @return The point of operation method class
	 * - !=nullptr The point of operation method class
	 * - nullptr Can't find the read method
	 */
	inline CBaseOperation* GetOperation(OPERATION_TYPE OperationType);
private:
	BYTE m_bySlotNo;///<The slot number the board inserted
	BYTE m_byControllerIndex;///<The controller index
	std::map<BYTE, CBaseOperation*> m_mapOperation;///<The map of operation methodThe key is operation type, and the value is the point of the operation class
};

