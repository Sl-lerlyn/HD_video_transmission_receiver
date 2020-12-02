#include <QCoreApplication>
#include "ProfileCallback.h"

ProfileCallback::ProfileCallback(QObject* owner) : 
	m_owner(owner), 
	m_refCount(1)
{
}

/// IUnknown methods

HRESULT ProfileCallback::QueryInterface(REFIID iid, LPVOID *ppv)
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
		*ppv = static_cast<IUnknown*>(this);
		AddRef();
		result = S_OK;
	}
	else if (memcmp(&iid, &IID_IDeckLinkProfileCallback, sizeof(REFIID)) == 0)
	{
		*ppv = static_cast<IDeckLinkProfileCallback*>(this);
		AddRef();
		result = S_OK;
	}
	
	return result;
}

ULONG ProfileCallback::AddRef(void)
{
	return ++m_refCount;
}

ULONG ProfileCallback::Release(void)
{
	ULONG newRefValue = --m_refCount;
	if (newRefValue == 0)
		delete this;

	return newRefValue;
}

/// IDeckLinkProfileCallback methods

HRESULT ProfileCallback::ProfileChanging(IDeckLinkProfile* /* profileToBeActivated */, bool streamsWillBeForcedToStop)
{
	// When streamsWillBeForcedToStop is true, the profile to be activated is incompatible with the 
	// current profile and playback will be stopped by the driver. It is better to notify the
	// controller to gracefully stop capture, so that the output is set to a known state.
	// The profile change completes on return from ProfileChanging, so the callback is called
	// directly, rather than by posting an event.
	if ((streamsWillBeForcedToStop) && (m_profileChangingCallback != nullptr))
		m_profileChangingCallback();

	return S_OK;
}

HRESULT ProfileCallback::ProfileActivated(IDeckLinkProfile *activatedProfile)
{
	// New profile activated, inform owner to update popup menus
	QCoreApplication::postEvent(m_owner, new ProfileActivatedEvent(activatedProfile));

	return S_OK;
}
