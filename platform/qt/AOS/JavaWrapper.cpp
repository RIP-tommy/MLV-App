#include <QJniObject>
#include <QJniEnvironment>

QString createFolderInAndroidUri(const QString &parentUri, const QString &folderName) {
    // Get Android Activity (which acts as a Context)
    QJniObject activity = QJniObject::callStaticObjectMethod(
        "org/qtproject/qt/android/QtNative", "activity", "()Landroid/app/Activity;");

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

qint64 getFreeSpaceAndroid(const QString &contentUri) {
    QJniObject activity = QJniObject::callStaticObjectMethod(
        "org/qtproject/qt/android/QtNative", "activity", "()Landroid/app/Activity;");

    if (!activity.isValid()) {
        return -1;
    }

    jlong freeSpace = QJniObject::callStaticMethod<jlong>(
        "fm/magiclantern/forum/MyJavaHelper",
        "getFreeSpace",
        "(Landroid/content/Context;Ljava/lang/String;)J",
        activity.object(),
        QJniObject::fromString(contentUri).object());

    return static_cast<qint64>(freeSpace);
}
