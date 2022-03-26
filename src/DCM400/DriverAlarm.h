#pragma once
/**
 * @file DriverAlarm.h
 * @brief Include the class of CDriverAlarm
 * @detail The class is using for output the alarm information
 * @author Guangyun Wang
 * @date 2020/05/31
 * @copyright AccoTEST Business Unit of Beijing Huafeng Test & Control Technology Co., Ltd.
*/
#include "pch.h"
#include "ShieldAlaram.h"
#include <string>
#include "Alarm.h"
#include "AlarmID.h"
class DCM400Data;
/**
 * @class CDriverAlarm
 * @brief The driver alarm output
*/
class CDriverAlarm
{
public:
	/**
	 * @enum DATA_FORMAT
	 * @brief The data type
	*/
	enum class DATA_FORMAT
	{
		INT_FORMAT = 0,///<Integer value
		DOUBLE_ONE_DECIMAL,///<Double value with one decimal
		DOUBLE_TWO_DECIMAL,///<Double value with two decimals
		DOUBLE_THREE_DECIMAL,///<Double value with three decimals
		DOUBLE_FOUR_DECIMAL,///<Double value with four decimals
		DOUBLE_FIVE_DECIMAL,///<Double value with five decimals
		DOUBLE_SIX_DECIMAL,///<Double value with six decimals
	};
	/**
	 * @enum ALARM_LEVEL
	 * @brief The alarm level
	*/
	enum class ALARM_LEVEL
	{
		UnknownLevel = 0,///<Unknow level
		AlarmNormal,///<Normal alarm
		AlarmWarning,///<Warning alarm
		AlarmError,///<Error alarm
		AlarmFatal///<Fatal alarm
	};
	/**
	 * @enum ALARM_TYPE
	 * @brief The alarm type
	*/
	enum class ALARM_TYPE
	{
		CURRENT_CLAMP = 0,///<Current clamp
		VOLTAGE_CLAMP = 1,///<Voltage clamp
		PARAMETER_OVERRANGE = 4,///<The parameter is over range
		PARAMETER_OCCURALARM = 5, ///<The parameter occur alarm
		PARAMETER_NOT_DEFINED = 6,///<The parameter is not defined
		PARAMETER_SITE_INVALID = 7,///<The site is invalid
	};
	/**
	 * @brief Get the instance CDriverAlarm
	 * @return The point of instance
	*/
	static CDriverAlarm* Instance();
	/*
	 * @brief Destructor
	*/
	~CDriverAlarm();
	/**
	 * @brief Set the alarm ID
	 * @param[in] ID The alarm ID
	*/
	inline void SetAlarmID(ALARM_ID ID);
	/**
	 * @brief Get the pin name
	 * @param[out] strPin Pin string
	 * @param[out] usSiteNo Site number
	 * @return Pin type
	 * - TRUE Pin name
	 * - FALSE Pin group
	*/
	BOOL GetPin(std::string& strPin, USHORT& usSiteNo);
	/**
	 * @brief Set alarm type
	 * @param[in] AlarmLevel The alarm level
	*/
	void SetAlarmLevel(ALARM_LEVEL AlarmLevel);
	/**
	 * @brief Set the alarm type
	 * @param[in] AlaramType The alarm type
	*/
	void SetAlarmType(ALARM_TYPE AlaramType = ALARM_TYPE::PARAMETER_OCCURALARM);
	/**
	 * @brief Set the function name of driver pack
	 * @param[in] lpszName The function name
	 * @return Execute result
	 * - 0 Set driver pack name successfully
	 * - -1 The point of name is nullptr
	*/
	int SetDriverPackName(const char* lpszName);
	/**
	 * @brief Set the site number
	 * @param[in] uSiteNo The site number
	*/
	void SetSite(USHORT uSiteNo);
	/**
	 * @brief Set the pin string
	 * @param[in] lpszName The name of the pin string
	 * @param[in] bPinName Whether the pin string is pin name
	*/
	void SetPinString(const char* lpszPinString, BOOL bPinName);
	/**
	 * @brief Set the site count
	 * @param[in] uSiteCount The site count
	*/
	void SetSiteCount(USHORT uSiteCount);
	/**
	 * @brief Set the site invalid alarm
	 * @param[in] bAlarm Whether alarm when site is invalid
	*/
	void SetInvalidSiteAlarm(BOOL bAlarm);
	/**
	 * @brief Set the alarm message
	 * @param[in] lpszFormat The format of mess
	 * @param[in]  The message will be formated
	*/
	void SetAlarmMsg(const char* lpszFormat, ...);
	/**
	 * @brief Set the parameter name
	 * @param[in] lpszParameterName The parameter name
	 * @return Execute result
	 * - 0 Set parameter name successfully
	 * - -1 The point of parameter name is nullptr
	*/
	int SetParamName(const char* lpszParameterName);
	/**
	 * @brief Not load vector alarm
	*/
	void VectorNotLoadedAlarm();
	/**
	 * @brief Output alarm
	 * @param[in] bClearFunctionName Whether clear the function name
	*/
	void Output(BOOL bClearFunctionName = TRUE);
	/**
	 * @brief The parameter is nullptr 
	 * @param[in] lpszParamName The parameter name
	 * @param[in] lpszPinStriong The site number
	 * @param[in] lpszPinString The pin string
	 * @param[in] bPinName Whether the pin string is pin name
	 * @return Execute result
	 * - 0 Set alarm successfully
	 * - -1 The point of parameter is nullptr
	*/
	int ParameterNullptrAlarm(const char* lpszParamName, USHORT usSiteNo, const char* lpszPinString, BOOL bPinName = TRUE);
	/**
	 * @brief The pin error alarm
	 * @param[in] lpszPinString The pin string
	 * @param[in] bPinName Whether the pin string is pin name
	 * @return Execute result
	 * - 0 Set pin error alarm successfully
	 * - -1 The point of pin string is nullptr
	*/
	int PinError(const char* lpszPinString, BOOL bPinName);
	/**
	 * @brief The timeset alarm
	 * @param[in] lpszTimeset The timeset name
	 * @return Execute result
	 * - 0 Set timeset error successfully
	 * - -1 The timeset name is nullptr
	*/
	int TimesetError(const char* lpszTimeset);
	/**
	 * @brief Set allocate memory error alarm
	*/
	void AllocateMemoryError();
	/**
	 * @brief The function used error
	 * @param[in] lpszPreviousFunction The previous function name
	 * @return Execute result
	 * - 0 Set alarm successfully
	 * - -1 The point of function is nullptr
	*/
	int FunctionUseError(const char* lpszPreviousFunction);
	/**
	 * @brief Unknown error
	*/
	void UnknownError();
	/**
	 * @brief The site over scale alarm
	 * @param[in] lpszPinString The pin string
	 * @param[in] uSiteNo The site number
	 * @param[in] uMaxSiteNo The maximum site number
	 * @param[in] bPinName Whether the pin string is pin name
	 * @return Execute result
	 * - 0 Set alarm successfully
	 * - -1 The point of pin string is nullptr
	*/
	int SiteOverScaleAlarm(const char* lpszPinString, USHORT uSiteNo, USHORT uMaxSiteNo, BOOL bPinName);
	/**
	 * @brief No board alarm
	 * @param[in] lpszPinString The point of pin string
	 * @param[in] bPinName Whether the pin string is pin name
	 * @param[in] usSiteNo The site number
	 * @return Execute result
	 * - 0 Set alarm successfully
	 * - -1 The point of pin string is nullptr
	*/
	int SetNoBoardAlarm(const char* lpszPinString, BOOL bPinName = TRUE, USHORT usSiteNo = -1);
	/**
	 * @brief Set site invalid alarm
	 * @param[in] lpszPinName The pin name
	 * @param[in] usSiteNo The site number
	 * @param[in] bPinName Whether the pin string is pin name
	 * @return Execute result
	 * - 0 Set alarm successfully
	 * - -1 The point of pin string is nullptr
	*/
	int SiteInvalidAlarm(const char* lpszPinName, USHORT usSiteNo, BOOL bPinName = TRUE);
	/**
	 * @brief Set the pin whose alarm will be shielded
	 * @param[in] lpszPinName The pin name
	 * @param[in] usSiteNo The site number
	 * @param[in] Channel The channel of the pin
	 * @param[in] bShielf Whether shield alarm
	 * @return Execute result
	 * - 0 Set alarm successfully
	 * - -1 The point of pin string is nullptr
	*/
	int ShieldPin(const char* lpszPinName, USHORT usSiteNo, CHANNEL_INFO& Channel, BOOL bShielf);
	/**
	 * @brief Get the pin shield status
	 * @param[in] lpszPinName The pin name
	 * @param[in] usSiteNo The site number
	 * @return Shield status
	 * - 0 Not shield
	 * - 1 Shield
	 * - -1 The point of pin name is nullptr
	*/
	int GetShieldStatus(const char* lpszPinName, USHORT usSiteNo);
	/**
	 * @brief Whether set alarm message
	 * @return Result
	 * - TRUE Set message already
	 * - FALSE Not set message
	*/
	BOOL IsSetMsg();
	/**
	 * @brief Shield alarm
	 * @param[in] lpszPinName The pin name
	 * @param[in] usSiteNo The site number
	 * @param[in] ChannelInfo The channel information of the pin in the site
	 * @param[in] lpszFunction The function name
	 * @param[in] bShield Whether shield alarm
	 * @param[in] AlarmID The alarm id
	 * @return Execute result
	 * - 0 Shield alarm successfully
	 * - -1 The point is nullptr
	*/
	int ShieldAlarm(const char* lpszPinName, USHORT usSiteNo, CHANNEL_INFO& ChannelInfo, const char* lpszFunction, BOOL bShield, ALARM_ID AlarmID = ALARM_ID::ALARM_PMU_CLAMP);
	/**
	 * @brief Get whether shield alarm
	 * @param[in] lpszPinName The pin name
	 * @param[in] usSiteNo The site number
	 * @param[in] lpszFunction The function name
	 * @param[in] AlarmID The alarm id
	 * @return Shield status
	 * - 0 Not shield
	 * - 1 Shield
	 * - -1 The point is nullptr
	*/
	int GetShieldAlarm(const char* lpszPinName, USHORT usSiteNo, const char* lpszFunction, ALARM_ID AlarmID  = ALARM_ID::ALARM_PMU_CLAMP);
	/**
	 * @brief Get the shielded channel
	 * @param[in] lpszFunction The function name
	 * @param[out] vecShieldChannel The shield channel
	 * @param[in] AlarmID The alarm ID
	*/
	void GetShieldChannel(const char* lpszFunction, std::vector<CHANNEL_INFO>& vecShieldChannel, ALARM_ID AlarmID = ALARM_ID::ALARM_PMU_CLAMP);
	/**
	 * @brief Check whether the alarm is open
	 * @return Whether the alarm open
	 * - TRUE Alarm opened
	 * - FALSE Alarm close
	*/
	int IsAlarmOpen();
private:
	friend DCM400Data;
	/**
	 * @brief Constructor
	*/
	CDriverAlarm();
	/**
	 * @brief Set the module information
	 * @param[in] lpszInputValue The input value before format
	*/
	inline void SetModuleInfo();
	/**
	 * @brief Output alarm to file
	 * @return Execute result
	 * - 1 Output alarm
	 * - 0 Not output alarm
	*/
	int OutputAlarm();
	/**
	 * @brief Set range alarm
	 * @param[in] lpszParamName The parameter
	 * @param[in] dInputValue The input value
	 * @param[in] dMinValue The minimum value of the range
	 * @param[in] dMaxValue The maximum value of the range
	 * @param[in] byDecimalDigits The decimal count
	 * @return Execute result
	 * - 0 set range alarm successfully
	 * - -1 The site number is over range
	*/
	void SetRangeAlarm(const char* lpszParamName, double dInputValue, double dMinValue, double dMaxValue, BYTE byDecimalDigits);
	/**
	 * @brief Check whether site is valid
	 * @param[in] usSiteNo The site number
	 * @return Whether site is valid
	 * - true Site valid
	 * - false Site invalid
	*/
	inline bool IsSiteValid(USHORT usSiteNo);
	/**
	 * @brief Clear the alarm information
	 * @param[in] bClear Clear the function name
	*/
	inline void ClearInfo(BOOL bClear = TRUE);
private:
	std::string m_strPinString;///<The pin string
	USHORT m_uSiteCount;///<The site count
	std::string m_strModuleInfo;///<The module name
	char m_lpszAlarmMsg[512];///<The alarm message
	std::string m_strParamName;///<The parameter name
	BOOL m_bSiteInvalidAlarm;///<Whether output invalid site alarm
	BOOL m_bPinName;///<Whether the pin string is pin name
	USHORT m_uSiteNo;///<The current site number
	ALARM_ID m_AlarmID;///<The alarm ID
	ParamAlarm m_AlarmInfo;///<The alarm information
	std::map<std::string, CShieldPin*> m_mapShieldPin;///<The pin shielded in all function
	std::map<std::string, CShieldFunction*> m_mapShiedFunction;///<The function whose alarm will be shielded
	ALARM_TYPE m_AlarmType;///<The alarm type
};