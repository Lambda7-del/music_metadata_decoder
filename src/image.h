#ifndef _MD_IMAGE_H_
#define _MD_IMAGE_H_

#include "bytearray.h"

#include <string.h>
#include <unordered_map>

namespace music_data {

class Image; 

/**
 * @brief 图片类型
*/
enum ImageType {
    JPEG = 0, 
    PNG = 1, 
    UNKNOW
}; 

struct EnumClassHash
{
    template <typename T>
    std::size_t operator()(T t) const
    {
        return static_cast<std::size_t>(t);
    }
};

// image type对应的mime type字符串
static const std::unordered_map<ImageType, std::string, EnumClassHash> s_mimeStr = {
    {PNG, "image/png"},
    {JPEG, "image/jpeg"} 
}; 

/**
 * @brief 图像数据（能用就行，功能不多）
*/
class Image {
public: 
    typedef std::shared_ptr<Image> ptr; 

    /**
     * @brief 尝试创建Image
     * @param[in] data 源数据
     * @param[in] length 数据长度
     * @param[in] mimeType 图片mimeType，用于优先判断图片数据属于哪种格式
     * @retval Image智能指针，创建失败返回nullptr
    */
    static ptr TryCreateImage(void* data, size_t length, const std::string& mimeType = ""); 

    /**
     * @brief 根据ImageType返回mime类型字符串
     * @param[in] type ImageType
     * @retval mime type字符串
    */
    static std::string getMimeTyepFromImageType(ImageType type) { return s_mimeStr.at(type); }

    /**
     * @brief 构造函数
    */
    Image(); 

    /**
     * @brief 析构函数
    */
    ~Image(); 

     /**
     * @brief 加载音频文件
     * @param[in] file_path 文件路径
     * @retval 是否成功打开
    */
    bool openFile(const wchar_t* file_path); 

    /**
     * @brief 加载音频文件
     * @param[in] file_path 文件路径
     * @retval 是否成功打开
    */
    bool openFile(const std::wstring& file_path); 

    /**
     * @brief 返回数据是否有效
     * @retval 数据是否有效
    */
    bool isValid() const { return m_isValid; }

    /**
     * @brief 返回width
     * @retval width
    */
    uint32_t getWidth() const { return m_width; }

    /**
     * @brief 返回height
     * @retval height
    */
    uint32_t getHeight() const { return m_height; }

    /**
     * @brief 返回ColorDepthBit
     * @retval ColorDepthBit
    */
    uint32_t getColorDepthBit() const { return m_colorDepthBit; }

    /**
     * @brief 返回图片类型
     * @retval 图片类型
    */
    ImageType getType() const { return m_type; }

    /**
     * @brief 返回数据大小
     * @retval 数据大小
    */
    size_t getDataSize() const { return m_data.getSize(); }

    /**
     * @brief 获取图片源数据
     * @param[in] dest 获取数据目的地
     * @param[in] length dest长度
     * @retval 是否获取成功
    */
    bool getData(void* dest, size_t length) const; 

    /**
     * @brief 另存为
     * @param[in] path 另存路径
     * @param[in] ifCheckSuffix 是否检查文件路径后缀，默认false，设为true则自动添加或修改后缀
     * @retval 是否另存成功
    */
    bool resave(const wchar_t* path, bool ifCheckSuffix = false) const; 

    /**
     * @brief 另存为
     * @param[in] path 另存路径（wstring）
     * @param[in] ifCheckSuffix 是否检查文件路径后缀，默认false，设为true则自动添加或修改后缀
     * @retval 是否另存成功
    */
    bool resave(const std::wstring& path, bool ifCheckSuffix = false) const; 

protected: 
    /**
     * @brief 设置是否有效
     * @param[in] val 设置值
    */
    void setIsValid(bool val) { m_isValid = val; }

    /**
     * @brief 设置图片类型
     * @param[in] val 设置值
    */
    void setType(ImageType val) { m_type = val; }

    /**
     * @brief 初始化数据
     * @param[in] data 源数据
     * @param[in] length 数据长度
    */
    virtual void initImage(void* data, size_t length) = 0; 

    /**
     * @brief 检查路径后缀是否合理
     * @param[in] path 路径字符串
     * @retval 检验后结果
    */
    virtual std::wstring checkSuffix(const wchar_t* path) const = 0; 

protected: 
    /// @brief 数据路径（若是从文件打开）
    std::wstring m_path = L""; 
    /// @brief 类型
    ImageType m_type = UNKNOW; 
    /// @brief 图像源数据
    ByteArray m_data; 
    /// @brief 图像width
    uint32_t m_width = 0; 
    /// @brief 图像height
    uint32_t m_height = 0; 
    /// @brief 图片的色位深度，单个像素表示颜色使用的总位数 (32 bit)
    uint32_t m_colorDepthBit = 0; 

private: 
    /// @brief 是否有效
    bool m_isValid = true; 
}; 

/**
 * @brief png图像
*/
class PngImage: public Image {
public: 
    typedef std::shared_ptr<PngImage> ptr; 

    /**
     * @brief 构造函数
     * @param[in] data 数据
     * @param[in] length 数据长度
    */
    PngImage(void* data, size_t length); 

    /**
     * @brief 构造函数
     * @param[in] data 数据
     * @param[in] length 数据长度
    */
    PngImage(const wchar_t* path); 

    /**
     * @brief 构造函数
     * @param[in] data 数据
     * @param[in] length 数据长度
    */
    PngImage(const std::wstring& path); 

    /**
     * @brief 析构函数
    */
    ~PngImage(); 

    virtual void initImage(void* data, size_t length) override; 
    virtual std::wstring checkSuffix(const wchar_t* path) const override; 

private: 
    bool isPng(void* label) const { return memcmp(label, s_png_label, 8)==0; }

private: 
    /// @brief png文件标识
    static constexpr uint8_t s_png_label[] = {0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a}; 
}; 

/**
 * @brief jpeg图像
*/
class JpegImage: public Image {
public: 
    typedef std::shared_ptr<JpegImage> ptr; 

    /**
     * @brief 构造函数
     * @param[in] data 数据
     * @param[in] length 数据长度
    */
    JpegImage(void* data, size_t length); 

    /**
     * @brief 构造函数
     * @param[in] data 数据
     * @param[in] length 数据长度
    */
    JpegImage(const wchar_t* path); 

    /**
     * @brief 构造函数
     * @param[in] data 数据
     * @param[in] length 数据长度
    */
    JpegImage(const std::wstring& path); 

    /**
     * @brief 析构函数
    */
    ~JpegImage(); 

    virtual void initImage(void* data, size_t length) override; 
    virtual std::wstring checkSuffix(const wchar_t* path) const override; 

private: 
    bool isJpeg(void* label) const { return memcmp(label, s_jpeg_label, 2)==0; }; 

private: 
    /// @brief png文件标识
    static constexpr uint8_t s_jpeg_label[] = {0xFF, 0xd8}; 
}; 

}

#endif