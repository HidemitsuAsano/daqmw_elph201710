#ifndef REGISTERH_
#define REGISTERH_

//-------------------------------------------------------------------------
// DRS4 controller 
//-------------------------------------------------------------------------
namespace DRS{
  static const int mid = 0x0;
  enum LocalAddress{
    laddr_exec_command  = 0x000, // W, 0x0: exec spi, 0x1, exec scc
    laddr_register      = 0x010, // W [4:0]+[7:0] addr and reg for DRS4
                                 // 0x10:config, 0x11:wsr, 0x12:wcr
                                 // 0x13:only addr is set
    laddr_control       = 0x020, // W/R [3:0], refclk, denable, init_srin
    laddr_maxcount      = 0x030, // W/R [9:0], max count of en_srclk
    laddr_status        = 0x040, // R   [7:0], dtap & pll-lock
    laddr_spidata       = 0x050, // R   [7:0], decoded srout data (4 chips)
    laddr_cellnum       = 0x060, // R   [9:0], decoded cell num
                                 // 0x60-0x63 (lower 8bit), 0x64-0x67 (higher 2bit),
    laddr_scc_delay     = 0x070  // W   [7:0], delay count of SCC
  };

  // lower bit of laddr
  enum Exec{
    laddr_exec_spi = 0x0,
    laddr_exec_scc = 0x1
  };

  // lower bit of laddr
  enum DrsReg{
    laddr_drsconfig = 0x0,
    laddr_drswsr    = 0x1,
    laddr_drswcr    = 0x2,
    laddr_drsaddr   = 0x3
  };

  // data register
  enum Config{
    c_init_srin    = 0x1,
    c_denable      = 0x2,
    c_refclk       = 0x4,
    c_block_den    = 0x8,
    c_block_rsr    = 0x10,
    c_rst          = 0x20
  };

  // DRS4 part --------------------------------------------------
  // These bit perttern are used in register at 0x30
  // Address on DRS4
  enum DRS_Address{
    addr_mux_ch0    = 0x0U,  // Ch 0 at MUXOUT
    addr_mux_ch1    = 0x1U,  // Ch 1 at MUXOUT
    addr_mux_ch2    = 0x2U,  // Ch 2 at MUXOUT
    addr_mux_ch3    = 0x3U,  // Ch 3 at MUXOUT
    addr_mux_ch4    = 0x4U,  // Ch 4 at MUXOUT
    addr_mux_ch5    = 0x5U,  // Ch 5 at MUXOUT
    addr_mux_ch6    = 0x6U,  // Ch 6 at MUXOUT
    addr_mux_ch7    = 0x7U,  // Ch 7 at MUXOUT
    addr_mux_ch8    = 0x8U,  // Ch 8 at MUXOUT
    addr_enall      = 0x9U,  // Enable OUT0-OUT8
    addr_trans      = 0xAU,  // Enable Transparent Mode
    addr_read       = 0xBU,  // Addr read shift register (default)
    addr_config     = 0xCU,  // Addr config shift register
    addr_write      = 0xDU,  // Addr write shift register
    addr_wconfig    = 0xEU,  // Addr write config register
    addr_disen      = 0xFU   // Disable all outputs
  };

  enum Reg_Config{
    reg_dmode       = 0x1,  // DMODE
    reg_pllen       = 0x2,  // PLL enable
    reg_wsrloop     = 0x4,  // WSRLOOP
    reg_reserve     = 0xF8  // must be all 1
  };

  // These bit perttern are used in register at 0x34
  enum Reg_WSR{
    reg_nocascade   = 0xFF, // Enable all ch, no channel cascading
    reg_cascade     = 0x55  // Channel cascade, 01010101
  };

};

//-------------------------------------------------------------------------
// LTC2656-12
//-------------------------------------------------------------------------
namespace LTC{
  static const int mid = 0x1;
  enum LocalAddress{
    addr_exec     = 0x000,  // W            Araise Exec command
    addr_chipsel  = 0x010,  // W/(R) [2:0]  Set target chip
    addr_regsdi   = 0x020   // W     [23:0] SDI register
  };
  
  enum Control{
    ctrl_dac1  = 0x1, // Target device is first LTC2656_12
    ctrl_dac2  = 0x2, // Target device is second LTC2656_12
    ctrl_dac3  = 0x4  // Target device is third LTC2656_12
  };

  // LTC2656fa_12 part ------------------------------------------
  // CMD bits (4 bits)
  enum Command{
    cmd_wr_n             = 0x0U, // write register n ch
    cmd_upd_n            = 0x1U, // update n
    cmd_wr_n_upd_all     = 0x2U, // write register n, update all
    cmd_wr_n_upd_n       = 0x3U, // write and update n
    cmd_pwd_n            = 0x4U, // power down n
    cmd_pwd_chip         = 0x5U, // power down chip
    cmd_sel_internal_ref = 0x6U, // select internal reference
    cmd_sel_external_ref = 0x7U, // select external reference
    cmd_no_operation     = 0x15U // no operation
  };

  // ADDR bits (4 bits)
  enum Address{
    addr_A   = 0x0U,
    addr_B   = 0x1U,
    addr_C   = 0x2U,
    addr_D   = 0x3U,
    addr_E   = 0x4U,
    addr_F   = 0x5U,
    addr_G   = 0x6U,
    addr_H   = 0x7U,
    addr_All = 0x15U
  };
};

//-------------------------------------------------------------------------
// AD9637
//-------------------------------------------------------------------------
namespace AD9637{
  static const int mid = 0x2;
  enum LocalAddress{
    laddr_exec     = 0x000, // W/R 4chips (0x0, 1, 2, 3)
    laddr_set_reg  = 0x010, // W [12:0]+[7:0] addr+data to AD9636 (use ex data)
    laddr_adc_sync = 0x020  // arise ad_sync
  };

  // AD9637 part -------------------------------------------------------------
  // Internal memory map address
  enum Address{
    addr_spiconfig  = 0x0U,  // SPI port configuration
    addr_cid        = 0x1U,  // Chip ID
    addr_cgd        = 0x2U,  // Chip grade
    addr_dindex2    = 0x4U,  // Device index 2
    addr_dindex1    = 0x5U,  // Device index 1
    addr_transfer   = 0xFFU, // Transfer
    addr_pwmode     = 0x8U,  // Power mode
    addr_clk        = 0x9U,  // Clock
    addr_clkdiv     = 0xBU,  // Clock divide
    addr_clken      = 0xCU,  // Clock enhancement
    addr_test       = 0xDU,  // Test mode
    addr_offset     = 0x10U, // Offset adjust
    addr_outmode    = 0x14U, // Output mode
    addr_outadjust  = 0x15U, // Output adjust
    addr_outphase   = 0x16U, // Output phase
    addr_vref       = 0x18U, // Vref
    addr_user1_lsb  = 0x19U, // User pattern 1 LSB side
    addr_user1_msb  = 0x1AU, // User pattern 1 MSB side
    addr_user2_lsb  = 0x1BU, // User pattern 2 LSB side
    addr_user2_msb  = 0x1CU, // User pattern 2 MSB side
    addr_sctrl      = 0x21U, // Serial control
    addr_sch        = 0x22U, // Serial channel
    addr_resolution = 0x100U, // Resolution/ sample rate
    addr_user_IO2   = 0x101U, // User I/O control 2
    addr_user_IO3   = 0x102U, // User I/O control 3
    addr_sync       = 0x109U  // Sync
  };

};

// Local trigger manager --------------------------------------------------
namespace LTM{
  static const int mid = 0x3;
  enum LocalAddress{
    laddr_en_discri = 0x0 // R/W [15:0], [15:8] + [7:0] (0x0, 0x1) (R)
  };
};

// Master triger manager --------------------------------------------------
namespace MTM{
  static const int mid = 0x4;
  enum LocalAddress{
    laddr_sel_trig = 0x0,  // R/W [9:0]
    laddr_delay    = 0x10, // R/W [7:0],  1bit = 10ns
    laddr_clk_freq = 0x20  // R/W [31:0], clk trigger freq
  };
  
  enum SelTrig{
    // level1 
    reg_L1clk   = 0x1,
    reg_L1local = 0x2,
    reg_L1ext   = 0x4,
    reg_L1j0    = 0x8,
    // level2 
    reg_L2ext   = 0x10,
    reg_L2j0    = 0x20,
    // clear
    reg_clrext  = 0x40,
    reg_clrj0   = 0x80,
    // enable
    reg_enl2    = 0x100,
    reg_enj0    = 0x200
  };
  
  enum Freq{
    reg_freq_1hz   = 0x01fca055,
    reg_freq_5hz   = 0x00fe502a,
    reg_freq_10hz  = 0x0032dcd5,
    reg_freq_50hz  = 0x000a2c2a,
    reg_freq_100hz = 0x00051615,
    reg_freq_xxxhz = 0x0003b5fd
    //    reg_freq_xxxhz = 0x0003b1fd
  };

};

// DAC FCN ----------------------------------------------------------------
namespace DAQ{
  static const int mid = 0x6;
  enum LocalAddress{
    laddr_ExecDAQ = 0x0, // W [0:0], write excute DAQ command
    laddr_EnBusy  = 0x10 // W [2:0], enble busy
  };

  enum Busy{
    reg_self = 0x1,
    reg_DRS  = 0x2,
    reg_EVB  = 0x4,
    reg_TDC  = 0x8
  };
};

// EVB --------------------------------------------------------------------
namespace EVB{
  static const int mid = 0x7;
  enum LocalAddress{
    laddr_reset       = 0x000, // W,[0:0]  arise reset
    laddr_dready      = 0x010, // R,[0:0]  data ready bit
    laddr_we_delay    = 0x020, // W,[4:0]  we_fifo delay count
    laddr_w_count     = 0x030, // W,[10:0] write count
    laddr_control     = 0x040, // W,[1:0]  0:reverse window, 1:noise filter
    laddr_en_wf       = 0x050, // W,[15:0] enable wave form
    laddr_nevent      = 0x060, // W,[11:0] max multi-event number
    laddr_ddata       = 0x070, // W/R,[0:0] double data flag
    laddr_cascade     = 0x080, // W, [19:0] cascade threshold
    laddr_readfifo    = 0x100, // R, x100 + ADC ch // W, x100 + LUT ch
    laddr_low_th      = 0x200, // W,[11:0] noise filter low th
    laddr_zs_th       = 0x300, // W,[15:0] zero suppress th
    laddr_blcor1      = 0x400, // W,[6:0]  baseline correction1
    laddr_blcor2      = 0x500  // W,[6:0]  baseline correction2
  };
  
  enum Control{
    c_reverse_window = 0x1,
    c_noise_filter   = 0x2,
    c_zs_wf          = 0x4,
    c_zs_QDC         = 0x8,
    c_short_header   = 0x10
  };
};

// OMN --------------------------------------------------------------------
namespace OMN{
  static const int mid = 0x8;
  enum LocalAddress{
    laddr_out1 = 0x0,   // W/R [4:0] select signal to output NIMOUT1
    laddr_out2 = 0x10   // W/R [4:0] select signal to output NIMOUT2
  };

  enum OutSignal{
    reg_discri0,  reg_discri1,  reg_discri2,  reg_discri3,  reg_discri4,
    reg_discri5,  reg_discri6,  reg_discri7,  reg_discri8,  reg_discri9,
    reg_discri10, reg_discri11, reg_discri12, reg_discri13, reg_discri14,
    reg_discri15,
    reg_or16,     reg_busy,     reg_daqgate,
    reg_l1req,    reg_l1acc,
    sizeOutSignal
  };
};

// BCT --------------------------------------------------------------------
namespace BCT{
  static const int mid = 0xe;
  enum LocalAddress{
    laddr_Reset   = 0x0, // W , arise reset signal from BCT
    laddr_Version = 0x10 // R [31:0]
  };
};

// TDC --------------------------------------------------------------------
namespace TDC{
  static const int mid = 0x9;
  enum LocalAddress{
    // channel function enable registers
    // address  = channel id
    // bits
    // bit [0] : enable external hit
    // bit [1] : always enable calibration clock pulse
    // bit [2] : enable LUT histogram update
    // bit [3] : enable channel buffer
    // bit [5:4] : measurement mode [5:4] = "01" trailing edge, "10" 2nd leading edge
    // default value = 011001  (0x19)
    laddr_enable_ch          = 0x000,

    // channel buffer almost full threshold
    laddr_ch_th_assert       = 0x800,
    laddr_ch_th_negate       = 0x801,
    
    // channel buffer for L1 trigger almost full threshold
    laddr_trig_th_assert     = 0x802,
    laddr_trig_th_negate     = 0x803,
    
    // L1 buffer almost full threshold
    laddr_l1buf_th_assert0   = 0x804,
    laddr_l1buf_th_assert1   = 0x805, 
    laddr_l1buf_th_negate0   = 0x806,
    laddr_l1buf_th_negate1   = 0x807,

    // L1 trigger matching enable/disable
    // laddr_en_window_match    = 0x810,
    // laddr_en_l1buf_timeout   = 0x811,

    // time window settings
    laddr_reject_latency0    = 0x813,
    laddr_reject_latency1    = 0x814,
    laddr_window_offset0     = 0x815,
    laddr_window_offset1     = 0x816,
    laddr_search_window0     = 0x817,
    laddr_search_window1     = 0x818,
    laddr_l1_delay0          = 0x819,
    laddr_l1_delay1          = 0x81A,  
    
    // readout FIFO almost full threshold 
    laddr_rofifo_th_assert0  = 0x820,
    laddr_rofifo_th_assert1  = 0x821,
    laddr_rofifo_th_negate0  = 0x822,
    laddr_rofifo_th_negate1  = 0x823,
    // TDC subtraction mode
    laddr_l1_subtract        = 0x824,


    laddr_l2buf_th_assert0  = 0x830,
    laddr_l2buf_th_assert1  = 0x831,
    laddr_l2buf_th_negate0  = 0x832,
    laddr_l2buf_th_negate1  = 0x833,

    laddr_l2fi_th_assert0  = 0x840,
    laddr_l2fi_th_assert1  = 0x841,
    laddr_l2fi_th_negate0  = 0x842,
    laddr_l2fi_th_negate1  = 0x843,
    
    
    // write access to this address issue a start signal of LUT calibration
    laddr_start_calibration  = 0x900,
    // calibration sequence is stopped when the counter reaches to the preset value.
    laddr_Lut_cnt_max0       = 0x910,
    laddr_Lut_cnt_max1       = 0x911,
    laddr_Lut_cnt_max2       = 0x912,

    // peek LUT control registers
    // channel to be peeked (0 to 255)
    laddr_peek_ch            = 0x920,
    // bin id to be peekded (0 to 255)
    laddr_peek_bin_id0       = 0x930,
    laddr_peek_bin_id1       = 0x931,
    // start peek sequence (write only)
    laddr_peek_lut           = 0x940,
    // fine time lower byte
    laddr_peek_ftime0        = 0x941,
    // fine time upper byte
    laddr_peek_ftime1        = 0x942,

    // BUSY output enable/disable control register
    // value
    //  1                    = enable output from TDC module.
    //  0                    = no output from TDC module.
    // bit [0] : calibration LUT building with an internal clock generator is ongoing
    // bit [1] : readout FIFO is almost full
    laddr_enable_busy        = 0xB00,
    // monitoring busy signal (read only)
    laddr_monitor_busy       = 0xBA0,

    // TDC thermo code edge debug
    laddr_edge_ch   = 0xd00,
    laddr_edge_oct  = 0xd01,
    laddr_edge_data = 0xd02, 

    // disable TDC
    laddr_disable_tdc        = 0xFE0, 
    
    // firmware version registers (read only)
    laddr_firmware_version0  = 0xFF0,
    laddr_firmware_version1  = 0xFF1,
    laddr_firmware_version2  = 0xFF2,
    laddr_firmware_version3  = 0xFF3
  }; // enum LocalAddress

  enum EMeasurement {
    kMeasurementTrailing           = 1,
    kMeasurementVeryHighResolution = 2
  };
  
  enum EL1Subtract {
    kL1SubtractCommonMinusChannel,
    kL1SubtractChannelMinusCommon,
    kL1SubtractNoOperation
  };
}; // namespace TDC



#endif
