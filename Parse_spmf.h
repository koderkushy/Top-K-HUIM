#pragma once

#define all(x)              x.begin(), x.end()


// Parses data from a text file in SPMF format to a vector of transactions

using uint = int;

auto spmf_parse = [](const string& input_file) {

	using Transaction = vector<pair<int, uint>>;
	using Dataset = vector<Transaction>;

	ifstream fin (input_file);
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

