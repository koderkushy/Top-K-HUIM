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


    template<class A, class... Args>
    void log_if_stats (A message, Args... args) {
        if (mode & modes::PRINT_STATS)
            cout << message, log_if_stats(args...);
    }

    void log_if_stats () {}


    uint32_t numberofitemsets = 0, numberofcandidates = 0;

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

        // Computing transaction weighted utilities (TWU) for all single item itemsets

        map<int, uint> twu;

        for_each (all(base), [&](Transaction& x) {
            uint transaction_util{0};

            for_each (all(x), [&](const pair<int, uint>& p) {
                transaction_util += p.second;
            });

            for_each (all(x), [&](pair<int, uint>& p) {
                twu[p.first] += transaction_util;
            });
        });

        int n_items = twu.size();

        vector<pair<uint, int>> order;

        for (auto &[it, twu_it]: twu)
            order.emplace_back(twu_it, it);

        // Sorting by TWU order and hashing item ids

        sort(all(order));
        map<int, int> new_id;
        vector<int> old_id(n_items);

        for (int i = 0; i < order.size(); i++)
            new_id[order[i].second] = i,
            old_id[i] = order[i].second;

        Bin_lu.resize(n_items);
        Bin_su.resize(n_items);


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

        timer ("Dataset preparation time\t: ");

        timer ();
        search (D, util_alpha_in, all_items, all_items);
        timer ("Search time\t\t\t: "), timer ();

        log_if_stats("Number of itemsets visited\t: ", numberofitemsets, '\n');
        log_if_stats("Number of candidates added\t: ", numberofcandidates, '\n');

        // Printing the Top K High Utility Itemsets

        if (mode & modes::PRINT_HUIS) {
            ofstream fout (opath);
            if(!fout) return;

            vector<pair<uint, vector<int>>> topk;

            while(kPatterns.size())
                topk.push_back(kPatterns.top()), kPatterns.pop();

            for(auto &[util, itemset]: topk)
                for(auto &item: itemset)
                    item = old_id[item];

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

        numberofitemsets += primary.size();
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
                kPatterns.push({beta_util, beta}), numberofcandidates++;
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
            cout << message << duration.count() << "\tms\n" << flush;
            mode = false;
        }
    }
};

template<class uint>
class BS_EFIM {
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

    int n_actual_items;
    // Number of distinct items in the transactional database

    enum modes {
        PRINT_STATS = 1,
        PRINT_HUIS = 2,
        ALL = 3
    };
    // Flags for program mode

    template<class A, class... Args>
    void log_if_stats (A message, Args... args) {
        if (mode & modes::PRINT_STATS)
            cout << message, log_if_stats(args...);
    }

    void log_if_stats () {}


    map<int, int> outer_hash_id;
    vector<int> undo_outer_hash;
    // To compress stupidly large item ids


    template<class parser>
    void prepare_base (const string& in, parser&& parse) {

        // Read the data into base

        base = move(parse(in));

        // Compressing item ids

        for_each (all(base), [&](const Transaction& T) {
            for_each (all(T), [&](const pair<int, uint>& x) {
                outer_hash_id[x.first] = 1;
            });
        });

        int hashvalue= 0;

        for (auto &[it, hv]: outer_hash_id)
            hv = hashvalue++,
            undo_outer_hash.emplace_back(it);

        for_each (all(base), [&](Transaction& T) {
            for_each (all(T), [&](pair<int, uint>& x) {
                x.first = outer_hash_id[x.first];
            });
        });

        n_actual_items = outer_hash_id.size();

        // Computing transaction weighted utilities (TWU) for all single item itemsets
        // and offsetting item ids

        twu.resize(n_actual_items, 0);

        for_each (all(base), [&](Transaction& x) {
            uint transaction_util{0};

            for_each (all(x), [&](const pair<int, uint>& p) {
                transaction_util += p.second;
            });

            for_each (all(x), [&](pair<int, uint>& p) {
                twu[p.first] += transaction_util;
            });
        });

        Bin_lu.resize(n_actual_items);
        Bin_su.resize(n_actual_items);
    }


    uint highest_utility_estimate () {

        // Returns highest subtree utility among single item itemsets
        // (Need a better estimate...)
        vector subtree_utility(n_actual_items, uint(0));

        for_each (all(base), [&](const Transaction& T) {
            uint su = 0;
            for (auto i = T.rbegin(); i != T.rend(); i++)
                su += i -> second,
                subtree_utility[i -> first] += i -> second;
        });

        return *max_element(all(subtree_utility));
    }


    // Class constructor which also drives the algorithm

    template<class parser>
    BS_EFIM (const string& in,
             const int k,
             parser&& parse,
             const string& out = "",
             const int mode = 0
            ): k(k), mode(mode), opath(out) {

        // Parsing dataset into base structure

        timer ();
        prepare_base (in, parse);
        timer ("Base DB preparation time\t: ");

        upto = k + 7000;

        // Main binary search procedure

        uint l = 0, r = highest_utility_estimate();
        // [l, r) -> current range of interesting values of min_util

        while (l < r-1) {

            uint m = l + (r - l) / 2;

            log_if_stats("Counting for min_util\t\t: ", m, '\n');
            log_if_stats("With cap on count\t\t: ", upto, '\n');

            count_huis(m, upto);
            // Counting number of itemsets with utility >= min_util

            log_if_stats("Found ", count, " itemsets, ");

            // Usual binary search transitions
            if (count < k)
                r = m,
                log_if_stats("too few..\n\n");
            else {
                l = m;
                if (count < upto){
                    log_if_stats("just about right.\n\nRetrieving all these itemsets\n\n");
                    break;
                }else 
                    log_if_stats("too many..\n\n");
                // If the number of itemsets found is close enough, we break out of the binary search
                // This turns out to be a powerful optimization since the cost of each iteration is very high
                // and further increases as min_util attains smaller values where there are more itemsets to explore
            }           
        }

        // Collecting the final set of itemsets and preparing the output.

        store = true;
        count_huis(l);

    }


    // Prepares the pruned dataset and calls the recursive search procedure to mine HUIs

    void count_huis (const uint& min_util, const int& upto = -1) {

        timer ();

        vector<pair<uint, int>> order;

        for (int i = 0; i < twu.size(); i++)
            if (twu[i] >= min_util)
                // Pruning items by the TWU upperbound property
                order.emplace_back(twu[i], i);

        int n_items = order.size();

        // Sorting by TWU order and hashing item ids

        sort(all(order));
        map<int, int> new_id;
        vector<int> old_id(n_items);

        for (int i = 0; i < order.size(); i++)
            new_id[order[i].second] = i,
            old_id[i] = order[i].second;


        // Creating pruned dataset D

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
                    Y[i].second += Y[i + q].second, q++;
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
            upto(&)         - break search if too many HUIs were encountered
            count(&)        - count of number of HUIs encountered
            alpha(&)        - current itemset in the set enumeration tree
            util_alpha_in   - utilities of the current itemset in the transactions
            pri, sec        - primary and secondary itemsets wrt alpha
            store(&)        - boolean value corresponding to whether or not to store HUIs
        */

        BS_EFIM::min_util = min_util;
        BS_EFIM::upto = upto;
        count = 0;

        alpha.clear();

        vector util_alpha_in(D.size(), uint(0));

        vector secondary(n_items, 0);
        iota(all(secondary), 0);

        // Computing subtree utilities of single items to find the primary set of items
        vector subtree_utility(n_items, uint(0));

        for_each (all(D), [&](const Transaction& T) {
            uint su = 0;

            for (auto i = T.rbegin(); i != T.rend(); i++)
                su += i -> second,
                subtree_utility[i -> first] += su;
        });

        vector<int> primary;
        for (int i = 0; i < n_items; i++)
            if (subtree_utility[i] >= min_util)
                primary.emplace_back(i);


        timer ("Search preparation time\t\t: ");

        timer ();

        search (D, util_alpha_in, primary, secondary);
        
        timer ("Search time\t\t\t: ");

        // Printing the Top K High Utility Itemsets

        if (store and (mode & modes::PRINT_HUIS)) {         
            ofstream fout (opath);
            if(!fout) cout << "Invalid output file name\n";
            else {

                // Sorting itemsets by utility
                sort(kPatterns.rbegin(), kPatterns.rend());
                kPatterns.resize(k);

                // Retrieving actual item IDs
                for(auto &[util, itemset]: kPatterns)
                    for(auto &item: itemset)
                        item = old_id[item],
                        item = undo_outer_hash[item];

                // Printing in SPMF format
                for(auto &[util, itemset]: kPatterns){
                    for(int& x: itemset) fout << x << ' ';
                    fout << "#UTIL: " << util << '\n';
                }
            }
        }

    }

    // Data used by the search procedure
    uint min_util;
    int upto;
    int count;
    vector<int> alpha;
    set<int> pri, sec;
    bool store = false;

    using pattern = pair<uint, vector<int>>;
    vector<pattern> kPatterns;

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
                // Primary(beta) is the set of items that satisfy subtree_utility(beta U {z}) >= min_util
                if (Bin_lu[i] >= min_util)
                    secondary_beta.emplace_back(i);
                // Secondary(beta) is the set of items that satisfy local_utility(beta U {z}) >= min_util
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

            if (beta_util >= min_util) {
                count++;
                if (store)
                    kPatterns.emplace_back(beta_util, beta);

                if (!store and count > upto) return;
                // too many HUIs were found, breaking search
            }

            if(beta_D.size()){
                search(beta_D, beta_utils, primary_beta, secondary_beta);
                // If there are more items to explore in the dataset, recursively call search

                if (!store and count > upto) return;
                // too many HUIs were found, breaking search
            }

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
            cout << message << duration.count() << "\tms\n" << flush;
            mode = false;
        }
    }
};

auto spmf_parse = [](const string& input_file) {

    using uint = int;
    using Transaction = vector<pair<int, uint>>;
    using Dataset = vector<Transaction>;

    ifstream fin (input_file);

    if (!fin) {
        cout << "Invalid input file path\n";
        exit(0);
    }

    string line;

    auto split = [](string& s, const char& d = ' ') {
        s.push_back(d);
        vector<string> a;
        for (int i = 0, last = 0; i < s.size(); i++)
            if (s[i] == d)
                a.push_back(string(s.begin()+last, s.begin()+i)), last = i+1;
        return a;
    };

    Dataset D;

    while(getline(fin, line)) {
        vector block{split(line, ':')},
               items{split(block[0])},
               utils{split(block[2])};

        Transaction T;

        for (int i = 0; i < items.size(); i++) {
            const int& it = stoi(items[i]);
            const uint& ul = stoll(utils[i]);
            T.push_back(pair{it, ul});
        }

        if(T.size()) D.push_back(T);
    }

    return D;
};
