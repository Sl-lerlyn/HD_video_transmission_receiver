#pragma once 

#include "DeckLinkAPI.h"

#include <QMessageBox>
#include <QWidget>

#include <condition_variable>
#include <functional>
#include <map>
#include <memory>
#include <mutex>

#include "com_ptr.h"
#include "DeckLinkOpenGLWidget.h"
#include "DeckLinkOutputDevice.h"
#include "DeckLinkDeviceDiscovery.h"
#include "ProfileCallback.h"

#include "ui_SignalGenerator.h"

class Timecode
{
public:
	Timecode(int f, int d)
		: fps(f), framecount(0), dropframes(d), frames_(0),seconds_(0),minutes_(0),hours_(0)
	{
	}
	void update()
	{
		unsigned long frameCountNormalized = framecount++;

		if (dropframes)
		{
			int deciMins, deciMinsRemainder;

			int framesIn10mins = (60 * 10 * fps) - (9 * dropframes);
			deciMins = frameCountNormalized / framesIn10mins;
			deciMinsRemainder = frameCountNormalized - (deciMins * framesIn10mins);

			// Add drop frames for 9 minutes of every 10 minutes that have elapsed
			// AND drop frames for every minute (over the first minute) in this 10-minute block.
			frameCountNormalized += dropframes * 9 * deciMins;
			if (deciMinsRemainder >= dropframes)
				frameCountNormalized += dropframes * ((deciMinsRemainder - dropframes) / (framesIn10mins / 10));
		}

		frames_ = (int)(frameCountNormalized % fps);
		frameCountNormalized /= fps;
		seconds_ = (int)(frameCountNormalized % 60);
		frameCountNormalized /= 60;
		minutes_ = (int)(frameCountNormalized % 60);
		frameCountNormalized /= 60;
		hours_ = (int)frameCountNormalized;
	}
	int hours() const { return hours_; }
	int minutes() const { return minutes_; }
	int seconds() const { return seconds_; }
	int frames() const { return frames_; }
private:
	int fps;
	unsigned long framecount;
	int dropframes;
	int frames_;
	int seconds_;
	int minutes_;
	int hours_;
};

enum OutputSignal
{
	kOutputSignalPip		= 0,
	kOutputSignalDrop		= 1
};

class SignalGenerator : public QDialog
{
	Q_OBJECT

	using FillFrameFunction = std::function<void(com_ptr<IDeckLinkMutableVideoFrame>&)>;

public:
	SignalGenerator();
	~SignalGenerator() = default;
	Ui::SignalGeneratorDialog *ui;
	
	bool									running;
	com_ptr<DeckLinkOutputDevice> 			selectedDevice;
	com_ptr<DeckLinkDeviceDiscovery>		deckLinkDiscovery;
	BMDDisplayMode							selectedDisplayMode;
	BMDPixelFormat							selectedPixelFormat;
	DeckLinkOpenGLWidget*					previewView;
	com_ptr<ProfileCallback>				profileCallback;
	
	uint32_t								frameWidth;
	uint32_t								frameHeight;
	BMDTimeValue							frameDuration;
	BMDTimeScale							frameTimescale;
	uint32_t								framesPerSecond;
	uint32_t								dropFrames;
	com_ptr<IDeckLinkMutableVideoFrame>		videoFrameBlack;
	com_ptr<IDeckLinkMutableVideoFrame>		videoFrameBars;
	uint32_t								totalFramesScheduled;
	//
	OutputSignal							outputSignal;
	void*									audioBuffer;
	uint32_t								audioBufferSampleLength;
	uint32_t								audioSamplesPerFrame;
	uint32_t								audioChannelCount;
	BMDAudioSampleRate						audioSampleRate;
	uint32_t								audioSampleDepth;
	uint32_t								totalAudioSecondsScheduled;
	//
	std::mutex								mutex;
	std::condition_variable					stopPlaybackCondition;
	
	BMDTimecodeFormat						timeCodeFormat;
	bool									hfrtcSupported;

	void customEvent(QEvent* event);
	void closeEvent(QCloseEvent *event);

	void setup();

	void scheduleNextFrame(bool prerolling);
	void writeNextAudioSamples();
	void enableInterface(bool);

	void startRunning();
	void stopRunning();
	
	void refreshDisplayModeMenu(void);
	void refreshPixelFormatMenu(void);
	void refreshAudioChannelMenu(void);
	void addDevice(com_ptr<IDeckLink>& deckLink);
	void removeDevice(com_ptr<IDeckLink>& deckLink);
	void playbackStopped(void);
	void haltStreams(void);
	void updateProfile(com_ptr<IDeckLinkProfile>& newProfile);
	
public slots:
	void outputDeviceChanged(int selectedDeviceIndex);
	void videoFormatChanged(int videoFormatIndex);
	void toggleStart();
	
private:
	QGridLayout *layout;
	std::unique_ptr<Timecode> timeCode;

	bool scheduledPlaybackStopped;
	std::map<intptr_t, com_ptr<DeckLinkOutputDevice>>		outputDevices;

	com_ptr<IDeckLinkMutableVideoFrame> CreateOutputFrame(FillFrameFunction fillFrame);
};

int		GetRowBytes(BMDPixelFormat pixelFormat, uint32_t frameWidth);
void	FillSine (void* audioBuffer, uint32_t samplesToWrite, uint32_t channels, uint32_t sampleDepth);
void	FillColorBars (com_ptr<IDeckLinkMutableVideoFrame>& theFrame);
void	FillBlack (com_ptr<IDeckLinkMutableVideoFrame>& theFrame);
void	ScheduleNextVideoFrame (void);
