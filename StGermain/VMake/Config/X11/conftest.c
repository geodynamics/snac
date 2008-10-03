#include <X11/Xlib.h>
int main() {
	void* display;
	display = XOpenDisplay(NULL);
        return 0;
}
