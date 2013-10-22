//
//  CURLObject.h
//  HelloExt
//
//  Created by Li Jinlong on 13-10-16.
//
//

#ifndef __HelloExt__CURLObject__
#define __HelloExt__CURLObject__

#include <curl/curl.h>
#include <iostream>
#include <vector>
#include "libMyExt.h"

NS_MY_EXT_BEGIN

class CURLRequest;
class CURLObject
{
    
private:
    /// Instance of CURL
    CURL *_curl;
    /// Keeps custom header data
    curl_slist *_headers;
public:
    CURLObject() : _curl(curl_easy_init()), _headers(NULL) { }
    
    ~CURLObject()
    {
        if (_curl)
            curl_easy_cleanup(_curl);
        /* free the linked list for header data */
        if (_headers)
            curl_slist_free_all(_headers);
    }
    
    template <class T>
    bool setOption(CURLoption option, T data);
    
    /** 
     * @brief Config CURL timeout and error buffer
     */
    bool configureCURL(int timeoutForRead, int timeoutForConnect, char* errorbuffer);
    
    /** init with CURLRequest
     */
    bool initWithRequest(CURLRequest* request, int timeoutForRead, int timeoutForConnect, char* errorbuffer);
    /// @param responseCode Null not allowed
    bool perform(long *responseCode);
    bool performRequestTask(CURLRequest* request, int timeoutForRead, int timeoutForConnect, char* errorbuffer);
};

NS_MY_EXT_END

#endif /* defined(__HelloExt__CURLObject__) */
