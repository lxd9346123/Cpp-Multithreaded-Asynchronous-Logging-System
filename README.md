# C++ Async Logging System

A header-only, lightweight C++17 async logging library. Producer-consumer model, thread-safe, colorized console output with plain-text file persistence.

## Quick Start

Copy the two headers under `include/` into your project, then `#include "Logger.h"`:

```cpp
#include "Logger.h"

int main() {
    mLOG_INFO("server started on port %d", 8080);
    mLOG_WARN("retry %d/%d", 3, 5);
    mLOG_ERROR("connection refused: %s", "timeout");
    return 0;
}
```

Requires C++17:

```bash
g++ -std=c++17 -I include/ main.cpp -o main -lpthread
```

## Log Levels

| Macro | Level | Console Color |
|-------|-------|---------------|
| `mLOG_DEBUG` | Debug | Cyan |
| `mLOG_INFO` | Info | Green |
| `mLOG_WARN` | Warning | Yellow |
| `mLOG_ERROR` | Error | Red |

`mLOG_DEBUG` is disabled by default. Define `DEBUG_ENV` to enable it:

```bash
g++ -std=c++17 -DDEBUG_ENV -I include/ main.cpp -o main -lpthread
```

## Output

- **Console** — ANSI colorized, with timestamp and level label.
- **File** — Plain text, auto-created under `log/` directory, named `YYYY-MM-DD_HH-MM-SS.log`.