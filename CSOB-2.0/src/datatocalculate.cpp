#include <cstdint>
#include <iostream>
#include <vector>
#include <ctime>
#include "./sub-pub/redis_publisher.h"
#include "./sub-pub/redis_subscriber.h"
#include "./include/basic.h"
#include "./include/getbasic.h"
#include "./include/calculate.h"


VectorSerialization<trade_t> trades;
VectorSerialization<order_t> orders;
int size_order = 0, size_trade = 0;
clock_t start, end;
std::fstream fout;
std::string log_name;

int my_id;
// 收到消息的回调函数
void recieve_message(const char *channel_name,
                     const char *message, int len) {
    printf("    Recieve message:\n    channel name: %s\n    len: %d\n",
            channel_name, len);
    try
    {  
        if (len == 0) return ;
         std::string str = message;
        
        if (str == "finish") {
            start = clock();
            calculator *calcu = new calculator;
            calcu->do_calculation(orders,trades);
            end = clock();
            // std::cout << "cal: " << (double)(end-start)/CLOCKS_PER_SEC << "s" << std::endl;
            fout.open(log_name, std::ios::app);
            fout <<orders[0].get_instrument_id() << " " <<  (double)(end-start)/CLOCKS_PER_SEC << '\n';
            fout.close();
            
            trades.clear();
            orders.clear();
            
            size_order = size_trade = 0;
            // std::cerr << "------------------finish cal done------------------\n";
            delete calcu;

             CRedisPublisher publisher;
             bool ret = publisher.init();
            if (!ret)
            {
                printf("Init failed.\n");
                return;
            }

            ret = publisher.connect();
            if (!ret)
            {
                printf("connect failed.");
                return;
            }
            publisher.publish("producer", "getdata"+std::to_string(my_id));
            sleep(1);
            publisher.disconnect(); 
            publisher.uninit();
        }else {

            // std::cerr << "--------------------start cal---------------------" << '\n' ;
            // std::cerr<<'\n'<<"The new size is "<<orders.size()<<" "<<trades.size()<<'\n';
            int size = std::stoi(str.substr(0, 10));
            
            std::istringstream is1(str.substr(10, size)), is2(str.substr(10+size, len));
            VectorSerialization<trade_t> trade_tmp;
            VectorSerialization<order_t> order_tmp;
            trade_tmp.unserialization(is2);
            order_tmp.unserialization(is1);
            trades.insert(trades.end(), trade_tmp.begin(), trade_tmp.end());
            orders.insert(orders.end(), order_tmp.begin(), order_tmp.end());
            size_order += order_tmp.size();
            size_trade += trade_tmp.size();

            printf("orders len is %ld\n",orders.size());
        }
    }
    catch(const std::exception& e)
    {
         std::cerr << e.what() << '\n';
    }

}

int main(int argc, char* argv[]) {

    CRedisSubscriber subscriber;
    CRedisSubscriber::NotifyMessageFn fn =
        bind(recieve_message, std::placeholders::_1,
            std::placeholders::_2, std::placeholders::_3);
    bool ret = subscriber.init(fn);

    if (!ret) {
        printf("Init failed.\n");
        return 0;
    }

    ret = subscriber.connect();
    if (!ret) {
        printf("Connect failed.\n");
        return 0;
    }

    std::string channel_name = argv[1];
    log_name = argv[2];

    std::string tmp = argv[3];
    my_id = std::stoi(tmp);
    subscriber.subscribe(channel_name);

    while (true) {
        std::cerr << "in cycle";
        sleep(1);
    }

    subscriber.disconnect();
    subscriber.uninit();

    return 0;
}