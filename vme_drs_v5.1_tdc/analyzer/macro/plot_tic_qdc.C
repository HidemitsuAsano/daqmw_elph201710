int n_cell = 100;

void plot_tic_qdc(char* source)
{
  new TFile(source);
  if(!gFile->IsOpen()){
    std::cerr << "#E : File open error, " << source << std::endl;
    return;
  }

  TTree* tree = (TTree*)gFile->Get("tree");
  int TIC;
  int OF;
  int CELLN[4];
  int QDC[16];
  tree->SetBranchAddress("TIC", &TIC);
  tree->SetBranchAddress("OF",  &OF);
  tree->SetBranchAddress("CELLN",CELLN);
  tree->SetBranchAddress("QDC",  QDC);

  std::vector<double> *data = 0;
  TBranch *brptr = 0;
  tree->SetBranchAddress("WaveForm0", &data, &brptr);
  
  std::vector<int> vtic(1024);
  std::vector<int> this_read(1024);
  // reset
  for(int i = 0; i<1024; ++i){
    vtic[i]      = -1;
    this_read[i] = 0;
  }

  HB2(0, "nanika", 130100,0,130100, 500,0,500);

  int n = tree->GetEntries();
  TCanvas *c1 = new TCanvas("c1","c1");
  for(int i = 0; i<n; ++i){
    tree->GetEntry(i);
    
    // check over flow
    int tmp_tic = 0;
    if(OF == 1){
      tmp_tic = -1;
    }else{
      tmp_tic = TIC;
    }// if(OF)

    // init this_read
    for(int cell = 0; cell<1024; ++cell){
      this_read[cell] = 0;
    }

    // check tic for all
    bool flag_all  = false;
    int  first_tic = 0;
    int  count_ok  = 0;

    double integ = 0;
    n_cell = (*data).size();
    for(int cell = 0; cell<n_cell; ++cell){
      int act_cell = CELLN[0]+cell;
      if(act_cell > 1023) act_cell -= 1024;
      this_read[act_cell] = 1;
      int ptic = vtic[act_cell];
      if(ptic == -1) ptic = 130001;
      int time = ptic + TIC;
      if(time > 130000) time = 130099;
      //      if(time ==0)  std::cout << TIC << std::endl;
      HF2(0, time, (*data)[cell]);
    }// for(cell)

    //    std::cout << "Event " <<first_tic << std::endl;

    for(int cell = 0; cell<1024; ++cell){
      if(this_read[cell] == 1){
	vtic[cell] = 0;
      }else{
	if(tmp_tic == -1){
	  vtic[cell] = tmp_tic;
	}else{
	  if(vtic[cell] != -1){
	    vtic[cell] += tmp_tic;
	    if(vtic[cell] > 130000) vtic[cell] = -1;
	  }
	}//if
      }//if
      //      std::cout << vtic[cell] << " ";
    }// for(cell)
    //    std::cout << std::endl;
  }// for(i)

  HG2(0)->Draw("colz");
  gPad->SetLogx();

  return;
}// main
