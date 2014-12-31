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
#include "TSystem.h"
#include "TStyle.h"
#include "TH1F.h"
#include "TH1D.h"
#include "TProfile.h"
#include "TH2F.h"
#include "TF1.h"
#include "TCanvas.h"

#include "Event.hh"
#include <iostream>
#include <vector>
#include <map>
//#include <fstream>
#include "DmpEvtBTAnc.h"
#include "DmpEvtBgoHits.h"
#include "DmpEvtHeader.h"
#include "DmpEvtNudHits.h"
//#include "DmpEvtRec0.h"
//#include "DmpRoootIOSvc.h"

#define NLadder 5
#define AMSTreeName "t4"
#define AMSBranchName "cluster_branch"
#define ANCTreeName "AncillaryEvent"

using namespace std;

/*
  //  printf("%s\n", gSystem->GetMakeSharedLib());
  //gSystem->SetMakeSharedLib("cd $BuildDir ; clang++ -c $Opt -m64 -pipe -W -fsigned-char -fno-common -stdlib=libc++ -pthread $IncludePath $SourceFiles ; clang++ $ObjectFiles  -dynamiclib -single_module -Wl,-dead_strip_dylibs -O2 -m64 -mmacosx-version-min=10.9 -stdlib=libc++ $LinkedLibs -o $SharedLib");
  //  printf("%s\n", gSystem->GetMakeSharedLib());
  */

//-------------------------------------------------------------------
namespace MyError{
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
    case MyError::CombineAMS_RunID_NotMatch:
      cout<<"ERROR:  run ID not match: "<<argv1<<endl;
      break;
    case MyError::Entries_NotMatch:
      cout<<"ERROR:  entries not match: "<<argv1<<endl;
      break;
    case MyError::NotSPS:
      cout<<"ERROR:  Conf::ExHall is not SPS..."<<endl;
      break;
  }
  return type;
}

//-------------------------------------------------------------------
namespace Conf{

enum Location{
  PS = 0,
  SPS = 1,
};

namespace EnableCut{
  bool TotSN_Cut = true;
  bool Length_Cut = false;
  bool Cut_2 = false;
  bool Cut_3 = false;
  bool Cut_4 = false;
  bool Cut_5 = false;
  bool Cut_6 = false;
};

  void SetCutStatus(int st){
// *
// *  TODO:  how to set cut as gSytle->SetOptxx(011011011)?
// *
    //int defaultCut=1000000;  // order: refer to namespace EnableCut, from up to down
    st += 0;
  }

  TString Path = "./DATA/";
  short ExHall = SPS;// or PS
// *
// *  TODO:   which side near upstream is important
// *
  /*
   *  at PS, wich side (s or k) near upstream??
   *  at SPS, k side near upstream
   *
   */
  TString  File = "NO";     // file includes 5 ladders data
  long     entries = -1;
  long     evtID = -1;
  TTree    *tree = 0;
  Event    *AMS_Evt = 0;
  float     Distance[2][4] = {{119.5,   112,    45,     95},        // PS
                              {113.3,   761.1,  39.0,   96.5}};     // SPS.     unit cm. beam --->
  float     Pitch[2] = {0.011,0.0208};    // unit cm. first is s side, then k side
  float     kGap = 0.1392 + 0;      // unit cm.     1392 = 676*2 + 40 (um);  40: sensor distance,  676: active area to edge of sensor
  short     SensorNumber[NLadder] = {4,4,4,12,12};  // same for PS and SPS. from upstream to downstream

  // alignment parameter
// *
// *  TODO:   offset for PS and SPS
// *
  float     Offset[NLadder][2] = {
          {0.0,             0.},     // ladder 0 as reference. CoG of verticle track
          {-0.04936, -0.1965},    // others are offset
          {-0.5868, -1.72 },   // TODO, offset
          {-0.5671, -1.673},
          {-0.4975, -0.9338}};
  float  Position_Z[2][NLadder]={  // update it while load input
          {0,10,20,30,40},        // PS
          {0,10,20,30,40}};     // SPS.     unit cm. beam --->

  TF1 *gausFit = new TF1("GF","gaus",0,150);
  TF1 *linearFit = new TF1("LF","pol1",-10,1200);

  void LoadAlignmentParameter(TString aFile){
    cout<<"loading new alignment parameter. from file: "<<aFile<<endl;
    //ifstream input();
    // TODO: update automatically
  }

  int LoadInput(TString fName){
    gStyle->SetOptStat(11111111);
    gStyle->SetOptFit(111111111);
    File = fName;
    TFile *f = TFile::Open(File,"READ");
    if(!f){
      return MyError::OpenFileError;
    }
    //sss
    tree = (TTree*)(f->Get(AMSTreeName));
    entries = tree->GetEntries();
    if(AMS_Evt){
      delete AMS_Evt;
      AMS_Evt = 0;
    }
    AMS_Evt = new Event();
    tree->SetBranchAddress(AMSBranchName,&AMS_Evt);

    LoadAlignmentParameter("TODOME");

    // load z position
    if(Position_Z[PS][1] == 10){
      for(short i=0;i<NLadder;++i){
         Position_Z[PS][i] =0;
         Position_Z[SPS][i] =0;
         for(short iD=0;iD<i;++iD){
           Position_Z[PS][i] += Distance[PS][iD];
           Position_Z[SPS][i] += Distance[SPS][iD];
         }
      }
    }
  }

};

//-------------------------------------------------------------------
void LoadEvent(){
  Conf::tree->GetEntry(Conf::evtID);
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
      if(n != 2 || n != 3 || n!=4){
        cout<<"ERROR: ladder "<<ladderID<<" not exist at "<<Conf::ExHall<<endl;
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
      }
    }
  }
  return n;
}

//-------------------------------------------------------------------
bool GoodClusterCheck(Cluster *aC){
  if(Conf::EnableCut::TotSN_Cut){
    if(aC->GetTotSN()< 3.8 || aC->GetTotSN()>14.0){
      return false;
    }
  }
  if(Conf::EnableCut::Length_Cut){
// *
// *  TODO:  check cut value
// *
    if(aC->GetLength()< 1 || aC->GetTotSN()>10){
      return false;
    }
  }
  return true;
}

//-------------------------------------------------------------------
void ClusterNumberInLadder(vector<int> &r, bool choosingGoodCluster=true){
  r.resize(NLadder*2,0);
  int n_cls = Conf::AMS_Evt->Cls->GetEntriesFast();
  for(short ic=0;ic<n_cls;++ic){
    Cluster *aCluster = Conf::AMS_Evt->GetCluster(ic);
    if(choosingGoodCluster){
      if(! GoodClusterCheck(aCluster)){
        continue;
      }
    }
    ++r[LadderInOrder(aCluster->ladder)*2 + aCluster->side];
  }
}

//-------------------------------------------------------------------
bool N_ClustersInLadder_I(int N, short I, bool OnlyGoodCluster=true){
  // both sides has n clusters
  vector<int> clusBN;
  ClusterNumberInLadder(clusBN,OnlyGoodCluster);
  if(clusBN[I*2+0] !=N || clusBN[I*2+1] != N){
    return false;
  }
  return true;
}

//-------------------------------------------------------------------
bool ClusterNumberLessThan2_forAllS_Side(){
  vector<int> clusBN;
  ClusterNumberInLadder(clusBN);
  for(short id=1;id<NLadder;++id){
    if(clusBN[id*2+0]>1){
      return false;
    }
  }
  return true;
}

//-------------------------------------------------------------------
float GetPosition(Cluster *aCluster, bool includeOffset=true){
        /*
         *  include offset defautly,
         *
         *  but for alignment, must set the flag false
         *
         */
  float posi=0.0;
  short side = aCluster->side;
  float CoG = aCluster->GetCoG();
  if(side == 0){    // s-side
    posi = CoG * Conf::Pitch[side];
  }else{    // k-side
    CoG = CoG-640;
    posi = CoG*Conf::Pitch[side] + (CoG<192?0:Conf::kGap);
  }
  return posi + (includeOffset ? Conf::Offset[LadderInOrder(aCluster->ladder)][side] : 0);
}

//-------------------------------------------------------------------
namespace CombineData{
/*
 *  only save data of clusters in Event.hh
 *
 */

//-------------------------------------------------------------------
long AMSSPS(TString file_s0="run_1416155587_ANC_387.root",TString file_s1="run_1416155589_ANC_387.root"){

  if(Conf::ExHall != Conf::SPS){
    return MyException(MyError::NotSPS);
  }
  TString MyPath = "./AMS";
  TString tf0 = file_s0,tf1 = file_s1;
  if(tf0.Remove(0,tf0.Length()-13) != tf1.Remove(0,tf1.Length()-13)){
    return MyException(MyError::CombineAMS_RunID_NotMatch,tf0+"   "+tf1);
  }else{
    tf0 = MyPath+"/SPS_side0/"+file_s0;
    tf1 = MyPath+"/SPS_side1/"+file_s1;
  }
  TFile *f_s0 = TFile::Open(tf0,"READ");
  if(!f_s0){
    return MyError::OpenFileError;
  }
  TTree *tree_s0 = (TTree*)(f_s0->Get(AMSTreeName));
  Conf::entries = tree_s0->GetEntries();
  TFile *f_s1 = TFile::Open(tf1,"READ");
  if(!f_s1){
    return MyError::OpenFileError;
  }
  TTree *tree_s1 = (TTree*)(f_s1->Get(AMSTreeName));
  if(tree_s1->GetEntries() != Conf::entries){
    delete f_s0;
    delete f_s1;
    return MyException(MyError::Entries_NotMatch,file_s0+" "+tree_s0->GetEntries()+"\t"+file_s1+" "+tree_s1->GetEntries());
  }

  Event *event_s0 = new Event();
  tree_s0->SetBranchAddress(AMSBranchName,&event_s0);
  Event *event_s1 = new Event();
  tree_s1->SetBranchAddress(AMSBranchName,&event_s1);
  //return true;
//}

  // output
  Event *event_01 = new Event();
  TString name = MyPath+"/SPS/Combine_"+file_s1;
  TFile *f_01 = new TFile(name,"RECREATE");
  TTree *tree_01 = new TTree(AMSTreeName,"Cluster tree");
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
  }

  tree_01->Write("",TObject::kOverwrite);
  f_01->Close();
  delete f_01;
  delete f_s0;
  delete f_s1;
  delete event_01;
  delete event_s0;
  delete event_s1;
  return Conf::entries;
}

//-------------------------------------------------------------------
long DAMPE_AMS_ANC(TString file_name_DAMPE="A2Data00_20141118_154848_Hits.root", TString file_name_AMS="Combine_run_1416053170_ANC_357.root", TString file_name_ANC="VMEAncillary_Data_357.root"){
//bool DAMPE_AMS_ANC(TString file_name_DAMPE="A2Data00_20141115_131911_Hits.root", TString file_name_AMS="Combine_run_1416053170_ANC_357.root", TString file_name_ANC="VMEAncillary_Data_357.root"){

  TString MyPath = "./Rec0";
  TString tf0 = file_name_AMS;    // AMS
  TString tf1 = file_name_ANC;    // ANC
  TString tf2 = file_name_DAMPE;    // DAMPE
  if(tf0.Remove(0,tf0.Length()-8) != tf1.Remove(0,tf1.Length()-8)){
    return MyException(MyError::CombineAMS_RunID_NotMatch,tf0+"  "+tf1);
  }else{
    tf0 = MyPath+"/AMS/"+file_name_AMS;
    tf1 = MyPath+"/ANC/"+file_name_ANC;
    tf2 = MyPath+"/DAMPE/"+file_name_DAMPE;
  }
  TFile *f_s0 = TFile::Open(tf0,"READ");
  if(!f_s0){
    return MyError::OpenFileError;
  }
  TTree *tree_s0 = (TTree*)(f_s0->Get(AMSTreeName));
  long AMS_entries = tree_s0->GetEntries();
  TFile *f_s1 = TFile::Open(tf1,"READ");
  if(!f_s1){
    return MyError::OpenFileError;
  }
  TTree *tree_s1 = (TTree*)(f_s1->Get(ANCTreeName));
  if(tree_s1->GetEntries() != AMS_entries){
    delete f_s0;
    delete f_s1;
    return MyException(MyError::Entries_NotMatch,file_name_AMS+" "+AMS_entries+"\t"+file_name_ANC+" "+tree_s1->GetEntries());
  }
  TFile *f_s2 = TFile::Open(tf2,"READ");
  if(!f_s2){
    return MyError::OpenFileError;
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
  file_name_DAMPE.Remove(0,9);
  file_name_DAMPE.Replace(file_name_DAMPE.Length()-9,4,"Rec0");
  TString name = MyPath+"/ALL/DAMPE_AMS_ANC_"+file_name_DAMPE;
  TFile *f_out = new TFile(name,"RECREATE");
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
    /*
    cout<<" i = "<<Conf::evtID<<endl;
    int nC = event_AMS->Cls->GetEntriesFast();
    for(int xx=0;xx<nC;++xx){
      const Cluster *in = (Cluster*) event_AMS->Cls->At(xx);
      Cluster *out_Cls = (Cluster*)evt_Anc->fAMSCls->New(xx);
      out_Cls->LoadFrom(in);
      out_Cls->ladder = LadderInOrder(out_Cls->ladder);
      cout<<"  "<<out_Cls->ladder;
    }
    cout<<endl;
    */
    evt_Anc->fAMSCls->AbsorbObjects(event_AMS->Cls);
    //evt_Anc->fAMSCls = (TClonesArray*)event_AMS->Cls->Clone();
    int ClsSize = evt_Anc->fAMSCls->GetEntriesFast();
    for(int xx=0;xx<ClsSize;++xx){
      Cluster *aC = (Cluster*)evt_Anc->fAMSCls->At(xx);
      aC->ladder = LadderInOrder(aC->ladder);
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
  //delete evt_Rec0;
  return Conf::entries;
}
};

//-------------------------------------------------------------------
namespace AMS{
//-------------------------------------------------------------------
namespace Performance{ // without any cuts
  void Clusters(){
    vector<TH1F*>   h_clsSeed(NLadder*2);  // cluster seed
    vector<TH1F*>   h_clsNB(NLadder*2);  // cluster numbers
    vector<TH2F*>   h_COG_SNR(NLadder*2);  // COG, center of geometry. GetCoG(). SNR: signal inoise ratio of cluster. GetTotSN()
    vector<TH2F*>   h_COG_Length(NLadder*2);  // length of this cluster: GetLength()
    for(short i =0;i<NLadder; ++i){
      for(short j =0;j<2;++j){
        h_clsSeed[i*2+j] = new TH1F(Form("L%d_S%d--cluster seed",i,j),Form("L%d_S%d cluster seed",i,j),1024,0,1024);
        h_clsSeed[i*2+j]->SetLabelSize(0.12);
        h_clsSeed[i*2+j]->SetLabelSize(0.08,"Y");
        h_clsSeed[i*2+j]->SetLineColor(j+3);
        h_clsNB[i*2+j] = new TH1F(Form("L%d_S%d--cluster number",i,j),Form("L%d_S%d cluster number",i,j),8,0,8);
        h_clsNB[i*2+j]->SetLabelSize(0.12);
        h_clsNB[i*2+j]->SetLabelSize(0.08,"Y");
        h_COG_SNR[i*2+j] = new TH2F(Form("L%d_S%d--COG VS SNR",i,j),Form("L%d_S%d CoG VS SNR",i,j),1024*2,j*640,640+j*384,200,0,40);
        h_COG_SNR[i*2+j]->SetLabelSize(0.12);
        h_COG_SNR[i*2+j]->SetLabelSize(0.08,"Y");
        h_COG_Length[i*2+j]=new TH2F(Form("L%d_S%d--COG VS Length",i,j),Form("L%d_S%d CoG VS Length",i,j),1024*2,j*640,640+j*384,30,0,30);
        h_COG_Length[i*2+j]->SetLabelSize(0.12);
        h_COG_Length[i*2+j]->SetLabelSize(0.08,"Y");
      }
    }

    // event loop
    for(Conf::evtID =0;Conf::evtID<Conf::entries;++Conf::evtID){
      LoadEvent();
      vector<int> clusBN;
      ClusterNumberInLadder(clusBN);
      for(short id=0;id<NLadder;++id){
        for(short s=0;s<2;++s){
          h_clsNB[id*2+s]->Fill(clusBN[id*2+s]);
        }
      }
      int n_cls = Conf::AMS_Evt->Cls->GetEntriesFast();
      for(short ic=0;ic<n_cls;++ic){
        Cluster *aCluster = Conf::AMS_Evt->GetCluster(ic);
        short order = LadderInOrder(aCluster->ladder);
        short side = aCluster->side;
        float CoG = aCluster->GetCoG();
        h_COG_SNR[order*2+side]->Fill(CoG,aCluster->GetTotSN());
        h_COG_Length[order*2+side]->Fill(CoG,aCluster->GetLength());
        h_clsSeed[order*2+side]->Fill(aCluster->GetSeedAdd());
      }
    }

    // draw
    TCanvas *c0 = new TCanvas(Conf::File+"  Cluster seed",Conf::File+"  Cluster seed");
    c0->Divide(1,5,0,0);
    TCanvas *c1 = new TCanvas(Conf::File+"  Cluster number",Conf::File+"  Clusters number");
    c1->Divide(2,5,0,0);
    TCanvas *c2 = new TCanvas(Conf::File+"  Cluster CoG_SNR",Conf::File+"  Cluster CoG_SNR");
    c2->Divide(2,5,0,0);
    TCanvas *c3 = new TCanvas(Conf::File+"  Cluster CoG_Length",Conf::File+"  Cluster CoG_Length");
    c3->Divide(2,5,0,0);
    TCanvas *c4 = new TCanvas(Conf::File+"  Cluster SNR",Conf::File+"  Cluster SNR");
    c4->Divide(2,5,0,0);
    TCanvas *c5 = new TCanvas(Conf::File+"  Cluster Length",Conf::File+"  Cluster Length");
    c5->Divide(2,5,0,0);
    for(short id=0;id<NLadder;++id){
        c0->cd(id+1);
        h_clsSeed[id*2+0]->Draw();
        h_clsSeed[id*2+1]->Draw("same");
      for(short s=0;s<2;++s){
        c1->cd(id*2+s+1);
        h_clsNB[id*2+s]->Draw();
        c2->cd(id*2+s+1);
        h_COG_SNR[id*2+s]->Draw("colz");
        //h_COG_SNR[id][s]->ProfileX()->Draw("same");
        c3->cd(id*2+s+1);
        h_COG_Length[id*2+s]->Draw("colz");
        c4->cd(id*2+s+1);
        TH1D *h_SNR = h_COG_SNR[id*2+s]->ProjectionY();
        h_SNR->SetTitle(Form("L%d_S%d Cluster SNR",id,s));
        h_SNR->SetLabelSize(0.12);
        h_SNR->SetLabelSize(0.08,"Y");
        gPad->SetLogy();
        h_SNR->Draw();
        c5->cd(id*2+s+1);
        TH1D *h_Length = h_COG_Length[id*2+s]->ProjectionY();
        h_Length->SetTitle(Form("L%d_S%d Cluster Length",id,s));
        h_Length->SetLabelSize(0.12);
        h_Length->SetLabelSize(0.08,"Y");
        gPad->SetLogy();
        h_Length->Draw();
      }
    }
  }

};

//-------------------------------------------------------------------
namespace Alignment{
// *
// *  TODO: ios has not be declared
// *
  TString outFilename = "./Calibration/AMS/"+Conf::File+"_align.txt";
  //ofstream output(outFilename,ios::out | iso::app);
  //output<<"ladder id\tmean\terror";

  void SingleStrack_S_Side(bool reload=true){    // for each ladder
    gStyle->SetOptStat(1111);
    gStyle->SetOptFit(100111);

    TH1F *h_align_CoG[NLadder][3] = {{0}};  // cluster numbers, 0: s-side, 1: k-side-sensor0, 2: k-side-sensor-1
    h_align_CoG[0][0] = new TH1F("L0_S0 Reference Position","L0_S0 Reference Position",1500,0,10);
    h_align_CoG[0][1] = new TH1F("L0_S1_0 Reference Position","L0_S1 Reference Position",1500,0,10);
    h_align_CoG[0][2] = new TH1F("L0_S1_1 Reference Position","L0_S1 Reference Position",1500,0,10);
    for(short i =1;i<NLadder; ++i){
      h_align_CoG[i][0] = new TH1F(Form("L%d_S0 Offset",i),Form("L%d_S0 Offset",i),1000,-2.5,1.5);
      h_align_CoG[i][1] = new TH1F(Form("L%d_S1_0 Offset",i),Form("L%d_S0 Offset",i),1000,-2.5,1.5);
      h_align_CoG[i][2] = new TH1F(Form("L%d_S1_1 Offset",i),Form("L%d_S0 Offset",i),1000,-2.5,1.5);
    }

    TH2F *h_offset_SeedAdd[NLadder][3] = {{0}};  // cluster numbers, 0: s-side, 1: k-side-sensor0, 2: k-side-sensor-1
    for(short i =0;i<NLadder-1; ++i){
      h_offset_SeedAdd[i][0] = new TH2F(Form("L%d_S0 Offset_SeedAdd",i+1),  Form("L%d_S0 Offset_SeedAdd",i+1),640,0,640,1000,-2.5,1.5);
      h_offset_SeedAdd[i][1] = new TH2F(Form("L%d_S1_0 Offset_SeedAdd",i+1),Form("L%d_S1 Offset_SeedAdd",i+1),384,640,1024,1000,-2.5,1.5);
      h_offset_SeedAdd[i][2] = new TH2F(Form("L%d_S1_1 Offset_SeedAdd",i+1),Form("L%d_S1 Offset_SeedAdd",i+1),384,640,1024,1000,-2.5,1.5);
    }

    for(Conf::evtID =0;Conf::evtID<Conf::entries;++Conf::evtID){
      LoadEvent();
      // one track event
      if(! N_ClustersInLadder_I(1,0)){   // both sides
        continue;
      }
      if(! ClusterNumberLessThan2_forAllS_Side()){
        continue;
      }
      // update reference
      float Posi_Ref_ladder0[2]={0.0,0.};   // ladder 0, side 0, 1
      short k_Ref_SensorID = 0;       // for k-side, short(long) ladder has 2(6) group, one group contains 2 silicon sensors. While alignmenting, we'd use vertical tracks (means, clusters of ladder 1~4 must in the same sensor(0 or 1) as the cluster which belongs to ladder 0)
      int n_cls = Conf::AMS_Evt->Cls->GetEntriesFast();
      for(short ic=0;ic<n_cls;++ic){
        Cluster *aCluster = Conf::AMS_Evt->GetCluster(ic);
        short order = LadderInOrder(aCluster->ladder);
        if(order != 0){
          continue;
        }
        short side = aCluster->side;
        Posi_Ref_ladder0[side] = GetPosition(aCluster,false);
        if(side == 1 && aCluster->GetSeedAdd()>(640+192)){  // 640: number of s-side readout strips(one sensor), 192: number of k-side readout strips (one sensor)
          k_Ref_SensorID = 1;
        }
        h_align_CoG[0][side+(side ==1 ? k_Ref_SensorID : 0)]->Fill(Posi_Ref_ladder0[side]);
      }
      // alignment
      for(short ic=0;ic<n_cls;++ic){
        Cluster *aCluster = Conf::AMS_Evt->GetCluster(ic);
        short order = LadderInOrder(aCluster->ladder);
        if(order == 0){
          continue;
        }
        short side = aCluster->side;
        float offV = GetPosition(aCluster,false) - Posi_Ref_ladder0[side];
        if(side == 1){
          short k_SensorID = (aCluster->GetSeedAdd()<(640+192)) ? 0 : 1;
          if(k_SensorID == k_Ref_SensorID){
            h_align_CoG[order][side + k_SensorID]->Fill(offV);
            h_offset_SeedAdd[order-1][side+k_SensorID]->Fill(aCluster->GetSeedAdd(),offV);
          }
        }else{
          h_align_CoG[order][side]->Fill(offV);
          h_offset_SeedAdd[order-1][side]->Fill(aCluster->GetSeedAdd(),offV);
        }
      }
    }

    TCanvas *c0 = new TCanvas(Conf::File+"  Alignment Ref.",Conf::File+"  Alignment Ref.");
    c0->Divide(1,2);
    h_align_CoG[0][0]->SetXTitle("X / cm");
    h_align_CoG[0][1]->SetXTitle("Y / cm");
    for(short s=0;s<2;++s){
      c0->cd(s+1);
      gPad->SetLogy();
      h_align_CoG[0][s]->SetLabelSize(0.06);
      h_align_CoG[0][s]->SetLabelSize(0.04,"Y");
      h_align_CoG[0][s]->SetTitleSize(0.05,"X");
      h_align_CoG[0][s]->Draw();
      float mean = h_align_CoG[0][s]->GetMean(), rms = h_align_CoG[0][s]->GetRMS();
      Conf::gausFit->SetRange(mean-rms,mean+rms);
      h_align_CoG[0][s]->Fit(Conf::gausFit,"R0Q");
      Conf::gausFit->DrawCopy("lsame");
      if(s ==1){
        h_align_CoG[0][2]->SetLineColor(3);
        h_align_CoG[0][2]->Draw("same");
      }
    }

    TCanvas *c1 = new TCanvas(Conf::File+"  Alignment",Conf::File+"  Alignment");
    c1->Divide(2,4,0.,0.0);
    for(short id=1;id<NLadder;++id){
      for(short s=0;s<2;++s){
        c1->cd((id-1)*2+s+1);
        gPad->SetLogy();
        if(s==0){
          h_align_CoG[id][s]->SetXTitle("X / cm");
        }else{
          h_align_CoG[id][s]->SetXTitle("Y / cm");
        }
        h_align_CoG[id][s]->SetLabelSize(0.12);
        h_align_CoG[id][s]->SetLabelSize(0.08,"Y");
        h_align_CoG[id][s]->SetTitleSize(0.04,"X");
        h_align_CoG[id][s]->Draw();
        float mean = h_align_CoG[id][s]->GetMean(), rms = h_align_CoG[id][s]->GetRMS();
        Conf::gausFit->SetRange(mean-rms,mean+rms);
        h_align_CoG[id][s]->Fit(Conf::gausFit,"R0Q");
        Conf::gausFit->DrawCopy("lsame");
        //output<<
      }
      c1->cd((id-1)*2+2);
      h_align_CoG[id][2]->SetLineColor(3);
      h_align_CoG[id][2]->Draw("same");
    }
    TCanvas *c2 = new TCanvas(Conf::File+"  Offset_SeedAdd",Conf::File+"  Offset_SeedAdd");
    c2->Divide(2,4,0.,0.0);
    for(short id=0;id<NLadder-1;++id){
      for(short s=0;s<2;++s){
        c2->cd(id*2+s+1);
        gStyle->SetOptStat(11111111);
        gStyle->SetOptFit(111111111);
        h_offset_SeedAdd[id][s]->SetXTitle("Seed ID");
        h_offset_SeedAdd[id][s]->SetYTitle("Offset / cm");
        h_offset_SeedAdd[id][s]->SetLabelSize(0.12);
        h_offset_SeedAdd[id][s]->SetLabelSize(0.08,"Y");
        //h_offset_SeedAdd[id][s]->SetTitleSize(0.04,"Y");
        h_offset_SeedAdd[id][s]->Draw("colz");
        //h_offset_SeedAdd[id][s]->ProfileX()->Fit(Conf::linearFit,"0Q");//QF
        //Conf::linearFit->DrawCopy("same");
        //output<<
      }
      c2->cd(id*2+2);
      h_offset_SeedAdd[id][2]->Draw("same");
    }
    if(reload) Conf::LoadAlignmentParameter(outFilename); 
  }
}

//-------------------------------------------------------------------
namespace Tracking{
  TCanvas *c_track = 0;
  long trackID = 0;

//-------------------------------------------------------------------
  void Initi(){
    c_track = new TCanvas("AMS Track",Conf::File+"__AMS Track");
    c_track->Divide(2,1);
  }

//-------------------------------------------------------------------
  void FitTrack_XZ(vector<Cluster*> &xClus,double &p0, double &p1, vector<float> &xPosi,vector<float> &zPosi){
    gStyle->SetOptStat(00000000);
    gStyle->SetOptFit(000000000);
  //void FitTrack_XZ(vector<Cluster*> &xClus,double &p0, double &p1, vector<float> &xPosi,vector<float> &zPosi, vector<float> &totSig = vector<float>(0)){
       /*
        * input is all x Clusters(s side),
        *
        *  output are:
        *   linear fit parameter
        *   and, clusters' position and tot Sig,
        *
        */
    vector<float> totSig;
    p0 = -999; p1 = -999;
    xPosi.clear();
    zPosi.clear();
    totSig.clear();
    TH2F *track_xz = new TH2F(Form("Event%09ld track_xz",Conf::evtID),Form("Event%09ld track_xz",Conf::evtID),(Conf::ExHall*65+40),-10,Conf::ExHall*650+400,500,0,10);
    if(trackID == 0){
      track_xz->SetTitle("Track X-Z");
      track_xz->SetXTitle("Z / cm");
      track_xz->SetYTitle("X / cm");
    }
    track_xz->SetMarkerSize(4);
    for(unsigned short i=0;i<xClus.size();++i){
      xPosi.push_back(GetPosition(xClus[i]));
      zPosi.push_back(Conf::Position_Z[Conf::ExHall][LadderInOrder(xClus[i]->ladder)]);
      totSig.push_back(xClus[i]->GetTotSig());
      track_xz->Fill(zPosi[i],xPosi[i],totSig[i]);
    }
    track_xz->Fit(Conf::linearFit,"0Q");//QF
    if(trackID%100 ==0){
      c_track->cd(1);
      track_xz->Draw("same");
      Conf::linearFit->DrawCopy("same");
    }
    p0 = Conf::linearFit->GetParameter(0);
    p1 = Conf::linearFit->GetParameter(1);
  }

//-------------------------------------------------------------------
  void FitTrack_YZ(vector<Cluster*> &yClus,double &p0, double &p1, vector<float> &yPosi,vector<float> &zPosi){
    gStyle->SetOptStat(00000000);
    gStyle->SetOptFit(000000000);
  //void FitTrack_YZ(vector<Cluster*> &yClus,double &p0, double &p1, vector<float> &yPosi,vector<float> &zPosi, vector<float> &totSig = vector<float>(0)){
       /*
        * input is all y Clusters(k side),
        *
        *  output are:
        *   linear fit parameter
        *   and, clusters' position and tot Sig,
        *
        */
    vector<float> totSig;
    p0 = -999; p1 = -999;
    yPosi.clear();
    zPosi.clear();
    totSig.clear();
    TH2F *track_yz = new TH2F(Form("Event%09ld track_yz",Conf::evtID),Form("Event%09ld track_yz",Conf::evtID),(Conf::ExHall*65+40),-10,Conf::ExHall*650+400,500,0,50);
    if(trackID == 0){
      track_yz->SetTitle("Track Y-Z");
      track_yz->SetXTitle("Z / cm");
      track_yz->SetYTitle("Y / cm");
    }
    track_yz->SetMarkerSize(4);
    for(unsigned short i=0;i<yClus.size();++i){
      yPosi.push_back(GetPosition(yClus[i])); // back GetPos
      zPosi.push_back(Conf::Position_Z[Conf::ExHall][LadderInOrder(yClus[i]->ladder)]);
      totSig.push_back(yClus[i]->GetTotSig());
      track_yz->Fill(zPosi[i],yPosi[i],totSig[i]);
    }
    track_yz->Fit(Conf::linearFit,"0Q");//QF
    if(trackID%100 ==0){
      c_track->cd(2);
      track_yz->Draw("same");
      Conf::linearFit->DrawCopy("same");
    }
    p0 = Conf::linearFit->GetParameter(0);
    p1 = Conf::linearFit->GetParameter(1);
  }

//-------------------------------------------------------------------
  void Plots(long maxevt=999999999){
     Initi();
    // DX VS s-side strips
    //TH1F *h_p1[NLadder][2] = {{0}};  // linear fit parameter 1. 0: s-side, 1: k-side
    //TH1F *h_Dx_XPos[NLadder] = {0};  // 0: Dx VS s-side position.  Dx = hitted pos  - Fitted pos
    
    for(Conf::evtID =0;(Conf::evtID<Conf::entries && Conf::evtID<maxevt);++Conf::evtID){
      LoadEvent();
      // one track event
      if(! N_ClustersInLadder_I(1,0)){
        continue;
      }
      if(! ClusterNumberLessThan2_forAllS_Side()){
        continue;
      }

      vector<Cluster*>  goodClusters[2];    // s-side , k-side
      int n_cls = Conf::AMS_Evt->Cls->GetEntriesFast();
      for(short ic=0;ic<n_cls;++ic){
        Cluster *aCluster = Conf::AMS_Evt->GetCluster(ic);
        if( ! GoodClusterCheck(aCluster)){
          continue;
        }
        goodClusters[aCluster->side].push_back(aCluster);
      }
      if(goodClusters[0].size() <2 || goodClusters[1].size()<2){
        continue;
      }

      vector<float>  xz_Pos[2];    // (0,1): (x,z)
      double xz_p0,  xz_p1;
      FitTrack_XZ(goodClusters[0],xz_p0,xz_p1,xz_Pos[0],xz_Pos[1]);


      vector<float>  yz_Pos[2];    // (0,1): (y,z)
      double yz_p0,  yz_p1;
      FitTrack_YZ(goodClusters[1],yz_p0,yz_p1,yz_Pos[0],yz_Pos[1]);
      ++trackID;

    }
    // DX VS k-side strips
  }

};
};


