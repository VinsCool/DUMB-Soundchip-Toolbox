//Poop 

int main(int argc, char** argv)  
{ 
	clear_console();
	int keynum = 0;
	int keyid = 0;
	char* key = NULL; 
	
	const char* keys[] = { "NONE", "SPACEBAR", "BACKSPACE", "ENTER", "ESCAPE" };
	
	bool running = true;
	
	while(running)
	{ 		
		if(inputkey.kbhit()) 
		{
			keynum = inputkey.getch(); //return int 
			
			switch(keynum)
			{
			case 10:	//enter
				keyid = 3;
				break;
				
			case 27:	//escape
				keyid = 4;
				running = false;
				break;
				
			case 32:	//spacebar
				keyid = 1;
				break;
				
			case 127:	//backspace
				keyid = 2;
				break;

			default:	//none of them 
				keyid = 0;
				break;
			} 			

		}

		key = (char*)keys[keyid]; 
		
		cout << "\r\033[A\033[A\033[A\033[A\n\33[2K"; 
		cout << "keynum = [ " << keynum << " ], ";
		cout << "key: = [ " << key << " ]\n\33[2K\n\33[2K" << flush; 

	} 
	
	cout << "Out of all keys, you had to chose [ " << key << " ], Yay!\n" << endl << flush; 
	quit();
}

