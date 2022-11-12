#include "cars_char.h"
#include <time.h>
#include <stdint.h>
#include <pthread.h>
#include <linux/fb.h>
#include <unistd.h>
#include "screen_initialization.h"
#include "threads.h"
#include "cars_movements.h"
int life_flag = 1;
int red_win = 1;
int blue_win = 1;
int key_flag = 1;
char old_red_key = 'd';
char old_blue_key = 'a';
pthread_t host_thread = 0;
pthread_t rival_thread = 0;
pthread_t sync_controls_thread = 0;
struct coords* red_car;
struct coords* blue_car;
clock_t time_of_last_draw_end = 0;
int need_answer = 0;

void BorderDraw (struct framebuffer_info fb_info)
{
	for (int i = 0; i <= fb_info.scr.xres; i++)
	{
		fb_info.ptr[i] = border_color;
		fb_info.ptr[fb_info.scr.yres * fb_info.scr.xres_virtual + i] = border_color;
	}
	for (int i = 0; i <= fb_info.scr.yres; i++)
	{
		fb_info.ptr[i * fb_info.scr.xres_virtual] = border_color;
		fb_info.ptr[i * fb_info.scr.xres_virtual + fb_info.scr.xres] = border_color;
	}
}

void CarStartPos (struct framebuffer_info fb_info)
{
	for (int i = 0; i < car_len; i++)
		for (int j = 0; j < car_width; j++)
		{
			red_car[i*car_width+j].x = i+1;
			red_car[i*car_width+j].y = j+1;
			blue_car[i*car_width+j].x = fb_info.scr.xres-1-i;
			blue_car[i*car_width+j].y = fb_info.scr.yres-1-j;
		}
}

void CarDraw(struct framebuffer_info fb_info, struct coords* car, uint32_t color)
{
	for (int i = 0; i < car_len; i++)
		for (int j = 0; j < car_width; j++)
			fb_info.ptr[car[i*car_width+j].y * fb_info.scr.xres_virtual + car[i*car_width+j].x] = color;
}

static void MoveToLeft(struct coords* car)
{
	car[car_width/2].x -= 1;
	for (int i = 0; i < car_len; i++)
		for (int j = 0; j < car_width; j++)
		{
			car[i * car_width + j].x = car[car_width/2].x - i;
			car[i * car_width + j].y = car[car_width/2].y + (car_width/2 - j);
		}
}

static void MoveToRight(struct coords* car)
{
	car[car_width/2].x += 1;
	for (int i = 0; i < car_len; i++)
		for (int j = 0; j < car_width; j++)
		{
			car[i * car_width + j].x = car[car_width/2].x + i;
			car[i * car_width + j].y = car[car_width/2].y - (car_width/2 - j);
		}
}

static void MoveUp (struct coords* car)
{
	car[car_width/2].y -= 1;
	for (int i = 0; i < car_len; i++)
		for (int j = 0; j < car_width; j++)
		{
			car[i * car_width + j].y = car[car_width/2].y - i;
			car[i * car_width + j].x = car[car_width/2].x - (car_width/2 - j);
		}
}

static void MoveDown (struct coords* car)
{
	car[car_width/2].y += 1;
	for (int i = 0; i < car_len; i++)
		for (int j = 0; j < car_width; j++)
		{
			car[i * car_width + j].y = car[car_width/2].y + i;
			car[i * car_width + j].x = car[car_width/2].x + (car_width/2 - j);
		}
}

static void LifeCheck (struct framebuffer_info fb_info, struct coords* car, int* win)
{
	for (int i = 0; i < car_len*car_width; i++)
	{
			if ((car[i].x >= fb_info.scr.xres)||(car[i].x <= 0)||(car[i].y >= fb_info.scr.yres)||(car[i].y <= 0))
			{
				life_flag = 0;
				key_flag = 0;
				*win = 0;
				Pthread_cancel(host_thread);
				Pthread_cancel(rival_thread);
				need_answer = 0;
				break;
			}
			else if (fb_info.ptr[car[i].y * fb_info.scr.xres_virtual + car[i].x] != default_color)
			{
				life_flag = 0;
				key_flag = 0;
				*win = 0;
				Pthread_cancel(host_thread);
				Pthread_cancel(rival_thread);
				need_answer = 0;
				break;
			}
	}
}

static void DirecrionChoose (char key, struct coords* car)
{
	switch(key)
		{
			case 'a':
				MoveToLeft(car);
				break;
			case 'd':
				MoveToRight(car);
				break;
			case 'w':
				MoveUp(car);
				break;
			case 's':
				MoveDown(car);
				break;
			default:
				break;
		}
}

static void OneMove(struct framebuffer_info fb_info, struct coords* car, uint32_t car_color, uint32_t tail_color, char key, int* win)
{
	CarDraw(fb_info, car, default_color);
	fb_info.ptr[car[car_width/2].y * fb_info.scr.xres_virtual + car[car_width/2].x] = tail_color;
	DirecrionChoose (key, car);
	LifeCheck(fb_info, car, win);
	if (life_flag)
		CarDraw(fb_info, car, car_color);
}

static void Turn(struct framebuffer_info fb_info, struct coords* car, uint32_t car_color, uint32_t tail_color, char key, int* win)
{
	CarDraw(fb_info, car, default_color);
	fb_info.ptr[car[car_width/2].y * fb_info.scr.xres_virtual + car[car_width/2].x] = tail_color;
	DirecrionChoose (key, car);
	LifeCheck(fb_info, car, win);
}

void* CarsMovements(void* void_info)
{
	struct CarsMovements_info* info = (struct CarsMovements_info*)void_info;
	if (old_red_key != info->red_key)
		Turn(info->fb_info, red_car, red_color, red_tail_color, info->red_key, &red_win);
	if (old_blue_key != info->blue_key)
		Turn(info->fb_info, blue_car, blue_color, blue_tail_color, info->blue_key, &blue_win);
	while(key_flag&&life_flag)
	{
		OneMove(info->fb_info, red_car, red_color, red_tail_color, info->red_key, &red_win);
		OneMove(info->fb_info, blue_car, blue_color, blue_tail_color, info->blue_key, &blue_win);
		#if CLOCK_PER_SEC == 1000000
		usleep(speed - (useconds_t)(clock() - time_of_last_draw_end));
		time_of_last_draw_end = clock();
		#else
		usleep(speed);
		#endif
	}
	old_red_key = info->red_key;
	old_blue_key = info->blue_key;
}

void CarMovements_sync(struct CarsMovements_info* info)
{
	if (old_red_key != info->red_key)
		Turn(info->fb_info, red_car, red_color, red_tail_color, info->red_key, &red_win);
	if (old_blue_key != info->blue_key)
		Turn(info->fb_info, blue_car, blue_color, blue_tail_color, info->blue_key, &blue_win);
	if (!life_flag) return;
	OneMove(info->fb_info, red_car, red_color, red_tail_color, info->red_key, &red_win);
	OneMove(info->fb_info, blue_car, blue_color, blue_tail_color, info->blue_key, &blue_win);
	old_red_key = info->red_key;
	old_blue_key = info->blue_key;
}
