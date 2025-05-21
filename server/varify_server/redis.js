const config_module = require("./config");
const Redis = require("ioredis");

// 创建Redis客户端实例
const RedisCli = new Redis({
    host: config_module.redis_host,
    port: config_module.redis_port,
    password: config_module.redis_passwd,
});

// 监听错误信息
RedisCli.on("error", function (err) {
    console.log("RedisCli connect error");
    RedisCli.quit();
});

async function setRedisExpire(key, value, exptime) {
    try {
        await RedisCli.set(key, value);
        await RedisCli.expire(key, exptime);
        return true;
    } catch (error) {
        console.log("SetRedisExpire error:", error);
        return false;
    }
}

function quit() {
    RedisCli.quit();
}

module.exports = { setRedisExpire };