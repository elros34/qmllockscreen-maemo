#ifndef SKYMODEL_H
#define SKYMODEL_H

#include <QAbstractListModel>
#include <QList>
#include "appitem.h"
#include <QFileInfo>
#include <QFileInfoList>
#include <QDir>
#include <QDebug>

class FileModel : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit FileModel(QObject *parent = 0);

    ~FileModel();

    enum ListRoles {
        PathRole = Qt::UserRole + 1,
        NameRole,
        IsDirRole,
        ServiceRole
    };

    QVariant data(const QModelIndex &index, int role) const;
    int rowCount(const QModelIndex &parent) const;
    void append(AppItem *item);

    void clear();
    void sortElements();


signals:

public slots:
    void setDir(QString directory);  
    void setFilters(const QStringList &filters);
    QStringList getDesktopFileInfo(const QString &path);

private:
    QList<AppItem*> appList;
    QStringList _filters;
    QString findIcon(QString appIcon);
    static bool compareAppItem(const AppItem *a, const AppItem *b)
    {
        return a->name().toLower() < b->name().toLower();
    }
};

#endif // SKYMODEL_H
