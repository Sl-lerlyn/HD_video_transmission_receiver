#pragma once

#include <csignal>
#include <cstring>
#include <sstream>
#include <string>
#include <thread>
#include <queue>
#include <unistd.h>
#include <pthread.h>

#include "AgoraRefCountedObject.h"
#include "IAgoraService.h"
#include "NGIAgoraRtcConnection.h"
#include "common/opt_parser.h"
#include "common/sample_common.h"
#include "common/sample_connection_observer.h"
#include "common/sample_local_user_observer.h"
#include "utils/log.h"
using namespace std;

#define DEFAULT_SAMPLE_RATE (48000)
#define DEFAULT_NUM_OF_CHANNELS (2)
#define DEFAULT_AUDIO_FILE "received_audio.pcm"
#define DEFAULT_VIDEO_FILE "received_video.yuv"
#define DEFAULT_FILE_LIMIT (100 * 1024 * 1024)

struct SampleOptions {
  std::string appId;
  std::string channelId;
  std::string userId;
  std::string remoteUserId;
  std::string audioFile = DEFAULT_AUDIO_FILE;
  std::string videoFile = DEFAULT_VIDEO_FILE;

  struct {
    int sampleRate = DEFAULT_SAMPLE_RATE;
    int numOfChannels = DEFAULT_NUM_OF_CHANNELS;
  } audio;
};

class PcmFrameObserver : public agora::media::IAudioFrameObserver {
 public:
  PcmFrameObserver(const std::string& outputFilePath)
      : outputFilePath_(outputFilePath), pcmFile_(nullptr), fileCount(0), fileSize_(0) {}

  bool onPlaybackAudioFrame(AudioFrame& audioFrame) override { return true; };

  bool onRecordAudioFrame(AudioFrame& audioFrame) override { return true; };

  bool onMixedAudioFrame(AudioFrame& audioFrame) override { return true; };

  bool onPlaybackAudioFrameBeforeMixing(unsigned int uid, AudioFrame& audioFrame) override;

 private:
  std::string outputFilePath_;
  FILE* pcmFile_;
  int fileCount;
  int fileSize_;
};

class YuvFrameObserver : public agora::rtc::IVideoSinkBase {
 public:
  YuvFrameObserver(const std::string& outputFilePath)
      : outputFilePath_(outputFilePath), yuvFile_(nullptr), fileCount(0), fileSize_(0) {}

  int onFrame(const agora::media::base::VideoFrame& videoFrame) override;

  virtual ~YuvFrameObserver() = default;

 private:
  std::string outputFilePath_;
  FILE* yuvFile_;
  int fileCount;
  int fileSize_;
};

/*!
    用于连接至声网服务器，配置token以及channel等参数

    \param 无

    \return 错误码，1表示成功，其它表示失败

    \todo
*/
int connectAgora();

/*!
    用于断开声网服务器，并回收所有临时申请的内存

    \param 无

    \return 错误码，1表示成功，其它表示失败

    \todo
*/
int disconnectAgora();

/*!
    用于接收单帧yuv数据,提供给blackmagic进行播放

    \param[out] frameBuf 指向接收的单帧yuv数据的指针

    \return 错误码，1表示成功，其它表示失败

    \todo
*/
int getFrame(uint8_t* buffer);
int getpcm(uint16_t* buffer);
