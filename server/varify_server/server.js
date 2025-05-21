const grpc = require("@grpc/grpc-js")
const message_proto = require("./proto")
const const_module = require("./const")
const emailModule = require("./email")
//const { v4: uuidv4 } = require("uuid")
const crypto = require("crypto");
const redis_module = require("./redis")

async function GetVarifyCode(call, callback) {
    console.log("email is ", call.request.email)
    try {
        //uniqueId = uuidv4();
        const code = crypto.randomInt(0, 1000000);
        const paddedCode = code.toString().padStart(6, "0");
        console.log("varify code is ", paddedCode)
        let bres = await redis_module.setRedisExpire(const_module.code_prefix + call.request.email, paddedCode, 1800);
        if (!bres) {
            callback(null, {
                email: call.request.email,
                error: const_module.Errors.RedisErr
            });
            return;
        }
        let text_str = '您的验证码为 ' + paddedCode + ' 三十分钟内有效'
        //发送邮件
        let mailOptions = {
            from: 'qtchatting@163.com',
            to: call.request.email,
            subject: '验证码',
            text: text_str,
        };

        let send_res = await emailModule.SendMail(mailOptions);
        console.log("Send res: ", send_res)

        callback(null, {
            email: call.request.email,
            error: const_module.Errors.Success
        })


    } catch (error) {
        console.log("catch error is ", error)

        callback(null, {
            email: call.request.email,
            error: const_module.Errors.Exception
        })
    }

}

function main() {
    var server = new grpc.Server()
    server.addService(message_proto.VarifyService.service, { GetVarifyCode: GetVarifyCode })
    server.bindAsync('0.0.0.0:50051', grpc.ServerCredentials.createInsecure(), () => {
        console.log('grpc server started')
    })
}

main()