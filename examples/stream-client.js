const kcp = require('bindings')('kcp');
const Kcp = kcp.KCP;
const dgram = require('dgram');
const { log, address, port } = require('./common');

const kcpObj = new Kcp(255, { address, port });
kcpObj.stream(1);
const client = dgram.createSocket('udp4');

kcpObj.output((data, size, context) => {
    client.send(data, 0, size, context.port, context.address);
});

client.on('error', (err) => {
    log(`cleint error:${err.stack}`);
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
        log(`recv: ${buffer.length}`);
    }
}, 100);

let i=0;
setInterval(() => {
    const rand = 200000 + Math.floor(Math.random() * 50000);
    const buff = Buffer.allocUnsafe(rand).fill(65 + i%26);
    i++;
    log(`send: ${buff}`);
    kcpObj.send(buff.toString('hex'));
}, 1000);
