#include <bits/stdc++.h>
using namespace std;

#include "Parse_spmf.h"
#include "PriorityQueue_EFIM.h"
#include "BinarySearch_EFIM.h"


int main(int argc, char** argv){


    string path ="datasets\\RNG\\random_1.txt";

    BS_EFIM<int> pq(path, 400, spmf_parse, "my_out.txt", 1);


    PQ_EFIM<int> bs(path, 400, spmf_parse, "bs_out.txt", 3);

}

