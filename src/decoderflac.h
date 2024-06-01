#ifndef __MD_DECODERFLAC_H_
#define __MD_DECODERFLAC_H_

#include "decoder.h"
#include "noncopyable.h"
#include "bytearray.h"
#include "image.h"

#include <string>
#include <stdint.h>
#include <string.h>
#include <set>
#include <vector>
#include <list>
#include <unordered_map>

#define STREAMINFO_MD5_SIZE 16
#define CUESHEET_TRACK_INDEX_REVERSED_SIZE 3
#define CUESHEET_TRACK_ISRC_SIZE 12
#define CUESHEET_TRACK_REVERSED_SIZE 14
#define CUESHEET_MEDIACATALOGNUM_size 128
#define CUESHEET_REVERSED_SIZE 259

#define UINT24_MAX (uint32_t)0xFFFFFF

namespace music_data {

/**
 * @brief metadata block类
*/
class Metadata_block: Noncopyable {
public: 
    typedef std::shared_ptr<Metadata_block> ptr; 

    /**
     * @brief metadata block 类型
    */
    enum MetadataBlockType {
        STREAM_INFO = 0, 
        PADDING = 1, 
        APPLICATION = 2, 
        SEEKTABLE = 3, 
        VORBIS_COMMEN = 4, 
        CUESHEET = 5, 
        PICTURE = 6, 
        UNKNOWN_RESERVED = 7, 
        INVALID = 127
    }; 

    /**
     * @brief 创建指定类型MetaBlock
     * @param[in] data 源数据指针
     * @param[in] length 数据长度
     * @param[in] type metadata类型
     * @param[in] typeNum metadata类型号 (unknownblock用)
     * @retval Metadata_block智能指针，后续自行向下转换
    */
    static Metadata_block::ptr CreateMetadataBlock(void* data, uint32_t length, MetadataBlockType type, uint8_t typeNum = 6); 

    /**
     * @brief 构造函数
     * @param[in] length 数据字节长度
    */
    Metadata_block(uint32_t length, MetadataBlockType type, bool dataValid = true); 

    /**
     * @brief 析构函数
    */
    virtual ~Metadata_block(); 

    /**
     * @brief 返回block数据是否有效
     * @retval block数据是否有效
    */
    bool isDataValid() const { return m_dataValided; }

    /**
     * @brief 取得block type
     * @retval block type
    */
    MetadataBlockType getBlockType() const { return m_type; }

    /**
     * @brief 设置block数据是否有效
     * @param[in] val 设置值
    */
    void setDataValid(bool val) { m_dataValided = val; }

    /**
     * @brief 取得block size
     * @retval block size
    */
    virtual uint32_t getBlockSize() const = 0; 

    /**
     * @brief 数据转存
     * @param[in] data 转存目的指针
     * @retval 写入字节数
    */
    virtual uint32_t resave(void* data, bool ifLast = false) = 0; 

protected: 
    /**
     * @brief 初始化block
     * @param[in] data 数据指针
     * @param[in] length 数据长度
    */
    virtual void initBlock(void* data, uint32_t length) = 0; 


private: 
    /// @brief metablock类型
    MetadataBlockType m_type = MetadataBlockType::INVALID; 
    /// @brief block数据是否有效，true为有效
    bool m_dataValided; 
}; 

/**
 * @brief StreamInfo metablock包含整个比特流的一些信息，如采样率、声道数、采样总数等。
 * 他一定是第一个metadata而且必须有。
 * 之后可以接其他metadata，这些metadata可以不用识别直接跳过
*/
class StreamInfoMetaBlock: public Metadata_block {
public: 
    typedef std::shared_ptr<StreamInfoMetaBlock> ptr; 

    /**
     * @brief 构造函数
     * @param[in] data 数据指针
     * @param[in] length 数据字节长度
    */
    StreamInfoMetaBlock(void* data, uint32_t length); 

    /**
     * @brief 析构函数
    */
    virtual ~StreamInfoMetaBlock(); 

    /**
     * @brief 取得最小block size值
     * @retval 最小block size值
    */
    uint16_t getMinBlockSize() const { return m_minBlockSize; }

    /**
     * @brief 取得最大block size值
     * @retval 最大block size值
    */
    uint16_t getMaxBlockSize() const { return m_maxBlockSize; }

    /**
     * @brief 取得最小frame size值
     * @retval 最小frame size值
    */
    uint32_t getMinFrameSize() const { return m_minFrameSize; }

    /**
     * @brief 取得最大frame size值
     * @retval 最大frame size值
    */
    uint32_t getMaxFrameSize() const { return m_maxFrameSize; }

    /**
     * @brief 取得采样率
     * @retval 采样率
    */
    uint32_t getSampleRate() const { return m_sampleRate; }

    /**
     * @brief 取得声道数(需要+1)
     * @retval 声道数
    */
    uint8_t getChannels() const { return m_channels+1; }

    /**
     * @brief 取得采样位数(需要+1)
     * @retval 采样位数
    */
    uint8_t getSampleBits() const { return m_sampleBits+1; }

    /**
     * @brief 取得一个声道的总采样数
     * @retval 一个声道的总采样数
    */
    uint64_t getSamplePerChannel() const { return m_samplePerChannel; }

    /**
     * @brief 取得未编码时的原始信号的MD5信息
     * @param[in] dest 赋值目标指针
     * @param[in] length dest长度，必须为STREAMINFO_MD5_SIZE
    */
    bool getUnencoderedMD5(void* dest, uint32_t length) const; 

    /**
     * @brief 设置最小block size值
     * @param[in] val 设置值
    */
    void setMinBlockSize(uint16_t val) { m_minBlockSize = val; }

    /**
     * @brief 设置最大block size值
     * @param[in] val 设置值
    */
    void setMaxBlockSize(uint16_t val) { m_maxBlockSize = val; }

    /**
     * @brief 设置最小frame size值
     * @param[in] val 设置值
     * @retval 是否设置成功
    */
    bool setMinFrameSize(uint32_t val); 

    /**
     * @brief 设置最大frame size值
     * @param[in] val 设置值
     * @retval 是否设置成功
    */
    bool setMaxFrameSize(uint32_t val); 

    /**
     * @brief 设置采样率
     * @param[in] val 设置值
     * @retval 是否设置成功
    */
    bool setSampleRate(uint32_t val); 

    /**
     * @brief 设置声道数(需要-1)
     * @param[in] val 设置值
     * @retval 是否设置成功
    */
    bool setChannels(uint8_t val); 

    /**
     * @brief 设置采样位数(需要-1)
     * @param[in] val 设置值
     * @retval 是否设置成功
    */
    bool setSampleBits(uint8_t val); 

    /**
     * @brief 设置一个声道的总采样数
     * @param[in] val 设置值
     * @retval 是否设置成功
    */
    bool setSamplePerChannel(uint64_t val); 

    /**
     * @brief 设置未编码时的原始信号的MD5信息
     * @param[in] val 设置值指针
     * @param[in] length 设置值长度(必须为STREAMINFO_MD5_SIZE)
    */
    bool setUnencoderedMD5(void* val, uint32_t length); 

    virtual uint32_t getBlockSize() const override; 
    virtual uint32_t resave(void* data, bool ifLast = false) override; 

private: 
    virtual void initBlock(void* data, uint32_t length) override; 

private: 
    /// @brief 最小的block size，单位sample (16bit)
    uint16_t m_minBlockSize; 
    /// @brief 最大的block size，单位sample (16bit)
    uint16_t m_maxBlockSize; 
    /// @brief 最小的frame size，单位byte，0表示未知 (24bit)
    uint32_t m_minFrameSize; 
    /// @brief 最大的frame size，单位byte，0表示未知 (24bit)
    uint32_t m_maxFrameSize; 
    /// @brief 采样率(Hz) (20bit)
    uint32_t m_sampleRate; 
    /// @brief 声道数减一，flac支持1~8个声道 (3bit)
    uint8_t m_channels; 
    /// @brief 采样位数减一，flac支持4~32位采样位数 (5bit)
    uint8_t m_sampleBits; 
    /// @brief 一个声道的总采样数，0表示未知 (36bit)
    uint64_t m_samplePerChannel; 
    /// @brief 未编码时的原始信号的MD5信息 (128bit)
    char m_unencoderedMD5[STREAMINFO_MD5_SIZE]; 
}; 

/**
 * @brief Padding metablock 没有意义的东西，主要用来后期添加其他metadata。全设为0
*/
class PaddingMetaBlock: public Metadata_block {
public: 
    typedef std::shared_ptr<PaddingMetaBlock> ptr; 

    /**
     * @brief 构造函数
     * @param[in] length padding长度(byte)
    */
    PaddingMetaBlock(uint32_t length); 

    /**
     * @brief 析构函数
    */
    virtual ~PaddingMetaBlock(); 

    /**
     * @brief 添加padding数量，自动限制0xFFFFFF以内
     * @param[in] val 添加值
    */
    void addPaddingByte(uint32_t val) { m_blockSize=(0xFFFFFF-val>=m_blockSize)?(m_blockSize+val):0xFFFFFF; }

    /**
     * @brief 减少padding数量，自动限制>=0
     * @param[in] val 减少值
    */
    void delPaddingByte(uint32_t val) { m_blockSize=(val<=m_blockSize)?(m_blockSize-val):0; }

    virtual uint32_t getBlockSize() const override; 
    virtual uint32_t resave(void* data, bool ifLast = false) override; 

private: 
    virtual void initBlock(void* data, uint32_t length) override; 

private: 
    /// @brief padding大小byte (24bit)
    uint32_t m_blockSize; 
}; 

/**
 * @brief Application matablock 包含第三方应用软件信息，这个段里的32位识别码是flac维护组织提供的，是唯一的。
*/
class ApplicationMetaBlock: public Metadata_block {
public: 
    typedef std::shared_ptr<ApplicationMetaBlock> ptr; 

    /**
     * @brief 构造函数
     * @param[in] data 数据指针
     * @param[in] length 数据字节长度
    */
    ApplicationMetaBlock(void* data, uint32_t length); 

    /**
     * @brief 析构函数
    */
    virtual ~ApplicationMetaBlock(); 

    /**
     * @brief 取得应用程序ID
     * @retval 应用程序ID
    */
    uint32_t getAppId() const { return m_appId; }

    /**
     * @brief 取得应用程序数据
     * @param[in] dest 赋值目标指针
     * @param[in] length dest长度，判断与app data长度相同
     * @retval 是否取得成功
    */
    bool getAppData(void* dest, uint32_t length) const; 

    /**
     * @brief 设置应用程序ID
     * @param[in] val 设置值
    */
    void setAppId(uint32_t val) { m_appId = val; }

    /**
     * @brief 设置应用程序数据
     * @param[in] val 设置值
     * @param[in] length 设置值长度
    */
    void setAppData(void* val, uint8_t length); 

    virtual uint32_t getBlockSize() const override; 
    virtual uint32_t resave(void* data, bool ifLast = false) override; 

private: 
    virtual void initBlock(void* data, uint32_t length) override; 

private: 
    /// @brief 应用程序ID (32bit)
    uint32_t m_appId; 
    /// @brief 应用程序数据，必须是byte整数单位 (N bit)
    ByteArray m_appData; 
}; 

/**
 * @brief SeekTable metablock 保存快速定位点，一个点由18bytes组成（2k就可以精确到1%的定位），表里可以有任意多个定位点。
*/
class SeekTableMetaBlock: public Metadata_block {
public: 
    typedef std::shared_ptr<SeekTableMetaBlock> ptr; 
    
    /**
     * @brief 目标帧定位点
    */
    struct SeekPoint {
        /// @brief 目标帧中第一个sample的序号 (64 bit)
        uint64_t firstSampleNO; 
        /// @brief 相对于第一帧开始的偏移(byte) (64 bit)
        uint64_t offsetFromFirst; 
        /// @brief 目标帧中的采样数 (16 bit)
        uint16_t sampleNum; 

        bool operator<(const SeekPoint& cp) const {
            if (firstSampleNO<cp.firstSampleNO) {
                return true; 
            } else if (firstSampleNO==cp.firstSampleNO&&offsetFromFirst<cp.offsetFromFirst) {
                return true; 
            } else if (firstSampleNO==cp.firstSampleNO&&offsetFromFirst==cp.offsetFromFirst&&sampleNum<cp.sampleNum) {
                return true; 
            } else {
                return false; 
            }
        }
    }; 

    /**
     * @brief 构造函数
     * @param[in] data 数据指针
     * @param[in] length 数据字节长度
    */
    SeekTableMetaBlock(void* data, uint32_t length); 

    /**
     * @brief 析构函数
    */
    virtual ~SeekTableMetaBlock(); 

    /**
     * @brief 取得seekpoints数量
     * @retval seekpoints数量
    */
    uint32_t getSeekPointsLength() const { return m_seekPoints.size(); }

    /**
     * @brief 返回seekpoints值
     * @param[in] dest 以vector的形式赋值目的地
     * @retval 是否返回成功
    */
    bool getSeekPoints(std::vector<SeekPoint>& dest) const; 

    /**
     * @brief 添加seekpoint
     * @param[in] val 添加值
     * @retval 是否添加成功
    */
    bool addSeekPoint(SeekPoint& val); 

    /**
     * @brief 添加seekpoint
     * @param[in] fsn seekpoint.firstSampleNO
     * @param[in] ofs seekpoint.offsetFromFirst
     * @param[in] sn seekpoint.sampleNum
     * @retval 是否添加成功
    */
    bool addSeekPoint(uint64_t fsn, uint64_t ofs, uint16_t sn); 
    
    /**
     * @brief 删除seekpoint
     * @param[in] val 删除值
     * @retval 是否删除成功
    */
    bool delSeekPoint(SeekPoint& val); 

    /**
     * @brief 删除seekpoint
     * @param[in] fsn seekpoint.firstSampleNO
     * @param[in] ofs seekpoint.offsetFromFirst
     * @param[in] sn seekpoint.sampleNum
     * @retval 是否删除成功
    */
    bool delSeekPoint(uint64_t fsn, uint64_t ofs, uint16_t sn); 

    virtual uint32_t getBlockSize() const override; 
    virtual uint32_t resave(void* data, bool ifLast = false) override; 

private: 
    virtual void initBlock(void* data, uint32_t length) override; 

private: 
    /// @brief 目标定位点
    std::set<SeekPoint> m_seekPoints; 
}; 

/***
 * @brief 存储了一系列可读的“名/值”的键值对，使用UTF-8编码。这是flac唯一官方支持的标签段。此数据块中的数值信息使用低位字节序（小端序）表示
*/
class VorbisCommentMetaBlock: public Metadata_block {
public: 
    typedef std::shared_ptr<VorbisCommentMetaBlock> ptr; 

    /**
     * @brief 构造函数
     * @param[in] data 数据指针
     * @param[in] length 数据字节长度
    */
    VorbisCommentMetaBlock(void* data, uint32_t length); 

    /**
     * @brief 构造函数
     * @param[in] encoderIdentification 编码器标识
    */
    VorbisCommentMetaBlock(const std::string& encoderIdentification); 

    /**
     * @brief 析构函数
    */
    virtual ~VorbisCommentMetaBlock(); 

    /**
     * @brief 取得编码器标识的长度
     * @retval 编码器标识的长度
    */
    uint32_t getEncoderIdentificationLength() const { return m_encoderIdentificationLength; }

    /**
     * @brief 取得编码器标识的字符串
     * @param[in] dest 赋值目标指针
     * @param[in] length dest 长度（判断是否合理）
     * @retval 是否取得成功
    */
    bool getEncoderIdentification(void* dest, uint32_t length) const; 

    /**
     * @brief 取得编码器标识的字符串 (std::string)
     * @retval 编码器标识的字符串
    */
    std::string getEncoderIdentificationToString() const; 

    /**
     * @brief 取得标签数量
     * @retval 标签数量
    */
    uint32_t getLabelNum() const; 

    /**
     * @brief 根据key获得标签值
     * @param[in] key key值
     * @param[in] dest 返回目的vector
     * @retval 是否返回成功
    */
    bool getLabelListWithKey(const std::string& key, std::vector<std::string>& dest) const; 

    /**
     * @brief 根据key获得标签值
     * @param[in] key key值
     * @param[in] pos 第pos个value，默认第一个pos=0
     * @retval 标签值，若key或pos错误返回空字符串
    */
    std::string getLabelWithKey(const std::string& key, uint32_t pos = 0) const; 

    /**
     * @brief 设置编码器标识的字符串
     * @param[in] val 设置值
     * @retval 是否设置成功
    */
    bool setEncoderIdentification(const std::string& val); 

    /**
     * @brief 添加标签
     * @param[in] key 标签key
     * @param[in] val 标签值
     * @param[in] pos 添加值在同key的vector中位置，默认为-1，即加载末尾，其他值默认不超过最末或小于0，否则置于最后或最前
     * @retval 是否添加成功
    */
    bool addInfoLabel(const std::string& key, const std::string& val, int pos = -1); 

    /**
     * @brief 删除标签
     * @param[in] key 标签key
     * @param[in] val 标签值
     * @retval 是否删除成功
    */
    bool delInfoLabel(const std::string& key, const std::string& val); 

    /**
     * @brief 删除标签
     * @param[in] key 标签key
     * @param[in] pos 删除值位置
     * @retval 是否删除成功
    */
    bool delInfoLabel(const std::string& key, uint32_t pos); 

    /**
     * @brief 删除所有对应值标签
     * @param[in] key 标签key
     * @param[in] val 标签值
     * @retval 删除标签key-val数量
    */
    uint32_t delAllMatchInfoLabel(const std::string& key, const std::string& val); 

    /**
     * @brief 设置key对应标签值
     * @param[in] key 标签key
     * @param[in] val 设置值
     * @param[in] pos 设置值所在位置，默认第一位
     * @retval 是否修改成功: 0表示成功，1表示key不存在，2表示pos不合理，3表示val过长
    */
    int8_t setLabelVal(const std::string& key, const std::string& val, uint32_t pos = 0); 

    /**
     * @brief 重设同key中某个标签值的位置
     * @param[in] key 标签
     * @param[in] old_pos 标签原位置
     * @param[in] new_pos 标签设置位置
     * @retval 是否成功
    */
    bool resetPosLabelVal(const std::string& key, uint32_t old_pos, uint32_t new_pos); 

    /**
     * @brief 去重，同key下去重（保存第一个，不改变顺序）
     * @retval 删除值个数
    */
    uint32_t deduplication(); 

    virtual uint32_t getBlockSize() const override; 
    virtual uint32_t resave(void* data, bool ifLast = false) override; 

private: 
    virtual void initBlock(void* data, uint32_t length) override; 

private: 
    /// @brief 其后编码器标识的长度,编码器标识是标签信息数据块里的第一个字段，也是必需的字段 (32 byte) (小端)
    uint32_t m_encoderIdentificationLength; 
    /// @brief 表示编码器标识的字符串，可以随意填写，使用 UTF-8 编码 (N byte)
    char* m_encoderIdentification = nullptr; 
    /// @brief 标签键值对(标签个数32 bit, 每个标签大小占32bit，具体内容占N byte)
    std::unordered_map<std::string, std::vector<std::string>> m_infoLabels; 
}; 

/***
 * @brief 存储用在cue sheet中的各种信息。可以用来划分音轨，在备份CD时十分有用。
*/
class CuesheetMetaBlock: public Metadata_block {
public: 
    typedef std::shared_ptr<CuesheetMetaBlock> ptr; 

    /**
     * @brief cue sheet track
    */
    struct Track {
        /**
         * @brief Index of Track
        */
        struct Index {
            /// @brief 偏移，单位sample (64 bit)
            uint64_t offset; 
            /// @brief 索引号 (8 bit)
            uint8_t indexNO; 
            /// @brief Reserved (3*8 bit)
            char reserved[CUESHEET_TRACK_INDEX_REVERSED_SIZE]; 
        }; 

        /// @brief Track偏移量，单位sample (64 bit)
        uint64_t offset; 
        /// @brief Track号 (8bit)
        uint8_t trackNO; 
        /// @brief 	TrackISRC (12*8 bit)
        char trackISRC[CUESHEET_TRACK_ISRC_SIZE]; 
        
        /// @brief 6+13*8为reserved (6+13*8 bit)；第一bit为Track类型：0 音乐 1非音乐 (1bit)；第二bit为pre-emphasis标记 (1bit)
        char reserved[CUESHEET_TRACK_REVERSED_SIZE]; 

        /// @brief 索引数目 标准 CD 音频中，单轨索引数量最大为 100 (8bit)
        uint8_t indexNum; 
        /// @brief 多个INDEX
        std::vector<Index> indexs; 
    }; 

    /**
     * @brief 构造函数
     * @param[in] data 数据指针
     * @param[in] length 数据字节长度
    */
    CuesheetMetaBlock(void* data, uint32_t length); 

    /**
     * @brief 析构函数
    */
    virtual ~CuesheetMetaBlock(); 

    /**
     * @brief 取得媒体目录号
     * @retval 媒体目录号 string
    */
    std::string getMediaCatalogNumber() const { return std::string(m_mediaCatalogNumber); }

    /**
     * @brief 取得引导sample的个数
     * @retval 引导sample的个数
    */
    uint64_t getGuidingSampleNum() const { return m_guidingSampleNum; }

    /**
     * @brief 取得Reserved，m_reserved中1~259 (用于判断是否合法)
     * @param[in] dest 赋值目标指针
     * @param[in] length dest长度，用于判断长度是否合适
     * @retval Reserved string
    */
    bool getReserved(void* dest, uint16_t length) const; 

    /**
     * @brief 取得Track的个数
     * @retval Track的个数
    */
    uint8_t getTrackNum() const; 

    /**
     * @brief 取得所有TRACK
     * @param[in] dest 赋值目的vector
     * @retval 取得track数量
    */
    uint8_t getTracks(std::vector<Track>& dest) const; 

    /**
     * @brief 是否对应一个Compact Disc
     * @retval 是否对应一个Compact Disc
    */
    bool isCompactDisc() const { return (m_reserved[0]>>7)==1; }

    /**
     * @brief 设置是否对应一个Compact Disc
     * @param[in] val 设置值
    */
    void setIsCompactDisc(bool val) { m_reserved[0]=val?(m_reserved[0]|0x80):(m_reserved[0]&0x7F); }

    virtual uint32_t getBlockSize() const override; 
    virtual uint32_t resave(void* data, bool ifLast = false) override; 
    
private: 
    virtual void initBlock(void* data, uint32_t length) override; 

private: 
    /// @brief 媒体目录号，ASCII为0x20-0x7e, 若不足128字节，应用空字符（0x00）在后方补足 (128*8 bit)
    char m_mediaCatalogNumber[CUESHEET_MEDIACATALOGNUM_size]; 
    /// @brief 引导sample的个数 (64 bit)
    uint64_t m_guidingSampleNum; 
    /// @brief 7+258*8为Reserved, 保留区域，此区域所有数位必须为0 (7+258*8 bit), 第一bit为1表示CUESHEET对应一个Compact Disc (1 bit)
    char m_reserved[CUESHEET_REVERSED_SIZE]; 

    /// @brief 多个TRACK (Track个数占8 bit，标准 CD 音频最大音轨数为 100)
    std::vector<Track> m_tracks; 
}; 

/**
 * @brief 保存相关图片，同时还有url、分辨率等信息，可以有不止一个picture block
*/
class PictureMetaBlock: public Metadata_block {
public: 
    typedef std::shared_ptr<PictureMetaBlock> ptr; 

    /**
     * 图片类型
    */
    enum PictureType {
        OTHER = 0, 
        PNG_ICON = 1, 
        OTHER_ICON = 2, 
        FRONT_COVER = 3, 
        BACK_COVER = 4, 
        LEAFLET = 5, 
        MEDIA = 6, 
        LEAD_ARTIST = 7, 
        ARTIST = 8, 
        CONDUCTOR = 9, 
        BAND = 10, 
        COMPOSER = 11, 
        LYRICIST = 12, 
        RECORDING_STUDIO_OR_LOCATION = 13, 
        RECORDING_SESSION = 14, 
        PERFORMANCE = 15, 
        CAPTURE_FROM_MOVIE_OR_VEDIO = 16, 
        BRIGHT_COLORED_FISH = 17, 
        ILLUSTRATION = 18, 
        BAND_LOGO = 19, 
        PUBLISHER_LOGO = 20
    }; 

    /**
     * @brief 构造函数
     * @param[in] data 数据指针
     * @param[in] length 数据字节长度
    */
    PictureMetaBlock(void* data, uint32_t length); 

    /**
     * @brief 构造函数
     * @param[in] img 图片数据
    */
    PictureMetaBlock(const Image& img); 

    /**
     * @brief 析构函数
    */
    virtual ~PictureMetaBlock(); 

    /**
     * @brief 取得图片类型
     * @retval 图片类型
    */
    PictureType getPictureType() const { return m_pictureType; }
    
    /**
     * @brief 取得MIME类型字符长度
     * @retval MIME类型字符长度
    */
    uint32_t getMimeLength() const { return m_mimeLength; }
    
    /**
     * @brief 取得MIME类型
     * @retval MIME类型 string
    */
    std::string getMimeTypeToString() const; 

    /**
     * @brief 取得MIME类型
     * @param[in] dest 赋值目的地指针
     * @param[in] length 赋值指针长度
     * @retval 是否取得成功
    */
    bool getMimeType(void* dest, uint32_t length) const; 

    /**
     * @brief 取得描述符长度
     * @retval 描述符长度
    */
    uint32_t getDescriptorLength() const { return m_descriptorLength; }

    /**
     * @brief 取得描述符
     * @retval 描述符 string
    */
    std::string getDescriptorToString() const; 
    
    /**
     * @brief 取得描述符
     * @param[in] dest 赋值目的地指针
     * @param[in] length 赋值指针长度
     * @retval 是否取得成功
    */
    bool getDescriptor(void* dest, uint32_t length) const; 

    /**
     * @brief 取得图片宽度
     * @retval 图片宽度
    */
    uint32_t getPictureWidth() const { return m_pictureWidth; }
    
    /**
     * @brief 取得图片高度
     * @retval 图片高度
    */
    uint32_t getPictureHeight() const { return m_pictureHeight; }
    
    /**
     * @brief 取得图片颜色深度
     * @retval 图片颜色深度
    */
    uint32_t getPictureColorDepth() const { return m_pictureColorDepth; }
    
    /**
     * @brief 取得索引图使用的颜色数目
     * @retval 索引图使用的颜色数目
    */
    uint32_t getPictureIndexColorNum() const { return m_pictureIndexColorNum; }
    
    /**
     * @brief 取得图片数据长度
     * @retval 图片数据长度
    */
    uint32_t getPictureDataLength() const { return m_pictureData.getSize(); }
    
    /**
     * @brief 取得图片数据
     * @param[in] dest 赋值目的地指针
     * @param[in] length 赋值指针长度
     * @retval 是否取得成功
    */
    bool getPictureData(void* dest, uint32_t length) const; 

    /**
     * @brief 设置图片类型
     * @param[in] val 设置值
    */
    void setPictureType(PictureType val) { m_pictureType = val; }; 

    /**
     * @brief 设置描述符
     * @param[in] val 设置值
     * @retval 是否设置成功
    */
    bool setDescriptor(const std::string& val); 

    /**
     * @brief 设置图片数据
     * @param[in] img 图片信息
     * @retval 是否设置成功
    */
    bool setPicture(const Image& img); 

    virtual uint32_t getBlockSize() const override; 
    virtual uint32_t resave(void* data, bool ifLast = false) override; 

private: 
    virtual void initBlock(void* data, uint32_t length) override; 

private: 
    /// @brief 图片类型（同ID3v2 APIC） (32 bit)
    PictureType m_pictureType = FRONT_COVER; 
    /// @brief MIME类型字符长度（byte） (32 bit)
    uint32_t m_mimeLength; 
    /// @brief MIME类型(字符串) (N*8 bit)
    char* m_mimeType = nullptr; 
    /// @brief 描述符长度（byte） (32 bit)
    uint32_t m_descriptorLength; 
    /// @brief 描述符(字符串)UTF-8 (N*8 bit)
    char* m_descriptor = nullptr; 
    /// @brief 图片宽度 (32 bit)
    uint32_t m_pictureWidth; 
    /// @brief 图片高度 (32 bit)
    uint32_t m_pictureHeight; 
    /// @brief 图片的色位深度，单个像素表示颜色使用的总位数 (32 bit)
    uint32_t m_pictureColorDepth; 
    /// @brief 索引图使用的颜色数目，0非索引图 (32 bit)
    uint32_t m_pictureIndexColorNum; 
    /// @brief 图片数据 (数据长度占32bit, 具体数据N*8 bit)
    ByteArray m_pictureData; 
}; 

/**
 * @brief 未知metablock，BLOCK TYPE 7~126 reserved
*/
class UnknownMetaBlock: public Metadata_block {
public: 
    typedef std::shared_ptr<UnknownMetaBlock> ptr; 

    /**
     * @brief 构造函数
     * @param[in] data 数据指针
     * @param[in] length 数据字节长度
    */
    UnknownMetaBlock(void* data, uint32_t length, uint8_t typeNum); 

    /**
     * @brief 析构函数
    */
    virtual ~UnknownMetaBlock(); 

    /**
     * @brief 取得block数据
     * @param[in] dest 赋值目的地指针
     * @param[in] length 赋值指针长度
     * @retval 是否取得成功
    */
    bool getData(void* dest, uint32_t length) const; 

    virtual uint32_t getBlockSize() const override; 
    virtual uint32_t resave(void* data, bool ifLast = false) override; 

private: 
    virtual void initBlock(void* data, uint32_t length) override; 

private: 
    /// @brief data大小
    uint32_t m_length = 0; 
    /// @brief block数据
    char* m_data = nullptr; 
    /// @brief block类型号
    uint8_t m_typeNum; 
}; 

/**
 * @brief Invalid metablock，BLOCK TYPE 127
*/
class InvalidMetaBlock: public Metadata_block {
public: 
    typedef std::shared_ptr<InvalidMetaBlock> ptr; 

    /**
     * @brief 构造函数
     * @param[in] data 数据指针
     * @param[in] length 数据字节长度
    */
    InvalidMetaBlock(void* data, uint32_t length); 

    /**
     * @brief 析构函数
    */
    virtual ~InvalidMetaBlock(); 

    /**
     * @brief 取得block数据
     * @param[in] dest 赋值目的地指针
     * @param[in] length 赋值指针长度
     * @retval 是否取得成功
    */
    bool getData(void* dest, uint32_t length) const; 
    
    virtual uint32_t getBlockSize() const override; 
    virtual uint32_t resave(void* data, bool ifLast = false) override; 

private: 
    virtual void initBlock(void* data, uint32_t length) override; 

private: 
    /// @brief data大小
    uint32_t m_length = 0; 
    /// @brief block数据
    char* m_data = nullptr; 
}; 

/**
 * @brief flac文件解码数据类
*/
class MusicDecoderflac: public MusicDecoder {
public: 
    typedef std::shared_ptr<MusicDecoderflac> ptr; 

    /**
     * @brief 默认构造函数
    */
    MusicDecoderflac(); 

    /**
     * @brief 带路径参数构造函数
     * @param[in] file_path 文件路径
    */
    MusicDecoderflac(std::wstring& file_path); 

    /**
     * @brief 带路径参数构造函数
     * @param[in] file_path 文件路径
    */
    MusicDecoderflac(const wchar_t* file_path); 

    /**
     * @brief 析构函数
    */
    virtual ~MusicDecoderflac(); 

    /**
     * @brief 添加metadata block
     * @param[in] data metadata block 导入数据
     * @param[in] length 导入数据长度
     * @param[in] type metadata block 类型
     * @retval 添加的是否为有效block
    */
    bool addMetaDataBlock(void* data, uint32_t length, uint8_t type); 

    /**
     * @brief 添加VorbisCommentMetaBlock
     * @param[in] encoderIdentification 新VorbisCommentMetaBlock的编码标识，默认为"f"
     * @retval 是否添加成功
    */
    bool addVorbisCommentMetaBlock(const std::string& encoderIdentification = "f"); 

    /**
     * @brief 返回streamInfo指针
     * @retval streamInfo指针
    */
    StreamInfoMetaBlock::ptr getStreamInfo() const { return m_streamInfo; }

    /**
     * @brief 返回padding指针
     * @retval padding指针
    */
    PaddingMetaBlock::ptr getPadding() const { return m_padding; }

    /**
     * @brief 返回Application指针
     * @retval Application指针
    */
    ApplicationMetaBlock::ptr getApplication() const { return m_application; }

    /**
     * @brief 返回SeekTable指针
     * @retval SeekTable指针
    */
    SeekTableMetaBlock::ptr getSeekTable() const { return m_seekTable; }

    /**
     * @brief 返回vorbisComment指针
     * @retval vorbisComment指针
    */
    VorbisCommentMetaBlock::ptr getVorbisComment() const { return m_vorbisComment; }

    /**
     * @brief 返回Cuesheet指针
     * @retval Cuesheet指针
    */
    CuesheetMetaBlock::ptr getCuesheet() const { return m_cuesheet; }

    /**
     * @brief 取得Picture vector
     * @param[in] dest 赋值目标vector
     * @retval 是否成功取得
    */
    bool getPictures(std::vector<PictureMetaBlock::ptr>& dest) const; 

    /**
     * @brief 设置VorbisComment的标记值
     * @param[in] key 标签key
     * @param[in] val 标签设置值
     * @param[in] pos 设置值相对位置，默认为0
     * @retval 是否成功
    */
    bool setVorbisCommentLabel(const std::string& key, const std::string& val, uint32_t pos = 0); 

public: 
    virtual bool setbackTitle(const std::string& val) override; 
    virtual bool setbackAlbumArtist(const std::string& val) override; 
    virtual bool setbackAlbum(const std::string& val) override; 
    virtual bool setbackArtist(const std::string& val, uint32_t pos) override; 
    virtual bool addbackArtist(const std::string& val, int pos = -1) override; 
    virtual bool delbackArtist(const std::string& val) override; 
    virtual bool delbackArtist(uint32_t index) override; 
    virtual bool resetPosbackArtist(uint32_t old_pos, uint32_t new_pos) override; 
    virtual bool setbackTrack(uint8_t val) override; 
    virtual bool setbackCover(const Image& img, uint32_t pos = 0) override; 
    virtual bool addbackCover(const Image& img, uint32_t pos = 0) override; 
    virtual bool delbackCover(uint32_t pos = 0) override; 
    virtual bool resetPosbackCover(uint32_t old_pos, uint32_t new_pos) override; 
    virtual std::string getTitle() const override; 
    virtual std::string getAlbumArtist() const override; 
    virtual std::string getAlbum() const override; 
    virtual bool getArtists(std::vector<std::string>& dest) const override; 
    virtual int getTrack() const override; 
    virtual bool getCovers(std::vector<Image::ptr>& dest) const override; 
    virtual bool resaveCover(const wchar_t* path, bool ifCheckSuffix = false, uint32_t index = 0) const override; 
    virtual bool resaveCover(const std::wstring& path, bool ifCheckSuffix = false, uint32_t index = 0) const override; 

    virtual std::wstring checkSuffix(const wchar_t* path) const override; 
    virtual bool resave(const wchar_t* path, bool ifCheckSuffix = false) const override; 
    virtual bool resave(const std::wstring& path, bool ifCheckSuffix = false) const override; 

protected: 
    virtual void initData(void* data, size_t length) override; 

private: 
    /**
     * @brief 判断是否为flac文件标记
     * @retval 是否为flac文件标记
    */
    bool isFlac(char* label) { return strcmp(label, s_label_flac)==0; }

private: 
    /// @brief flac文件标记常量(长度为5，最后包括'\0')
    static constexpr char s_label_flac[] = "fLaC"; 

    bool isValid = true; 

    /// @brief STREAMINFO：包含整个比特流的一些信息，如采样率、声道数、采样总数等。他一定是第一个metadata而且必须有。
    StreamInfoMetaBlock::ptr m_streamInfo = nullptr; 
    /// @brief 没有意义的东西，主要用来后期添加其他metadata。
    PaddingMetaBlock::ptr m_padding = nullptr; 
    /// @brief 包含第三方应用软件信息，这个段里的32位识别码是flac维护组织提供的，是唯一的。
    ApplicationMetaBlock::ptr m_application = nullptr; 
    /// @brief 保存快速定位点，一个点由18bytes组成（2k就可以精确到1%的定位），表里可以有任意多个定位点。
    SeekTableMetaBlock::ptr m_seekTable = nullptr; 
    /// @brief 存储了一系列可读的“名/值”的键值对，使用UTF-8编码。这是flac唯一官方支持的标签段。
    VorbisCommentMetaBlock::ptr m_vorbisComment = nullptr; 
    /// @brief 存储用在cue sheet中的各种信息。可以用来划分音轨，在备份CD时十分有用。
    CuesheetMetaBlock::ptr m_cuesheet = nullptr; 
    /// @brief 保存相关图片，同时还有url、分辨率等信息，可以有不止一个picture block。
    std::vector<PictureMetaBlock::ptr> m_pictures; 
    /// @brief 未知reserved metablocks
    std::list<UnknownMetaBlock::ptr> m_unknownReservedData; 
    /// @brief Invalid metablocks
    std::list<InvalidMetaBlock::ptr> m_invalidData; 

    /// @brief 暂存flac的audio frames数据（无解码）（解码板有缘更新）
    ByteArray m_audioFrames; 
    /// @brief flac的audio frames数据（无解码）数据长度(byte)
    size_t m_audioFramesLength; 
}; 

}

#endif