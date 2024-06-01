#include "decoderflac.h"
#include "log.h"
#include "bytearray.h"
#include "image.h"

#include <iostream>
#include <iomanip>

INITONLYLOGGER(); 

void test_loadflac() {
    std::wstring fn = L"D:\\projects\\C++\\musicmetadata\\bin\\tmp\\01. 虹ヶ咲学園校歌 (Rock Ver.).flac";  
    // std::wstring fn = L"D:\\projects\\C++\\musicmetadata\\bin\\tmp\\01. 燦々.flac";  
    // std::wstring fn = L"D:\\projects\\C++\\musicmetadata\\bin\\tmp\\Beat in Angel - 星空 凛(CV.飯田里穂); 西木野 真姫(CV.Pile).flac";  
    // std::wstring fn = L"D:\\projects\\C++\\musicmetadata\\bin\\tmp\\ff-16b-2c-44100hz.flac";  
    // std::wstring fn = L"D:\\projects\\C++\\musicmetadata\\bin\\tmp\\sample-1.flac";  


    music_data::MusicDecoderflac flac_data(fn); 

    auto siptr = flac_data.getStreamInfo(); 
    // siptr.reset(); 
    // if (siptr==nullptr) printf("goood\n"); 

    auto pdlist = flac_data.getPadding(); 

    auto vbptr = flac_data.getVorbisComment(); 

    int length = siptr->getBlockSize()+4; 

    char a[length]; 

    siptr->resave(a);  

    uint8_t* pin = (uint8_t*)a; 
    for (int i=0; i<length; ++i) {
        if (i%16==0) std::cout << std::setfill('0') << std::setw(6) << i << ": \t"; 
        std::cout << std::hex << std::setfill('0') << std::setw(2) << (int)*pin << "\t"; 
        ++pin; 
        if (i%16==15) std::cout << std::endl; 
    }

    LOGD("break dot\n"); 
}

void test_getInfo() {
    // std::wstring fn = L"D:\\projects\\C++\\musicmetadata\\bin\\tmp\\01. 虹ヶ咲学園校歌 (Rock Ver.).flac";  
    // std::wstring fn = L"D:\\projects\\C++\\musicmetadata\\bin\\tmp\\01. 燦々.flac";  
    std::wstring fn = L"D:\\projects\\C++\\musicmetadata\\bin\\tmp\\Beat in Angel - 星空 凛(CV.飯田里穂); 西木野 真姫(CV.Pile).flac";  
    // std::wstring fn = L"D:\\projects\\C++\\musicmetadata\\bin\\tmp\\ff-16b-2c-44100hz.flac";  

    music_data::MusicDecoderflac flac_data(fn); 

    auto info = flac_data.getTitle(); 

    LOGD("%s\n", info.c_str()); 

    // printf("%s\n", info.c_str()); 

    LOGD("break dot\n"); 
}

void test_streamInfo() {
    std::wstring fn = L"D:\\projects\\C++\\musicmetadata\\bin\\tmp\\Beat in Angel - 星空 凛(CV.飯田里穂); 西木野 真姫(CV.Pile).flac";  
    music_data::MusicDecoderflac flac_data(fn); 

    auto siptr = flac_data.getStreamInfo(); 

    {siptr->setMinBlockSize(123); 
    if (siptr->getMinBlockSize()!=123) {
        LOGF("res = %d, not %d", siptr->getMinBlockSize(), 123); 
    } else {
        LOGT("res = %d, got %d", siptr->getMinBlockSize(), 123); 
    }}

    {siptr->setMaxBlockSize(456); 
    if (siptr->getMaxBlockSize()!=456) {
        LOGF("res = %d, not %d", siptr->getMaxBlockSize(), 456); 
    } else {
        LOGT("res = %d, got %d", siptr->getMaxBlockSize(), 456); 
    }}

    {siptr->setMinFrameSize(123); 
    if (siptr->getMinFrameSize()!=123) {
        LOGF("res = %d, not %d", siptr->getMinFrameSize(), 123); 
    } else {
        LOGT("res = %d, got %d", siptr->getMinFrameSize(), 123); 
    }}

    {auto minFrameSize = siptr->getMinFrameSize(); 
    bool smfs_res = siptr->setMinFrameSize(0x1FFFFFF); 
    if (smfs_res) {
        LOGF("res = %d, not %d", false, smfs_res); 
    } else {
        LOGT("res = %d, got %d", false, smfs_res); 
    }
    if (siptr->getMinFrameSize()!=minFrameSize) {
        LOGF("res = %d, not %d", siptr->getMinFrameSize(), minFrameSize); 
    } else {
        LOGT("res = %d, got %d", siptr->getMinFrameSize(), minFrameSize); 
    }}

    {siptr->setMaxFrameSize(456); 
    if (siptr->getMaxFrameSize()!=456) {
        LOGF("res = %d, not %d", siptr->getMaxFrameSize(), 456); 
    } else {
        LOGT("res = %d, not %d", siptr->getMaxFrameSize(), 456); 
    }}

    {auto maxFrameSize = siptr->getMaxFrameSize(); 
    auto smfs_res = siptr->setMaxFrameSize(0xFFFFFFF); 
    if (smfs_res) {
        LOGF("res = %d, not %d", false, smfs_res); 
    } else {
        LOGT("res = %d, not %d", false, smfs_res); 
    }
    if (siptr->getMaxFrameSize()!=maxFrameSize) {
        LOGF("res = %d, not %d", siptr->getMaxFrameSize(), maxFrameSize); 
    } else {
        LOGT("res = %d, not %d", siptr->getMaxFrameSize(), maxFrameSize); 
    }}

    {siptr->setSampleRate(5466); 
    if (siptr->getSampleRate()!=5466) {
        LOGF("res = %d, not %d", siptr->getSampleRate(), 5466); 
    } else {
        LOGT("res = %d, not %d", siptr->getSampleRate(), 5466); 
    }}

    {siptr->setSampleRate(0xFFFFF); 
    if (siptr->getSampleRate()!=0xFFFFF) {
        LOGF("res = %d, not %d", siptr->getSampleRate(), 0xFFFFF); 
    } else {
        LOGT("res = %d, not %d", siptr->getSampleRate(), 0xFFFFF); 
    }}

    {auto sbs_ans = siptr->getSampleRate(); 
    bool bool_res = siptr->setSampleRate(0x1FFFFF); 
    if (bool_res) {
        LOGF("res = %d, not %d", false, bool_res); 
    } else {
        LOGT("res = %d, not %d", false, bool_res); 
    }
    if (siptr->getSampleRate()!=sbs_ans) {
        LOGF("res = %d, not %d", siptr->getSampleRate(), sbs_ans); 
    } else {
        LOGT("res = %d, not %d", siptr->getSampleRate(), sbs_ans); 
    }}

    {siptr->setChannels(5); 
    if (siptr->getChannels()!=5) {
        LOGF("res = %d, not %d", siptr->getChannels(), 5); 
    } else {
        LOGT("res = %d, not %d", siptr->getChannels(), 5); 
    }}

    {siptr->setChannels(1); 
    if (siptr->getChannels()!=1) {
        LOGF("res = %d, not %d", siptr->getChannels(), 1); 
    } else {
        LOGT("res = %d, not %d", siptr->getChannels(), 1); 
    }}

    {siptr->setChannels(8); 
    if (siptr->getChannels()!=8) {
        LOGF("res = %d, not %d", siptr->getChannels(), 8); 
    } else {
        LOGT("res = %d, not %d", siptr->getChannels(), 8); 
    }}

    {auto sc_ans = siptr->getChannels(); 
    bool bool_res = siptr->setChannels(0); 
    if (bool_res) {
        LOGF("res = %d, not %d", false, bool_res); 
    } else {
        LOGT("res = %d, not %d", false, bool_res); 
    }
    if (siptr->getChannels()!=sc_ans) {
        LOGF("res = %d, not %d", siptr->getChannels(), sc_ans); 
    } else {
        LOGT("res = %d, not %d", siptr->getChannels(), sc_ans); 
    }}

    {auto sc_ans = siptr->getChannels(); 
    bool bool_res = siptr->setChannels(9); 
    if (bool_res) {
        LOGF("res = %d, not %d", false, bool_res); 
    } else {
        LOGT("res = %d, not %d", false, bool_res); 
    }
    if (siptr->getChannels()!=sc_ans) {
        LOGF("res = %d, not %d", siptr->getChannels(), sc_ans); 
    } else {
        LOGT("res = %d, not %d", siptr->getChannels(), sc_ans); 
    }}

    {siptr->setSampleBits(4); 
    if (siptr->getSampleBits()!=4) {
        LOGF("res = %d, not %d", siptr->getSampleBits(), 4); 
    } else {
        LOGT("res = %d, not %d", siptr->getSampleBits(), 4); 
    }}

    {siptr->setSampleBits(32); 
    if (siptr->getSampleBits()!=32) {
        LOGF("res = %d, not %d", siptr->getSampleBits(), 32); 
    } else {
        LOGT("res = %d, not %d", siptr->getSampleBits(), 32); 
    }}

    {siptr->setSampleBits(16); 
    if (siptr->getSampleBits()!=16) {
        LOGF("res = %d, not %d", siptr->getSampleBits(), 16); 
    } else {
        LOGT("res = %d, not %d", siptr->getSampleBits(), 16); 
    }}

    {auto sc_ans = siptr->getSampleBits(); 
    auto bool_res = siptr->setSampleBits(1); 
    if (bool_res) {
        LOGF("res = %d, not %d", false, bool_res); 
    } else {
        LOGT("res = %d, not %d", false, bool_res); 
    }
    if (siptr->getSampleBits()!=sc_ans) {
        LOGF("res = %d, not %d", siptr->getSampleBits(), sc_ans); 
    } else LOGT("res = %d, not %d", siptr->getSampleBits(), sc_ans); }

    {auto sc_ans = siptr->getSampleBits(); 
    auto bool_res = siptr->setSampleBits(54); 
    if (bool_res) {
        LOGF("res = %d, not %d", false, bool_res); 
    } else LOGT("res = %d, not %d", false, bool_res); 
    if (siptr->getSampleBits()!=sc_ans) {
        LOGF("res = %d, not %d", siptr->getSampleBits(), sc_ans); 
    } else LOGT("res = %d, not %d", siptr->getSampleBits(), sc_ans); }

    {siptr->setSamplePerChannel(15756); 
    if (siptr->getSamplePerChannel()!=15756) {
        LOGF("res = %d, not %d", siptr->getSamplePerChannel(), 15756); 
    } else LOGT("res = %d, not %d", siptr->getSamplePerChannel(), 15756); }

    {
        auto val = 0xFFFFFFFFF; 
        siptr->setSamplePerChannel(val); 
        if (siptr->getSamplePerChannel()!=val) {
            LOGF("res = %d, not %d", siptr->getSamplePerChannel(), val); 
        } else LOGT("res = %d, not %d", siptr->getSamplePerChannel(), val); 
    }

    {
        auto moto = siptr->getSamplePerChannel(); 
        auto res = siptr->setSamplePerChannel(0x1FFFFFFFFF); 
        if (res) {
            LOGF("res = %d, not %d", false, res); 
        } else LOGT("res = %d, got %d", false, res); 
        if (siptr->getSamplePerChannel()!=moto) {
            LOGF("res = %d, not %d", siptr->getSamplePerChannel(), moto); 
        } else LOGT("res = %d, got %d", siptr->getSamplePerChannel(), moto); 
    }

    {
        char data[124]; 
        bool res = siptr->getUnencoderedMD5(data, 124); 
        if (res) {
            LOGF("res = %d, not %d", false, res); 
        } else LOGT("res = %d, got %d", false, res); 
    }

    {
        char data[16]; 
        bool res = siptr->getUnencoderedMD5(data, 16); 
        if (!res) {
            LOGF("res = %d, not %d", false, res); 
        } else LOGT("res = %d, got %d", false, res); 
    }

    {
        char data[16]; 
        std::fill(data, data+16, 'd'); 
        bool res = siptr->setUnencoderedMD5(data, 16); 
        if (!res) {
            LOGF("res = %d, not %d", false, res); 
        } else LOGT("res = %d, got %d", false, res); 
        char backdata[16]; 
        res = siptr->getUnencoderedMD5(backdata, 16); 
        if (memcmp(data, backdata, 16)!=0) {
            LOGF("data!=backdata"); 
        } else LOGT("data==backdata"); 
    }

    {
        if (siptr->getBlockSize()!=34) {
            LOGF("res = %d, not %d", 34, siptr->getBlockSize()); 
        } else LOGT("res = %d, got %d", 34, siptr->getBlockSize()); 
    }
}

#define TEST(ans, res) \
    if (ans!=res) { \
        LOGF("res should be %d, not %d", ans, res); \
    } else { \
        LOGT("res should be %d, got %d", ans, res); \
    }

#define TEST_STRING(ans, res) \
    if (ans!=res) { \
        LOGF("res should be %s, not %s", ans, res); \
    } else { \
        LOGT("res should be %s, got %s", ans, res); \
    }

#define TEST_INT64(ans, res) \
    if (ans!=res) { \
        LOGF("res should be %lld, not %lld", ans, res); \
    } else { \
        LOGT("res should be %lld, got %lld", ans, res); \
    }


void test_padding() {
    std::wstring fn = L"D:\\projects\\C++\\musicmetadata\\bin\\tmp\\Beat in Angel - 星空 凛(CV.飯田里穂); 西木野 真姫(CV.Pile).flac";  
    music_data::MusicDecoderflac flac_data(fn); 

    auto pdptr = flac_data.getPadding(); 

    {
        auto size = pdptr->getBlockSize(); 
        pdptr->addPaddingByte(5); 
        TEST(size+5, pdptr->getBlockSize()); 
    }
    {
        auto size = pdptr->getBlockSize(); 
        pdptr->delPaddingByte(5); 
        TEST(size-5, pdptr->getBlockSize()); 
    }
    {
        auto size = pdptr->getBlockSize(); 
        pdptr->addPaddingByte(UINT24_MAX-size); 
        TEST(UINT24_MAX, pdptr->getBlockSize()); 
    }
    {
        auto size = pdptr->getBlockSize(); 
        pdptr->addPaddingByte(1); 
        TEST(UINT24_MAX, pdptr->getBlockSize()); 
    }
    {
        auto size = pdptr->getBlockSize(); 
        pdptr->delPaddingByte(UINT24_MAX); 
        TEST(0, pdptr->getBlockSize()); 
    }
    {
        auto size = pdptr->getBlockSize(); 
        pdptr->delPaddingByte(1); 
        TEST(0, pdptr->getBlockSize()); 
    }
}

void test_bytearray() {
    {
        music_data::ByteArray ba; 
        char a[]="abcd"; 
        ba.rewrite(a, 5); 
        char b[5]; 
        ba.read(b, 4, 1); 
        int res = strcmp(a+1, b); 
        TEST(0, res); 
    }
}

void test_vbcmt() {
    std::wstring fn = L"D:\\projects\\C++\\musicmetadata\\bin\\tmp\\Beat in Angel - 星空 凛(CV.飯田里穂); 西木野 真姫(CV.Pile).flac";  
    music_data::MusicDecoderflac flac_data(fn); 

    auto vcptr = flac_data.getVorbisComment(); 

    {
        char out[vcptr->getEncoderIdentificationLength()]; 
        vcptr->getEncoderIdentification(out, vcptr->getEncoderIdentificationLength()); 
        std::string out_s = vcptr->getEncoderIdentificationToString(); 

        TEST(vcptr->getEncoderIdentificationLength(), out_s.size()); 
    }

    {
        std::string ne = "ttt"; 
        bool res = vcptr->setEncoderIdentification(ne); 
        TEST(true, res); 
        TEST(ne.size(), vcptr->getEncoderIdentificationLength()); 
        TEST_STRING(ne, vcptr->getEncoderIdentificationToString()); 
        char nec[4]; 
        res=vcptr->getEncoderIdentification(nec, 3); 
        nec[3]='\0'; 
        TEST(true, res); 
        TEST_STRING(ne, std::string(nec)); 
        res=vcptr->getEncoderIdentification(nec, 4); 
        TEST(false, res); 
    }

    {
        std::string ans = vcptr->getEncoderIdentificationToString(); 
        std::string ns(0xFFFFFF, 'a'); 
        bool res = vcptr->setEncoderIdentification(ns); 
        TEST(false, res); 
        TEST_STRING(ans, vcptr->getEncoderIdentificationToString()); 
    }

    {
        auto yl = vcptr->getLabelNum(); 
        std::string k="ARTIST"; 
        std::string v="me"; 
        bool res = vcptr->addInfoLabel(k, v, 1); 
        TEST(true, res); 
        TEST_STRING(v, vcptr->getLabelWithKey(k, 1)); 
        TEST(yl+1, vcptr->getLabelNum()); 
        res = vcptr->addInfoLabel(k, v); 
        TEST(true, res); 
        TEST(yl+2, vcptr->getLabelNum()); 
        std::vector<std::string> vec; 
        vcptr->getLabelListWithKey(k, vec); 
        TEST_STRING(v, vec[vec.size()-1]); 
        res = vcptr->delAllMatchInfoLabel(k, v); 
        TEST(yl, vcptr->getLabelNum()); 
        vcptr->addInfoLabel(k, v, 1); 
        vcptr->addInfoLabel(k, v); 
        TEST(yl+2, vcptr->getLabelNum()); 
        res = vcptr->delInfoLabel(k, v); 
        TEST(true, res); 
        TEST(yl+1, vcptr->getLabelNum()); 
        vcptr->getLabelListWithKey(k, vec); 
        TEST_STRING(v, vec[vec.size()-1]); 
        vcptr->addInfoLabel(k, v, 1); 
        vcptr->addInfoLabel(k, v); 
        vcptr->deduplication(); 
        TEST(yl+1, vcptr->getLabelNum()); 
        
        auto tmp_s = vcptr->getLabelWithKey(k, 2); 
        vcptr->delInfoLabel(k, 2); 
        printf("\n"); 
        vcptr->addInfoLabel(k, tmp_s, 2); 
    }

    {
        vcptr->setLabelVal("TITLE", "666"); 
        TEST_STRING("666", vcptr->getLabelWithKey("TITLE")); 

        vcptr->setLabelVal("ARTIST", "66"); 
        TEST_STRING("66", vcptr->getLabelWithKey("ARTIST")); 

        vcptr->addInfoLabel("ABC", "d", 5); 
        vcptr->addInfoLabel("ABC", "e", 0); 

        std::string x(0xFFFFFF, 'a'); 
        bool res = vcptr->addInfoLabel("ABC", x); 
        TEST(false, res); 
        int ret = vcptr->setLabelVal("ABC", x, 1); 
        TEST(3, ret); 
        ret = vcptr->setLabelVal("ABC", "dag", 6); 
        TEST(2, ret); 
        ret = vcptr->setLabelVal("ABDC", "dag", 6); 
        TEST(1, ret); 
    }
}

void test_flac() {
    std::wstring fn = L"D:\\projects\\C++\\musicmetadata\\bin\\tmp\\Beat in Angel - 星空 凛(CV.飯田里穂); 西木野 真姫(CV.Pile).flac";  
    music_data::MusicDecoderflac flac_data(fn); 

    {
        flac_data.setVorbisCommentLabel("TITLE", "new title"); 
        TEST_STRING("new title", flac_data.getTitle()); 
    }
}

void test_resave() {
    std::wstring fn = L"D:\\projects\\C++\\musicmetadata\\bin\\tmp\\Beat in Angel - 星空 凛(CV.飯田里穂); 西木野 真姫(CV.Pile).flac";  
    music_data::MusicDecoderflac flac_data(fn);

    flac_data.setbackTitle("beat out demon"); 

    flac_data.setbackTrack(2); 
    flac_data.addbackArtist("あおい", 1); 

    std::wstring rs = L"D:\\projects\\C++\\musicmetadata\\bin\\tmp\\resave_test.flac";  
    flac_data.resave(rs); 
}

void test_pic() {
    std::wstring fn = L"D:\\projects\\C++\\musicmetadata\\bin\\tmp\\Beat in Angel - 星空 凛(CV.飯田里穂); 西木野 真姫(CV.Pile).flac";  
    music_data::MusicDecoderflac flac_data(fn); 

    std::vector<music_data::PictureMetaBlock::ptr> pics; 
    bool res = flac_data.getPictures(pics); 

    TEST(true, res); 

    LOGI("pic_num=%lld", pics.size()); 

    music_data::ByteArray ba; 
    auto& pic = pics[0]; 
    uint32_t pic_size = pic->getPictureDataLength(); 
    void* pin = malloc(pic_size); 
    pic->getPictureData(pin, pic_size); 
    ba.write(pin, 8); 
    free(pin); 

    std::string img_type = pic->getMimeTypeToString(); 
    LOGI("type: %s", img_type.c_str()); 

    ba.setPosition(0); 
    std::string hexstring = ba.toHexString(); 
    LOGI("%s", hexstring.c_str()); 
}

void test_img() {
    std::wstring img = L"D:\\projects\\C++\\musicmetadata\\bin\\tmp\\W5.png"; 
    // std::wstring img = L"D:\\projects\\C++\\musicmetadata\\bin\\tmp\\W3.jpg"; 

    // music_data::PngImage img_data(img); 
    // music_data::JpegImage img_data(img); 

    // std::wstring fn = L"D:\\projects\\C++\\musicmetadata\\bin\\tmp\\Beat in Angel - 星空 凛(CV.飯田里穂); 西木野 真姫(CV.Pile).flac"; 
    std::wstring fn = L"D:\\projects\\C++\\musicmetadata\\bin\\tmp\\01. 虹ヶ咲学園校歌 (Rock Ver.).flac";  
    // std::wstring fn = L"D:\\projects\\C++\\musicmetadata\\bin\\tmp\\01. 燦々.flac"; 
    std::wstring rs_flac = L"D:\\projects\\C++\\musicmetadata\\bin\\tmp\\resave_test.flac";  
    music_data::MusicDecoderflac flac_data(fn); 

    // std::vector<music_data::Image::ptr> covers; 
    // flac_data.getCovers(covers); 

    // flac_data.addbackCover(img_data); 

    // flac_data.delbackCover(1); 

    // flac_data.setbackCover(img_data); 

    // std::wstring rs = L"D:\\projects\\C++\\musicmetadata\\bin\\tmp\\resave_img_test.jpeg"; 
    std::wstring rs = L"D:\\projects\\C++\\musicmetadata\\bin\\tmp\\resave_img_test.png";  

    // img_data.resave(rs); 

    flac_data.resaveCover(rs, true); 

    printf("\n"); 
}

void test_resetPos() {
    std::wstring fn = L"D:\\projects\\C++\\musicmetadata\\bin\\tmp\\Beat in Angel - 星空 凛(CV.飯田里穂); 西木野 真姫(CV.Pile).flac"; 
    // std::wstring fn = L"D:\\projects\\C++\\musicmetadata\\bin\\tmp\\01. 虹ヶ咲学園校歌 (Rock Ver.).flac";  
    // std::wstring fn = L"D:\\projects\\C++\\musicmetadata\\bin\\tmp\\01. 燦々.flac"; 
    std::wstring rs_flac = L"D:\\projects\\C++\\musicmetadata\\bin\\tmp\\resave_test.flac";  
    music_data::MusicDecoderflac flac_data(fn); 

    std::wstring img = L"D:\\projects\\C++\\musicmetadata\\bin\\tmp\\W5.png"; 
    music_data::PngImage img_data(img); 

    flac_data.addbackCover(img_data); 

    flac_data.resetPosbackCover(0, 1); 

    // flac_data.addbackArtist("new peple"); 

    // flac_data.resetPosbackArtist(0, 2); 

    flac_data.resave(rs_flac, true); 

    printf("\n"); 
}

int main(int argc, char** argv) {
    test_resetPos(); 
    
    return 0; 
}