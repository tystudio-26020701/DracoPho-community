#pragma once

namespace markshot::cli {

// 输出运行环境自检 JSON（平台/会话/显示器/无头配置/窗口检测链路）并退出。
// 供智能体与脚本一条命令完成排障，代替猜测性的环境探测。
int runDoctor();

} // namespace markshot::cli
