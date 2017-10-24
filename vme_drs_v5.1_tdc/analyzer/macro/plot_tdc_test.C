// -*- C++ -*-

#include <iomanip>

const int kNCh = 16;


//______________________________________________________________________________
void plot_tdc_test()
{
  gErrorIgnoreLevel = kFatal;
  
  //  TTree* tree = dynamic_cast<TTree*>(gFile->Get("tree"));
  TTree* tree = create_chain("tree");
  std::vector<double> sigma_dt(kNCh, -999);
  std::vector<double> sigma_dt2(kNCh, -999);
  std::vector<double> sigma_wt(kNCh, -999);

  TCanvas* c1 = new TCanvas("c1", "time diff");
  c1->SetWindowSize(2400, 1600);
  //  c1->Divide(8, 4);
  c1->Divide(4, 4);
  for (int i=0; i<kNCh; ++i) {
    {
      c1->cd(i+1);
      TString var=Form("dt%d/375e6/1024*1e12>>hdt%d", i, i);
      tree->Draw(var);
      TH1* h = dynamic_cast<TH1*>(gDirectory->Get(Form("hdt%d", i)));
      if (h) {
        h->SetXTitle("t [psec]");
        h->SetYTitle("Counts");
        h->Fit("gaus");
        TF1* f = h->GetFunction("gaus");
        if (f) {
          sigma_dt[i] = f->GetParameter(2);
          h->Draw("");
        }
      }
    }
    // {
    //   c1->cd(i+1+kNCh);
    //   TString var=Form("dt_2nd%d/375e6/1024*1e12>>hdt_2nd%d", i, i);
    //   tree->Draw(var);
    //   TH1* h = dynamic_cast<TH1*>(gDirectory->Get(Form("hdt_2nd%d", i)));
    //   if (h) {
    //     h->Fit("gaus");
    //     TF1* f = h->GetFunction("gaus");
    //     if (f) {
    //       sigma_dt2[i] = f->GetParameter(2);
    //       h->Draw("");
    //     }

    //   }
    //    }
  }
  

  TCanvas* c2 = new TCanvas("c2", "width");
  c2->SetWindowSize(2400, 1600);
  c2->Divide(4, 4);
  for (int i=0; i<kNCh; ++i) {
    c2->cd(i+1);
    TString var=Form("width%d/375e6/1024*1e12>>hwt%d", i, i);
    tree->Draw(var);
    TH1* h = dynamic_cast<TH1*>(gDirectory->Get(Form("hwt%d", i)));
    if (h) {
      h->SetXTitle("width [psec]");
      h->SetYTitle("Counts");
      h->Fit("gaus");
        TF1* f = h->GetFunction("gaus");
        if (f) {
          sigma_wt[i] = f->GetParameter(2);
          h->Draw("");
        }
    }
  }
  
  TCanvas* c0 = new TCanvas("c0", "c0");
  tree->Draw("((l1_tdc1-l1_tdc) & 0xffffff)/375e6/1024*1e12>>hl1");
  TH1* hl1 = dynamic_cast<TH1*>(gDirectory->Get("hl1"));
  hl1->SetXTitle("t [psec]");
  hl1->SetYTitle("Counts");
  hl1->Fit("gaus");
  TF1* fl1 = hl1->GetFunction("gaus");
  double dt_l1 = -999;
  if (fl1) {
    dt_l1 = fl1->GetParameter(2);
  }

  TGraph* gr_sigma_dt = new TGraph(sigma_dt.size());
  TGraph* gr_sigma_wt = new TGraph(sigma_wt.size());
  //  TGraph* gr_sigma_l1 = new TGraph(sigma_dt.size());
  std::cout << "\n#D sigma dt" << std::endl;
  for (int i=0; i<kNCh; ++i) {
    std::cout << std::setw(2) << i << " " << std::setw(10) << sigma_dt[i]/TMath::Sqrt(2) << std::endl;
    gr_sigma_dt->SetPoint(i, i, sigma_dt[i]/TMath::Sqrt(2));
  }
  
  std::cout << "\n#D sigma dt (2)" << std::endl;
  for (int i=0; i<kNCh; ++i) {
    std::cout << std::setw(2) << i << " " << std::setw(10) << sigma_dt2[i]/TMath::Sqrt(2) << std::endl;
  }
  std::cout << "\n#D sigma width" << std::endl;
  for (int i=0; i<kNCh; ++i) {
    std::cout << std::setw(2) << i << " " << std::setw(10) << sigma_wt[i]/TMath::Sqrt(2) << std::endl;
    //gr_sigma_wt->SetPoint(i, i, sigma_wt[i]/TMath::Sqrt(2));
    gr_sigma_wt->SetPoint(i, i, sigma_wt[i]);
  }

  std::cout << "\n#D sigma l1" << std::endl;
  std::cout << std::setw(10) << dt_l1/TMath::Sqrt(2) << std::endl;

  TCanvas* c3 = new TCanvas("c3", "c3");
  TH1* hf0 = new TH1F("hf0", "", kNCh, -0.5, kNCh-0.5);
  hf0->SetStats(false);
  hf0->SetXTitle("channel");
  hf0->SetYTitle("#sigma_{t} [psec]");
  hf0->GetYaxis()->SetRangeUser(0, 30);
  hf0->Draw();
  gr_sigma_dt->SetMarkerStyle(3);
  gr_sigma_dt->Draw("p same");

  TCanvas* c4 = new TCanvas("c4", "c4");
  TH1* hf1 = new TH1F("hf0", "", kNCh, -0.5, kNCh-0.5);
  hf1->SetStats(false);
  hf1->SetXTitle("channel");
  hf1->SetYTitle("#sigma_{width} [psec]");
  hf1->GetYaxis()->SetRangeUser(0, 30);
  hf1->Draw();
  gr_sigma_wt->SetMarkerStyle(20);
  gr_sigma_wt->Draw("p same");

  
}

