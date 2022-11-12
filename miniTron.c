#include "cars_char.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdint.h>
#include <linux/fb.h>
#include <pthread.h>
#include <termios.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include "cars_char.h"
#include "udp_p2p.h"
#include "screen_initialization.h"
#include "threads.h"
#include "cars_movements.h"
#include "controls.h"
extern int life_flag;
extern int  move_flag;
extern int tie;
extern pthread_t host_thread;
extern pthread_t rival_thread;
extern pthread_t sync_controls_thread;
extern pthread_mutex_t move_mutex;
extern int need_answer;
extern char move_num;
extern int red_win;
extern int blue_win;
extern struct coords* red_car;
extern struct coords* blue_car;

struct destructor_targets
{
	int sd;
	uint32_t *fb_ptr;
	size_t map_size;
	int fb_fd;
	struct termios old_terminal_info;
	int terminal_set;
	char host_color[20];
};

struct destructor_targets des_tar = {-1, MAP_FAILED, 0, -1, {0}, 0, "ты"};
int sync_mode = 0;

void handler(int none)
{
	life_flag = 0;
	move_flag = 0;
	tie = 1;
	if (host_thread != 0)
		Pthread_cancel(host_thread);
	if (rival_thread != 0)
		Pthread_cancel(rival_thread);
	if (sync_controls_thread != 0)
	{
		need_answer = 0;
		usleep(5);
		Pthread_cancel(sync_controls_thread);
	}
	printf("Программа остановлена\n");
}

int main (int argc, char* argv[])
{
	if ((argc < 4)||(argc > 5))
	{
		printf("Запускать следующим образом: ./miniTron <длина экрана> <ширина экрана> <ip соперника>\n");
		tie = 1;
		return -1;
	}
	else if (argc == 5)
		sync_mode = 1;
	if ((atoi(argv[1])< car_len*2)||(atoi(argv[2])< car_width))
	{
		printf("Размеры экрана заданы неправильно\n");
		tie = 1; 
		return -2;
	}
	signal(SIGINT, handler);
	struct connection_data con_data = InternetConnection(argv[3]);
	des_tar.sd = con_data.sd;
	struct framebuffer_info fb_info = ScreenInitialization((unsigned int)atoi(argv[1]), (unsigned int)atoi(argv[2]));
	des_tar.fb_ptr = fb_info.ptr;
	des_tar.map_size = fb_info.map_size;
	des_tar.fb_fd = fb_info.fd;
	des_tar.old_terminal_info = NonCanonNoEchoInput(&des_tar.terminal_set);
	red_car = (struct coords*)malloc(car_len*car_width*sizeof(struct coords));
	blue_car = (struct coords*)malloc(car_len*car_width*sizeof(struct coords));
	CarStartPos (fb_info);
	BorderDraw(fb_info);
	CarDraw(fb_info, red_car, red_color);
	CarDraw(fb_info, blue_car, blue_color);
	struct Controls_info host_control, rival_control;
	struct CarsMovements_info CM_info = {'d', 'a', fb_info};
	if (con_data.addr_host.sin_addr.s_addr > con_data.addr_rival.sin_addr.s_addr)
	{
		struct Controls_info h_control = {con_data.sd, 1, (struct sockaddr *)&con_data.addr_rival, &CM_info};
		struct Controls_info r_control = {con_data.sd, 0, (struct sockaddr *)&con_data.addr_rival, &CM_info};
		host_control = h_control;
		rival_control = r_control;
		strcpy(des_tar.host_color, "красный");
	}
	else
	{
		struct Controls_info h_control = {con_data.sd, 0, (struct sockaddr *)&con_data.addr_rival, &CM_info};
		struct Controls_info r_control = {con_data.sd, 1, (struct sockaddr *)&con_data.addr_rival, &CM_info};
		host_control = h_control;
		rival_control = r_control;
		strcpy(des_tar.host_color, "синий");
	}
	Pthread_mutex_init(&move_mutex, NULL);
	if (sync_mode)
	{
		sync_controls_thread = pthread_self();
		Pthread_create(&host_thread, NULL, HostControls_sync, (void*)&host_control);
		Pthread_create(&rival_thread, NULL, RivalControls_sync, (void*)&rival_control);
		SyncControls(&host_control);
	}
	else
	{
		rival_thread = pthread_self();
		Pthread_create(&host_thread, NULL, HostControls, (void*)&host_control);
		RivalControls (&rival_control);
	}
	char package;
	if (con_data.addr_host.sin_addr.s_addr > con_data.addr_rival.sin_addr.s_addr)
		package = CM_info.red_key + (move_num-1)%2;
	else
		package = CM_info.blue_key + (move_num-1)%2;
	for (int i = 0; i<10; i++)
		sendto(con_data.sd, &package, 1, 0, (struct sockaddr *)&con_data.addr_rival, sizeof(struct sockaddr));
}

void __attribute__((destructor)) destructor(void)
{
	if ((red_car!=NULL)&&(blue_car!=NULL))
		for (int i = 0; i < car_len*car_width; i++)
			for (int j = 0; j < car_len*car_width; j++)
				if ((red_car[i].x == blue_car[j].x)&&(red_car[i].y == blue_car[j].y))
					tie = 1;
	if (tie)
		printf("Проиграл %s\n", des_tar.host_color);
	else if (red_win)
		printf("Проиграл синий\n");
	else if (blue_win)
		printf("Проиграл красный\n");
	else
		printf("Проиграл %s\n", des_tar.host_color);
	if (des_tar.terminal_set)
		Tcsetattr(0, TCSANOW, &des_tar.old_terminal_info);
	if (red_car!=NULL)
		free(red_car);
	if (blue_car!=NULL)
		free(blue_car);
	if (des_tar.sd != -1)
		close(des_tar.sd);
	if (des_tar.fb_ptr != MAP_FAILED)
		munmap(des_tar.fb_ptr, des_tar.map_size);
	if (des_tar.fb_fd>=0)
  		close(des_tar.fb_fd);
  	pthread_mutex_destroy(&move_mutex);
  	tcflush(0, TCIFLUSH);
}
