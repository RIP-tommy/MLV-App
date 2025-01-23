#ifndef ANDROID_H
#define ANDROID_H

#include "../../src/mlv_include.h"

#include <QFile>
#include <QDebug>
#include <QString>

void requestAllFilesAccess();
int save_dng_frame(mlvObject_t* mlv_data, dngObject_t* dng_data, uint32_t frame_index, const QString& dng_filename);

#endif // ANDROID_H
