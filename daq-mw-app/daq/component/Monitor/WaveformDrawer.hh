// -*- C++ -*-

#ifndef WaveformDrawer_hh
#define WaveformDrawer_hh

#include <string>

class THttpServer;

class WaveformDrawer
{
public:
  static constexpr double kAmplitudeUnit = 0.445; // mV
  static constexpr double kTimeUnit      = 1.0/960e6/1e-9; // nsec


  WaveformDrawer();
  ~WaveformDrawer();

  WaveformDrawer(const WaveformDrawer&);
  WaveformDrawer& operator=(const WaveformDrawer&);

  void fill(const dataDrs& cont,
            const std::vector<double>& peakX, 
            const std::vector<double>& baseline);
  int initialize(THttpServer* server=nullptr);
  void reset();

private:
  double m_XUnit;
  double m_YUnit;
  int m_nIns; 
  static int m_ncalled ;
};

#endif
