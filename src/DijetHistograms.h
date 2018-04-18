// Class for histograms needed in the dijet analysis

#ifndef DIJETHISTOGRAMS_H
#define DIJETHISTOGRAMS_H

// Root includes
#include <TH1.h>
#include <TH2.h>
#include <THnSparse.h>

// Own includes
#include "ConfigurationCard.h"

class DijetHistograms{
  
public:
  
  // Enumeration for event types to event histogram
  enum enumEventTypes {kAll, kVzCut, kDijet, knEventTypes};
  
  // Constructors and destructor
  DijetHistograms(); // Default constructor
  DijetHistograms(ConfigurationCard *newCard); // Custom constructor
  DijetHistograms(const DijetHistograms& in); // Copy constructor
  virtual ~DijetHistograms(); // Destructor
  DijetHistograms& operator=(const DijetHistograms& obj); // Equal sign operator
  
  // Methods
  void CreateHistograms();                   // Create all histograms
  void Write() const;                        // Write the histograms to a file that is opened somewhere else
  void Write(TString outputFileName) const;  // Write the histograms to a file
  void SetCard(ConfigurationCard *newCard);  // Set a new configuration card for the histogram class
  
  // Histograms defined public to allow easier access to them. Should not be abused
  TH1D *fhVertexZ;                          // Vertex z-position
  TH1D *fhEvents;                           // Number of events. Bin 1 = all. Bin 2 = good vz bin. Bin 3 = Dijet found.
  TH1D *fhCentrality;                       // Centrality information. -0.5 for pp or PYTHIA.
  THnSparseD *fhDijetDphi;                  // deltaPhi for dijet events
  THnSparseD *fhDijetAsymmetry;             // Asymmetry in dijet events
  THnSparseD *fhLeadingJetPt;               // pT for the leading jet
  THnSparseD *fhSubleadingJetPt;            // pT for the subleading jet
  THnSparseD *fhAnyJetPt;                   // pT for all jets
  THnSparseD *fhLeadingJetPhi;              // phi for the leading jet
  THnSparseD *fhSubleadingJetPhi;           // phi for the subleading jet
  THnSparseD *fhAnyJetPhi;                  // phi for all jets
  THnSparseD *fhLeadingJetEta;              // eta for the leading jet
  THnSparseD *fhSubleadingJetEta;           // eta for the subleading jet
  THnSparseD *fhAnyJetEta;                  // eta for all jets
  THnSparseD *fhDijetAsymmetryVsDphi;       // Asymmetry vs. deltaPhi
  THnSparseD *fhDijetLeadingVsSubleadingPt; // Leading jet pT vs. subleading jet pT

  
private:
  
  ConfigurationCard *fCard;    // Card for binning info
  const TString kEventTypeStrings[knEventTypes] = {"All", "v_{z} cut", "Dijet"}; // Strings corresponding to event types
  
};

#endif
