#ifndef COMMON_TRUCK_H
#define COMMON_TRUCK_H

#include "WarehouseCommon.h"

#define TRUCK_ARRIVAL "truck_arrival_semaphore_name"
#define TRUCK_DOCK_AVAILALBLE "truck_dock_available_semaphore_name"
#define TRUCK_HAS_DOCKED "truck_has_dock_semaphore_name"
#define TRUCK_FINISHED "truck_finish_semaphore_name"


#define TRUCK_SHARED_MUTEX "truck_shared_mutex"
#define TRUCK_MEMORY_NAME "truck_shared_memory"
#define MAX_TRUCK_CAPACITY 200
#define MAX_CHAR 40
#define MAX_DOCKS 9
#define MAGIC_NUM 4321
#define MAX_ITEMS 10


struct SharedItem {
	int itemQuantity;
	char itemName[MAX_CHAR];
	int itemID;
	double itemWeight;
};

struct TruckDock {
	bool isOccupied;
	bool isDeliveryTruck;
	bool isDone;
	SharedItem items[MAX_ITEMS];
};

struct SharedDockBay {
	TruckDock dockBay[MAX_DOCKS];
	int ndocks;
	int magic_num;
	bool quit;
};

#endif //COMMON_TRUCK_h