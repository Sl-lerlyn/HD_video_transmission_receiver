#pragma once

#include <QEvent>

// Define custom event type
static const QEvent::Type kAddDeviceEvent				= static_cast<QEvent::Type>(QEvent::User + 1);
static const QEvent::Type kRemoveDeviceEvent			= static_cast<QEvent::Type>(QEvent::User + 2);
static const QEvent::Type kProfileActivatedEvent		= static_cast<QEvent::Type>(QEvent::User + 3);
