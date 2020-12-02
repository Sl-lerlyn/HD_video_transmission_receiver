#include "ConnectToAgora.h"

SampleOptions options;
const int tSize = 1920 * 1080;
const int frameSize = 1920 * 1080 * 2;

uint8_t yuv420[frameSize] , yuv422[frameSize];
uint8_t yuvbuf[frameSize];

struct oneyuvframe
{
    uint8_t *buf;
};
struct onepcmframe
{
    uint16_t *buf;
};
queue<oneyuvframe> que;
queue<onepcmframe> pcmque;

static bool exitFlag = false;
static void SignalHandler(int sigNo) { exitFlag = true; }

agora::base::IAgoraService *service;
agora::rtc::AudioSubscriptionOptions audioSubOpt;
agora::rtc::RtcConnectionConfiguration ccfg;
agora::agora_refptr<agora::rtc::IRtcConnection> connection;
agora::rtc::ILocalUser::VideoSubscriptionOptions subscriptionOptions;
agora::agora_refptr<agora::rtc::IVideoSinkBase> yuvFrameObserver;
std::shared_ptr<SampleConnectionObserver> connObserver;
std::shared_ptr<SampleLocalUserObserver> localUserObserver;
std::shared_ptr<PcmFrameObserver> pcmFrameObserver;
size_t writeBytes;
pthread_mutex_t mutexyuv = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexpcm = PTHREAD_MUTEX_INITIALIZER;

bool PcmFrameObserver::onPlaybackAudioFrameBeforeMixing(unsigned int uid, AudioFrame& audioFrame) {
    // Write PCM samples
    writeBytes = audioFrame.samplesPerChannel * audioFrame.channels;

    onepcmframe temp;
    uint16_t* pbuf = new uint16_t[writeBytes];
    for (int i=0; i<writeBytes; i++){
        pbuf[i] = *(uint16_t*)(audioFrame.buffer+i*2);
    }
    temp.buf = pbuf;

    pthread_mutex_lock(&mutexpcm);
    pcmque.push(temp);
    pthread_mutex_unlock(&mutexpcm);
    return true;
}

void convert_yuv420p_to_uyvy422(unsigned char *src, unsigned char *dst, int width,int height)
{
    int i, j;
    unsigned char *pY420_0 = src;
    unsigned char *pY420_1 = src +width;
    unsigned char *pU420 = src + width*height;
    unsigned char *pV420 = src + width*height*5/4;
    unsigned char *pY422_0 = dst;
    unsigned char *pY422_1 = dst+width*2;

    for (i = 0; i < height/2; i++){
        for (j = 0; j < width*2; j +=4){
            *pY422_0++ = *pU420;
            *pY422_1++ = *pU420++;
            *pY422_0++ = *pY420_0++;
            *pY422_1++ = *pY420_1++;

            *pY422_0++ = *pV420;
            *pY422_1++ = *pV420++;
            *pY422_0++ = *pY420_0++;
            *pY422_1++ = *pY420_1++;
        }

        pY420_0 +=width;
        pY420_1 +=width;
        pY422_0+=width*2;
        pY422_1+=width*2;
    }
}

int YuvFrameObserver::onFrame(const agora::media::base::VideoFrame& videoFrame) {


    /*for (int i=0; i<frameSize; i++){
        if (i % 4 == 0)  yuvbuf[i] = videoFrame.uBuffer[i/4];
        else if (i % 4 == 1)  yuvbuf[i] = videoFrame.yBuffer[i/2];
        else if (i % 4 == 2)  yuvbuf[i] = videoFrame.vBuffer[i/4];
        else if (i % 4 == 3)  yuvbuf[i] = videoFrame.yBuffer[i/2];
    }*/

    for (int i=0; i<tSize; i++)  yuv420[i] = videoFrame.yBuffer[i];
    for (int i=tSize; i<tSize*5/4; i++)  yuv420[i] = videoFrame.uBuffer[i-tSize];
    for (int i=tSize*5/4; i<tSize*3/2; i++)  yuv420[i] = videoFrame.vBuffer[i-tSize*5/4];
    convert_yuv420p_to_uyvy422(yuv420, yuv422, 1920, 1080);

    oneyuvframe temp;
    uint8_t* pbuf = new uint8_t[frameSize];
    for (int i=0; i<frameSize; i++){
        //yuvbuf[i] = yuv422[i];
        pbuf[i] = yuv422[i];
    }

    temp.buf = pbuf;


    pthread_mutex_lock(&mutexyuv);
    que.push(temp);
    pthread_mutex_unlock(&mutexyuv);
    return 0;
}



int connectAgora()
{
    options.appId = "00606d5161998b4427e9476ea06b3015425IAAnnq+DUzxxAWIMj8kugImzz3OjTh+MrR79J5jRLE2bbAx+f9gAAAAAEADGU1aHlCavXwEAAQCUJq9f";
    options.channelId = "test";

    if (options.appId.empty()) {
      AG_LOG(ERROR, "Must provide appId!");
      return -1;
    }

    if (options.channelId.empty()) {
      AG_LOG(ERROR, "Must provide channelId!");
      return -1;
    }

    std::signal(SIGQUIT, SignalHandler);
    std::signal(SIGABRT, SignalHandler);
    std::signal(SIGINT, SignalHandler);

    // Create Agora service
    service = createAndInitAgoraService(false, true, true);
    if (!service) {
      AG_LOG(ERROR, "Failed to creating Agora service!");
    }

    // Create Agora connection
    audioSubOpt.bytesPerSample = sizeof(int16_t) * options.audio.numOfChannels;
    audioSubOpt.numberOfChannels = options.audio.numOfChannels;
    audioSubOpt.sampleRateHz = options.audio.sampleRate;

    ccfg.clientRoleType = agora::rtc::CLIENT_ROLE_BROADCASTER;
    ccfg.audioSubscriptionOptions = audioSubOpt;
    ccfg.autoSubscribeAudio = false;
    ccfg.autoSubscribeVideo = false;
    ccfg.enableAudioRecordingOrPlayout = false;  // Subscribe audio but without playback

    connection = service->createRtcConnection(ccfg);
    if (!connection) {
      AG_LOG(ERROR, "Failed to creating Agora connection!");
      return -1;
    }

    // Subcribe streams from all remote users or specific remote user
    agora::rtc::ILocalUser::VideoSubscriptionOptions subscriptionOptions;
    if (options.remoteUserId.empty()) {
      AG_LOG(INFO, "Subscribe streams from all remote users");
      connection->getLocalUser()->subscribeAllAudio();
      connection->getLocalUser()->subscribeAllVideo(subscriptionOptions);
    } else {
      connection->getLocalUser()->subscribeAudio(options.remoteUserId.c_str());
      connection->getLocalUser()->subscribeVideo(options.remoteUserId.c_str(), subscriptionOptions);
    }

    // Register connection observer to monitor connection event
    connObserver = std::make_shared<SampleConnectionObserver>();
    connection->registerObserver(connObserver.get());

    // Create local user observer
    localUserObserver = std::make_shared<SampleLocalUserObserver>(connection->getLocalUser());

    // Register audio frame observer to receive audio stream
    pcmFrameObserver = std::make_shared<PcmFrameObserver>(options.audioFile);
    if (connection->getLocalUser()->setPlaybackAudioFrameBeforeMixingParameters(
            options.audio.numOfChannels, options.audio.sampleRate)) {
      AG_LOG(ERROR, "Failed to set audio frame parameters!");
      return -1;
    }
    localUserObserver->setAudioFrameObserver(pcmFrameObserver.get());

    // Register video frame observer to receive video stream
    yuvFrameObserver = new agora::RefCountedObject<YuvFrameObserver>(options.videoFile);
    localUserObserver->setVideoFrameObserver(yuvFrameObserver);

    // Connect to Agora channel
    if (connection->connect(options.appId.c_str(), options.channelId.c_str(),
                            options.userId.c_str())) {
      AG_LOG(ERROR, "Failed to connect to Agora channel!");
      return -1;
    }

    // Start receiving incoming media data
    AG_LOG(INFO, "Start receiving audio & video data ...");

    // Periodically check exit flag
    //while (!exitFlag) {
     // std::this_thread::sleep_for(std::chrono::milliseconds(10));
    //}

    return 1;
}

int disconnectAgora()
{
    // Unregister audio & video frame observers
    localUserObserver->unsetAudioFrameObserver();
    localUserObserver->unsetVideoFrameObserver();

    // Disconnect from Agora channel
    if (connection->disconnect()) {
      AG_LOG(ERROR, "Failed to disconnect from Agora channel!");
      return -1;
    }

    // Destroy Agora connection and related resources
    localUserObserver.reset();
    pcmFrameObserver.reset();
    yuvFrameObserver = nullptr;
    connection = nullptr;

    // Destroy Agora Service
    service->release();
    service = nullptr;

    while (!que.empty()){
        oneyuvframe temp;
        temp = que.front();
        delete temp.buf;
        que.pop();
    }
    while (!pcmque.empty()){
        onepcmframe temp;
        temp = pcmque.front();
        delete temp.buf;
        pcmque.pop();
    }

    pthread_mutex_destroy(&mutexyuv);
    pthread_mutex_destroy(&mutexpcm);
    AG_LOG(INFO, "Disconnected from Agora channel successfully");

    return 1;
}

int getFrame(uint8_t* buffer)
{
    if (que.empty())  return 0;

    oneyuvframe temp;
    temp = que.front();

    for (int i=0; i<frameSize; i++){
        //buffer[i] = yuvbuf[i];
        buffer[i] = temp.buf[i];
    }

    pthread_mutex_lock(&mutexyuv);
    delete temp.buf;
    que.pop();
    pthread_mutex_unlock(&mutexyuv);
    return 1;
}

int getpcm(uint16_t* buffer)
{
    if (pcmque.empty())  return 0;

    onepcmframe temp;
    for (int j=0; j<4; j++){
        while (pcmque.empty()) std::this_thread::sleep_for(std::chrono::milliseconds(10));
        temp = pcmque.front();
        for (int i=0; i<writeBytes; i++){
            //buffer[i] = yuvbuf[i];
            buffer[i+j*writeBytes] = temp.buf[i];
        }

        pthread_mutex_lock(&mutexpcm);
        delete temp.buf;
        pcmque.pop();
        pthread_mutex_unlock(&mutexpcm);
    }

    return 1;
}
