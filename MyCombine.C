/*
 *  $Id: MyCombine.C, 2014-12-30 16:21:17 DAMPE $
 *  Author(s):
 *    Chi WANG (chiwang@mail.ustc.edu.cn) 30/11/2014
*/

/*
 * refer to Readme.md
 *
 * // unit for length: cm 
 * // strip 0,0 as reference.
 *
 */

#include "TFile.h"
#include "TTree.h"

#include "Event.hh"
#include <iostream>
#include "DmpEvtBTAnc.h"
#include "DmpEvtBgoHits.h"
#include "DmpEvtHeader.h"
#include "DmpEvtNudHits.h"
//#include "DmpRoootIOSvc.h"

#define AMSTreeName "t4"
#define AMSBranchName "cluster_branch"
#define ANCTreeName "AncillaryEvent"

namespace CombineData{
using namespace std;
/*
 *  only save data of clusters in Event.hh
 *
 */
//-------------------------------------------------------------------
namespace Conf{
  enum Location{
    PS = 0,
    SPS = 1,
  };
  short ExHall = SPS;// or PS
  long     entries = -1;
  long     evtID = -1;
  TString  outFileName="";
namespace Path{
  TString AMS_Side0 = "./AMS/SPS_side0/";
  TString AMS_Side1 = "./AMS/SPS_side1/";
  TString AMS_Out = "./AMS/SPS/";
  TString Sub_AMS = "./Rec0/AMS/";
  TString Sub_ANC = "./Rec0/ANC/";
  TString Sub_DAMPE = "./Rec0/DAMPE/";
  TString ALLCombine = "./Rec0/ALL/";
};
};

//-------------------------------------------------------------------
namespace Error{
enum Type{
  CombineAMS_RunID_NotMatch = -1,
  Entries_NotMatch = -2,
  NotSPS = -3,
  OpenFileError = -4,
};
};

//-------------------------------------------------------------------
int MyException(short type,TString argv1=""){
  switch (type) {
    case Error::CombineAMS_RunID_NotMatch:
      cout<<"ERROR:  run ID not match: "<<argv1<<endl;
      break;
    case Error::Entries_NotMatch:
      cout<<"ERROR:  entries not match: "<<argv1<<endl;
      break;
    case Error::NotSPS:
      cout<<"ERROR:  Conf::ExHall is not SPS..."<<endl;
      break;
  }
  return type;
}

//-------------------------------------------------------------------
short LadderInOrder(int ladderID){
  short n=0;
  if(Conf::ExHall == Conf::PS){
    // beam (--->):  ladder 0, 1, 5, 4, 3
    if(ladderID <=1){
      n = ladderID;
    }else{
      n = 7-ladderID;
      if(n != 2 && n != 3 && n!=4){
        cout<<"ERROR: ladder "<<ladderID<<" not exist at "<<Conf::ExHall<<endl;
        n = -1;
      }
    }
  }else if(Conf::ExHall == Conf::SPS){
    // beam (--->): ladder 12, 13, 0, 1, 2
    if(ladderID <=2){
      n = 2+ladderID;
    }else{
      n = ladderID-12;
      if(n != 0 && n !=1){
        cout<<"ERROR: ladder "<<ladderID<<" not exist at "<<Conf::ExHall<<endl;
        n = -1;
      }
    }
  }
  return n;
}

//-------------------------------------------------------------------
void ConstructOutFileName(TString Dmp_tag,TString Anc_tag){
  Conf::outFileName = "Rec0_H";
  Conf::outFileName +=Conf::ExHall;
  Anc_tag.Remove(Anc_tag.Length()-5,5);
  Anc_tag.Remove(0,18);
  if(Anc_tag.Sizeof() == 3){
    Anc_tag = "-ANC_0"+Anc_tag;
  }else{
    Anc_tag = "-ANC_"+Anc_tag;
  }
  Conf::outFileName += Anc_tag;
  //Dmp_tag = Dmp_tag.Replace(0,9,"DMP_");
  Dmp_tag.Replace(0,9,"_DMP_");
  Dmp_tag.Remove(Dmp_tag.Length()-10,5);
  Conf::outFileName += Dmp_tag;
}

//-------------------------------------------------------------------
long AMSSPS(TString file_s0="run_1416155587_ANC_387.root",TString file_s1="run_1416155589_ANC_387.root"){
cout<<endl;
  cout<<"\nProcessing\t"<<file_s0<<"\t"<<file_s1<<endl;
  if(Conf::ExHall != Conf::SPS){
    return MyException(Error::NotSPS);
  }
  TString tf0 = file_s0,tf1 = file_s1;
  if(tf0.Remove(0,tf0.Length()-13) != tf1.Remove(0,tf1.Length()-13)){
    return MyException(Error::CombineAMS_RunID_NotMatch,tf0+"   "+tf1);
  }else{
    tf0 = Conf::Path::AMS_Side0+file_s0;
    tf1 = Conf::Path::AMS_Side1+file_s1;
  }
  TFile *f_s0 = TFile::Open(tf0,"READ");
  if(!f_s0){
    return Error::OpenFileError;
  }
  TTree *tree_s0 = (TTree*)(f_s0->Get(AMSTreeName));
  Conf::entries = tree_s0->GetEntries();
  TFile *f_s1 = TFile::Open(tf1,"READ");
  if(!f_s1){
    return Error::OpenFileError;
  }
  TTree *tree_s1 = (TTree*)(f_s1->Get(AMSTreeName));
  if(tree_s1->GetEntries() != Conf::entries){
    delete f_s0;
    delete f_s1;
    return MyException(Error::Entries_NotMatch,file_s0+" "+tree_s0->GetEntries()+"\t"+file_s1+" "+tree_s1->GetEntries());
  }

  Event *event_s0 = new Event();
  tree_s0->SetBranchAddress(AMSBranchName,&event_s0);
  Event *event_s1 = new Event();
  tree_s1->SetBranchAddress(AMSBranchName,&event_s1);

  // output
  Event *event_01 = new Event();
  TString name = Conf::Path::AMS_Out+"Combine_"+file_s1;
  TFile *f_01 = new TFile(name,"RECREATE");
  TTree *tree_01 = new TTree(AMSTreeName,"AMSClusters");
  tree_01->Branch(AMSBranchName,"Event",&event_01,32000,2);

  // event loop, combine side 0 and side 1
  for(Conf::evtID = 0;Conf::evtID<Conf::entries;++Conf::evtID){
    tree_s0->GetEntry(Conf::evtID);
    int nC = event_s0->Cls->GetEntriesFast();
    for(int ic=0;ic<nC;++ic){
      const Cluster* cl = event_s0->GetCluster(ic);
      event_01->AddCluster(cl);
    }
    tree_s1->GetEntry(Conf::evtID);
    int nC1 = event_s1->Cls->GetEntriesFast();
    for(int ic=0;ic<nC1;++ic){
      const Cluster* cl = event_s1->GetCluster(ic);
      event_01->AddCluster(cl);
    }
    tree_01->Fill();
    event_01->Clear();
    if(Conf::evtID%(Conf::entries / 5) == 0)cout<<".";
  }

  tree_01->Write("",TObject::kOverwrite);
  f_01->Close();
  delete f_01;
  delete f_s0;
  delete f_s1;
  delete event_01;
  delete event_s0;
  delete event_s1;
  cout<<Conf::entries<<endl;
  return Conf::entries;
}

//-------------------------------------------------------------------
//bool DAMPE_AMS_ANC(TString file_name_DAMPE="A2Data00_20141115_131911_Hits.root", TString file_name_AMS="Combine_run_1416053170_ANC_357.root", TString file_name_ANC="VMEAncillary_Data_357.root"){
long DAMPE_AMS_ANC(TString file_name_DAMPE="A2Data00_20141118_154848_Hits.root", TString file_name_AMS="Combine_run_1416053170_ANC_357.root", TString file_name_ANC="VMEAncillary_Data_357.root"){
  cout<<"\nProcessing\t"<<file_name_DAMPE<<"\t"<<file_name_AMS<<"\t"<<file_name_ANC<<endl;

  TString tf0 = file_name_AMS;    // AMS
  TString tf1 = file_name_ANC;    // ANC
  TString tf2 = file_name_DAMPE;    // DAMPE
  if(tf0.Remove(0,tf0.Length()-8) != tf1.Remove(0,tf1.Length()-8)){
    return MyException(Error::CombineAMS_RunID_NotMatch,tf0+"  "+tf1);
  }else{
    tf0 = Conf::Path::Sub_AMS+file_name_AMS;
    tf1 = Conf::Path::Sub_ANC+file_name_ANC;
    tf2 = Conf::Path::Sub_DAMPE+file_name_DAMPE;
  }
  TFile *f_s0 = TFile::Open(tf0,"READ");
  if(!f_s0){
    return Error::OpenFileError;
  }
  TTree *tree_s0 = (TTree*)(f_s0->Get(AMSTreeName));
  long AMS_entries = tree_s0->GetEntries();
  TFile *f_s1 = TFile::Open(tf1,"READ");
  if(!f_s1){
    return Error::OpenFileError;
  }
  TTree *tree_s1 = (TTree*)(f_s1->Get(ANCTreeName));
  if(tree_s1->GetEntries() != AMS_entries){
    delete f_s0;
    delete f_s1;
    return MyException(Error::Entries_NotMatch,file_name_AMS+" "+AMS_entries+"\t"+file_name_ANC+" "+tree_s1->GetEntries());
  }
  TFile *f_s2 = TFile::Open(tf2,"READ");
  if(!f_s2){
    return Error::OpenFileError;
  }
  TTree *tree_s2 = (TTree*)(f_s2->Get("/Event/Cal"));

  // input
  Event *event_AMS = new Event();
  Int_t V792[1][32];
  DmpEvtBgoHits *event_BgoHits = new DmpEvtBgoHits();
  DmpEvtPsdHits *event_PsdHits = new DmpEvtPsdHits();
  DmpEvtHeader *event_Header = new DmpEvtHeader();
  DmpEvtNudHits *event_NudHits = new DmpEvtNudHits();
  tree_s0->SetBranchAddress(AMSBranchName,&event_AMS);
  tree_s1->SetBranchAddress("V792",V792);
  tree_s2->SetBranchAddress("Hits",&event_BgoHits);
  tree_s2->SetBranchAddress("PsdHits",&event_PsdHits);
  tree_s2->SetBranchAddress("NudHits",&event_NudHits);
  tree_s2->SetBranchAddress("EventHeader",&event_Header);

//-------------------------------------------------------------------
  ConstructOutFileName(file_name_DAMPE,file_name_ANC);
  TFile *f_out = new TFile(Conf::Path::ALLCombine+Conf::outFileName,"RECREATE");
  f_out->mkdir("Event");
  TTree *tree_out = new TTree("Rec0","Rec0");
  tree_out->SetAutoSave(50000000);

  //output
  DmpEvtHeader *evt_Header = new DmpEvtHeader();
  tree_out->Branch("Header",evt_Header->GetName(),&evt_Header,32000,2);
  //DmpEvtRec0 *evt_Rec0 = new DmpEvtRec0();
  //tree_out->Branch("Rec0",evt_Rec0->GetName(),&evt_Rec0,32000,2);
  DmpEvtBgoHits *evt_Bgo = new DmpEvtBgoHits();
  DmpEvtPsdHits *evt_Psd = new DmpEvtPsdHits();
  DmpEvtNudHits *evt_Nud = new DmpEvtNudHits();
  DmpEvtBTAnc *evt_Anc = new DmpEvtBTAnc();
  tree_out->Branch("Bgo",evt_Bgo->GetName(),&evt_Bgo,32000,2);
  tree_out->Branch("Psd",evt_Psd->GetName(),&evt_Psd,32000,2);
  tree_out->Branch("Nud",evt_Nud->GetName(),&evt_Nud,32000,2);
  tree_out->Branch("Anc",evt_Anc->GetName(),&evt_Anc,32000,2);

  // event loop, combine DAMPE, AMS, ANC
  Conf::entries = tree_s2->GetEntries();
  for(Conf::evtID = 0;Conf::evtID<Conf::entries;++Conf::evtID){
    tree_s0->GetEntry(Conf::evtID);
    tree_s1->GetEntry(Conf::evtID);
    tree_s2->GetEntry(Conf::evtID);
    evt_Header->LoadFrom(event_Header);
    evt_Bgo->LoadFrom(event_BgoHits);
    evt_Psd->LoadFrom(event_PsdHits);
    for(unsigned int iNud=0;iNud<4;++iNud){
      evt_Nud->fChannelID.push_back(iNud);
      evt_Nud->fEnergy.push_back(event_NudHits->fEnergy.at(iNud));
    }
    evt_Anc->fAMSCls->AbsorbObjects(event_AMS->Cls);
    //evt_Anc->fAMSCls = (TClonesArray*)event_AMS->Cls->Clone();
    int ClsSize = evt_Anc->fAMSCls->GetEntriesFast();
    for(int xx=0;xx<ClsSize;++xx){
      Cluster *aC = (Cluster*)evt_Anc->fAMSCls->At(xx);
      aC->ladder = LadderInOrder(aC->ladder);
      if(aC->ladder == -1){
        return -5;
      }
    }
    evt_Anc->fAdcC1 = V792[0][1];
    evt_Anc->fAdcC2 = V792[0][9];
    evt_Anc->fAdcPbGlass = V792[0][3];
    evt_Anc->fAdcSc1 = V792[0][4];
    evt_Anc->fAdcSc2 = V792[0][5];
    evt_Anc->fAdcSc3 = V792[0][8];
    evt_Anc->fAdcSc4 = V792[0][7];
    evt_Anc->fAdcSd1 = V792[0][11];
    evt_Anc->fAdcSd2 = V792[0][12];
    tree_out->Fill();
    evt_Header->Reset();
    evt_Bgo->Reset();
    evt_Psd->Reset();
    evt_Anc->Reset();
  }

  f_out->cd("Event");
  tree_out->Write("",TObject::kOverwrite);
  f_out->Close();
//-------------------------------------------------------------------

  delete f_s0;
  delete f_s1;
  delete f_s2;
  delete event_AMS;
  delete event_Header;
  delete event_BgoHits;
  delete event_PsdHits;
  delete event_NudHits;
  delete evt_Anc;
  delete evt_Bgo;
  delete evt_Nud;
  delete evt_Psd;
  delete evt_Header;
  //delete evt_Rec0;
  cout<<"====>\t"<<Conf::outFileName<<"\t"<<Conf::entries<<endl;
  return Conf::entries;
}
};

