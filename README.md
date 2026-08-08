# deki-http

Provides an `IDekiHttpClient` implementation for desktop builds, registered into `DekiHttp` (engine-core) via a SetupComponent that runs automatically when the editor finishes loading modules.

Other modules consume HTTP via the one-call facade:

```cpp
std::string body = DekiHttp::FetchUrl("http://example.com/api");
// returns "" if no client is registered or the fetch failed
```

## Implementations

- `WinHttpClient` (Windows): WinHTTP-backed synchronous GET. Linked via `winhttp.lib` (declared in `module.cmake` as `MODULE_SYSTEM_LIBS`).

## Embedded targets

This module is editor / desktop only. Embedded boards do not link it; their network stack lives in board-specific integration modules.
