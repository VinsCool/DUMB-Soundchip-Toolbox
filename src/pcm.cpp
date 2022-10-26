// PCM related stuff, part of DUMB POKEY Toolbox 
//-----------------------------------------------------------------------------------------------------------//

//TODO 

//unsigned char* inputwav; 

/*
int main()
{
	SEPARATOR;
	cout << "﻿DUMB 4-bit PCM Converter " VERSION " by VinsCool" << endl;
	SEPARATOR;

	// Backup streambuffers of cout
	streambuf* stream_buffer_cout = cout.rdbuf();

	fstream::pos_type size;
	fstream input;
	fstream output;
	
	cout << fixed; 
	
	input.open("sample.wav", ios::in | ios::binary | ios::ate);

	if (input.is_open())
	{
		cout << "Loading file 'sample.wav'" << endl;
		size = input.tellg();
		inputwav = new unsigned char [size];
		input.seekg (0, ios::beg);
		input.read ((char*)inputwav, size);
		input.close();
		cout << size << " bytes read\nConversion procedure set to 'NybbleMerged'\nPlease wait..." << endl << endl;
		wait(1); 
	}
	else 
	{
		cout << "Unable to open file 'sample.wav', process aborted!" << endl << flush; 
		SEPARATOR;
		wait(1); 
		return 1;
	}

	// Get the streambuffer of the file output
	streambuf* stream_buffer_out = output.rdbuf();

	output.open("output.pcm", ios::out | ios::binary | ios::ate);

	// Redirect cout to file
	cout.rdbuf(stream_buffer_out);
	
	unsigned char nybblehi = 0, nybblelo = 0, nybblemerged = 0; 
	
	for (int i = 0; i < size; i++)
	{
		if (i < 44 || i + 1 == size) continue; 		//RIFF WAV header data, and last byte are unwanted in the output 
		
		nybblehi = inputwav[i] >> 4;
		nybblelo = inputwav[i] & 0x0F;
		
		nybblehi &= 0x0F;				//keep the highest bits clear
		if (nybblelo >= 8 && nybblehi < 15) nybblehi++;	//finer volume adjustment, but avoid overflow
		
		if (i % 2 == 0) 
		{
			nybblemerged = nybblehi << 4; 
		}
		else 
		{ 
			nybblemerged += nybblehi; 
			cout << nybblemerged;
		}
		
		//nybblehi = nybblehi & 0x0F;			//keep the highest bits clear
		//nybblehi = nybblehi | 0xF0;			//set volume only mode
		//if (i % 2 == 0) { membuffer[i] = nybblehi << 4; }
		//else { membuffer[i-1] += nybblehi; }

	}	

	output.close();	//done, save the file
	delete[] inputwav; //flush the memory 
	
	// Redirect cout back to screen
	cout.rdbuf(stream_buffer_cout);

	cout << "Finished, file 'output.pcm' has been written successfully!" << endl << flush;
	SEPARATOR;
	wait(1); 
	return 0;
}
*/

