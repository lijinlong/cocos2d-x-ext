//
//  MyUpdateCheaker.h
//  project: myext
//
//  Created by Li Jinlong on 2013-10-13.
//
//

#ifndef _myext_MyUpdateCheaker_H_
#define _myext_MyUpdateCheaker_H_

#include "myExtConfig.h"

NS_MY_EXT_BEGIN

class DownloadList;
class UpdateChecker;
class UpdateCheckerDelegate {
public:
    void dataCheckFinished(UpdateChecker* checker, bool shouldUpdate);
};

class UpdateChecker : public cocos2d::Object {
public:
    // declare order: typedef enum const vals constructor/distructor methods data, override method
    UpdateChecker();
    virtual ~UpdateChecker();
    static UpdateChecker* create();
    bool init();
    
    void update();
    void finished();
    void start();
    
    void setDownloadList(DownloadList* list);
    DownloadList* getDownloadList();
protected:
    // declare order: typedef enum const vals constructor/distructor methods data, override method
    DownloadList* _updatelist;
private:
    // declare order: typedef enum const vals constructor/distructor methods data, override method
    
    DISALLOW_COPY_AND_ASSIGN(UpdateChecker);
};

NS_MY_EXT_END

#endif /* defined(_myext_MyUpdateCheaker_H_) */
