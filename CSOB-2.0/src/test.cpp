#include <cstring>
#include <iostream>
#include <vector>
#include <ctime>
#include "./include/basic.h"
#include "./include/getbasic.h"

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/vector.hpp>
using namespace std;


class test
{
private:
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ar & id;
        ar & str;
        ar & vec;
        ar & vec2;

    }
    /* data */
public:
    int id;
    string str;
    vector<int> vec;
    vector<vector<string>> vec2;
    test(int id, string str, vector<int> vec, vector<vector<string>>vec2);
    test(int id);
    test();
    ~test();
};

test::test(int id, string str, vector<int> vec, vector<vector<string>>vec2)
{
    this->id = id;
    this->str = str;
    this->vec = vec;
    this->vec2 = vec2;
}
test::test(int id):id(id){}
test::test(){}

test::~test()
{
}




int main() {

    auto t1 = vector<int>(1, 1);
    auto t2 = vector<vector<string>>(1,vector<string>(1, "happy"));
    test t(1, "hello", t1, t2 );
    // test t3(1);
    ostringstream os1, os2;
    // boost::archive::text_oarchive oa(os);
    // oa << t;

    clock_t start, end;

    
    VectorSerialization<order_t> orders;
    VectorSerialization<trade_t> trades;
    GetBasic getbasic("./data");
    start = clock();
    VectorSerialization<order_t> tmp1 = getbasic.get_Orders()[0];
    VectorSerialization<trade_t> tmp2 = getbasic.get_Trades()[0];
    tmp1.serialization(os1);
    tmp2.serialization(os2);


    istringstream is1(os1.str()), is2(os2.str());
    orders.unserialization(is1);
    trades.unserialization(is2);
    
    end = clock();
    std::cout << "load: " << (double)(end-start)/CLOCKS_PER_SEC << "s" << std::endl;
    if (tmp1.size() != orders.size() ) {std::cout << "size not equal\n"; return 0;}
    for (int i = 0; i < orders.size(); i++) {
        if (!(tmp1[i]==orders[i])) {
            std::cerr << i << " " << orders[i].get_order_vol() << " " << tmp1[i].get_order_vol() << "\n";
            std::cerr << "order not equal\n";return 0;  }
        // if (!(tmp2[i] == trades[i])) std::cerr << "trade not equal\n";
    }
    // orders.emplace_back()

    // string content = os.str();
    // cout << content << '\n';
    // test tmp;
    // istringstream is(content);
    // boost::archive::text_iarchive ia(is);
    // ia >> tmp;
    // cout << tmp.id << tmp.str << tmp.vec[0] << tmp.vec2[0][0];

    return 0;
}