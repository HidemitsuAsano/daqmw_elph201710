// -*- C++ -*-

#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>

#include <THttpServer.h>
#include <TMath.h>
#include <TH1.h>
#include <TCanvas.h>
#include <TVirtualPad.h>
#include <TGraph.h>
#include <TF1.h>
#include <TSystem.h>


#include "datadrs.hh"
#include "WaveformDrawer.hh"
#include "Drs4IntegralAna.hh"


namespace {
  const double kTimeUnit      = WaveformDrawer::kTimeUnit; // nsec
  const double kAmplitudeUnit = WaveformDrawer::kAmplitudeUnit; // mV
  const double kChargeUnit    = kTimeUnit*kAmplitudeUnit/50; // pC
}

//______________________________________________________________________________

namespace QDC {
  const int kDefaultNbins   = 2020;
  const double kDefaultXmin =  -10;
  const double kDefaultXmax = 1000;
}

namespace BaseLine {
  const int kDefaultNbins   = 400;
  const double kDefaultXmin = 400;
  const double kDefaultXmax = 600;
}

namespace PADC {
  const int kDefaultNbins   = 1600;
  const double kDefaultXmin = 0;
  const double kDefaultXmax = 1600;
}

namespace PeakX{
  const int kDefaultNbins   = 1024;
  const double kDefaultXmin = 0;
  const double kDefaultXmax = 1024;
}

//______________________________________________________________________________
TCanvas* cqdc[Nboards]   = {nullptr};
TCanvas* cbaseline[Nboards] = {nullptr};
TCanvas* cpadc[Nboards]     = {nullptr};
TCanvas* cpeakx[Nboards]   = {nullptr};
std::vector<TH1*> hQdc(NofChModule*Nboards);
std::vector<TH1*> hBaseLine(NofChModule*Nboards);
std::vector<TH1*> hPadc(NofChModule*Nboards);
std::vector<TH1*> hPeakX(NofChModule*Nboards);

std::vector<AnaImpl*> implList(NofChModule*Nboards);
//std::vector<AnaImpl*> implList(NofChModule);

//______________________________________________________________________________
//______________________________________________________________________________
void
AnaImpl::CalcBaseLine()
{
  Int_t range1 = 1;
  Int_t range2 = fPeakX - 50;
  
  //  const Int_t n  = fData->size();
  const std::vector<Double_t>& data = *fData;
  Double_t sum=0;
  Int_t    num=0;
  for (Int_t i=range1; i<range2; ++i) {
    if (TMath::Abs(data[i+1] - data[i]) < 10) {
      ++num;
      sum += data[i];
    }
  }
  fBaseLine = sum/num;
  // TGraph gra(n, &(a[0]), &(p0[0]));
  // TF1 f("f", "pol0", 1, range2);
  // gra.Fit("f", "Q", "", 0, range2);
  // fBaseLine = f.GetParameter(0);

  fAmplitude = fPeakY - fBaseLine;
}

//______________________________________________________________________________
void
AnaImpl::Integral()
{
  const std::vector<Double_t>& data = *fData;
  Int_t n = data.size();
  Int_t imin = fPeakX-20;
  if (imin<0) imin = 0;
  Int_t imax = fPeakX+30;
  if (imax>n) imax = n;

  // std::cout << "#D Integral() "
  //            << " peak x = " << fPeakX
  //            << " imin = " << imin
  //            << " imax = " << imax
  //            << std::endl;
  for (Int_t i=imin; i<imax; ++i) {
    fIntegral += data[i] - fBaseLine;
  }
}

//______________________________________________________________________________
void
AnaImpl::Reset()
{
  fPeakX     = -9999;
  fPeakY     = -9999;
  fBaseLine  = 0;
  fAmplitude = 0;
  fIntegral  = 0;
}

//______________________________________________________________________________
void
AnaImpl::SearchPeak()
{
  const std::vector<Double_t>& data = *fData;
  double da;
  for (Int_t i=fIStart; i<fIEnd; ++i) {
    if (fPeakY < data[i]) {
      fPeakY = data[i];
      fPeakX = i;
    }
  }
}



//______________________________________________________________________________
//______________________________________________________________________________
//______________________________________________________________________________
int Drs4IntegralAna::m_ncalled = 0;
Drs4IntegralAna::Drs4IntegralAna()
{
  m_nIns = m_ncalled;
  m_ncalled++;
}

//______________________________________________________________________________
Drs4IntegralAna::~Drs4IntegralAna()
{
}

//______________________________________________________________________________
void
Drs4IntegralAna::fill(const dataDrs& cont)
{
  for (auto&& p : implList) if (p) p->Reset();
  
  for (Int_t i=0; i<NofChModule; ++i) {
    int iIns = i + NofChModule*m_nIns;
    //AnaImpl* impl = implList[iIns];
    AnaImpl* impl = implList[i];
    Int_t ch = (impl->fCh < 0) ? i : impl->fCh;
    //Int_t ch =  i ; 
    impl->fData = &cont.data_wf[ch];
    impl->SearchPeak();
    impl->CalcBaseLine();
    impl->Integral();

    hQdc[iIns]->Fill(impl->fIntegral * kChargeUnit);
    hBaseLine[iIns]->Fill(impl->fBaseLine * kAmplitudeUnit);
    hPadc[iIns]->Fill(impl->fAmplitude    * kAmplitudeUnit);
    hPeakX[iIns]->Fill(impl->fPeakX       * kTimeUnit);
  }
  
}

//______________________________________________________________________________
double
Drs4IntegralAna::getAmplitude(int ch) const
{
  //int iCh = ch + NofChModule*m_nIns;
  //return implList[iCh]->fAmplitude * kAmplitudeUnit;
  return implList[ch]->fAmplitude * kAmplitudeUnit;
}

//______________________________________________________________________________
double
Drs4IntegralAna::getBaseLine(int ch) const
{
  //int iCh = ch + NofChModule*m_nIns;
  //return implList[iCh]->fBaseLine * kAmplitudeUnit;
  return implList[ch]->fBaseLine * kAmplitudeUnit;
}

//______________________________________________________________________________
double
Drs4IntegralAna::getIntegral(int ch) const
{
  //int iCh = ch + NofChModule*m_nIns;
  //return implList[iCh]->fIntegral * kChargeUnit;
  return implList[ch]->fIntegral * kChargeUnit;
}

//______________________________________________________________________________
double
Drs4IntegralAna::getPeakX(int ch) const
{
  //int iCh = ch + NofChModule*m_nIns;
  //return implList[iCh]->fPeakX * kTimeUnit;
  return implList[ch]->fPeakX * kTimeUnit;
}

//______________________________________________________________________________
void
Drs4IntegralAna::initialize(THttpServer* server)
{
  for (Int_t i=0; i<NofChModule; ++i) {
    //int iIns = i + NofChModule*m_nIns;
    //implList[iIns] = new AnaImpl;
    
    if (implList[i]) continue;
    
    implList[i] = new AnaImpl;
  }

  if (!server) return;

  if (!cqdc[m_nIns]) {
    cqdc[m_nIns] = new TCanvas(Form("cqdc%d",m_nIns+1), "QDC");
    cqdc[m_nIns]->Divide(4, 4);
  }
  for (Int_t i=0; i<NofChModule; ++i) {
    int iIns = i + NofChModule*m_nIns;
    if (hQdc[iIns]) continue;
    TString hname  = Form("hqdc%03d", iIns);
    TString htitle = Form("QDC ch %03d", iIns);
    int nbin    = QDC::kDefaultNbins;
    double xmin = QDC::kDefaultXmin;
    double xmax = QDC::kDefaultXmax;
    
    hQdc[iIns] = new TH1F(hname, htitle, nbin, xmin, xmax);
    hQdc[iIns]->SetXTitle("Charge [pC]");
    hQdc[iIns]->SetYTitle("[Counts]");
    cqdc[m_nIns]->cd(i+1);
    gPad->SetLogy();
    hQdc[iIns]->Draw("hist");

    hQdc[iIns]->SetDirectory(0);
    char sname[256];
    sprintf(sname,"/drs4qdc%d/qdc",m_nIns+1);
    server->Register(sname, hQdc[iIns]);
  }

  if (!cbaseline[m_nIns]) {
    cbaseline[m_nIns] = new TCanvas(Form("cbaseline%d",m_nIns), "Base line");
    cbaseline[m_nIns]->Divide(4, 4);
  }

  for (Int_t i=0; i<NofChModule; ++i) {
    int iIns = i + NofChModule*m_nIns;
    if (hBaseLine[iIns]) continue;
    TString hname  = Form("hbl%03d", iIns);
    TString htitle = Form("Base line ch %03d", iIns);
    int nbin    = BaseLine::kDefaultNbins;
    double xmin = BaseLine::kDefaultXmin;
    double xmax = BaseLine::kDefaultXmax;
    
    
    hBaseLine[iIns] = new TH1F(hname, htitle, nbin, xmin, xmax);
    hBaseLine[iIns]->SetXTitle("[mV]");
    hBaseLine[iIns]->SetYTitle("[Counts]");
    cbaseline[m_nIns]->cd(i+1);
    gPad->SetLogy();
    hBaseLine[m_nIns]->Draw("hist");

    hBaseLine[iIns]->SetDirectory(0);
    char sname[256];
    sprintf(sname,"/drs4qdc%d/baseline",m_nIns+1);
    server->Register(sname, hBaseLine[iIns]);
  }

  if (!cpadc[m_nIns]) {
    cpadc[m_nIns] = new TCanvas(Form("cpadc%d",m_nIns), "PADC");
    cpadc[m_nIns]->Divide(4, 4);
  }
  for (Int_t i=0; i<NofChModule; ++i) {
    int iIns = i + NofChModule*m_nIns;
    if (hPadc[iIns]) continue;
    TString hname  = Form("hpadc%03d", iIns);
    TString htitle = Form("PADC ch %03d", iIns);
    int nbin    = PADC::kDefaultNbins;
    double xmin = PADC::kDefaultXmin;
    double xmax = PADC::kDefaultXmax;
    
    hPadc[iIns] = new TH1F(hname, htitle, nbin, xmin, xmax);
    hPadc[iIns]->SetXTitle("Amplitude [mV]");
    hPadc[iIns]->SetYTitle("[Counts]");
    cpadc[m_nIns]->cd(i+1);
    gPad->SetLogy();
    hPadc[iIns]->Draw("hist");

    hPadc[iIns]->SetDirectory(0);
    char sname[256];
    sprintf(sname,"/drs4qdc%d/padc",m_nIns+1);
    server->Register(sname, hPadc[iIns]);
  }

  if (!cpeakx[m_nIns]) {
    cpeakx[m_nIns] = new TCanvas(Form("cpeakx%d",m_nIns), "peak X");
    cpeakx[m_nIns]->Divide(4, 4);
  }
  for (Int_t i=0; i<NofChModule; ++i) {
    int iIns = i + NofChModule*m_nIns;
    if (hPeakX[iIns]) continue;
    TString hname = Form("hpeakx%03d", iIns);
    TString htitle = Form("peak X ch %03d", iIns);
    int nbin    = PeakX::kDefaultNbins;
    double xmin = PeakX::kDefaultXmin;
    double xmax = PeakX::kDefaultXmax;

    hPeakX[iIns] = new TH1F(hname, htitle, nbin, xmin, xmax);
    hPeakX[iIns]->SetXTitle("[nsec]");
    hPeakX[iIns]->SetYTitle("[Counts]");
    
    cpeakx[m_nIns]->cd(i+1);
    gPad->SetLogy();
    hPeakX[iIns]->Draw("hist");
    hPeakX[iIns]->SetDirectory(0);
    char sname[256];
    sprintf(sname,"/drs4qdc%d/peak_x",m_nIns+1);
    server->Register(sname, hPeakX[iIns]) ;
  }
  
}

//______________________________________________________________________________
void
Drs4IntegralAna::reset()
{
  for (auto&& p: hQdc)      if (p) p->Reset();
  for (auto&& p: hBaseLine) if (p) p->Reset();
  for (auto&& p: hPadc)     if (p) p->Reset();
  for (auto&& p: hPeakX)    if (p) p->Reset();
}

