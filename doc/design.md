# NeverJudge 项目功能设计文档

## 1. 项目概述

NeverJudge 是一个专门为方便同学写OJ题目开发的代码编辑器，提供基本的代码高亮和补全功能，并允许用户登录后把代码提交到 OJ 服务器。该项目使用 C++ 和 Qt6 框架开发。

## 2. 技术栈

- 核心语言：C++ 23
- GUI框架：Qt6
- 依赖组件：
  - `Qt6::Widgets`：基础GUI组件
  - `Qt6::Svg`：矢量图形支持
  - `Qt6::Network`：网络功能支持
  - `QTermWidget6`：终端模拟器
  - `QCoro6`：协程支持
  - `tree-sitter`：代码解析和语法高亮

## 3. 核心功能模块

### 3.1 编辑器核心（ide）

- 语言支持（`language.cpp`）
- 项目管理（`project.cpp`）
- 命令执行（`cmd.cpp`）
- IDE核心功能（`ide.cpp`）
- 代码高亮（`highlighter.cpp`）
- LSP 支持（`lsp.cpp`）

### 3.2 界面组件（widgets）

- 主窗口（`window.cpp`）
- 设置界面（`setting.cpp`）
- 图标管理（`icon.cpp`）
- 状态栏（`footer.cpp`）
- 图标导航栏（`iconNav.cpp`）
- OJ 题目预览（`preview.cpp`）
- 代码编辑器（`code.cpp`）
- 文件树（`fileTree.cpp`）
- 终端（`terminal.cpp`）
- 菜单系统（`menu.cpp`）

### 3.3 Web功能（web）

- 网络爬虫（`crawl.cpp`）
- 数据解析（`parse.cpp`）

### 3.4 工具类（util）

- 文件操作（`file.cpp`）
- Python 脚本执行（`script.cpp`）

## 4. 主要功能特性

### 4.1 已实现功能

- ✅ 图标工具栏
- ✅ 动态代码修改检测
- ✅ 用户偏好设置
- ✅ OpenJudge在线评测集成
- ✅ 提交结果反馈（AC/WA状态）
- ✅ 文件运行配置系统
- ✅ 二进制文件处理
- ✅ 语法解析高亮
- ✅ 括号配对高亮（部分完成）
- ✅ LSP 有关支持

### 4.2 待实现功能

- ⏳ `Python.h` 集成（替代进程调用方式）
- ⏳ 开发过程中继续补充

## 5. 特殊功能说明

### 5.1 调试模式特性

- 临时文件缓存清理
- 配置重置功能

### 5.2 资源管理

- 使用Qt资源系统（resource.qrc）
- 支持SVG图标

## 6. 项目结构

```
NeverJudge/
├── CMakeLists.txt      # 项目构建配置
├── main.cpp            # 程序入口
├── ide/               # IDE核心功能
├── widgets/           # GUI组件
├── web/               # 网络相关功能
├── util/              # 工具类
└── res/               # 资源文件
```
