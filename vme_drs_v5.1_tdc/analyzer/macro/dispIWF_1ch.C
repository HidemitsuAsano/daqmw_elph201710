const int n_cell = 1024;

void dispIWF_1ch(char* source, int ch)
{
  gROOT->SetStyle("MyStyle");

  new TFile(source);
  if(!gFile->IsOpen()){
    std::cerr << "#E : File open error, " << source << std::endl;
    return;
  }

  TTree* tree = (TTree*)gFile->Get("tree");
  std::vector<double> *data = 0;
  TBranch *brptr = 0;
  tree->SetBranchAddress(Form("WaveForm%d", ch), &data, &brptr);
  
  SetTitle(HB2(0, "hoge", 1024, 0, 1024, 4000, 0, 4000),
	   "Sampling time [ns]",
	   "ADC ch"
	   );

  int n = tree->GetEntries();
  TCanvas *c1 = new TCanvas("c1","c1");
  for(int i = 0; i<n; ++i){
    tree->GetEntry(i);
    
    int n_sample = data->size();
    for(int i_read = 0; i_read<n_sample; ++i_read){
      HF2(0, i_read, (*data)[i_read]);
    }// for(i_read)
  }// for(i)


  const int n_word = 1024;
  TH2F* hist = HG2(0);
  hist->Draw("colz");

  std::ofstream ofs("baseline_cor1.txt");
  TF1 *fit = new TF1("fit","gaus",270,300);
  int mean[n_word];
  double y[n_word];
  double x[n_word];
  int min_value = 9999;
  for(int i = 0; i<n_word; ++i){
    x[i] = i + 0.5;
    TH1D* tmp = hist->ProjectionY(Form("py%d",i),i+1,i+1);
    tmp->Fit("fit","RQNO");
    double tmp_mean = fit->GetParameter(1);
    double point    = tmp_mean - (int)tmp_mean;
    int extra_bit   = 0;
    if(0.25 <= point && point < 0.5){
      extra_bit = 1;
    }else if(0.5 <= point && point < 0.75){
      extra_bit = 2;
    }else if(0.75 <= point){
      extra_bit = 3;      
    }

    mean[i] = (int)tmp_mean;
    mean[i] = (mean[i] << 2) + extra_bit;
    y[i]    = (int)tmp_mean + extra_bit*0.25;
    if(mean[i] < min_value) min_value = mean[i]; 
  }

  TGraph *graph = new TGraph(n_word, x, y);
  graph->SetMarkerColor(4);
  graph->Draw("p");

  for(int i = 0; i<n_word; ++i){
    ofs << (int)mean[i]-min_value << std::endl;
  }
  ofs.close();
  
  return;
}// main
