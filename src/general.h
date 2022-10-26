// Random stuff such as defines and configuration, part of DUMB POKEY Toolbox 
//-----------------------------------------------------------------------------------------------------------//

#pragma once 

#define VERSION				"v0.1 "
#define BRUTEFORCE			1 
#define GENERATE			2 
#define GETALL				3 

#define EOL "\x0d\x0a"	//Carriage Return (\r) and Line Feed (\n), for strings used during Exports

#define FREQ_17_NTSC			1789773		//The true clock frequency for the NTSC Atari 8-bit computer is 1.7897725 MHz
#define FREQ_17_PAL			1773447		//The true clock frequency for the PAL Atari 8-bit computer is 1.7734470 MHz
#define FREQ_17_HALFNTSC		0894886		//The true clock frequency for half NTSC clockspeed is 0.89488625 MHz (894.88 KHz)
#define FREQ_17_HALFPAL			0886724		//The true clock frequency for half PAL clockspeed is 0.8867235 MHz (886.72 KHz)
#define FREQ_17_MSX			3579545		//The true clock frequency for NTSC MSX computers are 3.579545 MHz

#define FREQ_10_PAL			985248 		//PAL Commodore 64 clock 
#define FREQ_10_NTSC			1022727		//NTSC Commodore 64 clock

#define FREQ_GAMEBOY			131072 		//Gameboy clock

//This macro was shamelessly stolen from this stackoverflow post: https://stackoverflow.com/a/42450151 
#define PADHEX(width, val) setfill('0') << setw(width) << hex << uppercase << (unsigned)val

// ----------------------------------------------------------------------------
// Soundchips definitions
// Anything that has a known formula could potentially be implemented in the calculations 
// Model variations may also require specific definitions, if there is anything major changed to them  
// If no soundchip parameter is provided, the output is assumed to be theoretically perfect calculations  
// 
#define SOUNDCHIP_NONE			0x00 	// SORRY NOTHING
// 
// Nintendo soundchips
#define SOUNDCHIP_2A03 			0x10 	// MMC5, VRC6
#define SOUNDCHIP_N163			0x11	// Uses Phase Accumulator
#define SOUNDCHIP_2C33			0x12	// FDS (cringe)
#define SOUNDCHIP_LR35902		0x13	// Hiroto Nakamura	
#define SOUNDCHIP_DSPSG			0x14	// Nintendo DS
// 
// Atari soundchips
#define SOUNDCHIP_MIKEY 		0x20 
#define SOUNDCHIP_POKEY 		0x21 
#define SOUNDCHIP_TIA 			0x22 
// 
// Commodore chips
#define SOUNDCHIP_SID 			0x30 
#define SOUNDCHIP_VIC 			0x31 
#define SOUNDCHIP_TED			0x32
// 
// SN chips
#define SOUNDCHIP_TMS9919		0x40	// different input clock from other SN7 models (500KHz Max)
#define SOUNDCHIP_TI_SN7		0x41	// Noise LFSR is 15-bit length
#define SOUNDCHIP_SEGA_SN7		0x42	// Noise LFSR is 16-bit length
// SN chips using the 500KHz clock:
// TMS9919, SN94624, SN76494
// SN chips using the 4MHz "normal" clock:
// SN76489, SEGA PSG, SN76496, NCR 8496, Tandy PSSJ
// 
// AY chips
#define SOUNDCHIP_AY8910		0x50
#define SOUNDCHIP_SUNS5B		0x51	// half 8910 clock (applies to YM2149F with clock divider pin held high)
#define SOUNDCHIP_AY8930		0x50	// AY8930 generates tone and envelope periods differently due to increased pitch depth.
// 
// YM chips 
#define SOUNDCHIP_YM2608 		0x60	// FM calculations may be universal to all OPN chips, SSG may vary 
#define SOUNDCHIP_YM2612 		0x61	// ...What SSG?
//
// Konami chips
#define SOUNDCHIP_VRC7			0x70	// Might be helpful to just reuse this for the YM2413 as well?
#define SOUNDCHIP_SCC			0x71	// SCC
#define SOUNDCHIP_SCC2			0x72	// SCC+

// ----------------------------------------------------------------------------
// POKEY AUDCTL definitions
// The values are the exact same ones used on the real thing
// This is mainly to save time trying to memorise each ones that may be wanted to be used 
// 
#define PAR_AUDCTL_15KHZ		0x01
#define PAR_AUDCTL_HPF_CH2		0x02
#define PAR_AUDCTL_HPF_CH1		0x04
#define PAR_AUDCTL_JOIN_3_4		0x08
#define PAR_AUDCTL_JOIN_1_2		0x10
#define PAR_AUDCTL_179_CH3		0x20
#define PAR_AUDCTL_179_CH1		0x40
#define PAR_AUDCTL_POLY9		0x80

#define PAR_AUDCTL_64KHZ		(!PAR_AUDCTL_15KHZ)	//not 15kHz implies 64kHz clock by default 
#define PAR_AUDCTL_HIGHPASS_FILTER	(PAR_AUDCTL_HPF_CH1 || PAR_AUDCTL_HPF_CH2)	//Either is fine if the channel isn't relevant 
#define PAR_AUDCTL_JOIN_16BIT		(PAR_AUDCTL_JOIN_1_2 || PAR_AUDCTL_JOIN_3_4)	//Either is fine if the channel isn't relevant 
#define PAR_AUDCTL_179MHZ		(PAR_AUDCTL_179_CH1 || PAR_AUDCTL_179_CH3)	//Either is fine if the channel isn't relevant 

// ----------------------------------------------------------------------------
// Timbre definitions, used for tuning calculations
// The values also define which is the appropriate Distortion (AUDC) to use
// Example: "audc = TIMBRE_BUZZY_C & 0xF0" 
// The value of audc is then 0xC0, which corresponds to Distortion C 
//
#define TIMBRE_PINK_NOISE		0x00	// Distortion 0, by default
#define TIMBRE_BROWNIAN_NOISE		0x01	// (MOD7 && POLY9), Distortion 0
#define TIMBRE_FUZZY_NOISE		0x02	// (!MOD7 && POLY9), Distortion 0
#define TIMBRE_BELL			0x20	// (!MOD31), Distortion 2, by default
#define TIMBRE_BUZZY_4			0x40	// (!MOD3 && !MOD5),  used by Distortion 4, this mode is actually identical to Distortion C (Gritty) 
#define TIMBRE_SMOOTH_4			0x41	// (MOD3 && !MOD5), used by Distortion 4, this mode is actually identical to Distortion C (Buzzy) 
#define TIMBRE_WHITE_NOISE		0x80	// Distortion 8, by default
#define TIMBRE_METALLIC_NOISE		0x81	// (MOD7 && POLY9), Distortion 8 
#define TIMBRE_BUZZY_NOISE		0x82	// (!MOD7 && POLY9), Distortion 8
#define TIMBRE_PURE			0xA0	// Distortion A, by default 
#define TIMBRE_GRITTY_C			0xC0	// (!MOD3 && !MOD5), also known as RMT Distortion E
#define TIMBRE_BUZZY_C			0xC1	// (MOD3 && !MOD5), also known as RMT Distortion C
#define TIMBRE_UNSTABLE_C		0xC2	// (!MOD3 && MOD5), must be avoided unless there is a purpose for it

