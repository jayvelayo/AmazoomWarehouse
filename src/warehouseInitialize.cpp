/**
* @file
*
* This is the main server process.  When it starts it listens for clients.  It then
* accepts remote commands for modifying/viewing the music database.
*
*/

#include <fstream>
#include <iostream>
#include <string>
#include <thread>
#include <memory>
#include <mutex>

#include <cpen333/process/mutex.h>
#include <cpen333/process/shared_memory.h>

#include "TruckQueue.h"
#include "Truck.h"

#include "safe_printf.h"
#include "Robot.h"
#include "Warehouse.h"
#include "WarehouseCommon.h"
#include "WarehouseInventory.h"
#include "JsonConverter.h"



/**
* Reads a warehouse from a filename and populates the warehouse
* @param filename file to load warehouse from
* @param winfo warehouse info to populate
*/
void load_layout(const std::string& filename, WarehouseInfo &winfo, RobotInfo &rinfo) {

	// initialize number of rows and columns
	winfo.rows = 0;
	winfo.cols = 0;

	std::ifstream fin(filename);
	std::string line;

	// read maze file
	if (fin.is_open()) {
		int row = 0;  // zeroeth row


		while (std::getline(fin, line)) {
			int cols = line.length();
			if (cols > 0) {
				// longest row defines columns
				if (cols > winfo.cols) {
					winfo.cols = cols;
				}
				for (size_t col = 0; col<cols; ++col) {
					winfo.maze[col][row] = line[col];
				}
				++row;
			}
		}
		winfo.rows = row;
		fin.close();
	}

	//initialize docks
	int ndocks = 0;
	winfo.docks.ndocks = 0;
	for (int i = 0; i <= winfo.rows; i++) {
		for (int j = 0; j <= winfo.cols; j++) {
			if (winfo.maze[j][i] == DOCK_CHAR) {
				winfo.docks.ndocks += 1;
				winfo.docks.dloc[ndocks][COL_IDX] = j;
				winfo.docks.dloc[ndocks][ROW_IDX] = i;
				ndocks++;
			}
		}
	}
	int test = 0;

}


/**
* Loads items from a JSON file
* @param filename file to load items from
* @return warehouse inventory
*/
WarehouseInventory load_inventory(const std::string& filename) {

	std::ifstream fin(filename);
	if (fin.is_open()) {
		JSON j;
		fin >> j;
		std::vector<ItemEntry> entries = JsonConverter::parseEntries(j); //can't call function, keeps throwing error

		WarehouseInventory inventory(entries);  //return an initialized warehouse of item entires
		return inventory; 
	}
	else {
		std::cerr << "Failed to open file: " << filename << std::endl;
		return WarehouseInventory();
	}
}

/**
* Loads items from a JSON file
* @param filename file to load items from
* @return warehouse 
*/
Warehouse load_warehouse(const std::string& filename, Warehouse warehouse) {

	std::ifstream fin(filename);
	if (fin.is_open()) {
		JSON j;
		fin >> j;
		std::vector<Item> items = JsonConverter::parseItems(j);

		for (int i = 0; i < items.size(); i++) {
			warehouse.Shelves[i].storeItem(items[i]);

			if (i == warehouse.Shelves.size()-1) {
				i = 0;
			}
		}
		return warehouse;
	}
	else {
		std::cerr << "Failed to open file: " << filename << std::endl;
		return Warehouse();
	}
}


int main(int argc, char* argv[]) {

	// read maze from command-line, default to maze0
	std::string maze = "./data/maze0.txt";
	if (argc > 1) {
		maze = argv[1];
	}
	cpen333::process::shared_object<SharedData> memory(WAREHOUSE_MEMORY_NAME);
	cpen333::process::mutex mutex(WAREHOUSE_MUTEX_NAME);
	load_layout(maze, memory->winfo, memory->rinfo);
	memory->rinfo.nrobots = 0;
	memory->quit = false;
	memory->magic = MAGIC_NUMBER;

	//make a warehouse 
	Warehouse warehouse;
	PickupQueue pick;
//	DeliveryQueue deliver; 
	//TruckQueue truck; 

	//find all the shelves in the warehouse 
	warehouse.findAllShelves(maze, memory->winfo);
	//memory->winfo.Shelves = warehouse.Shelves;
	//put products on the shelf!! (items) 
	warehouse = load_warehouse("./data/inventory.json", warehouse); 
	//find start location of robots
	warehouse.findStart(maze, memory->rinfo, memory->winfo);

	//initialize inventory (item entries)
	WarehouseInventory inventory = load_inventory("./data/inventory.json"); 
	
	// create and store threads
	std::vector<Robot*> robots;
	Order poisonorder(-1);
	Order POISON_ORDER = poisonorder;
	
	{
		memory->rinfo.nrobots = memory->rinfo.nrobots + 1; 
		int id_ = memory->rinfo.nrobots;
		robots.push_back(new Robot(id_, std::ref(warehouse), (pick)));
	}
	
	Order sampleOrder(10);
	pick.addToPQueue(sampleOrder);
	pick.addToPQueue(poisonorder);

	for (auto& PickingRobot : robots) {
		PickingRobot->join();
	}

	std::cout << "Keep this running until you are done with the program." << std::endl << std::endl;
	std::cout << "Press ENTER to quit." << std::endl;
	std::cin.get();

	memory->magic = 0;
	memory->quit = true;
	memory.unlink();

	return 0;
}