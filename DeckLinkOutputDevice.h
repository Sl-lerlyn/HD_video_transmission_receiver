#pragma once

#include <atomic>
#include <functional>

#include <QObject>
#include <QString>

#include "com_ptr.h"
#include "DeckLinkAPI.h"

class DeckLinkOutputDevice : public IDeckLinkVideoOutputCallback, public IDeckLinkAudioOutputCallback
{
	using ScheduledFrameCompletedFunc	= std::function<void(void)>;
	using RenderAudioSamplesFunc		= std::function<void(void)>;
	using ScheduledPlaybackStoppedFunc	= std::function<void(void)>; 
	using DisplayModeQueryFunc			= std::function<void(com_ptr<IDeckLinkDisplayMode>&)>;

public:
	DeckLinkOutputDevice(QObject* owner, com_ptr<IDeckLink>& deckLink);
	virtual ~DeckLinkOutputDevice() = default;

	// IUnknown
	HRESULT		QueryInterface(REFIID iid, LPVOID *ppv) override;
	ULONG		AddRef() override;
	ULONG		Release() override;

	// IDeckLinkVideoOutputCallback
	HRESULT		ScheduledFrameCompleted(IDeckLinkVideoFrame* completedFrame, BMDOutputFrameCompletionResult result) override;
	HRESULT		ScheduledPlaybackHasStopped() override;

	// IDeckLinkAudioOutputCallback
	HRESULT		RenderAudioSamples(bool preroll) override;

	// Other methods
	const QString						getDeviceName() const { return m_deviceName; }
	com_ptr<IDeckLinkOutput>			getDeviceOutput() const { return m_deckLinkOutput; }
	com_ptr<IDeckLink>					getDeckLinkInstance() const { return m_deckLink; }
	com_ptr<IDeckLinkConfiguration>		getDeviceConfiguration() const { return m_deckLinkConfiguration; }
	com_ptr<IDeckLinkProfileManager>	getProfileManager() const { return m_deckLinkProfileManager; }

	void	queryDisplayModes(DisplayModeQueryFunc func);
	void	onScheduledFrameCompleted(const ScheduledFrameCompletedFunc& callback) { m_scheduledFrameCompletedCallback = callback; }
	void	onRenderAudioSamples(const RenderAudioSamplesFunc& callback) { m_renderAudioSamplesCallback = callback; }
	void	onScheduledPlaybackStopped(const ScheduledPlaybackStoppedFunc& callback) { m_scheduledPlaybackStoppedCallback = callback; }

private:
	std::atomic<ULONG>					m_refCount;
	QObject*							m_owner;

	com_ptr<IDeckLink>					m_deckLink;
	com_ptr<IDeckLinkOutput>			m_deckLinkOutput;
	com_ptr<IDeckLinkConfiguration>		m_deckLinkConfiguration;
	com_ptr<IDeckLinkProfileManager>	m_deckLinkProfileManager;
	QString								m_deviceName;

	ScheduledFrameCompletedFunc			m_scheduledFrameCompletedCallback;
	ScheduledPlaybackStoppedFunc		m_scheduledPlaybackStoppedCallback;
	RenderAudioSamplesFunc				m_renderAudioSamplesCallback;
};

