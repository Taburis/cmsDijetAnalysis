#ifndef JFFCORRECTOR_H
#define JFFCORRECTOR_H

// Root includes
#include <TFile.h>
#include <TH1.h>

// Own includes
#include "DijetCard.h"
#include "DijetHistogramManager.h"

/*
 * Class for drawing the histograms produced in the dijet analysis
 */
class JffCorrector {

public:
  
  enum enumUncertaintySources{kBackgroundFluctuation, kFragmentationBias, kJetEnergyScale, kTrackingEfficiency, kResidualTracking, kPairAcceptance, kBackgroundSubtraction, kTotal, knUncertaintySources};
  TString uncertaintyName[knUncertaintySources] = {"backgroundFluctuation", "fragmentationBias", "jetEnergyScale", "trackingEfficiency", "residualTracking", "pairAcceptance", "backgroundSubtraction", "total"};
  
  JffCorrector();                                       // Default constructor
  JffCorrector(TFile *inputFile);                       // Constructor
  JffCorrector(TFile *inputFile, TFile* spilloverFile); // Constructor
  JffCorrector(const JffCorrector& in);                 // Copy constructor
  ~JffCorrector();                                      // Destructor
  
  // Setter for input file
  void ReadInputFile(TFile *inputFile);           // Read the histograms related to JFF correction
  void ReadSpilloverFile(TFile *spilloverFile);   // Read the histograms related to spillover correction
  void ReadSystematicFile(TFile *systematicFile); // Read thehistograms related to systematic uncertainties
  
  // Getters JFF correction histograms
  TH1D* GetJetShapeJffCorrection(const int iJetTrackCorrelation, const int iCentrality, const int iTrackPt, int iAsymmetry = DijetHistogramManager::kMaxAsymmetryBins) const;  // Jet shape JFF correction histograms
  TH2D* GetDeltaEtaDeltaPhiJffCorrection(const int iJetTrackCorrelation, const int iCentrality, const int iTrackPt, int iAsymmetry = DijetHistogramManager::kMaxAsymmetryBins) const;  // DeltaEta-DeltaPhi JFF correction histograms
  TH2D* GetDeltaEtaDeltaPhiSpilloverCorrection(const int iJetTrackCorrelation, const int iCentrality, const int iTrackPt, int iAsymmetry = DijetHistogramManager::kMaxAsymmetryBins) const;  // DeltaEta-DeltaPhi spillover correction histograms
  
  // Getters related to systematic uncertainties
  TString GetUncertaintyName(const int iUncertainty);
  TH1D* GetJetShapeSystematicUncertainty(const int iJetTrackCorrelation, const int iCentrality, const int iTrackPt, int iAsymmetry = DijetHistogramManager::kMaxAsymmetryBins, int iUncertainty = kTotal) const;
  
  // Return information, if correction is ready to be obtained
  bool CorrectionReady();  // True if histograms loaded from file, otherwise false
  bool SpilloverReady();   // True if spillover correction is loaded, otherwise false
  bool SystematicsReady(); // True if systematic uncertainties are loaded, otherwise false
  
private:
  
  // Data members
  bool fFileLoaded;                // Flag if the input file has been loaded
  int  fJffAsymmetryBins;          // Number of asymmetry bins in the JFF correction file
  bool fSpilloverLoaded;           // Flag if the spillover file has been loaded
  int  fSpilloverAsymmetryBins;    // Number of asymmetry bins in the spillover file
  bool fSystematicErrorLoaded;     // Flag if a systematic error file has been loaded
  int  fSystematicAsymmetryBins;   // Number of asymmetry bins in the systematic uncertainty file

  // JFF correction histograms for jet shape
  TH1D *fhJetShapeCorrection[DijetHistogramManager::knJetTrackCorrelations][DijetHistogramManager::kMaxAsymmetryBins+1][DijetHistogramManager::kMaxCentralityBins][DijetHistogramManager::kMaxTrackPtBins];  // JFF correction histograms for jet shape
  TH2D *fhDeltaEtaDeltaPhiCorrection[DijetHistogramManager::knJetTrackCorrelations][DijetHistogramManager::kMaxAsymmetryBins+1][DijetHistogramManager::kMaxCentralityBins][DijetHistogramManager::kMaxTrackPtBins];  // JFF correction histograms for deltaEta-deltaPhi histograms
  
  // Spillover correction
  TH2D *fhDeltaEtaDeltaPhiSpilloverCorrection[DijetHistogramManager::knJetTrackCorrelations][DijetHistogramManager::kMaxAsymmetryBins+1][DijetHistogramManager::kMaxCentralityBins][DijetHistogramManager::kMaxTrackPtBins];
  
  // Systematic uncertainty
  TH1D *fhJetShapeUncertainty[DijetHistogramManager::knJetTrackCorrelations][DijetHistogramManager::kMaxAsymmetryBins+1][DijetHistogramManager::kMaxCentralityBins][DijetHistogramManager::kMaxTrackPtBins][knUncertaintySources];
  
};

#endif
