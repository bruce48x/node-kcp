const kcp = require('..');
const Kcp = kcp.KCP;
const dgram = require('dgram');
const { log, port, conv, fec } = require('./common');

const server = dgram.createSocket('udp4');
const clients = {};

function output(data, size, context) {
    server.send(data, 0, size, context.port, context.address);
}

server.on('error', (err) => {
    log(`server error: ${err.stack}`);
    server.close();
});

server.on('message', (msg, { address, port }) => {
    const k = address + '_' + port;
    if (!clients[k]) {
        const kcpObj = new Kcp(conv, {
            address,
            port,
        });
        kcpObj.output(output);
        if (fec && Object.keys(fec).length > 0) {
            kcpObj.fec(fec.dataShards, fec.parityShards);
        }
        clients[k] = kcpObj;
        check(kcpObj);
    }

    const kcpObj = clients[k];
    kcpObj.input(msg);

    const size = kcpObj.peeksize();
    if (size > 0) {
        const buffer = kcpObj.recv();
        log(`recv: ${buffer} from ${kcpObj.context().address}:${kcpObj.context().port}`);
        kcpObj.send(Buffer.from(buffer));
    }
});

server.on('listening', () => {
    const address = server.address();
    log(`server listening ${address.address} : ${address.port}`);
});

function check(kcpObj) {
    if (!kcpObj) {
        return;
    }
    const now = Date.now();
    kcpObj.update(now);
    setTimeout(() => {
        check(kcpObj);
    }, kcpObj.check(now));
}

server.bind(port);
