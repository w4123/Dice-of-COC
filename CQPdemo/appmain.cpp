/*
* CoolQ Demo for VC++ 
* Api Version 9
* Written by Coxxs & Thanks for the help of orzFly
*/
/*
* Dice of COC(VC++) 
* Written by w4123
* Version 1.2.0
* Commercial use is prohibited.
*/
#include "stdafx.h"
#include "string"
#include "cqp.h"
#include "appmain.h" //应用AppID等信息，请正确填写，否则酷Q可能无法加载

using namespace std;

int ac = -1; //AuthCode 调用酷Q的方法时需要用到
static string FILENAME;
static string hlpmsg = "使用方法:\n\t普通骰子: .o 骰子类型 原因(可选)\n\r本软件还附带六版和七版人物作成，命令为.o 六版人物作成和.o 七版人物作成\n\t版本:1.2.0\n\t问题反馈请联系QQ1840686745";
/* 
* 返回应用的ApiVer、Appid，打包后将不会调用
*/
CQEVENT(const char*, AppInfo, 0)() {
	return CQAPPINFO;
}
/* 
* 接收 应用AuthCode，酷Q读取应用信息后，如果接受该应用，将会调用这个函数并传递AuthCode。
* 不要在本函数处理其他任何代码，以免发生异常情况。如需执行初始化代码请在Startup事件中执行（Type=1001）。
*/
CQEVENT(int32_t, Initialize, 4)(int32_t AuthCode) {
	ac = AuthCode;
	return 0;
}

CQEVENT(int32_t, __eventEnable, 0)() {
	/*FILENAME = CQ_getAppDirectory(ac);
	FILENAME += "config.ini";
	fstream i(FILENAME.c_str());
	if (!i) {
		i.open(FILENAME.c_str(), ios::in);
		i<<"[人物作成]\nDiceCount="
	}*/

	return 0;
}

int RollDice(string Dice,string &strAns) {
	if (Dice == "")return -1;
	int cntNNum = 0;
	for (auto i : Dice) {
		if (!isdigit(i))cntNNum++;
	}
	if (cntNNum > 1)return -1;
	if (cntNNum == 1 && Dice.find("d") == Dice.npos && Dice.find("D") == Dice.npos)return -1;
	if (Dice.find("d") != Dice.npos ) {//Find "d"
		int dPosition = Dice.find("d");
		int DiceCnt = 0;//Count of dice
		int DiceType = 0;//The biggest number of the dice
		if (dPosition == 0) {
			DiceCnt = 1;//The 1 before d/D can be omitted
		}
		else {
			int tmp = 0;
			while (tmp!=dPosition) {
				DiceCnt = DiceCnt * 10 + (Dice[tmp] - 48);
				tmp++;//Get the count of the dice
			}
		}
		int tmp = dPosition + 1;
		while (tmp != Dice.length()) {
			DiceType = DiceType * 10 + (Dice[tmp] - 48);
			tmp++;//Get the biggest number of the dice
		}
		if (DiceType <= 1)return -1;
		srand((unsigned)rand()+(unsigned)time(NULL));//Generate a new key
		int sum = 0;
		for (int i = 1; i <= DiceCnt; i++) {
			int tmpRes = rand() % (DiceType - 1) + 1;//Get the random number and add it to the output string
			strAns += to_string(tmpRes);
			sum += tmpRes;
			strAns += '+';
		}
		return sum;
	}
	else if(Dice.find("D") != Dice.npos) {
			int dPosition = Dice.find("D");
			int DiceCnt = 0;
			int DiceType = 0;
			if (dPosition == 0) {
				DiceCnt = 1;
			}
			else {
				int tmp = 0;
				while (tmp != dPosition) {
					DiceCnt = DiceCnt * 10 + (Dice[tmp] - 48);
					tmp++;
				}
			}
			int tmp = dPosition + 1;
			while (tmp != Dice.length()) {
				DiceType = DiceType * 10 + (Dice[tmp] - 48);
				tmp++;
			}
			if (DiceType <= 1)return -1;
			srand((unsigned)rand() + (unsigned)time(NULL));
			int sum = 0;
			for (int i = 1; i <= DiceCnt; i++) {
				int tmpRes = rand() % (DiceType - 1) + 1;
				strAns += to_string(tmpRes);
				sum += tmpRes;
				strAns += '+';
			}
			return sum;
		
	}
	else {
		//Add a number
		strAns += Dice;
		strAns += '+';
		int intTmpAns = 0;
		for (int Cnt = 0; Cnt !=Dice.length();Cnt++) {
			intTmpAns = intTmpAns * 10 + (Dice[Cnt] - 48);
		}//Convert the string to an integer
		return intTmpAns;
	}
}

int RollDice(string Dice) {
	if (Dice == "")return -1;
	int cntNNum = 0;
	for (auto i : Dice) {
		if (!isdigit(i))cntNNum++;
	}
	if (cntNNum > 1)return -1;
	if (cntNNum == 1 && Dice.find("d") == Dice.npos && Dice.find("D") == Dice.npos)return -1;
	if (Dice.find("d") != Dice.npos) {//Find "d"
		int dPosition = Dice.find("d");
		int DiceCnt = 0;//Count of dice
		int DiceType = 0;//The biggest number of the dice
		if (dPosition == 0) {
			DiceCnt = 1;//The 1 before d/D can be omitted
		}
		else {
			int tmp = 0;
			while (tmp != dPosition) {
				DiceCnt = DiceCnt * 10 + (Dice[tmp] - 48);
				tmp++;//Get the count of the dice
			}
		}
		int tmp = dPosition + 1;
		while (tmp != Dice.length()) {
			DiceType = DiceType * 10 + (Dice[tmp] - 48);
			tmp++;//Get the biggest number of the dice
		}
		if (DiceType <= 1)return -1;
		srand((unsigned)rand() + (unsigned)time(NULL));//Generate a new key
		int sum = 0;
		for (int i = 1; i <= DiceCnt; i++) {
			int tmpRes = rand() % (DiceType - 1) + 1;//Get the random number and add it to the output string
			sum += tmpRes;
		}
		return sum;
	}
	else if (Dice.find("D") != Dice.npos) {
		int dPosition = Dice.find("D");
		int DiceCnt = 0;
		int DiceType = 0;
		if (dPosition == 0) {
			DiceCnt = 1;
		}
		else {
			int tmp = 0;
			while (tmp != dPosition) {
				DiceCnt = DiceCnt * 10 + (Dice[tmp] - 48);
				tmp++;
			}
		}
		int tmp = dPosition + 1;
		while (tmp != Dice.length()) {
			DiceType = DiceType * 10 + (Dice[tmp] - 48);
			tmp++;
		}
		if (DiceType <= 1)return -1;
		srand((unsigned)rand() + (unsigned)time(NULL));
		int sum = 0;
		for (int i = 1; i <= DiceCnt; i++) {
			int tmpRes = rand() % (DiceType - 1) + 1;
			sum += tmpRes;
		}
		return sum;

	}
	else {
		//Add a number
		int intTmpAns = 0;
		for (int Cnt = 0; Cnt != Dice.length(); Cnt++) {
			intTmpAns = intTmpAns * 10 + (Dice[Cnt] - 48);
		}//Convert the string to an integer
		return intTmpAns;
	}
}

CQEVENT(int32_t, __eventGroupMsg, 36)(int32_t subType, int32_t sendTime, int64_t fromGroup, int64_t fromQQ, const char *fromAnonymous, const char *msg, int32_t font) {
	if (msg[0] == '.'&&msg[1] == 'o') {
		string strSecondInput;
		int intCustomMsg = 2;
		while (msg[intCustomMsg] == ' ')intCustomMsg++;
		while (msg[intCustomMsg] != ' '&&msg[intCustomMsg] != '\0') {
			strSecondInput += msg[intCustomMsg];
			intCustomMsg++;
		}
		if (strSecondInput == "人物作成") {
			string strMAns = to_string(fromQQ);
			strMAns += "的人物作成: ";
			strMAns += '\n';
			strMAns += "力量STR=3D6*5=";
			int STR = RollDice("3D6") * 5;
			strMAns += to_string(STR) + "/" + to_string(STR / 2) + "/" + to_string(STR / 5);
			strMAns += '\n';
			strMAns += "体质CON=3D6*5=";
			int CON = RollDice("3D6") * 5;
			strMAns += to_string(CON) + "/" + to_string(CON / 2) + "/" + to_string(CON / 5);
			strMAns += '\n';
			strMAns += "体型SIZ=3D6*5=";
			int SIZ = RollDice("3D6") * 5;
			strMAns += to_string(SIZ) + "/" + to_string(SIZ / 2) + "/" + to_string(SIZ / 5);
			strMAns += '\n';
			strMAns += "敏捷DEX=3D6*5=";
			int DEX = RollDice("3D6") * 5;
			strMAns += to_string(DEX) + "/" + to_string(DEX / 2) + "/" + to_string(DEX / 5);
			strMAns += '\n';
			strMAns += "外貌APP=3D6*5=";
			int APP = RollDice("3D6") * 5;
			strMAns += to_string(APP) + "/" + to_string(APP / 2) + "/" + to_string(APP / 5);
			strMAns += '\n';
			strMAns += "智力INT=(2D6+6)*5=";
			int INT = (RollDice("2D6") + 6) * 5;
			strMAns += to_string(INT) + "/" + to_string(INT / 2) + "/" + to_string(INT / 5);
			strMAns += '\n';
			strMAns += "意志POW=(2D6+6)*5=";
			int POW = (RollDice("2D6") + 6) * 5;
			strMAns += to_string(POW) + "/" + to_string(POW / 2) + "/" + to_string(POW / 5);
			strMAns += '\n';
			strMAns += "教育EDU=(2D6+6)*5=";
			int EDU = (RollDice("2D6") + 6) * 5;
			strMAns += to_string(EDU) + "/" + to_string(EDU / 2) + "/" + to_string(EDU / 5);
			strMAns += '\n';
			strMAns += "幸运LUCK=3D6*5=";
			int LUCK = RollDice("3D6") * 5;
			strMAns += to_string(LUCK);
			strMAns += "\n理智SAN=POW=";
			int SAN = POW;
			strMAns += to_string(SAN);
			strMAns += "\n生命值HP=(SIZ+CON)/10=";
			int HP = (SIZ + CON) / 10;
			strMAns += to_string(HP);
			strMAns += "\n魔法值MP=POW/5=";
			int MP = POW / 5;
			strMAns += to_string(MP);
			string DB;
			int build = 0;
			if (STR + SIZ >= 2 && STR + SIZ <= 64) {
				DB = "-2";
				build = -2;
			}
			else if (STR + SIZ >= 65 && STR + SIZ <= 84) {
				DB = "-1";
				build = -1;
			}
			else if (STR + SIZ >= 85 && STR + SIZ <= 124) {
				DB = "0";
				build = 0;
			}
			else if (STR + SIZ >= 125 && STR + SIZ <= 164) {
				DB = "1D4";
				build = 1;
			}
			else if (STR + SIZ >= 165 && STR + SIZ <= 204) {
				DB = "1d6";
				build = 2;
			}
			else {
				DB = "计算错误!";
				build = -10;
			}
			strMAns += "\n伤害加值DB=" + DB + "\n体格=" + (build == -10 ? "计算错误" : to_string(build));
			int MOV = 0;
			if (DEX < SIZ&&STR < SIZ)MOV = 7;
			else if (DEX > SIZ && STR > SIZ)MOV = 9;
			else MOV = 8;
			strMAns += "\n移动力MOV=" + to_string(MOV);
			CQ_sendGroupMsg(ac,fromGroup,strMAns.c_str());
		}
		else if (strSecondInput == "六版人物作成") {
			string strMAns = to_string(fromQQ);
			strMAns += "的人物作成: ";
			strMAns += '\n';
			strMAns += "力量STR=3D6=";
			int STR = RollDice("3D6");
			strMAns += to_string(STR);
			strMAns += '\n';
			strMAns += "体质CON=3D6=";
			int CON = RollDice("3D6");
			strMAns += to_string(CON);
			strMAns += '\n';
			strMAns += "体型SIZ=3D6=";
			int SIZ = RollDice("3D6");
			strMAns += to_string(SIZ);
			strMAns += '\n';
			strMAns += "敏捷DEX=3D6=";
			int DEX = RollDice("3D6");
			strMAns += to_string(DEX);
			strMAns += '\n';
			strMAns += "外貌APP=3D6=";
			int APP = RollDice("3D6");
			strMAns += to_string(APP);
			strMAns += '\n';
			strMAns += "智力INT=2D6+6=";
			int INT = RollDice("2D6") + 6;
			strMAns += to_string(INT);
			strMAns += '\n';
			strMAns += "意志POW=2D6+6=";
			int POW = RollDice("2D6") + 6;
			strMAns += to_string(POW);
			strMAns += '\n';
			strMAns += "教育EDU=3D6+3=";
			int EDU = RollDice("3D6") + 3;
			strMAns += to_string(EDU);
			int SAN = POW * 5;
			int IDEA = INT * 5;
			int LUCK = POW * 5;
			int KNOW = EDU * 5;
			int HP = CON + SIZ;
			int MP = POW;
			strMAns += "\n理智SAN=POW*5=" + to_string(SAN) + "\n灵感IDEA=INT*5=" + to_string(IDEA) + "\n幸运LUCK=POW*5=" + to_string(LUCK) + "\n知识KNOW=EDU*5=" + to_string(KNOW);
			strMAns += "\n生命值HP=CON+SIZ=" + to_string(HP) + "\n魔法值MP=POW=" + to_string(MP);
			strMAns += "\n伤害奖励=";
			string DB;
			if (STR + SIZ >= 2 && STR + SIZ <= 12) {
				DB = "-1D6";
			}
			else if (STR + SIZ >= 13 && STR + SIZ <= 16) {
				DB = "-1D4";
			}
			else if (STR + SIZ >= 17 && STR + SIZ <= 24) {
				DB = "0";
			}
			else if (STR + SIZ >= 25 && STR + SIZ <= 32) {
				DB = "1D4";
			}
			else if (STR + SIZ >= 33 && STR + SIZ <= 40) {
				DB = "1D6";
			}
			else {
				DB = "计算错误!";
			}
			strMAns += DB;
			CQ_sendGroupMsg(ac, fromGroup, strMAns.c_str());
		}
		else {
			int intReadMsgCnt = 2;
			int intTmp;
			string strAns;//Output string
			strAns += to_string(fromQQ);//Add the QQ number at the front(Next version:Nickname)
			strAns += "骰出了";
			strAns += ": ";

			while (msg[intReadMsgCnt] == ' ')intReadMsgCnt++;//Skip the space
			for (intTmp = intReadMsgCnt; msg[intTmp] != ' '&&msg[intTmp] != '\0'; intTmp++) {
				strAns += msg[intTmp];
			}
			strAns += '=';
			string strSeparateDice;
			int mainsum = 0;//Result
			string secondaryRes;
			for (; msg[intReadMsgCnt] != ' '&&msg[intReadMsgCnt] != '\0'; intReadMsgCnt++) {
				if (msg[intReadMsgCnt] == '+') {
					int ttmpRes = RollDice(strSeparateDice, strAns);
					if (ttmpRes == -1) {
						CQ_sendGroupMsg(ac, fromGroup, "输入错误!");
						return EVENT_BLOCK;
					}
					mainsum += ttmpRes;
					secondaryRes += to_string(ttmpRes);
					secondaryRes += '+';
					strSeparateDice = "";
					//Reset
				}
				else {
					strSeparateDice += msg[intReadMsgCnt];
				}
			}
			int ttmpRes = RollDice(strSeparateDice, strAns);
			if (ttmpRes == -1) {
				CQ_sendGroupMsg(ac, fromGroup, "输入错误!");
				return EVENT_BLOCK;
			}
			mainsum += ttmpRes;
			secondaryRes += to_string(ttmpRes);
			//Call the function of rolling dice again because of the last die
			while (msg[intReadMsgCnt] == ' ')intReadMsgCnt++;
			string strReason;//Get the reason from the third part of the input
			while (msg[intReadMsgCnt] != '\0') {
				strReason += msg[intReadMsgCnt];
				intReadMsgCnt++;
			}
			if (strReason != "") {
				strAns = strReason + strAns;
				strAns = "由于" + strAns;//Add the reason at the front of the output
			}
			strAns[strAns.length() - 1] = '=';//There will be an unnecessary "+" in the end.Use "=" to replace it.
			strAns += secondaryRes;
			strAns += '=';
			strAns += to_string(mainsum);//Merge all the results to the string "strAns"
			CQ_sendGroupMsg(ac, fromGroup, strAns.c_str());//Change it to a C type string and output
		}
		return EVENT_BLOCK;
	}
	return EVENT_IGNORE;
}

CQEVENT(int32_t, __eventPrivateMsg, 24)(int32_t subType, int32_t sendTime, int64_t fromQQ, const char *msg, int32_t font) {
	if (msg[0] == '.'&&msg[1] == 'o') {
		string strSecondInput;
		int intCustomMsg = 2;
		while (msg[intCustomMsg] == ' ')intCustomMsg++;
		while (msg[intCustomMsg] != ' '&&msg[intCustomMsg] != '\0') {
			strSecondInput += msg[intCustomMsg];
			intCustomMsg++;
		}
		if (strSecondInput == "人物作成") {
			string strMAns = to_string(fromQQ);
			strMAns += "的人物作成: ";
			strMAns += '\n';
			strMAns += "力量STR=3D6*5=";
			int STR = RollDice("3D6") * 5;
			strMAns += to_string(STR) + "/" + to_string(STR / 2) + "/" + to_string(STR / 5);
			strMAns += '\n';
			strMAns += "体质CON=3D6*5=";
			int CON = RollDice("3D6") * 5;
			strMAns += to_string(CON) + "/" + to_string(CON / 2) + "/" + to_string(CON / 5);
			strMAns += '\n';
			strMAns += "体型SIZ=3D6*5=";
			int SIZ = RollDice("3D6") * 5;
			strMAns += to_string(SIZ) + "/" + to_string(SIZ / 2) + "/" + to_string(SIZ / 5);
			strMAns += '\n';
			strMAns += "敏捷DEX=3D6*5=";
			int DEX = RollDice("3D6") * 5;
			strMAns += to_string(DEX) + "/" + to_string(DEX / 2) + "/" + to_string(DEX / 5);
			strMAns += '\n';
			strMAns += "外貌APP=3D6*5=";
			int APP = RollDice("3D6") * 5;
			strMAns += to_string(APP) + "/" + to_string(APP / 2) + "/" + to_string(APP / 5);
			strMAns += '\n';
			strMAns += "智力INT=(2D6+6)*5=";
			int INT = (RollDice("2D6") + 6) * 5;
			strMAns += to_string(INT) + "/" + to_string(INT / 2) + "/" + to_string(INT / 5);
			strMAns += '\n';
			strMAns += "意志POW=(2D6+6)*5=";
			int POW = (RollDice("2D6") + 6) * 5;
			strMAns += to_string(POW) + "/" + to_string(POW / 2) + "/" + to_string(POW / 5);
			strMAns += '\n';
			strMAns += "教育EDU=(2D6+6)*5=";
			int EDU = (RollDice("2D6") + 6) * 5;
			strMAns += to_string(EDU) + "/" + to_string(EDU / 2) + "/" + to_string(EDU / 5);
			strMAns += '\n';
			strMAns += "幸运LUCK=3D6*5=";
			int LUCK = RollDice("3D6") * 5;
			strMAns += to_string(LUCK);
			strMAns += "\n理智SAN=POW=";
			int SAN = POW;
			strMAns += to_string(SAN);
			strMAns += "\n生命值HP=(SIZ+CON)/10=";
			int HP = (SIZ + CON) / 10;
			strMAns += to_string(HP);
			strMAns += "\n魔法值MP=POW/5=";
			int MP = POW / 5;
			strMAns += to_string(MP);
			string DB;
			int build = 0;
			if (STR + SIZ >= 2 && STR + SIZ <= 64) {
				DB = "-2";
				build = -2;
			}
			else if (STR + SIZ >= 65 && STR + SIZ <= 84) {
				DB = "-1";
				build = -1;
			}
			else if (STR + SIZ >= 85 && STR + SIZ <= 124) {
				DB = "0";
				build = 0;
			}
			else if (STR + SIZ >= 125 && STR + SIZ <= 164) {
				DB = "1D4";
				build = 1;
			}
			else if (STR + SIZ >= 165 && STR + SIZ <= 204) {
				DB = "1d6";
				build = 2;
			}
			else {
				DB = "计算错误!";
				build = -10;
			}
			strMAns += "\n伤害加值DB=" + DB + "\n体格=" + (build == -10 ? "计算错误" : to_string(build));
			int MOV = 0;
			if (DEX < SIZ&&STR < SIZ)MOV = 7;
			else if (DEX > SIZ && STR > SIZ)MOV = 9;
			else MOV = 8;
			strMAns += "\n移动力MOV=" + to_string(MOV);
			CQ_sendPrivateMsg(ac, fromQQ, strMAns.c_str());
		}
		else if (strSecondInput == "六版人物作成") {
			string strMAns = to_string(fromQQ);
			strMAns += "的人物作成: ";
			strMAns += '\n';
			strMAns += "力量STR=3D6=";
			int STR = RollDice("3D6");
			strMAns += to_string(STR);
			strMAns += '\n';
			strMAns += "体质CON=3D6=";
			int CON = RollDice("3D6");
			strMAns += to_string(CON);
			strMAns += '\n';
			strMAns += "体型SIZ=3D6=";
			int SIZ = RollDice("3D6");
			strMAns += to_string(SIZ);
			strMAns += '\n';
			strMAns += "敏捷DEX=3D6=";
			int DEX = RollDice("3D6");
			strMAns += to_string(DEX);
			strMAns += '\n';
			strMAns += "外貌APP=3D6=";
			int APP = RollDice("3D6");
			strMAns += to_string(APP);
			strMAns += '\n';
			strMAns += "智力INT=2D6+6=";
			int INT = RollDice("2D6") + 6;
			strMAns += to_string(INT);
			strMAns += '\n';
			strMAns += "意志POW=2D6+6=";
			int POW = RollDice("2D6") + 6;
			strMAns += to_string(POW);
			strMAns += '\n';
			strMAns += "教育EDU=3D6+3=";
			int EDU = RollDice("3D6") + 3;
			strMAns += to_string(EDU);
			int SAN = POW * 5;
			int IDEA = INT * 5;
			int LUCK = POW * 5;
			int KNOW = EDU * 5;
			int HP = CON + SIZ;
			int MP = POW;
			strMAns += "\n理智SAN=POW*5=" + to_string(SAN) + "\n灵感IDEA=INT*5=" + to_string(IDEA) + "\n幸运LUCK=POW*5=" + to_string(LUCK) + "\n知识KNOW=EDU*5=" + to_string(KNOW);
			strMAns += "\n生命值HP=CON+SIZ=" + to_string(HP) + "\n魔法值MP=POW=" + to_string(MP);
			strMAns += "\n伤害奖励=";
			string DB;
			if (STR + SIZ >= 2 && STR + SIZ <= 12) {
				DB = "-1D6";
			}
			else if (STR + SIZ >= 13 && STR + SIZ <= 16) {
				DB = "-1D4";
			}
			else if (STR + SIZ >= 17 && STR + SIZ <= 24) {
				DB = "0";
			}
			else if (STR + SIZ >= 25 && STR + SIZ <= 32) {
				DB = "1D4";
			}
			else if (STR + SIZ >= 33 && STR + SIZ <= 40) {
				DB = "1D6";
			}
			else {
				DB = "计算错误!";
			}
			strMAns += DB;
			CQ_sendPrivateMsg(ac, fromQQ, strMAns.c_str());
		}
		else {
			int intReadMsgCnt = 2;
			int intTmp;
			string strAns;//Output string
			strAns += to_string(fromQQ);//Add the QQ number at the front(Next version:Nickname)
			strAns += "骰出了";
			strAns += ": ";

			while (msg[intReadMsgCnt] == ' ')intReadMsgCnt++;//Skip the space
			for (intTmp = intReadMsgCnt; msg[intTmp] != ' '&&msg[intTmp] != '\0'; intTmp++) {
				strAns += msg[intTmp];//Copy the Dice in the input to the output string
			}
			strAns += '=';
			string strSeparateDice;//Separate each die
			int mainsum = 0;//The result number
			string secondaryRes;//The direct result of each part of the input
			for (; msg[intReadMsgCnt] != ' '&&msg[intReadMsgCnt] != '\0'; intReadMsgCnt++) {
				if (msg[intReadMsgCnt] == '+') {
					int ttmpRes = RollDice(strSeparateDice, strAns);
					if (ttmpRes == -1) {
						CQ_sendPrivateMsg(ac, fromQQ, "输入错误!");
						return EVENT_BLOCK;
					}
					//Call the function of rolling dice
					mainsum += ttmpRes;
					secondaryRes += to_string(ttmpRes);
					secondaryRes += '+';
					strSeparateDice = "";
					//Reset
				}
				else {
					strSeparateDice += msg[intReadMsgCnt];
				}
			}
			int ttmpRes = RollDice(strSeparateDice, strAns);
			if (ttmpRes == -1) {
				CQ_sendPrivateMsg(ac, fromQQ, "输入错误!");
				return EVENT_BLOCK;
			}
			mainsum += ttmpRes;
			secondaryRes += to_string(ttmpRes);
			//Call the function of rolling dice again because of the last die
			while (msg[intReadMsgCnt] == ' ')intReadMsgCnt++;
			string strReason;//Get the reason from the third part of the input
			while (msg[intReadMsgCnt] != '\0') {
				strReason += msg[intReadMsgCnt];
				intReadMsgCnt++;
			}
			if (strReason != "") {
				strAns = strReason + strAns;
				strAns = "由于" + strAns;//Add the reason at the front of the output
			}
			strAns[strAns.length() - 1] = '=';//There will be an unnecessary "+" in the end.Use "=" to replace it.
			strAns += secondaryRes;
			strAns += '=';
			strAns += to_string(mainsum);//Merge all the results to the string "strAns"
			CQ_sendPrivateMsg(ac, fromQQ, strAns.c_str());//Change it to a C type string and output
		}
		return EVENT_BLOCK;
	}
	return EVENT_IGNORE;
}

CQEVENT(int32_t, __menuA, 0)() {
	MessageBoxA(nullptr, hlpmsg.c_str(), "Help", MB_OK+MB_ICONINFORMATION);
	return 0;
}
/*\n\t在应用程序目录下的config.ini中可以设置自定义掷骰,命令为.rd 自定义掷骰命令\n\t在config.ini中默认*/
