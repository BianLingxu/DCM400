#pragma once
/**
 * @file PTE.h
 * @brief The head file for class CPTE using for PTE calculation
 * @author Guangyun Wang
 * @date 2021/08/31
 * @copyright AccoTEST Business Unit of Beijing Huafeng Test & Control Co., Ltd.
*/
#include <Windows.h>
#include "HardwareInfo.h"
#include <vector>
#include <map>
#include <string>
/**
 * @class CPinInfo
 * @brief The pin information
*/
class CPinInfo
{
public:
	/**
	 * @brief Constructor
	 * @param[in] lpszPinName The pin name
	*/
	CPinInfo(const char* lpszPinName);
	/**
	 * @brief The name of the pin
	 * @return The name of the pin
	*/
	const char* Name();
	/**
	 * @brief Add channel to current pin
	 * @param[in] bySlotNo The slot number
	 * @param[in] usChannel The channel number
	*/
	void AddChannel(BYTE bySlotNo, USHORT usChannel);
	/**
	 * @brief Get the channel belongs to current pin
	 * @param[in] vecChannel The channel information
	*/
	void GetChannel(std::vector<CHANNEL_INFO>& vecChannel);
	/**
	 * @brief Get the site number of the channel
	 * @param[in] Channel The channel information
	 * @return The site number
	 * - >=0 The site number
	 * - -1 The channel is not existed in current site
	*/
	int GetSiteNo(const CHANNEL_INFO& Channel);
private:
	std::string m_strPinName;///<The pin name of current pin
	std::vector<CHANNEL_INFO> m_vecChannel;///<The channel information of each site
};
/**
 * @class CPTE
 * @brief The pure virtual class for PTE calculation
*/
class CPTE
{
public:
	/**
	 * @brief Constructor
	*/
	CPTE();
	/**
	 * @brief Destructor
	*/
	~CPTE();
	/**
	 * @brief Set the vector file used
	 * @param lpszVectorFile The vector file path
	 * @return Executer result
	 * - 0 Set vector file successfully
	 * - -1 The point of the vector file is nullptr
	 * - -2 The file is not existed
	 * - -3 Load vector file fail
	*/
	int SetVectorFile(const char* lpszVectorFile);
	/**
	 * @brief The function whose PTE will be calculated
	*/
	virtual void FuncExecute() = 0;
	/**
	 * @brief Reset the status afer function execution
	*/
	virtual void ResetStatus() = 0;
	/**
	 * @brief The operation when channel distribution had been redistributed
	 * @return Execute result
	 * - 0 Continue 
	 * - -1 Abort
	*/
	virtual int ChannelRedistribution();
	/**
	 * @brief Distribute channel in specific role
	 * @param[in] byController The controller count used
	 * @param[in] bParallel Whether the channel distribution is in parallel
	 * @param[in] usChannelCountPerSite The channel count per site
	 * @param[in] usSiteCount The site count
	 * @return Execute result
	 * - 0 Distribute channel successfully
	 * - -1 The controller count is over the controller existed
	 * - -2 The site count should be not less than 1
	 * - -2 The controller count not enough for so much site
	 * - -3 The controller count is not enough for parallel
	*/
	virtual int ChannelDistribute(BYTE byController, BOOL bParallel, USHORT usChannelCountPerSite, USHORT usSiteCount);
	/**
	 * @brief Get the PTE value
	 * @param[in] byControllerCount The controller count used
	 * @param[in] bParallel Whether the channel distribution is in parallel
	 * @param[in] usChannelCountPerSite The channel count per site
	 * @param[in] usSiteCount The site count of the PTE
	 * @param[in] nCycleTimes The cycle times for function execution
	 * @return The PTE value in percentage
	*/
	double GetPTE(BYTE byControllerCount, BOOL bParallel, USHORT usChannelCountPerSite, USHORT usSiteCount, int nCycleTimes = 1);
private:
	/**
	 * @brief Reset
	*/
	void Reset();
protected:
	std::vector<BYTE> m_vecBoard;///<The board existed
	std::string m_strAllPin;///<The pin group of all pin
	USHORT m_usSiteCount;///<The site count
	std::map<std::string, CPinInfo*> m_mapPinInfo;///<The pin information
};
