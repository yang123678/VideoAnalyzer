### 视频行为分析系统 流媒体服务器（基于ZLMediaKit编译）


### ZLMediaKit播放规则
~~~

//rtsp播放
rtsp://127.0.0.1:554/live/test

//rtmp播放
rtmp://127.0.0.1:1935/live/test

//hls播放
http://127.0.0.1:80/live/test/hls.m3u8

//http-flv播放
http://127.0.0.1:80/live/test.live.flv

//http-ts播放
http://127.0.0.1:80/live/test.live.ts

~~~




###  ZLMediaKit推流

~~~
1、使用rtsp方式推流

	# 文件rtsp推流
	ffmpeg -re -i test.mp4 -rtsp_transport tcp -c copy -f rtsp rtsp://127.0.0.1:554/live/test

	# 文件（循环）rtsp推流  [-stream_loop  -1]
	ffmpeg -re -stream_loop  -1  -i test.mp4 -rtsp_transport tcp -c copy -f rtsp rtsp://127.0.0.1:554/live/test


2、使用rtmp方式推流

	# RTMP标准不支持H265,但是国内有自行扩展的，如果你想让FFmpeg支持RTMP-H265,请按照此文章编译：https://github.com/ksvc/FFmpeg/wiki/hevcpush

	# 文件rtmp推流
	ffmpeg -re -i test.mp4 -vcodec h264_nvenc  -acodec aac -f flv  rtmp://192.168.1.3:1935/live/test

	# 文件（循环）rtmp推流  [-stream_loop  -1]
   	ffmpeg -re -stream_loop  -1 -i test.mp4 -vcodec h264_nvenc  -acodec aac -f flv  rtmp://127.0.0.1:1935/live/test
	ffmpeg -re -stream_loop  -1 -i test.mp4 -vcodec h264  -acodec aac -f flv  rtmp://127.0.0.1:1935/live/test

	# 强行修改视频流的分辨率
               -vf scale=1080:1920


~~~





