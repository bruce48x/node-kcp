// 捕获普通异常
process.on('uncaughtException', function (err) {
    console.error('Caught exception: ' + err.stack);
});

// 捕获async异常
process.on('unhandledRejection', function (reason, p) {
    console.error('Caught Unhandled Rejection at:' + p + 'reason:' + reason.stack);
});

function log(...msg) {
    console.log('[', new Date().toISOString(), ']', ...msg);
}

module.exports = {
    log,
    address: '127.0.0.1',
    port: 22333,
}
