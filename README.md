# deki-http

Documentation: https://dekiengine.github.io/deki-http/ (components and properties, generated from the code)

Provides an `IDekiHttpClient` implementation for desktop builds, registered into `DekiHttp` (engine-core) via a SetupComponent that runs automatically when the editor finishes loading packages.

Other packages consume HTTP via the one-call facade:

```cpp
std::string body = DekiHttp::FetchUrl("http://example.com/api");
// returns "" if no client is registered or the fetch failed
```

## Implementations

- `WinHttpClient` (Windows): WinHTTP-backed synchronous GET. Linked via `winhttp.lib` (declared in `package.cmake` as `PACKAGE_SYSTEM_LIBS`).

## Embedded targets

This package is editor / desktop only. Embedded boards do not link it; their network stack lives in board-specific integration packages.

## Namespace

This package's types live in `DekiHttp`. Scene files store the qualified
name, so a component is `DekiHttp::SomeComponent` there, and code naming one
needs the namespace:

```cpp
using namespace DekiHttp;
obj->AddComponent<SomeComponent>();
```

Scenes saved before 0.16.0 used bare names and still load: every component
records what it used to be called, and a save writes the current name.

