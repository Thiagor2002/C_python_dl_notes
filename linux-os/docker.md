# Docker & Kubernetes 容器技术指南

本文介绍 Docker 容器技术和 Kubernetes (K8s) 容器编排平台的核心理念、常用命令及实战用例。

## Table of Contents

- [Part 1: Docker](#part-1-docker)
  - [1. 核心概念](#1-核心概念)
  - [2. 镜像管理 (Images)](#2-镜像管理-images)
  - [3. 容器管理 (Containers)](#3-容器管理-containers)
  - [4. 数据卷管理 (Volumes)](#4-数据卷管理-volumes)
  - [5. 网络管理 (Networks)](#5-网络管理-networks)
  - [6. Dockerfile 详解](#6-dockerfile-详解)
  - [7. Docker Compose](#7-docker-compose)
  - [8. 清理与维护](#8-清理与维护)
  - [9. Docker 实战案例](#9-docker-实战案例)
- [Part 2: Kubernetes](#part-2-kubernetes)
  - [10. K8s 核心概念](#10-k8s-核心概念)
  - [11. Pod（容器组）](#11-pod容器组)
  - [12. Deployment（部署）](#12-deployment部署)
  - [13. Service（服务）](#13-service服务)
  - [14. ConfigMap & Secret](#14-configmap--secret)
  - [15. Namespace（命名空间）](#15-namespace命名空间)
  - [16. 常用 kubectl 命令速查](#16-常用-kubectl-命令速查)
  - [17. K8s 集群管理工具](#17-k8s-集群管理工具)
- [18. 辅助工具](#18-辅助工具)

---

# Part 1: Docker

## 1. 核心概念

| 概念         | 说明                                                                 |
| ------------ | -------------------------------------------------------------------- |
| **Image**    | **镜像** — 创建容器的只读模板，由 `Dockerfile` 定义                  |
| **Container**| **容器** — 镜像的可运行实例，轻量级，共享宿主机内核，**默认无状态**  |
| **Volume**   | **数据卷** — 独立于容器生命周期的持久化存储                          |
| **Network**  | **网络** — 容器间通信的虚拟网络                                      |
| **Registry** | **仓库** — 存储和分发镜像的服务（如 Docker Hub、私有 Registry）      |
| **Dockerfile**| **构建文件** — 定义镜像构建步骤的文本文件                            |
| **Docker Compose** | **编排工具** — 用 YAML 定义和管理多容器应用                    |

> **容器 vs 虚拟机**：容器共享宿主机 OS 内核，启动快（秒级），资源开销小；虚拟机包含完整 OS，隔离性更强但更重。

---

## 2. 镜像管理 (Images)

### 基本操作

```bash
# 列出本地镜像
docker images
docker image ls

# 搜索 Docker Hub 上的镜像
docker search nginx
docker search --filter=stars=100 ubuntu    # 至少 100 个 star

# 拉取镜像
docker pull nginx                          # 拉取最新版
docker pull nginx:1.25-alpine              # 拉取指定版本
docker pull python:3.12-slim               # 拉取精简版

# 查看镜像详细信息（JSON 格式）
docker image inspect nginx:latest
docker image history nginx:latest          # 查看镜像构建历史（层）

# 导出/导入镜像（离线分发）
docker save -o nginx.tar nginx:latest      # 导出为 tar 文件
docker load -i nginx.tar                   # 从 tar 文件导入

# 镜像标签
docker tag nginx:latest myrepo/nginx:v1.0  # 打标签
docker tag <image_id> <new_tag>            # 基于 ID 打标签

# 推送/登录
docker login                               # 登录 Docker Hub
docker push myrepo/nginx:v1.0              # 推送到仓库
docker logout                              # 登出
```

### 删除镜像

```bash
# 删除镜像
docker rmi <image_id>                      # 按 ID 删除
docker rmi nginx:1.25                      # 按 仓库:标签 删除
docker rmi -f <image_id>                   # 强制删除（删除所有关联标签）

# 批量删除
docker rmi $(docker images -q)             # 删除所有镜像（危险！）
docker rmi $(docker images -q -f dangling=true) # 仅删除悬空镜像

# 清理镜像
docker image prune                         # 删除悬空镜像（无标签）
docker image prune -a                      # 删除所有未被容器使用的镜像
docker image prune -a --filter "until=24h" # 删除 24 小时前的未用镜像
```

### 构建镜像

```bash
# 基本构建
docker build -t myapp:v1 .                 # 在当前目录找 Dockerfile 构建
docker build -f Dockerfile.prod -t myapp:prod .  # 指定 Dockerfile
docker build --no-cache -t myapp:v2 .      # 不使用缓存构建

# 多平台构建（需要 buildx）
docker buildx create --use                 # 创建 builder
docker buildx build --platform linux/amd64,linux/arm64 -t myapp:multi --push .
```

---

## 3. 容器管理 (Containers)

### 运行容器

```bash
# 基本运行
docker run nginx                           # 前台运行
docker run -d nginx                        # 后台（detached）运行
docker run --name my-nginx nginx           # 指定容器名称
docker run --rm nginx                      # 退出后自动删除容器

# 端口映射 (host:container = 外面:里面)
docker run -p 8080:80 nginx                # 访问 localhost:8080 → 容器 80 端口
docker run -p 3000:3000 -p 8000:8000 myapp # 映射多个端口

# 环境变量
docker run -e MYSQL_ROOT_PASSWORD=secret mysql
docker run --env-file .env myapp           # 从文件加载环境变量

# 数据卷挂载
docker run -v mydata:/var/lib/mysql mysql  # 命名卷
docker run -v $(pwd):/app myapp            # 绑定挂载（开发常用）

# 资源限制
docker run --memory=512m --cpus=1 myapp    # 限制内存和 CPU
docker run --memory=1g --memory-swap=2g myapp # 内存 + swap

# 重启策略
docker run --restart=always nginx           # 总是重启
docker run --restart=unless-stopped nginx   # 除非手动停止
docker run --restart=on-failure:3 nginx     # 失败时重启，最多 3 次

# 交互模式
docker run -it ubuntu bash                 # 进入交互式 bash
docker run -it --rm --entrypoint=bash nginx:latest  # 覆盖默认入口
```

### 容器生命周期管理

```bash
docker start <container>                   # 启动已停止的容器
docker stop <container>                    # 优雅停止（SIGTERM → 10s 超时 → SIGKILL）
docker stop -t 30 <container>              # 自定义超时 30 秒
docker restart <container>                 # 重启容器
docker pause <container>                   # 暂停容器进程（冻结）
docker unpause <container>                 # 恢复暂停的容器
docker kill <container>                    # 立即强制停止（SIGKILL）
docker rm <container>                      # 删除已停止的容器
docker rm -f <container>                   # 强制删除（即使正在运行）
docker rename <old_name> <new_name>        # 重命名容器

# 批量操作
docker stop $(docker ps -q)                # 停止所有运行中的容器
docker rm $(docker ps -aq)                 # 删除所有容器（危险!）
```

### 查看容器信息

```bash
docker ps                                  # 列出运行中的容器
docker ps -a                               # 列出所有容器（含已停止）
docker ps -q                               # 只显示容器 ID
docker ps --format "table {{.ID}}\t{{.Image}}\t{{.Status}}"

# 详细信息
docker inspect <container>                 # 完整 JSON 信息
docker inspect -f '{{.NetworkSettings.IPAddress}}' <container>  # 提取特定字段
docker stats                               # 实时资源使用（CPU、内存、网络、磁盘）
docker stats --no-stream                   # 一次性快照
docker top <container>                     # 容器内进程列表
docker port <container>                    # 端口映射信息
docker logs <container>                    # 查看日志
docker logs -f <container>                 # 实时跟踪日志
docker logs --tail 50 <container>          # 最后 50 行
docker logs --since 1h <container>         # 最近 1 小时
docker diff <container>                    # 容器文件系统变更
```

### 与容器交互

```bash
# 在运行中的容器里执行命令
docker exec -it <container> bash           # 打开交互式 bash
docker exec -it <container> sh             # 没有 bash 时用 sh
docker exec <container> ls -la /app        # 执行单条命令

# 文件传输
docker cp <container>:/path/file ./local   # 容器 → 宿主机
docker cp ./local-file <container>:/path/  # 宿主机 → 容器

# 连接到容器的主进程
docker attach <container>                  # 附加到容器（Ctrl+P Ctrl+Q 安全退出）
```

---

## 4. 数据卷管理 (Volumes)

容器默认是无状态的 — 删除容器后所有内部数据丢失。Volume 提供持久化存储。

### Volume 类型对比

| 类型          | 存储位置                      | 适用场景          |
| ------------- | ----------------------------- | ----------------- |
| **Volumes**   | Docker 管理 (`/var/lib/docker/volumes/`) | 生产环境，持久化   |
| **Bind mounts** | 宿主机任意路径              | 开发环境，代码热更新 |
| **tmpfs**     | 宿主机内存                    | 敏感信息，高性能临时数据 |
| **Named pipes**| 宿主机和容器间 FIFO 通信     | Windows 下内存存储 |

### Volume 命令

```bash
# 管理 Volume
docker volume ls                          # 列出所有卷
docker volume create mydata               # 创建命名卷
docker volume inspect mydata              # 查看卷详情（JSON，含挂载点位置）
docker volume rm mydata                   # 删除卷
docker volume prune                       # 删除所有未被容器使用的匿名卷
docker volume prune -a                    # 删除所有未使用卷（含命名卷）

# 挂载方式1：-v（简写）
docker run -v mydata:/data alpine         # 命名卷挂载
docker run -v /host/path:/data alpine     # 绑定挂载（绝对路径）
docker run -v /data alpine                # 匿名卷（仅容器路径）

# 挂载方式2：--mount（显式，推荐用于生产）
docker run --mount type=volume,src=mydata,dst=/data alpine
docker run --mount type=bind,src=$(pwd),dst=/app alpine
docker run --mount type=tmpfs,dst=/tmp alpine

# 只读挂载
docker run -v mydata:/data:ro alpine

# Volume 备份与恢复
docker run --rm -v mydata:/src -v $(pwd):/backup alpine \
    tar -czf /backup/mydata-backup.tar.gz -C /src .
docker run --rm -v newdata:/dst -v $(pwd):/backup alpine \
    tar -xzf /backup/mydata-backup.tar.gz -C /dst
```

---

## 5. 网络管理 (Networks)

### 网络驱动类型

| 驱动        | 说明                                                         |
| ----------- | ------------------------------------------------------------ |
| **bridge**  | 默认网络驱动，单机容器间通过 IP 通信（自定义 bridge 支持 DNS 名称解析） |
| **host**    | 容器直接使用宿主机网络栈，无隔离                             |
| **none**    | 完全禁用网络                                                 |
| **overlay** | 跨主机的 Swarm 集群网络                                      |
| **macvlan** | 容器拥有独立 MAC 地址，在物理网络中显示为独立设备            |

### 网络命令

```bash
# 管理网络
docker network ls                                      # 列出所有网络
docker network create mynet                            # 创建 bridge 网络（推荐使用自定义网络）
docker network create --driver overlay myoverlay       # 创建 overlay 网络
docker network create --subnet=192.168.10.0/24 mynet   # 指定子网
docker network inspect mynet                           # 查看网络详情
docker network rm mynet                                # 删除网络
docker network prune                                   # 删除所有未使用的网络

# 连接/断开
docker network connect mynet <container>               # 将已有容器加入网络
docker network connect --alias db mynet <container>     # 加入网络并设置别名
docker network disconnect mynet <container>             # 断开容器与网络的连接

# 运行时指定网络
docker run --network mynet nginx                       # 启动时加入网络

# 自定义 bridge 网络中，容器可通过名称互相 ping 通
docker run -d --name app --network mynet myapp
docker run -d --name db --network mynet mysql
# 在 app 容器内可直接 ping db
```

---

## 6. Dockerfile 详解

### 核心指令

| 指令          | 说明                                                         |
| ------------- | ------------------------------------------------------------ |
| `FROM`        | 指定基础镜像，必须是第一条指令                               |
| `WORKDIR`     | 设置工作目录（后续指令的 CWD）                               |
| `COPY`        | 从宿主机复制文件到镜像（推荐，比 ADD 更透明）                |
| `ADD`         | 比 COPY 多了 URL 下载和自动解压 tar 功能                     |
| `RUN`         | 在镜像构建时执行命令（每执行一次创建一个新层）               |
| `CMD`         | 容器启动时的**默认**命令（可被 `docker run` 的命令覆盖）     |
| `ENTRYPOINT`  | 容器启动时的**固定**入口点（不会被覆盖，CMD 作为默认参数）   |
| `ENV`         | 设置环境变量（构建时 + 运行时都生效）                        |
| `ARG`         | 构建参数（仅构建时可用，`--build-arg` 传入）                 |
| `EXPOSE`      | 声明容器监听端口（文档性质，实际映射需 `-p` 或 `--publish`） |
| `VOLUME`      | 创建匿名卷挂载点                                             |
| `USER`        | 切换执行用户                                                 |
| `HEALTHCHECK` | 定义健康检查命令                                             |
| `SHELL`       | 指定 shell 类型                                              |

### 基础 Dockerfile 示例

```dockerfile
# 指定基础镜像
FROM python:3.12-slim

# 设置工作目录
WORKDIR /app

# 复制依赖文件（先复制依赖可利用 Docker 层缓存）
COPY requirements.txt .

# 安装依赖
RUN pip install --no-cache-dir -r requirements.txt

# 复制应用代码
COPY . .

# 声明端口（文档用途）
EXPOSE 8000

# 启动命令
CMD ["python", "main.py"]
```

### 进阶 Dockerfile 示例

```dockerfile
# 多阶段构建（Multi-stage Build）：减小最终镜像体积
# === 阶段1：构建 ===
FROM golang:1.22-alpine AS builder
WORKDIR /build
COPY go.mod go.sum ./
RUN go mod download
COPY . .
RUN CGO_ENABLED=0 go build -o myapp .

# === 阶段2：运行（最终镜像） ===
FROM alpine:3.20
RUN apk add --no-cache ca-certificates
WORKDIR /app
COPY --from=builder /build/myapp .
EXPOSE 8080
HEALTHCHECK --interval=30s --timeout=3s --retries=3 \
    CMD wget -qO- http://localhost:8080/health || exit 1
USER 1000:1000
ENTRYPOINT ["./myapp"]
```

### 构建最佳实践

```bash
# 1. 使用 .dockerignore 排除不需要的文件
# .dockerignore 示例：
# node_modules
# .git
# *.md
# .env

# 2. 先复制不常变的文件（如依赖清单），利用层缓存
# 3. 合并 RUN 指令减少层数
RUN apt-get update && apt-get install -y \
    curl \
    vim \
    && rm -rf /var/lib/apt/lists/*

# 4. 选择精简基础镜像
#    python:3.12 (full)     ~1GB
#    python:3.12-slim       ~150MB
#    python:3.12-alpine     ~50MB

# 5. 使用特定版本标签，避免 :latest
FROM python:3.12.4-slim    # 好
FROM python:latest         # 避免
```

### CMD vs ENTRYPOINT

```dockerfile
# CMD：可被 docker run 的命令覆盖
CMD ["python", "app.py"]
# docker run myimage → 执行 python app.py
# docker run myimage bash → 执行 bash（CMD 被覆盖）

# ENTRYPOINT：固定入口，CMD 作为默认参数
ENTRYPOINT ["python"]
CMD ["app.py"]
# docker run myimage → 执行 python app.py
# docker run myimage test.py → 执行 python test.py（CMD 被替换）

# 组合使用：ENTRYPOINT 定义主程序，CMD 定义默认参数
ENTRYPOINT ["nginx"]
CMD ["-g", "daemon off;"]
```

---

## 7. Docker Compose

### 基本 Compose 文件

```yaml
# docker-compose.yml
services:
  # === 应用服务 ===
  app:
    build: .                  # 从当前目录 Dockerfile 构建
    # image: myapp:v1         # 或直接使用镜像
    container_name: myapp
    ports:
      - "8000:8000"
    environment:
      - DB_HOST=db
      - DB_PORT=5432
      - APP_ENV=${APP_ENV:-production}  # 使用 .env 文件或默认值
    env_file:
      - .env.app              # 从文件加载环境变量
    volumes:
      - ./src:/app/src        # 代码热更新（开发模式）
      - app_data:/data        # 命名卷
    depends_on:
      db:
        condition: service_healthy  # 等待 db 健康检查通过
    restart: unless-stopped
    networks:
      - app-network

  # === 数据库服务 ===
  db:
    image: postgres:16-alpine
    container_name: postgres_db
    environment:
      POSTGRES_USER: myuser
      POSTGRES_PASSWORD: ${DB_PASSWORD}   # 敏感信息放 .env
      POSTGRES_DB: myapp
    volumes:
      - pgdata:/var/lib/postgresql/data    # 持久化数据
    ports:
      - "5432:5432"
    healthcheck:
      test: ["CMD-SHELL", "pg_isready -U myuser"]
      interval: 10s
      timeout: 5s
      retries: 5
    networks:
      - app-network

  # === Nginx 反向代理 ===
  nginx:
    image: nginx:1.25-alpine
    ports:
      - "80:80"
    volumes:
      - ./nginx.conf:/etc/nginx/nginx.conf:ro
    depends_on:
      - app
    networks:
      - app-network

# 定义命名卷（必须在此声明）
volumes:
  pgdata:
  app_data:

# 定义自定义网络
networks:
  app-network:
    driver: bridge
```

### Compose 命令

```bash
# 启动/停止
docker compose up                    # 前台启动所有服务
docker compose up -d                 # 后台启动
docker compose up --build            # 重新构建并启动
docker compose up -d --scale app=3   # 启动 app 服务的 3 个实例
docker compose down                  # 停止并删除容器、网络
docker compose down -v               # 同时删除数据卷
docker compose down --rmi all        # 同时删除镜像

# 服务管理
docker compose start                 # 启动已停止的服务
docker compose stop                  # 停止服务（保留容器）
docker compose restart               # 重启所有服务
docker compose pause / unpause       # 暂停/恢复

# 查看状态
docker compose ps                    # 列出所有服务
docker compose ls                    # 列出所有 compose 项目
docker compose logs                  # 查看所有日志
docker compose logs -f app           # 实时跟踪特定服务日志
docker compose logs --tail=50 app    # 最后 50 行
docker compose top                   # 查看运行中的进程

# 与运行中的服务交互
docker compose exec app bash         # 进入 app 服务的容器
docker compose exec db psql -U myuser  # 在 db 中执行命令

# 构建与拉取
docker compose build                 # 构建服务镜像
docker compose build --no-cache      # 强制重新构建
docker compose pull                  # 拉取最新镜像

# 配置验证
docker compose config                # 验证并查看最终的 compose 配置
docker compose config --services     # 列出所有服务名
```

### `.env` 文件

在 `docker-compose.yml` 同目录下创建 `.env` 文件：

```bash
# .env
DB_PASSWORD=super_secret_password
APP_ENV=development
MY_CUSTOM_VAR=some_value
```

---

## 8. 清理与维护

```bash
# 查看磁盘占用
docker system df                     # 概览
docker system df -v                  # 详细清单

# 一键清理
docker system prune                  # 删除所有停止的容器、未用网络、悬空镜像、构建缓存
docker system prune -a               # 再删除所有未使用的镜像
docker system prune -a --volumes     # 再删除所有未使用的卷（危险！）

# 分类清理
docker container prune               # 删除已停止的容器
docker image prune -a                # 删除未使用的镜像
docker volume prune                  # 删除未使用的卷
docker network prune                 # 删除未使用的网络

# 查看 Docker 事件流（调试用）
docker events                        # 实时事件
docker events --since 1h             # 最近 1 小时的事件
```

---

## 9. Docker 实战案例

### 案例1：Python Web 应用容器化

```dockerfile
# Dockerfile
FROM python:3.12-slim

WORKDIR /app

# 安装系统依赖
RUN apt-get update && apt-get install -y --no-install-recommends \
    gcc \
    && rm -rf /var/lib/apt/lists/*

# 安装 Python 依赖
COPY requirements.txt .
RUN pip install --no-cache-dir -r requirements.txt

# 复制代码
COPY . .

# 创建非 root 用户
RUN useradd -m -u 1000 appuser && chown -R appuser:appuser /app
USER appuser

EXPOSE 8000

# 健康检查
HEALTHCHECK --interval=30s --timeout=3s --start-period=5s \
    CMD curl -f http://localhost:8000/health || exit 1

CMD ["uvicorn", "main:app", "--host", "0.0.0.0", "--port", "8000"]
```

### 案例2：Django + PostgreSQL + Redis 的 Compose 配置

```yaml
services:
  web:
    build: .
    command: python manage.py runserver 0.0.0.0:8000
    volumes:
      - .:/app
    ports:
      - "8000:8000"
    environment:
      - DATABASE_URL=postgres://user:pass@db:5432/mydb
      - REDIS_URL=redis://redis:6379/0
    depends_on:
      - db
      - redis

  db:
    image: postgres:16-alpine
    volumes:
      - postgres_data:/var/lib/postgresql/data
    environment:
      POSTGRES_USER: user
      POSTGRES_PASSWORD: pass
      POSTGRES_DB: mydb

  redis:
    image: redis:7-alpine
    volumes:
      - redis_data:/data

  celery:
    build: .
    command: celery -A myproject worker -l INFO
    volumes:
      - .:/app
    environment:
      - DATABASE_URL=postgres://user:pass@db:5432/mydb
      - REDIS_URL=redis://redis:6379/0
    depends_on:
      - db
      - redis

volumes:
  postgres_data:
  redis_data:
```

### 案例3：一键开发环境启动脚本

```bash
#!/bin/bash
set -euo pipefail

echo "🚀 启动开发环境..."

# 检查 Docker 是否运行
if ! docker info > /dev/null 2>&1; then
    echo "❌ Docker 未运行，请先启动 Docker" >&2
    exit 1
fi

# 后端
echo "  → 启动 PostgreSQL..."
docker run -d --name dev-postgres \
    -e POSTGRES_PASSWORD=devpass \
    -p 5432:5432 \
    postgres:16-alpine

# 缓存
echo "  → 启动 Redis..."
docker run -d --name dev-redis \
    -p 6379:6379 \
    redis:7-alpine

# 消息队列
echo "  → 启动 RabbitMQ..."
docker run -d --name dev-rabbitmq \
    -p 5672:5672 -p 15672:15672 \
    rabbitmq:3-management-alpine

echo "✅ 开发环境就绪!"
echo ""
echo "连接信息:"
echo "  PostgreSQL: localhost:5432 (user: postgres, pass: devpass)"
echo "  Redis:      localhost:6379"
echo "  RabbitMQ:   localhost:5672 (管理界面: http://localhost:15672)"
```

---

# Part 2: Kubernetes

## 10. K8s 核心概念

### 架构概览

```
┌──────────────────────────────────────────────────┐
│                   Control Plane                   │
│  ┌─────────┐  ┌──────────┐  ┌─────────────────┐ │
│  │ API     │  │ Scheduler│  │ Controller      │ │
│  │ Server  │  │          │  │ Manager         │ │
│  └─────────┘  └──────────┘  └─────────────────┘ │
│  ┌──────────────────────────────────────────┐    │
│  │              etcd (数据存储)               │    │
│  └──────────────────────────────────────────┘    │
└──────────────────────────────────────────────────┘
          │                  │
    ┌─────┴──────┐    ┌─────┴──────┐
    │  Worker    │    │  Worker    │
    │  Node 1    │    │  Node 2    │
    │ ┌───────┐  │    │ ┌───────┐  │
    │ │ Pods  │  │    │ │ Pods  │  │
    │ └───────┘  │    │ └───────┘  │
    └────────────┘    └────────────┘
```

### 核心资源对象

| 资源              | 缩写   | 说明                                          |
| ----------------- | ------ | --------------------------------------------- |
| **Pod**           | `po`   | 最小部署单元，包含一个或多个容器              |
| **Deployment**    | `deploy` | 声明式管理 Pod 和 ReplicaSet，支持滚动更新   |
| **Service**       | `svc`  | 为 Pod 提供稳定的网络访问入口（负载均衡）     |
| **ConfigMap**     | `cm`   | 非敏感配置数据，以键值对存储                  |
| **Secret**        | —      | 敏感数据（密码、Token 等），Base64 编码存储   |
| **Namespace**     | `ns`   | 虚拟集群，资源隔离                            |
| **Ingress**       | `ing`  | 管理集群外部 HTTP/HTTPS 访问                  |
| **ReplicaSet**    | `rs`   | 维护指定数量的 Pod 副本                       |
| **StatefulSet**   | `sts`  | 有状态应用（数据库等）                        |
| **DaemonSet**     | `ds`   | 每个节点运行一个 Pod                          |
| **Job**/ **CronJob** | `cj` | 一次性/定时任务                               |
| **PersistentVolume** | `pv` | 持久化存储抽象                                |
| **PersistentVolumeClaim** | `pvc` | 用户对存储的请求                          |

---

## 11. Pod（容器组）

Pod 是 K8s 中可部署的最小单元，包含一个或多个共享网络和存储的容器。

### 创建和管理 Pod

```bash
# 快速运行一个 Pod
kubectl run nginx --image=nginx:latest
kubectl run debug --rm -it --image=busybox -- sh   # 临时调试 Pod

# 从 YAML 创建
kubectl apply -f pod.yaml

# 查看 Pod
kubectl get pods                           # 默认命名空间
kubectl get pods -A                        # 所有命名空间
kubectl get pods -o wide                   # 显示 IP、节点等
kubectl get pods --show-labels             # 显示标签
kubectl get pods -l app=nginx              # 按标签筛选
kubectl get pods --field-selector=status.phase=Running  # 按状态筛选

# Pod 详情
kubectl describe pod <pod-name>            # 详细状态 + Events
kubectl logs <pod-name>                    # 查看日志
kubectl logs -f <pod-name>                 # 实时跟踪日志
kubectl logs --tail=100 <pod-name>         # 最后 100 行
kubectl logs -c <container> <pod-name>     # 多容器 Pod 指定容器
kubectl logs --previous <pod-name>         # 查看之前崩溃容器的日志

# 与 Pod 交互
kubectl exec -it <pod-name> -- /bin/bash   # 进入 Pod 内的 shell
kubectl exec <pod-name> -- ls /app         # 执行单条命令
kubectl cp <pod>:/path /local/path         # 复制文件 (pod → 本地)
kubectl cp /local/path <pod>:/path         # 复制文件 (本地 → pod)
kubectl port-forward <pod> 8080:80         # 端口转发（本地调试）

# 删除 Pod
kubectl delete pod <pod-name>
kubectl delete pod <pod-name> --grace-period=0 --force  # 强制删除
```

### Pod YAML 模板

```yaml
apiVersion: v1
kind: Pod
metadata:
  name: my-app-pod
  labels:
    app: my-app
    env: dev
spec:
  containers:
  - name: app
    image: myapp:v1.0
    ports:
    - containerPort: 8000
      protocol: TCP
    env:
    - name: DB_HOST
      value: "db-service"
    - name: DB_PASSWORD
      valueFrom:
        secretKeyRef:
          name: db-secret
          key: password
    resources:
      requests:               # 调度所需的最小资源
        memory: "64Mi"
        cpu: "100m"           # 100m = 0.1 CPU
      limits:                 # 最大可用资源
        memory: "256Mi"
        cpu: "500m"
    livenessProbe:            # 存活探针（失败 → 重启容器）
      httpGet:
        path: /healthz
        port: 8000
      initialDelaySeconds: 15
      periodSeconds: 10
    readinessProbe:           # 就绪探针（失败 → 从 Service 移除）
      httpGet:
        path: /ready
        port: 8000
      initialDelaySeconds: 5
      periodSeconds: 5
  restartPolicy: Always       # Always | OnFailure | Never
```

---

## 12. Deployment（部署）

Deployment 管理 Pod 的声明式更新，支持滚动更新和回滚。

### 命令操作

```bash
# 创建 Deployment
kubectl create deployment myapp --image=myapp:v1
kubectl create deployment myapp --image=myapp:v1 --replicas=3
kubectl apply -f deployment.yaml

# 查看
kubectl get deployments
kubectl get deploy -o wide
kubectl describe deployment <name>

# 扩缩容
kubectl scale deployment myapp --replicas=5

# 更新镜像（触发滚动更新）
kubectl set image deployment/myapp app=myapp:v2

# 滚动更新管理
kubectl rollout status deployment/myapp       # 查看更新进度
kubectl rollout history deployment/myapp      # 查看发布历史
kubectl rollout undo deployment/myapp         # 回滚到上一版本
kubectl rollout undo deployment/myapp --to-revision=2  # 回滚到指定版本
kubectl rollout pause deployment/myapp        # 暂停发布
kubectl rollout resume deployment/myapp       # 恢复发布

# 编辑
kubectl edit deployment myapp                 # 在线编辑

# 自动伸缩（HPA）
kubectl autoscale deployment myapp --min=2 --max=10 --cpu-percent=80

# 删除
kubectl delete deployment myapp
```

### Deployment YAML 模板

```yaml
apiVersion: apps/v1
kind: Deployment
metadata:
  name: myapp-deployment
  labels:
    app: myapp
spec:
  replicas: 3                          # Pod 副本数
  revisionHistoryLimit: 10             # 保留的历史版本数
  strategy:
    type: RollingUpdate                # RollingUpdate | Recreate
    rollingUpdate:
      maxUnavailable: 25%             # 更新期间最多不可用的 Pod 比例
      maxSurge: 25%                   # 更新期间最多超出期望的 Pod 比例
  selector:
    matchLabels:
      app: myapp
  template:                            # Pod 模板
    metadata:
      labels:
        app: myapp
        version: v1
    spec:
      containers:
      - name: myapp
        image: myapp:v1.0
        ports:
        - containerPort: 8000
        resources:
          requests:
            memory: "128Mi"
            cpu: "250m"
          limits:
            memory: "256Mi"
            cpu: "500m"
```

---

## 13. Service（服务）

Service 为 Pod 提供稳定的 IP 和 DNS 名称，实现负载均衡。

### Service 类型

| 类型             | 访问方式                     | 使用场景            |
| ---------------- | ---------------------------- | ------------------- |
| **ClusterIP**    | 仅集群内部访问               | 内部服务间通信（默认）|
| **NodePort**     | `<NodeIP>:30000-32767`       | 开发测试外部访问    |
| **LoadBalancer** | 云提供商负载均衡器分配外部IP  | 生产环境外部访问    |
| **ExternalName** | DNS CNAME 映射               | 将外部服务映射到集群 |

### 命令操作

```bash
# 为 Deployment 创建 Service
kubectl expose deployment myapp --port=80 --target-port=8000
kubectl expose deployment myapp --type=NodePort --port=80
kubectl expose deployment myapp --type=LoadBalancer --port=80

# 查看
kubectl get services
kubectl get svc -o wide
kubectl describe service <name>
kubectl get endpoints <name>                  # 查看 Service 后端 Pod IP

# 端口转发（本地调试）
kubectl port-forward service/myapp 8080:80

# 删除
kubectl delete service <name>
```

### Service YAML 模板

```yaml
apiVersion: v1
kind: Service
metadata:
  name: myapp-service
spec:
  type: NodePort               # ClusterIP | NodePort | LoadBalancer
  selector:
    app: myapp                 # 选择后端 Pod 的标签
  ports:
  - name: http
    protocol: TCP
    port: 80                   # Service 的端口
    targetPort: 8000           # 后端 Pod 的容器端口
    nodePort: 30080            # 节点端口 (30000-32767，可选)

---
# LoadBalancer 类型示例
apiVersion: v1
kind: Service
metadata:
  name: myapp-lb
spec:
  type: LoadBalancer
  selector:
    app: myapp
  ports:
  - port: 80
    targetPort: 8000
```

---

## 14. ConfigMap & Secret

### ConfigMap（配置管理）

```bash
# 从文件/目录创建
kubectl create configmap app-config --from-file=config.yaml
kubectl create configmap app-config --from-file=configs/

# 从字面量创建
kubectl create configmap app-config \
    --from-literal=DB_HOST=postgres \
    --from-literal=LOG_LEVEL=debug

# 查看
kubectl get configmaps
kubectl describe configmap app-config

# 删除
kubectl delete configmap app-config
```

ConfigMap YAML:

```yaml
apiVersion: v1
kind: ConfigMap
metadata:
  name: app-config
data:
  database_url: "postgres://db:5432/mydb"
  log_level: "info"
  config.yaml: |
    server:
      port: 8000
      workers: 4
    cache:
      ttl: 300
```

在 Pod 中使用 ConfigMap:

```yaml
spec:
  containers:
  - name: app
    image: myapp
    # 方式1：作为环境变量
    env:
    - name: DATABASE_URL
      valueFrom:
        configMapKeyRef:
          name: app-config
          key: database_url
    # 方式2：全部作为环境变量
    envFrom:
    - configMapRef:
        name: app-config
    # 方式3：挂载为文件
    volumeMounts:
    - name: config
      mountPath: /app/config
  volumes:
  - name: config
    configMap:
      name: app-config
```

### Secret（密钥管理）

```bash
# 从字面量创建
kubectl create secret generic db-secret \
    --from-literal=username=admin \
    --from-literal=password=MyP@ssw0rd

# 从文件创建
kubectl create secret generic tls-secret \
    --from-file=tls.crt --from-file=tls.key

# 创建 Docker Registry 密钥
kubectl create secret docker-registry regcred \
    --docker-server=myregistry.com \
    --docker-username=user \
    --docker-password=pass

# 查看（值以 Base64 显示）
kubectl get secrets
kubectl describe secret db-secret
kubectl get secret db-secret -o jsonpath='{.data.password}' | base64 -d

# 删除
kubectl delete secret db-secret
```

---

## 15. Namespace（命名空间）

```bash
# 查看
kubectl get namespaces
kubectl get ns

# 创建/删除
kubectl create namespace dev
kubectl create namespace staging
kubectl create namespace production
kubectl delete namespace dev

# 在特定命名空间操作（使用 -n 标志）
kubectl get pods -n production
kubectl apply -f deployment.yaml -n production
kubectl delete -f deployment.yaml -n production

# 切换默认命名空间
kubectl config set-context --current --namespace=production

# 查看所有命名空间的资源
kubectl get pods -A
kubectl get pods --all-namespaces
```

---

## 16. 常用 kubectl 命令速查

### 输出格式

```bash
kubectl get pods -o wide          # 扩展信息
kubectl get pods -o yaml          # YAML 格式
kubectl get pods -o json          # JSON 格式
kubectl get pods -o jsonpath='{.items[*].status.podIP}'  # 自定义字段提取
kubectl get pods -o custom-columns=NAME:.metadata.name,IP:.status.podIP
kubectl get pods --sort-by=.metadata.creationTimestamp  # 排序
kubectl get pods -w               # 持续监听变化（watch）
```

### 声明式 vs 命令式

```bash
# 声明式（推荐，适合生产）：使用 apply
kubectl apply -f deployment.yaml           # 创建或更新
kubectl apply -f ./k8s/                    # 应用整个目录
kubectl apply -k ./k8s/overlays/prod/      # 使用 kustomize

# 命令式（适合开发调试）
kubectl create -f deployment.yaml          # 仅创建（如已存在则报错）
kubectl delete -f deployment.yaml
kubectl replace -f deployment.new.yaml     # 完整替换
```

### 调试命令

```bash
# 查看集群事件
kubectl get events --sort-by='.lastTimestamp' -A
kubectl get events -w                     # 持续监听

# 资源使用
kubectl top pods                          # Pod 资源使用（需要 metrics-server）
kubectl top pods -n production --sort-by=memory
kubectl top nodes                         # 节点资源使用

# API 资源
kubectl api-resources                     # 列出所有 API 资源类型
kubectl api-versions                      # 列出所有 API 版本
kubectl explain pod                       # 查看 Pod 资源文档
kubectl explain deployment.spec.strategy  # 查看特定字段文档

# 查看集群信息
kubectl cluster-info
kubectl version
kubectl config view                       # 查看 kubeconfig
kubectl config get-contexts               # 列出所有上下文
kubectl config use-context <name>         # 切换上下文（集群）
```

### 常见故障排查

| 状态                  | 原因                     | 排查命令                             |
| --------------------- | ------------------------ | ------------------------------------ |
| `ImagePullBackOff`    | 镜像拉取失败             | `kubectl describe pod`              |
| `ErrImagePull`        | 镜像不存在或认证失败     | 检查镜像名、`docker-registry` secret |
| `CrashLoopBackOff`    | 容器反复崩溃             | `kubectl logs --previous`           |
| `Pending`             | 资源不足或无法调度       | `kubectl describe pod`              |
| `OOMKilled`           | 内存超限                 | 增大 `limits.memory`                |
| `Evicted`             | 节点资源压力被驱逐       | `kubectl top nodes`                 |
| `CreateContainerConfigError` | ConfigMap/Secret 问题 | `kubectl describe pod`              |

### 资源缩写速查

| 完整名                  | 缩写    | 完整名              | 缩写   |
| ----------------------- | ------- | ------------------- | ------ |
| `pods`                  | `po`    | `services`          | `svc`  |
| `deployments`           | `deploy`| `replicasets`       | `rs`   |
| `statefulsets`          | `sts`   | `daemonsets`        | `ds`   |
| `configmaps`            | `cm`    | `secrets`           | —      |
| `namespaces`            | `ns`    | `nodes`             | `no`   |
| `persistentvolumes`     | `pv`    | `persistentvolumeclaims` | `pvc` |
| `ingresses`             | `ing`   | `horizontalpodautoscalers` | `hpa` |
| `cronjobs`              | `cj`    | `serviceaccounts`   | `sa`   |
| `networkpolicies`       | `netpol`| `clusterroles`      | —      |

---

## 17. K8s 集群管理工具

### 本地开发集群

| 工具        | 说明                                     |
| ----------- | ---------------------------------------- |
| **Kind**    | Docker 中运行 K8s 集群（轻量推荐）       |
| **Minikube**| 单节点 K8s（功能最全）                   |
| **k3s**     | 轻量级 K8s（适合边缘/嵌入式）            |
| **MicroK8s**| Canonical 出品，snap 安装                 |

### Kind 命令

```bash
kind create cluster                        # 创建默认集群
kind create cluster --name mycluster       # 创建命名集群
kind create cluster --config kind-config.yaml  # 从配置创建
kind get clusters                          # 列出集群
kind load docker-image myapp:v1 --name mycluster  # 加载本地镜像到集群
kind delete cluster                        # 删除默认集群
kind delete cluster --name mycluster       # 删除命名集群
```

### eksctl（AWS EKS）

```bash
# 创建集群
eksctl create cluster --name mycluster --region us-east-1

# 从 YAML 配置创建
eksctl create cluster -f cluster.yaml

# 删除
eksctl delete cluster --name mycluster
eksctl delete cluster -f cluster.yaml
```

EKS 配置示例：

```yaml
apiVersion: eksctl.io/v1alpha5
kind: ClusterConfig

metadata:
  name: my-cluster
  region: us-east-1
  version: "1.29"

nodeGroups:
  - name: standard-workers
    instanceType: t3.medium
    desiredCapacity: 3
    minSize: 1
    maxSize: 10
    volumeSize: 50
```

---

## 18. 辅助工具

### Multipass（轻量 Ubuntu VM）

```bash
multipass launch --name dev-vm           # 创建实例（最新 LTS）
multipass list                           # 列出所有实例
multipass shell dev-vm                   # 进入实例 shell
multipass exec dev-vm -- ls /            # 执行命令
multipass mount ./code dev-vm:/code      # 挂载本地目录
multipass umount dev-vm                  # 卸载所有挂载
multipass stop dev-vm                    # 停止实例
multipass start dev-vm                   # 启动实例
multipass info dev-vm                    # 查看详情
multipass delete dev-vm                  # 删除（移到回收站）
multipass recover dev-vm                 # 恢复已删除实例
multipass purge                          # 永久删除所有已删除实例
```

---

> **参考资源**:
> - [Docker 官方文档](https://docs.docker.com/)
> - [Kubernetes 官方文档](https://kubernetes.io/docs/)
> - [kubectl 命令速查表](https://kubernetes.io/docs/reference/kubectl/cheatsheet/)
> - [Docker Compose 文档](https://docs.docker.com/compose/)
> - [Kubernetes 对象详解](https://kubernetes.io/docs/concepts/overview/working-with-objects/)
