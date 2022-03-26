#ifndef __6B97B958_F326_4D6E_A6FD_81F80AEE0B55_HDDEBUGTOOLITEMMCUMEAS_H__
#define __6B97B958_F326_4D6E_A6FD_81F80AEE0B55_HDDEBUGTOOLITEMMCUMEAS_H__

#include "HDDebugToolItem.h"

class HDDebugToolItemMcuMeas : public HDDebugToolItem
{
public:
	HDDebugToolItemMcuMeas(HDDebugTool * debugTool);
	~HDDebugToolItemMcuMeas();



    virtual int Type() const;
    virtual const char * Name() const;

    virtual int CanModifyData() const;

    virtual int GetData(int site, int loagicChannel, STSVariant & data, STSVariant & mark) const;
};


#endif /* __6B97B958_F326_4D6E_A6FD_81F80AEE0B55_HDDEBUGTOOLITEMMCUMEAS_H__ */