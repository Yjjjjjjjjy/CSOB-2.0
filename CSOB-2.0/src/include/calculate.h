#ifndef _CALCULATE_H_
#define _CALCULATE_H_

#include "skipList.h"
#include <vector>
#include <unordered_set>
#include <bsoncxx/builder/stream/helpers.hpp>
#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/builder/stream/array.hpp>
#include <bsoncxx/json.hpp>
#include "getbasic.h"

// 价格-时间对，用来做orderbook存储的key
struct price_time{

    price_t price;
    std::unordered_set<int> seq_nums;

    price_time(){}
    price_time(price_t p){price = price_t(p.get_price_num_in_int());}

    // 运算符重载，用在模板类skiplist里面
    bool operator<(const price_time& a) const{return a.price < price;}
    bool operator>(const price_time& a) const{return a.price > price;}
    bool operator==(const price_time& a) const{return a.price == price;}
    bool operator!=(const price_time& a) const{return a.price != price;}

    void add(const price_time& a) {   
        auto set = a.seq_nums;
        for (auto &element: set) this->seq_nums.emplace(element);
    }

    friend price_time operator* (const price_time&a, int i) {
        price_time rt;
        rt.price.set_price_num(a.price.get_price_num_in_int()*i);
        return rt;
    }

    friend price_time operator* (int i, const price_time&a ) {
        price_time rt;
        rt.price.set_price_num(a.price.get_price_num_in_int()*i);
        return rt;
    }

    friend std::ostream& operator<<(std::ostream& out, const price_time& a) {
        out << "[" << a.price<< " , " ;
        for (auto it = a.seq_nums.begin(); it != a.seq_nums.end(); it++) out << *it <<" ";
        out << "]\n" ;
        return out;
    }

    bool contains(int seq_num) {
        return seq_nums.find(seq_num) != seq_nums.end();
    }
};


class order_book_t { 

public:
    void insert_buyer_order(order_t);
    void insert_seller_order(order_t);

    order_t get_highest_buyer();
    order_t get_lowest_seller();

    // void del_highest_buyer();
    // void del_lowest_seller();

    void del_target_buyer(int seq_num, int vol);
    void del_target_seller(int seq_num, int vol);

    void display_buyer(std::string file);
    void display_seller(std::string file);

    std::string serialize(time_stamp_t) const;
    static bsoncxx::document::value deserialize(std::string);

    order_book_t();
    ~order_book_t();

private:
    Skiplist<price_time, order_t> *buyer_queue;
    Skiplist<price_time, order_t> *seller_queue;
    

};

class calculator {
public:
    void do_calculation(VectorSerialization<order_t>& orders, VectorSerialization<trade_t>& trades);
    void handle_trade(trade_t trade);    // 处理成交或撤单
    void handle_order(order_t order);   // 处理委托
    ~calculator(){
        delete order_book;
    };
    calculator(){
        order_book = new order_book_t();
    };
private:
    time_stamp_t cur_time;
    order_book_t *order_book;
};



#endif