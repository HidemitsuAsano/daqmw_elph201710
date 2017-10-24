// -*- C++ -*-
//
// author . T.N. Takahashi
// modified by H.Asano
#ifndef Drs4IntegralAna_hh
#define Drs4IntegralAna_hh

#include <string>

class THttpServer;

//______________________________________________________________________________
struct AnaImpl
{
  AnaImpl(const std::string& name="",
          const std::string& title="", 
          Int_t ch = -1, 
          Int_t istart = 200,
          Int_t iend   = 450,
          Int_t threshold = 0)
    : fName(name),
      fTitle(title),
      fData(nullptr),
      fCh(ch), 
      fIStart(istart),
      fIEnd(iend),
      fThreshold(threshold),
      fPeakX(0),
      fPeakY(0),
      fBaseLine(0),
      fAmplitude(0),
      fIntegral(0)
  {}
  ~AnaImpl()
  {}

  std::string fName;
  std::string fTitle;
  const std::vector<Double_t>* fData;
  Int_t fCh;
  Int_t fIStart;
  Int_t fIEnd;
  Int_t fThreshold;
  Int_t fPeakX;
  Double_t fPeakY;
  Double_t fBaseLine;
  Double_t fAmplitude;
  Double_t fIntegral;

  void CalcBaseLine();
  void Integral();
  void Reset();
  void SearchPeak();
      
};

//______________________________________________________________________________
class Drs4IntegralAna
{
public:
  Drs4IntegralAna();
  ~Drs4IntegralAna();

  void fill(const dataDrs& cont);
  double getAmplitude(int ch) const;
  double getBaseLine(int ch) const;
  double getIntegral(int ch) const;
  double getPeakX(int ch) const;
  void initialize(THttpServer* server);
  void parse(const std::string& fileName="");
  void reset();
private:
  static int m_ncalled;
  int m_nIns;

};

#endif
