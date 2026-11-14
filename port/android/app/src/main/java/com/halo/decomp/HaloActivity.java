package com.halo.decomp;

import android.app.AlertDialog;
import android.content.Context;
import android.content.Intent;
import android.graphics.Insets;
import android.net.Uri;
import android.net.wifi.WifiManager;
import android.os.Build;
import android.os.Bundle;
import android.view.Display;
import android.view.View;
import android.view.ViewGroup;
import android.view.WindowInsets;
import android.view.WindowManager;
import android.widget.Toast;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.nio.charset.StandardCharsets;

import org.libsdl.app.SDLActivity;

/**
 * The game: SDL3's activity, running libmain.so (port/android/host), which
 * loads the game image from the APK's assets.
 */
public class HaloActivity extends SDLActivity {
    /** lets system link's broadcasts in over Wi-Fi while the game runs */
    private WifiManager.MulticastLock multicastLock;
    /**
     * the latest system gesture insets {left, top, right, bottom}, in pixels;
     * written on the UI thread, read by the game's thread: replaced as a
     * whole, never changed in place
     */
    private volatile int[] gestureInsets = new int[] { 0, 0, 0, 0 };
    /** the on-screen touch controls, over SDL's surface */
    private TouchControls touchControls;
    private static final int EXPORT_LAYOUT = 401, IMPORT_LAYOUT = 402;
    private String pendingLayoutExport;
    // ---------- the Steam sign-in's seal (port/android/host/steam_seal.c, from the Steam client's threads):
    // an AES-GCM key of the Android Keystore, which never leaves it, seals the refresh token the host keeps in
    // the app's private storage. A copy of the file opens on no other phone, nor here once the key is gone
    // (steamForget, at a log out; an uninstall)

    private static final String STEAM_KEY_ALIAS = "steam_session";
    private static final int STEAM_TAG_BITS = 128;

    private static javax.crypto.SecretKey steamKey(boolean create) throws Exception {
        java.security.KeyStore store = java.security.KeyStore.getInstance("AndroidKeyStore");
        store.load(null);
        java.security.Key key = store.getKey(STEAM_KEY_ALIAS, null);
        if (key instanceof javax.crypto.SecretKey)
            return (javax.crypto.SecretKey) key;
        if (!create)
            return null;
        javax.crypto.KeyGenerator generator = javax.crypto.KeyGenerator.getInstance(
                android.security.keystore.KeyProperties.KEY_ALGORITHM_AES, "AndroidKeyStore");
        generator.init(new android.security.keystore.KeyGenParameterSpec.Builder(STEAM_KEY_ALIAS,
                android.security.keystore.KeyProperties.PURPOSE_ENCRYPT
                        | android.security.keystore.KeyProperties.PURPOSE_DECRYPT)
                .setBlockModes(android.security.keystore.KeyProperties.BLOCK_MODE_GCM)
                .setEncryptionPaddings(android.security.keystore.KeyProperties.ENCRYPTION_PADDING_NONE)
                .setKeySize(256)
                .build());
        return generator.generateKey();
    }

    /** plain sealed: its vector's length (a byte), the vector, then the cipher text and its tag; null on a failure */
    public static byte[] steamSeal(byte[] plain) {
        try {
            javax.crypto.Cipher cipher = javax.crypto.Cipher.getInstance("AES/GCM/NoPadding");
            cipher.init(javax.crypto.Cipher.ENCRYPT_MODE, steamKey(true));
            byte[] vector = cipher.getIV();
            byte[] sealed = cipher.doFinal(plain);
            byte[] out = new byte[1 + vector.length + sealed.length];
            out[0] = (byte) vector.length;
            System.arraycopy(vector, 0, out, 1, vector.length);
            System.arraycopy(sealed, 0, out, 1 + vector.length, sealed.length);
            return out;
        } catch (Exception e) {
            android.util.Log.w("Halo", "steam: the Keystore could not seal the sign-in: " + e);
            return null;
        }
    }

    /** steamSeal's undone: the plain bytes, or null (another key's, damaged, or the key gone) */
    public static byte[] steamUnseal(byte[] sealed) {
        try {
            javax.crypto.SecretKey key = steamKey(false);
            int vector = sealed.length > 0 ? sealed[0] & 255 : 0;
            if (key == null || vector < 12 || sealed.length < 1 + vector + STEAM_TAG_BITS / 8)
                return null;
            javax.crypto.Cipher cipher = javax.crypto.Cipher.getInstance("AES/GCM/NoPadding");
            cipher.init(javax.crypto.Cipher.DECRYPT_MODE, key,
                    new javax.crypto.spec.GCMParameterSpec(STEAM_TAG_BITS, sealed, 1, vector));
            return cipher.doFinal(sealed, 1 + vector, sealed.length - 1 - vector);
        } catch (Exception e) {
            android.util.Log.w("Halo", "steam: the Keystore could not open the sign-in: " + e);
            return null;
        }
    }

    /** the key deleted (after a log out): what it sealed opens no more */
    public static byte[] steamForget(byte[] unused) {
        try {
            java.security.KeyStore store = java.security.KeyStore.getInstance("AndroidKeyStore");
            store.load(null);
            store.deleteEntry(STEAM_KEY_ALIAS);
        } catch (Exception e) {
            android.util.Log.w("Halo", "steam: the Keystore's key could not be deleted: " + e);
        }
        return null;
    }

    /** the side of a Workshop preview (steam_downloads.c's PREVIEW_SIDE) */
    private static final int STEAM_PREVIEW_SIDE = 256;

    /** a Workshop item's preview picture (port/android/host/steam_downloads.c's preview_decode) fitted to
     * STEAM_PREVIEW_SIDE square on black: its 0xAARRGGBB pixels, least significant byte first; null if it
     * cannot be decoded */
    public static byte[] steamPreview(byte[] data) {
        final int side = STEAM_PREVIEW_SIDE;
        try {
            android.graphics.BitmapFactory.Options options = new android.graphics.BitmapFactory.Options();
            options.inJustDecodeBounds = true;
            android.graphics.BitmapFactory.decodeByteArray(data, 0, data.length, options);
            if (options.outWidth <= 0 || options.outHeight <= 0)
                return null;
            // (a large picture decoded at a fraction of its size, still at least the side)
            int sample = 1;
            while (options.outWidth / (sample * 2) >= side && options.outHeight / (sample * 2) >= side)
                sample *= 2;
            options = new android.graphics.BitmapFactory.Options();
            options.inSampleSize = sample;
            options.inPreferredConfig = android.graphics.Bitmap.Config.ARGB_8888;
            android.graphics.Bitmap picture = android.graphics.BitmapFactory.decodeByteArray(data, 0, data.length,
                    options);
            if (picture == null)
                return null;
            android.graphics.Bitmap fitted = android.graphics.Bitmap.createBitmap(side, side,
                    android.graphics.Bitmap.Config.ARGB_8888);
            android.graphics.Canvas canvas = new android.graphics.Canvas(fitted);
            canvas.drawColor(android.graphics.Color.BLACK);
            float scale = Math.min((float) side / picture.getWidth(), (float) side / picture.getHeight());
            int width = Math.max(1, Math.round(picture.getWidth() * scale));
            int height = Math.max(1, Math.round(picture.getHeight() * scale));
            int left = (side - width) / 2, top = (side - height) / 2;
            canvas.drawBitmap(picture, null, new android.graphics.Rect(left, top, left + width, top + height),
                    new android.graphics.Paint(android.graphics.Paint.FILTER_BITMAP_FLAG));
            int[] pixels = new int[side * side];
            fitted.getPixels(pixels, 0, side, 0, 0, side, side);
            picture.recycle();
            fitted.recycle();
            byte[] out = new byte[side * side * 4];
            for (int i = 0; i < pixels.length; i++) {
                int pixel = pixels[i] | 0xFF000000;
                out[4 * i] = (byte) pixel;
                out[4 * i + 1] = (byte) (pixel >> 8);
                out[4 * i + 2] = (byte) (pixel >> 16);
                out[4 * i + 3] = (byte) (pixel >> 24);
            }
            return out;
        } catch (Exception | OutOfMemoryError e) {
            android.util.Log.w("Halo", "steam: a preview could not be decoded: " + e);
            return null;
        }
    }


    @Override
    protected String[] getLibraries() {
        return new String[] { "SDL3", "main" };
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        if (savedInstanceState != null)
            pendingLayoutExport = savedInstanceState.getString("pending-layout-export");
        if (mLayout != null) {
            touchControls = new TouchControls(this);
            mLayout.addView(touchControls, new ViewGroup.LayoutParams(
                    ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.MATCH_PARENT));
        }
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        preferHighestRefreshRate();
        trackGestureInsets();
        acquireMulticastLock();
        // a new version looked for while the game starts
        Updater.start(this);
    }

    /** the system's file picker, for the touch layout; no storage permission */
    public void chooseLayoutFile(boolean export, String configuration) {
        String message = export
            ? "Choose the folder and the name of the touch layout file."
            : "Choose a touch layout file. It replaces your buttons, the look sensitivity and the General settings.";
        new AlertDialog.Builder(this)
            .setTitle(export ? "Export the layout" : "Import a layout")
            .setMessage(message)
            .setNegativeButton("Cancel", null)
            .setPositiveButton(export ? "Choose the place" : "Choose the file", (dialog, which) -> {
                Intent intent = new Intent(export ? Intent.ACTION_CREATE_DOCUMENT : Intent.ACTION_OPEN_DOCUMENT);
                intent.addCategory(Intent.CATEGORY_OPENABLE);
                intent.setType(export ? "text/plain" : "*/*");
                if (export) {
                    pendingLayoutExport = configuration;
                    intent.putExtra(Intent.EXTRA_TITLE, "halo-touch-layout.halolayout");
                }
                try {
                    startActivityForResult(intent, export ? EXPORT_LAYOUT : IMPORT_LAYOUT);
                } catch (android.content.ActivityNotFoundException e) {
                    pendingLayoutExport = null;
                    Toast.makeText(this, "This device has no file picker.", Toast.LENGTH_LONG).show();
                }
            })
            .show();
    }

    @Override
    protected void onSaveInstanceState(Bundle state) {
        super.onSaveInstanceState(state);
        state.putString("pending-layout-export", pendingLayoutExport);
    }

    @Override
    protected void onActivityResult(int request, int result, Intent data) {
        if (request != EXPORT_LAYOUT && request != IMPORT_LAYOUT) {
            super.onActivityResult(request, result, data);
            return;
        }
        String exported = pendingLayoutExport;
        pendingLayoutExport = null;
        if (result != RESULT_OK || data == null || data.getData() == null)
            return;
        Uri document = data.getData();
        // (the file is read and written away from the UI thread)
        new Thread(() -> {
            try {
                if (request == EXPORT_LAYOUT)
                    writeLayoutFile(document, exported);
                else
                    readLayoutFile(document);
            } catch (Exception e) {
                runOnUiThread(() -> layoutFileError(e));
            }
        }, "halo-touch-layout-file").start();
    }

    private void writeLayoutFile(Uri document, String configuration) throws IOException {
        if (configuration == null)
            throw new IOException("the layout to export is gone");
        try (OutputStream output = getContentResolver().openOutputStream(document, "wt")) {
            if (output == null)
                throw new IOException("the file cannot be opened");
            output.write(configuration.getBytes(StandardCharsets.UTF_8));
        }
        runOnUiThread(() -> Toast.makeText(this, "The layout is exported.", Toast.LENGTH_SHORT).show());
    }

    private void readLayoutFile(Uri document) throws IOException {
        ByteArrayOutputStream contents = new ByteArrayOutputStream();
        try (InputStream input = getContentResolver().openInputStream(document)) {
            if (input == null)
                throw new IOException("the file cannot be opened");
            byte[] buffer = new byte[4096];
            int size;
            while ((size = input.read(buffer)) != -1) {
                if (contents.size() + size > 65536)
                    throw new IOException("the file is too large");
                contents.write(buffer, 0, size);
            }
        }
        String configuration = new String(contents.toByteArray(), StandardCharsets.UTF_8);
        // checked here, then applied to the overlay at once
        TouchLayout.importConfiguration(configuration);
        runOnUiThread(() -> {
            if (isFinishing() || isDestroyed() || touchControls == null)
                return;
            try {
                touchControls.importLayout(configuration);
                Toast.makeText(this, "The layout is imported and saved.", Toast.LENGTH_SHORT).show();
            } catch (IllegalArgumentException e) {
                layoutFileError(e);
            }
        });
    }

    private void layoutFileError(Exception error) {
        if (isFinishing() || isDestroyed())
            return;
        new AlertDialog.Builder(this)
            .setTitle("Touch layout file")
            .setMessage("The operation did not complete: " + error.getMessage())
            .setPositiveButton("OK", null)
            .show();
    }

    @Override
    protected void onResume() {
        super.onResume();
        if (touchControls != null && getWindow().getDecorView().hasWindowFocus())
            touchControls.startDeviceInput();
    }

    @Override
    protected void onPause() {
        if (touchControls != null)
            touchControls.stopDeviceInput();
        super.onPause();
    }

    @Override
    public void onWindowFocusChanged(boolean hasFocus) {
        if (touchControls != null) {
            if (hasFocus)
                touchControls.startDeviceInput();
            else
                touchControls.stopDeviceInput();
        }
        super.onWindowFocusChanged(hasFocus);
    }

    @Override
    protected void onDestroy() {
        if (touchControls != null)
            touchControls.stopDeviceInput();
        if (multicastLock != null && multicastLock.isHeld())
            multicastLock.release();
        multicastLock = null;
        super.onDestroy();
    }

    /**
     * keeps gestureInsets current; Android sends the insets again when the
     * activity rotates (after the game's native code has started on a phone
     * launched from portrait), so a single read at startup would keep the
     * portrait values; the listener hands the insets on so SDL's own
     * handling still sees them
     */
    private void trackGestureInsets() {
        getWindow().getDecorView().setOnApplyWindowInsetsListener(new View.OnApplyWindowInsetsListener() {
            @Override
            public WindowInsets onApplyWindowInsets(View view, WindowInsets insets) {
                gestureInsets = readGestureInsets(insets);
                return view.onApplyWindowInsets(insets);
            }
        });
    }

    @SuppressWarnings("deprecation") // getSystemGestureInsets is the only call on Android 10
    private static int[] readGestureInsets(WindowInsets insets) {
        Insets gesture;

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R)
            gesture = insets.getInsets(WindowInsets.Type.systemGestures());
        else if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.Q)
            gesture = insets.getSystemGestureInsets();
        else
            return new int[] { 0, 0, 0, 0 };
        return new int[] { gesture.left, gesture.top, gesture.right, gesture.bottom };
    }

    /**
     * the edges of the screen where Android keeps its gestures; in sticky
     * full screen the first swipe from an edge only shows the system bars,
     * and Android hands that swipe to the game as an ordinary finger, so
     * the game (port/linux/src/touch_input.c) must ignore touches that
     * begin there; returns a copy of {left, top, right, bottom} in pixels:
     * all 0 before Android 10 (which has no insets) and until the first
     * insets arrive. The game's native code calls this through JNI by name
     * and signature (host_main.c GetMethodID(...,
     * "getSystemGestureInsetsPixels", "()[I")), at every finger down, so
     * it must not be renamed, retyped or removed as unused
     */
    public int[] getSystemGestureInsetsPixels() {
        return gestureInsets.clone();
    }

    /**
     * Many phones drop the Wi-Fi's broadcast and multicast datagrams to
     * save power unless an app holds this: without it they would not see
     * system link games on the local network, nor be seen hosting one.
     */
    private void acquireMulticastLock() {
        try {
            WifiManager wifi = (WifiManager) getApplicationContext().getSystemService(Context.WIFI_SERVICE);
            if (wifi == null)
                return;
            multicastLock = wifi.createMulticastLock("halo-system-link");
            multicastLock.setReferenceCounted(false);
            multicastLock.acquire();
        } catch (RuntimeException e) {
            // (no Wi-Fi, or not allowed: the local network may miss games)
            multicastLock = null;
        }
    }

    /**
     * The game draws a frame at every display refresh, between its 30 Hz
     * ticks (port/linux/game/render_interpolation.c); Android otherwise
     * often keeps an app at 60 Hz on a faster display.
     */
    private void preferHighestRefreshRate() {
        Display display = getWindowManager().getDefaultDisplay();
        Display.Mode current = display.getMode();
        Display.Mode best = current;

        for (Display.Mode mode : display.getSupportedModes()) {
            if (mode.getPhysicalWidth() == current.getPhysicalWidth() &&
                mode.getPhysicalHeight() == current.getPhysicalHeight() &&
                mode.getRefreshRate() > best.getRefreshRate()) {
                best = mode;
            }
        }
        WindowManager.LayoutParams attributes = getWindow().getAttributes();
        attributes.preferredDisplayModeId = best.getModeId();
        getWindow().setAttributes(attributes);
    }
}
