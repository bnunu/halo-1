/*
TOUCH.JS

The on-screen touch controls over the game, as the Android app has them
(port/android/app/.../TouchControls.java, TouchLayout.java): a stick on the
left, the controller's buttons, and the rest of the screen a swipe that
turns the view. They show in a game (not in the menus, nor during a
cinematic), as input.touch_controls says: "auto" on a touchscreen while no
controller is connected, "on", "off".

They write the controller state into the game's shared memory, which the
game reads with port 0's controller (port/web/src/web_touch.c, web_shared.h),
and read back from it when to show, the buttons' names (the profile's
mapping) and the rumble. Places are on Android's 960 x 540 grid, stretched
to the screen; sizes keep their shape.
*/

'use strict';

window.HaloTouch = (() => {
  // web_shared.h's word indices
  const SCENE = 1;
  const RUMBLE = 2;
  const BINDINGS_SERIAL = 3;
  const BINDINGS = 4;
  const STATE = 20;
  const PRESSES = 27;
  const LOOK = 44;
  // touch_input.c's _touch_scene_* bits
  const SCENE_KNOWN = 1;
  const SCENE_MENUS = 2;
  const SCENE_ON = 4;
  const SCENE_OFF = 8;

  const GRID_WIDTH = 960;
  const GRID_HEIGHT = 540;
  const STICK_DEAD_ZONE = 0.12;
  const STICK_REACH = 1.28;
  const FLOATING_ZONE_WIDTH = 0.45;
  const FLOATING_ZONE_TOP = 0.35;
  const VIBRATION_MS = 110;
  const VIBRATION_RENEW_MS = 70;

  // each control: its SDL button bit or trigger (axis 4 left, 5 right), the
  // game's controller button that names it (input.h), its label, its place on
  // the grid and its radius (TouchLayout.java's DEFAULTS and RADII)
  const CONTROLS = [
    { bit: 0, button: 0, name: 'A', x: 856, y: 447, radius: 36 },
    { bit: 1, button: 1, name: 'B', x: 913, y: 377, radius: 32 },
    { bit: 2, button: 2, name: 'X', x: 794, y: 377, radius: 34 },
    { bit: 3, button: 3, name: 'Y', x: 850, y: 312, radius: 32 },
    { trigger: 5, button: 7, name: 'RT', x: 915, y: 239, radius: 39 },
    { trigger: 4, button: 6, name: 'LT', x: 802, y: 239, radius: 35 },
    { bit: 7, button: 14, name: 'LS', x: 236, y: 449, radius: 32 },
    { bit: 8, button: 15, name: 'RS', x: 691, y: 449, radius: 32 },
    { bit: 9, button: 5, name: 'White', x: 360, y: 490, radius: 27 },
    { bit: 10, button: 4, name: 'Black', x: 438, y: 490, radius: 29 },
    { bit: 6, button: 12, name: 'Start', x: 570, y: 36, radius: 28 },
    { bit: 4, button: 13, name: 'Back', x: 390, y: 36, radius: 28 },
    { bit: 11, button: 8, name: 'Up', x: 100, y: 237, radius: 25 },
    { bit: 12, button: 9, name: 'Down', x: 100, y: 335, radius: 25 },
    { bit: 13, button: 10, name: 'Left', x: 51, y: 286, radius: 25 },
    { bit: 14, button: 11, name: 'Right', x: 149, y: 286, radius: 25 },
    { stick: true, name: 'Move', x: 115, y: 440, radius: 64 },
    { trigger: 5, button: 7, name: 'RT', x: 255, y: 239, radius: 39 },
  ];
  // the game's controls (input_abstraction.c's _game_control_*), as the
  // buttons are named by them
  const GAME_CONTROLS = ['Jump', 'Gren. type', 'Reload', 'Weapon', 'Melee', 'Light', 'Grenade', 'Fire', 'Pause',
    'Back', 'Crouch', 'Zoom'];

  let words = null;
  let base = 0;
  let overlay = null;
  let stickKnob = null;
  let stickRing = null;
  let shown = false;
  let bindingsSerial = -1;
  let lastVibration = 0;
  const fingers = new Map();
  const held = new Map();
  let stick = null;

  const read = (index) => Atomics.load(words, base + index);

  function setting() {
    const scene = read(SCENE);
    if (!(scene & SCENE_KNOWN) || (scene & SCENE_MENUS) || (scene & SCENE_OFF)) return false;
    if (scene & SCENE_ON) return true;
    const touchscreen = navigator.maxTouchPoints > 0 && matchMedia('(any-pointer: coarse)').matches;
    const pads = navigator.getGamepads ? [...navigator.getGamepads()].some((pad) => pad && pad.connected) : false;
    return touchscreen && !pads;
  }

  // pixels per grid unit across, down, and for sizes
  function scales() {
    const width = overlay.clientWidth;
    const height = overlay.clientHeight;
    return { x: width / GRID_WIDTH, y: height / GRID_HEIGHT, size: Math.min(width / GRID_WIDTH, height / GRID_HEIGHT) };
  }

  function layout() {
    const scale = scales();
    for (const control of CONTROLS) {
      const radius = control.radius * scale.size;
      Object.assign(control.element.style, {
        left: `${control.x * scale.x - radius}px`,
        top: `${control.y * scale.y - radius}px`,
        width: `${radius * 2}px`,
        height: `${radius * 2}px`,
        fontSize: `${Math.max(9, radius * 0.42)}px`,
      });
    }
  }

  function labels() {
    const serial = Atomics.load(words, base + BINDINGS_SERIAL);
    if (serial === bindingsSerial) return;
    bindingsSerial = serial;
    for (const control of CONTROLS) {
      if (control.stick) continue;
      const action = serial ? read(BINDINGS + control.button) : -1;
      control.element.textContent = GAME_CONTROLS[action] || control.name;
    }
  }

  // the state the game reads: the stick, the buttons held, the triggers
  function write() {
    let buttons = 0;
    let left = 0;
    let right = 0;
    for (const control of held.values()) {
      if (control.bit !== undefined) buttons |= 1 << control.bit;
      if (control.trigger === 4) left = 32767;
      if (control.trigger === 5) right = 32767;
    }
    let x = 0;
    let y = 0;
    if (stick) {
      const scale = scales();
      const reach = CONTROLS[16].radius * scale.size * STICK_REACH;
      x = (stick.x - stick.centreX) / reach;
      y = (stick.y - stick.centreY) / reach;
      const length = Math.hypot(x, y);
      if (length < STICK_DEAD_ZONE) {
        x = y = 0;
      } else if (length > 1) {
        x /= length;
        y /= length;
      }
      const knob = CONTROLS[16].radius * scale.size * 0.45;
      Object.assign(stickKnob.style, { left: `${stick.x - knob}px`, top: `${stick.y - knob}px`,
        width: `${knob * 2}px`, height: `${knob * 2}px`, display: 'block' });
      const ring = CONTROLS[16].radius * scale.size;
      Object.assign(stickRing.style, { left: `${stick.centreX - ring}px`, top: `${stick.centreY - ring}px`,
        width: `${ring * 2}px`, height: `${ring * 2}px`, display: 'block' });
    } else {
      stickKnob.style.display = 'none';
      stickRing.style.display = 'none';
    }
    const state = [Math.round(x * 32767), Math.round(y * 32767), 0, 0, left, right, buttons];
    state.forEach((value, index) => Atomics.store(words, base + STATE + index, value));
  }

  function press(control) {
    const input = control.bit !== undefined ? control.bit : 15 + (control.trigger === 5 ? 1 : 0);
    Atomics.add(words, base + PRESSES + input, 1);
    control.element.classList.add('held');
  }

  function controlAt(x, y) {
    const scale = scales();
    let best = null;
    let bestDistance = Infinity;
    for (const control of CONTROLS) {
      const distance = Math.hypot(x - control.x * scale.x, y - control.y * scale.y);
      if (distance <= control.radius * scale.size * 1.15 && distance < bestDistance) {
        best = control;
        bestDistance = distance;
      }
    }
    return best;
  }

  function down(event) {
    const rect = overlay.getBoundingClientRect();
    const x = event.clientX - rect.left;
    const y = event.clientY - rect.top;
    const control = controlAt(x, y);
    // (a touch is captured as it is; a mouse may not be while the game holds
    // the pointer locked)
    try {
      overlay.setPointerCapture(event.pointerId);
    } catch {
      // as it is
    }
    event.preventDefault();
    if ((control && control.stick) || (!control && !stick && x < rect.width * FLOATING_ZONE_WIDTH &&
        y > rect.height * FLOATING_ZONE_TOP)) {
      // the stick: where its control is, or floating where the thumb went down
      const scale = scales();
      const fixed = control && control.stick;
      stick = { pointer: event.pointerId, centreX: fixed ? control.x * scale.x : x, centreY: fixed ? control.y * scale.y : y,
        x, y };
      fingers.set(event.pointerId, { kind: 'stick' });
    } else if (control) {
      held.set(event.pointerId, control);
      press(control);
      fingers.set(event.pointerId, { kind: 'button', control, x, y });
    } else {
      fingers.set(event.pointerId, { kind: 'look', x, y });
    }
    write();
  }

  function move(event) {
    const finger = fingers.get(event.pointerId);
    if (!finger) return;
    event.preventDefault();
    const rect = overlay.getBoundingClientRect();
    const x = event.clientX - rect.left;
    const y = event.clientY - rect.top;
    if (finger.kind === 'stick' && stick) {
      stick.x = x;
      stick.y = y;
      write();
    } else if (finger.kind === 'look' || finger.kind === 'button') {
      // (a button dragged turns the view too, as on Android)
      const size = scales().size;
      Atomics.add(words, base + LOOK, Math.round((x - finger.x) / size * 256));
      Atomics.add(words, base + LOOK + 1, Math.round((y - finger.y) / size * 256));
      finger.x = x;
      finger.y = y;
    }
  }

  function up(event) {
    const finger = fingers.get(event.pointerId);
    if (!finger) return;
    fingers.delete(event.pointerId);
    if (finger.kind === 'stick') stick = null;
    if (finger.kind === 'button') {
      held.delete(event.pointerId);
      if (![...held.values()].includes(finger.control)) finger.control.element.classList.remove('held');
    }
    write();
  }

  function release() {
    fingers.clear();
    held.clear();
    stick = null;
    for (const control of CONTROLS) control.element.classList.remove('held');
    write();
  }

  function frame() {
    if (!words) return;
    const want = setting();
    if (want !== shown) {
      shown = want;
      overlay.hidden = !shown;
      if (!shown) release();
      else layout();
    }
    if (shown) {
      // (the screen's fingers aim, not a mouse: a pointer the game locked,
      // a touch laptop's, would freeze every finger's place)
      if (document.pointerLockElement) document.exitPointerLock();
      labels();
      const rumble = read(RUMBLE);
      const now = performance.now();
      if (rumble && navigator.vibrate && now - lastVibration > VIBRATION_RENEW_MS) {
        navigator.vibrate(VIBRATION_MS);
        lastVibration = now;
      }
    }
    requestAnimationFrame(frame);
  }

  // starts the controls over the game: module is the game's (createHalo),
  // element the overlay's
  function start(module, element) {
    if (!module || !module._web_state || !module.HEAPU32) return;
    words = new Int32Array(module.HEAPU32.buffer);
    base = module._web_state() >> 2;
    overlay = element;
    overlay.textContent = '';
    for (const control of CONTROLS) {
      control.element = document.createElement('div');
      control.element.className = control.stick ? 'touch-control touch-stick-zone' : 'touch-control';
      control.element.textContent = control.stick ? '' : control.name;
      overlay.append(control.element);
    }
    stickRing = document.createElement('div');
    stickRing.className = 'touch-stick-ring';
    stickKnob = document.createElement('div');
    stickKnob.className = 'touch-stick-knob';
    overlay.append(stickRing, stickKnob);
    overlay.addEventListener('pointerdown', down);
    overlay.addEventListener('pointermove', move);
    overlay.addEventListener('pointerup', up);
    overlay.addEventListener('pointercancel', up);
    overlay.addEventListener('contextmenu', (event) => event.preventDefault());
    // (the overlay's fingers are not the mouse's, which the game would take
    // as aiming and firing: with "on" on a computer, say)
    for (const type of ['mousedown', 'mousemove', 'mouseup', 'wheel']) {
      overlay.addEventListener(type, (event) => event.stopPropagation());
    }
    addEventListener('resize', () => shown && layout());
    addEventListener('blur', release);
    requestAnimationFrame(frame);
  }

  return { start };
})();
