//
//  MyDownloadList.h
//  project: myext
//
//  Created by Li Jinlong on 2013-10-13.
//
//

#ifndef _myext_MyDownloadList_H_
#define _myext_MyDownloadList_H_

#include "cocos2d.h"
#include "myExtConfig.h"
#include "MyDownloadList.h"

NS_MY_EXT_BEGIN

class CURLRequestDelegate;
class DownLoadList : public cocos2d::Object {
public:
    // declare order: typedef enum const vals constructor/distructor methods data, override method
    static DownLoadList* getInstance();
    static void destroyInstance();
   
    inline void setBaseURL(const char* url)
    {
        _baseURL = url;
    }
    inline void setSavePath(const char* savePath)
    {
        _savePath = savePath;
    }
    void addDownloadFile(const char* file, bool unZipFile, CURLRequestDelegate* delegate);
    void removeFile(const char* file);
    void clear();
  
    void startDownload();
    inline int getTotolNumber()
    {
        return _totalDownloadNumbers;
    }
    inline int getFinishedNumbers()
    {
        return _finishedNumbers;
    }
  protected:
    // declare order: typedef enum const vals constructor/distructor methods data, override method
    
  private:
    // declare order: typedef enum const vals constructor/distructor methods data, override method
    std::string _baseURL;
    std::string _savePath;
    int _dispathThreads;
    int _totalDownloadNumbers;
    int _finishedNumbers;
    
    DownLoadList();
    virtual ~DownLoadList();
    bool init(int threadNum);
    /**
     * Init pthread mutex, semaphore, and create new thread for http requests
     * @return bool
     */
    bool lazyInitThreadSemphore();
    /** Poll function called from main thread to dispatch callbacks when http requests finished **/
    void dispatchResponseCallbacks(float delta);
    
    DISALLOW_COPY_AND_ASSIGN(DownLoadList);
};
#define NotifyDownLoadFinished "downloadfinishedNotify"

NS_MY_EXT_END

#endif /* defined(_myext_MyDownloadList_H_) */
