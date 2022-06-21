#include "bits/stdc++.h"
using namespace std;

#define all(x)              x.begin(), x.end()

auto normalise (string path) {
    vector <pair<int, vector<int>>> a;

    auto split = [](string& s, const char& d = ' ') {
        s.push_back(d);
        vector<string> a;
        for (int i = 0, last = 0; i < s.size(); i++)
            if (s[i] == d)
                a.push_back(string(s.begin()+last, s.begin()+i)), last = i+1;
        return a;
    };

    ifstream fin (path);

    if(!fin)
        cout << "Invalid file path", exit(0);

    string s;
    while (getline(fin, s)) {
        vector z = split(s);
        int u = stoi(z.back());

        vector<int> i;
        for(auto& x: z){
            if(x == "#UTIL:") break;
            i.push_back(stoi(x));
        }

        sort(all(i));
        a.push_back({u, i});
    }

    sort(all(a));

    return a;
}

int main(int argc, char** argv){
    ios_base::sync_with_stdio(0), cin.tie(0);


    if (normalise(argv[1]) == normalise(argv[2]))
        cout << "Same";
    else cout << "Not same";

}