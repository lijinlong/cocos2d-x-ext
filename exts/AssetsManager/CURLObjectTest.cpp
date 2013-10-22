//
//  CURLObjectTest.cpp
//  HelloExt
//
//  Created by Li Jinlong on 13-10-18.
//
//

#include "CURLObjectTest.h"
#include "CURLObject.h"
#include "MyDownloadList.h"

USING_NS_CC;
USING_NS_MY_EXT;

#define BASE_URL "http://localhost:7193/"
#define DownFileName "test_file.txt"
#define DownZipFile "test_zip.zip"
//#define DownZipFile "catalog.zip"

static void printResponseData(CURLRequest* request)
{
    CURLRequest* response = request;
    const char* url = request->getUrl();
    log("Response for <%s>: %s", url, (response->isSucceed()? "succeed": "failed"));
    log("Request Type: %d", request->getRequestType());
    const char* tag = request->getTag();
    if (tag) {
        log("Tag: %s", tag);
    }
    std::vector<std::string> headers = request->getHeaders();
    if (!headers.empty()) {
        log("> Headers: ");
        for (auto it = headers.begin(); it != headers.end(); ++it) {
            log("\t\t%s", (*it).c_str());
        }
    }
    int reqDataSz = request->getRequestDataSize();
    if (reqDataSz > 0) {
        char* reqData = request->getRequestData();
        std::string str(reqData, reqData+reqDataSz);
        log("> Request Data: %s", str.c_str());
    }

    log("< Responsed Code: %d", response->getResponseCode());
    std::vector<char>* vec = response->getResponseHeader();
    std::string header(vec->begin(), vec->end());
    log("< Responsed Header: %s", header.c_str());
    vec = response->getResponseData();
    std::string responseData(vec->begin(), vec->end());
    log("< Responsed Data : %s\n\n", responseData.c_str());
    const char* errorbuffer = response->getErrorBuffer();
    if (strlen(errorbuffer) > 0) {
        log("< Error buffer: %s", response->getErrorBuffer());
    }
}

Scene* CURLObjectTest::scene()
{
    Scene* scene = Scene::create();
    CURLObjectTest* test = CURLObjectTest::create();
    scene->addChild(test);
    return scene;
}

CURLObjectTest* CURLObjectTest::create()
{
    CURLObjectTest* test = new CURLObjectTest();
    if (test && test->init()) {
        test->autorelease();
    } else {
        CC_SAFE_DELETE(test);
    }
    return test;
}

CURLObjectTest::~CURLObjectTest()
{
    NotificationCenter::getInstance()->removeAllObservers(this);
}

bool CURLObjectTest::init()
{
    if ( !Layer::init() )
    {
        return false;
    }
    
    curl_global_init(CURL_GLOBAL_ALL);
    
    Size visibleSize = Director::getInstance()->getVisibleSize();
    Point origin = Director::getInstance()->getVisibleOrigin();
    
    /////////////////////////////
    // 2. add a menu item with "X" image, which is clicked to quit the program
    //    you may modify it.
    
    // add a "close" icon to exit the progress. it's an autorelease object
    auto closeItem = MenuItemImage::create(
                                           "CloseNormal.png",
                                           "CloseSelected.png",
                                           CC_CALLBACK_1(CURLObjectTest::menuCloseCallback, this));
    
	closeItem->setPosition(Point(origin.x + visibleSize.width - closeItem->getContentSize().width/2 ,
                                 origin.y + closeItem->getContentSize().height/2));

    auto downFile = MenuItemFont::create("down file", CC_CALLBACK_1(CURLObjectTest::dowloadFile, this));
    auto downZip = MenuItemFont::create("down zip", CC_CALLBACK_1(CURLObjectTest::dowloadZip, this));
    auto downQueue = MenuItemFont::create("down queue", CC_CALLBACK_1(CURLObjectTest::dowloadQueue, this));
    
    // create menu, it's an autorelease object
    auto menu = Menu::create(downFile, downZip, downQueue, NULL);
    menu->alignItemsHorizontally();
    Point mid(visibleSize.width/2, 60);
    menu->setPosition(mid);
    this->addChild(menu, 1);
    
    /////////////////////////////
    // 3. add your codes below...
    
    // add a label shows "Hello World"
    // create and initialize a label
    
    _label = LabelTTF::create("Hello World", "Arial", 24);
    
    // position the label on the center of the screen
    _label->setPosition(Point(origin.x + visibleSize.width/2,
                             origin.y + visibleSize.height - _label->getContentSize().height));
    
    // add the label as a child to this layer
    this->addChild(_label, 1);
    
    _labelProgress = LabelTTF::create("sub", "Arial", 24);
    
    // position the label on the center of the screen
    _labelProgress->setPosition(Point(origin.x + visibleSize.width/2,
                              origin.y + visibleSize.height - _label->getContentSize().height - 40));
    
    // add the label as a child to this layer
    this->addChild(_labelProgress, 1);
    _labelProgress->setVisible(false);
    
    // add "HelloWorld" splash screen"
    auto layer = LayerColor::create(Color4B(0,0,0,255));
    
    // position the sprite on the center of the screen
    layer->setPosition(Point(visibleSize.width/2 + origin.x, visibleSize.height/2 + origin.y));
    
    // add the sprite as a child to this layer
    this->addChild(layer, 0);
    
    NotificationCenter::getInstance()->addObserver(this, callfuncO_selector(CURLObjectTest::dowloadQueueFinishNotification), NotifyDownLoadFinished, NULL);
    
    DownLoadList* list = DownLoadList::getInstance();
    list->setBaseURL(BASE_URL);
    const char* savePath = FileUtils::getInstance()->getWritablePath().c_str();
    list->setSavePath(savePath);
    
    return true;
}

void CURLObjectTest::menuCloseCallback(Object* pSender)
{
    Director::getInstance()->popScene();
}

void CURLObjectTest::dowloadFile(cocos2d::Object* pSender)
{
    std::string url = BASE_URL;
    url += DownFileName; //DownZipFile;
    std::string savePath = FileUtils::getInstance()->getWritablePath();
    savePath += DownFileName; //DownZipFile;
    CURLRequest* request = new CURLRequest(url.c_str());
    request->setDelegate(this);
    request->setFileSavePath(savePath.c_str());
    request->setRequestType(CURLRequest::Type::HTTP_FILEDOWNLOAD);
    CURLObject object;
    char errorBuf[512];
    memset(errorBuf, 0, 512);
    object.performRequestTask(request, 30, 60, errorBuf);
    
    if (request->isSucceed()) {
        _label->setString("Download File and save ok.");
    } else {
        printResponseData(request);
    }
    delete request;
}
void CURLObjectTest::dowloadZip(cocos2d::Object* pSender)
{
    std::string url = BASE_URL;
    url += DownZipFile;
    std::string savePath = FileUtils::getInstance()->getWritablePath();
    savePath += DownZipFile;
    CURLRequest* request = new CURLRequest(url.c_str());
    request->setDelegate(this);
    request->setFileSavePath(savePath.c_str());
    request->setRequestType(CURLRequest::Type::HTTP_ZIPBUNDLE);
    CURLObject object;
    char errorBuf[512];
    memset(errorBuf, 0, 512);
    object.performRequestTask(request, 30, 60, errorBuf);
    
    if (request->isSucceed()) {
        _label->setString("Download Zip File and uncompress ok.");
    } else {
        printResponseData(request);
    }
    delete request;
}

void CURLObjectTest::dowloadQueue(cocos2d::Object* pSender)
{
    DownLoadList* list = DownLoadList::getInstance();
    const int num = 8;
    char str[60];
    for (int i = 1; i <= num; ++i) {
        snprintf(str, 60, "%d.txt", i);
        list->addDownloadFile(str, false, this);
    }
    list->startDownload();
    _label->setString("download started.");
    snprintf(str, 60, "%d/%d", DownLoadList::getInstance()->getFinishedNumbers(), DownLoadList::getInstance()->getTotolNumber());
    _labelProgress->setString(str);
    _labelProgress->setVisible(true);
}

void CURLObjectTest::dowloadQueueFinishNotification(cocos2d::Object* pSender)
{
    _label->setString("Down queue finished.");
}

void CURLObjectTest::requestSucceed(CURLRequest* request)
{
    log("File %s downloaded.", request->getTag());
    char str[60];
    snprintf(str, 60, "%d/%d", DownLoadList::getInstance()->getFinishedNumbers(), DownLoadList::getInstance()->getTotolNumber());
    _labelProgress->setString(str);
}

void CURLObjectTest::requestFailed(CURLRequest* request)
{
    log("File %s downloaded failed.", request->getTag());
    char str[60];
    snprintf(str, 60, "%s1", request->getTag());
    DownLoadList::getInstance()->addDownloadFile(str, false, this);
}
