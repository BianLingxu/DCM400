/**
 * @file DRAM.h
 * @brief Inlcude the class of CDRAM
 * @detail The class can realize read and write operation of DRAM
 * @author Guangyun Wang
 * @date 2020/05/31
 * @copyright AccoTEST Business Unit of Beijing Huafeng Test & Control Technology Co., Ltd.
*/
#pragma once
#include "COM.h"
/**
 * @class CDRAM
 * @brief The DRAM operation
*/
class CDRAM :
	public CCOM
{
public:
	/**
	 * @enum MEM_TYPE
	 * @brief The memory type
	*/
	enum class MEM_TYPE
	{
		DRAM1 = 0x01,///<The memory of DRAM1
		DRAM2 = 0x02,///<The memory of DRAM2
		DRAM3 = 0x04,///<The memory of DRAM3
		DRAM4 = 0x08,///<The memory of DRAM4
		DRAM5 = 0x10,///<The memory of DRAM5
		DRAM6 = 0x20,///<The memory of DRAM6
	};
	/**
	 * @brief Constructor
	 * @param[in] bySlotNo The slot number
	*/
	CDRAM(BYTE bySlotNo);
	/**
	 * @brief Set the start line number
	 * @param[in] nType The memory type
	 * @param[in] ulStartLineNo The start line number
	 * @return Execute result
	 * - 0 Set the start line number successfully
	 * - -1 The memory type is not supported
	 * - -2 The start line number is over range
	*/
	int SetStartLineNo(int nType, ULONG ulStartLineNo);
	/**
	 * @brief Read data from DRAM
	 * @param[in] uReadDataCount The read data count
	 * @param[out] pulDataBuff The memory address for saving the data read
	 * @return Execute result
	 * - 0 Read data successfully
	 * - -2 The data count is 0 or the point of memory address is nullptr
	 * - -3 The line count is over range
	*/
	int Read(UINT uReadDataCount, ULONG* pulDataBuff);
	/**
	 * @brief Write data to DRAM
	 * @param[in] uWriteDataCount The data count will be written
	 * @param[in] pulData The memory address of the data will be written
	 * @return Execute result
	 * - 0 Write data successfully
	 * - -2 The data count is over range
	 * - -3 The data count is 0 or the memory address of the data is nullptr
	*/
	int Write(UINT uWriteDataCount, const ULONG* pulData);

	ULONG GetStatus();
	int SetRegisterType(USHORT usType);
	int SetAddress(int nType, USHORT usAddress);
	ULONG Read(USHORT usAddress);
	int Write(USHORT usAddress, ULONG ulData);
	int Read(USHORT usAddress, UINT uReadDataCount, ULONG* pulDataBuff);
	int Write(USHORT usAddress, UINT uWriteDataCount, const ULONG* pulData);
	int Read(BYTE byAddress, std::map<USHORT, ULONG>& mapChannelData);
	int Write(BYTE byAddress, std::map<USHORT, ULONG>& mapChannelData);
	int SetControllerIndex(BYTE byControllerIndex);
private:
	ULONG m_ulStartAddress;///<The start address which will be read or written
	ULONG m_ulStartLineNo;///<The start line number
	BYTE m_bySlot;///<The board number belonged
	BYTE m_byControllerIndex;///<The controller index
};

