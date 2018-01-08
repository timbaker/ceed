#ifndef CEED_PATHS_H
#define CEED_PATHS_H

#include "CEEDBase.h"

#include <QFileInfo>
#include <QDir>

struct os_t
{
    struct
    {
        QString abspath(const QString& s) { return QFileInfo(s).absoluteFilePath(); }
        QString basename(const QString&s) { return QFileInfo(s).baseName(); }
        QString curdir() { return QDir::currentPath(); }
        QString dirname(const QString& s) { return QFileInfo(s).path(); }
        bool exists(const QString& s) { return QFileInfo(s).exists(); }
        quint64 getsize(const QString&s) { return QFileInfo(s).size(); }
        bool isdir(const QString& s) { return QDir(s).exists(); }
        QString join(const QString& a, const QString& b) { return a + "/" + b; }
        QString normpath(const QString& s) { return QDir::cleanPath(s); } // TODO: canonical if exists
        QString relpath(const QString& a, const QString& b) { return QDir(b).relativeFilePath(a); }
    } path;

    QStringList listdir(const QString& s)
    {
        return QDir(s).entryList();
    }
    void mkdir(const QString&s)
    {
        if (!QDir(s).exists() && !QDir(s).mkdir("."))
            throw OSError("QDir::mkdir failed");
    }
    void rename(const QString& from, const QString& to)
    {
        if (!QFile::rename(from, to))
            throw OSError("QFile::rename failed");
    }
};

extern os_t os;

struct glob_t
{
    QStringList glob(const QString& path, const QString& pattern)
    {
        return QDir(path).entryList(QStringList() << pattern);
    }

};

extern glob_t glob;

#endif // CEED_PATHS_H
