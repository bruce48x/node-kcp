# node-kcp-x

<p align="center">
| <b>English</b> | <a href="./README_CN.md"><b>简体中文</b></a> |
</p>

[1]: https://api.travis-ci.org/leenjewel/node-kcp.svg?branch=master
[2]: https://travis-ci.org/leenjewel/node-kcp

[KCP Protocol](https://github.com/skywind3000/kcp) for Node.js

A fork of [node-kcp](https://github.com/leenjewel/node-kcp), rewritten using [node-addon-api](https://github.com/nodejs/node-addon-api)

## Features

- **High Performance**: 30%-40% reduction in average RTT compared to traditional ARQ protocols like TCP
- **Low Latency**: Maximum RTT reduced by 3x compared to TCP
- **Lightweight**: Distributed as a single source file
- **Modern API**: Built with node-addon-api for better Node.js compatibility
- **Cross Platform**: Supports Windows, Linux, and macOS

## Installation

```bash
npm install node-kcp-x
```

## Building from Source

### Prerequisites

```bash
npm install -g node-gyp prebuildify
```

### Build Steps

```bash
git clone https://github.com/bruce48x/node-kcp
cd node-kcp
npm i
npm run build
```

## Usage Examples

### Example 1: Basic UDP Test

```bash
npm run install
node test/udpserver.js
node test/udpclient.js
```

### Example 2: Client-Server Communication

```bash
# Run server
node examples/server.js
# Run client
node examples/client.js
```

### Example 3: Stream Mode

```bash
# Run stream server
node examples/stream-server.js
# Run stream client
node examples/stream-client.js
```

## API Reference

### KCP Class

```javascript
const kcp = require('node-kcp-x');
const Kcp = kcp.KCP;

// Create KCP instance
const kcpObj = new Kcp(conv, context);

// Set output callback
kcpObj.output((data, size, context) => {
    // Send data to network
});

// Send data
kcpObj.send(buffer);

// Receive data
const buffer = kcpObj.recv();

// Update KCP state
kcpObj.update(timestamp);

// Check next update time
const nextUpdate = kcpObj.check(timestamp);
```

## About Pomelo and Pomelo-kcp

If you want to use [node-kcp](https://github.com/leenjewel/node-kcp) in [pomelo](https://github.com/NetEase/pomelo/) server, you need [pomelo-kcp](https://github.com/leenjewel/pomelo-kcp)

## License

Apache-2.0