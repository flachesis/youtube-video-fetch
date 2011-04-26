/* 
 * File:   videoFetch.cpp
 * Author: flachesis
 * 
 * Created on 2011年4月17日, 上午 12:59
 */

#include "videoFetch.h"

videoFetch::videoFetch(std::set<std::string> *vidSet, std::set<std::string>::iterator start, std::set<std::string>::iterator end) {
    this->vidSet = vidSet;
    this->start = start;
    this->end = end;
    this->fetchHtmlHcs = new Poco::Net::HTTPClientSession("www.youtube.com",  Poco::Net::HTTPClientSession::HTTP_PORT);
    curl_global_init(CURL_GLOBAL_WIN32);
}

videoFetch::videoFetch(const videoFetch& orig) {
}

videoFetch::~videoFetch() {
    delete this->fetchHtmlHcs;
    curl_global_cleanup();
}

void videoFetch::run(){
    std::ofstream fout;
    std::string fileName;
    for(std::set<std::string>::iterator it = this->start; it != this->end;){
        try{
            std::auto_ptr<std::string> htmlContent(this->getHtmlContent(*it));
            if(htmlContent->find("verificationImage") != std::string::npos){
                Poco::Thread::sleep(1000 * 60);
                continue;
            }
            std::auto_ptr<std::vector<std::pair<std::string, std::string> > > urlExtensionVector(htmlParser::getDownloadUrlExtension(*htmlContent));
            if(urlExtensionVector.get() == NULL){
                it++;
                Poco::Thread::sleep(1000 * 60);
                continue;
            }
            for(std::vector<std::pair<std::string, std::string> >::iterator it2 = urlExtensionVector->begin(); it2 != urlExtensionVector->end(); it2++){
                fileName = *it + "." + it2->second;
                fout.open(fileName.c_str(), std::ios_base::out | std::ios_base::trunc | std::ios_base::binary);
                int videoBinData = this->getVideoUsingCurl(it2->first, fout);
                fout.close();
                if(videoBinData != 0){
                    std::remove(fileName.c_str());
                    continue;
                }else{
                    break;
                }
            }
            it++;
        }catch(Poco::Exception ex){
            std::cerr << ex.message() << std::endl;
        }
    }
}

std::string* videoFetch::getHtmlContent(const std::string &vid) throw (Poco::Exception){
    std::string *htmlContent = new std::string;
    Poco::Net::HTTPRequest req(Poco::Net::HTTPRequest::HTTP_GET, "/watch", Poco::Net::HTTPRequest::HTTP_1_1);
    Poco::Net::HTMLForm form;
    form.add("v", vid);
    form.prepareSubmit(req);
    this->fetchHtmlHcs->sendRequest(req);
    Poco::Net::HTTPResponse resp;
    std::istream &rs = this->fetchHtmlHcs->receiveResponse(resp);
    Poco::StreamCopier::copyToString(rs, *htmlContent);
    return htmlContent;
}

size_t videoFetch::getVideo(const std::string &url, std::ostream &outStream) throw (Poco::Exception){
    Poco::URI uri(url);
    std::auto_ptr<Poco::Net::HTTPClientSession> fetchVideoHcs(new Poco::Net::HTTPClientSession(uri.getHost(),  Poco::Net::HTTPClientSession::HTTP_PORT));
    Poco::Net::HTTPRequest req(Poco::Net::HTTPRequest::HTTP_GET, url, Poco::Net::HTTPRequest::HTTP_1_1);
    fetchVideoHcs->sendRequest(req);
    Poco::Net::HTTPResponse resp;
    std::istream &rs = fetchVideoHcs->receiveResponse(resp);
    Poco::StreamCopier::copyStream(rs, outStream);
    return outStream.tellp();
}

int videoFetch::getVideoUsingCurl(const std::string &url, std::ostream &outStream){
    CURL *curl = curl_easy_init();
    if(curl == NULL){
        return -1;
    }
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0 (Windows; U; Windows NT 6.1; zh-TW; rv:1.9.2.13) Gecko/20101203 Firefox/3.6.13");
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &outStream);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curlWriteCallBack);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
    curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 5);
    curl_easy_setopt(curl, CURLOPT_LOW_SPEED_LIMIT, 1);
    curl_easy_setopt(curl, CURLOPT_LOW_SPEED_TIME, 120);
    CURLcode res = curl_easy_perform(curl);
    long code;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &code);
    curl_easy_cleanup(curl);
    if(code >= 400){
        return CURLE_HTTP_RETURNED_ERROR;
    }
    return res;
}

size_t videoFetch::curlWriteCallBack( void *ptr, size_t size, size_t nmemb, void *userdata){
    std::ostream *outStream = (std::ostream*)userdata;
    outStream->write((char*)ptr, size * nmemb);
    return (size * nmemb);
}
