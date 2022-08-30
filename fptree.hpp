#include <string>
#include <sstream>
#include <map>
#include <vector>
#include <list>
#include <algorithm>
#include <memory>
#include <cassert>

namespace kdd
{
    struct default_stringifier
    {
        template<typename T>
        std::string operator()(const T& item) const { return item; }
    };

    template<class keyconv_t = default_stringifier>
    class fptree
    {
    public:
        using itemkey_t = std::string;
        using itemid_t = uint64_t;
        using counter_t = uint64_t;
        using itemid_counter_pair = std::pair<itemid_t, counter_t>;
        using itemdict_t = std::map<itemkey_t, itemid_counter_pair>;
        using itemdict_inv_t = std::vector<std::string>;
        using id_itemset_t = std::vector<itemid_t>;
        using id_dataset_t = std::vector<id_itemset_t>;
    private:
        struct node;
        using shared_node = std::shared_ptr<node>;
        using weak_node = std::weak_ptr<node>;
        using nodelist_t = std::list<shared_node>;
        using item_nodelist_t = std::vector<nodelist_t>;
        struct node {
            node() : id(0), cnt(0) {}
            node(itemid_t in_id, counter_t in_cnt, weak_node in_parent) : id(in_id), cnt(in_cnt), parent(in_parent) {}
            itemid_t id;
            counter_t cnt;
            std::map<itemid_t, shared_node> children;
            weak_node parent;
        };
    public:

        template<class It>
        fptree(uint32_t in_min_support,It begin, It end) : min_support(in_min_support), keyconv(keyconv_t()), root(std::make_shared<node>()) {
            
            // 1. scan to build item frequency map
            using itemcnt_t = std::map<itemkey_t, counter_t>;
            itemcnt_t itemcnt;
            size_t num_records = 0;
            for(It it = begin; it != end; ++it) {
                for(auto& item: *it) {
                    std::string itemstr = keyconv(item);
                    if(itemcnt.find(itemstr) == itemcnt.end()) {
                        itemcnt[itemstr] = 0;
                    }
                    itemcnt[itemstr] += 1;
                }
                ++num_records;
            }

            // 2. sort in order of frequecy ascending
            using item_t = std::pair<itemkey_t, counter_t>;
            using items_t = std::vector<item_t>;
            items_t items;
            std::transform(itemcnt.begin(), itemcnt.end(), std::back_inserter(items), [](const itemcnt_t::value_type& pair){ return pair; });
            itemcnt.clear(); // no more usage of itemcnt
            std::sort(items.begin(), items.end(), [](const items_t::value_type& lhs, const items_t::value_type& rhs){ return lhs.second == rhs.second ? (lhs.first > rhs.first) : (lhs.second < rhs.second); });

            // 3. filter out unsupported (less than min_support)
            items_t::iterator unsupported_end = std::lower_bound(items.begin(), items.end(), min_support, [this](const items_t::value_type& item, uint32_t value){ return item.second < value; });
            items.erase(items.begin(), unsupported_end);
            std::reverse(items.begin(), items.end());

            // 4. construct itemdict, itemstr => (id, cnt)
            itemid_t id = 0;
            for (items_t::iterator it = items.begin(); it != items.end(); ++it, ++id )
                itemdict[it->first] = std::make_pair(id,it->second);
            std::transform(items.begin(), items.end(), std::back_inserter(itemdict_inv), [](const items_t::value_type& pair){ return pair.first; });
            items.clear(); // no more usage of items

            // 5. scan to build id dataset
            id_dataset.reserve(num_records);
            for(It it = begin; it != end; ++it) {
                id_itemset_t id_itemset;
                for(auto& item: *it) {
                    std::string itemstr = keyconv(item);
                    auto find_it = itemdict.find(itemstr);
                    if (find_it == itemdict.end())
                        continue;
                    id_itemset.push_back(find_it->second.first);
                }
                id_dataset.push_back(id_itemset);
            }

            // 6. sort for each itemset in order of frequency descending
            std::for_each(id_dataset.begin(), id_dataset.end(), [](id_itemset_t& itemset){ std::sort(itemset.begin(), itemset.end()); });

            // 7. build tree & nodelist
            item_nodelist.resize(itemdict_inv.size());
            for(id_dataset_t::const_iterator it = id_dataset.begin(); it != id_dataset.end(); ++it) {
                shared_node curr_node = root;
                for(auto id: *it) {
                    // adding a new child or find the corresponding child
                    if(curr_node->children.find(id) == curr_node->children.end()) {
                        shared_node new_node = std::make_shared<node>(id,0,curr_node);
                        curr_node->children[id] = new_node;
                        item_nodelist[id].push_back(new_node);
                    }
                    curr_node = curr_node->children[id];
                    assert(curr_node->id == id);
                    ++curr_node->cnt;
                }
            }
        }

        template<class It>
        counter_t probe_frequency(It begin, It end) const
        {
            id_itemset_t target_ids;
            for(auto it = begin; it != end; ++it) {
                itemkey_t key = keyconv(*it);
                if(itemdict.find(key) == itemdict.end())
                    return 0;
                target_ids.push_back(itemdict.at(key).first);
            }
            if(target_ids.empty())
                return 0;
            std::sort(target_ids.begin(), target_ids.end());

            counter_t frequency = 0;
            const nodelist_t& bottom_nodes = item_nodelist.at(target_ids.back()); // bottom-most nodes
            for(auto it = bottom_nodes.begin(); it != bottom_nodes.end(); ++it)
            {
                const shared_node& bottom_node = *it;
                id_itemset_t search_ids{bottom_node->id};
                for(shared_node curr_node = bottom_node->parent.lock(); curr_node && curr_node != root; curr_node = curr_node->parent.lock()) {
                    search_ids.push_back(curr_node->id);
                }
                id_itemset_t inter_ids;
                std::set_intersection(target_ids.begin(), target_ids.end(), search_ids.rbegin(), search_ids.rend(), std::back_inserter(inter_ids));
                if(inter_ids.size() == target_ids.size())
                    frequency += bottom_node->cnt;
            }
            return frequency;
        }

        std::string dump_meta() const {
            std::stringstream ss;
            ss << "{" << std::endl;
            ss << "min_support: " << min_support << "," << std::endl;
            //ss << itemdict << std::endl;
            ss << "itemdict: [";
            std::for_each(itemdict_inv.cbegin(), itemdict_inv.cend(), [&ss,this](const std::string& key){ auto& pair = itemdict.at(key); ss << "[" << key << "," << pair.first << "," << pair.second << "],"; });
            ss << "]," << std::endl;
            ss << "itemdict_inv: [";
            std::for_each(itemdict_inv.cbegin(), itemdict_inv.cend(), [&ss](const std::string& s){ ss << s << ","; });
            ss << "]," << std::endl;
            ss << "id_dataset: [";
            std::for_each(id_dataset.begin(), id_dataset.end(), [&ss](const id_itemset_t& itemset){ ss << "["; std::for_each(itemset.cbegin(), itemset.cend(), [&ss](const id_itemset_t::value_type& id){ ss << id << ","; }); ss << "],"; });
            ss << "]," << std::endl;
            ss << "}";
            return ss.str();
        }
        
    private:
        itemdict_t itemdict;
        itemdict_inv_t itemdict_inv;
        const uint32_t min_support;
        const keyconv_t keyconv;
        id_dataset_t id_dataset;
        shared_node root;
        item_nodelist_t item_nodelist;
    };
}