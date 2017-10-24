{
  string home = getenv("HOME");
  std::cout << "macro " << home <<"/macro/rootlogon.C is loaded "<< std::endl;
  system("pwd");
  std::cout << "This ROOT version: " << gROOT->GetVersion() << std::endl; 
  gROOT->SetStyle("Plain");
  gStyle->SetOptFit(1111);
  gStyle->SetOptStat(1111111);
//  gStyle->SetTitleX(0.1f);
//  gStyle->SetTitleW(0.8f);

  gROOT->GetColor(3)->SetRGB(0., 0.7, 0.); // Green  (0, 1, 0)->(0, 0.7, 0)
  gROOT->GetColor(5)->SetRGB(1., 0.5, 0.); // Yellow (1, 1, 0)->(0, 0.5, 0)
  
  Int_t font=42;
  gStyle->SetTextFont(font);
  gStyle->SetLabelFont(font, "XYZ");
  gStyle->SetTitleFont(font,"XYZ");
//  gStyle->SetTitleAlign(13);
//  gStyle->SetTextAlign(12);
  gStyle->SetTitleXOffset(1.1);
  gStyle->SetTitleYOffset(1.1);
//  gStyle->SetTitleX(0.085);
//  gStyle->SetTitleY(0.958);
  gStyle->SetStatFont(font);
  gStyle->SetStatFontSize(0.03);

  gStyle->SetHistLineWidth(2.0);

  gStyle->SetPadTickX(1);
  gStyle->SetPadTickY(1);

  gStyle->SetPadGridX(1);
  gStyle->SetPadGridY(1);

  gStyle->SetLabelSize(0.03,"x");
  gStyle->SetLabelSize(0.03,"y");
  
  gStyle->SetFuncColor(kRed);
}
