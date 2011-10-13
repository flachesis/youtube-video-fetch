
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>
#include <set>
#include <map>
#include <iostream>
#include <sstream>
#include <unistd.h>
#include <curl/curl.h>

#ifdef __MINGW32__ // MINGW
#define fopen(X,Y) fopen64(X,Y)
#define ftello(X) ftello64(X)
#define fseeko(X,Y,Z) fseeko64(X,Y,Z)
#endif

bool init();
void uninit();
std::set<std::string> processVideoFetch(std::string savePath, std::set<std::string> &vids);
bool getVideoURI(std::set<std::string> &vids, std::map<std::string, std::vector<std::string> > &result);
bool getMaxQualityURI(CURL *handle, std::string &content, std::vector<std::string> &results);
std::set<std::string> getVideoFile(std::string &savePath, std::map<std::string, std::string> &result);
std::string getExtensionName(std::string &url);
size_t writeStringCallback(char *ptr, size_t size, size_t nmemb, void *userdata);
size_t writeFileCallback(char *ptr, size_t size, size_t nmemb, void *userdata);

int main(int argc, char **argv){
	init();
	std::set<std::string> vids;
	int threadNum = 10;
	char *vidsRaw = new char[1200010];
	char *fileName = new char[4096];
	char *workingDir = new char[4096];
	vidsRaw[0] = '\0';
	fileName[0] = '\0';
	int ch;
	opterr = 0;
	while((ch = getopt(argc, argv, "k:f:s:t:")) != -1){
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
	std::set<std::string>::iterator it = vids.begin();
	while(it != vids.end()){
		count = 0;
		for(; it != vids.end() && count < threadNum; it++){
			vidsTmp.insert(*it);
			count++;
		}
		failGetVids = processVideoFetch(workingDir, vidsTmp);
		for(std::set<std::string>::iterator it = failGetVids.begin(); it != failGetVids.end(); it++){
			std::cerr << *it << " fetch fail." << std::endl;
		}
	}
	delete [] workingDir;
	uninit();
	return 0;
}

std::set<std::string> processVideoFetch(std::string savePath, std::set<std::string> &vids){
	std::set<std::string> failGetVids;
	std::map<std::string, std::vector<std::string> > result;
	if(!getVideoURI(vids, result)){
		return failGetVids;
	}
	std::map<std::string, std::string> processList;
	for(std::map<std::string, std::vector<std::string> >::iterator it = result.begin(); it != result.end(); it++){
		processList.insert(std::pair<std::string, std::string>(it->first, *(it->second.begin())));
		it->second.erase(it->second.begin());
	}
	std::set<std::string> unFinishVids = getVideoFile(savePath, processList);
	std::map<std::string, std::vector<std::string> >::iterator it2;
	while(unFinishVids.size() != 0){
		processList.clear();
		for(std::set<std::string>::iterator it = unFinishVids.begin(); it != unFinishVids.end(); it++){
			std::cerr << *it << "start retry..." << std::endl;
			it2 = result.find(*it);
			if(it2 != result.end() && it2->second.size() > 0){
				processList.insert(std::pair<std::string, std::string>(it2->first, *(it2->second.begin())));
				it2->second.erase(it2->second.begin());
			}else{
				failGetVids.insert(*it);
				remove(it->c_str());
			}
		}
		unFinishVids = getVideoFile(savePath, processList);
	}
	return failGetVids;
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

bool getVideoURI(std::set<std::string> &vids, std::map<std::string, std::vector<std::string> > &result){
	CURL *handle = curl_easy_init();
	if(handle == NULL){
		return false;
	}
	std::string uriPrefix = "http://www.youtube.com/watch?v=";
	std::string vidURI;
	std::string content;
	CURLcode code;
	curl_easy_setopt(handle, CURLOPT_WRITEDATA, &content);
	curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, writeStringCallback);
	curl_easy_setopt(handle, CURLOPT_FOLLOWLOCATION, 1);
	curl_easy_setopt(handle, CURLOPT_MAXREDIRS, 10);
	curl_easy_setopt(handle, CURLOPT_USERAGENT, "Mozilla/5.0 (compatible; MSIE 9.0; Windows NT 6.1; WOW64; Trident/5.0)");
	curl_easy_setopt(handle, CURLOPT_CONNECTTIMEOUT, 10);
	curl_easy_setopt(handle, CURLOPT_TIMEOUT, 60);
	std::vector<std::string> vURIList;
	for(std::set<std::string>::iterator it = vids.begin(); it != vids.end(); it++){
		vidURI = uriPrefix + *it;
		curl_easy_setopt(handle, CURLOPT_URL, vidURI.c_str());
		code = curl_easy_perform(handle);
		if(code == 0){
			if(getMaxQualityURI(handle, content, vURIList)){
				result.insert(std::pair<std::string, std::vector<std::string> >(*it, vURIList));
			}
			vURIList.clear();
		}
		content.clear();
	}
	curl_easy_cleanup(handle);
	return true;
}

std::set<std::string> getVideoFile(std::string &savePath, std::map<std::string, std::string> &result){
	std::set<std::string> unFinishVids;
	if(result.size() == 0){
		return unFinishVids;
	}
	CURL *handle = curl_easy_init();
	if(handle == NULL){
		return unFinishVids;
	}
	CURLM *mhandle = curl_multi_init();
	if(mhandle == NULL){
		return unFinishVids;
	}
	CURLcode code;
	curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, writeFileCallback);
	curl_easy_setopt(handle, CURLOPT_FOLLOWLOCATION, 1);
	curl_easy_setopt(handle, CURLOPT_MAXREDIRS, 10);
	curl_easy_setopt(handle, CURLOPT_USERAGENT, "Mozilla/5.0 (compatible; MSIE 9.0; Windows NT 6.1; WOW64; Trident/5.0)");
	curl_easy_setopt(handle, CURLOPT_CONNECTTIMEOUT, 10);
	curl_easy_setopt(handle, CURLOPT_NOSIGNAL, 1);
	curl_easy_setopt(handle, CURLOPT_LOW_SPEED_LIMIT, 1);
	curl_easy_setopt(handle, CURLOPT_LOW_SPEED_TIME, 60);
	std::map<std::string, CURL*> handleList;
	std::map<std::string, FILE*> fileList;
	FILE *tmpFile = NULL;
	std::string filePath;
	for(std::map<std::string, std::string>::iterator it = result.begin(); it != result.end(); it++){
		filePath = savePath + it->first + "." + getExtensionName(it->second);
		tmpFile = fopen(filePath.c_str(), "wb");
		fileList.insert(std::pair<std::string, FILE*>(it->first, tmpFile));
		curl_easy_setopt(handle, CURLOPT_WRITEDATA, tmpFile);
		curl_easy_setopt(handle, CURLOPT_URL, it->second.c_str());
		handleList.insert(std::pair<std::string, CURL*>(it->first, handle));
		curl_multi_add_handle(mhandle, handle);
		handle = curl_easy_duphandle(handle);
	}
	curl_easy_cleanup(handle);
	int runningHandles = 0;
	while(curl_multi_perform(mhandle, &runningHandles) == CURLM_CALL_MULTI_PERFORM){}
	struct timeval timeout;
	timeout.tv_sec = 0;
	timeout.tv_usec = 0;
	int max_fd = 0;
	fd_set read_fd_set;
	fd_set write_fd_set;
	fd_set exc_fd_set;
	FD_ZERO(&read_fd_set);
	FD_ZERO(&write_fd_set);
	FD_ZERO(&exc_fd_set);
	while(runningHandles > 0){
		curl_multi_fdset(mhandle, &read_fd_set, &write_fd_set, &exc_fd_set, &max_fd);
		curl_multi_timeout(mhandle, &(timeout.tv_usec));
		if(timeout.tv_usec > 0){
			select(max_fd + 1, &read_fd_set, &write_fd_set, &exc_fd_set, &timeout);
		}
		while(curl_multi_perform(mhandle, &runningHandles) == CURLM_CALL_MULTI_PERFORM){}
	}
	for(std::map<std::string, CURL*>::iterator it = handleList.begin(); it != handleList.end(); it++){
		curl_multi_remove_handle(mhandle, it->second);
		curl_easy_cleanup(it->second);
	}
	curl_multi_cleanup(mhandle);
	for(std::map<std::string, FILE*>::iterator it = fileList.begin(); it != fileList.end(); it++){
		if(ftello(it->second) == 0){
			unFinishVids.insert(it->first);
		}
		fclose(it->second);
	}
	return unFinishVids;
}

bool getMaxQualityURI(CURL *handle, std::string &content, std::vector<std::string> &results){
	size_t startPos = content.find("url_encoded_fmt_stream_map=");
	if(startPos == std::string::npos){
		return false;
	}
	startPos += 27;
	size_t endPos = content.find("&amp;", startPos);
	if(endPos == std::string::npos){
		endPos = content.find("\"", startPos);
		if(endPos == std::string::npos){
			return false;
		}
	}
	int cstringLen = 0;
	char *decodeContent = curl_easy_unescape(handle, content.c_str() + startPos, endPos - startPos, &cstringLen);
	content.assign(decodeContent, cstringLen);
	curl_free(decodeContent);
	startPos = content.find("url=");
	std::string tmpURI;
	while(startPos != std::string::npos){
		startPos += 4;
		endPos = content.find(",", startPos);
		if(endPos == std::string::npos){
			endPos = content.size();
		}
		decodeContent = curl_easy_unescape(handle, content.c_str() + startPos, endPos - startPos, &cstringLen);
		tmpURI.assign(decodeContent, cstringLen);
		tmpURI = tmpURI.substr(0, tmpURI.find(";"));
		curl_free(decodeContent);
		results.push_back(tmpURI);
		startPos = content.find("url=", endPos + 1);
	}
	if(results.size() == 0){
		return false;
	}
	return true;
}

std::string getExtensionName(std::string &url){
	size_t startPos = url.find("itag=");
	if(startPos == std::string::npos){
		return "xxx";
	}
	startPos += 5;
	std::istringstream iss(url.substr(startPos, url.find("&", startPos)), std::istringstream::in);
	int fmt = 0;
	iss >> fmt;
	switch(fmt){
		case 38:
		case 37:
		case 22:
		case 18:
			return "mp4";
		break;
		case 45:
		case 44:
		case 43:
			return "WebM";
		break;
		case 35:
		case 34:
		case 5:
			return "flv";
		break;
		default:
			return "xxx";
		break;
	}
	return "xxx";
}

size_t writeStringCallback(char *ptr, size_t size, size_t nmemb, void *userdata){
	size = size * nmemb;
	std::string *data = (std::string*)userdata;
	data->append(ptr, size);
	return size;
}

size_t writeFileCallback(char *ptr, size_t size, size_t nmemb, void *userdata){
	size = size * nmemb;
	FILE *file = (FILE*)userdata;
	fwrite(ptr, 1, size, file);
	return size;
}
