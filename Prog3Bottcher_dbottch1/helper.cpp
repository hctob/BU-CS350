#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <list>
#include <unordered_map>
#include <map>
#include <stdlib.h>
#include "helper.h"

using namespace std;

Helper::Helper() {
	this->pages = 100;
	this->accesses = 10000;
	cout << "Pages: " << this->pages << "\nAccesses: " << this->accesses << endl;
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
	/*for(unsigned int i : h.getNL_R()) {
		os << "Random page: " << std::dec << i << "\n";
	}
	for(unsigned int k : h.getET_R()) {
		os << "80_20 pages: " << std::dec << k << "\n";
	}*/
	/*for(unsigned int k : h.getLP_R()) {
		os << "Looping pages: " << std::dec << k << "\n";
	}*/
	os << std::dec;
	return os;
}

void Helper::generate_no_locality() {
	srand(time(NULL));
	for(unsigned int i = 0; i < this->accesses; i++) {
		unsigned int random_page = (rand() % this->pages) + 1;
		//cout << "Page: " << random_page << "\n";
		no_locality_res.push_back(random_page);
	}
}

void Helper::generate_e_t() {
	int i;
	srand(time(NULL));
	cout << "Generating 80\% of accesses(hot pages)\n";
	for(i = 0; i < this->accesses * 0.8; i++) {
		unsigned int num = (rand() % this->pages * .2) + 1;
		//cout << num << ": HP, ";
		e_t_res.push_back(num);
	}
	//cout << "Size of hot pages: " << e_t_res.size() << endl;
	//cout << "Generating Remaining 20% (cold pages)\n";
	for( ;i < this->accesses; i++) {
		unsigned int num  = (rand() % (int)(this->pages - 20) + (int)(20)) + 1;
		//cout << num << ": CP, ";
		e_t_res.push_back(num);
	}
	cout << "Size of cold pages: " << e_t_res.size() << endl;
	random_shuffle(e_t_res.begin(), e_t_res.end());
}



unsigned int Helper::getNumPages() {
	return this->pages;
}

unsigned int Helper::getNumAccesses() {
	return this->accesses;
}

void Helper::generate_looping() {
	unsigned int page = 1;
	for(int i = 0; i < this->accesses; i++) {
		if(page == this->pages / 2 + 1) page = 1;
		loop_res.push_back(page);
		page++;
	}
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

void Helper::outputCSV(ostream& o, int cache_size, int opt, int lru, int fifo, int rand, int clock) {
		o << cache_size << "," << (float)100 * opt / (float)this->accesses << "," << (float)100 * lru / (float)this->accesses << "," << (float)100 * fifo / (float)this->accesses << "," << (float)100 * rand / (float)this->accesses << ", " << (float)100 * clock / (float)this->accesses << endl;
}

unsigned int Helper::FIFO(vector<unsigned int> workload, int cache_size) {
	unsigned int success = 0;
	vector<unsigned int> cache;
	//srand(time(NULL));
	for(int i = 0; i < (this->accesses); i++){
		int x = workload.at(i);
		//not present in cache
		if((find(cache.begin(), cache.end(), x)) == cache.end()){

			//cache is full
			if(cache.size() == cache_size){
				cache.erase(cache.begin()); //first in first out
				cache.push_back(x);
			}

			//cache is not full
			else{
				cache.push_back(x);
			}
		}

		//present in cache
		else{
			success++;
		}
	}
	
	return success;

}

//Matt
unsigned int Helper::LRU(vector<unsigned int> workload, int cache_size){
	unsigned int success = 0;
	list<int> cache;
	unordered_map<int, list<int>::iterator> ma;

	for(int i = 0; i< (this->accesses); i++){
		int x = workload.at(i);
		//not present in cache
		if(ma.find(x) == ma.end()){

			//cache is full
			if(cache.size() == cache_size){
				int last = cache.back();
				cache.pop_back();
				ma.erase(last);
			}
		}

		//present in cache
		else{
			cache.erase(ma[x]);
			success++;
		}

		//update reference
		cache.push_front(x);
		ma[x] = cache.begin();
	}

	return success;

}

unsigned int Helper::Rand(vector<unsigned int> workload, int cache_size){
	unsigned int success = 0;
	vector<unsigned int> cache;
	srand(time(NULL));
	for(int i = 0; i< (this->accesses); i++){
		int x = workload.at(i);
		//not present in cache
		if((find(cache.begin(), cache.end(), x)) == cache.end()){

			//cache is full
			if(cache.size() == cache_size){
				unsigned int index = (rand() % (cache_size-1));
				cache.erase(cache.begin() + index);
				cache.push_back(x);
			}

			//cache is not full
			else{
				cache.push_back(x);
			}
		}

		//present in cache
		else{
			success++;
		}
	}
	
	return success;

}

unsigned int Helper::OPT(vector<unsigned int> workload, int cache_size){
	unsigned int success = 0;
	vector<unsigned int> cache;
	//vector<unsigned int> future = cache;
	for(int i = 0; i< workload.size(); i++){
		unsigned int x = workload.at(i);
		//not present in cache
		if((find(cache.begin(), cache.end(), x)) == cache.end()){

			//cache is full
			vector<unsigned int> future = cache;
			if(cache.size() == cache_size){
				
				//check if last element in workload
				if(i + 1 != workload.size()) {
					
				
					//cache is full, so loop through each element
					for(int j = i; j < workload.size(); j++){
						unsigned int y = workload.at(j);

						//this entry in the workload will be used in the future; not last one
						if(find(future.begin(), future.end(), y) != future.end()){
							future.erase(remove(future.begin(), future.end(), y), future.end());
						}

						//future has been whittled down to last entry; furthest page out
						if(future.size() == 1) break;
					}

					cache.erase(remove(cache.begin(), cache.end(), future.at(0)), cache.end());
					cache.push_back(x);
				}
			}

			//cache is not full
			else{
				cache.push_back(x);
			}
		}

		//present in cache
		else{
			success++;
		}
	}
	
	return success;
}

unsigned int Helper::Clock(vector<unsigned int> workload, int cache_size){
	unsigned int success = 0;
/*
	//calculate size of clock
	int clock_size = 0;
	for(int i = 0; i < workload.size(); i++){
		if(workload.at(i) > clock_size) clock_size = workload.at(1);
	}
	//construct and populate clock with pages 1-100 OR 1-50
	map<unsigned int, bool> clock;
	for(int i = 1; i < clock_size + 1; i++){
		clock.insert(pair<unsigned int, bool> (i, false));
	}
*/

	map<unsigned int, bool> clock;
	int clock_size = 0;
	int hand = 0;
	vector<unsigned int> cache;
	for(int i = 0; i< (this->accesses); i++){
		int x = workload.at(i);
		//not present in cache
		if((find(cache.begin(), cache.end(), x)) == cache.end()){

			//cache is full
			if(cache.size() == cache_size){
				//int hand = 0;

				//find first page in clock with use bit == 0
				map<unsigned int, bool>::iterator it = clock.find(cache.at(hand));
				if(it != clock.end()){
					while(it->second == true){
						it->second = false;
						hand++;
						if(hand == clock_size) hand = 0;
						it = clock.find(cache.at(hand));
					}
				}
				clock.erase(cache.at(hand));
				clock.insert(pair<unsigned int, bool> (x, false));
				cache.erase(cache.begin() + hand);
				cache.push_back(x);
				it = clock.find(x);
				if(it != clock.end()){
					it->second = true;
				}
			}

			//cache is not full
			else{
				cache.push_back(x);
				clock.insert(pair<unsigned int, bool> (x, false));
				clock_size++;
			}
		}

		//present in cache
		else{
			success++;
			map<unsigned int, bool>::iterator it = clock.find(x);
			if(it != clock.end()){
				it->second = true;
			}
		}
	}
	
	return success;

}



