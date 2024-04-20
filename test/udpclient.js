const kcp = require('..');
const kcpobj = new kcp.KCP(123, { address: '127.0.0.1', port: 41234 });
const dgram = require('dgram');
const client = dgram.createSocket('udp4');
const idx = 1;
const interval = 100;

kcpobj.stream(1);
kcpobj.nodelay(0, interval, 0, 0);

kcpobj.output((data, size, context) => {
    client.send(data, 0, size, context.port, context.address);
});

client.on('error', (err) => {
    console.log(`client error:\n${err.stack}`);
    client.close();
});

client.on('message', (data, rinfo) => {
    kcpobj.input(data);
    const recv = kcpobj.recv();
    if (recv) {
        console.log(`[${new Date().toISOString()}] Client recv: ${recv}`);
    }
});

setInterval(() => {
    kcpobj.update(Date.now());
}, interval);

setInterval(() => {
    const msg = new Date().toISOString();
    console.log(`[${new Date().toISOString()}]`, 'send', msg);
    kcpobj.send(Buffer.from(msg));
}, 1000);
