//
//  MyAssetsManager.h
//  HelloExt
//
//  Created by Li Jinlong on 13-10-13.
//
//

#ifndef __HelloExt__MyAssetsManager__
#define __HelloExt__MyAssetsManager__

#include "myExtConfig.h"

NS_MY_EXT_BEGIN

class AssetsDownLoader;
class DownloadList;
class UpdateChecker;

enum class AssetsManagerStatus {
    kStart = 0,
    kChecking,
    kDownloading,
    kDownloaded,
    kEnd,
};

class AssetsManager {
public:
    // declare order: typedef enum const vals constructor/distructor methods data, override method
    AssetsManager();
    virtual ~AssetsManager();
    static AssetsManager* create();
    bool init();
    
    void update();
    inline AssetsManagerStatus getStatus() { return _status; }
    
protected:
    // declare order: typedef enum const vals constructor/distructor methods data, override method
    
private:
    // declare order: typedef enum const vals constructor/distructor methods data, override method
    void setStatus(AssetsManagerStatus status);
    
    AssetsManagerStatus _status;
    std::string _remoteURL;
    std::string _AssetsSavedPath;
    std::string _errorBuffer;
    UpdateChecker* _updateChecker;
    AssetsDownLoader* _assetsDownloader;
    DownloadList* _downloadList;
    DISALLOW_COPY_AND_ASSIGN(AssetsManager);
};

NS_MY_EXT_END

#endif /* defined(__HelloExt__MyAssetsManager__) */
