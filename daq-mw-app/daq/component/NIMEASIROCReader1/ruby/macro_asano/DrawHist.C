void DrawHist(const char *filename="../data/1.root")
{
  gStyle->SetOptStat("e");
  const int maxhist = 32;//maxhist
  TH1F *hadcmean_low = new TH1F("hadcmean_low","ADC LOW mean",maxhist,0,maxhist);
  TH1F *hadcmean_high = new TH1F("hadcmean_high","ADC HIGH mean",maxhist,0,maxhist);
  TH1F *hadcstddev_low = new TH1F("hadcstddev_low","ADC LOW std.dev",maxhist,0,maxhist);
  TH1F *hadcstddev_high = new TH1F("hadcstddev_high","ADC HIGH std.dev",maxhist,0,maxhist);
 
  TFile *fin = new TFile(filename);
  TCanvas *cadclow = new TCanvas("cadclow","ADC low gain");
  TH1I *hadclow;
  cadclow->Divide(8,4);
  for(int ich = 0; ich < maxhist; ich++){
    cadclow->cd(ich+1);
    char histname[256];
    sprintf(histname,"ADC_LOW_%d",ich);
    hadclow = (TH1I*)fin->Get(histname);
    hadclow->GetXaxis()->SetRangeUser(750,850);
    hadclow->Draw();
    double mean_low = hadclow->GetMean();
    hadcmean_low->Fill(ich,mean_low);
    double stddev_low = hadclow->GetRMS();
    hadcstddev_low->Fill(ich,stddev_low);
  }

  TCanvas *cadchigh = new TCanvas("cadchigh","ADC high gain");
  TH1I *hadchigh;
  cadchigh->Divide(8,4);
  for(int ich = 0; ich < maxhist; ich++){
    cadchigh->cd(ich+1);
    char histname[256];
    sprintf(histname,"ADC_HIGH_%d",ich);
    hadchigh = (TH1I*)fin->Get(histname);
    hadchigh->GetXaxis()->SetRangeUser(750,1200);
    gPad->SetLogy();
    hadchigh->Draw();
    double mean_high = hadchigh->GetMean();
    hadcmean_high->Fill(ich,mean_high);
    double stddev_high = hadchigh->GetRMS();
    hadcstddev_high->Fill(ich,stddev_high);
  }
 
  TCanvas *cadcmean = new TCanvas("cadcmean","ADC mean");
  cadcmean->cd();
  hadcmean_low->SetLineColor(2);
  hadcmean_low->SetTitle("ADC Mean");
  hadcmean_low->SetXTitle("ch#");
  hadcmean_low->SetYTitle("ADC Mean (ch)");
  hadcmean_low->SetMinimum(760);
  hadcmean_low->Draw();
 
  hadcmean_high->SetLineColor(4);
  hadcmean_high->Draw("same");

  TCanvas *cadcstddev = new TCanvas("cadcstddev","ADC std.dev");
  cadcstddev->cd();
  hadcstddev_high->SetLineColor(4);
  hadcstddev_high->SetMinimum(0);
  hadcstddev_high->SetTitle("ADC Std.Dev.");
  hadcstddev_high->SetXTitle("ch#");
  hadcstddev_high->SetYTitle("ADC Std.Dev. (ch)");
  hadcstddev_high->Draw("");
  hadcstddev_low->SetLineColor(2);
  hadcstddev_low->Draw("same");
 
}
