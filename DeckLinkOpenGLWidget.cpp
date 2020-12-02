#include "DeckLinkOpenGLWidget.h"
#include <QOpenGLFunctions>
#include "ConnectToAgora.h"

///
/// DeckLinkOpenGLDelegate
///

DeckLinkOpenGLDelegate::DeckLinkOpenGLDelegate() : 
	m_refCount(1)
{
}

/// IUnknown methods

HRESULT DeckLinkOpenGLDelegate::QueryInterface(REFIID iid, LPVOID *ppv)
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
	else if (memcmp(&iid, &IID_IDeckLinkScreenPreviewCallback, sizeof(REFIID)) == 0)
	{
		*ppv = static_cast<IDeckLinkScreenPreviewCallback*>(this);
		AddRef();
		result = S_OK;
	}

	return result;
}

ULONG DeckLinkOpenGLDelegate::AddRef ()
{
	return ++m_refCount;
}

ULONG DeckLinkOpenGLDelegate::Release()
{
	ULONG newRefValue = --m_refCount;
	if (newRefValue == 0)
		delete this;

	return newRefValue;
}

/// IDeckLinkScreenPreviewCallback methods

HRESULT DeckLinkOpenGLDelegate::DrawFrame(IDeckLinkVideoFrame* frame)
{
	// Signal to QOpenGLWidget to update with new video frame
	emit frameArrived(com_ptr<IDeckLinkVideoFrame>(frame));

	return S_OK;
}

///
/// DeckLinkOpenGLWidget
///

DeckLinkOpenGLWidget::DeckLinkOpenGLWidget(QWidget* parent) : 
	QOpenGLWidget(parent)
{
	m_deckLinkScreenPreviewHelper = CreateOpenGLScreenPreviewHelper();
	m_delegate = make_com_ptr<DeckLinkOpenGLDelegate>();

	connect(m_delegate.get(), &DeckLinkOpenGLDelegate::frameArrived, this, &DeckLinkOpenGLWidget::setFrame, Qt::QueuedConnection);
}

void DeckLinkOpenGLWidget::clear()
{
	std::lock_guard<std::mutex> lock(m_mutex);
	if (m_delegate)
		m_delegate->DrawFrame(nullptr);
}

/// QOpenGLWidget methods

void DeckLinkOpenGLWidget::initializeGL()
{
	if (m_deckLinkScreenPreviewHelper)
	{
		m_deckLinkScreenPreviewHelper->InitializeGL();
	}
}

void DeckLinkOpenGLWidget::paintGL()
{
	if (m_deckLinkScreenPreviewHelper)
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		m_deckLinkScreenPreviewHelper->PaintGL();
	}
}

void DeckLinkOpenGLWidget::resizeGL(int width, int height)
{
	std::lock_guard<std::mutex> lock(m_mutex);
	QOpenGLFunctions* f = context()->functions();
	f->glViewport(0, 0, width, height);
}

/// DeckLinkOpenGLWidget slots 

void DeckLinkOpenGLWidget::setFrame(com_ptr<IDeckLinkVideoFrame> frame)
{
	if (m_deckLinkScreenPreviewHelper)
	{
        m_deckLinkScreenPreviewHelper->SetFrame(frame.get());
		update();
	}
}
