const kcp = require('..');
const dgram = require('dgram');
const server = dgram.createSocket('udp4');
const clients = {};
const interval = 100;

const output = function (data, size, context) {
    server.send(data, 0, size, context.port, context.address);
};

server.on('error', (err) => {
    console.log(`server error:\n${err.stack}`);
    server.close();
});

server.on('message', (data, rinfo) => {
    const k = rinfo.address + '_' + rinfo.port;
    if (undefined === clients[k]) {
        const context = {
            address: rinfo.address,
            port: rinfo.port
        };
        const kcpobj = new kcp.KCP(123, context);
        kcpobj.stream(1);
        kcpobj.nodelay(0, interval, 0, 0);
        kcpobj.output(output);
        clients[k] = kcpobj;
    }
    const kcpobj = clients[k];
    kcpobj.input(data);
    const recv = kcpobj.recv();
    if (recv) {
        console.log(`Server recv ${recv} from ${kcpobj.context().address}:${kcpobj.context().port}`);
        kcpobj.send(recv);
    }
});

server.on('listening', () => {
    const address = server.address();
    console.log(`server listening ${address.address} : ${address.port}`);
    setInterval(() => {
        for (const k in clients) {
            const kcpobj = clients[k];
            kcpobj.update(Date.now());
        }
    }, interval);
});

server.bind(41234);
