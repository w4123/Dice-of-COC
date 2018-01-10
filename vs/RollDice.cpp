#include <string>
#include <ctime>
#include <random>
#include <functional>
#include <windows.h>
#include "..\src\RollDice.h"
using namespace std;
inline int RollDice(string Dice) {
	if (Dice == "")return -1;
	int cntNNum = 0;
	for (auto i : Dice) {
		if (!isdigit(i))cntNNum++;
	}
	if (cntNNum > 2)return -1;
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
		if (DiceType <= 0)return -1;
		uniform_int_distribution<int> dis(1, DiceType);
		mt19937 gen(clock());
		//srand((unsigned)rand() + (unsigned)time(nullptr));//Generate a new key
		int sum = 0;
		for (int i = 1; i <= DiceCnt; i++) {
			int tmpRes = dis(gen);//Get the random number and add it to the output string
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
		if (DiceType <= 0)return -1;
		uniform_int_distribution<int> dis(1, DiceType);
		mt19937 gen(clock());
		//srand((unsigned)rand() + (unsigned)time(nullptr));
		int sum = 0;
		for (int i = 1; i <= DiceCnt; i++) {
			int tmpRes = dis(gen);
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
inline int RollDice(string Dice, string &strAns, char Operator = '+') {
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
		if (DiceType <= 0)return -1;
		//srand((unsigned)rand() + (unsigned)time(nullptr));//Generate a new key
		uniform_int_distribution<int> dis(1, DiceType);
		mt19937 gen(clock());
		int sum = 0;
		for (int i = 1; i <= DiceCnt; i++) {
			int tmpRes = dis(gen);//Get the random number and add it to the output string
			strAns += to_string(tmpRes);
			sum += tmpRes;
			strAns += Operator;
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
		if (DiceType <= 0)return -1;
		uniform_int_distribution<int> dis(1, DiceType);
		mt19937 gen(clock());
		//srand((unsigned)rand() + (unsigned)time(nullptr));
		int sum = 0;
		for (int i = 1; i <= DiceCnt; i++) {
			int tmpRes = dis(gen);
			strAns += to_string(tmpRes);
			sum += tmpRes;
			strAns += Operator;
		}
		return sum;

	}
	else {
		//Add a number
		strAns += Dice;
		strAns += Operator;
		int intTmpAns = 0;
		for (int Cnt = 0; Cnt != Dice.length(); Cnt++) {
			intTmpAns = intTmpAns * 10 + (Dice[Cnt] - 48);
		}//Convert the string to an integer
		return intTmpAns;
	}
}

inline int MainRoll(int &intSum, string strInput, string &strFirstOutput, string &strSecondOutput) {
	int positiveornegative = 1;
	int intSecondLocation = 0;
	int intTmpLoc = 0;
	if (strInput[0] == '-') {
		positiveornegative = -1;
		strSecondOutput += '-';
		intSecondLocation = 1;
		intTmpLoc = 1;
	}
	bool boolAddBracket = true;
	if (strInput.find("+") == string::npos && (strInput.find("-") == 0 || strInput.find("-") == string::npos)) boolAddBracket = false;
	int ttmpRes = 0;
	while (intSecondLocation != strInput.size()) {
		if (strInput[intSecondLocation] == '+' || strInput[intSecondLocation] == '-') {
			if (boolAddBracket)
				strFirstOutput += "(";
			if (positiveornegative == -1)strFirstOutput += '-';
			ttmpRes = RollDice(strInput.substr(intTmpLoc, intSecondLocation - intTmpLoc), strFirstOutput, positiveornegative == 1 ? '+' : '-');
			if (boolAddBracket) {
				strFirstOutput[strFirstOutput.size() - 1] = ')';
				strFirstOutput += "+";
			}
			else
				strFirstOutput.erase(strFirstOutput.end() - 1);
			Sleep(1);
			intSum += ttmpRes * positiveornegative;
			strSecondOutput += to_string(ttmpRes) + strInput[intSecondLocation];
			intTmpLoc = intSecondLocation + 1;
			if (ttmpRes == -1) {
				return -1;
			}
			positiveornegative = strInput[intSecondLocation] == '+' ? 1 : -1;
		}
		intSecondLocation++;
	}
	if (boolAddBracket)strFirstOutput += '(';
	if (positiveornegative == -1)strFirstOutput += '-';
	ttmpRes = RollDice(strInput.substr(intTmpLoc), strFirstOutput, positiveornegative == 1 ? '+' : '-');
	if (ttmpRes == -1) {
		return -1;
	}
	intSum += ttmpRes * positiveornegative;
	strSecondOutput += to_string(ttmpRes);
	if (boolAddBracket) {
		strFirstOutput[strFirstOutput.size() - 1] = ')';
		strFirstOutput += "=";
	}
	else
		strFirstOutput[strFirstOutput.length() - 1] = '=';
	return 0;
}
inline int MainRoll(int &intSum,string strInput) {
	int positiveornegative = 1;
	int intLocation = 0;
	int intTmpLoc = 0;
	if (strInput[0] == '-') {
		positiveornegative = -1;
		intLocation = 1;
		intTmpLoc = 1;
	}
	int ttmpRes = 0;
	while (intLocation != strInput.size()) {
		if (strInput[intLocation] == '+' || strInput[intLocation] == '-') {
			ttmpRes = RollDice(strInput.substr(intTmpLoc, intLocation - intTmpLoc));
			Sleep(1);
			intSum += ttmpRes * positiveornegative;
			intTmpLoc = intLocation + 1;
			if (ttmpRes == -1) {
				return -1;
			}
			positiveornegative = strInput[intLocation] == '+' ? 1 : -1;
		}
		intLocation++;
	}
	ttmpRes = RollDice(strInput.substr(intTmpLoc));
	if (ttmpRes == -1) {
		return -1;
	}
	intSum += ttmpRes * positiveornegative;
	return 0;
}
inline void COC6(string &strMAns) {
	strMAns += "的人物作成: ";
	strMAns += '\n';
	strMAns += "力量STR=3D6=";
	int STR = RollDice("3D6");
	Sleep(1);
	strMAns += to_string(STR);
	strMAns += '\n';
	strMAns += "体质CON=3D6=";
	int CON = RollDice("3D6");
	Sleep(1);
	strMAns += to_string(CON);
	strMAns += '\n';
	strMAns += "体型SIZ=3D6=";
	int SIZ = RollDice("3D6");
	Sleep(1);
	strMAns += to_string(SIZ);
	strMAns += '\n';
	strMAns += "敏捷DEX=3D6=";
	int DEX = RollDice("3D6");
	Sleep(1);
	strMAns += to_string(DEX);
	strMAns += '\n';
	strMAns += "外貌APP=3D6=";
	int APP = RollDice("3D6");
	Sleep(1);
	strMAns += to_string(APP);
	strMAns += '\n';
	strMAns += "智力INT=2D6+6=";
	int INT = RollDice("2D6") + 6;
	Sleep(1);
	strMAns += to_string(INT);
	strMAns += '\n';
	strMAns += "意志POW=2D6+6=";
	int POW = RollDice("2D6") + 6;
	Sleep(1);
	strMAns += to_string(POW);
	strMAns += '\n';
	strMAns += "教育EDU=3D6+3=";
	int EDU = RollDice("3D6") + 3;
	Sleep(1);
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
}
inline void COC7(string &strMAns) {
	strMAns += "的人物作成: ";
	strMAns += '\n';
	strMAns += "力量STR=3D6*5=";
	int STR = RollDice("3D6") * 5;
	Sleep(1);
	strMAns += to_string(STR) + "/" + to_string(STR / 2) + "/" + to_string(STR / 5);
	strMAns += '\n';
	strMAns += "体质CON=3D6*5=";
	int CON = RollDice("3D6") * 5;
	Sleep(1);
	strMAns += to_string(CON) + "/" + to_string(CON / 2) + "/" + to_string(CON / 5);
	strMAns += '\n';
	strMAns += "体型SIZ=(2D6+6)*5=";
	int SIZ = (RollDice("2D6") + 6) * 5;
	Sleep(1);
	strMAns += to_string(SIZ) + "/" + to_string(SIZ / 2) + "/" + to_string(SIZ / 5);
	strMAns += '\n';
	strMAns += "敏捷DEX=3D6*5=";
	int DEX = RollDice("3D6") * 5;
	Sleep(1);
	strMAns += to_string(DEX) + "/" + to_string(DEX / 2) + "/" + to_string(DEX / 5);
	strMAns += '\n';
	strMAns += "外貌APP=3D6*5=";
	int APP = RollDice("3D6") * 5;
	Sleep(1);
	strMAns += to_string(APP) + "/" + to_string(APP / 2) + "/" + to_string(APP / 5);
	strMAns += '\n';
	strMAns += "智力INT=(2D6+6)*5=";
	int INT = (RollDice("2D6") + 6) * 5;
	Sleep(1);
	strMAns += to_string(INT) + "/" + to_string(INT / 2) + "/" + to_string(INT / 5);
	strMAns += '\n';
	strMAns += "意志POW=3D6*5=";
	int POW = RollDice("3D6") * 5;
	Sleep(1);
	strMAns += to_string(POW) + "/" + to_string(POW / 2) + "/" + to_string(POW / 5);
	strMAns += '\n';
	strMAns += "教育EDU=(2D6+6)*5=";
	int EDU = (RollDice("2D6") + 6) * 5;
	Sleep(1);
	strMAns += to_string(EDU) + "/" + to_string(EDU / 2) + "/" + to_string(EDU / 5);
	strMAns += '\n';
	strMAns += "幸运LUCK=3D6*5=";
	int LUCK = RollDice("3D6") * 5;
	Sleep(1);
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
}
void DND(string &strOutput) {
	vector<int> res;
	string strDNDName[6] = { "力量","体质","敏捷","智力","感知","魅力" };
	for (int i = 0; i <= 5; i++) {
		for (int j = 1; j <= 4; j++) {
			res.push_back(RollDice("D6"));
			Sleep(1);
		}
		sort(res.begin(), res.end(), greater<int>());
		strOutput += strDNDName[i] + ":" + to_string(res[0] + res[1] + res[2]) + " ";
		res.clear();
	}
}