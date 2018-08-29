#ifndef BOXES_GOODS_CHECKER_H
#define BOXES_GOODS_CHECKER_H

#include <stdint.h>
#include <vector>
#include <string>
using namespace std;

class Checker
{
public:
    Checker();
    Checker(bool simple_check);

    /* @brief main interface
     */
    void init(vector<int> &boxes, vector<int> &goods);
    void run();

private:

    /* @brief the first rule to check if all goods can be put into given boxes.
     * loop all boxes from small size to large size.
     * smaller size good is prior to put into smaller size box.
     * issue case: sizes of boxes are 6, 8, sizes of goods are 2, 3, 4, 5.
     */ 
    bool doSimpleCheckRule1();

    /* @brief the second rule to check if all goods can be put into given boxes.
     * loop all boxes from large size to small size.
     * larger size good is prior to put into larger size box.
     * issue case: sizes of boxes are 10, 9, sizes of goods are 9, 5, 5.
     */ 
    bool doSimpleCheckRule2();

    /* @brief the third rule to check if all goods can be put into given boxes.
     * loop all boxes from small size to large size.
     * larger size good is prior to put into smaller size box.
     * issue case: sizes of boxes are 9,10,11, sizes of goods are 8,3,3,2,2,2,2,2,2,2,2.
     */ 
    bool doSimpleCheckRule3();

    /* @brief a complex method
     * get all goods combinations for each box,
     * select the best combination respectively,
     * (best means the sum of size of remained goods is largest)
     * to see if these best combinations can cover all goods.
     * Remark: as we use 64-bits integer to record a combination,
     * the maximum number of goods is 64 currently,
     * we can change to vector<uint64_t> for improvement.
     */
    bool doComplexCheck();
    void findGoodCombPerBox(int target, int &current, int good_id, uint64_t &goods_comb, vector<uint64_t> &goods_combs);
    uint64_t getBestGoodCombPerBox(uint64_t &current, const vector<uint64_t> &goods_combs);

    /* @brief other small utils
     */
    string size_to_string(const vector<int> &list);
    string comb_to_string(uint64_t goods_comb);

private:

    bool m_simple_check;
    int m_boxes_num;
    int m_goods_num;
    vector<int> m_boxes_size;
    vector<int> m_goods_size;
    // the result container for complex method,
    // has same size as boxes.
    vector<uint64_t> m_result;
};

#endif
