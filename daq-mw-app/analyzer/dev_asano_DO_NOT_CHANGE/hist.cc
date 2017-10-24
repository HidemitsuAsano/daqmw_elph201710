#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <vector>

#include <TFile.h>
#include <TH1.h>
#include <TH2.h>
#include <TTree.h>
#include <TNtuple.h>

using namespace std;

//H.Asano 
//
//how to Unpack raw data
//1 word = 4 bytes = 32 bits and those 32bits are ordered as BigEndian.
//So,
//1. run getBigEndian32() for 1 word 
//2. 32bits data should be shifted by Decode32bitWord();

//ch scheme 
//0 origin 
//MPPC 1 ch  0 - 31 L0 subL0
//MPPC 2 ch 32 - 63 L1 subL1
//MPPC 3 ch 64 - 95 L2 subL0
//MPPC 4 ch 96 - 127 L3 subL1
//layer 0 (beam downstream) - 3 (beam upstream)
//EASIROC 0-3
//fiber no. 0 origin max is 11
//X:0,U:1,V:2

const char XUV[3][2] = {"X","U","V"};
const int sigmathre[4] = {20,20,20,20};//adc threshold for each easiroc (0-3), unit is 1 sigma of pedestal


int getXUV(int ch=-1)
{
  if(64<= ch) ch -= 64;

  if(0 <= ch && ch <= 7){
    return 1;   //U (8 fibers)
  }else if(8  <= ch && ch <= 17){
    return 1;  //U (10 fibers)
  }else if(18 <= ch && ch <= 23){
    return 0;  //X (6 fibers)
  }else if(24 <= ch && ch <= 25){
    return 1;  //U (2 fibers)
  }else if(26 <= ch && ch <= 31){
    return 0; //X (6 fibers)
  }else if(32 <= ch && ch <= 37){
    return 0; //X (6 fibers)
  }else if(38 <= ch && ch <= 39){
    return 2; //V (2 fibers)
  }else if(40 <= ch && ch <= 45){
    return 0; //X (6 fibers)
  }else if(46 <= ch && ch <= 63){
    return 2; //V (18 fibers)
  }else{ 
    cout << "invalid ch" << ch << endl;
    return -1;
  }
}

//return sublayer  0 or 1
int getsublayer(int ch=-1)
{
  if(64<= ch) ch -= 64;

  int type = getXUV(ch);
  int sublayer = -1;
  if(type == 1){//U
    if(0 <= ch && ch <= 7) sublayer = 0; //L0 U0-U8 
    else if(8  <= ch && ch <= 15) sublayer = 1;//L1 U0-U7
    else if(16 <= ch && ch <= 17) sublayer = 0;//L0 U8-U9
    else if(24 <= ch && ch <= 25) sublayer = 1;//L1 U8-U9
  }else if(type == 0){//X
    if(18 <= ch && ch <= 23) sublayer = 0; //L0 X0-X5
    else if(26 <= ch && ch <= 31) sublayer = 1;//L1 X0-X5
    else if(32 <= ch && ch <= 37) sublayer = 0;//L0 X6-X11
    else if(40 <= ch && ch <= 45) sublayer = 1;//L1 X6-X11                               
  }else if(type == 2){//V
    if(38 <= ch && ch <= 39) sublayer = 0 ; //L0 V0-V1
    else if(46 <= ch && ch <= 47) sublayer = 1 ;//L1 V0-V1
    else if(48 <= ch && ch <= 55) sublayer = 0 ;//L0 V2-V9 
    else if(56 <= ch && ch <= 63) sublayer = 1 ; //L1 V2-V9
  }else{
    cout << "invalid ch" << ch << endl;
  }
  
  return sublayer;
}

//return layer0 -3
int getlayer(int ch=-1)
{
  bool is64over = false;
  if(64<= ch){
    ch -= 64;
    is64over = true;
  }

  int type = getXUV(ch);
  int layer = -1;
  if(type==1){//U
    if(0 <= ch && ch <= 7) layer = 0; //L0 U0-U8 
    else if(8 <= ch && ch <= 15) layer = 1;//L1 U0-U7
    else if(16<= ch && ch <= 17) layer = 0;//L0 U8-U9
    else if(24 <= ch && ch <= 25) layer = 1;//L1 U8-U9
  }else if(type == 0){//X
    if(18<= ch && ch <= 23) layer = 0; //L0 X0-X5
    else if(26<= ch && ch <= 31) layer = 1;//L1 X0-X5
    else if(32<= ch && ch <= 37) layer = 0;//L0 X6-X11
    else if(40<= ch && ch <= 45) layer = 1;//L1 X6-X11                               
  }else if(type == 2){//V
    if(38<= ch && ch <= 39) layer = 0 ; //L0 V0-V1
    else if(46<= ch && ch <= 47) layer = 1 ;//L1 V0-V1
    else if(48<= ch && ch <= 55) layer = 0 ;//L0 V2-V9 
    else if(56<= ch && ch <= 63) layer = 1 ; //L1 V2-V9
  }else{
    cout << "invalid ch" << ch << endl;
  }
  if(is64over) layer += 2; 
  return layer;
}

int getfiber(int ch=-1)
{
  if(64<= ch) ch = ch - 64;
  int type = getXUV(ch);
  int fiber = -1;
  //U type
  if(type==1){//U
    if(0 <= ch && ch <= 7) fiber = ch; // L0 U0-U8 
    else if(8 <= ch && ch <= 15) fiber = ch - 8;//L1 U0-U7
    else if(16<= ch && ch <= 17) fiber = ch - 8;//L0 U8-U9
    else if(24 <= ch && ch <= 25) fiber = ch - 16;//L1 U8-U9
  }else if(type == 0){//X
    if(18<= ch && ch <= 23) fiber = ch - 18; //L0 X0-X5
    else if(26<= ch && ch <= 31) fiber = ch - 26;//L1 X0-X5
    else if(32<= ch && ch <= 37) fiber = ch - 26;//L0 X6-X11
    else if(40<= ch && ch <= 45) fiber = ch - 34;//L1 X6-X11                               
  }else if(type == 2){//V
    if(38<= ch && ch <= 39) fiber = ch - 38 ; //L0 V0-V1
    else if(46<= ch && ch <= 47) fiber = ch - 46;//L1 V0-V1
    else if(48<= ch && ch <= 55) fiber = ch - 46;//L0 V2-V9 
    else if(56<= ch && ch <= 63) fiber = ch - 54; //L1 V2-V9
  }else{
    cout << "invalid ch" << ch << endl;
  }
   
  return fiber;
}

//get easiroc chip #0-3 
int geteasiroc(int ch=-1)
{
  int easiroc = -1;
  if(0 <= ch && ch <= 31) easiroc = 0;
  else if(32<= ch && ch <= 63) easiroc = 1;
  else if(64<= ch && ch <= 95) easiroc = 2;
  else if(96<= ch && ch <= 127) easiroc = 3;
  else cout << "invalid ch" << ch << endl;
  return easiroc;
}


unsigned int getBigEndian32(const char* b=NULL)
{
    //std::cout << "size of b " << sizeof(b) << std::endl;
    return ((b[0] << 24) & 0xff000000) |
           ((b[1] << 16) & 0x00ff0000) |
           ((b[2] <<  8) & 0x0000ff00) |
           ((b[3] <<  0) & 0x000000ff);
}

unsigned int Decode32bitWord(unsigned int word32bit=0)
{
  //check data format
  unsigned int frame = word32bit & 0x80808080;
  if(frame != 0x80000000){
    std::cerr << __FILE__ << " L." << __LINE__ << " Frame Error! " << std::endl;
    std::cerr << "32 bit word: " << std::hex << word32bit << std::dec << std::endl;
    return 0;
  }

  return ((word32bit & 0x7f000000) >> 3) | 
         ((word32bit & 0x007f0000) >> 2) |
         ((word32bit & 0x00007f00) >> 1) |
         ((word32bit & 0x0000007f) >> 0);
}


//ADC High Gain
bool isAdcHg(unsigned int data=0)
{
    return (data & 0x00680000) == 0x00000000;
}

//ADC Low Gain
bool isAdcLg(unsigned int data=0)
{
    return (data & 0x00680000) == 0x00080000;
}

//TDC Leading
bool isTdcLeading(unsigned int data=0)
{
    return (data & 0x00601000) == 0x00201000;
}

//TDC Trailing
bool isTdcTrailing(unsigned int data=0)
{
    return (data & 0x00601000) == 0x00200000;
}

//not impletented yet 
bool isScaler(unsigned int data=0)
{
    return (data & 0x00600000) == 0x00400000;
}

//create structure for each ch.
struct FiberHit{
  int layer;
  int fiber;
  int easiroc;
  int type;
  double pos;
  int adchigh;
  bool otradchigh;
  int adclow;
  bool otradclow;
  int tdcleading;
  int tdctrailing;

  FiberHit(){
    layer = -1;
    fiber = -1;
    easiroc = -1;
    type = -1;
    pos = -99.;
    adchigh = -1;
    otradchigh = false;
    adclow = -1;
    otradclow = false;
    tdcleading = -1; 
    tdctrailing = -1;
  }
  void clear(){
    layer = -1;
    fiber = -1;
    easiroc = -1;
    type = -1;
    pos = -99.;
    adchigh = -1;
    otradchigh = false;
    adclow = -1;
    otradclow = false;
    tdcleading = -1; 
    tdctrailing = -1;
  }

};



void hist(const string& filename=".dat")
{
    string::size_type pos = filename.find(".dat");
    if(pos == string::npos) {
        cerr << filename << " is not a dat file" << endl;
        return;
    }
    //TNtuple *ntuple = new TNtuple("ntuple","rawhit wise ntuple","event:ch:adchigh:tdcleading:tdctrailing");
    //string rootfile_name(filename);
    int ipath = filename.find_last_of("/")+1;
    int iext  = filename.find_last_of(".");
    int irun = filename.find_last_of("_");
    //cout << ipath << endl;
    string rootfile_name = filename.substr(ipath,iext-ipath);
    rootfile_name+=".root";
    //for(int i=0;i<3;i++) rootfile_name.erase(rootfile_name.begin());
    std::cout << "output file name: " << rootfile_name.c_str() << std::endl;
    TFile *f = new TFile(rootfile_name.c_str(), "RECREATE");
    ifstream datFile(filename.c_str(), ios::in | ios::binary);
    
    //read threshold

    int iext2  = rootfile_name.find_last_of(".");
    string fresult = rootfile_name.substr(0,iext2);
    fresult +="_ret.txt";
    string ftable = "pede_table/";
    ftable += fresult;
    ifstream threfile(ftable.c_str(),ios::in); 
    cout << fresult.c_str() << endl;
    if(!threfile){
      cout << "can not open" << ftable.c_str() << endl;
      return ;
    }
    int ithch=0;
    double thre[128];
    double ahitmean[128];
    double sigmathreshold=10.0;
    while(threfile){
      double ch,mean,sigma;
      double hitmean, hitmeanerr;
      double hitsigma, hitsigmaerr;
      threfile >> ch >> mean >> sigma >> hitmean >> hitmeanerr >> hitsigma >> hitsigmaerr;
      thre[ithch] = mean + sigmathreshold*sigma;
      ahitmean[ithch] = hitmean;
      //if(ithch!=0){
        //double diff = ahitmean[ithch] - ahitmean[0];
        //if(ahitmean[ithch] > 1200) thre[ithch] += diff;
      /}
      cout << ch << " " << mean << " " << sigma << " " <<thre[ithch] << endl;
      ithch++;
    }


    const int totalch = 128;

    TH1I* adcHigh[totalch];
    TH1I* adcLow[totalch];
    TH1I* tdcLeading[totalch];
    TH1I* tdcTrailing[totalch];
    TH1I* tdcToT[totalch];
    TH1F* scaler[67*2];
    TH1F* hitprofile[4][3];//layer , type(xuv)
    TH1F* hitprofile_ch[4][3];//layer , type(xuv)
    TH1I* hitmulti[4][3];//layer , type(xuv)
    TH2F* hitcorrXX[4];//layer , type(xuv)
    TH2F* hitcorrUU[4];//
    TH2F* hitcorrVV[4];
    TH2F* hitcorrXX_ch[4];//layer , type(xuv)
    TH2F* hitcorrUU_ch[4];//
    TH2F* hitcorrVV_ch[4];
    TH2I* adcToTcorr[totalch];

    int nbin = 4096;
    for(int i = 0; i < totalch; ++i) {
        adcHigh[i] = new TH1I(Form("ADC_HIGH_%d", i),
                              Form("ADC high gain %d", i),
                              nbin, 0, 4096);
        adcHigh[i]->SetXTitle("ADC [ch.]");
        adcLow[i] = new TH1I(Form("ADC_LOW_%d", i),
                             Form("ADC low gain %d", i),
                             nbin, 0, 4096);
        adcLow[i]->SetXTitle("ch.");
        tdcLeading[i] = new TH1I(Form("TDC_LEADING_%d", i),
                                 Form("TDC leading %d", i),
                                 nbin, 0, 4096);
        tdcLeading[i]->SetXTitle("TDC [ch.]");
        tdcTrailing[i] = new TH1I(Form("TDC_TRAILING_%d", i),
                                  Form("TDC trailing %d", i),
                                  nbin, 0, 4096);
        tdcTrailing[i]->SetXTitle("ch.");
        scaler[i] = new TH1F(Form("SCALER_%d", i),
                             Form("Scaler %d", i),
                             //4096, 0, 5.0);
                             nbin, 0, 5.0*20.);
        tdcToT[i] = new TH1I(Form("TDC_TOT_%d",i),
                             Form("TDC_TOT_%d",i),
                             256,0,256);
        adcToTcorr[i] = new TH2I(Form("adcToTcorr%d",i),Form("Adc to TDC corr%d",i),256,0,4096,256,0,256);
        adcToTcorr[i]->SetXTitle("ADC [ch.]");
        adcToTcorr[i]->SetYTitle("ToT [ch.]");
    }


    for(int ilr =0 ; ilr<4; ilr++){
      for(int itype=0;itype<3;itype++){
        hitprofile[ilr][itype] = new TH1F(Form("Hitproflayer%s%d",XUV[itype],ilr),Form("Hit profile %s layer %d",XUV[itype],ilr), 40,-10,10); 
        hitprofile[ilr][itype]->SetXTitle("local pos. [mm]");
      }
    }
    
    for(int ilr =0 ; ilr<4; ilr++){
      for(int itype=0;itype<3;itype++){
        hitprofile_ch[ilr][itype] = new TH1F(Form("Hitproflayer_ch%s%d",XUV[itype],ilr),Form("Hit ch. profile %s layer %d",XUV[itype],ilr), 12,0,12); 
        hitprofile_ch[ilr][itype]->SetXTitle("fiber ch.#");
      }
    }
    
    for(int ilr =0 ; ilr<4; ilr++){
      for(int itype=0;itype<3;itype++){
        hitmulti[ilr][itype] = new TH1I(Form("HitMulti%s%d",XUV[itype],ilr),Form("Hit Multiplicity %s layer %d",XUV[itype],ilr), 20,0,20); 
        hitmulti[ilr][itype]->SetXTitle("hit multiplicity");
      }
    }

    for(int ipat=1 ; ipat<4;ipat++){
      hitcorrXX[ipat] = new TH2F(Form("HitCorrX0_X%d",ipat),Form("Hit correlation X0 - X%d",ipat),40,-10,10,40,-10,10);
      hitcorrXX[ipat]->SetXTitle("X pos. [mm]");
      hitcorrXX[ipat]->SetYTitle("X pos. [mm]");
      hitcorrUU[ipat] = new TH2F(Form("HitCorrU0_U%d",ipat),Form("Hit correlation U0 - U%d",ipat),40,-10,10,40,-10,10);
      hitcorrUU[ipat]->SetXTitle("U pos. [mm]");
      hitcorrUU[ipat]->SetYTitle("U pos. [mm]");
      hitcorrVV[ipat] = new TH2F(Form("HitCorrV0_V%d",ipat),Form("Hit correlation V0 - V%d",ipat),40,-10,10,40,-10,10);
      hitcorrVV[ipat]->SetXTitle("V pos. [mm]");
      hitcorrVV[ipat]->SetYTitle("V pos. [mm]");
    }
    
    for(int ipat=1 ; ipat<4;ipat++){
      hitcorrXX_ch[ipat] = new TH2F(Form("HitCorr_chX0_X%d",ipat),Form("Hit correlation X0 - X%d",ipat),13,0,13,13,0,13);
      hitcorrXX_ch[ipat]->SetXTitle("X0 fiber #");
      hitcorrXX_ch[ipat]->SetYTitle("fiber #");
      hitcorrUU_ch[ipat] = new TH2F(Form("HitCorr_chU0_U%d",ipat),Form("Hit correlation U0 - U%d",ipat),13,0,13,13,0,13);
      hitcorrUU_ch[ipat]->SetXTitle("U");
      hitcorrUU_ch[ipat]->SetYTitle("U pos. [mm]");
      hitcorrVV_ch[ipat] = new TH2F(Form("HitCorr_chV0_V%d",ipat),Form("Hit correlation V0 - V%d",ipat),13,0,13,13,0,13);
      hitcorrVV_ch[ipat]->SetXTitle("V pos. [mm]");
      hitcorrVV_ch[ipat]->SetYTitle("V pos. [mm]");
    }


    scaler[64] = new TH1F("SCALER_OR32U", "Scaler OR32U",
                          4096, 0, 200);
    scaler[65] = new TH1F("SCALER_OR32L", "Scaler OR32L",
                          4096, 0, 200);
    scaler[66] = new TH1F("SCALER_OR64", "Scaler OR64",
                          //4096, 0, 200);
                          4096*10, 0, 200*10);

    struct FiberHit sfiber[totalch];
    
    unsigned int scalerValuesArray[10][69];
    //true # of events *2, because it is incremented when one module is finished
    unsigned int events = 0;
    const int maxbuf = 20;
    float hitpos[4][2][3][maxbuf]; //4pat. 2 pos.(L0-L?),type(X,U,V), buffer
    int fiberarr[4][2][3][maxbuf]; //4pat. 2 pos.(L0-L?),type(X,U,V), buffer
    int multi[4][3];//layer , type
    //int ihit[4][3];
    while(datFile) {
      if(events%2==0){//end of 1 event loop, then initialize
        //initialize TTree info.
        /*
        Tevent = events/2;
        vTlayer.clear();
        vTch.clear();
        vTfiber.clear();
        vTeasiroc.clear();
        vTtype.clear();
        vTlpos.clear();
        vTadchigh.clear();
        vTotradchigh.clear();
        vTadclow.clear();
        vTotradclow.clear();
        vTtdcLead.clear();
        vTtdcTrail.clear();
        */
        //initialize struct.
        for(int ich=0;ich<totalch;ich++) sfiber[ich].clear();
        if(events!=0){
          for(int ipat=1;ipat<4;ipat++){
            for(int itype =0 ;itype<3;itype++){
              for(int imul0=0;imul0<multi[0][itype];imul0++){
                for(int imul=0;imul<multi[ipat][itype];imul++){
                  if(itype==0){
                    hitcorrXX[ipat]->Fill(hitpos[ipat][0][itype][imul0],hitpos[ipat][1][itype][imul]);
                    hitcorrXX_ch[ipat]->Fill(fiberarr[ipat][0][itype][imul0],fiberarr[ipat][1][itype][imul]);
                  }else if(itype==1){
                    hitcorrUU[ipat]->Fill(hitpos[ipat][0][1][imul0],hitpos[ipat][1][1][imul]);
                    hitcorrUU_ch[ipat]->Fill(fiberarr[ipat][0][itype][imul0],fiberarr[ipat][1][itype][imul]);
                  }else if(itype==2){
                    hitcorrVV[ipat]->Fill(hitpos[ipat][0][2][imul0],hitpos[ipat][1][2][imul]);
                    hitcorrVV_ch[ipat]->Fill(fiberarr[ipat][0][itype][imul0],fiberarr[ipat][1][itype][imul]);
                  }
                }
              }
            }
          }
        }
        for(int ipat=0;ipat<4;ipat++){
          for(int i=0;i<2;i++){
            for(int itype=0;itype<3;itype++){
              for(int ibuf=0;ibuf<maxbuf;ibuf++){
                hitpos[ipat][i][itype][ibuf] = 99.;
                fiberarr[ipat][i][itype][ibuf] = -1;
              }
            }
          }
        }
        for(int ilr=0;ilr<4;ilr++){
          for(int itype=0;itype<3;itype++){
            if(events!=0) hitmulti[ilr][itype]->Fill(multi[ilr][itype]);
            multi[ilr][itype]=0;//layer , type
          }
        }
      }//if end of 1 event loop
      
      char DAQMWhead[8];
      datFile.read(DAQMWhead, 8);

      char headerByte[4];
      datFile.read(headerByte, 4);
      unsigned int header32 = getBigEndian32(headerByte);
      unsigned int header = Decode32bitWord(header32);
      bool isHeader = ((header >> 27) & 0x01) == 0x01;
      if(!isHeader) {
        std::cerr << "Frame Error of header data" << std::endl;
        fprintf(stderr, "    %08X\n", header);
        std::exit(1);
      }
      size_t dataSize = header & 0x0fff;
      //std::cout << "datasize " << dataSize << std::endl;
      unsigned int scalerValues[69];
      char* dataBytes = new char[dataSize * 4];
      datFile.read(dataBytes, dataSize * 4);
      for(size_t i = 0; i < dataSize; ++i) {
        unsigned int data32 = getBigEndian32(dataBytes + 4 * i);
        unsigned int data = Decode32bitWord(data32); 
        if(!data){
          cout << "Invalid data event " << events/2. << endl;
          cout << "data size " << dataSize << endl;
          break;
        }
        //int ch_test = (data >> 13) & 0x3f;
        //cout << "event " << events << " ch " << ch_test << endl;
        int ch = (data >> 13) & 0x3f;
        if(events%2==1) ch += 64; 
        int type = getXUV(ch);
        int layer = getlayer(ch);
        int fiber = getfiber(ch); 
        int easiroc = geteasiroc(ch);
        sfiber[ch].type = type;
        sfiber[ch].layer = layer;
        sfiber[ch].fiber = fiber;
        sfiber[ch].easiroc = easiroc;

        if(isAdcHg(data)) {
          bool otr = ((data >> 12) & 0x01) != 0;
          int value = data & 0x0fff;
          float pos = fiber/2.0 - 6.0;
          sfiber[ch].otradchigh = otr;
          sfiber[ch].adchigh = value;
          sfiber[ch].pos = pos;
          if(!otr) {
            //hit ?
            if(value > thre[ch]){
              //cout << "HIT layer " << layer << "type" << type << " ch " << ch << " fiber "<< fiber << endl;
              hitprofile[layer][type]->Fill(pos);
              hitprofile_ch[layer][type]->Fill(fiber);
              if(layer==0){
                hitpos[1][0][type][multi[layer][type]]=pos;//pat. 1 L0-L1
                hitpos[2][0][type][multi[layer][type]]=pos;//pat. 2 L0-L2
                hitpos[3][0][type][multi[layer][type]]=pos;//pat. 3 L0-L3
                fiberarr[1][0][type][multi[layer][type]]=fiber;//pat. 1 L0-L1
                fiberarr[2][0][type][multi[layer][type]]=fiber;//pat. 2 L0-L2
                fiberarr[3][0][type][multi[layer][type]]=fiber;//pat. 3 L0-L3
              }else{
                hitpos[layer][1][type][multi[layer][type]]=pos;
                fiberarr[layer][1][type][multi[layer][type]]=fiber;
              }
              multi[layer][type]++;
              if(multi[layer][type] > maxbuf) cout << "too many events" << endl;
            }
            adcHigh[ch]->Fill(value);
          }
        }else if(isAdcLg(data)) {
          bool otr = ((data >> 12) & 0x01) != 0;
          int value = data & 0x0fff;
          sfiber[ch].otradclow = otr;
          sfiber[ch].adclow = value;
          if(!otr) {
            adcLow[ch]->Fill(value);
          }
        }else if(isTdcLeading(data)) {
          int value = data & 0x0fff;
          sfiber[ch].tdcleading = value;
          tdcLeading[ch]->Fill(value);
        }else if(isTdcTrailing(data)) {
          int value = data & 0x0fff;
          sfiber[ch].tdctrailing = value;
          tdcTrailing[ch]->Fill(value);
        }else if(isScaler(data)) {
          int value = data & 0x3fff;
          scalerValues[ch] = value;
          //cout << "event:"<<events<<"/scalerValues["<<ch<<"]:"<<scalerValues[ch] << endl; 
          if(ch == 68) {
            int scalerValuesArrayIndex = events % 100;
            memcpy(scalerValuesArray[scalerValuesArrayIndex], scalerValues,
                sizeof(scalerValues));
          }
        }else {
          int ch = (data >> 13) & 0x3f;
          int value = data & 0x0fff;
          std::cout << "adchg:"  << (data & 0x00680000);
          std::cout << "adclg:"  << (data & 0x00680000);
          std::cout << "tdcl:"   << (data & 0x00601000);
          std::cout << "tdct:"   << (data & 0x00601000);
          std::cout << "scaler:" << (data & 0x00600000);
          std::cout << "data:" << data << std::endl; 
          std::cout << "ch:" << ch << " value:" << value << std::endl;
          std::cerr << "Unknown data type" << std::endl;
        }


          //store ADC vs ToT correlation
         // if((sfiber[ch].tdctrailing != -1) && (sfiber[ch].tdcleading !=-1)){
            double tot =  sfiber[ch].tdcleading - sfiber[ch].tdctrailing;
            tdcToT[ch]->Fill(tot);
            adcToTcorr[ch]->Fill(sfiber[ch].adchigh,tot);
         // 
         // }

        

      }//end of loop of dataSize 
      delete[] dataBytes;
      events++;
      if(events%1000==0) std::cout << "reading events#:" << events/2.0 << std::endl;
#if 1
        if(events % 10 == 0) {
            unsigned int scalerValuesSum[69];
            for(int i = 0; i < 69; ++i) {
                scalerValuesSum[i] = 0;
            }
            for(int i = 0; i < 10; ++i) {
		for(int j = 0; j < 69; ++j) {
                    scalerValuesSum[j] += scalerValuesArray[i][j];
                }
            }

            int counterCount1MHz = scalerValuesSum[67];
            int counterCount1KHz = scalerValuesSum[68];

            // 1count = 1.0ms
            double counterCount = (double)counterCount1KHz + counterCount1MHz / 1000.0;
            counterCount /= 2.0;
            for(size_t j = 0; j < 67; ++j) {
		bool ovf = ((scalerValuesSum[j] >> 13) & 0x01) != 0;
                ovf = true;
                double scalerCount = scalerValuesSum[j] & 0xffff;
                if(!ovf && scalerCount != 0) {
                    double rate = scalerCount / counterCount;
                    scaler[j]->Fill(rate);
                }
            }
        }
#endif
      
      /*
      if(events%2==0){
        Tevent = (int) events/2.;
        for(int ich=0;ich<totalch;ich++){
          vTch.push_back(ich);
          vTfiber.push_back(sfiber[ich].fiber);
          vTlayer.push_back(sfiber[ich].layer);
          vTeasiroc.push_back(sfiber[ich].easiroc);
          vTtype.push_back(sfiber[ich].type);
          vTlpos.push_back((double)sfiber[ich].pos);
          vTadchigh.push_back(sfiber[ich].adchigh);
          vTotradchigh.push_back(sfiber[ich].otradchigh);
          vTadclow.push_back(sfiber[ich].adclow);
          vTotradclow.push_back(sfiber[ich].otradclow);
          vTtdcLead.push_back(sfiber[ich].tdcleading);
          vTtdcTrail.push_back(sfiber[ich].tdctrailing);
        }
        //tree->Fill();
      }*/


    }//while dataFile
    cout << "total event analyzed " << events/2  << endl; 
    datFile.close();
    //tree->Write();
    //ntuple->Write();
    f->Write();
    f->Close();
}

int main(int argc, char** argv)
{
    if(argc != 2) {
        cerr << "hist <dat file>" << endl;
        return -1;
    }
    hist(argv[1]);

    return 0;
}
