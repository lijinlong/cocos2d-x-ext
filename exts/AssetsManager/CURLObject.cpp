//
//  CURLObject.cpp
//  HelloExt
//
//  Created by Li Jinlong on 13-10-16.
//
//

#include "CURLObject.h"
#include <fstream>
#include "support/zip_support/unzip.h"
#include "CURLRequest.h"


using namespace cocos2d;
USING_NS_MY_EXT;

//#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
//typedef int int32_t;
//#endif

static std::string s_userAgent = "libcurl-agent/1.0";
static int s_defaultTimeoutForRead = 30;
static int s_defaultTimeoutForConnect = 60;

typedef size_t (*write_callback)(void *ptr, size_t size, size_t nmemb, void *stream);

static std::string s_cookieFilename = "";

// Callback function used by libcurl for collect response data
static size_t writeData(void *ptr, size_t size, size_t nmemb, void *stream)
{
    std::vector<char> *recvBuffer = (std::vector<char>*)stream;
    size_t sizes = size * nmemb;
    
    // add data to the end of recvBuffer
    // write data maybe called more than once in a single request
    recvBuffer->insert(recvBuffer->end(), (char*)ptr, (char*)ptr+sizes);
    
    return sizes;
}

// Callback function used by libcurl for collect header data
static size_t writeHeaderData(void *ptr, size_t size, size_t nmemb, void *stream)
{
    std::vector<char> *recvBuffer = (std::vector<char>*)stream;
    size_t sizes = size * nmemb;
    
    // add data to the end of recvBuffer
    // write data maybe called more than once in a single request
    recvBuffer->insert(recvBuffer->end(), (char*)ptr, (char*)ptr+sizes);
    
    return sizes;
}

static bool createDirectory(const char* dir)
{
    // cmd is mkdir -p "<dir>" use "" to parse <space> in dir.
#if CC_TARGET_PLATFORM != CC_PLATFORM_WIN32
    //#if CC_TARGET_PLATFORM == CC_PLATFORM_IOS
    std::string cmd = "mkdir -p \"";
#else
    std::string cmd = "mkdir \"";
#endif
    cmd.append(dir);
    cmd.append("\"");
    system(cmd.c_str());
    return true;
}

static void createFolderForFile(const char* fileName)
{
    std::string path(fileName);
    std::string dir;
    std::string::size_type pos = path.find_last_of('/');
    if (pos == std::string::npos) {
        return;
    } else {
        dir = path.substr(0, pos);
    }
    
    createDirectory(dir.c_str());
}

static bool saveFile(const char* fileName, std::vector<char>* data)
{
    bool result = false;
    createFolderForFile(fileName);
    std::ofstream fos(fileName, std::ios::binary);
    if (fos) {
        for (std::vector<char>::iterator iter = data->begin(); iter != data->end(); ++iter) {
            fos<<(*iter);
        }
        fos.flush();
        fos.close();
        result = true;
    } else {
        log("save file %s error", fileName);
    }
    return result;
}

bool uncompress(const char* fileName)
{
    // Open the zip file
    std::string outFileName(fileName);
    std::string _storagePath;
    std::size_t pos = outFileName.find_last_of("/");
    if (pos != std::string::npos) {
        _storagePath = outFileName.substr(0, pos+1);

    }
    unzFile zipfile = unzOpen(outFileName.c_str());
    if (! zipfile)
    {
        CCLOG("can not open downloaded zip file %s", outFileName.c_str());
        return false;
    }
    
    // Get info about the zip file
    unz_global_info global_info;
    if (unzGetGlobalInfo(zipfile, &global_info) != UNZ_OK)
    {
        CCLOG("can not read file global info of %s", outFileName.c_str());
        unzClose(zipfile);
        return false;
    }
    
    // Buffer to hold data read from the zip file
    const int BUFFER_SIZE = 8192;
    const int MAX_FILENAME = 1024;
    char readBuffer[BUFFER_SIZE];
    
    CCLOG("start uncompressing");
    
    // Loop to extract all files.
    uLong i;
    for (i = 0; i < global_info.number_entry; ++i)
    {
        // Get info about current file.
        unz_file_info fileInfo;
        char fileName[MAX_FILENAME];
        if (unzGetCurrentFileInfo(zipfile,
                                  &fileInfo,
                                  fileName,
                                  MAX_FILENAME,
                                  NULL,
                                  0,
                                  NULL,
                                  0) != UNZ_OK)
        {
            CCLOG("can not read file info");
            unzClose(zipfile);
            return false;
        }
        
        std::string fullPath = _storagePath + fileName;
        
        // Check if this entry is a directory or a file.
        const size_t filenameLength = strlen(fileName);
        if (fileName[filenameLength-1] == '/')
        {
            // Entry is a direcotry, so create it.
            // If the directory exists, it will failed scilently.
            if (!createDirectory(fullPath.c_str()))
            {
                CCLOG("can not create directory %s", fullPath.c_str());
                unzClose(zipfile);
                return false;
            }
        }
        else
        {
            // Entry is a file, so extract it.
            
            // Open current file.
            if (unzOpenCurrentFile(zipfile) != UNZ_OK)
            {
                CCLOG("can not open file %s", fileName);
                unzClose(zipfile);
                return false;
            }
            
            // Create a file to store current file.
            FILE *out = fopen(fullPath.c_str(), "wb");
            if (! out)
            {
                CCLOG("can not open destination file %s", fullPath.c_str());
                unzCloseCurrentFile(zipfile);
                unzClose(zipfile);
                return false;
            }
            
            // Write current file content to destinate file.
            int error = UNZ_OK;
            do
            {
                error = unzReadCurrentFile(zipfile, readBuffer, BUFFER_SIZE);
                if (error < 0)
                {
                    CCLOG("can not read zip file %s, error code is %d", fileName, error);
                    unzCloseCurrentFile(zipfile);
                    unzClose(zipfile);
                    return false;
                }
                
                if (error > 0)
                {
                    fwrite(readBuffer, error, 1, out);
                }
            } while(error > 0);
            
            fclose(out);
        }
        
        unzCloseCurrentFile(zipfile);
        
        // Goto next entry listed in the zip file.
        if ((i+1) < global_info.number_entry)
        {
            if (unzGoToNextFile(zipfile) != UNZ_OK)
            {
                CCLOG("can not read next file");
                unzClose(zipfile);
                return false;
            }
        }
    }
    
    CCLOG("end uncompressing");
    
    return true;
}

template <class T>
bool CURLObject::setOption(CURLoption option, T data)
{
    return CURLE_OK == curl_easy_setopt(_curl, option, data);
}

//Configure curl's timeout property
bool CURLObject::configureCURL(int timeoutForRead, int timeoutForConnect, char* errorbuffer)
{
    CURL *handle = _curl;
    if (!handle) {
        return false;
    }
    
    int32_t code;
    if (!s_userAgent.empty()) {
        code = curl_easy_setopt(handle, CURLOPT_USERAGENT, s_userAgent.c_str());
        if (code != CURLE_OK) {
            return false;
        }
    }
    if (errorbuffer) {
        code = curl_easy_setopt(handle, CURLOPT_ERRORBUFFER, errorbuffer);
        if (code != CURLE_OK) {
            return false;
        }
    }
    code = curl_easy_setopt(handle, CURLOPT_TIMEOUT, (timeoutForRead>0? timeoutForRead: s_defaultTimeoutForRead));
    if (code != CURLE_OK) {
        return false;
    }
    code = curl_easy_setopt(handle, CURLOPT_CONNECTTIMEOUT, (timeoutForConnect>0? timeoutForConnect: s_defaultTimeoutForConnect));
    if (code != CURLE_OK) {
        return false;
    }
    curl_easy_setopt(handle, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(handle, CURLOPT_SSL_VERIFYHOST, 0L);
    
    return true;
}

bool CURLObject::initWithRequest(CURLRequest* request, int timeoutForRead, int timeoutForConnect, char* errorbuffer)
{
    if (!_curl)
        return false;
//    if (!configureCURL(timeoutForRead, timeoutForConnect, errorbuffer))
//        return false;
//
//    /* get custom header data (if set) */
//    std::vector<std::string> headers=request->getHeaders();
//    if(!headers.empty())
//    {
//        /* append custom headers one by one */
//        for (std::vector<std::string>::iterator it = headers.begin(); it != headers.end(); ++it) {
//            _headers = curl_slist_append(_headers,it->c_str());
//        }
//        /* set custom headers for curl */
//        if (!setOption(CURLOPT_HTTPHEADER, _headers)) {
//            return false;
//        }
//    }
////    if (!s_cookieFilename.empty()) {
////        if (!setOption(CURLOPT_COOKIEFILE, s_cookieFilename.c_str())) {
////            return false;
////        }
////        if (!setOption(CURLOPT_COOKIEJAR, s_cookieFilename.c_str())) {
////            return false;
////        }
////    }

    bool ok = setOption(CURLOPT_URL, request->getUrl())
    && setOption(CURLOPT_WRITEFUNCTION, writeData)
    && setOption(CURLOPT_WRITEDATA, request->getResponseData());
//    && setOption(CURLOPT_HEADERFUNCTION, writeHeaderData)
//    && setOption(CURLOPT_HEADERDATA, request->getResponseHeader());


//    if (ok && request->getRequestType() == CURLRequest::Type::HTTP_FILEDOWNLOAD) {
//        ok = setOption(CURLOPT_WRITEFUNCTION, writeData)
//        && setOption(CURLOPT_WRITEDATA, request->getResponseData());
//    }
    return ok;
}

bool CURLObject::perform(long *responseCode)
{
    *responseCode = -1;
    if (CURLE_OK != curl_easy_perform(_curl)) {
        return false;
    }
    CURLcode code = curl_easy_getinfo(_curl, CURLINFO_RESPONSE_CODE, responseCode);
    if (code != CURLE_OK || *responseCode != 200) {
        CCLOGERROR("CURL curl_easy_getinfo(CURLINFO_RESPONSE_CODE) %s, response code: %ld", curl_easy_strerror(code), *responseCode);
        return false;
    }
    // Get some mor data.
    
    return true;
}

bool CURLObject::performRequestTask(CURLRequest* request, int timeoutForRead, int timeoutForConnect, char* errorbuffer)
{
    long responseCode;
    bool ok = false;
    
    // Process the request -> get response packet
    switch (request->getRequestType())
    {
        case CURLRequest::Type::HTTP_GET: { // HTTP GET
            ok = this->initWithRequest(request, timeoutForRead, timeoutForConnect, errorbuffer)
            && this->setOption(CURLOPT_FOLLOWLOCATION, true)
            && this->perform(&responseCode);
            break;
        }
        case CURLRequest::Type::HTTP_POST: { // HTTP POST
            ok = this->initWithRequest(request, timeoutForRead, timeoutForConnect, errorbuffer)
            && this->setOption(CURLOPT_POST, 1)
            && this->setOption(CURLOPT_POSTFIELDS, request->getRequestData())
            && this->setOption(CURLOPT_POSTFIELDSIZE, request->getRequestDataSize())
            && this->perform(&responseCode);
            break;
        }
        case CURLRequest::Type::HTTP_PUT: {
            ok = this->initWithRequest(request, timeoutForRead, timeoutForConnect, errorbuffer)
            && this->setOption(CURLOPT_CUSTOMREQUEST, "PUT")
            && this->setOption(CURLOPT_POSTFIELDS, request->getRequestData())
            && this->setOption(CURLOPT_POSTFIELDSIZE, request->getRequestDataSize())
            && this->perform(&responseCode);
            break;
        }
        case CURLRequest::Type::HTTP_DELETE: {
            ok = this->initWithRequest(request, timeoutForRead, timeoutForConnect, errorbuffer)
            && this->setOption(CURLOPT_CUSTOMREQUEST, "DELETE")
            && this->setOption(CURLOPT_FOLLOWLOCATION, true)
            && this->perform(&responseCode);
            break;
        }
        case CURLRequest::Type::HTTP_FILEDOWNLOAD:
        case CURLRequest::Type::HTTP_ZIPBUNDLE: {
            ok = this->initWithRequest(request, timeoutForRead, timeoutForConnect, errorbuffer)
            && this->setOption(CURLOPT_FOLLOWLOCATION, true)
            && this->perform(&responseCode);
            break;
        }
        default: {
            ok = false;
            CCASSERT(true, "CCHttpClient: unkown request type, only GET and POSt are supported");
            break;
        }
    }
    request->setResponseCode(responseCode);
    if (!ok)
    {
        request->setSucceed(false);
        request->setErrorBuffer(errorbuffer);
    }
    else
    {
        request->setSucceed(true);
        if (request->getRequestType() == CURLRequest::Type::HTTP_FILEDOWNLOAD) {
            const char* fileName = request->getFileSavePath();
            ok = saveFile(fileName, request->getResponseData());
            if (!ok) {
                request->setSucceed(false);
                request->setErrorBuffer("save download file error.");
            }
        } else if (request->getRequestType() == CURLRequest::Type::HTTP_ZIPBUNDLE) {
            const char* fileName = request->getFileSavePath();
            ok = saveFile(fileName, request->getResponseData());
            ok = ok && uncompress(fileName);
            if (!ok) {
                request->setSucceed(false);
                request->setErrorBuffer("uncompress download file error.");
            } else {
                remove(fileName);
            }
        }
    }
    return ok;
}
