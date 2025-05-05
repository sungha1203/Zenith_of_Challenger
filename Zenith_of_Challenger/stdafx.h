#pragma once
#include <WS2tcpip.h>
#include <MSWSock.h>
#include <iostream>
#include <algorithm>
#include <thread>
#include <mutex>
#include <unordered_map>		
#include <map>
#include <fstream>				
#include <sstream>				
#include <vector>
#include <array>
#include <random>
#include "network.h"
#include "protocol.h"
#include "ClientManager.h"
#include "atomic"
#include "thread"

#pragma comment(lib, "WS2_32.lib")
#pragma comment(lib, "MSWSock.lib")

extern Network		g_network;
extern SOCKET		g_ListenSocket;
extern OVER_EXP		g_a_over;