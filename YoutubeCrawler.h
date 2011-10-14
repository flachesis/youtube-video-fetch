
#ifndef YOUTUBE_CRAWLER
#define YOUTUBE_CRAWLER

#include <string>
#include <vector>
#include <set>
#include <map>
#include <fstream>
#include <curl/curl.h>
#include "bdb/bdb.hpp"

class YoutubeCrawler{
	public:
		YoutubeCrawler(BDB::BehaviorDB &bdb, std::ofstream &logHandle);
		virtual ~YoutubeCrawler();
		std::set<std::string> processVideoFetch(std::set<std::string> &vids);
	private:
		BDB::BehaviorDB &ybdb;
		std::ofstream &logRids;
		
		struct RecInfo{
			BDB::BehaviorDB *ybdb;
			BDB::AddrType addr;
			size_t size;
		};
		
		bool getVideoURI(std::set<std::string> &vids, std::map<std::string, std::vector<std::string> > &result);
		bool getMaxQualityURI(CURL *handle, std::string &content, std::vector<std::string> &results);
		std::set<std::string> getVideoFile(std::map<std::string, std::string> &result);
		std::string getExtensionName(std::string &url);
		static size_t writeStringCallback(char *ptr, size_t size, size_t nmemb, void *userdata);
		static size_t writeFileCallback(char *ptr, size_t size, size_t nmemb, void *userdata);
};

#endif
