# SubForge — 视频字幕自动生成与编辑桌面工具

基于 C++17 + Qt 6 + cpp-httplib 构建的前后端分离桌面应用，支持视频上传、DashScope 大模型自动语音转字幕、可视化时间轴编辑、字幕样式自定义、FFmpeg 字幕硬烧录导出全链路。

## 项目概览

| 项目 | 说明 |
|------|------|
| **类型** | 桌面 GUI 应用（前后端分离架构） |
| **语言** | C++17 |
| **前端** | Qt 6 QWidget（Windows 桌面客户端） |
| **后端** | C++ HTTP 服务（Linux 服务器） |
| **AI 引擎** | 阿里云 DashScope（`fun-asr` 语音识别模型） |
| **视频处理** | FFmpeg（音频提取 + 字幕硬烧录） |
| **数据格式** | JSON（nlohmann/json） |
| **构建工具** | CMake + MinGW（Win）/ GCC（Linux） |

## 核心功能

- **视频上传与播放** — 支持本地视频打开，QMediaPlayer 实现播放/暂停/进度/音量控制
- **语音转字幕** — 后端提取音频，调用阿里云 DashScope `fun-asr` 模型自动生成带时间戳的字幕
- **可视化时间轴编辑** — 自定义 QWidget 绘制，支持字幕块拖拽移动、边缘缩放、逐句编辑、增删合并、撤销/重做
- **字幕样式自定义** — 字体/字号/颜色/背景透明度/描边/位置、预设模板一键切换
- **字幕烧录导出** — 后端调用 FFmpeg 将字幕硬编码至视频，前端下载 MP4 文件

## 系统架构

```
┌──────────────────────────────────────┐
│         Qt Desktop App (Windows)      │
│  ┌──────────────────────────────────┐ │
│  │ MainWindow                       │ │
│  │ ┌────────────┬──────────────────┐│ │
│  │ │VideoPlayer │ TimelineEditor   ││ │
│  │ │QVideoWidget│ (自定义 QWidget)   ││ │
│  │ └────────────┴──────────────────┘│ │
│  │ StyleEditorWidget                │ │
│  │ SubtitleListWidget               │ │
│  └──────────────────────────────────┘ │
│  ApiClient (QNetworkAccessManager)    │
└──────────────┬───────────────────────┘
               │ REST API (JSON)
               ▼
┌──────────────┴───────────────────────┐
│       C++ HTTP Backend (Linux)       │
│  (cpp-httplib)                       │
│  ┌──────────────────────────────────┐ │
│  │ POST /api/videos/upload          │ │
│  │ POST /api/tasks/transcribe       │ │
│  │ GET  /api/tasks/:task_id         │ │
│  │ POST /api/tasks/export           │ │
│  │ GET  /api/videos/:id/download    │ │
│  └──────────────────────────────────┘ │
└──────────────────────────────────────┘
```

## 技术栈

| 层级 | 技术 | 说明 |
|------|------|------|
| 语言 | C++17 | 前后端统一语言 |
| 前端 GUI | Qt 6 (Widgets/Multimedia/Network) | 桌面应用 |
| HTTP 后端 | cpp-httplib | 轻量单头文件 C++ HTTP 库 |
| AI API | 阿里云 DashScope (fun-asr) | 语音识别转字幕 |
| 视频处理 | FFmpeg | 音频提取、字幕硬烧录 |
| JSON | nlohmann/json | 前后端数据交换 |
| 构建 | CMake + MinGW / GCC | 跨平台构建管理 |
| 部署 | windeployqt / Shell 脚本 | Windows 打包 / Linux 一键启动 |

## 项目结构

```
SubForge/
├── CMakeLists.txt          # 顶层构建配置
├── .env.example            # 环境变量模板
├── .gitignore              # Git 忽略规则
├── shared/                 # 前后端共享数据模型
│   ├── subtitle.h          # Subtitle / SubtitleStyle
│   ├── video_info.h        # VideoInfo
│   ├── task_status.h       # TaskStatus
│   └── api_response.h      # 统一 API 响应格式
├── backend/                # C++ HTTP 后端（Linux）
│   ├── CMakeLists.txt
│   ├── src/
│   │   ├── main.cpp        # 服务入口
│   │   ├── server.cpp/h    # 路由注册
│   │   ├── transcriber.cpp/h    # DashScope API 封装
│   │   ├── export_handler.cpp/h # FFmpeg 烧录导出
│   │   ├── upload_handler.cpp/h # 视频上传处理
│   │   ├── transcribe_handler.cpp/h # 转写任务处理
│   │   ├── audio_extractor.cpp/h   # FFmpeg 音频提取
│   │   ├── srt_parser.cpp/h       # SRT 格式解析
│   │   ├── task_manager.cpp/h     # 异步任务管理
│   │   └── config.cpp/h     # .env 配置读取
│   └── third_party/         # 第三方库
├── frontend/               # Qt 6 前端（Windows）
│   ├── CMakeLists.txt
│   └── src/
│       ├── main.cpp              # 应用入口
│       ├── MainWindow.cpp/h      # 主窗口
│       ├── VideoPlayerWidget.cpp/h   # 视频播放器
│       ├── TimelineEditor.cpp/h  # 时间轴编辑器（自定义控件）
│       ├── SubtitleListWidget.cpp/h  # 字幕列表
│       ├── StyleEditorWidget.cpp/h   # 样式编辑面板
│       └── ApiClient.cpp/h      # HTTP API 客户端
├── scripts/                # 构建与测试脚本
└── docs/                   # 设计文档
```

## 快速开始

### 环境要求

- **前端**：Windows + Qt 6.11.0 (MinGW 64-bit) + CMake 3.16+
- **后端**：Linux (Ubuntu 20.04+) + CMake 3.16+ + FFmpeg + cURL
- **API Key**：阿里云 DashScope API Key（配置在 `.env` 中）

### 后端启动（Linux）

```bash
cd SubForge
cp .env.example .env          # 编辑填入 DASHSCOPE_API_KEY
bash scripts/build_and_start.sh
```

### 前端构建与运行（Windows）

```bash
# 构建
.\scripts\build_frontend.bat

# 部署 DLL 并运行
.\scripts\run_frontend.bat
```

### 配置 .env 文件

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
UPLOAD_DIR=/var/lib/subforge/uploads
```

### 使用流程

1. **打开视频** — 点击 Open 选择本地视频文件
2. **上传视频** — 点击 Upload 上传至后端服务器
3. **生成字幕** — 点击 Generate Subtitles，后端调用 DashScope 自动转写
4. **编辑字幕** — 在时间轴上拖拽调整字幕块，编辑文本内容
5. **调整样式** — 右侧面板修改字体、大小、颜色、位置
6. **导出视频** — 点击 Export Video，后端 FFmpeg 烧录字幕后下载 MP4

## API 接口

| 方法 | 路径 | 说明 |
|------|------|------|
| GET | `/api/health` | 健康检查 |
| POST | `/api/videos/upload` | 上传视频文件（multipart） |
| POST | `/api/tasks/transcribe` | 提交语音转写任务 |
| GET | `/api/tasks/:task_id` | 查询任务状态与进度 |
| GET | `/api/videos/:video_id/subtitles` | 获取字幕数据 |
| POST | `/api/tasks/export` | 提交字幕烧录导出任务 |
| GET | `/api/videos/:video_id/download` | 下载导出后的视频文件 |
| GET | `/api/videos/:video_id/audio` | 获取提取的音频文件 |

## 数据模型

### Subtitle（字幕）

```json
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
```

### VideoInfo（视频信息）

```json
{
  "video_id": "uuid-string",
  "duration": 120.5,
  "resolution": { "width": 1920, "height": 1080 },
  "fps": 30,
  "format": "mp4",
  "size_mb": 85.3
}
```
