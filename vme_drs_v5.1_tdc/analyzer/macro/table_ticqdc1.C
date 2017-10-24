static const double pa = 0.475703;
static const double pb = -10.8766;
static const double pc = 355.101;

void table_ticqdc1()
{
  gROOT->SetStyle("MyStyle");
  new TFile("rootfile/analysis.root");
  TH2F *hist = (TH2F*)gFile->Get("hist0");
  
  std::vector<double> xaxis;
  std::vector<double> yaxis;

  TF1 *fit = new TF1("fgaus","gaus");
  for(int i = 0; i<3; ++i){
    double multi = pow(128.,i);
    for(int j = 1; j<128; ++j){
      double xmin = j*multi;
      double xmax = (j+1)*multi;
      double xmean = (xmin+xmax)/2.;
      xaxis.push_back(xmean);

      int bin_min = hist->GetXaxis()->FindBin(xmin);
      int bin_max = hist->GetXaxis()->FindBin(xmax);
      //      if(i ==0)      std::cout << bin_min << " " << bin_max << std::endl;
      if(i==0) bin_max = bin_min;
      TH1F *tmp = (TH1F*)hist->ProjectionY(Form("py_%d_%d", i, j),bin_min, bin_max);
      double max_x = tmp->GetBinCenter(tmp->GetMaximumBin());
      tmp->Fit("fgaus","QNO","", max_x-10, max_x+10);
      double yval = fit->GetParameter(1);
      
      //      yaxis.push_back(tmp->GetBinCenter(tmp->GetMaximumBin()));
      if(i==0) std::cout << yval << std::endl;
      yaxis.push_back(yval);
      //      double ymean = pa*log(xmean)*log(xmean) + pb*log(xmean) + pc;
      //      ymean += 0.5;
      //      yaxis.push_back((int)ymean);
    }// for(j)
  }// for(i)

  TGraph *graph = new TGraph(xaxis.size(), &xaxis[0], &yaxis[0]);
  hist->Draw("colz");
  graph->SetMarkerColor(2);
  graph->Draw("p");
  gPad->SetLogx();

  TF1 *flog = new TF1("flog","[0] + [1]*log(x) + [2]*pow(2,log(x)) + [3]*pow(3,log(x)) + [4]*pow(4, log(x))");
  graph->Fit("flog","","",1, 100000);

  // std::ofstream ofs("baseline_cor2.txt");
  // for(int i = 0; i<512; ++i){
  //   double diff = 0;
  //   if(i < 128) diff = 200;
  //   if(128 <= i && i < 256) diff = 100;
  //   if(i < 256){
  //     //      diff = yaxis[i]-yaxis[256];
  //   }

  //   int    lut_val = (int)(diff+0.5);
  //   ofs << lut_val << std::endl;
  // }// for(i)
  // ofs.close();

}// main
