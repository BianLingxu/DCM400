#pragma once
/**
 * @file MainFunction.h
 * @brief The head file of main function
 * @author Guangyun Wang
 * @date 2022/02/16
 * @copyright AccoTEST Business Unit of Beijing Huafeng Test & Control Technology Co., Ltd
*/
#include "DriverAlarm.h"
#include "Board.h"
#include "Pin.h"
#include "PinGroup.h"
#include "Timeset.h"
#include "Site.h"
#include "AccoTESTGlobal.h"
/**
 * @class CMainFunction
 * @brief The main function for the board
*/
class CMainFunction
{
public:
	/**
	 * @brief Constructor
	 * @param[in] pAlarm The point pointed to class CDriverAlarm
	*/
	CMainFunction(CDriverAlarm* pAlarm);
	/**
	 * @brief Copy constructor
	 * @param[in] pAlarm The point pointed to class CDriverAlarm
	*/
	void CopyBoard(const CMainFunction& Source);
	/**
	 * @brief Set the driver alarm
	 * @param[in] pAlarm The point pointed to class CDriverAlarm
	*/
	void SetDriverAlarm(CDriverAlarm* pAlarm);
	/**
	 * @brief Add the board
	 * @param[in] bySlotNo The slot number the board inserted
	 * @param[in] usChannelCount The channel count of board
	 * @return Execute result
	 * - 0 Add board successfully
	 * - -1 The board is not existed
	*/
	int AddBoard(BYTE bySlotNo, USHORT usChannelCount = 0);
	/**
	 * @brief Get the channel count of the board
	 * @param bySlotNo The slot number the board inserted
	 * @return The channel count
	 * - >=0 The channel count
	 * - -1 The board is not existed
	 * - -2 The channel count is not recorded
	*/
	int GetChannelCount(BYTE bySlotNo);
	/**
	 * @brief Reset the board added
	*/
	void Reset();
	/**
	 * @brief Get the board existed
	 * @param[in] vecBoard The board existed
	*/
	void GetBoardExisted(std::vector<BYTE>& vecBoard);
	/**
	 * @brief Load vector file
	 * @param[in] lpszVectorFile The full path of vector file
	 * @param[in] bReload Whether reload vector file
	 * @return Execute result
	 * - 0 Load vector file successfully
	 * - -1 The vector fil name is nullptr
	 * - -2 No board existed
	 * - -3 Can't open the file
	 * - -4 Can't find VectorEditor module
	 * - -5 The format of vector file is wrong
	 * - -6 Can't get the label of vector
	 * - -7 Allocate memory fail
	 * - -8 No valid pin in vector file
	 * - -9 The channel number in pin is over range
	 * - -10
	*/
	int LoadVectorFile(const char* lpszVectorFile, BOOL bReload);
	/**
	 * @brief Reset the vector information
	*/
	void ResetVector();
	/**
	 * @brief Load the timeset in vector file
	 * @param[in] vecSite The site information 
	 * @param[in] mapTimeset The timeset information
	 * @return Execute result
	 * - 0 Load timeset successfully
	 * - -1 No board existed
	 * - -2 Get the timeset edge fail
	 * - -3 Set tiemset edge fail
	*/
	int LoadVectorFileTimeset(const std::vector<USHORT>& vecSite, std::map<std::string, CSeriesValue*>& mapPinSeries);
	/**
	 * @brief Classify the channel to board
	 * @param[in] vecPin The pin name
	 * @param[in] vecSite The site number
	 * @param[out] mapBoardChannel The board channel information
	 * @return Execute result
	 * - 0 Get the board channel successfully
	 * - -1 No pin
	*/
	int GetBoardChannel(const std::vector<std::string>& vecPin, const std::vector<USHORT>& vecSite, std::map<BYTE, std::vector<USHORT>>& mapBoardChannel);
	/**
	 * @brief Initialize the site information from vector file
	 * @param[in] bAddAlarm Whether add alarm
	 * @return Execute result
	 * - 0 Initialize site successfully
	 * - -1 The channel number is over range
	*/
	inline int InitSite(BOOL bAddAlarm = TRUE);
private:
	/**
	 * @brief Clear the memory
	 * @tparam Key The key type of map
	 * @tparam Value The value of map
	 * @param[in] mapMemory The variable whose memory will be delete
	*/
	template <typename Key, typename Value>
	void DeleteMemory(std::map<Key, Value*>& mapMemory);
	/**
	 * @brief Whether clear all bind
	*/
	void ClearBind();
	/**
	 * @brief Get the file name of vector information
	 * @param[out] strVectorInfoFile The vector file name
	*/
	inline void GetVectorInfoFile(std::string& strVectorInfoFile);
	/**
	 * @brief Save the information to file
	 * @param[in] lpszVectorFile The vector file name
	*/
	void SaveVectorInformation(const std::set<std::string>& setFailSynPin);
	/**
	 * @brief Load the vector information
	 * @param[in] strInfoFile The information file
	 * @return Execute result
	*/
	int LoadVectorInfo(const std::string& strInfoFile);
	/**
	 * @brief Set the fail synchronous
	 * @param[in] bFailSyn Whether set fail synchronous
	 * - TRUE The fail synchronized according to the channel distribution
	 * - FALSE Fail signal not synchronized
	*/
	void SetFailSyn(const std::set<std::string>& setFailSynPin);
	/**
	 * @brief Get the pin group information file
	 * @param[out] strFile The file with whole path
	*/
	inline void GetPinGroupInfoFile(std::string& strFile);
	/**
	 * @brief Check the vector validity
	 * @param[in] lpszVectorFile The vector file
	 * @return The validity of the vector
	 * - TRUE The vector is valid
	 * - FALSE The vector is invalid
	*/
	BOOL IsVectorValid(const char* lpszVectorFile);
private:
	CDriverAlarm* m_pAlarm;///<The point pointed to calss CDriverAlarm
	std::string m_strVectorFile;///<The vector file name
	BOOL m_bLoadVector;///<Whether load vector file
	std::map<BYTE, CBoard*> m_mapBoard;///<The board existed,key is slot number and value is point pointed to class CBoard
	std::map<std::string, UINT> m_mapLabel;///<The label information, key is string name and value is its line number
	std::map<std::string, CPin*> m_mapPin;///<The pin information, key is pin name and value is its point pointed to class
	std::map<std::string, CPinGroup*> m_mapPinGroup;///<The pin group information, key is pin name and value is its point pointed to class.
	CSite m_Site;///<The site information
	std::map<std::string, CTimeSet*> m_mapTimeSet;///<The timeset information, key is timeset name and value is its timeset index
	CClassifyBoard m_ClassifyBoard;///<The channel in the board of latest query, like query pin group or pin name
	BOOL m_bVectorBind;///<Whether pin channel distribution can binded
	int m_nVectorLineCout;///<The vector line count
	STS_PROGRESS_INFO_FUN m_pProgressInfo;///<The point of testui's progress information
	STS_PROGRESS_FUN m_pProgressStep;///<The point of testui's progress step information
};

template <typename Key, typename Value>
void CMainFunction::DeleteMemory(std::map<Key, Value*>& mapMemory)
{
	for (auto& Pair : mapMemory)
	{
		if (nullptr != Pair.second)
		{
			delete Pair.second;
			Pair.second = nullptr;
		}
	}
	mapMemory.clear();
}
