//peak search area
const int istart =550 ;
const int iend =650 ;
const int n_ch=16;
//cable assign 
const int s1 = 1;
const int sb = 2;
const int gc = 3;
const int s2 = 4;
const int lg = 5;
const int s3 = 6;
const int s4 = 7;
const int ac = 8;
//threshold for efficiency
const int ths1 = 10;
const int thsb = 10;
const int thgc = 10;
const int ths2 = 10;
const int thlg = 10;
const int ths3 = 10;
const int ths4 = 10;
const int thac = 10;

#include <stdio.h>

//peak search
speak(std::vector<double> & dats, Double_t* peakx, Double_t* peakval){
    Double_t stop =0;
    Double_t stopx =0;
    for( int i=istart; i< iend; i++){
        if(stop < dats[i]){
            stop = dats[i];
            stopx = i;
        }
       
    }
    *peakx = stopx;
    *peakval = stop; 
    }


//decide baseline
double baseline(std::vector<double> & dat, Double_t* pk){
    Double_t dxL;
    Double_t peakph;
    speak(dat, &dxL, &peakph);
    Int_t xstart = 0;
    Int_t range1 = 1;
    Int_t range2=dxL-10;
    int sums = 0;
    Double_t p0[1000]= {0};
    Double_t a[1000]= {0};
    Double_t average = 0;    
    for( int k=range1; k< range2; k++){
        if(fabs(dat[k]-dat[k-1])<10){
            p0[k]= dat[k];
            a[k]= k;
            sums +=1;
        }
    }

    TGraph *h2 = new TGraph(sums,a,p0);
    TF1 *f = new TF1("f","pol0",1,range2);
    h2->Fit("f","Q","",0,range2);
    Double_t pp = f->GetParameter(0);

    *pk = pp;
}

plus(std::vector<double> &dat, int &k, int &nrun, int* peakL, Double_t* integ, Double_t* hight, Double_t* basel){
    
    Double_t sum = 0;
    //    Double_t xaxis = 0;
    Double_t p;
    baseline(dat, &p);
    Double_t xL =0;
    Double_t pulsehight =0;
    speak(dat, &xL, &pulsehight);
    Int_t neve = 0;
    for(int j=0; j<30; ++j){
        int x1 = xL + j -10;
        sum  += dat[x1] - p;
    }   
    Double_t key = pulsehight-p;
    *integ = sum;
    *peakL = xL;
    *hight = key;
    *basel = p;
}

//total integral & draw histgram
void easyint(Int_t nrun, Int_t pl, Int_t an, Int_t ntype){
  
    if(!nrun){
        std::cout << "Please input a run-number whose histogram you want to see." << std::endl;
        exit(0);
    }

    TString rootfile;
    rootfile.Form("../run%d.root", nrun);
    std::string rootfile_name(rootfile);
    TFile *f = new TFile(rootfile);
    f = (TFile*)gROOT->GetListOfFiles()->FindObject(rootfile.Data());
    if(!f){
        std::cout << "  failed to open rootfile.  " << std::endl;
        exit(0);
    }
    TTree *t1 = (TTree*)f->Get("tree");
    
    std::vector<double> *data[n_ch] = {0};
    TBranch *brptr[n_ch] = {0};
    for(int m=0; m< n_ch; m++){
      tree->SetBranchAddress(Form("WaveForm%d",m), &data[m], &brptr[m]);
      
    }
    
    // x axis
    std::vector<double> x(1000);
    for(int i = 0; i<1000; ++i){
        x[i] = i;
    }
    //efficiency
	int effs1, effsb, effgc, effs2, efflg, effs3, effs4, effac;
	Double_t es1, esb, egc, es2, elg, es3, es4, eac;
    hs1=new TH1F("S1", "S1", 200, 0, 10000);
    hsb=new TH1F("SB", "SB", 200, 0, 10000);
    hgc=new TH1F("GC", "GC", 200, 0, 10000);
    hs2=new TH1F("S2", "S2", 200, 0, 10000);
    hlg=new TH1F("LG", "LG", 200, 0, 10000);
    hs3=new TH1F("S3", "S3", 200, 0, 10000);
    hs4=new TH1F("S4", "S4", 200, 0, 10000);
    hac=new TH1F("AC", "AC", 200, 0, 10000);
  


    int n = tree->GetEntries();
    for(int i = 0; i<n; ++i){
        tree->GetEntry(i);
        std::vector<double>& dats1= *data[s1];       
        std::vector<double>& datsb= *data[sb]; 
        std::vector<double>& datgc= *data[gc]; 
        std::vector<double>& dats2= *data[s2]; 
	std::vector<double>& datlg= *data[lg]; 
        std::vector<double>& dats3= *data[s3]; 
	std::vector<double>& dats4= *data[s4]; 
        std::vector<double>& datac= *data[ac]; 
        int k=i;
 
        Double_t integs1 =0,  phs1 =0, bases1=0;
        Int_t peakLs1 =0;
        Double_t integsb =0,  phsb =0, basesb=0;
        Int_t peakLsb =0;
        Double_t integgc =0,  phgc =0, basegc=0;
        Int_t peakLgc =0;
	Double_t integs2 =0,  phs2 =0, bases2=0;
        Int_t peakLs2 =0;
        Double_t integlg =0,  phlg =0, baselg=0;
        Int_t peakLlg =0;
	Double_t integs3 =0,  phs3 =0, bases3=0;
        Int_t peakLs3 =0;
	Double_t integs4 =0,  phs4 =0, bases4=0;
        Int_t peakLs4 =0;
	Double_t integac =0,  phac =0, baseac=0;
        Int_t peakLac =0;

        plus(dats1, k, nrun, &peakLs1, &integs1, &phs1, &bases1);
	plus(datsb, k, nrun, &peakLsb, &integsb, &phsb, &basesb);
	plus(dats2, k, nrun, &peakLs2, &integs2, &phs2, &bases2);
	plus(datgc, k, nrun, &peakLgc, &integgc, &phgc, &basegc);
	plus(dats3, k, nrun, &peakLs3, &integs3, &phs3, &bases3);
	plus(dats4, k, nrun, &peakLs4, &integs4, &phs4, &bases4);
	plus(datac, k, nrun, &peakLac, &integac, &phac, &baseac);
        
	hs1->Fill(integs1);
	hsb->Fill(integsb);
	hgc->Fill(integgc);
	hs2->Fill(integs2);
	hlg->Fill(integlg);
	hs3->Fill(integs3);
	hs4->Fill(integs4);
        hac->Fill(integac);
	
	//efficiency
        if(phs1>ths1){
	  effs1 +=1;
	}
	 if(phsb>thsb){
	  effsb +=1;
	}
        if(phgc>thgc){
	  effgc +=1;
	}
	  if(phs2>ths2){
	  effs2 +=1;
	}
	 if(phlg>thlg){
	  efflg +=1;
	}
        if(phs3>ths3){
	  effs3 +=1;
	}
	 if(phs4>ths4){
	  effs4 +=1;
	}
        if(phac>thac){
	  effac +=1;
	}

    }
  
    es1 = double (effs1)*100/n;
    esb = double (effsb)*100/n;
    egc = double (effgc)*100/n;
    es2 = double (effs2)*100/n;
    elg = double (efflg)*100/n;
    es3 = double (effs3)*100/n;
    es4 = double (effs4)*100/n;
    eac = double (effac)*100/n;

   TCanvas *cinteg = new TCanvas("integral","integral");
    cinteg->Divide(4,2);
    cinteg->cd(1);
    hs1->Draw();
    /*TText *t=new TText(2,2,Form("efficiency%f",es1));
    t->SetTextSize(30);
    t->Draw();*/
    cinteg->cd(2);
    hsb->Draw();
    cinteg->cd(3);
    hgc->Draw();
    cinteg->cd(4);
    hs2->Draw();
    cinteg->cd(5);
    hlg->Draw();
    cinteg->cd(6);
    hs3->Draw();
    cinteg->cd(7);
    hs4->Draw();
    cinteg->cd(8);
    hac->Draw();

    std::cout<<"*****efficiency*****"<<std::endl;
    std::cout<<"s1  "<<es1<<std::endl;
    std::cout<<"sb  "<<esb<<std::endl;
    std::cout<<"gc  "<<egc<<std::endl;
    std::cout<<"s2  "<<es2<<std::endl;
    std::cout<<"lg  "<<elg<<std::endl;
    std::cout<<"s3  "<<es3<<std::endl;
    std::cout<<"s4  "<<es4<<std::endl;
    std::cout<<"ac  "<<eac<<std::endl;
    
    cinteg->Print(Form("../pdf/run%deasyhist.pdf",nrun));

}
