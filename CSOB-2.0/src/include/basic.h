#ifndef _BASIC_H
#define _BASIC_H


#include <boost/serialization/base_object.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/vector.hpp>

enum trade_flag_t{chargeback_flag,deal_flag} ;
enum order_side_t{bid_flag,ask_flag};
enum order_type_t{market_flag,limit_flag,best_price_flag};
typedef unsigned long long time_stamp_t;

class price_t
{
private:
    int price_num;//正常price会保留4位小数，price_num记录price*10000的大小，以避免float运算的误差
public:
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version) {
        ar & price_num;
    }

    void set_price_num(int num) {this->price_num = num;};
    int get_price_num_in_int() const{return price_num;};
    float get_price_num_in_float()const {float tmp=(float)price_num/10000.0;return tmp;};
    price_t(int price=0){this->price_num=price;};
    price_t(const price_t& p){this->price_num=p.price_num;};
    price_t& operator=(const price_t& p){this->price_num=p.price_num;return *this;};
    bool operator<(const price_t& p)const{return this->price_num<p.price_num;};
    bool operator>(const price_t& p)const{return this->price_num>p.price_num;};
    bool operator==(const price_t& p)const{return this->price_num==p.price_num;};
    bool operator!=(const price_t& p)const{return this->price_num!=p.price_num;};
    int operator*(int p)const {return this->price_num*p;}
    friend std::ostream &operator<<(std::ostream &o, const price_t& p){o<<p.get_price_num_in_int();return o;}
    price_t(float price){this->price_num=(int)(price*10000);};
    ~price_t(){};
};

class instrument_t
{
protected:
    time_stamp_t nano_second;//时间戳
    int instrument_id;//债券代码
    //src_id 过滤4
public:
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version) {
        ar & nano_second;
        ar & instrument_id;
    }
    void set_nano_second(time_stamp_t ns){this->nano_second=ns;};
    time_stamp_t get_nano_second()const {return this->nano_second;};
    void set_instrument_id(int id){this->instrument_id=id;};
    int get_instrument_id() const {return this->instrument_id;};
    instrument_t(time_stamp_t ns=0,int id=0){this->nano_second=ns;this->instrument_id=id;};
    ~instrument_t(){};
};

class base_t:public instrument_t
{
private:
    price_t open;// 开盘价
    price_t upper_limit;// 涨停
    price_t lower_limit;// 跌停
    price_t pre_close;//昨天收盘价
    price_t pre_settlement;//昨天结算价
public:
    void set_open(price_t open_price) {this->open=open_price;};
    price_t get_open()const {return this->open;};
    void set_upper_limit(price_t upper_price) {this->upper_limit=upper_price;};
    price_t get_upper_limit()const {return this->upper_limit;};
    void set_lower_limit(price_t lower_price) {this->lower_limit=lower_price;};
    price_t get_lower_limit()const {return this->lower_limit;};
    void set_pre_close(price_t pre_close_price) {this->pre_close=pre_close_price;};
    price_t get_pre_close()const {return this->pre_close;};
    void set_pre_settlement(price_t pre_settlement_price) {this->pre_settlement=pre_settlement_price;};
    price_t get_pre_settlement()const {return this->pre_settlement;};
    base_t(time_stamp_t ns=0,int id=0,price_t open_price=0,price_t upper_price=0,price_t lower_price=0,price_t pre_close_price=0,price_t pre_settlement_price=0):instrument_t(ns,id)
    {
        this->open=open_price;
        this->upper_limit=upper_price;
        this->lower_limit=lower_price;
        this->pre_close=pre_close_price;
        this->pre_settlement=pre_settlement_price;
    };
    ~base_t(){};
};

class trade_t:public instrument_t
{
private:
    trade_flag_t trade_flag;// 0 is 撤单, 1 is 成交 ; defined before namespace
    price_t trade_price;// 单价
    int trade_vol;// 数量
    //Msgtype  过滤掉300192
    int seq_num;
    int bid_seq_num;//bid_order's seq_num
    int ask_seq_num;//ask_order's seq_num
public:
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version) {
        ar & boost::serialization::base_object<instrument_t>(*this);
        ar & trade_flag;
        ar & trade_price;
        ar & trade_vol;
        ar & seq_num;
        ar & bid_seq_num;
        ar & ask_seq_num;
    }

    void set_trade_flag(trade_flag_t flag){this->trade_flag=flag;};
    trade_flag_t get_trade_flag()const {return this->trade_flag;};
    void set_trade_price(price_t price){this->trade_price=price;};
    price_t get_trade_price()const {return this->trade_price;};
    void set_trade_vol(int vol){this->trade_vol=vol;};
    int get_trade_vol()const {return this->trade_vol;};
    void set_seq_num(int seq){this->seq_num=seq;};
    int get_seq_num()const {return this->seq_num;};
    void set_bid_seq_num(int bid_seq){this->bid_seq_num=bid_seq;};
    int get_bid_seq_num()const {return this->bid_seq_num;};
    void set_ask_seq_num(int ask_seq){this->ask_seq_num=ask_seq;};
    int get_ask_seq_num()const {return this->ask_seq_num;};

    trade_t(time_stamp_t ns=0,int id=0,trade_flag_t flag=chargeback_flag,price_t price=0,int vol=0,int seq=0,int bid_seq=0,int ask_seq=0):instrument_t(ns,id)
    {
        this->trade_flag=flag;
        this->trade_price=price;
        this->trade_vol=vol;
        this->seq_num=seq;
        this->bid_seq_num=bid_seq;
        this->ask_seq_num=ask_seq;
    };
    ~trade_t(){};
};

class order_t:public instrument_t
{
private:
    order_side_t side;//0 is bid, 1 is ask ; defined before namespace
    price_t order_price;// 单价
    int order_vol;// 数量
    order_type_t ord_type;//0 is 市价单 ,1 is 限价单 ,2 is 本方最优 ; defined before namespace
    int seq_num;
public:
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version) {
        ar & boost::serialization::base_object<instrument_t>(*this);
        ar & side;
        ar & order_price;
        ar & order_vol;
        ar & ord_type;
        ar & seq_num;
    }
    void set_side(order_side_t side){this->side=side;};
    order_side_t get_side()const {return this->side;}; 
    void set_order_price(price_t price){this->order_price=price;};
    price_t get_order_price()const {return this->order_price;};
    void set_order_vol(int vol){this->order_vol=vol;};
    int get_order_vol()const {return this->order_vol;};
    void set_ord_type(order_type_t type){this->ord_type=type;};
    order_type_t get_ord_type()const {return this->ord_type;};
    void set_seq_num(int seq){this->seq_num=seq;};
    int get_seq_num()const {return this->seq_num;};
    friend std::ostream& operator<<(std::ostream &o, const order_t & order) {
        o << order.get_order_vol(); return o;}
    bool operator==(const order_t o) const{
        return this->side==o.get_side()
        &&
        this->instrument_id==o.get_instrument_id()&&
        this->nano_second==o.get_nano_second()&&
        this->ord_type==o.get_ord_type()
        &&
        this->order_price==o.get_order_price()&&
        this->order_vol==o.get_order_vol()
        &&
        this->seq_num==o.get_seq_num();
    }
    order_t(time_stamp_t ns=0,int id=0,order_side_t side=bid_flag,price_t price=0,int vol=0,order_type_t type=limit_flag,int seq=0):instrument_t(ns,id)
    {
        this->side=side;
        this->order_price=price;
        this->order_vol=vol;
        this->ord_type=type;
        this->seq_num=seq;
    };
    ~order_t(){};
};



#endif
