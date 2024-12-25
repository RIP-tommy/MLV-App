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
