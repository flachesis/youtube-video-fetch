
#include "YoutubeCrawler.h"
#include "bdb/addr_iter.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <cstdlib>
#include <unistd.h>

bool init();
void uninit();
size_t chunk_size_est(unsigned int dir, size_t min_size);
bool capacity_test(size_t chunk_size, size_t data_size);

int main(int argc, char **argv){
	init();
	std::set<std::string> vids;
	int threadNum = 10;
	char *vidsRaw = new char[1200010];
	char *fileName = new char[4096];
	char *workingDir = new char[4096];
	char *logFile = new char[4096];
	vidsRaw[0] = '\0';
	fileName[0] = '\0';
	workingDir[0] = '\0';
	strcpy(logFile, "rids.log");
	int ch;
	opterr = 0;
	while((ch = getopt(argc, argv, "k:f:s:t:l:")) != -1){
		switch (ch){
			case 'k':
				strcpy(vidsRaw, optarg);
			break;
			case 'f':
				strcpy(fileName, optarg);
			break;
			case 's':
				strcpy(workingDir, optarg);
			break;
			case 't':
				threadNum = atoi(optarg);
			break;
			case 'l':
				strcpy(logFile, optarg);
			break;
			default:
			{
				std::cerr << "-k \"xxxxxxxxxxx,yyyyyyyyyyy...\"" << 
				" OR -f \"file name. each video id should with delimiter '\\n'\"" << std::endl;
				return 0;
			}
			break;
		}
	}
	if(threadNum <= 0){
		threadNum = 10;
	}
	size_t cstringLen = 0;
	if((cstringLen = strlen(workingDir)) > 0){
		if(workingDir[cstringLen - 1] != '/'){
			workingDir[cstringLen] = '/';
			workingDir[cstringLen + 1] = '\0';
		}
	}else{
		std::cerr << "need bdb dir [-s]." << std::endl;
		return 0;
	}
	char *pch = strtok(vidsRaw, ",");
	while(pch != NULL){
		if(strlen(pch) == 11){
			vids.insert(pch);
		}else{
			std::cerr << pch << " is not valid video id." << std::endl;
		}
		pch = strtok(NULL, ",");
	}
	if(strlen(fileName) > 0){
		FILE *fin = fopen(fileName, "r");
		if(fin != NULL){
			while(fgets(vidsRaw, 1200010, fin) != NULL){
				pch = strtok(vidsRaw, "\r\n");
				if(strlen(pch) == 11){
					vids.insert(pch);
				}else{
					std::cerr << pch << " is not valid video id." << std::endl;
				}
			}
		}
	}
	delete [] vidsRaw;
	delete [] fileName;
	std::set<std::string> vidsTmp;
	int count = 0;
	std::set<std::string> failGetVids;
	std::set<std::string>::iterator it;
	BDB::Config conf;
	conf.root_dir = workingDir;
	conf.min_size = 1024 * 1024;
	conf.log_dir = NULL;
	conf.addr_prefix_len = 5;
	//conf.ct_func = &capacity_test;
	conf.cse_func = &chunk_size_est;
	BDB::BehaviorDB ybdb(conf);
	{
		std::set<BDB::AddrType> inDBAddrList;
		std::ifstream fin(logFile, std::ifstream::in);
		if(fin.is_open()){
			std::string tmpVarString;
			std::istringstream iss(std::istringstream::in);
			std::string vid = "";
			BDB::AddrType addr = 0;
			long long int recSize;
			std::string ext = "";
			while(!fin.eof()){
				fin >> std::setw(11) >> vid;
				fin >> std::setw(9) >> tmpVarString;
				iss.str(tmpVarString);
				iss >> std::hex >> addr;
				iss.clear();
				tmpVarString.clear();
				fin >> std::setw(9) >> tmpVarString;
				iss.str(tmpVarString);
				iss >> std::hex >> recSize;
				iss.clear();
				tmpVarString.clear();
				fin >> ext;
				if((it = vids.find(vid)) != vids.end()){
					vids.erase(it);
				}
				inDBAddrList.insert(addr);
			}
			fin.close();
		}
		BDB::AddrIterator begin = ybdb.begin();
		while(begin != ybdb.end()){
			if(inDBAddrList.find(*begin) == inDBAddrList.end()){
				ybdb.del(*begin);
			}
			++begin;
		}
	}
	std::ofstream fout(logFile, std::ofstream::out | std::ofstream::app);
	delete [] logFile;
	YoutubeCrawler yc(ybdb, fout);
	it = vids.begin();
	while(it != vids.end()){
		count = 0;
		for(; it != vids.end() && count < threadNum; it++){
			vidsTmp.insert(*it);
			count++;
		}
		vids.erase(vids.begin(), it);
		it = vids.begin();
		failGetVids = yc.processVideoFetch(vidsTmp);
		for(std::set<std::string>::iterator it = failGetVids.begin(); it != failGetVids.end(); it++){
			std::cerr << *it << " fetch fail." << std::endl;
		}
	}
	fout.close();
	//delete [] workingDir;
	uninit();
	return 0;
}

bool init(){
	CURLcode code = curl_global_init(CURL_GLOBAL_ALL);
	if(code != 0){
		return false;
	}
	return true;
}

void uninit(){
	curl_global_cleanup();
}

bool capacity_test(size_t chunk_size, size_t data_size){
	return chunk_size >= data_size;
}

size_t chunk_size_est(unsigned int dir, size_t min_size){
	if(dir > 1){
		size_t a = 1;
		size_t b = 2;
		size_t tmp;
		for(unsigned int i = 1; i < dir; i++){
			tmp = b;
			b = a + b;
			a = tmp;
		}
		return b * min_size;
	}else if(dir == 0){
		return 1 * min_size;
	}else{
		return 2 * min_size;
	}
}

