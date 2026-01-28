#ifndef BINDATAFORMAT_H
#define BINDATAFORMAT_H

#include <fstream>
#include <iostream>
#include <TNtuple.h>
#include <TFile.h>
#include <Rtypes.h>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <TGraph.h>
#include <TObject.h>
#include <TF1.h>
#include <TCanvas.h>

#include <TString.h>

#include "event_data_struct.h"
#include "PronyFitter.h"
#include "PronyFitter.cpp"

#define readdebug 0

void MeanRMScalc(Float_t *DataArr, float *Mean, float *RMS, int begin, int end, int step = 1)
{
	begin = (begin < 0) ? 0 : begin;

	if (begin > end)
	{
		float swap = end;
		end = begin;
		begin = swap;
	};
	step = TMath::Abs(step);

	*Mean = *RMS = 0.;
	int Delta = 0;
	for (int n = begin; n <= end; n += step)
	{
		*Mean += DataArr[n];
		Delta++;
	}
	*Mean /= (float)Delta;

	for (int n = begin; n <= end; n += step)
		*RMS += (DataArr[n] - *Mean) * (DataArr[n] - *Mean);
	*RMS = TMath::Sqrt(*RMS / ((float)Delta));

	// printf("AMPL %.2f, RMS %.2f\n",*Mean,*RMS);
}

float_t LevelBy2Points(float_t X1, float_t Y1, float_t X2, float_t Y2, float_t Y0)
{

	return (X1 * Y0 - X1 * Y2 - X2 * Y0 + X2 * Y1) / (Y1 - Y2);
}

// -------------------------------------------------
float GoToLevel(Float_t *DataArr, float Level, int *point, int iterator, int iLastPoint)
{

	float ResultTime;
	while ((*point >= 0) && (*point < iLastPoint))
	{

		if ((Level - DataArr[*point]) * (Level - DataArr[*point + iterator]) <= 0)
		{
			ResultTime = LevelBy2Points((float)(*point), DataArr[*point], (float)(*point + iterator), DataArr[*point + iterator], Level);
			return ResultTime;
		}
		// printf("point %i, ampl1 %.2f ampl2 %.2f\n",*point,Level-DataArr[*point],Level-DataArr[*point+iterator]);
		*point += iterator;
	} // while

	*point = -1;
	return 0;
}

class BinDataReader
{
public:
	BinDataReader() : DataFile(NULL), /*ch_num_table(NULL),ch_num_table_rev(NULL),*/ samples_data(NULL), channel_total(64) {}
	~BinDataReader() { Clear(); }

	Int_t SetFile(TString file_name);
	Int_t ReadEvent(Int_t eventn, UInt_t board_serial_to_read = 0);

	void Calculate_waveform(event_data_struct &result_event, Int_t ch_num, Int_t gate_beg, Int_t gate_end, Int_t gate_maximum_beg, Int_t gate_maximum_end, Float_t ampl_scale = 1.0, Bool_t IsFIT = true, Double_t first_fit_harmonic = 0., Double_t second_fit_harmonic = 0., TString FIT_QA_mode = "", TObjArray *check_fit_arr = NULL, Float_t *fitQA_arg_arr = NULL);

	void Calculate_fit_harmonics(event_fit_data_struct &result_fit_event, Int_t &event_counter, Int_t event_num, Int_t ch_num, Int_t gate_beg, Int_t gate_end, Int_t gate_maximum_beg, Int_t gate_maximum_end, TString source_path, TString run_name, Float_t ampl_scale = 1.0, TObjArray *check_fit_arr = NULL);

	// data param once file open
	Int_t events_total;

	// data param each event
	Int_t Event_Num;
	Long_t TimeStamp_date;
	Long_t TimeStamp_ns;
	UShort_t sample_total;
	Int_t channel_total; // now constant
	UInt_t board_serial;

	// DATA
	Short_t **samples_data; //[board][channel][point]

private:
	Int_t ReadDataHeader();
	Int_t ReadLastEvent();
	Int_t FindMagic();

	void Clear();
	void alloc_data();
	void delete_data();

	FILE *DataFile;
};

void BinDataReader::alloc_data()
{
	if (channel_total < 1)
	{
		printf("BinDataReader::alloc_samples_arr ERROR: channel_total %i\n", channel_total);
		return;
	}
	if (sample_total < 1)
	{
		printf("BinDataReader::alloc_samples_arr ERROR: sample_total %i\n", sample_total);
		return;
	}

	if (samples_data)
	{
		if (readdebug)
			printf("BinDataReader::alloc_data: data not void\n");
		delete_data();
	}

	samples_data = new Short_t *[channel_total];
	for (Int_t ch = 0; ch < channel_total; ch++)
	{
		samples_data[ch] = new Short_t[sample_total];
		for (Int_t sm = 0; sm < sample_total; sm++)
			samples_data[ch][sm] = 0.;
	}

	if (readdebug)
		printf("BinDataReader::alloc_data: data allocated\n");
}

void BinDataReader::delete_data()
{
	if (samples_data == NULL)
	{
		if (readdebug)
			printf("BinDataReader::delete_data: data void\n");
		return;
	}

	for (Int_t ch = 0; ch < channel_total; ch++)
		delete[] samples_data[ch];

	delete[] samples_data;
	samples_data = NULL;

	if (readdebug)
		printf("BinDataReader::delete_data: data unallocated: %p\n", samples_data);
}

void BinDataReader::Clear()
{
	if (DataFile)
		fclose(DataFile);

	Event_Num = 0;
	TimeStamp_date = 0;
	TimeStamp_ns = 0;

	sample_total = 0;
	board_serial = 0;

	delete_data();
}

Int_t BinDataReader::ReadDataHeader()
{
	if (DataFile == NULL)
	{
		printf("file was NOT opened\n");
		return -1;
	};

	//    const int max_header_size = 100;
	uint32_t buffer[10];

	if (readdebug)
		printf("\n\n --- Data Header ---\n");

	fseek(DataFile, 0, SEEK_SET);

	fread(buffer, sizeof(buffer[0]), 1, DataFile);
	if (feof(DataFile))
		return 0;

	if (buffer[0] == 0x2a502a50)
	{
		if (readdebug)
			printf("Magic found: no header\n");
		return 1;
	}
	else
	{
		/***
		fread(buffer, sizeof(buffer[0]), 7, DataFile);

		/***/
		while (buffer[0] != 0x2a502a50)
		{

			fread(buffer, sizeof(buffer[0]), 1, DataFile);
			if (feof(DataFile))
				return 0;

			printf("header[i] = %x\n", buffer[0]);
		}
		/****/
	}
	if (readdebug)
		printf("Header read\n");

	// Decode header
}

Int_t BinDataReader::ReadLastEvent()
{
	uint32_t buffer[3];
	UInt_t Magic, Payload_EH, last_event;

	fseek(DataFile, 0, SEEK_SET);
	fseek(DataFile, 0L, SEEK_END);
	Long_t file_size = ftell(DataFile);
	// file_size = _ftelli64(DataFile);
	printf("File size: %i bytes\n", file_size);

	// for(Int_t iPos = file_size-4; iPos > 0; iPos = iPos-sizeof(uint32_t))
	for (Long_t iPos = file_size - 4; iPos > 0; iPos = iPos - 1)
	{
		fseek(DataFile, iPos, SEEK_SET);

		fread(buffer, sizeof(buffer[0]), 1, DataFile);
		Magic = buffer[0];

		// if(readdebug) printf("[%i, %x] ", iPos, buffer[0]);

		if (Magic == 0x2a502a50)
		{
			fread(buffer, sizeof(buffer[0]), 2, DataFile);
			Payload_EH = buffer[0];
			last_event = buffer[1];

			if (readdebug)
				printf("\nMagic found at %i: payload:%x [%i], event %x [%i]\n",
					   iPos, buffer[0], buffer[0], buffer[1], buffer[1]);
			break;
		}
	}

	fseek(DataFile, 0, SEEK_SET);
	events_total = last_event;
	printf("Last event number: %i\n", last_event);

	return 1;
}

Int_t BinDataReader::FindMagic()
{
	uint32_t buffer;

	fread(&buffer, sizeof(buffer), 1, DataFile);
	if (feof(DataFile))
		return 0;

	while (buffer != 0x2a502a50)
	{
		if (buffer == 0x72617453)
			printf("Start run found: %08x\n", buffer);

		if (buffer == 0x706F7453)
			printf("Stop run found: %08x\n", buffer);

		// if(readdebug)printf("Wrong magic : %08x\n", buffer);
		fread(&buffer, sizeof(buffer), 1, DataFile);
		if (feof(DataFile))
			return 0;
	}

	if (readdebug)
		printf("Next magic found\n");

	return 1;
}

Int_t BinDataReader::SetFile(TString file_name)
{
	Clear();

	DataFile = fopen(file_name.Data(), "rb");
	if (DataFile == NULL)
	{
		printf("file \"%s\" was NOT opened\n", file_name.Data());
		return -1;
	}
	else
	{
		printf("\n\n=================================================\nfile \"%s\" was opened\n", file_name.Data());
	};

	ReadLastEvent();

	ReadDataHeader();

	return 1;
}

// return -1 error; 0 eof; 1 success
Int_t BinDataReader::ReadEvent(Int_t eventn, UInt_t board_serial_to_read)
{

	if (DataFile == NULL)
	{
		printf("file was NOT opened\n");
		return -1;
	};

	if (feof(DataFile))
		return 0;

	delete_data();
	Int_t EH_counter = 0;
	uint32_t buffer[2];

	if (readdebug)
		printf("\n\nEvent header -------------------------------------------\n");
	// Event Header
	fread(buffer, sizeof(buffer[0]), 2, DataFile);
	EH_counter += 2 * 4;
	if (feof(DataFile))
		return 0;

	UInt_t Payload_EH = buffer[0];
	UInt_t EvNum = buffer[1];
	Event_Num = EvNum;

	if (readdebug)
		printf("Event header:  size %i; evNo %i\n", buffer[0], buffer[1]);
	if (readdebug)
		printf("Event header X:  size %x; evNo %x\n", buffer[0], buffer[1]);

	for (Int_t board_counter = 0; EH_counter < Payload_EH; board_counter++)
	{
		if (readdebug)
			printf("\n\n========== Board %i [%i/%i] ==========\n", board_counter, EH_counter, Payload_EH);
		if (board_counter > 200)
		{
			printf("Boards number too much!!!");
			return -1;
		}

		// Device Event Header
		fread(buffer, sizeof(buffer[0]), 2, DataFile);
		if (feof(DataFile))
			return 0;
		EH_counter += 4;

		// if(readdebug) printf("\nBo");
		if (readdebug)
			printf("Dev Event header buffer [%08x] [%08x]:\n", buffer[0], buffer[1]);
		if (buffer[0] == 0x2a502a50)
		{ // data format must be cleatify !!!
			if (readdebug)
				printf("Magic found in header, finish event reading\n");

			Long_t position = ftell(DataFile);
			fseek(DataFile, position - sizeof(buffer[0]) * 2, SEEK_SET);

			break;
		}

		UInt_t DevSerial = buffer[0];
		UInt_t Payload_DEH;
		UInt_t DeviceID;
		Payload_DEH = buffer[1] & 0x00ffffff;
		buffer[1] = buffer[1] >> 24;
		DeviceID = buffer[1] & 0x000000ff;

		if (readdebug)
			printf("Dev Event header X:  Dev Serial: %x; size %x; Dev ID %x\n", DevSerial, Payload_DEH, DeviceID);
		if (readdebug)
			printf("Dev Event header:  Dev Serial: %i; size %i; Dev ID %i\n", DevSerial, Payload_DEH, DeviceID);
		// printf("card Serial: %i %x\n", DevSerial, DevSerial);
		if (readdebug)
			printf("requested Serial: %i %x\n", board_serial_to_read, board_serial_to_read);

		Bool_t ReadThisCard = false;
		if (board_serial_to_read == 0)
			ReadThisCard = (board_counter == 0);
		else
			ReadThisCard = (board_serial_to_read == DevSerial);

		if (readdebug)
			printf("This card will be read: %i\n", ReadThisCard);

		if (!ReadThisCard)
		{
			for (Int_t pl_count = 0; pl_count < Payload_DEH; pl_count += 4)
			{
				fread(buffer, sizeof(buffer[0]), 1, DataFile);
				if (feof(DataFile))
					return 0;

				EH_counter += 4;
			}
			if (readdebug)
				printf("Skipped %i bytes\n", Payload_DEH);

			continue;
		}

		board_serial = DevSerial;

		for (Int_t DEH_counter = 0; DEH_counter < Payload_DEH;)
		{
			// MStream Header
			fread(buffer, sizeof(buffer[0]), 1, DataFile);
			if (feof(DataFile))
				return 0;

			EH_counter += 4;
			DEH_counter += 4;
			if (readdebug)
				printf("\nMStream header [%i/%i] %i/%i [%08x]:\n",
					   EH_counter, Payload_EH, DEH_counter, Payload_DEH, buffer[0]);

			UInt_t MStream_Subtype, Payload_MSH, Subtype_MS;
			MStream_Subtype = buffer[0] & 0x00000003;
			buffer[0] = buffer[0] >> 2;
			Payload_MSH = buffer[0] & 0x3fffff;
			buffer[0] = buffer[0] >> 22;
			Subtype_MS = buffer[0] & 0x000000ff;

			if (readdebug)
				printf("MStream header X:  Subtype: %x; size %x; MS subtype %X\n", MStream_Subtype, Payload_MSH, Subtype_MS);
			if (readdebug)
				printf("MStream header:  Subtype: %i; size %i; MS subtype(ch) %i\n", MStream_Subtype, Payload_MSH, Subtype_MS);

			Int_t data_iter = 0;
			for (Int_t n_sub0 = 0; n_sub0 < Payload_MSH; n_sub0++)
			{
				// MStreams
				fread(buffer, sizeof(buffer[0]), 1, DataFile);
				if (feof(DataFile))
					return 0;

				EH_counter += 4;
				DEH_counter += 4;
				/***/
				if (readdebug)
					printf("MStream subtype [%i/%i] %i/%i [%08x]:\n",
						   EH_counter, Payload_EH, DEH_counter, Payload_DEH, buffer[0]);
				/***/
				if (DEH_counter > Payload_DEH)
				{
					printf("ERROR: Oversize MStream\n");
					printf("\n[%i/%i] %i/%i\n", EH_counter, Payload_EH, DEH_counter, Payload_DEH);

					return -1;
				}

				int32_t readout_mask_0;
				int32_t readout_mask_1;

				// decoding
				switch (MStream_Subtype)
				{
				case 0:
					if (n_sub0 == 0)
						TimeStamp_date = buffer[0];
					if (n_sub0 == 1)
						TimeStamp_ns = buffer[0];
					if (n_sub0 == 2)
						readout_mask_0 = buffer[0];
					if (n_sub0 == 3)
						readout_mask_1 = buffer[0];
					break;

				case 1:

					switch (n_sub0)
					{
					case 0: // timestamp 1
						// if(readdebug) printf("sample_data: %p\n", samples_data);
						if (ReadThisCard)
							if (samples_data == NULL)
							{
								sample_total = (Payload_MSH - 2) * 2;
								if (readdebug)
									printf("Samples: %i; data allocation\n", sample_total);
								alloc_data();
							}
						break;

					case 1: // timestamp 2
						break;

					default: // samples
						// printf("buf %x ", buffer[0]);
						// printf("n_sub = %i\n", n_sub0);
						Short_t sample1 = buffer[0] & 0x0000ffff;
						buffer[0] = buffer[0] >> 16;
						Short_t sample0 = buffer[0] & 0x0000ffff;

						if (ReadThisCard)
						{
							samples_data[Subtype_MS][data_iter++] = sample0;
							samples_data[Subtype_MS][data_iter++] = sample1;
							if (readdebug)
								printf(" ++data_iter: %i, val0 %i, val1 %i\n", data_iter, sample0, sample1);
						}
						break;
					}

					break;
				}

			} // subevent

		} // Mstream
	} // board

	FindMagic();

	if (readdebug)
	{
	}

	return 1;
}

// #####################################################################
void BinDataReader::Calculate_fit_harmonics(event_fit_data_struct &result_fit_event, Int_t &event_counter, Int_t event_num, Int_t ch_num, Int_t gate_beg, Int_t gate_end, Int_t gate_maximum_beg, Int_t gate_maximum_end, TString source_path, TString run_name, Float_t ampl_scale, TObjArray *check_fit_arr)
{

	if (readdebug)
		printf("Calculating fit harmonics -------------------------------------------\n");
	if (readdebug)
		printf("h_num: %i; gate_beg: %i; gate_end %i; ampl_scale %i\n", ch_num, gate_beg, gate_end, ampl_scale);

	Int_t ch_iter = ch_num;

	result_fit_event.reset();
	if (samples_data == NULL)
		return;

	Float_t *scale_samples = new Float_t[sample_total];
	for (Int_t isaml = 0; isaml < sample_total; isaml++)
		scale_samples[isaml] = ampl_scale * (float)samples_data[ch_iter][isaml];

	// Zero level calculation
	const int n_gates = 3;
	int gate_npoints = (int)floor((gate_beg - 2.) / n_gates);

	Float_t gates_mean[n_gates], gates_rms[n_gates];
	for (Int_t igate = 0; igate < n_gates; igate++)
		MeanRMScalc(scale_samples, gates_mean + igate, gates_rms + igate, igate * gate_npoints, (igate + 1) * gate_npoints);

	int best_gate = 0;
	for (Int_t igate = 0; igate < n_gates; igate++)
		if (gates_rms[igate] < gates_rms[best_gate])
			best_gate = igate;

	Float_t zero_level_ = gates_mean[best_gate];
	Short_t MAX_in_gate_ = -32760;
	Int_t time_max_in_gate_ = 0;

	for (UShort_t sample_curr = 0; sample_curr < sample_total; sample_curr++)
	{
		Float_t val_sample = scale_samples[sample_curr];
		if ((sample_curr >= gate_maximum_beg) && (sample_curr <= gate_maximum_end))
		{
			if (val_sample > MAX_in_gate_)
			{
				MAX_in_gate_ = (Short_t)val_sample;
				time_max_in_gate_ = sample_curr;
			}
		}
	}

	Double_t integral_in_gate_ = 0.;

	for (UShort_t sample_curr = 0; sample_curr < sample_total; sample_curr++)
	{
		Float_t val_sample = scale_samples[sample_curr];

		if ((sample_curr >= gate_beg) && (sample_curr <= gate_end)) // in of gate
		{
			integral_in_gate_ += (val_sample - zero_level_);
			// printf("%f \n", integral_in_gate_);
		}
	}

	if (MAX_in_gate_ - zero_level_ > 0)
	{
		Bool_t IsDebug = false;
		if (IsDebug)
			printf("event for harmonic %i event counter %i\n", event_num, event_counter);

		// Calculating timing
		Float_t Amplitude = MAX_in_gate_ - zero_level_;
		Float_t trsh_03 = zero_level_ + Amplitude * 0.3;
		Float_t trsh_09 = zero_level_ + Amplitude * 0.9;

		Int_t point = time_max_in_gate_;
		Float_t front_time_beg_03 = GoToLevel(scale_samples, trsh_03, &point, -1, sample_total);

		point = time_max_in_gate_;
		Float_t front_time_end = GoToLevel(scale_samples, trsh_09, &point, -1, sample_total);

		// Fitting

		if (readdebug)
			printf("\n\nFit signal procedure --- \n");

		PronyFitter Pfitter(2, 2, gate_beg, gate_end, sample_total);
		// Pfitter.SetDebugMode(1);
		Pfitter.SetWaveform(scale_samples, zero_level_);
		int SignalBeg = Pfitter.CalcSignalBegin(front_time_beg_03, front_time_end);
		Int_t best_signal_begin = Pfitter.ChooseBestSignalBeginHarmonics(SignalBeg - 1, SignalBeg + 2);
		double *harmonics;
		if (best_signal_begin > gate_beg)
		{
			Pfitter.SetSignalBegin(best_signal_begin);
			Pfitter.CalculateFitHarmonics();
			Pfitter.CalculateFitAmplitudes();
			harmonics = Pfitter.GetHarmonics();
			Float_t fit_integral = Pfitter.GetIntegral(gate_beg, gate_end);
			Float_t fit_chi2 = Pfitter.GetChiSquare(gate_beg, gate_end, time_max_in_gate_);
			Float_t fit_R2 = Pfitter.GetRSquare(gate_beg, gate_end);
			// if (IsDebug)
			printf("fit integral %.0f integral %.0f chi2 %.1f R2 %.3f\n", fit_integral, integral_in_gate_, fit_chi2, fit_R2);

			if (fit_R2 < 0.02)
			{
				event_counter++;
				result_fit_event.first_harmonic = (Float_t)harmonics[1];
				result_fit_event.second_harmonic = (Float_t)harmonics[2];
				result_fit_event.fit_chi2 = fit_chi2;
				result_fit_event.fit_R2 = fit_R2;
				result_fit_event.integral_in_gate = (Float_t)integral_in_gate_;
				result_fit_event.fit_integral_in_gate = fit_integral;
				TString signal_name = Form("ch_num %i fit_integral %.0f integral %.0f chi2 %.1f fit_R2 %.3f",
										   ch_iter, fit_integral, integral_in_gate_, fit_chi2, fit_R2);
				if (event_counter < 5)
					Pfitter.DrawFit(check_fit_arr, signal_name);
			}

			if (readdebug)
				printf("------------------------ \n");
		}
	}

	delete[] scale_samples;

	if (readdebug)
		printf("\n-----------------------------------------------------------------\n");
}
// #####################################################################
/*****/

// #####################################################################
void BinDataReader::Calculate_waveform(event_data_struct &result_event, Int_t ch_num, Int_t gate_beg, Int_t gate_end, Int_t gate_maximum_beg, Int_t gate_maximum_end, Float_t ampl_scale, Bool_t IsFIT, Double_t first_fit_harmonic, Double_t second_fit_harmonic, TString FIT_QA_mode, TObjArray *check_fit_arr, Float_t *fitQA_arg_arr)
{

	if (readdebug)
		printf("Calculating wave form -------------------------------------------\n");
	if (readdebug)
		printf("h_num: %i; gate_beg: %i; gate_end %i; ampl_scale %i; IsFIT %i\n", ch_num, gate_beg, gate_end, ampl_scale, IsFIT);

	Int_t ch_iter = ch_num;

	result_event.reset();
	if (samples_data == NULL)
		return;

	Float_t *scale_samples = new Float_t[sample_total];
	for (Int_t isaml = 0; isaml < sample_total; isaml++)
		scale_samples[isaml] = ampl_scale * (float)samples_data[ch_iter][isaml];

	Int_t samples_in_gate = 0;
	Int_t samples_out_gate = 0;

	// Zero level calculation
	const int n_gates = 3;
	int gate_npoints = (int)floor((gate_beg - 2.) / n_gates);

	Float_t gates_mean[n_gates], gates_rms[n_gates];
	for (Int_t igate = 0; igate < n_gates; igate++)
		MeanRMScalc(scale_samples, gates_mean + igate, gates_rms + igate, igate * gate_npoints, (igate + 1) * gate_npoints);

	int best_gate = 0;
	for (Int_t igate = 0; igate < n_gates; igate++)
		if (gates_rms[igate] < gates_rms[best_gate])
			best_gate = igate;

	result_event.zero_level = gates_mean[best_gate];
	result_event.zero_level_RMS = gates_rms[best_gate];

	// PASS 1
	result_event.mean_in_gate = 0.;
	result_event.mean_out_gate = 0.;
	result_event.MAX_in_gate = -32760;
	result_event.MIN_in_gate = 32767;
	result_event.MAX_out_gate = -32760;
	result_event.MIN_out_gate = 32767;

	for (UShort_t sample_curr = 0; sample_curr < sample_total; sample_curr++)
	{
		Float_t val_sample = scale_samples[sample_curr];

		// if(sample_curr < gate_beg) //out of gate
		if ((sample_curr < gate_beg) || (sample_curr > gate_end)) // out of gate
		{
			result_event.mean_out_gate += val_sample;

			if ((Short_t)val_sample < result_event.MIN_out_gate)
				result_event.MIN_out_gate = (Short_t)val_sample;
			if ((Short_t)val_sample > result_event.MAX_out_gate)
				result_event.MAX_out_gate = (Short_t)val_sample;

			samples_out_gate++;
		}

		if ((sample_curr >= gate_beg) && (sample_curr <= gate_end)) // in of gate
		{
			result_event.mean_in_gate += val_sample;

			if (val_sample < result_event.MIN_in_gate)
			{
				result_event.MIN_in_gate = (Short_t)val_sample;
				result_event.time_min_in_gate = sample_curr;
			}

			if ((sample_curr >= gate_maximum_beg) && (sample_curr <= gate_maximum_end))
			{
				if (val_sample > result_event.MAX_in_gate)
				{
					result_event.MAX_in_gate = (Short_t)val_sample;
					result_event.time_max_in_gate = sample_curr;
				}
			}
			samples_in_gate++;
		}
	}
	result_event.mean_in_gate /= (float)samples_in_gate;
	result_event.mean_out_gate /= (float)samples_out_gate;
	// result_event.integral_in_gate /= (float)samples_in_gate;

	// PASS 2
	Double_t integral_in_gate_ = 0.;
	Double_t integral_in_gate_noises_2 = 0.;
	Double_t integral_in_gate_noises = 0.;
	Double_t integral_in_gate_noises_1 = 0.;
	Double_t gate_left = 120.;
	Double_t gate_right = 200.;
	result_event.RMS_in_gate = 0.;
	result_event.RMS_out_gate = 0.;
	for (UShort_t sample_curr = 0; sample_curr < sample_total; sample_curr++)
	{
		Float_t val_sample = scale_samples[sample_curr];
		// if(sample_curr < gate_beg) //out of gate
		if ((sample_curr < gate_beg) || (sample_curr > gate_end)) // out of gate
		{
			result_event.RMS_out_gate +=
				(val_sample - result_event.mean_out_gate) * (val_sample - result_event.mean_out_gate);
		}

		if ((sample_curr >= gate_beg) && (sample_curr <= gate_end)) // in of gate
		{
			integral_in_gate_ += (val_sample - result_event.zero_level);
			// printf("%f \n", integral_in_gate_);
			result_event.RMS_in_gate +=
				(val_sample - result_event.mean_in_gate) * (val_sample - result_event.mean_in_gate);
		}
		if ((sample_curr > gate_left) && (sample_curr < (gate_right + gate_left) / 2))
			integral_in_gate_noises_1 += (val_sample);
		if ((sample_curr > (gate_right + gate_left) / 2) && (sample_curr < gate_right))
			integral_in_gate_noises_2 += (val_sample);
		//	integral_in_gate_noises = 2*(gate_end-gate_beg)/(gate_right-gate_left)*(integral_in_gate_noises_2-integral_in_gate_noises_1);
		integral_in_gate_noises = (integral_in_gate_noises_2 - integral_in_gate_noises_1);

		if (integral_in_gate_noises == 0)
			integral_in_gate_noises = 10000;
	}

	result_event.integral_in_gate = (Int_t)integral_in_gate_;
	result_event.integral_in_gate_noises = (Int_t)integral_in_gate_noises;

	result_event.RMS_in_gate = TMath::Sqrt(result_event.RMS_in_gate / (float)samples_in_gate);
	result_event.RMS_out_gate = TMath::Sqrt(result_event.RMS_out_gate / (float)samples_out_gate);
	// printf("res %i \n", result_event.integral_in_gate);

	// Calculating timing
	Float_t Amplitude = result_event.MAX_in_gate - result_event.zero_level;
	Float_t trsh_01 = result_event.zero_level + Amplitude * 0.1;
	Float_t trsh_03 = result_event.zero_level + Amplitude * 0.3;
	Float_t trsh_05 = result_event.zero_level + Amplitude * 0.5;
	Float_t trsh_09 = result_event.zero_level + Amplitude * 0.9;

	Int_t point = result_event.time_max_in_gate;
	Float_t front_time_beg = GoToLevel(scale_samples, trsh_01, &point, -1, sample_total);

	point = result_event.time_max_in_gate;
	Float_t front_time_beg_03 = GoToLevel(scale_samples, trsh_03, &point, -1, sample_total);

	point = result_event.time_max_in_gate;
	Float_t time_front = GoToLevel(scale_samples, trsh_05, &point, -1, sample_total);

	point = result_event.time_max_in_gate;
	Float_t front_time_end = GoToLevel(scale_samples, trsh_09, &point, -1, sample_total);

	Float_t time = (front_time_end + front_time_beg) * 0.5;
	// printf("begin: %f; end: %f; time_front: %f; time: %f\n", front_time_beg, front_time_end, time_front, time);
	result_event.time_cross = time_front;
	result_event.time_half = time;

	result_event.timestamp = TimeStamp_date * 1e9 + (TimeStamp_ns >> 2);

	// Fitting
	if (IsFIT)
	{
		if (readdebug)
			printf("\n\nFit signal procedure --- \n");
		if (result_event.MAX_in_gate - result_event.zero_level > 0) // cosmic ZS selection
		{
			PronyFitter Pfitter(2, 2, gate_beg, gate_end, sample_total);
			// Pfitter.SetDebugMode(1);
			Pfitter.SetWaveform(scale_samples, result_event.zero_level);
			// Pfitter.MakePileUpRejection(result_event.time_max_in_gate+1);
			Pfitter.SetExternalHarmonics(first_fit_harmonic, second_fit_harmonic);
			if (!isfinite(front_time_end))
				front_time_end = result_event.time_max_in_gate - 1;
			int SignalBeg = round(front_time_end -
								  (log(second_fit_harmonic) - log(first_fit_harmonic)) / (second_fit_harmonic - first_fit_harmonic));
			// Pfitter.SetSignalBegin(SignalBeg);
			Int_t best_signal_begin = Pfitter.ChooseBestSignalBegin(SignalBeg - 1, SignalBeg + 1);
			Pfitter.SetSignalBegin(best_signal_begin);
			Pfitter.CalculateFitAmplitudes();
			result_event.FIT_integral_in_gate = Pfitter.GetIntegral(gate_beg, gate_end);
			result_event.FIT_MAX_in_gate = Pfitter.GetMaxAmplitude();
			// result_event.FIT_MAX_in_gate = Pfitter.GetFitValue((Int_t)result_event.time_max_in_gate);
			result_event.FIT_Chi2 = Pfitter.GetChiSquare(gate_beg, gate_end, result_event.time_max_in_gate);
			result_event.FIT_R2_gate = Pfitter.GetRSquare(gate_beg, gate_end);
			result_event.FIT_R2_signal = Pfitter.GetRSquareSignal();
			result_event.FIT_zero_level = Pfitter.GetZeroLevel();
			result_event.FIT_first_harmonic = (Float_t)first_fit_harmonic;
			result_event.FIT_second_harmonic = (Float_t)second_fit_harmonic;
			Float_t FIT_amplitude = result_event.FIT_MAX_in_gate - result_event.FIT_zero_level;
			result_event.FIT_time_half = Pfitter.GetX(result_event.FIT_zero_level + FIT_amplitude / 2., SignalBeg - 1, round(front_time_end), 0.25);
			result_event.FIT_time_max_in_gate = (Pfitter.GetSignalBeginFromPhase() > 0) ? Pfitter.GetSignalBeginFromPhase() +
																							  (log(second_fit_harmonic) - log(first_fit_harmonic)) / (second_fit_harmonic - first_fit_harmonic)
																						: 0;

			// FIT QA
			if (!FIT_QA_mode.Contains("false"))
			{
				if (check_fit_arr->GetLast() + 1 < (Int_t)fitQA_arg_arr[0])
				{
					Bool_t FIT_QA = true;
					if (FIT_QA_mode.Contains("default"))
						FIT_QA *= true;
					if (FIT_QA_mode.Contains("neg_fit_integral"))
						FIT_QA *= result_event.FIT_integral_in_gate < 0;
					if (FIT_QA_mode.Contains("diff_fit_int_and_wf_int"))
						FIT_QA *=
							abs(result_event.FIT_integral_in_gate - result_event.integral_in_gate) > (fitQA_arg_arr[2]) / 100. * abs(result_event.FIT_integral_in_gate);
					if (FIT_QA_mode.Contains("diff_fit_ampl_and_wf_ampl"))
						FIT_QA *=
							abs(result_event.FIT_MAX_in_gate - result_event.zero_level - result_event.MAX_in_gate + result_event.zero_level) >
							(fitQA_arg_arr[3]) / 100. * abs(result_event.MAX_in_gate - result_event.zero_level);
					if (FIT_QA_mode.Contains("saturation"))
						FIT_QA *= result_event.integral_in_gate > fitQA_arg_arr[4];
					if (FIT_QA_mode.Contains("integral_region"))
						FIT_QA *= (result_event.integral_in_gate > fitQA_arg_arr[5]) &&
								  (result_event.integral_in_gate < fitQA_arg_arr[6]);
					if (FIT_QA_mode.Contains("amlitude_region"))
						FIT_QA *= (result_event.MAX_in_gate - result_event.zero_level > fitQA_arg_arr[7]) &&
								  (result_event.MAX_in_gate - result_event.zero_level < fitQA_arg_arr[8]);
					if (FIT_QA_mode.Contains("chi_square"))
						FIT_QA *= (result_event.FIT_Chi2 > fitQA_arg_arr[9]) &&
								  (result_event.FIT_Chi2 < fitQA_arg_arr[10]);
					if (FIT_QA_mode.Contains("R_square"))
						FIT_QA *= (result_event.FIT_R2_gate > fitQA_arg_arr[11]) &&
								  (result_event.FIT_R2_gate < fitQA_arg_arr[12]);
					if (FIT_QA_mode.Contains("R_square_signal"))
						FIT_QA *= (result_event.FIT_R2_signal > fitQA_arg_arr[13]) &&
								  (result_event.FIT_R2_signal < fitQA_arg_arr[14]);
					if (FIT_QA_mode.Contains("time_half"))
						FIT_QA *= (result_event.FIT_time_half > fitQA_arg_arr[15]) &&
								  (result_event.FIT_time_half < fitQA_arg_arr[16]);
					if (FIT_QA_mode.Contains("time_max_in_gate"))
						FIT_QA *= (result_event.FIT_time_max_in_gate > fitQA_arg_arr[17]) &&
								  (result_event.FIT_time_max_in_gate < fitQA_arg_arr[18]);
					Bool_t Selected_channel = true;
					if (fitQA_arg_arr[1] != -1)
						if (ch_iter != (Int_t)fitQA_arg_arr[1])
							Selected_channel = false;

					if (FIT_QA && Selected_channel)
					{
						TString signal_name = Form("FIT QA mode '%s' ch_num %i fit_integral %i integral %i chi2 %.1f R2 %.2f",
												   FIT_QA_mode.Data(), ch_iter, (Int_t)result_event.FIT_integral_in_gate, result_event.integral_in_gate,
												   result_event.FIT_Chi2, result_event.FIT_R2_gate);
						Pfitter.DrawFit(check_fit_arr, signal_name);
					}
				}
			}
		}

		if (readdebug)
			printf("------------------------ \n");
	}

	// Pule-up selection
	const Bool_t print_debug = false;
	const Float_t bl_trsh = 0.2;
	const Int_t filter_len_fr = 5;
	const Float_t min_delta_fr = -10.;
	const Int_t filter_len_bk = 5;
	const Float_t min_delta_bk = -10.;

	Float_t pu_base_line = result_event.zero_level + ((Amplitude > 1000.) ? Amplitude * 0.1 : 100.);
	if (print_debug)
		printf("Ampl %f;  bline: %f\n", Amplitude, pu_base_line);

	Int_t last_point_front;
	Int_t last_point_back;
	Float_t last_value_front;
	Float_t last_value_back;
	Bool_t is_monotonous_front = true;
	Bool_t is_monotonous_back = true;
	Bool_t is_undertrs_front = true;
	Bool_t is_undertrs_back = true;
	Bool_t is_pile_up;

	Float_t filtered_past;
	Float_t filter_diff;

	// signal front
	filtered_past = 0.;
	filter_diff = 0.;
	for (Int_t sample_curr = result_event.time_max_in_gate - filter_len_fr; sample_curr >= gate_beg; sample_curr--)
	{
		last_point_front = sample_curr;
		Float_t val_sample = scale_samples[sample_curr];

		Float_t filtered_sample = 0.;
		for (Int_t f_s = sample_curr; f_s < sample_curr + filter_len_fr; f_s++)
		{
			filtered_sample += scale_samples[f_s];
		}
		filtered_sample /= filter_len_fr;
		last_value_front = filtered_sample;

		if (filtered_past != 0.)
			filter_diff = filtered_past - filtered_sample;

		if (print_debug)
			printf("front t%i : l%f : f%f : fd%f\n", sample_curr, min_delta_fr, filtered_sample, filter_diff);

		if (filtered_sample < pu_base_line)
			break;

		if (filter_diff < min_delta_fr)
		{
			is_monotonous_front = false;
			break;
		}

		filtered_past = filtered_sample;
	}

	// signal back
	filtered_past = 0.;
	filter_diff = 0.;
	for (Int_t sample_curr = result_event.time_max_in_gate + filter_len_bk; sample_curr < gate_end; sample_curr++)
	{
		last_point_back = sample_curr;
		Float_t val_sample = scale_samples[sample_curr];

		Float_t filtered_sample = 0.;
		for (Int_t f_s = sample_curr; f_s > sample_curr - filter_len_bk; f_s--)
		{
			filtered_sample += scale_samples[f_s];
		}
		filtered_sample /= filter_len_bk;
		last_value_back = filtered_sample;

		if (filtered_past != 0.)
			filter_diff = filtered_past - filtered_sample;

		if (print_debug)
			printf("back t%i : a%f : f%f : fd%f\n", sample_curr, val_sample, filtered_sample, filter_diff);

		if (filtered_sample < pu_base_line)
			break;

		if (filter_diff < min_delta_bk)
		{
			is_monotonous_back = false;
			break;
		}

		filtered_past = filtered_sample;
	}

	// under treshold front
	for (Int_t sample_curr = last_point_front; sample_curr > gate_beg; sample_curr--)
	{
		Float_t val_sample = scale_samples[sample_curr];
		Float_t filtered_sample = 0.;
		for (Int_t f_s = sample_curr; f_s < sample_curr + filter_len_fr; f_s++)
		{
			filtered_sample += scale_samples[f_s];
		}
		filtered_sample /= filter_len_fr;

		if (print_debug)
			printf("utf t%i : v%f : l%f : f%f\n", sample_curr, val_sample, last_value_front, filtered_sample);

		if (filtered_sample > last_value_front)
		{
			is_undertrs_front = false;
			break;
		}
	}

	// under treshold back
	for (Int_t sample_curr = last_point_back; sample_curr < gate_end; sample_curr++)
	{
		Float_t val_sample = scale_samples[sample_curr];
		Float_t filtered_sample = 0.;
		for (Int_t f_s = sample_curr; f_s > sample_curr - filter_len_bk; f_s--)
		{
			filtered_sample += scale_samples[f_s];
		}
		filtered_sample /= filter_len_bk;

		if (print_debug)
			printf("utb t%i : v%f : l%f : f%f\n", sample_curr, val_sample, last_value_back, filtered_sample);

		if (filtered_sample > last_value_back)
		{
			is_undertrs_back = false;
			break;
		}
	}

	is_pile_up = !(is_monotonous_back && is_monotonous_front && is_undertrs_back && is_undertrs_front);
	// is_pile_up = !(is_monotonous_back && is_monotonous_front);

	if (print_debug)
		if (is_pile_up)
			printf("PILE-UP\n");

	result_event.is_pile_up = is_pile_up;

	delete[] scale_samples;

	if (readdebug)
		printf("\n-----------------------------------------------------------------\n");
}
// #####################################################################
/*****/

#endif // BINDATAFORMAT_H
