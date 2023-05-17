import argparse
import sys
import json
import os
import random
import time
from datetime import datetime
import requests

"""
第三方库
pip install requests


# 布控40路视频流不推流
python test.py --api control/add    --repeat 40 --controlCode api --behaviorCode ZHOUJIERUQIN --streamUrl rtmp://127.0.0.1:1935/live/test

# 取消布控40路视频流
python test.py --api control/cancel --repeat 40 --controlCode api

# 查看布控的视频流
python test.py --api control        --repeat 40 --controlCode api

# 查看所有的布控视频流
python test.py --api controls

# 布控1路视频流不推流
python test.py --api control/add   --repeat 1 --controlCode api --behaviorCode ZHOUJIERUQIN --streamUrl rtmp://127.0.0.1:1935/live/test --pushStream 0

# 布控1路视频流并推流
python test.py --api control/add   --repeat 1 --controlCode api --behaviorCode ZHOUJIERUQIN --streamUrl rtmp://127.0.0.1:1935/live/test --pushStream 1 --pushStreamUrl rtmp://127.0.0.1/analyzer/test


"""

class Analyzer():

    def __init__(self,analyzerApiHost):
        self.analyzerApiHost = analyzerApiHost
        self.timeout = 8

    def controls(self):
        """
        """
        __state = False
        __msg = "error"
        __data = []

        try:
            headers = {
                "Content-Type": "application/json;"
            }

            data = {
            }

            data_json = json.dumps(data)

            res = requests.post(url='%s/api/controls' % self.analyzerApiHost, headers=headers,
                                data=data_json, timeout=self.timeout)
            if res.status_code:
                res_result = res.json()
                __msg = res_result["msg"]
                if res_result["code"] == 1000:

                    res_result_data = res_result.get("data")
                    if res_result_data:
                        __data = res_result_data
                    __state = True

            else:
                __msg = "status_code=%d " % (res.status_code)

        except Exception as e:
            __msg = str(e)

        return __state, __msg, __data

    def control(self, code):
        """
        @code   布控编号    [str]  xxxxxxxxx
        """
        __state = False
        __msg = "error"
        __control = {}
        try:
            headers = {
                "Content-Type": "application/json;"
            }
            data = {
                "code": code,
            }

            data_json = json.dumps(data)
            res = requests.post(url='%s/api/control' % self.analyzerApiHost, headers=headers,
                                data=data_json, timeout=self.timeout)
            if res.status_code:
                res_result = res.json()
                __msg = res_result["msg"]
                if res_result["code"] == 1000:
                    __control = res_result.get("control")
                    __state = True

            else:
                __msg = "status_code=%d " % (res.status_code)

        except Exception as e:
            __msg = str(e)

        return __state, __msg, __control

    def control_add(self, code, behaviorCode, streamUrl, pushStream, pushStreamUrl):
        """
        @code          布控编号                    [str]  xxxxxxxxx
        @behaviorCode  布控的视频流处理算法          [str]ZHOUJIERUQIN
        @streamUrl     布控视频流的拉流地址          [str]rtmp://192.168.1.3:1935/live/m2
        @pushStream  布控的视频流经处理后是否推流      [bool] True
        @pushStreamUrl 布控的视频流经过处理的推流地址  [str]rtmp://192.168.1.3:1935/live/m2-behavior
        """
        __state = False
        __msg = "error"

        try:
            headers = {
                "Content-Type": "application/json;"
            }

            data = {
                "code": code,
                "streamUrl": streamUrl,
                "pushStream": pushStream,
                "pushStreamUrl": pushStreamUrl,
                "behaviorCode": behaviorCode,
            }

            data_json = json.dumps(data)

            res = requests.post(url='%s/api/control/add' % self.analyzerApiHost, headers=headers,
                                data=data_json, timeout=self.timeout)
            if res.status_code:
                res_result = res.json()
                __msg = res_result["msg"]
                if res_result["code"] == 1000:
                    __state = True

            else:
                __msg = "status_code=%d " % (res.status_code)

        except Exception as e:
            __msg = str(e)

        return __state, __msg

    def control_cancel(self, code):
        """
        @code   布控编号    [str]  xxxxxxxxx
        """
        __state = False
        __msg = "error"

        try:
            headers = {
                "Content-Type": "application/json;"
            }
            data = {
                "code": code,
            }

            data_json = json.dumps(data)
            res = requests.post(url='%s/api/control/cancel' % self.analyzerApiHost, headers=headers,
                                data=data_json, timeout=self.timeout)
            if res.status_code:
                res_result = res.json()
                __msg = res_result["msg"]
                if res_result["code"] == 1000:
                    __state = True

            else:
                __msg = "status_code=%d " % (res.status_code)

        except Exception as e:
            __msg = str(e)

        return __state, __msg

if __name__ == '__main__':

    parse = argparse.ArgumentParser()
    parse.add_argument("--api", type=str, default="none", help="调试的接口 [control/add,control/cancel,controls,control]")
    parse.add_argument("--repeat", type=int, default=1, help="调试的接口包含于 [control/add,control/cancel,control]时，操作数据重复的数量")
    parse.add_argument("--controlCode", type=str, default="api", help="布控编号")
    parse.add_argument("--behaviorCode", type=str, default="none", help="行为算法编号")
    parse.add_argument("--streamUrl", type=str, default="rtmp://127.0.0.1/live/test", help="拉流地址")
    parse.add_argument("--pushStream", type=int, default=0, help="是否推流")
    parse.add_argument("--pushStreamUrl", type=str, default="rtmp://127.0.0.1/analyzer/test", help="推流地址")
    parse.add_argument("--analyzerApiHost", type=str, default="http://127.0.0.1:9002", help="分析器接口地址")
    
    flags, unparsed = parse.parse_known_args(sys.argv[1:])

    api = flags.api
    repeat = flags.repeat
    controlCode = flags.controlCode
    behaviorCode = flags.behaviorCode
    streamUrl = flags.streamUrl
    pushStream = True if 1==flags.pushStream else False
    pushStreamUrl = flags.pushStreamUrl
    analyzerApiHost = flags.analyzerApiHost


    analyzer = Analyzer(analyzerApiHost=analyzerApiHost)

    if "control/add" == api:
        for i in range(repeat):
            t1 = time.time()
            _code = "%s-%d" % (controlCode,i)

            __state, __msg = analyzer.control_add(
                code=_code,
                behaviorCode=behaviorCode,
                streamUrl=streamUrl,
                pushStream=pushStream,
                pushStreamUrl="%s-%s"%(pushStreamUrl,_code)
            )
            t2 = time.time()

            print("api=%s,code=%s,耗时 %.5f 秒,__state=%d " % (api, _code, (t2 - t1), __state), "---->", __msg)

    elif "control/cancel" == api:
        for i in range(repeat):
            t1 = time.time()
            _code = "%s-%d" % (controlCode,i)
            __state, __msg = analyzer.control_cancel(code=_code)

            t2 = time.time()

            print("api=%s,code=%s,耗时 %.5f 秒,__state=%d " % (api, _code, (t2 - t1), __state), "---->", __msg)

    elif "controls" == api:
        t1 = time.time()
        __state, __msg, __data = analyzer.controls()
        t2 = time.time()

        checkFpsTotal = 0
        checkFpsAvg = 0

        if len(__data) > 0:
            for __d in __data:
                checkFpsTotal += float(__d["checkFps"])

            checkFpsAvg = checkFpsTotal / len(__data)

        print("api=%s,耗时 %.5f 秒,__state=%d, checkFpsAvg=%.4f, 分析器服务当前布控总数量 len(__data)=%d " % (api, (t2 - t1), __state,
                                                                                               checkFpsAvg,
                                                                                               len(__data)), "---->",
              __msg)

        # if len(__data) > 0:
        #    print("\t第一条数据：",__data[0])

    elif "control" == api:
        for i in range(repeat):
            t1 = time.time()
            _code = "%s-%d" % (controlCode,i)
            __state, __msg, __control = analyzer.control(code=_code)

            t2 = time.time()

            print("api=%s,code=%s,耗时 %.5f 秒,__state=%d " % (api, _code, (t2 - t1), __state), "---->", __msg, __control)

    else:
        print("未定义的api=%s" % api)








