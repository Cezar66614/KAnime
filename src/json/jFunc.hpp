#pragma once

#ifndef _jFunc_H
#define _jFunc_H

#include <cstdio>
#include <cstdlib>
#include <string>

#include "./json.hpp"
using json = nlohmann::json;

void readJson(const char *, json &);
void writeJson(const char *, json);

#endif //_jFunc_H
