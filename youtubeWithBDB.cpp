
#include "YoutubeCrawler.h"
#include "bdb/addr_iter.hpp"
#include <iostream>
#include <fstream>
#include <iomanip>
#include <cstdlib>
#include <unistd.h>

bool init();
void uninit();

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
	conf.min_size = 26214400;
	BDB::BehaviorDB ybdb(conf);
	{
		std::set<BDB::AddrType> inDBAddrList;
		std::ifstream fin(logFile, std::ifstream::in);
		if(fin.is_open()){
			std::string vid = "";
			BDB::AddrType addr = 0;
			std::string ext = "";
			while(!fin.eof()){
				fin >> std::setw(11) >> vid >> std::setw(9) >> std::hex >> addr >> ext;
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