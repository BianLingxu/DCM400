#pragma once
/**
 * @file ATE305.h
 * @brief Include the operation of ATE305
 * @author Guangyun Wang
 * @date 2020/07/10
 * @copyright AccoTEST Business Unit of Beijing Huafeng Test & Control Technology Co., Ltd.
*/
#include "Register.h"
#include <vector>
#include <map>
/**
 * @class CATE305
 * @brief The operation of ATE305
*/
class CATE318 :
	public CRegister
{
public:
	/**
	 * @brief Constructor
	 * @param[in] bySlotNo The slot number
	*/
	CATE318(BYTE bySlotNo);
	/**
	 * @brief Get the status of ATE305
	 * @return The status
	*/
	ULONG GetStatus();
	/**
	 * @brief Read data of ATE305
	 * @param[in] byAddress The address
	 * @param[out] mapChannelData The data of each channel which will be read
	 * @return Execute result
	 * - 0 Read data successfully
	 * - -2 No data will be read
	*/
	int Read(BYTE byAddress, std::map<USHORT, ULONG>& mapChannelData);
	/**
	 * @brief Write data to ATE305
	 * @param[in] byAddress The address
	 * @param[in] mapChannelData The data of each channel which will be written
	 * @return Execute result
	 * - 0 Write data successfully
	 * - -2 No data will be written
	*/
	int Write(BYTE byAddress, std::map<USHORT, ULONG>& mapChannelData);
	int SetAddress(ULONG ulAddress);
	int SetAddress(int nType, USHORT usAddress);
	int SetStartLineNo(int nType, ULONG ulStartLineNo);
	ULONG Read(int nAddress);
	int Write(int nAddress, ULONG ulData);
	int Read(UINT uReadDataCount, ULONG* pulDataBuff);
	int Write(UINT uWriteDataCount, const ULONG* pulData);
	int Read(USHORT usAddress, UINT uReadDataCount, ULONG* pulDataBuff);
	int Write(USHORT usAddress, UINT uWriteDataCount, const ULONG* pulData);	   

private:
	/**
	 * @brief Get the control data
	 * @param[in] byAddress The address
	 * @param[in] uEnableChannel The channels be enabled
	 * @param[in] bRead Whether the control data is belong to read operation
	 * @param[in] bSame Whether the data is all same
	 * @return The control data
	*/
	inline ULONG GetCTRLData(BYTE byAddress, USHORT uEnableChannel, BOOL bRead, BOOL bSame);
	/**
	 * @brief Read the channel data
	 * @param[in] byAddress The address
	 * @param[in] byChannel The channel number
	 * @return The channel data
	*/
	ULONG ReadData(BYTE byAddress, BYTE byChannel);
	/**
	 * @brief Write channel data
	 * @param[in] byAddress The address
	 * @param[in] byChannel The channel number
	 * @param[in] uData The data will be written
	*/
	void WriteData(BYTE byAddress, BYTE byChannel, USHORT uData);
};

