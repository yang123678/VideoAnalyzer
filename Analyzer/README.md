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
3. 实现一个动态库，用于实现两种调用方式（Api接口调用、c++调用python）。
   其中c++调用python包括两种方式：
   1.将视频帧使用jpg压缩然后使用base64编码，然后通过接口调用算法服务，算法服务接收到base64编码的数据之后，先解码base64再使用jpg解压缩，然后调用算法模型处理，最后返回结果。
   2.将视频帧使用numpy进行矩阵格式转换，然后可以直接传递到python的函数中，然后可以直接返回结果。
   对于上述两种方法，对于1080p的视频帧，相差10ms左右。
