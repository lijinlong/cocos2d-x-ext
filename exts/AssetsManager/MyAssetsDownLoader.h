//
//  MyAssetsDownLoader.h
//  project: myext
//
//  Created by Li Jinlong on 2013-10-13.
//
//

#ifndef _myext_MyAssetsDownLoader_H_
#define _myext_MyAssetsDownLoader_H_

#include "myExtConfig.h"

NS_MY_EXT_BEGIN
class AssetsDownloader;
class DownloadList;

class AssetsDownloaderDelegate {
public:
    void downloadFinished(AssetsDownloader* downloader);
    void downloadField(AssetsDownloader* downloader);
};

class AssetsDownloader : public cocos2d::Object {
public:
    // declare order: typedef enum const vals constructor/distructor methods data, override method
    AssetsDownloader();
    virtual ~AssetsDownloader();
    static AssetsDownloader* create(DownloadList* list);
    bool init(DownloadList* list);
    
    void startDownload();
    void cancleDownload();
    
protected:
    // declare order: typedef enum const vals constructor/distructor methods data, override method
    DownloadList* _list;
private:
    // declare order: typedef enum const vals constructor/distructor methods data, override method
    
    DISALLOW_COPY_AND_ASSIGN(AssetsDownloader);
};

NS_MY_EXT_END


#endif /* defined(_myext_MyAssetsDownLoader_H_) */
