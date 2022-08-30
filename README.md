# libfptree
C++ implementation of FPtree, a part of FPGrowth algorithm (Frequent Pattern Mining)

# Currently available interface
1. fptree::probe_frequency()
    - return frequency given a single pattern

# How to use
1. declare variable of fptree<[|key-converter-typename]>(min-support, record-iterator-begin, record-iterator-end)
    - each record is a itemset
    - a itemset contains items
    - a item can be a custom struct when it provides
        - string conversion operator(operator std::string()) or
        - custom key-converter class, as a class template argument of fptree, that converts its contents into std::string
2. call probe_frequency(itemset-iterator-begin, itemset-iterator-end)

# Reference
* Han J, Pei J, Yin Y. Mining frequent patterns without candidate generation. In: ACM sigmod record. vol. 29. ACM; 2000. p. 1–12.
