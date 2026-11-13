package com.halo.decomp;

/** Standalone geometry/persistence regression checks; no phone required. */
public final class TouchLayoutTest {
    private static void check(boolean condition, String message) {
        if (!condition) throw new AssertionError(message);
    }
    public static void main(String[] args) {
        TouchLayout layout = new TouchLayout();
        check(layout.size() == 18, "All 17 buttons and the movement stick must be editable");
        check(layout.x(TouchLayout.LEFT) == 115 && layout.y(TouchLayout.FIRE_LEFT) == 239,
              "Existing layouts must start with the original controls");
        layout.move(TouchLayout.LEFT, -500, -500);
        check(layout.x(TouchLayout.LEFT) >= 64 && layout.y(TouchLayout.LEFT) == 64,
              "Dragging must keep the full stick inside the screen without a toolbar exclusion");
        layout.move(TouchLayout.FIRE_LEFT, 10000, 10000);
        check(layout.x(TouchLayout.FIRE_LEFT)+39 <= 960 && layout.y(TouchLayout.FIRE_LEFT)+39 <= 540,
              "Dragging beyond the bottom/right edge must keep the stick reachable");
        layout.move(0, 230, 320);
        layout.move(5, 440, 390);
        TouchLayout reopened = new TouchLayout();
        for (int i = 0; i < layout.size(); i++) reopened.restore(i, layout.x(i), layout.y(i));
        for (int i = 0; i < layout.size(); i++) {
            check(layout.x(i) == reopened.x(i) && layout.y(i) == reopened.y(i),
                  "Saved coordinates must round-trip for control "+i);
        }
        float before = reopened.x(0);
        reopened.restore(0, Float.NaN, 200);
        reopened.restore(0, Float.POSITIVE_INFINITY, 200);
        reopened.restore(0, -20, 200);
        check(reopened.x(0) == before, "Invalid saved coordinates must not corrupt the layout");
        reopened.move(0, Float.NaN, 200);
        check(reopened.x(0) == before, "Invalid drag coordinates must not corrupt the layout");
        // A saved top-row Pause button is valid even before it has been moved.
        check(reopened.y(10) == 36, "Reopening must preserve unmoved top-row controls");
        reopened.bounds(1200, 540);
        reopened.move(0, 9999, 1);
        check(reopened.x(0) == 1164 && reopened.y(0) == 36,
              "Controls must reach the full widescreen edge and top edge");
        TouchLayout wide = new TouchLayout();
        wide.restore(0, reopened.savedX(0), reopened.savedY(0));
        wide.bounds(1200, 540);
        check(wide.x(0) == reopened.x(0), "Widescreen positions must survive reload");
        // Duplicates retain their action and are independently placed and hidden.
        int copy = wide.duplicate(4);
        check(copy == 18 && wide.type(copy) == 4, "Fire copies must retain the fire action");
        wide.setShown(4, false);
        wide.move(copy, 300, 100);
        check(!wide.shown(4) && wide.shown(copy), "Hiding an original must not hide its copy");
        check(wide.duplicate(TouchLayout.LEFT) == -1, "There can only be one movement stick");
        String exported = wide.exportConfiguration(2.25f);
        TouchLayout.Configuration imported = TouchLayout.importConfiguration(exported);
        imported.layout.bounds(1200, 540);
        check(imported.sensitivity == 2.25f && imported.layout.size() == 19,
              "Configuration must preserve sensitivity and duplicates");
        for (int i = 0; i < wide.size(); i++) {
            check(imported.layout.type(i) == wide.type(i) && imported.layout.shown(i) == wide.shown(i)
                && Math.abs(imported.layout.x(i)-wide.x(i)) < 0.001f
                && Math.abs(imported.layout.y(i)-wide.y(i)) < 0.001f,
                "Export/import must preserve each control on widescreen");
        }
        check(wide.add(4) == 4 && wide.shown(4), "Add must restore a hidden button first");
        wide.setShown(TouchLayout.LEFT, false);
        check(wide.add(TouchLayout.LEFT) == TouchLayout.LEFT, "Hidden move stick must be restorable");
        reject(exported.replace("version=4", "version=9"));
        reject(exported.replace("control.18.type=4", "control.18.type=16"));
        reject(exported.replace("control.18.x=240.0", "control.18.x=NaN"));
        reject(exported.replace("count=19", "count=10000"));
        reject(exported.replace("sensitivity=2.25", "sensitivity=Infinity"));
        reject(exported.replace("control.0.visible=false", "control.0.visible=maybe")
            .replace("control.0.visible=true", "control.0.visible=maybe"));
        reject("not a layout");
        wide.setSize(copy, 1.6f);
        wide.rumbleEnabled = false; wide.gyroscopeEnabled = true;
        TouchLayout.Configuration sized = TouchLayout.importConfiguration(wide.exportConfiguration(2.25f));
        check(sized.layout.sizeScale(copy) == 1.6f && !sized.layout.rumbleEnabled && sized.layout.gyroscopeEnabled,
              "Sizes, rumble and gyro settings must survive export/import");
        int sizedCopy = wide.duplicate(copy);
        check(wide.sizeScale(sizedCopy) == 1.6f, "Duplicates inherit their source size");
        wide.move(copy, 99999, 99999);
        wide.setSize(copy, 2f);
        check(wide.x(copy)+wide.radius(copy) <= 1200 && wide.y(copy)+wide.radius(copy) <= 540,
              "Growing a button at the edge must keep it reachable");
        String sizedText = sized.layout.exportConfiguration(2.25f);
        reject(sizedText.replace("control.18.size=1.6", "control.18.size=NaN"));
        reject(sizedText.replace("control.18.size=1.6", "control.18.size=3.0"));
        reject(sizedText.replace("rumble=false", "rumble=invalid"));
        reject(sizedText.replace("gyroscope=true", "gyroscope=invalid"));
        String legacy = sizedText.replace("version=4", "version=1").replaceAll("(?m)^.*\\.size=.*\\R", "")
            .replaceAll("(?m)^(rumble|gyroscope|opacity|floating-stick|buttons-aim|options\\.x|options\\.y)=.*\\R", "");
        TouchLayout.Configuration old = TouchLayout.importConfiguration(legacy);
        check(old.layout.sizeScale(copy) == 1 && old.layout.rumbleEnabled && !old.layout.gyroscopeEnabled,
              "Legacy layouts must load with default sizes and gyro disabled");
        // Opacity (version 3) round-trips; version 2 files load fully opaque.
        sized.layout.setOpacity(0.45f);
        String translucent = sized.layout.exportConfiguration(2.25f);
        check(TouchLayout.importConfiguration(translucent).layout.opacity == 0.45f, "Opacity must survive export/import");
        sized.layout.setOpacity(0.05f);
        check(TouchLayout.importConfiguration(sized.layout.exportConfiguration(2.25f)).layout.opacity == 0.05f,
              "Minimum opacity must survive export/import");
        reject(translucent.replace("opacity=0.45", "opacity=0.04"));
        reject(translucent.replace("opacity=0.45", "opacity=NaN"));
        sized.layout.floatingStick = true;
        check(TouchLayout.importConfiguration(sized.layout.exportConfiguration(2.25f)).layout.floatingStick,
              "The floating stick must survive export/import");
        sized.layout.floatingStick = false;
        reject(translucent.replace("floating-stick=false", "floating-stick=maybe"));
        String version2 = translucent.replace("version=4", "version=2").replaceAll("(?m)^(opacity|floating-stick|buttons-aim)=.*\\R", "");
        check(TouchLayout.importConfiguration(version2).layout.opacity == 1f
              && !TouchLayout.importConfiguration(version2).layout.floatingStick,
              "Version 2 layouts load fully opaque, with the stick in its place");
        reject(translucent.replaceAll("(?m)^opacity=.*\\R", ""));
        // Held buttons aim (version 4) unless turned off.
        check(TouchLayout.importConfiguration(translucent).layout.buttonsAim, "Held buttons aim by default");
        sized.layout.buttonsAim = false;
        String buttonsStill = sized.layout.exportConfiguration(2.25f);
        sized.layout.buttonsAim = true;
        check(!TouchLayout.importConfiguration(buttonsStill).layout.buttonsAim,
              "Held buttons that do not aim must survive export/import");
        reject(buttonsStill.replace("buttons-aim=false", "buttons-aim=maybe"));
        reject(buttonsStill.replaceAll("(?m)^buttons-aim=.*\\R", ""));
        // Options stays at the top right until dragged; a dragged place round-trips and scales.
        check(!TouchLayout.importConfiguration(translucent).layout.optionsMoved(), "Options starts in its place");
        sized.layout.moveOptions(300, 200);
        String movedOptions = sized.layout.exportConfiguration(2.25f);
        TouchLayout.Configuration placed = TouchLayout.importConfiguration(movedOptions);
        placed.layout.bounds(1200, 540);
        check(placed.layout.optionsMoved() && Math.abs(placed.layout.optionsX() - 300 * 1200 / 960f) < 0.01f
              && Math.abs(placed.layout.optionsY() - 200) < 0.01f,
              "A dragged Options must survive export/import");
        sized.layout.moveOptions(99999, -99999);
        check(sized.layout.optionsX() + sized.layout.optionsRadius() <= 960
              && sized.layout.optionsY() - sized.layout.optionsRadius() >= 0, "Options must stay on the display");
        reject(movedOptions.replaceAll("(?m)^options\\.y=.*\\R", ""));
        reject(movedOptions.replaceAll("(?m)^options\\.x=.*", "options.x=NaN"));
        reject(movedOptions.replaceAll("(?m)^options\\.x=.*", "options.x=5000"));
        sized.layout.resetDefaults();
        check(!sized.layout.optionsMoved(), "Reset puts Options back in its place");
        String version3 = buttonsStill.replace("version=4", "version=3").replaceAll("(?m)^buttons-aim=.*\\R", "");
        check(TouchLayout.importConfiguration(version3).layout.buttonsAim, "Version 3 layouts' held buttons aim");
        try {
            sized.layout.setOpacity(1.5f);
            throw new AssertionError("Out-of-range opacity was accepted");
        } catch (IllegalArgumentException expected) {
            // refused
        }
        // A finger-sized minimum radius enlarges small buttons and keeps them on the display.
        TouchLayout fingers = new TouchLayout();
        fingers.setMinimumRadius(30);
        check(fingers.radius(12) == 30 && fingers.radius(TouchLayout.LEFT) == 64,
              "The minimum radius enlarges small controls only");
        fingers.move(12, 0, 0);
        check(fingers.x(12) == 30 && fingers.y(12) == 30, "Enlarged controls stay inside the display");
        check(fingers.savedX(12) == 30, "The minimum radius does not change saved places");
        fingers.setMinimumRadius(80);
        check(fingers.radius(12) == TouchLayout.MAX_MINIMUM_RADIUS, "The minimum radius is capped");
        TouchLayout dense = new TouchLayout();
        dense.setMinimumRadius(80);
        for (int a = 12; a <= 15; a++) {
            for (int b = a + 1; b <= 15; b++) {
                check(Math.hypot(dense.x(a) - dense.x(b), dense.y(a) - dense.y(b)) >= dense.radius(a) + dense.radius(b),
                      "At the capped minimum radius the D-pad's buttons stay apart");
            }
        }
        fingers.setMinimumRadius(Float.NaN);
        check(fingers.radius(12) == 25, "An invalid minimum radius is none");
        wide.resetDefaults();
        check(wide.size() == 18 && wide.shown(4) && wide.x(4) == 915*1200f/960,
              "Reset restores defaults on the current display and removes all copies");
        check(wide.sizeScale(4) == 1f && !wide.rumbleEnabled && wide.gyroscopeEnabled,
              "Button reset must restore sizes without changing General settings");
        while (wide.duplicate(4) >= 0) {}
        check(wide.size() == TouchLayout.MAX_CONTROLS, "Duplicate count must be bounded");
        System.out.println("Touch layout, visibility, duplication and import/export checks passed");
    }

    private static void reject(String text) {
        try { TouchLayout.importConfiguration(text); }
        catch (IllegalArgumentException expected) { return; }
        throw new AssertionError("Invalid configuration was accepted");
    }
}
