#include "MyDownloadList.h"
#include "CURLObject.h"
#include "CURLRequest.h"

USING_NS_CC;
USING_NS_MY_EXT;

static std::mutex       s_requestQueueMutex;
static std::mutex       s_responseQueueMutex;

static std::mutex		s_SleepMutex;
static std::condition_variable		s_SleepCondition;

static unsigned long    s_asyncRequestCount = 0;

static bool s_need_quit = false;
static bool s_waiting = true;

static Array* s_requestQueue = NULL;
static Array* s_responseQueue = NULL;

static DownLoadList *s_pDownloadList = NULL; // pointer to singleton
static char s_errorBuffer[CURL_ERROR_SIZE];

// Worker thread
static void networkThread(int /*i*/)
{
    CURLRequest *request = NULL;
    
    while (true)
    {
        if (s_need_quit)
        {
            break;
        }
        if (s_waiting)
        {
            continue;
        }
        
        // step 1: send http request if the requestQueue isn't empty
        request = NULL;
        
        s_requestQueueMutex.lock();
        
        //Get request task from queue
        
        if (0 != s_requestQueue->count())
        {
            request = dynamic_cast<CURLRequest*>(s_requestQueue->getObjectAtIndex(0));
            request->retain();
            s_requestQueue->removeObjectAtIndex(0);
        }
        
        s_requestQueueMutex.unlock();
        
        if (NULL == request)
        {
            // Wait for http request tasks from main thread
            std::unique_lock<std::mutex> lk(s_SleepMutex);
            s_SleepCondition.wait(lk);
            continue;
        }
        
        // step 2: libcurl sync access
        {
            CURLObject obj;
            obj.performRequestTask(request, 30, 60, s_errorBuffer);
        }
        
        // add response packet into queue
        s_responseQueueMutex.lock();
        s_responseQueue->addObject(request);
        s_responseQueueMutex.unlock();
        request->release();
        
        // resume dispatcher selector
        Director::getInstance()->getScheduler()->resumeTarget(DownLoadList::getInstance());
    }
    
    // cleanup: if worker thread received quit signal, clean up un-completed request queue
    s_requestQueueMutex.lock();
    s_requestQueue->removeAllObjects();
    s_requestQueueMutex.unlock();
    
    s_asyncRequestCount -= s_requestQueue->count();
    
    if (s_requestQueue != NULL) {
        
        s_requestQueue->release();
        s_requestQueue = NULL;
        s_responseQueue->release();
        s_responseQueue = NULL;
    }
    
}


DownLoadList::DownLoadList()
: _dispathThreads(1), _totalDownloadNumbers(0), _finishedNumbers(0)
{
    Director::getInstance()->getScheduler()->scheduleSelector(
                                                              schedule_selector(DownLoadList::dispatchResponseCallbacks), this, 0, false);
    Director::getInstance()->getScheduler()->pauseTarget(this);
}

DownLoadList::~DownLoadList()
{
    s_need_quit = true;
    
    if (s_requestQueue != NULL) {
    	s_SleepCondition.notify_one();
    }
    
    s_pDownloadList = NULL;
}

DownLoadList* DownLoadList::getInstance()
{
    if (s_pDownloadList == NULL) {
        s_pDownloadList = new DownLoadList();
        s_pDownloadList->init(1);
    }
    
    return s_pDownloadList;
}

void DownLoadList::destroyInstance()
{
    CCASSERT(s_pDownloadList, "");
    Director::getInstance()->getScheduler()->unscheduleSelector(schedule_selector(DownLoadList::dispatchResponseCallbacks), s_pDownloadList);
    s_pDownloadList->release();
}

bool DownLoadList::init(int threadNum)
{
    _dispathThreads = threadNum;
    _totalDownloadNumbers = 0;
    _finishedNumbers = 0;
    
    return true;
}

void DownLoadList::addDownloadFile(const char* file, bool unZipFile, CURLRequestDelegate* delegate)
{
    if (false == lazyInitThreadSemphore())
    {
        return;
    }
    
    std::string url = _baseURL;
    url.append(file);
    std::string savePath = _savePath;
    savePath.append(file);
    CURLRequest* request = new CURLRequest(url.c_str());
    request->setRequestType(unZipFile? CURLRequest::Type::HTTP_ZIPBUNDLE: CURLRequest::Type::HTTP_FILEDOWNLOAD);
    request->setDelegate(delegate);
    request->setFileSavePath(savePath.c_str());
    request->setTag(file);
    
    ++s_asyncRequestCount;
    ++_totalDownloadNumbers;
    
    s_requestQueueMutex.lock();
    s_requestQueue->addObject(request);
    s_requestQueueMutex.unlock();
    
    request->release();
    // Notify thread start to work
    s_SleepCondition.notify_one();
}

void DownLoadList::removeFile(const char* file)
{
    cocos2d::Object* pObj;
    cocos2d::Array* array = new cocos2d::Array();
    s_requestQueueMutex.lock();
    CCARRAY_FOREACH(s_requestQueue, pObj) {
        CURLRequest* request = dynamic_cast<CURLRequest*>(pObj);
        if (request && request->getTag() == file) {
            array->addObject(request);
        }
    }
    
    s_requestQueue->removeObjectsInArray(array);
    s_requestQueueMutex.unlock();
    s_asyncRequestCount -= array->count();
    _totalDownloadNumbers -= array->count();
    array->release();
}

void DownLoadList::clear()
{
    int count = 0;
    s_requestQueueMutex.lock();
    count = s_requestQueue->count();
    s_requestQueue->removeAllObjects();
    s_requestQueueMutex.unlock();
    
    s_asyncRequestCount -= count;
    _totalDownloadNumbers -= count;
}

void DownLoadList::startDownload()
{
    s_waiting = false;
    s_SleepCondition.notify_one();
}

bool DownLoadList::lazyInitThreadSemphore()
{
    if (s_requestQueue != NULL) {
        return true;
    } else {
        
        s_requestQueue = new Array();
        s_requestQueue->init();
        
        s_responseQueue = new Array();
        s_responseQueue->init();
        
        //int n = _dispathThreads;
        //for (int i = 0; i < n; ++i)
        //{
            auto t = std::thread(&networkThread, 0);
            t.detach();
        //}
        
        s_need_quit = false;
        s_waiting = true;
    }
    
    return true;
}

void DownLoadList::dispatchResponseCallbacks(float delta)
{
    CURLRequest* response = NULL;
    
    s_responseQueueMutex.lock();
    
    if (s_responseQueue->count())
    {
        response = dynamic_cast<CURLRequest*>(s_responseQueue->getObjectAtIndex(0));
        response->retain();
        s_responseQueue->removeObjectAtIndex(0);
    }
    
    s_responseQueueMutex.unlock();
    
    if (response)
    {
        --s_asyncRequestCount;
        ++_finishedNumbers;
        
        if (response->isSucceed()) {
            response->requestSucceedCallback();
        } else {
            response->requestFailedCallback();
        }
        
        response->release();
    }
    
    if (0 == s_asyncRequestCount)
    {
        NotificationCenter::getInstance()->postNotification(NotifyDownLoadFinished, this);
        Director::getInstance()->getScheduler()->pauseTarget(this);
    }
}
