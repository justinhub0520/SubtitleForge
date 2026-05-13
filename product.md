# SubForge - 视频字幕生成与剪辑工具 产品设计文档

> **项目代号**: SubForge
> **目标用户**: 视频创作者、字幕翻译人员
> **技术定位**: C++ 实习简历项目，展示 C++ 后端工程 + Qt GUI + 前后端分离架构能力
> **日期**: 2026-04-28

## 1. 项目概述

SubForge 是一款视频字幕生成与剪辑桌面工具，支持上传视频、自动生成字幕、时间轴编辑、字幕样式调整、导出带字幕的视频。全功能端到端可用，界面专业，交互完整。

**核心价值链**:

```
上传视频 → 提取音频 → DashScope 大模型 (fun-asr) 生成字幕 → 时间轴可视化编辑 → 样式调整 → FFmpeg 烧录导出
```

## 2. 技术栈

| 层级 | 技术 | 说明 | 平台 |
|------|------|------|------|
| **前端 GUI** | Qt 6 (QWidget) | 桌面应用，自定义组件 | **Windows** |
| **视频播放** | QVideoWidget + QMediaPlayer | Qt 多媒体模块 | **Windows** |
| **HTTP 客户端** | QNetworkAccessManager | Qt 网络模块，与后端通信 | **Windows** |
| **后端 HTTP 服务** | cpp-httplib | 轻量级 C++ HTTP 库，单头文件引入 | **Linux** |
| **语音识别** | 阿里云 DashScope (fun-asr) | 大模型语音识别 API，后端转发调用 | **Linux** |
| **视频处理** | FFmpeg (C++ 调用) | 音频提取、字幕烧录、视频编码导出 | **Linux** |
| **数据交换** | JSON (nlohmann/json) | 前后端统一数据格式 | **Win + Linux** |
| **构建系统** | CMake | 前后端独立构建管理 | **Win + Linux** |
| **配置管理** | .env 文件 | API Key 等敏感配置，不硬编码 | **Win + Linux** |

## 3. 系统架构

### 3.1 整体架构（方案 B：Qt 前端 + C++ HTTP 后端分离）

```
┌──────────────────────────────────────┐
│           Qt Desktop App (前端)       │
│  ┌──────────────────────────────────┐│
│  │ MainWindow                       ││
│  │ ┌────────────┬──────────────────┐││
│  │ │VideoPlayerWidget│TimelineEditor  │││
│  │ │(QVideoWidget)│(自定义QWidget) │││
│  │ └────────────┴──────────────────┘││
│  │ SubtitleListWidget               ││
│  │ StyleEditorWidget                ││
│  └──────────────────────────────────┘│
│  ApiClient (QNetworkAccessManager)   │
└──────────────┬───────────────────────┘
               │ REST API (JSON)
               │ http://<linux-server-ip>:8080
┌──────────────┴───────────────────────┐
│        C++ HTTP Backend (后端)        │
│  (cpp-httplib)                       │
│  ┌──────────────────────────────────┐│
│  │ POST /api/videos/upload    → 接收视频 ││
│  │ POST /api/tasks/transcribe → 转写任务 ││
│  │ GET  /api/tasks/:task_id   → 任务进度 ││
│  │ POST /api/tasks/export     → FFmpeg  ││
│  │ GET  /api/videos/:id/download → 下载  ││
│  │ GET  /api/videos/:id/subtitles→字幕  ││
│  └──────────────────────────────────┘│
└──────────────────────────────────────┘
```

### 3.2 架构选择理由

| 对比项 | 方案A(Qt全栈本地) | 方案B(Qt+HTTP分离) | 方案C(Qt+gRPC) |
|--------|-------------------|--------------------|-----------------|
| UI 阻塞风险 | 高(重处理同进程) | 低(前后端分离) | 低 |
| 工程复杂度 | 低 | 中 | 高 |
| 简历亮点 | C++/Qt 全栈 | C++后端+Qt+REST设计 | 过度设计 |
| 可扩展性 | 低 | 高 | 高 |
| **选择** | ❌ | ✅ | ❌ |

选择方案 B 的核心原因：
- FFmpeg 编码和 DashScope 转写调用耗时较长，分离后端避免 UI 卡顿
- REST API 设计体现前后端分离工程素养，简历加分
- 后端可独立测试，便于调试和迭代
- **Linux 后端 + Windows 前端跨平台部署**，简历上体现跨平台 C++ 开发能力

### 3.3 跨平台部署架构

```
┌─────────────────────────────────┐     ┌──────────────────────────────────────┐
│  Windows 开发机 (前端运行环境)    │     │  VMware Ubuntu 20.04 (后端运行环境)   │
│                                 │     │  IP: 192.168.199.132                  │
│  Qt Desktop App                 │─────│  C++ HTTP Backend (cpp-httplib)      │
│  - 视频播放、时间轴编辑           │ HTTP│  - 视频接收、音频提取(FFmpeg)        │
│  - 样式调整、导出设置             │ REST│  - DashScope 转写                  │
│  - 字幕数据管理(本地)             │ :8080│  - FFmpeg 字幕烧录导出              │
│                                 │     │  - 任务进度管理                      │
│  视频文件本地存储                 │     │  视频文件服务器存储                   │
│  字幕数据本地 JSON               │     │  临时文件 /tmp/subforge/            │
└─────────────────────────────────┘     └──────────────────────────────────────┘
```

**关键设计决策**：
- 前端(Windows)负责所有 UI 交互和字幕编辑，视频文件在本地播放
- 后端运行在 VMware Ubuntu 20.04 虚拟机 (IP: 192.168.199.132)，负责视频处理（音频提取、字幕烧录）
- 前端上传视频文件到后端 (http://192.168.199.132:8080)，后端处理完成后前端下载导出结果
- 开发阶段：后端运行在 VMware Ubuntu 20.04 虚拟机
- 部署阶段：后端部署在 Linux 服务器，前端打包为 Windows 安装包

## 4. 功能模块设计

### 4.1 功能模块总览

| 模块 | 前端(Qt) | 后端(C++) | 说明 |
|------|----------|-----------|------|
| 1. 视频管理 | 文件选择、视频预览、播放控制 | `/api/videos/upload` 接收视频、提取音频 | QVideoWidget 播放，后端 FFmpeg 提取音频 |
| 2. 字幕生成 | 提交任务、进度显示、结果预览 | `/api/tasks/transcribe` 调用 DashScope (fun-asr) | 支持中英文，返回带时间戳的字幕数据 |
| 3. 时间轴编辑 | 可视化时间轴、拖拽字幕块、逐句编辑 | 纯前端操作(无需后端) | 自定义 QWidget，字幕块可拖拽缩放 |
| 4. 字幕样式 | 字体、大小、颜色、位置、背景、实时预览 | `/api/tasks/export` 时应用样式到 FFmpeg 参数 | 预设模板 + 自定义 |
| 5. 视频导出 | 导出设置、进度条 | `/api/tasks/export` FFmpeg 硬字幕烧录 | MP4 输出，进度实时推送 |

### 4.2 各模块详细设计

#### 模块1: 视频管理

**前端组件**: `VideoPlayerWidget` (基于 QVideoWidget + QMediaPlayer)
- 支持点击选择打开本地视频文件
- 视频播放器：播放/暂停/停止、进度联动
- 播放时同步高亮当前字幕

**后端接口**:
- `POST /api/videos/upload`：接收视频文件(multipart/form-data)，提取音频(WAV格式)，返回 video_id 和 video_info
- 后端使用 FFmpeg 命令 `ffmpeg -i input.mp4 -vn -acodec pcm_s16le audio.wav` 提取音频

**数据模型**:
```json
{
  "video_id": "uuid-string",
  "video_info": {
    "file_path": "/tmp/subforge/uuid/input.mp4",
    "audio_path": "/tmp/subforge/uuid/audio.wav",
    "duration": 120.5,
    "resolution": { "width": 1920, "height": 1080 },
    "fps": 30,
    "format": "mp4",
    "size_mb": 85.3
  }
}
```

#### 模块2: 字幕生成

**前端组件**: MainWindow 内置 Generate Subtitles 功能
- 上传视频后点击 "Generate Subtitles" → 调用 `/api/tasks/transcribe`
- 通过轮询 `/api/tasks/:task_id` 获取进度
- 生成完成后字幕数据加载到时间轴

**后端接口**:
- `POST /api/tasks/transcribe`：参数 `{video_id, language}`, 调用阿里云 DashScope 语音识别 API (fun-asr 模型)
- DashScope 转写调用流程：
  1. 读取提取的音频文件
  2. 通过 `/usr/bin/curl` 调用 `https://dashscope.aliyuncs.com/api/v1/files` 上传音频
  3. 提交异步转写任务至 `https://dashscope.aliyuncs.com/api/v1/services/audio/asr/transcription`
  4. 轮询 `/api/v1/tasks/{task_id}` 等待转写完成
  5. 解析返回的 SRT/JSON 格式结果，转换为内部字幕数据结构
- `GET /api/tasks/:task_id`：返回 `{task_id, progress: 0-100, status: "pending/processing/done/error"}`

**字幕数据结构**:
```json
{
  "subtitles": [
    {
      "id": 1,
      "start_time": 0.5,
      "end_time": 3.2,
      "text": "大家好，欢迎来到我的频道",
      "style": {
        "font_family": "Microsoft YaHei",
        "font_size": 24,
        "color": "#FFFFFF",
        "bg_color": "#00000080",
        "position": "bottom_center",
        "bold": false,
        "italic": false,
        "outline_width": 2,
        "outline_color": "#000000"
      }
    }
  ]
}
```

**字幕样式预设值**（style 字段的默认值）:
- `font_family`: "Microsoft YaHei"（中文默认），"Arial"（英文默认）
- `font_size`: 24
- `color`: "#FFFFFF"（白色）
- `bg_color`: "#00000080"（半透明黑色背景）
- `position`: "bottom_center"（底部居中）
- `bold`: false
- `italic`: false
- `outline_width`: 2
- `outline_color`: "#000000"（黑色描边）

#### 模块3: 时间轴编辑

**前端组件**: `TimelineEditor` (自定义 QWidget)
- 水平时间轴，时间刻度标注(每秒/每5秒/每10秒自适应)
- 字幕块渲染为彩色矩形条，宽度对应时间长度
- 交互操作：
  - 拖拽字幕块：调整起止时间
  - 拖拽字幕块边缘：缩放时长
  - 双击字幕块：弹出文本编辑框
  - 右键菜单：删除、拆分、合并相邻字幕
  - 点击空白区域：添加新字幕块
- 视频播放进度指示线（红色竖线），同步联动
- 缩放控制：放大/缩小时间轴精度
- 撤销/重做支持(QUndoCommand)

**纯前端操作**: 时间轴编辑不涉及后端调用，所有修改在本地内存中完成，导出时才将最终字幕数据发送给后端

**键盘快捷键**:
- `Space`: 播放/暂停
- `Ctrl+Z`: 撤销
- `Ctrl+Y`: 重做
- `Delete`: 删除选中字幕
- `Ctrl+S`: 保存字幕数据到本地 JSON 文件（保存到 `~/.subforge/projects/{video_id}/subtitles.json`，下次打开同一视频自动加载历史字幕）

#### 模块4: 字幕样式

**前端组件**: `StyleEditorWidget` (QWidget)
- 字体选择下拉框(QFontComboBox)
- 字体大小选择(QSpinBox: 8-72)
- 位置选择(top_center / middle_center / bottom_center)
- 加粗/斜体切换
- 颜色选择（待添加）
- 描边调节（待添加）

**样式应用方式**: 导出时将样式参数转换为 FFmpeg subtitles filter 的参数：
```
ffmpeg -i input.mp4 -vf "subtitles=sub.srt:force_style='FontName=Microsoft YaHei,FontSize=24,PrimaryColour=&H00FFFFFF,OutlineColour=&H00000000,Outline=2,Alignment=2'" output.mp4
```

#### 模块5: 视频导出

**前端组件**: MainWindow 集成导出功能
- "Export Video with Subtitles" 菜单项 → 弹出 `QFileDialog::getSaveFileName` 选择保存路径
- 默认保存至系统 Videos 文件夹
- 点击保存后前端调用 `POST /api/tasks/export` → 等待响应 → 调用 `GET /api/videos/:video_id/download` 下载

**后端接口**:
- `POST /api/tasks/export`：参数 `{video_id}`
  - 后端流程：
    1. 根据 video_id 定位视频文件和字幕 SRT 文件
    2. 构建 FFmpeg 命令 `ffmpeg -y -i video.mp4 -vf subtitles=sub.srt -c:a copy output.mp4`
    3. 执行 FFmpeg 烧录编码（同步阻塞）
    4. 返回 `{file_path}` 供前端下载
- `GET /api/videos/:video_id/download`：返回导出的视频文件流（`Content-Type: video/mp4`），前端写入用户指定路径

## 5. REST API 详细规格

| Endpoint | Method | 请求体 | 响应体 | 说明 |
|----------|--------|--------|--------|------|
| `/api/videos/upload` | POST | multipart/form-data (video file) | `{video_id, video_info}` | 上传视频，提取音频 |
| `/api/tasks/transcribe` | POST | `{video_id, language}` | `{task_id}` | 提交 DashScope 转写任务 |
| `/api/tasks/:task_id` | GET | - | `{task_id, progress, status}` | 查询转写任务进度 |
| `/api/videos/:video_id/subtitles` | GET | - | `[{id, start_time, end_time, text, style}]` | 获取字幕数据 |
| `/api/videos/:video_id/audio` | GET | - | audio/wav 文件流 | 获取提取的音频文件 |
| `/api/tasks/export` | POST | `{video_id}` | `{video_id, file_path}` | FFmpeg 烧录导出 |
| `/api/videos/:video_id/download` | GET | - | video/mp4 文件流 | 下载导出视频 |
| `/api/health` | GET | - | `{status, service, version}` | 健康检查 |

**错误响应统一格式**:
```json
{
  "error": true,
  "code": "TRANSCRIBE_FAILED",
  "message": "DashScope API call failed: timeout",
  "details": {}
}
```

**错误码定义**:
| 错误码 | 说明 |
|--------|------|
| `UPLOAD_FAILED` | 视频上传失败(格式不支持/文件损坏) |
| `TRANSCRIBE_FAILED` | 字幕生成失败(API错误/网络问题) |
| `EXPORT_FAILED` | 导出失败(FFmpeg错误/编码失败) |
| `VIDEO_NOT_FOUND` | video_id 不存在 |
| `INVALID_FORMAT` | 不支持的视频格式 |

## 6. 数据模型定义

### 6.1 Subtitle (字幕条)

```cpp
struct Subtitle {
    int id;
    double start_time;    // 秒, 精度到 0.001
    double end_time;      // 秒, 精度到 0.001
    QString text;
    SubtitleStyle style;
};
```

### 6.2 SubtitleStyle (字幕样式)

```cpp
struct SubtitleStyle {
    QString font_family = "Microsoft YaHei";
    int font_size = 24;
    QString color = "#FFFFFF";
    QString bg_color = "#00000080";
    QString position = "bottom_center";  // top_center/center/bottom_center
    bool bold = false;
    bool italic = false;
    int outline_width = 2;
    QString outline_color = "#000000";
};
```

### 6.3 VideoInfo (视频信息)

```cpp
struct VideoInfo {
    QString video_id;
    QString file_path;
    QString audio_path;
    double duration;
    int width;
    int height;
    int fps;
    QString format;
    double size_mb;
};
```

### 6.4 TaskStatus (任务状态)

```cpp
struct TaskStatus {
    QString task_id;
    int progress = 0;       // 0-100
    QString status;          // pending/processing/done/error
    QString error_message;
    int eta_seconds = 0;
};
```
## 7. 项目目录结构

```
SubForge/
├── CMakeLists.txt                    # 顶层构建配置
├── README.md                         # 项目说明
├── .env.example                      # 环境变量模板 (DASHSCOPE_API_KEY=sk-xxx)
├── .gitignore
├── docs/
│   ├── product.md                    # 产品设计文档
│   └── superpowers/plans/
├── shared/                           # 前后端共享数据模型
│   ├── CMakeLists.txt
│   ├── subtitle.h                    # Subtitle/SubtitleStyle 结构定义
│   ├── video_info.h                  # VideoInfo 结构定义
│   ├── task_status.h                 # TaskStatus 结构定义
│   └── api_response.h                # 统一API响应格式定义
├── backend/                          # C++ HTTP 后端（Linux）
│   ├── CMakeLists.txt
│   ├── src/
│   │   ├── main.cpp                  # 服务入口, 启动 HTTP server
│   │   ├── server.h/cpp              # HTTP 路由注册与处理
│   │   ├── transcriber.h/cpp         # DashScope API 调用封装
│   │   ├── transcribe_handler.h/cpp  # 转写任务 HTTP 处理
│   │   ├── upload_handler.h/cpp      # 视频上传 HTTP 处理
│   │   ├── export_handler.h/cpp      # FFmpeg 烧录导出封装
│   │   ├── audio_extractor.h/cpp     # FFmpeg 音频提取封装
│   │   ├── srt_parser.h/cpp          # SRT 格式解析
│   │   ├── task_manager.h/cpp        # 异步任务管理(生成/导出进度)
│   │   └── config.h/cpp              # .env 配置读取
│   ├── third_party/
│   │   ├── httplib.h                 # cpp-httplib 单头文件
│   │   └── json.hpp                  # nlohmann/json 单头文件
│   └── tests/
│       ├── CMakeLists.txt
│       └── test_server.cpp
├── frontend/                         # Qt 6 前端（Windows）
│   ├── CMakeLists.txt
│   └── src/
│       ├── main.cpp                  # Qt 应用入口
│       ├── MainWindow.h/cpp          # 主窗口布局管理
│       ├── VideoPlayerWidget.h/cpp   # 视频播放组件
│       ├── TimelineEditor.h/cpp      # 时间轴编辑器(核心自定义组件)
│       ├── SubtitleListWidget.h/cpp  # 字幕列表组件
│       ├── StyleEditorWidget.h/cpp   # 样式编辑面板
│       └── ApiClient.h/cpp           # HTTP 客户端封装
└── scripts/                          # 构建与调试脚本
    ├── build_frontend.bat            # Windows 前端构建
    ├── build_backend.sh              # Linux 后端构建
    ├── build_and_start.sh            # 后端一键构建+启动
    ├── run_frontend.bat              # windeployqt + 启动前端
    ├── rebuild_and_start.sh          # 后端强制重建+启动
    └── ...                           # 调试与测试脚本
```

## 8. UI 交互设计

### 8.1 主窗口布局

```
┌─────────────────────────────────────────────────────────────────┐
│  MenuBar: File | Tools | Export                                 │
│  ToolBar: [Open] [Upload] [Play] [Pause] [Stop] [Generate]      │
├────────────────────────────────┬────────────────────────────────┤
│                                │  StyleEditorWidget             │
│      VideoPlayerWidget         │  ┌──────────────────────────┐ │
│      (QVideoWidget)            │  │ Font: [Microsoft YaHei ▼]│ │
│      视频播放区域                │  │ Size: [24]               │ │
│                                │  │ Position: [bottom_center] │ │
│                                │  │ [B] [I]                   │ │
│                                │  └──────────────────────────┘ │
├────────────────────────────────┴────────────────────────────────┤
│                 SubtitleListWidget (字幕列表)                     │
├─────────────────────────────────────────────────────────────────┤
│  TimelineEditor (时间轴编辑器 - 自定义 QWidget)                   │
│  ┌──────────────────────────────────────────────────────────┐  │
│  │ 00:00   00:05   00:10   00:15   00:20   00:25   00:30  │  │
│  │ ┌─────┐┌──────────┐┌────┐┌──────────────────┐          │  │
│  │ │字幕1││  字幕2    ││字3 ││    字幕4          │          │  │
│  │ └─────┘└──────────┘└────┘└──────────────────┘          │  │
│  │ │ ← 播放进度指示线 (红色)                                │  │
│  └──────────────────────────────────────────────────────────┘  │
└─────────────────────────────────────────────────────────────────┘
│  StatusBar: 就绪                                                │
└─────────────────────────────────────────────────────────────────┘
```

### 8.2 交互流程

**步骤1: 打开并上传视频**
- 用户点击 "Open Video" 打开本地视频文件
- 视频在前端直接播放，无需上传即可预览
- 点击 "Upload" 将视频上传至后端 `/api/videos/upload`
- 后端提取音频，返回 video_id 和 video_info

**步骤2: 生成字幕**
- 用户点击 "Generate Subtitles" 按钮
- 前端调用 `/api/tasks/transcribe`
- 自动轮询 `/api/tasks/:task_id` 显示进度
- 生成完成后，字幕数据加载到时间轴编辑器和字幕列表

**步骤3: 编辑字幕**
- 用户在时间轴上点击选中字幕块
- 在字幕列表中查看/管理所有字幕句
- 在样式面板调整字体、大小、位置、加粗/斜体
- 时间轴与播放器进度联动

**步骤4: 导出视频**
- 用户点击 "Export Video with Subtitles"
- 弹出保存对话框（默认 Videos 文件夹）
- 前端调用 `/api/tasks/export` → 等待后端 FFmpeg 烧录
- 调用 `/api/videos/:video_id/download` 下载至用户指定路径

### 8.3 暗色主题

应用采用暗色主题(QSS样式表)，符合视频剪辑工具的专业审美：
- 主背景色: `#1e1e2e`
- 面板背景色: `#2d2d3d`
- 文字色: `#e0e0e0`
- 时间轴背景: `#252535`
- 字幕块色: `#4a90d9` (选中高亮 `#6ab0ff`)
- 播放指示线: `#ff4444`

## 9. 配置管理

### 9.1 .env 文件

**后端(Linux) .env 配置** (项目根目录 `.env`):
```env
# 阿里云百炼 API (语音识别)
DASHSCOPE_API_KEY=sk-xxxxxxxxxxxxxxxxxxxxxxxx

# 后端服务配置
SERVER_PORT=8080
SERVER_HOST=0.0.0.0
SERVER_BASE_URL=http://192.168.199.132:8080

# FFmpeg 路径
FFMPEG_PATH=/usr/local/bin/ffmpeg
FFPROBE_PATH=/usr/local/bin/ffprobe

# 临时文件目录
TEMP_DIR=/tmp/subforge

# 上传视频存储目录
UPLOAD_DIR=/var/lib/subforge/uploads
```

**前端(Windows) 配置**：通过 `QSettings("SubForge", "SubForge")` 保存后端地址，默认 `http://192.168.199.132:8080`。

### 9.2 配置加载

后端 `config.h/cpp` 负责加载 `.env` 文件，解析环境变量，提供全局配置访问接口。API Key 等敏感信息绝不硬编码到源码中。

## 10. 错误处理策略

### 10.1 后端错误处理

- HTTP 服务全局异常捕获，统一返回 JSON 错误格式
- DashScope API 调用失败：返回具体错误信息，超时 300 秒
- FFmpeg 处理失败：返回具体错误信息(编码器不支持/格式问题等)
- 文件不存在/损坏：返回 `VIDEO_NOT_FOUND` 或 `UPLOAD_FAILED`
- 所有错误记录日志文件 `logs/subforge.log`

### 10.2 前端错误处理

- API 调用失败：弹出错误提示对话框，显示具体原因（含保存位置建议）
- 网络断开：连接后端失败时提示检查服务状态

## 11. 简历亮点总结

作为大三 C++ 实习简历项目，SubForge 的核心展示点：

1. **C++ 后端工程能力**：Linux 环境 HTTP REST 服务设计、FFmpeg 集成调用、DashScope 大模型 API 封装、异步任务管理、JSON 数据序列化
2. **Qt GUI 开发能力**：Windows 环境 Qt 6 桌面应用、自定义 QWidget(时间轴编辑器)、多媒体播放、信号槽机制、前后端 HTTP 通信
3. **前后端分离架构思维**：REST API 规格设计、跨平台(Win/Linux)进程间 HTTP 通信、统一数据模型、错误码体系
4. **跨平台 C++ 开发能力**：Linux 后端服务 + Windows Qt 前端，CMake 跨平台构建，VMware 虚拟机开发调试
5. **完整产品交付能力**：从上传到导出的端到端全链路闭环，windeployqt 打包部署，功能可用

## 12. 约束与边界

### 12.1 不做的事情(NOT scope)

- 不做视频剪辑(裁剪/拼接视频片段) — 只做字幕相关
- 不做软字幕嵌入(只做硬字幕烧录)
- 不做实时语音转字幕(只做离线处理已有视频)
- 不做多语言翻译(只做语音识别转字幕)
- 不做云部署(后端部署在本地/内网 Linux 服务器，非公网云服务)
- 不做用户账户系统(单用户本地工具)

### 12.2 技术约束

- 视频格式支持: MP4、AVI、MKV、MOV
- 导出格式: MP4 (H.264 硬字幕烧录)
- 目标平台: 前端 Windows 10+，后端 Linux (Ubuntu 20.04+ / VMware 虚拟机)
- 前后端通信: HTTP REST API (后端 IP: 192.168.199.132:8080)

## 13. 依赖库清单

| 库 | 版本 | 用途 | 引入方式 | 平台 |
|----|------|------|----------|------|
| Qt 6 | 6.11.0 | GUI框架、多媒体、网络 | CMake find_package | **Windows (前端)** |
| cpp-httplib | 0.18+ | HTTP服务 | 单头文件 third_party/ | **Linux (后端)** |
| nlohmann/json | 3.11+ | JSON序列化 | 单头文件 third_party/ | **Win + Linux** |
| FFmpeg | 6.0+ | 视频音频处理 | 编译安装 / 系统包管理 | **Linux (后端)** |

## 14. 验收标准

### 14.1 功能验收

- [x] 上传视频至后端，成功提取音频并返回 video_info
- [x] 调用 DashScope (fun-asr) 生成字幕，返回带时间戳的字幕数据
- [x] 时间轴编辑器可视化显示字幕块，可点击选中
- [ ] 拖拽调整字幕块起止时间
- [ ] 双击字幕块编辑文本
- [x] 样式面板调整字体/大小/位置/加粗/斜体
- [x] 导出 MP4 视频，字幕硬烧录到画面
- [x] 全流程端到端可走通（上传 → 转写 → 编辑 → 导出 → 下载）

### 14.2 工程质量验收

- [x] CMake 构建：Windows 前端 + Linux 后端独立构建，零错误零警告
- [x] API Key 配置：从 .env 文件读取，不硬编码
- [x] 错误处理：所有 API 错误有统一 JSON 格式响应
- [x] 前端 UI 主窗口完整（菜单/Toolbar/播放器/时间轴/字幕列表/样式面板）
- [ ] 暗色主题/QSS 样式美化
- [x] windeployqt 打包 DLL，双击 exe 可直接运行