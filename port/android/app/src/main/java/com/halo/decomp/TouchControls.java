package com.halo.decomp;

import android.app.AlertDialog;
import android.content.Context;
import android.content.SharedPreferences;
import android.content.pm.PackageManager;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.hardware.input.InputManager;
import android.media.AudioAttributes;
import android.os.Build;
import android.os.SystemClock;
import android.os.VibrationEffect;
import android.os.Vibrator;
import android.os.VibratorManager;
import android.util.SparseArray;
import android.util.SparseIntArray;
import android.view.InputDevice;
import android.view.MotionEvent;
import android.view.View;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.LinearLayout;
import android.widget.ScrollView;
import android.widget.SeekBar;
import android.widget.Switch;
import android.widget.TextView;
import android.widget.Toast;

import java.util.Arrays;
import java.util.Locale;

/**
 * The on-screen touch controls: a multitouch overlay over SDL's surface,
 * with a stick, the controller's buttons, swipe (and optionally gyroscope)
 * aiming and an editor for the layout (TouchLayout). Its state goes to the
 * game through port/android/host/host_touch.c, as port 0's controller.
 *
 * It shows only in a game, never at the main menu, over a menu (which
 * takes taps itself, port/linux/src/touch_input.c) or during a cinematic,
 * only on a touchscreen, and with input.touch_controls = "auto" only while
 * no controller is connected. While it is not shown it lets every finger
 * through to the game.
 */
public final class TouchControls extends View implements SensorEventListener, InputManager.InputDeviceListener {
    /** what a finger holds besides a control: the view swipe */
    private static final int LOOK = -5;
    private static final int NOTHING = Integer.MIN_VALUE;
    /**
     * the toolbar's items: Hide at the top, Options at the top right until
     * it is dragged, and the layout editor's Export, Import and Save in the
     * middle of the screen
     */
    private static final int TOOLBAR_TOGGLE = -3;
    private static final int TOOLBAR_OPTIONS = -4;
    private static final int TOOLBAR_EXPORT = -6;
    private static final int TOOLBAR_IMPORT = -7;
    private static final int TOOLBAR_SAVE = -8;
    private static final float TOOLBAR_RADIUS = 28;
    private static final float OPTIONS_RADIUS = TouchLayout.OPTIONS_RADIUS;
    /** the editor's items, this far apart */
    private static final float EDITOR_SPACING = 78;

    /** the logical grid the controls are laid out on, scaled to the display's height */
    private static final float GRID_WIDTH = 960;
    private static final float GRID_HEIGHT = 540;
    /** a control is never smaller than a finger: 24 dp of radius, 48 across */
    private static final float MINIMUM_RADIUS_DP = 24;
    /** a finger moves this far (logical units) before a press becomes a swipe */
    private static final float SLOP = 10;
    /** the move stick's dead zone and its touch area beyond its circle */
    private static final float STICK_DEAD_ZONE = 0.12f;
    private static final float STICK_REACH = 1.28f;
    private static final float STICK_KNOB_RADIUS = 24;
    /** a floating stick goes where a thumb lands in this part of the display (from the lower left) */
    private static final float FLOATING_ZONE_WIDTH = 0.45f;
    private static final float FLOATING_ZONE_TOP = 0.35f;
    /** radians per logical unit of swipe (the mouse's rate: xinput_sdl.c) */
    private static final float LOOK_RADIANS_PER_UNIT = 0.0022f;
    /** a label is at most this many radii wide */
    private static final float LABEL_WIDTH = 1.7f;
    private static final long POLL_MS = 16;
    /** the game is in play this long before the controls show (no flash at a level's first frame) */
    private static final long SHOW_DELAY_MS = 200;
    private static final long VIBRATION_MS = 110;
    private static final long VIBRATION_RENEW_MS = 70;

    /** the game's HALO_TOUCH_SCENE bits (port/linux/src/touch_input.c) */
    private static final int SCENE_KNOWN = 1;
    private static final int SCENE_MENUS = 2;
    private static final int SCENE_ON = 4;
    private static final int SCENE_OFF = 8;

    private static final String PREFERENCES = "touch-layout-v1";
    private static final String KEY_CONFIGURATION = "configuration";
    private static final String KEY_HIDDEN = "hidden";

    /**
     * a kind of control: the SDL button bit or trigger axis it sends (-1 for
     * none), and the game's controller button it is (port/linux/game/
     * touch_game.c numbers them: input.h's _gamepad_*), which names it
     */
    private static final class Kind {
        final int bit;
        final int trigger;
        final int gamepadButton;
        final String buttonName;
        final boolean faceButton;

        Kind(int bit, int trigger, int gamepadButton, String buttonName, boolean faceButton) {
            this.bit = bit;
            this.trigger = trigger;
            this.gamepadButton = gamepadButton;
            this.buttonName = buttonName;
            this.faceButton = faceButton;
        }
    }

    /** by TouchLayout's kinds; bits follow SDL_GamepadButton, triggers are SDL axes 4 and 5 */
    private static final Kind[] KINDS = {
        new Kind(0, -1, 0, "A", true),
        new Kind(1, -1, 1, "B", true),
        new Kind(2, -1, 2, "X", true),
        new Kind(3, -1, 3, "Y", true),
        new Kind(-1, 5, 7, "RT", false),
        new Kind(-1, 4, 6, "LT", false),
        new Kind(7, -1, 14, "LS", false),
        new Kind(8, -1, 15, "RS", false),
        new Kind(9, -1, 5, "White", false),
        new Kind(10, -1, 4, "Black", false),
        new Kind(6, -1, 12, "Start", false),
        new Kind(4, -1, 13, "Back", false),
        new Kind(11, -1, 8, "Up", false),
        new Kind(12, -1, 9, "Down", false),
        new Kind(13, -1, 10, "Left", false),
        new Kind(14, -1, 11, "Right", false),
        new Kind(-1, -1, -1, "Move", false),
        new Kind(-1, 5, 7, "RT", false),
    };

    /** the game's controls (input_abstraction.c's _game_control_*), as the buttons name them */
    private static final String[] GAME_CONTROLS = {
        "Jump", "Gren. type", "Reload", "Weapon", "Melee", "Light",
        "Grenade", "Fire", "Pause", "Back", "Crouch", "Zoom",
    };
    /** the default profile's mapping, until the game sends the player's own */
    private static final int[] DEFAULT_BINDINGS = {
        0, 4, 2, 3, 1, 5, 6, 7, -1, -1, -1, -1, 8, 9, 10, 11,
    };

    private static native void nativeState(int lx, int ly, int rx, int ry, int lt, int rt, int buttons);
    private static native void nativeLook(float dx, float dy);
    private static native void nativeGyro(float dx, float dy);
    private static native void nativeLookReset();
    private static native int nativeRumble();
    private static native int nativeScene();
    private static native int nativeBindings(int[] controls);
    private static native void nativeDevices(boolean touchscreen, boolean controller);

    private final Paint paint = new Paint(Paint.ANTI_ALIAS_FLAG);
    private final SharedPreferences preferences;
    private final SensorManager sensors;
    private final Sensor gyroscope;
    private final Vibrator vibrator;
    private final InputManager inputManager;
    private final boolean touchscreenFeature;
    private final GyroscopeAim gyroAim = new GyroscopeAim();
    private final float[] gyroDelta = new float[2];
    private final AudioAttributes rumbleAttributes = new AudioAttributes.Builder()
        .setUsage(AudioAttributes.USAGE_GAME)
        .setContentType(AudioAttributes.CONTENT_TYPE_SONIFICATION)
        .build();

    private TouchLayout layout = new TouchLayout();
    private float sensitivity = 1f;
    /** the controls removed by the toolbar's Hide, until its Touch */
    private boolean hidden;
    private boolean editing;
    /** the option dialogs open (one opens before the last one's dismissal is told) */
    private int openDialogs;

    /** what each finger holds: a control, LOOK, or (in toolbarFingers) a toolbar item */
    private final SparseIntArray owners = new SparseIntArray();
    /** where each finger on a button went down, in pixels: dragged, it swipes */
    private final SparseArray<float[]> buttonTouches = new SparseArray<>();
    /** each finger on a toolbar item: {x, y, item}; the item acts when it lifts there */
    private final SparseArray<float[]> toolbarFingers = new SparseArray<>();
    private final int[] axes = new int[6];
    private int lookPointer = -1;
    private float lookX;
    private float lookY;
    /** a floating stick's centre while a thumb holds it (logical units) */
    private boolean stickFloating;
    private float stickX;
    private float stickY;
    private int dragPointer = -1;
    private int dragControl = -1;
    private float dragOffsetX;
    private float dragOffsetY;

    /** pixels per logical unit, and the display in logical units */
    private float scale = 1;
    private float logicalWidth = GRID_WIDTH;
    private float logicalHeight = GRID_HEIGHT;
    private int insetRight;
    private int insetTop;

    private int scene;
    private boolean controllerConnected;
    private boolean touchscreen;
    private boolean deviceInputActive;
    /** the controls are on screen: shown() for SHOW_DELAY_MS */
    private boolean showing;
    private long shownSince = -1;
    private boolean gyroRegistered;
    private int lastAmplitude;
    private long lastVibration;
    private final int[] bindings = DEFAULT_BINDINGS.clone();
    private final int[] bindingsRead = new int[bindings.length];
    private int bindingsSerial;

    /** reads what the game says every frame: when to show, the buttons' names, and the rumble */
    private final Runnable poll = new Runnable() {
        @Override
        public void run() {
            if (!deviceInputActive)
                return;
            scene = nativeScene();
            updateShowing();
            readBindings();
            updateRumble();
            postDelayed(this, POLL_MS);
        }
    };

    public TouchControls(Context context) {
        super(context);
        sensors = (SensorManager) context.getSystemService(Context.SENSOR_SERVICE);
        gyroscope = sensors == null ? null : sensors.getDefaultSensor(Sensor.TYPE_GYROSCOPE);
        if (Build.VERSION.SDK_INT >= 31) {
            VibratorManager manager = (VibratorManager) context.getSystemService(Context.VIBRATOR_MANAGER_SERVICE);
            vibrator = manager == null ? null : manager.getDefaultVibrator();
        } else {
            vibrator = (Vibrator) context.getSystemService(Context.VIBRATOR_SERVICE);
        }
        inputManager = (InputManager) context.getSystemService(Context.INPUT_SERVICE);
        touchscreenFeature = context.getPackageManager().hasSystemFeature(PackageManager.FEATURE_TOUCHSCREEN);
        preferences = context.getSharedPreferences(PREFERENCES, Context.MODE_PRIVATE);
        loadLayout(context);
        hidden = preferences.getBoolean(KEY_HIDDEN, false);
        refreshInputDevices();
        setFocusable(false);
        setContentDescription("Halo touch controller");
        setOnApplyWindowInsetsListener((view, insets) -> {
            // (only the toolbar moves: the fingers stay held as the system bars come and go)
            int right = insetRight;
            int top = insetTop;
            insetRight = insets.getSystemWindowInsetRight();
            insetTop = insets.getSystemWindowInsetTop();
            if (insets.getDisplayCutout() != null) {
                insetRight = Math.max(insetRight, insets.getDisplayCutout().getSafeInsetRight());
                insetTop = Math.max(insetTop, insets.getDisplayCutout().getSafeInsetTop());
            }
            if (insetRight != right || insetTop != top)
                invalidate();
            return insets;
        });
    }

    private void loadLayout(Context context) {
        String configuration = preferences.getString(KEY_CONFIGURATION, null);
        if (configuration == null)
            return;
        try {
            TouchLayout.Configuration saved = TouchLayout.importConfiguration(configuration);
            layout = saved.layout;
            sensitivity = saved.sensitivity;
        } catch (IllegalArgumentException e) {
            Toast.makeText(context, "The saved touch layout could not be read. Using the default one.",
                Toast.LENGTH_LONG).show();
        }
    }

    // ---------- the activity's lifecycle (HaloActivity)

    public void startDeviceInput() {
        if (deviceInputActive)
            return;
        deviceInputActive = true;
        if (inputManager != null)
            inputManager.registerInputDeviceListener(this, null);
        refreshInputDevices();
        gyroAim.reset();
        updateSensors();
        post(poll);
    }

    public void stopDeviceInput() {
        deviceInputActive = false;
        removeCallbacks(poll);
        if (inputManager != null)
            inputManager.unregisterInputDeviceListener(this);
        updateSensors();
        cancelRumble();
        reset();
    }

    // ---------- the display

    @Override
    protected void onSizeChanged(int width, int height, int oldWidth, int oldHeight) {
        layoutControls();
        requestApplyInsets();
    }

    /** scales the grid to the display (its height, keeping the circles round) */
    private void layoutControls() {
        if (getWidth() <= 0 || getHeight() <= 0)
            return;
        reset();
        float width = getWidth();
        float height = getHeight();
        scale = Math.max(0.01f, Math.min(width / GRID_WIDTH, height / GRID_HEIGHT));
        logicalWidth = width / scale;
        logicalHeight = height / scale;
        layout.bounds(logicalWidth, logicalHeight);
        layout.setMinimumRadius(minimumRadius());
        invalidate();
    }

    /** a finger's width, in logical units (at most TouchLayout.MAX_MINIMUM_RADIUS) */
    private float minimumRadius() {
        float radius = MINIMUM_RADIUS_DP * getResources().getDisplayMetrics().density / scale;
        return Math.min(radius, TouchLayout.MAX_MINIMUM_RADIUS);
    }

    private float logical(float pixels) {
        return pixels / scale;
    }

    private float toolbarY() {
        return Math.max(38, insetTop / scale + 38);
    }

    /** Options: where it was dragged, else at the top right, clear of a cutout */
    private float optionsX() {
        if (layout.optionsMoved())
            return layout.optionsX();
        return logicalWidth - Math.max(42, insetRight / scale + 42);
    }

    private float optionsY() {
        return layout.optionsMoved() ? layout.optionsY() : toolbarY();
    }

    private float editorY() {
        return logicalHeight / 2;
    }

    private float toolbarRadius(float radius) {
        return Math.max(radius, minimumRadius());
    }

    // ---------- when the controls show

    /**
     * whether the controls belong on screen: in a game (no menu or
     * cinematic), never without a touchscreen, and with
     * input.touch_controls = "auto" only while no controller is connected
     */
    private boolean shown() {
        if ((scene & SCENE_KNOWN) == 0 || (scene & (SCENE_MENUS | SCENE_OFF)) != 0 || !touchscreen)
            return false;
        return (scene & SCENE_ON) != 0 || !controllerConnected;
    }

    /** shows the controls once shown() has held for SHOW_DELAY_MS; hides them at once */
    private void updateShowing() {
        boolean show = false;
        if (!shown()) {
            shownSince = -1;
        } else {
            long now = SystemClock.uptimeMillis();
            if (shownSince < 0)
                shownSince = now;
            show = now - shownSince >= SHOW_DELAY_MS;
        }
        if (show != showing) {
            showing = show;
            reset();
            updateSensors();
        }
    }

    /** a game controller: a stick or a hat, not a phone's few gamepad keys */
    private static boolean isController(InputDevice device) {
        if (device == null || device.isVirtual())
            return false;
        if ((device.getSources() & InputDevice.SOURCE_JOYSTICK) != InputDevice.SOURCE_JOYSTICK)
            return false;
        return device.getMotionRange(MotionEvent.AXIS_X, InputDevice.SOURCE_JOYSTICK) != null
            || device.getMotionRange(MotionEvent.AXIS_HAT_X, InputDevice.SOURCE_JOYSTICK) != null;
    }

    private void refreshInputDevices() {
        boolean controller = false;
        boolean touch = touchscreenFeature;
        for (int id : InputDevice.getDeviceIds()) {
            InputDevice device = InputDevice.getDevice(id);
            if (device == null)
                continue;
            if (isController(device))
                controller = true;
            boolean touchSource = (device.getSources() & InputDevice.SOURCE_TOUCHSCREEN) == InputDevice.SOURCE_TOUCHSCREEN;
            if (!device.isVirtual() && touchSource)
                touch = true;
        }
        controllerConnected = controller;
        touchscreen = touch;
        // (the game's own touchscreen buttons follow the same rule: touch_input.c)
        nativeDevices(touch, controller);
        updateShowing();
    }

    @Override
    public void onInputDeviceAdded(int deviceId) {
        refreshInputDevices();
    }

    @Override
    public void onInputDeviceRemoved(int deviceId) {
        refreshInputDevices();
    }

    @Override
    public void onInputDeviceChanged(int deviceId) {
        refreshInputDevices();
    }

    private void setHidden(boolean hidden) {
        reset();
        this.hidden = hidden;
        preferences.edit().putBoolean(KEY_HIDDEN, hidden).apply();
        updateSensors();
    }

    // ---------- the controller state the game reads

    /** lets go of every finger and sends a released controller */
    public void reset() {
        dragPointer = -1;
        dragControl = -1;
        owners.clear();
        buttonTouches.clear();
        toolbarFingers.clear();
        lookPointer = -1;
        stickFloating = false;
        gyroAim.reset();
        nativeLookReset();
        Arrays.fill(axes, 0);
        publish();
        invalidate();
    }

    private void publish() {
        if (editing || openDialogs > 0) {
            nativeState(0, 0, 0, 0, 0, 0, 0);
            return;
        }
        int bits = 0;
        axes[4] = 0;
        axes[5] = 0;
        for (int i = 0; i < owners.size(); i++) {
            int control = owners.valueAt(i);
            if (control < 0 || control >= layout.size())
                continue;
            Kind kind = KINDS[layout.type(control)];
            if (kind.bit >= 0)
                bits |= 1 << kind.bit;
            if (kind.trigger >= 0)
                axes[kind.trigger] = 32767;
        }
        nativeState(axes[0], axes[1], axes[2], axes[3], axes[4], axes[5], bits);
    }

    private boolean held(int control) {
        return owners.indexOfValue(control) >= 0;
    }

    private float stickCenterX() {
        return stickFloating ? stickX : layout.x(TouchLayout.LEFT);
    }

    private float stickCenterY() {
        return stickFloating ? stickY : layout.y(TouchLayout.LEFT);
    }

    /** the floating stick, centred where a thumb landed (at rest there, even near an edge) */
    private void floatStick(float x, float y) {
        stickFloating = true;
        stickX = x;
        stickY = y;
    }

    private boolean inFloatingZone(float x, float y) {
        return x < logicalWidth * FLOATING_ZONE_WIDTH && y > logicalHeight * FLOATING_ZONE_TOP;
    }

    private void moveStick(int control, float x, float y) {
        float dx = (x - stickCenterX()) / layout.radius(control);
        float dy = (y - stickCenterY()) / layout.radius(control);
        float length = (float) Math.sqrt(dx * dx + dy * dy);
        if (length < STICK_DEAD_ZONE) {
            dx = 0;
            dy = 0;
        } else if (length > 1) {
            dx /= length;
            dy /= length;
        }
        axes[0] = Math.round(dx * 32767);
        axes[1] = Math.round(dy * 32767);
    }

    // ---------- fingers

    private static boolean inside(float x, float y, float cx, float cy, float radius) {
        return (x - cx) * (x - cx) + (y - cy) * (y - cy) <= radius * radius;
    }

    /** what is under a point (logical units): a toolbar item, a control, or NOTHING */
    private int hit(float x, float y) {
        if (inside(x, y, optionsX(), optionsY(), toolbarRadius(OPTIONS_RADIUS)))
            return TOOLBAR_OPTIONS;
        if (!editing) {
            if (inside(x, y, logicalWidth / 2, toolbarY(), toolbarRadius(TOOLBAR_RADIUS)))
                return TOOLBAR_TOGGLE;
            return hidden ? NOTHING : hitControl(x, y);
        }
        // (the controls first: one dropped over Export, Import or Save can still be dragged away)
        int control = hitControl(x, y);
        if (control != NOTHING)
            return control;
        float middle = logicalWidth / 2;
        if (inside(x, y, middle - EDITOR_SPACING, editorY(), toolbarRadius(OPTIONS_RADIUS)))
            return TOOLBAR_EXPORT;
        if (inside(x, y, middle, editorY(), toolbarRadius(OPTIONS_RADIUS)))
            return TOOLBAR_IMPORT;
        if (inside(x, y, middle + EDITOR_SPACING, editorY(), toolbarRadius(OPTIONS_RADIUS)))
            return TOOLBAR_SAVE;
        return NOTHING;
    }

    /** the control under a point (logical units), or NOTHING */
    private int hitControl(float x, float y) {
        for (int i = 0; i < layout.size(); i++) {
            if (layout.shown(i) && layout.type(i) != TouchLayout.LEFT
                    && inside(x, y, layout.x(i), layout.y(i), layout.radius(i)))
                return i;
        }
        int stick = TouchLayout.LEFT;
        float reach = editing ? 1 : STICK_REACH;
        if (layout.shown(stick) && !held(stick)
                && inside(x, y, layout.x(stick), layout.y(stick), layout.radius(stick) * reach))
            return stick;
        return NOTHING;
    }

    /**
     * whether a finger went down where Android keeps its edge gestures; in
     * sticky full screen the first swipe in from an edge only shows the
     * system bars, and Android also hands it over as an ordinary finger.
     * Only fingers on no control are kept out of them: the zones stay their
     * full size while the bars are hidden (all of the hidden 3-button bar),
     * and a control drawn there would never press
     */
    private boolean inGestureZone(float x, float y) {
        int[] insets = ((HaloActivity) getContext()).getSystemGestureInsetsPixels();
        return x < insets[0] || x >= getWidth() - insets[2]
            || y < insets[1] || y >= getHeight() - insets[3];
    }

    @Override
    public boolean onTouchEvent(MotionEvent event) {
        int action = event.getActionMasked();
        if (!showing && !editing && openDialogs == 0) {
            // the menus and cinematics take the fingers (SDL's surface below)
            if (action == MotionEvent.ACTION_DOWN)
                return false;
            if (owners.size() > 0 || lookPointer >= 0)
                reset();
            return true;
        }
        if (editing)
            editTouch(event);
        else
            gameTouch(event);
        publish();
        invalidate();
        return true;
    }

    @Override
    public boolean performClick() {
        super.performClick();
        return true;
    }

    private void gameTouch(MotionEvent event) {
        int index = event.getActionIndex();
        int id = event.getPointerId(index);
        switch (event.getActionMasked()) {
            case MotionEvent.ACTION_DOWN:
            case MotionEvent.ACTION_POINTER_DOWN:
                fingerDown(id, event.getX(index), event.getY(index));
                break;
            case MotionEvent.ACTION_MOVE:
                for (int i = 0; i < event.getPointerCount(); i++)
                    fingerMoved(event.getPointerId(i), event.getX(i), event.getY(i));
                break;
            case MotionEvent.ACTION_UP:
            case MotionEvent.ACTION_POINTER_UP:
                fingerUp(id, event.getX(index), event.getY(index));
                break;
            case MotionEvent.ACTION_CANCEL:
                reset();
                break;
            default:
                break;
        }
    }

    /** a finger down in a game, at a point in pixels */
    private void fingerDown(int id, float px, float py) {
        float x = logical(px);
        float y = logical(py);
        int target = hit(x, y);
        int stick = TouchLayout.LEFT;
        if (target == NOTHING && layout.floatingStick && !hidden && layout.shown(stick) && !held(stick)
                && inFloatingZone(x, y) && !inGestureZone(px, py)) {
            floatStick(x, y);
            target = stick;
        }
        if (target == TOOLBAR_OPTIONS || target == TOOLBAR_TOGGLE) {
            toolbarFingers.put(id, new float[] {px, py, target});
        } else if (target != NOTHING) {
            owners.put(id, target);
            if (layout.type(target) == TouchLayout.LEFT)
                moveStick(target, x, y);
            else if (layout.buttonsAim)
                buttonTouches.put(id, new float[] {px, py});
        } else if (!hidden && lookPointer < 0 && !inGestureZone(px, py)) {
            startLook(id, px, py);
        }
    }

    private void startLook(int id, float px, float py) {
        lookPointer = id;
        lookX = px;
        lookY = py;
        if (owners.get(id, NOTHING) == NOTHING)
            owners.put(id, LOOK);
    }

    private boolean movedPastSlop(float[] origin, float px, float py) {
        return Math.hypot(px - origin[0], py - origin[1]) > SLOP * scale;
    }

    private void fingerMoved(int id, float px, float py) {
        int control = owners.get(id, NOTHING);
        if (control >= 0 && layout.type(control) == TouchLayout.LEFT)
            moveStick(control, logical(px), logical(py));
        float[] toolbar = toolbarFingers.get(id);
        if (toolbar != null && movedPastSlop(toolbar, px, py)) {
            // a swipe that began on the toolbar aims; the item does not act
            toolbarFingers.remove(id);
            if (!hidden && lookPointer < 0)
                startLook(id, toolbar[0], toolbar[1]);
        }
        float[] origin = buttonTouches.get(id);
        if (origin != null && lookPointer >= 0 && lookPointer != id) {
            // while another finger aims, a held button's finger may only take over from where it is
            origin[0] = px;
            origin[1] = py;
        }
        if (lookPointer < 0 && origin != null && movedPastSlop(origin, px, py)) {
            // a held button (either Fire too) also aims
            lookPointer = id;
            lookX = origin[0];
            lookY = origin[1];
        }
        if (id == lookPointer) {
            nativeLook(logical(px - lookX) * sensitivity, logical(py - lookY) * sensitivity);
            lookX = px;
            lookY = py;
        }
    }

    private void fingerUp(int id, float px, float py) {
        float[] toolbar = toolbarFingers.get(id);
        if (toolbar != null) {
            toolbarFingers.remove(id);
            int item = (int) toolbar[2];
            if (hit(logical(px), logical(py)) == item)
                toolbarAction(item);
            return;
        }
        int control = owners.get(id, NOTHING);
        if (control >= 0 && layout.type(control) == TouchLayout.LEFT) {
            axes[0] = 0;
            axes[1] = 0;
            stickFloating = false;
        }
        if (id == lookPointer)
            lookPointer = -1;
        owners.delete(id);
        buttonTouches.remove(id);
    }

    private void toolbarAction(int item) {
        if (item == TOOLBAR_OPTIONS)
            showOptions();
        else if (item == TOOLBAR_TOGGLE)
            setHidden(!hidden);
        performClick();
    }

    /** a finger in the layout editor: drags controls; Save, Export and Import act at once */
    private void editTouch(MotionEvent event) {
        int action = event.getActionMasked();
        int index = event.getActionIndex();
        int id = event.getPointerId(index);
        if (action == MotionEvent.ACTION_DOWN || action == MotionEvent.ACTION_POINTER_DOWN) {
            if (dragPointer >= 0)
                return;
            float x = logical(event.getX(index));
            float y = logical(event.getY(index));
            int target = hit(x, y);
            if (target == TOOLBAR_SAVE) {
                if (saveLayout()) {
                    reset();
                    editing = false;
                    performClick();
                }
            } else if (target == TOOLBAR_EXPORT || target == TOOLBAR_IMPORT) {
                reset();
                ((HaloActivity) getContext()).chooseLayoutFile(target == TOOLBAR_EXPORT, exportLayout());
            } else if (target == TOOLBAR_OPTIONS) {
                // (Options moves as a control does; Save is in the middle)
                dragPointer = id;
                dragControl = target;
                dragOffsetX = x - optionsX();
                dragOffsetY = y - optionsY();
            } else if (target >= 0) {
                dragPointer = id;
                dragControl = target;
                dragOffsetX = x - layout.x(target);
                dragOffsetY = y - layout.y(target);
            }
        } else if (action == MotionEvent.ACTION_MOVE && dragPointer >= 0) {
            int pointer = event.findPointerIndex(dragPointer);
            if (pointer >= 0) {
                float x = logical(event.getX(pointer)) - dragOffsetX;
                float y = logical(event.getY(pointer)) - dragOffsetY;
                if (dragControl == TOOLBAR_OPTIONS)
                    layout.moveOptions(x, y);
                else
                    layout.move(dragControl, x, y);
            }
        } else if (action == MotionEvent.ACTION_CANCEL
                || ((action == MotionEvent.ACTION_UP || action == MotionEvent.ACTION_POINTER_UP) && id == dragPointer)) {
            dragPointer = -1;
            dragControl = -1;
        }
    }

    // ---------- the buttons' names, from the player's profile

    private void readBindings() {
        int serial = nativeBindings(bindingsRead);
        if (serial == 0 || serial == bindingsSerial)
            return;
        bindingsSerial = serial;
        if (!Arrays.equals(bindingsRead, bindings)) {
            System.arraycopy(bindingsRead, 0, bindings, 0, bindings.length);
            invalidate();
        }
    }

    /** a control's name: its button and what the profile has it do ("A / Jump", "Fire", "RT") */
    private String label(int type) {
        Kind kind = KINDS[type];
        if (kind.gamepadButton < 0)
            return kind.buttonName;
        int control = bindings[kind.gamepadButton];
        if (control < 0 || control >= GAME_CONTROLS.length)
            return kind.buttonName;
        return kind.faceButton ? kind.buttonName + " / " + GAME_CONTROLS[control] : GAME_CONTROLS[control];
    }

    private String controlName(int type) {
        return type == TouchLayout.LEFT ? "Move stick" : label(type);
    }

    // ---------- the layout, saved and in files

    private boolean saveLayout() {
        boolean saved = preferences.edit().putString(KEY_CONFIGURATION, exportLayout()).commit();
        if (!saved)
            Toast.makeText(getContext(), "The touch layout could not be saved. Try again.", Toast.LENGTH_LONG).show();
        return saved;
    }

    public String exportLayout() {
        return layout.exportConfiguration(sensitivity);
    }

    /** replaces the layout with a layout file's (saved first: it throws if the file is not valid) */
    public void importLayout(String text) {
        TouchLayout.Configuration imported = TouchLayout.importConfiguration(text);
        imported.layout.bounds(logicalWidth, logicalHeight);
        imported.layout.setMinimumRadius(minimumRadius());
        String normalized = imported.layout.exportConfiguration(imported.sensitivity);
        if (!preferences.edit().putString(KEY_CONFIGURATION, normalized).commit())
            throw new IllegalArgumentException("Could not save the imported layout");
        reset();
        layout = imported.layout;
        sensitivity = imported.sensitivity;
        setHidden(false);
        cancelRumble();
        invalidate();
    }

    // ---------- the options

    /** opens an option dialog; the controls are released while any is open, and saved when the last closes */
    private void openDialog(AlertDialog dialog) {
        dialog.setOnDismissListener(d -> {
            openDialogs--;
            if (openDialogs > 0)
                return;
            openDialogs = 0;
            saveLayout();
            reset();
            updateSensors();
        });
        openDialogs++;
        reset();
        dialog.show();
    }

    private void showOptions() {
        String[] items = {"General", "Edit buttons layout", "Look sensitivity"};
        AlertDialog dialog = new AlertDialog.Builder(getContext())
            .setTitle("Options")
            .setItems(items, (d, which) -> {
                if (which == 0) {
                    post(this::showGeneral);
                } else if (which == 1) {
                    editing = true;
                    setHidden(false);
                } else {
                    post(this::showSensitivity);
                }
            })
            .setNegativeButton("Close", null)
            .create();
        openDialog(dialog);
    }

    private TextView text(String value) {
        TextView view = new TextView(getContext());
        view.setText(value);
        return view;
    }

    private Button button(String label, View.OnClickListener listener) {
        Button button = new Button(getContext());
        button.setText(label);
        button.setOnClickListener(listener);
        return button;
    }

    private void showGeneral() {
        cancelRumble();
        LinearLayout panel = new LinearLayout(getContext());
        panel.setOrientation(LinearLayout.VERTICAL);
        panel.setPadding(24, 16, 24, 16);

        boolean hasRumble = vibrator != null && vibrator.hasVibrator();
        Switch rumble = new Switch(getContext());
        rumble.setText(hasRumble ? "Vibration" : "Vibration (not on this phone)");
        rumble.setChecked(layout.rumbleEnabled);
        rumble.setEnabled(hasRumble);
        rumble.setOnCheckedChangeListener((view, enabled) -> {
            layout.rumbleEnabled = enabled;
            cancelRumble();
            saveLayout();
        });
        panel.addView(rumble);

        Switch gyro = new Switch(getContext());
        gyro.setText(gyroscope != null ? "Gyroscope aiming (experimental)" : "Gyroscope aiming (not on this phone)");
        gyro.setChecked(layout.gyroscopeEnabled);
        gyro.setEnabled(gyroscope != null);
        gyro.setOnCheckedChangeListener((view, enabled) -> {
            layout.gyroscopeEnabled = enabled;
            gyroAim.reset();
            saveLayout();
        });
        panel.addView(gyro);

        Switch floating = new Switch(getContext());
        floating.setText("Floating move stick: it goes where your thumb lands in the lower left");
        floating.setChecked(layout.floatingStick);
        floating.setOnCheckedChangeListener((view, enabled) -> {
            layout.floatingStick = enabled;
            saveLayout();
        });
        panel.addView(floating);

        Switch buttonsStill = new Switch(getContext());
        buttonsStill.setText("Held buttons don't aim: a finger that swipes from a button only holds it");
        buttonsStill.setChecked(!layout.buttonsAim);
        buttonsStill.setOnCheckedChangeListener((view, enabled) -> {
            layout.buttonsAim = !enabled;
            saveLayout();
        });
        panel.addView(buttonsStill);

        TextView opacityLabel = text("");
        SeekBar opacity = new SeekBar(getContext());
        int steps = Math.round((TouchLayout.MAX_OPACITY - TouchLayout.MIN_OPACITY) * 20);
        opacity.setMax(steps);
        opacity.setProgress(Math.round((layout.opacity - TouchLayout.MIN_OPACITY) * 20));
        Runnable showOpacity = () -> opacityLabel.setText(String.format(Locale.US, "Opacity: %d%%",
            Math.round(layout.opacity * 100)));
        showOpacity.run();
        opacity.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar bar, int progress, boolean user) {
                layout.setOpacity(Math.min(TouchLayout.MAX_OPACITY, TouchLayout.MIN_OPACITY + progress / 20f));
                showOpacity.run();
                invalidate();
            }

            @Override
            public void onStartTrackingTouch(SeekBar bar) {
            }

            @Override
            public void onStopTrackingTouch(SeekBar bar) {
                saveLayout();
            }
        });
        panel.addView(opacityLabel);
        panel.addView(opacity);

        // (scrolled: in portrait, or on a short display, the rows do not all fit)
        ScrollView scroll = new ScrollView(getContext());
        scroll.addView(panel);
        AlertDialog dialog = new AlertDialog.Builder(getContext())
            .setTitle("General")
            .setView(scroll)
            .setPositiveButton("Done", null)
            .create();
        panel.addView(button("Hide or add buttons", view -> {
            dialog.dismiss();
            post(this::showButtonManager);
        }));
        panel.addView(button("Edit buttons size", view -> {
            dialog.dismiss();
            post(this::showButtonSizes);
        }));
        openDialog(dialog);
    }

    private String rowName(int control, boolean withHidden) {
        String name = controlName(layout.type(control));
        if (control >= TouchLayout.BASE_COUNT)
            name += " (copy)";
        if (withHidden && !layout.shown(control))
            name += " (hidden)";
        return name;
    }

    private LinearLayout row(int control, boolean withHidden) {
        LinearLayout row = new LinearLayout(getContext());
        row.setPadding(16, 4, 16, 4);
        TextView name = text(rowName(control, withHidden));
        name.setTextColor(Color.WHITE);
        row.addView(name, new LinearLayout.LayoutParams(0, LinearLayout.LayoutParams.WRAP_CONTENT, 1));
        return row;
    }

    private void showButtonSizes() {
        LinearLayout list = new LinearLayout(getContext());
        list.setOrientation(LinearLayout.VERTICAL);
        for (int i = 0; i < layout.size(); i++) {
            final int control = i;
            LinearLayout row = row(control, true);
            TextView value = text("");
            value.setTextColor(Color.WHITE);
            Button smaller = new Button(getContext());
            Button larger = new Button(getContext());
            Runnable refresh = () -> {
                int percent = Math.round(layout.sizeScale(control) * 100);
                value.setText(percent + "%");
                smaller.setEnabled(percent > Math.round(TouchLayout.MIN_SIZE * 100));
                larger.setEnabled(percent < Math.round(TouchLayout.MAX_SIZE * 100));
            };
            smaller.setText("-");
            smaller.setOnClickListener(view -> resize(control, -10, refresh));
            larger.setText("+");
            larger.setOnClickListener(view -> resize(control, 10, refresh));
            row.addView(smaller);
            row.addView(value);
            row.addView(larger);
            list.addView(row);
            refresh.run();
        }
        ScrollView scroll = new ScrollView(getContext());
        scroll.addView(list);
        openDialog(new AlertDialog.Builder(getContext())
            .setTitle("Edit buttons size")
            .setView(scroll)
            .setPositiveButton("Done", null)
            .create());
    }

    private void resize(int control, int percent, Runnable refresh) {
        int size = Math.round(layout.sizeScale(control) * 100) + percent;
        size = Math.max(Math.round(TouchLayout.MIN_SIZE * 100), Math.min(Math.round(TouchLayout.MAX_SIZE * 100), size));
        layout.setSize(control, size / 100f);
        saveLayout();
        refresh.run();
        invalidate();
    }

    private void showButtonManager() {
        LinearLayout list = new LinearLayout(getContext());
        list.setOrientation(LinearLayout.VERTICAL);
        Runnable[] refresh = new Runnable[1];
        refresh[0] = () -> {
            list.removeAllViews();
            for (int i = 0; i < layout.size(); i++) {
                final int control = i;
                LinearLayout row = row(control, false);
                row.addView(button(layout.shown(control) ? "Hide" : "Show", view -> {
                    reset();
                    layout.setShown(control, !layout.shown(control));
                    saveLayout();
                    refresh[0].run();
                    invalidate();
                }));
                if (layout.type(control) != TouchLayout.LEFT) {
                    row.addView(button("Duplicate", view -> {
                        reset();
                        if (layout.duplicate(control) < 0)
                            toast("At most " + TouchLayout.MAX_CONTROLS + " controls. Reset removes the copies.");
                        else
                            saveLayout();
                        refresh[0].run();
                        invalidate();
                    }));
                }
                list.addView(row);
            }
        };
        refresh[0].run();
        ScrollView scroll = new ScrollView(getContext());
        scroll.addView(list);
        AlertDialog dialog = new AlertDialog.Builder(getContext())
            .setTitle("Hide or add buttons")
            .setView(scroll)
            .setPositiveButton("Done", null)
            .setNegativeButton("Reset", null)
            .setNeutralButton("Add button", null)
            .create();
        openDialog(dialog);
        dialog.getButton(AlertDialog.BUTTON_NEGATIVE).setOnClickListener(view -> confirmReset(refresh[0]));
        dialog.getButton(AlertDialog.BUTTON_NEUTRAL).setOnClickListener(view -> chooseButtonToAdd(refresh[0]));
    }

    private void confirmReset(Runnable refresh) {
        new AlertDialog.Builder(getContext())
            .setTitle("Reset the buttons?")
            .setMessage("This puts every button, Options too, back in its place and size, shows them all and removes the copies.")
            .setNegativeButton("Cancel", null)
            .setPositiveButton("Reset", (d, which) -> {
                reset();
                layout.resetDefaults();
                setHidden(false);
                saveLayout();
                refresh.run();
                invalidate();
            })
            .show();
    }

    private void chooseButtonToAdd(Runnable refresh) {
        String[] names = new String[TouchLayout.BASE_COUNT];
        for (int i = 0; i < names.length; i++)
            names[i] = controlName(i);
        new AlertDialog.Builder(getContext())
            .setTitle("Add button")
            .setItems(names, (d, type) -> {
                reset();
                if (layout.add(type) < 0)
                    toast(type == TouchLayout.LEFT ? "The move stick already shows." : "At most " + TouchLayout.MAX_CONTROLS + " controls.");
                else
                    saveLayout();
                refresh.run();
                invalidate();
            })
            .setNegativeButton("Cancel", null)
            .show();
    }

    private void showSensitivity() {
        LinearLayout panel = new LinearLayout(getContext());
        panel.setOrientation(LinearLayout.VERTICAL);
        panel.setPadding(32, 16, 32, 16);
        TextView value = text("");
        SeekBar slider = new SeekBar(getContext());
        int steps = Math.round((TouchLayout.MAX_SENSITIVITY - TouchLayout.MIN_SENSITIVITY) / 0.025f);
        slider.setMax(steps);
        slider.setProgress(Math.round((sensitivity - TouchLayout.MIN_SENSITIVITY) / 0.025f));
        Runnable showValue = () -> value.setText(String.format(Locale.US, "Look sensitivity: %.2fx", sensitivity));
        showValue.run();
        slider.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar bar, int progress, boolean user) {
                sensitivity = Math.min(TouchLayout.MAX_SENSITIVITY, TouchLayout.MIN_SENSITIVITY + progress * 0.025f);
                showValue.run();
            }

            @Override
            public void onStartTrackingTouch(SeekBar bar) {
            }

            @Override
            public void onStopTrackingTouch(SeekBar bar) {
                saveLayout();
            }
        });
        panel.addView(value);
        panel.addView(slider);
        openDialog(new AlertDialog.Builder(getContext())
            .setTitle("Look sensitivity")
            .setView(panel)
            .setPositiveButton("Done", null)
            .create());
    }

    private void toast(String message) {
        Toast.makeText(getContext(), message, Toast.LENGTH_LONG).show();
    }

    // ---------- the gyroscope and the vibration

    private void updateSensors() {
        boolean needed = deviceInputActive && showing && !hidden && layout.gyroscopeEnabled && gyroscope != null;
        if (needed && !gyroRegistered) {
            gyroAim.reset();
            gyroRegistered = sensors.registerListener(this, gyroscope, SensorManager.SENSOR_DELAY_GAME);
        } else if (!needed && gyroRegistered) {
            sensors.unregisterListener(this);
            gyroRegistered = false;
            gyroAim.reset();
        }
    }

    @Override
    public void onSensorChanged(SensorEvent event) {
        if (!deviceInputActive || !showing || hidden || !layout.gyroscopeEnabled || editing || openDialogs > 0) {
            gyroAim.reset();
            return;
        }
        int rotation = ((WindowManager) getContext().getSystemService(Context.WINDOW_SERVICE))
            .getDefaultDisplay().getRotation();
        if (gyroAim.sample(event.timestamp, event.values[0], event.values[1], rotation, gyroDelta)) {
            // as a swipe of this many logical units (which the profile's invert leaves alone)
            nativeGyro(-gyroDelta[0] / LOOK_RADIANS_PER_UNIT * sensitivity,
                -gyroDelta[1] / LOOK_RADIANS_PER_UNIT * sensitivity);
        }
    }

    @Override
    public void onAccuracyChanged(Sensor sensor, int accuracy) {
    }

    /** the phone vibrates as port 0's controller would, while the controls show */
    private void updateRumble() {
        if (vibrator == null || !vibrator.hasVibrator())
            return;
        boolean active = showing && !hidden && layout.rumbleEnabled && !editing && openDialogs == 0;
        int amplitude = active ? nativeRumble() : 0;
        if (amplitude == 0) {
            cancelRumble();
            return;
        }
        long now = SystemClock.uptimeMillis();
        if (amplitude == lastAmplitude && now - lastVibration < VIBRATION_RENEW_MS)
            return;
        try {
            int strength = vibrator.hasAmplitudeControl() ? amplitude : VibrationEffect.DEFAULT_AMPLITUDE;
            vibrator.vibrate(VibrationEffect.createOneShot(VIBRATION_MS, strength), rumbleAttributes);
            lastAmplitude = amplitude;
            lastVibration = now;
        } catch (RuntimeException e) {
            cancelRumble();
        }
    }

    private void cancelRumble() {
        if (lastAmplitude != 0 && vibrator != null) {
            try {
                vibrator.cancel();
            } catch (RuntimeException ignored) {
                // the vibrator may already be gone with the activity
            }
        }
        lastAmplitude = 0;
    }

    // ---------- drawing

    private static int alpha(int color, float opacity) {
        int alpha = Math.round(Color.alpha(color) * opacity);
        return (color & 0x00ffffff) | (Math.max(0, Math.min(255, alpha)) << 24);
    }

    private void circle(Canvas canvas, float x, float y, float radius, String label, boolean active,
            float textSize, float opacity) {
        paint.setStyle(Paint.Style.FILL);
        paint.setColor(alpha(active ? 0x9983d9ff : 0x55304050, opacity));
        canvas.drawCircle(x, y, radius, paint);
        paint.setStyle(Paint.Style.STROKE);
        paint.setStrokeWidth(2);
        paint.setColor(alpha(active ? 0xffaee7ff : 0xffffffff, opacity));
        canvas.drawCircle(x, y, radius, paint);
        paint.setStyle(Paint.Style.FILL);
        paint.setColor(alpha(Color.WHITE, Math.max(opacity, 0.5f)));
        paint.setTextAlign(Paint.Align.CENTER);
        paint.setTextSize(textSize);
        // (a long name, "Y / Weapon", is made smaller to fit inside the circle)
        float width = paint.measureText(label);
        if (width > radius * LABEL_WIDTH) {
            textSize *= radius * LABEL_WIDTH / width;
            paint.setTextSize(textSize);
        }
        canvas.drawText(label, x, y + textSize * 0.36f, paint);
    }

    /** a toolbar item: never fainter than half, so that it can always be found */
    private void toolbarItem(Canvas canvas, float x, float y, float radius, String label, boolean pressed) {
        circle(canvas, x, y, toolbarRadius(radius), label, pressed, 11, Math.max(layout.opacity, 0.5f));
    }

    private void drawToolbar(Canvas canvas) {
        toolbarItem(canvas, optionsX(), optionsY(), OPTIONS_RADIUS, "Options", dragControl == TOOLBAR_OPTIONS);
        if (editing) {
            float middle = logicalWidth / 2;
            toolbarItem(canvas, middle - EDITOR_SPACING, editorY(), OPTIONS_RADIUS, "Export", false);
            toolbarItem(canvas, middle, editorY(), OPTIONS_RADIUS, "Import", false);
            toolbarItem(canvas, middle + EDITOR_SPACING, editorY(), OPTIONS_RADIUS, "Save", false);
            paint.setColor(Color.WHITE);
            paint.setTextSize(14);
            canvas.drawText("Drag the controls and Options, then push Save", middle,
                editorY() + toolbarRadius(OPTIONS_RADIUS) + 24, paint);
        } else {
            toolbarItem(canvas, logicalWidth / 2, toolbarY(), TOOLBAR_RADIUS, hidden ? "Touch" : "Hide", false);
        }
    }

    private void drawStick(Canvas canvas) {
        int stick = TouchLayout.LEFT;
        float radius = layout.radius(stick);
        float size = layout.sizeScale(stick);
        // (a floating stick near an edge is drawn whole, nearer the middle than its centre)
        float x = Math.max(radius, Math.min(logicalWidth - radius, stickCenterX()));
        float y = Math.max(radius, Math.min(logicalHeight - radius, stickCenterY()));
        circle(canvas, x, y, radius, "Move", false, 11 * size, layout.opacity);
        circle(canvas, x + axes[0] / 32767f * radius, y + axes[1] / 32767f * radius,
            Math.max(STICK_KNOB_RADIUS * size, radius * 0.375f), "", held(stick), 11, layout.opacity);
    }

    @Override
    protected void onDraw(Canvas canvas) {
        if (!showing && !editing)
            return;
        canvas.save();
        canvas.scale(scale, scale);
        if (!hidden || editing) {
            if (layout.shown(TouchLayout.LEFT))
                drawStick(canvas);
            for (int i = 0; i < layout.size(); i++) {
                if (!layout.shown(i) || layout.type(i) == TouchLayout.LEFT)
                    continue;
                float textSize = 11 * Math.max(layout.sizeScale(i), layout.radius(i) / 36f);
                circle(canvas, layout.x(i), layout.y(i), layout.radius(i), label(layout.type(i)),
                    held(i) || dragControl == i, Math.min(textSize, 16), layout.opacity);
            }
        }
        drawToolbar(canvas);
        canvas.restore();
    }
}
