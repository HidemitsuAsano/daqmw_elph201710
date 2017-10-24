// CAENModule.cpp
// 2014.10.23
// T. Yamaga

#include "CAENModule.h"

CAENModule::CAENModule()
{
}

CAENModule::~CAENModule()
{
}

void CAENModule::Module_Init(const CAENv1718& v1718, uint32_t baddr, unsigned short geo)
{
    this->v1718 = v1718;
    BaseAddr = baddr;
    GEOAddr = geo;
    Module_SetGEOAddrReg(GEOAddr);
    Module_SoftwReset();

}

void CAENModule::Module_End()
{
    Module_SoftwReset();
}

CVErrorCodes CAENModule::Module_SetGEOAddrReg(unsigned short Mask)
{
    return v1718.WriteCycle(BaseAddr+cmGEOAddrReg,&Mask);
}

CVErrorCodes CAENModule::Module_ReadGEOAddrReg(unsigned short* pData)
{
    CVErrorCodes ret =  v1718.ReadCycle(BaseAddr+cmGEOAddrReg,pData);
    if (ret == cvCommError){
        ret =  v1718.ReadCycle(BaseAddr+cmGEOAddrReg,pData);
    }
}

CVErrorCodes CAENModule::Module_SetBitSet1Reg(unsigned short Mask)
{
    return v1718.WriteCycle(BaseAddr+cmBitSet1Reg,&Mask);
}

CVErrorCodes CAENModule::Module_SetBitClear1Reg(unsigned short Mask)
{
    return v1718.WriteCycle(BaseAddr+cmBitClear1Reg,&Mask);
}

CVErrorCodes CAENModule::Module_SetInterruptLevelReg(unsigned short Mask)
{
    return v1718.WriteCycle(BaseAddr+cmInterruptLevelReg,&Mask);
}

CVErrorCodes CAENModule::Module_SetInterruptVectorReg(unsigned short Mask)
{
    return v1718.WriteCycle(BaseAddr+cmInterruptVectorReg,&Mask);
}

CVErrorCodes CAENModule::Module_ReadStatusReg1(unsigned short* pData)
{
    CVErrorCodes ret =  v1718.ReadCycle(BaseAddr+cmStatusReg1,pData);
    if (ret != cvSuccess){
        printf("BaseAddr+cmStatusReg1 : %08X : %04X\n",BaseAddr+cmStatusReg1,*pData);
        ret =  v1718.ReadCycle(BaseAddr+cmStatusReg1,pData);
    }

    return ret;
}

CVErrorCodes CAENModule::Module_SetControlReg1(unsigned short Mask)
{
    return v1718.WriteCycle(BaseAddr+cmControlReg1,&Mask);
}

CVErrorCodes CAENModule::Module_SetEventTriggerReg(unsigned short Mask)
{
    return v1718.WriteCycle(BaseAddr+cmEventTriggerReg,&Mask);
}

CVErrorCodes CAENModule::Module_ReadStatusReg2(unsigned short* pData)
{
    return v1718.ReadCycle(BaseAddr+cmStatusReg2,pData);
}

CVErrorCodes CAENModule::Module_ReadEventCounterL(unsigned short* pData)
{
    return v1718.ReadCycle(BaseAddr+cmEventCounterLowReg,pData);
}

CVErrorCodes CAENModule::Module_ReadEventCounterH(unsigned short* pData)
{
    return v1718.ReadCycle(BaseAddr+cmEventCounterHighReg,pData);
}

CVErrorCodes CAENModule::Module_SetBitSet2Reg(unsigned short Mask) {
    return v1718.WriteCycle(BaseAddr+cmBitSet2Reg,&Mask);
}

CVErrorCodes CAENModule::Module_SetBitClear2Reg(unsigned short Mask)
{
    return v1718.WriteCycle(BaseAddr+cmBitClear2Reg,&Mask);
}

CVErrorCodes CAENModule::Module_SetEventCounterResetReg(unsigned short Mask)
{
    return v1718.WriteCycle(BaseAddr+cmEventCounterResetReg,&Mask);
}

CVErrorCodes CAENModule::Module_SetRegister(uint32_t Addr,unsigned short Mask)
{
    return v1718.WriteCycle(Addr,&Mask);
}

CVErrorCodes CAENModule::Module_SoftwReset()
{
    Module_SetBitSet1Reg(cmSoftwReset);
    return Module_SetBitClear1Reg(cmSoftwReset);
}
