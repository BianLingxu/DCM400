#pragma once
/**
 * @file Relay.h
 * @brief Include the class using for relay operation
 * @author Guangyun Wang
 * @date 2020/05/28
 * @copyright Copyright (c) AccoTEST business unit of Beijing Huafeng Test & Control Technology Co,. Ltd.
*/
#include "Operation.h"
#include "DCM400HardwareInfo.h"
#include <vector>
#include <set>
using namespace std;

/**
 * @class CRelayRegister
 * @brief This class is singleton class for saving the register value of relay
*/
class CRelayRigister
{
public:
	/**
	 * @brief Get the instance of class
	 * @return The point of the instance of CRelayRegister
	*/
	static CRelayRigister* Instance();
	/**
	 * @brief Destructor
	*/
	~CRelayRigister();
	/**
	 * @brief Set the relay memory
	 * @param[in] bySlotNo The slot number
	 * @param[in] pulFuncRelay The function relay
	 * @return Execute result
	 * - 0 Set memory of function relay register successfully
	 * - -1 The board not set
	*/
	int SetRelayMem(BYTE bySlotNo, ULONG* pulFuncRelay);
	/**
	 * @brief Get the relay register memory
	 * @param[in] bySlotNo The slot number
	 * @param[out] pulFuncRelay The function relay memory
	 * @return Execute result
	 * - 0 Get the relay register successfully
	 * - -1 The board not set
	*/
	int GetRelayREG(BYTE bySlotNo, ULONG*& pulFuncRelay);
private:
	/**
	 * @brief Constructor
	*/
	CRelayRigister();
private:
	std::map<BYTE, ULONG*> m_mapRelayREG;///<The relay register value of each board, key is board slot number and value is register value
};

/**
 * @class CRelaySafety
 * @brief The class to ensure the relay safety
*/
class CRelaySafety
{
public:
	/**
	 * @brief Get the instance of the class
	 * @return The point of the class
	*/
	static CRelaySafety* Instance();
	/**
	 * @brief Apply relay operation
	 * @param[in] bySlotNo The slot number of board whose relay will be operated
	*/
	void Apply(BYTE bySlotNo);
	/**
	 * @brief Release the relay authority
	 * @param[in] bySlotNo The slot number
	*/
	void Release(BYTE bySlotNo);
	/**
	 * @brief Destructor
	*/
	~CRelaySafety();
private:
	/**
	 * @brief Constructor
	*/
	CRelaySafety();
	/**
	 * @brief Get the critical
	 * @param[in] bySlotNo The slot number
	 * @return The point of critial section for the slot
	*/
	CRITICAL_SECTION* GetCritical(BYTE bySlotNo);
private:
	CRITICAL_SECTION m_criDistribution;///<The critical section to ensure the slot critical application thread safety
	std::map<BYTE, CRITICAL_SECTION*> m_mapSlotSafety;///<The critic section to ensure the relay operation thread safety, key is slot number and value is its critical section
};
/**
 * @class CRelay
 * @brief The operation of relay, include function relay,DC relay and calibration relay
*/
class CRelay
{
public:
	/**
	 * @brief Constructor
	 * @param[in] The operation class
	*/
	CRelay(COperation& Operation);
	/**
	 * @brief Set the function relay
	 * @param[in] vecChannel The channel number
	 * @param[in] bConnect Whether connect relay
	 * @return Execute result
	 * - 0 Set function relay successfully
	 * - -1 The channel is over range
	*/
	int FunctionRelay(const std::vector<USHORT>& vecChannel, BOOL bConnect);
	/**
	 * @brief Set the high voltage relay
	 * @param[in] usChannel The channel number
	 * @return Execute result
	 * - 0 Set calibration relay successfully
	 * - -1 The channel is over range
	 * - -2 The channel is not the high voltage channel
	*/
	int SetHighVoltageRelay(const std::vector<USHORT>& usChannel, BOOL bConnect);
	/**
	 * @brief Get the connected channel
	 * @param[in] RelayType The relay type
	 * @param[out] vecConnectedChannel The connected channel
	 * @return Execute result
	 * - 0 Get the connected channel successfully
	 * - -1 The relay type is error
	*/
	int GetConnectedChannel(RELAY_TYPE RelayType, std::vector<USHORT>& vecConnectedChannel);
	/**
	 * @brief Get the high voltage channel
	 * @param[out] setChannel The high voltage channel
	*/
	void GetHighVoltageChannel(std::set<USHORT>& setChannel);
private:
	COperation* m_pOperation;///<The point pointed to operation class
};
