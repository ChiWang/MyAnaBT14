
#include <fstream>

void Run(){
  FileStat_t x;
  TString libName = "libMyCombine";
  if(gSystem->GetPathInfo(libName,x)){
    //gROOT->ProcessLine(".include $DMPSWSYS/include"); // same as the next line
    gInterpreter->AddIncludePath("$DMPSWSYS/include");
    gSystem->Load("$DMPSWSYS/lib/libDmpEvent.so");
    //gSystem->Load("$DMPSWSYS/lib/libDmpKernel.so");
    //gSystem->Load("$DMPSWWORK/lib/libDmpEvtAms.so");
    //gSystem->Load("$DMPSWWORK/lib/libDmpEventRaw.so");
    gSystem->CompileMacro("./MyCombine.C","k",libName);
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
      f = "./AMS/SPS/Combine_run_1416070809_ANC_366.root";
      break;
    case muon:
      f = "./AMS/SPS/Combine_run_1416338929_ANC_476.root";
      break;
    case photon:
      f = "./AMS/SPS/Combine_run_1416276173_ANC_451.root";
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
  ifstream f_0("./AMS/GoodRunList_Side0.log");
  ifstream f_1("./AMS/GoodRunList_Side1.log");
  char name0[256],name1[256];

  while((!f_0.eof()) && (!f_1.eof())){
    f_0.getline(name0,256);
    f_1.getline(name1,256);
    cout<<"Processing "<<name0<<"\t"<<name1<<endl;
    CombineData::AMSSPS(name0,name1);
  }
}


