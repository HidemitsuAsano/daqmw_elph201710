#ifndef TDCCONTROLLER_
#define TDCCONTROLLER_

#include <string>

#include "FPGAModule.hh"
#include "Uncopyable.hh"

struct rbcp_header;


class TDCController
  : public FPGAModule,
    private Uncopyable<TDCController>
{
public:
  TDCController(char* ip,
                rbcp_header *rbcpHeader, 
                int disp_mode = 1);
  virtual ~TDCController();

  void Disable(bool val=true);
  unsigned int GetCDCBits(int ch,
                          int i_octet);
  bool GetCDCBits(int ch,
                  std::vector<unsigned char>& bit_vector);
  bool IsDisabled();
  void PeekLUT(int ch,
               int nbins,
               int verbose=1);
  void SetBufferAlmostFullThreshold(int th_assert,
                                    int th_negate,
                                    int laddr_assert_begin,
                                    int laddr_assert_end, 
                                    int laddr_negate_begin,
                                    int laddr_negate_end,
                                    const std::string& funcName="");

  void SetChannelBufferAlmostFullThreshold(int th_assert,
                                           int th_negate);
  void SetL1BufferAlmostFullThreshold(int th_assert,
                                      int th_negate);
  void SetL1Delay(int val);
  void SetL1TriggerFIFOAlmostFullThreshold(int th_assert,
                                           int th_negate);
  void SetL2BufferAlmostFullThreshold(int th_assert,
                                      int th_negate);
  void SetL2FrameIndicatorFIFOAlmostFullThreshold(int th_assert,
                                                  int th_negate);
  void SetMeasurementMode(int ch,
                          int mode);
  void SetReadOutFIFOAlmostFullThreshold(int th_assert,
                                         int th_negate);
  void SetReferenceChannelBufferAlmostFullThreshold(int th_assert,
                                                    int th_negate);
  void SetRejectLatency(int val);
  void SetWindowOffset(int val);
  void SetSearchWindow(int val);

  void StartCalibration();

  
// protected:
//   FPGAModule& fModule_;
};

#endif
