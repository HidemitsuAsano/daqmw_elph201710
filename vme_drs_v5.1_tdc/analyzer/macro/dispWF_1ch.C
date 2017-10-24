const int n_cell = 1024;

void dispWF_1ch(char* source, int ch)
{
  new TFile(source);
  if(!gFile->IsOpen()){
    std::cerr << "#E : File open error, " << source << std::endl;
    return;
  }

  TTree* tree = (TTree*)gFile->Get("tree");
  std::vector<double> *data = 0;
  TBranch *brptr = 0;
  tree->SetBranchAddress(Form("WaveForm%d", ch), &data, &brptr);
  
  // x axis
  std::vector<double> x(n_cell);
  for(int i = 0; i<n_cell; ++i){
    x[i] = i;
  }

  int n = tree->GetEntries();
  TCanvas *c1 = new TCanvas("c1","c1");
  for(int i = 0; i<n; ++i){
    tree->GetEntry(i);
    
    int n_sample = data->size();
    if(n_sample != 0){
      gPad->SetGridx(true);
      gPad->SetGridy(true);

      TGraph *graph = new TGraph(n_sample, &x[0], &(*data)[0]);
      graph->SetMarkerStyle(7);
      graph->SetMarkerColor(2);
      graph->Draw("apl");
      c1->Modified();
      c1->Update();
      getchar();
    }// if(n_sample)

  }// for(i)

  return;
}// main
