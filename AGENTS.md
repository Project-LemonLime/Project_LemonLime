# AGENTS.md

## Project Overview

Project LemonLime 是一个面向 OI（信息学奥林匹克）竞赛的轻量级评测系统，基于 Lemon + LemonPlus 开发。支持 Linux、Windows、macOS 三平台。

- **Qt 版本**: Qt 6.8 或更高（可通过 `-DLEMON_QT_MAJOR_VERSION=<6|7>` 指定主版本）
- **Qt 模块**: Core, Gui, Widgets（核心），LinguistTools（翻译），AxContainer（仅 Windows XLS 导出）
- **C++ 标准**: C++17（`CMAKE_CXX_STANDARD 17`，无扩展）
- **第三方依赖**: SingleApplication（单实例保护），spdlog（日志系统），均作为 git submodule 在 `3rdparty/` 下
- **许可证**: GPL-3.0-or-later

## Build & Run

### 依赖

- CMake ≥ 3.16
- Qt 6.8+（需要 Core, Gui, Widgets, LinguistTools 模块）
- C++17 兼容的编译器（MSVC / GCC / Clang）
- Ninja（推荐）或 Make

### 克隆

```bash
git clone https://github.com/Project-LemonLime/Project_LemonLime.git --recursive
```

### 构建（通用）

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release -GNinja
cmake --build build --parallel
```

### CMake 选项

| 选项 | 默认值 | 说明 |
|------|--------|------|
| `EMBED_TRANSLATIONS` | `ON` | 嵌入翻译文件到二进制 |
| `EMBED_DOCS` | `ON` | 嵌入手册文档 |
| `ENABLE_XLS_EXPORT` | `OFF` | XLS 导出支持（仅 Windows） |
| `ENABLE_LTO` | `ON` | 链接时优化 |
| `ENABLE_CCACHE` | `OFF` | ccache 加速编译 |
| `BUILD_DEB` | `OFF` | 构建 DEB 包 |
| `BUILD_RPM` | `OFF` | 构建 RPM 包 |
| `LEMON_QT_MAJOR_VERSION` | `6` | Qt 主版本号 |
| `LEMON_QT_MIN_VERSION` | `6.8` | 最低 Qt 版本 |
| `LEMON_BUILD_INFO` | 空 | 自定义构建信息字符串 |
| `LEMON_CONFIG_DIR` | 空 | 自定义配置目录 |

### 版本信息

版本号存储在 `makespec/VERSION`（如 `0.3.6`），后缀存储在 `makespec/VERSIONSUFFIX`（如 `.1`）。构建版本自动取 git short hash。

## Architecture

### 目录结构

```
Project_LemonLime/
├── src/
│   ├── base/           # 基础设施层（静态库 lemon-base）
│   ├── core/           # 核心业务逻辑（静态库 lemon-core，依赖 lemon-base）
│   ├── component/      # 组件（exportutil 导出工具）
│   ├── forms/          # Qt Designer .ui 文件（26 个）
│   ├── main.cpp        # 入口
│   ├── pch.h           # 预编译头（QtCore + QtGui）
│   ├── lemon.h/.cpp    # 主窗口 LemonLime : QMainWindow
│   └── *.h/*.cpp       # UI 层各对话框和控件
├── 3rdparty/
│   ├── SingleApplication/  # 单实例应用
│   └── spdlog/             # 日志库
├── cmake/              # CMake 模块文件
│   ├── lemon-base.cmake
│   ├── lemon-core.cmake
│   ├── lemon-ui.cmake
│   ├── LemonDocs.cmake
│   ├── LemonTranslations.cmake
│   ├── deployment.cmake
│   └── platforms/      # 平台安装配置
├── translations/       # 翻译文件（en_US, zh_CN, zh_TW）
├── assets/             # 图标、图片资源
├── manual/             # 用户手册
├── unix/               # Unix 平台 watcher 程序
├── makespec/           # 版本号、打包 spec 文件
└── .github/workflows/  # CI 工作流
```

### 三层架构

```
┌─────────────────────────────┐
│       UI Layer (exe)        │  主窗口、对话框、自定义控件
│  LemonLime, JudgingDialog,  │  直接链接 lemon-core, lemon-base
│  ResultViewer, SummaryTree  │
├─────────────────────────────┤
│     Core Layer (lemon-core) │  评测逻辑、比赛/选手/题目管理
│  Contest, Task, Contestant, │
│  JudgingThread, TaskJudger, │
│  JudgingController          │
├─────────────────────────────┤
│    Base Layer (lemon-base)  │  编译器抽象、全局设置、工具函数
│  Settings, Compiler,        │  日志、配置、翻译、JSON 工具
│  LemonLog, LemonConfig      │
└─────────────────────────────┘
```

### 核心数据模型

```
Contest (QObject)
├── Settings*
├── QList<Task*>
│   └── Task (QObject)
│       ├── QList<TestCase*>
│       │   └── TestCase (非 QObject)
│       └── TaskType: Traditional | AnswersOnly | Interaction | Communication
└── QMap<QString, Contestant*>
    └── Contestant (QObject)
        └── 存储每题的编译状态、评测结果、得分、时间、内存
```

### 评测流程

1. `Contest::judge()` 创建 `TaskJudger` 并提交给 `JudgingController`
2. `JudgingController` 管理线程池（`QThread` + `QQueue`），按 `maxJudgingThreads` 并行评测
3. `TaskJudger` 编译源代码，然后为每个测试点创建 `JudgingThread`
4. `JudgingThread` 继承 `QThread`，负责运行程序并对比输出
5. 结果通过信号链逐级回传至 UI

### Unix Watcher

在 Linux/macOS 平台，会编译一个单独的 `watcher_unix` 可执行文件（包含 `watcher_unix.cpp` + 平台相关的 `watcher_linux.cpp` 或 `watcher_macos.mm`），用于监控被评测程序的资源使用（时间/内存），嵌入 `watcher.qrc` 资源到主程序。

## Qt Conventions

### 信号槽风格

**100% 使用新式（C++11 函数指针）连接**，项目中无任何 `SIGNAL()`/`SLOT()` 宏的使用。

```cpp
// 典型连接方式（摘自 taskeditwidget.cpp）
connect(ui->problemTitle, &QLineEdit::textChanged, this, &TaskEditWidget::problemTitleChanged);
connect(ui->comparisonMode, qOverload<int>(&QComboBox::currentIndexChanged), this,
        &TaskEditWidget::comparisonModeChanged);
```

对于有重载的信号，使用 `qOverload<>()` 消歧。

### 内存管理

- **Qt 对象树机制**为主：大部分 QObject 派生类通过 `parent` 参数管理生命周期
- `JudgingController` 中手动 `new QThread` + `delete thread` / `delete taskJudger`
- `LemonTranslator` 使用 `std::unique_ptr<QTranslator>`
- `spdlog::logger` 使用 `std::shared_ptr`
- `TestCase` 不继承 QObject，存储在 `QList<TestCase*>` 中

```cpp
// 手动线程管理示例（摘自 judgingcontroller.cpp）
QThread *thread = new QThread;
taskJudger->moveToThread(thread);
// ...
thread->quit();
thread->wait();
delete thread;
delete taskJudger;
```

### 多线程

- `JudgingThread` 继承 `QThread`（重写 `run()`）
- `JudgingController` 使用 **moveToThread 模式**：创建 `QThread` → `moveToThread()` → `QMetaObject::invokeMethod()` 调用
- 通过 `maxJudgingThreads` 设置控制最大并发线程数

### 资源管理

- `resource.qrc`：图标、logo、版本文件等
- `translations/translations.qrc`：翻译文件（可嵌入或外置）
- `manual/manual.qrc`：用户手册（可嵌入）
- `unix/watcher.qrc`：watcher 二进制（仅 Unix）

### 国际化

- 全面使用 `tr()` 和 `QObject::tr()` 进行翻译标记
- 翻译文件：`translations/zh_CN.ts`、`translations/zh_TW.ts`、`translations/en_US.ts`
- `LemonTranslator` 类管理翻译加载，搜索多个路径（含 Snap/AppImage 支持）
- 非 QObject 类使用 `Q_DECLARE_TR_FUNCTIONS` 宏（如 `Settings`）
- CMake 通过 `qt_add_translation()` 编译 `.ts` → `.qm`

## C++ Code Style

### 格式化

使用 `.clang-format` 配置（CI 中强制检查）：

```yaml
BasedOnStyle: LLVM
BreakBeforeBraces: Attach
IndentWidth: 4
TabWidth: 4
UseTab: ForIndentation       # 使用 Tab 缩进
ColumnLimit: 110
ContinuationIndentWidth: 4
IndentCaseLabels: true
NamespaceIndentation: All    # 命名空间内容也缩进
SpaceAfterLogicalNot: true   # "! expr" 而非 "!expr"
```

### 头文件保护

**100% 使用 `#pragma once`**，无传统 include guard。

### 命名约定

- **类名**: PascalCase（`JudgingThread`, `TaskJudger`, `LemonBaseApplication`）
- **成员变量**: camelCase，**无前缀**（`contestTitle`, `compilerList`, `isJudging`）
- **方法名**: camelCase（`getContestTitle()`, `setCompilerType()`, `judgeAll()`）
- **枚举值**: PascalCase（`CompileSuccessfully`, `TimeLimitExceeded`, `Traditional`）
- **命名空间**: PascalCase（`Lemon::base::config`，`Lemon::detail`，`Lemon::common`）
- **宏/常量**: 全大写或 PascalCase（`LEMON_MODULE_NAME`, `MagicNumber`, `maxDependValue`）
- **Getter/Setter**: `getXxx()` / `setXxx()` 风格

### 前向声明

广泛使用前向声明减少头文件依赖：

```cpp
// 摘自 contest.h
class Task;
class Settings;
class Contestant;
class JudgingController;
```

### 命名空间

项目使用 `Lemon` 作为顶级命名空间，子空间包括 `Lemon::base`、`Lemon::base::config`、`Lemon::detail`、`Lemon::common`。旧代码中的核心类（`Contest`, `Task` 等）不在命名空间内。

### UI 命名空间

Qt Designer 生成的类放在 `Ui` 命名空间中：

```cpp
namespace Ui {
    class LemonLime;
}
```

### const 正确性

Getter 方法一致使用 `const` 修饰，但返回引用时使用 `const Type&`：

```cpp
const QString &getContestTitle() const;
const QList<Task *> &getTaskList() const;
```

### 构造函数

QObject 派生类统一使用 `explicit` 修饰和默认 `parent = nullptr` 参数：

```cpp
explicit Contest(QObject *parent = nullptr);
explicit JudgingThread(QObject *parent = nullptr);
```

### QT_NO_FOREACH

项目定义了 `-DQT_NO_FOREACH`，禁止使用 Qt 的 `foreach` 宏，统一使用标准 C++ 范围 for 循环。

## Key Patterns

### JSON 序列化

所有数据模型类实现 `read(const QJsonObject&)` / `write(QJsonObject&) const` 方法对，通过 `LemonUtils.hpp` 中的模板工具函数实现：

```cpp
// LemonUtils.hpp 提供的宏（摘自实际代码）
#define READ_JSON(json, x) Lemon::readJson(x, #x, json)
#define WRITE_JSON(json, x) Lemon::writeJson(x, #x, json)
```

底层使用 SFINAE 模板 (`std::enable_if_t`) 支持多种类型（`int`, `bool`, `double`, `QString`, `QJsonArray`, `QList<T>`, 自定义枚举等）的自动序列化/反序列化。

### 日志系统

基于 spdlog，通过模块化宏封装：

```cpp
// 每个 .cpp 文件开头定义模块名
#define LEMON_MODULE_NAME "JudgingController"

// 使用宏打日志
LOG("Starting judge for", contestantName);
WARN("Compile failed:", message);
DEBUG("Score:", score, "Time:", timeUsed);
```

日志同时输出到 console（warn 级别以上）和 daily file（trace 级别）。日志文件保留最近 30 天。

### 预编译头

`src/pch.h` 包含 `<QtCore>` 和 `<QtGui>`，在所有三个编译目标（`lemon-base`, `lemon-core`, 主 `lemon`）中使用。

### 应用初始化

```cpp
// main.cpp 中的启动流程
initLogger();                          // 初始化 spdlog
Lemon::LemonBaseApplication app(...);  // 继承 SingleApplication（单实例）
app.Initialize();                      // 解析命令行、初始化翻译
if (app.sendMessage("")) { ... }       // 已有实例则激活已有窗口
LemonLime w;                           // 创建主窗口
screen.show(); ... screen.finish(&w);  // 启动画面
w.show(); w.welcome();                 // 显示欢迎对话框
```

### UI 结构

- `.ui` 文件定义在 `src/forms/` 目录（26 个文件）
- CMake 自动处理 `AUTOUIC`（搜索路径设为 `src/forms`）、`AUTOMOC`、`AUTORCC`
- UI 类以 `Ui::ClassName` 命名空间持有指针

### 评测结果状态机

```cpp
// 摘自 LemonType.hpp
enum ResultState {
    CorrectAnswer, WrongAnswer, PartlyCorrect,
    TimeLimitExceeded, MemoryLimitExceeded,
    CannotStartProgram, FileError, RunTimeError,
    InvalidSpecialJudge, SpecialJudgeTimeLimitExceeded,
    SpecialJudgeRunTimeError, Skipped,
    InteractorError, PresentationError, OutputLimitExceeded,
    LastResultState
};
```

### 子任务依赖

`subtaskdependencelib.h` 中定义了子任务依赖得分计算逻辑（`stateToStatus` / `statusToScore`），使用百万分比精度（`maxDependValue = 1000000`）。

## Testing

- **当前状态**: 项目 `tests/` 目录为空，`TODO` 文件中标注 "Need Test"
- **无测试框架**: 目前未集成任何测试框架
- `unix/test/` 目录包含 watcher 相关的测试 CMakeLists 和测试程序

## Important Notes

### 平台兼容性

- **Windows**: 推荐从源码构建而非直接使用 Release 二进制，以确保时间/内存检测功能正常。CI 使用 MSVC。
- **macOS**: 需使用 `watcher_macos.mm`（Objective-C++）编译 watcher，否则内存限制功能异常。Apple Silicon 不保证评测稳定性。
- **Linux**: 默认栈空间与内存限制相同。watcher 使用 `watcher_linux.cpp`。静态编译为推荐分发方式。

### 禁止的写法

- 不得使用 `foreach` 宏（已通过 `QT_NO_FOREACH` 编译定义禁止）
- 不得使用旧式 `SIGNAL()`/`SLOT()` 宏连接信号槽
- 代码必须通过 `clang-format` 检查（CI 自动执行）

### Commit 规范

使用 [Conventional Commits](https://www.conventionalcommits.org/) 规范。

**格式**：

```
<type>(<scope>): <subject>

<body>
```

**type 必须是以下之一**：

| type | 说明 |
|------|------|
| `feat` | 新功能 |
| `fix` | 修复 bug |
| `refactor` | 重构（不改变功能） |
| `style` | 代码格式调整（不影响逻辑） |
| `docs` | 文档变更 |
| `build` | 构建系统或依赖变更 |
| `ci` | CI 配置变更 |
| `perf` | 性能优化 |
| `test` | 测试相关 |
| `chore` | 其他杂项 |

**规则**：

- **首行（subject line）不得超过 72 个字符**，保持简短概括
- `scope` 可选，用于标注影响范围（如 `feat(judging):`、`fix(export):`）
- 首行使用英文小写开头，不加句号
- 详细说明放到 body 中（空一行后书写），或在代码注释中说明
- 破坏性变更需在 body 中添加 `BREAKING CHANGE:` 前缀
- **AGENTS.md 维护**：当代码中涉及本文档描述的约定、架构、构建方式、代码风格等内容发生变更时，须同步更新 `AGENTS.md`
- **Commit 拆分**：`AGENTS.md` 的变更必须作为**独立 commit** 提交，不得与其他代码或文档变更混在同一个 commit 中（便于 cherry-pick 到 master 分支）

**示例**：

```
feat(judging): add subtask dependency support

Implement subtask dependency checking in TaskJudger.
Each test case can now specify dependent subtasks that
must pass before it is evaluated.
```

```
fix(export): correct HTML encoding for CJK characters
```

```
refactor: extract JSON serialization helpers to LemonUtils
```

### CI 工作流

| 工作流 | 说明 |
|--------|------|
| `windows-qt6.yml` | Windows MSVC + Qt 6.9.3 构建 |
| `linux-static-qt6.yml` | Linux 静态链接 Qt6 构建 |
| `macos-qt6.yml` | macOS Qt6 构建 |
| `cpack-deb-debian.yml` | Debian DEB 包构建 |
| `check_format.yml` | 代码格式检查（clang-format） |

### 文件编码

- 源文件使用 CRLF 行尾（Windows 风格，通过 `.gitattributes` 管理）
- 注释和文档包含中文

### SPDX 许可证头

所有源文件以 SPDX 格式的许可证头开始：

```cpp
/*
 * SPDX-FileCopyrightText: 2019-2022 Project LemonLime
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */
```
