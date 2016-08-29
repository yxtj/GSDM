#include "Util.h"

#if defined(_WIN32) || defined(_WIN64)
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#define _OS_WIN
#include <windows.h>
#else
#include <sys/ioctl.h>
#include <unistd.h>
#endif

using namespace std;

std::pair<int, int> getScreenSize() {
	int cols = -1;
	int lines = -1;

#ifdef _OS_WIN
//	printf("win\n");
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
	cols = csbi.dwSize.X;
	lines = csbi.dwSize.Y;
#else
//	printf("posix\n");
	struct winsize ts;
	ioctl(STDIN_FILENO, TIOCGWINSZ, &ts);
	cols = ts.ws_col;
	lines = ts.ws_row;
#endif 
	return make_pair(cols, lines);
}

#undef _OS_WIN
