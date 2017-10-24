void zero()
{
  for(int i = 0; i<32; ++i){
    ofstream ofs(Form("offsettable%02d.txt", i));
    for(int cell =0; cell<1024; ++cell){
      ofs << 0 << std::endl;
    }
  }

}
