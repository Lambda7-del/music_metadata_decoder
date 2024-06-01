#include "decoderflac.h"
#include "utils.h"
#include "log.h"

#include <stdlib.h>
#include <algorithm>
#include <unordered_set>
#include <fileapi.h>
#include <Windows.h>

namespace music_data {

INITONLYLOGGER(); 

Metadata_block::ptr Metadata_block::CreateMetadataBlock(void* data, uint32_t length, MetadataBlockType type, uint8_t typeNum) {
    switch (type) {
        case STREAM_INFO: {
            Metadata_block::ptr ans(new StreamInfoMetaBlock(data, length)); 
            return ans; 
        }
        case PADDING: {
            Metadata_block::ptr ans(new PaddingMetaBlock(length)); 
            return ans; 
        }
        case APPLICATION: {
            Metadata_block::ptr ans(new ApplicationMetaBlock(data, length)); 
            return ans; 
        }
        case SEEKTABLE: {
            Metadata_block::ptr ans(new SeekTableMetaBlock(data, length)); 
            return ans; 
        }
        case VORBIS_COMMEN: {
            Metadata_block::ptr ans(new VorbisCommentMetaBlock(data, length)); 
            return ans; 
        }
        case CUESHEET: {
            Metadata_block::ptr ans(new CuesheetMetaBlock(data, length)); 
            return ans; 
        }
        case PICTURE: {
            Metadata_block::ptr ans(new PictureMetaBlock(data, length)); 
            return ans; 
        }
        case UNKNOWN_RESERVED: {
            Metadata_block::ptr ans(new UnknownMetaBlock(data, length, typeNum)); 
            return ans; 
        }
        default: {
            Metadata_block::ptr ans(new InvalidMetaBlock(data, length)); 
            return ans; 
        }
    }
}

Metadata_block::Metadata_block(uint32_t length, MetadataBlockType type, bool dataValid) {
    if (length!=0) {
        m_type = type; 
    }
    m_dataValided = dataValid; 
}

Metadata_block::~Metadata_block() {
}

StreamInfoMetaBlock::StreamInfoMetaBlock(void* data, uint32_t length)
    : Metadata_block(length, STREAM_INFO) {
    // 判断长度是否合理
    if (length==34) {
        initBlock(data, length); 
    } else {
        LOGE("invalid length for StreamInfo block: length should be 34 but length=%d\n", length); 
        setDataValid(false); 
    }
}

StreamInfoMetaBlock::~StreamInfoMetaBlock() {
}

bool StreamInfoMetaBlock::getUnencoderedMD5(void* dest, uint32_t length) const {
    if (length!=STREAMINFO_MD5_SIZE) {
        LOGW("fail getUnencoderedMD5, dest's length should be %d, but got %d\n", STREAMINFO_MD5_SIZE, length); 
        return false; 
    }
    memcpy(dest, m_unencoderedMD5, STREAMINFO_MD5_SIZE); 
    return true; 
}; 

bool StreamInfoMetaBlock::setMinFrameSize(uint32_t val) {
    if (val>UINT24_MAX) {
        LOGW("fail setMinFrameSize, val should <= 16777215, but got %d\n", val); 
        return false; 
    }
    m_minFrameSize = val; 
    return true; 
}

bool StreamInfoMetaBlock::setMaxFrameSize(uint32_t val) {
    if (val>UINT24_MAX) {
        LOGW("fail setMaxFrameSize, val should <= 16777215, but got %d\n", val); 
        return false; 
    }
    m_maxFrameSize = val; 
    return true; 
}

bool StreamInfoMetaBlock::setSampleRate(uint32_t val) {
    if (val>0xFFFFF) {
        LOGW("fail setSampleRate, val should <= 1048575, but got %d\n", val); 
        return false; 
    }
    m_sampleRate = val; 
    return true; 
}

bool StreamInfoMetaBlock::setChannels(uint8_t val) {
    if (val<1||val>8) {
        LOGW("fail setChannels, val should 1~8, but got %d\n", val); 
        return false; 
    }
    m_channels = val - 1; 
    return true; 
}

bool StreamInfoMetaBlock::setSampleBits(uint8_t val) {
    if (val<4||val>32) {
        LOGW("fail setSampleBits, val should 4~32, but got %d\n", val); 
        return false; 
    }
    m_sampleBits = val - 1; 
    return true; 
}

bool StreamInfoMetaBlock::setSamplePerChannel(uint64_t val) {
    if (val>0xFFFFFFFFF) {
        LOGW("fail setSamplePerChannel, val should <= 68719476735, but got %lld\n", val); 
        return false; 
    }
    m_samplePerChannel = val; 
    return true; 
}

bool StreamInfoMetaBlock::setUnencoderedMD5(void* val, uint32_t length) {
    if (length!=STREAMINFO_MD5_SIZE) {
        LOGW("fail setUnencoderedMD5, val's length should be %d, but got %d\n", STREAMINFO_MD5_SIZE, length); 
        return false; 
    }
    memcpy(m_unencoderedMD5, val, STREAMINFO_MD5_SIZE); 
    return true; 
}

uint32_t StreamInfoMetaBlock::getBlockSize() const {
    if (!isDataValid()) {
        LOGE("getBlockSize fail, data not valid"); 
        return 0; 
    }
    return 34; 
}

void StreamInfoMetaBlock::initBlock(void* data, uint32_t length) {
    uint8_t* pin = (uint8_t*) data; 

    m_minBlockSize = byteswap(*(uint16_t*)pin); 
    pin+=2; 

    m_maxBlockSize = byteswap(*(uint16_t*)pin); 
    pin+=2; 

    m_minFrameSize = byteswap(*(uint32_t*)pin<<8); 
    pin+=3; 

    m_maxFrameSize = byteswap(*(uint32_t*)pin<<8); 
    pin+=3; 

    uint64_t tmp_val = byteswap(*(uint64_t*)pin); 
    m_sampleRate = tmp_val>>44; 
    m_channels = (tmp_val&0xFFFFFFFFFFF)>>41; 
    m_sampleBits = (tmp_val&0x1FFFFFFFFFF)>>36; 
    m_samplePerChannel = tmp_val&0xFFFFFFFFF; 
    pin+=8; 

    memcpy(m_unencoderedMD5, pin, STREAMINFO_MD5_SIZE); 
}

uint32_t StreamInfoMetaBlock::resave(void* data, bool ifLast) {
    if (!isDataValid()) {
        LOGE("can not convert invalid block!\n"); 
        return 0; 
    }

    uint32_t blockSize = getBlockSize(); 

    if (blockSize>UINT24_MAX) {
        LOGE("Block size = %d too large !\n", blockSize); 
        return 0; 
    }

    uint8_t* pin = (uint8_t*) data; 

    uint8_t blockType = STREAM_INFO; 
    if (ifLast) {
        blockType|=0x80; 
    }
    memcpy(pin, &blockType, 1); 
    pin+=1; 

    uint32_t unblockSize = byteswap(blockSize); 
    memcpy(pin, (char*)&unblockSize+1, 3); 
    pin+=3; 

    uint16_t minBlockSize = byteswap(m_minBlockSize); 
    memcpy(pin, &minBlockSize, 2); 
    pin+=2; 

    uint16_t maxBlockSize = byteswap(m_maxBlockSize); 
    memcpy(pin, &maxBlockSize, 2); 
    pin+=2; 

    uint32_t minFrameSize = byteswap(m_minFrameSize); 
    memcpy(pin, (char*)&minFrameSize+1, 3); 
    pin+=3; 

    uint32_t maxFrameSize = byteswap(m_maxFrameSize); 
    memcpy(pin, (char*)&maxFrameSize+1, 3); 
    pin+=3; 

    uint64_t tmp_val = byteswap(((uint64_t)m_sampleRate)<<44|((uint64_t)(m_channels&0x7))<<41|((uint64_t)(m_sampleBits&0x1F))<<36|(m_samplePerChannel&0xFFFFFFFFF)); 
    memcpy(pin, &tmp_val, 8); 
    pin+=8; 

    memcpy(pin, m_unencoderedMD5, STREAMINFO_MD5_SIZE); 

    return blockSize+4; 
}

PaddingMetaBlock::PaddingMetaBlock(uint32_t length)
    : Metadata_block(length, PADDING) {
    // length为0则无意义
    if (length==0||length>UINT24_MAX) {
        LOGW("invalid length for Padding block, length shouldn't be 0 or >16777215 but length=%d\n"); 
        setDataValid(false); 
    } else {
        m_blockSize = length; 
    }
}

PaddingMetaBlock::~PaddingMetaBlock() {
}

uint32_t PaddingMetaBlock::getBlockSize() const {
    if (!isDataValid()) {
        LOGE("getBlockSize fail, data not valid"); 
        return 0; 
    }
    return m_blockSize; 
}

void PaddingMetaBlock::initBlock(void* data, uint32_t length) {
}

uint32_t PaddingMetaBlock::resave(void* data, bool ifLast) {
    if (!isDataValid()) {
        LOGE("can not convert invalid block!\n"); 
        return 0; 
    }

    uint32_t blockSize = getBlockSize(); 

    if (blockSize>UINT24_MAX) {
        LOGE("Block size = %d too large !\n", blockSize); 
        return 0; 
    }

    uint8_t* pin = (uint8_t*) data; 

    uint8_t blockType = PADDING; 
    if (ifLast) {
        blockType|=0x80; 
    }
    memcpy(pin, &blockType, 1); 
    pin+=1; 

    uint32_t unblockSize = byteswap(blockSize); 
    memcpy(pin, (char*)&unblockSize+1, 3); 
    pin+=3; 

    if (m_blockSize>0) {
        std::fill((uint8_t*)pin, ((uint8_t*)pin)+m_blockSize, 0); 
    }
    
    return 4+m_blockSize;
}

ApplicationMetaBlock::ApplicationMetaBlock(void* data, uint32_t length)
    : Metadata_block(length, APPLICATION)
    , m_appData() {
    // AppId为4byte，总共要大于4byte
    if (length>=4&&length<=UINT24_MAX) {
        initBlock(data, length); 
    } else {
        LOGE("invalid length for Application block, length should be >=4 and <=16777215 but length=%d\n", length); 
        setDataValid(false); 
    }
}

ApplicationMetaBlock::~ApplicationMetaBlock() {
}

bool ApplicationMetaBlock::getAppData(void* dest, uint32_t length) const {
    if (m_appData.getSize()==0) {
        LOGW("no app data\n"); 
        return false; 
    } else if (length!=m_appData.getSize()) {
        LOGW("warning: data length is not match\n"); 
        return false; 
    } else {
        m_appData.getDataBuffers(dest, length); 
        return true; 
    }
}

void ApplicationMetaBlock::setAppData(void* val, uint8_t length) {
    m_appData.rewrite(val, length); 
}

uint32_t ApplicationMetaBlock::getBlockSize() const {
    if (!isDataValid()) {
        LOGE("getBlockSize fail, data not valid"); 
        return 0; 
    }

    return 4+m_appData.getSize(); 
}

void ApplicationMetaBlock::initBlock(void* data, uint32_t length) {
    m_appId = byteswap(*(uint32_t*)data); 

    if (length>4) {
        m_appData.rewrite((char*)data+4, length-4); 
    }
}

uint32_t ApplicationMetaBlock::resave(void* data, bool ifLast) {
    if (!isDataValid()) {
        LOGE("can not convert invalid block!\n"); 
        return 0; 
    }

    uint32_t blockSize = getBlockSize(); 

    if (blockSize>UINT24_MAX) {
        LOGE("Block size = %d too large !\n", blockSize); 
        return 0; 
    }

    uint8_t* pin = (uint8_t*) data; 

    uint8_t blockType = APPLICATION; 
    if (ifLast) {
        blockType|=0x80; 
    }
    memcpy(pin, &blockType, 1); 
    pin+=1; 

    uint32_t unblockSize = byteswap(blockSize); 
    memcpy(pin, (char*)&unblockSize+1, 3); 
    pin+=3; 

    uint32_t appId = byteswap(m_appId); 
    memcpy(pin, &appId, 4); 
    pin+=4; 

    if (m_appData.getSize()>0) {
        m_appData.getDataBuffers(pin, m_appData.getSize()); 
    }

    return blockSize+4;
}

SeekTableMetaBlock::SeekTableMetaBlock(void* data, uint32_t length)
    : Metadata_block(length, SEEKTABLE) {
    // 每个seekpoint为18byte
    if (length%18==0&&length<=UINT24_MAX) {
        initBlock(data, length); 
    } else {
        LOGE("invalid length for SeekTable block, length should be divided by 18 and <=16777215 but length=%d\n", length); 
        setDataValid(false); 
    }
}

SeekTableMetaBlock::~SeekTableMetaBlock() {
}

bool SeekTableMetaBlock::getSeekPoints(std::vector<SeekPoint>& dest) const {
    if (m_seekPoints.size()==0) {
        LOGW("no seekPoints exists\n"); 
        return false; 
    }
    std::vector<SeekTableMetaBlock::SeekPoint> vec(m_seekPoints.size()); 
    dest.assign(m_seekPoints.begin(), m_seekPoints.end()); 
    return true; 
}

bool SeekTableMetaBlock::addSeekPoint(SeekTableMetaBlock::SeekPoint& val) {
    uint32_t blockSize = getBlockSize(); 
    if (UINT24_MAX-18<blockSize) {
        LOGW("fail addSeekPoint, the block is too much, size = %d\n", blockSize); 
        return false; 
    }
    if (m_seekPoints.find(val)!=m_seekPoints.end()) {
        LOGW("fail addSeekPoint, seekPoint {firstSampleNO = %d, offsetFromFirst = %d, sampleNum = %d} already exists\n", 
            val.firstSampleNO, val.offsetFromFirst, val.sampleNum); 
        return false; 
    }
    m_seekPoints.emplace(val); 
    return true; 
}

bool SeekTableMetaBlock::addSeekPoint(uint64_t fsn, uint64_t ofs, uint16_t sn) {
    SeekPoint newSeekPoint = {fsn, ofs, sn}; 
    return addSeekPoint(newSeekPoint); 
}

bool SeekTableMetaBlock::delSeekPoint(SeekTableMetaBlock::SeekPoint& val) {
    if (m_seekPoints.find(val)==m_seekPoints.end()) {
        LOGW("fail delSeekPoint, seekPoint {firstSampleNO = %d, offsetFromFirst = %d, sampleNum = %d} not exists\n",
            val.firstSampleNO, val.offsetFromFirst, val.sampleNum); 
        return false; 
    }
    m_seekPoints.erase(val); 
    return true; 
}

bool SeekTableMetaBlock::delSeekPoint(uint64_t fsn, uint64_t ofs, uint16_t sn) {
    SeekPoint newSeekPoint = {fsn, ofs, sn}; 
    return delSeekPoint(newSeekPoint); 
}

uint32_t SeekTableMetaBlock::getBlockSize() const {
    if (!isDataValid()) {
        LOGE("getBlockSize fail, data not valid"); 
        return 0; 
    }

    return 18*m_seekPoints.size(); 
}

void SeekTableMetaBlock::initBlock(void* data, uint32_t length) {
    uint8_t* pin = (uint8_t*)data; 
    uint32_t dataLength=length/18; 

    for (uint32_t i=0; i<dataLength; ++i) {
        SeekPoint tmp_seekPoint; 

        tmp_seekPoint.firstSampleNO = byteswap(*(uint64_t*)pin); 
        pin+=8; 

        tmp_seekPoint.offsetFromFirst = byteswap(*(uint64_t*)pin); 
        pin+=8; 

        tmp_seekPoint.sampleNum = byteswap(*(uint16_t*)pin); 
        pin+=2; 

        m_seekPoints.emplace(tmp_seekPoint); 
    }
}

uint32_t SeekTableMetaBlock::resave(void* data, bool ifLast) {
    if (!isDataValid()) {
        LOGE("can not convert invalid block!\n"); 
        return 0; 
    }

    uint32_t blockSize = getBlockSize(); 

    if (blockSize>UINT24_MAX) {
        LOGE("Block size = %d too large !\n", blockSize); 
        return 0; 
    }

    uint8_t* pin = (uint8_t*) data; 

    uint8_t blockType = SEEKTABLE; 
    if (ifLast) {
        blockType|=0x80; 
    }
    memcpy(pin, &blockType, 1); 
    pin+=1; 

    uint32_t unblockSize = byteswap(blockSize); 
    memcpy(pin, (char*)&unblockSize+1, 3); 
    pin+=3; 

    for (auto& item: m_seekPoints) {
        uint64_t firstSampleNO = byteswap(item.firstSampleNO); 
        memcpy(pin, &firstSampleNO, 8); 
        pin+=8; 

        uint64_t offsetFromFirst = byteswap(item.offsetFromFirst); 
        memcpy(pin, &offsetFromFirst, 8); 
        pin+=8; 

        uint16_t sampleNum = byteswap(item.sampleNum); 
        memcpy(pin, &sampleNum, 2); 
        pin+=2; 
    }

    return blockSize+4;
}

VorbisCommentMetaBlock::VorbisCommentMetaBlock(void* data, uint32_t length)
    : Metadata_block(length, VORBIS_COMMEN) {
    if (length>=8&&length<=UINT24_MAX) {
        initBlock(data, length); 
    } else {
        LOGE("invalid length for VorbisComment block, length should be >=8 and <=16777215 but length=%d\n", length); 
        setDataValid(false); 
    }
}

VorbisCommentMetaBlock::VorbisCommentMetaBlock(const std::string& encoderIdentification)
    : Metadata_block(0, VORBIS_COMMEN) {
    if (encoderIdentification.size()>UINT24_MAX-8) {
        LOGE("invalid length for encoderIdentification\n"); 
        setDataValid(false); 
    } else {
        m_encoderIdentificationLength = encoderIdentification.size(); 
        m_encoderIdentification = new char[m_encoderIdentificationLength]; 
        memcpy(m_encoderIdentification, encoderIdentification.c_str(), m_encoderIdentificationLength); 
    }
}

VorbisCommentMetaBlock::~VorbisCommentMetaBlock() {
    if (m_encoderIdentification!=nullptr) {
        delete[] m_encoderIdentification; 
    }
}

bool VorbisCommentMetaBlock::getEncoderIdentification(void* dest, uint32_t length) const {
    if (m_encoderIdentification==nullptr||m_encoderIdentificationLength==0) {
        LOGW("fail getEncoderIdentification, encoderIdentification is null\n"); 
        return false; 
    } else if (m_encoderIdentificationLength!=length) {
        LOGW("fail getEncoderIdentification, dest's length should be %d, but got %d\n", m_encoderIdentificationLength, length); 
        return false; 
    } 

    memcpy(dest, m_encoderIdentification, length); 
    
    return true; 
}

std::string VorbisCommentMetaBlock::getEncoderIdentificationToString() const {
    if (m_encoderIdentificationLength!=0&&m_encoderIdentification!=nullptr) {
        char tmp_c[m_encoderIdentificationLength+1]; 
        memcpy(tmp_c, m_encoderIdentification, m_encoderIdentificationLength); 
        tmp_c[m_encoderIdentificationLength]='\0'; 
        return std::string(tmp_c); 
    } else {
        return ""; 
    }
}

uint32_t VorbisCommentMetaBlock::getLabelNum() const {
    uint32_t labelNum = 0; 
    for (auto& kv: m_infoLabels) {
        labelNum+=kv.second.size(); 
    }

    return labelNum; 
}

bool VorbisCommentMetaBlock::getLabelListWithKey(const std::string& key, std::vector<std::string>& dest) const {
    if (m_infoLabels.find(key)==m_infoLabels.end()) {
        LOGW("no key = \"%s\"\n", key); 
        return false; 
    }

    dest.resize(m_infoLabels.at(key).size()); 
    dest.assign(m_infoLabels.at(key).begin(), m_infoLabels.at(key).end()); \
    
    return true; 
}

std::string VorbisCommentMetaBlock::getLabelWithKey(const std::string& key, uint32_t pos) const {
    if (m_infoLabels.find(key)==m_infoLabels.end()) {
        LOGW("no key = \"%s\"\n", key); 
        return ""; 
    }
    
    auto& key_labels = m_infoLabels.at(key); 
    if (pos>=key_labels.size()) {
        LOGW("pos should be <%d, but got %d\n", key_labels.size(), pos); 
        return ""; 
    }

    return key_labels[pos]; 
}

bool VorbisCommentMetaBlock::setEncoderIdentification(const std::string& val) {
    if (val.size()>m_encoderIdentificationLength
        &&val.size()>UINT24_MAX-getBlockSize()+m_encoderIdentificationLength) {
        LOGW("fail setEncoderIdentification, new val is too long, should <=%d, length = %d\n"
            , UINT24_MAX-getBlockSize()+m_encoderIdentificationLength, val.size()); 
        return false; 
    }
    
    m_encoderIdentificationLength = val.size(); 
    if (m_encoderIdentification!=nullptr) {
        delete[] m_encoderIdentification; 
        m_encoderIdentification = nullptr; 
    }
    if (m_encoderIdentificationLength>0) {
        m_encoderIdentification = new char[m_encoderIdentificationLength]; 
        memcpy(m_encoderIdentification, val.c_str(), m_encoderIdentificationLength); 
    }
    
    return true; 
}

bool VorbisCommentMetaBlock::addInfoLabel(const std::string& key, const std::string& val, int pos) {
    if (key.size()>UINT24_MAX||val.size()>UINT24_MAX||
        key.size()+val.size()+5+getBlockSize()>UINT24_MAX) {
        LOGW("fail addInfoLabel, new val is too long, key length = %d, val length = %d\n", key.size(), val.size()); 
        return false; 
    }

    if (m_infoLabels.find(key)==m_infoLabels.end()) {
        m_infoLabels.emplace(key, std::vector<std::string>(1, val)); 
    } else if (pos==-1||pos>m_infoLabels.at(key).size()) {
        m_infoLabels.at(key).emplace_back(val); 
    } else if (pos<0) {
        m_infoLabels.at(key).emplace(m_infoLabels.at(key).begin(), val); 
    } else {
        m_infoLabels.at(key).emplace(m_infoLabels.at(key).begin()+pos, val); 
    }

    return true; 
}

bool VorbisCommentMetaBlock::delInfoLabel(const std::string& key, const std::string& val) {
    if (m_infoLabels.find(key)==m_infoLabels.end()) {
        LOGW("fail delInfoLabel, key not exists, key = %s", key); 
        return false; 
    }

    auto& key_labels = m_infoLabels.at(key); 
    auto it = std::find(key_labels.begin(), key_labels.end(), val); 
    if (it==m_infoLabels.at(key).end()) {
        LOGW("fail delInfoLabel, key-val not exists, key = %s, val = %s\n", key, val); 
        return false; 
    }

    key_labels.erase(it); 
    if (key_labels.size()==0) {
        m_infoLabels.erase(key); 
    }

    return true; 
}

bool VorbisCommentMetaBlock::delInfoLabel(const std::string& key, uint32_t pos) {
    if (m_infoLabels.find(key)==m_infoLabels.end()) {
        LOGW("fail delInfoLabel, key not exists, key = %s", key); 
        return false; 
    }

    auto& key_labels = m_infoLabels.at(key); 
    if (pos>=key_labels.size()) {
        LOGW("fail delInfoLabel, pos should be <%d, but got %d", key_labels.size(), pos); 
        return false; 
    }

    key_labels.erase(key_labels.begin()+pos); 
    if (key_labels.size()==0) {
        m_infoLabels.erase(key); 
    }

    return true; 
}

uint32_t VorbisCommentMetaBlock::delAllMatchInfoLabel(const std::string& key, const std::string& val) {
    if (m_infoLabels.find(key)==m_infoLabels.end()) {
        LOGW("fail delAllMatchInfoLabel, key not exists, key = %s", key); 
        return 0; 
    }

    auto& key_labels = m_infoLabels.at(key); 
    auto it = std::find(key_labels.begin(), key_labels.end(), val); 
    if (it==m_infoLabels.at(key).end()) {
        LOGW("fail delAllMatchInfoLabel, key-val not exists, key = %s, val = %s\n", key, val); 
        return 0; 
    }

    uint32_t num = 0; 
    while (it!=key_labels.end()) {
        key_labels.erase(it); 
        ++num; 
        it = std::find(key_labels.begin(), key_labels.end(), val); 
    }
    if (key_labels.size()==0) {
        m_infoLabels.erase(key); 
    }

    return num; 
}

int8_t VorbisCommentMetaBlock::setLabelVal(const std::string& key, const std::string& val, uint32_t pos) {
    if (m_infoLabels.find(key)==m_infoLabels.end()) {
        LOGW("fail setLabelVal, key not exists, key = %s", key); 
        return 1; 
    }

    auto& key_label = m_infoLabels.at(key); 
    if (pos>=key_label.size()) {
        LOGE("fail setLabelVal, pos should be <%d, but got %d", key_label.size(), pos); 
        return 2; 
    }

    std::string old_val = key_label[pos]; 
    
    if (val.size()>UINT24_MAX
        ||getBlockSize()-old_val.size()>UINT24_MAX-val.size()) {
        LOGW("fail setLabelVal, new value too long, length = %d", val.size()); 
        return 3; 
    }

    key_label[pos] = val; 

    return 0; 
}

bool  VorbisCommentMetaBlock::resetPosLabelVal(const std::string& key, uint32_t old_pos, uint32_t new_pos) {
    if (old_pos==new_pos) {
        LOGW("new_pos=old_pos, do not need to change"); 
        return true; 
    }
    if (m_infoLabels.find(key)==m_infoLabels.end()) {
        LOGE("no key = %s", key); 
        return false; 
    }

    auto& labels = m_infoLabels.at(key); 

    if (old_pos>labels.size()||new_pos>labels.size()) {
        LOGE("old_pos and new_pos should <= %d, but old_pos = %d, new_pos = %d", labels.size(), old_pos, new_pos); 
        return false; 
    }
    std::string value = labels[old_pos]; 
    labels.erase(labels.begin()+old_pos); 
    labels.emplace(labels.begin()+new_pos, value); 

    return true; 
}

uint32_t VorbisCommentMetaBlock::deduplication() {
    uint32_t decreaseByte = 0; 
    uint32_t num = 0; 

    for (auto& item: m_infoLabels) {
        auto& key = item.first; 
        auto& labels = item.second; 

        std::unordered_set<std::string> tmp_set; 
        for (uint32_t i=0; i<labels.size(); ++i) {
            if (tmp_set.find(labels[i])==tmp_set.end()) {
                tmp_set.emplace(labels[i]); 
            } else {
                decreaseByte+=key.size()+labels[i].size()+5; 
                labels.erase(labels.begin()+i); 
                --i; 
                ++num; 
            }
        }
    }

    return num; 
}

uint32_t VorbisCommentMetaBlock::getBlockSize() const {
    if (!isDataValid()) {
        LOGE("getBlockSize fail, data not valid"); 
        return 0; 
    }

    uint32_t blockSize = 8+m_encoderIdentificationLength; 

    for (auto& kv: m_infoLabels) {
        for (auto& item: kv.second) {
            blockSize+=kv.first.size()+item.size()+5; 
        }
    }

    return blockSize; 
}

void VorbisCommentMetaBlock::initBlock(void* data, uint32_t length) {
    uint8_t* pin = (uint8_t*)data; 
    uint32_t dataSize = length; 

    // 文件中小端序保存，故无需字节转化
    m_encoderIdentificationLength = *(uint32_t*)pin; 
    pin+=4; 

    if (m_encoderIdentification!=nullptr) {
        delete[] m_encoderIdentification; 
        m_encoderIdentification = nullptr; 
    }
    if (m_encoderIdentificationLength>0) {
        m_encoderIdentification = new char[m_encoderIdentificationLength]; 
        memcpy(m_encoderIdentification, pin, m_encoderIdentificationLength); 
        pin+=m_encoderIdentificationLength; 
    }

    // 文件中小端序保存，故无需字节转化
    uint32_t labelNum = *(uint32_t*)pin; 
    pin+=4; 

    dataSize-=8+m_encoderIdentificationLength; 
    if (dataSize<0) {
        LOGE("incorrect length for VorbisComment block\n"); 
        setDataValid(false); 
        return; 
    }

    for (int i=0; i<labelNum; ++i) {
        // 文件中小端序保存，故无需字节转化
        uint32_t label_length = *(uint32_t*)pin; 
        pin+=4; 

        char tmp_str[label_length]; 
        memcpy(tmp_str, pin, label_length); 
        pin+=label_length; 

        dataSize-=4+label_length; 

        std::string tmp_s = tmp_str; 
        
        uint32_t equ_pos = tmp_s.find_first_of('=', 0); 
        if (equ_pos==-1) {
            LOGE("label format is incorrect, do not include '='\n"); 
            setDataValid(false); 
            return; 
        }
        std::string key = tmp_s.substr(0, equ_pos); 
        if (m_infoLabels.find(key)==m_infoLabels.end()) {
            m_infoLabels[key]=std::vector<std::string>(0); 
        }

        m_infoLabels[key].emplace_back(tmp_s.substr(equ_pos+1, label_length-equ_pos-1)); 
    }

    if (dataSize!=0) {
        LOGW("VorbisCommentMetaBlock size incorrect, not finish reading\n"); 
    }
}

uint32_t VorbisCommentMetaBlock::resave(void* data, bool ifLast) {
    if (!isDataValid()) {
        LOGE("can not convert invalid block!\n"); 
        return 0; 
    }

    uint32_t blockSize = getBlockSize(); 

    if (blockSize>UINT24_MAX) {
        LOGE("Block size = %d too large !\n", blockSize); 
        return 0; 
    }

    uint8_t* pin = (uint8_t*) data; 

    uint8_t blockType = VORBIS_COMMEN; 
    if (ifLast) {
        blockType|=0x80; 
    }
    memcpy(pin, &blockType, 1); 
    pin+=1; 

    uint32_t unblockSize = byteswap(blockSize); 
    memcpy(pin, (char*)&unblockSize+1, 3); 
    pin+=3; 

    memcpy(pin, &m_encoderIdentificationLength, 4); 
    pin+=4; 

    if (m_encoderIdentificationLength>0) {
        memcpy(pin, m_encoderIdentification, m_encoderIdentificationLength); 
        pin+=m_encoderIdentificationLength; 
    }

    uint32_t labelNum = 0; 
    for (auto& item: m_infoLabels) {
        labelNum+=item.second.size(); 
    }

    // 文件中小端序保存，故无需字节转化
    memcpy(pin, &labelNum, 4); 
    pin+=4; 

    for (auto& label: m_infoLabels) {
        auto& key = label.first; 
        for (auto& value: label.second) {
            uint32_t lable_length = key.size()+value.size()+1; 
            // 文件中小端序保存，故无需字节转化
            memcpy(pin, &lable_length, 4); 
            pin+=4; 

            std::string tmp_s = key + "=" + value; 
            memcpy(pin, tmp_s.c_str(), lable_length); 
            pin+=lable_length; 
        }
    }

    return blockSize+4;
}

CuesheetMetaBlock::CuesheetMetaBlock(void* data, uint32_t length)
    : Metadata_block(length, CUESHEET) {
    // Header要大于396byte
    if (length>=396&&length<=UINT24_MAX) {
        initBlock(data, length); 
    } else {
        LOGE("invalid length for Cuesheet block, length should be >=396 and <=16777215 but length=%d\n", length); 
        setDataValid(false); 
    }
}

CuesheetMetaBlock::~CuesheetMetaBlock() {
}

bool CuesheetMetaBlock::getReserved(void* dest, uint16_t length) const {
    if (length!=CUESHEET_REVERSED_SIZE-1) {
        LOGE("fail getReserved, dest's length should be %d, but got %d\n", CUESHEET_REVERSED_SIZE-1, length); 
        return false; 
    }

    memcpy(dest, m_reserved+1, length); 

    return true; 
}

uint8_t CuesheetMetaBlock::getTrackNum() const {
    if (m_tracks.size()>100) {
        LOGE("error: tracks num should be <=100, but got %d", m_tracks.size()); 
    }
    return m_tracks.size(); 
}

uint8_t CuesheetMetaBlock::getTracks(std::vector<CuesheetMetaBlock::Track>& dest) const {
    uint8_t trackNum = getTrackNum(); 
    if (trackNum==0) {
        LOGW("fail getTracks, no track\n"); 
        return 0; 
    }

    dest.resize(trackNum); 
    dest.assign(m_tracks.begin(), m_tracks.end()); 

    return trackNum; 
}

uint32_t CuesheetMetaBlock::getBlockSize() const {
    if (!isDataValid()) {
        LOGE("getBlockSize fail, data not valid"); 
        return 0; 
    }

    uint32_t blockSize = 396; 

    for (auto& item: m_tracks) {
        blockSize+=36+item.indexs.size()*12; 
    }

    return blockSize; 
}

void CuesheetMetaBlock::initBlock(void* data, uint32_t length) {
    uint8_t* pin = (uint8_t*)data; 

    memcpy(m_mediaCatalogNumber, pin, CUESHEET_MEDIACATALOGNUM_size); 
    pin+=CUESHEET_MEDIACATALOGNUM_size; 

    m_guidingSampleNum = byteswap(*(uint64_t*)pin); 
    pin+=8; 

    memcpy(m_reserved, pin, CUESHEET_REVERSED_SIZE); 
    pin+=CUESHEET_REVERSED_SIZE; 

    uint8_t trackNum = *pin; 
    pin+=1; 

    uint32_t pin_left=length-396; 
    for (uint8_t i=0; i<trackNum; ++i) {
        Track tmp_track; 
        
        tmp_track.offset = byteswap(*(uint64_t*)pin); 
        pin+=8; 

        tmp_track.trackNO = *pin; 
        pin+=1; 

        memcpy(tmp_track.trackISRC, pin, CUESHEET_TRACK_ISRC_SIZE); 
        pin+=CUESHEET_TRACK_ISRC_SIZE; 

        memcpy(tmp_track.reserved, pin, CUESHEET_TRACK_REVERSED_SIZE); 
        pin+=CUESHEET_TRACK_REVERSED_SIZE; 

        tmp_track.indexNum = *pin; 
        pin+=1; 

        // 此处判断一次data长度是否足够
        pin_left-=36; 
        if (pin_left<12*tmp_track.indexNum) {
            LOGE("invalid length for Track of Cuesheet block: length not enough\n"); 
            setDataValid(false); 
            break; 
        }

        tmp_track.indexs.resize(tmp_track.indexNum); 

        for (uint8_t j=0; j<tmp_track.indexNum; ++j) {
            Track::Index tmp_index; 

            tmp_index.offset = byteswap(*(uint64_t*)pin); 
            pin+=8; 

            tmp_index.indexNO = *pin; 
            pin+=1; 

            memcpy(tmp_index.reserved, pin, CUESHEET_TRACK_INDEX_REVERSED_SIZE); 
            pin+=CUESHEET_TRACK_INDEX_REVERSED_SIZE; 

            tmp_track.indexs[j]=tmp_index; 
        }
        pin_left-=12*tmp_track.indexNum; 

        m_tracks.emplace_back(tmp_track); 
    }
    if (pin_left!=0) {
        LOGE("invalid length for Track of Cuesheet block: length not fit the data\n"); 
        setDataValid(false); 
    }
}

uint32_t CuesheetMetaBlock::resave(void* data, bool ifLast) {
    if (!isDataValid()) {
        LOGE("can not convert invalid block!\n"); 
        return 0; 
    }

    uint32_t blockSize = getBlockSize(); 

    if (blockSize>UINT24_MAX) {
        LOGE("Block size = %d too large !\n", blockSize); 
        return 0; 
    }

    uint8_t* pin = (uint8_t*) data; 

    uint8_t blockType = CUESHEET; 
    if (ifLast) {
        blockType|=0x80; 
    }
    memcpy(pin, &blockType, 1); 
    pin+=1; 

    uint32_t unblockSize = byteswap(blockSize); 
    memcpy(pin, (char*)&unblockSize+1, 3); 
    pin+=3; 

    memcpy(pin, m_mediaCatalogNumber, CUESHEET_MEDIACATALOGNUM_size); 
    pin+=CUESHEET_MEDIACATALOGNUM_size; 

    uint64_t guidingSampleNum = byteswap(m_guidingSampleNum); 
    memcpy(pin, &guidingSampleNum, 8); 
    pin+=8; 

    memcpy(pin, m_reserved, CUESHEET_REVERSED_SIZE); 
    pin+=CUESHEET_REVERSED_SIZE; 

    uint8_t trackNum = getTrackNum(); 

    memcpy(pin, &trackNum, 1); 
    pin+=1; 

    for (auto track: m_tracks) {
        uint64_t t_offset = byteswap(track.offset); 
        memcpy(pin, &t_offset, 8); 
        pin+=8; 

        memcpy(pin, &track.trackNO, 1); 
        pin+=1; 

        memcpy(pin, track.trackISRC, CUESHEET_TRACK_ISRC_SIZE); 
        pin+=CUESHEET_TRACK_ISRC_SIZE; 

        memcpy(pin, track.reserved, CUESHEET_TRACK_REVERSED_SIZE); 
        pin+=CUESHEET_TRACK_REVERSED_SIZE; 

        // 判断track的index number是否正确
        if (track.indexNum!=track.indexs.size()) {
            track.indexNum=track.indexs.size(); 
            LOGW("index num in track is not match, has been corrected\n"); 
        }

        memcpy(pin, &track.indexNum, 1); 
        pin+=1; 

        for (auto& t_index: track.indexs) {
            uint64_t t_i_offset = byteswap(t_index.offset); 
            memcpy(pin, &t_i_offset, 8); 
            pin+=8; 

            memcpy(pin, &t_index.indexNO, 1); 
            pin+=1; 

            memcpy(pin, t_index.reserved, 3); 
            pin+=3; 
        }
    }

    return blockSize+4;
}

PictureMetaBlock::PictureMetaBlock(void* data, uint32_t length)
    : Metadata_block(length, PICTURE)
    , m_pictureData() {
    // 数据至少是32 byte
    if (length>=32&&length<=UINT24_MAX) {
        initBlock(data, length); 
    } else {
        LOGE("invalid length for Picture block, length should be >=32 and <=16777215 but length=%d\n", length); 
        setDataValid(false); 
    }
}

PictureMetaBlock::PictureMetaBlock(const Image& img)
    : Metadata_block(1, PICTURE) {
    if (img.isValid()) {
        std::string mimeType = img.getMimeTyepFromImageType(img.getType()); 
        m_mimeLength = mimeType.size(); 
        size_t dataLength = img.getDataSize(); 
        if (dataLength+m_mimeLength>UINT24_MAX-32) {
            LOGE("img data too long"); 
            setDataValid(false); 
        } else {
            m_mimeType = new char[m_mimeLength]; 
            memcpy(m_mimeType, mimeType.c_str(), m_mimeLength); 
            m_descriptorLength = 0; 
            m_pictureWidth = img.getWidth(); 
            m_pictureHeight = img.getHeight(); 
            m_pictureColorDepth = img.getColorDepthBit(); 
            m_pictureIndexColorNum = 0; 
            void* buf = malloc(dataLength); 
            img.getData(buf, dataLength); 
            m_pictureData.rewrite(buf, dataLength); 
            free(buf); 
        }
    } else {
        LOGE("img not valid"); 
        setDataValid(false); 
    }
}

PictureMetaBlock::~PictureMetaBlock() {
    if (m_mimeType!=nullptr) {
        delete[] m_mimeType; 
    }
    if (m_descriptor!=nullptr) {
        delete[] m_descriptor; 
    }
}

std::string PictureMetaBlock::getMimeTypeToString() const { 
    if (m_mimeLength!=0&&m_mimeType!=nullptr) {
        char tmp_c[m_mimeLength+1]; 
        memcpy(tmp_c, m_mimeType, m_mimeLength); 
        tmp_c[m_mimeLength]='\0'; 
        return std::string(tmp_c); 
    } else {
        return ""; 
    }
}

bool PictureMetaBlock::getMimeType(void* dest, uint32_t length) const {
    if (length!=m_mimeLength) {
        LOGE("fail getMimeType, length should be %d, but got %d\n", m_mimeLength, length); 
        return false; 
    }

    memcpy(dest, m_mimeType, length); 

    return true; 
}

std::string PictureMetaBlock::getDescriptorToString() const {
    if (m_descriptorLength!=0&&m_descriptor!=nullptr) {
        char tmp_c[m_descriptorLength+1]; 
        memcpy(tmp_c, m_descriptor, m_descriptorLength); 
        tmp_c[m_descriptorLength]='\0'; 
        return std::string(tmp_c); 
    } else {
        return ""; 
    }
}

bool PictureMetaBlock::getDescriptor(void* dest, uint32_t length) const {
    if (length!=m_descriptorLength) {
        LOGE("fail getDescriptor, length should be %d, but got %d\n", m_descriptorLength, length); 
        return false; 
    }

    memcpy(dest, m_descriptor, length); 

    return true; 
}

bool PictureMetaBlock::getPictureData(void* dest, uint32_t length) const {
    if (length!=m_pictureData.getSize()) {
        LOGE("fail getPictureData, length should be %d, but got %d\n", m_pictureData.getSize(), length); 
        return false; 
    }

    m_pictureData.getDataBuffers(dest, length); 

    return true; 
}

bool PictureMetaBlock::setDescriptor(const std::string& val) {
    if (val.size()>0xFFFFFF-getBlockSize()-m_descriptorLength) {
        LOGE("fail setDescriptor, length too long, set length = \n", val.size()); 
        return false; 
    }

    if (m_descriptorLength!=val.size()) {
        m_descriptorLength = val.size(); 
        if (m_descriptor!=nullptr) {
            delete[] m_descriptor; 
            m_descriptor=nullptr; 
        }
        if (m_descriptorLength>0) {
            m_descriptor=new char[val.size()]; 
            memcpy(m_descriptor, val.c_str(), m_descriptorLength); 
        }
    }

    return true; 
}

bool PictureMetaBlock::setPicture(const Image& img) {
    size_t img_size = img.getDataSize(); 
    std::string mimeType = Image::getMimeTyepFromImageType(img.getType()); 
    // 判断大小是否合适，应该img data和mimeType字符串长度和应该小于0xFFFFFF-32
    if (img_size==0||img_size+mimeType.size()>UINT24_MAX-32) {
        LOGE("can not set new picture, may be length is 0 or too big, new data length=%lld", img_size); 
        return false; 
    }

    if (m_mimeType!=nullptr) {
        delete[] m_mimeType; 
        m_mimeType = nullptr; 
    }
    m_mimeLength = mimeType.size(); 
    m_mimeType = new char[m_mimeLength]; 
    memcpy(m_mimeType, mimeType.c_str(), m_mimeLength); 

    if (m_descriptor!=nullptr) {
        delete[] m_descriptor; 
        m_descriptor = nullptr; 
    }
    m_descriptor = 0; 

    m_pictureWidth = img.getWidth(); 
    m_pictureHeight = img.getHeight(); 
    m_pictureColorDepth = img.getColorDepthBit(); 
    m_pictureIndexColorNum = 0; 

    void* buf = malloc(img_size); 
    img.getData(buf, img_size); 
    m_pictureData.rewrite(buf, img_size); 
    free(buf); 

    return true; 
}

uint32_t PictureMetaBlock::getBlockSize() const {
    if (!isDataValid()) {
        LOGE("getBlockSize fail, data not valid"); 
        return 0; 
    }

    uint32_t blockSize = 32+m_mimeLength+m_descriptorLength+m_pictureData.getSize(); 

    return blockSize; 
}

void PictureMetaBlock::initBlock(void* data, uint32_t length) {
    uint8_t* pin = (uint8_t*)data; 

    uint32_t tmp_ptype = byteswap(*(uint32_t*)pin); 
    m_pictureType = (tmp_ptype<=20)?(PictureType)tmp_ptype:OTHER; 
    pin+=4; 

    m_mimeLength = byteswap(*(uint32_t*)pin); 
    pin+=4; 

    if (m_mimeType!=nullptr) {
        delete[] m_mimeType; 
        m_mimeType = nullptr; 
    }
    if (m_mimeLength>0) {
        m_mimeType = new char[m_mimeLength]; 
    }
    memcpy(m_mimeType, pin, m_mimeLength); 
    pin+=m_mimeLength; 

    m_descriptorLength = byteswap(*(uint32_t*)pin); 
    pin+=4; 

    if (m_descriptor!=nullptr) {
        delete[] m_descriptor; 
        m_descriptor = nullptr; 
    }
    if (m_descriptorLength>0) {
        m_descriptor = new char[m_descriptorLength]; 
    }
    memcpy(m_descriptor, pin, m_descriptorLength); 
    pin+=m_descriptorLength; 

    m_pictureWidth = byteswap(*(uint32_t*)pin); 
    pin+=4; 

    m_pictureHeight = byteswap(*(uint32_t*)pin); 
    pin+=4; 

    m_pictureColorDepth = byteswap(*(uint32_t*)pin); 
    pin+=4; 

    m_pictureIndexColorNum = byteswap(*(uint32_t*)pin); 
    pin+=4; 

    uint32_t pictureDataLength = byteswap(*(uint32_t*)pin); 
    pin+=4; 

    m_pictureData.rewrite(pin, pictureDataLength); 

    if (m_mimeLength+m_descriptorLength+pictureDataLength+32!=length) {
        LOGE("invalid length for Picture block, length not fit data but length=%d\n"); 
        setDataValid(false); 
    }
}

uint32_t PictureMetaBlock::resave(void* data, bool ifLast) {
    if (!isDataValid()) {
        LOGE("can not convert invalid block!\n"); 
        return 0; 
    }

    uint32_t blockSize = getBlockSize(); 

    if (blockSize>UINT24_MAX) {
        LOGE("Block size = %d too large !\n", blockSize); 
        return 0; 
    }

    uint8_t* pin = (uint8_t*) data; 

    uint8_t blockType = PICTURE; 
    if (ifLast) {
        blockType|=0x80; 
    }
    memcpy(pin, &blockType, 1); 
    pin+=1; 

    uint32_t unblockSize = byteswap(blockSize); 
    memcpy(pin, (char*)&unblockSize+1, 3); 
    pin+=3; 

    uint32_t pic_type = byteswap((uint32_t)m_pictureType); 
    memcpy(pin, &pic_type, 4); 
    pin+=4; 

    uint32_t mimeLength = byteswap(m_mimeLength); 
    memcpy(pin, &mimeLength, 4); 
    pin+=4; 

    if (m_mimeLength>0) {
        memcpy(pin, m_mimeType, m_mimeLength); 
        pin+=m_mimeLength; 
    }

    uint32_t descriptorLength = byteswap(m_descriptorLength); 
    memcpy(pin, &descriptorLength, 4); 
    pin+=4; 

    if (m_descriptorLength>0) {
        memcpy(pin, m_descriptor, m_descriptorLength); 
        pin+=m_descriptorLength; 
    }

    uint32_t pic_w = byteswap(m_pictureWidth); 
    memcpy(pin, &pic_w, 4); 
    pin+=4; 

    uint32_t pic_h = byteswap(m_pictureHeight); 
    memcpy(pin, &pic_h, 4); 
    pin+=4; 

    uint32_t pic_cd = byteswap(m_pictureColorDepth); 
    memcpy(pin, &pic_cd, 4); 
    pin+=4; 

    uint32_t pic_cn = byteswap(m_pictureIndexColorNum); 
    memcpy(pin, &pic_cn, 4); 
    pin+=4; 

    uint32_t pictureDataLength = m_pictureData.getSize(); 
    uint32_t pic_dl = byteswap(pictureDataLength); 
    memcpy(pin, &pic_dl, 4); 
    pin+=4; 

    if (pictureDataLength>0) {
        m_pictureData.getDataBuffers(pin, pictureDataLength); 
    }

    return blockSize+4;
}

UnknownMetaBlock::UnknownMetaBlock(void* data, uint32_t length, uint8_t typeNum)
    : Metadata_block(length, UNKNOWN_RESERVED)
    , m_typeNum(typeNum) {
    if (length>0) {
        initBlock(data, length); 
        m_length = length; 
    } else {
        LOGE("invalid length for Unknown block, length should be >0 but length=%d\n"); 
        setDataValid(false); 
    }
}

UnknownMetaBlock::~UnknownMetaBlock() {
    if (m_data!=nullptr) {
        delete[] m_data; 
    }
}

bool UnknownMetaBlock::getData(void* dest, uint32_t length) const {
    if (length!=m_length) {
        LOGE("fail getData, length should be %d, but got %d\n", m_length, length); 
        return false; 
    }

    memcpy(dest, m_data, length); 

    return true; 
}

uint32_t UnknownMetaBlock::getBlockSize() const {
    if (!isDataValid()) {
        LOGE("getBlockSize fail, data not valid"); 
        return 0; 
    }

    return m_length; 
}

void UnknownMetaBlock::initBlock(void* data, uint32_t length) {
    if (m_data!=nullptr) {
        delete[] m_data; 
        m_data = nullptr; 
    }
    uint32_t data_length = length; 
    if (data_length>0) {
        m_data = new char[data_length]; 
        memcpy(m_data, data, data_length); 
    }
}

uint32_t UnknownMetaBlock::resave(void* data, bool ifLast) {
    if (!isDataValid()) {
        LOGE("can not convert invalid block!\n"); 
        return 0; 
    }

    uint32_t blockSize = getBlockSize(); 

    if (blockSize>UINT24_MAX) {
        LOGE("Block size = %d too large !\n", blockSize); 
        return 0; 
    }

    uint8_t* pin = (uint8_t*) data; 
    
    uint8_t blockType = m_typeNum; 
    if (ifLast) {
        blockType|=0x80; 
    }
    memcpy(pin, &blockType, 1); 
    pin+=1; 

    uint32_t unblockSize = byteswap(getBlockSize()); 
    memcpy(pin, (char*)&unblockSize+1, 3); 
    pin+=3; 

    if (blockSize>0) {
        memcpy(pin, m_data, blockSize); 
    }

    return blockSize+4;
}

InvalidMetaBlock::InvalidMetaBlock(void* data, uint32_t length)
    : Metadata_block(length, UNKNOWN_RESERVED) {
    if (length>0) {
        initBlock(data, length); 
        m_length = length; 
    } else {
        LOGE("invalid length for Unknown block, length should be >0 but length=%d\n"); 
        setDataValid(false); 
    }
}

InvalidMetaBlock::~InvalidMetaBlock() {
    if (m_data!=nullptr) {
        delete[] m_data; 
    }
}

bool InvalidMetaBlock::getData(void* dest, uint32_t length) const {
    if (length!=m_length) {
        LOGE("fail getData, length should be %d, but got %d\n", m_length, length); 
        return false; 
    }

    memcpy(dest, m_data, length); 

    return true; 
}

uint32_t InvalidMetaBlock::getBlockSize() const {
    if (!isDataValid()) {
        LOGE("getBlockSize fail, data not valid"); 
        return 0; 
    }

    return m_length; 
}

void InvalidMetaBlock::initBlock(void* data, uint32_t length) {
    if (m_data!=nullptr) {
        delete[] m_data; 
        m_data = nullptr; 
    }
    uint32_t data_length = length; 
    if (data_length>0) {
        m_data = new char[data_length]; 
        memcpy(m_data, data, data_length); 
    }
}

uint32_t InvalidMetaBlock::resave(void* data, bool ifLast) {
    if (!isDataValid()) {
        LOGE("can not convert invalid block!\n"); 
        return 0; 
    }

    uint32_t blockSize = getBlockSize(); 

    if (blockSize>UINT24_MAX) {
        LOGE("Block size = %d too large !\n", blockSize); 
        return 0; 
    }

    uint8_t* pin = (uint8_t*) data; 

    uint8_t blockType = INVALID; 
    if (ifLast) {
        blockType|=0x80; 
    }
    memcpy(pin, &blockType, 1); 
    pin+=1; 

    uint32_t unblockSize = byteswap(blockSize); 
    memcpy(pin, (char*)&unblockSize+1, 3); 
    pin+=3; 

    if (blockSize>0) {
        memcpy(pin, m_data, blockSize); 
    }

    return blockSize+4;
}

MusicDecoderflac::MusicDecoderflac() {
}

MusicDecoderflac::MusicDecoderflac(std::wstring& file_name) {
    openFile(file_name); 
}

MusicDecoderflac::MusicDecoderflac(const wchar_t* file_name) {
    openFile(file_name); 
}

MusicDecoderflac::~MusicDecoderflac() {
}

bool MusicDecoderflac::addMetaDataBlock(void* data, uint32_t length, uint8_t type) {
    Metadata_block::MetadataBlockType mtype = (type>=7&&type<127)?Metadata_block::UNKNOWN_RESERVED:Metadata_block::MetadataBlockType(type); 
    bool isvalid = true; 

    switch (mtype) {
        case Metadata_block::STREAM_INFO: {
            if (m_streamInfo==nullptr) {
                m_streamInfo = std::make_shared<StreamInfoMetaBlock>(data, length); 
                isvalid = m_streamInfo->isDataValid(); 
            } else {
                LOGE("StreamInfoBlock is exsiting already, please do not add new StreamInfoBlock\n"); 
            }
            break; 
        }
        case Metadata_block::PADDING: {
            if (m_padding==nullptr) {
                m_padding = std::make_shared<PaddingMetaBlock>(length); 
                isvalid = m_padding->isDataValid(); 
            } else {
                LOGE("PaddingBlock is exsiting already, please do not add new PaddingBlock\n"); 
            }
            break; 
        }
        case Metadata_block::APPLICATION: {
            if (m_application==nullptr) {
                m_application = std::make_shared<ApplicationMetaBlock>(data, length); 
                isvalid = m_application->isDataValid(); 
            } else {
                LOGE("ApplicationMetaBlock is exsiting already, please do not add new ApplicationMetaBlock\n"); 
            }
            break; 
        }
        case Metadata_block::SEEKTABLE: {
            if (m_seekTable==nullptr) {
                m_seekTable = std::make_shared<SeekTableMetaBlock>(data, length); 
                isvalid = m_seekTable->isDataValid(); 
            } else {
                LOGE("SeekTableMetaBlock is exsiting already, please do not add new SeekTableMetaBlock\n"); 
            }
            break; 
        }
        case Metadata_block::VORBIS_COMMEN: {
            if (m_vorbisComment==nullptr) {
                m_vorbisComment = std::make_shared<VorbisCommentMetaBlock>(data, length); 
                isvalid = m_vorbisComment->isDataValid(); 
            } else {
                LOGE("VorbisCommentMetaBlock is exsiting already, please do not add new VorbisCommentMetaBlock\n"); 
            }
            break; 
        }
        case Metadata_block::CUESHEET: {
            if (m_cuesheet==nullptr) {
                m_cuesheet = std::make_shared<CuesheetMetaBlock>(data, length); 
                isvalid = m_cuesheet->isDataValid(); 
            } else {
                LOGE("CuesheetMetaBlock is exsiting already, please do not add new CuesheetMetaBlock\n"); 
            }
            break; 
        }
        case Metadata_block::PICTURE: {
            PictureMetaBlock::ptr ans(new PictureMetaBlock(data, length)); 
            isvalid = ans->isDataValid(); 
            m_pictures.emplace_back(ans); 
            break; 
        }
        case Metadata_block::INVALID: {
            InvalidMetaBlock::ptr ans(new InvalidMetaBlock(data, length)); 
            isvalid = ans->isDataValid(); 
            m_invalidData.emplace_back(ans); 
            break; 
        }
        default: {
            UnknownMetaBlock::ptr ans(new UnknownMetaBlock(data, length, type)); 
            isvalid = ans->isDataValid(); 
            m_unknownReservedData.emplace_back(ans); 
            break; 
        }
    }

    return isvalid; 
}

bool MusicDecoderflac::addVorbisCommentMetaBlock(const std::string& encoderIdentification) {
    if (m_vorbisComment!=nullptr) {
        LOGE("fail addVorbisCommentMetaBlock, already exists\n"); 
        return false; 
    }

    m_vorbisComment.reset(new VorbisCommentMetaBlock(encoderIdentification)); 
    if (!m_vorbisComment->isDataValid()) {
        LOGE("fail addVorbisCommentMetaBlock, create VorbisCommentMetaBlock fail\n"); 
        m_vorbisComment.reset(); 
        return false; 
    }

    return true; 
}

bool MusicDecoderflac::getPictures(std::vector<PictureMetaBlock::ptr>& dest) const {
    if (m_pictures.size()==0) {
        LOGW("no picture\n"); 
        return false; 
    }

    dest.resize(m_pictures.size()); 
    dest.assign(m_pictures.begin(), m_pictures.end()); 

    return true; 
}

void MusicDecoderflac::initData(void* data, size_t length) {
    uint8_t* pin = (uint8_t*)data; 
    char label[5]; 
    strncpy(label, (const char*)pin, 4); 
    label[4]='\0'; 
    if (!isFlac(label)) {
        LOGE("file is not flac\n"); 
        setIsValid(false); 
        return; 
    }

    pin+=4; 

    uint64_t n_position=4; 
    bool ifMetaOver = false; 
    while (n_position<length) {
        // 判断metadata是否读完
        if (ifMetaOver) {
            m_audioFramesLength = length-n_position; 
            m_audioFrames.rewrite(pin, m_audioFramesLength); 
            n_position=length; 
            break; 
        }
        
        uint8_t metaHeaderType = *(uint8_t*)pin; 
        if (metaHeaderType>>7==1) {
            ifMetaOver = true; 
        }

        uint8_t metaBlockType = 0x7F&metaHeaderType; 
        
        pin+=1; 

        uint32_t blockSize = byteswap(*(uint32_t*)pin<<8); 
        pin+=3; 

        m_isValid = addMetaDataBlock((void*)pin, blockSize, metaBlockType); 

        if (!m_isValid) {
            LOGE("flac file broken in block ID=%d", metaBlockType); 
            return; 
        }

        pin+=blockSize; 
        n_position+=4+blockSize; 
    }
    if (m_streamInfo==nullptr) {
        setIsValid(false); 
        LOGE("flac file broken, no streaminfo block"); 
    }
}

bool MusicDecoderflac::setVorbisCommentLabel(const std::string& key, const std::string& val, uint32_t pos) {
    if (m_vorbisComment==nullptr) {
        bool res = addVorbisCommentMetaBlock(); 
        if (!res) {
            LOGE("fail setVorbisCommentLabel, couldn't create VorbisCommentMetaBlock\n"); 
            return false; 
        }
    }

    int8_t ret = m_vorbisComment->setLabelVal(key, val, pos); 
    if (ret!=0) {
        if (ret==1) {
            ret = m_vorbisComment->addInfoLabel(key, val, 0); 
            if (!ret) {
                LOGE("fail setVorbisCommentLabel, couldn't set new label\n"); 
                return false; 
            }
        } else {
            LOGE("fail setbackTitle, couldn't set new label\n"); 
            return false; 
        }
    }

    return true; 
}

bool MusicDecoderflac::setbackTitle(const std::string& val) {
    return setVorbisCommentLabel("TITLE", val); 
}

bool MusicDecoderflac::setbackAlbumArtist(const std::string& val) {
    return setVorbisCommentLabel("ALBUMARTIST", val); 
}

bool MusicDecoderflac::setbackAlbum(const std::string& val) {
    return setVorbisCommentLabel("ALBUM", val); 
}

bool MusicDecoderflac::setbackArtist(const std::string& val, uint32_t pos) {
    return setVorbisCommentLabel("ARTIST", val, pos); 
}

bool MusicDecoderflac::addbackArtist(const std::string& val, int pos) {
    return m_vorbisComment->addInfoLabel("ARTIST", val, pos); 
}

bool MusicDecoderflac::delbackArtist(const std::string& val) {
    return m_vorbisComment->delInfoLabel("ARTIST", val); 
}

bool MusicDecoderflac::delbackArtist(uint32_t index) {
    return m_vorbisComment->delInfoLabel("ARTIST", index); 
}

bool MusicDecoderflac::resetPosbackArtist(uint32_t old_pos, uint32_t new_pos) {
    return m_vorbisComment->resetPosLabelVal("ARTIST", old_pos, new_pos); 
}

bool MusicDecoderflac::setbackTrack(uint8_t val) {
    return setVorbisCommentLabel("TRACKNUMBER", std::to_string(val)); 
}

bool MusicDecoderflac::setbackCover(const Image& img, uint32_t pos) {
    if (m_pictures.size()==0) {
        return addbackCover(img); 
    }

    if (pos>=m_pictures.size()) {
        LOGE("pos is not correct, should <%d, but got %d", m_pictures.size(), pos);
        return false;  
    }

    bool res = m_pictures[pos]->setPicture(img); 

    return res; 
}

bool MusicDecoderflac::addbackCover(const Image& img, uint32_t pos) {
    if (pos>m_pictures.size()) {
        LOGE("pos is not correct, should <=%d, but got %d", m_pictures.size(), pos);
        return false; 
    }

    m_pictures.emplace(m_pictures.begin()+pos, new PictureMetaBlock(img)); 

    return true; 
}

bool MusicDecoderflac::delbackCover(uint32_t pos) {
    if (pos>=m_pictures.size()) {
        LOGE("pos is not correct, should <%d, but got %d", m_pictures.size(), pos);
        return false; 
    }

    m_pictures.erase(m_pictures.begin()+pos); 

    return true; 
}

bool MusicDecoderflac::resetPosbackCover(uint32_t old_pos, uint32_t new_pos) {
    if (old_pos==new_pos) {
        LOGW("new_pos=old_pos, do not need to change"); 
        return true; 
    }
    if (m_pictures.size()<=1) {
        LOGI("cover num = %d, do not need to change", m_pictures.size()); 
        return true; 
    }

    if (old_pos>m_pictures.size()||new_pos>m_pictures.size()) {
        LOGE("old_pos and new_pos should <= %d, but old_pos = %d, new_pos = %d", m_pictures.size(), old_pos, new_pos); 
        return false; 
    }
    auto value = m_pictures[old_pos]; 
    m_pictures.erase(m_pictures.begin()+old_pos); 
    m_pictures.emplace(m_pictures.begin()+new_pos, value); 

    return true; 
}

std::string MusicDecoderflac::getTitle() const {
    return m_vorbisComment->getLabelWithKey("TITLE"); 
}

std::string MusicDecoderflac::getAlbumArtist() const {
    return m_vorbisComment->getLabelWithKey("ALBUMARTIST"); 
}

std::string MusicDecoderflac::getAlbum() const {
    return m_vorbisComment->getLabelWithKey("ALBUM"); 
}

bool MusicDecoderflac::getArtists(std::vector<std::string>& dest) const {
    return m_vorbisComment->getLabelListWithKey("ARTIST", dest); 
}

int MusicDecoderflac::getTrack() const {
    int ans = std::stoi(m_vorbisComment->getLabelWithKey("TRACKNUMBER")); 
    if (ans<=0) {
        LOGW("track num should be >0, the info in vorbisComment is not valid\n"); 
    }
    return ans; 
}

bool MusicDecoderflac::getCovers(std::vector<Image::ptr>& dest) const {
    if (m_pictures.size()==0) {
        LOGW("no covers"); 
        return false; 
    }
    dest.resize(m_pictures.size()); 
    for (int i=0; i<m_pictures.size(); ++i) {
        uint32_t img_size = m_pictures[i]->getPictureDataLength(); 
        void* buf = malloc(img_size); 
        bool res = m_pictures[i]->getPictureData(buf, img_size); 
        if (!res) {
            LOGE("get picture data fail!"); 
            free(buf); 
            return false; 
        }
        Image::ptr tmp_image_ptr = Image::TryCreateImage(buf, img_size); 
        dest[i] = tmp_image_ptr; 
        free(buf); 
    }
    return true; 
}

bool MusicDecoderflac::resaveCover(const wchar_t* path, bool ifCheckSuffix, uint32_t index) const {
    if (m_pictures.size()==0) {
        LOGE("no cover!"); 
        return false; 
    }

    if (index>=m_pictures.size()) {
        LOGE("error: invalid index, index should <%d but got %d", m_pictures.size(), index); 
        return false; 
    }

    auto pic = m_pictures[index]; 
    uint32_t pic_size = pic->getPictureDataLength(); 
    void* buf = malloc(pic_size); 
    pic->getPictureData(buf, pic_size); 
    std::string mimeType = pic->getMimeTypeToString(); 
    Image::ptr img = Image::TryCreateImage(buf, pic_size, mimeType); 
    free(buf); 

    if (img==nullptr) {
        LOGE("Unknown img type, covert fail"); 
        return false; 
    }

    bool res = img->resave(path, ifCheckSuffix); 

    return res; 
}

bool MusicDecoderflac::resaveCover(const std::wstring& path, bool ifCheckSuffix, uint32_t index) const {
    return resaveCover(path.c_str(), ifCheckSuffix, index); 
}

std::wstring MusicDecoderflac::checkSuffix(const wchar_t* path) const {
    std::wstring tmp_s(path); 
    size_t t_size = tmp_s.size(); 
    size_t dot_pos = tmp_s.find_last_of(L"."); 
    if (dot_pos!=std::wstring::npos) {
        std::wstring suffix = tmp_s.substr(dot_pos, t_size-dot_pos); 
        if (suffix!=L".flac") {
            tmp_s = tmp_s.substr(0, dot_pos) + L".flac"; 
            LOGI("suffix \".flac\" is adjusted"); 
        }
    } else {
        tmp_s+=L".flac"; 
        LOGI("suffix \".flac\" is added"); 
    }

    return tmp_s; 
}

bool MusicDecoderflac::resave(const wchar_t* path, bool ifCheckSuffix) const {
    if (m_streamInfo==nullptr) {
        LOGE("no stream info block"); 
        return false; 
    }

    std::list<Metadata_block::ptr> allMetaBlock; 

    allMetaBlock.emplace_back(m_streamInfo); 

    if (m_application!=nullptr) {
        allMetaBlock.emplace_back(m_application); 
    }
    if (m_seekTable!=nullptr) {
        allMetaBlock.emplace_back(m_seekTable); 
    }
    if (m_vorbisComment!=nullptr) {
        allMetaBlock.emplace_back(m_vorbisComment);
    }
    if (m_cuesheet!=nullptr) {
        allMetaBlock.emplace_back(m_cuesheet);
    }
    if (m_pictures.size()!=0) {
        for (auto& item: m_pictures) {
            allMetaBlock.emplace_back(item);
        }
    }
    if (m_unknownReservedData.size()!=0) {
        for (auto& item: m_unknownReservedData) {
            allMetaBlock.emplace_back(item);
        }
    }
    if (m_invalidData.size()!=0) {
        for (auto& item:m_invalidData) {
            allMetaBlock.emplace_back(item);
        }
    }
    if (m_padding!=nullptr) {
        allMetaBlock.emplace_back(m_padding);
    }

    ByteArray ba; 

    ba.write(s_label_flac, 4); 

    auto lastIt = allMetaBlock.end(); 
    --lastIt; 
    for (auto it=allMetaBlock.begin(); it!=allMetaBlock.end(); ++it) {
        uint32_t blockSize = (*it)->getBlockSize(); 
        if ((*it)->isDataValid()&&blockSize<=UINT24_MAX) {
            void* pin = malloc(blockSize+4); 
            bool ifLast = it==lastIt; 
            uint32_t ret = (*it)->resave(pin, ifLast); 

            if (ret!=blockSize+4) {
                LOGE("metablock %d resave fail!", (*it)->getBlockType()); 
                return false; 
            }

            ba.write(pin, ret); 

            free(pin); 
        } else {
            LOGE("block not valid, resave termination"); 
            return false; 
        }
    }

    void* audioPin = malloc(m_audioFramesLength); 
    m_audioFrames.getDataBuffers(audioPin, m_audioFramesLength); 
    ba.write(audioPin, m_audioFramesLength); 
    free(audioPin); 

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
    
    size_t dataSize = ba.getSize(); 
    void* metaPin = malloc(dataSize); 
    ba.getDataBuffers(metaPin, dataSize); 
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

bool MusicDecoderflac::resave(const std::wstring& path, bool ifCheckSuffix) const {
    return resave(path.c_str(), ifCheckSuffix); 
}

}