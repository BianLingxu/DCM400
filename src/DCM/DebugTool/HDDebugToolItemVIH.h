#ifndef __68993BDC_C9C5_4983_A056_A8515957BA2F_HDDEBUGTOOLITEMVIH_H__
#define __68993BDC_C9C5_4983_A056_A8515957BA2F_HDDEBUGTOOLITEMVIH_H__

#include "HDDebugToolItem.h"

class HDDebugToolItemVIH : public HDDebugToolItem
{
public :
	HDDebugToolItemVIH(HDDebugTool * debugTool);
	~HDDebugToolItemVIH();

	virtual int Type() const;
	virtual const char * Name() const;

	virtual int GetData(int site, int logicChannel, STSVariant & data, STSVariant & mark) const;
};

#endif /*__68993BDC_C9C5_4983_A056_A8515957BA2F_HDDEBUGTOOLITEMVIH_H__*/