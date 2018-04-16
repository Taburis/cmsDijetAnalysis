// Class for reading necessary configuration for an analysis from a card file
// Original code written by Jyväskylä ALICE group
// Modified for CMS by Jussi Viinikainen

#ifndef CONFIGURATIONCARD_H
#define CONFIGURATIONCARD_H

// C++ includes
#include <iostream>
#include <fstream>
#include <vector>

// Root includes
#include <TString.h>
#include <TVector.h>
#include <TObjString.h>
#include <TFile.h>
#include <TF1.h>
#include <THashList.h>
#include <TNamed.h>

using namespace std;
#ifndef MAXDIMBUFFER
#define MAXDIMBUFFER
const int kMaxDimBuffer = 300; // Maximum length of a line read to a buffer
#endif

class ConfigurationCard {

  //====   M e m b e r    F u n c t i o n s   ========

public:

  ConfigurationCard(); // constructor
  ConfigurationCard(const char *filename); // constructor
  ConfigurationCard& operator=(const ConfigurationCard& obj);

  virtual ~ConfigurationCard();

  void AddToKeyTable( TString key, int index );

  float  Get(TString keyword, int VectorComponent=0); //get TVector component 
  TString  GetStr(TString keyword ); //get TVector component 
  TVector* GetVector( TString keyword );
  int GetN(TString keyword);       //get TVector dimension
  int GetBin(TString keyword, double value);  // Find the bin for value from keyword vector
  int GetNBin(TString keyword);   // Get number of bins related to keyword
  void PrintOut();
  void WriteCard(TDirectory *file);
  void ReadInputLine( const char* buffer );

protected:
  
  void    ReadInputCard();
  int     GetNwithIndex(int i){ return fValuesVector[i].GetNrows(); }
  unsigned int GetTVectorIndex(TString keyword, int tol=0);

  //====   D a t a    M e m b e r s  ========

  char fCardName[255];                       // File name for card
  int  fnEntry;                              // Number of lines in configuration file
  std::vector< TString > fKeyWordVector;     // Array of key words
  std::vector< TVector > fValuesVector;      // Array of float number confg parameter vectors
  std::vector< TString > fValueString;       // Storage of raw inut string for each item
  THashList fKeyTable;                       // key map with hash algorithm

};

#endif






















