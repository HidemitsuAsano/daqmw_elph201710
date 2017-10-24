const int n_ch   = 16;
const int n_cell = 1024;

void dispWF_all(char* source)
{
  new TFile(source);
  if(!gFile->IsOpen()){
    std::cerr << "#E : File open error, " << source << std::endl;
    return;
  }

  TTree* tree = (TTree*)gFile->Get("tree");
  std::vector<double> *data[n_ch] = {0};
  TBranch *brptr[n_ch] = {0};
  for(int i = 0; i<n_ch; ++i){
    tree->SetBranchAddress(Form("WaveForm%d", i), &data[i], &brptr[i]);
  }
  
  // x axis
  std::vector<double> x(n_cell);
  for(int i = 0; i<n_cell; ++i){
    x[i] = i;
  }

  int n = tree->GetEntries();
  TCanvas *c1 = new TCanvas("c1","c1");
  c1->Divide(4,4);
  for(int i = 0; i<n; ++i){
    tree->GetEntry(i);
    
    for(int ch = 0; ch<n_ch; ++ch){
      c1->cd(ch+1);

      int n_sample = data[ch]->size();
      std::cout << n_sample << std::endl;
      if(n_sample != 0){
	TGraph *graph = new TGraph(n_sample, &x[0], &(*data[ch])[0]);
	graph->SetMarkerStyle(7);
	graph->SetMarkerColor(2);
	graph->Draw("apl");
      }// if(n_sample)
    }// for(ch)

    c1->Modified();
    c1->Update();
    getchar();

  }// for(i)

  return;
}// main
