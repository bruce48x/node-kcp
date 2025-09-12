# node-kcp-x

<p align="center">
| <a href="./README.md"><b>English</b></a> | <b>简体中文</b> |
</p>

[1]: https://api.travis-ci.org/leenjewel/node-kcp.svg?branch=master
[2]: https://travis-ci.org/leenjewel/node-kcp

Node.js 的 [KCP 协议](https://github.com/skywind3000/kcp) 实现

基于 [node-kcp](https://github.com/leenjewel/node-kcp) 修改，使用 [node-addon-api](https://github.com/nodejs/node-addon-api) 重写

## 特性

- **高性能**: 相比传统 ARQ 协议（如 TCP），平均 RTT 减少 30%-40%
- **低延迟**: 最大 RTT 比 TCP 减少 3 倍
- **轻量级**: 单文件分发
- **现代 API**: 使用 node-addon-api 构建，更好的 Node.js 兼容性
- **跨平台**: 支持 Windows、Linux 和 macOS

## 安装

```bash
npm install node-kcp-x
```

## 从源码构建

### 前置要求

```bash
npm install -g node-gyp prebuildify
```

### 构建步骤

```bash
git clone https://github.com/bruce48x/node-kcp
cd node-kcp
npm i
npm run build
```

## 使用示例

### 示例 1: 基础 UDP 测试

```bash
npm run install
node test/udpserver.js
node test/udpclient.js
```

### 示例 2: 客户端-服务器通信

```bash
# 运行服务器
node examples/server.js
# 运行客户端
node examples/client.js
```

### 示例 3: 流模式

```bash
# 运行流服务器
node examples/stream-server.js
# 运行流客户端
node examples/stream-client.js
```

## API 参考

### KCP 类

```javascript
const kcp = require('node-kcp-x');
const Kcp = kcp.KCP;

// 创建 KCP 实例
const kcpObj = new Kcp(conv, context);

// 设置输出回调
kcpObj.output((data, size, context) => {
    // 发送数据到网络
});

// 发送数据
kcpObj.send(buffer);

// 接收数据
const buffer = kcpObj.recv();

// 更新 KCP 状态
kcpObj.update(timestamp);

// 检查下次更新时间
const nextUpdate = kcpObj.check(timestamp);
```

## 关于 Pomelo 和 Pomelo-kcp

如果您想在 [pomelo](https://github.com/NetEase/pomelo/) 服务器中使用 [node-kcp](https://github.com/leenjewel/node-kcp)，您需要 [pomelo-kcp](https://github.com/leenjewel/pomelo-kcp)

## 许可证

Apache-2.0
