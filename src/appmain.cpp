#include <windows.h>
#include <string>
#include <map>
#include <set>
#include <fstream>
#include <sstream>
#include <ctime>
#include "CQSDK/CQAPI_EX.h"
#include "CQSDK/CQEVE_ALL.h"
#include "CQSDK/CQTools.h"
#include "CQSDK/CQLogger.h"
#include "APPINFO.h"
#include "RollDice.h"

using namespace std;
using namespace CQ;

struct RP {
	int RPVal;
	string Date;
};
map<long long, RP> JRRP;
map<pair<long long, long long>, string> GroupName;
map<pair<long long, long long>, string> DiscussName;
set<long long> DisabledGroup;
set<long long> DisabledDiscuss;
set<long long> DisabledJRRPGroup;
set<long long> DisabledJRRPDiscuss;
set<long long> DisabledMEGroup;
set<long long> DisabledMEDiscuss;
multimap<long long, long long> ObserveGroup;
multimap<long long, long long> ObserveDiscuss;

static string strFileLoc;
static string strTooManyTimesNotice = "掷骰失败!掷骰次数超过了最大限制10次!";
//Help Message
static string strHlpMsg = 
R"(欢迎使用Dice!机器人,下面列出了本机器人的全部命令:
	注：[ ]中的为可选命令
	.o/r/d/rd [表达式/命令] [原因]   掷骰
	.h/rh [表达式/命令] [原因]   暗骰,结果私聊发送
	.o/r/d/rd/h/rh [数目#表达式/命令] 多轮掷骰,如.o 3#1d3+3
	其中，掷骰中的命令包括(以下命令执行方式为.o/r/d/rd/h/rh 命令):	
		COC6             COC6版人物作成
		COC7             COC7版人物作成
		DND              DND英雄做成
		不定性疯狂/LI     不定性疯狂
		临时性疯狂/TI     临时疯狂
		Bx               D100奖励骰(x为奖励骰个数,如.o B2,B与x之间无空格)
		Px               D100惩罚骰(x为惩罚骰个数,如.o P2,P与x之间无空格)
	.nn 名称/del          设置及删除昵称
	.bot [on/off] [QQ号]       机器人控制命令:直接输入为查看状态
	.ob [exit/list/clr]        只输入.ob为加入旁观模式,该群中的所有暗骰信息会私聊发给你
	.sc 所扣San值 当前San值     自动Sancheck,所扣San值的格式是"成功扣San/失败扣San" 如.sc 1/1d6 55
	.me on/off/动作 (群/多人聊天)或.me 群号 动作 (私聊)  以第三方视角做出动作
	.jrrp [on/off]             今日人品检定
	.rules 关键字               COC7规则查询
	.help     	                获取帮助        
版本:1.6.0
本软件遵循GPLv3开源协议,作者w4123,问题反馈/交流/查看源代码请加QQ群624807593)";


//Error Message
static string strErrMsg = "输入错误!输入.help来获得帮助!";

long long Str2ll(const string a) {
	stringstream ConvertStream;
	long long b;
	ConvertStream << a;
	ConvertStream >> b;
	return b;
}

int Str2Int(const string a) {
	stringstream ConvertStream;
	int b;
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
	ifstream ifstreamDisabledJRRPGroup(strFileLoc + "DisabledJRRPGroup.config");
	if (ifstreamDisabledJRRPGroup) {
		long long Group;
		while (ifstreamDisabledJRRPGroup >> Group) {
			DisabledJRRPGroup.insert(Group);
		}
	}
	ifstreamDisabledJRRPGroup.close();
	ifstream ifstreamDisabledJRRPDiscuss(strFileLoc + "DisabledJRRPDiscuss.config");
	if (ifstreamDisabledJRRPDiscuss) {
		long long Discuss;
		while (ifstreamDisabledJRRPDiscuss >> Discuss) {
			DisabledJRRPDiscuss.insert(Discuss);
		}
	}
	ifstreamDisabledJRRPDiscuss.close();
	ifstream ifstreamDisabledMEGroup(strFileLoc + "DisabledMEGroup.config");
	if (ifstreamDisabledMEGroup) {
		long long Group;
		while (ifstreamDisabledMEGroup >> Group) {
			DisabledMEGroup.insert(Group);
		}
	}
	ifstreamDisabledMEGroup.close();
	ifstream ifstreamDisabledMEDiscuss(strFileLoc + "DisabledMEDiscuss.config");
	if (ifstreamDisabledMEDiscuss) {
		long long Discuss;
		while (ifstreamDisabledMEDiscuss >> Discuss) {
			DisabledMEDiscuss.insert(Discuss);
		}
	}
	ifstreamDisabledMEDiscuss.close();
	ifstream ifstreamObserveGroup(strFileLoc + "ObserveGroup.config");
	if (ifstreamObserveGroup) {
		long long Group,QQ;
		while (ifstreamObserveGroup >> Group>>QQ) {
			ObserveGroup.insert(make_pair(Group,QQ));
		}
	}
	ifstreamObserveGroup.close();

	ifstream ifstreamObserveDiscuss(strFileLoc + "ObserveDiscuss.config");
	if (ifstreamObserveDiscuss) {
		long long Discuss, QQ;
		while (ifstreamObserveDiscuss >> Discuss >> QQ) {
			ObserveDiscuss.insert(make_pair(Discuss, QQ));
		}
	}
	ifstreamObserveDiscuss.close();
	ifstream ifstreamJRRP(strFileLoc + "JRRP.config");
		if (ifstreamJRRP) {
			long long QQ;
			int Val;
			string strDate;
			while (ifstreamJRRP >> QQ >> strDate >> Val) {
				JRRP[QQ].Date = strDate;
				JRRP[QQ].RPVal = Val;
			}
		}
	ifstreamJRRP.close();
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
		if ((strFirstInput[k] == 'd'||strFirstInput[k] == 'D')&&(isdigit(strFirstInput[k+1]))&&((strFirstInput.substr(k+1,(strFirstInput.find("+")<strFirstInput.find("-")? strFirstInput.find("+"): strFirstInput.find("-"))-k-1)).find("d")==string::npos)&& ((strFirstInput.substr(k + 1, (strFirstInput.find("+")<strFirstInput.find("-") ? strFirstInput.find("+") : strFirstInput.find("-")) - k - 1)).find("D") == string::npos)) {
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
			if (isHidden) {
				sendPrivateMsg(eve.fromQQ, strRollDiceTurnNotice);
				auto Range = ObserveGroup.equal_range(eve.fromGroup);
				for (auto it = Range.first; it != Range.second; ++it) {
					sendPrivateMsg(it->second, strRollDiceTurnNotice);
				}
			} else
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
					auto Range = ObserveGroup.equal_range(eve.fromGroup);
					for (auto it = Range.first; it != Range.second; ++it) {
						if(it->second!=eve.fromQQ)sendPrivateMsg(it->second, strMAns);
					}
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
					auto Range = ObserveGroup.equal_range(eve.fromGroup);
					for (auto it = Range.first; it != Range.second; ++it) {
						if (it->second != eve.fromQQ)sendPrivateMsg(it->second, strMAns);
					}
				}
				else
				sendGroupMsg(eve.fromGroup, strMAns);
			}
			else if (strSecondInput == "疯狂" || strSecondInput == "INSANE" || strSecondInput == "INSANITY" || strSecondInput == "临时疯狂"|| strSecondInput == "临时性疯狂" || strSecondInput == "短期疯狂"|| strSecondInput == "TI") {
				string strAns = strNickName + "的临时性疯狂:\n";
				TempInsane(strAns);
				if (isHidden) sendPrivateMsg(eve.fromQQ, strAns);
				else sendGroupMsg(eve.fromGroup, strAns);
			}
			else if (strSecondInput == "长期疯狂" || strSecondInput == "不定期疯狂" || strSecondInput == "不定时疯狂" || strSecondInput == "不定疯狂" || strSecondInput == "LI"||strSecondInput=="不定性疯狂") {
				string strAns = strNickName + "的不定性疯狂:\n";
				LongInsane(strAns);
				if (isHidden) sendPrivateMsg(eve.fromQQ, strAns);
				else sendGroupMsg(eve.fromGroup, strAns);
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
					auto Range = ObserveGroup.equal_range(eve.fromGroup);
					for (auto it = Range.first; it != Range.second; ++it) {
						if (it->second != eve.fromQQ)sendPrivateMsg(it->second, strMAns);
					}
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
				if (strBonusNum == "")intBonusNum = 1;
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
					
					int D10res = -!!(D100res % 10);
					MainRoll(D10res, "D10");
					if (D10res < MinVal)MinVal = D10res;
					strAns += to_string(D10res) + " ";
				}
				strAns[strAns.length() - 1] = '\n';
				strAns += "结果为:" + to_string((D100res / 10 < MinVal ? D100res / 10 : MinVal) * 10 + D100res % 10);
				if (intMsgCnt != eve.message.length() && w == 0)strReason = eve.message.substr(intMsgCnt);
				//If the reason is available, add the reason in the front of the output
				if (!strReason.empty()) {
					strAns = "由于" + strReason + strAns;
				}
				if (isHidden) {
					strAns = "在群" + strGroupName + "中" + strAns;
					sendPrivateMsg(eve.fromQQ, strAns);
					auto Range = ObserveGroup.equal_range(eve.fromGroup);
					for (auto it = Range.first; it != Range.second; ++it) {
						if (it->second != eve.fromQQ)sendPrivateMsg(it->second, strAns);
					}
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
				if (strBonusNum == "")intBonusNum = 1;
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
					
					int D10res = -!!(D100res % 10);
					MainRoll(D10res, "D10");
					if (D10res > MaxVal)MaxVal = D10res;
					strAns += to_string(D10res) + " ";
				}
				strAns[strAns.length() - 1] = '\n';
				strAns += "结果为:" + to_string((D100res / 10 > MaxVal ? D100res / 10 : MaxVal) * 10 + D100res % 10);
				if (intMsgCnt != eve.message.length() && w == 0)strReason = eve.message.substr(intMsgCnt);
				//If the reason is available, add the reason in the front of the output
				if (!strReason.empty()) {
					strAns = "由于" + strReason + strAns;
				}
				if (isHidden) {
					strAns = "在群" + strGroupName + "中" + strAns;
					sendPrivateMsg(eve.fromQQ, strAns);
					auto Range = ObserveGroup.equal_range(eve.fromGroup);
					for (auto it = Range.first; it != Range.second; ++it) {
						if (it->second != eve.fromQQ)sendPrivateMsg(it->second, strAns);
					}
				}
				else
					sendGroupMsg(eve.fromGroup, strAns);
			}
			else {

				for (int k = 0; k != strSecondInput.length(); k++) {
					if (!isprint(strSecondInput[k]) || (!isdigit(strSecondInput[k])&& strSecondInput[k]!='-'&&strSecondInput[k]!='+' && strSecondInput[k] != 'D') || (strSecondInput[k] == 'D' && !isdigit(strSecondInput[k + 1]))) {
						strReason += strSecondInputCopy.substr(k);
						strSecondInput = strSecondInput.substr(0, k);
						strSecondInputCopy = strSecondInput.substr(0, k);
						break;
					}
				}
				if (strSecondInput.find("d") == string::npos&&strSecondInput.find("D") == string::npos) {
					strReason = strSecondInputCopy + strReason;
					strSecondInput = "";
					strSecondInputCopy = "";
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
				if (intMsgCnt != eve.message.length() && w == 0)strReason += eve.message.substr(intMsgCnt);
				
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
					auto Range = ObserveGroup.equal_range(eve.fromGroup);
					for (auto it = Range.first; it != Range.second; ++it) {
						if (it->second != eve.fromQQ)sendPrivateMsg(it->second, strAns);
					}
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
	else if (strFirstInput == ".nn" && !DisabledGroup.count(eve.fromGroup)) {
		int intMsgCnt = strFirstInput.length();
		while (eve.message[intMsgCnt] == ' ')intMsgCnt++;
		string name = eve.message.substr(intMsgCnt);
		if (name == "del") {
			if (GroupName.count(pairQQGroup)) {
				GroupName.erase(pairQQGroup);
				sendGroupMsg(eve.fromGroup, "已将" + strNickName + "的名称删除");
			}
			else sendGroupMsg(eve.fromGroup, strErrMsg);
		}
		else {
			if (!name.empty()) {
				GroupName[pairQQGroup] = name;
				sendGroupMsg(eve.fromGroup, "已将" + strNickName + "的名称更改为" + name);
			}
			else {
				sendGroupMsg(eve.fromGroup, "名称不能为空!");
			}
		}
		eve.message_block();
		return;
	}
	else if (strFirstInput == ".bot") {
		int intMsgCnt = strFirstInput.length();
		while (eve.message[intMsgCnt] == ' ')intMsgCnt++;
		string Command = eve.message.substr(intMsgCnt, eve.message.find(' ', intMsgCnt) == string::npos ? eve.message.length() : eve.message.find(' ', intMsgCnt) - intMsgCnt);
		intMsgCnt = eve.message.find(' ', intMsgCnt) == string::npos ? eve.message.length() : eve.message.find(' ', intMsgCnt);
		while (eve.message[intMsgCnt] == ' ')intMsgCnt++;
		string Number = eve.message.substr(intMsgCnt, eve.message.find(' ', intMsgCnt) == string::npos ? eve.message.length() : eve.message.find(' ', intMsgCnt) - intMsgCnt);
		for (auto &kk : Command)kk = tolower(kk);
		if (isdigit(Command[0])) {
			Command = "";
			Number = Command;
		}
		for (int index = 0; index != Command.length(); index++) {
			if (isdigit(Command[index])) {
				Number = Command.substr(index);
				Command = Command.substr(0, index);
				break;
			}
		}
		if (Command == "") {
			if (Number == "" || Number == to_string(getLoginQQ())) {
				if (DisabledGroup.count(eve.fromGroup)) sendGroupMsg(eve.fromGroup, "机器人处于关闭状态!");
				else sendGroupMsg(eve.fromGroup, "机器人处于开启状态!");
			}
		}
		else if (Command == "on") {
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
		}
		else if (Command == "off") {
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
		}
		else sendGroupMsg(eve.fromGroup, "无法识别的命令!");
		eve.message_block();
		return;
	}
	else if (strFirstInput == ".rules" && !DisabledGroup.count(eve.fromGroup)) {
		int intMsgCnt = strFirstInput.length();
		while (eve.message[intMsgCnt] == ' ')intMsgCnt++;
		string strSearch = eve.message.substr(intMsgCnt);
		for (auto &n : strSearch)n = toupper(n);
		for (int i = 0; i != strSearch.length(); ++i)if (strSearch[i] == ' ') {
			strSearch.erase(strSearch.begin() + i);
			i--;
		}
		strSearch = "||" + strSearch + "||";
		int Loc = strRules.find(strSearch);
		if (Loc != string::npos||strSearch=="||战斗||"||strSearch=="||战斗流程||"||strSearch=="||战斗伤害||") {
			if (strSearch == "||战斗||" || strSearch == "||战斗流程||") {
				string IMAGE = "[CQ:image,file=";
				string IMAGENAME = "Combat.jpg";
				IMAGE += msg_encode(IMAGENAME, true) + "]";
				int res = sendGroupMsg(eve.fromGroup, IMAGE);
				if (res < 0)sendGroupMsg(eve.fromGroup, "未找到对应的规则信息!");
			}
			else if (strSearch == "||战斗伤害||") {
				string IMAGE = "[CQ:image,file=";
				string IMAGENAME = "Combat1.jpg";
				IMAGE += msg_encode(IMAGENAME, true) + "]";
				int res = sendGroupMsg(eve.fromGroup, IMAGE);
				if (res < 0)sendGroupMsg(eve.fromGroup, "未找到对应的规则信息!");
			}
			else {
				int LocStart = strRules.find("]", Loc) + 1;
				sendGroupMsg(eve.fromGroup, strRules.substr(LocStart, strRules.find("[", LocStart) - LocStart - 1));
			}
		}
		else {
			sendGroupMsg(eve.fromGroup, "未找到对应的规则信息!");
		}

		eve.message_block();
		return;
	}
	else if (strFirstInput == ".ob" && !DisabledGroup.count(eve.fromGroup)) {
		int intMsgCnt = strFirstInput.length();
		while (eve.message[intMsgCnt] == ' ')intMsgCnt++;
		string Command = eve.message.substr(intMsgCnt, eve.message.find(' ', intMsgCnt) == string::npos ? eve.message.length() : eve.message.find(' ', intMsgCnt) - intMsgCnt);
		for (auto &kk : Command)kk = tolower(kk);
		if (Command == "") {
			auto Range = ObserveGroup.equal_range(eve.fromGroup);
			for (auto it = Range.first; it != Range.second; ++it) {
				if (it->second == eve.fromQQ) {
					sendGroupMsg(eve.fromGroup, strNickName + "已经处于旁观模式!");
					eve.message_block();
					return;
				}
			}
			ObserveGroup.insert(make_pair(eve.fromGroup, eve.fromQQ));
			sendGroupMsg(eve.fromGroup, strNickName + "成功加入旁观模式!");
		}
		else if(Command=="exit"){
			auto Range = ObserveGroup.equal_range(eve.fromGroup);
			for (auto it = Range.first; it != Range.second; ++it) {
				if (it->second == eve.fromQQ) {
					ObserveGroup.erase(it);
					sendGroupMsg(eve.fromGroup, strNickName + "成功退出旁观模式!");
					eve.message_block();
					return;
				}
			}
			sendGroupMsg(eve.fromGroup, strNickName + "没有加入旁观模式!");
		}
		else if (Command == "list") {
			string Msg = "当前的旁观者有:";
			auto Range = ObserveGroup.equal_range(eve.fromGroup);
			for (auto it = Range.first; it != Range.second; ++it) {
				pair<long long, long long> ObGroup;
				ObGroup.first = it->second;
				ObGroup.second = eve.fromGroup;
				Msg += "\n" + (GroupName.count(ObGroup) ? GroupName[ObGroup] : getGroupMemberInfo(eve.fromGroup, it->second).GroupNickname.empty() ? getStrangerInfo(it->second).nick : getGroupMemberInfo(eve.fromGroup, it->second).GroupNickname) + "(" + to_string(it->second) + ")";
			}
			sendGroupMsg(eve.fromGroup, Msg == "当前的旁观者有:" ? "当前暂无旁观者" : Msg);
		}
		else if (Command == "clr") {
			if (getGroupMemberInfo(eve.fromGroup, eve.fromQQ).permissions >= 2) {
				ObserveGroup.erase(eve.fromGroup);
				sendGroupMsg(eve.fromGroup, "成功删除所有旁观者!");
			}
			else {
				sendGroupMsg(eve.fromGroup, "你没有权限执行此命令!");
			}
		}
		else sendGroupMsg(eve.fromGroup, "无法识别的命令!");
		eve.message_block();
		return;
	}
	else if (strFirstInput == ".jrrp" && !DisabledGroup.count(eve.fromGroup)) {
		int intMsgCnt = strFirstInput.length();
		while (eve.message[intMsgCnt] == ' ')intMsgCnt++;
		string Command = eve.message.substr(intMsgCnt, eve.message.find(' ', intMsgCnt) == string::npos ? eve.message.length() : eve.message.find(' ', intMsgCnt) - intMsgCnt);
		for (auto &kk : Command)kk = tolower(kk);
		if (Command == "on") {
			if (getGroupMemberInfo(eve.fromGroup, eve.fromQQ).permissions >= 2) {
				if (DisabledJRRPGroup.count(eve.fromGroup)) {
					DisabledJRRPGroup.erase(eve.fromGroup);
					sendGroupMsg(eve.fromGroup, "成功在本群中启用JRRP!");
				}
				else {
					sendGroupMsg(eve.fromGroup, "在本群中JRRP没有被禁用!");
				}
			}
			else sendGroupMsg(eve.fromGroup, "您没有权限执行此命令!");

		}
		else if (Command == "off") {
			if (getGroupMemberInfo(eve.fromGroup, eve.fromQQ).permissions >= 2) {
				if (!DisabledJRRPGroup.count(eve.fromGroup)) {
					DisabledJRRPGroup.insert(eve.fromGroup);
					sendGroupMsg(eve.fromGroup, "成功在本群中禁用JRRP!");
				}
				else {
					sendGroupMsg(eve.fromGroup, "在本群中JRRP没有被启用!");
				}
			}
			else sendGroupMsg(eve.fromGroup, "您没有权限执行此命令!");
		}
		else if(!DisabledJRRPGroup.count(eve.fromGroup)){
			char cstrDate[100] = {};
			time_t time_tTime = 0;
			time(&time_tTime);
			tm tmTime;
			localtime_s(&tmTime, &time_tTime);
			strftime(cstrDate, 100, "%F", &tmTime);
			if (JRRP.count(eve.fromQQ) && JRRP[eve.fromQQ].Date == cstrDate) {
				sendGroupMsg(eve.fromGroup, strNickName + "今天的人品值是:" + to_string(JRRP[eve.fromQQ].RPVal));
			}
			else {
				int rdD100 = 0;
				MainRoll(rdD100, "D100");
				JRRP[eve.fromQQ].Date = cstrDate;
				JRRP[eve.fromQQ].RPVal = rdD100;
				sendGroupMsg(eve.fromGroup, strNickName + "今天的人品值是:" + to_string(JRRP[eve.fromQQ].RPVal));
			}
		}	
		eve.message_block();
		return;
	}
	else if (strFirstInput == ".help" && !DisabledGroup.count(eve.fromGroup)) {
		sendGroupMsg(eve.fromGroup, strHlpMsg);
		eve.message_block();
		return;
	}
	else if (strFirstInput == ".me" && !DisabledGroup.count(eve.fromGroup)) {
		int intMsgCnt = strFirstInput.length();
		while (eve.message[intMsgCnt] == ' ')intMsgCnt++;
		string Command = eve.message.substr(intMsgCnt, eve.message.find(' ', intMsgCnt) == string::npos ? eve.message.length() : eve.message.find(' ', intMsgCnt) - intMsgCnt);
		for (auto &kk : Command)kk = tolower(kk);
		if (Command == "on") {
			if (getGroupMemberInfo(eve.fromGroup, eve.fromQQ).permissions >= 2) {
				if (DisabledMEGroup.count(eve.fromGroup)) {
					DisabledMEGroup.erase(eve.fromGroup);
					sendGroupMsg(eve.fromGroup, "成功在本群中启用ME!");
				}
				else {
					sendGroupMsg(eve.fromGroup, "在本群中ME没有被禁用!");
				}
			}
			else sendGroupMsg(eve.fromGroup, "您没有权限执行此命令!");
		}
		else if (Command == "off") {
			if (getGroupMemberInfo(eve.fromGroup, eve.fromQQ).permissions >= 2) {
				if (!DisabledMEGroup.count(eve.fromGroup)) {
					DisabledMEGroup.insert(eve.fromGroup);
					sendGroupMsg(eve.fromGroup, "成功在本群中禁用ME!");
				}
				else {
					sendGroupMsg(eve.fromGroup, "在本群中ME没有被启用!");
				}
			}
			else sendGroupMsg(eve.fromGroup, "您没有权限执行此命令!");
		}
		else if (Command != "" && !DisabledMEGroup.count(eve.fromGroup)) {
			sendGroupMsg(eve.fromGroup, strNickName+eve.message.substr(intMsgCnt));
		}
		else if (!DisabledMEGroup.count(eve.fromGroup)) {
			sendGroupMsg(eve.fromGroup, "动作不能为空!");
		}
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
	if (eve.message[0] == '.'&&!DisabledGroup.count(eve.fromGroup))sendGroupMsg(eve.fromGroup, "命令输入错误!");
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
		if ((strFirstInput[k] == 'd' || strFirstInput[k] == 'D') && (isdigit(strFirstInput[k + 1])) && ((strFirstInput.substr(k + 1, (strFirstInput.find("+")<strFirstInput.find("-") ? strFirstInput.find("+") : strFirstInput.find("-")) - k - 1)).find("d") == string::npos) && ((strFirstInput.substr(k + 1, (strFirstInput.find("+")<strFirstInput.find("-") ? strFirstInput.find("+") : strFirstInput.find("-")) - k - 1)).find("D") == string::npos)) {
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
			else if (strSecondInput == "疯狂" || strSecondInput == "INSANE" || strSecondInput == "INSANITY" || strSecondInput == "临时疯狂" || strSecondInput == "临时性疯狂" || strSecondInput == "短期疯狂" || strSecondInput == "TI") {
				string strAns = strNickName + "的临时性疯狂:\n";
				TempInsane(strAns);
				sendPrivateMsg(eve.fromQQ, strAns);
			}
			else if (strSecondInput == "长期疯狂" || strSecondInput == "不定期疯狂" || strSecondInput == "不定时疯狂" || strSecondInput == "不定疯狂" || strSecondInput == "LI" || strSecondInput == "不定性疯狂") {
				string strAns = strNickName + "的不定性疯狂:\n";
				LongInsane(strAns);
				sendPrivateMsg(eve.fromQQ, strAns);
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
				if (strBonusNum == "")intBonusNum = 1;
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
					
					int D10res = -!!(D100res % 10);
					MainRoll(D10res, "D10");
					if (D10res < MinVal)MinVal = D10res;
					strAns += to_string(D10res) + " ";
				}
				strAns[strAns.length() - 1] = '\n';
				strAns += "结果为:" + to_string((D100res / 10 < MinVal ? D100res / 10 : MinVal) * 10 + D100res % 10);
				if (intMsgCnt != eve.message.length() && w == 0)strReason = eve.message.substr(intMsgCnt);
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
				if (strBonusNum == "")intBonusNum = 1;
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
					
					int D10res = -!!(D100res % 10);
					MainRoll(D10res, "D10");
					if (D10res > MaxVal)MaxVal = D10res;
					strAns += to_string(D10res) + " ";
				}
				strAns[strAns.length() - 1] = '\n';
				strAns += "结果为:" + to_string((D100res / 10 > MaxVal ? D100res / 10 : MaxVal) * 10 + D100res % 10);
				if (intMsgCnt != eve.message.length() && w == 0)strReason = eve.message.substr(intMsgCnt);
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
						strSecondInputCopy = strSecondInput.substr(0, k);
						break;
					}
				}
				if (strSecondInput.find("d") == string::npos&&strSecondInput.find("D") == string::npos) {
					strReason = strSecondInputCopy + strReason;
					strSecondInput = "";
					strSecondInputCopy = "";
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
				if (intMsgCnt != eve.message.length() && w == 0)strReason += eve.message.substr(intMsgCnt);

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
	else if (strFirstInput == ".rules") {
		int intMsgCnt = strFirstInput.length();
		while (eve.message[intMsgCnt] == ' ')intMsgCnt++;
		string strSearch = eve.message.substr(intMsgCnt);
		for (auto &n : strSearch)n = toupper(n);
		for (int i = 0; i != strSearch.length(); ++i)if (strSearch[i] == ' ') {
			strSearch.erase(strSearch.begin() + i);
			i--;
		}
		strSearch = "||" + strSearch + "||";
		int Loc = strRules.find(strSearch);
		if (Loc != string::npos || strSearch == "||战斗||" || strSearch == "||战斗流程||" || strSearch == "||战斗伤害||") {
			if (strSearch == "||战斗||" || strSearch == "||战斗流程||") {
				string IMAGE = "[CQ:image,file=";
				string IMAGENAME = "Combat.jpg";
				IMAGE += msg_encode(IMAGENAME, true) + "]";
				int res = sendPrivateMsg(eve.fromQQ, IMAGE);
				if (res < 0)sendPrivateMsg(eve.fromQQ, "未找到对应的规则信息!");
			}
			else if (strSearch == "||战斗伤害||") {
				string IMAGE = "[CQ:image,file=";
				string IMAGENAME = "Combat1.jpg";
				IMAGE += msg_encode(IMAGENAME, true) + "]";
				int res = sendPrivateMsg(eve.fromQQ, IMAGE);
				if (res < 0)sendPrivateMsg(eve.fromQQ, "未找到对应的规则信息!");
			}
			else {
				int LocStart = strRules.find("]", Loc) + 1;
				sendPrivateMsg(eve.fromQQ, strRules.substr(LocStart, strRules.find("[", LocStart) - LocStart - 1));
			}
		}
		else {
			sendPrivateMsg(eve.fromQQ, "未找到对应的规则信息!");
		}

		eve.message_block();
		return;
	}
	else if (strFirstInput == ".jrrp") {
		char cstrDate[100] = {};
		time_t time_tTime = 0;
		time(&time_tTime);
		tm tmTime;
		localtime_s(&tmTime, &time_tTime);
		strftime(cstrDate, 100, "%F", &tmTime);
		if (JRRP.count(eve.fromQQ) && JRRP[eve.fromQQ].Date == cstrDate) {
			sendPrivateMsg(eve.fromQQ, strNickName + "今天的人品值是:" + to_string(JRRP[eve.fromQQ].RPVal));
		}
		else {
			int rdD100 = 0;
			MainRoll(rdD100, "D100");
			JRRP[eve.fromQQ].Date = cstrDate;
			JRRP[eve.fromQQ].RPVal = rdD100;
			sendPrivateMsg(eve.fromQQ, strNickName + "今天的人品值是:" + to_string(JRRP[eve.fromQQ].RPVal));
		}
		eve.message_block();
		return;
	}
	else if (strFirstInput == ".help") {
		sendPrivateMsg(eve.fromQQ, strHlpMsg);
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
		long long llGroupnum = Str2ll(strGroupnum);
		if (strGroupnum == "") {
			sendPrivateMsg(eve.fromQQ, "未输入群号!输入.help来获得帮助!");
		}
		else if (DisabledGroup.count(llGroupnum)) {
			sendPrivateMsg(eve.fromQQ, "在此群中机器人已被关闭!");
		}
		else if (DisabledMEGroup.count(llGroupnum)) {
			sendPrivateMsg(eve.fromQQ, "在此群中ME功能已经被禁用!");
		}
		else if (Action == "") {
			sendPrivateMsg(eve.fromQQ, "动作不能为空!");
		}
		else {
			pair<long long, long long>pairQQGroup(eve.fromQQ, llGroupnum);
			int Sendres=sendGroupMsg(llGroupnum, (GroupName.count(pairQQGroup) ? GroupName[pairQQGroup] : getGroupMemberInfo(llGroupnum, eve.fromQQ).GroupNickname.empty() ? getStrangerInfo(eve.fromQQ).nick : getGroupMemberInfo(llGroupnum, eve.fromQQ).GroupNickname) + Action);
			if(Sendres>=0)sendPrivateMsg(eve.fromQQ, "命令执行成功!");
			else sendPrivateMsg(eve.fromQQ, "命令执行失败!是否输入了错误的群号?");
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

	if(eve.message[0]=='.')sendPrivateMsg(eve.fromQQ, "命令输入错误!");
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
		if ((strFirstInput[k] == 'd' || strFirstInput[k] == 'D') && (isdigit(strFirstInput[k + 1])) && ((strFirstInput.substr(k + 1, (strFirstInput.find("+")<strFirstInput.find("-") ? strFirstInput.find("+") : strFirstInput.find("-")) - k - 1)).find("d") == string::npos) && ((strFirstInput.substr(k + 1, (strFirstInput.find("+")<strFirstInput.find("-") ? strFirstInput.find("+") : strFirstInput.find("-")) - k - 1)).find("D") == string::npos)) {
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
			if (isHidden) {
				sendPrivateMsg(eve.fromQQ, strRollDiceTurnNotice);
				auto Range = ObserveDiscuss.equal_range(eve.fromDiscuss);
				for (auto it = Range.first; it != Range.second; ++it) {
					if (it->second != eve.fromQQ)sendPrivateMsg(it->second, strRollDiceTurnNotice);
				}
			} else
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
					auto Range = ObserveDiscuss.equal_range(eve.fromDiscuss);
					for (auto it = Range.first; it != Range.second; ++it) {
						if (it->second != eve.fromQQ)sendPrivateMsg(it->second, strMAns);
					}
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
					auto Range = ObserveDiscuss.equal_range(eve.fromDiscuss);
					for (auto it = Range.first; it != Range.second; ++it) {
						if (it->second != eve.fromQQ)sendPrivateMsg(it->second, strMAns);
					}
				}
				else
					sendDiscussMsg(eve.fromDiscuss, strMAns);
			}
			else if (strSecondInput == "疯狂" || strSecondInput == "INSANE" || strSecondInput == "INSANITY" || strSecondInput == "临时疯狂" || strSecondInput == "临时性疯狂" || strSecondInput == "短期疯狂" || strSecondInput == "TI") {
				string strAns = strNickName + "的临时性疯狂:\n";
				TempInsane(strAns);
				if (isHidden) sendPrivateMsg(eve.fromQQ, strAns);
				else sendDiscussMsg(eve.fromDiscuss, strAns);
			}
			else if (strSecondInput == "长期疯狂" || strSecondInput == "不定期疯狂" || strSecondInput == "不定时疯狂" || strSecondInput == "不定疯狂" || strSecondInput == "LI" || strSecondInput == "不定性疯狂") {
				string strAns = strNickName + "的不定性疯狂:\n";
				LongInsane(strAns);
				if (isHidden) sendPrivateMsg(eve.fromQQ, strAns);
				else sendDiscussMsg(eve.fromDiscuss, strAns);
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
					auto Range = ObserveDiscuss.equal_range(eve.fromDiscuss);
					for (auto it = Range.first; it != Range.second; ++it) {
						if (it->second != eve.fromQQ)sendPrivateMsg(it->second, strMAns);
					}
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
				if (strBonusNum == "")intBonusNum = 1;
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
					
					int D10res = -!!(D100res % 10);
					MainRoll(D10res, "D10");
					if (D10res < MinVal)MinVal = D10res;
					strAns += to_string(D10res) + " ";
				}
				strAns[strAns.length() - 1] = '\n';
				strAns += "结果为:" + to_string((D100res / 10 < MinVal ? D100res / 10 : MinVal) * 10 + D100res % 10);
				if (intMsgCnt != eve.message.length() && w == 0)strReason = eve.message.substr(intMsgCnt);
				//If the reason is available, add the reason in the front of the output
				if (!strReason.empty()) {
					strAns = "由于" + strReason + strAns;
				}
				if (isHidden) {
					strAns = "在多人聊天" + strDiscussName + "中" + strAns;
					sendPrivateMsg(eve.fromQQ, strAns);
					auto Range = ObserveDiscuss.equal_range(eve.fromDiscuss);
					for (auto it = Range.first; it != Range.second; ++it) {
						if (it->second != eve.fromQQ)sendPrivateMsg(it->second, strAns);
					}
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
				if (strBonusNum == "")intBonusNum = 1;
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
					
					int D10res = -!!(D100res % 10);
					MainRoll(D10res, "D10");
					if (D10res > MaxVal)MaxVal = D10res;
					strAns += to_string(D10res) + " ";
				}
				strAns[strAns.length() - 1] = '\n';
				strAns += "结果为:" + to_string((D100res / 10 > MaxVal ? D100res / 10 : MaxVal) * 10 + D100res % 10);
				while (eve.message[intMsgCnt] == ' '&&intMsgCnt != eve.message.length())intMsgCnt++;
				if (intMsgCnt != eve.message.length() && w == 0)strReason = eve.message.substr(intMsgCnt);
				//If the reason is available, add the reason in the front of the output
				if (!strReason.empty()) {
					strAns = "由于" + strReason + strAns;
				}
				if (isHidden) {
					strAns = "在多人聊天" + strDiscussName + "中" + strAns;
					sendPrivateMsg(eve.fromQQ, strAns);
					auto Range = ObserveDiscuss.equal_range(eve.fromDiscuss);
					for (auto it = Range.first; it != Range.second; ++it) {
						if (it->second != eve.fromQQ)sendPrivateMsg(it->second, strAns);
					}
				}
				else
					sendDiscussMsg(eve.fromDiscuss, strAns);
			}
			else {

				for (int k = 0; k != strSecondInput.length(); k++) {
					if (!isprint(strSecondInput[k]) || (!isdigit(strSecondInput[k]) && strSecondInput[k] != '-'&&strSecondInput[k] != '+' && strSecondInput[k] != 'D') || (strSecondInput[k] == 'D' && !isdigit(strSecondInput[k + 1]))) {
						strReason += strSecondInputCopy.substr(k);
						strSecondInput = strSecondInput.substr(0, k);
						strSecondInputCopy = strSecondInput.substr(0, k);
						break;
					}
				}
				if (strSecondInput.find("d") == string::npos&&strSecondInput.find("D") == string::npos) {
					strReason = strSecondInputCopy + strReason;
					strSecondInput = "";
					strSecondInputCopy = "";
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
				if (intMsgCnt != eve.message.length() && w == 0)strReason += eve.message.substr(intMsgCnt);

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
					auto Range = ObserveDiscuss.equal_range(eve.fromDiscuss);
					for (auto it = Range.first; it != Range.second; ++it) {
						if (it->second != eve.fromQQ)sendPrivateMsg(it->second, strAns);
					}
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
else if (strFirstInput == ".nn" && !DisabledDiscuss.count(eve.fromDiscuss)) {
	int intMsgCnt = strFirstInput.length();
	while (eve.message[intMsgCnt] == ' ')intMsgCnt++;
	string name = eve.message.substr(intMsgCnt);
	if (name == "del") {
		if (DiscussName.count(pairQQDiscuss)) {
			DiscussName.erase(pairQQDiscuss);
			sendDiscussMsg(eve.fromDiscuss, "已将" + strNickName + "的名称删除");
		}
		else sendDiscussMsg(eve.fromDiscuss, strErrMsg);
	}
	else {
		if (!name.empty()) {
			DiscussName[pairQQDiscuss] = name;
			sendDiscussMsg(eve.fromDiscuss, "已将" + strNickName + "的名称更改为" + name);
		}
		else {
			sendDiscussMsg(eve.fromDiscuss, "名称不能为空!");
		}
	}
	eve.message_block();
	return;
}
else if (strFirstInput == ".bot") {
	int intMsgCnt = strFirstInput.length();
	while (eve.message[intMsgCnt] == ' ')intMsgCnt++;
	string Command = eve.message.substr(intMsgCnt, eve.message.find(' ', intMsgCnt) == string::npos ? eve.message.length() : eve.message.find(' ', intMsgCnt) - intMsgCnt);
	intMsgCnt = eve.message.find(' ', intMsgCnt) == string::npos ? eve.message.length() : eve.message.find(' ', intMsgCnt);
	while (eve.message[intMsgCnt] == ' ')intMsgCnt++;
	string Number = eve.message.substr(intMsgCnt, eve.message.find(' ', intMsgCnt) == string::npos ? eve.message.length() : eve.message.find(' ', intMsgCnt) - intMsgCnt);
	for (auto &kk : Command)kk = tolower(kk);
	if (isdigit(Command[0])) {
		Command = "";
		Number = Command;
	}
	for (int index = 0; index != Command.length(); index++) {
		if (isdigit(Command[index])) {
			Number = Command.substr(index);
			Command = Command.substr(0, index);
			break;
		}
	}
	if (Command == "") {
		if (Number == "" || Number == to_string(getLoginQQ())) {
			if (DisabledDiscuss.count(eve.fromDiscuss)) sendDiscussMsg(eve.fromDiscuss, "机器人处于关闭状态!");
			else sendDiscussMsg(eve.fromDiscuss, "机器人处于开启状态!");
		}
	}
	else if (Command == "on") {
		if (Number == "" || Number == to_string(getLoginQQ())) {
			if (DisabledDiscuss.count(eve.fromDiscuss)) {
				DisabledDiscuss.erase(eve.fromDiscuss);
				sendDiscussMsg(eve.fromDiscuss, "成功开启本机器人!");
			}
			else sendDiscussMsg(eve.fromDiscuss, "本机器人已经处于开启状态!");

		}
	}
	else if (Command == "off") {
		if (Number == "" || Number == to_string(getLoginQQ())) {
			if (!DisabledDiscuss.count(eve.fromDiscuss)) {
				DisabledDiscuss.insert(eve.fromDiscuss);
				sendDiscussMsg(eve.fromDiscuss, "成功关闭本机器人!");
			}
			else sendDiscussMsg(eve.fromDiscuss, "本机器人已经处于关闭状态!");

		}
	}
	else sendDiscussMsg(eve.fromDiscuss, "无法识别的命令!");
	eve.message_block();
	return;
}
else if (strFirstInput == ".jrrp" && !DisabledDiscuss.count(eve.fromDiscuss)) {
	int intMsgCnt = strFirstInput.length();
	while (eve.message[intMsgCnt] == ' ')intMsgCnt++;
	string Command = eve.message.substr(intMsgCnt, eve.message.find(' ', intMsgCnt) == string::npos ? eve.message.length() : eve.message.find(' ', intMsgCnt) - intMsgCnt);
	for (auto &kk : Command)kk = tolower(kk);
	if (Command == "on") {
		if (DisabledJRRPDiscuss.count(eve.fromDiscuss)) {
			DisabledJRRPDiscuss.erase(eve.fromDiscuss);
			sendDiscussMsg(eve.fromDiscuss, "成功在本多人聊天中启用JRRP!");
		}
		else {
			sendDiscussMsg(eve.fromDiscuss, "在本多人聊天中JRRP没有被禁用!");
		}
	}
	else if (Command == "off") {
		if (!DisabledJRRPDiscuss.count(eve.fromDiscuss)) {
			DisabledJRRPDiscuss.insert(eve.fromDiscuss);
			sendDiscussMsg(eve.fromDiscuss, "成功在本多人聊天中禁用JRRP!");
		}
		else {
			sendDiscussMsg(eve.fromDiscuss, "在本多人聊天中JRRP没有被启用!");
		}

	}
	else if(!DisabledDiscuss.count(eve.fromDiscuss)){
		char cstrDate[100] = {};
		time_t time_tTime = 0;
		time(&time_tTime);
		tm tmTime;
		localtime_s(&tmTime, &time_tTime);
		strftime(cstrDate, 100, "%F", &tmTime);
		if (JRRP.count(eve.fromQQ) && JRRP[eve.fromQQ].Date == cstrDate) {
			sendDiscussMsg(eve.fromDiscuss, strNickName + "今天的人品值是:" + to_string(JRRP[eve.fromQQ].RPVal));
		}
		else {
			int rdD100 = 0;
			MainRoll(rdD100, "D100");
			JRRP[eve.fromQQ].Date = cstrDate;
			JRRP[eve.fromQQ].RPVal = rdD100;
			sendDiscussMsg(eve.fromDiscuss, strNickName + "今天的人品值是:" + to_string(JRRP[eve.fromQQ].RPVal));
		}
	}
	eve.message_block();
	return;
}
	else if (strFirstInput == ".rules" && !DisabledDiscuss.count(eve.fromDiscuss)) {
		int intMsgCnt = strFirstInput.length();
		while (eve.message[intMsgCnt] == ' ')intMsgCnt++;
		string strSearch = eve.message.substr(intMsgCnt);
		for (auto &n : strSearch)n = toupper(n);
		for (int i = 0; i != strSearch.length(); ++i)if (strSearch[i] == ' ') {
			strSearch.erase(strSearch.begin() + i);
			i--;
		}
		strSearch = "||" + strSearch + "||";
		int Loc = strRules.find(strSearch);
		if (Loc != string::npos || strSearch == "||战斗||" || strSearch == "||战斗流程||" || strSearch == "||战斗伤害||") {
			if (strSearch == "||战斗||" || strSearch == "||战斗流程||") {
				string IMAGE = "[CQ:image,file=";
				string IMAGENAME = "Combat.jpg";
				IMAGE += msg_encode(IMAGENAME, true) + "]";
				int res = sendDiscussMsg(eve.fromDiscuss, IMAGE);
				if (res < 0)sendDiscussMsg(eve.fromDiscuss, "未找到对应的规则信息!");
			}
			else if (strSearch == "||战斗伤害||") {
				string IMAGE = "[CQ:image,file=";
				string IMAGENAME = "Combat1.jpg";
				IMAGE += msg_encode(IMAGENAME, true) + "]";
				int res = sendDiscussMsg(eve.fromDiscuss, IMAGE);
				if (res < 0)sendDiscussMsg(eve.fromDiscuss, "未找到对应的规则信息!");
			}
			else {
				int LocStart = strRules.find("]", Loc) + 1;
				sendDiscussMsg(eve.fromDiscuss, strRules.substr(LocStart, strRules.find("[", LocStart) - LocStart - 1));
			}
		}
		else {
			sendDiscussMsg(eve.fromDiscuss, "未找到对应的规则信息!");
		}
		eve.message_block();
		return;
	}
	else if (strFirstInput == ".ob" && !DisabledDiscuss.count(eve.fromDiscuss)) {
		int intMsgCnt = strFirstInput.length();
		while (eve.message[intMsgCnt] == ' ')intMsgCnt++;
		string Command = eve.message.substr(intMsgCnt, eve.message.find(' ', intMsgCnt) == string::npos ? eve.message.length() : eve.message.find(' ', intMsgCnt) - intMsgCnt);
		for (auto &kk : Command)kk = tolower(kk);
		if (Command == "") {
			auto Range = ObserveDiscuss.equal_range(eve.fromDiscuss);
			for (auto it = Range.first; it != Range.second; ++it) {
				if (it->second == eve.fromQQ) {
					sendDiscussMsg(eve.fromDiscuss, strNickName + "已经处于旁观模式!");
					eve.message_block();
					return;
				}
			}
			ObserveDiscuss.insert(make_pair(eve.fromDiscuss, eve.fromQQ));
			sendDiscussMsg(eve.fromDiscuss, strNickName + "成功加入旁观模式!");
		}
		else if (Command == "exit") {
			auto Range = ObserveDiscuss.equal_range(eve.fromDiscuss);
			for (auto it = Range.first; it != Range.second; ++it) {
				if (it->second == eve.fromQQ) {
					ObserveDiscuss.erase(it);
					sendDiscussMsg(eve.fromDiscuss, strNickName + "成功退出旁观模式!");
					eve.message_block();
					return;
				}
			}
			sendDiscussMsg(eve.fromDiscuss, strNickName + "没有加入旁观模式!");
		}
		else if (Command == "list") {
			string Msg = "当前的旁观者有:";
			auto Range = ObserveDiscuss.equal_range(eve.fromDiscuss);
			for (auto it = Range.first; it != Range.second; ++it) {
				pair<long long, long long> ObDiscuss;
				ObDiscuss.first = it->second;
				ObDiscuss.second = eve.fromDiscuss;
				Msg += "\n" + (DiscussName.count(ObDiscuss) ? DiscussName[ObDiscuss] : getStrangerInfo(it->second).nick) + "(" + to_string(it->second) + ")";
			}
			sendDiscussMsg(eve.fromDiscuss, Msg == "当前的旁观者有:" ? "当前暂无旁观者" : Msg);
		}
		else if (Command == "clr") {
			ObserveDiscuss.erase(eve.fromDiscuss);
			sendDiscussMsg(eve.fromDiscuss, "成功删除所有旁观者!");
		}
		else sendDiscussMsg(eve.fromDiscuss, "无法识别的命令!");
		eve.message_block();
		return;
	}
	else if (strFirstInput == ".help" && !DisabledDiscuss.count(eve.fromDiscuss)) {
		sendDiscussMsg(eve.fromDiscuss, strHlpMsg);
		eve.message_block();
		return;
	}
	else if (strFirstInput == ".me" && !DisabledDiscuss.count(eve.fromDiscuss)) {
		int intMsgCnt = strFirstInput.length();
		while (eve.message[intMsgCnt] == ' ')intMsgCnt++;
		string Command = eve.message.substr(intMsgCnt, eve.message.find(' ', intMsgCnt) == string::npos ? eve.message.length() : eve.message.find(' ', intMsgCnt) - intMsgCnt);
		for (auto &kk : Command)kk = tolower(kk);
		if (Command == "on") {
			if (DisabledMEDiscuss.count(eve.fromDiscuss)) {
				DisabledMEDiscuss.erase(eve.fromDiscuss);
				sendDiscussMsg(eve.fromDiscuss, "成功在本群中启用ME!");
			}
			else {
				sendDiscussMsg(eve.fromDiscuss, "在本群中ME没有被禁用!");
			}
		}
		else if (Command == "off") {
			if (!DisabledMEDiscuss.count(eve.fromDiscuss)) {
				DisabledMEDiscuss.insert(eve.fromDiscuss);
				sendDiscussMsg(eve.fromDiscuss, "成功在本群中禁用ME!");
			}
			else {
				sendDiscussMsg(eve.fromDiscuss, "在本群中ME没有被启用!");
			}
		}
		else if (Command != "" && !DisabledMEDiscuss.count(eve.fromDiscuss)) {
			sendDiscussMsg(eve.fromDiscuss, strNickName + eve.message.substr(intMsgCnt));
		}
		else if (!DisabledMEDiscuss.count(eve.fromDiscuss)) {
			sendDiscussMsg(eve.fromDiscuss, "动作不能为空!");
		}
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
	if (eve.message[0] == '.'&&!DisabledDiscuss.count(eve.fromDiscuss))sendDiscussMsg(eve.fromDiscuss, "命令输入错误!");
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
	ofstream ofstreamDisabledJRRPGroup(strFileLoc + "DisabledJRRPGroup.config", ios::in | ios::trunc);
	for (auto it = DisabledJRRPGroup.begin(); it != DisabledJRRPGroup.end(); it++) {
		ofstreamDisabledJRRPGroup << *it << endl;
	}
	ofstreamDisabledJRRPGroup.close();

	ofstream ofstreamDisabledJRRPDiscuss(strFileLoc + "DisabledJRRPDiscuss.config", ios::in | ios::trunc);
	for (auto it = DisabledJRRPDiscuss.begin(); it != DisabledJRRPDiscuss.end(); it++) {
		ofstreamDisabledJRRPDiscuss << *it << endl;
	}
	ofstreamDisabledJRRPDiscuss.close();

	ofstream ofstreamDisabledMEGroup(strFileLoc + "DisabledMEGroup.config", ios::in | ios::trunc);
	for (auto it = DisabledMEGroup.begin(); it != DisabledMEGroup.end(); it++) {
		ofstreamDisabledMEGroup << *it << endl;
	}
	ofstreamDisabledMEGroup.close();

	ofstream ofstreamDisabledMEDiscuss(strFileLoc + "DisabledMEDiscuss.config", ios::in | ios::trunc);
	for (auto it = DisabledMEDiscuss.begin(); it != DisabledMEDiscuss.end(); it++) {
		ofstreamDisabledMEDiscuss << *it << endl;
	}
	ofstreamDisabledMEDiscuss.close();

	ofstream ofstreamObserveGroup(strFileLoc + "ObserveGroup.config", ios::in | ios::trunc);
	for (auto it = ObserveGroup.begin(); it != ObserveGroup.end(); it++) {
		ofstreamObserveGroup << it->first << " " << it->second << endl;
	}
	ofstreamObserveGroup.close();

	ofstream ofstreamObserveDiscuss(strFileLoc + "ObserveDiscuss.config", ios::in | ios::trunc);
	for (auto it = ObserveDiscuss.begin(); it != ObserveDiscuss.end(); it++) {
		ofstreamObserveDiscuss << it->first << " " << it->second << endl;
	}
	ofstreamObserveDiscuss.close();
	ofstream ofstreamJRRP(strFileLoc + "JRRP.config", ios::in | ios::trunc);
	for (auto it = JRRP.begin(); it != JRRP.end(); it++) {
		ofstreamJRRP << it->first << " " << it->second.Date << " " << it->second.RPVal << endl;
	}
	ofstreamJRRP.close();
	return 0;
}

MUST_AppInfo_RETURN(CQAPPID)
