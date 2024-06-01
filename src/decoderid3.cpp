#include "decoderid3.h"
#include "utils.h"
#include "log.h"

#include <stdlib.h>
#include <stdio.h>

namespace music_data {

INITONLYLOGGER(); 

ID3Tag::ptr ID3Tag::TryCreateID3Tag(void* data, size_t length) {
    ID3Tag::ptr ans(new ID3v1(data, length)); 
    if (ans->isDataValid()) {
        return ans; 
    }

    ans.reset(new ID3v2(data, length)); 
    if (ans->isDataValid()) {
        return ans; 
    }

    return nullptr; 
}

ID3Tag::ID3Tag(ID3TagType type, uint32_t datasize, bool dataValid)
    : m_tagType(type)
    , m_size(datasize)
    , m_dataValided(dataValid) {
}

ID3Tag::~ID3Tag() {
}

ID3v1::ID3v1(void* data, uint32_t length)
    : ID3Tag(ID3TagType::ID3V1, length) {
    if (length==128) {
        initTag(data); 
    } else {
        LOGE("invalid length for ID3v1: length should be 128 but length=%d\n", length); 
        setDataValid(false); 
    }
}

ID3v1::~ID3v1() {
}

void ID3v1::initTag(void* data) {
    uint8_t* pin = (uint8_t*)data; 

    char tagLabel[4]; 
    strncpy(tagLabel, (const char*)pin, 3); 
    tagLabel[3]='\0'; 
    if (!isID3v1(tagLabel)) {
        LOGE("file is not ID3v1"); 
        setDataValid(false); 
        return; 
    }
    pin+=3; 

    strncpy(m_title, (const char*)pin, ID3V1_TITLE_SIZE); 
    pin+=ID3V1_TITLE_SIZE; 

    strncpy(m_artist, (const char*)pin, ID3V1_ARTIST_SIZE); 
    pin+=ID3V1_ARTIST_SIZE; 

    strncpy(m_album, (const char*)pin, ID3V1_ALBUM_SIZE); 
    pin+=ID3V1_ALBUM_SIZE; 

    strncpy(m_year, (const char*)pin, ID3V1_YEAR_SIZE); 
    pin+=ID3V1_YEAR_SIZE; 

    strncpy(m_comment, (const char*)pin, ID3V1_COMMENT_SIZE); 
    pin+=ID3V1_COMMENT_SIZE; 

    m_reserved = *pin; 
    pin+=1; 

    m_trackNum = *pin; 
    pin+=1; 

    m_genre = *pin; 
}

ID3v2::ID3v2(void* data, uint32_t length)
    : ID3Tag(ID3TagType::ID3V2, length) {
    if (length>10) {
        initTag(data); 
    } else {
        LOGE("invalid length for ID3v2: length should be >10 but length=%d\n", length); 
        setDataValid(false); 
    }
}

ID3v2::~ID3v2() {
}

void ID3v2::initTag(void* data) {
    uint8_t* pin = (uint8_t*)data; 

    char label[4]; 
    strncpy(label, (const char*)pin, 3); 
    label[3]='\0'; 
    if (!isID3v2(label)) {
        LOGE("file is not ID3v2"); 
        setDataValid(false); 
        return; 
    }
    pin+=3; 
    
    m_header.version = byteswap(*(uint16_t*)pin); 
    pin+=2; 

    m_header.flags = *pin; 
    pin+=1; 

    m_header.size = byteswap(*(uint32_t*)pin); 
    pin+=4; 

    uint32_t dataSize = (m_header.size&0x7)|((m_header.size&0x70)>>1)
                            |((m_header.size&0x700)>>2)|((m_header.size&0x7000)>>3); 

    if (dataSize!=getTagSize()-10) {
        LOGE("size error, should be %d but got %d", getTagSize()-10, dataSize); 
        setDataValid(false); 
        return; 
    }

    // 是否有extended header
    if ((m_header.flags>>6)&0x1) {
        m_extendedHeader.reset(new TagExtendedHeader); 

        m_extendedHeader->extendedHeaderSize = byteswap(*(uint32_t*)pin); 
        pin+=4; 

        m_extendedHeader->extendedFlags = byteswap(*(uint16_t*)pin); 
        pin+=2; 

        m_extendedHeader->sizeofPadding = byteswap(*(uint32_t*)pin); 
        pin+=4; 

        dataSize-=10+m_extendedHeader->sizeofPadding; 
    }

    while (dataSize>0) {
        TagFrame newFrame((void*)pin); 
        pin+=10+newFrame.frameSize; 
        dataSize-=10+newFrame.frameSize; 

        // Frame大小不合理
        if (dataSize<0) {
            LOGE("Frame size not correct with total tag size"); 
            setDataValid(false); 
            return; 
        }

        m_frames.emplace_back(newFrame); 
    }
}

ID3v2::TagFrame::TagFrame(void* data) {
    uint8_t* pin = (uint8_t*)data; 

    strncpy(frameId, (const char*)pin, ID3V2_FRAMEID_SIZE); 
    pin+=ID3V2_FRAMEID_SIZE; 

    frameSize = byteswap(*(uint32_t*)pin); 
    pin+=4; 

    flags = byteswap(*(uint16_t*)pin); 
    pin+=2; 

    if (frameSize>0) {
        if (this->data!=nullptr) {
            delete[] this->data; 
        }
        this->data = new char[frameSize]; 
        strncpy(this->data, (const char*)pin, frameSize); 
    }
}

ID3v2::TagFrame::~TagFrame() {
    if (data!=nullptr) {
        delete[] data; 
    }
}

}