# Dart/Flutter UI VST3 Plugin (`dartui-plugin`)

Embed a Flutter engine as the plugin editor UI, replacing VSTGUI. The Flutter view renders as a child window inside the DAW's view hierarchy. Communication between the C++ audio processor and the Dart UI uses Flutter platform channels (MethodChannel + EventChannel).

## Decisions

- **Resizable windows**: Yes. `canResize()` returns `kResultTrue`. Bi-directional resize between DAW host and Flutter.
- **UI content**: Minimal/experimental — standard sliders/knobs driven by VST parameters. No specific design.
- **Engine lifecycle**: Created on `attached()`, destroyed on `removed()`. One FlutterEngine per editor open. No engine caching between open/close cycles. (If startup latency becomes an issue later, cache in the controller.)
- **Target platforms**: macOS + Windows initially.

## Architecture

```
DAW Host
 ┌──────────────────────────────────────────┐
 │  Parent NSView/HWND                       │
 │  ┌────────────────────────────────────┐   │
 │  │  Flutter Child View                │   │
 │  │  ┌──────────────────────────────┐  │   │
 │  │  │  Dart Widgets (Flutter Engine)│  │   │
 │  │  └──────────────┬───────────────┘  │   │
 │  │                 │ Platform Channels │   │
 │  └─────────────────┼──────────────────┘   │
 │  ┌──────────────────┴──────────────────┐   │
 │  │  PlatformChannelBridge (C++)         │   │
 │  └──────────────────┬──────────────────┘   │
 │  ┌──────────────────┴──────────────────┐   │
 │  │  Controller (EditControllerEx1)      │   │
 │  └─────────────────────────────────────┘   │
 │                    ↕ IBStream                │
 │  ┌─────────────────────────────────────┐   │
 │  │  Processor (AudioEffect)             │   │
 │  └─────────────────────────────────────┘   │
 └──────────────────────────────────────────┘
```

## Directory Structure

```
src/dartui-plugin/
├── CMakeLists.txt
├── source/
│   ├── cids.h                            # UID definitions
│   ├── pid.h                             # Parameter definitions
│   ├── version.h                         # Plugin version
│   ├── entry.cpp                         # VST3 factory entry point
│   ├── processor.h / .cpp                # Audio processing
│   ├── controller.h / .cpp               # EditController + creates FlutterEditorView
│   ├── flutter_editor_view.h / .cpp      # IPlugView impl — embeds Flutter child view
│   ├── flutter_engine_manager.h / .cpp   # Flutter engine lifecycle per editor
│   └── platform_channel_bridge.h / .cpp  # C++ side of method/event channels
├── flutter_ui/
│   ├── pubspec.yaml
│   ├── lib/
│   │   ├── main.dart                     # Entry point
│   │   ├── plugin_ui.dart                # Widget tree (sliders, knobs)
│   │   ├── parameter_store.dart          # ChangeNotifier for VST params
│   │   └── platform_bridge.dart          # Dart side of method channel
├── resource/
│   └── snapshots/
└── build_dartui.sh                       # Orchestrates flutter build + cmake
```

## Component Specifications

### 1. Plugin Scaffold (following existing patterns)
- `processor.h/.cpp` — extends `Steinberg::Vst::AudioEffect`, standard audio pass-through with parameter handling.
- `controller.h/.cpp` — extends `Steinberg::Vst::EditControllerEx1`. `createView()` returns a new `FlutterEditorView` instance. Parameter registration in `initialize()`.
- `entry.cpp` — factory definitions for processor + controller.
- `cids.h`, `pid.h`, `version.h` — standard plugin metadata.

### 2. `FlutterEngineManager` (engine lifecycle per editor)
```
created in attached() ──→ FlutterEngine::run(assets_path)
  ├── loads kernel snapshot + flutter_assets
  ├── creates platform view (FlutterView on macOS / child HWND on Windows)
  ├── attaches as child of parent handle from DAW
  ├── sets up platform channel bridge
  └── pumps initial frame

destroyed in removed() ──→ FlutterEngine::shutdown()
  ├── detaches child view from parent
  └── frees all engine resources
```

### 3. `FlutterEditorView` (IPlugView implementation)
```
IPlugView interface:
  attached(parent, type)     → hands parent to engine manager, creates Flutter child view
  removed()                  → tears down engine
  onSize(newSize)            → resizes Flutter child view to match
  getSize(size)              → returns current Flutter view size
  checkSizeConstraint(rect)  → enforces min size (e.g. 400x200)
  canResize()                → kResultTrue
  onKeyDown(key, code, mod)  → forwarded to Flutter engine
  onKeyUp(key, code, mod)    → forwarded to Flutter engine
  onWheel(distance)          → forwarded to Flutter engine
  onFocus(state)             → forwarded to Flutter engine
  setFrame(frame)            → stores IPlugFrame ref for resize requests
  isPlatformTypeSupported(t) → accepts kPlatformTypeNSView (macOS) / kPlatformTypeHWND (Win)
```

When Flutter requests resize (user drags corner in Dart) → calls `IPlugFrame::resizeView()`.

### 4. `PlatformChannelBridge` (C++ side)
```
MethodChannel("vst_plugin/params"):
  C++ handlers:
    "getAllParams"    → returns map of {id: {value, name, displayValue}}
    "setParam"        → calls EditController::setParamNormalized(id, value)
    "beginEdit"       → calls EditController::beginEdit(id)
    "endEdit"         → calls EditController::endEdit(id)

EventChannel("vst_plugin/param_events"):
  C++ sends:
    "paramChanged"    → {id, value} (when DAW automates a param)
```

- On engine ready: sends all registered parameter initial values to Dart.
- When `setParamNormalized()` is called on the controller (from DAW automation), push event to Dart.
- When `setComponentState()` is called (preset load), push all params to Dart.

### 5. Dart Side (`flutter_ui/lib/`)
```
main.dart:
  - runApp() with PluginUI widget
  - initialize PlatformBridge

platform_bridge.dart:
  - setUpMethodChannel("vst_plugin/params"):
    - calls getAllParams on startup
    - listens to paramChanged events → updates ParameterStore

parameter_store.dart:
  - ChangeNotifier holding Map<int, ParamInfo>
  - setParam(id, value) → sends "setParam" to C++ via MethodChannel
  - Notify listeners on param change from C++

plugin_ui.dart:
  - Widget tree: Column of Slider widgets, one per parameter
  - Slider onChanged → parameterStore.setParam(id, value)
```

### 6. Build Integration
```
build_dartui.sh:
  1. cd flutter_ui && dart compile kernel → kernel_blob.bin
  2. Collect flutter_assets/ → copied to plugin resource dir
  3. cmake -S src/dartui-plugin -B build/dartui-plugin-release
     - Flutter engine headers from Flutter SDK installation
     - Link Flutter engine libraries (FlutterMacOS.framework / flutter_windows.dll)
     - Bundle flutter_assets + kernel_blob as plugin resources
  4. cmake --build → produces dartui-plugin.vst3 bundle
```

### 7. Platform-Specific Details
```
macOS:
  Flutter Embedder: FlutterMacOS.framework (from Flutter SDK)
  Rendering: Metal (default) or OpenGL
  Child view: FlutterViewController.view (NSView subclass)
  Attach: [parentView addSubview:flutterView]
  Resize: setFrame: on the NSView

Windows:
  Flutter Embedder: flutter_windows.dll + FlutterWindowsEngine
  Rendering: ANGLE (OpenGL ES → Direct3D)
  Child view: FlutterWindowsView (child HWND)
  Attach: SetParent(flutterHwnd, parentHwnd) + SetWindowPos(...)
  Resize: SetWindowPos(...)
```

## Implementation Steps (in order)

| # | Step | What to build |
|---|---|---|
| 1 | Scaffold `dartui-plugin/` | processor.cpp, controller.cpp, entry.cpp, cids.h, pid.h, version.h — copy pattern from gain-plugin |
| 2 | Create `flutter_ui/` Flutter project | pubspec.yaml, main.dart, plugin_ui.dart, parameter_store.dart, platform_bridge.dart |
| 3 | Implement `flutter_engine_manager` | Engine create/destroy per editor, platform view creation, macOS + Windows paths |
| 4 | Implement `flutter_editor_view` | IPlugView wrapping engine + platform view, resize, key/scroll forwarding |
| 5 | Implement `platform_channel_bridge` | C++ MethodChannel + EventChannel, Dart-side bridge, wire controller param changes through channels |
| 6 | Build integration | CMakeLists.txt (find Flutter SDK, link engine libs, bundle assets) + build_dartui.sh |
| 7 | Wire resize support | Bi-directional resize between Flutter and DAW host via IPlugFrame |

## Risks

- **DAW compatibility quirks**: Some DAWs handle plugin UI embedding differently. The `FlutterEngineManager` / `FlutterEditorView` abstractions isolate platform-specific code so fixes are localized.
- **Flutter engine binary size**: ~15-30MB per platform. Acceptable for desktop; compress in distribution.
- **Multiple engines memory**: ~50-80MB per plugin instance. Acceptable for expected use (1-2 instances typical).
- **Runloop integration**: Flutter engine runs its own event loop. May need custom task runner integration with DAW run loop.
- **NSApplication ownership on macOS**: Flutter macOS embedder assumes it owns NSApplication. Test with Logic, Ableton, Cubase for any conflicts.
- **No Flutter SDK guarantee**: User may not have Flutter SDK installed. build_dartui.sh should detect and report missing SDK.
