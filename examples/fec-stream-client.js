const kcp = require('..');
const Kcp = kcp.KCP;
const dgram = require('dgram');
const { log, address, port } = require('./common');

const kcpObj = new Kcp(255, {
    address, port, fec: {
        dataShards: 4,
        parityShards: 1,
    },
});
kcpObj.stream(1);
const client = dgram.createSocket('udp4');

kcpObj.output((data, size, context) => {
    client.send(data, 0, size, context.port, context.address);
});

client.on('error', (err) => {
    log(`client error:${err.stack}`);
    client.close();
});

client.on('message', (msg, rinfo) => {
    kcpObj.input(msg);
});

setInterval(() => {
    const now = Date.now();
    kcpObj.update(now);
    const size = kcpObj.peeksize();
    if (size > 0) {
        const buffer = kcpObj.recv();
        log(`recv data length: ${buffer.byteLength}`);
    }
}, 100);

let i = 0;
setInterval(() => {
    const rand = 200000 + Math.floor(Math.random() * 50000);
    const buff = Buffer.allocUnsafe(rand).fill(65 + i % 26);
    i++;
    log(`send data length: ${buff.byteLength}`);
    kcpObj.send(buff);
}, 1000);
