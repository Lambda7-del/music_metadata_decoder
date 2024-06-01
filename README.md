# music_metadata_decoder
基于Windows平台的音频(flac/ID3)metadata的解码与编辑工具C++

## 文件说明
1、decoder.h 解码器基类  
2、decoderflac.h flac文件解码编辑器  
3、decoderid3.h ID3v1与ID3v2标签解码  
4、image.h 封面图片封装  

## 实现功能
1、flac文件metadata读取解析  
2、flac文件metadata编辑，picture，vorbisComment，streaminfo的相关操作已测试 (其它block大部分flac文件都不会有，懒得找文件测了)  
3、flac文件另存，flac文件封面图片另存  
4、其他音频格式解析随缘更新   
