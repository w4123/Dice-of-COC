#include <windows.h>
#include <string>
#include <map>
#include <set>
#include <fstream>
#include <sstream>
#include "CQSDK/CQAPI_EX.h"
#include "CQSDK/CQEVE_ALL.h"
#include "APPINFO.h"
#include "RollDice.h"

using namespace std;
using namespace CQ;

map<pair<long long, long long>, string> GroupName;
map<pair<long long, long long>, string> DiscussName;
set<long long> DisabledGroup;
set<long long> DisabledDiscuss;

static string strFileLoc;
static string strTooManyTimesNotice = "掷骰失败!掷骰次数超过了最大限制10次!";
//Help Message
static string strHlpMsg = 
R"(下面列出了本机器人的全部命令(使用".help 命令"获取该命令的详细信息):
	.o/r/d/rd 骰子类型/命令 [原因]   普通掷骰
	.h/rh 骰子类型/命令 [原因]     暗骰
	.sn [名称]                         设置/删除名称
	.dn		                        删除名称
	.on [QQ号]                       开启骰子
	.off [QQ号]                       关闭骰子
	.st [QQ号]                         查看骰子开启状态
	.me 群号(仅私聊) 动作	   以第三方视角做出动作
	.sc 所扣San值 当前San值     自动Sancheck,所扣San值的格式是"Sancheck成功扣的点数/Sancheck失败扣的点数"
	.help [命令]	                  获取帮助
掷骰中的命令:
	.o/r/d/rd/h/rh COC6       COC6版人物作成
	.o/r/d/rd/h/rh COC7       COC7版人物作成
	.o/r/d/rd/h/rh DND        DND英雄做成
	.o/r/d/rd/h/rh Bx         D100奖励骰(x为奖励骰个数,如.o B2,B与x之间无空格)
	.o/r/d/rd/h/rh Px         D100惩罚骰(x为惩罚骰个数,如.o B2,P与x之间无空格)
注:强烈建议输入命令时不要省略空格,虽然本插件可以在某种程度上识别不加空格的命令,但也有一定几率会发生错误
版本:1.5.2
问题反馈/交流群624807593)";

static string strNormalDiceMsg = R"(普通掷骰:
	.o/r/rd 骰子类型 [原因]
这是本机器人最基本的功能,在骰子类型中可以识别各类加减符号,并且支持使用#进行多轮掷骰,使用方法为个数#骰子类型
如:.o 1d3#1d6-2d2+3就是骰了1d3次的1d6-2d2+3)";

static string strHiddenDiceMsg = R"(暗骰:
	.h/rh 骰子类型 [原因]
暗骰只在群/多人聊天中有效,结果会私聊给指掷骰人,并在群/多人聊天中发出暗骰通知,其中支持的骰子类型与普通骰子相同)";

static string strSNMsg = R"(设置/删除名称:
	.sn [名称]
此命令只在群/多人聊天中有效,通过此条命令可以改变机器人对你的称呼,已使用此命令设置过名称则会将其覆盖,如果名称为空则与dn命令相同为删除名称,称呼优先级为sn设置的称呼>群名片>昵称)";

static string strDNMsg = R"(删除名称:
	.dn
此命令只在群/多人聊天中有效,通过此条命令可以删除sn命令设置的称呼,恢复到其他可用的称呼,称呼优先级为sn设置的称呼>群名片>昵称)";

static string strONMsg = R"(开启骰子:
	.on [QQ号]
此命令是为了在群中有多个机器人的情况而准备的,所以此命令只在群/多人聊天中有效,此外,在群中本命令需要管理员/群主权限。输入QQ号则只开启对应QQ号的机器人,不输入则对所有机器人有效)";

static string strOFFMsg = R"(关闭骰子:
	.on [QQ号]
此命令是为了在群中有多个机器人的情况而准备的,所以此命令只在群/多人聊天中有效,此外,在群中本命令需要管理员/群主权限。输入QQ号则只关闭对应QQ号的机器人,不输入则对所有机器人有效)";

static string strSTMsg = R"(查看骰子开启状态:
	.st [QQ号]
此命令是为了在群中有多个机器人的情况而准备的,所以此命令只在群/多人聊天中有效。输入QQ号则只查看对应QQ号的机器人状态,不输入则对所有机器人有效)";

static string strMEMsg = R"(以第三方视角做出动作:
	群号(仅私聊) 动作
在群/多人聊天中使用的时候命令为.me 动作,机器人会以第三方的视角重复描述这一动作以增加游戏代入感,为防止输入命令破环游戏感,也可以使用私聊命令.me 群号 动作,机器人会直接将此动作发送到指定的群中(私聊me命令不支持多人聊天))";

static string strSCMsg = R"(Sancheck:
	.sc 所扣San值 当前San值
此命令为自动Sancheck命令,其中所扣San值的格式是"Sancheck成功扣的点数/Sancheck失败扣的点数",如:.sc 1/1d6 60
此命令会先投掷1d100与San比较,如果成功San值便扣除"/"前面的点数,否则扣除"/"后面的点数,最后将整个流程以及当前San值输出")";

static string strHMsg = R"(什么?一个帮助你还要来问?)";

//Error Message
static string strErrMsg = "输入错误!输入.help或.帮助来获得帮助!";

long long Str2Int(const string a) {
	stringstream ConvertStream;
	long long b;
	ConvertStream << a;
	ConvertStream >> b;
	return b;
}

EVE_Startup(startUp) {
	strFileLoc = getAppDirectory();
	ifstream ifstreamGroupName(strFileLoc + "GroupName.config");
	if (ifstreamGroupName) {
		long long QQ;
		long long Group;
		string name;
		pair<long long, long long> pairQQGroup;
		while (ifstreamGroupName >> QQ) {
			ifstreamGroupName >> Group >> name;
			pairQQGroup.first = QQ;
			pairQQGroup.second = Group;
			GroupName[pairQQGroup] = name;
		}
	}
	ifstreamGroupName.close();
	ifstream ifstreamDiscussName(strFileLoc + "DiscussName.config");
	if (ifstreamDiscussName) {
		long long QQ;
		long long Discuss;
		string name;
		pair<long long, long long> pairQQDiscuss;
		while (ifstreamDiscussName >> QQ) {
			ifstreamDiscussName >> Discuss >> name;
			pairQQDiscuss.first = QQ;
			pairQQDiscuss.second = Discuss;
			DiscussName[pairQQDiscuss] = name;
		}
	}
	ifstreamDiscussName.close();
	ifstream ifstreamDisabledGroup(strFileLoc + "DisabledGroup.config");
	if (ifstreamDisabledGroup) {
		long long Group;
		while (ifstreamDisabledGroup >> Group) {
			DisabledGroup.insert(Group);
		}
	}
	ifstreamDisabledGroup.close();
	ifstream ifstreamDisabledDiscuss(strFileLoc + "DisabledDiscuss.config");
	if (ifstreamDisabledDiscuss) {
		long long Discuss;
		while (ifstreamDisabledDiscuss >> Discuss) {
			DisabledDiscuss.insert(Discuss);
		}
	}
	ifstreamDisabledDiscuss.close();
	return 0;
}

///<summary>
///The function is used for receiving and dealing with group message
///Notice:groupMsg is the NAME of the Function
///</summary>
///<param name="eve">Main class of the message</param>
EVE_GroupMsg_EX(groupMsg) {

	pair<long long, long long> pairQQGroup;
	pairQQGroup.first = eve.fromQQ;
	pairQQGroup.second = eve.fromGroup;
	
	//Skip the space
	while (eve.message[0] == ' ')eve.message.erase(eve.message.begin());
	while (eve.message[eve.message.length()-1] == ' ')eve.message.erase(eve.message.end()-1);
	if (eve.message.find("。") == 0) {
		eve.message.erase(eve.message.begin());
		eve.message[0] = '.';
	}
	
	//Save the first input
	string strFirstInput = eve.message.substr(0, eve.message.find(" "));

	//Use the string to save the second part of the Input
	string strSecondInput;

	//Replace the "。" in the front to "."
	if (strFirstInput[0] != '.')return;

	if (eve.message == "." || strFirstInput[1] == '.' || strFirstInput.find("。") == 1)return;
	for (int k = 0; k != strFirstInput.length(); k++) {
		if (isdigit(strFirstInput[k])) {
			strSecondInput += strFirstInput.substr(k);
			strFirstInput = strFirstInput.substr(0, k);
			break;
		}
		if ((strFirstInput[k] == 'd'||strFirstInput[k] == 'D')&&(isdigit(strFirstInput[k+1]))) {
			strSecondInput += strFirstInput.substr(k);
			strFirstInput = strFirstInput.substr(0, k);
			break;
		}
		if (!isprint(strFirstInput[k])) {
			strSecondInput += strFirstInput.substr(k);
			strFirstInput = strFirstInput.substr(0, k);
			break;
		}
	}
	if (strFirstInput == ".")strFirstInput = ".rd";
	//Save the GroupNick. if the GroupNick is empty, save the Nickname
	string strNickName = GroupName.count(pairQQGroup) ? GroupName[pairQQGroup] : getGroupMemberInfo(eve.fromGroup, eve.fromQQ).GroupNickname.empty() ? getStrangerInfo(eve.fromQQ).nick : getGroupMemberInfo(eve.fromGroup, eve.fromQQ).GroupNickname;
	
	//Tolower the first input
	for (auto &i : strFirstInput)if (isupper(i))i = tolower(i);
	
	//Judge if it is a command
	if ((strFirstInput==".o" || strFirstInput==".h" || strFirstInput == ".rd"|| strFirstInput==".r"||strFirstInput==".rh" || strFirstInput == ".d")&&!DisabledGroup.count(eve.fromGroup)) {
		
		//Save the group name
		string strGroupName = getGroupList()[eve.fromGroup];

		//Use it to save the HiddenDice Notice
		string strHiddenNotice = strNickName + "进行了一次暗骰";

		//Use it to save the count of the Msg have read
		int intMsgCnt = eve.message.find(" ") == string::npos ? eve.message.length() : eve.message.find(" ");

		//Skip the space
		while (eve.message[intMsgCnt] == ' ')intMsgCnt++;
		
		//Save the Reason
		string strReason;
		
		//Get the Second Input
		strSecondInput += eve.message.substr(intMsgCnt, eve.message.find(" ", intMsgCnt) == string::npos ? eve.message.length() : eve.message.find(" ", intMsgCnt) - intMsgCnt);
		string strSecondInputCopy = strSecondInput;
		//Toupper the second input
		for (auto &i : strSecondInput)if (islower(i))i = toupper(i);

		//Make the intMsgCnt jump to the space after the second input
		intMsgCnt = eve.message.find(" ", intMsgCnt) == string::npos ? eve.message.length() : eve.message.find(" ", intMsgCnt);

		//Check if it is a "hidden dice"
		bool isHidden = false;
		if (strFirstInput==".h"||strFirstInput == ".rh")isHidden = true;
		
		//Save the number or dice before # to get the time of rolling dice
		int intRollDiceTime = 0;
		if (strSecondInput.find("#") != string::npos) {
			if (MainRoll(intRollDiceTime, strSecondInput.substr(0, strSecondInput.find("#"))) == -1 || intRollDiceTime <= 0) {
				sendGroupMsg(eve.fromGroup, strErrMsg);
				eve.message_block();
				return;
			}
		}
		else {

			//"#" not found, so only roll for one time
			intRollDiceTime = 1;
		}
			
		//If the input contains # and the input before # is dice, send a roll-dice-time notice
		if (strSecondInput.find("#") != string::npos && (strSecondInput.substr(0, strSecondInput.find("#")).find("d") != string::npos || strSecondInput.substr(0, strSecondInput.find("#")).find("D") != string::npos)) {
			string strRollDiceTurnNotice;
			strRollDiceTurnNotice += "掷骰次数: " + strSecondInput.substr(0, strSecondInput.find("#")) + "=" + to_string(intRollDiceTime) + "轮";
			if (isHidden)sendPrivateMsg(eve.fromQQ, strRollDiceTurnNotice);else
			sendGroupMsg(eve.fromGroup, strRollDiceTurnNotice);
		}
		if (intRollDiceTime > 10) {
			sendGroupMsg(eve.fromGroup, strTooManyTimesNotice);
			eve.message_block();
			return;
		}

		//Save the count of erasing characters
		int erase_cnt = strSecondInput.find("#") == string::npos ? -1 : strSecondInput.find("#");

		//Erase the input # and the char before it
		for (int i = 0; i <= erase_cnt; ++i) strSecondInput.erase(strSecondInput.begin());
		for (int i = 0; i <= erase_cnt; ++i) strSecondInputCopy.erase(strSecondInputCopy.begin());
		//Roll the dice
		for (int w = 0; w != intRollDiceTime; ++w) {
			if (strSecondInput == "人物作成"|| strSecondInput == "人物做成"|| strSecondInput == "人物制作"|| strSecondInput == "七版人物作成" || strSecondInput == "七版人物做成" || strSecondInput == "七版人物制作" || strSecondInput == "COC" || strSecondInput == "COC7") {
				//COC7

				//Save the result of COC7
				string strMAns = strNickName;

				//Call the COC7 roll dice function
				COC7(strMAns);

				//If is hidden, send a private msg instead of a group one
				if (isHidden) {
					strMAns = "在群" + strGroupName + "中" + strMAns;
					sendPrivateMsg(eve.fromQQ, strMAns);
				}
				else
				sendGroupMsg(eve.fromGroup, strMAns);
			}
			else if (strSecondInput == "六版人物作成"|| strSecondInput == "六版人物做成" || strSecondInput == "六版人物制作" || strSecondInput == "COC6") {
				//COC6

				//Save the result of COC6
				string strMAns = strNickName;

				//Call the COC6 roll dice function
				COC6(strMAns);

				//If is hidden, send a private msg instead of a group one
				if (isHidden) {
					strMAns = "在群" + strGroupName + "中" + strMAns;
					sendPrivateMsg(eve.fromQQ, strMAns);
				}
				else
				sendGroupMsg(eve.fromGroup, strMAns);
			}
			else if (strSecondInput=="DND"||strSecondInput=="英雄作成"||strSecondInput=="英雄做成"||strSecondInput=="英雄制作") {
				//DND
				
				//Save the result of DND
				string strMAns = strNickName + "的英雄做成:\n";

				//Call the DND roll dice function
				DND(strMAns);

				//If is hidden, send a private msg instead of a group one
				if (isHidden) {
					strMAns = "在群" + strGroupName + "中" + strMAns;
					sendPrivateMsg(eve.fromQQ, strMAns);
				}
				else
					sendGroupMsg(eve.fromGroup, strMAns);
			}
			else if (strSecondInput[0]=='B') {
				string strBonusNum = strSecondInput.substr(1);
				for (const auto i : strBonusNum) {
					if (!isdigit(i)) {
						sendGroupMsg(eve.fromGroup, strErrMsg);
						eve.message_block();
						return;
					}
				}
				int intBonusNum = Str2Int(strBonusNum);
				if (intBonusNum <= 0) {
					sendGroupMsg(eve.fromGroup, "奖励骰个数错误!");
					eve.message_block();
					return;
				}
				int D100res = 0;
				MainRoll(D100res, "D100");
				int MinVal = 10;
				string strAns = strNickName + "骰出了:\nD100=" + to_string(D100res) + "\n奖励骰:";
				for (int i = 1; i <= intBonusNum; i++) {
					Sleep(1);
					int D10res = -!!(D100res % 10);
					MainRoll(D10res, "D10");
					if (D10res < MinVal)MinVal = D10res;
					strAns += to_string(D10res) + " ";
				}
				strAns[strAns.length() - 1] = '\n';
				strAns += "结果为:" + to_string((D100res / 10 < MinVal ? D100res / 10 : MinVal) * 10 + D100res % 10);
				if (intMsgCnt != eve.message.length())strReason = eve.message.substr(intMsgCnt);
				//If the reason is available, add the reason in the front of the output
				if (!strReason.empty()) {
					strAns = "由于" + strReason + strAns;
				}
				if (isHidden) {
					strAns = "在群" + strGroupName + "中" + strAns;
					sendPrivateMsg(eve.fromQQ, strAns);
				}
				else
					sendGroupMsg(eve.fromGroup, strAns);
				
			}
			else if (strSecondInput[0] == 'P') {
				string strBonusNum = strSecondInput.substr(1);
				for (const auto i : strBonusNum) {
					if (!isdigit(i)) {
						sendGroupMsg(eve.fromGroup, strErrMsg);
						eve.message_block();
						return;
					}
				}
				int intBonusNum = Str2Int(strBonusNum);
				if (intBonusNum <= 0) {
					sendGroupMsg(eve.fromGroup, "惩罚骰个数错误!");
					eve.message_block();
					return;
				}
				int D100res = 0;
				MainRoll(D100res, "D100");
				int MaxVal = 0;
				string strAns = strNickName + "骰出了:\nD100=" + to_string(D100res) + "\n惩罚骰:";
				for (int i = 1; i <= intBonusNum; i++) {
					Sleep(1);
					int D10res = -!!(D100res % 10);
					MainRoll(D10res, "D10");
					if (D10res > MaxVal)MaxVal = D10res;
					strAns += to_string(D10res) + " ";
				}
				strAns[strAns.length() - 1] = '\n';
				strAns += "结果为:" + to_string((D100res / 10 > MaxVal ? D100res / 10 : MaxVal) * 10 + D100res % 10);
				if (intMsgCnt != eve.message.length())strReason = eve.message.substr(intMsgCnt);
				//If the reason is available, add the reason in the front of the output
				if (!strReason.empty()) {
					strAns = "由于" + strReason + strAns;
				}
				if (isHidden) {
					strAns = "在群" + strGroupName + "中" + strAns;
					sendPrivateMsg(eve.fromQQ, strAns);
				}
				else
					sendGroupMsg(eve.fromGroup, strAns);
			}
			else {
				
				for (int k = 0; k != strSecondInput.length(); k++) {
					if (!isprint(strSecondInput[k]) || (!isdigit(strSecondInput[k])&& strSecondInput[k]!='-'&&strSecondInput[k]!='+' && strSecondInput[k] != 'D') || (strSecondInput[k] == 'D' && !isdigit(strSecondInput[k + 1]))) {
						strReason += strSecondInputCopy.substr(k);
						strSecondInput = strSecondInput.substr(0, k);
						break;
					}
				}
				
				//Default Dice
				if (strSecondInput == "")strSecondInput = "D100";

				//Output string, save the main answer
				string strAns = strNickName + "骰出了: " + strSecondInput + "=";

				//Save the final result
				int intMainSum = 0;

				//Save the second result
				string strSecondRes;

				//Call the function of rolling dice
				int intRollRes = MainRoll(intMainSum, strSecondInput, strAns, strSecondRes);

				//If the return value is -1, the input is illegal
				if (intRollRes == -1) {
					sendGroupMsg(eve.fromGroup, strErrMsg);
					eve.message_block();
					return;
				}

				//Skip the space
				while (eve.message[intMsgCnt] == ' '&&intMsgCnt != eve.message.length())intMsgCnt++;



				//Get the reason from the third part of the input
				if (intMsgCnt != eve.message.length())strReason += eve.message.substr(intMsgCnt);
				
				//If the reason is available, add the reason in the front of the output
				if (!strReason.empty()) {
					strAns = "由于" + strReason + strAns;
				}

				//Avoid Repeat Result
				if (strAns.substr(strAns.rfind("=", strAns.length() - 2) + 1, strAns.length() - strAns.rfind("=", strAns.length() - 2) - 2) != strSecondRes) {
					strAns += strSecondRes;
					if (strSecondRes != to_string(intMainSum)) {
						strAns += '=';
						strAns += to_string(intMainSum);
					}
				}
				else {

					//Avoid unnecessary character
					strAns.erase(strAns.end() - 1);
				}

				//If is hidden, send a private msg instead of a group one
				if (isHidden) {
					strAns = "在群" + strGroupName + "中" + strAns;
					sendPrivateMsg(eve.fromQQ, strAns);
				}
				else
				sendGroupMsg(eve.fromGroup, strAns);
			}

			//Avoid being frozen
			Sleep(50);
		}

		//If is hidden, send a hidden dice notice
		if (isHidden) {
			sendGroupMsg(eve.fromGroup, strHiddenNotice);
		}
		eve.message_block();
		return;
	}
	else if ((strFirstInput==".sn"|| strFirstInput == ".set"||strFirstInput == ".setname"|| strFirstInput == ".nickname"|| strFirstInput == ".nn") && !DisabledGroup.count(eve.fromGroup)) {
		int intMsgCnt = strFirstInput.length();
		while (eve.message[intMsgCnt] == ' ')intMsgCnt++;
		string name = eve.message.substr(intMsgCnt);
		if (!name.empty()) {
			GroupName[pairQQGroup] = name;
			sendGroupMsg(eve.fromGroup, "已将" + strNickName + "的名称更改为" + name);
		}
		else {
			if (GroupName.count(pairQQGroup)){
				GroupName.erase(pairQQGroup);
				sendGroupMsg(eve.fromGroup, "已将" + strNickName + "的名称删除");
			}
			else sendGroupMsg(eve.fromGroup, strErrMsg);
		}
		eve.message_block();
		return;
	}
	else if ((strFirstInput == ".deletename" || strFirstInput == ".delete" || strFirstInput == ".del" || strFirstInput == ".dn") && !DisabledGroup.count(eve.fromGroup)) {
		if (GroupName.count(pairQQGroup)) {
			GroupName.erase(pairQQGroup);
			sendGroupMsg(eve.fromGroup, "已将" + strNickName + "的名称删除");
		}
		else sendGroupMsg(eve.fromGroup, strErrMsg);
		eve.message_block();
		return;
	}
	else if (strFirstInput==".on") {
		int intMsgCnt = strFirstInput.length();
		while (eve.message[intMsgCnt] == ' ')intMsgCnt++;
		string Number = eve.message.substr(intMsgCnt, eve.message.find(' ', intMsgCnt) == string::npos ? eve.message.length() : eve.message.find(' ', intMsgCnt) - intMsgCnt);
		if (Number == "" || Number == to_string(getLoginQQ())) {
			if (getGroupMemberInfo(eve.fromGroup, eve.fromQQ).permissions >= 2) {
				if (DisabledGroup.count(eve.fromGroup)) {
					DisabledGroup.erase(eve.fromGroup);
					sendGroupMsg(eve.fromGroup, "成功开启本机器人!");
				}
				else sendGroupMsg(eve.fromGroup, "本机器人已经处于开启状态!");
			}
			else sendGroupMsg(eve.fromGroup, "访问被拒绝,您没有足够的权限!");
		}
		eve.message_block();
		return;
	}
	else if (strFirstInput == ".off") {
		int intMsgCnt = strFirstInput.length();
		while (eve.message[intMsgCnt] == ' ')intMsgCnt++;
		string Number = eve.message.substr(intMsgCnt, eve.message.find(' ', intMsgCnt) == string::npos ? eve.message.length() : eve.message.find(' ', intMsgCnt) - intMsgCnt);
		if (Number == "" || Number == to_string(getLoginQQ())) {
			if (getGroupMemberInfo(eve.fromGroup, eve.fromQQ).permissions >= 2) {
				if (!DisabledGroup.count(eve.fromGroup)) {
					DisabledGroup.insert(eve.fromGroup);
					sendGroupMsg(eve.fromGroup, "成功关闭本机器人!");
				}
				else sendGroupMsg(eve.fromGroup, "本机器人已经处于关闭状态!");
			}
			else sendGroupMsg(eve.fromGroup, "访问被拒绝,您没有足够的权限!");
		}
		eve.message_block();
		return;
	}
	else if (strFirstInput == ".status"||strFirstInput==".st") {
		int intMsgCnt = strFirstInput.length();
		while (eve.message[intMsgCnt] == ' ')intMsgCnt++;
		string Number = eve.message.substr(intMsgCnt, eve.message.find(' ', intMsgCnt) == string::npos ? eve.message.length() : eve.message.find(' ', intMsgCnt) - intMsgCnt);
		if (Number == "" || Number == to_string(getLoginQQ())) {
			if(DisabledGroup.count(eve.fromGroup)) sendGroupMsg(eve.fromGroup, "机器人处于关闭状态!");
			else sendGroupMsg(eve.fromGroup, "机器人处于开启状态!");
		}
		eve.message_block();
		return;
	}
	else if ((strFirstInput == ".帮助" || strFirstInput == ".help") && !DisabledGroup.count(eve.fromGroup)) {
		int intMsgCnt = strFirstInput.length();
		while (eve.message[intMsgCnt] == ' ')intMsgCnt++;
		string Help = eve.message.substr(intMsgCnt, eve.message.find(' ', intMsgCnt) == string::npos ? eve.message.length() : eve.message.find(' ', intMsgCnt) - intMsgCnt);
		for (auto &i : Help)i = tolower(i);	
		if (Help[0] == '.')Help.erase(Help.begin());
		if (Help == "") 
			sendGroupMsg(eve.fromGroup, strHlpMsg);
		else if (Help == "o" || Help == "r"|| Help == "rd") 
			sendGroupMsg(eve.fromGroup, strNormalDiceMsg);
		else if (Help == "h" || Help == "rh") 
			sendGroupMsg(eve.fromGroup, strHiddenDiceMsg);
		else if (Help == "sn" || Help == "set" ||Help=="setname"||Help=="nn"||Help=="nickname") 
			sendGroupMsg(eve.fromGroup, strSNMsg);
		else if (Help == "dn" || Help == "del" || Help == "delete" || Help == "deletename")
			sendGroupMsg(eve.fromGroup, strDNMsg);
		else if (Help == "on")
			sendGroupMsg(eve.fromGroup, strONMsg);
		else if (Help == "off")
			sendGroupMsg(eve.fromGroup, strOFFMsg);
		else if (Help == "st" || Help == "status")
			sendGroupMsg(eve.fromGroup, strSTMsg);
		else if (Help == "me")
			sendGroupMsg(eve.fromGroup, strMEMsg);
		else if (Help == "help"|| Help == "帮助")
			sendGroupMsg(eve.fromGroup, strHMsg);
		else if (Help == "sc")
			sendGroupMsg(eve.fromGroup, strSCMsg);
		else 
			sendGroupMsg(eve.fromGroup, "未找到对应的帮助文本");
		eve.message_block();
		return;
	}
	else if (strFirstInput == ".me" && !DisabledGroup.count(eve.fromGroup)) {
		int intMsgCnt = strFirstInput.length();
		while (eve.message[intMsgCnt] == ' ')intMsgCnt++;
		string Action = eve.message.substr(intMsgCnt);
		if (Action != "")sendGroupMsg(eve.fromGroup, strNickName + Action);
		eve.message_block();
		return;
	}
	else if (strFirstInput == ".sc" && !DisabledGroup.count(eve.fromGroup)) {
		int intMsgCnt = strFirstInput.length();
		while (eve.message[intMsgCnt] == ' ')intMsgCnt++;
		string SanCost = eve.message.substr(intMsgCnt, eve.message.find(' ', intMsgCnt) == string::npos ? eve.message.length() : eve.message.find(' ', intMsgCnt) - intMsgCnt);
		intMsgCnt = eve.message.find(' ', intMsgCnt) == string::npos ? eve.message.length() : eve.message.find(' ', intMsgCnt);
		while (eve.message[intMsgCnt] == ' ')intMsgCnt++;
		string San= eve.message.substr(intMsgCnt, eve.message.find(' ', intMsgCnt) == string::npos ? eve.message.length() : eve.message.find(' ', intMsgCnt) - intMsgCnt);
		if (San == "" || SanCost == "" || SanCost.find("/")==string::npos) {
			sendGroupMsg(eve.fromGroup, strErrMsg);
			eve.message_block();
			return;
		}
		for (auto i : San)
			if (!isdigit(i)) {
				sendGroupMsg(eve.fromGroup, strErrMsg);
				eve.message_block();
				return;
			}
		int intSan = 0;
		stringstream toint;
		toint << San;
		toint >> intSan;
		if (San.length() >= 3 || intSan == 0) {
			sendGroupMsg(eve.fromGroup, "San值输入不正确!");
			eve.message_block();
			return;
		}
		string strAns = strNickName + "的Sancheck:\n1D100=";
		int D100res = 0;
		MainRoll(D100res, "D100");
		strAns += to_string(D100res);
		if (D100res <= intSan) {
			strAns += " 成功\n你的San值减少" + SanCost.substr(0, SanCost.find("/"));
			int intReduceSan = 0;
			int Rollres=MainRoll(intReduceSan, SanCost.substr(0, SanCost.find("/")));
			if (Rollres == -1) {
				sendGroupMsg(eve.fromGroup, strErrMsg);
				eve.message_block();
				return;
			}
			if(SanCost.substr(0, SanCost.find("/")).find("d") != string::npos|| SanCost.substr(0, SanCost.find("/")).find("D") != string::npos)strAns += "=" + to_string(intReduceSan);
			strAns += +"点,当前剩余" + to_string(intSan - intReduceSan) + "点";
		}
		else {
			strAns += " 失败\n你的San值减少" + SanCost.substr(SanCost.find("/")+1);
			int intReduceSan = 0;
			int Rollres=MainRoll(intReduceSan,SanCost.substr(SanCost.find("/")+1));
			if (Rollres == -1) {
				sendGroupMsg(eve.fromGroup, strErrMsg);
				eve.message_block();
				return;
			}
			if (SanCost.substr(SanCost.find("/") + 1).find("d") != string::npos || SanCost.substr(SanCost.find("/") + 1).find("D") != string::npos)strAns += "=" + to_string(intReduceSan);
			strAns += +"点,当前剩余" + to_string(intSan - intReduceSan) + "点";
		}
		sendGroupMsg(eve.fromGroup, strAns);
		eve.message_block();
		return;
	}
	if (eve.message[0] == '.')sendGroupMsg(eve.fromGroup, "命令输入错误!是否少写了空格?输入.help获取帮助");
	eve.message_ignore();
	return;
}

///<summary>
///The function is used for receiving and dealing with private message
///Notice:privateMsg is the NAME of the Function
///</summary>
///<param name="eve">Main class of the message</param>
EVE_PrivateMsg_EX(privateMsg)
{//Skip the space
	while (eve.message[0] == ' ')eve.message.erase(eve.message.begin());
	while (eve.message[eve.message.length() - 1] == ' ')eve.message.erase(eve.message.end() - 1);
	if (eve.message.find("。") == 0) {
		eve.message.erase(eve.message.begin());
		eve.message[0] = '.';
	}

	//Save the first input
	string strFirstInput = eve.message.substr(0, eve.message.find(" "));

	//Use the string to save the second part of the Input
	string strSecondInput;

	//Replace the "。" in the front to "."
	if (strFirstInput[0] != '.')return;

	if (eve.message == "." || strFirstInput[1] == '.' || strFirstInput.find("。") == 1)return;

	for (int k = 0; k != strFirstInput.length(); k++) {
		if (isdigit(strFirstInput[k])) {
			strSecondInput += strFirstInput.substr(k);
			strFirstInput = strFirstInput.substr(0, k);
			break;
		}
		if ((strFirstInput[k] == 'd' || strFirstInput[k] == 'D') && (isdigit(strFirstInput[k + 1]))) {
			strSecondInput += strFirstInput.substr(k);
			strFirstInput = strFirstInput.substr(0, k);
			break;
		}
		if (!isprint(strFirstInput[k])) {
			strSecondInput += strFirstInput.substr(k);
			strFirstInput = strFirstInput.substr(0, k);
			break;
		}
	}
	if (strFirstInput == ".")strFirstInput = ".rd";

	//Save the Nickname
	string strNickName = getStrangerInfo(eve.fromQQ).nick;

	//Tolower the first input
	for (auto &i : strFirstInput)if (isupper(i))i = tolower(i);

	//Judge if it is a command
	if (strFirstInput == ".o" || strFirstInput == ".rd" || strFirstInput == ".r" || strFirstInput == ".d") {
		//Use it to save the count of the Msg have read
		int intMsgCnt = eve.message.find(" ") == string::npos ? eve.message.length() : eve.message.find(" ");

		//Skip the space
		while (eve.message[intMsgCnt] == ' ')intMsgCnt++;

		//Save the Reason
		string strReason;

		//Get the Second Input
		strSecondInput += eve.message.substr(intMsgCnt, eve.message.find(" ", intMsgCnt) == string::npos ? eve.message.length() : eve.message.find(" ", intMsgCnt) - intMsgCnt);
		string strSecondInputCopy = strSecondInput;
		//Toupper the second input
		for (auto &i : strSecondInput)if (islower(i))i = toupper(i);

		//Make the intMsgCnt jump to the space after the second input
		intMsgCnt = eve.message.find(" ", intMsgCnt) == string::npos ? eve.message.length() : eve.message.find(" ", intMsgCnt);

		//Save the number or dice before # to get the time of rolling dice
		int intRollDiceTime = 0;
		if (strSecondInput.find("#") != string::npos) {
			if (MainRoll(intRollDiceTime, strSecondInput.substr(0, strSecondInput.find("#"))) == -1 || intRollDiceTime <= 0) {
				sendPrivateMsg(eve.fromQQ, strErrMsg);
				eve.message_block();
				return;
			}
		}
		else {

			//"#" not found, so only roll for one time
			intRollDiceTime = 1;
		}

		//If the input contains # and the input before # is dice, send a roll-dice-time notice
		if (strSecondInput.find("#") != string::npos && (strSecondInput.substr(0, strSecondInput.find("#")).find("d") != string::npos || strSecondInput.substr(0, strSecondInput.find("#")).find("D") != string::npos)) {
			string strRollDiceTurnNotice;
			strRollDiceTurnNotice += "掷骰次数: " + strSecondInput.substr(0, strSecondInput.find("#")) + "=" + to_string(intRollDiceTime) + "轮";
			sendPrivateMsg(eve.fromQQ, strRollDiceTurnNotice);
		}
		if (intRollDiceTime > 10) {
			sendPrivateMsg(eve.fromQQ, strTooManyTimesNotice);
			eve.message_block();
			return;
		}
		//Save the count of erasing characters
		int erase_cnt = strSecondInput.find("#") == string::npos ? -1 : strSecondInput.find("#");

		//Erase the input # and the char before it
		for (int i = 0; i <= erase_cnt; ++i) strSecondInput.erase(strSecondInput.begin());
		for (int i = 0; i <= erase_cnt; ++i) strSecondInputCopy.erase(strSecondInputCopy.begin());
		//Roll the dice
		for (int w = 0; w != intRollDiceTime; ++w) {
			if (strSecondInput == "人物作成" || strSecondInput == "人物做成" || strSecondInput == "人物制作" || strSecondInput == "七版人物作成" || strSecondInput == "七版人物做成" || strSecondInput == "七版人物制作" || strSecondInput == "COC" || strSecondInput == "COC7") {
				//COC7

				//Save the result of COC7
				string strMAns = strNickName;

				//Call the COC7 roll dice function
				COC7(strMAns);

				sendPrivateMsg(eve.fromQQ, strMAns);
			}
			else if (strSecondInput == "六版人物作成" || strSecondInput == "六版人物做成" || strSecondInput == "六版人物制作" || strSecondInput == "COC6") {
				//COC6

				//Save the result of COC6
				string strMAns = strNickName;

				//Call the COC6 roll dice function
				COC6(strMAns);

				sendPrivateMsg(eve.fromQQ, strMAns);

			}
			else if (strSecondInput == "DND" || strSecondInput == "英雄作成" || strSecondInput == "英雄做成" || strSecondInput == "英雄制作") {
				//DND

				//Save the result of DND
				string strMAns = strNickName + "的英雄做成:\n";

				//Call the DND roll dice function
				DND(strMAns);


				sendPrivateMsg(eve.fromQQ, strMAns);

			}
			else if (strSecondInput[0] == 'B') {
				string strBonusNum = strSecondInput.substr(1);
				for (const auto i : strBonusNum) {
					if (!isdigit(i)) {
						sendPrivateMsg(eve.fromQQ, strErrMsg);
						eve.message_block();
						return;
					}
				}
				int intBonusNum = Str2Int(strBonusNum);
				if (intBonusNum <= 0) {
					sendPrivateMsg(eve.fromQQ, "奖励骰个数错误!");
					eve.message_block();
					return;
				}
				int D100res = 0;
				MainRoll(D100res, "D100");
				int MinVal = 10;
				string strAns = strNickName + "骰出了:\nD100=" + to_string(D100res) + "\nD10奖励骰:";
				for (int i = 1; i <= intBonusNum; i++) {
					Sleep(1);
					int D10res = -!!(D100res % 10);
					MainRoll(D10res, "D10");
					if (D10res < MinVal)MinVal = D10res;
					strAns += to_string(D10res) + " ";
				}
				strAns[strAns.length() - 1] = '\n';
				strAns += "结果为:" + to_string((D100res / 10 < MinVal ? D100res / 10 : MinVal) * 10 + D100res % 10);
				if (intMsgCnt != eve.message.length())strReason = eve.message.substr(intMsgCnt);
				//If the reason is available, add the reason in the front of the output
				if (!strReason.empty()) {
					strAns = "由于" + strReason + strAns;
				}
				sendPrivateMsg(eve.fromQQ, strAns);
			}
			else if (strSecondInput[0] == 'P') {
				string strBonusNum = strSecondInput.substr(1);
				for (const auto i : strBonusNum) {
					if (!isdigit(i)) {
						sendPrivateMsg(eve.fromQQ, strErrMsg);
						eve.message_block();
						return;
					}
				}
				int intBonusNum = Str2Int(strBonusNum);
				if (intBonusNum <= 0) {
					sendPrivateMsg(eve.fromQQ, "惩罚骰个数错误!");
					eve.message_block();
					return;
				}
				int D100res = 0;
				MainRoll(D100res, "D100");
				int MaxVal = 0;
				string strAns = strNickName + "骰出了:\nD100=" + to_string(D100res) + "\n惩罚骰:";
				for (int i = 1; i <= intBonusNum; i++) {
					Sleep(1);
					int D10res = -!!(D100res % 10);
					MainRoll(D10res, "D10");
					if (D10res > MaxVal)MaxVal = D10res;
					strAns += to_string(D10res) + " ";
				}
				strAns[strAns.length() - 1] = '\n';
				strAns += "结果为:" + to_string((D100res / 10 > MaxVal ? D100res / 10 : MaxVal) * 10 + D100res % 10);
				if (intMsgCnt != eve.message.length())strReason = eve.message.substr(intMsgCnt);
				//If the reason is available, add the reason in the front of the output
				if (!strReason.empty()) {
					strAns = "由于" + strReason + strAns;
				}
				sendPrivateMsg(eve.fromQQ, strAns);
			}
			else {

				for (int k = 0; k != strSecondInput.length(); k++) {
					if (!isprint(strSecondInput[k]) || (!isdigit(strSecondInput[k]) && strSecondInput[k] != '-'&&strSecondInput[k] != '+' && strSecondInput[k] != 'D') || (strSecondInput[k] == 'D' && !isdigit(strSecondInput[k + 1]))) {
						strReason += strSecondInputCopy.substr(k);
						strSecondInput = strSecondInput.substr(0, k);
						break;
					}
				}

				//Default Dice
				if (strSecondInput == "")strSecondInput = "D100";

				//Output string, save the main answer
				string strAns = strNickName + "骰出了: " + strSecondInput + "=";

				//Save the final result
				int intMainSum = 0;

				//Save the second result
				string strSecondRes;

				//Call the function of rolling dice
				int intRollRes = MainRoll(intMainSum, strSecondInput, strAns, strSecondRes);

				//If the return value is -1, the input is illegal
				if (intRollRes == -1) {
					sendPrivateMsg(eve.fromQQ, strErrMsg);
					eve.message_block();
					return;
				}

				//Skip the space
				while (eve.message[intMsgCnt] == ' '&&intMsgCnt != eve.message.length())intMsgCnt++;



				//Get the reason from the third part of the input
				if (intMsgCnt != eve.message.length())strReason += eve.message.substr(intMsgCnt);

				//If the reason is available, add the reason in the front of the output
				if (!strReason.empty()) {
					strAns = "由于" + strReason + strAns;
				}

				//Avoid Repeat Result
				if (strAns.substr(strAns.rfind("=", strAns.length() - 2) + 1, strAns.length() - strAns.rfind("=", strAns.length() - 2) - 2) != strSecondRes) {
					strAns += strSecondRes;
					if (strSecondRes != to_string(intMainSum)) {
						strAns += '=';
						strAns += to_string(intMainSum);
					}
				}
				else {

					//Avoid unnecessary character
					strAns.erase(strAns.end() - 1);
				}
				sendPrivateMsg(eve.fromQQ, strAns);
			}

			//Avoid being frozen
			Sleep(50);
		}
		eve.message_block();
		return;
	}
	else if (strFirstInput == ".帮助" || strFirstInput == ".help") {
		int intMsgCnt = strFirstInput.length();
		while (eve.message[intMsgCnt] == ' ')intMsgCnt++;
		string Help = eve.message.substr(intMsgCnt, eve.message.find(' ', intMsgCnt) == string::npos ? eve.message.length() : eve.message.find(' ', intMsgCnt) - intMsgCnt);
		for (auto &i : Help)i = tolower(i);
		if (Help[0] == '.')Help.erase(Help.begin());
		if (Help == "")
			sendPrivateMsg(eve.fromQQ, strHlpMsg);
		else if (Help == "o" || Help == "r" || Help == "rd")
			sendPrivateMsg(eve.fromQQ, strNormalDiceMsg);
		else if (Help == "h" || Help == "rh")
			sendPrivateMsg(eve.fromQQ, strHiddenDiceMsg);
		else if (Help == "sn" || Help == "set" || Help == "setname" || Help == "nn" || Help == "nickname")
			sendPrivateMsg(eve.fromQQ, strSNMsg);
		else if (Help == "dn" || Help == "del" || Help == "delete" || Help == "deletename")
			sendPrivateMsg(eve.fromQQ, strDNMsg);
		else if (Help == "on")
			sendPrivateMsg(eve.fromQQ, strONMsg);
		else if (Help == "off")
			sendPrivateMsg(eve.fromQQ, strOFFMsg);
		else if (Help == "st" || Help == "status")
			sendPrivateMsg(eve.fromQQ, strSTMsg);
		else if (Help == "me")
			sendPrivateMsg(eve.fromQQ, strMEMsg);
		else if (Help == "help" || Help == "帮助")
			sendPrivateMsg(eve.fromQQ, strHMsg);
		else if (Help == "sc")
			sendPrivateMsg(eve.fromQQ, strSCMsg);
		else
			sendPrivateMsg(eve.fromQQ, "未找到对应的帮助文本");
		eve.message_block();
		return;
	}
	else if (strFirstInput == ".me") {
		int intMsgCnt = strFirstInput.length();
		while (eve.message[intMsgCnt] == ' ')intMsgCnt++;
		string strGroupnum = eve.message.substr(intMsgCnt, eve.message.find(' ', intMsgCnt) == string::npos ? eve.message.length() : eve.message.find(' ', intMsgCnt) - intMsgCnt);
		string Action;
		for (int q = 0; q != strGroupnum.length(); q++) {
			if (!isdigit(strGroupnum[q])) {
				Action += strGroupnum.substr(q);
				strGroupnum = strGroupnum.substr(0, q);
				break;
			}
		}
		intMsgCnt = eve.message.find(' ', intMsgCnt) == string::npos ? eve.message.length() : eve.message.find(' ', intMsgCnt);
		while (eve.message[intMsgCnt] == ' ')intMsgCnt++;
		Action += eve.message.substr(intMsgCnt);
		long long llGroupnum;
		stringstream toint;
		toint << strGroupnum;
		toint >> llGroupnum;
		if (Action == "") {
			sendPrivateMsg(eve.fromQQ, strErrMsg);
		}
		else if (strGroupnum == "") {
			sendPrivateMsg(eve.fromQQ, "未输入群号!输入.help或.帮助来获得帮助!");
		}
		else {
			pair<long long, long long>pairQQGroup(eve.fromQQ, llGroupnum);
			if (!DisabledGroup.count(llGroupnum)) {
				int Sendres=sendGroupMsg(llGroupnum, (GroupName.count(pairQQGroup) ? GroupName[pairQQGroup] : getGroupMemberInfo(llGroupnum, eve.fromQQ).GroupNickname.empty() ? getStrangerInfo(eve.fromQQ).nick : getGroupMemberInfo(llGroupnum, eve.fromQQ).GroupNickname) + Action);
				if(Sendres>=0)sendPrivateMsg(eve.fromQQ, "命令执行成功!");
				else sendPrivateMsg(eve.fromQQ, "命令执行失败!是否输入了错误的群号?");
			}
			else {
				sendPrivateMsg(eve.fromQQ, "发送失败!在此群中机器人已被关闭!");
			}
		}
		eve.message_block();
		return;
	}
	else if (strFirstInput == ".sc") {
		int intMsgCnt = strFirstInput.length();
		while (eve.message[intMsgCnt] == ' ')intMsgCnt++;
		string SanCost = eve.message.substr(intMsgCnt, eve.message.find(' ', intMsgCnt) == string::npos ? eve.message.length() : eve.message.find(' ', intMsgCnt) - intMsgCnt);
		intMsgCnt = eve.message.find(' ', intMsgCnt) == string::npos ? eve.message.length() : eve.message.find(' ', intMsgCnt);
		while (eve.message[intMsgCnt] == ' ')intMsgCnt++;
		string San = eve.message.substr(intMsgCnt, eve.message.find(' ', intMsgCnt) == string::npos ? eve.message.length() : eve.message.find(' ', intMsgCnt) - intMsgCnt);
		if (San == "" || SanCost == "" || SanCost.find("/") == string::npos) {
			sendPrivateMsg(eve.fromQQ, strErrMsg);
			eve.message_block();
			return;
		}
		for (auto i : San)
			if (!isdigit(i)) {
				sendPrivateMsg(eve.fromQQ, strErrMsg);
				eve.message_block();
				return;
			}
		int intSan = 0;
		stringstream toint;
		toint << San;
		toint >> intSan;
		if (San.length() >= 3 || intSan == 0) {
			sendPrivateMsg(eve.fromQQ, "San值输入不正确!");
			eve.message_block();
			return;
		}
		string strAns = strNickName + "的Sancheck:\n1D100=";
		int D100res = 0;
		MainRoll(D100res, "D100");
		strAns += to_string(D100res);
		if (D100res <= intSan) {
			strAns += " 成功\n你的San值减少" + SanCost.substr(0, SanCost.find("/"));
			int intReduceSan = 0;
			int Rollres = MainRoll(intReduceSan, SanCost.substr(0, SanCost.find("/")));
			if (Rollres == -1) {
				sendPrivateMsg(eve.fromQQ, strErrMsg);
				eve.message_block();
				return;
			}
			if (SanCost.substr(0, SanCost.find("/")).find("d") != string::npos || SanCost.substr(0, SanCost.find("/")).find("D") != string::npos)strAns += "=" + to_string(intReduceSan);
			strAns += +"点,当前剩余" + to_string(intSan - intReduceSan) + "点";
		}
		else {
			strAns += " 失败\n你的San值减少" + SanCost.substr(SanCost.find("/")+1);
			int intReduceSan = 0;
			int Rollres = MainRoll(intReduceSan, SanCost.substr(SanCost.find("/")+1));
			if (Rollres == -1) {
				sendPrivateMsg(eve.fromQQ, strErrMsg);
				eve.message_block();
				return;
			}
			if (SanCost.substr(SanCost.find("/") + 1).find("d") != string::npos || SanCost.substr(SanCost.find("/") + 1).find("D") != string::npos)strAns += "=" + to_string(intReduceSan);
			strAns += +"点,当前剩余" + to_string(intSan - intReduceSan) + "点";
		}
		sendPrivateMsg(eve.fromQQ, strAns);
		eve.message_block();
		return;
	}

	if(eve.message[0]=='.')sendPrivateMsg(eve.fromQQ, "命令输入错误!是否少写了空格?输入.help获取帮助");
	eve.message_ignore();
	return;
}
///<summary>
///The function is used for receiving and dealing with discuss message
///Notice:discussMsg is the NAME of the Function
///</summary>
///<param name="eve">Main class of the message</param>
EVE_DiscussMsg_EX(discussMsg) {
	pair<long long, long long> pairQQDiscuss;
	pairQQDiscuss.first = eve.fromQQ;
	pairQQDiscuss.second = eve.fromDiscuss;

	//Skip the space
	while (eve.message[0] == ' ')eve.message.erase(eve.message.begin());
	while (eve.message[eve.message.length() - 1] == ' ')eve.message.erase(eve.message.end() - 1);
	if (eve.message.find("。") == 0) {
		eve.message.erase(eve.message.begin());
		eve.message[0] = '.';
	}

	//Save the first input
	string strFirstInput = eve.message.substr(0, eve.message.find(" "));

	//Use the string to save the second part of the Input
	string strSecondInput;

	//Replace the "。" in the front to "."
	if (strFirstInput[0] != '.')return;
	if (eve.message == "." || strFirstInput[1] == '.' || strFirstInput.find("。") == 1)return;
	for (int k = 0; k != strFirstInput.length(); k++) {
		if (isdigit(strFirstInput[k])) {
			strSecondInput += strFirstInput.substr(k);
			strFirstInput = strFirstInput.substr(0, k);
			break;
		}
		if ((strFirstInput[k] == 'd' || strFirstInput[k] == 'D') && (isdigit(strFirstInput[k + 1]))) {
			strSecondInput += strFirstInput.substr(k);
			strFirstInput = strFirstInput.substr(0, k);
			break;
		}
		if (!isprint(strFirstInput[k])) {
			strSecondInput += strFirstInput.substr(k);
			strFirstInput = strFirstInput.substr(0, k);
			break;
		}
	}

	if (strFirstInput == ".")strFirstInput = ".rd";

	//Save the DiscussNick. if the DiscussNick is empty, save the Nickname
	string strNickName = DiscussName.count(pairQQDiscuss) ? DiscussName[pairQQDiscuss] : getStrangerInfo(eve.fromQQ).nick;

	//Tolower the first input
	for (auto &i : strFirstInput)if (isupper(i))i = tolower(i);

	//Judge if it is a command
	if ((strFirstInput == ".o" || strFirstInput == ".h" || strFirstInput == ".rd" || strFirstInput == ".r" || strFirstInput == ".rh" || strFirstInput == ".d") && !DisabledDiscuss.count(eve.fromDiscuss)) {

		//Save the Discuss name
		string strDiscussName = to_string(eve.fromDiscuss);

		//Use it to save the HiddenDice Notice
		string strHiddenNotice = strNickName + "进行了一次暗骰";

		//Use it to save the count of the Msg have read
		int intMsgCnt = eve.message.find(" ") == string::npos ? eve.message.length() : eve.message.find(" ");

		//Skip the space
		while (eve.message[intMsgCnt] == ' ')intMsgCnt++;

		//Save the Reason
		string strReason;

		//Get the Second Input
		strSecondInput += eve.message.substr(intMsgCnt, eve.message.find(" ", intMsgCnt) == string::npos ? eve.message.length() : eve.message.find(" ", intMsgCnt) - intMsgCnt);
		string strSecondInputCopy = strSecondInput;
		//Toupper the second input
		for (auto &i : strSecondInput)if (islower(i))i = toupper(i);

		//Make the intMsgCnt jump to the space after the second input
		intMsgCnt = eve.message.find(" ", intMsgCnt) == string::npos ? eve.message.length() : eve.message.find(" ", intMsgCnt);

		//Check if it is a "hidden dice"
		bool isHidden = false;
		if (strFirstInput == ".h" || strFirstInput == ".rh")isHidden = true;

		//Save the number or dice before # to get the time of rolling dice
		int intRollDiceTime = 0;
		if (strSecondInput.find("#") != string::npos) {
			if (MainRoll(intRollDiceTime, strSecondInput.substr(0, strSecondInput.find("#"))) == -1 || intRollDiceTime <= 0) {
				sendDiscussMsg(eve.fromDiscuss, strErrMsg);
				eve.message_block();
				return;
			}
		}
		else {

			//"#" not found, so only roll for one time
			intRollDiceTime = 1;
		}

		//If the input contains # and the input before # is dice, send a roll-dice-time notice
		if (strSecondInput.find("#") != string::npos && (strSecondInput.substr(0, strSecondInput.find("#")).find("d") != string::npos || strSecondInput.substr(0, strSecondInput.find("#")).find("D") != string::npos)) {
			string strRollDiceTurnNotice;
			strRollDiceTurnNotice += "掷骰次数: " + strSecondInput.substr(0, strSecondInput.find("#")) + "=" + to_string(intRollDiceTime) + "轮";
			if (isHidden)sendPrivateMsg(eve.fromQQ, strRollDiceTurnNotice); else
				sendDiscussMsg(eve.fromDiscuss, strRollDiceTurnNotice);
		}
		if (intRollDiceTime > 10) {
			sendDiscussMsg(eve.fromDiscuss, strTooManyTimesNotice);
			eve.message_block();
			return;
		}
		//Save the count of erasing characters
		int erase_cnt = strSecondInput.find("#") == string::npos ? -1 : strSecondInput.find("#");

		//Erase the input # and the char before it
		for (int i = 0; i <= erase_cnt; ++i) strSecondInput.erase(strSecondInput.begin());
		for (int i = 0; i <= erase_cnt; ++i) strSecondInputCopy.erase(strSecondInputCopy.begin());
		//Roll the dice
		for (int w = 0; w != intRollDiceTime; ++w) {
			if (strSecondInput == "人物作成" || strSecondInput == "人物做成" || strSecondInput == "人物制作" || strSecondInput == "七版人物作成" || strSecondInput == "七版人物做成" || strSecondInput == "七版人物制作" || strSecondInput == "COC" || strSecondInput == "COC7") {
				//COC7

				//Save the result of COC7
				string strMAns = strNickName;

				//Call the COC7 roll dice function
				COC7(strMAns);

				//If is hidden, send a private msg instead of a Discuss one
				if (isHidden) {
					strMAns = "在多人聊天" + strDiscussName + "中" + strMAns;
					sendPrivateMsg(eve.fromQQ, strMAns);
				}
				else
					sendDiscussMsg(eve.fromDiscuss, strMAns);
			}
			else if (strSecondInput == "六版人物作成" || strSecondInput == "六版人物做成" || strSecondInput == "六版人物制作" || strSecondInput == "COC6") {
				//COC6

				//Save the result of COC6
				string strMAns = strNickName;

				//Call the COC6 roll dice function
				COC6(strMAns);

				//If is hidden, send a private msg instead of a Discuss one
				if (isHidden) {
					strMAns = "在多人聊天" + strDiscussName + "中" + strMAns;
					sendPrivateMsg(eve.fromQQ, strMAns);
				}
				else
					sendDiscussMsg(eve.fromDiscuss, strMAns);
			}
			else if (strSecondInput == "DND" || strSecondInput == "英雄作成" || strSecondInput == "英雄做成" || strSecondInput == "英雄制作") {
				//DND

				//Save the result of DND
				string strMAns = strNickName + "的英雄做成:\n";

				//Call the DND roll dice function
				DND(strMAns);

				//If is hidden, send a private msg instead of a Discuss one
				if (isHidden) {
					strMAns = "在多人聊天" + strDiscussName + "中" + strMAns;
					sendPrivateMsg(eve.fromQQ, strMAns);
				}
				else
					sendDiscussMsg(eve.fromDiscuss, strMAns);
			}
			else if (strSecondInput[0] == 'B') {
				string strBonusNum = strSecondInput.substr(1);
				for (const auto i : strBonusNum) {
					if (!isdigit(i)) {
						sendDiscussMsg(eve.fromDiscuss, strErrMsg);
						eve.message_block();
						return;
					}
				}
				int intBonusNum = Str2Int(strBonusNum);
				if (intBonusNum <= 0) {
					sendDiscussMsg(eve.fromDiscuss, "奖励骰个数错误!");
					eve.message_block();
					return;
				}
				int D100res = 0;
				MainRoll(D100res, "D100");
				int MinVal = 10;
				string strAns = strNickName + "骰出了:\nD100=" + to_string(D100res) + "\n奖励骰:";
				for (int i = 1; i <= intBonusNum; i++) {
					Sleep(1);
					int D10res = -!!(D100res % 10);
					MainRoll(D10res, "D10");
					if (D10res < MinVal)MinVal = D10res;
					strAns += to_string(D10res) + " ";
				}
				strAns[strAns.length() - 1] = '\n';
				strAns += "结果为:" + to_string((D100res / 10 < MinVal ? D100res / 10 : MinVal) * 10 + D100res % 10);
				if (intMsgCnt != eve.message.length())strReason = eve.message.substr(intMsgCnt);
				//If the reason is available, add the reason in the front of the output
				if (!strReason.empty()) {
					strAns = "由于" + strReason + strAns;
				}
				if (isHidden) {
					strAns = "在多人聊天" + strDiscussName + "中" + strAns;
					sendPrivateMsg(eve.fromQQ, strAns);
				}
				else
					sendDiscussMsg(eve.fromDiscuss, strAns);
			}
			else if (strSecondInput[0] == 'P') {
				string strBonusNum = strSecondInput.substr(1);
				for (const auto i : strBonusNum) {
					if (!isdigit(i)) {
						sendDiscussMsg(eve.fromDiscuss, strErrMsg);
						eve.message_block();
						return;
					}
				}
				int intBonusNum = Str2Int(strBonusNum);
				if (intBonusNum <= 0) {
					sendDiscussMsg(eve.fromDiscuss, "惩罚骰个数错误!");
					eve.message_block();
					return;
				}
				int D100res = 0;
				MainRoll(D100res, "D100");
				int MaxVal = 0;
				string strAns = strNickName + "骰出了:\nD100=" + to_string(D100res) + "\n惩罚骰:";
				for (int i = 1; i <= intBonusNum; i++) {
					Sleep(1);
					int D10res = -!!(D100res % 10);
					MainRoll(D10res, "D10");
					if (D10res > MaxVal)MaxVal = D10res;
					strAns += to_string(D10res) + " ";
				}
				strAns[strAns.length() - 1] = '\n';
				strAns += "结果为:" + to_string((D100res / 10 > MaxVal ? D100res / 10 : MaxVal) * 10 + D100res % 10);
				while (eve.message[intMsgCnt] == ' '&&intMsgCnt != eve.message.length())intMsgCnt++;
				if (intMsgCnt != eve.message.length())strReason = eve.message.substr(intMsgCnt);
				//If the reason is available, add the reason in the front of the output
				if (!strReason.empty()) {
					strAns = "由于" + strReason + strAns;
				}
				if (isHidden) {
					strAns = "在多人聊天" + strDiscussName + "中" + strAns;
					sendPrivateMsg(eve.fromQQ, strAns);
				}
				else
					sendDiscussMsg(eve.fromDiscuss, strAns);
			}
			else {

				for (int k = 0; k != strSecondInput.length(); k++) {
					if (!isprint(strSecondInput[k]) || (!isdigit(strSecondInput[k]) && strSecondInput[k] != '-'&&strSecondInput[k] != '+' && strSecondInput[k] != 'D') || (strSecondInput[k] == 'D' && !isdigit(strSecondInput[k + 1]))) {
						strReason += strSecondInputCopy.substr(k);
						strSecondInput = strSecondInput.substr(0, k);
						break;
					}
				}

				//Default Dice
				if (strSecondInput == "")strSecondInput = "D100";

				//Output string, save the main answer
				string strAns = strNickName + "骰出了: " + strSecondInput + "=";

				//Save the final result
				int intMainSum = 0;

				//Save the second result
				string strSecondRes;

				//Call the function of rolling dice
				int intRollRes = MainRoll(intMainSum, strSecondInput, strAns, strSecondRes);

				//If the return value is -1, the input is illegal
				if (intRollRes == -1) {
					sendDiscussMsg(eve.fromDiscuss, strErrMsg);
					eve.message_block();
					return;
				}

				//Skip the space
				while (eve.message[intMsgCnt] == ' '&&intMsgCnt != eve.message.length())intMsgCnt++;



				//Get the reason from the third part of the input
				if (intMsgCnt != eve.message.length())strReason += eve.message.substr(intMsgCnt);

				//If the reason is available, add the reason in the front of the output
				if (!strReason.empty()) {
					strAns = "由于" + strReason + strAns;
				}

				//Avoid Repeat Result
				if (strAns.substr(strAns.rfind("=", strAns.length() - 2) + 1, strAns.length() - strAns.rfind("=", strAns.length() - 2) - 2) != strSecondRes) {
					strAns += strSecondRes;
					if (strSecondRes != to_string(intMainSum)) {
						strAns += '=';
						strAns += to_string(intMainSum);
					}
				}
				else {

					//Avoid unnecessary character
					strAns.erase(strAns.end() - 1);
				}

				//If is hidden, send a private msg instead of a Discuss one
				if (isHidden) {
					strAns = "在多人聊天" + strDiscussName + "中" + strAns;
					sendPrivateMsg(eve.fromQQ, strAns);
				}
				else
					sendDiscussMsg(eve.fromDiscuss, strAns);
			}

			//Avoid being frozen
			Sleep(50);
		}

		//If is hidden, send a hidden dice notice
		if (isHidden) {
			sendDiscussMsg(eve.fromDiscuss, strHiddenNotice);
		}
		eve.message_block();
		return;
	}
	else if ((strFirstInput == ".sn" || strFirstInput == ".set" || strFirstInput == ".setname" || strFirstInput == ".nickname" || strFirstInput == ".nn") && !DisabledDiscuss.count(eve.fromDiscuss)) {
		int intMsgCnt = strFirstInput.length();
		while (eve.message[intMsgCnt] == ' ')intMsgCnt++;
		string name = eve.message.substr(intMsgCnt);
		if (!name.empty()) {
			DiscussName[pairQQDiscuss] = name;
			sendDiscussMsg(eve.fromDiscuss, "已将" + strNickName + "的名称更改为" + name);
		}
		else {
			if (DiscussName.count(pairQQDiscuss)) {
				DiscussName.erase(pairQQDiscuss);
				sendDiscussMsg(eve.fromDiscuss, "已将" + strNickName + "的名称删除");
			}
			else sendDiscussMsg(eve.fromDiscuss, strErrMsg);
		}
		eve.message_block();
		return;
	}
	else if ((strFirstInput == ".deletename" || strFirstInput == ".delete" || strFirstInput == ".del" || strFirstInput == ".dn") && !DisabledDiscuss.count(eve.fromDiscuss)) {
		if (DiscussName.count(pairQQDiscuss)) {
			DiscussName.erase(pairQQDiscuss);
			sendDiscussMsg(eve.fromDiscuss, "已将" + strNickName + "的名称删除");
		}
		else sendDiscussMsg(eve.fromDiscuss, strErrMsg);
		eve.message_block();
		return;
	}
	else if (strFirstInput == ".on") {
		int intMsgCnt = strFirstInput.length();
		while (eve.message[intMsgCnt] == ' ')intMsgCnt++;
		string Number = eve.message.substr(intMsgCnt, eve.message.find(' ', intMsgCnt) == string::npos ? eve.message.length() : eve.message.find(' ', intMsgCnt) - intMsgCnt);
		if (Number == "" || Number == to_string(getLoginQQ())) {
			if (DisabledDiscuss.count(eve.fromDiscuss)) {
				DisabledDiscuss.erase(eve.fromDiscuss);
				sendDiscussMsg(eve.fromDiscuss, "成功开启本机器人!");
			}
			else sendDiscussMsg(eve.fromDiscuss, "本机器人已经处于开启状态!");
		}
		eve.message_block();
		return;
	}
	else if (strFirstInput == ".off") {
		int intMsgCnt = strFirstInput.length();
		while (eve.message[intMsgCnt] == ' ')intMsgCnt++;
		string Number = eve.message.substr(intMsgCnt, eve.message.find(' ', intMsgCnt) == string::npos ? eve.message.length() : eve.message.find(' ', intMsgCnt) - intMsgCnt);
		if (Number == "" || Number == to_string(getLoginQQ())) {
			if (!DisabledDiscuss.count(eve.fromDiscuss)) {
				DisabledDiscuss.insert(eve.fromDiscuss);
				sendDiscussMsg(eve.fromDiscuss, "成功关闭本机器人!");
			}
			else sendDiscussMsg(eve.fromDiscuss, "本机器人已经处于关闭状态!");
		}
		eve.message_block();
		return;
	}
	else if (strFirstInput == ".status" || strFirstInput == ".st") {
		int intMsgCnt = strFirstInput.length();
		while (eve.message[intMsgCnt] == ' ')intMsgCnt++;
		string Number = eve.message.substr(intMsgCnt, eve.message.find(' ', intMsgCnt) == string::npos ? eve.message.length() : eve.message.find(' ', intMsgCnt) - intMsgCnt);
		if (Number == "" || Number == to_string(getLoginQQ())) {
			if (DisabledDiscuss.count(eve.fromDiscuss)) sendDiscussMsg(eve.fromDiscuss, "机器人处于关闭状态!");
			else sendDiscussMsg(eve.fromDiscuss, "机器人处于开启状态!");
		}
		eve.message_block();
		return;
	}
	else if ((strFirstInput == ".帮助" || strFirstInput == ".help") && !DisabledDiscuss.count(eve.fromDiscuss)) {
		int intMsgCnt = strFirstInput.length();
		while (eve.message[intMsgCnt] == ' ')intMsgCnt++;
		string Help = eve.message.substr(intMsgCnt, eve.message.find(' ', intMsgCnt) == string::npos ? eve.message.length() : eve.message.find(' ', intMsgCnt) - intMsgCnt);
		for (auto &i : Help)i = tolower(i);
		if (Help[0] == '.')Help.erase(Help.begin());
		if (Help == "")
			sendDiscussMsg(eve.fromDiscuss, strHlpMsg);
		else if (Help == "o" || Help == "r" || Help == "rd")
			sendDiscussMsg(eve.fromDiscuss, strNormalDiceMsg);
		else if (Help == "h" || Help == "rh")
			sendDiscussMsg(eve.fromDiscuss, strHiddenDiceMsg);
		else if (Help == "sn" || Help == "set" || Help == "setname" || Help == "nn" || Help == "nickname")
			sendDiscussMsg(eve.fromDiscuss, strSNMsg);
		else if (Help == "dn" || Help == "del" || Help == "delete" || Help == "deletename")
			sendDiscussMsg(eve.fromDiscuss, strDNMsg);
		else if (Help == "on")
			sendDiscussMsg(eve.fromDiscuss, strONMsg);
		else if (Help == "off")
			sendDiscussMsg(eve.fromDiscuss, strOFFMsg);
		else if (Help == "st" || Help == "status")
			sendDiscussMsg(eve.fromDiscuss, strSTMsg);
		else if (Help == "me")
			sendDiscussMsg(eve.fromDiscuss, strMEMsg);
		else if (Help == "sc")
			sendDiscussMsg(eve.fromDiscuss, strSCMsg);
		else if (Help == "help" || Help == "帮助")
			sendDiscussMsg(eve.fromDiscuss, strHMsg);
		else
			sendDiscussMsg(eve.fromDiscuss, "未找到对应的帮助文本");
		eve.message_block();
		return;
	}
	else if (strFirstInput == ".me" && !DisabledDiscuss.count(eve.fromDiscuss)) {
		int intMsgCnt = strFirstInput.length();
		while (eve.message[intMsgCnt] == ' ')intMsgCnt++;
		string Action = eve.message.substr(intMsgCnt);
		if (Action != "")sendDiscussMsg(eve.fromDiscuss, strNickName + Action);
		eve.message_block();
		return;
	}
	else if (strFirstInput == ".sc" && !DisabledDiscuss.count(eve.fromDiscuss)) {
		int intMsgCnt = strFirstInput.length();
		while (eve.message[intMsgCnt] == ' ')intMsgCnt++;
		string SanCost = eve.message.substr(intMsgCnt, eve.message.find(' ', intMsgCnt) == string::npos ? eve.message.length() : eve.message.find(' ', intMsgCnt) - intMsgCnt);
		intMsgCnt = eve.message.find(' ', intMsgCnt) == string::npos ? eve.message.length() : eve.message.find(' ', intMsgCnt);
		while (eve.message[intMsgCnt] == ' ')intMsgCnt++;
		string San = eve.message.substr(intMsgCnt, eve.message.find(' ', intMsgCnt) == string::npos ? eve.message.length() : eve.message.find(' ', intMsgCnt) - intMsgCnt);
		if (San == "" || SanCost == ""||SanCost.find("/")==string::npos) {
			sendDiscussMsg(eve.fromDiscuss, strErrMsg);
			eve.message_block();
			return;
		}
		for (auto i : San)
			if (!isdigit(i)) {
				sendDiscussMsg(eve.fromDiscuss, strErrMsg);
				eve.message_block();
				return;
			}
		int intSan = 0;
		stringstream toint;
		toint << San;
		toint >> intSan;
		if (San.length() >= 3 || intSan == 0) {
			sendDiscussMsg(eve.fromDiscuss, "San值输入不正确!");
			eve.message_block();
			return;
		}
		string strAns = strNickName + "的Sancheck:\n1D100=";
		int D100res = 0;
		MainRoll(D100res, "D100");
		strAns += to_string(D100res);
		if (D100res <= intSan) {
			strAns += " 成功\n你的San值减少" + SanCost.substr(0, SanCost.find("/"));
			int intReduceSan = 0;
			int Rollres = MainRoll(intReduceSan, SanCost.substr(0, SanCost.find("/")));
			if (Rollres == -1) {
				sendDiscussMsg(eve.fromDiscuss, strErrMsg);
				eve.message_block();
				return;
			}
			if (SanCost.substr(0, SanCost.find("/")).find("d") != string::npos || SanCost.substr(0, SanCost.find("/")).find("D") != string::npos)strAns += "=" + to_string(intReduceSan);
			strAns += +"点,当前剩余" + to_string(intSan - intReduceSan) + "点";
		}
		else {
			strAns += " 失败\n你的San值减少" + SanCost.substr(SanCost.find("/")+1);
			int intReduceSan = 0;
			int Rollres = MainRoll(intReduceSan, SanCost.substr(SanCost.find("/")+1));
			if (Rollres == -1) {
				sendDiscussMsg(eve.fromDiscuss, strErrMsg);
				eve.message_block();
				return;
			}
			if (SanCost.substr(SanCost.find("/") + 1).find("d") != string::npos || SanCost.substr(SanCost.find("/") + 1).find("D") != string::npos)strAns += "=" + to_string(intReduceSan);
			strAns += +"点,当前剩余" + to_string(intSan - intReduceSan) + "点";
		}
		sendDiscussMsg(eve.fromDiscuss, strAns);
		eve.message_block();
		return;
	}
	if (eve.message[0] == '.')sendDiscussMsg(eve.fromDiscuss, "命令输入错误!是否少写了空格?输入.help获取帮助");
	eve.message_ignore();
	return;
}

EVE_Menu(__menuA)
{
	MessageBoxA(nullptr, strHlpMsg.c_str(), "Help", MB_OK | MB_ICONINFORMATION);
	return 0;
}

EVE_Exit(eventexit) {
	ofstream ofstreamGroupName(strFileLoc + "GroupName.config", ios::in|ios::trunc);
	for (auto it = GroupName.begin(); it != GroupName.end(); it++) {
		if (!(it->second).empty())
			ofstreamGroupName << it->first.first << " " << it->first.second << " " << it->second << endl;
	}
	ofstreamGroupName.close();
	ofstream ofstreamDiscussName(strFileLoc + "DiscussName.config", ios::in | ios::trunc);
	for (auto it = DiscussName.begin(); it != DiscussName.end(); it++) {
		if (!(it->second).empty())
			ofstreamDiscussName << it->first.first << " " << it->first.second << " " << it->second << endl;
	}
	ofstreamDiscussName.close();

	ofstream ofstreamDisabledGroup(strFileLoc + "DisabledGroup.config", ios::in | ios::trunc);
	for (auto it = DisabledGroup.begin(); it != DisabledGroup.end(); it++) {
		ofstreamDisabledGroup << *it << endl;
	}
	ofstreamDisabledGroup.close();

	ofstream ofstreamDisabledDiscuss(strFileLoc + "DisabledDiscuss.config", ios::in | ios::trunc);
	for (auto it = DisabledDiscuss.begin(); it != DisabledDiscuss.end(); it++) {
		ofstreamDisabledDiscuss << *it << endl;
	}
	ofstreamDisabledDiscuss.close();
	return 0;
}

MUST_AppInfo_RETURN(CQAPPID)
