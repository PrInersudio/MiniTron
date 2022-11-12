#include "cars_char.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <termios.h>
#include <time.h>
#include <linux/fb.h>
#include "screen_initialization.h"
#include "threads.h"
#include "cars_movements.h"
#include "controls.h"
int host_start = 0;
int rival_start = 0;
int move_flag = 0;
char move_num = 0;
pthread_mutex_t move_mutex;
pthread_t move_thread;
extern int need_answer;
extern int life_flag;
extern clock_t time_of_last_draw_end;
extern int key_flag;

void* HostControls (void* void_info)
{
	struct Controls_info* info = (struct Controls_info*)void_info;
	char key;
	char* key_pointer;
	if (info->is_red)
		key_pointer = &(info->CM_info->red_key);
	else
		key_pointer = &(info->CM_info->blue_key);
	socklen_t tolen = (socklen_t)sizeof(*(info->addr));
	getchar(); /*ждём первого нажатия*/
	sendto(info->sd, "b", 1, 0, info->addr, tolen); //что угодно послать, чтобы он перестал ждать
	host_start = 1;
	while ((!rival_start)&&(life_flag)) /*ждём соперника*/
		sendto(info->sd, "b", 1, 0, info->addr, tolen); //что угодно послать, чтобы он перестал ждать
	time_of_last_draw_end = clock();
	tcflush(0, TCIFLUSH);
	while (life_flag)
	{
		Pthread_mutex_lock(&move_mutex);
		if (!move_flag)
		{
			Pthread_create(&move_thread, NULL, CarsMovements, (void*)info->CM_info);
			move_flag = 1;
		}
		Pthread_mutex_unlock(&move_mutex);
		do
			key = (char)getchar();
		while((!((key=='d')||(key=='a')||(key=='w')||(key=='s')) || ((*key_pointer=='s')&&(key=='w')||(*key_pointer=='w')&&(key=='s')||(*key_pointer=='a')&&(key=='d')||(*key_pointer=='d')&&(key=='a')||(key==*key_pointer))) && (life_flag));
		sendto(info->sd, &key, 1, 0, info->addr, tolen);
		Pthread_mutex_lock(&move_mutex);
		key_flag = 0;
		if (move_flag)
			Pthread_join(move_thread, NULL);
		move_flag = 0;
		key_flag = 1;
		Pthread_mutex_unlock(&move_mutex);
		*key_pointer = key;
	}
}

void RivalControls (struct Controls_info* info)
{
	struct sockaddr_in* in_addr = (struct sockaddr_in*)(info->addr);
	struct sockaddr_in local_addr_info = *(in_addr);
	char key;
	char* key_pointer;
	if (info->is_red)
		key_pointer = &(info->CM_info->red_key);
	else
		key_pointer = &(info->CM_info->blue_key);
	socklen_t fromlen = (socklen_t)sizeof(struct sockaddr);
	while (1) /*ждём первого нажатия*/
	{
		recvfrom(info->sd, &key, 1, 0, (struct sockaddr*)&local_addr_info, &fromlen);
		if (local_addr_info.sin_addr.s_addr != in_addr->sin_addr.s_addr) continue;
		break;
	}
	rival_start = 1; 
	while ((!host_start)&&(life_flag)) {/*нас ждут*/};
	time_of_last_draw_end = clock();
	while (life_flag)
	{
		Pthread_mutex_lock(&move_mutex);
		if (!move_flag)
		{
			Pthread_create(&move_thread, NULL, CarsMovements, (void*)info->CM_info);
			move_flag = 1;
		}
		Pthread_mutex_unlock(&move_mutex);
		do
		{
			recvfrom(info->sd, &key, 1, 0, (struct sockaddr*)&local_addr_info, &fromlen);
			if (local_addr_info.sin_addr.s_addr != in_addr->sin_addr.s_addr) continue;
		}
		while((!((key=='d')||(key=='a')||(key=='w')||(key=='s')) || ((*key_pointer=='s')&&(key=='w')||(*key_pointer=='w')&&(key=='s')||(*key_pointer=='a')&&(key=='d')||(*key_pointer=='d')&&(key=='a')||(key==*key_pointer))) && (life_flag));
		Pthread_mutex_lock(&move_mutex);
		key_flag = 0;
		if (move_flag)
			Pthread_join(move_thread, NULL);
		move_flag = 0;
		key_flag = 1;
		Pthread_mutex_unlock(&move_mutex);
		*key_pointer = key;
	}
}

void* HostControls_sync(void* void_info)
{
	struct Controls_info* info = (struct Controls_info*)void_info;
	char key;
	char* key_pointer;
	if (info->is_red)
		key_pointer = &(info->CM_info->red_key);
	else
		key_pointer = &(info->CM_info->blue_key);
	socklen_t tolen = (socklen_t)sizeof(*(info->addr));
	getchar(); /*ждём первого нажатия*/
	sendto(info->sd, "\0", 1, 0, info->addr, tolen); //что угодно послать, чтобы он перестал ждать
	host_start = 1;
	while ((!rival_start)&&(life_flag)) /*ждём соперника*/
		sendto(info->sd, "\0", 1, 0, info->addr, tolen); //что угодно послать, чтобы он перестал ждать
	tcflush(0, TCIFLUSH);
	while (life_flag)
	{
		key = (char)getchar();
		if (((key=='d')||(key=='a')||(key=='w')||(key=='s')) && !((*key_pointer=='s')&&(key=='w')||(*key_pointer=='w')&&(key=='s')||(*key_pointer=='a')&&(key=='d')||(*key_pointer=='d')&&(key=='a')||(key==*key_pointer)))
		{
			Pthread_mutex_lock(&move_mutex);
			*key_pointer = key;
			Pthread_mutex_unlock(&move_mutex);
		}
	}
}

void* RivalControls_sync (void* void_info)
{
	struct Controls_info* info = (struct Controls_info*)void_info;
	struct sockaddr_in* in_addr = (struct sockaddr_in*)(info->addr);
	struct sockaddr_in local_addr_info = *(in_addr);
	char key;
	char* key_pointer;
	if (info->is_red)
		key_pointer = &(info->CM_info->red_key);
	else
		key_pointer = &(info->CM_info->blue_key);
	socklen_t fromlen = (socklen_t)sizeof(struct sockaddr);
	while (1) /*ждём первого нажатия*/
	{
		recvfrom(info->sd, &key, 1, 0, (struct sockaddr*)&local_addr_info, &fromlen);
		if (local_addr_info.sin_addr.s_addr != in_addr->sin_addr.s_addr) continue;
		break;
	}
	rival_start = 1; 
	while ((!host_start)&&(life_flag)) {/*нас ждут*/};
	while (life_flag)
	{
		recvfrom(info->sd, &key, 1, 0, (struct sockaddr*)&local_addr_info, &fromlen);
		if (local_addr_info.sin_addr.s_addr != in_addr->sin_addr.s_addr) continue;
		key -= move_num%2;
		if (((key=='d')||(key=='a')||(key=='w')||(key=='s')) && need_answer)
		{
			need_answer = 0;
			if (!((*key_pointer=='s')&&(key=='w')||(*key_pointer=='w')&&(key=='s')||(*key_pointer=='a')&&(key=='d')||(*key_pointer=='d')&&(key=='a')||(key==*key_pointer)))
			*key_pointer = key;
		}
	}
}

void* SyncControls (struct Controls_info* info)
{
	char package;
	socklen_t tolen = (socklen_t)sizeof(*(info->addr));
	while (!(host_start&&rival_start)) {/*ждём, пока они оба начнут*/};
	if (info->is_red)
	{
		#if CLOCK_PER_SEC == 1000000
		time_of_last_draw_end = clock();
		#endif
		while (life_flag)
		{
			Pthread_mutex_lock(&move_mutex);
			package = (info->CM_info->red_key) + move_num%2;
			sendto(info->sd, &package, 1, 0, info->addr, tolen);
			need_answer = 1;
			while (need_answer)
			{
				sendto(info->sd, &package, 1, 0, info->addr, tolen);
				usleep(50);
			}
			CarMovements_sync(info->CM_info);
			Pthread_mutex_unlock(&move_mutex);
			#if CLOCK_PER_SEC == 1000000
			usleep(speed - (useconds_t)(clock() - time_of_last_draw_end));
			time_of_last_draw_end = clock();
			#else
			usleep(speed);
			#endif
			move_num++;
		}
	}
	else
	{
		while (life_flag)
		{
			need_answer = 1;
			while (need_answer)
			{
				sendto(info->sd, &package, 1, 0, info->addr, tolen);
				usleep(5);
			}
			Pthread_mutex_lock(&move_mutex);
			if (!life_flag)
			{
				Pthread_mutex_unlock(&move_mutex);
				continue;
			}
			package = (info->CM_info->blue_key) + move_num%2;
			for (int i = 0; i < 10; i++)
				sendto(info->sd, &package, 1, 0, info->addr, tolen);
			CarMovements_sync(info->CM_info);
			Pthread_mutex_unlock(&move_mutex);
			move_num++;
			usleep(30000);
		}
	}
}
