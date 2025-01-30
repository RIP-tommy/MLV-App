#include <QFile>
#include <QTextStream>

#include <unistd.h>
#include <cstdio> // For FILE

// Custom function to open a file using QFile and return FILE* for compatibility
extern "C" FILE* openFileWithQFile(const char* filePath, const char* mode) {
    QFile* file = new QFile( filePath );

    // Map fopen mode to QFile mode
    QIODevice::OpenModeFlag openMode;
    if (mode[0] == 'r') {
        openMode = QIODevice::ReadOnly;
    } else if (mode[0] == 'w') {
        openMode = QIODevice::WriteOnly;
    } else if (mode[0] == 'a') {
        openMode = QIODevice::Append;
    } else {
        return nullptr; // Unsupported mode
    }

    // Open file with QFile
    if (!file->open(openMode)) {
        return nullptr; // Return null on failure
    }

    // Get native file descriptor
    int fd = file->handle();
    if (fd == -1) {
        delete file;
        return nullptr;
    }

    // Convert file descriptor to FILE*
    FILE* fp = fdopen(dup(fd), mode);
    if (!fp) {
        file->close();
        return nullptr;
    }

    // We can delete the QFile now as fdopen has its own handle
    delete file;

    return fp;
}
