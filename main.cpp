#include <iostream>
#include <vector>
#include <string>
#include <random>
#include <algorithm>
#include <chrono>
#include <functional>
#include <fstream>
using namespace std;
using Clock = chrono::high_resolution_clock;
using Duration = chrono::duration<double, milli>;

class StringGenerator {
    mt19937_64 rng;
    uniform_int_distribution<int> dist_len;
    vector<char> alphabet;
public:
    StringGenerator()
            : rng(random_device{}()), dist_len(10, 200)
    {
        for(char c = 'A'; c <= 'Z'; ++c) alphabet.push_back(c);
        for(char c = 'a'; c <= 'z'; ++c) alphabet.push_back(c);
        for(char c = '0'; c <= '9'; ++c) alphabet.push_back(c);
        string extras = "!@#%:;^&*()-.";
        for(char c: extras) alphabet.push_back(c);
    }
    vector<string> random(size_t n) {
        vector<string> v(n);
        uniform_int_distribution<int> dist_alpha(0, (int)alphabet.size() - 1);
        for(size_t i=0;i<n;++i) {
            int len = dist_len(rng);
            string s;
            s.reserve(len);
            for(int j=0;j<len;++j)
                s.push_back(alphabet[dist_alpha(rng)]);
            v[i] = move(s);
        }
        return v;
    }
    vector<string> reversed_sorted(const vector<string>& src) {
        auto v = src;
        sort(v.begin(), v.end());
        reverse(v.begin(), v.end());
        return v;
    }
    vector<string> nearly_sorted(const vector<string>& src, int swaps=10) {
        auto v = src;
        sort(v.begin(), v.end());
        uniform_int_distribution<int> dist(0, (int)v.size()-1);
        for(int i=0;i<swaps; ++i) swap(v[dist(rng)], v[dist(rng)]);
        return v;
    }
};

struct CharCompareCounter {
    uint64_t count = 0;
    bool operator()(const string& a, const string& b) {
        size_t n = min(a.size(), b.size());
        for(size_t i = 0; i < n; ++i) {
            ++count;
            if (a[i] != b[i]) return a[i] < b[i];
        }
        ++count;
        return a.size() < b.size();
    }
};

class StringSortTester {
public:
    template<typename Func>
    pair<double, uint64_t> measure(const vector<string>& data, Func sortFunc, int trials=5) {
        Duration totalDur{0};
        uint64_t totalCmp = 0;
        for(int t=0; t<trials; ++t) {
            auto v = data;
            CharCompareCounter cmp;
            auto start = Clock::now();
            sortFunc(v, cmp);
            auto end = Clock::now();
            totalDur += end - start;
            totalCmp += cmp.count;
        }
        return { totalDur.count() / trials, totalCmp / trials };
    }
};

int computeLCP(const string& a, const string& b, CharCompareCounter& cmp) {
    int len = min(a.size(), b.size());
    int lcp = 0;
    while (lcp < len) {
        cmp.count++;
        if (a[lcp] != b[lcp]) break;
        lcp++;
    }
    if (lcp == len) {
        cmp.count++;
    }
    return lcp;
}

void merge(vector<string>& arr, int l, int mid, int r, CharCompareCounter& cmp) {
    vector<string> temp;
    int i = l, j = mid + 1;
    while (i <= mid && j <= r) {
        int lcp = computeLCP(arr[i], arr[j], cmp);
        int ci = (lcp < arr[i].size()) ? static_cast<unsigned char>(arr[i][lcp]) : -1;
        int cj = (lcp < arr[j].size()) ? static_cast<unsigned char>(arr[j][lcp]) : -1;
        cmp.count++;
        if (ci <= cj) {
            temp.push_back(arr[i++]);
        } else {
            temp.push_back(arr[j++]);
        }
    }
    while (i <= mid) temp.push_back(arr[i++]);
    while (j <= r) temp.push_back(arr[j++]);
    for (int k = 0; k < temp.size(); ++k) {
        arr[l + k] = temp[k];
    }
}

void mergeSort(vector<string>& arr, int l, int r, CharCompareCounter& cmp) {
    if (l >= r) return;
    int mid = l + (r - l)/2;
    mergeSort(arr, l, mid, cmp);
    mergeSort(arr, mid + 1, r, cmp);
    merge(arr, l, mid, r, cmp);
}

void stringMergeSortWrapper(vector<string>& arr, CharCompareCounter& cmp) {
    mergeSort(arr, 0, arr.size()-1, cmp);
}

int getChar(const string& s, int d, CharCompareCounter& cmp) {
    if (d < s.size()) {
        cmp.count++;
        return static_cast<unsigned char>(s[d]);
    } else {
        cmp.count++;
        return -1;
    }
}

void stringQuickSort(vector<string>& arr, int l, int r, int d, CharCompareCounter& cmp) {
    if (l >= r) return;
    int pivot = getChar(arr[l], d, cmp);
    int i = l;
    int j = r;
    int k = l;
    while (k <= j) {
        int t = getChar(arr[k], d, cmp);
        if (t < pivot) {
            swap(arr[i], arr[k]);
            i++;
            k++;
        } else if (t > pivot) {
            swap(arr[k], arr[j]);
            j--;
        } else {
            k++;
        }
    }
    stringQuickSort(arr, l, i-1, d, cmp);
    if (getChar(arr[i], d, cmp) != -1) {
        stringQuickSort(arr, i, j, d+1, cmp);
    }
    stringQuickSort(arr, j+1, r, d, cmp);
}

void stringQuickSortWrapper(vector<string>& arr, CharCompareCounter& cmp) {
    stringQuickSort(arr, 0, arr.size()-1, 0, cmp);
}

int charAt(const string& s, int d, CharCompareCounter& cmp) {
    if (d < s.size()) {
        cmp.count++;
        return static_cast<unsigned char>(s[d]);
    } else {
        cmp.count++;
        return -1;
    }
}

void msdRadixSort(vector<string>& arr, int l, int r, int d, CharCompareCounter& cmp) {
    if (l >= r) return;
    const int R = 256;
    vector<int> count(R + 2, 0);
    vector<string> aux(r - l + 1);

    for (int i = l; i <= r; ++i) {
        int c = charAt(arr[i], d, cmp) + 2;
        count[c]++;
    }

    for (int i = 0; i < R + 1; ++i) {
        count[i + 1] += count[i];
    }

    for (int i = l; i <= r; ++i) {
        int c = charAt(arr[i], d, cmp) + 1;
        aux[count[c]++] = arr[i];
    }

    for (int i = l; i <= r; ++i) {
        arr[i] = aux[i - l];
    }

    for (int i = 0; i < R; ++i) {
        int start = l + count[i];
        int end = l + count[i + 1] - 1;
        if (start < end) {
            msdRadixSort(arr, start, end, d + 1, cmp);
        }
    }
}

void msdRadixSortWrapper(vector<string>& arr, CharCompareCounter& cmp) {
    msdRadixSort(arr, 0, arr.size()-1, 0, cmp);
}

void msdRadixSortWithSwitch(vector<string>& arr, int l, int r, int d, CharCompareCounter& cmp) {
    if (l >= r) return;
    if (r - l + 1 < 74) {
        stringQuickSort(arr, l, r, d, cmp);
        return;
    }
    const int R = 256;
    vector<int> count(R + 2, 0);
    vector<string> aux(r - l + 1);

    for (int i = l; i <= r; ++i) {
        int c = charAt(arr[i], d, cmp) + 2;
        count[c]++;
    }

    for (int i = 0; i < R + 1; ++i) {
        count[i + 1] += count[i];
    }

    for (int i = l; i <= r; ++i) {
        int c = charAt(arr[i], d, cmp) + 1;
        aux[count[c]++] = arr[i];
    }

    for (int i = l; i <= r; ++i) {
        arr[i] = aux[i - l];
    }

    for (int i = 0; i < R; ++i) {
        int start = l + count[i];
        int end = l + count[i + 1] - 1;
        if (start < end) {
            msdRadixSortWithSwitch(arr, start, end, d + 1, cmp);
        }
    }
}

void msdRadixQuickSortWrapper(vector<string>& arr, CharCompareCounter& cmp) {
    msdRadixSortWithSwitch(arr, 0, arr.size()-1, 0, cmp);
}

void stdQuickSort(vector<string>& v, CharCompareCounter& cmp) {
    sort(v.begin(), v.end(), ref(cmp));
}

void stdMergeSort(vector<string>& arr, CharCompareCounter& cmp) {
    stable_sort(arr.begin(), arr.end(), ref(cmp));
}

int main() {
    StringGenerator gen;
    StringSortTester tester;
    vector<int> sizes;
    for (int n = 100; n <= 3000; n += 100) sizes.push_back(n);

    ofstream fr("randomSorted.csv");
    ofstream fe("reverseSorted.csv");
    ofstream fn("nearlySorted.csv");
    fr << "Size,Alg,Time(ms),Comparisons\n";
    fe << "Size,Alg,Time(ms),Comparisons\n";
    fn << "Size,Alg,Time(ms),Comparisons\n";

    auto base = gen.random(3000);

    for (int n: sizes) {
        vector<pair<string, vector<string>>> sets = {
                {"randomSorted",  gen.random(3000)},
                {"reverseSorted", gen.reversed_sorted(base)},
                {"nearlySorted",  gen.nearly_sorted(base)}
        };
        for (auto &ds: sets) {
            auto data = ds.second;
            data.resize(n);
            vector<pair<string, function<void(vector<string> &, CharCompareCounter &)>>> algorithms = {
                    {"stdQuickSort",   stdQuickSort},
                    {"stdMergeSort",   stdMergeSort},
                    {"strQuickSort",   stringQuickSortWrapper},
                    {"strMergeSort",   stringMergeSortWrapper},
                    {"radixSort",      msdRadixSortWrapper},
                    {"radixQuickSort", msdRadixQuickSortWrapper}
            };
            for (auto &a: algorithms) {
                auto res = tester.measure(data, a.second);
                auto &out = ds.first == "randomSorted" ? fr : ds.first == "reverseSorted" ? fe : fn;
                out << n << "," << a.first << "," << res.first << "," << res.second << "\n";
            }
        }
    }

    fr.close();
    fe.close();
    fn.close();
    return 0;
}