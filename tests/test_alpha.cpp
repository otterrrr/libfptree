#include <iostream>
#include <string>
#include "../fptree.hpp"

std::ostream& operator<<(std::ostream& out, const std::vector<std::string>& v){
    out << "[";
    for(size_t i = 0; i < v.size(); ++i)
        out << v[i] << (i == (v.size()-1) ? "" : ",");
    out << "]";
    return out;
}

int main()
{
    std::vector<std::vector<std::string>> itemsets {
        { "a", "d", "f" },
        { "a", "c", "d", "e" },
        { "b", "d" },
        { "b", "c", "d" },
        { "b", "c" },
        { "a", "b", "d" },
        { "b", "d", "e"},
        { "b", "c", "e", "g" },
        { "c", "d", "f" },
        { "a", "b", "d" }
    };
    kdd::fptree fptree(2, itemsets.begin(), itemsets.end());
    std::string dumpstr = fptree.dump_meta();
    if (dumpstr.find("[d,0,8],[b,1,7],[c,2,5],[a,3,4],[e,4,3],[f,5,2]") == std::string::npos ||
        dumpstr.find("[0,3,5,],[0,2,3,4,],[0,1,],[0,1,2,],[1,2,],[0,1,3,],[0,1,4,],[1,2,4,],[0,2,5,],[0,1,3,]") == std::string::npos) {
        std::cout << dumpstr << std::endl;
        return -1;
    }
    std::vector<std::string> test_probe1 { "d","b" };
    std::vector<std::string> test_probe2 { "x","y" };
    if(fptree.probe_frequency(test_probe1.begin(), test_probe1.end()) != 5) {
        std::cout << "probe_frequency(" << test_probe1 << ") failed" <<  std::endl;
        return -1;
    }
    if(fptree.probe_frequency(test_probe2.begin(), test_probe2.end()) != 0) {
        std::cout << "probe_frequency(" << test_probe2 << ") failed" <<  std::endl;
        return -1;
    }
    return 0;
}