#pragma once
#include "DCM400HardwareInfo.h"
#include <set>
#include <vector>
#include <map>
/**
 * @class CControllerChannel
 * @brief The channel number belong to controller
*/
class CControllerChannel
{
public:
	/**
	 * @brief Constructor
	 * @param[in] byIndex The controller index
	 */
	CControllerChannel(BYTE byIndex);
	/**
	 * @brief Destructor
	 */
	~CControllerChannel();
	/**
	 * @brief Add channel to controller
	 * @param[in] usChannel The channel number
	 * @return Execute result
	 * - 0 Add channel successfully
	 * - -1 The channel is over range
	 */
	int AddChannel(USHORT usChannel);
	/**
	 * @brief Get the channel in the controller
	 * @param[out] vecChannel The channel number
	 */
	void GetChannel(std::vector<USHORT>& vecChannel);
	/**
	 * @brief Get the channel count in the controller
	 * @return The channel count
	 */
	int GetChannelCount();
	/**
	 * @brief Reset and clear channel added
	 */
	void Reset();
private:
	BYTE m_byIndex;///<The controller index
	std::set<USHORT> m_setChannel;///<The channel in the controller
};

/**
 * @class CBoardChannelClassify
 * @brief Classify board channel to controller channel
*/
class CBoardChannelClassify
{
public:
	/**
	 * @brief Constructor
	*/
	CBoardChannelClassify();
	/**
	 * @brief Destructor
	*/
	~CBoardChannelClassify();
	/**
	 * @brief Set the channel needed to be classified
	 * @param[in] vecChannel The channel number
	 * @return Execute result
	 * - 0 Classify channel successfully
	 * - -1 Some channels are over range
	*/
	int SetChannel(const std::vector<USHORT>& vecChannel);
	/**
	 * @brief Get the channel of specific controller
	 * @param[in] byControllerIndex The controller index
	 * @param[out] vecChannel The channel in controller
	 * @return Execute result
	 * - 0 Get channels successfully
	 * - -1 The controller index is over range
	*/
	int GetChannel(BYTE byControllerIndex, std::vector<USHORT>& vecChannel);
	/**
	 * @brief Get the controller used
	 * @param[in] vecChannel The channel number
	 * @param[out] setController The controller
	*/
	void GetController(const std::vector<USHORT>&vecChannel, std::set<BYTE>& setController);
private:
	std::map<BYTE, CControllerChannel*> m_mapControllerChannel;///<The channel of each controller, using for classifying channels
};


/**
 * @class CBoardChannel
 * @brief The board channel
*/
class CBoardChannel
{
public:
	/**
	 * @brief Constructor
	 * @param[in] bySlotNo The slot number of the board
	*/
	CBoardChannel(BYTE bySlotNo);
	/**
	 * @brief Destructor
	*/
	~CBoardChannel();
	/**
	 * @brief Get the channel of the board
	 * @param[out] vecChannel The channel of the board
	 * @param[out] pvecChannelID The point of vector which save the channel ID
	*/
	void GetChannel(std::vector<USHORT>& vecChannel, std::vector<USHORT>* pvecChannelID = nullptr);
	/**
	 * @brief Add channel to the board
	 * @param[in] usChannel The channel number
	 * @param[in] usID The channel ID
	 * @return Execute result
	 * - 0 Add channel successfully
	 * - -1 The channel has been added before
	*/
	int AddChannel(USHORT usChannel, USHORT usID);
	/**
	 * @brief Reset the board channel
	*/
	void Reset();
	/**
	 * @brief Get channel count of current board
	 * @return The channel count of current board
	*/
	USHORT GetChannelCount();
private:
	BYTE m_bySlotNo;///<The slot number of the board
	std::map<USHORT, USHORT> m_mapChannel;///<The channel of current board
};

/**
 * @CClassifyBoard
 * @brief Classify channel to board channel
*/
class CClassifyBoard
{
public:
	/**
	 * @brief Destructor
	*/
	~CClassifyBoard();
	/**
	 * @brief Set the channel number with its slot number
	 * @param[in] vecChannel The channel number
	*/
	void SetChannel(const std::vector<CHANNEL_INFO>& vecChannel);
	/**
	 * @brief Get the board used
	 * @param[out] setBoard The board slot number
	*/
	void GetBoard(std::set<BYTE>& setBoard) const;
	/**
	 * @brief Get the channel of the board
	 * @param[in] bySlotNo The slot number
	 * @param[out] vecChannel The channel of the board
	 * @param[out] pvecChannelID The point of vector which save the channel ID
	*/
	void GetBoardChannel(BYTE bySlotNo, std::vector<USHORT>& vecChannel, std::vector<USHORT>* pvecChannelID = nullptr) const;
	/**
	 * @brief Get the controller channel
	 * @param[in] usChannel The channel number in board
	 * @param[out] byController The controller index
	 * @return The channel in controller
	 * - >=0 The channel in controller
	 * - -1 The channel is over range
	*/
	int GetControllerChannel(USHORT usChannel, BYTE& byController) const;
private:
	std::map<BYTE, CBoardChannel*> m_mapBoardChannel;///<The channel of each board
};

/**
 * @brief The channel Group
*/
class CChannelGroup
{
public:
	/**
	 * @brief Constructor
	*/
	CChannelGroup();
	/**
	 * @brief Add channel to group
	 * @param usChannel The channel number of board
	 * @return Execute result
	 * - 0 Add channel successfully
	 * - -1 The channel is over range
	*/
	int AddChannel(USHORT usChannel);
	/**
	 * @brief Get channel in group
	 * @param[out] vecChannel The channel in group
	*/
	void GetChannel(std::vector<USHORT>& vecChannel) const;

private:
	std::set<USHORT> m_setChannel;///<The channel in group
};