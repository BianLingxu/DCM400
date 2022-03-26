#ifndef __BASEDATAOP_H__
#define __BASEDATAOP_H__
#include <Windows.h>
class STSDataStream;

class BaseDataOp
{
public:
	virtual bool Write(STSDataStream & ds, int nChannelIndex) = 0;
	virtual bool Read(STSDataStream & ds, int nChannelIndex) = 0;
	virtual void Clear() = 0;

	virtual int WriteToFlash() = 0;
	virtual int ReadFromFlash(DWORD dwGetChannel) = 0;
};

#endif /*__BASEDATAOP_H__*/
