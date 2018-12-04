ACDaemon： 守护服务工程

ServiceTest： “主服务”模板工程，通讯已调好（可用于同机或跨机通讯（同一网段）），直接在“ServiceMain”函数写业务代码。

StartACDaemon：启动守护服务工程。

备注： 守护进程和主进程都被封装成服务，直接打开StartACDaemon的exe文件即可实现注册并运行守护服务和主服务！