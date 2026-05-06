# C++ 多线程异步日志系统 — 架构设计

## Context

从零搭建一个 C++ 多线程异步日志系统，面向初学者。核心思路：**多个业务线程产生日志 → 放入线程安全队列 → 一个后台线程取出并写入磁盘/控制台**。

## 整体架构

```
┌──────────┐    ┌──────────┐    ┌──────────┐
│ 线程 1    │    │ 线程 2    │    │ 线程 N    │
│ LOG_INFO  │    │ LOG_INFO  │    │ LOG_INFO  │
└─────┬─────┘    └─────┬─────┘    └─────┬─────┘
      │                │                │
      └────────────────┼────────────────┘
                       │ 格式化 + 入队（在调用线程完成）
                       ▼
              ┌─────────────────┐
              │  AsyncQueue     │  mutex + condition_variable
              │  (线程安全队列)   │
              └────────┬────────┘
                       │ 后台线程出队
                       ▼
              ┌─────────────────┐
              │  后台写入线程    │
              │  (只有1个)      │
              └────────┬────────┘
                       │
          ┌────────────┼────────────┐
          ▼            ▼            ▼
     ConsoleSink   FileSink     (未来: RollingFileSink)
```

**为什么在调用线程格式化：** 把格式化操作放在调用线程（而非后台线程），这样后台线程只做纯粹的磁盘 I/O，不会因为格式化成为瓶颈。字符串格式化很快，而磁盘写入慢。

## 核心模块（6 个）

### 1. LogLevel — 日志级别
- 枚举：`TRACE, DEBUG, INFO, WARN, ERROR, FATAL`
- 提供 `to_string()` 和流输出

### 2. LogMessage — 一条日志记录
- 字段：`timestamp`, `level`, `file`, `line`, `thread_id`, `message`
- 在调用线程构造，格式化后变成字符串入队

### 3. Sink — 输出目标（抽象基类）
- 纯虚接口 `write(const std::string&)` 和 `flush()`
- ConsoleSink：输出到 stdout
- FileSink：写入普通文件
- 一个 Logger 可以挂多个 Sink（比如同时输出到控制台和文件）

### 4. AsyncLogger — 核心异步引擎
- 内部持有：
  - `std::queue<std::string>` + `std::mutex` + `std::condition_variable` 组成的有界阻塞队列
  - 一个后台 `std::thread` 不断从队列取日志、写入所有 Sink
  - `std::atomic<bool>` 控制线程启停
- 主要方法：
  - `log(level, message)` → 入队
  - `start()` / `stop()` → 控制后台线程生命周期
  - `flush()` → 等待队列清空

### 5. Logger — 用户入口（单例）
- 封装一个 AsyncLogger 实例 + 多个 Sink
- 提供 `initialize(config)` 做一次性配置
- 提供 `debug()`, `info()`, `warn()`, `error()`, `fatal()` 方法

### 6. 便捷宏 — 自动捕获 `__FILE__` 和 `__LINE__`
```cpp
#define LOG_INFO(fmt, ...)  Logger::instance().log(LogLevel::INFO, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
```

## 文件结构

```
include/logger/
  LogLevel.h       — 日志级别枚举
  LogMessage.h     — 日志消息结构体
  Sink.h           — Sink 抽象基类 + ConsoleSink + FileSink
  AsyncLogger.h    — 异步日志核心（队列 + 后台线程）
  Logger.h         — 用户入口 + 宏
src/
  Sink.cpp
  AsyncLogger.cpp
  Logger.cpp
examples/
  basic_usage.cpp  — 演示用法
CMakeLists.txt     — 构建配置
```

总共约 400–500 行代码，文件不多但职责清晰。

## 关键设计点

1. **线程安全**：只用标准库的 `std::mutex` + `std::condition_variable`，适合初学者理解
2. **有界队列**：队列设上限（默认 10000 条），满了调用线程可选择阻塞等待或丢弃，防止内存暴涨
3. **优雅关闭**：`stop()` 会等待队列清空后再退出后台线程，保证日志不丢失
4. **格式化**：用 `std::format`（C++20）或手写简单的占位符替换，不引入第三方库
5. **RAII**：Logger 析构时自动 `stop()` 和 `flush()`
6. **时间戳**：用 `std::chrono` 格式化到毫秒精度

## 不做什么（避免过度设计）

- 不做 lock-free 队列（初学者理解成本高）
- 不做日志级别运行时过滤（初期用编译期宏控制更简单）
- 不做 RollingFileSink（按大小/时间切分），先做普通文件写入
- 不引入任何第三方依赖（spdlog, fmt 等），纯标准库

## 实现清单

按顺序完成：

1. **创建目录结构** — `mkdir -p include/logger src examples`
2. **LogLevel.h** — 定义 `enum class LogLevel { TRACE, DEBUG, INFO, WARN, ERROR, FATAL }` + `to_string()` + `operator<<`
3. **LogMessage.h** — 结构体：`time_point`, `level`, `file`, `line`, `thread_id`, `message` + `format()` 方法
4. **Sink.h + Sink.cpp** — 抽象基类 `Sink`（`write()` + `flush()` 纯虚函数），`ConsoleSink` 写 stdout，`FileSink` 写文件
5. **AsyncLogger.h + AsyncLogger.cpp** — 核心模块：
   - `std::queue<std::string>` + `std::mutex` + `std::condition_variable`
   - 一个 `std::thread` 后台线程，循环 pop → 写入所有 Sink
   - `log(msg)`：加锁 → push → notify
   - `start()` / `stop()` + `flush()`
   - 队列上限 10000，满了可选择阻塞或丢弃
6. **Logger.h + Logger.cpp** — 单例 + 宏：
   - `static Logger& instance()` 返回引用
   - 持有 `AsyncLogger` + `std::vector<std::unique_ptr<Sink>>`
   - 宏 `LOG_INFO(...)`, `LOG_WARN(...)` 等，自动填入 `__FILE__`, `__LINE__`
7. **CMakeLists.txt** — C++17/20，静态库 `logger_lib` + 可执行文件 `basic_usage`
8. **examples/basic_usage.cpp** — 两个测试：单线程基本用法 + 10 线程各 100 条日志
9. **构建验证**：
   ```bash
   mkdir build && cd build && cmake .. && make
   ./examples/basic_usage
   ```

## 学习重点

第 5 步 AsyncLogger 是最难也最有价值的部分：
- `condition_variable::wait` 的正确用法（用 lambda 做谓词检查）
- 锁的粒度：哪些操作需要加锁，哪些不需要
- `stop()` 时如何安全唤醒后台线程、等待队列清空
- `unique_lock` vs `lock_guard` 的使用场景
- `atomic<bool>` 在线程间传递停止信号
