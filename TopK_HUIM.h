#pragma once

#define all(x)              x.begin(), x.end()

#include "Metadata.h"

template<class U>
struct PQ_EFIM_Map : Metadata<U> {

    using Transaction = vector<pair<int, U>>;
    using Dataset = vector<Transaction>;
    using Metadata = Metadata<U>;

    using Metadata::D, Metadata::n_items, Metadata::Metadata_init;

    const int k;
    U min_util = 1;
    vector<int> alpha;

    using pattern = pair<U, vector<int>>;

    constexpr static auto pattern_cmp = [](const pattern& x, const pattern& y) {
        return x.first > y.first;
    };
  
    priority_queue<pattern, vector<pattern>, decltype(pattern_cmp)> kPatterns{pattern_cmp};

    template<class Parser>
    PQ_EFIM_Map (const string& inpath,
              const int k,
              Parser&& parse,
              const string& outpath = "",
              const bool PRINT_STATS = false,
              const bool DEBUG_DATASET = false,
              const bool DEBUG_SEARCH = false
            ): k(k) {

        if (PRINT_STATS) timer ();
        Metadata_init (parse, inpath);
        if (PRINT_STATS) timer ("Dataset preparation time: ");

        if (DEBUG_DATASET) {
            for (auto& T: D) {
                for (auto& [it, ul]: T)
                    cout << "(" << it << ", " << ul << ") ";
                cout << "\n";
            }
        }

        if (k <= n_items) {
            if(PRINT_STATS) timer ();
            RIU(k, min_util);
            if(PRINT_STATS) timer ("RIU strategy time: ");
        }

        vector itemset(n_items, 0);
        iota(all(itemset), 0);

        set primary(all(itemset)), secondary(all(itemset));

        if(PRINT_STATS) timer ();
        search(D, vector(D.size(), U(0)), primary, secondary, DEBUG_SEARCH);
        if(PRINT_STATS) timer ("Search time: ");

        if(outpath.size()) write_patterns(outpath);

    }

    using Metadata::item_id_offset, Metadata::old_item_id;

    void write_patterns (const string& outpath) {
        ofstream fout (outpath);
        if(!fout) return;

        vector<pair<U, vector<int>>> topk;

        while(kPatterns.size())
            topk.push_back(kPatterns.top()), kPatterns.pop();

        for(auto &[util, itemset]: topk)
            for(auto &item: itemset)
                item = old_item_id[item] + item_id_offset;


        for(auto &[util, itemset]: topk){
            for(int& x: itemset) fout << x << ' ';
            fout << "#UTIL: " << util << '\n';
        }

    }

    void search (const Dataset& D,
                 const vector<U>& alpha_utils,
                 set<int>& primary,
                 set<int>& secondary,
                 const bool DEBUG_SEARCH = false) {

        if (DEBUG_SEARCH) {
            cout << "at ";
            for (auto& it: alpha)
                cout << it << ' ';
            cout << "\n";
            for (auto& T: D) {
                for (auto& [it, ul]: T)
                    cout << "(" << it << ", " << ul << ") ";
                cout << "\n";
            }
        }

        map<int, U> utility_map;
        // U utility_map[n_items]{0};

        for (int i = 0; i < D.size(); i++) {
            for (auto& [item, util]: D[i])
                if (primary.count(item))
                    utility_map[item] += alpha_utils[i] + util;
        }

        for(auto& [x, beta_util]: utility_map) {
            
            alpha.push_back(x);
            auto &beta = alpha;

            if(kPatterns.size() < k or kPatterns.top().first < beta_util)
                kPatterns.push({beta_util, beta});
            if(kPatterns.size() > k)
                kPatterns.pop();
            if(kPatterns.size() == k)
                min_util = max(min_util, kPatterns.top().first);


            // Subtree utility and local utility upperbounds wrt extensions of beta
            unordered_map<int, U> beta_su, beta_lu;
            // U beta_su[n_items]{0}, beta_lu[n_items]{0};

            for (int i = 0; i < D.size(); i++) {
                const auto& Transaction = D[i];
                const auto& loc_x = lower_bound(all(Transaction), pair{x, U(0)});
                
                if(loc_x == Transaction.end() or loc_x -> first != x) continue;
                
                U lu, su;
                lu = su = alpha_utils[i] + loc_x -> second;

                for(auto i = loc_x + 1; i != Transaction.end(); i++)
                    lu += i -> second;
                for(auto i = Transaction.rbegin(); i -> first != x; i++)
                    beta_lu[i -> first] += lu,
                    beta_su[i -> first] += (su += i -> second);
            }

            primary.clear();
            secondary.clear();

            for (auto [item, su]: beta_su)
                if (su >= min_util)
                    primary.insert(item);
            for (auto [item, lu]: beta_lu)
                if (lu >= min_util)
                    secondary.insert(item);

            Dataset beta_D;
            vector<U> beta_utils;

            // Projection

            for (int i = 0; i < D.size(); i++) {
                Transaction beta_T;
                bool found = 0;
                U util_component = alpha_utils[i];

                for (auto& [item, util]: D[i])
                    if (item == x)
                        found = true, util_component += util;
                    else if (found and secondary.count(item))
                        beta_T.push_back({item, util});

                // Merging
                if(beta_D.size() and beta_D.back() == beta_T){
                    for (int i{0}; i < beta_T.size(); i++)
                        beta_D.back()[i].second += beta_T[i].second;
                    beta_utils.back() += util_component;
                }
                else if(beta_T.size())
                    beta_D.push_back(beta_T),
                    beta_utils.push_back(util_component);
            }

            if (DEBUG_SEARCH) {
                cout << "check ";
                for (auto& it: beta)
                    cout << it << ' ';
                cout << "\n";
                for (auto& T: beta_D) {
                    for (auto& [it, ul]: T)
                        cout << "(" << it << ", " << ul << ") ";
                    cout << "\n";
                }
            }

            if(beta_D.size())
                search(beta_D, beta_utils, primary, secondary, DEBUG_SEARCH);

            alpha.pop_back();
        }
    }

    void timer (string&& message = "") {

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

    void RIU (const int& k, U& min_util) {

        vector single_item_utility(Metadata::n_items, U());

        for_each (all(Metadata::D), [&](const Transaction& x) {
            for_each (all(x), [&](const pair<int, U>& p) {
                single_item_utility[p.first] += p.second;
            });
        });

        sort(all(single_item_utility));

        min_util = max(min_util, single_item_utility[Metadata::n_items - k]);
    }
};