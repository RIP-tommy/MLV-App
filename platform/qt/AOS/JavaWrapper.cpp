#include <QJniObject>
#include <QJniEnvironment>
#include <QCoreApplication>

void requestAllFilesAccess() {
    // Check If all files permission is already granted
    if (QJniObject::callStaticMethod<jboolean>(
            "android/os/Environment",
            "isExternalStorageManager",
            "()Z"
            ))
        return;
    // Get the current Android activity context
    QJniObject activity = QNativeInterface::QAndroidApplication::context();

    // Get the ACTION_MANAGE_APP_ALL_FILES_ACCESS_PERMISSION string as a jstring
    QJniObject actionManageAppAllFilesAccessPermission = QJniObject::getStaticObjectField<jstring>(
        "android/provider/Settings", "ACTION_MANAGE_APP_ALL_FILES_ACCESS_PERMISSION");


    // Create an intent to open the "All Files Access" settings screen
    QJniObject intent("android/content/Intent", "(Ljava/lang/String;)V",
                      actionManageAppAllFilesAccessPermission.object<jstring>());


    // Get the app's package name
    QJniObject packageName = activity.callObjectMethod<jstring>("getPackageName");

    // Create a URI with the package name
    QJniObject uri = QJniObject::callStaticObjectMethod(
        "android/net/Uri", "fromParts", "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)Landroid/net/Uri;",
        QJniObject::fromString("package").object<jstring>(),
        packageName.object<jstring>(),
        nullptr);

    // Set the intent's data with the URI
    intent.callObjectMethod("setData", "(Landroid/net/Uri;)Landroid/content/Intent;", uri.object());

    // Start the activity to request all files access
    activity.callMethod<void>("startActivity", "(Landroid/content/Intent;)V", intent.object());
}

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
