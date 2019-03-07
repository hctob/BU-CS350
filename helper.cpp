#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <stdlib.h>
#include "helper.h"

using namespace std;

Helper::Helper() {
	this->pages = 100;
	this->accesses = 10000;
}

Helper::Helper(unsigned int pages, unsigned int accesses) {
	this->pages = pages;
	this->accesses = accesses;
}

Helper::~Helper() {
	no_locality_res.clear();
	e_t_res.clear();
	loop_res.clear();
}

ostream& operator<<(ostream& os, Helper h) {
	os << "Pages: " << h.getNumPages() << "\nAccesses: " << h.getNumAccesses() << endl;
	for(unsigned int i : h.getNL_R()) {
		os << "Random page: " << std::hex << i << "\n";
	}
	os << std::dec;
	return os;
}

void Helper::generate_no_locality() {
	srand(time(NULL));
	for(unsigned int i = 0; i < this->accesses; i++) {
		unsigned int random_page = (rand() % this->pages) + 1;
		cout << "Page: " << random_page << "\n";
		no_locality_res.push_back(random_page);
	}
}

void Helper::generate_e_t() {
}

unsigned int Helper::getNumPages() {
	return this->pages;
}

unsigned int Helper::getNumAccesses() {
	return this->accesses;
}

void Helper::generate_e_t() {
	
}

void Helper::generate_looping() {
	
}

vector<unsigned int> Helper::getNL_R() {
	return this->no_locality_res;
}

vector<unsigned int> Helper::getET_R() {
	return this->e_t_res;
}

vector<unsigned int> Helper::getLP_R() {
	return this->loop_res;
}

