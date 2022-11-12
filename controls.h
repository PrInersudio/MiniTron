#ifndef CONTROLS
#define CONTROLS
struct Controls_info
{
	int sd;
	int is_red;
	struct sockaddr *addr;
	struct CarsMovements_info* CM_info;
};

void* HostControls (void* void_info);
void RivalControls (struct Controls_info* info);
void* HostControls_sync(void* void_info);
void* RivalControls_sync (void* void_info);
void* SyncControls (struct Controls_info* info);
#endif
