/**
 * @file FPGAE
 * @brief The class be used to operate DCM board SM8213
 * @date 2020/02/20
 * @author Guangyun Wang
 * @copyright AccoTEST Business Unit of Beijing Huafeng Test & Control Technology Co., Ltd.
*/
#pragma once
#include "BaseOperation.h"
/**
 * @class CFPGAE
 * @brief The operation of FPAGE
*/
class CFPGAE :
	public CBaseOperation
{
public:
	/**
	 * @brief Constructor
	 * @param[in] bySlotNo The slot number
	*/
	CFPGAE(BYTE bySlotNo);
	/**
	 * @brief set the controller board
	 * @param[in] byControllerIndex The controller index, the parameter will be ignored
	 * @return Execute result
	 * - 0 Set controller index successfully
	*/
	int SetControllerIndex(BYTE byControllerIndex);
	/**
	 * @brief read data from board
	 * @param[in] usAddress The start address of data
	 * @param[in] uReadDataCount The data count will be read
	 * @param[out] pulData The memory address for saving the data read
	 * @return Execute result
	 * - 0 Read data successfully
	 * - -2 The data count 0 or the memory address is nullptr
	*/
	int Read(USHORT usAddress, UINT uReadDataCount, ULONG* pulData);
	/**
	 * @brief Write data to board
	 * @param[in] usAddress The start address
	 * @param[in] uWriteDataCount The data count will be written
	 * @param[in] pulData The memory address of the data
	 * @return Execute result
	 * - 0 Write data successfully
	 * - -2 The data count is 0 or the memory address is nullptr
	*/
	int Write(USHORT usAddress, UINT uWriteDataCount, const ULONG* pulData);
	
	ULONG GetStatus();
	int SetRegisterType(USHORT usType);
	int SetAddress(int nType, USHORT usAddress);
	int SetStartLineNo(int nType, ULONG ulStartLineNo);
	ULONG Read(USHORT usAddress);
	int Write(USHORT usAddress, ULONG ulData);
	int Read(UINT uReadDataCount, ULONG* pulDataBuff);
	int Write(UINT uWriteDataCount, const ULONG* pulData);
	int Read(BYTE byAddress, std::map<USHORT, ULONG>& mapChannelData);
	int Write(BYTE byAddress, std::map<USHORT, ULONG>& mapChannelData);
};

