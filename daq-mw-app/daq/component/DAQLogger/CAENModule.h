// CAENModule.h
// 2014.10.23
// T. Yamaga

#ifndef CAENModule_h
#define CAENModule_h

#include <iostream>
#include <string>

#include "CAENVMEtypes.h"
#include "CAENModuletypes.h"
#include "CAENv1718.h"

class CAENModule
{
    public :
        CAENModule();
        ~CAENModule();

    protected :
        CAENv1718 v1718;
        uint32_t BaseAddr;
        unsigned short GEOAddr;

    protected :
        void Module_Init(const CAENv1718&,uint32_t,unsigned short=0xFF);
        void Module_End();
        CVErrorCodes Module_SetGEOAddrReg(unsigned short);
        CVErrorCodes Module_ReadGEOAddrReg(unsigned short*);
        CVErrorCodes Module_SetBitSet1Reg(unsigned short);
        CVErrorCodes Module_SetBitClear1Reg(unsigned short);
        CVErrorCodes Module_SetInterruptLevelReg(unsigned short);
        CVErrorCodes Module_SetInterruptVectorReg(unsigned short);
        CVErrorCodes Module_ReadStatusReg1(unsigned short*);
        CVErrorCodes Module_SetControlReg1(unsigned short);
        CVErrorCodes Module_SetEventTriggerReg(unsigned short);
        CVErrorCodes Module_ReadStatusReg2(unsigned short*);
        CVErrorCodes Module_ReadEventCounterL(unsigned short*);
        CVErrorCodes Module_ReadEventCounterH(unsigned short*);
        CVErrorCodes Module_SetBitSet2Reg(unsigned short);
        CVErrorCodes Module_SetBitClear2Reg(unsigned short);
        CVErrorCodes Module_SetEventCounterResetReg(unsigned short=1);
        CVErrorCodes Module_SetRegister(uint32_t,unsigned short);
        CVErrorCodes Module_SoftwReset();
};

#endif
