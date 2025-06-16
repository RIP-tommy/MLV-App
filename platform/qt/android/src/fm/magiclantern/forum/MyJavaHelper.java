package fm.magiclantern.forum;

import android.app.Activity;
import android.content.Context;
import android.content.ContentResolver;
import android.content.ContentValues;
import android.content.Intent;
import android.net.Uri;
import android.provider.DocumentsContract;
import android.database.Cursor;
import android.util.Log;
import android.os.StatFs;
import android.os.Build;
import android.os.storage.StorageManager;
import android.os.storage.StorageVolume;

import com.arthenica.ffmpegkit.FFmpegKit;
import com.arthenica.ffmpegkit.FFmpegKitConfig;
import com.arthenica.ffmpegkit.FFmpegSession;
import com.arthenica.ffmpegkit.SessionState;
import com.arthenica.ffmpegkit.ReturnCode;
import com.arthenica.ffmpegkit.FFmpegSessionCompleteCallback;
import com.arthenica.smartexception.java.Exceptions;

import java.io.FileOutputStream;
import java.io.IOException;
import java.util.HashMap;
import java.util.Map;

public class MyJavaHelper {
    private static final String TAG = "FFmpegKit";

    public static String createFolderInUri(Context context, String parentUri, String folderName) {
        if (context == null) {
            return null;
        }

       try {
            ContentResolver resolver = context.getContentResolver();
            Uri uri = Uri.parse(parentUri);
            Uri parentDocumentUri = DocumentsContract.buildDocumentUriUsingTree(uri,
                   DocumentsContract.getTreeDocumentId(uri));

            ContentValues values = new ContentValues();
            values.put(DocumentsContract.Document.COLUMN_DISPLAY_NAME, folderName);
            values.put(DocumentsContract.Document.COLUMN_MIME_TYPE, DocumentsContract.Document.MIME_TYPE_DIR);

            Uri newFolderUri = DocumentsContract.createDocument(resolver, parentDocumentUri, DocumentsContract.Document.MIME_TYPE_DIR, folderName);

            return newFolderUri != null ? newFolderUri.toString() : null;
        } catch (Exception e) {
            Log.e("MyJavaHelper", "Failed to create folder", e);
            return null;
        }
    }

    public static long getFreeSpace(Context context, String uriString) {
        try {
            Uri uri = Uri.parse(uriString);

            // Convert tree URI to document URI
            String documentId = DocumentsContract.getTreeDocumentId(uri);
            String[] parts = documentId.split(":");
            String storageType = parts[0];
            String basePath = null;

            if (storageType.matches("\\w{4}-\\w{4}")) {
                StorageManager storageManager = (StorageManager) context.getSystemService(Context.STORAGE_SERVICE);
                for (StorageVolume volume : storageManager.getStorageVolumes()) {
                    if (volume.getUuid() != null && volume.getUuid().equals(storageType)) {
                        basePath = volume.getDirectory().getPath();
                        break;
                    }
                }
            }

            if (basePath != null) {
                StatFs statFs = new StatFs(basePath);
                return statFs.getAvailableBytes();
            }
        } catch (Exception e) {
            Log.e("myjava", "err", e);
            return -1;
        }
        return -1;
    }

    public static boolean checkFFmpegReady(Context context) {
        return FFmpegKitConfig.isLTSBuild();
    }

    public static String getSAF(Context context, String filePath, int mode) {
        Uri pathUri = Uri.parse(filePath);
        Log.d(TAG, pathUri.toString());
        if (mode == 1) return FFmpegKitConfig.getSafParameterForRead(context, pathUri);
        if (mode == 2) return FFmpegKitConfig.getSafParameterForWrite(context, pathUri);
        return null;
    }

    public static boolean runFFmpegCmd(Context context, String cmd, String outputFile) {
        Uri outputUri = Uri.parse(outputFile);

        String outputVideoPath = FFmpegKitConfig.getSafParameterForWrite(context, outputUri);
        Log.d(TAG, String.format("output file path %s", outputVideoPath));
        String command = String.format("-protocol_whitelist saf,file,crypto %s %s", cmd, outputVideoPath);
        Log.d(TAG, command);
        FFmpegSession session = FFmpegKit.execute(command);

        // State of the execution. Shows whether it is still running or completed
        SessionState state = session.getState();

        // Return code for completed sessions. Will be null if session is still running or ends with a failure
        ReturnCode returnCode = session.getReturnCode();

        // Console output generated for this execution
        String output = session.getOutput();

        // The stack trace if FFmpegKit fails to run a command
        String failStackTrace = session.getFailStackTrace();

        if (ReturnCode.isSuccess(returnCode)) {
            // SUCCESS
            Log.d(TAG, String.format("Command succeeded with state %s and rc %s.%s", state, returnCode, failStackTrace));
            return true;
        } else {
            // FAILURE
            Log.d(TAG, String.format("Command failed with state %s and rc %s.%s", state, returnCode, failStackTrace));
            return false;
        }
    }

    public static String getFFmpegPipe(Activity mainActivity) {
        return FFmpegKitConfig.registerNewFFmpegPipe(mainActivity);
    }

    public static boolean runFFmpegCmdInPipe(Context context, String imgPath, String cmd, String pipe) {
        try {
            String pipe1 = FFmpegKitConfig.registerNewFFmpegPipe(context);
            String updatedCmd = cmd.replace("-i -", "-i " + pipe1);
            // FFmpegSession session = FFmpegKit.execute(cmd);
            FFmpegKit.executeAsync("-protocol_whitelist saf,file,crypto" + updatedCmd, new FFmpegSessionCompleteCallback() {
                @Override
                public void apply(final FFmpegSession session) {
                    final SessionState state = session.getState();
                    final ReturnCode returnCode = session.getReturnCode();

                    Log.d(TAG, String.format("FFmpeg process exited with state %s and rc %s.%s", state, returnCode, session.getFailStackTrace()));

                    // CLOSE PIPES
                    FFmpegKitConfig.closeFFmpegPipe(pipe1);
                }
            });

            String asyncCommand = "cat " + imgPath + " > " + pipe1;
            final Process process = Runtime.getRuntime().exec(new String[]{"sh", "-c", asyncCommand});
            int rc = process.waitFor();

            Log.d(TAG, String.format("Async cat image command: %s exited with %d.", asyncCommand, rc));

            return true;
        } catch (final IOException | InterruptedException e) {
            Log.e(TAG, String.format("Async cat image command failed for %s.%s", cmd, Exceptions.getStackTraceString(e)));
            return false;
        }
    }

    public static void closeFFmpegPipe(String pipe) {
        FFmpegKitConfig.closeFFmpegPipe(pipe);
    }
}
