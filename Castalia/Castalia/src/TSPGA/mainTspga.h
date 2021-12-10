#include <cstdio>
#include <ctime>
#include <cmath>
#include <cstdlib>
#include <vector>
#include <list>
#include <algorithm>
#include "evolution_tsp.h"
#include "solution_tsp.h"
#include "problem_tsp.h"
#include "CastaliaModule.h"

using namespace std;

vector<vector<int>> callTspga(CastaliaModule &module, int sinkId, std::vector<int> landmarks, int numUAVs, double maxDelay);