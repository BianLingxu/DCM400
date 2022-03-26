#pragma once
/**
 * @file ConfigFile.h
 * @brief The class for reading and writting configuration file with ini extension
 * @author Guangyun Wang
 * @date 2021/08/09
 * @copyright AccoTEST Business Unit of Beijing Huafeng Test & Control Technology Co., Ltd.
*/
#include <string>
#include <map>
#include <vector>
/**
 * @CSection
 * @brief The class for saving the key under the app
*/
class CSection
{
public:
	/**
	 * @brief Constructor
	 * @param[in] lpszName The section name
	*/
	CSection(const char* lpszName);
	/**
	 * @brief Set the key value
	 * @param lpszKey The key name
	 * @param lpszValue The value of the key
	 * @return Execute result
	 * - 0 Set the key successfully
	 * - -1 The key name is nullptr
	 * - -2 The value is nullptr
	*/
	int SetKey(const char* lpszKey, const char* lpszValue);
	/**
	 * @brief Get the app name
	 * @return The app name
	*/
	const char* Name() const;
	/**
	 * @brief Get the key value
	 * @param[in] lpszKey The key name
	 * @return The value of the key
	 * - != nullptr The key value
	 * - nullptr The key is nullptr or not existed
	 * - -2 The key is not existed
	*/
	const char* GetValue(const char* lpszKey) const;
	/**
	 * @brief Get the key under app
	 * @param[out] vecKey All key under the app
	*/
	void GetKey(std::vector<std::string>& vecKey) const;
private:
	std::string m_strName;///<The section name
	std::map<std::string, std::string> m_mapKeyValue;///<The key value under App
};
/**
 * @class CConfigFile
 * @brief The class for ini configuration file reading and writing
*/
class CConfigFile
{
public:
	/**
	 * @brief Constructor
	 * @param[in] lpszFile The file name
	*/
	CConfigFile(const char* lpszFile);
	/**
	 * @brief Destructor
	*/
	~CConfigFile();
	/**
	 * @brief Set value to init file
	 * @param[in] lpszSection The section name
	 * @param[in] lpszKey The key name
	 * @param[in] lpszFormat The value format
	 * @param  The value need to be format
	 * @return Execute result
	 * - 0 Set the value successfully
	 * - -1 The section name is nullptr
	 * - -2 The key name is nullptr
	 * - -3 The format is nullptr
	*/
	int SetValue(const char* lpszSection, const char* lpszKey, const char* lpszFormat, ...);
	/**
	 * @brief Get the value
	 * @param[in] lpszSection The section name
	 * @param[in] lpszKey The key name
	 * @return The key value
	 * - !=nullptr The value of the key upder the app
	 * - nullptr The section or key is nullptr or not existed
	*/
	const char* GetValue(const char* lpszSection, const char* lpszKey);
	/**
	 * @brief Clear the value update section
	 * @param lpszApp The section name
	 * @return Execute result
	 * - 0 Clear setion successfully
	 * - -1 The section is nullptr
	*/
	int ClearSection(const char* lpszApp);
	/**
	 * @brief Save the configuration file
	 * @return Execute result
	 * - 0 Save configuration successfully
	 * - -1 The file can't be save
	*/
	int Save();
private:
	/**
	 * @brief Reset data
	*/
	void Reset();
private:
	std::string m_strFile;///<The file name
	std::map<std::string, CSection*> m_mapSection;///<The app in file, key is section name and value is its point of CSection
};

