#pragma once

#include <cstring>
#include <string>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cstdio>
#include <filesystem>
#include <ctime>
#include <algorithm>
#include <vector>
#include <map>
#include <chrono>
#include <thread>

#include "requests.h"

#include <nlohmann/json.hpp>


bool Loader(const std::string FName, const nlohmann::json& json_config);


extern "C"
{
	bool onApiInput(const char* FName, const char* config);
}
