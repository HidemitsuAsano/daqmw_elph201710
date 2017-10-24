void fit_ticqdc1()
{
  gROOT->SetStyle("MyStyle");
  new TFile("rootfile/analysis.root");
  TH2F *hist = (TH2F*)gFile->Get("hist0");

  TF1 *fgaus = new TF1("fgaus","gaus");

  std::vector<double> xaxis;
  std::vector<double> yaxis;
  for(int i = 0; i<3; ++i){
    double multi = pow(128.,i);
    for(int j = 1; j<128; ++j){
      double xmin = j*multi;
      double xmax = (j+1)*multi;
      double xmean = (xmin+xmax)/2.;
      xaxis.push_back(xmean);

      int bin_min = hist->GetXaxis()->FindBin(xmin);
      int bin_max = hist->GetXaxis()->FindBin(xmax);
      
      TH1D *tmp = hist->ProjectionY(Form("py_%d_%d", i, j),bin_min, bin_max);
      tmp->Fit("fgaus","QNO");
      double yvalue = fgaus->GetParameter(1);
      //      double yvalue = tmp->GetBinCenter(tmp->GetMaximumBin());
      yaxis.push_back(yvalue);

    }// for(j)
  }// for(i)

  TGraph *graph = new TGraph(xaxis.size(), &xaxis[0], &yaxis[0]);
  hist->Draw("colz");
  graph->SetMarkerColor(2);
  graph->Draw("p");
  gPad->SetLogx();

  TF1 *fit = new TF1("fit", "[4]*log(x)*log(x)*log(x)*log(x) + [3]*log(x)*log(x)*log(x) + [2]*log(x)*log(x) + [1]*log(x) + [0]");
  //  TF1 *fit = new TF1("fit", "[0]*pow(x,[1]) + [2] + [3]*pow(x,[4])");
  fit->SetParameter(0, 100);
  fit->SetParameter(1, -0.5);
  fit->SetParameter(2, 295);
  fit->SetLineWidth(1);
  fit->SetLineColor(1);
  graph->Fit("fit","","",2,100000);
  gPad->SetLogx();
}
