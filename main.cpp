/* 
 * File:   main.cpp
 * Author: flachesis
 *
 * Created on 2011年4月16日, 下午 4:10
 */

#include "videoFetch.h"
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <memory>
#include <cstring>
#include <cstdlib>
#include <Poco/Thread.h>
#include <unistd.h>

/*
 * 
 */


int main(int argc, char** argv) {
    int threadNum = 10;
    std::set<std::string> *vidSet = new std::set<std::string>;
    int ch;
    opterr = 0;
    char recordInfo[1200010];
    char workingDir[1024];
    strcpy(workingDir, "./");
    bool fileMode = false;
    while((ch = getopt(argc, argv, "k:f:s:")) != -1){
        switch (ch){
            case 'k':
                strcpy(recordInfo, optarg);
                break;
            case 'f':
                fileMode = true;
                strcpy(recordInfo, optarg);
                break;
            case 's':
		strcpy(workingDir, optarg);
		break;
            default:
		std::cout << "-k \"xxxxxxxxxxx,yyyyyyyyyyy...\" OR -f \"file name. each video id should with delimiter '\\n'\"" << std::endl;
		return 0;
                break;
        }
    }
    if(!fileMode){
        char *pch = strtok(recordInfo, ",");
        while(pch != NULL){
            if(strlen(pch) == 11){
                vidSet->insert(pch);
            }else{
                std::cout << pch << " is not valid video id." << std::endl;
            }
            pch = strtok(NULL, ",");
        }
    }else{
        std::ifstream fin;
        fin.open(recordInfo, std::ios_base::in);
        if(!fin.is_open()){
            std::cout << "file: " << recordInfo << " open error!" << std::endl;
            return 0;
        }
        while(!fin.eof()){
            fin.getline(recordInfo, 1200010);
            if(strlen(recordInfo) == 11){
                vidSet->insert(recordInfo);
            }else{
                std::cout << recordInfo << " is not valid video id." << std::endl;
            }
        }
    }
    if(vidSet->size() == 0){
        std::cout << "no video id found." << std::endl;
        return 0;
    }
    if(chdir(workingDir) != 0){
	std::cout << "changing working dir" << optarg << " fail." << std::endl;
	return 0;
    }
    unsigned int perThreadHandleVids = vidSet->size() / threadNum;
    if(perThreadHandleVids == 0){
        std::map<Poco::Thread*, videoFetch*> threadList;
        Poco::Thread *threadHandle;
        videoFetch *fetch;
        for(std::set<std::string>::iterator it = vidSet->begin(); it != vidSet->end();){
            threadHandle = new Poco::Thread;
            std::set<std::string>::iterator it2 = it;
            it2++;
            fetch = new videoFetch(vidSet,it, it2);
            threadList.insert(std::pair<Poco::Thread*, videoFetch*>(threadHandle, fetch));
            threadHandle->start(*fetch);
            it++;
        }
        for(std::map<Poco::Thread*, videoFetch*>::iterator it = threadList.begin(); it != threadList.end(); it++){
            it->first->join();
            delete it->first;
            delete it->second;
        }
    }else{
        std::map<Poco::Thread*, videoFetch*> threadList;
        Poco::Thread *threadHandle;
        videoFetch *fetch;
        for(std::set<std::string>::iterator it = vidSet->begin(); it != vidSet->end();){
            threadHandle = new Poco::Thread;
            std::set<std::string>::iterator it2;
            unsigned int count = 0;
            for(it2 = it; it2 != vidSet->end() && count < perThreadHandleVids;){
                it2++;
                count++;
            }
            fetch = new videoFetch(vidSet,it, it2);
            threadList.insert(std::pair<Poco::Thread*, videoFetch*>(threadHandle, fetch));
            threadHandle->start(*fetch);
            it = it2;
        }
        for(std::map<Poco::Thread*, videoFetch*>::iterator it = threadList.begin(); it != threadList.end(); it++){
            it->first->join();
            delete it->first;
            delete it->second;
        }
    }
    return 0;
}
