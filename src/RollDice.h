#pragma once
#ifndef _ROLLDICE_
#define _ROLLDICE_
#include <string>
extern inline int MainRoll(int&, std::string, std::string&, std::string&);
extern inline int MainRoll(int&, std::string);
extern inline void COC6(std::string&);
extern inline void COC7(std::string&);
extern inline void DND(std::string&);
#endif