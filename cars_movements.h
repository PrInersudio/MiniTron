#ifndef CARS_MOVEMENTS
#define CARS_MOVEMENTS
struct coords
{
	int x;
	int y;
};

struct CarsMovements_info
{
	char red_key;
	char blue_key;
	struct framebuffer_info fb_info;
};

void* CarsMovements(void* void_info);
void CarMovements_sync(struct CarsMovements_info* info);
void CarStartPos (struct framebuffer_info fb_info);
void BorderDraw (struct framebuffer_info fb_info);
void CarDraw(struct framebuffer_info fb_info, struct coords* car, uint32_t color);
#endif
