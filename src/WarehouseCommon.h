#ifndef PROJECT_WAREHOUSE_COMMON_H
#define PROJECT_WAREHOUSE_COMMON_H

#include "WarehouseObjects.h"
#include "Shelf.h"

#define WAREHOUSE_MEMORY_NAME "Amazoom_Warehouse"
#define WAREHOUSE_MUTEX_NAME "Amazoom_Warehouse_mutex"

#define WALL_CHAR 'X'
#define SHELF_CHAR 'S'
#define EMPTY_CHAR ' '
#define START_CHAR 'B'
#define EXIT_CHAR 'E'
#define ROBOT_PATH_CHAR 'Z'
#define DOCK_CHAR 'L'

#define COL_IDX 0
#define ROW_IDX 1

#define MAX_WAREHOUSE_SIZE 80
#define MAX_ROBOTS   50
#define MAX_SHELF_CAPACITY 200
#define MAX_DOCK_CAPACITY 9
#define MAX_ROBOT_CAPACITY 100

#define MAGIC_NUMBER 1234

struct DockInfo {
	int ndocks;
	int dloc[MAX_DOCK_CAPACITY][2];
};

struct WarehouseInfo {
	int rows;           // rows in warehouse
	int cols;           // columns in warehouse
	char maze[MAX_WAREHOUSE_SIZE][MAX_WAREHOUSE_SIZE];  // warehouse storage layout
	DockInfo docks;
	//cols, rows
};



struct RobotInfo {
	int nrobots;      // number robots
	int startx;		  // robot row start location
	int starty;		  // robot column start location
	int endx;
	int endy;
	int rloc[MAX_ROBOTS][2];
};

struct Shelfcoord {
	int row;
	int col;
};

struct SharedData {
	WarehouseInfo winfo;    // warehouse info
	RobotInfo rinfo;  // robot info
	Shelfcoord shelves[MAX_SHELF_CAPACITY];
	bool quit;         // tell everyone to quit
	int magic;
};

#endif //PROJECT_WAREHOUSE_COMMON_H