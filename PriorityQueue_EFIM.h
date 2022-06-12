#pragma once

#define all(x)              x.begin(), x.end()

template<class uint>
class PQ_EFIM {
public:

    using Transaction = vector<pair<int, uint>>;
    using Dataset = vector<Transaction>;

    Dataset base;
    // Stores the raw dataset after reading from input file

    const int k, mode;
    // The number of top HUIs to mine and the mode of the program

    vector<uint> twu;
    // Transaction weighted utilities of all single item itemsets

    const string opath;
    // Paths of input and output files

    int min_id = INT_MAX, 
        max_id = INT_MIN,
        offset;
    // Item ID extremes, useful in hashing by TWU order and retrieving old IDs

    enum modes {
        PRINT_STATS = 1,
        PRINT_HUIS = 2,
        ALL = 3
    };
    // Flags for program mode


    // Class constructor which also drives the algorithm

    template<class parser>
    PQ_EFIM (const string& in,
             const int k,
             parser&& parse,
             const string& out = "",
             const int mode = 0
            ): k(k), mode(mode), opath(out) {

        // Parsing dataset into base structure

        timer ();

        base = move(parse(in));

        // Computing min, max id
        min_id = accumulate(all(base), INT_MAX, [](const int& mi, const Transaction& a) { return min(mi, min_element(all(a)) -> first); });
        offset = min_id;
        max_id = accumulate(all(base), INT_MIN, [](const int& mx, const Transaction& a) { return max(mx, max_element(all(a)) -> first); });


        // Computing transaction weighted utilities (TWU) for all single item itemsets
        // and offsetting item ids

        twu.resize(max_id - min_id + 1, 0);

        for_each (all(base), [&](Transaction& x) {
            uint transaction_util{0};

            for_each (all(x), [&](const pair<int, uint>& p) {
                transaction_util += p.second;
            });

            for_each (all(x), [&](pair<int, uint>& p) {
                p.first -= offset,
                twu[p.first] += transaction_util;
            });
        });


        Bin_lu.resize(max_id - min_id + 1);
        Bin_su.resize(max_id - min_id + 1);

        vector<pair<uint, int>> order;

        for (int i = 0; i < twu.size(); i++)
            if (twu[i] > 0)
                order.emplace_back(twu[i], i);

        int n_items = order.size();


        // Sorting by TWU order and hashing item ids

        sort(all(order));
        map<int, int> new_id, old_id;

        for (int i = 0; i < order.size(); i++)
            new_id[order[i].second] = i,
            old_id[i] = order[i].second;


        // Creating a new dataset with merged items and transactions
        Dataset D;

        for (auto T: base) {

            // Reading selected items into a vector
            Transaction Y;
            for (auto& [it, ul]: T)
                if (auto j = new_id.find(it); j != new_id.end())
                    Y.emplace_back(j -> second, ul);

            // Sorting lexicographically and merging adjacent pairs
            sort(all(Y));
            Transaction X;

            int i = 0;
            while (i < Y.size()) {
                int q = 1;
                while (i+q < Y.size() and Y[i].first == Y[i+q].first)
                    Y[i].second += Y[i + q++].second;
                    X.emplace_back(Y[i]);
                i += q;
            }

            // Merging transactions
            if (D.size() and can_merge(D.back(), X))
                for (int i = 0; i < X.size(); i++)
                    D.back()[i].second += X[i].second;
            else if (X.size())
                D.emplace_back(move(X));
        };


        // Sorting the dataset by the reverse lexicographical order on transactions

        sort(all(D), [](const Transaction& x, const Transaction& y) {
            const int &nx = x.size(), &ny = y.size(), &n = min(nx, ny);
            for (int i = 1; i <= n; i++){
                const int &ix = x[nx-i].first;
                const int &iy = y[ny-i].first;
                if (ix != iy) return ix < iy;
            }
            return nx < ny;
        });

        /* Search procedure checkist
            D               - pruned, projected and merged Dataset
            min_util(&)     - minimum utility threshold
            count(&)        - count of number of HUIs encountered
            alpha(&)        - current itemset in the set enumeration tree
            util_alpha_in   - utilities of the current itemset in the transactions
            pri, sec(&)     - primary and secondary itemsets wrt alpha
            store(&)        - boolean value corresponding to whether or not to store HUIs
        */

        vector util_alpha_in(D.size(), uint());

        vector all_items(n_items, 0);
        iota(all(all_items), 0);

        timer ("Dataset preparation time:");

        timer ();
        search (D, util_alpha_in, all_items, all_items);
        timer ("Search time:"), timer ();

        // Printing the Top K High Utility Itemsets

        if (mode & modes::PRINT_HUIS) {
            ofstream fout (opath);
            if(!fout) return;

            vector<pair<uint, vector<int>>> topk;

            while(kPatterns.size())
                topk.push_back(kPatterns.top()), kPatterns.pop();

            for(auto &[util, itemset]: topk)
                for(auto &item: itemset)
                    item = old_id[item] + offset;

            for(auto &[util, itemset]: topk){
                for(int& x: itemset) fout << x << ' ';
                fout << "#UTIL: " << util << '\n';
            }
        }

    }

    // Objects used by the search procedure

    uint min_util = 1;
    vector<int> alpha;

    using pattern = pair<uint, vector<int>>;

    constexpr static auto pattern_cmp = [](const pattern& x, const pattern& y) {
        return x.first > y.first;
    };

    priority_queue<pattern, vector<pattern>, decltype(pattern_cmp)> kPatterns{pattern_cmp};

    vector<uint> Bin_su, Bin_lu;

    // Recursive search function that depth first searches the set enumeration tree
    // of itemsets and prunes using the subtree utility and local utility upperbounds
    void search (const Dataset& D, const vector<uint>& util_alpha_in,
                    const vector<int>& primary, const vector<int>& secondary) {

        // Iterating over extensions

        for (auto x: primary) {
            
            alpha.emplace_back(x);
            auto &beta = alpha;
            // itemset beta = alpha union {x}

            // Computing subtree utility and local utility upperbounds wrt extensions of beta

            for (int i = 0; i < D.size(); i++) {
                const auto& T = D[i];

                const auto& loc_x = lower_bound(all(T), pair{x, uint(0)});
                // Finding x in the transaction using binary search

                if(loc_x == T.end() or loc_x -> first != x) continue;
                // If x is not found, this transaction is useless
                
                uint lu, su;
                /*
                    Let U(x, T) = utility of itemset x in transaction T
                        E(x) = set of items that can extend x
                        re(x, T) = remaining utility of itemset x in transaction T (re(x, T) = sum {U({z}, T)} for z in E(x))                   
                */

                lu = su = util_alpha_in[i] + loc_x -> second;       
                // base value = U(alpha, T) + U(x, T)

                for(auto i = loc_x + 1; i != T.end(); i++)
                    lu += i -> second;
                // lu(beta, z) = sum {U(alpha, T) + U(x, T) + re(beta, T)}

                for(auto i = T.rbegin(); i -> first != x; i++){
                    su += i -> second;
                    Bin_lu[i -> first] += lu;
                    Bin_su[i -> first] += su;
                }
                // su(beta, z) = sum {U(alpha, T) + U(x, T) + re(beta union {z}, T)}
            }


            // Preparing primary and secondary itemsets wrt beta

            vector<int> primary_beta, secondary_beta;

            for (auto& i: secondary) {
                if (Bin_su[i] >= min_util)
                    primary_beta.emplace_back(i);
                if (Bin_lu[i] >= min_util)
                    secondary_beta.emplace_back(i);
            }

            // Preparing projected dataset beta-D

            Dataset beta_D;
            vector<uint> beta_utils;
            uint beta_util = 0;

            for (int i = 0; i < D.size(); i++) {
                const auto& T = D[i];
                const auto& loc_x = lower_bound(all(T), pair{x, uint(0)});
                // Finding x in the transaction using binary search

                if(loc_x == T.end() or loc_x -> first != x) continue;
                // If x is not found, this transaction is useless

                uint util_beta_in_T = util_alpha_in[i] + loc_x -> second;
                // Utility of itemset beta in this transaction

                beta_util += util_beta_in_T;
                // Adding it to utility of beta

                Transaction X;

                for (auto i = loc_x + 1; i != T.end(); i++)
                    if (Bin_lu[i -> first] >= min_util)
                        X.emplace_back(*i);
                // Pruning items using local utility upperbound
                // lu(beta union {z}) >= min_util

                // Merging transactions
                if (beta_D.size() and can_merge(beta_D.back(), X)) {
                    for (int i = 0; i < X.size(); i++)
                        beta_D.back()[i].second += X[i].second;

                    beta_utils.back() += util_beta_in_T;
                }
                else if (X.size()) {
                    beta_D.emplace_back(move(X));
                    beta_utils.emplace_back(util_beta_in_T);
                }
            }

            // Resetting the bin arrays

            for (auto& i: secondary)
                Bin_su[i] = Bin_lu[i] = 0;

            // Counting in HUIs

            if (kPatterns.size() < k or kPatterns.top().first < beta_util)
                kPatterns.push({beta_util, beta});
            if (kPatterns.size() > k)
                kPatterns.pop();
            if (kPatterns.size() == k)
                min_util = max(min_util, kPatterns.top().first);


            // If there are more items to explore in the dataset, recursively call search
            if(beta_D.size())
                search(beta_D, beta_utils, primary_beta, secondary_beta);

            alpha.pop_back();       // alpha = beta - {x}
        }
    }


    // Checks whether two transactions can be merged

    bool can_merge (const Transaction& a, const Transaction& b) {
        if (a.size() != b.size())
            return false;
        auto p = [](const pair<int, uint>& x, const pair<int, uint>& y) { 
            return x.first == y.first;
        };
        return mismatch (all(a), all(b), p).first == a.end();
    }


    // Maintains time interval between successive calls

    void timer (string&& message = "") {
        if (!(mode & modes::PRINT_STATS)) return;

        static auto start = chrono::high_resolution_clock::now();
        static bool mode = false;

        if(!mode) start = chrono::high_resolution_clock::now(), mode = true;
        else {
            auto stop = chrono::high_resolution_clock::now();
            auto duration = chrono::duration_cast<chrono::milliseconds>(stop - start);
            cout << message << duration.count() << "ms\n" << flush;
            mode = false;
        }
    }

};