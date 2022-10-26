#ifndef KBHIT_H
#define KBHIT_H

#include <termios.h>

class keyboard
{
public:
	keyboard();
	~keyboard();
	int kbhit();
	int getch();
	bool pressed(int key);
private:
	struct termios initial_settings, new_settings;
	int peek_character;
};

#endif

