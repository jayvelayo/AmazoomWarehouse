

#include <cpen333\process\semaphore.h>
#include <cpen333\process\shared_memory.h>
#include <cpen333\process\mutex.h>
#include <cpen333\process\condition_variable.h>
#include <vector>
#include "JsonConverter.h"

#include "WarehouseObjects.h"
#include "Common_truck.h"
#include "Truck.h"


int main(void) {
	std::vector<Item> itemList;
	DeliveryTruck deliveryTruck(itemList, false, true);
	

	//initialization
	cpen333::process::semaphore truck_arrival(TRUCK_ARRIVAL, 0);
	cpen333::process::semaphore dock_available(TRUCK_DOCK_AVAILALBLE, 0);
	cpen333::process::semaphore truck_ready(TRUCK_HAS_DOCKED, 0);
	cpen333::process::condition_variable is_loaded(TRUCK_FINISHED);
	cpen333::process::mutex mutex(TRUCK_SHARED_MUTEX);

	cpen333::process::shared_object<SharedDockBay> memory(TRUCK_MEMORY_NAME);



	//notify computer i have arrived
	truck_arrival.notify();

	//wait for instructions
	dock_available.wait();

	//dock at the lowest # port
	int lowestDock = -1;
	int ndocks = memory->ndocks;
	while (lowestDock == -1) {
		for (int i = 0; i < memory->ndocks; i++) {
			if (!(memory->dockBay[i].isOccupied)) {
				memory->dockBay[i].isOccupied = true;
				lowestDock = i;
				break;
			}
		}
	}

	std::cout << "Parking at dock # " << lowestDock << std::endl;
	{
		std::lock_guard<decltype(mutex)> mylock(mutex);
		memory->dockBay[lowestDock].isDeliveryTruck = true;
	}
	truck_ready.notify();

	std::unique_lock<decltype(mutex)> mylock(mutex);
	while (!memory->dockBay[lowestDock].isDone) {
		is_loaded.wait(mylock);
	}
	std::cout << "Truck has been unloaded and is now leaving\n";

	//reset memory
	{
		std::lock_guard<decltype(mutex)> mylock(mutex);
		memory->dockBay[lowestDock].isDone = false;
		memory->dockBay[lowestDock].isDeliveryTruck = false;
		memory->dockBay[lowestDock].isOccupied = false;
	}

	std::cin.get();




	


	return 0;
}