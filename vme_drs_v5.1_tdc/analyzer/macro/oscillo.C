#include<fstream>
void oscillo(){
double x;
TH1D *ho = new TH1D("ho","ho",5000,-0.1,1000);
TGraph *g = new TGraph(); 
const string& filename = "RefCurve_2016-12-06_26_223855.Wfm.Wfm.csv";
ifstream data(filename);
while(data>>x){
ho->Fill(x);
int n = g->GetN();
g->Set(n+1);
g->SetPoint(n,n,x);
//std::cout<<"a"<<std::endl;
}
TCanvas *os = new TCanvas("co");
g->Draw();

/*rootfile_name.replace(pos, 5, ".root");
TFile *f = new TFile(rootfile_name.c_str(), "RECREATE");
TTree *tree = new TTree("osc","FG data tree");
tree-> ReadFile(filename.c_str(),"wave");
f-> Write();*/
}
