#include <iostream>
#include <sstream>
#include <algorithm>
#include <list>
#include <sys/time.h>
#include "boxes_goods_checker.h"
using namespace std;

long long time()
{
    timeval tv;
    gettimeofday(&tv, NULL);
    return (long long)(tv.tv_sec%10000)*1e6 + tv.tv_usec; // microsecond
}

Checker::Checker(): m_simple_check(true), m_boxes_num(0), m_goods_num(0)
{
}

Checker::Checker(bool simple_check): m_simple_check(simple_check), m_boxes_num(0), m_goods_num(0)
{
}

void Checker::init(vector<int> &boxes, vector<int> &goods)
{
    m_boxes_num = boxes.size();
    m_goods_num = goods.size();
    m_boxes_size.swap(boxes);
    m_goods_size.swap(goods);
    if (!m_result.empty()) m_result.clear();
    m_result.resize(m_boxes_num, 0);
}

void Checker::run()
{
    cout << "\nRunning...\n";
    if (m_goods_num > 64) {
        cout << "WARNING: currenly only support 64 goods.\n";
        return;
    }
    long long time0 = time();
    bool enough = false;
    if (m_goods_size.empty()) {
        cout << "No goods, nothing to do ...\n";
        return;
    } else if (!m_boxes_size.empty()) {
        long long time1 = time();
        long long time2 = time1;
        sort(m_boxes_size.begin(), m_boxes_size.end());
        sort(m_goods_size.begin(), m_goods_size.end());
        cout << "Sorted boxes: " << size_to_string(m_boxes_size) << "\n";
        cout << "Sorted goods: " << size_to_string(m_goods_size) << "\n";
        if (m_simple_check) {
            cout << "=============== Simple check ================\n";
            enough = doSimpleCheckRule1() ||
                     doSimpleCheckRule2() ||
                     doSimpleCheckRule3();
            time2 = time();
            cout << "Finished, duration: " << time2 - time1 << " us\n";
        }
        if (!enough) {
            cout << "=============== Complex check ================\n";
            enough = doComplexCheck();
            uint64_t included_goods = 0;
            for (int box_id = 0; box_id < m_boxes_num; ++box_id)
            {
                cout << "- for box " << box_id << ", size " << m_boxes_size[box_id]
                     << ": " << comb_to_string(m_result[box_id]) << "\n";
                included_goods |= m_result[box_id];
            }
            uint64_t remained_goods = ~included_goods;
            cout << "- remained goods: " << comb_to_string(remained_goods) << "\n";
            long long time3 = time();
            cout << "Finished, duration: " << time3 - time2 << " us\n";
        }
    }
    if (enough) {
        cout << "\nAll goods can be put into the given boxes.\n";
    } else {
        cout << "\nThe goods cannot be put into the given boxes.\n";
    }
    long long time4 = time();
    cout << "Finished all, duration: " << time4 - time0 << " us\n";
}

// target: current box size.
// current: sum of size of goods which have been put into current box.
// good_id: current good index.
// goods_comb: record one goods combination, index of bit stands for that of good.
// goods_combs: record all goods combinations of current box.
void Checker::findGoodCombPerBox(int target, int &current, int good_id, uint64_t &goods_comb, vector<uint64_t> &goods_combs)
{
    if (good_id >= m_goods_num) return;
    current += m_goods_size[good_id];
    goods_comb |= uint64_t(1) << good_id;
    if (current == target) {
        // it must be a combination.
        goods_combs.push_back(goods_comb);
    } else if (current < target) {
        // it can be a combination if
        // 1. no remained goods (all goods are in this combination);
        // or
        // 2. the min size of remained goods cannot be put into current box.
        int min_remain = m_goods_num;
        for (int i = 0; i < m_goods_num; ++i)
        {
            if (!(goods_comb & (uint64_t(1) << i))) {
                min_remain = i;
                break;
            }
        }
        if (min_remain >= m_goods_num ||
            current + m_goods_size[min_remain] > target) {
            goods_combs.push_back(goods_comb);
        }
    }
    // forward trace
    findGoodCombPerBox(target, current, good_id + 1, goods_comb, goods_combs);

    // back trace
    current -= m_goods_size[good_id];
    goods_comb ^= uint64_t(1) << good_id;
    findGoodCombPerBox(target, current, good_id + 1, goods_comb, goods_combs);
}

// current: record the goods which have been put into boxes.
// goods_combs: record all goods combinations of current box.
uint64_t Checker::getBestGoodCombPerBox(uint64_t &current, const vector<uint64_t> &goods_combs)
{
    int max_good_id = 0;
    int max_size = 0;
    int best_id = -1;
    uint64_t best_choice = 0;
    //cout << "- initial handled goods: " << comb_to_string(current) << "\n";
    for (int i = 0; i < goods_combs.size(); ++i)
    {
        uint64_t choice = goods_combs[i];
        uint64_t filtered_choice = 0;
        int size = 0;
        for (int good_id = 0; good_id < m_goods_num; ++good_id)
        {
            // only calculate the sum of size of remained goods.
            if (!(current & uint64_t(1) << good_id) &&
                (choice & uint64_t(1) << good_id)) {
                filtered_choice |= uint64_t(1) << good_id;
                size += m_goods_size[good_id];
            }
        }
        // 'best' means:
        // 1. the sum of size of remained goods is largest.
        // for sorted boxes, the combination with smaller sum of size must be included in next box.
        bool update = size > max_size;
        if (!update && size == max_size) {
            // 2. if the value of sum is same, compare the good size descending,
            // select the one containing larger goods.
            uint64_t diff = best_choice ^ filtered_choice;
            for (int good_id = m_goods_num - 1; good_id >= 0; --good_id)
            {
                if (diff & uint64_t(1) << good_id) {
                    update = filtered_choice & uint64_t(1) << good_id;
                    break;
                }
            }
        }
        if (update) {
            max_size = size;
            best_id = i;
            best_choice = filtered_choice;
        }
    }
    if (best_id >= 0) {
        uint64_t best_choice = goods_combs[best_id];
        current |= best_choice;
        //cout << "- best choice: " << comb_to_string(best_choice) << "\n";
    }
    //cout << "- final handled goods " << comb_to_string(current) << "\n";
    return best_choice;
}

bool Checker::doComplexCheck()
{
    cout << "Find all goods combs for each box, select the best ones to see if they can cover all goods.\n";
    // Step 1: get all goods combination for each box.
    vector<vector<uint64_t > > box_goods_combs(m_boxes_num);
    uint64_t goods_comb = 0;
    for (int i = 0; i < m_boxes_num; ++i)
    {
        int current = 0;
        box_goods_combs[i].reserve(256);
        findGoodCombPerBox(m_boxes_size[i], current, 0, goods_comb, box_goods_combs[i]);
    }
    //for (int i = 0; i < m_boxes_num; ++i)
    //{
    //    vector<uint64_t> &goods_combs = box_goods_combs[i];
    //    cout << "For box " << i << ", size " << m_boxes_size[i]
    //         << ", has" << goods_combs.size() << " combs:\n";
    //    for (int j = 0; j < goods_combs.size(); ++j)
    //    {
    //        uint64_t goods_comb = goods_combs[j];
    //        cout << "- comb: " << comb_to_string(goods_comb) << "\n";
    //    }
    //}

    // Step 2: select best combination of each box,
    // and calculate the goods which have been put into all boxes.
    bool ret = false;
    uint64_t curr_comb = 0;
    uint64_t full_comb = 0;
    for (int good_id = 0; good_id < m_goods_num; ++good_id)
    {
        full_comb |= uint64_t(1) << good_id;
    }
    for (int box_id = 0; box_id < m_boxes_num; ++box_id)
    {
        //cout << "For box " << box_id << ", size " << m_boxes_size[box_id] << "\n";
        const vector<uint64_t> &goods_combs = box_goods_combs[box_id];
        m_result[box_id] = getBestGoodCombPerBox(curr_comb, goods_combs);
        if (curr_comb == full_comb) {
            ret = true;
            break;
        }
    }
    if (ret) {
        cout << "- successful!\n";
    } else {
        cout << "- failed!\n";
    }
    return ret;
}

bool Checker::doSimpleCheckRule1()
{
    cout << "Rule1: loop boxes ascending, loop goods ascending, ";
    cout << "smaller good is prior to be put into smaller box.\n";
    vector<int>::const_iterator bit = m_boxes_size.begin();
    vector<int>::const_iterator git = m_goods_size.begin();
    while (bit != m_boxes_size.end())
    {
        float remain_sz = *bit;
        int good_id = 0;
        while (git != m_goods_size.end())
        {
            if (remain_sz < *git) {
                break;
            } else {
                remain_sz -= *git;
                git++;
            }
            good_id++;
        }
        bit++;
    }
    if (git == m_goods_size.end()) {
        cout << "- successful!\n";
        return true;
    }
    cout << "- failed!\n";
    return false;
}

bool Checker::doSimpleCheckRule2()
{
    cout << "Rule2: loop boxes descending, loop goods descending, ";
    cout << "larger good is prior to be put into larger box.\n";
    vector<int>::reverse_iterator bit = m_boxes_size.rbegin();
    list<int> temp_goods;
    temp_goods.assign(m_goods_size.begin(), m_goods_size.end());
    list<int>::reverse_iterator git = temp_goods.rbegin();
    while (bit != m_boxes_size.rend())
    {
        int remain_sz = *bit;
        git = temp_goods.rbegin();
        while (git != temp_goods.rend())
        {
            if (remain_sz < *git) {
                break;
            } else {
                remain_sz -= *git;
                // if the goods can be put into this box,
                // remove it from the list.
                git = list<int>::reverse_iterator(temp_goods.erase((++git).base()));
            }
        }
        bit++;
    }
    if (temp_goods.empty()) {
        cout << "- successful!\n";
        return true;
    }
    cout << "- failed!\n";
    return false;
}

bool Checker::doSimpleCheckRule3()
{
    cout << "Rule3: loop boxes ascending, loop goods descending, ";
    cout << "larger good is prior to be put into smaller box.\n";
    vector<int> temp_boxes;
    temp_boxes.assign(m_boxes_size.begin(), m_boxes_size.end());
    vector<int>::iterator bit = temp_boxes.begin();
    list<int> temp_goods;
    temp_goods.assign(m_goods_size.begin(), m_goods_size.end());
    list<int>::reverse_iterator git = temp_goods.rbegin();
    while (bit != temp_boxes.end())
    {
        int remain_sz = *bit;
        git = temp_goods.rbegin();
        while (git != temp_goods.rend())
        {
            if (remain_sz < *git) {
                if (remain_sz < *bit) {
                    // reset and re-sort boxes size,
                    // loop from beginning as the goods are descending.
                    *bit = remain_sz;
                    sort(temp_boxes.begin(), bit + 1);
                    bit = temp_boxes.begin();
                } else {
                    bit++;
                }
                break;
            } else {
                remain_sz -= *git;
                // if the goods can be put into this box,
                // remove it from the list.
                git = list<int>::reverse_iterator(temp_goods.erase((++git).base()));
            }
        }
    }
    if (temp_goods.empty()) {
        cout << "- successful!\n";
        return true;
    }
    cout << "- failed!\n";
    return false;
}

string Checker::size_to_string(const vector<int> &list)
{
    string ret("[");
    int num = list.size();
    int k = 0;
    for (; k < num; ++k)
    {
        ostringstream ss;
        ss << list[k];
        ret += ss.str();
        break;
    }
    ++k;
    for (; k < num; ++k)
    {
        ostringstream ss;
        ss << list[k];
        ret += (", " + ss.str());
    }
    ret += "]";
    return ret;
}

string Checker::comb_to_string(uint64_t goods_comb)
{
    string ret("[");
    int k = 0;
    for (; k < m_goods_num; ++k)
    {
        if (goods_comb & (uint64_t(1) << k))
        {
            ostringstream ss;
            ss << m_goods_size[k] << "(" << k << ")";
            ret += ss.str();
            break;
        }
    }
    ++k;
    for (; k < m_goods_num; ++k)
    {
        if (goods_comb & (uint64_t(1) << k))
        {
            ostringstream ss;
            ss << m_goods_size[k] << "(" << k << ")";
            ret += (", " + ss.str());
        }
    }
    ret += "]";
    return ret;
}

