#include "DijetHistogramManager.h" R__LOAD_LIBRARY(plotting/DrawingClasses.so)
#include "DijetCard.h"
#include "DijetMethods.h"
#include "JDrawer.h"

/*
 * Macro for producing the JFF correction from PYTHIA simulation results
 */
void checkSpilloverAsymmetry(){
  
  // ==================================================================
  // ========================= Configuration ==========================
  // ==================================================================
  
  TString spilloverFileName = "newSpilloverTest_symmetrizedDistribution_matchedDijets_radial.root";
  TString spilloverComparisonFileName = "newSpilloverTest_symmetrizedDistribution_genJets_radial.root";
  // newSpilloverTest_symmetrizedDistribution_xj_radial.root
  // newSpilloverTest_symmetrizedDistribution_matchedDijets_radial.root
  // newSpilloverTest_symmetrizedDistribution_genJets_radial.root
  
  bool drawAsymmetryComparison = false;
  bool drawFileComparison = true;
  
  bool saveFigures = false;
  
  // Open the input files
  TFile *spilloverFile = TFile::Open(spilloverFileName);
  TFile *comparisonFile = TFile::Open(spilloverComparisonFileName);
  
  const int nCentralityBins = 4;
  const int nTrackPtBins = 6;
  const int nAsymmetryBins = 3;
  double centralityBinBorders[] = {0,10,30,50,100};  // Bin borders for centrality
  double trackPtBinBorders[] = {0.7,1,2,3,4,8,12};  // Bin borders for track pT
  double xjBinBorders[] = {0,0.5,0.75,1}; // Bin borders for xj
  
  // Define arrays for the histograms
  TH2D *spilloverHistogram[nAsymmetryBins+1][nCentralityBins][nTrackPtBins];
  TH1D *spilloverDeltaR[nAsymmetryBins+1][nCentralityBins][nTrackPtBins];
  TH1D *spilloverRatio[nAsymmetryBins][nCentralityBins][nTrackPtBins];
  TH2D *spilloverHistogramComparison[nAsymmetryBins+1][nCentralityBins][nTrackPtBins];
  TH1D *spilloverDeltaRComparison[nAsymmetryBins+1][nCentralityBins][nTrackPtBins];
  TH1D *spilloverRatioComparison[nAsymmetryBins+1][nCentralityBins][nTrackPtBins];
  
  // Define a DijetMethods to get the spillover correction as a function of DeltaR
  DijetMethods *calculator = new DijetMethods;
  
  // Read the histograms from spillover file
  for(int iCentrality = 0; iCentrality < nCentralityBins; iCentrality++){
    for(int iTrackPt = 0; iTrackPt < nTrackPtBins; iTrackPt++){
      /*for(int iAsymmetry = 0; iAsymmetry < nAsymmetryBins; iAsymmetry++){
        spilloverHistogram[iAsymmetry][iCentrality][iTrackPt] = (TH2D*) spilloverFile->Get(Form("trackLeadingJetDeltaEtaDeltaPhi/nofitSpilloverCorrection_trackLeadingJetDeltaEtaDeltaPhi_A%dC%dT%d",iAsymmetry,iCentrality,iTrackPt));
        spilloverDeltaR[iAsymmetry][iCentrality][iTrackPt] = calculator->GetJetShape(spilloverHistogram[iAsymmetry][iCentrality][iTrackPt]);
      }*/
      spilloverHistogram[nAsymmetryBins][iCentrality][iTrackPt] = (TH2D*) spilloverFile->Get(Form("trackLeadingJetDeltaEtaDeltaPhi/nofitSpilloverCorrection_trackLeadingJetDeltaEtaDeltaPhi_C%dT%d",iCentrality,iTrackPt));
      spilloverHistogram[nAsymmetryBins][iCentrality][iTrackPt]->SetName(Form("regularSpillover%d%d",iCentrality,iTrackPt));
      spilloverDeltaR[nAsymmetryBins][iCentrality][iTrackPt] = calculator->GetJetShape(spilloverHistogram[nAsymmetryBins][iCentrality][iTrackPt]);
      
      spilloverHistogramComparison[nAsymmetryBins][iCentrality][iTrackPt] = (TH2D*) comparisonFile->Get(Form("trackLeadingJetDeltaEtaDeltaPhi/nofitSpilloverCorrection_trackLeadingJetDeltaEtaDeltaPhi_C%dT%d",iCentrality,iTrackPt));
      spilloverHistogramComparison[nAsymmetryBins][iCentrality][iTrackPt]->SetName(Form("comparisonSpillover%d%d",iCentrality,iTrackPt));
      spilloverDeltaRComparison[nAsymmetryBins][iCentrality][iTrackPt] = calculator->GetJetShape(spilloverHistogramComparison[nAsymmetryBins][iCentrality][iTrackPt]);
    }
  }
  
  // Draw all asymmery bins deltaR curves to a single plot to see if the spillover is similar regardless of asymmetry bin
  JDrawer *drawer = new JDrawer();
  drawer->SetDefaultAppearanceSplitCanvas();
  
  // Make subeNon0 to spillover comparison in all bins
  TLegend *legend;
  int colors[] = {kBlue,kRed,kMagenta,kBlack};
  
  if(drawAsymmetryComparison){
    for(int iCentrality = 0; iCentrality < nCentralityBins; iCentrality++){
      for(int iTrackPt = 0; iTrackPt < nTrackPtBins; iTrackPt++){
        drawer->CreateSplitCanvas();
        legend = new TLegend(0.5,0.55,0.9,0.8);
        legend->SetFillStyle(0);legend->SetBorderSize(0);legend->SetTextSize(0.05);legend->SetTextFont(62);
        legend->SetHeader(Form("%.1f < p_{T} < %.1f, C: %.0f-%.0f %%",trackPtBinBorders[iTrackPt],trackPtBinBorders[iTrackPt+1], centralityBinBorders[iCentrality],centralityBinBorders[iCentrality+1]));
        
        
        spilloverDeltaR[0][iCentrality][iTrackPt]->SetLineColor(colors[0]);
        drawer->DrawHistogramToUpperPad(spilloverDeltaR[0][iCentrality][iTrackPt],"#DeltaR","P(#DeltaR)"," ");
        
        for(int iAsymmetry = 1; iAsymmetry <= nAsymmetryBins; iAsymmetry++){
          spilloverDeltaR[iAsymmetry][iCentrality][iTrackPt]->SetLineColor(colors[iAsymmetry]);
          spilloverDeltaR[iAsymmetry][iCentrality][iTrackPt]->Draw("same");
          
        }
        
        legend->AddEntry(spilloverDeltaR[nAsymmetryBins][iCentrality][iTrackPt],"All x_{j}","l");
        
        for(int iAsymmetry = 0; iAsymmetry < nAsymmetryBins; iAsymmetry++){
          legend->AddEntry(spilloverDeltaR[iAsymmetry][iCentrality][iTrackPt],Form("%.2f < x_{j} < %.2f",xjBinBorders[iAsymmetry], xjBinBorders[iAsymmetry+1]),"l");
        }
        
        legend->Draw();
        
        for(int iAsymmetry = 0; iAsymmetry < nAsymmetryBins; iAsymmetry++){
          spilloverRatio[iAsymmetry][iCentrality][iTrackPt] = (TH1D*) spilloverDeltaR[iAsymmetry][iCentrality][iTrackPt]->Clone(Form("ratioOfDeltaR%d%d%d",iAsymmetry,iCentrality,iTrackPt));
          spilloverRatio[iAsymmetry][iCentrality][iTrackPt]->Divide(spilloverDeltaR[nAsymmetryBins][iCentrality][iTrackPt]);
          if(iAsymmetry == 0){
            spilloverRatio[iAsymmetry][iCentrality][iTrackPt]->GetYaxis()->SetRangeUser(0.5,1.5);
            drawer->DrawHistogramToLowerPad(spilloverRatio[iAsymmetry][iCentrality][iTrackPt],"#DeltaR","x_{j}/All x_{j}");
          } else {
            spilloverRatio[iAsymmetry][iCentrality][iTrackPt]->Draw("same");
          }
        }
        
        if(saveFigures){
          gPad->GetCanvas()->SaveAs(Form("figures/spilloverAsymmetryDeltaRComparison_C=%.0f-%.0f_pT=%.0f-%.0f.pdf", centralityBinBorders[iCentrality], centralityBinBorders[iCentrality+1], trackPtBinBorders[iTrackPt], trackPtBinBorders[iTrackPt+1]));
        }
      } // Track pt Loop
    } // Centrality loop
  } // Asymmetry comparison if
  
  if(drawFileComparison){
    for(int iCentrality = 0; iCentrality < nCentralityBins; iCentrality++){
      for(int iTrackPt = 0; iTrackPt < nTrackPtBins; iTrackPt++){
        drawer->CreateSplitCanvas();
        legend = new TLegend(0.5,0.55,0.9,0.8);
        legend->SetFillStyle(0);legend->SetBorderSize(0);legend->SetTextSize(0.05);legend->SetTextFont(62);
        legend->SetHeader(Form("%.1f < p_{T} < %.1f, C: %.0f-%.0f %%",trackPtBinBorders[iTrackPt],trackPtBinBorders[iTrackPt+1], centralityBinBorders[iCentrality],centralityBinBorders[iCentrality+1]));
        
        
        spilloverDeltaR[nAsymmetryBins][iCentrality][iTrackPt]->SetLineColor(kRed);
        drawer->DrawHistogramToUpperPad(spilloverDeltaR[nAsymmetryBins][iCentrality][iTrackPt],"#DeltaR","P(#DeltaR)"," ");
        legend->AddEntry(spilloverDeltaR[nAsymmetryBins][iCentrality][iTrackPt],"Matched","l");
        
        spilloverDeltaRComparison[nAsymmetryBins][iCentrality][iTrackPt]->SetLineColor(kBlue);
        spilloverDeltaRComparison[nAsymmetryBins][iCentrality][iTrackPt]->Draw("same");
        legend->AddEntry(spilloverDeltaRComparison[nAsymmetryBins][iCentrality][iTrackPt],"Gen jets","l");
        
        legend->Draw();
        
        spilloverRatioComparison[nAsymmetryBins][iCentrality][iTrackPt] = (TH1D*) spilloverDeltaRComparison[nAsymmetryBins][iCentrality][iTrackPt]->Clone(Form("ratioOfDeltaRComparison%d%d",iCentrality,iTrackPt));
        spilloverRatioComparison[nAsymmetryBins][iCentrality][iTrackPt]->Divide(spilloverDeltaR[nAsymmetryBins][iCentrality][iTrackPt]);
        spilloverRatioComparison[nAsymmetryBins][iCentrality][iTrackPt]->GetYaxis()->SetRangeUser(0,2);
        drawer->DrawHistogramToLowerPad(spilloverRatioComparison[nAsymmetryBins][iCentrality][iTrackPt],"#DeltaR","Matched/Regular");
        
        if(saveFigures){
          gPad->GetCanvas()->SaveAs(Form("figures/spilloverAsymmetryFileComparison_C=%.0f-%.0f_pT=%.0f-%.0f.pdf", centralityBinBorders[iCentrality], centralityBinBorders[iCentrality+1], trackPtBinBorders[iTrackPt], trackPtBinBorders[iTrackPt+1]));
        }
      } // Track pt Loop
    } // Centrality loop
  } // Asymmetry comparison if
}
