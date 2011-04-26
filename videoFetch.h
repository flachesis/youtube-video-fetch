/* 
 * File:   videoFetch.h
 * Author: flachesis
 *
 * Created on 2011年4月17日, 上午 12:59
 */

#ifndef VIDEOFETCH_H
#define	VIDEOFETCH_H

#include <set>
#include <istream>
#include <iostream>
#include "Poco/Thread.h"
#include "Poco/Runnable.h"
#include "Poco/Net/HTTPClientSession.h"
#include "Poco/Net/HTTPRequest.h"
#include "Poco/Net/HTTPResponse.h"
#include "Poco/Net/HTMLForm.h"
#include "Poco/StreamCopier.h"
#include "Poco/Exception.h"
#include "htmlParser.h"
#include "curl/curl.h"

class videoFetch : public Poco::Runnable {
public:
    videoFetch(std::set<std::string> *vidSet, std::set<std::string>::iterator start, std::set<std::string>::iterator end);
    videoFetch(const videoFetch& orig);
    virtual ~videoFetch();
    virtual void run();
private:
    std::set<std::string> *vidSet;
    std::set<std::string>::iterator start;
    std::set<std::string>::iterator end;
    Poco::Net::HTTPClientSession *fetchHtmlHcs;

    std::string* getHtmlContent(const std::string &vid) throw (Poco::Exception);
    size_t getVideo(const std::string &url, std::ostream &outStream) throw (Poco::Exception);
    int getVideoUsingCurl(const std::string &url, std::ostream &outStream);
    static size_t curlWriteCallBack( void *ptr, size_t size, size_t nmemb, void *userdata);
};

#endif	/* VIDEOFETCH_H */

