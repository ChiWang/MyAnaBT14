
#include <fstream>

void Run(){
  FileStat_t x;
  TString libName = "libAMSAnalysis";
  if(gSystem->GetPathInfo(libName,x)){
    gSystem->Load("$DMPSWWORK/lib/libDmpEvent.so");
    gSystem->CompileMacro("./Analysis.C","k",libName);
  }else{
    gSystem->Load(libName);
  }
}

//-------------------------------------------------------------------
enum FileType{
  electron = 0,
  muon = 1,
  photon = 2,
};

//-------------------------------------------------------------------
void Load(short type, TString InFN="NO")
{
  TString f = "NO";
  switch (type){
    case electron:
      f = "./Data/Root_Data/Combine_run_1416070809_ANC_366.root";
      break;
    case muon:
      f = "./Data/Root_Data/Combine_run_1416338929_ANC_476.root";
      break;
    case photon:
      f = "./Data/Root_Data/Combine_run_1416276173_ANC_451.root";
      break;
  }
  Conf::LoadInput(f);
}

//-------------------------------------------------------------------
void test()
{
  AMS::Performance::Clusters();
  AMS::Alignment::SingleStrack_S_Side();
  //Tracking::Plots(nEvt);
}

//-------------------------------------------------------------------
void CombineAllAMSSPS(){
  ifstream f_0("./DATA/sci/AMS/SPS/Root_Data/GoodRunList_Side0.log");
  ifstream f_1("./DATA/sci/AMS/SPS/Root_Data/GoodRunList_Side1.log");
  char name0[256],name1[256];

  while((!f_0.eof()) && (!f_1.eof())){
    f_0.getline(name0,256);
    f_1.getline(name1,256);
    cout<<"Processing "<<name0<<"\t"<<name1<<endl;
    CombineData::AMSSPS(name0,name1);
  }
}


