
# Dice-of-COC Version 1.5.1

QQ跑团用骰子，开源，使用VC++2017编写

下面列出了本机器人的全部命令(使用".help 命令"获取该命令的详细信息):

	.o/r/rd 骰子类型/命令 [原因]   普通掷骰
	.h/rh 骰子类型/命令 [原因]     暗骰
	.sn [名称]                    设置/删除名称
	.dn		              删除名称
	.on [QQ号]                    开启骰子
	.off [QQ号]                   关闭骰子
	.st [QQ号]                    查看骰子开启状态
	.me 群号(仅私聊) 动作	      以第三方视角做出动作
	.help [命令]	              获取帮助
	
掷骰中的命令:

	.o/r/rd/h/rh COC6       COC6版人物作成
	.o/r/rd/h/rh COC7       COC7版人物作成
	.o/r/rd/h/rh DND        DND英雄做成
	.o/r/rd/h/rh Bx         D100奖励骰(x为奖励骰个数,如.o B2,B与x之间无空格)
	.o/r/rd/h/rh Px         D100惩罚骰(x为惩罚骰个数,如.o B2,P与x之间无空格)
	
注:强烈建议输入命令时不要省略空格,虽然本插件可以在某种程度上识别不加空格的命令,但也有一定几率会发生错误

问题反馈/交流群624807593

源代码可以修改编译后自用，但不能发布或者商业使用

有问题请联系QQ1840686745(或者直接提issue) 希望大家能够支持，谢谢

PS:未安装VC++2017运行库程序将报错(-103),微软官方下载地址：64位:https://aka.ms/vs/15/release/VC_redist.x64.exe 
32位: https://aka.ms/vs/15/release/VC_redist.x86.exe

PPS:最近有人反馈在群中收不到骰子的问题，经检查应该是由于小号是新建的，并且短时间发送了大量信息，所以被腾讯误认为是广告之类的给拦截了，此问题暂时无解，可以换老一点了小号或者是将你的机器人小号闲置一段时间便可以恢复正常
