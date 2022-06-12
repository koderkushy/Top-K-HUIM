#include <bits/stdc++.h>
#include <stdlib.h>

using namespace std;

#include "Parse_spmf.h"
#include "PriorityQueue_EFIM.h"
#include "BinarySearch_EFIM.h"


void perform (int k) {



}

int time_since_last () {
    static auto last = chrono::high_resolution_clock::now();

    auto now = chrono::high_resolution_clock::now();
    int dur = chrono::duration_cast<chrono::milliseconds>(now - last).count();
    last = now;

    return dur;
}

int main(){


    string path ="datasets\\RNG\\random_1.txt";

    BS_EFIM<int> pq(path, 400, spmf_parse, "my_out.txt", 1);


    PQ_EFIM<int> bs(path, 400, spmf_parse, "bs_out.txt", 3);

}

