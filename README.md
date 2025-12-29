# Multi-thread HTTP/HTTPS Downloader (C)

---

## 中文说明 (Chinese)

### 📌 项目简介

这是一个 **用 C 语言编写的多线程 HTTP/HTTPS 下载器**，工作方式类似 **IDM / DAP**。

它通过 **HTTP Range Requests** 将文件拆分为多个区块（chunks），并使用 **多个线程并行下载**，从而显著提升下载速度，尤其是在服务器对单连接限速的情况下。

> ⚠️ 当前版本：**仅支持 Linux（For Now）**

---

### ✨ 功能特性

* 支持 **HTTP 和 HTTPS**
* 原生 **POSIX socket**
* **OpenSSL** 实现 TLS
* 多线程并行下载（pthread）
* 支持 `Accept-Ranges`
* 实时进度显示：

  * 下载进度 %
  * 实时速度 (MB/s)
  * 已用时间
  * 预计剩余时间 (ETA)
* 自动获取文件大小（HEAD 请求）
* 自动预分配文件空间（`ftruncate`）
* 模块化结构（core / headers）

---

### 🧵 线程工作原理说明

假设文件大小是 **1GB**，线程数为 **8**：

* 文件被平均分成 8 个区块
* 每个线程负责一个独立的字节区间，例如：

```
Thread 1 → bytes 0 - 134217727
Thread 2 → bytes 134217728 - 268435455
Thread 3 → bytes 268435456 - 402653183
...
```

每个线程：

1. 建立独立的 TCP / TLS 连接
2. 发送带 `Range` 头的 HTTP GET 请求
3. 只下载自己负责的字节区间
4. 使用 `pwrite()` 写入文件的正确位置

所有线程并行工作，**最终组成一个完整文件**。

> ⚠️ 如果服务器已经提供很高的单连接速度，增加线程数不会显著提升速度。

---

### 🛠 编译方法（Linux）

#### 依赖

```bash
sudo pacman -S openssl gcc make   # Arch
sudo apt install build-essential libssl-dev  # Debian/Ubuntu
```

#### 编译

```bash
make
```

生成的可执行文件位于：

```text
out/downloader
```

---

### ▶️ 使用方法

```bash
./out/downloader <url> <output_file> <threads>
```

示例：

```bash
./out/downloader https://speedtest.tele2.net/1GB.zip test.zip 8
```

---

### ⚠️ 注意事项

* 并非所有服务器都允许多线程 Range 下载
* 使用过多线程可能导致：

  * 服务器限速
  * IP 暂时封禁
* 推荐线程数：**4 ~ 8**

---

### 📦 项目结构

```text
.
├── core/
│   ├── tcp.c
│   ├── http.c
│   ├── downloader.c
│   ├── progress.c
│   └── util.c
├── headers/
│   ├── tcp.h
│   ├── http.h
│   ├── downloader.h
│   ├── progress.h
│   └── config.h
├── main.c
├── Makefile
└── out/
```

---

## ENGLISH

### 📌 Project Overview

This is a **multi-threaded HTTP/HTTPS downloader written in C**, similar in behavior to **IDM / DAP**.

It works by splitting a file into multiple byte ranges and downloading them **in parallel using multiple threads**, which can significantly improve download speed on servers that limit single connections.

> ⚠️ Current status: **Linux Only (For Now)**

---

### ✨ Features

* HTTP and HTTPS support
* Native **POSIX sockets**
* TLS via **OpenSSL**
* Multi-threaded downloading (pthread)
* HTTP `Range` support
* Real-time progress display:

  * Percentage
  * Download speed (MB/s)
  * Elapsed time
  * ETA
* Automatic file size detection (HEAD request)
* Pre-allocated output file (`ftruncate`)
* Modular codebase (core / headers)

---

### 🧵 How Threads Work

Example:
File size = **1GB**, Threads = **8**

* The file is split into 8 equal chunks
* Each thread downloads a specific byte range:

```
Thread 1 → bytes 0 - 134217727
Thread 2 → bytes 134217728 - 268435455
Thread 3 → bytes 268435456 - 402653183
...
```

Each thread:

1. Opens its own TCP/TLS connection
2. Sends an HTTP GET request with a `Range` header
3. Downloads only its assigned byte range
4. Writes data directly to the correct file offset using `pwrite()`

All threads run concurrently and **assemble the final file without locking**.

> ⚠️ Increasing thread count does not help if the server already provides high single-connection bandwidth.

---

### 🛠 Build Instructions (Linux)

#### Dependencies

```bash
sudo apt install build-essential libssl-dev
# or
sudo pacman -S gcc make openssl
```

#### Build

```bash
make
```

Binary output:

```text
out/downloader
```

---

### ▶️ Usage

```bash
./out/downloader <url> <output_file> <threads>
```

Example:

```bash
./out/downloader https://speedtest.tele2.net/1GB.zip test.zip 8
```

---

### ⚠️ Notes

* Not all servers allow multi-range downloads
* Too many threads may trigger:

  * Server throttling
  * Temporary IP blocking
* Recommended thread count: **4 ~ 8**

---

### 🚧 Future Improvements

* Resume support (`.part`)
* Dynamic thread scaling
* Retry per chunk
* Checksum verification
* HTTP/2 support

---

### 🧠 Final Note

This project is a **real networking MVP**, not a toy example.
It demonstrates low-level networking, TLS, concurrency, and performance-oriented file I/O.
