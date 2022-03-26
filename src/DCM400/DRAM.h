#pragma once
/**
 * @file DRAM.h
 * @brief Include the class of CDRAM
 * @detail The class can realize read and write operation of DRAM
 * @author Guangyun Wang
 * @date 2021/12/23
 * @copyright AccoTEST Business Unit of Beijing Huafeng Test & Control Technology Co., Ltd.
*/
#include "BaseOperation.h"
/**
 * @class CDRAM
 * @brief The DRAM operation
*/ 
class CDRAM :
	public CBaseOperation
{
public:
	/**
	 * @enum MEM_TYPE
	 * @brief The memory type
	*/
	enum MEM_TYPE
	{
		CMD_DRAM0 = 0,
		CMD_DRAM1,
		PAT_DRAM0,
		PAT_DRAM1,
	};

	/**
	 * @brief Constructor
	 * @param[in] bySlotNo The slot number
	*/
	CDRAM(BYTE bySlotNo);
	/**
	 * @brief Set the controller index of the register
	 * @param[in] byControllerIndex The controller index
	 * @return Execute result
	 * - 0 Set the controller index successfully
	 * - -1 The Controller index is over range
	*/
	int SetControllerIndex(BYTE byControllerIndex);
	/**
	 * @brief Set the start line number
	 * @param[in] nType The memory type
	 * @param[in] ulStartByteNo The start line number(BYTE address), must be 64 byte align
	 * @return Execute result
	 * - 0 Set the start line number successfully
	 * - -1 The memory type is not supported
	 * - -2 The start line number is over range
	*/
	int SetStartLineNo(int nType, ULONG ulStartByteNo);
	/**
	 * @brief Write data to DRAM
	 * @param[in] uWriteDataCount The data count will be written
	 * @param[in] pulData The memory address of the data will be written
	 * @return Execute result
	 * - 0 Write data successfully
	 * - -1 The data count is 0 or the memory address of the data is nullptr
	 * - -2 The data count written is not integral multiple of minimum operation size
	*/
	int Write(UINT uWriteDataCount, const ULONG* pulData);
	/**
	 * @brief Read data from DRAM
	 * @param[in] uReadDataCount The read data count
	 * @param[out] pulDataBuff The memory address for saving the data read
	 * @return Execute result
	 * - 0 Read data successfully
	 * - -1 The data count is 0 or the point of memory address is nullptr
	 * - -2 The data count read is not integral multiple of minimum operation size
	 * - -3 Timeout
	*/
	int Read(UINT uReadDataCount, ULONG* pulDataBuff);

private:
	/**
	 * @brief Wait operation ready
	 * @return 
	*/
	int WaitReady();
	/**
	 * @brief Get the DRAM start address
	 * @return The DRAM start address
	*/
	ULONG GetStartAddr();

private:
	BYTE m_byControllerIndex;///<The controller index
	ULONG m_ulDramAddr;///<The DRAM address
	BOOL m_bRead;///<Whether the operation is read
	BYTE m_byFPGA;///<The FPGA will be operation
	BYTE m_bySelAdd;///<The chip select address
};

