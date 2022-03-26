/**
 * @file COM.h
 * @brief Include the class of CCOM
 * @detail The com read and write function
 * @author Guangyun Wang
 * @date 2020/05/31
 * @copyright AccoTEST Business Unit of Beijing Huafeng Test & Control Technology Co., Ltd.
*/

#pragma once
#include "FPGAA.h"
/**
 * @class CCOM
 * @brief The COM operation class
*/
class CCOM :
	public CFPGAA
{
public:
	/**
	 * @brief Constructor
	 * @param[in] bySlotNo The slot number
	*/
	CCOM(BYTE bySlotNo);
protected:
	void SetSelectAddress(USHORT usSelectAddress, USHORT usSelectAddressWrite);
	void SetDataAddress(USHORT usReadAddress, USHORT usWriteAddress);
	void SetRequestAddress(USHORT usRequestAddress);
	void SetStatusAddress(USHORT usStatusAddress, USHORT usQueryTime);
	void SetStartAddress(ULONG ulAddress);
	int Read(ULONG ulReadDataCount, ULONG* pulData);
	int Write(ULONG ulWriteCount, const ULONG* pulData);
private:
	/**
	 * @brief Wait RAM ready
	 * @param[in] ulStatusBit The status bit of the register value
	 * @return Execute result
	 * - 0 RAM ready
	 * - -1 Not ready and timeout
	*/
	inline int WaitReady(ULONG ulStatusBit = 0x01);
private:
	USHORT m_usSelectAddress[2];///<The register address for setting the start address
	USHORT m_uDataAddress[2];///<The register address for data read or written, first element is for read the second is for write
	USHORT m_usRequestAddress;///<The register address for start read or write
	USHORT m_usStatusAddress;///<The address for check RAM status
	USHORT m_usQueryTime;///<The query times of check RAM ready
	ULONG m_ulStartAddresss;///<The start address
};

