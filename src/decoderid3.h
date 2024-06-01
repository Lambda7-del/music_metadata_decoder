#ifndef __MD_DECODERID3_H_
#define __MD_DECODERID3_H_

#include <memory>
#include <stdint.h>
#include <string.h>
#include <string>
#include <vector>

#define ID3V1_TITLE_SIZE 30
#define ID3V1_ARTIST_SIZE 30
#define ID3V1_ALBUM_SIZE 30
#define ID3V1_YEAR_SIZE 4
#define ID3V1_COMMENT_SIZE 28
#define ID3V2_FRAMEID_SIZE 4

namespace music_data {

/***
 * @brief ID3Tag类
*/
class ID3Tag {
public: 
    typedef std::shared_ptr<ID3Tag> ptr; 

    /**
     * @brief ID3Tag类型枚举
    */
    enum ID3TagType {
        ID3V1 = 0, 
        ID3V2 = 1
    }; 

    /**
     * @brief 尝试创建ID3Tag
     * @param[in] 数据指针
     * @param[in] 数据长度
     * @retval 成功返回智能指针，失败返回空指针
    */
    static ID3Tag::ptr TryCreateID3Tag(void* data, size_t length);  

    /**
     * @brief 构造函数
     * @param[in] type tag版本类型
     * @param[in] datasize Tag数据长度
     * @param[in] dataValid tag数据是否有效
    */
    ID3Tag(ID3TagType type, uint32_t datasize, bool dataValid = true); 
    
    /**
     * @brief 析构函数
    */
    virtual ~ID3Tag(); 

    /**
     * @brief 返回tag数据是否有效
     * @retval tag数据是否有效
    */
    bool isDataValid() const { return m_dataValided; }

    /**
     * @brief 取得tag size
     * @retval tag size
    */
    uint32_t getTagSize() const { return m_size; }

    /**
     * @brief 取得tag type
     * @retval tag type
    */
    ID3TagType getTagType() const { return m_tagType; }

    /**
     * @brief 设置tag数据是否有效
     * @param[in] val 设置值
    */
    void setDataValid(bool val) { m_dataValided = val; }

protected: 
    /**
     * @brief 设置tag size
     * @param[in] val 设置值
    */
    void setTagSize(uint32_t val) { m_size = val; }

    /**
     * @brief 初始化Tag数据
     * @param[in] data 数据指针
    */
    virtual void initTag(void* data) = 0; 

private: 
    /// @brief ID3Tag版本类型
    ID3TagType m_tagType; 
    /// @brief 标签长度 (单位byte)
    uint32_t m_size; 
    /// @brief Tag数据是否有效，true为有效
    bool m_dataValided; 
}; 

/**
 * @brief ID3 v1数据，默认为v1.1
*/
class ID3v1: public ID3Tag {
public: 
    typedef std::shared_ptr<ID3v1> ptr; 

    /**
     * @brief 构造函数
     * @param[in] data 数据指针
     * @param[in] size 数据大小
    */
    ID3v1(void* data, uint32_t size); 

    /**
     * @brief 析构函数
    */
    virtual ~ID3v1(); 

    /**
     * @brief 取得标题
     * @retval 标题
    */
    std::string getTitle() const { return std::string(m_title); }

    /**
     * @brief 取得艺术家
     * @retval 艺术家
    */
    std::string getArtist() const { return std::string(m_artist); }

    /**
     * @brief 取得专辑
     * @retval 专辑
    */
    std::string getAlbum() const { return std::string(m_album); }
    
    /**
     * @brief 取得年份
     * @retval 年份
    */
    std::string getYear() const { return std::string(m_year); }

    /**
     * @brief 取得备注
     * @retval 备注
    */
    std::string getComment() const { return std::string(m_comment); }

    /**
     * @brief 取得保留位
     * @retval 保留位
    */
    uint8_t getReserved() const { return m_reserved; }

    /**
     * @brief 取得音轨
     * @retval 音轨
    */
    uint8_t getTrackNum() const { return m_trackNum; }

    /**
     * @brief 取得类型和曲风编码
     * @retval 类型和曲风编码
    */
    uint8_t getGenre() const { return m_genre; }

    /**
     * @brief 设置标题
     * @param[in] val 设置值
    */
    void setTitle(const char* val) { strncpy(m_title, val, ID3V1_TITLE_SIZE); }

    /**
     * @brief 设置艺术家
     * @param[in] val 设置值
    */
    void setArtist(const char* val) { strncpy(m_artist, val, ID3V1_ARTIST_SIZE); }

    /**
     * @brief 设置专辑
     * @param[in] val 设置值
    */
    void setAlbum(const char* val) { strncpy(m_album, val, ID3V1_ALBUM_SIZE); }

    /**
     * @brief 设置年份
     * @param[in] val 设置值
    */
    void setYear(const char* val) { strncpy(m_year, val, ID3V1_YEAR_SIZE); }

    /**
     * @brief 设置备注
     * @param[in] val 设置值
    */
    void setComment(const char* val) { strncpy(m_comment, val, ID3V1_COMMENT_SIZE); }

    /**
     * @brief 设置音轨
     * @param[in] val 设置值
    */
    void setTrackNum(uint8_t val) { m_trackNum = val; }

    /**
     * @brief 设置类型和曲风编码
     * @param[in] val 设置值 (0~147)
    */
    void setGenre(uint8_t val) { m_genre = (val<=147)?val:m_genre; }
    
protected: 
    virtual void initTag(void* data) override; 

private: 
    /**
     * @brief 判断是否为ID3v1文件标记
     * @retval 是否为ID3v1文件标记
    */
    bool isID3v1(char* label) { return strcmp(label, s_header)==0; }

private: 
    /// @brief ID3V1标识 (3 byte)
    static constexpr char s_header[] = "TAG"; 

    /// @brief 标题 (30 byte)
    char m_title[ID3V1_TITLE_SIZE]; 
    /// @brief 艺术家 (30 byte)
    char m_artist[ID3V1_ARTIST_SIZE]; 
    /// @brief 专辑 (30 byte)
    char m_album[ID3V1_ALBUM_SIZE]; 
    /// @brief 年份 (4 byte)
    char m_year[ID3V1_YEAR_SIZE]; 
    /// @brief 备注 (28 byte)
    char m_comment[ID3V1_COMMENT_SIZE]; 
    /// @brief 保留位，说明有无音轨 (1 byte)，若为0说明后面一位为音轨，否则为comment的一部分
    uint8_t m_reserved; 
    /// @brief 音轨 (1 byte)
    uint8_t m_trackNum; 
    /// @brief 类型和曲风编码，其中0~79是标准的Genre，80至147是扩展的Genre (1 byte)
    uint8_t m_genre; 
}; 

class ID3v2: public ID3Tag {
public: 
    typedef std::shared_ptr<ID3v2> ptr; 

    /**
     * @brief ID3v2 header
    */
    struct TagHeader {
        /// @brief ID3v2 Identifier (3 byte)
        static constexpr char s_identifier[] = "ID3"; 

        /// @brief ID3v2 version, 第一个字节表示主版本号，第二个字节表示修订号 (2 byte)
        uint16_t version; 
        /**
         * @brief ID3v2 Flags (1 byte), 其中只有三位是有意义的 %abc00000
         * a- 标签头标识(flags)的第7位用来表示当前ID3v2信息是否经过非同步编码。当这位为1时，表示ID3v2信息经过非同步编码。
         * b- 标签头标识(flags)的第6位用来表示标签头数据后面是否有扩展标签头数据。
         * c- 标签头标识(flags)的第5位用来表示当前是否为测试版。如果有测试阶段，请将此位置1。
         * 标签头标识(flags)的其它位统统要设为0。如果其中一位为1，那么程序可能不能正确识别该标签。
        */
        uint8_t flags; 
        /**
         * @brief ID3v2 Size (4 byte) 但这四个字节的最高位都设为0。所以总共有28位来表示大小
         * 这里说的大小是ID3v2信息经过非同步编码，包括扩展标签头(extended header),补白数据(padding)，但不包括标签头的大小，也就是整个ID3v2标签信息大小减去10。
         * 只用28位(256MB)是为了避免与MP3的同步信号冲突。
        */
        uint32_t size; 
    }; 

    /**
     * @brief ID3v2 extended header
    */
    struct TagExtendedHeader {
        /// @brief Extended Header Size 扩展标签头大小，现在可以是6或10个字节，不包括它自己 (4 byte)
        uint32_t extendedHeaderSize; 
        /**
         * @brief Extended Flags (2 byte)
         * 如果第一位为1，那扩展标签头后就会有4个字节的CRC-32数据。
         * CRC是根据扩展标签头与补白数据之间的数据计算出来的，也就是根据所有的帧计算出来的。
        */
        uint16_t extendedFlags; 
        /// @brief Size of padding 字节，不包括它自己。补白大小就是整个ID3v2标签信息的大小减去标签头(header)和帧(frame)的大小，换言之就是补白的大小 (4 byte)
        uint32_t sizeofPadding; 
    }; 

    struct TagFrame {
        /**
         * @brief 构造函数
         * @param[in] data 数据指针
        */
        TagFrame(void* data); 

        /**
         * @brief 析构函数
        */
        ~TagFrame(); 

        /// @brief Frame ID such as "AENC"/"APIC"/"COMM"/... (4 byte)
        char frameId[ID3V2_FRAMEID_SIZE]; 
        /// @brief Frame Size 帧大小是用帧的总大小减去帧头大小计算出来的(data 以外, 帧的总大小-10) (4 byte)
        uint32_t frameSize; 
        /**
         * @brief 帧标识
         * 帧标识的第一个字节是状态信息，第二个字节是为了编码用的。
         * 如果在第一字节中未定义的标识位被置1，那么帧的内容是不可以修改的，除非那位标识位清零。
         * 如果在第二字节中有未定义的标识位被置1，那么帧的内容可能根本就不可读。
         * 定义为：%abc00000 %ijk00000, abc ijk为可读
         * a - 标签改变保留位, 告诉软件当在标签信息改变时如何处理当前帧, 0保留，1丢弃
         * b - 文件改变保留位, 告诉软件当文件（不包括标签信息）改变时如何处理当前帧, , 0保留，1丢弃
         * c - 只读, 将这位置1则告诉软件本帧内容是只读的。
         * i - 压缩, 这位标识位表示当前帧是否经过压缩。0帧未经过压缩, 1帧用[#ZLIB zlib]压缩，在帧头的后面有4个字节的解压缩大小
         * j - 加密, 当前帧是否经过加密。如果这位置1，则在帧头后面会有一个字节的数据表示是用哪种方式加密的。
         * k - 分组标识, 该标识位表示当前帧是否隶属于其它帧下的分组。0帧内不包含有分组标识信息, 1帧内包含有分组标识信息。
        */
        uint16_t flags; 
        /// @brief 帧数据
        char* data = nullptr; 
    }; 

    /**
     * @brief 构造函数
     * @param[in] data 数据指针
     * @param[in] length 数据长度
    */
    ID3v2(void* data, uint32_t length); 
    
    /**
     * @brief 析构函数
    */
    virtual ~ID3v2(); 

protected: 
    virtual void initTag(void* data) override; 

private: 
    /**
     * @brief 判断是否为ID3v2文件标记
     * @retval 是否为ID3v2文件标记
    */
    bool isID3v2(char* label) { return strcmp(label, TagHeader::s_identifier)==0; }

private: 
    /// @brief header
    TagHeader m_header; 
    /// @brief extended header
    std::shared_ptr<TagExtendedHeader> m_extendedHeader = nullptr; 
    /// @brief 所有帧集合
    std::vector<TagFrame> m_frames; 
}; 

}

#endif