/**
 * @file BRAM.h
 * @brief The class of BRAM
 * @detail The class be used to read and write the memory data in the FPGA of DCM board SE8212
 * @author Guangyun Wang
 * @date 2020/05/31
 * @copyright AccoTEST Business Unit of Beijing Huafeng Test & Control Technology Co., Ltd.
*/
#pragma once
#include "COM.h"
/**
 * @class CBRAM
 * @brief The operation of BRAM
*/
class CBRAM :
	public CCOM
{
public:
	/**
	 * @enum MEM_TYPE
	 * @brief The memory type
	*/
	enum MEM_TYPE
	{
		IMM1 = 0x01,///<The IMM1
		IMM2 = 0x02,///<The IMM2
		IMM3 = 0x04,///<The IMM3
		FM = 0x08,///<The pattern code memory FM
		MM = 0x10,//<The pattern code memory of MM
		IOM = 0x20,//<The pattern code memory of IOM
		PMU_BUF = 0x40,///<The PMU memory
		MEM_PERIOD = 0x80,///<The period memory
		MEM_RSU = 0x100,///<The result memory
		MEM_HIS = 0x200,///<The history memory
		MEM_TIMING = 0x400,///<The timing memory
	};
	/**
	 * @brief Constructor
	 * @param[in] bySlotNo The slot number
	*/
	CBRAM(BYTE bySlotNo);
	/**
	 * @brief Set the memory type and the address
	 * @param[in] nMemType The memory type
	 * @param[in] usLow16Data The low 16 bits data
	 * @return Execute result
	 * - 0 Set the address successfully
	 * - -1 The memory type is not supported
	*/
	int SetAddress(int nMemType, USHORT usLow16Data);
	/**
	 * @brief Write data to BRAM
	 * @param[in] uWriteDataCount The data count be written
	 * @param[in] pulData The memory address
	 * @return Execute result
	 * - 0 Write data successfully
	 * - -2 The data count is 0 or the memory address is nullptr
	*/
	int Write(UINT uWriteDataCount, const ULONG* pulData);
	/**
	 * @brief Read data from BRAM
	 * @param[in] uReadDataCount The data count read
	 * @param[out] pulData The memory address for saving data
	 * @return Execute result
	 * - 0 Read data successfully
	 * - -1 The data count is 0 or the memory address is nullptr
	*/
	int Read(UINT uReadDataCount, ULONG* pulData);

	ULONG GetStatus();
	int SetRegisterType(USHORT usType);
	int SetStartLineNo(int nType, ULONG ulStartLineNo);
	ULONG Read(USHORT usAddress);
	int Write(USHORT usAddress, ULONG ulData);
	int Read(USHORT usAddress, UINT uReadDataCount, ULONG* pulDataBuff);
	int Write(USHORT usAddress, UINT uWriteDataCount, const ULONG* pulData);
	int Read(BYTE byAddress, std::map<USHORT, ULONG>& mapChannelData);
	int Write(BYTE byAddress, std::map<USHORT, ULONG>& mapChannelData);
private:
	ULONG m_ulStartAddress;
};

