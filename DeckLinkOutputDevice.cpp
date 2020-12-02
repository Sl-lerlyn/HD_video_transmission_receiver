#include "DeckLinkOutputDevice.h"

DeckLinkOutputDevice::DeckLinkOutputDevice(QObject* owner, com_ptr<IDeckLink>& deckLink) : 
	m_refCount(1),
	m_owner(owner),
	m_deckLink(deckLink),
	m_deckLinkOutput(IID_IDeckLinkOutput, deckLink),
	m_deckLinkConfiguration(IID_IDeckLinkConfiguration, deckLink),
	m_deckLinkProfileManager(IID_IDeckLinkProfileManager, deckLink)  // nullptr if deckLink has only 1 profile
{
	const char* deviceNameCStr = nullptr;

	// Get device name
	if (m_deckLink->GetDisplayName(&deviceNameCStr) == S_OK)
	{
        m_deviceName = QString(deviceNameCStr);
		free((void*)deviceNameCStr);
	}
	else
	{
		m_deviceName = QString("DeckLink");
	}
}

/// IUnknown methods

HRESULT	DeckLinkOutputDevice::QueryInterface(REFIID iid, LPVOID *ppv)
{
	static const REFIID		iunknown	= IID_IUnknown;
	HRESULT					result		= E_NOINTERFACE;

	if (ppv == nullptr)
		return E_INVALIDARG;

	// Initialise the return result
	*ppv = nullptr;

	// Compare provided REFIID to IUnknown
	if (memcmp(&iid, &iunknown, sizeof(REFIID)) == 0)
	{
		*ppv = static_cast<IUnknown*>(static_cast<IDeckLinkVideoOutputCallback*>(this));
		AddRef();
		result = S_OK;
	}
	else if (memcmp(&iid, &IID_IDeckLinkVideoOutputCallback, sizeof(REFIID)) == 0)
	{
		*ppv = static_cast<IDeckLinkVideoOutputCallback*>(this);
		AddRef();
		result = S_OK;
	}
	else if (memcmp(&iid, &IID_IDeckLinkAudioOutputCallback, sizeof(REFIID)) == 0)
	{
		*ppv = static_cast<IDeckLinkAudioOutputCallback*>(this);
		AddRef();
		result = S_OK;
	}

	return result;
}

ULONG DeckLinkOutputDevice::AddRef(void)
{
	return ++m_refCount;
}

ULONG DeckLinkOutputDevice::Release(void)
{
	ULONG newRefCount = --m_refCount;
	if (newRefCount == 0)
		delete this;

	return newRefCount;
}

/// IDeckLinkVideoOutputCallback methods

HRESULT	DeckLinkOutputDevice::ScheduledFrameCompleted(IDeckLinkVideoFrame* /* completedFrame */, BMDOutputFrameCompletionResult /* result */)
{
	if (m_scheduledFrameCompletedCallback)
		m_scheduledFrameCompletedCallback();
	
	return S_OK;
}

HRESULT	DeckLinkOutputDevice::ScheduledPlaybackHasStopped()
{
	// Notify that playback has stopped, so it can disable output
	if (m_scheduledPlaybackStoppedCallback)
		m_scheduledPlaybackStoppedCallback();

	return S_OK;
}

/// IDeckLinkAudioOutputCallback methods

HRESULT	DeckLinkOutputDevice::RenderAudioSamples(bool preroll)
{
	if (m_renderAudioSamplesCallback)
		m_renderAudioSamplesCallback();

	if (preroll)
	{
		// Start audio and video output
		m_deckLinkOutput->StartScheduledPlayback(0, 100, 1.0);
	}

	return S_OK;
}

/// Other methods

void DeckLinkOutputDevice::queryDisplayModes(DisplayModeQueryFunc func)
{
	com_ptr<IDeckLinkDisplayModeIterator>	displayModeIterator;
	com_ptr<IDeckLinkDisplayMode>			displayMode;

	if (m_deckLinkOutput->GetDisplayModeIterator(displayModeIterator.releaseAndGetAddressOf()) != S_OK)
		return;

	while (displayModeIterator->Next(displayMode.releaseAndGetAddressOf()) == S_OK)
	{
		bool supported;
		if ((m_deckLinkOutput->DoesSupportVideoMode(bmdVideoConnectionUnspecified, displayMode->GetDisplayMode(),
													bmdFormatUnspecified, bmdNoVideoOutputConversion, 
													bmdSupportedVideoModeDefault, nullptr, &supported) == S_OK) && supported)
		{
			func(displayMode);
		}
	}
}
