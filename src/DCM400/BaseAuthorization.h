#pragma once
/**
 * @file BaseAuthorization.h
 * @brief The head file for the pure virtual class CBaseAuthorization
 * @author Guangyun Wang
 * @date 2022/02/19
 * @copyright AccoTEST Business Unit of Beijing Huafent Test & Control Technology Co., Ltd.
*/

/**
 * @class CBaseAuthorization
 * @brief The pure virtual class for authorization management
 * @tparam T The template type
*/
template <typename T> class CBaseAuthorization
{
public:
	/**
	 * @brief Get the instance of current class
	 * @return The point pointed to instance
	*/
	static CBaseAuthorization* Instance();
	/**
	 * @brief Destructor
	*/
	~CBaseAuthorization();
	/**
	 * @brief Apply authorization
	*/
	void Apply();
	/**
	 * @brief Release authorization
	*/
	void Release();
	/**
	 * @brief Constructor
	*/
	CBaseAuthorization();
private:
	CRITICAL_SECTION m_criAuthorzation;///<The critical section for authorization management
};

template <typename T>
CBaseAuthorization<T>* CBaseAuthorization<T>::Instance()
{
	static CBaseAuthorization<T> Authorization;
	return &Authorization;
}

template <typename T>
CBaseAuthorization<T>::~CBaseAuthorization()
{
	DeleteCriticalSection(&m_criAuthorzation);
}

template <typename T>
CBaseAuthorization<T>::CBaseAuthorization()
{
	InitializeCriticalSection(&m_criAuthorzation);
}

template <typename T>
void CBaseAuthorization<T>::Release()
{
	LeaveCriticalSection(&m_criAuthorzation);
}

template <typename T>
void CBaseAuthorization<T>::Apply()
{
	EnterCriticalSection(&m_criAuthorzation);
}
