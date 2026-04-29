# SubtitleForge - 视频字幕生成与剪辑工具

## 项目概述

SubtitleForge 是一款基于 C++ 后端 + Web 前端的视频字幕生成与剪辑工具，支持上传视频、自动生成字幕、时间轴编辑、字幕样式调整、导出带硬字幕的视频。采用任务队列架构，支持短视频到长视频的全长度处理。

**目标用户：** 视频创作者、字幕组、内容本地化团队

**核心价值：** 本地离线运行、零 API 费用、专业级字幕编辑、一键硬字幕烧录

## 技术栈

| 层级 | 技术 | 说明 |
|------|------|------|
| 前端 | HTML5 + CSS + Vanilla JS | 单 HTML 文件交付，无框架依赖 |
| 前端样式 | Tailwind CSS (CDN) | 暗色主题，专业编辑器风格 |
| 前端视频 | HTML5 Video + Canvas | 视频播放 + 字幕叠加预览 |
| 前端时间轴 | Canvas 2D | 拖拽、缩放、选中字幕块 |
| 后端语言 | C++17 | 现代C++，跨平台 |
| HTTP 服务器 | cpp-httplib (header-only) | 轻量 REST API 网关 |
| JSON 处理 | nlohmann/json (header-only) | 请求/响应序列化 |
| ASR 引擎 | whisper.cpp | OpenAI Whisper 本地模型，支持中英文 |
| 视频处理 | FFmpeg libav* (C API) | 音频提取、字幕烧录、截帧 |
| 日志 | spdlog | 结构化日志 |
| 构建 | CMake 3.20+ | 跨平台构建系统 |

## 系统架构

### 整体架构图

```
┌──────────────────────────────────────────────────────────┐
│                   单 HTML 前端 (浏览器)                    │
│                                                           │
│  ┌──────────┐ ┌──────────┐ ┌──────────┐ ┌──────────┐    │
│  │ 视频上传  │ │ 时间轴编辑│ │ 字幕编辑器│ │ 样式面板  │    │
│  │ 拖拽/选择 │ │ Canvas   │ │ 文字修改  │ │ 字体/颜色 │    │
│  └──────────┘ └──────────┘ └──────────┘ └──────────┘    │
│  ┌──────────┐ ┌──────────┐                               │
│  │ 视频预览  │ │ 导出面板  │                               │
│  │ HTML5 V  │ │ 进度/下载 │                               │
│  └──────────┘ └──────────┘                               │
└─────────────────────┬────────────────────────────────────┘
                      │ REST API (JSON)
┌─────────────────────▼────────────────────────────────────┐
│              C++ API 网关 (cpp-httplib)                    │
│                                                           │
│  ┌─────────────────────────────────────────────────────┐ │
│  │ 路由层                                               │ │
│  │  POST /api/upload      → 上传视频，返回任务ID         │ │
│  │  POST /api/transcribe  → 提交ASR识别任务              │ │
│  │  GET  /api/tasks/:id   → 查询任务状态/进度            │ │
│  │  GET  /api/subtitles   → 获取字幕列表                 │ │
│  │  PUT  /api/subtitles   → 更新字幕(时间/文字/样式)      │ │
│  │  POST /api/export      → 提交导出任务                 │ │
│  │  GET  /api/preview     → 获取预览帧(带字幕叠加)       │ │
│  │  GET  /api/download/:id→ 下载导出文件                 │ │
│  └─────────────────────────────────────────────────────┘ │
│  ┌──────────────────┐  ┌──────────────────┐              │
│  │ 中间件            │  │ 静态文件服务      │              │
│  │ CORS / 日志 / 限流│  │ index.html       │              │
│  └──────────────────┘  └──────────────────┘              │
└────────┬────────────────────────────┬────────────────────┘
         │                            │
┌────────▼─────────┐       ┌─────────▼────────────────────┐
│   任务调度器       │       │       Worker 进程池           │
│                   │       │                              │
│ ┌───────────────┐│       │  ┌────────────────────────┐  │
│ │ 任务队列       ││ 分发   │  │ ASR Worker             │  │
│ │ (内存+文件)    ││──────▶│  │ whisper.cpp            │  │
│ │               ││       │  │ 分片音频 → 文本         │  │
│ │ 优先级管理     ││       │  └────────────────────────┘  │
│ │ 状态追踪       ││       │  ┌────────────────────────┐  │
│ │               ││       │  │ Export Worker           │  │
│ └───────────────┘│       │  │ FFmpeg libav*           │  │
│                   │       │  │ 烧录字幕 → 新视频       │  │
│ ┌───────────────┐│       │  └────────────────────────┘  │
│ │ 进度追踪器     ││       │  ┌────────────────────────┐  │
│ │ 文件状态轮询   ││       │  │ Preview Worker          │  │
│ │ 前端进度推送   ││       │  │ FFmpeg 截帧+字幕叠加    │  │
│ └───────────────┘│       │  └────────────────────────┘  │
└──────────────────┘       └──────────────────────────────┘
```

### 进程模型

```
主进程 (subtitleforge)
├── HTTP 服务器线程
├── 任务调度线程
├── 进度轮询线程
└── Worker 子进程管理
    ├── asr_worker (按需启动)
    ├── export_worker (按需启动)
    └── preview_worker (按需启动)
```

**Worker 通信机制：**
- 主进程通过 `fork/exec` 启动 Worker 子进程
- 通过命令行参数传递任务配置（JSON 文件路径）
- Worker 通过写 JSON 状态文件汇报进度
- 主进程轮询状态文件，更新任务进度
- Worker 完成后退出，主进程回收资源

## 数据模型

### Task（任务）

```json
{
  "id": "550e8400-e29b-41d4-a716-446655440000",
  "type": "transcribe",
  "status": "running",
  "progress": 0.65,
  "video_id": "vid_abc123",
  "created_at": 1714300800,
  "started_at": 1714300801,
  "completed_at": null,
  "result": null,
  "error": null
}
```

| 字段 | 类型 | 说明 |
|------|------|------|
| id | string (UUID) | 任务唯一标识 |
| type | enum | "transcribe" / "export" / "preview" |
| status | enum | "queued" / "running" / "completed" / "failed" |
| progress | float | 进度 0.0 ~ 1.0 |
| video_id | string | 关联的视频ID |
| created_at | timestamp | 创建时间 |
| started_at | timestamp | 开始执行时间 |
| completed_at | timestamp | 完成时间 |
| result | json | 任务结果（字幕数据/文件路径） |
| error | string | 失败原因 |

### Subtitle（字幕）

```json
{
  "id": 1,
  "index": 1,
  "start_time": 0.0,
  "end_time": 3.5,
  "text": "你好，欢迎来到 SubtitleForge",
  "style": {
    "font_family": "Noto Sans SC",
    "font_size": 24,
    "font_color": "#FFFFFF",
    "stroke_color": "#000000",
    "stroke_width": 2,
    "background_color": "#000000",
    "background_opacity": 0.6,
    "position": "bottom",
    "offset_y": 30
  }
}
```

| 字段 | 类型 | 说明 |
|------|------|------|
| id | int | 字幕唯一标识 |
| index | int | 字幕序号 |
| start_time | float | 起始时间（秒） |
| end_time | float | 结束时间（秒） |
| text | string | 字幕文本内容 |
| style | SubtitleStyle | 字幕样式（继承全局样式，可单独覆盖） |

### SubtitleStyle（字幕样式）

| 字段 | 类型 | 默认值 | 说明 |
|------|------|--------|------|
| font_family | string | "Noto Sans SC" | 字体名称 |
| font_size | int | 24 | 字体大小（像素） |
| font_color | string | "#FFFFFF" | 字体颜色 |
| stroke_color | string | "#000000" | 描边颜色 |
| stroke_width | int | 2 | 描边宽度 |
| background_color | string | "#000000" | 背景颜色 |
| background_opacity | float | 0.6 | 背景透明度 |
| position | enum | "bottom" | 位置：top/center/bottom |
| offset_y | int | 30 | Y轴偏移（像素） |

### Video（视频）

```json
{
  "id": "vid_abc123",
  "filename": "demo.mp4",
  "file_path": "/data/uploads/vid_abc123/demo.mp4",
  "duration": 125.5,
  "width": 1920,
  "height": 1080,
  "fps": 30.0,
  "codec": "h264",
  "audio_codec": "aac",
  "thumbnail_path": "/data/uploads/vid_abc123/thumb.jpg"
}
```

## API 设计

### 上传视频

```
POST /api/upload
Content-Type: multipart/form-data

Body: video file

Response 200:
{
  "video_id": "vid_abc123",
  "filename": "demo.mp4",
  "duration": 125.5,
  "width": 1920,
  "height": 1080,
  "fps": 30.0,
  "thumbnail_url": "/api/thumbnail/vid_abc123"
}
```

### 提交 ASR 识别任务

```
POST /api/transcribe
Content-Type: application/json

Body:
{
  "video_id": "vid_abc123",
  "language": "zh",
  "model": "base"
}

Response 202:
{
  "task_id": "550e8400-e29b-41d4-a716-446655440000",
  "status": "queued"
}
```

### 查询任务状态

```
GET /api/tasks/:id

Response 200:
{
  "task_id": "550e8400-e29b-41d4-a716-446655440000",
  "type": "transcribe",
  "status": "running",
  "progress": 0.65,
  "current_segment": 13,
  "total_segments": 20
}
```

### 获取字幕列表

```
GET /api/subtitles?video_id=vid_abc123

Response 200:
{
  "video_id": "vid_abc123",
  "subtitles": [
    {
      "id": 1,
      "index": 1,
      "start_time": 0.0,
      "end_time": 3.5,
      "text": "你好，欢迎来到 SubtitleForge",
      "style": { ... }
    }
  ],
  "global_style": { ... }
}
```

### 更新字幕

```
PUT /api/subtitles
Content-Type: application/json

Body:
{
  "video_id": "vid_abc123",
  "subtitles": [
    {
      "id": 1,
      "start_time": 0.0,
      "end_time": 4.0,
      "text": "你好，欢迎使用 SubtitleForge",
      "style": { "font_size": 28 }
    }
  ],
  "global_style": { "font_family": "Microsoft YaHei" }
}

Response 200:
{
  "updated": 1,
  "global_style_updated": true
}
```

### 提交导出任务

```
POST /api/export
Content-Type: application/json

Body:
{
  "video_id": "vid_abc123",
  "format": "mp4",
  "quality": "high"
}

Response 202:
{
  "task_id": "660e8400-e29b-41d4-a716-446655440001",
  "status": "queued"
}
```

### 获取预览帧

```
GET /api/preview?video_id=vid_abc123&time=5.5&width=640

Response 200:
Content-Type: image/png
(PNG 图片数据，带字幕叠加)
```

### 下载导出文件

```
GET /api/download/:task_id

Response 200:
Content-Type: video/mp4
Content-Disposition: attachment; filename="demo_subtitled.mp4"
(视频文件数据)
```

## 核心工作流

### 完整流程

```
上传视频 → 提取音频 → ASR识别 → 生成字幕 → 用户编辑 → 导出视频
   │           │           │           │           │           │
   ▼           ▼           ▼           ▼           ▼           ▼
 POST      Worker      Worker     GET/PUT     实时预览    Worker
 /upload   FFmpeg      whisper    /subtitles  /preview    FFmpeg
           提取音频    逐片识别    编辑字幕    截帧+叠加   烧录字幕
```

### 大视频分片策略

**ASR 识别分片：**
1. FFmpeg 将音频提取为 16kHz WAV
2. 按 30 秒分片，片间重叠 2 秒（避免断句截断）
3. 每片送入 whisper.cpp 识别
4. 识别完成后立即写入状态文件，主进程更新进度
5. 全部片完成后合并结果，去重重叠部分
6. 前端可实时看到字幕逐步生成（轮询任务进度）

**导出分片处理：**
1. 解析视频关键帧位置
2. 按关键帧分段，每段独立烧录字幕
3. 最后用 FFmpeg concat 合并所有分段
4. 内存占用恒定，不受视频长度影响

### Worker 生命周期

```
1. 主进程创建任务 → 写入任务配置 JSON 文件
2. 主进程 fork/exec Worker 子进程，传入配置文件路径
3. Worker 读取配置 → 执行任务 → 定期写进度状态文件
4. 主进程轮询状态文件 → 更新任务进度 → 推送给前端
5. Worker 完成/失败 → 写最终状态 → 退出
6. 主进程回收子进程 → 更新任务最终状态
```

## 前端设计

### 布局

```
┌──────────────────────────────────────────────────────────┐
│  SubtitleForge                    [上传视频] [导出]        │
├──────────────────────────────────┬───────────────────────┤
│                                  │                       │
│                                  │   字幕列表面板         │
│       视频预览区域                │ ┌───────────────────┐ │
│    (HTML5 Video + Canvas叠加)     │ │ 1. 00:00-00:03    │ │
│                                  │ │    你好世界         │ │
│                                  │ │ 2. 00:03-00:06    │ │
│                                  │ │    这是测试         │ │
│                                  │ │ 3. 00:06-00:09    │ │
│                                  │ │    字幕编辑         │ │
│                                  │ └───────────────────┘ │
│                                  │                       │
│                                  │   样式调整面板         │
│                                  │ 字体: [下拉] 大小:[24] │
│                                  │ 颜色: [■] 描边: [■]   │
│                                  │ 位置: [底部▼] 偏移:[0] │
├──────────────────────────────────┴───────────────────────┤
│  时间轴编辑区域 (Canvas)                                   │
│  ┌─┬─┬─┬─┬─┬─┬─┬─┬─┬─┬─┬─┬─┬─┬─┬─┬─┬─┬─┬─┬─┬─┬─┬─┐   │
│  │▓▓▓│ │▓▓▓▓▓│ │▓▓▓▓│ │▓▓▓▓▓▓▓│ │     │              │   │
│  └─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┘   │
│  00:00    00:10    00:20    00:30    00:40    00:50       │
│  ▶ 播放  🔍 缩放  ✂ 拆分  🔄 合并                         │
└──────────────────────────────────────────────────────────┘
```

### 前端模块

| 模块 | 实现方式 | 功能 |
|------|----------|------|
| 视频上传 | 拖拽区域 + file input | 上传视频文件，显示上传进度 |
| 视频预览 | HTML5 `<video>` + Canvas 叠加 | 播放视频，实时叠加字幕预览 |
| 时间轴编辑 | Canvas 2D | 可视化字幕时间块，拖拽调整起止时间 |
| 字幕列表 | DOM 列表 | 显示/编辑字幕文本，点击跳转对应时间 |
| 样式面板 | 原生表单控件 | 调整字体、大小、颜色、描边、位置等 |
| 导出面板 | 模态框 | 选择导出参数，显示导出进度，下载文件 |

### 时间轴交互

- **拖拽移动：** 鼠标按住字幕块左右拖动，调整起止时间
- **拖拽边缘：** 拖动字幕块左右边缘，单独调整起始或结束时间
- **双击编辑：** 双击字幕块弹出文字编辑输入框
- **右键菜单：** 拆分、合并、删除、复制字幕
- **缩放：** 鼠标滚轮缩放时间轴精度
- **播放头：** 点击时间轴定位播放位置，播放时自动跟随
- **选中高亮：** 点击字幕块选中，视频跳转到对应时间

### Canvas 字幕叠加预览

```
视频帧 (HTML5 Video)
    ↓
Canvas drawImage(video, ...)
    ↓
Canvas fillText(subtitle.text, x, y)  ← 应用字幕样式
    ↓
显示带字幕的预览帧
```

- 使用 `requestAnimationFrame` 循环绘制
- 仅在播放或手动跳转时更新
- 字幕样式实时反映用户修改

## 项目目录结构

```
SubtitleForge/
├── CMakeLists.txt                    # 根 CMake 配置
├── product.md                        # 本设计文档
├── README.md                         # 项目说明
├── src/
│   ├── main.cpp                      # 程序入口
│   ├── server/
│   │   ├── http_server.h/cpp         # HTTP 服务器 + 路由注册
│   │   ├── api_handlers.h/cpp        # API 处理函数
│   │   └── middleware.h/cpp          # CORS、日志、限流中间件
│   ├── task/
│   │   ├── task_queue.h/cpp          # 任务队列 (内存+文件持久化)
│   │   ├── task_scheduler.h/cpp      # 任务调度器 (分发/回收)
│   │   └── task_types.h              # 任务类型/状态定义
│   ├── worker/
│   │   ├── worker_pool.h/cpp         # Worker 进程池管理
│   │   ├── worker_process.h/cpp      # 子进程启动/通信/回收
│   │   ├── asr_worker.cpp            # ASR Worker 主程序
│   │   ├── export_worker.cpp         # 导出 Worker 主程序
│   │   └── preview_worker.cpp        # 预览 Worker 主程序
│   ├── core/
│   │   ├── video_info.h/cpp          # 视频信息解析 (FFmpeg)
│   │   ├── audio_extractor.h/cpp     # 音频提取 (FFmpeg)
│   │   ├── subtitle_burner.h/cpp     # 字幕烧录 (FFmpeg drawtext)
│   │   ├── frame_capture.h/cpp       # 视频截帧 (FFmpeg)
│   │   └── subtitle_parser.h/cpp     # SRT/VTT 解析与生成
│   └── utils/
│       ├── logger.h/cpp              # spdlog 日志封装
│       ├── file_utils.h/cpp          # 文件操作工具
│       └── uuid.h/cpp                # UUID 生成
├── frontend/
│   └── index.html                    # 单 HTML 前端文件
├── third_party/
│   ├── cpp-httplib/                  # HTTP 库 (header-only)
│   ├── nlohmann/                     # JSON 库 (header-only)
│   └── spdlog/                       # 日志库
├── tests/
│   ├── test_task_queue.cpp           # 任务队列单元测试
│   ├── test_subtitle_parser.cpp      # 字幕解析单元测试
│   └── test_api.cpp                  # API 集成测试
└── docs/
    └── superpowers/
        └── specs/
            └── 2026-04-28-subtitle-forge-design.md
```

## 构建与部署

### 依赖安装

```bash
# Windows (vcpkg)
vcpkg install ffmpeg whisper.cpp spdlog nlohmann-json

# macOS (brew)
brew install ffmpeg whisper-cpp spdlog nlohmann-json

# Ubuntu (apt + manual)
sudo apt install libavcodec-dev libavformat-dev libavutil-dev libswscale-dev libswresample-dev
```

### CMake 构建

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . -j$(nproc)
```

### 运行

```bash
./subtitleforge --port 8080 --data-dir ./data
```

浏览器打开 `http://localhost:8080` 即可使用。

### 产出文件

| 文件 | 说明 |
|------|------|
| `subtitleforge` | 主程序（API 网关 + 任务调度） |
| `asr_worker` | ASR Worker 可执行文件 |
| `export_worker` | 导出 Worker 可执行文件 |
| `preview_worker` | 预览 Worker 可执行文件 |
| `index.html` | 前端界面（主程序自动服务） |

## 非功能性需求

### 性能

| 指标 | 目标 |
|------|------|
| ASR 识别速度 | 1分钟音频 ≤ 15秒（base 模型） |
| 视频上传 | 支持最大 2GB 文件 |
| 预览帧生成 | ≤ 500ms/帧 |
| 导出速度 | 1分钟视频 ≤ 30秒（硬字幕烧录） |
| 并发任务 | 最多 3 个 Worker 并行 |

### 可靠性

- 任务队列文件持久化：进程崩溃后重启可恢复未完成任务
- Worker 健康检查：主进程定期检查 Worker 存活状态
- 超时机制：ASR 任务超时 30 分钟自动标记失败
- 磁盘空间检查：上传前检查剩余空间

### 安全性

- 文件类型校验：仅允许视频文件上传
- 文件大小限制：默认 2GB 上限
- 路径安全：禁止路径遍历攻击
- CORS 限制：仅允许 localhost 访问

## 简历亮点

本项目在简历中可突出以下技术点：

1. **C++ 系统编程：** 多进程架构、进程间通信、文件持久化任务队列
2. **多媒体处理：** FFmpeg libav* API 调用、音频提取、视频编解码、字幕烧录
3. **AI 集成：** whisper.cpp 本地 ASR 模型集成、分片识别策略
4. **网络编程：** REST API 设计与实现、HTTP 服务器、CORS 中间件
5. **前端工程：** Canvas 2D 时间轴编辑器、HTML5 Video API、单文件交付
6. **架构设计：** 任务队列模式、Worker 进程池、分片处理策略
7. **工程实践：** CMake 构建、单元测试、结构化日志
