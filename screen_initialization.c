#include"cars_char.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <linux/fb.h>
#include <string.h>
#include <sys/mman.h>
#include <stdint.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <signal.h>
#include <termios.h>
#include <unistd.h>
#include "screen_initialization.h"
extern int tie;

static int Open(const char *pathname, int flags)
{
	int res = open(pathname, flags);
	if (res < 0)
	{
		perror("open");
		tie = 1;
		exit(EXIT_FAILURE);
	}
	return res;
}

static int Ioctl(int d, int request, struct fb_var_screeninfo* scr)
{
	int res = ioctl(d, request, scr);
	if (res == -1)
	{
		perror("ioctl");
		tie = 1;
		exit(EXIT_FAILURE);
	}
	return res;
}

static void* Mmap(void *start, size_t length, int prot , int flags, int fd, off_t offset)
{
	void* res = mmap(start, length, prot, flags, fd, offset);
	if ( MAP_FAILED == res)
	{
		perror("mmap");
		tie = 1;
		exit(EXIT_FAILURE);
	}
	return res;
	
}

static struct framebuffer_info FramebufferInit (unsigned int scr_lenth, unsigned int scr_hight)
{
	struct framebuffer_info fb_info;
	size_t fb_size, page_size;
	uint32_t color;
	page_size = sysconf(_SC_PAGESIZE);
	fb_info.fd = Open("/dev/fb0", O_RDWR);
	Ioctl(fb_info.fd, FBIOGET_VSCREENINFO, &(fb_info.scr));
	if ((fb_info.scr.xres - 1 <= scr_lenth)||(fb_info.scr.yres - 26 <= scr_hight))
	{
		printf("Заданы слишком большие размеры экрана\n");
		close(fb_info.fd);
		tie = 1;
		exit(EXIT_FAILURE);
	}
	fb_info.scr.xres = scr_lenth + 1;
	fb_info.scr.yres = scr_hight + 1;
	fb_size = sizeof(uint32_t) * fb_info.scr.xres_virtual * fb_info.scr.yres_virtual;	
	fb_info.map_size = (fb_size + (page_size - 1 )) & (~(page_size-1));
	fb_info.ptr = (uint32_t*)Mmap(NULL, fb_info.map_size, PROT_READ | PROT_WRITE, MAP_SHARED, fb_info.fd, 0);
	return fb_info;
}

struct framebuffer_info ScreenInitialization (unsigned int scr_len, unsigned int scr_hight)
{
	struct framebuffer_info fb_info = FramebufferInit (scr_len, scr_hight);
	for (int i = 0; i <= fb_info.scr.yres+1; i++)
		printf("\n");
	for (int i = 0; i < fb_info.scr.xres; i++)
		for (int j = 0; j < fb_info.scr.yres; j++)
			fb_info.ptr[j * fb_info.scr.xres_virtual + i] = default_color;
	return fb_info;
}

static void Tcgetattr(int fd, struct termios *termios_p)
{
	if (tcgetattr(fd, termios_p) < 0)
	{
		perror("tcsetattr error");
		tie = 1;
		exit(EXIT_FAILURE);
	}
}

void Tcsetattr(int fd, int optional_actions, struct termios *termios_p)
{
	if (tcsetattr(fd, optional_actions, termios_p) < 0)
	{
		perror("tcsetattr error");
		exit(EXIT_FAILURE);
	}
}

struct termios NonCanonNoEchoInput(int* terminal_set)
{
	struct termios old_terminal_info;
	Tcgetattr(0, &old_terminal_info);
	struct termios terminal_info = old_terminal_info;
	terminal_info.c_lflag &= ~(ECHO | ICANON);
	terminal_info.c_cc[VMIN] = 1;  
	terminal_info.c_cc[VTIME] = 0;
	Tcsetattr(0, TCSANOW, &terminal_info);
	(*terminal_set) = 1;
	return old_terminal_info;
}
