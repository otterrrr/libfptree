#include <iostream>
#include <string>
#include <sstream>
#include "../fptree.hpp"

struct titem
{
    uint16_t col;
    uint32_t value;

    operator std::string() const {
        std::stringstream ss;
        ss << col << ";" << value;
        return ss.str();
    }
};

std::ostream& operator<<(std::ostream& out, const titem& item)
{
    out << "{" << item.col << "," << item.value << "}";
    return out;
}

std::ostream& operator<<(std::ostream& out, const std::vector<titem>& items) {
    out << "[";
    for(size_t i=0; i < items.size(); ++i)
        out << items[i] << (i==items.size()-1 ? "":",");
    out << "]";
    return out;
}

int main()
{
    std::vector<std::vector<titem>> itemsets {
        { titem{0, 256}, titem{1, 128}, titem{2,64} },
        { titem{0, 8}, titem{1, 64}, titem{2, 32}},
        { titem{0, 8}, titem{1, 128}, titem{2, 16}},
        { titem{0,256}, titem{1, 64}, titem{2, 64} },
        { titem{0,16}, titem{1, 32}, titem{2, 32} },
        { titem{0, 8}, titem{1, 128}, titem{2, 64} }
    };

    kdd::fptree fptree(2, itemsets.begin(), itemsets.end());
    std::string dumpstr = fptree.dump_meta();
    if (dumpstr.find("[0;8,0,3],[1;128,1,3],[2;64,2,3],[0;256,3,2],[1;64,4,2],[2;32,5,2]") == std::string::npos ||
        dumpstr.find("[1,2,3,],[0,4,5,],[0,1,],[2,3,4,],[5,],[0,1,2,]") == std::string::npos) {
        std::cout << dumpstr << std::endl;
        return -1;
    }

    std::vector<titem> test_probe1 {titem{0, 8}, titem{1, 128}};
    std::vector<titem> test_probe2 {titem{0, 64}, titem{1, 128}};
    if(fptree.probe_frequency(test_probe1.begin(), test_probe1.end()) != 2) {
        std::cout << "probe_frequency("<< test_probe1 << ") failed" << std::endl;
        return -1;
    }
    if(fptree.probe_frequency(test_probe2.begin(), test_probe2.end()) != 0) {
        std::cout << "probe_frequency("<< test_probe2 << ") failed" << std::endl;
        return -1;
    }

    return 0;
}