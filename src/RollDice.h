#pragma once
#ifndef _ROLLDICE_
#define _ROLLDICE_
extern inline int MainRoll(int&, std::string, std::string&, std::string&);
extern inline int MainRoll(int&, std::string);
extern inline void COC6(std::string&);
extern inline void COC6S(std::string&, int);
extern inline void COC7(std::string&);
extern inline void COC7S(std::string&,int);
extern inline void DND(std::string&,int); 
extern inline void LongInsane(std::string&);
extern inline void TempInsane(std::string&);
extern std::string strRules;
extern inline std::string ImagePack();
#endif
