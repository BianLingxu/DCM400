#ifndef __B791CDA8_B590_4244_A91A_2471163E0786_HDDEBUGTOOLITEM_H__
#define __B791CDA8_B590_4244_A91A_2471163E0786_HDDEBUGTOOLITEM_H__

#include "IHDDebugToolItemCommon2.h"

class HDDebugTool;

class HDDebugToolItem : public IHDDebugToolItemCommon2
{
public:
    enum DebugItemType
    {
        // name               can modify

        // SetPinLevel->
        ITEM_VIH,           
        ITEM_VIL,			
        ITEM_VOH,           
        ITEM_VOL,           
        // <-setPinLevel

		// Connect/Disconnect->
		ITEM_OUT,
		// <-Connect/Disconnect

		// SetPTMU->
		ITEM_TMU_MODE,
		ITEM_TMU_CYLCETIMES,
		ITEM_TMU_MODENO,
		//<-SetPTMU

		// SetPPMU->
		ITEM_PMU_MODE,
		ITEM_PMU_FORCE,
		ITEM_PMU_IRNG,
		//<-SetPPMU

        // PPMUMeasure->
        ITEM_PMU_SAMPLES,
        ITEM_PMU_INTERVAL,
        // <-PPMUMeasure

		// I2CSet->
		ITEM_I2C_TIME,
		// <-I2CSet

		// I2CSetPinLevel->
		ITEM_I2C_VIH,
		ITEM_I2C_VIL,
		ITEM_I2C_VOH,
		ITEM_I2C_VOL,
		// <-I2CSetPinLevel

        // show->
		ITEM_TMU_MEAS,
		ITEM_MCU_MEAS,
		ITEM_PMU_MEAS,
        // <-show

		///<TMU
		ITEM_TMU_CHANNEL,
		ITEM_TMU_TRIGGER_EDGE,
		ITEM_TMU_HOLD_OFF_TIME,
		ITEM_TMU_HOLD_OFF_NUM,
		ITEM_TMU_TIMEOUT,

		///<Pin mode
		ITEM_PIN_MODE,
		///<Slot
		ITEM_PIN_SLOT,
    };
public:
    HDDebugToolItem(HDDebugTool * debugTool);
    ~HDDebugToolItem();

    virtual int QueryInterface(const char * interfaceName, void ** ptr);

    virtual const char * Caption() const;

    // 默认都支持修改数据
    virtual AccoTest::DataType DataType() const;
    
	virtual int GetData(int site, int logicChannel, STSVariant & data, STSVariant & mark) const;
	virtual int GetDataSizeAndDataType(int site, int logicChannel, STSVariant::Type & dataType, int & dataSize, STSVariant::Type & markType, int & markSize) const;

protected:
//     double GetIRang(int site, int logicChannel) const;
//     double GetVRang(int site, int logicChannel) const;

protected:
    HDDebugTool *m_pDebugTool;
};

#endif /* __B791CDA8_B590_4244_A91A_2471163E0786_HDDEBUGTOOLITEM_H__ */