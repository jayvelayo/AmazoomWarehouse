


#include <cpen333/process/shared_memory.h>
#include <cpen333/process/mutex.h>
#include <cpen333/console.h>
#include <cstdio>
#include <thread>
#include <chrono>

#include "WarehouseCommon.h"

/**
* Handles all drawing/memory synchronization for the
* User Interface process
* ====================================================
*  TODO: ADD ANY NECESSARY MUTUAL EXCLUSION
* ====================================================
*
*/
class WarehouseUI {
	// display offset for better visibility
	static const int XOFF = 2;
	static const int YOFF = 1;

	cpen333::console display_;
	cpen333::process::shared_object<SharedData> memory_;
	cpen333::process::mutex mutex_;



	// previous positions of robots
	int lastpos_[MAX_ROBOTS][2];
	int exit_[2];   // exit location

public:

	WarehouseUI() : display_(), memory_(WAREHOUSE_MEMORY_NAME), mutex_(WAREHOUSE_MUTEX_NAME) {

		// clear display and hide cursor
		display_.clear_all();
		display_.set_cursor_visible(false);

		// initialize last known runner positions
		for (size_t i = 0; i<MAX_ROBOTS; ++i) {
			lastpos_[i][COL_IDX] = -1;
			lastpos_[i][ROW_IDX] = -1;
		}

		/*
		// initialize exit location
		exit_[COL_IDX] = -1;
		exit_[ROW_IDX] = -1;

		//===========================================================
		// TODO: SEARCH MAZE FOR EXIT LOCATION
		//===========================================================
		std::lock_guard<cpen333::process::mutex> processLock(mutex_);
		for (int i = 0; i <= memory_->winfo.rows; i++) {
			for (int j = 0; j <= memory_->winfo.cols; j++) {
				//	std::cout << i << " " << j << std::endl;
				if (memory_->winfo.maze[j][i] == 'E') {
					exit_[COL_IDX] = j;
					exit_[ROW_IDX] = i;
					break;
				}
			}
		}
		*/
	}

	bool initializedMemory() {
		std::lock_guard<decltype(mutex_)> lock(mutex_);
		if (memory_->magic == MAGIC_NUMBER) return true;
		else return false;
	}

	/**
	* Draws the maze itself
	*/
	void draw_maze() {
		static const char WALL = 'x';  // WALL character
		static const char EXIT = 'e';  // EXIT character

		WarehouseInfo& winfo = memory_->winfo;
		RobotInfo& rinfo = memory_->rinfo;

		// clear display
		display_.clear_display();

		// draw maze
		int ndocks = 0;
		for (int r = 0; r < winfo.rows; ++r) {
			display_.set_cursor_position(YOFF + r, XOFF);
			for (int c = 0; c < winfo.cols; ++c) {
				char ch = winfo.maze[c][r];
				if (ch == WALL_CHAR) {
					std::printf("%c", WALL);
				}
				else if (ch == SHELF_CHAR) {
					std::printf("%c", SHELF_CHAR);
				}
				else if (ch == DOCK_CHAR) {
					std::printf("%d", ndocks);
					ndocks++;
				}
				else {
					std::printf("%c", EMPTY_CHAR);
				}
			}
		}
	}

	/**
	* Draws all robots in the maze
	*/
	void draw_robots() {

		RobotInfo& rinfo = memory_->rinfo;


		std::lock_guard<cpen333::process::mutex> memoryLock(mutex_);
		// draw all robot locations
		for (size_t i = 0; i<rinfo.nrobots; ++i) {
			char me = 'A' + i;
			int newr = rinfo.rloc[i][ROW_IDX];
			int newc = rinfo.rloc[i][COL_IDX];

			if (newc != lastpos_[i][COL_IDX]
				|| newr != lastpos_[i][ROW_IDX]) {

				// zero out last spot and update known location
				display_.set_cursor_position(YOFF + lastpos_[i][ROW_IDX], XOFF + lastpos_[i][COL_IDX]);
				std::printf("%c", EMPTY_CHAR);
				lastpos_[i][COL_IDX] = newc;
				lastpos_[i][ROW_IDX] = newr;
			}

			// print runner at new location
			display_.set_cursor_position(YOFF + newr, XOFF + newc);
			std::printf("%c", me);		
		}
	}

	/**
	* Checks if we are supposed to quit
	* @return true if memory tells us to quit
	*/
	bool quit() {
		// check if we need to quit
		return memory_->quit;
	}

	~WarehouseUI() {
		// reset console settings
		display_.clear_all();
		display_.reset();
	}
};

int main() {

	// initialize previous locations of characters
	WarehouseUI ui;
	if (ui.initializedMemory()) {
		ui.draw_maze();

		// continue looping until main program has quit
		while (!ui.quit()) {
			ui.draw_robots();
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}
	}
	else {
		std::cout << "Error: Memory not yet initialized. Please run maze_runner_main first." << std::endl;
		std::cin.get();
	}

	return 0;
}