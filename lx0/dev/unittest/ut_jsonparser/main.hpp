#pragma once

#include <functional>

void check(int line, bool b, std::string source);
void check_exception (int line, bool bShouldThrow, std::string source, std::function<void()> f);

#define CHECK(b) check(__LINE__, (b), "")
#define CHECK_EXCEPTION(Code) check_exception(__LINE__, true, #Code, [&]() Code )
