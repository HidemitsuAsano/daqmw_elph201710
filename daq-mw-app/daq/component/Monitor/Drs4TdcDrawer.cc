// -*- C++ -*-

#include <iostream>

#include <TCanvas.h>
#include <TVirtualPad.h>
#include <TH1.h>
#include <THttpServer.h>

#include "datadrs.hh"
#include "Drs4TdcDrawer.hh"

//______________________________________________________________________________
const double kTimeUnit = 1.0/375e6/1024 * 1e9; // nsec
const unsigned int kDefaultNbins = 150000;
const double kDefaultXmin = -0.5e3;
const double kDefaultXmax = 1.0e3; // nsec

//______________________________________________________________________________
TCanvas* cTdc[Nboards] = {nullptr};
TCanvas* cTdcWidth[Nboards] = {nullptr};
std::vector<TH1*> hTdc(NofChModule*Nboards);
std::vector<TH1*> hTdcTrailing(NofChModule*Nboards);
std::vector<TH1*> hTdcWidth(NofChModule*Nboards);


//______________________________________________________________________________

int Drs4TdcDrawer::m_ncalled = 0;
Drs4TdcDrawer::Drs4TdcDrawer()
  : m_t0_ch(0xffff)
{
  m_nIns = m_ncalled;
  m_ncalled++;
}

//______________________________________________________________________________
Drs4TdcDrawer::~Drs4TdcDrawer()
{
}

//______________________________________________________________________________
void
Drs4TdcDrawer::fill(const dataDrs& cont)
{
  unsigned int t0_ch = (m_t0_ch == 0xffff) ? NofChModule : m_t0_ch;
  double t0 = 0;
  //  std::cout << "#D t0_ch = " << t0_ch << std::endl;
  if (t0_ch < NofChModule) {
    const std::vector<unsigned int>& tdc_t0 = cont.data_tdc[t0_ch];
    if (tdc_t0.empty()) return;
    t0 = tdc_t0[0];
  } else t0 = cont.l1_tdc;
  // std::cout << "#D t0_ch = " << t0_ch
  //           << ", t0 = " << t0
  //           << std::endl;
  
  for (int ich=0; ich<NofChModule; ++ich) {
    int iIns = ich + NofChModule*m_nIns;
    TH1* htdc      = hTdc[iIns];
    TH1* htrailing = hTdcTrailing[iIns];
    TH1* hwidth    = hTdcWidth[iIns];
    
    if (!htdc || !htrailing || !hwidth) continue;

    // std::cout << "#D ich = " << ich << std::endl;
    const std::vector<unsigned int>& tdc          = (ich!=t0_ch) ? cont.data_tdc[ich] : cont.data_dt[ich];
    const std::vector<unsigned int>& tdc_trailing = (ich!=t0_ch) ? cont.data_tdc_2nd[ich] : cont.data_dt_2nd[ich];
    const std::vector<unsigned int>& width        = cont.data_width[ich];
    int nhit = width.size();
    // std::cout << " Drs4TdcDrawer::fil() ich = " << ich << " nhit = " <<  nhit << std::endl;
    for (int ihit=0; ihit<nhit; ++ihit) {
      // std::cout << "#D tdc = " << tdc[ihit] << " (" << (tdc[ihit]-t0)  << ")"
      //           << ", te = " << tdc_trailing[ihit] << " (" << (tdc_trailing[ihit]-t0) << ")"
      //           << std::endl;
      double le = (ich!=t0_ch) ? (tdc[ihit] - t0) : tdc[ihit];
      le *= kTimeUnit;
      double te = (ich!=t0_ch) ? (tdc_trailing[ihit] - t0) : tdc_trailing[ihit];
      te *= kTimeUnit;
      double w  = width[ihit] * kTimeUnit;

      // std::cout << " ihit = " << ihit
      //           << " " << le
      //           << " " << te
      //           << " " << w
      //           << std::endl;

      htdc->Fill(le); 
      htrailing->Fill(te);
      hwidth->Fill(w);
    }
    
    htdc->Sumw2(kFALSE);
    htrailing->Sumw2(kFALSE);
    hwidth->Sumw2(kFALSE);
  }
  
}

//______________________________________________________________________________
void
Drs4TdcDrawer::initialize(THttpServer* server)
{
  if (!cTdc[m_nIns]) {
    cTdc[m_nIns] = new TCanvas( Form("cTdc%d",m_nIns), "tdc");
    cTdc[m_nIns]->Divide(4, 4);
  }
  // if (!cTdcTrail) {
  //   cTdcTrail = new TCanvas("cTdcTrail", "tdc trailing edge");
  //   cTdcTrail->Divide(4, 4);
  // }

  // leading edge
  for (int ich=0; ich<NofChModule; ++ich) {
    int iIns = ich + NofChModule*m_nIns;
    if (hTdc[iIns]) continue;
    
    TH1* h = new TH1F(Form("htdc%03d", iIns), Form("TDC ch %02d", iIns),
                      kDefaultNbins, kDefaultXmin, kDefaultXmax);
    h->SetDirectory(0);
    h->SetXTitle("TDC [ns]");
    h->SetLineColor(kBlack);
    cTdc[m_nIns]->cd(ich+1); h->Draw("hist");
    gPad->SetLogy();
    hTdc[iIns] = h;
    char sname[256];
    sprintf(sname,"/drs4qdc%d/tdc",m_nIns+1);
    server->Register(sname, h);
  }

  // trailing edge
  for (int ich=0; ich<NofChModule; ++ich) {
    int iIns = ich + NofChModule*m_nIns;
    if (hTdcTrailing[iIns]) continue;
    
    TH1* ht = new TH1F(Form("htdc%03d", iIns+100), Form("TDC trailing edge ch %02d", iIns),
                      kDefaultNbins, kDefaultXmin, kDefaultXmax);
    ht->SetDirectory(0);
    ht->SetXTitle("TDC trailing edge [ns]");
    ht->SetLineColor(kBlue+1);
    cTdc[m_nIns]->cd(ich+1); ht->Draw("hist same");
    gPad->SetLogy();
    hTdcTrailing[iIns] = ht;
    char sname[256];
    sprintf(sname,"/drs4qdc%d/tdc_trailing",m_nIns+1);
    server->Register(sname, ht);
  }

  // width
  if (!cTdcWidth[m_nIns]) {
    cTdcWidth[m_nIns] = new TCanvas(Form("cTdcWidth%d",m_nIns), "tdc width");
    cTdcWidth[m_nIns]->Divide(4, 4);
  }

  for (int ich=0; ich<NofChModule; ++ich) {
    int iIns = ich + NofChModule*m_nIns;
    if (hTdcWidth[iIns]) continue;
    TH1* hw = new TH1F(Form("htdc%03d", iIns+200), Form("TDC width ch %02d", iIns),
                       kDefaultNbins/2, kDefaultXmin/2, kDefaultXmax/2);
    hw->SetDirectory(0);
    hw->SetXTitle("TDC width [ns]");
    cTdcWidth[m_nIns]->cd(ich+1); hw->Draw("hist");
    gPad->SetLogy();
    hTdcWidth[iIns] = hw;
    char sname[256];
    sprintf(sname,"/drs4qdc%d/tdc_width",m_nIns+1);
    server->Register(sname, hw);
  }
  
}

//______________________________________________________________________________
void
Drs4TdcDrawer::reset()
{
  for (auto&& p : hTdc)         if (p) p->Reset();
  for (auto&& p : hTdcTrailing) if (p) p->Reset();
  for (auto&& p : hTdcWidth)    if (p) p->Reset();
}

