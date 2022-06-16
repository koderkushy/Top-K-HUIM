#include "bits/stdc++.h"
using namespace std;

#include "../TopKHighUtilityItemsetMining.hpp"

extern "C" {
    void run (int, char*, int, char*, int);
}

void run (int who, char* in, int k, char* out, int mode = 0) {

    string name = (who? "Priority Queue": "Binary Search");
    cout << "______________ " << name << " Version " << "______________\n" << endl;

    auto start = chrono::high_resolution_clock::now();

    if (who)
        PQ_EFIM<int> (in, k, spmf_parse, out, mode);
    else
        BS_EFIM<int> (in, k, spmf_parse, out, mode);

    auto stop = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::milliseconds>(stop - start).count();

    cout << "Total time consumed\t\t: " << duration << "\tms" << endl;
    cout << "___________________________________________________\n";

}