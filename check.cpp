#include "bits/stdc++.h"
using namespace std;

typedef long long           lol;
typedef pair<int,int>       pii;
#define pb                  push_back
#define ub                  upper_bound
#define lb                  lower_bound
#define fo(i,l,r,d)         for(auto i=l; d<0?i>r:(d>0?i<r:0); i+=d)
#define all(x)              x.begin(), x.end()
#define ff                  first
#define ss                  second    

mt19937 rng (chrono::high_resolution_clock::now().time_since_epoch().count());
template <typename A, typename B> ostream& operator<< (ostream &cout, pair<A, B> const &p) { return cout << "(" << p.first << ", " << p.second << ")"; }
template <typename A, typename B> istream& operator>> (istream& cin, pair<A, B> &p) {cin >> p.first; return cin >> p.second;}
template <typename A> ostream& operator<< (ostream &cout, vector<A> const &v) {cout << "["; for(int i = 0; i < v.size(); i++) {if (i) cout << ", "; cout << v[i];} return cout << "]";}
template <typename A> istream& operator>> (istream& cin, vector<A> &x){for(int i = 0; i < x.size()-1; i++) cin >> x[i]; return cin >> x[x.size()-1];}
template <typename A, typename B> A amax (A &a, B b){ if (b > a) a = b ; return a; }
template <typename A, typename B> A amin (A &a, B b){ if (b < a) a = b ; return a; }

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
            i.pb(stoi(x));
        }

        sort(all(i));
        a.pb({u, i});
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