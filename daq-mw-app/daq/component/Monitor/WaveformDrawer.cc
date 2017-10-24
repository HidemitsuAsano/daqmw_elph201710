// -*- C++ -*-

#include <iostream>
#include <fstream>
#include <algorithm>

#include <TLine.h>
#include <TH1.h>
#include <TH2.h>
#include <TCanvas.h>
#include <TVirtualPad.h>
#include <TGraph.h>
#include <TGraphErrors.h>
#include <TMultiGraph.h>
#include <THttpServer.h>
#include <TMemFile.h>
#include <TDirectory.h>
#include <TSystem.h>

#include "datadrs.hh"
#include "WaveformDrawer.hh"


//______________________________________________________________________________
TCanvas* cwaveform[Nboards] = {nullptr};
//std::vector<TGraph*> grwf(NofChModule);
std::vector<TMultiGraph*> grwf(NofChModule*Nboards);
int WaveformDrawer::m_ncalled = 0;
//std::vector<TH2*> hwf(NofChModule);

//______________________________________________________________________________
//______________________________________________________________________________
//______________________________________________________________________________
WaveformDrawer::WaveformDrawer()
  : m_XUnit(kTimeUnit),
    m_YUnit(kAmplitudeUnit)
{
  m_nIns = m_ncalled;
  m_ncalled++;
 //std::cout << "wfd called " << m_ncalled << std::endl;
}

//______________________________________________________________________________
WaveformDrawer::~WaveformDrawer()
{
 m_ncalled--;
}

//______________________________________________________________________________
void
WaveformDrawer::fill(const dataDrs& cont,
                     const std::vector<double>& peakX,
                     const std::vector<double>& baseline)
{
  for (int ich=0; ich<NofChModule; ++ich) {
    const std::vector<double>& wf = cont.data_wf[ich];
    int ncell = wf.size();

    int trig_cell = cont.cellnum[ich/NofDrs];
    int cascade_cell = NofCell - trig_cell; // 1st cell of the cascading 


    int iIns= ich + NofChModule*(m_nIns);
    TMultiGraph* mgr = grwf[iIns];
    TGraph* gr = dynamic_cast<TGraph*>(grwf[iIns]->GetListOfGraphs()->At(0));
    gr->Set(ncell);

    double px   = peakX[ich];
    double base = baseline[ich];
    
    std::vector<double> v(ncell);
    for (int icell=0; icell<ncell; ++icell) {
      double x = icell*m_XUnit;
      double y = wf[icell]*m_YUnit - base;
      gr->SetPoint(icell, x, y);
      v[icell] = y;

      // hwf[ich]->Fill(x, y);
    }
    mgr->GetXaxis()->SetLimits(gr->GetX()[0], gr->GetX()[ncell-1]);
    double ymin = *std::min_element(v.begin(), v.end());
    double ymax = *std::max_element(v.begin(), v.end());
    mgr->SetMinimum(ymin-50);
    mgr->SetMaximum(ymax+50);

    TGraph* cascade = dynamic_cast<TGraph*>(grwf[iIns]->GetListOfGraphs()->At(1));
    if (cascade) {
      cascade->SetPoint(0, cascade_cell*m_XUnit, 100);
      cascade->SetPoint(1, cascade_cell*m_XUnit, 50);
    }

    TGraph* peakXpos = dynamic_cast<TGraph*>(grwf[iIns]->GetListOfGraphs()->At(2));
    if (peakXpos) {
      peakXpos->SetPoint(0, px, ymax + 10);
      peakXpos->SetPoint(1, px, ymax + 50);
    }
  }
}

//______________________________________________________________________________
int
WaveformDrawer::initialize(THttpServer* server)
{
  // m_XUnit = 1;
  // m_YUnit = 1;
  
  
  if (!cwaveform[m_nIns]) {
    cwaveform[m_nIns] = new TCanvas(Form("cwaveform%d",m_nIns), "Waveform");
    cwaveform[m_nIns]->Divide(4, 4);
  }
  
  for (int ich=0; ich<NofChModule; ++ich) {
    cwaveform[m_nIns]->cd(ich+1);
    int iInstance = ich + NofChModule*(m_nIns);
    if (!grwf[iInstance]) {
      TGraph* gr =  new TGraph;

      TGraph* cascade = new TGraph(2);
      cascade->SetLineWidth(2);
      cascade->SetLineColor(kRed);

      TGraph* peak = new TGraph(2);
      peak->SetLineWidth(2);
      peak->SetLineColor(kBlue);
      
      // TMultiGraph* mgr = new TMultiGraph(Form("gr%03d", ich),
      //                                    Form("waveform ch %03d;[nsec];[mV]", ich));
      TMultiGraph* mgr = new TMultiGraph(Form("gr%03d", iInstance), Form("waveform ch%d", iInstance));
      mgr->Add(gr);
      mgr->Add(cascade);
      mgr->Add(peak);
      mgr->Draw("al");
      mgr->GetXaxis()->SetTitle("[nsec]");
      mgr->GetYaxis()->SetTitle("[mV]");

      grwf[iInstance] = mgr;
      char regname[256];
      sprintf(regname,"/drs4qdc%d/waveform",m_nIns+1);
      server->Register(regname, mgr);
    }

    // if (!hwf[ich]) {
    //   TH2F* h = new TH2F(Form("hwf%02d", ich), Form("waveform ch %02d", ich),
    //                      1024, 0, 1024*kTimeUnit,
    //                      1200, 0, 1200);
    //   h->SetXTitle("[nsec]");
    //   h->SetYTitle("[mV]");
    //   h->SetDirectory(0);
    //   hwf[ich] = h;
    //   server->Register("/drs4qdc/2d", h);
    // }
    
  }
  return m_nIns;
}

//______________________________________________________________________________
void
WaveformDrawer::reset()
{
  // for (auto&& p : hwf) if (p) p->Reset();
  //  std::cout << " WaveformDrawer::reset()" << std::endl;

}
