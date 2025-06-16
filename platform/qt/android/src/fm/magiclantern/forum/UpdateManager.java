package fm.magiclantern.forum;

import android.app.Activity;
import android.content.IntentSender;

import com.google.android.play.core.appupdate.*;
import com.google.android.play.core.install.model.*;

public class UpdateManager {
    private AppUpdateManager appUpdateManager;
    private Activity activity;

    public void MyUpdateManager(Activity activity) {
        this.activity = activity;
        appUpdateManager = AppUpdateManagerFactory.create(activity);
    }

    public void checkForUpdate() {
        appUpdateManager.getAppUpdateInfo().addOnSuccessListener(appUpdateInfo -> {
            if (appUpdateInfo.updateAvailability() == UpdateAvailability.UPDATE_AVAILABLE
                && appUpdateInfo.isUpdateTypeAllowed(AppUpdateType.IMMEDIATE)) {
                try {
                    appUpdateManager.startUpdateFlowForResult(
                        appUpdateInfo,
                        AppUpdateType.IMMEDIATE,
                        activity,
                        1234);
                } catch (IntentSender.SendIntentException e) {
                    e.printStackTrace();
                }
            }
        });
    }
}
