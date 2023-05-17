###### Analyzer

#### 介绍
视频流实时分析器


#### 编译
~~~
windows版支持 x64/Release,x64/Debug

~~~

#### 第三方库

1.  ffmpeg [ffmpeg](http://ffmpeg.org/)
2.  curl
3.  event
4.  jpeg-turbo
5.  jsoncpp
6.  numpy 
7.  opencv [opencv](https://opencv.org/)
8.  python [python](https://www.python.org//)

## 主要包括三个部分

1. 主项目，包括拉流、解码、实时分析、编码、推流等功能的实现。
2. 实现一个动态库，用于合成报警视频。
3. 实现一个动态库，用于实现两种调用方式（Api接口调用、c++调用python）
