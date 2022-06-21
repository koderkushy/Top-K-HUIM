#include "bits/stdc++.h"
using namespace std;

#include "TopKHighUtilityItemsetMining.hpp"

int main(int argc, char** argv){
    ios_base::sync_with_stdio(0), cin.tie(0);

    string path = argv[1];

    vector k_arr(3, vector(0, 0));

    for (int s = 10; s < 100; s += 10)
    	k_arr[0].emplace_back(s);
    for (int s = 100; s < 1000; s += 100)
    	k_arr[1].emplace_back(s);
    for (int s = 1000; s < 10000; s += 1000)
    	k_arr[2].emplace_back(s);

    vector times(2, vector(3, vector(0, 0)));

    for (int j = 0; j < 3; j++) {
    	for (int k: k_arr[j]) {
    		auto s = chrono::high_resolution_clock::now();
    		// PQ_EFIM<int>(path, k, spmf_parse);
            string command = "java -jar spmf.jar run TKU foodmart.txt ot.txt " + to_string(k);
            system(command.c_str());
    		auto e = chrono::high_resolution_clock::now();
    		times[0][j].emplace_back(chrono::duration_cast<chrono::milliseconds>(e - s).count());

    		// s = chrono::high_resolution_clock::now();
    		// BS_EFIM<int>(path, k, spmf_parse);
    		// e = chrono::high_resolution_clock::now();
    		// times[1][j].emplace_back(chrono::duration_cast<chrono::milliseconds>(e - s).count());


    	}
    }

    string opath = path;
    opath.erase(opath.end() - 4, opath.end());

    opath += "_times.txt";

    freopen(opath.c_str(), "w", stdout);

    cout << "K\n";
    for (auto v: k_arr){
    	for (auto t: v) cout << t << ' ' ; cout << '\n';
    }

    cout << "PQ\n";
    for (auto v: times[0]){
    	for (auto t: v) cout << t << ' ' ; cout << '\n';
    }
	cout << "BS\n";
    for (auto v: times[1]){
    	for (auto t: v) cout << t << ' ' ; cout << '\n';
    }

}