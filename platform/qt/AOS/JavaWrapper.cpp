#include <QJniObject>
#include <QJniEnvironment>
#include <QCoreApplication>

// For keeping screen on
QJniObject wakeLock;

QString createFolderInAndroidUri(const QString &parentUri, const QString &folderName) {
    // Get Android Activity (which acts as a Context)
    QJniObject activity = QNativeInterface::QAndroidApplication::context();

    if (!activity.isValid()) {
        return QString();
    }

    // Call Java method to create folder in content:// URI
    QJniObject result = QJniObject::callStaticObjectMethod(
        "fm/magiclantern/forum/MyJavaHelper",
        "createFolderInUri",
        "(Landroid/content/Context;Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;",
        activity.object(),
        QJniObject::fromString(parentUri).object(),
        QJniObject::fromString(folderName).object()
        );

    return result.isValid() ? result.toString() : QString();
}

qint8 runExport(const QString &inputFile, const QString &outputFile) {
    QJniObject activity = QNativeInterface::QAndroidApplication::context();

    if (activity.isValid()) {
        jint result =  QJniObject::callStaticMethod<jint>(
            "fm/magiclantern/forum/MyJavaHelper",
            "runFfmpeg",
            "(Landroid/content/Context;Ljava/lang/String;Ljava/lang/String;)I",
            activity.object(),
            QJniObject::fromString(inputFile).object(),
            QJniObject::fromString(outputFile).object());
        return static_cast<qint8>(result);
    }
    return 0;
}

bool checkFFmpeg() {
    QJniObject activity = QNativeInterface::QAndroidApplication::context();

    if (activity.isValid()) {
        jboolean result =  QJniObject::callStaticMethod<jboolean>(
            "fm/magiclantern/forum/MyJavaHelper",
            "checkFFmpegReady",
            "(Landroid/content/Context;)Z",
            activity.object());
        return static_cast<bool>(result);
    }
    return false;
}

bool runFFmpegCmd(QString cmd, QString outputFile) {
    QJniObject activity = QNativeInterface::QAndroidApplication::context();

    jboolean result = QJniObject::callStaticMethod<jboolean>(
        "fm/magiclantern/forum/MyJavaHelper",
        "runFFmpegCmd",
        "(Landroid/content/Context;Ljava/lang/String;Ljava/lang/String;)Z",
        activity.object(),
        QJniObject::fromString(cmd).object(),
        QJniObject::fromString(outputFile).object());
    return static_cast<bool>(result);
}

QString getFFMpegPipe() {
    QJniObject activity = QNativeInterface::QAndroidApplication::context();

    return QJniObject::callStaticMethod<QString>(
        "fm/magiclantern/forum/MyJavaHelper",
        "getFFmpegPipe",
        "(Landroid/app/Activity;)Ljava/lang/String;",
        activity.object());
}

bool runFFmpegCmdInPipe(QString tmpImgPath, QString cmd, QString pipe) {
    QJniObject activity = QNativeInterface::QAndroidApplication::context();

    jboolean result = QJniObject::callStaticMethod<jboolean>(
        "fm/magiclantern/forum/MyJavaHelper",
        "runFFmpegCmdInPipe",
        "(Landroid/content/Context;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)Z",
        activity.object(),
        QJniObject::fromString(tmpImgPath).object(),
        QJniObject::fromString(cmd).object(),
        QJniObject::fromString(pipe).object());
    return static_cast<bool>(result);
}

void closeFFmpegPipe(QString pipe) {
    QJniObject::callStaticMethod<void>(
        "fm/magiclantern/forum/MyJavaHelper",
        "closeFFmpegPipe",
        "(Ljava/lang/String;)V",
        QJniObject::fromString(pipe).object());
}

void startExportService() {
    QJniObject activity = QNativeInterface::QAndroidApplication::context();
    if (activity.isValid()) {
        QJniObject::callStaticMethod<void>(
            "fm/magiclantern/forum/MyJavaHelper",
            "startExportService",
            "(Landroid/content/Context;)V",
            activity.object());
    }
}

void stopExportService() {
    QJniObject activity = QNativeInterface::QAndroidApplication::context();
    if (activity.isValid()) {
        QJniObject::callStaticMethod<void>(
            "fm/magiclantern/forum/MyJavaHelper",
            "stopExportService",
            "(Landroid/content/Context;)V",
            activity.object());
    }
}

// I tried to use `WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON`
// but I got the error below
// android.view.ViewRootImpl$CalledFromWrongThreadException:
// Only the original thread that created a view hierarchy can touch its views.
// Expected: main Calling: qtMainLoopThread
void triggerBrightWakeLock() {
    QJniObject activity = QNativeInterface::QAndroidApplication::context();
    QJniObject powerManager = activity.callObjectMethod(
        "getSystemService",
        "(Ljava/lang/String;)Ljava/lang/Object;",
        QJniObject::fromString("power").object());

    QJniObject packageName = activity.callObjectMethod<jstring>("getPackageName");
    QString wakeLockTag = QString("org.qtproject.example::WakeLock");

    wakeLock = powerManager.callObjectMethod(
        "newWakeLock",
        "(ILjava/lang/String;)Landroid/os/PowerManager$WakeLock;",
        0x0000000a | 0x20000000, // PowerManager.SCREEN_BRIGHT_WAKE_LOCK | PowerManager.ON_AFTER_RELEASE
        QJniObject::fromString(wakeLockTag).object());

    if (wakeLock.isValid()) {
        wakeLock.callMethod<void>("acquire", "()V");
        qDebug() << "Wake lock acquired with tag:" << wakeLockTag;
    }
}

void triggerDimWakeLock() {
    QJniObject activity = QNativeInterface::QAndroidApplication::context();
    QJniObject powerManager = activity.callObjectMethod(
        "getSystemService",
        "(Ljava/lang/String;)Ljava/lang/Object;",
        QJniObject::fromString("power").object());

    QJniObject packageName = activity.callObjectMethod<jstring>("getPackageName");
    QString wakeLockTag = QString("org.qtproject.example::WakeLock");

    wakeLock = powerManager.callObjectMethod(
        "newWakeLock",
        "(ILjava/lang/String;)Landroid/os/PowerManager$WakeLock;",
        0x00000006 | 0x20000000, // PowerManager.SCREEN_DIM_WAKE_LOCK | PowerManager.ON_AFTER_RELEASE
        QJniObject::fromString(wakeLockTag).object());

    if (wakeLock.isValid()) {
        wakeLock.callMethod<void>("acquire", "()V");
        qDebug() << "Wake lock acquired with tag:" << wakeLockTag;
    }
}

void releaseWakeLock() {
    if (wakeLock.isValid()) {
        wakeLock.callMethod<void>("release", "()V");
        wakeLock = QJniObject();  // Clear the wake lock
    }
}

void checkAppUpdate() {
    QJniObject activity = QNativeInterface::QAndroidApplication::context();

    if (!activity.isValid()) {
        qWarning("Failed to retrieve Android activity context");
        return;
    }

    QJniObject updateManager = QJniObject("fm/magiclantern/forum/UpdateManager",
                                          "(Landroid/app/Activity;)V",
                                          activity.object<jobject>());

    if (updateManager.isValid()) {
        updateManager.callMethod<void>("checkForUpdate");
    } else {
        qWarning("Failed to create update manager");
    }
}
