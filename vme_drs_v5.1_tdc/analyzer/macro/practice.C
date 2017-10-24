// void practice(TTree* tree){
void practice(){

TTree* tree = dynamic_cast<TTree*>(gFile->Get("tree"));
if (!tree) { 
std::cout << "tree ....." << std::endl;
std::exit(1);
}
std::vector<double> *data = 0;
std::vector<double> max, sum1, m1, dv, p;
tree->SetBranchAddress(Form("WaveForm9"),&data);
int n = tree->GetEntries();
std::vector<double>& dat = *data;
max.resize(n);
sum1.resize(n);
m1.resize(n);
dv.resize(1024);
p.resize(1024);

int range1, range2;
range1 =800;
range2 =1000;
double length=range2-range1;

TH1D *h1 = new TH1D("h1","h1",800,0,2000);
for(int ievent=0; ievent<n; ++ievent){
tree->GetEntry(ievent);
for(int itime=0; itime<1024; ++itime){
dv[itime] = data->at(itime);
sum1[ievent] = 0;
 for( int j=range1; j< range2;j++){
    sum1[ievent] += dv[j];
  }
m1[ievent]  = (sum1[ievent]/length);
p[itime] = dv[itime]-m1[ievent];
//std::cout << sum1[ievent] <<std::endl;
}
max[ievent] = *max_element(p.begin(), p.end());
//std::cout << max[ievent] <<std::endl;
h1->Fill(max[ievent]);
}
TCanvas *c2 = new TCanvas("c2"); 
h1->Draw();
}
