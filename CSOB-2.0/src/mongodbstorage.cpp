#include <cstdint>
#include <iostream>
#include <vector>
#include <bsoncxx/json.hpp>
#include <mongocxx/client.hpp>
#include <mongocxx/stdx.hpp>
#include <mongocxx/uri.hpp>
#include <mongocxx/instance.hpp>
#include <bsoncxx/builder/stream/helpers.hpp>
#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/builder/stream/array.hpp>
#include "./sub-pub/redis_subscriber.h"


using bsoncxx::builder::stream::close_array;
using bsoncxx::builder::stream::close_document;
using bsoncxx::builder::stream::document;
using bsoncxx::builder::stream::finalize;
using bsoncxx::builder::stream::open_array;
using bsoncxx::builder::stream::open_document;


/* 编译 
g++ mongodbstorage.cpp 
-I/usr/local/include/bsoncxx/v_noabi 
-I/usr/local/include/mongocxx/v_noabi 
-I/usr/local/include/libmongoc-1.0/ 
-I/usr/local/include/libbson-1.0 
-L/usr/local/lib -lmongocxx -lbsoncxx
*/



/*
    数据存mongodb的格式

    {
        "timestamp" : uint64,
        "seller_queue" : {
            price : vol,
            price : vol
            ...
        },
        "buyer_queue" : {
            price : vol,
            price : vol
            ...
        }
    }

    如果分库分表，那么以股票代码作为表名

*/
mongocxx::instance instance{};
mongocxx::client client{mongocxx::uri{"mongodb://localhost:27017"}};
// 如果不存在，会创建
mongocxx::database db = client["mydb"];
mongocxx::collection coll = db["test"];
auto index_specification = document{} << "timestamp" << 1 << finalize;
// coll.create_index(std::move(index_sepcification));

// 收到消息的回调函数
void recieve_message(const char *channel_name,
                     const char *message, int len) {
    printf("Recieve message:\n    channel name: %s\n    len: %d\n",
            channel_name, len);
    try
    {   // 把字符串转成bson格式
        auto msg = bsoncxx::from_json(message ? message : "");
        // std::cerr << bsoncxx::to_json(msg).size() << '\n';
        coll.insert_one(std::move(msg));

        // mongocxx::cursor cursor = coll.find({});
        // for(auto doc : cursor) {
        // std::cout << bsoncxx::to_json(doc) << "\n";
        // }
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }

}

int main() {

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

    subscriber.subscribe("test-channel");

    while (true) {
        sleep(1);
    }

    subscriber.disconnect();
    subscriber.uninit();

    return 0;
}