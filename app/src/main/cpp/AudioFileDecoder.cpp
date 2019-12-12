//
// Created by zu on 2019-12-11.
//

#include "AudioFileDecoder.h"

#define MODULE_NAME  "SLAudioPlayer"
#define LOGV(showLog, ...) if((showLog)) {__android_log_print(ANDROID_LOG_VERBOSE, MODULE_NAME, __VA_ARGS__);}
#define LOGD(showLog, ...) if((showLog)) {__android_log_print(ANDROID_LOG_DEBUG, MODULE_NAME, __VA_ARGS__);}
#define LOGI(showLog, ...) if((showLog)) {__android_log_print(ANDROID_LOG_INFO, MODULE_NAME, __VA_ARGS__);}
#define LOGW(showLog, ...) if((showLog)) {__android_log_print(ANDROID_LOG_WARN, MODULE_NAME, __VA_ARGS__);}
#define LOGE(showLog, ...) if((showLog)) {__android_log_print(ANDROID_LOG_ERROR, MODULE_NAME, __VA_ARGS__);}
#define LOGF(showLog, ...) if((showLog)) {__android_log_print(ANDROID_LOG_FATAL, MODULE_NAME, __VA_ARGS__);}

