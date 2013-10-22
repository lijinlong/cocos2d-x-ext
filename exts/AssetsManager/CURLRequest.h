//
//  CURLRequest.h
//  HelloExt
//
//  Created by Li Jinlong on 13-10-17.
//
//

#ifndef __HelloExt__CURLRequest__
#define __HelloExt__CURLRequest__

#include <curl/curl.h>
#include <iostream>
#include "cocos2d.h"
#include "libMyExt.h"

NS_MY_EXT_BEGIN

class CURLRequest;
class CURLRequestDelegate {
public:
    virtual void requestFailed(CURLRequest* request) { };
    virtual void requestSucceed(CURLRequest* request) { };
};

class CURLRequest : public cocos2d::Object {
public:
    enum class Type {
        HTTP_GET,
        HTTP_POST,
        HTTP_PUT,
        HTTP_DELETE,
        HTTP_FILEDOWNLOAD,
        HTTP_ZIPBUNDLE,
        UNKNOWN,
    };

    explicit CURLRequest(const char* url)
    {
        _type = Type::UNKNOWN;
        _url = url;
        _requestData.clear();
        _tag.clear();
        _delegate = NULL;
        _pUserData = NULL;
        _headers.clear();
        _succeed = false;
        _responseData.clear();
        _responseHeader.clear();
        _responseCode = -1;
        _errorBuffer.clear();
        _responseFileSavePath.clear();
    }
    virtual ~CURLRequest()
    {

    }
    
    CC_SYNTHESIZE(Type, _type, RequestType);
    
    /** Override autorelease method to avoid developers to call it */
    cocos2d::Object* autorelease(void)
    {
        CCASSERT(false, "HttpResponse is used between network thread and ui thread \
                 therefore, autorelease is forbidden here");
        return NULL;
    }
    
    /** Required field for HttpRequest object before being sent.
     */
    inline void setUrl(const char* url)
    {
        _url = url;
    };
    /** Get back the setted url */
    inline const char* getUrl()
    {
        return _url.c_str();
    };
    
    /** Option field. You can set your post data here
     */
    inline void setRequestData(const char* buffer, unsigned int len)
    {
        _requestData.assign(buffer, buffer + len);
    };
    /** Get the request data pointer back */
    inline char* getRequestData()
    {
        return &(_requestData.front());
    }
    /** Get the size of request data back */
    inline int getRequestDataSize()
    {
        return _requestData.size();
    }
    
    /** Option field. You can set a string tag to identify your request, this tag can be found in HttpResponse->getHttpRequest->getTag()
     */
    inline void setTag(const char* tag)
    {
        _tag = tag;
    };
    /** Get the string tag back to identify the request.
     The best practice is to use it in your MyClass::onMyHttpRequestCompleted(sender, HttpResponse*) callback
     */
    inline const char* getTag()
    {
        return _tag.c_str();
    };
    
    /** set delegate call back */
    inline void setDelegate(CURLRequestDelegate* del)
    {
        _delegate = del;
    }
    /** get delegate handler */
    CURLRequestDelegate* getDelegate()
    {
        return _delegate;
    }
    
    /** Option field. You can attach a customed data in each request, and get it back in response callback.
     But you need to new/delete the data pointer manully
     */
    inline void setUserData(void* pUserData)
    {
        _pUserData = pUserData;
    };
    /** Get the pre-setted custom data pointer back.
     Don't forget to delete it. HttpClient/HttpResponse/HttpRequest will do nothing with this pointer
     */
    inline void* getUserData()
    {
        return _pUserData;
    };
    
    /** Set any custom headers **/
    inline void setHeaders(std::vector<std::string> pHeaders)
   	{
   		_headers=pHeaders;
   	}
    
    /** Get custom headers **/
   	inline std::vector<std::string> getHeaders()
   	{
   		return _headers;
   	}
    
    /** To see if the http reqeust is returned successfully,
     Althrough users can judge if (http return code = 200), we want an easier way
     If this getter returns false, you can call getResponseCode and getErrorBuffer to find more details
     */
    inline bool isSucceed()
    {
        return _succeed;
    };
    
    /** Get the http response raw data */
    inline std::vector<char>* getResponseData()
    {
        return &_responseData;
    }
    
    /** get the Rawheader **/
    inline std::vector<char>* getResponseHeader()
    {
        return &_responseHeader;
    }
    
    /** Get the http response errorCode
     *  I know that you want to see http 200 :)
     */
    inline int getResponseCode()
    {
        return _responseCode;
    }
    
    /** Get the rror buffer which will tell you more about the reason why http request failed
     */
    inline const char* getErrorBuffer()
    {
        return _errorBuffer.c_str();
    }
    
    // setters, will be called by HttpClient
    // users should avoid invoking these methods
    
    
    /** Set if the http request is returned successfully,
     Althrough users can judge if (http code == 200), we want a easier way
     This setter is mainly used in HttpClient, users mustn't set it directly
     */
    inline void setSucceed(bool value)
    {
        _succeed = value;
    };
    
    /** Set the http response errorCode
     */
    inline void setResponseCode(int value)
    {
        _responseCode = value;
    }
    
    inline void setFileSavePath(const char* path)
    {
        _responseFileSavePath = path;
    }
    inline const char* getFileSavePath()
    {
        return _responseFileSavePath.c_str();
    }
    
    /** Set the error buffer which will tell you more the reason why http request failed
     */
    inline void setErrorBuffer(const char* value)
    {
        _errorBuffer.clear();
        _errorBuffer.assign(value);
    };
    
    inline void requestSucceedCallback()
    {
        if (_delegate) {
            _delegate->requestSucceed(this);
        }
    }
    
    inline void requestFailedCallback()
    {
        if (_delegate) {
            _delegate->requestFailed(this);
        }
    }
    
private:
    std::string _url;
    std::vector<char>           _requestData;    /// used for POST
    std::string                 _tag;            /// user defined tag, to identify different requests in response callback
    CURLRequestDelegate*        _delegate;      /// call back for request
    void*                       _pUserData;      /// You can add your customed data here
    std::vector<std::string>    _headers;		      /// custom http headers
    
    bool                _succeed;       /// to indecate if the http reqeust is successful simply
    std::vector<char>   _responseData;  /// the returned raw data. You can also dump it as a string
    std::vector<char>   _responseHeader;  /// the returned raw header data. You can also dump it as a string
    int                 _responseCode;    /// the status code returned from libcurl, e.g. 200, 404
    std::string         _errorBuffer;   /// if _responseCode != 200, please read _errorBuffer to find the reason
    std::string         _responseFileSavePath; /// file path for save response Data
};

NS_MY_EXT_END

#endif /* defined(__HelloExt__CURLRequest__) */
