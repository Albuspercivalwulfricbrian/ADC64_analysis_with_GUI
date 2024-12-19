/* 
    @file   PSDmuonTracker.h
    @class  PSDmuonTracker
    @author Nikolay Karpushkin (nkarpushkin@mail.ru)
    @brief  Class for calorimeter muon calibration using least squares method
*/

#ifndef MuonTracker_H
#define MuonTracker_H 1
#include <Fit/Fitter.h>
#include <Math/Functor.h>
#include "muon_struct.h"
#include "TString.h"
#include <TVector3.h>

class MuonTracker
{

    public:

	MuonTracker(TrackInfo*);
        ~MuonTracker();


	void CalculateTrack();
	void CalculateTrackByEigenVector();
	void CalculateTrackByMinuit();
	void line(Float_t, const Double_t *, Float_t &, Float_t &, Float_t &);

	void CalculateChargeStraightened();
	void MaxEigenVectorByDirectIterations(Double_t, Int_t, Double_t *, Double_t **);
	void MaxEigenVectorByJacobi(Double_t *, Double_t **);
	void MakeJacobiRotations(Int_t, Double_t **, Double_t *, Double_t **);

//         
//         Setters
//       
	void SetDebugMode(Bool_t debug) {fIsDebug = debug;}
	void SetCellSizes(Int_t, Int_t, Int_t);
	void SetZenithVector(Int_t, Int_t, Int_t);
	void SetAzimuthVector(Int_t, Int_t, Int_t);
	void SetTrackCalcMode(TString mode) {fTrackCalcMode = mode;}
 
//         
//         Getters
//         
	TVector3 GetChargeCenter();
	Float_t GetChargeCenterX();
	Float_t GetChargeCenterY();
	Float_t GetChargeCenterZ();
	TVector3 GetTrackVector();
	Float_t GetTrackZenithAngle();
	Float_t GetTrackAzimuthAngle();
	Double_t GetTrackSquareError();
	Float_t *GetChargeStraightened();
	Float_t *GetTrackLength();
	void GetCalibratedCharge(TrackInfo*);


    private:
        void InitializeWithVector(TrackInfo*);

        // void Initialize(Int_t, Float_t **, Float_t **, Float_t *, Float_t *);
	void AllocData();
	void DeleteData();
	void Clear();

	Int_t fDimensions;
	Bool_t fIsDebug;
	Int_t fN_Hits;
	TString fTrackCalcMode;

	TVector3 *fR;
	TVector3 *fRweighted;
	Float_t **fHitPosition;
	Float_t **fCellSizes;
	Float_t *fCharge_Hits;
	Float_t *fCharge_Calibration;
	Float_t *fChargeStraightened;
	Float_t *fTrackLength;

	Int_t fMaxChargeHitNumber;
	TVector3 fR_cm;
	TVector3 fTrackVector;
	TVector3 fZenithVector;
	TVector3 fAzimuthVector;
};

struct SumDistance2 { 

	Int_t N_Hits;
	TVector3 *R;
	Float_t *weight;
	SumDistance2(Int_t nhits, TVector3 *vect, Float_t* weight_) { N_Hits = nhits; R = vect; weight = weight_; }

	Double_t distance2(Float_t x, Float_t y, Float_t z, const Double_t *p) 
	{
		TVector3 xp(x,y,z);
		TVector3 x0(p[0], p[2], p[4] );
		TVector3 x1(p[0] + p[1], p[2] + p[3], p[4] + p[5]);
		TVector3 u = (x1-x0).Unit();
		Double_t d2 = ((xp-x0).Cross(u)).Mag2();
		return d2;
	}

	Double_t operator() (const Double_t *par) 
	{
		Double_t sum = 0;
		for (Int_t i  = 0; i < N_Hits; ++i) 
		{
			Double_t d = distance2(R[i](0), R[i](1), R[i](2), par);
			if(weight != nullptr) sum += d*weight[i];
			else sum += d;
		}		
		return sum;
	}

};

#endif
