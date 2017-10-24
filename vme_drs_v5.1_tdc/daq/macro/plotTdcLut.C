// -*- C++ -*-

#include <iomanip>
#include <sstream>
#include <TString.h>


const double kUnit = 1./375e6/1024*1e12;

const int kNCh   = 34;

//const int kNLine = 25;
const int kNLine = 50;
//const int kNBin  = 200;
//const int kNBin  = 400;
const int kNBin  = 512;
const double kXmin  = 0;
const double kXmax  = kNBin;

TH1* h[kNCh];
TH1* h1[kNCh];
TH1* hy[kNCh];
TH1* hyall;

std::vector<int> ifill(kNCh);
//______________________________________________________________________________
const TString
GetString(const TString src,
          int idx)
{
  TObjArray* objs = src.Tokenize(" ");
  int nobj = objs->GetEntries();
  TObjString* ostr = dynamic_cast<TObjString*>(objs->At(idx));
  return ostr->GetString();
}


//______________________________________________________________________________
void
decode(const TString src,
       int ch)
{
  const TString soff = GetString(src, 0);
  int offset = soff.Atoi();
  // std::cout << "#D decode() "
  //           << " ch = " << std::setw(4) << ch 
  //           << " offset = "  << offset << " src = " << src << std::endl;
  for (int idx=0; idx<8; ++idx) {
    const TString s = GetString(src, idx+2);
    int val = s.Atoi();
    if (!h[ch]) return;
    int bin = idx + offset;
    // std::cout << "#D s = " << s << " val = " << val << std::endl;
    // std::cout << "#D idx = " << std::setw(2) << idx
    //           << " h = " << std::setw(4) << h[ch]->GetName()
    //           << " : " << std::setw(4) << ifill[ch]
    //           << " fill "
    //           << std::setw(5) << bin << " "
    //           << std::setw(5) << val << " " 
    //           << h[ch]->GetXaxis()->GetBinCenter(bin+1)  << " " 
    //           << h[ch]->GetXaxis()->GetBinLowEdge(bin+1)
    //           << std::endl;
    //    h[ch]->SetBinContent(bin+1, val);
    if (val>0) {
      h[ch]->Fill(bin, val);
      int y = val - h[ch]->GetBinContent(bin);
      //    h1[ch]->SetBinContent(bin+1, y);
      //      if (h[ch]->Integral()>0 && h[ch]->GetBinContent(bin)<1023) {
        h1[ch]->Fill(bin, y);
        hy[ch]->Fill(y);
        hyall->Fill(y);
        //      }
    }

    ++ifill[ch];
  }
}

//______________________________________________________________________________
void
plotTdcLut(const TString& inFileName)
           //           const TString& outFileName)
{
  std::ifstream inFile(inFileName.Data());
  if (inFile.fail()) {
    std::cout << "#E failed to open file " << inFileName << std::endl;
    return;
  }
  TString line;

  unsigned int en_ch = 0xffff;
  unsigned int ch = 0xffff;

  for (int i=0; i<(kNCh); ++i) {
    h[i]  = new TH1D(Form("h0%03d", i), "", kNBin, kXmin, kXmax);
    h1[i] = new TH1D(Form("h1%03d", i), "", kNBin, kXmin, kXmax);
    hy[i] = new TH1D(Form("hy%03d", i), "", 100,   0, 100);

    h[i]->SetXTitle("virtual bin");
    h1[i]->SetXTitle("virtual bin");
    
    h[i]->SetStats(false);
    h1[i]->SetStats(false);
    hy[i]->SetStats(false);
  }
  hyall = new TH1D("hy", "", 100, 0, 100);

  // h[1]->SetLineColor(kRed);
  // h[2]->SetLineColor(kBlue);
  // h[3]->SetLineColor(kGreen+1);
  // h[4]->SetLineColor(kMagenta+1);

  std::vector<int> offsetList(kNLine);
  for (int i=0; i<kNLine; ++i) {
    offsetList[i] = 8*i;
    std::cout << "#D offse list i = " << offsetList[i] << std::endl;
  }
  
  while (inFile) {
//    std::cout << "__________________________________________________" << std::endl;
    line.ReadLine(inFile);
    if (line.Length()==0) continue;
    //    std::cout << "#D line = " << line << std::endl;
    if (line.Contains("enable")) {
      const TString s = GetString(line, 4);
      std::istringstream iss(s.Data());
      iss >> en_ch;
      //      std::cout << "#D enable channel = " << std::hex << en_ch << std::dec << std::endl;
      
    } else if (line.Contains("Peek")) {
      const TString s = GetString(line, 4);
      ch = s.Atoi();
    } else if (line.Contains("StartCalibration")) {
      continue;
    } else if (line.First("=")==0) {
      continue;
    } else if (line.First(".")==0) {
      continue;
    } else {
      decode(line, ch);
    }
  }

  TCanvas* c1 = new TCanvas("c1", "c1");
  TCanvas* c01 = new TCanvas("c01", "c01");
  c01->SetWindowSize(1200, 1000);
  c01->Divide(8, 5);
  for (int i=0; i<(kNCh); ++i) {
    c1->cd();
    h[i]->Scale(kUnit);
    h[i]->SetYTitle("[psec]");
    h[i]->GetXaxis()->SetRangeUser(0, 320);
    if (i==0) {
      h[i]->Draw("HIST");
    } else   h[i]->Draw("HIST same");
    h[i]->Fit("pol1");

    c01->cd(i+1);
    if (i==0) {
      h[i]->Draw("HIST");
    } else   h[i]->Draw("HIST same");
  }

  TCanvas* c2 = new TCanvas("c2", "c2");
  TCanvas* c02 = new TCanvas("c02", "c02");
  c02->SetWindowSize(1600, 1000);
  c02->Divide(8, 5);
  for (int i=0; i<(kNCh); ++i) {
    c2->cd();
    h1[i]->Scale(kUnit);
    h1[i]->SetYTitle("[psec]");
    h1[i]->GetXaxis()->SetRangeUser(0, 320);
    if (i==0) {
      h1[i]->Draw("HIST");
      //      h1[i]->GetYaxis()->SetRangeUser(0, 20);
    } else   h1[i]->Draw("HIST same");

    c02->cd(i+1);
    if (i==0) {
      h1[i]->Draw("HIST");
    } else   h1[i]->Draw("HIST same");
    
  }

  TCanvas* c3 = new TCanvas("c3", "c3");
  TCanvas* c03 = new TCanvas("c03", "c03");
  c03->SetWindowSize(1600, 1000);
  c03->Divide(8, 5);
  for (int i=0; i<(kNCh); ++i) {
    c3->cd();
    if (i==0) {
      hy[i]->Draw("HIST");
      //      hy[i]->GetYaxis()->SetRangeUser(0,100);
    } else   hy[i]->Draw("HIST same");

    c03->cd(i+1);
    if (i==0) {
      hy[i]->Draw("HIST");
    } else   hy[i]->Draw("HIST same");
  }

  TCanvas* c4 = new TCanvas("c4", "c4");
  hyall->Draw("HIST");
  
  
}
