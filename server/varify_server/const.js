let code_prefix = new Array("RegisterCode:", "ResetPwdCode:");

const Errors = {
    Success: 0,
    RedisErr: 1,
    Exception: 2,
};

const CodeModule = {
    Register: 0,
    ResetPwd: 1
};

module.exports = { code_prefix, Errors, CodeModule }