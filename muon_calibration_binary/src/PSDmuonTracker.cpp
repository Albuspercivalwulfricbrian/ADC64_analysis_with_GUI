#include "PSDmuonTracker.h"

MuonTracker::MuonTracker(TrackInfo* Extrackinfo)
{
	InitializeWithVector(Extrackinfo);

}

MuonTracker::~MuonTracker()
{
}

void MuonTracker::InitializeWithVector(TrackInfo* trackinfo)
{
        fDimensions = 3;
        fIsDebug = false;
        fTrackCalcMode = "EigenVector";

	fN_Hits = trackinfo->GetReduced().size();
	// std::cout << trackinfo->GetReduced().size() << std::endl;
	fHitPosition = new Float_t*[fN_Hits];
	fCellSizes = new Float_t*[fN_Hits];
	for(int i = 0; i < fN_Hits; ++i) 
	{
		fHitPosition[i] = new Float_t[3];
		fCellSizes[i] = new Float_t[3];
	}
	// Float_t **HitPosition = new Float_t*[total_channels];

		for(Int_t i = 0; i < fN_Hits; i++)
		{
			fHitPosition[i][0] = (Float_t)trackinfo->GetReduced()[i].X*15;
			fHitPosition[i][1] = (Float_t)trackinfo->GetReduced()[i].Y*15;
			fHitPosition[i][2] = (Float_t)trackinfo->GetReduced()[i].Z*12;
			fCellSizes[i][0] = 15.;
			fCellSizes[i][1] = 15.;
			fCellSizes[i][2] = 12.;
		}
	// fHitPosition = hit_position;
	// fCellSizes = cell_sizes;
	AllocData();

		for(Int_t i = 0; i < fN_Hits; i++)
		{
			fCharge_Hits[i] = trackinfo->GetReduced()[i].charge;
			fCharge_Calibration[i] = 1.;

		}
	// memcpy(fCharge_Hits, charge_hits, (fN_Hits)*sizeof(float));
	// memcpy(fCharge_Calibration, charge_calibration, (fN_Hits)*sizeof(float));
	
}
// MuonTracker::MuonTracker(Int_t nhits, Float_t **hit_position, Float_t **cell_sizes, Float_t *charge_hits, Float_t *charge_calibration)
// {
// 	 Initialize(nhits, hit_position, cell_sizes, charge_hits, charge_calibration);
// }

// void MuonTracker::Initialize(Int_t nhits, Float_t **hit_position, Float_t **cell_sizes, Float_t *charge_hits, Float_t *charge_calibration)
// {
//         fDimensions = 3;
//         fIsDebug = false;
//         fTrackCalcMode = "EigenVector";

// 	fN_Hits = nhits;
// 	fHitPosition = hit_position;
// 	fCellSizes = cell_sizes;
// 	AllocData();
// 	memcpy(fCharge_Hits, charge_hits, (fN_Hits)*sizeof(float));
// 	memcpy(fCharge_Calibration, charge_calibration, (fN_Hits)*sizeof(float));
// }

void MuonTracker::AllocData()
{
	fR = new TVector3[fN_Hits];
	fRweighted = new TVector3[fN_Hits];
	fCharge_Hits = new Float_t[fN_Hits];
	fCharge_Calibration = new Float_t[fN_Hits];
	fChargeStraightened = new Float_t[fN_Hits];
	fTrackLength = new Float_t[fN_Hits];

	for(Int_t i = 0; i < fN_Hits; i++)
	{
		fCharge_Hits[i] = 0.;
		fCharge_Calibration[i] = 0.;
		fChargeStraightened[i] = 0.;
		fTrackLength[i] = 0.;
	}
}

void MuonTracker::SetZenithVector(Int_t X, Int_t Y, Int_t Z)
{
	fZenithVector.SetXYZ(X,Y,Z);
	fZenithVector = fZenithVector.Unit();
}

void MuonTracker::SetAzimuthVector(Int_t X, Int_t Y, Int_t Z) //vector to count azimuth angle from it counterclockwise to the zenith vector
{
	fAzimuthVector.SetXYZ(X,Y,Z);
	fAzimuthVector = fAzimuthVector.Unit();
}

void MuonTracker::CalculateTrack()
{
	if(fTrackCalcMode == "EigenVector") CalculateTrackByEigenVector();
	if(fTrackCalcMode == "Minuit") CalculateTrackByMinuit();
}

void MuonTracker::CalculateTrackByEigenVector()
{
	Float_t total_charge = 0.;
	fMaxChargeHitNumber = 0;
	if(fIsDebug) printf("\n\n");

	for(Int_t i = 0; i < fN_Hits; i++)
	{
		fR[i].SetXYZ(fHitPosition[i][0], fHitPosition[i][1], fHitPosition[i][2]);
		if(fIsDebug)
		{
			printf("Hitted section: ");
			for(Int_t j = 0; j < fDimensions; j++) printf("%.0f ", fR[i][j]);
			printf("Charge: %.0f\n", fCharge_Hits[i]);
		}

		fCharge_Hits[i] /= fCharge_Calibration[i];
		fR_cm += fCharge_Hits[i] * fR[i];
		total_charge += fCharge_Hits[i];
		if(fCharge_Hits[i] > fCharge_Hits[fMaxChargeHitNumber]) fMaxChargeHitNumber = i;
	}
	fR_cm *= 1./total_charge;

	for(Int_t i = 0; i < fN_Hits; i++)
	{
		fR[i] -= fR_cm;
		fRweighted[i] = fR[i];//*fCharge_Hits[i];
	}

	Double_t **quadratic_form = new Double_t*[fDimensions];
	for(Int_t i = 0; i < fDimensions; i++)
	{
		quadratic_form[i] = new Double_t[fDimensions];
		for(Int_t j = 0; j < fDimensions; j++) 
		{
			quadratic_form[i][j] = 0.;
			for(Int_t k = 0; k < fN_Hits; k++) quadratic_form[i][j] += fRweighted[k](i) * fRweighted[k](j);
		}
	}

	if(fIsDebug)
	{
		printf("\nquadratic form to maximize\n");
		for(Int_t i = 0; i < fDimensions; i++)
		{
			for(Int_t j = 0; j < fDimensions; j++) printf("%.3e ", quadratic_form[i][j]);
			printf("\n");
		}
	}

	Double_t *max_eigenvector = new Double_t[fDimensions];
	for(Int_t i = 0; i < fDimensions; i++) max_eigenvector[i] = 0.;

	MaxEigenVectorByJacobi(max_eigenvector, quadratic_form);
	//MaxEigenVectorByDirectIterations(0.01, 5, max_eigenvector, quadratic_form);

	fTrackVector.SetXYZ(max_eigenvector[0], max_eigenvector[1], max_eigenvector[2]);
	fTrackVector = fTrackVector.Unit();
	if(fTrackVector*fZenithVector < 0.) fTrackVector = -fTrackVector;

	if(fIsDebug)
	{
		printf("Muon Track Vector  ");
		for(Int_t i = 0; i < fDimensions; i++) printf("%.3f ", fTrackVector(i));
		printf("\n\n");
	}

	for(Int_t i = 0; i < fDimensions; i++) delete[] quadratic_form[i];
	delete[] quadratic_form;
	delete[] max_eigenvector;
}

void MuonTracker::MaxEigenVectorByDirectIterations(Double_t eps, Int_t max_iterations, Double_t *max_eigenvector, Double_t **quadratic_form)
{
	if(fIsDebug) printf("\ncalculating max eigenvector by direct iterations\n");

	Double_t *vector_prev = new Double_t[fDimensions];
	Double_t max_component_prev = 0.;
	for(Int_t i = 0; i < fDimensions; i++)
	{
		vector_prev[i] = fRweighted[fMaxChargeHitNumber](i);
		if(abs(vector_prev[i]) > abs(max_component_prev)) max_component_prev = vector_prev[i];
	}
	
	Double_t *vector = new Double_t[fDimensions];
	Double_t max_component = 0.;

	for(Int_t iteration = 0; iteration < max_iterations; iteration++)
	{
		for(Int_t i = 0; i < fDimensions; i++)
		{
			vector[i] = 0.;
			for(Int_t j = 0; j < fDimensions; j++)
				vector[i] += quadratic_form[i][j] * vector_prev[j]/max_component_prev;
		}
		max_component = 0.;
		for(Int_t i = 0; i < fDimensions; i++)
		{
			vector_prev[i] = vector[i];
			if(abs(vector[i]) > abs(max_component)) max_component = vector[i];
		}

		if(fIsDebug)
		{
			printf("iteration %i delta %.3e vector coordinates  ", iteration, abs((max_component-max_component_prev)/max_component));
			for(Int_t i = 0; i < fDimensions; i++) printf("%.3e ", vector[i]);
			printf("\n");
		}
		if(abs((max_component-max_component_prev)/max_component) < eps) break;

		max_component_prev = max_component;
	}

	for(Int_t i = 0; i < fDimensions; i++)
		max_eigenvector[i] = vector[i];

	delete[] vector_prev;
	delete[] vector;
}

void MuonTracker::MaxEigenVectorByJacobi(Double_t *max_eigenvector, Double_t **quadratic_form)
{
	if(fIsDebug) printf("\ncalculating max eigenvector by Jacobi rotations\n");

	Double_t **eigenvectors = new Double_t*[fDimensions];
	for(Int_t i = 0; i < fDimensions; i++)
	{
		eigenvectors[i] = new Double_t[fDimensions];
		for(Int_t j = 0; j < fDimensions; j++) eigenvectors[i][j] = 0.;
	}

	Double_t *eigenvalues = new Double_t[fDimensions];
	for(Int_t i = 0; i < fDimensions; i++)
		eigenvalues[i] = 0.;

	MakeJacobiRotations(fDimensions, quadratic_form, eigenvalues, eigenvectors);

	if(fIsDebug)
	{
		printf("\n quadratic form after Jacobi                 eigenvectors\n");
		for(Int_t i = 0; i < fDimensions; i++)
		{
			for(Int_t j = 0; j < fDimensions; j++) printf("%.3e ", quadratic_form[i][j]);
			printf("     ");
			for(Int_t j = 0; j < fDimensions; j++) printf("%.3e ", eigenvectors[i][j]);
			printf("\n");
		}
		printf("\neigenvalues: ");
		for(Int_t i = 0; i < fDimensions; i++) printf("%.3e ", eigenvalues[i]);
		printf("\n");
	}

	Double_t max_eigenvalue = 0.;
	Int_t max_eigenvalue_column = 0;
	for(Int_t i = 0; i < fDimensions; i++)
		if(eigenvalues[i] > max_eigenvalue) { max_eigenvalue = eigenvalues[i]; max_eigenvalue_column = i; }

	for(Int_t i = 0; i < fDimensions; i++)
		max_eigenvector[i] = eigenvectors[i][max_eigenvalue_column];

	if(fIsDebug)
	{
		printf("eigenvector: ");
		for(Int_t i = 0; i < fDimensions; i++) printf("%.3e ", max_eigenvector[i]);
		printf("\n");
	}

	for(Int_t i =0; i < fDimensions; i++) delete[] eigenvectors[i];
	delete[] eigenvectors;
	delete[] eigenvalues;
}

void MuonTracker::MakeJacobiRotations(Int_t n, Double_t **a, Double_t *d, Double_t **v)
{
    if ( n == 0 ) return;
    Double_t * b = new Double_t[n+n];
    Double_t * z = b + n;
    Int_t i, j;
    for ( i = 0; i < n; ++i )
    {
        z[i] = 0.;
        b[i] = d[i] = a[i][i];
        for ( j = 0; j < n; ++j ) v[i][j] = i == j ? 1. : 0.;
    }
    for ( i = 0; i < 50; ++i )
    {
        Double_t sm = 0.;
        Int_t p, q;
        for ( p = 0; p < n - 1; ++p )
            for ( q = p + 1; q < n; ++q )
                sm += fabs ( a[p][q] );
        if ( sm == 0 ) { if(fIsDebug) printf("\nJacobi rotation break pass number: %i\n", i); break; }
        Double_t tresh = i < 3 ? 0.2 * sm / ( n*n ) : 0.;
        for ( p = 0; p < n - 1; ++p )
        {
            for ( q = p + 1; q < n; ++q )
            {
                Double_t g = 1e12 * fabs ( a[p][q] );
                if ( i >= 3 && fabs ( d[p] ) > g && fabs ( d[q] ) > g ) a[p][q] = 0.;
                else
                if ( fabs ( a[p][q] ) > tresh )
                {
                    Double_t theta = 0.5 * ( d[q] - d[p] ) / a[p][q];
                    Double_t t = 1. / ( fabs(theta) + sqrt(1.+theta*theta) );
                    if ( theta < 0 ) t = - t;
                    Double_t c = 1. / sqrt ( 1. + t*t );
                    Double_t s = t * c;
                    Double_t tau = s / ( 1. + c );
                    Double_t h = t * a[p][q];
                    z[p] -= h;
                    z[q] += h;
                    d[p] -= h;
                    d[q] += h;
                    a[p][q] = 0.;
                    for ( j = 0; j < p; ++j )
                    {
                        Double_t g = a[j][p];
                        Double_t h = a[j][q];
                        a[j][p] = g - s * ( h + g * tau );
                        a[j][q] = h + s * ( g - h * tau );
                    }
                    for ( j = p+1; j < q; ++j )
                    {
                        Double_t g = a[p][j];
                        Double_t h = a[j][q];
                        a[p][j] = g - s * ( h + g * tau );
                        a[j][q] = h + s * ( g - h * tau );
                    }
                    for ( j = q+1; j < n; ++j )
                    {
                        Double_t g = a[p][j];
                        Double_t h = a[q][j];
                        a[p][j] = g - s * ( h + g * tau );
                        a[q][j] = h + s * ( g - h * tau );
                    }
                    for ( j = 0; j < n; ++j )
                    {
                        Double_t g = v[j][p];
                        Double_t h = v[j][q];
                        v[j][p] = g - s * ( h + g * tau );
                        v[j][q] = h + s * ( g - h * tau );
                    }
                }
            }
        }
        for ( p = 0; p < n; ++p )
        {
            d[p] = ( b[p] += z[p] );
            z[p] = 0.;
        }
    }
    delete[] b;
}

void MuonTracker::CalculateTrackByMinuit()
{
	if(fIsDebug) printf("\n\n");
	for(Int_t i = 0; i < fN_Hits; i++)
	{
		fR[i].SetXYZ(fHitPosition[i][0], fHitPosition[i][1], fHitPosition[i][2]);
		if(fIsDebug)
		{
			printf("Hitted section: ");
			for(Int_t j = 0; j < fDimensions; j++) printf("%.0f ", fR[i][j]);
			printf("Charge: %.0f\n", fCharge_Hits[i]);
		}
		fCharge_Hits[i] /= fCharge_Calibration[i];
	}

	ROOT::Fit::Fitter fitter;
	SumDistance2 fSumDist(fN_Hits, fR, fCharge_Hits);
	ROOT::Math::Functor fcn(fSumDist, 6);
	Double_t pStart[6] = {1,1,1,1,1,1};
	fitter.SetFCN(fcn, pStart);

	Float_t max_X = 0.;
	Float_t max_Y = 0.;
	Float_t max_Z = 0.;
	Float_t max_cell_sizeX = 0.;
	Float_t max_cell_sizeY = 0.;
	Float_t max_cell_sizeZ = 0.;
	for(Int_t i = 0; i < fN_Hits; i++)
	{
		if(fHitPosition[i][0] > max_X) max_X = fHitPosition[i][0];
		if(fHitPosition[i][1] > max_Y) max_Y = fHitPosition[i][1];
		if(fHitPosition[i][2] > max_Z) max_Z = fHitPosition[i][2];
		if(fCellSizes[i][0] > max_cell_sizeX) max_cell_sizeX = fCellSizes[i][0];
		if(fCellSizes[i][1] > max_cell_sizeY) max_cell_sizeY = fCellSizes[i][1];
		if(fCellSizes[i][2] > max_cell_sizeZ) max_cell_sizeZ = fCellSizes[i][2];
	}
	max_X += 0.5*max_cell_sizeX;
	max_Y += 0.5*max_cell_sizeY;
	max_Z += 0.5*max_cell_sizeZ;
	
	fitter.Config().ParSettings(0).SetLimits(0, max_X);
	fitter.Config().ParSettings(2).SetLimits(0, max_Y);
	fitter.Config().ParSettings(4).SetLimits(0, max_Z);

	bool ok = fitter.FitFCN();
	const ROOT::Fit::FitResult & result = fitter.Result();

	if(fIsDebug) 
	{
		printf("Total final distance square %f\n", result.MinFcnValue());
		result.Print(std::cout);
	}
	const Double_t * parFit = result.GetParams();
	
	fR_cm.SetXYZ(parFit[0], parFit[2], parFit[4]);
	for(Int_t i = 0; i < fN_Hits; i++)
	    fR[i] -= fR_cm;

	fTrackVector.SetXYZ(parFit[1], parFit[3], parFit[5]);
	fTrackVector = fTrackVector.Unit();
	if(fTrackVector*fZenithVector < 0.) fTrackVector = -fTrackVector;

	if(fIsDebug)
	{
		printf("\nLine point Vector  ");
		for(Int_t i = 0; i < fDimensions; i++) printf("%.3f ", fR_cm(i));
		printf("\n");

		printf("Muon Track Vector  ");
		for(Int_t i = 0; i < fDimensions; i++) printf("%.3f ", fTrackVector(i));
		printf("\n");
		
		printf("Muon Track SqErr   %.2f", GetTrackSquareError());
		printf("\n\n");
	}
}

void MuonTracker::line(Float_t t, const Double_t *p, Float_t &x, Float_t &y, Float_t &z) 
{
	x = p[0] + p[1]*t;
	y = p[2] + p[3]*t;
	z = p[4] + p[5]*t;
}

void MuonTracker::CalculateChargeStraightened()
{
	Double_t *D = new Double_t[2*fDimensions];
	Double_t *lambda = new Double_t[2*fDimensions];
	for(Int_t i = 0; i < 2*fDimensions; i++) { D[i] = 0.; lambda[i] = 0.; }
	Int_t intersection_counter = 0;
	TVector3 *IntersectVector = new TVector3[2*fDimensions];
	TVector3 *SubstancePathArray = new TVector3[2*fDimensions];

	for(Int_t i = 0; i < fN_Hits; i++)
	{
		intersection_counter = 0;
		for(Int_t j = 0; j < fDimensions; j++)
		{
			D[j] = fR[i](j) + 0.5*fCellSizes[i][j];
			D[j+fDimensions] = fR[i](j) - 0.5*fCellSizes[i][j];
			lambda[j] = D[j]/(fTrackVector(j));
			lambda[j+fDimensions] = D[j+fDimensions]/(fTrackVector(j));
		}

		for(Int_t j = 0; j < 2*fDimensions; j++)
		{
			Bool_t is_intersection = true;
			for(Int_t dim_iter = 0; dim_iter < fDimensions; dim_iter++)
			{		
				if(dim_iter == j || dim_iter+fDimensions == j) continue;
				is_intersection *= (lambda[j]*fTrackVector)(dim_iter) >= (fR[i](dim_iter) - 0.5*fCellSizes[i][dim_iter]);
				is_intersection *= (lambda[j]*fTrackVector)(dim_iter) <= (fR[i](dim_iter) + 0.5*fCellSizes[i][dim_iter]);
				
				if(false) printf("stretched to cube face dim %i: check axis %i: is btw %.2f <= %.2f <= %.2f \n", j, dim_iter,
					((fR_cm+fR[i])(dim_iter) - 0.5*fCellSizes[i][dim_iter]), 
					((fR_cm+lambda[j]*fTrackVector))(dim_iter), 
					((fR_cm+fR[i])(dim_iter) + 0.5*fCellSizes[i][dim_iter]) );
			}
			if(is_intersection)
			{
				IntersectVector[intersection_counter] = lambda[j]*fTrackVector;
				intersection_counter++;
			}
		}

		if(intersection_counter >= 2)
		{
			Float_t SubstancePathSq = 0.;
			Float_t SubstancePathSqMax = 0.;
			for(Int_t k = 1; k < intersection_counter; k++)
			{
				SubstancePathArray[k] = IntersectVector[k] - IntersectVector[0];
				SubstancePathSq = SubstancePathArray[k] * SubstancePathArray[k];
				if(SubstancePathSq > SubstancePathSqMax) SubstancePathSqMax = SubstancePathSq;
			}

			Double_t track_length = sqrt(SubstancePathSqMax); //in cm
			if(track_length > 2.5 && GetTrackZenithAngle()>0.1) //to cross 5 mm of scintilator and reject most vertical tracks
			    fChargeStraightened[i] = fCellSizes[i][2]*(fCharge_Hits[i]*fCharge_Calibration[i]/track_length); //in adc channels
			    //fChargeStraightened[i] = fCharge_Hits[i]; //in adc channels

			fTrackLength[i] = track_length;

			if(fIsDebug)
			{
				printf("Hitted section: ");
				for(Int_t j = 0; j < fDimensions; j++) printf("%.0f ", (fR[i]+fR_cm)[j]);
				printf("\n");
				for(Int_t k = 0; k < intersection_counter; k++)
				{
				    printf("Intersection %i: ", k);
				    for(Int_t j = 0; j < fDimensions; j++) printf("%.0f ", (fR_cm+IntersectVector[k])[j]);
				    printf("\n");
				}
				printf("Track length in section: %.2f ", fTrackLength[i]);
				printf("Zenith angle : %.2f ", GetTrackZenithAngle());
				printf("Charge Adj: %.0f\n", fChargeStraightened[i]);
			}
		}
	}

	delete[] D;
	delete[] lambda;
	delete[] IntersectVector;
	delete[] SubstancePathArray;
}

TVector3 MuonTracker::GetChargeCenter()
{
	return fR_cm; 
}

Float_t MuonTracker::GetChargeCenterX()
{
	return (Float_t) fR_cm.X();
}

Float_t MuonTracker::GetChargeCenterY()
{
	return (Float_t) fR_cm.Y();
}

Float_t MuonTracker::GetChargeCenterZ()
{
	return (Float_t) fR_cm.Z();
}

TVector3 MuonTracker::GetTrackVector()
{
	return fTrackVector; 
}

void MuonTracker::GetCalibratedCharge(TrackInfo* trackinfo)
{
	for (int i = 0; i < fN_Hits; i++)
	{

		// trackinfo->GetReduced()[i].charge = fChargeStraightened[i];
		trackinfo->SetReducedCharge(i,fChargeStraightened[i]);
	}
}

Float_t MuonTracker::GetTrackZenithAngle()
{
	Float_t zenith_projection = fTrackVector*fZenithVector;
	TVector3 track_azimuth_plane = fTrackVector-zenith_projection*fZenithVector;
	Float_t track_azimuth_plane_length = sqrt(track_azimuth_plane*track_azimuth_plane);
	return track_azimuth_plane_length == 0.0 && zenith_projection == 0.0 ? 999 : TMath::ATan2(track_azimuth_plane_length, zenith_projection);
}

Float_t MuonTracker::GetTrackAzimuthAngle()
{
	Float_t zenith_projection = fTrackVector*fZenithVector;
	TVector3 track_azimuth_plane = fTrackVector-zenith_projection*fZenithVector;
	Float_t track_azimuth_plane_length = sqrt(track_azimuth_plane*track_azimuth_plane);
	if(track_azimuth_plane_length < 1e-5) return 999;

	Float_t azimuth_projection = fTrackVector*fAzimuthVector;
	TVector3 cntr_azimuth = fZenithVector.Cross(fAzimuthVector);
	Float_t ctr_azimuth_projection = fTrackVector*cntr_azimuth.Unit();
	return azimuth_projection == 0.0 && ctr_azimuth_projection == 0.0 ? 999 : TMath::ATan2(ctr_azimuth_projection, azimuth_projection);
}

Double_t MuonTracker::GetTrackSquareError()
{
	Double_t error = 0.;
	for(Int_t i = 0; i < fN_Hits; i++)
	{
		Double_t track_projection = fR[i]*fTrackVector;
 		error += fR[i]*fR[i] - track_projection*track_projection;
	}
	error /= fN_Hits;

	return error;
}

Float_t *MuonTracker::GetChargeStraightened()
{
	// cout << "Charge Straightened: " << endl;
	// for(Int_t i = 0; i < fN_Hits; i++)
	// {
	// 	cout << fCharge_Calibration[i] << " ";
	// }	
	// cout << endl;
	return fChargeStraightened;
}

Float_t *MuonTracker::GetTrackLength()
{
	return fTrackLength;
}

void MuonTracker::DeleteData()
{
	delete[] fR;
	delete[] fRweighted;
	delete[] fCharge_Hits;
	delete[] fCharge_Calibration;
	delete[] fChargeStraightened;
	delete[] fTrackLength;
}

void MuonTracker::Clear()
{
	fN_Hits = 0;
	fMaxChargeHitNumber = 0;
	fR_cm.SetXYZ(0., 0., 0.);
	fTrackVector.SetXYZ(0., 0., 0.);
}

