
#ifndef CAENMODULETYPES_H
#define CAENMODUELTYPES_H

#define CAEN_BYTE       unsigned char
#define CAEN_BOOL       int

/*
        Resgister addres
*/
typedef enum CMRegisters {
        cmGEOAddrReg            = 0x1002,       /* GEO Address Register
        */
        cmBitSet1Reg            = 0x1006,       /* Bit Set1 Register
        */
        cmBitClear1Reg          = 0x1008,       /* Bit Clear 1 Register
        */
        cmInterruptLevelReg     = 0x100A,       /* Interrupt Level Register
        */
        cmInterruptVectorReg    = 0x100C,       /* Interrupt Vector Register
        */
        cmStatusReg1            = 0x100E,       /* Status Register 1
        */
        cmControlReg1           = 0x1010,       /* Control Register 1
        */
        cmAddressDecoderHighReg = 0x1012,       /* Address Decoder High Register
        */
        cmAddressDecoderLowReg  = 0x1014,       /* Address Decoder Low Register
        */
        cmSingleShotResetReg    = 0x1016,       /* SIngle Shot Reset Register
        */
        cmEventTriggerReg       = 0x1020,       /* Event Trigger Register
        */
        cmStatusReg2            = 0x1022,       /* Status Register 2
        */
        cmEventCounterLowReg     = 0x1024,       /* Event Counter Low Register
        */
        cmEventCounterHighReg   = 0x1026,       /* Event Counter High Register
        */
        cmBitSet2Reg            = 0x1032,       /* Bit Set 2 Register
        */
        cmBitClear2Reg          = 0x1034,       /* Bet Clear 2 Register
        */
        cmEventCounterResetReg  = 0x1040,       /* Event COunter Reset Register
        */
        
} CMRegisters;

/*
        Bits for status register 1 decoding.
*/
typedef enum CMStatusRegister1Bits {
        cmDREADY            = 0x0001,           /* Data ready
        */
        cmGLOBALDREADY      = 0x0002,           /* Global Dready
        */
        cmBUSY              = 0x0004,           /* Busy
        */
        cmGLOBALBUSY        = 0x0008,           /* Global Busy
        */
        cmAMNESIA           = 0x0010,           /* about GEO address 
        */
        cmPURGED            = 0x0020,           /* the board is purged / not
        */
        cmTERMON            = 0x0040,           /* Control bus terminations
        */
        cmTERMOFF           = 0x0080,           /* Control bus terminations
        */
        cmEVRDY             = 0x0100,           /* about Event Trigger
        */
} CMStatusRegister1Bits;

/*
        Bits for status register 2 decoding.
*/
typedef enum CMStatusRegister2Bits {
        cmBUFFEREMPTY       = 0x0002,           /* Buffer is empty
        */
        cmBUFFERFULL        = 0x0004,           /* Buffer is full
        */
} CMStatusRegister2Bits;

/*
        Bits for control register 1 decoding.
*/
typedef enum CMControlRegister1Bits {
        cmBLKEND            = 0x0004,           /* Block transer setting
        */
        cmPROGRESET         = 0x0010,           /* about Porgrammable reset
        */
        cmBERENABLE         = 0x0020,           /* Bus error enable
        */
        cmALIGN64           = 0x0040,           /* about 64 bit CPU
        */
} CMControlRegister1Bits;

/*
        Bits for bit set 1 register decoding.
*/
typedef enum CVBitSet1RegisterBits {
        cmBerrFlag       = 0x0008,              /* Berr Flag bit
        */
        cmSelectAddress  = 0x0010,              /* Select Address bit
        */
        cmSoftwReset     = 0x0080,              /* Soft ware Reset bit
        */
} CMBitSet1RegisterBits;

/*
        Bits for bit set 2 register decoding.
*/
typedef enum CVBitSet2RegisterBits {
        cmEMPTYENABLE    = 0x0008,              /* writing header and EOB when there are no accepted channels.
        */
        cmAUTOINCR       = 0x0800,              /* writing header and EOB when there are no accepted channels.
        */
} CMBitSet2RegisterBits;

/*
        Bits for Interrupt Level register decoding.
*/
typedef enum CMInterruptLevelRegisterBits {
        cmIRQLev0       = 0x0000,              /* Level 0
        */
        cmIlQLev1       = 0x0001,              /* Level 1
        */
        cmIlQLev2       = 0x0002,              /* Level 2
        */
        cmIlQLev3       = 0x0003,              /* Level 3
        */
        cmIlQLev4       = 0x0004,              /* Level 4
        */
        cmIlQLev5       = 0x0005,              /* Level 5
        */
        cmIlQLev6       = 0x0006,              /* Level 6
        */
        cmIlQLev7       = 0x0007,              /* Level 7
        */
} CMInterruptLevelRegisterBits;

#endif // __CAENVMETYPES_H
