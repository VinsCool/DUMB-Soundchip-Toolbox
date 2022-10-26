// DUMB POKEY Toolbox
// by VinsCool 
// For experimenting with random ideas involving with the Atari POKEY soundchip
//
// DUMB stands for "Dumb Unless Made Better" because that's how I live somehow...
//-----------------------------------------------------------------------------------------------------------// 

#include <iostream>
#include <iomanip>
#include <fstream>
#include <time.h>
#include <cmath>
#include <limits> 
#include <cstring> 

#include "kbhit.h"

#include "general.h"		//global variables, #defines and other stuff shared between functions 
#include "tuning.h"		//soundchip pitch/frequency calculations formulae and general tuning functions 
//#include "pcm.h"		//incomplete PCM encoding/decoding(?) functions 

using namespace std; 

CTuning chiptuning; 		//tuning generation object 

keyboard inputkey;

fstream logwrite; 
char* logfile = NULL;		//optional logging file pointer (unfinished) 

int g_task = 0;			//what should the program do? 

bool g_ntsc = 0;		//1 for NTSC region, 0 for PAL region (default) 
int g_soundchip = 0;		//which soundchip is being used? (TODO) 
int g_freq_min = 0;		//lowest possible chip frequency 
int g_freq_max = 0;		//highest possible chip frequency


double g_basetuning = 440.0;	//A-4 tuning, used every bruteforce iteration (default) FIXME: this could be set on the fly instead

double g_begintuning = 420.0;	//initial tuning bruteforce pitch (default) 
double g_endtuning = 460.0;	//last tuning bruteforce pitch (default) 

double g_precision = 0.01;	//tiny amounts have greater precision at the cost of being slower to process (default) 
int g_temperament = 0;		//no temperament (might not be needed for now) 
int g_basenote = 3;		//the first note in the scale when tables are generated, C- by default 


int g_iterations = 0;		//number of bruteforce iteration in progress FIXME: fixme: this could be set on the fly instead 

int g_best_iteration = 0;	//last best iteration (might not be needed anymore) 
double g_best_score = 0;	//last best score, lower the better 
double g_best_tuning = 0;	//last best A-4 tuning pitch 


//these variables will need to be reworked later...
bool g_truncate_freq = 0;

bool g_16bit_freq = 0;		//POKEY 16-bit mode 
bool g_179mhz_freq = 0;		//POKEY 1.79mHz mode 
bool g_15khz_freq = 1;		//POKEY 15kHz mode


int g_audctl = 0;		//POKEY AUDCTL used in calculations (default) 
int g_timbre = TIMBRE_BELL;	//POKEY Distortion/Timbre used in calculations (default) 


//-----------------------------------------------------------------------------------------------------------//

//wait during arbitrary amounts of time while code is running, seconds are the default unit used 
void wait(int sec, bool ms = 0)
{ 
	int ticks = (ms) ? CLOCKS_PER_SEC / 1000 : CLOCKS_PER_SEC; 	
	clock_t count = clock() + sec * ticks;
	while (clock() < count) { /*Waiting...*/ } 
}

//quickly clear contole output as its own function for less confusion
void clear_console()
{
	cout << "\x1B[2J\x1B[H";
} 

//exit the program, an optional code may be provided for handling errors 
void quit(int code = 0) 
{ 
	//close the log file if it is still open 
	if (logfile) logwrite.close(); 
	
	if (code == 1) 
	{ 	//an error code was passed to the functions, TODO: add more stuff to this procedure later 
		cout << "\nThe program was terminated with a code of " << code << endl << endl << flush;
		exit(EXIT_FAILURE); 
	} 
	else if (code == 2) 
	{ 	//manually exited, which is not a failure 
		cout << "\nThe program was terminated early via quit(2), Finished without error.\n" << endl << flush; 
		wait(1);  
	}
	else 
	{ 	//finished successfully, exit the program like normal 
		cout << "A total of " << dec << setprecision(0) << fixed << g_iterations;
		cout << " iterations of the bruteforcer have been processed. Finished.\n" << endl << flush; 
		wait(1); 
	} 
	
	exit(EXIT_SUCCESS); 
} 

//parse arguments provided from running the program in console
bool get_argument(int argc, char** argv, int& argument) 
{
	argument++;	//the first argument is the program's own name/path and should be skipped 
	if (argument >= argc) return 0;	//finished 

	//argument and parameter pointers, the parameter is optional if the argument doesn't need it 
	char* option = argv[argument]; 
	char* parameter = NULL;	

	//an error has occured, abort immediately! 
	if (!option) 
	{
		cout << "Error: NULL argument" << endl; 
		quit(1); 
	}

	//the character '-' is checked explicitely, invalid arguments will be ignored
	bool error = (option[0] != '-') ? 1 : 0; 
	
	//everything that was not processed will be output in a warning message then ignored 
	if (error) 
	{
		cout << "Warning: invalid argument, '" << option << "', missing prefix '-', '--' or '?' in front of it" << endl;
		wait(500, 1);
		return 1;
	}
		
	//display the help screen, this will be ignored if the command is not the first 
	if(!strcmp(option + 1, "help") && argument <= 2)	
	{ 
		cout << "Usage: [-argument parameter], this page is in progress, so infos are missing..." << endl; 
		cout << "Multiple arguments supported, the argument must be in front of the parameter\n" << endl;
		cout << "[-log filename] log the process to a file" << endl;
		cout << "[-pal] set the machine region to PAL (default)" << endl;
		cout << "[-ntsc] set the machine region to NTSC" << endl;
		quit(2); 
	} 
	
	//option choosen is bruteforce, this will be ignored if the command is not the first 
	if(!strcmp(option + 1, "bruteforce") && argument <= 2) { g_task = BRUTEFORCE; g_basetuning = 0; return 1; } 
	
	//option choosen is generate, this will be ignored if the command is not the first 
	if(!strcmp(option + 1, "generate") && argument <= 2) { g_task = GENERATE; return 1; } 
	
	//option choosen is get all freqs, this will be ignored if the command is not the first 
	if(!strcmp(option + 1, "getall") && argument <= 2) { g_task = GETALL; return 1; } 
		
	//write to file, which is optional
	if(!strcmp(option + 1, "log")) 
	{
		argument++;	//skip to the next argument early
		parameter = argv[argument]; 
		
		if (!parameter) error = 1;	//null parameter 
		else if(!strcmp(parameter, "")) error = 1;	//invalid filename
		else logfile = parameter;	//if no error was found, use the argument as the log filename to save 
		
		if (error) cout << "Warning: invalid file name parameter, log file will be ignored" << endl; 
		return 1;
	} 
	
	//soundchip 
	if(!strcmp(option + 1, "2a03")) { g_soundchip = SOUNDCHIP_2A03; return 1; } 
	if(!strcmp(option + 1, "5b")) { g_soundchip = SOUNDCHIP_SUNS5B; return 1; }  
	if(!strcmp(option + 1, "pokey")) { g_soundchip = SOUNDCHIP_POKEY; return 1; } 
	if(!strcmp(option + 1, "sid")) { g_soundchip = SOUNDCHIP_SID; return 1; } 
	if(!strcmp(option + 1, "gameboy")) { g_soundchip = SOUNDCHIP_LR35902; return 1; } 

	//machine region 
	if(!strcmp(option + 1, "pal")) { g_ntsc = 0; return 1; } 
	if(!strcmp(option + 1, "ntsc")) { g_ntsc = 1; return 1; } 
	
	//precision used for tuning bruteforcing, the argument will be ignored if the tool isn't in BRUTEFORCE mode 
	if(!strcmp(option + 1, "precision") && g_task == BRUTEFORCE) 
	{ 
		if (parameter = argv[argument + 1]) 
		{
			g_precision = atof(parameter); 
			if (!g_precision) error = 1;
		} 
		
		if (!parameter || error) 
		{
			cout << "Warning: invalid precision parameter, the default value will be used" << endl;
			g_precision = 0.01;	//default value provided if the argument did not specify it 
		} 
		
		argument++;	//skip to the next argument early
		return 1;
	}

	if(!strcmp(option + 1, "begintuning") && g_task == BRUTEFORCE) 
	{ 
		if (parameter = argv[argument + 1]) 
		{
			g_begintuning = atof(parameter); 
			if (!g_begintuning) error = 1;
		} 
		
		if (!parameter || error) 
		{
			cout << "Warning: invalid precision parameter, the default value will be used" << endl;
			g_begintuning = 420;	//default value provided if the argument did not specify it 
		} 
		
		argument++;	//skip to the next argument early
		return 1;
	}

	if(!strcmp(option + 1, "endtuning") && g_task == BRUTEFORCE) 
	{ 
		if (parameter = argv[argument + 1]) 
		{
			g_endtuning = atof(parameter); 
			if (!g_endtuning) error = 1;
		} 
		
		if (!parameter || error) 
		{
			cout << "Warning: invalid precision parameter, the default value will be used" << endl;
			g_endtuning = 460;	//default value provided if the argument did not specify it 
		} 
		
		argument++;	//skip to the next argument early
		return 1;
	}

	//basetuning used for notes table calculation, the argument will be ignored if the tool isn't in GENERATE mode 
	if(!strcmp(option + 1, "basetuning") && g_task != BRUTEFORCE) 
	{ 
		if (parameter = argv[argument + 1]) 
		{
			g_basetuning = atof(parameter); 
			if (!g_basetuning) error = 1;
		} 
		
		if (!parameter || error) 
		{
			cout << "Warning: invalid tuning parameter, the default value will be used" << endl;
			g_basetuning = 440;	//default value provided if the argument did not specify it 
		} 

		argument++;	//skip to the next argument early
		return 1;
	}

	//basenote used for notes table calculation  
	if(!strcmp(option + 1, "basenote")) 
	{ 
		if (parameter = argv[argument + 1]) 
		{
			g_basenote = strtoul(parameter, NULL, 0); 
			if ((strcmp(parameter, "0")) && !g_basenote) error = 1;	//returned value is 0, but isn't really it 
		} 
		
		if (!parameter || error || g_basenote > 12) 
		{
			cout << "Warning: invalid base note parameter, the default value will be used" << endl;
			g_basenote = 3;	//A-
		} 
		
		argument++;	//skip to the next argument early
		return 1;
	} 

	//if the argument was not processed, it will be ignored and output with an error message 
	cout << "Warning: the argument '" << option << "' was not recognised" << endl;
	wait(500, 1);	

	return 1; 
}

//TODO: parse different chips, and get rid of all the hardcoded POKEY shit 
bool bruteforcer(int soundchip) 
{ 
	//initialisation, first set a few things before attempting to run the bruteforcer tool 
	if (!g_basetuning) g_basetuning = g_begintuning; 
	
	double average_cents = 0;  

	int freq_min = chiptuning.get_freq_min(soundchip);
	int freq_max = chiptuning.get_freq_max(soundchip);
	
	bool freq_inverted = (freq_min < freq_max); 

	double lowest_pitch = chiptuning.generate_pitch(soundchip, freq_min);
	double highest_pitch = chiptuning.generate_pitch(soundchip, freq_max);

	int lowest_note = chiptuning.GetNoteNumber(lowest_pitch, g_basetuning);	
	int highest_note = chiptuning.GetNoteNumber(highest_pitch, g_basetuning); 
	
	//if (lowest_note < 0) lowest_note = 0;	//C--1
	//if (lowest_note < 12) lowest_note = 12;	//C-0
	//if (highest_note > 119) highest_note = 119;	//B-8
	//if (highest_note > 107) highest_note = 107;	//B-7
	
	int notes_count = highest_note - lowest_note + 1; 
	
	int number = (freq_min > freq_max) ? freq_min : freq_max;
	int padding = -1; while (number != 0) { number /= 10; padding++; } 
	//cout << padding << endl;
	//quit(2);

	//process the bruteforce shit here afterwards...
	for (int i = 0; i < notes_count; i++)
	{ 
		int note = lowest_note + i;
		int freq = chiptuning.generate_freq(soundchip, note); 
		
		if (freq_inverted)
		{ 
			if (freq > freq_max) { freq = freq_max; } 
			if (freq < freq_min) { freq = freq_min; } 
		} 
		else 
		{
			if (freq < freq_max) { freq = freq_max; } 
			if (freq > freq_min) { freq = freq_min; } 
		}

		double bruteforced_pitch = chiptuning.generate_pitch(soundchip, freq); 
		double cents = chiptuning.GetCentsOff(note, bruteforced_pitch, g_basetuning); 

		if (cents < 0) cents *= -1;	//invert the number if negative 
		average_cents += cents;	//add up the number of cents off, lower the better 
	}

	if (!g_best_score || average_cents < g_best_score)	//if new score is lower, update it and keep it as current best 
	{ 
		g_best_score = average_cents;	//lower the better 
		g_best_tuning = g_basetuning;
		g_best_iteration = g_iterations;	//offset by 1 since 0 is the first iteration
	}

	g_basetuning += g_precision; 
	g_iterations++;

	char animation = 0;
	int character = g_iterations % 4;

	switch (character) 
	{ 
	case 0: animation = '-'; break; 
	case 1: animation = '\\'; break; 
	case 2: animation = '|'; break; 
	case 3: animation = '/'; break; 
	} 
	
	//the number of iterations is known before starting FIXME: maybe not needed anymore
	int bruteforce_score = (g_endtuning - g_begintuning) / g_precision; 
	
	//statistic displayed in the console as the bruteforce functions are being executed 
	int progress_bar = ((g_iterations * 50) / bruteforce_score);
	double percentage = ((g_iterations * 100.0) / bruteforce_score);
		
	cout << "\r\033[A\033[A\033[A\033[A[ " << setfill('=') << setw(progress_bar) << "";
	cout << setfill(' ') << setw(50 - progress_bar) << "" << " ] [ " << animation << " ";
	cout << dec << setprecision(2) << fixed << percentage << "% completed ]\n\n\33[2K"; //clear next line as well
	
	cout << "A-4 = " << dec << setprecision(6) << fixed << g_basetuning << "Hz, "; 
	cout << "current score: " << average_cents << "\n\33[2K"; //clear next line as well
	
	cout << "Best iteration: "  << dec << setprecision(0) << fixed << g_best_iteration;
	cout << ", with a score of " << dec << setprecision(6) << fixed << g_best_score;
	cout << ", A-4 = " << g_best_tuning << "Hz\n" << flush; 
	
	return (g_basetuning < g_endtuning);	//true is returned until the basetuning reached the maximal pitch 
}

//-----------------------------------------------------------------------------------------------------------//

//this is where things run from at the very start when the program is executed 
int main(int argc, char** argv)  
{ 
	int argument = 0; 
	clear_console();	//clear the console first thing first 
	wait(750, 1); 
	
	if (argc < 2) 
	{ 
		cout << "Missing arguments, the program couldn't continue\n\n";
		cout << "Run '" << argv[0] << " -help' for more details\n" << endl; 
		quit(); 
	} 
	
	//process all the arguments and parameters that were typed in the console 
	while (get_argument(argc, argv, argument)) { /*do stuff until 0 is returned*/ } 
	
	//initialise the log file if the argument -log was used 
	if (logfile) logwrite.open(logfile, ios::out);
	
	wait(1);

/*
	//FIXME: main screen displayed upon executing the tool, no longer POKEY exclusive! 
	int distortion = (g_timbre & 0xF0) >> 4; 

	cout << "\nDUMB POKEY Toolbox " << VERSION << "by VinsCool\n";
	cout << "For experimenting with random ideas involving with the Atari POKEY soundchip\n" << endl;
	wait(1);
	cout << "Current parameters: Distortion " << hex << uppercase << distortion << ", ";
	if (g_16bit_freq) cout << "16-bit mode ";
	cout << "(AUDCTL = $" << PADHEX(2, g_audctl) << "), ";
	if (g_ntsc) cout << "NTSC region"; else cout << "PAL region";
	cout << endl;
	wait(1);
	cout << "\nThis is a test run of the bruteforcer.\nIt will take SOME time to process, please wait..." << endl; 
	wait(1);
	cout << "\n\n\n\n" << endl; 
*/

	if (g_task == BRUTEFORCE) 
	{ 	
		bool success = 0; 
		do
		{ 
			success = 0;
			if(inputkey.pressed(27)) quit(1); 	//ESCAPE pressed  
			else if(inputkey.pressed(10)) break; 	//ENTER pressed 
			success = 1; 
		} 
		while (bruteforcer(g_soundchip)); 
	
		//bruteforcing finished here... output the results 
		if (success) cout << "\nDone!\n" << endl; 
		else 
		{ 
			cout << "\nThe bruteforcing process was interrupted!" << endl; 
			cout << "Tuning and lookup tables calculation will use the last values tested" << endl; 
		} 
		g_basetuning = g_best_tuning;	//optimal tuning 
		wait(3); 
	} 
	else if (g_task == GENERATE) 
	{ 
		g_best_tuning = g_basetuning;	//FIXME: this should not be done that way 
	} 
	else if (g_task == GETALL)
	{
		g_best_tuning = g_basetuning;	//FIXME: this should not be done that way 
	}
	else quit(1);

////////////////////////////////// FIXME: everything below is a jumbled mess /////////////////////////////////////////////// 
	
	int freq_min = chiptuning.get_freq_min(g_soundchip);
	int freq_max = chiptuning.get_freq_max(g_soundchip);
	
	bool freq_inverted = (freq_min < freq_max); 

	double lowest_pitch = chiptuning.generate_pitch(g_soundchip, freq_min);
	double highest_pitch = chiptuning.generate_pitch(g_soundchip, freq_max);

	int lowest_note = 12;	//chiptuning.GetNoteNumber(lowest_pitch, g_basetuning);
	int highest_note = 119;	//chiptuning.GetNoteNumber(highest_pitch, g_basetuning);

	if (lowest_note < 12) lowest_note = 12;	//C-0
	if (highest_note > 119) highest_note = 119;	//B-8
	
	int notes_count = 96;	//highest_note - lowest_note + 1; 
	int notes_order = 0;	//just to make sure the output tables are properly aligned in case notes are skipped
	int ordering = 12;	//12 notes per octave, so this looks ordered nicely	//(g_16bit_freq) ? 8 : 16; 
	int number = (freq_min > freq_max) ? freq_min : freq_max;
	int padding = -1; while (number != 0) { number /= 10; padding++; } 
	if (g_truncate_freq) padding = 2;
	
////////////////////////////////// TODO: move into its own function ////////////////////////////////////////////////////////

	//output the best bruteforced table 
	clear_console();
	cout << "\n";
	cout << dec << setprecision(2) << fixed << "Lowest pitch: " << lowest_pitch << "Hz, " << endl; 
	wait(750, 1);
	cout << dec << setprecision(2) << fixed << "Highest pitch: " << highest_pitch << "Hz, " << endl; 
	wait(750, 1);
	cout << "\nThis is the 'optimal' bruteforced table, using the A-4 tuning of ";
	cout << dec << setprecision(6) << fixed << g_best_tuning << "Hz\n" << endl;
	wait(250, 1); 
	
	int iteration = (g_truncate_freq) ? 2 : 1; 

generate_again:	
	for (int i = 0; i < notes_count; i++) 
	{ 	
		if (i == 0 && g_truncate_freq) 
		{
			if (iteration == 2) cout << "16-bit LSB table:\n" << endl; 
			else cout << "\n16-bit MSB table:\n" << endl; 
		}
	
		int note = lowest_note + i;
		int freq = chiptuning.generate_freq(g_soundchip, note); 

		if (freq_inverted)
		{ 
			if (freq > freq_max) { freq = freq_max; } 
			if (freq < freq_min) { freq = freq_min; } 
		} 
		else 
		{
			if (freq < freq_max) { freq = freq_max; } 
			if (freq > freq_min) { freq = freq_min; } 
		}		

		double bruteforced_pitch = chiptuning.generate_pitch(g_soundchip, freq); 
		
		if (g_truncate_freq && iteration == 1) freq >>= 8;		//MSB
		else if (g_truncate_freq && iteration == 2) freq &= 0xFF;	//LSB 
		
		cout << "$" << PADHEX(padding, freq); 
		if ((notes_order + 1) % ordering == 0 || i == notes_count - 1) cout << endl; 
		else cout << ", "; 
		cout << flush;
		notes_order++;
	} 

	//that's a very ugly hack but who are you going to call to stop me, Batman?
	if (iteration == 2) 
	{ 
		iteration--;
		goto generate_again;
	} 

////////////////////////////////// TODO: move into its own function ///////////////////////////////////////////////////////////

	//output detailed infos from the bruteforced table
	wait(1);
	cout << "\nStatistics for each notes calculated from the bruteforced table:\n" << endl;
	wait(1);
	
	number = (freq_min > freq_max) ? freq_min : freq_max;
	padding = -1; while (number != 0) { number /= 10; padding++; }
	
	for (int i = 0; i < notes_count; i++)
	{ 
		int note = lowest_note + i;
		int freq = chiptuning.generate_freq(g_soundchip, note); 
		
		if (freq_inverted)
		{ 
			if (freq > freq_max) { freq = freq_max; } 
			if (freq < freq_min) { freq = freq_min; } 
		} 
		else 
		{
			if (freq < freq_max) { freq = freq_max; } 
			if (freq > freq_min) { freq = freq_min; } 
		}
		
		double theoretical_pitch = chiptuning.GetTruePitch(g_basetuning, g_temperament, g_basenote, note); 
		double bruteforced_pitch = chiptuning.generate_pitch(g_soundchip, freq); 
		char* semitone = chiptuning.GetSemitone(bruteforced_pitch, g_basetuning);
		int octave = chiptuning.GetOctave(bruteforced_pitch, g_basetuning);
		double cents = chiptuning.GetCentsOff(note, bruteforced_pitch, g_basetuning);
		
		cout << semitone << dec << setprecision(0) << fixed << octave << ", "; 
		cout << "$" << PADHEX(padding, freq) << ", "; 
		cout << dec << setprecision(6) << fixed << bruteforced_pitch << "Hz, "; 
		cout << dec << setprecision(6) << fixed << cents << " cents off\n" << flush; 
		//wait(10, 1); 
		cout << "Theoretical: " << dec << setprecision(6) << fixed << theoretical_pitch << "Hz\n" << endl; 
		//wait(100, 1); 
	}

////////////////////////////////// TODO: move into its own function ///////////////////////////////////////////////////////////

	if (g_task == GETALL)
	{	
		int freqs_count = ((freq_min > freq_max) ? freq_min : freq_max) + 1; 
		
		for (int i = 0; i < freqs_count; i++) 
		{ 
			if (!i) 
			{ 
				wait(1);
				if (logfile) logwrite << "\nStatistics for every single pitches that could be calculated from freqs:\n" << endl;
				else cout << "\nStatistics for every single pitches that could be calculated from freqs:\n" << endl;
				wait(1);
			} 
			
			double bruteforced_pitch = chiptuning.generate_pitch(g_soundchip, i); 
			if (!bruteforced_pitch) continue;	//invalid pitch 
			int note = chiptuning.GetNoteNumber(bruteforced_pitch, g_basetuning);
			double theoretical_pitch = chiptuning.GetTruePitch(g_basetuning, g_temperament, g_basenote, note); 
			
			char* semitone = chiptuning.GetSemitone(bruteforced_pitch, g_basetuning);
			int octave = chiptuning.GetOctave(bruteforced_pitch, g_basetuning);
			double cents = chiptuning.GetCentsOff(note, bruteforced_pitch, g_basetuning);
		
			if (logfile) 
			{
				logwrite << semitone << dec << setprecision(0) << fixed << octave << ", "; 
				logwrite << "$" << PADHEX(padding, i) << ", "; 
				logwrite << dec << setprecision(16) << fixed << bruteforced_pitch << "Hz, "; 
				logwrite << dec << setprecision(16) << fixed << cents << " cents off\n" << flush; 
				logwrite << "Theoretical: " << dec << setprecision(16) << fixed << theoretical_pitch << "Hz\n" << endl; 
			}
			else 
			{
				cout << semitone << dec << setprecision(0) << fixed << octave << ", "; 
				cout << "$" << PADHEX(padding, i) << ", "; 
				cout << dec << setprecision(16) << fixed << bruteforced_pitch << "Hz, "; 
				cout << dec << setprecision(16) << fixed << cents << " cents off\n" << flush; 
				cout << "Theoretical: " << dec << setprecision(16) << fixed << theoretical_pitch << "Hz\n" << endl; 			
			}
			
		} 
	}
	
/////////////////////////////////////////////// FINIIIIIISHEEDDDDDDD //////////////////////////////////////////////////////////////////// 	

	//finished! 
	quit();
}


