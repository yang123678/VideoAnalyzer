# VideoAnalyzer

## Admin（后台管理模块）

基于Python实现的后台管理模块，提供系统操作界面，通过浏览器进行访问。

## Algorithm（算法模块）

基于Python实现的视频行为分析的算法服务，使用yolo-v5进行检测，在本项目中提供了两种算法的调用方式。
1. Api调用，耦合性较低。
2. c++调用python跨语言调用。

## Analyzer（分析器模块）

视频流的分析器，基于c++开发的视频流分析软件。
1. 主要实现了视频流的拉流、解码、解码之后调用算法实时分析视频流，如果分析出视频流中有报警行为需要合成报警视频，对算法生成的视频流实时编码，实时推流。
2. 实现了音频流解码、音频重采样、音频编码并合成报警视频。
3. 支持多路视频流同时分析，可以作为一个服务动态添加和取消视频流。

## zlm（流媒体模块）

基于zlmediakit实现的流媒体服务，用于将监控视频流拉到实现的服务，然后用算法分析服务转发的视频流。
使用zlmediakit直接编译生成的软件，目前只实现了Windows版本的MediaServer，支持rtmp拉流、rtsp拉流和rtsp、rtmp、flv的分发。
在本项目中，在模拟分析视频中主要使用rtsp、rtmp推流，在分析中使用flv分发的视频流信息。

## 快速运行

按照以下顺序依次启动各个模块：
1. Admin
2. zlm
3. Algorithm
4. Analyzer

相应配置文件与运行示例见各个文件夹中的README文件。
