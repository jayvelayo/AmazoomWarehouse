

#include <cpen333\process\semaphore.h>
#include <cpen333\process\shared_memory.h>
#include <cpen333\process\mutex.h>
#include <cpen333\process\condition_variable.h>
#include <vector>

#include "Common_truck.h"
#include "Truck.h"
#include "JsonConverter.h"

#include "WarehouseObjects.h"



/**
* Loads items from a JSON file
* @param filename file to load items from
* @return vector of items
*/
std::vector<Item> getTruckList(const std::string& filename) {

	std::ifstream fin(filename);
	if (fin.is_open()) {
		JSON j;
		fin >> j;
		std::vector<Item> itemList = JsonConverter::parseItems(j);
		std::cout << "File loaded.\n";
		return itemList;
	}
	else {
		std::cerr << "Failed to open file: " << filename << std::endl;
	}
}


int main(void) {
	std::vector<Item> itemList = getTruckList("./data/restocktruck.json");
	InventoryTruck truck(itemList, false, true);



	//initialization
	cpen333::process::semaphore truck_arrival(TRUCK_ARRIVAL, 0);
	cpen333::process::semaphore dock_available(TRUCK_DOCK_AVAILALBLE, 0);
	cpen333::process::semaphore truck_ready(TRUCK_HAS_DOCKED, 0);
	cpen333::process::condition_variable is_loaded(TRUCK_FINISHED);
	cpen333::process::mutex mutex(TRUCK_SHARED_MUTEX);

	cpen333::process::shared_object<SharedDockBay> memory(TRUCK_MEMORY_NAME);

	if (memory->magic_num != MAGIC_NUM) {
		std::cout << "Memory not initialized." << std::endl;
		return 1;
	}

	std::cout << "Truck is carrying:\n";
	std::cout << "Item name:" << "\t" << "item ID:" << "\t" << "Quantity:" << "\t" << "Weight:" << std::endl;
	for (auto item : itemList) {
		std::cout << item.itemName << "\t" << item.itemID << "\t\t" << item.itemQuantity << "\t\t" << "$" << item.itemWeight << std::endl;
	}

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
		memory->dockBay[lowestDock].isDeliveryTruck = false;
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