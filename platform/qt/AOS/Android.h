#ifndef ANDROID_H
#define ANDROID_H

#include "../../src/mlv_include.h"

#include <QFile>
#include <QDebug>
#include <QString>

void requestAllFilesAccess();
QString createFolderInAndroidUri(const QString &parentUri, const QString &folderName);
qint64 getFreeSpaceAndroid(const QString &contentUri);
void keepScreenOn(bool on);

#endif // ANDROID_H
