
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
int Load(short type, TString InFN="NO")
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
  return Conf::LoadInput(f);
}

//-------------------------------------------------------------------
void test()
{
  AMS::Performance::Clusters();
  AMS::Alignment::SingleStrack_S_Side();
  //Tracking::Plots(nEvt);
}

//-------------------------------------------------------------------
void CombineAMSSPS()
{
  ifstream f_0("./AMS/GoodRunList_Side0.log");
  ifstream f_1("./AMS/GoodRunList_Side1.log");
  ofstream logOut("CombineAMSSPS.log");
  logOut<<"file side0\tfile side1\tevents"<<endl;
  char name0[256],name1[256];

  while((!f_0.eof()) && (!f_1.eof())){
    f_0.getline(name0,256);
    f_1.getline(name1,256);
    long entries = CombineData::AMSSPS(name0,name1);
    cout<<"\nProcessing\t"<<name0<<"\t"<<name1;
    if(entries > 0){
      logOut<<name0<<"\t"<<name1<<"\t"<<entries<<endl;
    }
  }
  logOut.close();
}

//-------------------------------------------------------------------
void CombineAll()
{
  ifstream f_0("./Rec0/Match.log");
  ofstream logOut("CombineAll.log");
  logOut<<"file_dampe\tfile_ams\tfile_anc\tevents"<<endl;
  char tmp[256];

  do{
    f_0.getline(tmp,256);
    istringstream aLine(tmp);
    TString dampe_file="", ams_file="", anc_file="";
    aLine>>dampe_file>>ams_file>>anc_file;
    if(dampe_file == "" || ams_file =="" || anc_file ==""){
      continue;
    }else{
      cout<<"\nProcessing\t"<<dampe_file<<"\t"<<ams_file<<"\t"<<anc_file<<endl;
    }
    long nEvt = CombineData::DAMPE_AMS_ANC(dampe_file,ams_file,anc_file);
    if(nEvt > 0){
      logOut<<dampe_file<<"\t"<<ams_file<<"\t"<<anc_file<<"\t"<<nEvt<<endl;
    }
  }while(!f_0.eof());
  logOut.close();
}



