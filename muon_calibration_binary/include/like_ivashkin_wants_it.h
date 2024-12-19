#include <TH1.h>
#include <TString.h>
#include <TGraphErrors.h>
#include <TH2.h>
#include <TStyle.h>
#include <TAttMarker.h>
#include <TCanvas.h>

#ifndef LIWI
#define LIWI
void graph_like_ivashkin_wants_it(TGraph *graph_central,  TString XTitle="", TString YTitle="", TString Title = "",Int_t marker_color = 1, Int_t color = 1)

{
	gPad->SetFrameLineWidth(2);
	gStyle->SetOptStat(1000);
	gStyle->SetOptFit(111);
	gStyle->SetLineWidth(2);
	graph_central->SetTitle(Title);
	graph_central->GetXaxis()->SetTitle(XTitle);
	graph_central->GetYaxis()->SetTitle(YTitle);
	graph_central->SetLineColor(color);
	graph_central->SetLineWidth(2);

	graph_central->GetYaxis()->SetMaxDigits(4);        
	
	graph_central->GetXaxis()->SetLabelSize(0.06);        
	graph_central->GetYaxis()->SetLabelSize(0.06);        
	graph_central->GetXaxis()->SetTitleSize(0.06);        
	graph_central->GetYaxis()->SetTitleSize(0.06);
	graph_central->SetMarkerStyle(22); 
	graph_central->SetMarkerSize(1);
	graph_central->SetMarkerColor(marker_color);
	graph_central->GetYaxis()->SetLabelOffset(0.01);
	graph_central->GetXaxis()->SetLabelOffset(0.01);
	graph_central->GetYaxis()->SetTitleOffset(0.85);
	graph_central->GetXaxis()->SetTitleOffset(1);
	graph_central->GetYaxis()->SetNdivisions(510);
	graph_central->GetXaxis()->SetNdivisions(510);
	graph_central->SetLineWidth(3);
	graph_central->GetYaxis()->SetLabelFont(62);
	graph_central->GetXaxis()->SetLabelFont(62);
	graph_central->GetYaxis()->SetTitleFont(62);
	graph_central->GetXaxis()->SetTitleFont(62);
}

void hist_like_ivashkin_wants_it(TH1 *graph_central, TString XTitle, TString YTitle, Int_t linecolor = 1/*, Float_t Xminimum, Float_t Xmaximum*/)

{
	//gStyle->SetOptStat(1000000011);
	gStyle->SetOptStat(000);
	//gStyle->SetOptFit(111);
	gStyle->SetLineWidth(2);

	graph_central->GetXaxis()->SetTitle(XTitle);
	graph_central->GetYaxis()->SetTitle(YTitle);
	graph_central->GetXaxis()->SetLabelSize(0.06);        
	graph_central->GetYaxis()->SetLabelSize(0.06);        
	graph_central->GetXaxis()->SetTitleSize(0.06);        
	graph_central->GetYaxis()->SetTitleSize(0.06);
	graph_central->SetMarkerStyle(2); 
	graph_central->GetYaxis()->SetMaxDigits(4);        

	graph_central->GetYaxis()->SetLabelOffset(0.01);
	graph_central->GetXaxis()->SetLabelOffset(0.01);
	graph_central->GetYaxis()->SetTitleOffset(0.85);
	graph_central->GetXaxis()->SetTitleOffset(1);
	graph_central->GetYaxis()->SetNdivisions(510);
	graph_central->GetXaxis()->SetNdivisions(510);
	graph_central->SetLineWidth(2);
	graph_central->SetLineColor(linecolor);
	graph_central->GetYaxis()->SetLabelFont(62);
	graph_central->GetXaxis()->SetLabelFont(62);
	graph_central->GetYaxis()->SetTitleFont(62);
	graph_central->GetXaxis()->SetTitleFont(62);
}
void hist2d_like_ivashkin_wants_it(TH2 *graph_central, TString XTitle, TString YTitle, Int_t linecolor)
{
	gStyle->SetOptStat(0000);
	gStyle->SetOptFit(000);
	gStyle->SetLineWidth(2);
	//graph_central->SetTitle("");
	graph_central->GetXaxis()->SetTitle(XTitle);
	graph_central->GetYaxis()->SetTitle(YTitle);
	graph_central->GetXaxis()->SetLabelSize(0.045);        
	graph_central->GetYaxis()->SetLabelSize(0.045);        
	graph_central->GetXaxis()->SetTitleSize(0.045);        
	graph_central->GetYaxis()->SetTitleSize(0.045);
	graph_central->GetYaxis()->SetLabelOffset(0.01);
	graph_central->GetXaxis()->SetLabelOffset(0.01);
	graph_central->GetYaxis()->SetTitleOffset(1.15);
	graph_central->GetXaxis()->SetTitleOffset(1);
	graph_central->GetYaxis()->SetNdivisions(510);
	graph_central->GetXaxis()->SetNdivisions(510);
	graph_central->SetLineWidth(1);
	graph_central->SetLineColor(linecolor);
	graph_central->GetYaxis()->SetLabelFont(62);
	graph_central->GetXaxis()->SetLabelFont(62);
	graph_central->GetYaxis()->SetTitleFont(62);
	graph_central->GetXaxis()->SetTitleFont(62);
}
#endif LIWI