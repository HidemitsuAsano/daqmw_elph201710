#ifndef NIMEASIROCDATA_H
#define NIMEASIROCDATA_H

const int ONE_EVENT_SIZE = 4;//byte

namespace NIMEASIROC{
  const int headersize = 4;//bytes. fixed value;
  const unsigned int normalframe     = 0x80000000;
  const unsigned int checkmask = 0x80808080;
}

struct NIMEASIROCData {
  unsigned int  header;
  int ch;
  int Adchigh;
  int Adclow;
  int TdcLeading;
  int TdcTrailing;
  int Scaler;
};



struct FiberHit{
  //int layer;
  //int fiber;
  //int easiroc;
  //int type;
  //double pos;
  int adchigh;
  bool otradchigh;
  int adclow;
  bool otradclow;
  int tdcleading;
  int tdctrailing;

  FiberHit(){
    //layer = -1;
    //fiber = -1;
    //easiroc = -1;
    //type = -1;
    //pos = -99.;
    adchigh = -1;
    otradchigh = false;
    adclow = -1; 
    otradclow = false;
    tdcleading = -1;
    tdctrailing = -1;
  }
  void clear(){
    //layer = -1;
    //fiber = -1;
    //easiroc = -1;
    //type = -1;
    //pos = -99.;
    adchigh = -1;
    otradchigh = false;
    adclow = -1;
    otradclow = false;
    tdcleading = -1;
    tdctrailing = -1;
  }

};









#endif
