/**
 * @file
 *
 * This is the main server process.  When it starts it listens for clients.  It then
 * accepts remote commands for modifying/viewing the music database.
 *
 */

#include <fstream>
#include <iostream>
#include <thread>
#include <memory>
#include <mutex>
#include <limits>

#include "JsonUserClientApi.h"
#include "Warehouse.h"
#include "DeliveryCompQueue.h"
#include "DeliveryTruckQueue.h"
#include "PickupQueue.h"
#include "RestockingQueue.h"
#include "Common_truck.h"
#include "safe_printf.h"
#include "Robot.h"
#include "WarehouseCommon.h"
#include "WarehouseInventory.h"
#include "JsonConverter.h"

#include <cpen333/process/socket.h>
#include <cpen333/process/mutex.h>
#include <cpen333\process\semaphore.h>
#include <cpen333\process\shared_memory.h>
#include <cpen333\process\shared_mutex.h>

#define LOW_STOCK 5

static const char USER_CHECK_ORDER = '1';
static const char USER_CHECK_ITEM = '2';
static const char USER_ADD_ROBOT = '3';
static const char USER_QUIT = '4';

// print menu options
void print_menu() {

	std::cout << "=========================================" << std::endl;
	std::cout << "=            WAREHOUSE MENU             =" << std::endl;
	std::cout << "=========================================" << std::endl;
	std::cout << " (1) Check Order Status" << std::endl;
	std::cout << " (2) Check Item Availability" << std::endl;
	std::cout << " (3) Add Robot" << std::endl;
	std::cout << " (4) Quit " << std::endl;
	std::cout << "=========================================" << std::endl;
	std::cout << "Enter number: ";
	std::cout.flush();

}

 /**
 * Main thread function for handling communication with a single remote
 * client.
 *
 * @param lib shared library
 * @param api communication interface layer
 * @param id client id for printing messages to the console
 */
void service(WarehouseInventory &lib, OrderList &orderList, UserClientApi &&api, int id, PickupQueue &pick) {

	cpen333::process::mutex mutex("Server_Mutex");
	std::unique_lock<cpen333::process::mutex> memLock(mutex);
	memLock.unlock();

	std::cout << "Client " << id << " connected" << std::endl;

	// receive message
	std::unique_ptr<Message> msg = api.recvMessage();
	mutex.lock();
	/*
	Lock Mutex after receiving new data. This prevents any threads inserting data into
	the socket in between receiving and sending response message.
	*/

	// continue while we don't have an error
	while (msg != nullptr) {

		// react and respond to message
		MessageType type = msg->type();
		switch (type) {

		case MessageType::SEARCH: {
			// process "search" message
			// get reference to SEARCH
			SearchMessage &search = (SearchMessage &)(*msg);

			std::cout << "Client " << id << " searching for: "
				<< search.itemName << std::endl;

			// search library
			std::vector<ItemEntry> results;
			results = lib.find(search.itemName, search.itemID);

			// send response
			if (results.size() > 0) {
				api.sendMessage(SearchResponseMessage(results, MESSAGE_STATUS_OK));
			}
			else {
				api.sendMessage(SearchResponseMessage(results, MESSAGE_STATUS_ERROR));
			}

			break;
		}

		
		case MessageType::ADD: {
			// process "add" message
			// get reference to ADD
			AddMessage &add = (AddMessage &)(*msg);
			std::cout << "Client " << id << " adding item to order: " << add.itemName << std::endl;


			bool success = false;
			//check if item exists and if available
			std::vector<ItemEntry> results;
			results = lib.find(add.itemName, add.itemID);

			if (results.size() == 1) {
				success = lib.holdItem(add.itemID, add.itemQuantity);
			}

			// send response
			if (success == true) {
				api.sendMessage(AddResponseMessage(results, MESSAGE_STATUS_OK, "Item placed on hold successfully!"));
				std::cout <<add.itemQuantity<< " x " << add.itemName << " placed on hold successfully!" << std::endl;

				//updated result in library after placing product on hold
				results = lib.find(add.itemName, add.itemID);

				if (results[0].quantityAvailable <= LOW_STOCK) {
				std::cout << results[0].itemName << " is low on stock " << std::endl;
				std::cout << "Quantity Available: "<< results[0].quantityAvailable << std::endl;
				std::cout << "Quantity On Hold: " << results[0].quantityOnHold << std::endl;
				}
			}
			else {
				api.sendMessage(AddResponseMessage(results, MESSAGE_STATUS_ERROR, "Unable to add product"));
				std::cout << "Error: Client" << id << "unable to add product" << std::endl;
			}
			break;
		}

		case MessageType::CONFIRM_ORDER: {
			ConfirmOrder &confirm = (ConfirmOrder &)(*msg);
			 
			//find order entry associated with ordernum
			int orderNum = orderList.addEntry(confirm.order);
			OrderEntry entry = orderList.searchOrder(orderNum);
			std::vector<Item> items = entry.entrytoitem(entry);

			//generate an order 
			Order order(items, orderNum);

			//add to pick queue for robots
			pick.addToPQueue(order);

			api.sendMessage(ConfirmOrderResponseMessage(orderNum, MESSAGE_STATUS_OK));

			std::cout << "Client " << id << " confirmed order #" << orderNum <<std::endl;
			break;
		}

		case MessageType::CANCEL_ORDER: {
			// process "cancel" message
			// get reference to cancel
			CancelOrder &cancel = (CancelOrder &)(*msg);
			
			OrderEntry &order = orderList.searchOrder(cancel.orderNum);

			if (order.status == "Confirmed") 
			{
				api.sendMessage(CancelOrderResponseMessage(MESSAGE_STATUS_OK, "Cancelled"));
				orderList.changeStatus(cancel.orderNum, "Cancelled");
				std::cout << "Client " << id << " cancelled order #" << cancel.orderNum << std::endl;
			}
			else if (order.status == "Enroute to Delivery") 
			{
				api.sendMessage(CancelOrderResponseMessage(MESSAGE_STATUS_ERROR, "Enroute to Delivered"));
				std::cout << "Client " << id << "'s order #" << cancel.orderNum << "unable to be cancelled. Currently enroute to delivery"<<std::endl;
			}
			else if (order.status == "Cancelled") {
				api.sendMessage(CancelOrderResponseMessage(MESSAGE_STATUS_ERROR, "Already Cancelled"));
				std::cout << "Client " << id << " has already cancelled order #" << cancel.orderNum << std::endl;
			}
			break;

		}
		
		case MessageType::GOODBYE: {
			// process "goodbye" message
			std::cout << "Client " << id << " closing" << std::endl;
			return;
		}
		default: {
			std::cout << "Client " << id << " sent invalid message" << std::endl;
		}
		}

		// receive next message

		//very inefficient mutex locking lol
		mutex.unlock();
		msg = api.recvMessage();
		mutex.lock();
	}
}
 

void do_check_order(OrderList &list) {
	int ordernum;
	std::cout << "Enter order # : ";
	std::cin >> ordernum;
	OrderEntry entry = list.searchOrder(ordernum);
	if (entry.orderNum != -1) {
		std::cout << "Order ID: " << entry.orderNum << "\t Order status: " << entry.status << "\n";
		std::cout << "Items in order:\n";
		std::cout << "Item Name\tItemID\t\tQuantity Ordered\t\n";

		for (auto item: entry.itemList) {
		std::cout << item.itemName << "\t" << item.ID << "\t\t" << item.quantityAvailable << "\t" << std::endl;

		}
	}
	else {
		std::cout << "Order not found!\n\n";
	}
}

void do_check_item(WarehouseInventory &lib) {
	int itemID;

	std::cout << "Item ID: ";
	std::cin >> itemID;
	
	ItemEntry entry = lib.find_id(itemID);

	if (entry.ID == itemID) {
		std::cout << "Quantity Available: " << entry.quantityAvailable << "\t Quantity On Hold: " << entry.quantityOnHold << "\n";
	}
	else {
		std::cout << "ERROR: Invalid Item\n";
	}
}

void do_add_robot(std::vector<Robot*> &robots, Warehouse &warehouse, RestockingQueue &restock,
	DeliveryCompQueue &compQueue, DeliveryTruckQueue &truckQueue, PickupQueue &pickup) {
	cpen333::process::shared_object<SharedData> memory(WAREHOUSE_MEMORY_NAME);
	cpen333::process::mutex mutex(WAREHOUSE_MUTEX_NAME);

	{
		std::lock_guard<decltype(mutex)> mylock(mutex);
		memory->rinfo.nrobots = memory->rinfo.nrobots + 1;
		int id_ = memory->rinfo.nrobots;
		robots.push_back(new Robot(id_, std::ref(warehouse), std::ref(restock), std::ref(pickup), std::ref(compQueue), std::ref(truckQueue)));
		robots.back()->start();
	}

}

void do_quit() {
	std::cout << "Goodbye user! :(" << std::endl;
}

/** 
* Provides a User interface to access warehouse databases
*/
void warehouseUI(std::vector<Robot*> &robots,WarehouseInventory &lib, OrderList &orderList, Warehouse &warehouse, RestockingQueue &restock,
	DeliveryCompQueue &compQueue, DeliveryTruckQueue &truckQueue, PickupQueue &pickup) 
{
	
	char cmd=0;
	
	std::cout << "Hello user! :)" << std::endl;
	while (cmd != USER_QUIT) {
		print_menu();
		std::cin >> cmd;
		switch (cmd) {
		case USER_CHECK_ORDER:
			do_check_order(orderList);
			break;
		case USER_CHECK_ITEM:
			do_check_item(lib);
			break; 
		case USER_ADD_ROBOT:
			do_add_robot(robots, warehouse, restock,compQueue,truckQueue,pickup);
			break;
		case USER_QUIT: 
			do_quit();
			break; 
		default:
			std::cout << "Invalid command number " << cmd << std::endl << std::endl;
		}
	}

	for (auto& PickingRobot : robots) {
		PickingRobot->join();
	}

}



/**
* Loads items from a JSON file
* @param filename file to load itementries from
* @return number of new items added to the warehouse
*/
WarehouseInventory load_inventory(const std::string& filename) {

	std::ifstream fin(filename);
	if (fin.is_open()) {
		JSON j;
		fin >> j;
		std::vector<ItemEntry> entries = JsonConverter::parseEntries(j); 

		WarehouseInventory inventory(entries);  //return an initialized warehouse of item entires
		return inventory;
	}
	else {
		std::cerr << "Failed to open file: " << filename << std::endl;
	}
}

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
				for (unsigned int col = 0; col<cols; ++col) {
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

			if (i == warehouse.Shelves.size() - 1) {
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

		return itemList;
	}
	else {
		std::cerr << "Failed to open file: " << filename << std::endl;
		std::vector<Item> empty;
		return empty;
	}
}


void truckMonitor(Warehouse &warehouse, OrderList &orderList, RestockingQueue &restock, 
	DeliveryCompQueue &completedOrdersQueue, DeliveryTruckQueue &sendToTruckQueue, int ndocks) {
	//initialization
	cpen333::process::semaphore truck_arrival(TRUCK_ARRIVAL, 0);
	cpen333::process::semaphore dock_available(TRUCK_DOCK_AVAILALBLE,0);
	cpen333::process::semaphore truck_ready(TRUCK_HAS_DOCKED,0);
	cpen333::process::semaphore load_ready(TRUCK_FINISHED,0);
	cpen333::process::mutex mutex(TRUCK_SHARED_MUTEX);

	cpen333::process::shared_object<SharedDockBay> memory(TRUCK_MEMORY_NAME);

	//initialize shared memory
	{
		std::lock_guard<decltype(mutex)> mylock(mutex);
		memory->ndocks = ndocks;
		for (int i = 0; i < ndocks; i++) {
			memory->dockBay[i].isOccupied = false;
			memory->dockBay[i].isDeliveryTruck = false;
			memory->dockBay[i].isDone = false;
		}
		memory->magic_num = MAGIC_NUM;
		memory->quit = false;
	}

	while (!memory->quit) {
		bool truck_arrived = truck_arrival.try_wait();
		if (truck_arrived) {
			std::cout << "Truck Arrived" << std::endl;
			int lowestDock = -1;
			while (lowestDock == -1) {
				for (int i = 0; i < ndocks; i++) {
					if (!(memory->dockBay[i].isOccupied)) {
						lowestDock = i;
						break;
					}
				}
			}

			dock_available.notify();
			std::cout << "Truck parked at dock " << lowestDock << std::endl;
			truck_ready.wait();
			if (memory->dockBay[lowestDock].isDeliveryTruck) {
				Order newOrder = completedOrdersQueue.removeFromDCQueue();
				orderList.changeStatus(newOrder.getOrderNum(), "LOADING");
				sendToTruckQueue.addToDTQueue(tCommand(newOrder, lowestDock));
			}
			else {
				std::vector<Item> truckList = getTruckList(".\data\restocktruck.json");
				restock.addToTQueue(trCommand(lowestDock));
			}
		}
	}

}

int main() {

	//initialize the memory
	cpen333::process::shared_object<SharedData> memory(WAREHOUSE_MEMORY_NAME);
	cpen333::process::mutex mutex(WAREHOUSE_MUTEX_NAME);


	//Make a warehouse and DA QUEUES
	Warehouse warehouse;
	PickupQueue pick;
	DeliveryCompQueue delivercomp; //from robot to computer
	DeliveryTruckQueue delivertruck; //from computer to robot
	RestockingQueue restock;
	WarehouseInventory inv = load_inventory("./data/inventory.json");
	OrderList orderList;

	
	int ndocks;
	//initialize shared memory
	std::string maze = "./data/maze0.txt";
	{
		std::lock_guard<decltype(mutex)> mylock(mutex);
		load_layout(maze, memory->winfo, memory->rinfo);
		memory->rinfo.nrobots = 0;
		memory->quit = false;
		memory->magic = MAGIC_NUMBER;

		//find all the shelves in the warehouse 
		warehouse.findAllShelves(maze, memory->winfo);
		//put products on the shelf!! (items) 
		warehouse = load_warehouse("./data/inventory.json", warehouse);
		//find start location of robots
		warehouse.findStart(maze, memory->rinfo, memory->winfo);
		ndocks = memory->winfo.docks.ndocks;
	}


	

	
	std::vector<Robot*> robots;

	// start server
	cpen333::process::socket_server server(MUSIC_LIBRARY_SERVER_PORT);
	server.open();
	std::cout << "Server started on port " << server.port() << std::endl;

	cpen333::process::socket client;
	int clientCount = 1;

	//open UI thread
	std::thread userUI(warehouseUI, std::ref(robots), std::ref(inv), std::ref(orderList), std::ref(warehouse),std::ref(restock),std::ref(delivercomp), std::ref(delivertruck), std::ref(pick));
	

	//thread that handles trucks
	std::thread truckMonitoring(truckMonitor, std::ref(warehouse), std::ref(orderList), 
		std::ref(restock),std::ref(delivercomp), std::ref(delivertruck), ndocks);
	
	//listen for clients
	while (server.accept(client)) {
		// create API handler
		JsonUserClientApi api(std::move(client));
		// service client-server communication
		std::thread thread(service, std::ref(inv), std::ref(orderList), std::move(api), clientCount, std::ref(pick));
		thread.detach();
		clientCount++;
	}

	truckMonitoring.join(); //or detach?
	userUI.join();
	// close server
	server.close();

	return 0;
}

