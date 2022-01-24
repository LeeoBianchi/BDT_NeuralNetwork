#include <iostream>
#include <fstream>
#include <cstring>
#include <vector>
#include <TFile.h>
#include <TTree.h>
#include <TGraph.h>
#include <TMultiGraph.h>
#include <TLegend.h>
#include <TApplication.h>
#include <TCanvas.h>
#include <TH1D.h>
#include "Event.h"
#include <TMVA/Reader.h>

//global variables declaration main
int nBDT = 15; //number of different boosting iteration number values
std::vector <int> niter(nBDT, 0); //number of boosting iteration values vector
std::ofstream o;
int nSig, nBkg;

std::vector <int> MVA (char* imputfile, char* title){

// Set up an output file and book some histograms
  TFile* histFile = new TFile("analysis.root", "RECREATE");
  TH1D* hBDT200Sig = new TH1D("hBDT200Sig", title , 100, -2., 2.);
  TH1D* hBDT200Bkg = new TH1D("hBDT200Bkg", title, 100, -2., 2.);

  std::vector<int> err (nBDT, 0);  //number of events on the wrong side of the cut,
                                   //for every different BDT, it's the MVA function output
  std::vector<std::string> iter;
// Set up the TMVA Reader object.
// The names in AddVariable must be same as in the input (weight) file.

  TMVA::Reader* reader = new TMVA::Reader();
  float x, y, z;                       // TMVA needs float, not double
  reader->AddVariable("x", &x);
  reader->AddVariable("y", &y);
  reader->AddVariable("z", &z);
  std::string dir    = "../train/dataset/weights/";
  std::string prefix = "tmvaTest";
  reader->BookMVA("BDT200", dir + prefix + "_BDT200.weights.xml");

  //b) BDTs with different iterations number values
  for (int i=0; i <nBDT; i++){
    niter[i] = pow(10.,(double) i*5./nBDT);
    std::ostringstream strs;
    strs<<niter[i];
    iter.push_back(strs.str());
    reader->BookMVA("BDT"+iter[i],dir + prefix + "_BDT"+iter[i]+".weights.xml");
  }

// Open input file, get the TTrees, put into a vector

  TFile* inputFile = new TFile(imputfile);
  inputFile->ls();
  TTree* sig = dynamic_cast<TTree*>(inputFile->Get("sig"));
  TTree* bkg = dynamic_cast<TTree*>(inputFile->Get("bkg"));
  std::vector<TTree*> treeVec;
  treeVec.push_back(sig);
  treeVec.push_back(bkg);

  std::cout << "treeVec size = " << treeVec.size() << std::endl;

// Loop over TTrees

  const double tCut = 0.0;
  int s=0;
  int b=0;
  int p=0;
  int numEntries= 0;
  for (unsigned int i=0; i<treeVec.size(); i++){

    treeVec[i]->Print();
    Event evt;
    treeVec[i]->SetBranchAddress("evt", &evt);
    numEntries = treeVec[i]->GetEntries();
    if ( i == 0 ) { nSig = numEntries; }
    if ( i == 1 ) { nBkg = numEntries; }
    std::cout << "number of entries = " << numEntries << std::endl;

// Loop over events.  The test statistic is identified by the first
// argument used above in BookMVA (below, e.g., "Fisher").

    for (int j=0; j<numEntries; j++){
      treeVec[i]->GetEntry(j);                // sets evt
      x = evt.x;                              // set variables of reader
      y = evt.y;
      z = evt.z;

// a) BDT with 200 iterations
      double t = reader->EvaluateMVA("BDT200");

      if (i==0){           //signal
        if ( t>tCut ){
            hBDT200Sig->Fill(t);
            s++;
          }
        else if (t<tCut ){
            hBDT200Bkg->Fill(t);
        }
      }
      if (i==1){          //background
        if ( t>tCut ){
      	    hBDT200Sig->Fill(t);
      	    p++;
        }
        else if (t<tCut ){
      	    hBDT200Bkg->Fill(t);
      	    b++;
        }
      }

// b) we use different iterations number values
      for (int k=0; k <nBDT; k++){
       t = reader->EvaluateMVA("BDT"+iter[k]);
        if (i==0){           //signal
           if ( t>tCut ){
           }
           else if (t<tCut ){
             err[k]++;
           }
        }
        if (i==1){          //background
           if ( t>tCut ){
             err[k]++;
           }
           else if (t<tCut ){
           }
        }
      }
    }
  }
    //a) I create some histograms and print the results on output files.

  TCanvas* BDT200 = new TCanvas("BDT200", "BDT200", 700, 500);
  TLegend*leg = new TLegend(1.1, 1000., 2.4, 1367, "Legend", "b");
  hBDT200Bkg->Draw();
  hBDT200Sig->Draw("same");
  hBDT200Bkg->SetLineColor(2);
  leg->AddEntry(hBDT200Sig,"Signal","l");
  leg->AddEntry(hBDT200Bkg,"Background","l");
  leg->Draw();
  BDT200->Draw();
  BDT200->SaveAs(title, ".png");

  double eff_s=(double) s/nSig;
  double eff_b= (double) b/nBkg;
  double pur = (double) nSig / (s+p) *eff_s;
  o<<"BDT200: efficienza segnale = "<<eff_s<<";"<<std::endl;
  o<<"efficienza di reiezione del background = "<<eff_b<<";"<<std::endl;
  o<<"purezza del segnale= "<<pur<<std::endl;
  o<<std::endl;

  BDT200->Close();
  histFile->Write();
  histFile->Close();
  delete BDT200;
  return err;
}

int main(int argc, char* argv[]) {

  o.open("output22.txt");

  //training dataset
  o<<"CAMPIONE DI TRAINING "<<std::endl;
  o<<std::endl;
  TGraph* train = new TGraph();
  int a = niter[0];
  std::vector<int>err = MVA((char *)"../generate/trainingData.root",(char *) "BDT200-training data");
  int e = err[0];
  for (int k=0; k <nBDT; k++){
    if(err[k]<e){a=niter[k]; e=err[k];}
    train->SetPoint(k, niter[k], (double) err[k]/(nSig+nBkg));
  }

  //test dataset
  o<<std::endl;
  o<<"CAMPIONE DI TEST"<<std::endl;
  o<<std::endl;
  TGraph* test = new TGraph();
  a = niter[0];
  err = MVA((char *)"../generate/testData.root",(char *) "BDT200-test data");
  e = err[0];
  for (int k=0; k <nBDT; k++){
    if(err[k]<e){a=niter[k]; e=err[k];}
    test->SetPoint(k, niter[k], (double) err[k]/(nSig+nBkg));
  }
  o<<"Il numero ottimale di iterazioni di boosting è circa "<<a<<"."<<std::endl;

  //both the training and test function on the same canvas
  TApplication App("app", &argc, argv);
  TCanvas* c = new TCanvas("c");
  TMultiGraph *mg = new TMultiGraph();
    c->Draw();
    c->SetTitle("Total error in function of BDT iterations number");
    c->SetLogx();
    test->SetLineColor(2);
    mg->Add(train);
    mg->Add(test);
    mg->Draw("AL*");

  //legend
  TLegend*leg = new TLegend(0.31, 0.007, 2.61, 0.025, "Total error in function of #iter", "b");
    leg->AddEntry(test,"test data","l");
    leg->AddEntry(train,"training data","l");
    leg->Draw();

  c->SaveAs("plot22.png");
  o.close();
  App.Run();
  return 0;

}

