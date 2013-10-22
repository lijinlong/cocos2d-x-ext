//
//  CURLObjectTest.h
//  HelloExt
//
//  Created by Li Jinlong on 13-10-18.
//
//

#ifndef __HelloExt__CURLObjectTest__
#define __HelloExt__CURLObjectTest__

#include "cocos2d.h"
#include "CURLRequest.h"

class CURLObjectTest : public cocos2d::Layer, public myext::CURLRequestDelegate {
public:
    // declare order: typedef enum const vals constructor/distructor methods data, override method
    CURLObjectTest() {};
    virtual ~CURLObjectTest();
    static cocos2d::Scene* scene();
    static CURLObjectTest* create();
    bool init();
    
    void menuCloseCallback(Object* pSender);
    void dowloadFile(cocos2d::Object* pSender);
    void dowloadZip(cocos2d::Object* pSender);
    void dowloadQueue(cocos2d::Object* pSender);
    
    void dowloadQueueFinishNotification(cocos2d::Object* pSender);
    
    virtual void requestSucceed(myext::CURLRequest* request);
    virtual void requestFailed(myext::CURLRequest* request);
protected:
    // declare order: typedef enum const vals constructor/distructor methods data, override method
    
private:
    // declare order: typedef enum const vals constructor/distructor methods data, override method
    cocos2d::LabelTTF* _label;
    cocos2d::LabelTTF* _labelProgress;
    DISALLOW_COPY_AND_ASSIGN(CURLObjectTest);
};

#endif /* defined(__HelloExt__CURLObjectTest__) */
