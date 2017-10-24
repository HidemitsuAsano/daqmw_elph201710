{
   gROOT->Reset();

   //Add the tutorials directory to the macro path
   //This is necessary in case this macro is executed from another user directory
   TString dirName = gSystem->UnixPathName(__FILE__);
   //dirName.ReplaceAll("gui.C","");
   //dirName.ReplaceAll("/./","");
   const char *current = gROOT->GetMacroPath();
   gROOT->SetMacroPath(Form("%s:%s",current,dirName.Data()));

   TControlBar *bar = new TControlBar("vertical", "DAQ",10,10);
   bar->AddButton("start DAQ",".! ./start_daq.sh",        "start daq");
   
   bar->AddButton("kill all ", ".! ./kill_All.sh",        "kill all daq component");
   bar->SetTextColor("red");
   bar->SetButtonWidth(150);
   bar->Show();
   gROOT->SaveContext();
}
