#include <AOS/Android.h>

int save_dng_frame(mlvObject_t* mlv_data, dngObject_t* dng_data, uint32_t frame_index, const QString& dng_filename) {
    QFile dngFile(dng_filename);

    if (!dngFile.open(QIODevice::WriteOnly)) return 1;

    // Get filled dng_data struct
    if (dng_get_frame(mlv_data, dng_data, frame_index) != 0) {
        dngFile.close();
        return 1;
    }

    // Write DNG header
    if (dngFile.write((const char*)dng_data->header_buf, dng_data->header_size) != dng_data->header_size) {
        dngFile.close();
        return 1;
    }

    // Write DNG image data
    if (dngFile.write((const char*)dng_data->image_buf, dng_data->image_size) != dng_data->image_size) {
        dngFile.close();
        return 1;
    }

    dngFile.close();

// Logging
#ifndef STDOUT_SILENT
    if (frame_index == 0) {
        switch (dng_data->raw_output_state) {
        case UNCOMPRESSED_RAW:
            qDebug() << "\nWriting uncompressed frames...";
            break;
        case COMPRESSED_RAW:
            qDebug() << "\nWriting lossless frames...";
            break;
        case UNCOMPRESSED_ORIG:
            qDebug() << "\nPassing through original uncompressed raw...";
            break;
        case COMPRESSED_ORIG:
            qDebug() << "\nPassing through original lossless raw...";
            break;
        }
    }
    qDebug() << "Current frame" << dng_filename << "(frames saved:" << frame_index + 1 << ")";
#endif

    return 0;
}
