#ifndef SCREEN_INIT
#define SCREEN_INIT
struct framebuffer_info
{
	uint32_t *ptr;
	struct fb_var_screeninfo scr;
	int fd;
	size_t map_size;
};

struct framebuffer_info ScreenInitialization (unsigned int scr_len, unsigned int scr_hight);
struct termios NonCanonNoEchoInput(int* terminal_set);
void Tcsetattr(int fd, int optional_actions, struct termios *termios_p);
#endif
