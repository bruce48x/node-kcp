var kcp = require('./../build/Debug/kcp');
var kcpobj = new kcp.KCP(123, { address: '127.0.0.1', port: 41234 });
var dgram = require('dgram');
var client = dgram.createSocket('udp4');
var idx = 1;
var interval = 100;

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
    var recv = kcpobj.recv();
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
