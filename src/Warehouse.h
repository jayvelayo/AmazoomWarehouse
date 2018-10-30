#ifndef PROJECT_WAREHOUSE_H
#define PROJECT_WAREHOUSE_H

#include "Shelf.h"
#include "WarehouseCommon.h"
#include "WarehouseObjects.h"
#include <vector>
#include <iostream>
#include <string>
#include <fstream>

/**
* The physical warehouse in which items are stored in shelves
*/

class Warehouse
{
	
public:
	std::vector <Shelf> Shelves;
	

	/**
	* Constructor- creates a new warehouse 
	* @param Shelves_ a vector of shelves located within the warehouse
	*/
	Warehouse() : Shelves() {};
	Warehouse(std::vector <Shelf> Shelves_) : Shelves(Shelves_) {};


	/**
	* Looks in warehouse layout for all shelves and updates shared memory
	* @param filename filename of warehouse inventory layout 
	* @param winfo warehouse info located in shared memory 
	*/
	void findAllShelves(const std::string& filename, WarehouseInfo& winfo) {
		int rows = winfo.rows;
		int cols = winfo.cols;

		for (int i = 0; i < rows; i++) {
			for (int j = 0; j < cols; j++) {
				if (winfo.maze[j][i] == 'S') {
					Coordinates coord(i, j);
					Shelves.push_back(Shelf(coord, MAX_SHELF_CAPACITY));
				}
			}
		}
	}
	
	/**
	* Looks in warehouse layout for start location of all robots and updates shared memory
	* @param filename filename of warehouse inventory layout
	* @param winfo warehouse info located in shared memory
	* @param rinfo robot info located in shared memory 
	*/
	void findStart(const std::string& filename, RobotInfo& rinfo, WarehouseInfo& winfo) {
		int rows = winfo.rows;
		int cols = winfo.cols;
		int success = 0;

		for (int i = 0; i < rows; i++) {
			for (int j = 0; j < cols; j++) {
				if (winfo.maze[j][i] == START_CHAR) {
					rinfo.startx = i;
					rinfo.starty = j;
					success += 1;
					if (success == 2) break;
				}
				else if (winfo.maze[j][i] ==EXIT_CHAR) {
					rinfo.endx = i;
					rinfo.endy = j;
					success += 1;
					if (success == 2) break;
				}
			}
		}
	}
};


#endif // !PROJECT_WAREHOUSE_H
