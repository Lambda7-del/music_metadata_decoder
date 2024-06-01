#include "image.h"
#include "log.h"
#include "utils.h"

#include <Windows.h>
#include <string>
#include <functional>

namespace music_data {

INITONLYLOGGER(); 

// mime类型与构造函数的映射关系，通过mime类型推断优先用哪个构造函数生成Image
static const std::unordered_map<std::string, std::function<Image::ptr(void*, size_t)>> s_tryCreateImageFunc = {
#define XX(str, Constuctor) \
    {str, [](void* data, size_t length) -> Image::ptr { \
        Image::ptr ans(new Constuctor(data, length)); \
        if (ans->isValid()) { \
            return ans; \
        } else { \
            return nullptr; \
        } \
    }}

    XX("image/png", PngImage), 
    XX("image/jpeg", JpegImage)

#undef XX
}; 

Image::ptr Image::TryCreateImage(void* data, size_t length, const std::string& mimeType) {
    Image::ptr ans = nullptr; 

    // 先根据mimeType构造Image
    if (s_tryCreateImageFunc.find(mimeType)!=s_tryCreateImageFunc.end()) {
        ans = s_tryCreateImageFunc.at(mimeType)(data, length); 
        if (ans!=nullptr) {
            return ans; 
        }
        LOGW("mimeType %s provided is wrong", mimeType); 
    }

    // 构造失败，则尝试所有格式的Image构造函数
    for (auto& item: s_tryCreateImageFunc) {
        if (item.first == mimeType) {
            continue; 
        }
        ans = item.second(data, length); 
        if (ans!=nullptr) {
            return ans; 
        }
    }

    return nullptr; 
}

Image::Image() {
}

Image::~Image() {
}

bool Image::openFile(const wchar_t* file_path) {
    // open file for file mapping	
    HANDLE hFile = CreateFileW(file_path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);	
    
    if (hFile==INVALID_HANDLE_VALUE) {
        LOGE("file not exists \n"); 
        return false; 
    }

    // get file size
    size_t nStreamSize = GetFileSize(hFile, NULL); 

    if(nStreamSize == 0xFFFFFFFF || nStreamSize == 0)
	{
		CloseHandle(hFile);
		hFile = INVALID_HANDLE_VALUE; 
        LOGE("file not exists \n"); 
        return false;
	}

    // create file mapping
    HANDLE hFileMapping = CreateFileMappingW(hFile,NULL,PAGE_READONLY,0,0,NULL);

    if (hFileMapping==INVALID_HANDLE_VALUE) {
        LOGE("CreateFileMappingW fail \n"); 
        CloseHandle(hFile); 
        return false; 
    }

    // create file view
    LPVOID hViewOfFile = MapViewOfFile(hFileMapping,FILE_MAP_READ,0,0,0); 

    if (hViewOfFile==NULL) {
        LOGE("MapViewOfFile fail \n"); 
        CloseHandle(hFileMapping); 
        CloseHandle(hFile); 
        return false; 
    }

    LOGD("read file successfully, %d byte read", GetFileSize(hFile, NULL)); 

    initImage((void*)hViewOfFile, nStreamSize); 

    UnmapViewOfFile(hViewOfFile); 
    CloseHandle(hFileMapping); 
    CloseHandle(hFile); 

    return true; 
}

bool Image::openFile(const std::wstring& file_path) {
    const wchar_t* sFileName = file_path.c_str(); 
    return openFile(sFileName); 
}

bool Image::getData(void* dest, size_t length) const {
    if (length!=getDataSize()) {
        LOGE("length not match, data length should be %d, but got %d", getDataSize(), length); 
        return false; 
    }
    size_t ret = m_data.getDataBuffers(dest, length); 
    if (ret!=length) {
        LOGE("not read enough, should read %d byte, but read %d byte", length, ret); 
        return false; 
    }
    return true; 
}

bool Image::resave(const wchar_t* path, bool ifCheckSuffix) const {
    if (!isValid()) {
        LOGE("Image data not valid!"); 
        return false; 
    }

    std::wstring sfile; 
    if (ifCheckSuffix) {
        sfile = checkSuffix(path); 
    } else {
        sfile = std::wstring(path); 
    }

    HANDLE hFile = CreateFileW(sfile.c_str(), GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, 0, NULL);	
    if (hFile==INVALID_HANDLE_VALUE) {
        LOGE("file not exists, and create file fail \n"); 
        return false; 
    }

    bool ifSuccess = true; 

    //成功写入的数据大小
    DWORD dwWritedDateSize;
    
    size_t dataSize = m_data.getSize(); 
    void* metaPin = malloc(dataSize); 
    m_data.getDataBuffers(metaPin, dataSize); 
    SetFilePointer(hFile,0,0,FILE_BEGIN);
    if(!WriteFile(hFile, metaPin, dataSize, &dwWritedDateSize,NULL)) {
        LOGE("write metadata fail: %d\n", GetLastError()); 
        ifSuccess = false; 
    }
    
    SetEndOfFile(hFile); 
 
    CloseHandle(hFile); 
    LOGD("write metadata successfully, %d byte written。\n", dwWritedDateSize);

    return ifSuccess; 
}

bool Image::resave(const std::wstring& path, bool ifCheckSuffix) const {
    return resave(path.c_str(), ifCheckSuffix); 
}

PngImage::PngImage(void* data, size_t length) {
    if (length>21) {
        initImage(data, length); 
    } else {
        LOGE("error: png length<21"); 
    }
}

PngImage::PngImage(const wchar_t* path){
    bool res = openFile(path); 
    if (res) {
        setType(PNG); 
    }
}

PngImage::PngImage(const std::wstring& path){
    bool res = openFile(path); 
    if (res) {
        setType(PNG); 
    }
}

PngImage::~PngImage(){
}

void PngImage::initImage(void* data, size_t length) {
    bool isPngRes = isPng(data); 
    if (!isPngRes) {
        setIsValid(false); 
        LOGE("not png!"); 
        return; 
    }

    // 直接到宽度位置
    uint8_t* pin = (uint8_t*)data+16; 
    m_width = byteswap(*(uint32_t*)pin); 
    pin+=4; 

    m_height = byteswap(*(uint32_t*)pin); 
    pin+=4; 

    m_colorDepthBit = *pin; 

    m_data.rewrite(data, length); 

    setType(PNG); 
}

std::wstring PngImage::checkSuffix(const wchar_t* path) const {
    std::wstring tmp_s(path); 
    size_t t_size = tmp_s.size(); 
    size_t dot_pos = tmp_s.find_last_of(L"."); 
    if (dot_pos!=std::wstring::npos) {
        std::wstring suffix = tmp_s.substr(dot_pos, t_size-dot_pos); 
        if (suffix!=L".png") {
            tmp_s = tmp_s.substr(0, dot_pos) + L".png"; 
            LOGI("suffix \".png\" is adjusted"); 
        }
    } else {
        tmp_s+=L".png"; 
        LOGI("suffix \".png\" is added"); 
    }

    return tmp_s; 
}

JpegImage::JpegImage(void* data, size_t length) {
    if (length>21) {
        initImage(data, length); 
    } else {
        LOGE("error: png length<21"); 
    }
}

JpegImage::JpegImage(const wchar_t* path){
    bool res = openFile(path); 
    if (res) {
        setType(JPEG); 
        m_path = std::wstring(path); 
    }
}

JpegImage::JpegImage(const std::wstring& path){
    bool res = openFile(path); 
    if (res) {
        setType(JPEG); 
        m_path = std::wstring(path); 
    }
}

JpegImage::~JpegImage(){
}

void JpegImage::initImage(void* data, size_t length) {
    bool isJpegRes = isJpeg(data); 
    if (!isJpegRes) {
        setIsValid(false); 
        LOGE("not jpeg!"); 
        return; 
    }

    // 色深默认24
    m_colorDepthBit = 24; 

    // 开始找width，Height信息
    uint8_t* pin = (uint8_t*)data+2; 
    size_t dataLength = length-2; 

    // 找SOF0段
    uint8_t label_sof0 = 0xC0; 

    while (dataLength>0) {
        pin+=1; 
        int ret = memcmp(pin, &label_sof0, 1); 
        pin+=1; 
        // 长度包括段长本身，故减2
        uint16_t segment_length = byteswap(*(uint16_t*)pin) - 2; 
        pin+=2; 
        if (ret==0) {
            m_width = byteswap(*(uint16_t*)(pin+1)); 
            m_height = byteswap(*(uint16_t*)(pin+3)); 
            break; 
        } else {
            pin+=segment_length; 
            dataLength-=4+segment_length; 
        }
    }

    m_data.rewrite(data, length); 
    setType(JPEG); 
}

std::wstring JpegImage::checkSuffix(const wchar_t* path) const {
    std::wstring tmp_s(path); 
    size_t t_size = tmp_s.size(); 
    size_t dot_pos = tmp_s.find_last_of(L"."); 
    if (dot_pos!=std::wstring::npos) {
        std::wstring suffix = tmp_s.substr(dot_pos, t_size-dot_pos); 
        if (suffix!=L".jpeg"&&suffix!=L".jpg") {
            tmp_s = tmp_s.substr(0, dot_pos) + L".jpeg"; 
            LOGI("suffix \".jpeg\" is adjusted"); 
        }
    } else {
        tmp_s+=L".jpeg"; 
        LOGI("suffix \".png\" is added"); 
    }

    return tmp_s; 
}

}