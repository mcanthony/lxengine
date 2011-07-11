#include <iostream>
#include <algorithm>
#include <set>
#include <vector>
#include <functional>
#include <ctime>
#include <cassert>
#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/random.hpp>

void verify (const std::vector<int>& v)
{
    std::set<int> set;
    for (auto it = v.begin(); it != v.end(); ++it)
        set.insert(*it);

    if (set.size() != v.size())
        std::cerr << "Function did not remove duplicates correctly!\n";
}

void 
remove_duplicates_erase (std::vector<int>& vec)
{
    std::sort(vec.begin(), vec.end());
    vec.erase(std::unique(vec.begin(), vec.end()), vec.end());
}

void 
remove_duplicates_set (std::vector<int>& v)
{
    std::set<int> set;
    for (auto it = v.begin(); it != v.end(); ++it)
        set.insert(*it);
    
    v.clear();
    v.reserve(set.size());
    for (auto it = set.begin(); it != set.end(); ++it)
        v.push_back(*it);
}

void 
remove_duplicates_copy_vector (std::vector<int>& v)
{
    if (!v.empty())
    {
        std::vector<int> u;
        u.reserve(v.size());

        std::sort(v.begin(), v.end());

        u.push_back(v.front());
        for (auto it = v.begin() + 1; it != v.end(); ++it)
        {
            if (*it != u.back())
                u.push_back(*it);
        }
        u.swap(v);
    }
}

void 
remove_duplicates_copy_vector2 (std::vector<int>& v)
{
    const size_t size = v.size();

    if (size != 0)
    {
        std::vector<int> u;
        u.resize(size);

        std::sort(v.begin(), v.end());

        const int* p = &v[0];
        const int* e = p + size;
        int* q = &u[0];

        *q = *p++;
        ptrdiff_t d = (e - p);
        while (d != 0)
        {
            switch (d)
            {
            default:
            case 16: if (*q != *p) { ++q; *q = *p; } ++p;
            case 15: if (*q != *p) { ++q; *q = *p; } ++p;
            case 14: if (*q != *p) { ++q; *q = *p; } ++p;
            case 13: if (*q != *p) { ++q; *q = *p; } ++p;
            case 12: if (*q != *p) { ++q; *q = *p; } ++p;
            case 11: if (*q != *p) { ++q; *q = *p; } ++p;
            case 10: if (*q != *p) { ++q; *q = *p; } ++p;
            case  9: if (*q != *p) { ++q; *q = *p; } ++p;
            case  8: if (*q != *p) { ++q; *q = *p; } ++p;
            case  7: if (*q != *p) { ++q; *q = *p; } ++p;
            case  6: if (*q != *p) { ++q; *q = *p; } ++p;
            case  5: if (*q != *p) { ++q; *q = *p; } ++p;
            case  4: if (*q != *p) { ++q; *q = *p; } ++p;
            case  3: if (*q != *p) { ++q; *q = *p; } ++p;
            case  2: if (*q != *p) { ++q; *q = *p; } ++p;
            case  1: if (*q != *p) { ++q; *q = *p; } ++p;
                break;
            }
            d = (e - p);
        }
        u.resize(q - &u[0]);
        u.swap(v);
    }
}

void 
remove_duplicates_radix (std::vector<int>& v, int exp)
{
    const int R = (1 << exp);

    std::vector<std::vector<int>> t;
    t.resize(R);
    for (auto it = v.begin(); it != v.end(); ++it)
    {
        int index = (*it) & (R - 1);
        t[index].push_back(*it);
    }
    
    std::vector<int> s;
    for (auto it = t.begin(); it != t.end(); ++it)
    {
        if (!it->empty())
            remove_duplicates_erase(*it);

        for (auto jt = it->begin(); jt != it->end(); ++jt)
            s.push_back(*jt);
    }

    remove_duplicates_erase(s);
    s.swap(v);
}

void 
remove_duplicates_radix4 (std::vector<int>& v)
{
    remove_duplicates_radix(v, 4);
}

void 
remove_duplicates_radix8 (std::vector<int>& v)
{
    remove_duplicates_radix(v, 8);
}

void 
remove_duplicates_radix12 (std::vector<int>& v)
{
    remove_duplicates_radix(v, 12);
}

void 
remove_duplicates_radix14 (std::vector<int>& v)
{
    remove_duplicates_radix(v, 14);
}

void run_test (const std::vector<int>& v, std::string name, std::function<void (std::vector<int>&)> remove_func)
{
    auto u(v);
    clock_t start = clock();
    remove_func(u);
    clock_t end = clock();
    verify(u);

    double ms = (double(end - start) * 1000.0) / double(CLOCKS_PER_SEC);
    
    std::cout << boost::format("%20s %8.1lf ms\n") % name %  ms;
}

int 
main (int argc, char** argv)
{
    for (int exp = 0; exp <= 24; exp += 4)
    {
        boost::mt19937 gen;

        const int mod = (2 << exp);

        std::vector<int> base(6 * 1024 * 1024);
        for (size_t i = 0; i < base.size(); ++i)
            base[i] = gen() % mod; 
      
        std::cout << " --- test mod = " << mod << " --- " << std::endl;
        run_test(base,  "erase",            remove_duplicates_erase);
        run_test(base,  "set",              remove_duplicates_set);
        run_test(base,  "copy_vector",      remove_duplicates_copy_vector);
        run_test(base,  "copy_vector2",     remove_duplicates_copy_vector2);
        run_test(base,  "radix4",           remove_duplicates_radix4);
        run_test(base,  "radix8",           remove_duplicates_radix8);
        run_test(base,  "radix12",          remove_duplicates_radix12);
        run_test(base,  "radix14",          remove_duplicates_radix14);
    }

    return 0;
}
