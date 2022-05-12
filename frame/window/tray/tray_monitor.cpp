#include "tray_monitor.h"

TrayMonitor::TrayMonitor(QObject *parent)
    : QObject(parent)
    , m_trayInter(new DBusTrayManager(this))
    , m_sniWatcher(new StatusNotifierWatcher("org.kde.StatusNotifierWatcher", "/StatusNotifierWatcher", QDBusConnection::sessionBus(), this))
{
    //-------------------------------Tray Embed---------------------------------------------//
    connect(m_trayInter, &DBusTrayManager::TrayIconsChanged, this, &TrayMonitor::onTrayIconsChanged, Qt::QueuedConnection);
    connect(m_trayInter, &DBusTrayManager::Changed, this, &TrayMonitor::requestUpdateIcon, Qt::QueuedConnection);
    m_trayInter->Manage();
    QMetaObject::invokeMethod(this, "onTrayIconsChanged", Qt::QueuedConnection);

    //-------------------------------Tray SNI---------------------------------------------//
    connect(m_sniWatcher, &StatusNotifierWatcher::StatusNotifierItemRegistered, this, &TrayMonitor::onSniItemsChanged, Qt::QueuedConnection);
    connect(m_sniWatcher, &StatusNotifierWatcher::StatusNotifierItemUnregistered, this, &TrayMonitor::onSniItemsChanged, Qt::QueuedConnection);
    QMetaObject::invokeMethod(this, "onSniItemsChanged", Qt::QueuedConnection);

    //-------------------------------Tray Indicator---------------------------------------------//
    QMetaObject::invokeMethod(this, "startLoadIndicators", Qt::QueuedConnection);
}

void TrayMonitor::onTrayIconsChanged()
{
    QList<quint32> wids = m_trayInter->trayIcons();
    if (m_trayWids == wids)
        return;

    for (auto wid : wids) {
        if (!m_trayWids.contains(wid)) {
            Q_EMIT xEmbedTrayAdded(wid);
        }
    }

    for (auto wid : m_trayWids) {
        if (!wids.contains(wid)) {
            Q_EMIT xEmbedTrayRemoved(wid);
        }
    }

    m_trayWids = wids;
}

void TrayMonitor::onSniItemsChanged()
{
    //TODO 防止同一个进程注册多个sni服务
    const QStringList &sniServices = m_sniWatcher->registeredStatusNotifierItems();
    if (m_sniServices == sniServices)
        return;

    for (auto s : sniServices) {
        if (!m_sniServices.contains(s)) {
            if (s.startsWith("/") || !s.contains("/")) {
                qWarning() << __FUNCTION__ << "invalid sni service" << s;
                continue;
            }
            Q_EMIT sniTrayAdded(s);
        }
    }

    for (auto s : m_sniServices) {
        if (!sniServices.contains(s)) {
            Q_EMIT sniTrayRemoved(s);
        }
    }

    m_sniServices = sniServices;
}

void TrayMonitor::startLoadIndicators()
{
    QDir indicatorConfDir("/etc/dde-dock/indicator");

    for (const QFileInfo &fileInfo : indicatorConfDir.entryInfoList({"*.json"}, QDir::Files | QDir::NoDotAndDotDot)) {
        const QString &indicatorName = fileInfo.baseName();
        Q_EMIT indicatorFounded(indicatorName);
    }
}
