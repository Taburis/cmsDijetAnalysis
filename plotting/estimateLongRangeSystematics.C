#include "DijetHistogramManager.h" R__LOAD_LIBRARY(plotting/DrawingClasses.so)
#include "DijetCard.h"
#include "DijetMethods.h"
#include "JDrawer.h"
#include "JffCorrector.h"

// Function definition for background refitter
TF1* refitBackground(TH1D *backgroundHistogram, int backgroundRebin, int nRefit);

/*
 * Macro for estimating systematic uncertainties for long range correlation results
 *
 * The following sources of systematic uncertainty are estimated:
 *  - Adjustment on the gluing point of leading/subleading distributions
 *  - Fit projections over regions -2.5 < deltaEta < -1.5 and 1.5 < deltaEta < 2.5 and compare to nominal
 *  - Fit projections over regions 1.5 < |deltaEta| < 2 and 2 < |deltaEta| < 2.5 and compare to nominal
 *  - Fit projections before and after mixed event correction
 *  - Fit different v_{z} selection (like positive and negative)
 *
 *  The results will be saved to a file and also a slide with different contribution can be printed
 */
void estimateLongRangeSystematics(){
  
  // ==================================================================
  // ========================= Configuration ==========================
  // ==================================================================
  
  // Main files from which the long range asymmetries are obtained
  TString pbpbFileName = "data/dijetPbPb2018_highForest_akFlowPuCs4PfJets_5eveMix_xjBins_wtaAxis_JECv4_modifiedSeagull_noErrorJff_averagePeakMixing_adjustedBackground_processed_2019-08-13_fiveJobsMissing.root";
  // data/dijetPbPb_pfCsJets_xjBins_wtaAxis_noUncOrInc_improvisedMixing_allCorrections_adjustedBackground_processed_2019-07-05.root
  TString ppFileName = "data/ppData2017_highForest_pfJets_20eventsMixed_xjBins_JECv2_averagePeakMixing_wtaAxis_allCorrections_adjustedBackground_processed_2019-08-13.root";
  // data/dijet_pp_highForest_pfJets_noUncOrInc_allCorrections_adjustedBackground_wtaAxis_processed_2019-07-13.root
  
  // For systematic uncertainty estimation, need files in which the background is not adjusted between leading and subleading side
  TString pbpbUnadjustedFileName = "data/dijetPbPb2018_highForest_akFlowPuCs4PfJets_5eveMix_xjBins_wtaAxis_JECv4_modifiedSeagull_noErrorJff_averagePeakMixing_processed_2019-08-13_fiveJobsMissing.root";
  // data/dijetPbPb_pfCsJets_xjBins_wtaAxis_noUncOrInc_improvisedMixing_allCorrections_processed_2019-07-05.root
  TString ppUnadjustedFileName = "data/ppData2017_highForest_pfJets_20eventsMixed_xjBins_JECv2_averagePeakMixing_wtaAxis_allCorrections_processed_2019-08-13.root";
  // data/dijet_pp_highForest_pfJets_noUncOrInc_allCorrections_wtaAxis_processed_2019-07-13.root
  
  // For the systemtic uncertainty, read also files where distributions are constructed only with positive or negative vz values
  TString positiveVzPbPbFileName = "data/dijetPbPb2018_highForest_akFlowPuCs4PfJets_5eveMix_xjBins_wtaAxis_JECv4_positiveVz_allCorrections_processed_2019-08-13_fiveJobsMissing.root";
  TString negativeVzPbPbFileName = "data/dijetPbPb2018_highForest_akFlowPuCs4PfJets_5eveMix_xjBins_wtaAxis_JECv4_negativeVz_allCorrections_processed_2019-08-13_fiveJobsMissing.root";
  TString positiveVzPpFileName = "data/ppData2017_highForest_pfJets_20eventsMixed_xjBins_JECv2_positiveVz_wtaAxis_allCorrections_processed_2019-08-13.root";
  TString negativeVzPpFileName = "data/ppData2017_highForest_pfJets_20eventsMixed_xjBins_JECv2_negativeVz_wtaAxis_allCorrections_processed_2019-08-13.root";
  
  const bool printUncertainties = true;
  
  const int nRefit = 4;    // Number of vn:s included in the refit
  int backgroundRebin = 4; // Rebin applied to the long range distributions
  
  bool plotExample = false;
  
  const char *outputFileName("data/untest.txt");
  // "data/vnUncertaintyPreliminary2018.txt"
  
  // ==================================================================
  // ====================== Configuration done ========================
  // ==================================================================
  
  // Make a JffCorrector to name the final corrections
  JffCorrector *nameProvider = new JffCorrector();
  
  // Open the input files
  TFile *pbpbDataFile = TFile::Open(pbpbFileName);
  TFile *ppDataFile = TFile::Open(ppFileName);
  
  // Open the files needed for systematic uncertainty estimation
  TFile *pbpbUnadjustedFile = TFile::Open(pbpbUnadjustedFileName);
  TFile *ppUnadjustedFile = TFile::Open(ppUnadjustedFileName);
  
  // Open files for positive and negative vz
  TFile *positiveVzPbPbFile = TFile::Open(positiveVzPbPbFileName);
  TFile *negativeVzPbPbFile = TFile::Open(negativeVzPbPbFileName);
  TFile *positiveVzPpFile = TFile::Open(positiveVzPpFileName);
  TFile *negativeVzPpFile = TFile::Open(negativeVzPpFileName);
  
  // Create readers for the histograms and define binning
  DijetHistogramManager *pbpbReader = new DijetHistogramManager(pbpbDataFile);
  DijetHistogramManager *ppReader = new DijetHistogramManager(ppDataFile);
  DijetHistogramManager *pbpbUnadjustedReader = new DijetHistogramManager(pbpbUnadjustedFile);
  DijetHistogramManager *ppUnadjustedReader = new DijetHistogramManager(ppUnadjustedFile);
  DijetHistogramManager *pbpbPositiveVzReader = new DijetHistogramManager(positiveVzPbPbFile);
  DijetHistogramManager *pbpbNegativeVzReader = new DijetHistogramManager(negativeVzPbPbFile);
  DijetHistogramManager *ppPositiveVzReader = new DijetHistogramManager(positiveVzPpFile);
  DijetHistogramManager *ppNegativeVzReader = new DijetHistogramManager(negativeVzPpFile);
  
  const int nCentralityBins = pbpbReader->GetNCentralityBins();
  const int nTrackPtBins = pbpbReader->GetNTrackPtBins();
  const int nAsymmetryBins = pbpbReader->GetNAsymmetryBins();
  double centralityBinBorders[] = {0,10,30,50,90};  // Bin borders for centrality
  double trackPtBinBorders[] = {0.7,1,2,3,4,8,12,300};  // Bin borders for track pT
  double xjBinBorders[] = {0,0.6,0.8,1}; // Bin borders for xj
  
  // Define a fitter to test fitting the background distribution with different amount of vn:s
  DijetMethods *refitter = new DijetMethods();
  
  // Define arrays for the histograms
  TH1D *background[nAsymmetryBins+1][nCentralityBins+1][nTrackPtBins];
  TF1 *backgroundFit[nAsymmetryBins+1][nCentralityBins+1][nTrackPtBins];
  
  TH1D *sameEventLongRange[nAsymmetryBins+1][nCentralityBins+1][nTrackPtBins];
  TF1 *sameEventFit[nAsymmetryBins+1][nCentralityBins+1][nTrackPtBins];
  
  // Two dimensional histogram needed for projections over different regions
  TString typeString[2] = {"SameEvent","Corrected"};
  TH2D *deltaPhiDeltaEtaDistribution[2][2][nAsymmetryBins+1][nCentralityBins+1][nTrackPtBins]; // First bin: leading/subleading. Second bin: same/corrected
  
  // Custom projections from different regions
  TH1D *positiveEtaProjection[2][nAsymmetryBins+1][nCentralityBins+1][nTrackPtBins];  // First bin: same/corrected
  TH1D *negativeEtaProjection[2][nAsymmetryBins+1][nCentralityBins+1][nTrackPtBins];  // First bin: same/corrected
  TH1D *closeEtaProjection[2][nAsymmetryBins+1][nCentralityBins+1][nTrackPtBins];     // First bin: same/corrected
  TH1D *farEtaProjection[2][nAsymmetryBins+1][nCentralityBins+1][nTrackPtBins];       // First bin: same/corrected
  TH1D *negativeVzProjection[nAsymmetryBins+1][nCentralityBins+1][nTrackPtBins];
  TH1D *positiveVzProjection[nAsymmetryBins+1][nCentralityBins+1][nTrackPtBins];
  
  // Fits to projections from different regions
  TF1 *positiveEtaFit[2][nAsymmetryBins+1][nCentralityBins+1][nTrackPtBins];  // First bin: same/corrected
  TF1 *negativeEtaFit[2][nAsymmetryBins+1][nCentralityBins+1][nTrackPtBins];  // First bin: same/corrected
  TF1 *closeEtaFit[2][nAsymmetryBins+1][nCentralityBins+1][nTrackPtBins];     // First bin: same/corrected
  TF1 *farEtaFit[2][nAsymmetryBins+1][nCentralityBins+1][nTrackPtBins];       // First bin: same/corrected
  TF1 *positiveVzFit[nAsymmetryBins+1][nCentralityBins+1][nTrackPtBins];
  TF1 *negativeVzFit[nAsymmetryBins+1][nCentralityBins+1][nTrackPtBins];
  
  // Define arrays needed for systematic uncertainty estimation
  TH1D *leadingUnadjustedBackground[nAsymmetryBins+1][nCentralityBins+1][nTrackPtBins];
  TH1D *leadingUnadjustedBackgroundOverlap[nAsymmetryBins+1][nCentralityBins+1][nTrackPtBins];
  
  // We will fit also the unadjusted histogram to estimate systematic uncertainties
  TF1 *unadjustedFit[nAsymmetryBins+1][nCentralityBins+1][nTrackPtBins];
  
  // Define arrays for extracted vn numbers
  double masterFlowTable[nAsymmetryBins+1][nCentralityBins+1][nTrackPtBins][nRefit];
  double masterFlowError[nAsymmetryBins+1][nCentralityBins+1][nTrackPtBins][nRefit];
  
  // Read the numbers also for unadjusted fits to estimate systematic uncertainties
  double unadjustedFlowTable[nAsymmetryBins+1][nCentralityBins+1][nTrackPtBins][nRefit];
  double unadjustedFlowError[nAsymmetryBins+1][nCentralityBins+1][nTrackPtBins][nRefit];
  
  // Read the numbers from same event long range correlation
  double sameEventFlowTable[nAsymmetryBins+1][nCentralityBins+1][nTrackPtBins][nRefit];
  double sameEventFlowError[nAsymmetryBins+1][nCentralityBins+1][nTrackPtBins][nRefit];
  
  // Tables for fit to different eta regions
  double positiveEtaTable[2][nAsymmetryBins+1][nCentralityBins+1][nTrackPtBins][nRefit]; // First bin: value, error
  double negativeEtaTable[2][nAsymmetryBins+1][nCentralityBins+1][nTrackPtBins][nRefit]; // First bin: value, error
  double closeEtaTable[2][nAsymmetryBins+1][nCentralityBins+1][nTrackPtBins][nRefit];    // First bin: value, error
  double farEtaTable[2][nAsymmetryBins+1][nCentralityBins+1][nTrackPtBins][nRefit];      // First bin: value, error
  
  // Tables for fits to positive and negative vz distributions
  double positiveVzTable[2][nAsymmetryBins+1][nCentralityBins+1][nTrackPtBins][nRefit];  // First bin: value, error
  double negativeVzTable[2][nAsymmetryBins+1][nCentralityBins+1][nTrackPtBins][nRefit];  // First bin: value, error
  
  // Make a big table with all relative and absolute uncertainties
  double relativeUncertaintyTable[JffCorrector::knLongRangeUncertaintySources][nAsymmetryBins+1][nCentralityBins+1][nTrackPtBins][nRefit];
  double absoluteUncertaintyTable[JffCorrector::knLongRangeUncertaintySources][nAsymmetryBins+1][nCentralityBins+1][nTrackPtBins][nRefit];
  
  // Read the histograms from the input files
  pbpbReader->SetLoadTrackLeadingJetCorrelations(true);
  pbpbReader->SetAsymmetryBinRange(0,nAsymmetryBins);
  pbpbReader->SetLoad2DHistograms(true);
  pbpbReader->LoadProcessedHistograms();
  
  ppReader->SetLoadTrackLeadingJetCorrelations(true);
  ppReader->SetAsymmetryBinRange(0,nAsymmetryBins);
  ppReader->SetLoad2DHistograms(true);
  ppReader->LoadProcessedHistograms();
  
  pbpbUnadjustedReader->SetLoadTrackLeadingJetCorrelations(true);
  pbpbUnadjustedReader->SetAsymmetryBinRange(0,nAsymmetryBins);
  pbpbUnadjustedReader->LoadProcessedHistograms();
  
  ppUnadjustedReader->SetLoadTrackLeadingJetCorrelations(true);
  ppUnadjustedReader->SetAsymmetryBinRange(0,nAsymmetryBins);
  ppUnadjustedReader->LoadProcessedHistograms();
  
  pbpbPositiveVzReader->SetLoadTrackLeadingJetCorrelations(true);
  pbpbPositiveVzReader->SetAsymmetryBinRange(0,nAsymmetryBins);
  pbpbPositiveVzReader->LoadProcessedHistograms();
  
  pbpbNegativeVzReader->SetLoadTrackLeadingJetCorrelations(true);
  pbpbNegativeVzReader->SetAsymmetryBinRange(0,nAsymmetryBins);
  pbpbNegativeVzReader->LoadProcessedHistograms();
  
  ppPositiveVzReader->SetLoadTrackLeadingJetCorrelations(true);
  ppPositiveVzReader->SetAsymmetryBinRange(0,nAsymmetryBins);
  ppPositiveVzReader->LoadProcessedHistograms();
  
  ppNegativeVzReader->SetLoadTrackLeadingJetCorrelations(true);
  ppNegativeVzReader->SetAsymmetryBinRange(0,nAsymmetryBins);
  ppNegativeVzReader->LoadProcessedHistograms();
  
  double backgroundLevel;
  for(int iCentrality = 0; iCentrality <= nCentralityBins; iCentrality++){
    
    for(int iTrackPt = 0; iTrackPt < nTrackPtBins; iTrackPt++){
      for(int iAsymmetry = 0; iAsymmetry <= nAsymmetryBins; iAsymmetry++){
        
        // Read the histograms for pp
        if(iCentrality == nCentralityBins){
          
          // Regular background histogram
          background[iAsymmetry][nCentralityBins][iTrackPt] = ppReader->GetHistogramJetTrackDeltaPhi(DijetHistogramManager::kTrackLeadingJet, DijetHistogramManager::kBackground, iAsymmetry, 0, iTrackPt, DijetHistogramManager::kWholeEta);
          
          // Track-leading jet unadjusted background histogram for systematic uncertainty estimation
          leadingUnadjustedBackground[iAsymmetry][nCentralityBins][iTrackPt] = ppUnadjustedReader->GetHistogramJetTrackDeltaPhi(DijetHistogramManager::kTrackLeadingJet, DijetHistogramManager::kBackground, iAsymmetry, 0, iTrackPt, DijetHistogramManager::kWholeEta);
          
          // Track-subleading jet unadjusted background histogram for systematic uncertainty estimation
          leadingUnadjustedBackgroundOverlap[iAsymmetry][nCentralityBins][iTrackPt] = ppUnadjustedReader->GetHistogramJetTrackDeltaPhi(DijetHistogramManager::kTrackLeadingJet, DijetHistogramManager::kBackgroundOverlap, iAsymmetry, 0, iTrackPt, DijetHistogramManager::kWholeEta);
          
          // Long range distribution for positive and negative vz for systematic uncertainty estimation
          negativeVzProjection[iAsymmetry][nCentralityBins][iTrackPt] = ppNegativeVzReader->GetHistogramJetTrackDeltaPhi(DijetHistogramManager::kTrackLeadingJet, DijetHistogramManager::kBackground, iAsymmetry, 0, iTrackPt, DijetHistogramManager::kWholeEta);
          
          positiveVzProjection[iAsymmetry][nCentralityBins][iTrackPt] = ppPositiveVzReader->GetHistogramJetTrackDeltaPhi(DijetHistogramManager::kTrackLeadingJet, DijetHistogramManager::kBackground, iAsymmetry, 0, iTrackPt, DijetHistogramManager::kWholeEta);
          
          // Same event histograms
          deltaPhiDeltaEtaDistribution[0][0][iAsymmetry][nCentralityBins][iTrackPt] = ppReader->GetHistogramJetTrackDeltaEtaDeltaPhi(DijetHistogramManager::kTrackLeadingJet, DijetHistogramManager::kSameEvent, iAsymmetry, 0, iTrackPt);
          
          deltaPhiDeltaEtaDistribution[1][0][iAsymmetry][nCentralityBins][iTrackPt] = ppReader->GetHistogramJetTrackDeltaEtaDeltaPhi(DijetHistogramManager::kTrackSubleadingJet, DijetHistogramManager::kSameEvent, iAsymmetry, 0, iTrackPt);
          
          // Mixed event corrected histograms
          deltaPhiDeltaEtaDistribution[0][1][iAsymmetry][nCentralityBins][iTrackPt] = ppReader->GetHistogramJetTrackDeltaEtaDeltaPhi(DijetHistogramManager::kTrackLeadingJet, DijetHistogramManager::kCorrected, iAsymmetry, 0, iTrackPt);
          
          deltaPhiDeltaEtaDistribution[1][1][iAsymmetry][nCentralityBins][iTrackPt] = ppReader->GetHistogramJetTrackDeltaEtaDeltaPhi(DijetHistogramManager::kTrackSubleadingJet, DijetHistogramManager::kCorrected, iAsymmetry, 0, iTrackPt);
          
        // Read the histograms for PbPb
        } else {
          
          // Regular background histogram
          background[iAsymmetry][iCentrality][iTrackPt] = pbpbReader->GetHistogramJetTrackDeltaPhi(DijetHistogramManager::kTrackLeadingJet, DijetHistogramManager::kBackground, iAsymmetry, iCentrality, iTrackPt, DijetHistogramManager::kWholeEta);
          
          // Track-leading jet unadjusted background histogram for systematic uncertainty estimation
          leadingUnadjustedBackground[iAsymmetry][iCentrality][iTrackPt] = pbpbUnadjustedReader->GetHistogramJetTrackDeltaPhi(DijetHistogramManager::kTrackLeadingJet, DijetHistogramManager::kBackground, iAsymmetry, iCentrality, iTrackPt, DijetHistogramManager::kWholeEta);
          
          // Track-subleading jet unadjusted background histogram for systematic uncertainty estimation
          leadingUnadjustedBackgroundOverlap[iAsymmetry][iCentrality][iTrackPt] = pbpbUnadjustedReader->GetHistogramJetTrackDeltaPhi(DijetHistogramManager::kTrackLeadingJet, DijetHistogramManager::kBackgroundOverlap, iAsymmetry, iCentrality, iTrackPt, DijetHistogramManager::kWholeEta);
          
          // Long range distribution for positive and negative vz for systematic uncertainty estimation
          negativeVzProjection[iAsymmetry][iCentrality][iTrackPt] = pbpbNegativeVzReader->GetHistogramJetTrackDeltaPhi(DijetHistogramManager::kTrackLeadingJet, DijetHistogramManager::kBackground, iAsymmetry, iCentrality, iTrackPt, DijetHistogramManager::kWholeEta);
          
          positiveVzProjection[iAsymmetry][iCentrality][iTrackPt] = pbpbPositiveVzReader->GetHistogramJetTrackDeltaPhi(DijetHistogramManager::kTrackLeadingJet, DijetHistogramManager::kBackground, iAsymmetry, iCentrality, iTrackPt, DijetHistogramManager::kWholeEta);
          
          // Same event histograms
          deltaPhiDeltaEtaDistribution[0][0][iAsymmetry][iCentrality][iTrackPt] = pbpbReader->GetHistogramJetTrackDeltaEtaDeltaPhi(DijetHistogramManager::kTrackLeadingJet, DijetHistogramManager::kSameEvent, iAsymmetry, iCentrality, iTrackPt);
          
          deltaPhiDeltaEtaDistribution[1][0][iAsymmetry][iCentrality][iTrackPt] = pbpbReader->GetHistogramJetTrackDeltaEtaDeltaPhi(DijetHistogramManager::kTrackSubleadingJet, DijetHistogramManager::kSameEvent, iAsymmetry, iCentrality, iTrackPt);
          
          // Mixed event corrected histograms
          deltaPhiDeltaEtaDistribution[0][1][iAsymmetry][iCentrality][iTrackPt] = pbpbReader->GetHistogramJetTrackDeltaEtaDeltaPhi(DijetHistogramManager::kTrackLeadingJet, DijetHistogramManager::kCorrected, iAsymmetry, iCentrality, iTrackPt);
          
          deltaPhiDeltaEtaDistribution[1][1][iAsymmetry][iCentrality][iTrackPt] = pbpbReader->GetHistogramJetTrackDeltaEtaDeltaPhi(DijetHistogramManager::kTrackSubleadingJet, DijetHistogramManager::kCorrected, iAsymmetry, iCentrality, iTrackPt);
          
        }
        
        // Do the projection for different eta regions from same event and corrected distributions
        for(int iType = 0; iType < 2; iType++){
        
          positiveEtaProjection[iType][iAsymmetry][iCentrality][iTrackPt] = refitter->CombineDeltaPhi(deltaPhiDeltaEtaDistribution[0][iType][iAsymmetry][iCentrality][iTrackPt], deltaPhiDeltaEtaDistribution[1][iType][iAsymmetry][iCentrality][iTrackPt], 1.5, 2.5, Form("positiveProjection%s%d%d%d",typeString[iType].Data(),iAsymmetry,iCentrality,iTrackPt), true);
          
          negativeEtaProjection[iType][iAsymmetry][iCentrality][iTrackPt] = refitter->CombineDeltaPhi(deltaPhiDeltaEtaDistribution[0][iType][iAsymmetry][iCentrality][iTrackPt], deltaPhiDeltaEtaDistribution[1][iType][iAsymmetry][iCentrality][iTrackPt], -2.5, -1.5, Form("negativeProjection%s%d%d%d",typeString[iType].Data(),iAsymmetry,iCentrality,iTrackPt), true);
          
          closeEtaProjection[iType][iAsymmetry][iCentrality][iTrackPt] = refitter->CombineDeltaPhi(deltaPhiDeltaEtaDistribution[0][iType][iAsymmetry][iCentrality][iTrackPt], deltaPhiDeltaEtaDistribution[1][iType][iAsymmetry][iCentrality][iTrackPt], 1.5, 2, Form("closeProjection%s%d%d%d",typeString[iType].Data(),iAsymmetry,iCentrality,iTrackPt));
          
          farEtaProjection[iType][iAsymmetry][iCentrality][iTrackPt] = refitter->CombineDeltaPhi(deltaPhiDeltaEtaDistribution[0][iType][iAsymmetry][iCentrality][iTrackPt], deltaPhiDeltaEtaDistribution[1][iType][iAsymmetry][iCentrality][iTrackPt], 2, 2.5, Form("farProjection%s%d%d%d",typeString[iType].Data(),iAsymmetry,iCentrality,iTrackPt));
          
          // Fit the projections with a Fourier fit
          positiveEtaProjection[iType][iAsymmetry][iCentrality][iTrackPt]->Rebin(backgroundRebin);
          positiveEtaProjection[iType][iAsymmetry][iCentrality][iTrackPt]->Scale(1.0/backgroundRebin);
          refitter->FourierFit(positiveEtaProjection[iType][iAsymmetry][iCentrality][iTrackPt], nRefit);
          positiveEtaFit[iType][iAsymmetry][iCentrality][iTrackPt] = positiveEtaProjection[iType][iAsymmetry][iCentrality][iTrackPt]->GetFunction("fourier");
          
          negativeEtaProjection[iType][iAsymmetry][iCentrality][iTrackPt]->Rebin(backgroundRebin);
          negativeEtaProjection[iType][iAsymmetry][iCentrality][iTrackPt]->Scale(1.0/backgroundRebin);
          refitter->FourierFit(negativeEtaProjection[iType][iAsymmetry][iCentrality][iTrackPt], nRefit);
          negativeEtaFit[iType][iAsymmetry][iCentrality][iTrackPt] = negativeEtaProjection[iType][iAsymmetry][iCentrality][iTrackPt]->GetFunction("fourier");
          
          closeEtaProjection[iType][iAsymmetry][iCentrality][iTrackPt]->Rebin(backgroundRebin);
          closeEtaProjection[iType][iAsymmetry][iCentrality][iTrackPt]->Scale(1.0/backgroundRebin);
          refitter->FourierFit(closeEtaProjection[iType][iAsymmetry][iCentrality][iTrackPt], nRefit);
          closeEtaFit[iType][iAsymmetry][iCentrality][iTrackPt] = closeEtaProjection[iType][iAsymmetry][iCentrality][iTrackPt]->GetFunction("fourier");
          
          farEtaProjection[iType][iAsymmetry][iCentrality][iTrackPt]->Rebin(backgroundRebin);
          farEtaProjection[iType][iAsymmetry][iCentrality][iTrackPt]->Scale(1.0/backgroundRebin);
          refitter->FourierFit(farEtaProjection[iType][iAsymmetry][iCentrality][iTrackPt], nRefit);
          farEtaFit[iType][iAsymmetry][iCentrality][iTrackPt] = farEtaProjection[iType][iAsymmetry][iCentrality][iTrackPt]->GetFunction("fourier");
          
          // TODO: Temporary plotter to get some plots
          if(plotExample && iType == 1 && iAsymmetry == nAsymmetryBins && iCentrality == 0 && iTrackPt == 2){
            JDrawer *drawer = new JDrawer();
            positiveEtaProjection[iType][iAsymmetry][iCentrality][iTrackPt]->SetLineColor(kRed);
            positiveEtaFit[iType][iAsymmetry][iCentrality][iTrackPt]->SetLineColor(kRed);
            negativeEtaProjection[iType][iAsymmetry][iCentrality][iTrackPt]->SetLineColor(kBlue);
            negativeEtaFit[iType][iAsymmetry][iCentrality][iTrackPt]->SetLineColor(kBlue);
            drawer->DrawHistogram(positiveEtaProjection[iType][iAsymmetry][iCentrality][iTrackPt],"#Delta#phi","#frac{dN}{d#Delta#phi}", " ");
            negativeEtaProjection[iType][iAsymmetry][iCentrality][iTrackPt]->Draw("same");
            TLegend *legend = new TLegend(0.39,0.72,0.59,0.92);
            legend->SetFillStyle(0);legend->SetBorderSize(0);legend->SetTextSize(0.05);legend->SetTextFont(62);
            legend->SetHeader("C: 0-10, 2 < p_{T} < 3 GeV");
            legend->AddEntry(positiveEtaProjection[iType][iAsymmetry][iCentrality][iTrackPt],"1.5 < #Delta#eta < 2.5","l");
            legend->AddEntry(negativeEtaProjection[iType][iAsymmetry][iCentrality][iTrackPt],"-2.5 < #Delta#eta < -1.5","l");
            legend->Draw();
            return;
          } // Drawing example plot
          
        }
        
        // Get the long range correlations directly from the raw distribution
        sameEventLongRange[iAsymmetry][iCentrality][iTrackPt] = refitter->CombineDeltaPhi(deltaPhiDeltaEtaDistribution[0][0][iAsymmetry][iCentrality][iTrackPt], deltaPhiDeltaEtaDistribution[1][0][iAsymmetry][iCentrality][iTrackPt], 1.5, 2.5, Form("sameProjection%d%d%d",iAsymmetry,iCentrality,iTrackPt));
        
        sameEventLongRange[iAsymmetry][iCentrality][iTrackPt]->Rebin(backgroundRebin);
        sameEventLongRange[iAsymmetry][iCentrality][iTrackPt]->Scale(1.0/backgroundRebin);
        
        refitter->FourierFit(sameEventLongRange[iAsymmetry][iCentrality][iTrackPt], nRefit);
        sameEventFit[iAsymmetry][iCentrality][iTrackPt] = sameEventLongRange[iAsymmetry][iCentrality][iTrackPt]->GetFunction("fourier");
        
        // Refit the long range distributions
        backgroundFit[iAsymmetry][iCentrality][iTrackPt] = refitBackground(background[iAsymmetry][iCentrality][iTrackPt], backgroundRebin, nRefit);
        unadjustedFit[iAsymmetry][iCentrality][iTrackPt] = refitBackground(leadingUnadjustedBackground[iAsymmetry][iCentrality][iTrackPt], backgroundRebin, nRefit);
        positiveVzFit[iAsymmetry][iCentrality][iTrackPt] = refitBackground(positiveVzProjection[iAsymmetry][iCentrality][iTrackPt], backgroundRebin, nRefit);
        negativeVzFit[iAsymmetry][iCentrality][iTrackPt] = refitBackground(negativeVzProjection[iAsymmetry][iCentrality][iTrackPt], backgroundRebin, nRefit);
        
      } // asymmetry loop
    } // track pT loop
  } // centrality loop
  
  // Extract the vn parameters from the fits
  for(int iCentrality = 0; iCentrality <= nCentralityBins; iCentrality++){
    for(int iTrackPt = 0; iTrackPt < nTrackPtBins; iTrackPt++){
      for(int iAsymmetry = 0; iAsymmetry <=nAsymmetryBins; iAsymmetry++){
        for(int iFlow = 0; iFlow < nRefit; iFlow++){
          masterFlowTable[iAsymmetry][iCentrality][iTrackPt][iFlow] = backgroundFit[iAsymmetry][iCentrality][iTrackPt]->GetParameter(iFlow+1);
          masterFlowError[iAsymmetry][iCentrality][iTrackPt][iFlow] = backgroundFit[iAsymmetry][iCentrality][iTrackPt]->GetParError(iFlow+1);
          unadjustedFlowTable[iAsymmetry][iCentrality][iTrackPt][iFlow] = unadjustedFit[iAsymmetry][iCentrality][iTrackPt]->GetParameter(iFlow+1);
          unadjustedFlowError[iAsymmetry][iCentrality][iTrackPt][iFlow] = unadjustedFit[iAsymmetry][iCentrality][iTrackPt]->GetParError(iFlow+1);
          
          sameEventFlowTable[iAsymmetry][iCentrality][iTrackPt][iFlow] = sameEventFit[iAsymmetry][iCentrality][iTrackPt]->GetParameter(iFlow+1);
          sameEventFlowError[iAsymmetry][iCentrality][iTrackPt][iFlow] = sameEventFit[iAsymmetry][iCentrality][iTrackPt]->GetParError(iFlow+1);
          
          positiveVzTable[0][iAsymmetry][iCentrality][iTrackPt][iFlow] = positiveVzFit[iAsymmetry][iCentrality][iTrackPt]->GetParameter(iFlow+1);
          positiveVzTable[1][iAsymmetry][iCentrality][iTrackPt][iFlow] = positiveVzFit[iAsymmetry][iCentrality][iTrackPt]->GetParError(iFlow+1);
          negativeVzTable[0][iAsymmetry][iCentrality][iTrackPt][iFlow] = negativeVzFit[iAsymmetry][iCentrality][iTrackPt]->GetParameter(iFlow+1);
          negativeVzTable[1][iAsymmetry][iCentrality][iTrackPt][iFlow] = negativeVzFit[iAsymmetry][iCentrality][iTrackPt]->GetParError(iFlow+1);
          
          if(positiveEtaFit[1][iAsymmetry][iCentrality][iTrackPt]){
            positiveEtaTable[0][iAsymmetry][iCentrality][iTrackPt][iFlow] = positiveEtaFit[1][iAsymmetry][iCentrality][iTrackPt]->GetParameter(iFlow+1);
            positiveEtaTable[1][iAsymmetry][iCentrality][iTrackPt][iFlow] = positiveEtaFit[1][iAsymmetry][iCentrality][iTrackPt]->GetParError(iFlow+1);
          } else {
            positiveEtaTable[0][iAsymmetry][iCentrality][iTrackPt][iFlow] = -999;
            positiveEtaTable[1][iAsymmetry][iCentrality][iTrackPt][iFlow] = -999;
          }
          
          if(negativeEtaFit[1][iAsymmetry][iCentrality][iTrackPt]){
            negativeEtaTable[0][iAsymmetry][iCentrality][iTrackPt][iFlow] = negativeEtaFit[1][iAsymmetry][iCentrality][iTrackPt]->GetParameter(iFlow+1);
            negativeEtaTable[1][iAsymmetry][iCentrality][iTrackPt][iFlow] = negativeEtaFit[1][iAsymmetry][iCentrality][iTrackPt]->GetParError(iFlow+1);
          } else {
            negativeEtaTable[0][iAsymmetry][iCentrality][iTrackPt][iFlow] = -999;
            negativeEtaTable[1][iAsymmetry][iCentrality][iTrackPt][iFlow] = -999;
          }
          
          if(closeEtaFit[1][iAsymmetry][iCentrality][iTrackPt]){
            closeEtaTable[0][iAsymmetry][iCentrality][iTrackPt][iFlow] = closeEtaFit[1][iAsymmetry][iCentrality][iTrackPt]->GetParameter(iFlow+1);
            closeEtaTable[1][iAsymmetry][iCentrality][iTrackPt][iFlow] = closeEtaFit[1][iAsymmetry][iCentrality][iTrackPt]->GetParError(iFlow+1);
          } else {
            closeEtaTable[0][iAsymmetry][iCentrality][iTrackPt][iFlow] = -999;
            closeEtaTable[1][iAsymmetry][iCentrality][iTrackPt][iFlow] = -999;
          }
          
          if(farEtaFit[1][iAsymmetry][iCentrality][iTrackPt]){
            farEtaTable[0][iAsymmetry][iCentrality][iTrackPt][iFlow] = farEtaFit[1][iAsymmetry][iCentrality][iTrackPt]->GetParameter(iFlow+1);
            farEtaTable[1][iAsymmetry][iCentrality][iTrackPt][iFlow] = farEtaFit[1][iAsymmetry][iCentrality][iTrackPt]->GetParError(iFlow+1);
          } else {
            farEtaTable[0][iAsymmetry][iCentrality][iTrackPt][iFlow] = -999;
            farEtaTable[1][iAsymmetry][iCentrality][iTrackPt][iFlow] = -999;
          }
          
        } // flow components
      } // Asymmetry loop
    } // track pT
  } // centrality
  
  double firstValue, secondValue;
  
  // Collect all the numbers in one big table
  for(int iAsymmetry = 0; iAsymmetry <= nAsymmetryBins; iAsymmetry++){
    for(int iCentrality = 0; iCentrality <= nCentralityBins; iCentrality++){
      for(int iTrackPt = 0; iTrackPt < nTrackPtBins; iTrackPt++){
        for(int iFlow = 0; iFlow < nRefit; iFlow++){
          
          // Put the value from same and mixed event comparison to the uncertainty tables
          relativeUncertaintyTable[JffCorrector::kSameMixed][iAsymmetry][iCentrality][iTrackPt][iFlow] = 0;//TMath::Abs(1-sameEventFlowTable[iAsymmetry][iCentrality][iTrackPt][iFlow]/unadjustedFlowTable[iAsymmetry][iCentrality][iTrackPt][iFlow]);
          absoluteUncertaintyTable[JffCorrector::kSameMixed][iAsymmetry][iCentrality][iTrackPt][iFlow] = 0;//TMath::Abs(sameEventFlowTable[iAsymmetry][iCentrality][iTrackPt][iFlow] - unadjustedFlowTable[iAsymmetry][iCentrality][iTrackPt][iFlow]);
          
          // Put the value from background adjustment to the uncertainty tables
          relativeUncertaintyTable[JffCorrector::kBackgroundGlue][iAsymmetry][iCentrality][iTrackPt][iFlow] = TMath::Abs(1-masterFlowTable[iAsymmetry][iCentrality][iTrackPt][iFlow]/unadjustedFlowTable[iAsymmetry][iCentrality][iTrackPt][iFlow]);
          absoluteUncertaintyTable[JffCorrector::kBackgroundGlue][iAsymmetry][iCentrality][iTrackPt][iFlow] = TMath::Abs(masterFlowTable[iAsymmetry][iCentrality][iTrackPt][iFlow] - unadjustedFlowTable[iAsymmetry][iCentrality][iTrackPt][iFlow]);
          
          // Put the value from comparison pasitive and negative eat sides to the uncertainty tables
          firstValue = TMath::Abs(1-positiveEtaTable[0][iAsymmetry][iCentrality][iTrackPt][iFlow]/unadjustedFlowTable[iAsymmetry][iCentrality][iTrackPt][iFlow]);
          secondValue = TMath::Abs(1-negativeEtaTable[0][iAsymmetry][iCentrality][iTrackPt][iFlow]/unadjustedFlowTable[iAsymmetry][iCentrality][iTrackPt][iFlow]);
          relativeUncertaintyTable[JffCorrector::kEtaSide][iAsymmetry][iCentrality][iTrackPt][iFlow] = TMath::Max(firstValue,secondValue);
          
          firstValue = TMath::Abs(positiveEtaTable[0][iAsymmetry][iCentrality][iTrackPt][iFlow] - unadjustedFlowTable[iAsymmetry][iCentrality][iTrackPt][iFlow]);
          secondValue = TMath::Abs(negativeEtaTable[0][iAsymmetry][iCentrality][iTrackPt][iFlow] - unadjustedFlowTable[iAsymmetry][iCentrality][iTrackPt][iFlow]);
          absoluteUncertaintyTable[JffCorrector::kEtaSide][iAsymmetry][iCentrality][iTrackPt][iFlow] = TMath::Max(firstValue,secondValue);
          
          // Put the value from comparing near and far eta region to the uncertainty tables
          firstValue = TMath::Abs(1-closeEtaTable[0][iAsymmetry][iCentrality][iTrackPt][iFlow]/unadjustedFlowTable[iAsymmetry][iCentrality][iTrackPt][iFlow]);
          secondValue = TMath::Abs(1-farEtaTable[0][iAsymmetry][iCentrality][iTrackPt][iFlow]/unadjustedFlowTable[iAsymmetry][iCentrality][iTrackPt][iFlow]);
          relativeUncertaintyTable[JffCorrector::kEtaRegion][iAsymmetry][iCentrality][iTrackPt][iFlow] = TMath::Max(firstValue,secondValue);
          
          firstValue = TMath::Abs(closeEtaTable[0][iAsymmetry][iCentrality][iTrackPt][iFlow] - unadjustedFlowTable[iAsymmetry][iCentrality][iTrackPt][iFlow]);
          secondValue = TMath::Abs(farEtaTable[0][iAsymmetry][iCentrality][iTrackPt][iFlow] - unadjustedFlowTable[iAsymmetry][iCentrality][iTrackPt][iFlow]);
          absoluteUncertaintyTable[JffCorrector::kEtaRegion][iAsymmetry][iCentrality][iTrackPt][iFlow] = TMath::Max(firstValue,secondValue);
          
          // Put a value for different vz regions to the uncertainty tables
          firstValue = TMath::Abs(1-positiveVzTable[0][iAsymmetry][iCentrality][iTrackPt][iFlow]/unadjustedFlowTable[iAsymmetry][iCentrality][iTrackPt][iFlow]);
          secondValue = TMath::Abs(1-negativeVzTable[0][iAsymmetry][iCentrality][iTrackPt][iFlow]/unadjustedFlowTable[iAsymmetry][iCentrality][iTrackPt][iFlow]);
          relativeUncertaintyTable[JffCorrector::kVzVariation][iAsymmetry][iCentrality][iTrackPt][iFlow] = TMath::Max(firstValue,secondValue);
          
          firstValue = TMath::Abs(positiveVzTable[0][iAsymmetry][iCentrality][iTrackPt][iFlow] - unadjustedFlowTable[iAsymmetry][iCentrality][iTrackPt][iFlow]);
          secondValue = TMath::Abs(negativeVzTable[0][iAsymmetry][iCentrality][iTrackPt][iFlow] - unadjustedFlowTable[iAsymmetry][iCentrality][iTrackPt][iFlow]);
          absoluteUncertaintyTable[JffCorrector::kVzVariation][iAsymmetry][iCentrality][iTrackPt][iFlow] = TMath::Max(firstValue,secondValue);
          
          // Calculate the total uncertainty from all the sources combined
          absoluteUncertaintyTable[JffCorrector::kTotalLongRange][iAsymmetry][iCentrality][iTrackPt][iFlow] = 0;
          for(int iUncertainty = 0; iUncertainty < JffCorrector::kTotalLongRange; iUncertainty++){
            absoluteUncertaintyTable[JffCorrector::kTotalLongRange][iAsymmetry][iCentrality][iTrackPt][iFlow] += TMath::Power(absoluteUncertaintyTable[iUncertainty][iAsymmetry][iCentrality][iTrackPt][iFlow], 2);
          }
          absoluteUncertaintyTable[JffCorrector::kTotalLongRange][iAsymmetry][iCentrality][iTrackPt][iFlow] = TMath::Sqrt(absoluteUncertaintyTable[JffCorrector::kTotalLongRange][iAsymmetry][iCentrality][iTrackPt][iFlow]);
          
          relativeUncertaintyTable[JffCorrector::kTotalLongRange][iAsymmetry][iCentrality][iTrackPt][iFlow] =  TMath::Abs(absoluteUncertaintyTable[JffCorrector::kTotalLongRange][iAsymmetry][iCentrality][iTrackPt][iFlow] / unadjustedFlowTable[iAsymmetry][iCentrality][iTrackPt][iFlow]);
        }
      }
    }
  } // Asymmetry loop
  
  // If an output file name is given, put the total uncertainties to an output file
  if(strcmp(outputFileName, "") != 0){
    std::ofstream outputFile;
    outputFile.open(outputFileName);
    
    // Print first the information about number of bins
    outputFile << nCentralityBins << " " << nRefit << " " << nAsymmetryBins << " " << nTrackPtBins << endl;
    
    // Next, print all the uncertainties in a specific order
    for(int iCentrality = 0; iCentrality <= nCentralityBins; iCentrality++){
      for(int iFlow = 0; iFlow < nRefit; iFlow++){
        for(int iAsymmetry = 0; iAsymmetry <= nAsymmetryBins; iAsymmetry++){
          for(int iTrackPt = 0; iTrackPt < nTrackPtBins; iTrackPt++){
            outputFile << absoluteUncertaintyTable[JffCorrector::kTotalLongRange][iAsymmetry][iCentrality][iTrackPt][iFlow] << " ";
          } // Track pT loop
          outputFile << endl;
        } // Asymmetry loop
      } // Flow component loop
    } // Centrality loop
    
    outputFile.close();
  }
  
  if(printUncertainties){
    
    // Print a slide with uncertainties for each source and each centrality for each V
    char namer[100];
    for(int iFlow = 0; iFlow < 3; iFlow++){
      for(int iTrackPt = 0; iTrackPt < 5; iTrackPt++){
        
        sprintf(namer,"\\frametitle{Uncertainty for $V_{%d}$, $%.1f < p_{\\mathrm{T}} < %.1f$}", iFlow+1, trackPtBinBorders[iTrackPt], trackPtBinBorders[iTrackPt+1]);
        
        cout << endl;
        cout << "\\begin{frame}" << endl;
        cout << namer << endl;
        cout << "\\begin{center}" << endl;
        cout << "  \\begin{tabular}{cccccc}" << endl;
        cout << "    \\toprule" << endl;
        cout << "    Source & C: 0-10 & C: 10-30 & C: 30-50 & pp \\\\" << endl;
        cout << "    \\midrule" << endl;
        
        // Set the correct precision for printing floating point numbers
        cout << fixed << setprecision(3);
        
        // First, print the actual values of vn:s
        cout << "$V_{" << iFlow+1 << "}$ ";
        
        for(int iCentrality = 0; iCentrality <= nCentralityBins; iCentrality++){
          if(iCentrality == nCentralityBins - 1) continue;
          cout << " & $" << unadjustedFlowTable[nAsymmetryBins][iCentrality][iTrackPt][iFlow] << "$";
        }
        cout << " \\\\" << endl;
        cout << "    \\midrule" << endl;
        
        // Print the relative uncertainties
        
        for(int iUncertainty = 0; iUncertainty < JffCorrector::knLongRangeUncertaintySources; iUncertainty++){
          cout << nameProvider->GetLongRangeUncertaintyName(iUncertainty);
          for(int iCentrality = 0; iCentrality <= nCentralityBins; iCentrality++){
            if(iCentrality == nCentralityBins - 1) continue;
            cout << " & $" << relativeUncertaintyTable[iUncertainty][nAsymmetryBins][iCentrality][iTrackPt][iFlow] << "$";
          }
          cout << " \\\\" << endl;
        }
        
        cout << "    \\midrule" << endl;
        
        // Print the absolute uncertainties
        
        for(int iUncertainty = 0; iUncertainty < JffCorrector::knLongRangeUncertaintySources; iUncertainty++){
          cout << nameProvider->GetLongRangeUncertaintyName(iUncertainty);
          for(int iCentrality = 0; iCentrality <= nCentralityBins; iCentrality++){
            if(iCentrality == nCentralityBins - 1) continue;
            cout << " & $" << absoluteUncertaintyTable[iUncertainty][nAsymmetryBins][iCentrality][iTrackPt][iFlow] << "$";
          }
          cout << " \\\\" << endl;
        }
        
        
        cout << "    \\bottomrule" << endl;
        cout << "  \\end{tabular}" << endl;
        cout << "\\end{center}" << endl;
        cout << "\\begin{itemize}" << endl;
        cout << "  \\item Top: value. Middle: relative uncertainty. Bottom: absolute uncertainty." << endl;
        cout << "\\end{itemize}" << endl;
        cout << "\\end{frame}" << endl;
        cout << endl;
        
      } // Track pT loop
    } // vn loop*/
    
  } // Printing uncertainties
}

/*
 * Refit the background histogram with a Fourier fit
 */
TF1* refitBackground(TH1D *backgroundHistogram, int backgroundRebin, int nRefit){
  
  DijetMethods *refitter = new DijetMethods();
  
  // Find the fourier fit function from the histogram
  TF1 *backgroundFit = backgroundHistogram->GetFunction("fourier");
  
  // Rebin the background and remove previous fit
  backgroundHistogram->RecursiveRemove(backgroundFit);
  backgroundHistogram->Rebin(backgroundRebin);
  backgroundHistogram->Scale(1.0/backgroundRebin);
  
  // Make the new fit to the distribution
  refitter->FourierFit(backgroundHistogram, nRefit);
  backgroundFit = backgroundHistogram->GetFunction("fourier");
  
  // Delete the refitter and return the fitted function
  delete refitter;
  return backgroundFit;
}
