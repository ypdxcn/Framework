
#ifndef _MSG_DEF_H_
#define _MSG_DEF_H_
/**
 * 消息体字段定义
 * 
 * 编号规则
 *  - 16位整数表示不同消息字段
 *  - 公共字段从0x0001开始编号，宏定义在本头文件中
 *  - 私有字段从0x1000开始编号，每个编号段包含0x0400个编号，宏定义不包含在本头文件中
 *  - 私有字段的编号段分配必须预先在本文件的末尾标注
 */

/*
 * 定长块消息协议使用的公共字段
 */
#define MSG_PKG_LENGTH	(int)0x00000001	// 消息包长度
#define MSG_MAGIC		(int)0x00000002	// 魔术字(2字节)
#define MSG_CMD_TYPE	(int)0x00000003	// 命令类型(2字节,请求/应答/通知...)
#define MSG_CMD_ID		(int)0x00000004	// 消息指令类型，如CommandID
#define MSG_SEQ_ID		(int)0x00000005	// 消息包序列号
#define MSG_BODY_LEN	(int)0x00000006	// 消息体MSG_BODY的长度
#define MSG_NODE_ID 	(int)0x00000007	// 节点ID


#endif // _MSG_DEF_H_
