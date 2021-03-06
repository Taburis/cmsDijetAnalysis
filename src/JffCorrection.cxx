/*
 *  Implementation of the JffCorrection class
 */

#include "JffCorrection.h"

/*
 * Constructor for JffCorrection
 *
 * Arguments:
 *  bool ispp = True for pp, false for PbPb
 */
JffCorrection::JffCorrection(bool ispp){
  
  fIsPpData = ispp;
  
  if(fIsPpData) {
    fin = new TFile("jffcorr_ptcut50/Pythia6_jffcorr_file_May17.root");
  }
  else {
    //corr files id 145
    fin = new TFile("jffcorr_ptcut50/PythiaHydjetjffcorr_file_May17.root");
  }
  
  if(!fin) std::cout << "Input file for JffCorrection not found!! Aborting!!" << std::endl;
  else std::cout << "JffCorrection: Using correction file: "<< fin->GetName()<<std::endl;
  
  if(fIsPpData){
    for(int i=0; i<1; i++){
      f_param_a0[i] = (TF1*)fin->Get(Form("f2_par0_cent%d",i))->Clone(Form("f2_par0_cent%d",i));
      f_param_a1[i] = (TF1*)fin->Get(Form("f2_par1_cent%d",i))->Clone(Form("f2_par1_cent%d",i));
      flat_corr[i] = (TF1*)fin->Get(Form("f_flatcorr_cent%d",i))->Clone(Form("f_flatcorr_cent%d",i));
      f_pol1[i] = (TF1*)fin->Get(Form("f_reco_ratio_cent%d",i))->Clone(Form("f_reco_ratio_cent%d",i));
    }
  }
  
  else {
    for(int i=0; i<nCentBins; i++){
      f_param_a0[i] = (TF1*)fin->Get(Form("f2_par0_cent%d",i))->Clone(Form("f2_par0_cent%d",i));
      f_param_a1[i] = (TF1*)fin->Get(Form("f2_par1_cent%d",i))->Clone(Form("f2_par1_cent%d",i));
      flat_corr[i] = (TF1*)fin->Get(Form("f_flatcorr_cent%d",i))->Clone(Form("f_flatcorr_cent%d",i));
      f_pol1[i] = (TF1*)fin->Get(Form("f_reco_ratio_cent%d",i))->Clone(Form("f_reco_ratio_cent%d",i));
    }
  }
  
  centBins = new double[nCentBins+1];
  
  double tempCentBins[nCentBins+1] = {0,20,60,100,200};
  
  for(int i=0; i<=nCentBins; i++){
    centBins[i] = tempCentBins[i];
  }
  
}

/*
 * Get the Jet fragmentation function correction
 *
 * Arguments:
 *  int nCScand = Number of particle flow condidates in a jet
 *  int hiBin = CMS HI bin (centrality * 2)
 *  int jtpt = Uncorrected jet pT
 *  int jteta - Jet eta
 */
double JffCorrection::GetCorrection(int nCScand, int hiBin, double jtpt, double jteta){
  
  if(!fin){ std::cout << "Correction file is not loaded! No correction done!" << std::endl; return jtpt; }
  if(nCScand < 0) return jtpt; // No correction for generator level jets
  if(abs(jteta)>1.6) return jtpt; // TODO: It would be better to have correction upto 2. Now cannot be sure that leading jets are actually leading jets.
  if(jtpt>600 || jtpt<20) return jtpt;
  if((hiBin>200 || hiBin<0) && !fIsPpData){ std::cout << "Warning! hiBin is not between 0 and 200!! (=" << hiBin << ")" << std::endl; return jtpt; }
  
  int centBin=0;
  if(!fIsPpData){
    while(hiBin>centBins[centBin+1] && centBin<nCentBins-1){
      centBin++;
    }
  }
  
  //Apply nCS corrections
  double p0_cs2 = f_param_a0[centBin]->Eval(jtpt);
  double p1_cs2 = f_param_a1[centBin]->Eval(jtpt);
  
  
  double corr_factor = 1. + p1_cs2*(nCScand - p0_cs2);
  
  corrpt = jtpt/corr_factor;
  
  //Apply residual pol1 corrections to flatten
  corrpt /= (f_pol1[centBin]->Eval(jtpt));
  
  //Apply residual flat iterations to properly close refpt
  corrpt /= flat_corr[centBin]->GetParameter(0);
  
  return corrpt;
  
}
