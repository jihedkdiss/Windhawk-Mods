// ==WindhawkMod==
// @id              taskbar-media-beacon
// @name            Taskbar Media Beacon
// @description     A theme-driven, fully customizable now-playing widget for the taskbar.
// @version         1.0.0
// @author          0xjio
// @github          https://github.com/jihedkdiss
// @twitter         https://twitter.com/0xjio_
// @homepage        https://jihedkdiss.netlify.app/
// @include         explorer.exe
// @compilerOptions -lole32 -ldwmapi -lgdi32 -luser32 -lwindowsapp -lshcore -lgdiplus -lshell32 -lcomctl32
// ==/WindhawkMod==

// ==WindhawkModReadme==
/*
# Taskbar Media Beacon

![Taskbar Media Beacon](https://raw.githubusercontent.com/jihedkdiss/Windhawk-Mods/refs/heads/main/taskbar-media-beacon.png)

A theme-driven now-playing widget for the Windows 11 taskbar, built on
Global System Media Transport Controls (GSMTC) and rendered as a layered window.

This mod is intended to be tuned. Presets provide sane defaults, while every
major visual and interaction layer can be overridden without forking code.

## Rendering and Data Pipeline

1. Poll GSMTC sessions and choose the best candidate with metadata.
2. Pull media metadata, playback controls, timeline, and thumbnail.
3. Resolve the active preset plus user overrides into final runtime settings.
4. Compute component layout (art, text, controls, seekbar) based on `ComponentOrder`.
5. Draw with GDI+ to a 32-bit DIB and present via `UpdateLayeredWindow`.
6. Reposition against the taskbar window and react to taskbar location changes.

## What Is Actually Theme-Driven

Every preset defines:

* Geometry: width, height, corner radius, text scale, art radius.
* Composition: component order, alignment, compact mode, one-line vs two-line text.
* Interactions: controls visibility, seekbar style, scroll behavior.
* Surface style: acrylic/solid/none, border, accent stripe, accent band.
* Color palette pair: dark and light variants for Auto Theme switching.

Overrides are applied on top of the selected preset, then clamped to valid ranges.

## Override Semantics

* Use `Theme` to select a baseline.
* Numeric `-1` means "inherit from theme" for size/typography/radius fields.
* Enum `theme` means "inherit from theme" for enum/toggle fields.
* Toggle fields accept `on`, `off`, or `theme`.
* `BackgroundOpacity` only replaces alpha of the resolved background color.
* Final hard limits:
  * `PanelWidth >= 180`
  * `PanelHeight >= 30`
  * `FontSize >= 8`
  * `FontWeight` is clamped to Regular/Semibold/Bold
  * `CornerRadius` is clamped to half of the smaller panel dimension

## Colors and Parsing Rules

Color overrides support two formats:

* Comma: `R,G,B,A`
  * `A` can be `0..1` (fractional alpha) or `0..255` (byte alpha).
* Hex: `0xRRGGBB`, `0xAARRGGBB`, `#RRGGBB`, `#AARRGGBB`

Examples:

* `TextColor: 235,235,245,0.92`
* `BackgroundColor: 0xD91A1A22`
* `AccentColor: #FF1ED760`

## Seekbar Behavior

`SeekbarStyle` controls drawing mode:

* `line`: track + draggable knob inside the text area.
* `fill`: progress sweep blended into the panel background.

Playback progress is computed from timeline ticks:

`ratio = clamp((position - start) / (end - start), 0, 1)`

When playback is active, timeline position is extrapolated from last timeline
update timestamp to keep the animation smooth between polls.

## Visibility Rules

The panel is hidden if any of these conditions apply:

* No active media and `HideWhenNoMedia = true`
* Fullscreen notification state blocks overlays and `HideFullscreen = true`
* Playback is paused longer than `IdleTimeout` seconds (`IdleTimeout > 0`)

## Presets

### Default
Balanced showcase: solid surface, border, album art, controls, two-line text,
and line seekbar. Good baseline for incremental tuning.

### Pulse
Minimal overlay: transparent panel, two-line text, accent stripe, no controls,
no panel fill. Good for "text-first" desktops.

### Capsule
Compact capsule: tight height, inline controls, single-line scrolling text,
and fill-style progress sweep.

### Glass
Acrylic-forward style: rounded surface, album art, controls, line seekbar,
native Windows 11 visual tone.

### Ember
Accent-driven acrylic panel with warm banding, stronger highlight color, and
high-contrast content grouping.

## Advanced Tuning Recipes

### 1) Ultra-compact command strip

* `Theme: capsule`
* `PanelHeight: 32`
* `CompactMode: on`
* `ShowAlbumArt: off`
* `TwoLineText: off`
* `ComponentOrder: controls,text,art`

Result: small transport-first widget with minimal visual overhead.

### 2) Ambient overlay (no panel body)

* `Theme: pulse`
* `BackgroundStyle: none`
* `ShowBorder: off`
* `AccentStripe: on`
* `ShowControls: off`
* `ShowSeekbar: off`

Result: low-noise title/artist layer that floats over the taskbar.

### 3) Glass + explicit alpha tuning

* `Theme: glass`
* `BackgroundColor: 0xFF202226`
* `BackgroundOpacity: 170`
* `ShowBorder: on`
* `BorderColor: 0x50FFFFFF`

Result: acrylic-like depth with deterministic opacity and edge definition.

### 4) Playback scrub focused profile

* `ShowSeekbar: on`
* `EnableSeekbarInteraction: true`
* `SeekbarStyle: line`
* `ShowControls: on`

Result: easier pointer targeting and direct timeline control.

## Extending Beyond Settings (Code-Level)

If you want deeper customization than settings expose:

* Add/modify preset entries in `g_themePresets` for new house styles.
* Adjust layout constants in `ComputeLayout` (margins, gaps, button sizing).
* Refine draw primitives in `DrawThemeBackground` and `DrawMediaPanel`.
* Tune poll/animation cadence via `kPollIntervalMs` and `kAnimationFrameMs`.

### Source Organization

The source is organized into clearly labeled sections:

1. **Utility** — string helpers, color parsing, clamping, toggle resolution.
2. **Settings** — user override loading, theme preset merging, hard limits.
3. **Media session** — GSMTC polling, session selection, transport commands.
4. **Drawing primitives** — rounded rects, text rendering, icon drawing, scrolling text.
5. **Panel composition** — background, foreground, layout computation, seekbar.
6. **Window management** — layered window, DWM composition, taskbar positioning.
7. **Window procedure** — message loop, input handling, timer dispatch.
8. **Lifecycle** — thread entry, mod init/uninit, settings reload.

### Key Namespaces and Constants

Domain-specific integers use named constants to avoid magic numbers:

* `Component::{Art, Controls, Text}` — component order identifiers.
* `Weight::{Regular, Semibold, Bold}` — font weight levels.
* `Align::{Left, Center, Right}` — horizontal alignment modes.
* `Seekbar::{Line, Fill}` — seekbar rendering styles.
* `MediaCmd::{Previous, TogglePlayPause, Next}` — transport commands.
* `kMinPanelWidth`, `kMinPanelHeight`, `kMinFontSize` — hard limits.
* `kPollIntervalMs`, `kAnimationFrameMs` — timer intervals.
* `kScrollPauseFrames`, `kScrollGapPx` — scroll animation tuning.
* `kNumControlButtons` — number of transport buttons (Prev, Play/Pause, Next).

## Compatibility Notes

* Target: Windows 11 taskbar behavior.
* Acrylic rendering varies slightly across builds/compositor states.
* For best consistency, keep taskbar Widgets disabled.
*/
// ==/WindhawkModReadme==

// ==WindhawkModSettings==
/*
- Theme: default
  $name: Theme Preset
  $options:
  - default: Default
  - pulse: Pulse
  - capsule: Capsule
  - glass: Glass
  - ember: Ember
- FontFamily: ""
  $name: Font Family Override
  $description: Leave empty to use the selected theme font.
- PanelWidth: -1
  $name: Panel Width Override
  $description: Set -1 to inherit the theme default.
- PanelHeight: -1
  $name: Panel Height Override
  $description: Set -1 to inherit the theme default.
- HorizontalAlignment: theme
  $name: Horizontal Alignment
  $options:
    - theme: Theme Default
    - left: Left
    - center: Center
    - right: Right
- ComponentOrder: theme
  $name: Component Order
  $description: Left-to-right arrangement of components.
  $options:
    - theme: Theme Default
    - art,text,controls: "Art \u2192 Text \u2192 Controls"
    - art,controls,text: "Art \u2192 Controls \u2192 Text"
    - controls,art,text: "Controls \u2192 Art \u2192 Text"
    - controls,text,art: "Controls \u2192 Text \u2192 Art"
    - text,art,controls: "Text \u2192 Art \u2192 Controls"
    - text,controls,art: "Text \u2192 Controls \u2192 Art"
- CompactMode: theme
  $name: Compact Mode
  $options:
    - theme: Theme Default
    - on: On
    - off: Off
- TwoLineText: theme
  $name: Two-Line Text
  $description: Show title and artist on separate lines instead of a single scrolling line.
  $options:
    - theme: Theme Default
    - on: On
    - off: Off
- ScrollLongText: theme
  $name: Scroll Long Text
  $options:
    - theme: Theme Default
    - on: On
    - off: Off
- FontSize: -1
  $name: Font Size Override
  $description: Set -1 to inherit the theme default.
- FontWeight: theme
  $name: Font Weight Override
  $options:
    - theme: Theme Default
    - regular: Regular
    - semibold: Semibold
    - bold: Bold
- CornerRadius: -1
  $name: Corner Radius Override
  $description: Set -1 to inherit the theme default.
- AlbumArtCornerRadius: -1
  $name: Album Art Corner Radius Override
  $description: Set -1 to inherit the theme default. 0 makes the thumbnail square.
- OffsetX: 12
  $name: X Offset
- OffsetY: 0
  $name: Y Offset
- ShowAlbumArt: theme
  $name: Show Album Art
  $options:
    - theme: Theme Default
    - on: On
    - off: Off
- ShowControls: theme
  $name: Show Controls
  $options:
    - theme: Theme Default
    - on: On
    - off: Off
- ShowSeekbar: theme
  $name: Show Seekbar
  $options:
    - theme: Theme Default
    - on: On
    - off: Off
- SeekbarStyle: theme
  $name: Seekbar Style
  $description: Line draws a track with a draggable knob. Fill sweeps the entire background.
  $options:
    - theme: Theme Default
    - line: Line (Track + Knob)
    - fill: Fill (Background Sweep)
- EnableSeekbarInteraction: true
  $name: Enable Seekbar Interaction
- BackgroundStyle: theme
  $name: Background Style
  $options:
    - theme: Theme Default
    - acrylic: Acrylic
    - solid: Solid
    - none: None
- ShowBorder: theme
  $name: Show Border
  $options:
    - theme: Theme Default
    - on: On
    - off: Off
- AccentBand: theme
  $name: Accent Band
  $description: Warm gradient band at the bottom of the panel.
  $options:
    - theme: Theme Default
    - on: On
    - off: Off
- AccentStripe: theme
  $name: Accent Stripe
  $description: Thin colored stripe on the left edge.
  $options:
    - theme: Theme Default
    - on: On
    - off: Off
- AutoTheme: true
  $name: Auto Theme Palette
- BackgroundOpacity: -1
  $name: Background Opacity Override
  $description: Set -1 to keep the alpha from the chosen palette.
- TextColor: ""
  $name: Text Color Override
  $description: Supports R,G,B,A or hex formats like 0xFFFFFF or 0xCCFFFFFF.
- SecondaryTextColor: ""
  $name: Secondary Text Color Override
- BackgroundColor: ""
  $name: Background Color Override
- BorderColor: ""
  $name: Border Color Override
- AccentColor: ""
  $name: Accent Color Override
- ProgressColor: ""
  $name: Progress Color Override
- HoverColor: ""
  $name: Hover Color Override
- HideWhenNoMedia: true
  $name: Hide When No Media
- HideFullscreen: false
  $name: Hide When Fullscreen
- IdleTimeout: 0
  $name: Auto-hide When Paused (Seconds)
  $description: Set 0 to disable.
- MouseWheelVolume: true
  $name: Mouse Wheel Volume
*/
// ==/WindhawkModSettings==

#include <windows.h>
#include <windowsx.h>
#include <shobjidl.h>
#include <shellapi.h>
#include <dwmapi.h>
#include <gdiplus.h>
#include <shcore.h>

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cwctype>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Media.Control.h>
#include <winrt/Windows.Storage.Streams.h>

using namespace Gdiplus;
using namespace std;
using namespace winrt;
using namespace Windows::Media::Control;
using namespace Windows::Storage::Streams;

// Undocumented Windows DWM composition types for acrylic/blur effects.
typedef enum _WINDOWCOMPOSITIONATTRIB { WCA_ACCENT_POLICY = 19 } WINDOWCOMPOSITIONATTRIB;
typedef enum _ACCENT_STATE {
  ACCENT_DISABLED = 0,
  ACCENT_ENABLE_BLURBEHIND = 3,
  ACCENT_ENABLE_ACRYLICBLURBEHIND = 4,
  ACCENT_INVALID_STATE = 5
} ACCENT_STATE;
typedef struct _ACCENT_POLICY {
  ACCENT_STATE AccentState;
  DWORD AccentFlags;
  DWORD GradientColor;
  DWORD AnimationId;
} ACCENT_POLICY;
typedef struct _WINDOWCOMPOSITIONATTRIBDATA {
  WINDOWCOMPOSITIONATTRIB Attribute;
  PVOID Data;
  SIZE_T SizeOfData;
} WINDOWCOMPOSITIONATTRIBDATA;
typedef BOOL(WINAPI* pSetWindowCompositionAttribute)(HWND, WINDOWCOMPOSITIONATTRIBDATA*);

// Undocumented z-band IDs for CreateWindowInBand (places window in taskbar band).
enum ZBID {
  ZBID_DEFAULT = 0,
  ZBID_DESKTOP = 1,
  ZBID_UIACCESS = 2,
  ZBID_IMMERSIVE_IHM = 3,
  ZBID_IMMERSIVE_NOTIFICATION = 4,
  ZBID_IMMERSIVE_APPCHROME = 5,
  ZBID_IMMERSIVE_MOGO = 6,
  ZBID_IMMERSIVE_EDGY = 7,
  ZBID_IMMERSIVE_INACTIVEMOBODY = 8,
  ZBID_IMMERSIVE_INACTIVEDOCK = 9,
  ZBID_IMMERSIVE_ACTIVEMOBODY = 10,
  ZBID_IMMERSIVE_ACTIVEDOCK = 11,
  ZBID_IMMERSIVE_BACKGROUND = 12,
  ZBID_IMMERSIVE_SEARCH = 13,
  ZBID_GENUINE_WINDOWS = 14,
  ZBID_IMMERSIVE_RESTRICTED = 15,
  ZBID_SYSTEM_TOOLS = 16,
  ZBID_LOCK = 17,
  ZBID_ABOVELOCK_UX = 18,
};

// Undocumented API to create a window in a specific z-band (e.g., immersive notification).
typedef HWND(WINAPI* pCreateWindowInBand)(
  DWORD dwExStyle,
  LPCWSTR lpClassName,
  LPCWSTR lpWindowName,
  DWORD dwStyle,
  int x,
  int y,
  int nWidth,
  int nHeight,
  HWND hWndParent,
  HMENU hMenu,
  HINSTANCE hInstance,
  LPVOID lpParam,
  DWORD dwBand
);

enum class BackgroundStyle {
  ThemeDefault,
  Acrylic,
  Solid,
  None
};

// ─── Component identifiers for componentOrder arrays ────────────────────────
namespace Component {
  constexpr int Art      = 0;
  constexpr int Controls = 1;
  constexpr int Text     = 2;
}

// ─── Font weight levels ─────────────────────────────────────────────────────
namespace Weight {
  constexpr int Regular  = 0;
  constexpr int Semibold = 1;
  constexpr int Bold     = 2;
}

// ─── Horizontal alignment modes ─────────────────────────────────────────────
namespace Align {
  constexpr int Left   = 0;
  constexpr int Center = 1;
  constexpr int Right  = 2;
}

// ─── Seekbar rendering modes ────────────────────────────────────────────────
namespace Seekbar {
  constexpr int Line = 0;   // Track with draggable knob
  constexpr int Fill = 1;   // Progress sweep across background
}

// ─── Media transport commands ───────────────────────────────────────────────
namespace MediaCmd {
  constexpr int Previous        = 1;
  constexpr int TogglePlayPause = 2;
  constexpr int Next            = 3;
}

// ─── Layout and animation constants ─────────────────────────────────────────
constexpr int  kMinPanelWidth     = 180;
constexpr int  kMinPanelHeight    = 30;
constexpr int  kMinFontSize       = 8;
constexpr int  kPollIntervalMs    = 1000;    // GSMTC polling cadence
constexpr int  kAnimationFrameMs  = 33;      // ~30 fps redraw timer
constexpr int  kScrollPauseFrames = 60;      // Frames to pause before scroll restarts
constexpr int  kScrollGapPx       = 36;      // Pixel gap between repeated scroll text
constexpr int  kNumControlButtons = 3;       // Prev, Play/Pause, Next

struct ThemePalette {
  DWORD textColor;
  DWORD secondaryTextColor;
  DWORD backgroundColor;
  DWORD accentColor;
  DWORD progressColor;
  DWORD hoverColor;
  DWORD borderColor;
};

struct ThemePreset {
  PCWSTR id;
  PCWSTR displayName;
  PCWSTR fontFamily;
  int width;
  int height;
  int fontSize;
  int fontWeight;
  int cornerRadius;
  int horizontalAlignment;
  bool showAlbumArt;
  bool showControls;
  bool showSeekbar;
  int componentOrder[3];   // Component::Art, Controls, Text
  bool twoLineText;
  bool scrollLongText;
  bool compactMode;
  int seekbarStyle;        // Seekbar::Line or Seekbar::Fill
  BackgroundStyle backgroundStyle;
  bool showBorder;
  bool accentBand;
  bool accentStripe;
  int artRadius;           // -1=auto
  ThemePalette darkPalette;
  ThemePalette lightPalette;
};

struct UserSettings {
  wstring themeId;
  wstring fontFamily;
  int width = -1;
  int height = -1;
  int fontSize = -1;
  int fontWeight = -1;
  int cornerRadius = -1;
  int artRadius = -1;
  int horizontalAlignment = -1;
  int componentOrder[3] = {-1, -1, -1};
  bool hasComponentOrder = false;
  int seekbarStyle = -1;
  int offsetX = 12;
  int offsetY = 0;
  bool autoTheme = true;
  bool hideFullscreen = false;
  int idleTimeout = 0;
  bool hideWhenNoMedia = true;
  bool mouseWheelVolume = true;
  bool enableSeekbarInteraction = true;
  int backgroundOpacity = -1;
  int showAlbumArt = -1;
  int showControls = -1;
  int showSeekbar = -1;
  int scrollLongText = -1;
  int compactMode = -1;
  int twoLineText = -1;
  int showBorder = -1;
  int accentBand = -1;
  int accentStripe = -1;
  BackgroundStyle backgroundStyle = BackgroundStyle::ThemeDefault;
  wstring textColor;
  wstring secondaryTextColor;
  wstring backgroundColor;
  wstring accentColor;
  wstring progressColor;
  wstring hoverColor;
  wstring borderColor;
} g_UserSettings;

struct ResolvedSettings {
  const ThemePreset* preset = nullptr;
  wstring fontFamily;
  int width = 320;
  int height = 52;
  int fontSize = 12;
  int fontWeight = Weight::Regular;
  int cornerRadius = 10;
  int horizontalAlignment = Align::Left;
  int componentOrder[3] = {Component::Art, Component::Text, Component::Controls};
  int seekbarStyle = Seekbar::Line;
  int offsetX = 12;
  int offsetY = 0;
  bool autoTheme = true;
  bool hideFullscreen = false;
  int idleTimeout = 0;
  bool hideWhenNoMedia = true;
  bool mouseWheelVolume = true;
  bool enableSeekbarInteraction = true;
  bool showAlbumArt = true;
  bool showControls = true;
  bool showSeekbar = true;
  bool scrollLongText = true;
  bool compactMode = false;
  bool twoLineText = true;
  BackgroundStyle backgroundStyle = BackgroundStyle::Solid;
  bool showBorder = true;
  bool accentBand = false;
  bool accentStripe = false;
  int artRadius = -1;
  DWORD textColor = 0xFFFFFFFF;
  DWORD secondaryTextColor = 0xB3FFFFFF;
  DWORD backgroundColor = 0xE01E1E2E;
  DWORD accentColor = 0xFF6C8EEF;
  DWORD progressColor = 0xFF6C8EEF;
  DWORD hoverColor = 0x30FFFFFF;
  DWORD borderColor = 0x406C8EEF;
} g_Settings;

struct MediaState {
  wstring title;
  wstring artist;
  wstring sourceAppId;
  bool isPlaying = false;
  bool hasMedia = false;
  bool canSeek = false;
  bool canGoPrevious = false;
  bool canGoNext = false;
  bool canTogglePlayPause = false;
  Windows::Foundation::DateTime lastTimelineUpdate{};
  long long startTicks = 0;
  long long endTicks = 0;
  long long positionTicks = 0;
  long long minSeekTicks = 0;
  long long maxSeekTicks = 0;
  Bitmap* albumArt = nullptr;
  mutex lock;

  MediaState() = default;

  // Shallow-copy all fields except `lock` (non-copyable).
  // `albumArt` is NOT deep-copied — use CopyMediaState() for owned snapshots.
  MediaState(const MediaState& other)
    : title(other.title), artist(other.artist), sourceAppId(other.sourceAppId),
      isPlaying(other.isPlaying), hasMedia(other.hasMedia), canSeek(other.canSeek),
      canGoPrevious(other.canGoPrevious), canGoNext(other.canGoNext),
      canTogglePlayPause(other.canTogglePlayPause),
      lastTimelineUpdate(other.lastTimelineUpdate),
      startTicks(other.startTicks), endTicks(other.endTicks),
      positionTicks(other.positionTicks), minSeekTicks(other.minSeekTicks),
      maxSeekTicks(other.maxSeekTicks), albumArt(other.albumArt) {}

  MediaState& operator=(const MediaState& other) {
    if (this != &other) {
      title = other.title;
      artist = other.artist;
      sourceAppId = other.sourceAppId;
      isPlaying = other.isPlaying;
      hasMedia = other.hasMedia;
      canSeek = other.canSeek;
      canGoPrevious = other.canGoPrevious;
      canGoNext = other.canGoNext;
      canTogglePlayPause = other.canTogglePlayPause;
      lastTimelineUpdate = other.lastTimelineUpdate;
      startTicks = other.startTicks;
      endTicks = other.endTicks;
      positionTicks = other.positionTicks;
      minSeekTicks = other.minSeekTicks;
      maxSeekTicks = other.maxSeekTicks;
      albumArt = other.albumArt;
    }
    return *this;
  }
} g_MediaState;

struct PanelLayout {
  RECT artRect{};
  RECT textRect{};
  RECT subtitleRect{};
  RECT seekbarRect{};
  RECT buttonRects[kNumControlButtons]{};
  bool hasArt = false;
  bool hasSeekbar = false;
  bool hasButtons = false;
};

const ThemePreset g_themePresets[] = {
  // ── Default ───────────────────────────────────────────────────────────
  // Showcase theme: solid background, border, album art, controls,
  // two-line text, line seekbar — every configurable option applied.
  {
    L"default",                            // id
    L"Default",                            // displayName
    L"Segoe UI Variable Display",          // fontFamily
    320,                                   // width
    52,                                    // height
    12,                                    // fontSize
    1,                                     // fontWeight (Semibold)
    10,                                    // cornerRadius
    0,                                     // horizontalAlignment (Left)
    true,                                  // showAlbumArt
    true,                                  // showControls
    true,                                  // showSeekbar
    {Component::Art, Component::Text, Component::Controls},
    true,                                  // twoLineText
    true,                                  // scrollLongText
    false,                                 // compactMode
    Seekbar::Line,                         // seekbarStyle
    BackgroundStyle::Solid,                // backgroundStyle
    true,                                  // showBorder
    false,                                 // accentBand
    false,                                 // accentStripe
    8,                                     // artRadius
    {0xFFFFFFFF, 0xB3FFFFFF, 0xE01E1E2E, 0xFF6C8EEF, 0xFF6C8EEF, 0x30FFFFFF, 0x406C8EEF},
    {0xFF1A1A1A, 0x801A1A1A, 0xE0F0F0F8, 0xFF4A6CD4, 0xFF4A6CD4, 0x18000000, 0x404A6CD4},
  },
  // ── Pulse ─────────────────────────────────────────────────────────────
  // Minimal transparent overlay. Two-line title + artist with a thin
  // accent stripe on the left edge. No controls, no background.
  {
    L"pulse",                              // id
    L"Pulse",                              // displayName
    L"Inter",                              // fontFamily
    290,                                   // width
    46,                                    // height
    12,                                    // fontSize
    2,                                     // fontWeight (Bold)
    0,                                     // cornerRadius
    0,                                     // horizontalAlignment (Left)
    true,                                  // showAlbumArt
    false,                                 // showControls
    false,                                 // showSeekbar
    {Component::Art, Component::Text, Component::Controls},
    true,                                  // twoLineText
    true,                                  // scrollLongText
    false,                                 // compactMode
    Seekbar::Line,                         // seekbarStyle
    BackgroundStyle::None,                 // backgroundStyle
    false,                                 // showBorder
    false,                                 // accentBand
    true,                                  // accentStripe
    6,                                     // artRadius
    {0xFFFFFFFF, 0xAAFFFFFF, 0x00000000, 0xFF5DADE2, 0x22FFFFFF, 0x28FFFFFF, 0x00000000},
    {0xFF1A1A1A, 0x801A1A1A, 0x00000000, 0xFF2471A3, 0x22000000, 0x18000000, 0x00000000},
  },
  // ── Capsule ───────────────────────────────────────────────────────────
  // Compact solid-background capsule. Album art on the left, inline
  // playback controls, scrolling single-line text, and a track progress
  // fill that sweeps the background from left to right.
  {
    L"capsule",                            // id
    L"Capsule",                            // displayName
    L"Segoe UI",                           // fontFamily
    310,                                   // width
    36,                                    // height
    11,                                    // fontSize
    0,                                     // fontWeight (Regular)
    10,                                    // cornerRadius
    1,                                     // horizontalAlignment (Center)
    true,                                  // showAlbumArt
    true,                                  // showControls
    true,                                  // showSeekbar
    {Component::Art, Component::Controls, Component::Text},
    false,                                 // twoLineText
    true,                                  // scrollLongText
    false,                                 // compactMode
    Seekbar::Fill,                         // seekbarStyle
    BackgroundStyle::Solid,                // backgroundStyle
    false,                                 // showBorder
    false,                                 // accentBand
    false,                                 // accentStripe
    -1,                                    // artRadius (auto — follows container)
    {0xFFFFFFFF, 0xB3FFFFFF, 0xDD1E1E1E, 0xFFFFFFFF, 0x1AFFFFFF, 0x30FFFFFF, 0x00000000},
    {0xFF111111, 0x8A111111, 0xDDF5F5F5, 0xFF111111, 0x14000000, 0x18000000, 0x00000000},
  },
  // ── Glass ─────────────────────────────────────────────────────────────
  // Acrylic-glass widget with rounded corners, album art, right-side
  // controls, and an interactive seekbar with a draggable knob.
  // Feels native to Windows 11.
  {
    L"glass",                              // id
    L"Glass",                              // displayName
    L"Segoe UI Variable Display",          // fontFamily
    320,                                   // width
    52,                                    // height
    11,                                    // fontSize
    2,                                     // fontWeight (Bold)
    12,                                    // cornerRadius
    0,                                     // horizontalAlignment (Left)
    true,                                  // showAlbumArt
    true,                                  // showControls
    true,                                  // showSeekbar
    {Component::Art, Component::Text, Component::Controls},
    false,                                 // twoLineText
    true,                                  // scrollLongText
    false,                                 // compactMode
    Seekbar::Line,                         // seekbarStyle
    BackgroundStyle::Acrylic,              // backgroundStyle
    false,                                 // showBorder
    false,                                 // accentBand
    false,                                 // accentStripe
    9,                                     // artRadius
    {0xFFFFFFFF, 0xCCFFFFFF, 0xC8181818, 0xFF1ED760, 0xC0FFFFFF, 0x2BFFFFFF, 0x00000000},
    {0xFF111111, 0x99000000, 0xC8F8F8F8, 0xFF1DB954, 0xB0202020, 0x18000000, 0x00000000},
  },
  // ── Ember ─────────────────────────────────────────────────────────────
  // Bold, accent-driven panel. Album art on the left, controls next to
  // it, two-line text, seekbar, and a warm gradient accent band at the
  // bottom with a subtle accent border.
  {
    L"ember",                              // id
    L"Ember",                              // displayName
    L"Segoe UI Variable Display",          // fontFamily
    340,                                   // width
    52,                                    // height
    12,                                    // fontSize
    1,                                     // fontWeight (Semibold)
    14,                                    // cornerRadius
    0,                                     // horizontalAlignment (Left)
    true,                                  // showAlbumArt
    true,                                  // showControls
    true,                                  // showSeekbar
    {Component::Art, Component::Controls, Component::Text},
    true,                                  // twoLineText
    true,                                  // scrollLongText
    false,                                 // compactMode
    Seekbar::Line,                         // seekbarStyle
    BackgroundStyle::Acrylic,              // backgroundStyle
    false,                                 // showBorder
    true,                                  // accentBand
    false,                                 // accentStripe
    9,                                     // artRadius
    {0xFFFFFFFF, 0xC8FFFFFF, 0xC8101014, 0xFFFF6A3D, 0x80FF6A3D, 0x35FFFFFF, 0x3CFF6A3D},
    {0xFF1A1A1A, 0x801A1A1A, 0xD0FFF3EA, 0xFFFF5722, 0x70FF5722, 0x18000000, 0x30FF5722},
  },
};

HWND g_hMediaWindow = nullptr;
HWINEVENTHOOK g_TaskbarHook = nullptr;
UINT g_TaskbarCreatedMsg = RegisterWindowMessage(L"TaskbarCreated");
GlobalSystemMediaTransportControlsSessionManager g_SessionManager = nullptr;
thread* g_pMediaThread = nullptr;

int g_IdleSecondsCounter = 0;
bool g_IsHiddenByIdle = false;
int g_HoverState = 0;
bool g_IsDraggingSeek = false;
double g_DragSeekRatio = 0.0;
wstring g_ScrollKey;
int g_ScrollOffset = 0;
int g_ScrollWait = kScrollPauseFrames;
int g_TextWidth = 0;
bool g_IsScrolling = false;
PanelLayout g_LastLayout;

#define IDT_POLL_MEDIA 1001
#define IDT_ANIMATION  1002
#define APP_WM_CLOSE   WM_APP
#define APP_WM_REPOSITION (WM_APP + 10)

static inline BYTE GetA(DWORD color) { return (BYTE)((color >> 24) & 0xFF); }
static inline BYTE GetR(DWORD color) { return (BYTE)((color >> 16) & 0xFF); }
static inline BYTE GetG(DWORD color) { return (BYTE)((color >> 8) & 0xFF); }
static inline BYTE GetB(DWORD color) { return (BYTE)(color & 0xFF); }
static inline DWORD MakeArgb(BYTE a, BYTE r, BYTE g, BYTE b) {
  return ((DWORD)a << 24) | ((DWORD)r << 16) | ((DWORD)g << 8) | (DWORD)b;
}

// Convert packed ARGB DWORD to a GDI+ Color.
static inline Color ArgbColor(DWORD argb) {
  return Color(GetA(argb), GetR(argb), GetG(argb), GetB(argb));
}

// Convert packed ARGB DWORD to GDI+ Color, replacing alpha.
static inline Color ArgbColorAlpha(DWORD argb, BYTE alpha) {
  return Color(alpha, GetR(argb), GetG(argb), GetB(argb));
}

////////////////////////////////////////////////////////////////////////////////
// Utility: String, color parsing, clamping, and toggle resolution
////////////////////////////////////////////////////////////////////////////////

wstring ToLowerString(wstring value) {
  transform(value.begin(), value.end(), value.begin(), [](wchar_t ch) {
    return (wchar_t)towlower(ch);
  });
  return value;
}

// Read a Windhawk string setting and return it as a wstring. Frees the internal buffer.
wstring LoadStringSetting(PCWSTR name) {
  PCWSTR value = Wh_GetStringSetting(name);
  if (!value) {
    return L"";
  }

  wstring result = value;
  Wh_FreeStringSetting(value);
  return result;
}

// Check the Windows personalization registry for light/dark system theme.
bool IsSystemLightMode() {
  DWORD value = 0;
  DWORD size = sizeof(value);
  if (RegGetValueW(
      HKEY_CURRENT_USER,
      L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
      L"SystemUsesLightTheme",
      RRF_RT_DWORD,
      nullptr,
      &value,
      &size) == ERROR_SUCCESS) {
    return value != 0;
  }

  return false;
}

// Parse "R,G,B,A" color string. Alpha can be 0..1 (float) or 0..255 (int).
bool ParseCommaColor(PCWSTR value, DWORD* parsedColor) {
  if (!value || !*value || !parsedColor) {
    return false;
  }

  int red = 255;
  int green = 255;
  int blue = 255;
  float alphaValue = 1.0f;

  if (swscanf_s(value, L"%d,%d,%d,%f", &red, &green, &blue, &alphaValue) != 4) {
    return false;
  }

  red = max(0, min(255, red));
  green = max(0, min(255, green));
  blue = max(0, min(255, blue));

  int alpha = 255;
  if (alphaValue <= 1.0f) {
    alphaValue = max(0.0f, min(1.0f, alphaValue));
    alpha = (int)(alphaValue * 255.0f + 0.5f);
  } else {
    alphaValue = max(0.0f, min(255.0f, alphaValue));
    alpha = (int)(alphaValue + 0.5f);
  }

  *parsedColor = MakeArgb((BYTE)alpha, (BYTE)red, (BYTE)green, (BYTE)blue);
  return true;
}

// Parse hex color: 0xRRGGBB, 0xAARRGGBB, #RRGGBB, or #AARRGGBB.
bool ParseHexColor(PCWSTR value, DWORD* parsedColor) {
  if (!value || !*value || !parsedColor) {
    return false;
  }

  wstring normalized = value;
  if (normalized.rfind(L"0x", 0) == 0 || normalized.rfind(L"0X", 0) == 0) {
    normalized = normalized.substr(2);
  } else if (!normalized.empty() && normalized[0] == L'#') {
    normalized = normalized.substr(1);
  }

  if (normalized.empty()) {
    return false;
  }

  wchar_t* end = nullptr;
  unsigned long raw = wcstoul(normalized.c_str(), &end, 16);
  if (!end || *end != L'\0') {
    return false;
  }

  if (normalized.length() <= 6) {
    *parsedColor = 0xFF000000 | (DWORD)raw;
  } else {
    *parsedColor = (DWORD)raw;
  }

  return true;
}

// Try comma then hex format; return fallback on failure.
DWORD ParseColorOverride(const wstring& value, DWORD fallback) {
  if (value.empty()) {
    return fallback;
  }

  DWORD parsed = fallback;
  if (value.find(L',') != wstring::npos) {
    if (ParseCommaColor(value.c_str(), &parsed)) {
      return parsed;
    }
  }

  if (ParseHexColor(value.c_str(), &parsed)) {
    return parsed;
  }

  return fallback;
}

int ClampInt(int value, int minimum, int maximum) {
  return max(minimum, min(maximum, value));
}

long long ClampI64(long long value, long long minimum, long long maximum) {
  if (value < minimum) return minimum;
  if (value > maximum) return maximum;
  return value;
}

double ClampDouble(double value, double minimum, double maximum) {
  if (value < minimum) return minimum;
  if (value > maximum) return maximum;
  return value;
}

// Resolve a tri-state override: -1 = use theme fallback, 0 = false, 1+ = true.
bool ResolveTriState(int value, bool fallback) {
  if (value < 0) {
    return fallback;
  }

  return value != 0;
}

int ParseFontWeight(const wstring& value) {
  wstring normalized = ToLowerString(value);
  if (normalized == L"regular") return Weight::Regular;
  if (normalized == L"semibold") return Weight::Semibold;
  if (normalized == L"bold") return Weight::Bold;
  return -1;
}

int ParseHorizontalAlignment(const wstring& value) {
  wstring normalized = ToLowerString(value);
  if (normalized == L"left") return Align::Left;
  if (normalized == L"center") return Align::Center;
  if (normalized == L"right") return Align::Right;
  return -1;
}

int ParseToggleOverride(const wstring& value) {
  wstring normalized = ToLowerString(value);
  if (normalized == L"on") return 1;
  if (normalized == L"off") return 0;
  return -1;
}

// Parse a comma-separated component order like "art,controls,text".
// Returns false if the string is empty, "theme", or contains invalid/duplicate names.
bool ParseComponentOrder(const wstring& value, int outOrder[3]) {
  wstring normalized = ToLowerString(value);
  if (normalized.empty() || normalized == L"theme") {
    return false;
  }

  int order[3] = {-1, -1, -1};
  int idx = 0;
  size_t start = 0;
  bool used[3] = {false, false, false};

  while (start < normalized.length() && idx < 3) {
    size_t comma = normalized.find(L',', start);
    wstring token;
    if (comma == wstring::npos) {
      token = normalized.substr(start);
      start = normalized.length();
    } else {
      token = normalized.substr(start, comma - start);
      start = comma + 1;
    }

    // Trim whitespace.
    while (!token.empty() && token.front() == L' ') token.erase(token.begin());
    while (!token.empty() && token.back() == L' ') token.pop_back();

    int comp = -1;
    if (token == L"art") comp = Component::Art;
    else if (token == L"controls") comp = Component::Controls;
    else if (token == L"text") comp = Component::Text;

    if (comp < 0 || used[comp]) {
      return false;
    }

    used[comp] = true;
    order[idx++] = comp;
  }

  if (idx != 3 || !used[0] || !used[1] || !used[2]) {
    return false;
  }

  outOrder[0] = order[0];
  outOrder[1] = order[1];
  outOrder[2] = order[2];
  return true;
}

int ParseSeekbarStyle(const wstring& value) {
  wstring normalized = ToLowerString(value);
  if (normalized == L"line") return Seekbar::Line;
  if (normalized == L"fill") return Seekbar::Fill;
  return -1;
}

BackgroundStyle ParseBackgroundStyle(const wstring& value) {
  wstring normalized = ToLowerString(value);
  if (normalized == L"acrylic") return BackgroundStyle::Acrylic;
  if (normalized == L"solid") return BackgroundStyle::Solid;
  if (normalized == L"none") return BackgroundStyle::None;
  return BackgroundStyle::ThemeDefault;
}

////////////////////////////////////////////////////////////////////////////////
// Settings: Parse user overrides, resolve against theme presets, apply limits
////////////////////////////////////////////////////////////////////////////////

const ThemePreset* GetThemePreset(const wstring& requestedId) {
  for (const ThemePreset& preset : g_themePresets) {
    if (requestedId == preset.id) {
      return &preset;
    }
  }

  return &g_themePresets[0];
}

// Load all user settings, merge with the active preset, and apply hard limits.
void LoadSettings() {
  g_UserSettings.themeId = ToLowerString(LoadStringSetting(L"Theme"));
  if (g_UserSettings.themeId.empty()) {
    g_UserSettings.themeId = L"default";
  }

  g_UserSettings.fontFamily = LoadStringSetting(L"FontFamily");
  g_UserSettings.width = Wh_GetIntSetting(L"PanelWidth");
  g_UserSettings.height = Wh_GetIntSetting(L"PanelHeight");
  g_UserSettings.fontSize = Wh_GetIntSetting(L"FontSize");
  g_UserSettings.fontWeight = ParseFontWeight(LoadStringSetting(L"FontWeight"));
  g_UserSettings.cornerRadius = Wh_GetIntSetting(L"CornerRadius");
  g_UserSettings.artRadius = Wh_GetIntSetting(L"AlbumArtCornerRadius");
  g_UserSettings.horizontalAlignment = ParseHorizontalAlignment(LoadStringSetting(L"HorizontalAlignment"));
  g_UserSettings.hasComponentOrder = ParseComponentOrder(LoadStringSetting(L"ComponentOrder"), g_UserSettings.componentOrder);
  g_UserSettings.seekbarStyle = ParseSeekbarStyle(LoadStringSetting(L"SeekbarStyle"));
  g_UserSettings.offsetX = Wh_GetIntSetting(L"OffsetX");
  g_UserSettings.offsetY = Wh_GetIntSetting(L"OffsetY");
  g_UserSettings.autoTheme = Wh_GetIntSetting(L"AutoTheme") != 0;
  g_UserSettings.hideFullscreen = Wh_GetIntSetting(L"HideFullscreen") != 0;
  g_UserSettings.idleTimeout = Wh_GetIntSetting(L"IdleTimeout");
  g_UserSettings.hideWhenNoMedia = Wh_GetIntSetting(L"HideWhenNoMedia") != 0;
  g_UserSettings.mouseWheelVolume = Wh_GetIntSetting(L"MouseWheelVolume") != 0;
  g_UserSettings.enableSeekbarInteraction = Wh_GetIntSetting(L"EnableSeekbarInteraction") != 0;
  g_UserSettings.backgroundOpacity = Wh_GetIntSetting(L"BackgroundOpacity");
  g_UserSettings.showAlbumArt = ParseToggleOverride(LoadStringSetting(L"ShowAlbumArt"));
  g_UserSettings.showControls = ParseToggleOverride(LoadStringSetting(L"ShowControls"));
  g_UserSettings.showSeekbar = ParseToggleOverride(LoadStringSetting(L"ShowSeekbar"));
  g_UserSettings.scrollLongText = ParseToggleOverride(LoadStringSetting(L"ScrollLongText"));
  g_UserSettings.compactMode = ParseToggleOverride(LoadStringSetting(L"CompactMode"));
  g_UserSettings.twoLineText = ParseToggleOverride(LoadStringSetting(L"TwoLineText"));
  g_UserSettings.showBorder = ParseToggleOverride(LoadStringSetting(L"ShowBorder"));
  g_UserSettings.accentBand = ParseToggleOverride(LoadStringSetting(L"AccentBand"));
  g_UserSettings.accentStripe = ParseToggleOverride(LoadStringSetting(L"AccentStripe"));
  g_UserSettings.backgroundStyle = ParseBackgroundStyle(LoadStringSetting(L"BackgroundStyle"));
  g_UserSettings.textColor = LoadStringSetting(L"TextColor");
  g_UserSettings.secondaryTextColor = LoadStringSetting(L"SecondaryTextColor");
  g_UserSettings.backgroundColor = LoadStringSetting(L"BackgroundColor");
  g_UserSettings.accentColor = LoadStringSetting(L"AccentColor");
  g_UserSettings.progressColor = LoadStringSetting(L"ProgressColor");
  g_UserSettings.hoverColor = LoadStringSetting(L"HoverColor");
  g_UserSettings.borderColor = LoadStringSetting(L"BorderColor");

  const ThemePreset* preset = GetThemePreset(g_UserSettings.themeId);
  const ThemePalette& palette = g_UserSettings.autoTheme && IsSystemLightMode()
    ? preset->lightPalette
    : preset->darkPalette;

  g_Settings.preset = preset;
  g_Settings.fontFamily = g_UserSettings.fontFamily.empty() ? preset->fontFamily : g_UserSettings.fontFamily;
  g_Settings.width = g_UserSettings.width > 0 ? g_UserSettings.width : preset->width;
  g_Settings.height = g_UserSettings.height > 0 ? g_UserSettings.height : preset->height;
  g_Settings.fontSize = g_UserSettings.fontSize > 0 ? g_UserSettings.fontSize : preset->fontSize;
  g_Settings.fontWeight = g_UserSettings.fontWeight >= 0 ? g_UserSettings.fontWeight : preset->fontWeight;
  g_Settings.cornerRadius = g_UserSettings.cornerRadius >= 0 ? g_UserSettings.cornerRadius : preset->cornerRadius;
  g_Settings.horizontalAlignment = g_UserSettings.horizontalAlignment >= 0
    ? g_UserSettings.horizontalAlignment
    : preset->horizontalAlignment;
  g_Settings.horizontalAlignment = ClampInt(g_Settings.horizontalAlignment, Align::Left, Align::Right);
  if (g_UserSettings.hasComponentOrder) {
    g_Settings.componentOrder[0] = g_UserSettings.componentOrder[0];
    g_Settings.componentOrder[1] = g_UserSettings.componentOrder[1];
    g_Settings.componentOrder[2] = g_UserSettings.componentOrder[2];
  } else {
    g_Settings.componentOrder[0] = preset->componentOrder[0];
    g_Settings.componentOrder[1] = preset->componentOrder[1];
    g_Settings.componentOrder[2] = preset->componentOrder[2];
  }
  g_Settings.seekbarStyle = g_UserSettings.seekbarStyle >= 0 ? g_UserSettings.seekbarStyle : preset->seekbarStyle;
  g_Settings.offsetX = g_UserSettings.offsetX;
  g_Settings.offsetY = g_UserSettings.offsetY;
  g_Settings.autoTheme = g_UserSettings.autoTheme;
  g_Settings.hideFullscreen = g_UserSettings.hideFullscreen;
  g_Settings.idleTimeout = max(0, g_UserSettings.idleTimeout);
  g_Settings.hideWhenNoMedia = g_UserSettings.hideWhenNoMedia;
  g_Settings.mouseWheelVolume = g_UserSettings.mouseWheelVolume;
  g_Settings.enableSeekbarInteraction = g_UserSettings.enableSeekbarInteraction;
  g_Settings.showAlbumArt = ResolveTriState(g_UserSettings.showAlbumArt, preset->showAlbumArt);
  g_Settings.showControls = ResolveTriState(g_UserSettings.showControls, preset->showControls);
  g_Settings.showSeekbar = ResolveTriState(g_UserSettings.showSeekbar, preset->showSeekbar);
  g_Settings.scrollLongText = ResolveTriState(g_UserSettings.scrollLongText, preset->scrollLongText);
  g_Settings.compactMode = ResolveTriState(g_UserSettings.compactMode, preset->compactMode);
  g_Settings.twoLineText = ResolveTriState(g_UserSettings.twoLineText, preset->twoLineText);
  g_Settings.showBorder = ResolveTriState(g_UserSettings.showBorder, preset->showBorder);
  g_Settings.accentBand = ResolveTriState(g_UserSettings.accentBand, preset->accentBand);
  g_Settings.accentStripe = ResolveTriState(g_UserSettings.accentStripe, preset->accentStripe);
  g_Settings.artRadius = g_UserSettings.artRadius >= 0 ? g_UserSettings.artRadius : preset->artRadius;
  g_Settings.backgroundStyle = (g_UserSettings.backgroundStyle == BackgroundStyle::ThemeDefault)
    ? preset->backgroundStyle
    : g_UserSettings.backgroundStyle;

  g_Settings.textColor = ParseColorOverride(g_UserSettings.textColor, palette.textColor);
  g_Settings.secondaryTextColor = ParseColorOverride(g_UserSettings.secondaryTextColor, palette.secondaryTextColor);
  g_Settings.backgroundColor = ParseColorOverride(g_UserSettings.backgroundColor, palette.backgroundColor);
  g_Settings.accentColor = ParseColorOverride(g_UserSettings.accentColor, palette.accentColor);
  g_Settings.progressColor = ParseColorOverride(g_UserSettings.progressColor, palette.progressColor);
  g_Settings.hoverColor = ParseColorOverride(g_UserSettings.hoverColor, palette.hoverColor);
  g_Settings.borderColor = ParseColorOverride(g_UserSettings.borderColor, palette.borderColor);

  if (g_UserSettings.backgroundOpacity >= 0) {
    BYTE alpha = (BYTE)ClampInt(g_UserSettings.backgroundOpacity, 0, 255);
    g_Settings.backgroundColor = MakeArgb(alpha, GetR(g_Settings.backgroundColor), GetG(g_Settings.backgroundColor), GetB(g_Settings.backgroundColor));
  }

  g_Settings.width = max(kMinPanelWidth, g_Settings.width);
  g_Settings.height = max(kMinPanelHeight, g_Settings.height);
  g_Settings.fontSize = max(kMinFontSize, g_Settings.fontSize);
  g_Settings.fontWeight = ClampInt(g_Settings.fontWeight, Weight::Regular, Weight::Bold);
  g_Settings.cornerRadius = ClampInt(g_Settings.cornerRadius, 0, min(g_Settings.width, g_Settings.height) / 2);
}

////////////////////////////////////////////////////////////////////////////////
// Media session: GSMTC polling, session selection, and transport commands
////////////////////////////////////////////////////////////////////////////////

Bitmap* CloneBitmap(Bitmap* bitmap) {
  if (!bitmap) {
    return nullptr;
  }

  return bitmap->Clone(
    0,
    0,
    bitmap->GetWidth(),
    bitmap->GetHeight(),
    bitmap->GetPixelFormat());
}

Bitmap* StreamToBitmap(IRandomAccessStreamWithContentType const& stream) {
  if (!stream) {
    return nullptr;
  }

  IStream* nativeStream = nullptr;
  if (SUCCEEDED(CreateStreamOverRandomAccessStream(
      reinterpret_cast<IUnknown*>(winrt::get_abi(stream)),
      IID_PPV_ARGS(&nativeStream)))) {
    Bitmap* bitmap = Bitmap::FromStream(nativeStream);
    nativeStream->Release();
    if (bitmap && bitmap->GetLastStatus() == Ok) {
      return bitmap;
    }

    delete bitmap;
  }

  return nullptr;
}

bool SessionHasMetadata(const GlobalSystemMediaTransportControlsSession& session) {
  if (!session) {
    return false;
  }

  try {
    auto props = session.TryGetMediaPropertiesAsync().get();
    return !wstring(props.Title().c_str()).empty() || !wstring(props.Artist().c_str()).empty();
  } catch (...) {
    return false;
  }
}

// Pick the best GSMTC session: prefer playing with metadata, current, then any.
GlobalSystemMediaTransportControlsSession SelectBestSession() {
  if (!g_SessionManager) {
    return nullptr;
  }

  try {
    auto sessions = g_SessionManager.GetSessions();
    for (auto const& session : sessions) {
      auto playback = session.GetPlaybackInfo();
      if (playback &&
        playback.PlaybackStatus() == GlobalSystemMediaTransportControlsSessionPlaybackStatus::Playing &&
        SessionHasMetadata(session)) {
        return session;
      }
    }

    auto current = g_SessionManager.GetCurrentSession();
    if (current && SessionHasMetadata(current)) {
      return current;
    }

    for (auto const& session : sessions) {
      if (SessionHasMetadata(session)) {
        return session;
      }
    }
  } catch (...) {
  }

  return nullptr;
}

void ClearMediaStateLocked() {
  g_MediaState.title.clear();
  g_MediaState.artist.clear();
  g_MediaState.sourceAppId.clear();
  g_MediaState.isPlaying = false;
  g_MediaState.hasMedia = false;
  g_MediaState.canSeek = false;
  g_MediaState.canGoPrevious = false;
  g_MediaState.canGoNext = false;
  g_MediaState.canTogglePlayPause = false;
  g_MediaState.startTicks = 0;
  g_MediaState.endTicks = 0;
  g_MediaState.positionTicks = 0;
  g_MediaState.minSeekTicks = 0;
  g_MediaState.maxSeekTicks = 0;
  if (g_MediaState.albumArt) {
    delete g_MediaState.albumArt;
    g_MediaState.albumArt = nullptr;
  }
}

// Poll GSMTC, update g_MediaState, return true if anything user-visible changed.
bool UpdateMediaInfo() {
  bool changed = false;

  try {
    if (!g_SessionManager) {
      g_SessionManager = GlobalSystemMediaTransportControlsSessionManager::RequestAsync().get();
    }

    auto session = SelectBestSession();
    if (!session) {
      lock_guard<mutex> guard(g_MediaState.lock);
      changed = g_MediaState.hasMedia || g_MediaState.albumArt != nullptr;
      ClearMediaStateLocked();
      return changed;
    }

    auto props = session.TryGetMediaPropertiesAsync().get();
    auto playback = session.GetPlaybackInfo();
    auto controls = playback.Controls();
    auto timeline = session.GetTimelineProperties();

    wstring newTitle = props.Title().c_str();
    wstring newArtist = props.Artist().c_str();
    wstring newSourceAppId;
    try {
      newSourceAppId = session.SourceAppUserModelId().c_str();
    } catch (...) {
    }

    bool newHasMedia = !newTitle.empty() || !newArtist.empty();
    bool newIsPlaying = playback.PlaybackStatus() == GlobalSystemMediaTransportControlsSessionPlaybackStatus::Playing;
    bool newCanSeek = false;
    bool newCanGoPrevious = false;
    bool newCanGoNext = false;
    bool newCanTogglePlayPause = false;
    long long newStartTicks = 0;
    long long newEndTicks = 0;
    long long newPositionTicks = 0;
    long long newMinSeekTicks = 0;
    long long newMaxSeekTicks = 0;
    Windows::Foundation::DateTime newLastTimelineUpdate{};

    try {
      newCanSeek = controls.IsPlaybackPositionEnabled();
      newCanGoPrevious = controls.IsPreviousEnabled();
      newCanGoNext = controls.IsNextEnabled();
      newCanTogglePlayPause = controls.IsPauseEnabled() || controls.IsPlayEnabled();
      newStartTicks = timeline.StartTime().count();
      newEndTicks = timeline.EndTime().count();
      newPositionTicks = timeline.Position().count();
      newMinSeekTicks = timeline.MinSeekTime().count();
      newMaxSeekTicks = timeline.MaxSeekTime().count();
      newLastTimelineUpdate = timeline.LastUpdatedTime();
    } catch (...) {
      newCanSeek = false;
    }

    if (newEndTicks <= newStartTicks) {
      newCanSeek = false;
      newStartTicks = 0;
      newEndTicks = 0;
      newPositionTicks = 0;
      newMinSeekTicks = 0;
      newMaxSeekTicks = 0;
    } else if (newCanSeek) {
      newPositionTicks = ClampI64(newPositionTicks, newMinSeekTicks, newMaxSeekTicks);
    }

    auto thumbnail = props.Thumbnail();
    bool hasThumbnail = thumbnail != nullptr;

    lock_guard<mutex> guard(g_MediaState.lock);
    changed =
      g_MediaState.hasMedia != newHasMedia ||
      g_MediaState.title != newTitle ||
      g_MediaState.artist != newArtist ||
      g_MediaState.sourceAppId != newSourceAppId ||
      g_MediaState.isPlaying != newIsPlaying ||
      g_MediaState.canSeek != newCanSeek ||
      g_MediaState.canGoPrevious != newCanGoPrevious ||
      g_MediaState.canGoNext != newCanGoNext ||
      g_MediaState.canTogglePlayPause != newCanTogglePlayPause ||
      g_MediaState.positionTicks != newPositionTicks ||
      (g_MediaState.albumArt != nullptr) != hasThumbnail;

    bool reloadAlbumArt =
      g_MediaState.title != newTitle ||
      g_MediaState.artist != newArtist ||
      g_MediaState.sourceAppId != newSourceAppId ||
      (g_MediaState.albumArt == nullptr) != hasThumbnail;

    if (reloadAlbumArt) {
      if (g_MediaState.albumArt) {
        delete g_MediaState.albumArt;
        g_MediaState.albumArt = nullptr;
      }

      if (thumbnail) {
        auto stream = thumbnail.OpenReadAsync().get();
        g_MediaState.albumArt = StreamToBitmap(stream);
      }
    }

    g_MediaState.title = newTitle;
    g_MediaState.artist = newArtist;
    g_MediaState.sourceAppId = newSourceAppId;
    g_MediaState.isPlaying = newIsPlaying;
    g_MediaState.hasMedia = newHasMedia;
    g_MediaState.canSeek = newCanSeek;
    g_MediaState.canGoPrevious = newCanGoPrevious;
    g_MediaState.canGoNext = newCanGoNext;
    g_MediaState.canTogglePlayPause = newCanTogglePlayPause;
    g_MediaState.lastTimelineUpdate = newLastTimelineUpdate;
    g_MediaState.startTicks = newStartTicks;
    g_MediaState.endTicks = newEndTicks;
    g_MediaState.positionTicks = newPositionTicks;
    g_MediaState.minSeekTicks = newMinSeekTicks;
    g_MediaState.maxSeekTicks = newMaxSeekTicks;
    return changed;
  } catch (...) {
    lock_guard<mutex> guard(g_MediaState.lock);
    changed = g_MediaState.hasMedia || g_MediaState.albumArt != nullptr;
    ClearMediaStateLocked();
    return changed;
  }
}

// Create a thread-safe snapshot of global media state with a deep-copied bitmap.
MediaState CopyMediaState() {
  lock_guard<mutex> guard(g_MediaState.lock);
  MediaState copy(g_MediaState);
  copy.albumArt = CloneBitmap(g_MediaState.albumArt);
  return copy;
}

double GetPlaybackProgressRatio(const MediaState& state) {
  if (!state.canSeek || state.endTicks <= state.startTicks) {
    return 0.0;
  }

  long long positionTicks = state.positionTicks;
  if (state.isPlaying) {
    try {
      auto elapsed = winrt::clock::now() - state.lastTimelineUpdate;
      if (elapsed.count() > 0) {
        positionTicks += elapsed.count();
      }
    } catch (...) {
    }
  }

  positionTicks = ClampI64(positionTicks, state.startTicks, state.endTicks);
  long long duration = state.endTicks - state.startTicks;
  if (duration <= 0) {
    return 0.0;
  }

  return ClampDouble((double)(positionTicks - state.startTicks) / (double)duration, 0.0, 1.0);
}

void SendMediaCommand(int command) {
  try {
    if (!g_SessionManager) {
      return;
    }

    auto session = SelectBestSession();
    if (!session) {
      return;
    }

    if (command == MediaCmd::Previous) {
      session.TrySkipPreviousAsync();
    } else if (command == MediaCmd::TogglePlayPause) {
      session.TryTogglePlayPauseAsync();
    } else if (command == MediaCmd::Next) {
      session.TrySkipNextAsync();
    }
  } catch (...) {
  }
}

bool SeekToRatio(double ratio) {
  if (!g_Settings.enableSeekbarInteraction) {
    return false;
  }

  try {
    if (!g_SessionManager) {
      return false;
    }

    auto session = SelectBestSession();
    if (!session) {
      return false;
    }

    auto info = session.GetPlaybackInfo();
    if (!info.Controls().IsPlaybackPositionEnabled()) {
      return false;
    }

    auto timeline = session.GetTimelineProperties();
    long long startTicks = timeline.StartTime().count();
    long long endTicks = timeline.EndTime().count();
    long long minSeekTicks = timeline.MinSeekTime().count();
    long long maxSeekTicks = timeline.MaxSeekTime().count();

    if (endTicks <= startTicks) {
      return false;
    }

    ratio = ClampDouble(ratio, 0.0, 1.0);
    long long target = startTicks + (long long)((endTicks - startTicks) * ratio);
    target = ClampI64(target, minSeekTicks, maxSeekTicks);
    return session.TryChangePlaybackPositionAsync(target).get();
  } catch (...) {
    return false;
  }
}

////////////////////////////////////////////////////////////////////////////////
// Drawing primitives: Rounded rects, text, icons, scrolling text
////////////////////////////////////////////////////////////////////////////////

void CreateRoundedRectPath(GraphicsPath& path, int x, int y, int width, int height, int radius) {
  path.Reset();

  if (radius <= 0) {
    path.AddRectangle(Rect(x, y, width, height));
    return;
  }

  int clampedRadius = min(radius, min(width, height) / 2);
  int diameter = clampedRadius * 2;
  path.AddArc(x, y, diameter, diameter, 180.0f, 90.0f);
  path.AddArc(x + width - diameter, y, diameter, diameter, 270.0f, 90.0f);
  path.AddArc(x + width - diameter, y + height - diameter, diameter, diameter, 0.0f, 90.0f);
  path.AddArc(x, y + height - diameter, diameter, diameter, 90.0f, 90.0f);
  path.CloseFigure();
}

void FillRoundedRect(Graphics& graphics, Brush* brush, int x, int y, int width, int height, int radius) {
  GraphicsPath path;
  CreateRoundedRectPath(path, x, y, width, height, radius);
  graphics.FillPath(brush, &path);
}

void DrawRoundedImage(Graphics& graphics, Bitmap* bitmap, int x, int y, int size, int radius) {
  if (!bitmap) {
    return;
  }

  GraphicsPath path;
  CreateRoundedRectPath(path, x, y, size, size, radius);
  GraphicsState state = graphics.Save();
  graphics.SetClip(&path);
  graphics.DrawImage(bitmap, x, y, size, size);
  graphics.Restore(state);
}

RectF MeasureText(Graphics& graphics, const wstring& text, Font* font) {
  RectF layoutRect(0.0f, 0.0f, 4096.0f, 200.0f);
  RectF bounds;
  graphics.MeasureString(text.c_str(), -1, font, layoutRect, &bounds);
  return bounds;
}

Font* CreateFontForWeight(const wstring& family, REAL size, int weight) {
  INT style = FontStyleRegular;
  if (weight == Weight::Bold) {
    style = FontStyleBold;
  }

  if (weight == Weight::Semibold) {
    // Semibold: try "Segoe UI Semibold" or the family with Semibold style.
    // GDI+ has no native semibold, so we try a semibold font family first.
    wstring semiboldFamily = family + L" Semibold";
    Font* semibold = new Font(semiboldFamily.c_str(), size, FontStyleRegular, UnitPixel);
    if (semibold->GetLastStatus() == Ok) {
      return semibold;
    }
    delete semibold;
    // Fall through to regular style with the original family.
  }

  Font* primary = new Font(family.c_str(), size, style, UnitPixel);
  if (primary->GetLastStatus() == Ok) {
    return primary;
  }

  delete primary;
  return new Font(L"Segoe UI", size, style, UnitPixel);
}

wstring GetPrimaryText(const MediaState& state) {
  if (state.hasMedia) {
    return !state.title.empty() ? state.title : L"Unknown title";
  }

  return L"No media";
}

wstring GetSecondaryText(const MediaState& state) {
  if (state.hasMedia) {
    return state.artist;
  }

  return L"Waiting for a compatible media session";
}

// Arrange art, text, controls, and seekbar into concrete pixel rects.
PanelLayout ComputeLayout(int width, int height, const MediaState& state) {
  PanelLayout layout{};
  int margin = g_Settings.compactMode ? 5 : 6;
  int contentTop = margin;
  int contentBottom = height - margin;
  int seekbarHeight = (g_Settings.showSeekbar && g_Settings.seekbarStyle == Seekbar::Line)
    ? (g_Settings.compactMode ? 4 : 5) : 0;
  int artSize = g_Settings.showAlbumArt ? max(0, height - margin * 2) : 0;
  int buttonSize = g_Settings.compactMode ? 20 : 22;
  int buttonGap = g_Settings.compactMode ? 3 : 4;
  int controlsWidth = g_Settings.showControls ? (buttonSize * kNumControlButtons + buttonGap * 2) : 0;
  int gap = 8;

  // Find where the text component sits in the order.
  int textIdx = 0;
  for (int i = 0; i < kNumControlButtons; i++) {
    if (g_Settings.componentOrder[i] == Component::Text) {
      textIdx = i;
      break;
    }
  }

  // Place components before text (left to right).
  int leftCursor = margin;
  for (int i = 0; i < textIdx; i++) {
    int comp = g_Settings.componentOrder[i];
    if (comp == Component::Art && g_Settings.showAlbumArt) {
      layout.hasArt = true;
      layout.artRect.left = leftCursor;
      layout.artRect.top = (height - artSize) / 2;
      layout.artRect.right = leftCursor + artSize;
      layout.artRect.bottom = layout.artRect.top + artSize;
      leftCursor = layout.artRect.right + gap;
    } else if (comp == Component::Controls && g_Settings.showControls) {
      int top = contentTop + ((contentBottom - contentTop) - buttonSize) / 2;
      if (seekbarHeight > 0) top -= 2;
      for (int j = 0; j < kNumControlButtons; j++) {
        layout.buttonRects[j] = {
          leftCursor + j * (buttonSize + buttonGap), top,
          leftCursor + j * (buttonSize + buttonGap) + buttonSize, top + buttonSize
        };
      }
      layout.hasButtons = true;
      leftCursor += controlsWidth + gap;
    }
  }

  // Place components after text (right to left).
  int rightCursor = width - margin;
  for (int i = 2; i > textIdx; i--) {
    int comp = g_Settings.componentOrder[i];
    if (comp == Component::Art && g_Settings.showAlbumArt && !layout.hasArt) {
      rightCursor -= artSize;
      layout.hasArt = true;
      layout.artRect.left = rightCursor;
      layout.artRect.top = (height - artSize) / 2;
      layout.artRect.right = rightCursor + artSize;
      layout.artRect.bottom = layout.artRect.top + artSize;
      rightCursor -= gap;
    } else if (comp == Component::Controls && g_Settings.showControls && !layout.hasButtons) {
      rightCursor -= controlsWidth;
      int top = contentTop + ((contentBottom - contentTop) - buttonSize) / 2;
      if (seekbarHeight > 0) top -= 2;
      for (int j = 0; j < kNumControlButtons; j++) {
        layout.buttonRects[j] = {
          rightCursor + j * (buttonSize + buttonGap), top,
          rightCursor + j * (buttonSize + buttonGap) + buttonSize, top + buttonSize
        };
      }
      layout.hasButtons = true;
      rightCursor -= gap;
    }
  }

  // Seekbar at the bottom of the text area.
  if (seekbarHeight > 0 && g_Settings.showSeekbar) {
    layout.seekbarRect = {
      leftCursor,
      height - (g_Settings.compactMode ? 9 : 10),
      rightCursor,
      height - (g_Settings.compactMode ? 9 : 10) + seekbarHeight
    };
    layout.hasSeekbar = true;
  }

  // Text fills the remaining horizontal space.
  int textBottom = layout.hasSeekbar ? layout.seekbarRect.top - 3 : contentBottom;
  if (g_Settings.twoLineText) {
    int titleH = max(14, g_Settings.fontSize + 4);
    int subtitleH = max(12, g_Settings.fontSize + 1);
    int availH = max(1, textBottom - contentTop);
    int totalH = min(availH, titleH + subtitleH + 2);
    int startY = contentTop + max(0, (availH - totalH) / 2);
    layout.textRect = {leftCursor, startY, rightCursor, startY + titleH};
    layout.subtitleRect = {leftCursor, startY + titleH + 2, rightCursor, startY + titleH + 2 + subtitleH};
  } else {
    layout.textRect = {leftCursor, contentTop, rightCursor, textBottom};
    layout.subtitleRect = layout.textRect;
  }

  if (!state.hasMedia && g_Settings.hideWhenNoMedia) {
    layout.hasButtons = false;
    layout.hasSeekbar = false;
  }

  return layout;
}

void DrawPrevIcon(Graphics& graphics, const RECT& rc, Brush* brush) {
  int centerX = (rc.left + rc.right) / 2;
  int centerY = (rc.top + rc.bottom) / 2;
  Point points[3] = {
    Point(centerX + 2, centerY - 5),
    Point(centerX + 2, centerY + 5),
    Point(centerX - 4, centerY)
  };
  graphics.FillPolygon(brush, points, 3);
  graphics.FillRectangle(brush, centerX - 6, centerY - 5, 2, 10);
}

void DrawNextIcon(Graphics& graphics, const RECT& rc, Brush* brush) {
  int centerX = (rc.left + rc.right) / 2;
  int centerY = (rc.top + rc.bottom) / 2;
  Point points[3] = {
    Point(centerX - 2, centerY - 5),
    Point(centerX - 2, centerY + 5),
    Point(centerX + 4, centerY)
  };
  graphics.FillPolygon(brush, points, 3);
  graphics.FillRectangle(brush, centerX + 4, centerY - 5, 2, 10);
}

void DrawPlayPauseIcon(Graphics& graphics, const RECT& rc, Brush* brush, bool isPlaying) {
  int centerX = (rc.left + rc.right) / 2;
  int centerY = (rc.top + rc.bottom) / 2;
  if (isPlaying) {
    graphics.FillRectangle(brush, centerX - 5, centerY - 6, 3, 12);
    graphics.FillRectangle(brush, centerX + 2, centerY - 6, 3, 12);
  } else {
    Point points[3] = {
      Point(centerX - 3, centerY - 7),
      Point(centerX - 3, centerY + 7),
      Point(centerX + 6, centerY)
    };
    graphics.FillPolygon(brush, points, 3);
  }
}

void DrawHoverBackground(Graphics& graphics, const RECT& rc, DWORD color) {
  SolidBrush brush(ArgbColor(color));
  int radius = max(6, (int)(min((rc.right - rc.left), (rc.bottom - rc.top)) / 3));
  FillRoundedRect(graphics, &brush, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, radius);
}

void DrawTrimmedText(Graphics& graphics, const wstring& text, Font* font, DWORD color, const RECT& rc) {
  StringFormat format;
  format.SetTrimming(StringTrimmingEllipsisCharacter);
  format.SetFormatFlags(StringFormatFlagsNoWrap);
  SolidBrush brush(ArgbColor(color));
  RectF rect((REAL)rc.left, (REAL)rc.top, (REAL)(rc.right - rc.left), (REAL)(rc.bottom - rc.top));
  graphics.DrawString(text.c_str(), -1, font, rect, &format, &brush);
}

void ResetScrollState(const wstring& scrollKey = L"") {
  g_ScrollKey = scrollKey;
  g_ScrollOffset = 0;
  g_ScrollWait = kScrollPauseFrames;
  g_IsScrolling = false;
  g_TextWidth = 0;
}

void DrawScrollingText(Graphics& graphics, const wstring& text, Font* font, DWORD color, const RECT& rc) {
  if (text != g_ScrollKey) {
    ResetScrollState(text);
  }

  RectF bounds = MeasureText(graphics, text, font);
  g_TextWidth = (int)bounds.Width;
  SolidBrush brush(ArgbColor(color));
  Region clip(Rect(rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top));
  GraphicsState state = graphics.Save();
  graphics.SetClip(&clip);

  if (g_Settings.scrollLongText && g_TextWidth > (rc.right - rc.left)) {
    g_IsScrolling = true;
    REAL textY = (REAL)rc.top;
    REAL drawX = (REAL)(rc.left - g_ScrollOffset);
    graphics.DrawString(text.c_str(), -1, font, PointF(drawX, textY), &brush);
    graphics.DrawString(text.c_str(), -1, font, PointF(drawX + g_TextWidth + (REAL)kScrollGapPx, textY), &brush);
  } else {
    g_IsScrolling = false;
    g_ScrollOffset = 0;
    DrawTrimmedText(graphics, text, font, color, rc);
  }

  graphics.Restore(state);
}

////////////////////////////////////////////////////////////////////////////////
// Panel composition: Background, foreground, layout, and seekbar rendering
////////////////////////////////////////////////////////////////////////////////

void DrawThemeBackground(Graphics& graphics, const MediaState& state, const PanelLayout& layout) {
  GraphicsPath panelPath;
  CreateRoundedRectPath(panelPath, 0, 0, g_Settings.width, g_Settings.height, g_Settings.cornerRadius);

  // For Solid backgrounds, draw the bg rectangle ourselves (acrylic is handled by the compositor).
  if (g_Settings.backgroundStyle == BackgroundStyle::Solid) {
    DWORD bg = g_Settings.backgroundColor;
    SolidBrush brush(ArgbColor(bg));
    graphics.FillPath(&brush, &panelPath);
  }

  // Track progress fill — sweep the background from left to right (fill seekbar style).
  if (g_Settings.showSeekbar && g_Settings.seekbarStyle == Seekbar::Fill && state.hasMedia && g_Settings.backgroundStyle != BackgroundStyle::None) {
    double ratio = GetPlaybackProgressRatio(state);
    int fillW = (int)(g_Settings.width * ratio + 0.5);
    if (fillW > 0) {
      DWORD pc = g_Settings.progressColor;
      SolidBrush progressBrush(ArgbColor(pc));
      GraphicsState sid = graphics.Save();
      graphics.SetClip(&panelPath);
      graphics.FillRectangle(&progressBrush, 0, 0, fillW, g_Settings.height);
      graphics.Restore(sid);
    }
  }

  // Accent band — warm gradient at the bottom.
  if (g_Settings.accentBand && g_Settings.backgroundStyle != BackgroundStyle::None) {
    int bandH = g_Settings.compactMode ? 3 : 4;
    DWORD ac = g_Settings.accentColor;
    Color accentClear = ArgbColorAlpha(ac, 0);
    Color accentFull = ArgbColor(ac);
    LinearGradientBrush bandBrush(
      Point(0, g_Settings.height - bandH),
      Point(g_Settings.width, g_Settings.height),
      accentClear,
      accentFull);
    GraphicsState sid = graphics.Save();
    graphics.SetClip(&panelPath);
    graphics.FillRectangle(&bandBrush, 0, g_Settings.height - bandH, g_Settings.width, bandH);
    graphics.Restore(sid);

    if (!g_Settings.showBorder) {
      Pen borderPen(ArgbColorAlpha(ac, 60), 1.0f);
      graphics.DrawPath(&borderPen, &panelPath);
    }
  }

  // Accent stripe — thin colored line on the left edge.
  if (g_Settings.accentStripe && state.hasMedia) {
    DWORD ac = g_Settings.accentColor;
    SolidBrush stripe(ArgbColor(ac));
    FillRoundedRect(graphics, &stripe, 0, 4, 2, g_Settings.height - 8, 1);
  }

  // Explicit border.
  if (g_Settings.showBorder && g_Settings.backgroundStyle != BackgroundStyle::None) {
    DWORD bc = g_Settings.borderColor;
    Pen borderPen(ArgbColor(bc), 1.0f);
    graphics.DrawPath(&borderPen, &panelPath);
  }

  // Idle placeholder when no media and not hiding, for None backgrounds.
  if (!state.hasMedia && !g_Settings.hideWhenNoMedia && g_Settings.backgroundStyle == BackgroundStyle::None) {
    DWORD tc = g_Settings.textColor;
    SolidBrush placeholder(ArgbColorAlpha(tc, 18));
    FillRoundedRect(graphics, &placeholder, 0, 0, g_Settings.width, g_Settings.height, g_Settings.cornerRadius);
  }
}

// Composite the full media panel into an HDC for layered window presentation.
void DrawMediaPanel(HDC hdc, int width, int height) {
  Graphics graphics(hdc);
  graphics.SetSmoothingMode(SmoothingModeAntiAlias);
  graphics.SetTextRenderingHint(TextRenderingHintAntiAlias);
  graphics.Clear(Color(0, 0, 0, 0));

  MediaState state = CopyMediaState();
  PanelLayout layout = ComputeLayout(width, height, state);
  g_LastLayout = layout;

  DrawThemeBackground(graphics, state, layout);

  // Album art with per-theme radius.
  if (layout.hasArt) {
    int artSize = layout.artRect.right - layout.artRect.left;
    int artRadius;
    if (g_Settings.artRadius >= 0) {
      artRadius = g_Settings.artRadius;
    } else {
      artRadius = g_Settings.cornerRadius > 0
        ? max(0, g_Settings.cornerRadius - (int)layout.artRect.left)
        : max(4, min(8, artSize / 6));
    }
    artRadius = min(artRadius, artSize / 2);

    if (state.albumArt) {
      DrawRoundedImage(graphics, state.albumArt, layout.artRect.left, layout.artRect.top, artSize, artRadius);
    } else {
      SolidBrush placeholder(Color(36, 128, 128, 128));
      FillRoundedRect(graphics, &placeholder, layout.artRect.left, layout.artRect.top, artSize, artSize, artRadius);
    }
  }

  // Playback control buttons.
  if (layout.hasButtons) {
    for (int i = 0; i < kNumControlButtons; i++) {
      if (g_HoverState == i + 1) {
        DrawHoverBackground(graphics, layout.buttonRects[i], g_Settings.hoverColor);
      }
    }

    DWORD tc = g_Settings.textColor;
    SolidBrush iconBrush(ArgbColor(tc));
    DrawPrevIcon(graphics, layout.buttonRects[0], &iconBrush);
    DrawPlayPauseIcon(graphics, layout.buttonRects[1], &iconBrush, state.isPlaying);
    DrawNextIcon(graphics, layout.buttonRects[2], &iconBrush);
  }

  // Text rendering.
  Font* titleFont = CreateFontForWeight(g_Settings.fontFamily, (REAL)g_Settings.fontSize, g_Settings.fontWeight);
  Font* subtitleFont = CreateFontForWeight(g_Settings.fontFamily, (REAL)max(8, g_Settings.fontSize - 1), 0);

  wstring title = GetPrimaryText(state);
  wstring subtitle = GetSecondaryText(state);

  if (g_Settings.twoLineText) {
    DrawScrollingText(graphics, title, titleFont, g_Settings.textColor, layout.textRect);
    DrawTrimmedText(graphics, subtitle, subtitleFont, g_Settings.secondaryTextColor, layout.subtitleRect);
  } else {
    wstring combined = title;
    if (!subtitle.empty() && state.hasMedia) {
      combined += L" \u2022 ";
      combined += subtitle;
    }

    RECT textRect = layout.textRect;
    RectF bounds = MeasureText(graphics, combined, titleFont);
    int textH = (int)bounds.Height;
    if (textH > 0 && (textRect.bottom - textRect.top) > textH) {
      int pad = ((textRect.bottom - textRect.top) - textH) / 2;
      textRect.top += pad;
      textRect.bottom = textRect.top + textH + 2;
    }
    DrawScrollingText(graphics, combined, titleFont, g_Settings.textColor, textRect);
  }

  // Seekbar with track knob.
  if (layout.hasSeekbar && state.canSeek && state.hasMedia) {
    RECT bar = layout.seekbarRect;
    int barW = bar.right - bar.left;
    int barH = bar.bottom - bar.top;
    int radius = max(2, barH / 2);

    // Track background.
    DWORD tc = g_Settings.textColor;
    SolidBrush barBg(ArgbColorAlpha(tc, 50));
    FillRoundedRect(graphics, &barBg, bar.left, bar.top, barW, barH, radius);

    // Progress fill.
    double ratio = g_IsDraggingSeek ? g_DragSeekRatio : GetPlaybackProgressRatio(state);
    int fillW = (int)(barW * ClampDouble(ratio, 0.0, 1.0));
    if (fillW > 0) {
      DWORD pc = g_Settings.progressColor;
      SolidBrush barFill(ArgbColor(pc));
      FillRoundedRect(graphics, &barFill, bar.left, bar.top, fillW, barH, radius);
    }

    // Knob circle.
    int knobSize = max(7, barH + 3);
    int knobX = ClampInt(bar.left + fillW, bar.left, bar.right);
    int knobY = (bar.top + bar.bottom) / 2;
    SolidBrush knobBrush(ArgbColorAlpha(tc, 240));
    graphics.FillEllipse(&knobBrush, knobX - knobSize / 2, knobY - knobSize / 2, knobSize, knobSize);
  }

  if (state.albumArt) {
    delete state.albumArt;
    state.albumArt = nullptr;
  }

  delete titleFont;
  delete subtitleFont;
}

////////////////////////////////////////////////////////////////////////////////
// Window management: Layered window, DWM, positioning, taskbar integration
////////////////////////////////////////////////////////////////////////////////

// Render to a 32-bit DIB and present via UpdateLayeredWindow.
void UpdateLayeredContent(HWND hwnd, int width, int height) {
  if (width <= 0 || height <= 0) {
    return;
  }

  HDC screenDC = GetDC(nullptr);
  if (!screenDC) {
    return;
  }

  HDC memDC = CreateCompatibleDC(screenDC);
  if (!memDC) {
    ReleaseDC(nullptr, screenDC);
    return;
  }

  BITMAPINFO bmi = {};
  bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
  bmi.bmiHeader.biWidth = width;
  bmi.bmiHeader.biHeight = -height;
  bmi.bmiHeader.biPlanes = 1;
  bmi.bmiHeader.biBitCount = 32;
  bmi.bmiHeader.biCompression = BI_RGB;

  void* bits = nullptr;
  HBITMAP dib = CreateDIBSection(memDC, &bmi, DIB_RGB_COLORS, &bits, nullptr, 0);
  if (!dib) {
    DeleteDC(memDC);
    ReleaseDC(nullptr, screenDC);
    return;
  }

  HBITMAP oldBitmap = (HBITMAP)SelectObject(memDC, dib);
  DrawMediaPanel(memDC, width, height);

  RECT windowRect;
  GetWindowRect(hwnd, &windowRect);
  POINT srcPoint = {0, 0};
  POINT dstPoint = {windowRect.left, windowRect.top};
  SIZE windowSize = {width, height};
  BLENDFUNCTION blend = {AC_SRC_OVER, 0, 255, AC_SRC_ALPHA};
  UpdateLayeredWindow(hwnd, screenDC, &dstPoint, &windowSize, memDC, &srcPoint, 0, &blend, ULW_ALPHA);

  SelectObject(memDC, oldBitmap);
  DeleteObject(dib);
  DeleteDC(memDC);
  ReleaseDC(nullptr, screenDC);
}

// Convert ARGB to ABGR for DWM accent gradient color encoding.
DWORD ToAccentGradientColor(DWORD argbColor) {
  return MakeArgb(GetA(argbColor), GetB(argbColor), GetG(argbColor), GetR(argbColor));
}

void UpdateWindowRegion(HWND hwnd) {
  if (!hwnd) {
    return;
  }

  int radius = ClampInt(g_Settings.cornerRadius, 0, min(g_Settings.width, g_Settings.height) / 2);
  if (radius <= 0) {
    SetWindowRgn(hwnd, nullptr, TRUE);
    return;
  }

  int ellipse = radius * 2;
  HRGN region = CreateRoundRectRgn(0, 0, g_Settings.width + 1, g_Settings.height + 1, ellipse, ellipse);
  if (!region) {
    return;
  }

  if (!SetWindowRgn(hwnd, region, TRUE)) {
    DeleteObject(region);
  }
}

// Apply DWM corner policy, window region, and acrylic composition attributes.
void UpdateAppearance(HWND hwnd) {
  // Use the explicit region as the source of truth so the requested radius
  // is applied consistently for acrylic, solid, and backgroundless modes.
  DWM_WINDOW_CORNER_PREFERENCE preference = DWMWCP_DONOTROUND;
  DwmSetWindowAttribute(hwnd, DWMWA_WINDOW_CORNER_PREFERENCE, &preference, sizeof(preference));
  UpdateWindowRegion(hwnd);

  HMODULE user32Module = GetModuleHandle(L"user32.dll");
  if (!user32Module) {
    return;
  }

  auto setCompositionAttribute = (pSetWindowCompositionAttribute)GetProcAddress(user32Module, "SetWindowCompositionAttribute");
  if (!setCompositionAttribute) {
    return;
  }

  ACCENT_POLICY policy = {};
  if (g_Settings.backgroundStyle == BackgroundStyle::Acrylic) {
    policy.AccentState = ACCENT_ENABLE_ACRYLICBLURBEHIND;
    policy.GradientColor = ToAccentGradientColor(g_Settings.backgroundColor);
  } else {
    policy.AccentState = ACCENT_DISABLED;
  }

  WINDOWCOMPOSITIONATTRIBDATA data = {WCA_ACCENT_POLICY, &policy, sizeof(policy)};
  setCompositionAttribute(hwnd, &data);
}

bool IsTaskbarWindow(HWND hwnd) {
  WCHAR className[64] = {};
  if (!hwnd) {
    return false;
  }

  GetClassNameW(hwnd, className, ARRAYSIZE(className));
  return wcscmp(className, L"Shell_TrayWnd") == 0;
}

void CALLBACK TaskbarEventProc(HWINEVENTHOOK, DWORD, HWND hwnd, LONG, LONG, DWORD, DWORD) {
  if (!IsTaskbarWindow(hwnd) || !g_hMediaWindow) {
    return;
  }

  PostMessage(g_hMediaWindow, APP_WM_REPOSITION, 0, 0);
}

void RegisterTaskbarHook(HWND hwnd) {
  HWND taskbar = FindWindow(L"Shell_TrayWnd", nullptr);
  if (!taskbar) {
    return;
  }

  DWORD processId = 0;
  DWORD threadId = GetWindowThreadProcessId(taskbar, &processId);
  if (!threadId) {
    return;
  }

  g_TaskbarHook = SetWinEventHook(
    EVENT_OBJECT_LOCATIONCHANGE,
    EVENT_OBJECT_LOCATIONCHANGE,
    nullptr,
    TaskbarEventProc,
    processId,
    threadId,
    WINEVENT_OUTOFCONTEXT | WINEVENT_SKIPOWNPROCESS);

  PostMessage(hwnd, APP_WM_REPOSITION, 0, 0);
}

bool IsFullscreenBlocked() {
  if (!g_Settings.hideFullscreen) {
    return false;
  }

  QUERY_USER_NOTIFICATION_STATE notificationState;
  if (SUCCEEDED(SHQueryUserNotificationState(&notificationState))) {
    return notificationState == QUNS_BUSY ||
         notificationState == QUNS_RUNNING_D3D_FULL_SCREEN ||
         notificationState == QUNS_PRESENTATION_MODE;
  }

  return false;
}

// Determine whether the panel should be visible based on media, fullscreen, and idle state.
bool ShouldWindowBeVisible() {
  bool hasMedia = false;
  bool isPlaying = false;
  {
    lock_guard<mutex> guard(g_MediaState.lock);
    hasMedia = g_MediaState.hasMedia;
    isPlaying = g_MediaState.isPlaying;
  }

  if (!hasMedia && g_Settings.hideWhenNoMedia) {
    return false;
  }

  if (IsFullscreenBlocked()) {
    return false;
  }

  if (g_Settings.idleTimeout > 0) {
    if (isPlaying) {
      g_IdleSecondsCounter = 0;
      g_IsHiddenByIdle = false;
    } else {
      g_IdleSecondsCounter++;
      if (g_IdleSecondsCounter >= g_Settings.idleTimeout) {
        g_IsHiddenByIdle = true;
      }
    }
  } else {
    g_IdleSecondsCounter = 0;
    g_IsHiddenByIdle = false;
  }

  return !g_IsHiddenByIdle;
}

// Start or stop the animation timer based on whether we need smooth updates.
void UpdateAnimationTimer(HWND hwnd) {
  bool hasMedia, isPlaying;
  {
    lock_guard<mutex> guard(g_MediaState.lock);
    hasMedia = g_MediaState.hasMedia;
    isPlaying = g_MediaState.isPlaying;
  }

  bool needsAnimation = g_IsScrolling || (hasMedia && isPlaying && g_Settings.showSeekbar);

  if (needsAnimation) {
    SetTimer(hwnd, IDT_ANIMATION, kAnimationFrameMs, nullptr);
  } else {
    KillTimer(hwnd, IDT_ANIMATION);
    if (!g_IsScrolling) {
      g_ScrollOffset = 0;
      g_ScrollWait = kScrollPauseFrames;
    }
  }
}

// Position the layered window relative to the taskbar using alignment + offset.
void RepositionWindow(HWND hwnd) {
  HWND taskbar = FindWindow(L"Shell_TrayWnd", nullptr);
  if (!taskbar) {
    return;
  }

  if (!IsWindowVisible(taskbar)) {
    if (IsWindowVisible(hwnd)) {
      ShowWindow(hwnd, SW_HIDE);
    }
    return;
  }

  RECT taskbarRect;
  GetWindowRect(taskbar, &taskbarRect);
  int taskbarWidth = taskbarRect.right - taskbarRect.left;
  int taskbarHeight = taskbarRect.bottom - taskbarRect.top;

  int x = taskbarRect.left + g_Settings.offsetX;
  if (g_Settings.horizontalAlignment == Align::Center) {
    x = taskbarRect.left + (taskbarWidth - g_Settings.width) / 2 + g_Settings.offsetX;
  } else if (g_Settings.horizontalAlignment == Align::Right) {
    x = taskbarRect.right - g_Settings.width - g_Settings.offsetX;
  }

  int y = taskbarRect.top + (taskbarHeight - g_Settings.height) / 2 + g_Settings.offsetY;
  SetWindowPos(hwnd, HWND_TOPMOST, x, y, g_Settings.width, g_Settings.height, SWP_NOACTIVATE | SWP_SHOWWINDOW);
  UpdateWindowRegion(hwnd);
}

bool IsPointInRectInt(int x, int y, const RECT& rc) {
  return x >= rc.left && x < rc.right && y >= rc.top && y < rc.bottom;
}

double SeekRatioFromPoint(int x) {
  RECT rc = g_LastLayout.seekbarRect;
  int width = rc.right - rc.left;
  if (width <= 0) {
    return 0.0;
  }

  return ClampDouble((double)(x - rc.left) / (double)width, 0.0, 1.0);
}

////////////////////////////////////////////////////////////////////////////////
// Window procedure and message loop
////////////////////////////////////////////////////////////////////////////////

LRESULT CALLBACK MediaWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
  switch (msg) {
    case WM_CREATE:
      UpdateAppearance(hwnd);
      RegisterTaskbarHook(hwnd);
      SetTimer(hwnd, IDT_POLL_MEDIA, kPollIntervalMs, nullptr);
      return 0;

    case WM_ERASEBKGND:
      return 1;

    case WM_CLOSE:
      return 0;

    case APP_WM_CLOSE:
      DestroyWindow(hwnd);
      return 0;

    case WM_DESTROY:
      if (g_TaskbarHook) {
        UnhookWinEvent(g_TaskbarHook);
        g_TaskbarHook = nullptr;
      }
      KillTimer(hwnd, IDT_POLL_MEDIA);
      KillTimer(hwnd, IDT_ANIMATION);
      g_SessionManager = nullptr;
      PostQuitMessage(0);
      return 0;

    case WM_SETTINGCHANGE:
      UpdateAppearance(hwnd);
      InvalidateRect(hwnd, nullptr, FALSE);
      return 0;

    case WM_TIMER:
      if (wParam == IDT_POLL_MEDIA) {
        UpdateMediaInfo();
        bool shouldShow = ShouldWindowBeVisible();
        if (!shouldShow) {
          if (IsWindowVisible(hwnd)) {
            ShowWindow(hwnd, SW_HIDE);
          }
        } else {
          if (!IsWindowVisible(hwnd)) {
            ShowWindow(hwnd, SW_SHOWNOACTIVATE);
          }
          RepositionWindow(hwnd);
          InvalidateRect(hwnd, nullptr, FALSE);
        }

        UpdateAnimationTimer(hwnd);
      } else if (wParam == IDT_ANIMATION) {
        if (g_IsScrolling) {
          if (g_ScrollWait > 0) {
            g_ScrollWait--;
          } else {
            g_ScrollOffset++;
            if (g_ScrollOffset > g_TextWidth + kScrollGapPx) {
              g_ScrollOffset = 0;
              g_ScrollWait = kScrollPauseFrames;
            }
          }
        }

        if (IsWindowVisible(hwnd)) {
          InvalidateRect(hwnd, nullptr, FALSE);
        }
      }
      return 0;

    case APP_WM_REPOSITION:
      RepositionWindow(hwnd);
      InvalidateRect(hwnd, nullptr, FALSE);
      return 0;

    case WM_MOUSEMOVE: {
      int x = GET_X_LPARAM(lParam);
      int y = GET_Y_LPARAM(lParam);
      int newHoverState = 0;

      if (g_LastLayout.hasButtons) {
        for (int index = 0; index < kNumControlButtons; index++) {
          if (IsPointInRectInt(x, y, g_LastLayout.buttonRects[index])) {
            newHoverState = index + 1;
            break;
          }
        }
      }

      if (newHoverState != g_HoverState) {
        g_HoverState = newHoverState;
        InvalidateRect(hwnd, nullptr, FALSE);
      }

      if (g_IsDraggingSeek && g_LastLayout.hasSeekbar) {
        g_DragSeekRatio = SeekRatioFromPoint(x);
        InvalidateRect(hwnd, nullptr, FALSE);
      }

      TRACKMOUSEEVENT trackMouseEvent = {sizeof(trackMouseEvent), TME_LEAVE, hwnd, 0};
      TrackMouseEvent(&trackMouseEvent);
      return 0;
    }

    case WM_MOUSELEAVE:
      if (!g_IsDraggingSeek) {
        g_HoverState = 0;
        InvalidateRect(hwnd, nullptr, FALSE);
      }
      return 0;

    case WM_LBUTTONDOWN: {
      int x = GET_X_LPARAM(lParam);
      int y = GET_Y_LPARAM(lParam);
      if (g_LastLayout.hasSeekbar && IsPointInRectInt(x, y, g_LastLayout.seekbarRect)) {
        g_IsDraggingSeek = true;
        g_DragSeekRatio = SeekRatioFromPoint(x);
        SetCapture(hwnd);
        InvalidateRect(hwnd, nullptr, FALSE);
        return 0;
      }
      return 0;
    }

    case WM_LBUTTONUP: {
      int x = GET_X_LPARAM(lParam);
      int y = GET_Y_LPARAM(lParam);
      if (g_IsDraggingSeek) {
        g_IsDraggingSeek = false;
        ReleaseCapture();
        SeekToRatio(g_DragSeekRatio);
        InvalidateRect(hwnd, nullptr, FALSE);
        return 0;
      }

      if (g_LastLayout.hasButtons) {
        if (IsPointInRectInt(x, y, g_LastLayout.buttonRects[0])) {
          SendMediaCommand(MediaCmd::Previous);
        } else if (IsPointInRectInt(x, y, g_LastLayout.buttonRects[1])) {
          SendMediaCommand(MediaCmd::TogglePlayPause);
        } else if (IsPointInRectInt(x, y, g_LastLayout.buttonRects[2])) {
          SendMediaCommand(MediaCmd::Next);
        }
      }
      return 0;
    }

    case WM_CAPTURECHANGED:
      if (g_IsDraggingSeek) {
        g_IsDraggingSeek = false;
        InvalidateRect(hwnd, nullptr, FALSE);
      }
      return 0;

    case WM_MOUSEWHEEL:
      if (g_Settings.mouseWheelVolume) {
        short delta = GET_WHEEL_DELTA_WPARAM(wParam);
        WORD key = delta > 0 ? VK_VOLUME_UP : VK_VOLUME_DOWN;
        keybd_event(key, 0, 0, 0);
        keybd_event(key, 0, KEYEVENTF_KEYUP, 0);
      }
      return 0;

    case WM_PAINT: {
      PAINTSTRUCT paintStruct;
      BeginPaint(hwnd, &paintStruct);
      RECT clientRect;
      GetClientRect(hwnd, &clientRect);
      UpdateLayeredContent(hwnd, clientRect.right, clientRect.bottom);
      EndPaint(hwnd, &paintStruct);
      return 0;
    }

    default:
      if (msg == g_TaskbarCreatedMsg) {
        if (g_TaskbarHook) {
          UnhookWinEvent(g_TaskbarHook);
          g_TaskbarHook = nullptr;
        }
        RegisterTaskbarHook(hwnd);
        return 0;
      }
      break;
  }

  return DefWindowProc(hwnd, msg, wParam, lParam);
}

////////////////////////////////////////////////////////////////////////////////
// Thread entry and mod lifecycle
////////////////////////////////////////////////////////////////////////////////

void MediaThread() {
  winrt::init_apartment();

  GdiplusStartupInput gdiplusStartupInput;
  ULONG_PTR gdiplusToken = 0;
  GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, nullptr);

  WNDCLASS windowClass = {};
  windowClass.lpfnWndProc = MediaWndProc;
  windowClass.hInstance = GetModuleHandle(nullptr);
  windowClass.lpszClassName = TEXT("WindhawkMediaBeacon_GSMTC");
  windowClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
  RegisterClass(&windowClass);

  HMODULE user32Module = GetModuleHandle(L"user32.dll");
  pCreateWindowInBand createWindowInBand = nullptr;
  if (user32Module) {
    createWindowInBand = (pCreateWindowInBand)GetProcAddress(user32Module, "CreateWindowInBand");
  }

  if (createWindowInBand) {
    g_hMediaWindow = createWindowInBand(
      WS_EX_LAYERED | WS_EX_TOOLWINDOW | WS_EX_TOPMOST,
      windowClass.lpszClassName,
      TEXT("MediaBeacon"),
      WS_POPUP | WS_VISIBLE,
      0,
      0,
      g_Settings.width,
      g_Settings.height,
      nullptr,
      nullptr,
      windowClass.hInstance,
      nullptr,
      ZBID_IMMERSIVE_NOTIFICATION);
  }

  if (!g_hMediaWindow) {
    g_hMediaWindow = CreateWindowEx(
      WS_EX_LAYERED | WS_EX_TOOLWINDOW | WS_EX_TOPMOST,
      windowClass.lpszClassName,
      TEXT("MediaBeacon"),
      WS_POPUP | WS_VISIBLE,
      0,
      0,
      g_Settings.width,
      g_Settings.height,
      nullptr,
      nullptr,
      windowClass.hInstance,
      nullptr);
  }

  MSG message;
  while (GetMessage(&message, nullptr, 0, 0)) {
    TranslateMessage(&message);
    DispatchMessage(&message);
  }

  UnregisterClass(windowClass.lpszClassName, windowClass.hInstance);
  GdiplusShutdown(gdiplusToken);
  winrt::uninit_apartment();
}

BOOL WhTool_ModInit() {
  SetCurrentProcessExplicitAppUserModelID(L"taskbar-media-beacon");
  LoadSettings();
  g_pMediaThread = new thread(MediaThread);
  return TRUE;
}

void WhTool_ModUninit() {
  if (g_hMediaWindow) {
    SendMessage(g_hMediaWindow, APP_WM_CLOSE, 0, 0);
  }

  if (g_pMediaThread) {
    if (g_pMediaThread->joinable()) {
      g_pMediaThread->join();
    }
    delete g_pMediaThread;
    g_pMediaThread = nullptr;
  }
}

void WhTool_ModSettingsChanged() {
  LoadSettings();
  ResetScrollState();
  if (g_hMediaWindow) {
    SendMessage(g_hMediaWindow, WM_SETTINGCHANGE, 0, 0);
    SendMessage(g_hMediaWindow, APP_WM_REPOSITION, 0, 0);
    InvalidateRect(g_hMediaWindow, nullptr, FALSE);
  }
}

////////////////////////////////////////////////////////////////////////////////
// Windhawk tool mod implementation for mods which don't need to inject to other
// processes or hook other functions. Context:
// https://github.com/ramensoftware/windhawk-mods/pull/1916
//
// The mod will load and run in a dedicated windhawk.exe process.
//
// Paste the code below as part of the mod code, and use these callbacks:
// * WhTool_ModInit
// * WhTool_ModSettingsChanged
// * WhTool_ModUninit
//
// Currently, other callbacks are not supported.

bool g_isToolModProcessLauncher;
HANDLE g_toolModProcessMutex;

void WINAPI EntryPoint_Hook() {
  Wh_Log(L">");
  ExitThread(0);
}

BOOL Wh_ModInit() {
  bool isService = false;
  bool isToolModProcess = false;
  bool isCurrentToolModProcess = false;
  int argc;
  LPWSTR* argv = CommandLineToArgvW(GetCommandLine(), &argc);
  if (!argv) {
    Wh_Log(L"CommandLineToArgvW failed");
    return FALSE;
  }

  for (int i = 1; i < argc; i++) {
    if (wcscmp(argv[i], L"-service") == 0) {
      isService = true;
      break;
    }
  }

  for (int i = 1; i < argc - 1; i++) {
    if (wcscmp(argv[i], L"-tool-mod") == 0) {
      isToolModProcess = true;
      if (wcscmp(argv[i + 1], WH_MOD_ID) == 0) {
        isCurrentToolModProcess = true;
      }
      break;
    }
  }

  LocalFree(argv);

  if (isService) {
    return FALSE;
  }

  if (isCurrentToolModProcess) {
    g_toolModProcessMutex =
      CreateMutex(nullptr, TRUE, L"windhawk-tool-mod_" WH_MOD_ID);
    if (!g_toolModProcessMutex) {
      Wh_Log(L"CreateMutex failed");
      ExitProcess(1);
    }

    if (GetLastError() == ERROR_ALREADY_EXISTS) {
      Wh_Log(L"Tool mod already running (%s)", WH_MOD_ID);
      ExitProcess(1);
    }

    if (!WhTool_ModInit()) {
      ExitProcess(1);
    }

    IMAGE_DOS_HEADER* dosHeader =
      (IMAGE_DOS_HEADER*)GetModuleHandle(nullptr);
    IMAGE_NT_HEADERS* ntHeaders =
      (IMAGE_NT_HEADERS*)((BYTE*)dosHeader + dosHeader->e_lfanew);

    DWORD entryPointRVA = ntHeaders->OptionalHeader.AddressOfEntryPoint;
    void* entryPoint = (BYTE*)dosHeader + entryPointRVA;

    Wh_SetFunctionHook(entryPoint, (void*)EntryPoint_Hook, nullptr);
    return TRUE;
  }

  if (isToolModProcess) {
    return FALSE;
  }

  g_isToolModProcessLauncher = true;
  return TRUE;
}

void Wh_ModAfterInit() {
  if (!g_isToolModProcessLauncher) {
    return;
  }

  WCHAR currentProcessPath[MAX_PATH];
  switch (GetModuleFileName(nullptr, currentProcessPath, ARRAYSIZE(currentProcessPath))) {
    case 0:
    case ARRAYSIZE(currentProcessPath):
      Wh_Log(L"GetModuleFileName failed");
      return;
  }

  WCHAR commandLine[
    MAX_PATH + 2 +
    (sizeof(L" -tool-mod \"" WH_MOD_ID "\"") / sizeof(WCHAR)) - 1];
  swprintf_s(commandLine, L"\"%s\" -tool-mod \"%s\"", currentProcessPath, WH_MOD_ID);

  HMODULE kernelModule = GetModuleHandle(L"kernelbase.dll");
  if (!kernelModule) {
    kernelModule = GetModuleHandle(L"kernel32.dll");
    if (!kernelModule) {
      Wh_Log(L"No kernelbase.dll/kernel32.dll");
      return;
    }
  }

  using CreateProcessInternalW_t = BOOL(WINAPI*)(
    HANDLE hUserToken,
    LPCWSTR lpApplicationName,
    LPWSTR lpCommandLine,
    LPSECURITY_ATTRIBUTES lpProcessAttributes,
    LPSECURITY_ATTRIBUTES lpThreadAttributes,
    BOOL bInheritHandles,
    DWORD dwCreationFlags,
    LPVOID lpEnvironment,
    LPCWSTR lpCurrentDirectory,
    LPSTARTUPINFOW lpStartupInfo,
    LPPROCESS_INFORMATION lpProcessInformation,
    PHANDLE hRestrictedUserToken);

  CreateProcessInternalW_t pCreateProcessInternalW =
    (CreateProcessInternalW_t)GetProcAddress(kernelModule, "CreateProcessInternalW");
  if (!pCreateProcessInternalW) {
    Wh_Log(L"No CreateProcessInternalW");
    return;
  }

  STARTUPINFO startupInfo = {};
  startupInfo.cb = sizeof(STARTUPINFO);
  startupInfo.dwFlags = STARTF_FORCEOFFFEEDBACK;

  PROCESS_INFORMATION processInformation = {};
  if (!pCreateProcessInternalW(
      nullptr,
      currentProcessPath,
      commandLine,
      nullptr,
      nullptr,
      FALSE,
      NORMAL_PRIORITY_CLASS,
      nullptr,
      nullptr,
      &startupInfo,
      &processInformation,
      nullptr)) {
    Wh_Log(L"CreateProcess failed");
    return;
  }

  CloseHandle(processInformation.hProcess);
  CloseHandle(processInformation.hThread);
}

void Wh_ModSettingsChanged() {
  if (g_isToolModProcessLauncher) {
    return;
  }

  WhTool_ModSettingsChanged();
}

void Wh_ModUninit() {
  if (g_isToolModProcessLauncher) {
    return;
  }

  WhTool_ModUninit();
  ExitProcess(0);
}
