/* 
 * File:   htmlParser.h
 * Author: flachesis
 *
 * Created on 2011年4月17日, 上午 12:19
 */

#ifndef HTMLPARSER_H
#define	HTMLPARSER_H

#include <string>
#include <fstream>
#include <sstream>
#include <map>
#include <vector>
#include <memory>
#include "Poco/URI.h"
#include "Poco/StringTokenizer.h"
#include "Poco/NumberParser.h"

class htmlParser {
public:
    htmlParser();
    htmlParser(const htmlParser& orig);
    virtual ~htmlParser();
    static std::vector<std::pair<std::string, std::string> >* getDownloadUrlExtension(const std::string &htmlContent);
private:
    static std::map<int, std::string>* parser(const std::string &htmlContent);
    static std::vector<std::pair<int, std::string> >* highestQuality(std::map<int, std::string>* result);
    static std::string getExtensionName(int fmt);
};

#endif	/* HTMLPARSER_H */

