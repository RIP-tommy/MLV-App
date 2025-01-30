package fm.magiclantern.forum;  // Use the same package as in AndroidManifest.xml

import android.content.Context;
import android.content.ContentResolver;
import android.content.ContentValues;
import android.net.Uri;
import android.provider.DocumentsContract;
import android.database.Cursor;
import android.os.StatFs;
import android.util.Log;
import android.os.storage.StorageManager;
import android.os.storage.StorageVolume;

public class MyJavaHelper {
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
}
