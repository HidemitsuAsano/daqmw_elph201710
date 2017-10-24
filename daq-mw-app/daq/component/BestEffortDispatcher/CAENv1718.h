// CAENv1718.h
// 2014.10.22
// T. Yamaga
// 2017 10 8
// modified by Hidemitsu Asano

#ifndef CAENv1718_h
#define CAENv1718_h

#include <iostream>

#include "CAENVMEtypes.h"
#include "CAENVMElib.h"

#include <stdint.h>
#include <ctype.h>
#include <cstring>

class CAENv1718
{
public:
CAENv1718();
CAENv1718(CVBoardTypes,short,short,int32_t* pHandle);
CAENv1718(const CAENv1718&);
CAENv1718(const CAENv1718*&);
~CAENv1718();

private:
CVBoardTypes BdType;
short Link;
short BdNum;
int32_t* pHandle;

public:
int32_t Handle() {return *pHandle;}
CVErrorCodes Initialize();
CVErrorCodes Finalize();
CVErrorCodes IRQCheck(CAEN_BYTE* pMask);
CVErrorCodes SetPuls(CVPulserFreq);
CVErrorCodes SetPulserConf(CVPulserSelect=cvPulserA,unsigned char=255,
        unsigned char=2,CVTimeUnits=cvUnit25ns,unsigned char=0,
        CVIOSources=cvManualSW,CVIOSources=cvManualSW);
CVErrorCodes SetOutputConf(CVOutputSelect,CVIOPolarity=cvDirect,
            CVLEDPolarity=cvActiveHigh,CVIOSources=cvManualSW);
CVErrorCodes SetOutputRegister(unsigned short);
CVErrorCodes ClearOutputRegister(unsigned short);
CVErrorCodes SystemReset();
CVErrorCodes StartPulser(CVPulserSelect=cvPulserA);
CVErrorCodes OnePulse(CVPulserSelect=cvPulserA);
CVErrorCodes OnePulse2(CVPulserSelect=cvPulserB);
CVErrorCodes StopPulser(CVPulserSelect=cvPulserA);
CVErrorCodes IRQWait(uint32_t=0xFFFFFFFF,uint32_t=10000);
CVErrorCodes ReadCycle(uint32_t,void*,
        CVAddressModifier=cvA32_U_DATA,CVDataWidth=cvD16);
CVErrorCodes WriteCycle(uint32_t,void*,
        CVAddressModifier=cvA32_U_DATA,CVDataWidth=cvD16);
CVErrorCodes BLTReadCycle(uint32_t,void*,
        int=256,CVAddressModifier=cvA32_U_MBLT,CVDataWidth=cvD32);
CVErrorCodes BLTWriteCycle(uint32_t,void*,
        int=256,CVAddressModifier=cvA32_U_MBLT,CVDataWidth=cvD32);
CVErrorCodes SetBusy(int);
CVErrorCodes SetBusy2(int);
};

#endif
