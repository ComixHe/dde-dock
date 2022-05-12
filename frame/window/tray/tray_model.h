#ifndef TRAYMODEL_H
#define TRAYMODEL_H

#include <QAbstractListModel>
#include <QObject>
#include <QListView>

class TrayMonitor;
class IndicatorPlugin;

enum TrayIconType {
    UNKNOW,
    XEMBED,
    SNI,
    INDICATOR,
    EXPANDICON
};

struct WinInfo {
    TrayIconType type;
    QString key;
    quint32 winId;
    QString servicePath;
    bool isTypeWriting;

    WinInfo() {
        type = UNKNOW;
        key = QString();
        winId = 0;
        servicePath = QString();
        isTypeWriting = false;
    }
    bool operator==(const WinInfo &other) {
        return this->type == other.type
                && this->key == other.key
                && this->winId == other.winId
                && this->servicePath == other.servicePath
                && this->isTypeWriting == other.isTypeWriting;
    }
};

class TrayModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum Role {
        TypeRole = Qt::UserRole + 1,
        KeyRole,
        WinIdRole,
        ServiceRole,
        Blank
    };

    typedef QList<WinInfo> WinInfos;

    TrayModel(QListView *view, bool isIconTray, bool hasInputMethod, QObject *parent = Q_NULLPTR);

    void dropSwap(int newPos);
    void dropInsert(int newPos);

    void clearDragDropIndex();
    void setDragingIndex(const QModelIndex index);
    void setDragDropIndex(const QModelIndex index);

    void setDragKey(const QString &key);

    bool indexDragging(const QModelIndex &index) const;

    int rowCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    bool isIconTray();

    void clear();

public Q_SLOTS:
    void removeRow(const QString &itemKey);
    void addRow(WinInfo info);

Q_SIGNALS:
    void requestUpdateIcon(quint32);
    void requestUpdateWidget(const QList<int> &);

private Q_SLOTS:
    void onXEmbedTrayAdded(quint32 winId);
    void onXEmbedTrayRemoved(quint32 winId);
    void onSniTrayAdded(const QString &servicePath);
    void onSniTrayRemoved(const QString &servicePath);

    void onIndicatorFounded(const QString &indicatorName);
    void onIndicatorAdded(const QString &indicatorName);
    void onIndicatorRemoved(const QString &indicatorName);

private:
    bool exist(const QString &itemKey);
    QString fileNameByServiceName(const QString &serviceName);
    bool isTypeWriting(const QString &servicePath);

protected:
    QMimeData *mimeData(const QModelIndexList &indexes) const Q_DECL_OVERRIDE;
    QVariant data(const QModelIndex &index, int role) const Q_DECL_OVERRIDE;
    bool removeRows(int row, int count, const QModelIndex &parent) Q_DECL_OVERRIDE;
    bool canDropMimeData(const QMimeData *data, Qt::DropAction action,
                         int row, int column, const QModelIndex &parent) const Q_DECL_OVERRIDE;
    Qt::ItemFlags flags(const QModelIndex &index) const Q_DECL_OVERRIDE;

private:
    WinInfos m_winInfos;

    QModelIndex m_dragModelIndex;
    QModelIndex m_dropModelIndex;
    WinInfo m_dragInfo;
    QListView *m_view;
    TrayMonitor *m_monitor;

    QString m_dragKey;

    QMap<QString, IndicatorPlugin *> m_indicatorMap;
    bool m_isTrayIcon;
    bool m_hasInputMethod;
};

Q_DECLARE_METATYPE(TrayIconType);

#endif // TRAYMODEL_H
