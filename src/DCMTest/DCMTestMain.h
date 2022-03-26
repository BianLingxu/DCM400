/*!
 * @file      DCMTestMain.h
 *
 * Copyright (C) 北京华峰测控技术股份有限公司
 *
 * @author    xg
 * @date      2015/09/16
 * @version   v 1.0.0.0
 * @brief     N/A
 */
#ifndef __AE887F1B_E448_49C8_8278_5FC7F4C242C1_DCMTESTMAIN_H__
#define __AE887F1B_E448_49C8_8278_5FC7F4C242C1_DCMTESTMAIN_H__

#include <iostream>
#include "xgTest.h"
#include "Sts8100.h"
#include "STSCoreGlobal.h"
#include "Userres.h"
#include "DriverPackTestEnvironment.h"
#include "STSCoreGlobal.h"
#include "SM8213.h"
#include "HardwareInfo.h"
DCM dcm;

const char* g_lpszVectorFilePath = "E:\\HardwareDriver\\STS8250\\multiSite_change\\src\\DCMTest\\DCM_TEST.acvec";
char g_lpszReportFilePath[MAX_PATH] = "C:\\AccoTest\\STS8250\\test.txt";
const char* g_lpszI2CVectorFilePath = "E:\\HardwareDriver\\STS8250\\multiSite_change\\src\\DCMTest\\DCM_TEST_I2C.acvec";


#include "DCM_MCU_TEST/MCUCase.h"

#include "DCM_PPMU_Test/PPMUCase.h"

#include "DCM_TMU_TEST/TMUCase.h"

#endif /* __AE887F1B_E448_49C8_8278_5FC7F4C242C1_DCMTESTMAIN_H__ */
