//
//  MyDownloadList.h
//  project: myext
//
//  Created by Li Jinlong on 2013-10-13.
//
//

#ifndef _myext_MyDownloadList_H_
#define _myext_MyDownloadList_H_

#include "myExtConfig.h"

NS_MY_EXT_BEGIN

typedef struct _Asset {
    const char* file;
    const char* url;
    const char* savePath;
    bool isZipFile;
} AssetObject;

class DownLoadList : public  {
  public:
    // declare order: typedef enum const vals constructor/distructor methods data, override method
    DownLoadList();
    virtual ~DownLoadList();
    static DownLoadList* create();
    bool init(const char* baseURL);
  
    void addAssetObject(AssetObject* asset);
    AssetObject* firstAssetObject();
    void removeObject(AssetObject* asset);
    void clear();
  
  protected:
    // declare order: typedef enum const vals constructor/distructor methods data, override method
    
  private:
    // declare order: typedef enum const vals constructor/distructor methods data, override method
    
    DISALLOW_COPY_AND_ASSIGN(DownLoadList);
};

NS_MY_EXT_END

#endif /* defined(_myext_MyDownloadList_H_) */
