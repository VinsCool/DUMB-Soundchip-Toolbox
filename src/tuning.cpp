// POKEY Frequencies Calculator
// by VinsCool
// Based on the code originally used for the Pitch Calculations in Raster Music Tracker 1.31+
// Borrowed for DUMB POKEY Toolbox with few changes 
//-----------------------------------------------------------------------------------------------------------//

#include "tuning.h"
#include "general.h"

const char* g_notes[] =
{
	"C-", "C#", "D-", "D#", "E-", "F-", "F#", "G-", "G#", "A-", "A#", "B-"
};

double CTuning::generate_pitch(int soundchip, int freq) 
{
	//TODO: A lot of stuff, aaaaaaaaaa 
	//if (soundchip != SOUNDCHIP_POKEY) return get_pitch(soundchip, freq);

	//variables for pitch calculation, divisors must never be 0!
	double divisor = 1;	
	int coarse_divisor = 1;
	int cycle = 1;
	
	//hacked up fix for calculating tables like now aaaaaaa
	int audctl = g_audctl;
	int timbre = g_timbre;
	int distortion = g_timbre & 0xf0;
	int audf = freq;

	//register variables 
	//int distortion = audc & 0xf0;
	//int skctl = 0;	//not yet implemented in calculations
	//bool TWO_TONE = (skctl == 0x8B) ? 1 : 0;
	//bool CLOCK_15 = audctl & 0x01;
	//bool HPF_CH24 = audctl & 0x02;
	//bool HPF_CH13 = audctl & 0x04;
	//bool JOIN_34 = audctl & 0x08;
	//bool JOIN_12 = audctl & 0x10;
	//bool CH3_179 = audctl & 0x20;
	//bool CH1_179 = audctl & 0x40;
	bool POLY9 = audctl & 0x80;

	//combined modes for some special output...
	//bool JOIN_16BIT = ((JOIN_12 && CH1_179 && (channel == 1 || channel == 5)) || (JOIN_34 && CH3_179 && (channel == 3 || channel == 7))) ? 1 : 0;
	//bool CLOCK_179 = ((CH1_179 && (channel == 0 || channel == 4)) || (CH3_179 && (channel == 2 || channel == 6))) ? 1 : 0;
	
	bool JOIN_16BIT = g_16bit_freq; 
	bool CLOCK_179 = g_179mhz_freq;
	bool CLOCK_15 = g_15khz_freq;
	
	if (JOIN_16BIT || CLOCK_179) CLOCK_15 = 0;	//override, these 2 take priority over 15khz mode if they are enabled at the same time


	//TODO: Sawtooth generation needs to be optimal in order to compromise the high pitched hiss versus the tuning accuracy
	//SAWTOOTH = (CH1_179 && CH3_179 && HPF_CH13 && (dist == 0xA0 || dist == 0xE0) && (i == 0 || i == 4)) ? 1 : 0;
	//SAWTOOTH_INVERTED = 0;
	//if (i % 4 == 0)	//only in valid sawtooth channels
	//audf3 = g_atarimem[idx[i + 2]];


	//TODO: apply Two-Tone timer offset into calculations when channel 1+2 are linked in 1.79mhz mode
	//This would help generating tables using patterns discovered by synthpopalooza
	if (JOIN_16BIT) cycle = 7;
	else if (CLOCK_179) cycle = 4;
	else coarse_divisor = (CLOCK_15) ? 114 : 28;

	//Many combinations depend entirely on the Modulo of POKEY frequencies to generate different tones
	//If a known value provide unstable results, it may be avoided on purpose 
	bool MOD3 = ((audf + cycle) % 3 == 0);
	bool MOD5 = ((audf + cycle) % 5 == 0);
	bool MOD7 = ((audf + cycle) % 7 == 0);
	bool MOD15 = ((audf + cycle) % 15 == 0);
	bool MOD31 = ((audf + cycle) % 31 == 0);
	bool MOD73 = ((audf + cycle) % 73 == 0);

	switch (distortion)
	{
	case 0x00:
		if (POLY9)
		{
			divisor = 255.5;	//Metallic Buzzy
			if (MOD7 || (!CLOCK_15 && !CLOCK_179 && !JOIN_16BIT)) divisor = 36.5;	//seems to only sound "uniform" in 64kHz mode for some reason 
			if (MOD31 || MOD73) return 0;	//MOD31 and MOD73 values are invalid 
		}
		break;

	case 0x20:
	case 0x60:	//Duplicate of Distortion 2
		divisor = 31;
		if (MOD31) return 0;
		break;

	case 0x40:
		divisor = 232.5;		//Buzzy tones, neither MOD3 or MOD5 or MOD31
		if (MOD3 || CLOCK_15) divisor = 77.5;	//Smooth tones, MOD3 but not MOD5 or MOD31
		if (MOD5) divisor = 46.5;	//Unstable tones #1, MOD5 but not MOD3 or MOD31
		if (MOD31) divisor = (MOD3 || MOD5) ? 2.5 : 7.5;	//Unstables Tones #2 and #3, MOD31, with MOD3 or MOD5 
		if (MOD15 || (MOD5 && CLOCK_15)) return 0;	//Both MOD3 and MOD5 at once are invalid 
		break;

	case 0x80:
		if (POLY9)
		{
			divisor = 255.5;	//Metallic Buzzy
			if (MOD7 || (!CLOCK_15 && !CLOCK_179 && !JOIN_16BIT)) divisor = 36.5;	//seems to only sound "uniform" in 64kHz mode for some reason 
			if (MOD73) return 0;	//MOD73 values are invalid
		}
		break;

	case 0xC0:
		divisor = 7.5;	//Gritty tones, neither MOD3 or MOD5
		if (MOD3 || CLOCK_15) divisor = 2.5;	//Buzzy tones, MOD3 but not MOD5
		if (MOD5) divisor = 1.5;	//Unstable Buzzy tones, MOD5 but not MOD3
		if (MOD15 || (MOD5 && CLOCK_15)) return 0;	//Both MOD3 and MOD5 at once are invalid 
		
		if (timbre == TIMBRE_BUZZY_C && divisor != 2.5) return 0;	//not buzzy
		if (timbre == TIMBRE_GRITTY_C && divisor != 7.5) return 0;	//not gritty
		if (timbre == TIMBRE_UNSTABLE_C && divisor != 1.5) return 0;	//not unstable 
		
		break;		
	}
	
	return get_pokey_pitch(audf, coarse_divisor, divisor, cycle);


}

int CTuning::generate_freq(int soundchip, int semitone) 
{
	//TODO: A lot of stuff, aaaaaaaaaa 
	double pitch = GetTruePitch(g_basetuning, g_temperament, g_basenote, semitone); 
	
	//if (soundchip != SOUNDCHIP_POKEY) return get_freq(soundchip, pitch); 
	
	//variables for pitch calculation, divisors must never be 0!
	double divisor = 1;	
	int coarse_divisor = 1;
	int cycle = 1;
	
	//hacked up fix for calculating tables like now aaaaaaa
	int audctl = g_audctl;
	int timbre = g_timbre;
	int distortion = g_timbre & 0xf0;
	//int audf = freq;

	//register variables 
	//int distortion = audc & 0xf0;
	//int skctl = 0;	//not yet implemented in calculations
	//bool TWO_TONE = (skctl == 0x8B) ? 1 : 0;
	//bool CLOCK_15 = audctl & 0x01;
	//bool HPF_CH24 = audctl & 0x02;
	//bool HPF_CH13 = audctl & 0x04;
	//bool JOIN_34 = audctl & 0x08;
	//bool JOIN_12 = audctl & 0x10;
	//bool CH3_179 = audctl & 0x20;
	//bool CH1_179 = audctl & 0x40;
	bool POLY9 = audctl & 0x80;

	//combined modes for some special output...
	//bool JOIN_16BIT = ((JOIN_12 && CH1_179 && (channel == 1 || channel == 5)) || (JOIN_34 && CH3_179 && (channel == 3 || channel == 7))) ? 1 : 0;
	//bool CLOCK_179 = ((CH1_179 && (channel == 0 || channel == 4)) || (CH3_179 && (channel == 2 || channel == 6))) ? 1 : 0;
	
	bool JOIN_16BIT = g_16bit_freq; 
	bool CLOCK_179 = g_179mhz_freq;
	bool CLOCK_15 = g_15khz_freq;
	
	if (JOIN_16BIT || CLOCK_179) CLOCK_15 = 0;	//override, these 2 take priority over 15khz mode if they are enabled at the same time

	
/*
	//variables for pitch calculation, divisors must never be 0!
	double divisor = 1;
	int coarse_divisor = 1;
	int cycle = 1;

	//register variables 
	//int distortion = timbre & 0xF0;
	//int skctl = 0;	//not yet implemented in calculations
	//bool TWO_TONE = (skctl == 0x8B) ? 1 : 0;
	bool CLOCK_15 = audctl & 0x01;
	bool HPF_CH24 = audctl & 0x02;
	bool HPF_CH13 = audctl & 0x04;
	bool JOIN_34 = audctl & 0x08;
	bool JOIN_12 = audctl & 0x10;
	bool CH3_179 = audctl & 0x20;
	bool CH1_179 = audctl & 0x40;
	bool POLY9 = audctl & 0x80;

	//combined modes for some special output... 
	//the channel number doesn't actually matter for creating tables, so the parameter is omitted
	bool JOIN_16BIT = ((JOIN_12 && CH1_179) || (JOIN_34 && CH3_179)) ? 1 : 0;
	bool CLOCK_179 = (CH1_179 || CH3_179) ? 1 : 0;
	if (JOIN_16BIT || CLOCK_179) CLOCK_15 = 0;	//override, these 2 take priority over 15khz mode if they are enabled at the same time
*/

	//TODO: apply Two-Tone timer offset into calculations when channel 1+2 are linked in 1.79mhz mode
	//This would help generating tables using patterns discovered by synthpopalooza
	if (JOIN_16BIT) cycle = 7;
	else if (CLOCK_179) cycle = 4;
	else coarse_divisor = (CLOCK_15) ? 114 : 28;


	//Many combinations depend entirely on the Modulo of POKEY frequencies to generate different tones
	//If a known value provide unstable results, it may be avoided on purpose 
	bool MOD3 = 0;
	bool MOD5 = 0;
	bool MOD7 = 0;
	bool MOD15 = 0;
	bool MOD31 = 0;
	bool MOD73 = 0;

	//Use the modulo flags to make sure the correct timbre will be output
	switch (timbre)
	{
	case TIMBRE_PINK_NOISE:
		break;

	case TIMBRE_BROWNIAN_NOISE:
		divisor = 36.5;	//Brownian noise, not MOD31 and not MOD73
		break;

	case TIMBRE_FUZZY_NOISE:
		divisor = 255.5;	//Fuzzy noise, not MOD7, not MOD31 and not MOD73
		break;

	case TIMBRE_BELL:
		divisor = 31;	//Bell tones, not MOD31
		break;

	case TIMBRE_BUZZY_4:
		divisor = 232.5;	//Buzzy tones, neither MOD3 or MOD5 or MOD31
		break;

	case TIMBRE_SMOOTH_4:
		divisor = 77.5;	//Smooth tones, MOD3 but not MOD5 or MOD31
		break;

	case TIMBRE_WHITE_NOISE:
		break;

	case TIMBRE_METALLIC_NOISE:
		divisor = 36.5;	//Metallic noise, not MOD73
		break;

	case TIMBRE_BUZZY_NOISE:
		divisor = 255.5;	//Buzzy noise, not MOD7 and not MOD73
		break;

	case TIMBRE_PURE:
		break;

	case TIMBRE_GRITTY_C:
		divisor = 7.5;	//Gritty tones, neither MOD3 or MOD5
		break;

	case TIMBRE_BUZZY_C:
		divisor = 2.5;	//Buzzy tones, MOD3 but not MOD5
		break;

	case TIMBRE_UNSTABLE_C:
		divisor = 1.5;	//Unstable Buzzy tones, MOD5 but not MOD3
		break;

	default:
		//Distortion A is assumed if no valid parameter is supplied 
		break;

	}

	//generate the table using all the initialised parameters 
	//for (int i = 0; i < length; i++)
	//{
		//get the current semitone 
		//int note = i + semitone;

		//calculate the reference pitch using the semitone as an offset
		//double pitch = GetTruePitch(g_basetuning, g_temperament, g_basenote, semitone);

		//get the nearest POKEY frequency using the reference pitch
		int audf = get_pokey_audf(pitch, coarse_divisor, divisor, cycle);

		//TODO: insert whatever delta method that could be suitable here... 
		MOD3 = ((audf + cycle) % 3 == 0);
		MOD5 = ((audf + cycle) % 5 == 0);
		MOD7 = ((audf + cycle) % 7 == 0);
		MOD15 = ((audf + cycle) % 15 == 0);
		MOD31 = ((audf + cycle) % 31 == 0);
		MOD73 = ((audf + cycle) % 73 == 0);

		switch (timbre)
		{
		case TIMBRE_BELL:
			if (MOD31) audf = delta_pokey_audf(pitch, audf, coarse_divisor, divisor, cycle, timbre);
			break;

		case TIMBRE_BUZZY_4:
			if (MOD3 || MOD5 || MOD31) audf = delta_pokey_audf(pitch, audf, coarse_divisor, divisor, cycle, timbre);
			break;

		case TIMBRE_SMOOTH_4:
			if (!(MOD3 || CLOCK_15) || MOD5) audf = delta_pokey_audf(pitch, audf, coarse_divisor, divisor, cycle, timbre);
//			
//			if (!JOIN_16BIT && audf > 0xFF)
//			{	//use the buzzy timbre on the lower range instead
//				audf = get_audf(pitch, coarse_divisor, 232.5, cycle);
//				MOD3 = ((audf + cycle) % 3 == 0);
//				MOD5 = ((audf + cycle) % 5 == 0);
//				if (MOD3 || MOD5) audf = delta_pokey_audf(pitch, audf, coarse_divisor, 232.5, cycle, TIMBRE_BUZZY_4);
//			}
//			
			break;

		case TIMBRE_GRITTY_C:
			if (MOD3 || MOD5) audf = delta_pokey_audf(pitch, audf, coarse_divisor, divisor, cycle, timbre);
			break;

		case TIMBRE_BUZZY_C:
			if (!(MOD3 || CLOCK_15) || MOD5) audf = delta_pokey_audf(pitch, audf, coarse_divisor, divisor, cycle, timbre);
//			
//			if (!JOIN_16BIT && audf > 0xFF)
//			{	//use the gritty timbre on the lower range instead
//				audf = get_audf(pitch, coarse_divisor, 7.5, cycle);
//				MOD3 = ((audf + cycle) % 3 == 0);
//				MOD5 = ((audf + cycle) % 5 == 0);
//				if (MOD3 || MOD5) audf = delta_pokey_audf(pitch, audf, coarse_divisor, 7.5, cycle, TIMBRE_GRITTY_C); 
//			}
//			
			break;
			
		case TIMBRE_UNSTABLE_C:
			if (!MOD5 || MOD3) audf = delta_pokey_audf(pitch, audf, coarse_divisor, divisor, cycle, timbre);
			break;
			
		}

		if (audf < 0) audf = 0;
		if (!JOIN_16BIT && audf > 0xFF) audf = 0xFF;
		if (JOIN_16BIT && audf > 0xFFFF) audf = 0xFFFF;

		return audf;	//hacked to return the current freq directly, FIXME 
	
//		
//		//write the POKEY frequency to the table
//		if (JOIN_16BIT)
//		{	//in 16-bit tables, 2 bytes have to be written contiguously
//			table[i * 2] = audf & 0x0FF;	//LSB
//			table[i * 2 + 1] = audf >> 8;	//MSB
//			continue;
//		}
//		table[i] = audf; 
//
		
	//}
	


}




double CTuning::get_pokey_pitch(int audf, int coarse_divisor, double divisor, int cycle)
{
	return ((((g_ntsc) ? FREQ_17_NTSC : FREQ_17_PAL) / (coarse_divisor * divisor)) / (audf + cycle)) / 2;
}



int CTuning::get_pokey_audf(double pitch, int coarse_divisor, double divisor, int cycle)
{
	return (int)round(((((g_ntsc) ? FREQ_17_NTSC : FREQ_17_PAL) / (coarse_divisor * divisor)) / (2 * pitch)) - cycle);
}



int CTuning::delta_pokey_audf(double pitch, int audf, int coarse_divisor, double divisor, int cycle, int timbre)
{
	//TODO: Optimise this procedure a lot more, this is poorly written, but it gets the job done for now 
	int distortion = timbre & 0xF0;

	int tmp_audf_up = audf;		//begin from the currently invalid audf
	int tmp_audf_down = audf;
	double tmp_freq_up = 0;
	double tmp_freq_down = 0;
	double PITCH = 0;

	if (distortion != 0x40 && distortion != 0xC0) { tmp_audf_up++; tmp_audf_down--; }	//anything not distortion 4 or C, simpliest delta method

	else if (distortion == 0x40)
	{
		if (timbre == TIMBRE_SMOOTH_4)	//verify MOD3 integrity
		{
			for (int o = 0; o < 6; o++)
			{
				if ((tmp_audf_up + cycle) % 3 != 0 || (tmp_audf_up + cycle) % 5 == 0 || (tmp_audf_up + cycle) % 31 == 0) tmp_audf_up++;
				if ((tmp_audf_down + cycle) % 3 != 0 || (tmp_audf_down + cycle) % 5 == 0 || (tmp_audf_down + cycle) % 31 == 0) tmp_audf_down--;
			}
		}
		else if (timbre == TIMBRE_BUZZY_4)
		{
			for (int o = 0; o < 6; o++) 
			{
				if ((tmp_audf_up + cycle) % 3 == 0 || (tmp_audf_up + cycle) % 5 == 0 || (tmp_audf_up + cycle) % 31 == 0) tmp_audf_up++;
				if ((tmp_audf_down + cycle) % 3 == 0 || (tmp_audf_down + cycle) % 5 == 0 || (tmp_audf_down + cycle) % 31 == 0) tmp_audf_down--;
			}
		}
		else return 0;	//invalid parameter most likely 
	}

	else if (distortion == 0xC0)
	{
		//if (CLOCK_15)
		if (coarse_divisor == 114)	//15kHz mode
		{
			for (int o = 0; o < 3; o++)	//MOD5 must be avoided!
			{
				if ((tmp_audf_up + cycle) % 5 == 0) tmp_audf_up++;
				if ((tmp_audf_down + cycle) % 5 == 0) tmp_audf_down--;
			}
		}
		else if (timbre == TIMBRE_BUZZY_C)	//verify MOD3 integrity
		{
			for (int o = 0; o < 6; o++)
			{
				if ((tmp_audf_up + cycle) % 3 != 0 || (tmp_audf_up + cycle) % 5 == 0) tmp_audf_up++;
				if ((tmp_audf_down + cycle) % 3 != 0 || (tmp_audf_down + cycle) % 5 == 0) tmp_audf_down--;
			}
		}
		else if (timbre == TIMBRE_GRITTY_C)	//verify neither MOD3 or MOD5 is used
		{
			for (int o = 0; o < 6; o++)	//get the closest compromise up and down first
			{
				if ((tmp_audf_up + cycle) % 3 == 0 || (tmp_audf_up + cycle) % 5 == 0) tmp_audf_up++;
				if ((tmp_audf_down + cycle) % 3 == 0 || (tmp_audf_down + cycle) % 5 == 0) tmp_audf_down--;
			}
		}
		else if (timbre == TIMBRE_UNSTABLE_C)	//verify MOD5 integrity
		{
			for (int o = 0; o < 6; o++)	//get the closest compromise up and down first
			{
				if ((tmp_audf_up + cycle) % 3 == 0 || (tmp_audf_up + cycle) % 5 != 0) tmp_audf_up++;
				if ((tmp_audf_down + cycle) % 3 == 0 || (tmp_audf_down + cycle) % 5 != 0) tmp_audf_down--;
			}
		}
		else return 0;	//invalid parameter most likely
	}

	PITCH = get_pokey_pitch(tmp_audf_up, coarse_divisor, divisor, cycle);
	tmp_freq_up = pitch - PITCH;	//first delta, up
	PITCH = get_pokey_pitch(tmp_audf_down, coarse_divisor, divisor, cycle);
	tmp_freq_down = PITCH - pitch;	//second delta, down
	PITCH = tmp_freq_down - tmp_freq_up;

	if (PITCH > 0) audf = tmp_audf_up; //positive, meaning delta up is closer than delta down
	else audf = tmp_audf_down; //negative, meaning delta down is closer than delta up
	return audf;
}




double CTuning::GetTruePitch(double tuning, int temperament, int basenote, int semitone)
{
	int notesnum = 12;	//unless specified otherwise
	int note = (semitone + basenote) % notesnum;	//current note
	double ratio = 0;	//will be used for calculations
	double octave = 2;	//an octave is usually the frequency of a note multiplied by 2
	double multi = 1; //octave multiplyer for the ratio

	//Equal temperament is generated using the 12th root of 2
//	if (temperament == NO_TEMPERAMENT)
//	{
		ratio = pow(2.0, 1.0 / 12.0);
		return (tuning / 64) * pow(ratio, semitone + basenote);
//	}
/*
	if (temperament >= TUNING_CUSTOM)	//custom temperament will be used using ratio
	{
		octave = CUSTOM[notesnum];
		ratio = CUSTOM[note];
	}
	else	//any temperament preset will be used
	{
		for (int i = 0; i < PRESETS_LENGTH; i++)
		{
			if (temperament_preset[temperament][i]) continue;
			notesnum = i - 1;
			break;
		}
		octave = temperament_preset[temperament][notesnum];
		note = (semitone + basenote) % notesnum;
		ratio = temperament_preset[temperament][note];
	}
	multi = pow(octave, trunc((semitone + basenote) / notesnum));
	return (tuning / 64) * (multi * ratio);
*/
}


//some return formulae for calculations
//note that if the pitch parameter is ommited, 440Hz will be assumed by default

double CTuning::GetCentsOff(int semitone, double pitch, double tuning) 
{ 
	if (!pitch) return 0;
	
	double centnum = 1200 * log2(pitch / tuning);
	int notenum = (int)round(centnum * 0.01) + 60; 
	return (centnum - (notenum - 60) * 100); 
	
/*
	//this is somehow less accurate than just returning cents off the "closest" note...
	double theoretical_pitch = GetTruePitch(tuning, g_temperament, g_basenote, semitone); 
	double theoretical_centnum = 1200 * log2(theoretical_pitch / tuning);
	return centnum - theoretical_centnum;
*/	

}

char* CTuning::GetSemitone(double pitch, double tuning) 
{ 
	if (!pitch) return 0;
	
	double centnum = 1200 * log2(pitch / tuning);
	int notenum = (int)round(centnum * 0.01) + 60;
	int note = ((notenum + 96) - g_basenote) % 12;

	if (note < 0)
	note *= -1;	//invert the negative to prevent going out of bounds

	return (char*)g_notes[note];
}

int CTuning::GetOctave(double pitch, double tuning) 
{ 
	if (!pitch) return 0;
	
	double centnum = 1200 * log2(pitch / tuning);
	int notenum = (int)round(centnum * 0.01) + 60;
	int note = ((notenum + 96) - g_basenote) % 12;
	
	return (((notenum + 96) - g_basenote) / 12) - 8; 
}

int CTuning::GetNoteNumber(double pitch, double tuning) 
{	
	if (!pitch) return 0;
	
	double centnum = 1200 * log2(pitch / tuning);

	return (int)round(centnum * 0.01) + 72 - g_basenote;
}


//new functions, will make more sense later...
//TODO: make these actually do something lol 

int CTuning::get_freq_min(int soundchip) 
{ 
	switch(soundchip)
	{ 
	case SOUNDCHIP_2A03: return 0x7FF;
	
	case SOUNDCHIP_SUNS5B: return 0xFFF;
	
	case SOUNDCHIP_POKEY: return (g_16bit_freq) ? 0xFFFF : 0xFF;	//also 0xFFFF in 16-bit mode, a lot of this will vary between a lot of modes and shit but let's not bother for now... 

	case SOUNDCHIP_SID: return 0x0001;	//lowest possible non-zero pitch generated with this freq 
	
	case SOUNDCHIP_LR35902: return 0x000;
	} 
	return 0; 
}

int CTuning::get_freq_max(int soundchip) 
{
	switch(soundchip)
	{ 
	case SOUNDCHIP_2A03: return 0x000;
	
	case SOUNDCHIP_SUNS5B: return 0x001;
	
	case SOUNDCHIP_POKEY: return 0x00;	//same as above... totally jank soundchip...  
	
	case SOUNDCHIP_SID: return 0xFFFF;
	
	case SOUNDCHIP_LR35902: return 0x7FF;
	} 
	return 0;
}

//
//

double CTuning::get_pitch(int soundchip, int freq)
{
	//variables for pitch calculation, divisors must never be 0! 
	double divisor = 1;	
	int coarse_divisor = 1;
	int cycle = 1;

	switch(soundchip)
	{ 
	case SOUNDCHIP_2A03:  
		return (((g_ntsc) ? FREQ_17_NTSC : FREQ_17_PAL) / (16.0 * (freq + 1)));
		
	case SOUNDCHIP_SUNS5B:
		//if the freq value is 0, return 0Hz, to avoid a division by 0 crash 
		return (freq) ? (((g_ntsc) ? FREQ_17_NTSC : FREQ_17_PAL) / (32.0 * freq)) : 0; 
	
	case SOUNDCHIP_POKEY: 
		coarse_divisor = 28;	//standard 64kHz mode, hardcoded for now... 
		return ((((g_ntsc) ? FREQ_17_NTSC : FREQ_17_PAL) / (coarse_divisor * divisor)) / (freq + cycle)) / 2;

	case SOUNDCHIP_SID: 
		//if the freq value is 0, return 0Hz, to avoid a division by 0 crash 
		return (freq) ? ((freq / 16777216.0) * ((g_ntsc) ? FREQ_10_NTSC : FREQ_10_PAL)) : 0; 
		
	case SOUNDCHIP_LR35902:
		return (FREQ_GAMEBOY / (2048.0 - freq)); 
	} 
	return 0; 
}
	 
	
int CTuning::get_freq(int soundchip, double pitch) 
{
	//variables for pitch calculation, divisors must never be 0! 
	double divisor = 1;	
	int coarse_divisor = 1;
	int cycle = 1;
	int freq = 0;

	switch(soundchip)
	{ 
	case SOUNDCHIP_2A03: 
		return (int)round((((g_ntsc) ? FREQ_17_NTSC : FREQ_17_PAL) / (16.0 * pitch)) - 1); 
		
	case SOUNDCHIP_SUNS5B:
		return (int)round((((g_ntsc) ? FREQ_17_NTSC : FREQ_17_PAL) / (32.0 * pitch))); 
	
	case SOUNDCHIP_POKEY: 
		coarse_divisor = 28;	//standard 64kHz mode, hardcoded for now... 
		return (int)round(((((g_ntsc) ? FREQ_17_NTSC : FREQ_17_PAL) / (coarse_divisor * divisor)) / (2 * pitch)) - cycle);
		
	case SOUNDCHIP_SID:
		return (int)round((16777216.0 / ((g_ntsc) ? FREQ_10_NTSC : FREQ_10_PAL)) * pitch); 		

	case SOUNDCHIP_LR35902:
		return (int)round(2048.0 - (FREQ_GAMEBOY / pitch));
	} 
	
	return 0;
}




