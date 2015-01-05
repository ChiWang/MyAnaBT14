
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
void CombineAMSSPS()
{
  ifstream f_0("./AMS/GoodRunList_Side0.log");
  ifstream f_1("./AMS/GoodRunList_Side1.log");
  ofstream logOut("./AMS/CombineSPS.log");
  logOut<<"file side0\tfile side1\tevents"<<endl;
  char name0[256],name1[256];

  while((!f_0.eof()) && (!f_1.eof())){
    f_0.getline(name0,256);
    f_1.getline(name1,256);
    long entries = CombineData::AMSSPS(name0,name1);
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
  ofstream logOut("./Rec0/CombineAll.log");
  logOut<<"file_dampe\tfile_ams\tfile_anc\tevents"<<endl;
  char tmp[256];

  bool beg=false;
  do{
    f_0.getline(tmp,256);
    istringstream aLine(tmp);
    TString dampe_file="", ams_file="", anc_file="";
    aLine>>dampe_file>>ams_file>>anc_file;
    if(dampe_file.Contains("Begin")){
      beg = true;
    }
    if(!beg){
            continue;
    }
    if(dampe_file.Contains("END")){
            break;
    }
    if(dampe_file.Contains(".root")&& ams_file.Contains(".root") && anc_file.Contains(".root")){
      if(ams_file.Contains("Combine")){
        CombineData::Conf::ExHall = CombineData::Conf::SPS;
      }else{
        CombineData::Conf::ExHall = CombineData::Conf::PS;
      }
      long nEvt = CombineData::DAMPE_AMS_ANC(dampe_file,ams_file,anc_file);
      if(nEvt > 0){
        logOut<<CombineData::Conf::outFileName<<"\t"<<nEvt<<endl;
      }
    }
  }while(!f_0.eof());
  logOut.close();
}



