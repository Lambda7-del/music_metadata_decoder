#ifndef __MD_DECODER_H_
#define __MD_DECODER_H_

#include "image.h"

#include <memory>
#include <string>
#include <list>
#include <vector>

namespace music_data {

class MusicDecoder {
public: 
    typedef std::shared_ptr<MusicDecoder> ptr; 

    /**
     * @brief 默认构造函数
    */
    MusicDecoder(); 

    /**
     * @brief 析构函数
    */
    virtual ~MusicDecoder(); 

    /**
     * @brief 加载音频文件
     * @param[in] file_path 文件路径
     * @retval 是否成功打开
    */
    bool openFile(std::wstring& file_path); 
    
    /**
     * @brief 加载音频文件
     * @param[in] file_path 文件路径
     * @retval 是否成功打开
    */
    bool openFile(const wchar_t* file_path); 

    /**
     * @brief 取得文件路径
     * @retval 文件路径wstring
    */
    std::wstring getFileName() const { return m_file_path; }

    /**
     * @brief 返回是否有效数据
     * @retval 数据是否有效
    */
    bool isValid() const { return m_isValid; }

protected: 
    /**
     * @brief 设置是否有效
     * @param[in] val 设置值
    */
    void setIsValid(bool val) { m_isValid = val; }

    /**
     * @brief 初始化数据
     * @param[in] data 数据指针
     * @param[in] length 数据长度
    */
    virtual void initData(void* data, size_t length) = 0; 

    /**
     * @brief 设置标题并修改文件
     * @param[in] val 设置值
     * @retval 是否成功
    */
    virtual bool setbackTitle(const std::string& val) = 0; 

    /**
     * @brief 设置唱片集艺术家并修改文件
     * @param[in] val 设置值
     * @retval 是否成功
    */
    virtual bool setbackAlbumArtist(const std::string& val) = 0; 

    /**
     * @brief 设置唱片集并修改文件
     * @param[in] val 设置值
     * @retval 是否成功
    */
    virtual bool setbackAlbum(const std::string& val) = 0; 

    /**
     * @brief 设置艺术家并修改文件
     * @param[in] val 设置值
     * @param[in] pos 指定艺术家index
     * @retval 是否成功
    */
    virtual bool setbackArtist(const std::string& val, uint32_t pos) = 0; 

    /**
     * @brief 添加艺术家并修改文件
     * @param[in] val 添加值
     * @param[in] pos 添加位置，默认-1表示加在最后
     * @retval 是否成功
    */
    virtual bool addbackArtist(const std::string& val, int pos = -1) = 0; 

    /**
     * @brief 删除艺术家并修改文件
     * @param[in] val 删除值
     * @retval 是否成功
    */
    virtual bool delbackArtist(const std::string& val) = 0; 

    /**
     * @brief 删除艺术家并修改文件
     * @param[in] index 删除值index
     * @retval 是否成功
    */
    virtual bool delbackArtist(uint32_t index) = 0; 

    /**
     * @brief 重设指定艺术家的顺序位置
     * @param[in] old_pos 原位置
     * @param[in] new_pos 现位置
     * @retval 是否成功
    */
    virtual bool resetPosbackArtist(uint32_t old_pos, uint32_t new_pos) = 0; 

    /**
     * @brief 设置音轨并修改文件
     * @param[in] val 设置值
    */
    virtual bool setbackTrack(uint8_t val) = 0; 

    /**
     * @brief 设置封面
     * @param[in] img 设置值
     * @param[in] pos 设置位置，默认第一位
     * @retval 是否设置成功
    */
    virtual bool setbackCover(const Image& img, uint32_t pos = 0) = 0; 

    /**
     * @brief 添加封面
     * @param[in] img 添加值
     * @param[in] pos 添加位置，默认第一位
     * @retval 是否添加成功
    */
    virtual bool addbackCover(const Image& img, uint32_t pos = 0) = 0; 

    /**
     * @brief 删除封面
     * @param[in] pos 删除位置，默认第一位
     * @retval 是否删除成功
    */
    virtual bool delbackCover(uint32_t pos = 0) = 0; 

    /**
     * @brief 重设指定封面的顺序位置
     * @param[in] old_pos 原位置
     * @param[in] new_pos 现位置
     * @retval 是否成功
    */
    virtual bool resetPosbackCover(uint32_t old_pos, uint32_t new_pos) = 0; 

    /**
     * @brief 取得标题
     * @retval 标题wstring
    */
    virtual std::string getTitle() const = 0; 

    /**
     * @brief 取得唱片集艺术家
     * @retval 唱片集艺术家wstring
    */
    virtual std::string getAlbumArtist() const = 0; 

    /**
     * @brief 取得唱片集
     * @retval 唱片集wstring
    */
    virtual std::string getAlbum() const = 0; 

    /**
     * @brief 取得艺术家集合
     * @param[in] dest 赋值目的地vector
     * @retval 是否成功
    */
    virtual bool getArtists(std::vector<std::string>& dest) const = 0; 

    /**
     * @brief 取得音轨
     * @retval 音轨号，必定为整数，为负表示获取失败
    */
    virtual int getTrack() const = 0; 

    /**
     * @brief 取得封面图片集合
     * @param[in] dest 赋值目标
     * @retval 是否取得成功
    */
    virtual bool getCovers(std::vector<Image::ptr>& dest) const = 0; 

    /**
     * @brief 封面另存为
     * @param[in] path 另存地址
     * @param[in] ifCheckSuffix 是否检查后缀，默认否
     * @param[in] index 封面序号，默认首位
     * @retval 是否另存成功
    */
    virtual bool resaveCover(const wchar_t* path, bool ifCheckSuffix = false, uint32_t index = 0) const = 0; 

    /**
     * @brief 封面另存为
     * @param[in] path 另存地址 wstring
     * @param[in] ifCheckSuffix 是否检查后缀，默认否
     * @param[in] index 封面序号，默认首位
     * @retval 是否另存成功
    */
    virtual bool resaveCover(const std::wstring& path, bool ifCheckSuffix = false, uint32_t index = 0) const = 0; 

    /**
     * @brief 检查路径后缀是否合理
     * @param[in] path 路径字符串
     * @retval 检验后结果
    */
    virtual std::wstring checkSuffix(const wchar_t* path) const = 0; 

    /**
     * @brief 另存为
     * @param[in] path 另存路径
     * @param[in] ifCheckSuffix 是否检查后缀，默认否
     * @retval 是否另存成功
    */
    virtual bool resave(const wchar_t* path, bool ifCheckSuffix = false) const = 0; 

    /**
     * @brief 另存为
     * @param[in] path 另存路径 (wstirng)
     * @param[in] ifCheckSuffix 是否检查后缀，默认否
     * @retval 是否另存成功
    */
    virtual bool resave(const std::wstring& path, bool ifCheckSuffix = false) const = 0; 

protected: 
    /// @brief 文件路径
    std::wstring m_file_path = L""; 
    /// @brief 是否有效
    bool m_isValid; 
}; 

}

#endif