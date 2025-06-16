#ifndef ANDROID_H
#define ANDROID_H

#include "../../src/mlv_include.h"

#include <QFile>
#include <QDebug>
#include <QString>

QString createFolderInAndroidUri(const QString &parentUri, const QString &folderName);
qint8 runExport(const QString &inputFile, const QString &outputFile);
bool checkFFmpeg();
bool runFFmpegCmd(QString cmd, QString outputFile);
QString getFFMpegPipe();
bool runFFmpegCmdInPipe(QString tmpImgPath, QString cmd, QString pipe);
void closeFFmpegPipe(QString pipe);
void requestAllFilesAccess();
void triggerBrightWakeLock();
void releaseWakeLock();
void triggerDimWakeLock();

#endif // ANDROID_H
