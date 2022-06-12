#include "bits/stdc++.h"
using namespace std;

#include "Parse_spmf.h"
#include "BinarySearch_EFIM.h"

int main(){

    ios_base::sync_with_stdio(0), cin.tie(0);

    BS_EFIM<int> a("datasets\\fruithut.txt", 400, spmf_parse, "bs_out.txt", 3);

}