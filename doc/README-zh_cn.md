# NeverJudge

[English](../README.md) | [简体中文](./README-zh_cn.md)

![NeverJudge Logo](./img/logo.png)
<p align="center">评测！评测！以及评测！</p>

![Qt6](https://img.shields.io/badge/C%2B%2B-Qt6-green) ![GitHub Stars](https://img.shields.io/github/stars/LeoDreamer2004/Never-Judge) ![GitHub License](https://img.shields.io/github/license/LeoDreamer2004/Never-Judge) ![GitHub Releases](https://img.shields.io/github/v/release/LeoDreamer2004/Never-Judge)

NeverJudge 是一个集成了 OpenJudge 远程功能的简易代码编辑器，为 2025 年春季学期程序设计实习课程作业设计。

## 如何使用

### 依赖项

本项目使用 CMake 构建，支持 MacOS 和 Linux 系统，并遵循 C++23 标准。

必要依赖项包括：

- `Qt6`：项目使用的框架
- `QTermWidget6`：终端集成
- `QCoro6`：Qt 协程支持
- `tree-sitter`：语法高亮基础

可选依赖项包括：

- `tree-sitter-cpp`，`tree-sitter-python`：语法高亮支持
- `clangd`，`pylsp`：语言服务器协议支持
- Python 的 `requests` 库和 `BeautifulSoup` 库：OpenJudge 远程支持

### 构建

1. 克隆仓库：

    ```bash
    git clone https://github.com/LeoDreamer2004/Never-Judge.git
    cd Never-Judge
    ```

2. 在项目根目录下执行以下 CMake 命令：

    ```bash
    cmake . -B build
    cmake --build build
    ```

3. 构建完成后，`build` 目录下会生成 `NeverJudge` 可执行文件，直接运行即可。
4. 程序配置文件在系统默认配置目录下：
   - Linux：`~/.config/never-judge`
   - MacOS：`~/Library/Application Support/never-judge`

## 功能特性

- [x] 动态代码修改检测
- [x] 图标工具栏
- [x] 用户偏好设置
- [x] OpenJudge 远程提交与个性化
- [x] 提交后结果反馈
- [x] 代码运行配置
- [x] Deepseek Coder AI 助手
- [x] 语法高亮（由 [tree-sitter](https://tree-sitter.github.io/tree-sitter/) 支持）
- [x] 括号匹配高亮（部分实现，当前架构下难以完全实现）和快速注释
- [x] 基于语言服务器协议的高级代码分析（由 [Clangd](https://clangd.llvm.org/) 和 [PyLSP](https://github.com/python-lsp/python-lsp-server) 支持）

## 文档

更多信息请参考 [项目报告](./report.md)。

查看 [B站视频](https://www.bilibili.com/video/BV1Wy7FzNEF3/) 获取小组展示。

## 许可证

GNU 通用公共许可证 v3.0 (GPL-3.0)
