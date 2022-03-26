#pragma once
#include "Pin.h"
#include "Timeset.h"
#include "map"
#include "IACVInstance.h"
#include "IACVProject.h"
#include "IACVUnknown.h"
#include "IACVFailTag.h"
#include "CMDCode.h"
#include "PatternCMD.h"
using GETPROJECT = IACVInstance * (_stdcall*)();

struct LINE_DATA;
//The vector information of one pin.
typedef struct _Pin_Pattern_Info
{
	_Pin_Pattern_Info()
	{
		ucSerial = 1;
		memset(lpszPattern, 16, sizeof(lpszPattern));
	};
	unsigned char ucSerial;
	char          lpszPattern[16];//!< ÎÞ½áÎ²·ûºÅ
}PinPatternInfo, * pPinPatternInfo;

class CLinePattern
{
public:
	/**
	 * @brief Constructor
	 * @param usPinCount The pin count
	*/
	CLinePattern(USHORT usPinCount);
	/**
	 * @brief Destructor
	*/
	~CLinePattern();
	/**
	 * @brief Overide the opreator =
	 * @param[in] Source The source pattern
	 * @return The target pattern
	*/
	CLinePattern* operator=(const CLinePattern& Source);
	/**
	 * @brief Get the pin count
	 * @return The pin count
	*/
	USHORT GetPinCount();
	/**
	 * @brief Set the line pattern
	 * @param[in] pcPattern The pattern of each pin, the element count must not less than pin count
	 * @return Execute result
	 * - 0 The pattern is nullptr
	 * - -1 The point pointed to pattern is nullptr
	*/
	int SetPattern(const char* pcPattern);
	/**
	 * @brief Set the pattern
	 * @param[in] usPinIndex The pin index
	 * @param[in] cPattern The pattern index
	 * @return Execute result
	 * - 0 Set pattern successfully
	 * - -1 The pin index is over range
	*/
	int SetPattern(USHORT usPinIndex, char cPattern);
	/**
	 * @brief Get the line pattern
	 * @return Execute result
	*/
	const char* GetLinePattern() const;
	/**
	 * @brief Set the pattern validity
	 * @param[in] bValid Whether the pattern is valid
	*/
	void SetValid(BOOL bValid);
	/**
	 * @brief Whethher the pattern is validity
	 * @return Whethher the pattern is validity
	*/
	BOOL IsValid();
private:
	BOOL m_bValid;///<Whether the pattern is valid
	USHORT m_usPinCount;///<The pin count
	char* m_pucPattern;///<The pattern of each pin
};
/**
 * @class CVectorInfo
 * @brief The class of vector inforamtion
*/
class CVectorInfo
{
public:
	/**
	 * @brief Constructor
	*/
	CVectorInfo(CDriverAlarm* pAlarm);
	/**
	 * @brief Destructor
	*/
	~CVectorInfo();
	/**
	 * @brief Open vector file
	 * @param[in] lpszFileName The full path of the vector file
	 * @param[out] mapPin The pin information in vector file, the key is pin name and value is pin information
	 * @return Execute result
	 * - 0 Open file successfully
	 * - -1 The point of file name is nullptr
	 * - -2 The vector file is not existed
	 * - -3 Load vector editor module fail
	 * - -4 The vector format is wrong
	 * - -5 Can't get the label of the vector
	 * - -6 Allocate memory fail
	*/
	int OpenFile(const char* lpszFileName, std::map<std::string, CPin*>& mapPin);
	/**
	 * @brief Get the time set in vector file
	 * @param mapPinSeries The pin's time set series value
	 * @param mapTimeSet The time set information
	 * @return Execute result
	 * - 0 Get the time set successfully
	 * - -1 Not open vector file
	*/
	int GetTimeSet(std::map<std::string, CSeriesValue*>& mapPinSeries, std::map<std::string, CTimeSet*>& mapTimeSet);
	/**
	 * @brief Get the label
	 * @param[out] mapLabel The label information, key is name and value is its line number
	 * @return Execute result
	 * - 0 Get the label successfully
	 * - -1 The vector file can't be open
	*/
	int GetLabel(std::map<std::string, UINT>& mapLabel);
	/**
	* @brief Read the vector line from vector file.
	* @param[in] nStartLine The line number be read start from.
	* @param[I/O] nReadLineCount The number to be read and the count read successfully.
	* @param[out] bSaveDRAM Whether vector line read is saved in BRAM.
	* @param[out] bLastVectorCurMemory Whether the last line current read is the last line of current type of memory.
	* @return Execute result 
	* - 0 Read the vector line successfully
	* - -1 The vector is not the right vector file
	* - -2 Must open file before
	*/
	int ReadLine(UINT uStartLine, UINT& uReadLineCount);
	/**
	 * @brief Get the line read before
	 * @param[in] nLineOffset The line offset to the start line of latest read
	 * @param[out] pPinPattern The pattern information
	 * @param[out] byTimeset The timeset index
	 * @param[out] vecCMD The command information
	 * @param[out] lpszLabel The label name of current line
	 * @return Execute result
	 * - 0 Get the read line successfully
	 * - -1 Not open vector file
	 * - -2 The line offset is over range
	*/
	int GetReadLine(int nLine, const CLinePattern*& pLinePattern, const CPatternCMD* &PatternCMD);
	/**
	 * @brief Get vector line count
	 * @return The line count
	 * - >=0 The vector line count
	 * - -1 Not load vector file
	*/
	int GetLineCount();
	/**
	 * @brief Close vector file
	*/
	void CloseFile();
	/**
	 * @brief Get whether the vector editor is in debug mode
	 * @return
	 * - TRUE Debug mode
	 * - FALSE No debug mode
	*/
	BOOL IsDebugMode();
	/**
	 * @brief Get the vector file name
	 * @return The vector file name
	*/
	const char* GetVectorFileName();
	/**
	 * @brief Get the vector file ID
	 * @param[out] lpszID The ID buff
	 * @param[in] nIDLengh The buff size
	*/
	void GetID(char* lpszID, int nIDLengh);
	/**
	 * @brief Get the vector save mark
	 * @param[out] lpszSaveMark The save mark
	 * @param[in] nMarkLength The mark length
	*/
	void GetSaveMark(char* lpszSaveMark, int nMarkLength);
	/**
	 * @brief Reset vector information
	*/
	void Reset();
private:
	/**
	 * @brief Check whether the vector file is right
	 * @param[in] lpszFileName The vector file name
	 * @return Whether the file is right
	 * - 0 The file is right
	 * - -1 The point of the file is nullptr
	 * - -2 The file is not existed
	 * - -3 Can't load VectorEditor module
	 * - -4 The format of file is wrong
	 */
	int CheckFile(const char* lpszFileName);
	/**
	 * @brief Get the command string from code
	 * @param[in] usCMDCode The command code
	 * @return The command name
	 * - !=0 The command name
	 * - nullptr The command code is not supported
	*/
	int GetCommand(USHORT usCMDCode);
	/**
	 * @brief Get the parallel string from code
	 * @param[in] usParallelCMDCode The parallel command code
	 * @return The command name
	 * - !=0 The parallel command code
	 * - nullptr The command code is not supported
	*/
	int GetParallelCommand(USHORT usParallelCMDCode);
	/**
	 * @brief Read line
	 * @param[in] uLineNo The line number
	 * @param[out] usTimeSet The time set of current line
	 * @param[out] LinePattern The line pattern
	 * @param[out] vecCMDInfo The command information
	 * @return Execute result
	 * - 0 Read line successfully
	 * - -1 Read vector file error
	 * - -2 Allocate memory fail
	 * - -3 The line number is not existe
	 * - -4 The label used in command is not existed
	*/
	int ReadLine(UINT uLineNo, USHORT& usTimeSet, CLinePattern& LinePattern, std::vector<CMD_INFO>& vecCMDInfo);
	/*
	 * @brief Delte the data value
	 * @tparam Key The key type of map
	 * @tparam Value The value type of map
	 * @param[in] mapData The data whose memory will be deleted
	*/
	template <typename Key, typename Value>
	void Delete(std::map<Key, Value*>& mapData);
private:
	HMODULE m_hDll;///<The module of vector editor
	CDriverAlarm* m_pAlarm;///<The driver alarm
	std::string m_strFileName;///<The file name of vector file
	int m_nFileVersion;///<The file version of vector
	IACVInstance* m_pIns;///<The point of vector editor instance
	IACVProject* m_pVector;///<The point of project of vector editor
	IACVFailTag* m_pFailTag;///<The point of fail tag
	USHORT m_usPinCount;///<The pin count
	BOOL m_bFileReadError;///<Whether read file error
	char m_lpszID[256];///<The vector file ID
	char m_lpszSaveMark[256];///<The vector save mark
	std::map<USHORT, CPin*> m_mapPinInfo;///<The pin information
	int m_nLineCount;///<The vector line count
	CCMDCode m_CMDCode;///<The command code
	std::map<std::string, UINT> m_mapLabel;///<The label information, key is name and value is its line number
	std::map<UINT, CLineCMD> m_mapCMD;///<The command information of the line, key is line number and value is its command converted
	std::map<UINT, CLinePattern*> m_mapPattern;///<The pattern information of the line
	std::map<UINT, CPatternCMD> m_mapCMDData;///<The command data
};

template <typename Key, typename Value>
void CVectorInfo::Delete(std::map<Key, Value*>& mapData)
{
	for (auto& Data : mapData)
	{
		if (nullptr != Data.second)
		{
			delete Data.second;
			Data.second = nullptr;
		}
	}
	mapData.clear();
}

