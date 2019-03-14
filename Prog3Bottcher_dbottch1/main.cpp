#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <stdlib.h>
#include "helper.h"

using namespace std;

int main() {
	Helper h;
	h.generate_no_locality();
		//printf("(LRU cache size %d): %d / %d\n", i, h.LRU(h.getNL_R(), i), h.getNumAccesses());
		//printf("(rand cache size %d): %d / %d\n", i, h.Rand(h.getNL_R(), i), h.getNumAccesses());
		//printf("(FIFO cache size %d): %d / %d\n", i, h.FIFO(h.getNL_R(), i), h.getNumAccesses());
	ofstream o("nolocal_data.csv");
	o << 0 << ", " << 0 << ", " << 0 << ", " << 0 << ", " << 0 << ", " << 0 << ", " << 0 << endl;
	for(int i = 5; i <= 100; i += 5) {
		h.outputCSV(o, i, h.OPT(h.getNL_R(), i), h.LRU(h.getNL_R(), i), h.FIFO(h.getNL_R(), i), h.Rand(h.getNL_R(), i), h.Clock(h.getNL_R(), i));
	}
	o.close();
	cout << "no locality: printed to CSV\n";
	h.generate_e_t();
	ofstream p("et_data.csv");
	//if(!p) {cout << "ye\n"; exit(1);}
	for(int i = 5; i <= 100; i += 5) {
		h.outputCSV(p, i, h.OPT(h.getET_R(), i), h.LRU(h.getET_R(), i), h.FIFO(h.getET_R(), i), h.Rand(h.getET_R(), i), h.Clock(h.getET_R(), i));
	}
	p.close();
	cout << "80/20: printed to CSV\n";
	/*for(int i = 5; i <= 100; i += 5) {
		printf("80/20 (LRU cache size %d): %d / %d\n", i, h.LRU(h.getET_R(), i), h.getNumAccesses());
		printf("80/20 (rand cache size %d): %d / %d\n", i, h.Rand(h.getET_R(), i), h.getNumAccesses());
	}*/
	h.generate_looping();
	ofstream q("loop_data.csv");
	for(int i = 5; i <= 100; i += 5) {
		h.outputCSV(q, i, h.OPT(h.getLP_R(), i), h.LRU(h.getLP_R(), i), h.FIFO(h.getLP_R(), i), h.Rand(h.getLP_R(), i), h.Clock(h.getLP_R(), i));
	}
	q.close();
	cout << "looping: printed to CSV\n";
	/*for(int i = 5; i <= 100; i += 5) {
		printf("looping (LRU cache size %d): %d / %d\n", i, h.LRU(h.getLP_R(), i), h.getNumAccesses());
		printf("looping (rand cache size %d): %d / %d\n", i, h.Rand(h.getLP_R(), i), h.getNumAccesses());
	}*/
	//cout << h;
	//cache, opt, lru, fifo, rand
	//h.outputCSV(5, 0, 0, 0);
}
