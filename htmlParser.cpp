/* 
 * File:   htmlParser.cpp
 * Author: flachesis
 * 
 * Created on 2011年4月17日, 上午 12:19
 */

#include "htmlParser.h"

htmlParser::htmlParser() {
}

htmlParser::htmlParser(const htmlParser& orig) {
}

htmlParser::~htmlParser() {
}

std::vector<std::pair<std::string, std::string> >* htmlParser::getDownloadUrlExtension(const std::string &htmlContent){
    std::auto_ptr<std::map<int, std::string> > result(parser(htmlContent));
    if(result.get() == NULL){
        return NULL;
    }
    std::vector<std::pair<std::string, std::string> > *urlExtensionVector = new std::vector<std::pair<std::string, std::string> >;
    std::auto_ptr<std::vector<std::pair<int, std::string> > > downLoadList(highestQuality(result.get()));
    std::string extensionName;
    for(std::vector<std::pair<int, std::string> >::iterator it = downLoadList->begin(); it != downLoadList->end(); it++){
        extensionName = getExtensionName(it->first);
        urlExtensionVector->push_back(std::pair<std::string, std::string>(it->second, extensionName));
    }
    return urlExtensionVector;
}

std::vector<std::pair<int, std::string> >* htmlParser::highestQuality(std::map<int, std::string>* result) {
    std::vector<std::pair<int, std::string> > *downLoadList = new std::vector<std::pair<int, std::string> >;
    std::map<int, std::string>::iterator it;
    if ((it = result->find(38)) != result->end()) {
        downLoadList->push_back(std::pair<int, std::string>(it->first, it->second));
    }
    if ((it = result->find(37)) != result->end()) {
        downLoadList->push_back(std::pair<int, std::string>(it->first, it->second));
    }
    if ((it = result->find(22)) != result->end()) {
        downLoadList->push_back(std::pair<int, std::string>(it->first, it->second));
    }
    if ((it = result->find(35)) != result->end()) {
        downLoadList->push_back(std::pair<int, std::string>(it->first, it->second));
    }
    if ((it = result->find(34)) != result->end()) {
        downLoadList->push_back(std::pair<int, std::string>(it->first, it->second));
    }
    if ((it = result->find(45)) != result->end()) {
        downLoadList->push_back(std::pair<int, std::string>(it->first, it->second));
    }
    if ((it = result->find(43)) != result->end()) {
        downLoadList->push_back(std::pair<int, std::string>(it->first, it->second));
    }
    if ((it = result->find(18)) != result->end()) {
        downLoadList->push_back(std::pair<int, std::string>(it->first, it->second));
    }
    if ((it = result->find(5)) != result->end()) {
        downLoadList->push_back(std::pair<int, std::string>(it->first, it->second));
    }
    if ((it = result->find(17)) != result->end()) {
        downLoadList->push_back(std::pair<int, std::string>(it->first, it->second));
    }
    return downLoadList;
}

std::string htmlParser::getExtensionName(int fmt) {
    std::string extensionName;
    switch (fmt) {
        case 17:
            extensionName = "3gp";
            break;
        case 45:
        case 43:
            extensionName = "webm";
            break;
        case 38:
        case 37:
        case 22:
        case 18:
            extensionName = "mp4";
            break;
        case 35:
        case 34:
        case 5:
            extensionName = "flv";
            break;
        default:
            extensionName = "unknown";
            break;
    }
    return extensionName;
}

std::map<int, std::string>* htmlParser::parser(const std::string &htmlContent) {
    size_t begpos = htmlContent.find("application/x-shockwave-flash");
    if (begpos != std::string::npos) {
        begpos += 29;
        begpos = htmlContent.find("fmt_url_map=", begpos);
        if (begpos != std::string::npos) {
            begpos += 12;
            size_t endPos = htmlContent.find("&amp;", begpos);
            if (endPos != std::string::npos) {
                std::string urlList = htmlContent.substr(begpos, endPos - begpos);
                std::string decodeURLContent;
                Poco::URI::decode(urlList, decodeURLContent);
                Poco::StringTokenizer st1(decodeURLContent, ",", Poco::StringTokenizer::TOK_TRIM | Poco::StringTokenizer::TOK_IGNORE_EMPTY);
                std::map<int, std::string> *result = new std::map<int, std::string>;
                for (Poco::StringTokenizer::Iterator it = st1.begin(); it != st1.end(); it++) {
                    Poco::StringTokenizer st2(*it, "|", Poco::StringTokenizer::TOK_TRIM | Poco::StringTokenizer::TOK_IGNORE_EMPTY);
                    int fmt = Poco::NumberParser::parse(*(st2.begin()));
                    result->insert(std::pair<int, std::string > (fmt, *(st2.begin() + 1)));
                }
                return result;
            }
        }
    }
    return NULL;
}
