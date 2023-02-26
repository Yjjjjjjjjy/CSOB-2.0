#include "./include/calculate.h"
#include "./sub-pub/redis_publisher.h"
#include <assert.h>


void order_book_t::insert_buyer_order(order_t order) {

    price_time new_pt(order.get_order_price());
    new_pt.seq_nums.emplace(order.get_seq_num());
    order_t t = this->buyer_queue->search_element(new_pt);
    if (t.get_nano_second() == 0) // null
    {
        this->buyer_queue->insert_element(new_pt, order);
    }else {  // already exist order that have same price, just update num
        int pre_vol = t.get_order_vol();
        t.set_order_vol(pre_vol + order.get_order_vol());
        // update 不仅更新value，还设置新的key
        this->buyer_queue->update_element(new_pt, t);
    }

}

void order_book_t::insert_seller_order(order_t order) {

    price_time new_pt(order.get_order_price());
    new_pt.seq_nums.emplace(order.get_seq_num());
    order_t t = this->seller_queue->search_element(new_pt);
    if (t.get_nano_second() == 0) // null
    {
        this->seller_queue->insert_element(new_pt, order);
    }else {  // already exist order that have same price, just update num
        int pre_vol = t.get_order_vol();
        t.set_order_vol(pre_vol + order.get_order_vol());
        this->seller_queue->update_element(new_pt, t);
    }
}

order_t order_book_t::get_highest_buyer() {
    return this->buyer_queue->get_first();
}

order_t order_book_t::get_lowest_seller() {
    return this->seller_queue->get_first();
}

void order_book_t::del_target_buyer(int seq_num, int vol) {
    auto [pt, t] = this->buyer_queue->get_seq_vol(seq_num);

    assert(t.get_nano_second() != 0);

    int new_vol = t.get_order_vol() - vol;



    assert(new_vol >= 0);

    if (new_vol == 0) {
        this->buyer_queue->delete_element(pt);
    }else {
        t.set_order_vol(new_vol);
        this->buyer_queue->update_element(pt, t);
    }

}

void order_book_t::del_target_seller(int seq_num, int vol) {
    auto [pt, t] = this->seller_queue->get_seq_vol(seq_num);

    assert(t.get_nano_second() != 0);

    int new_vol = t.get_order_vol() - vol;
    

        if (!(new_vol >= 0)) {
        display_seller("./log.txt");
        std::fstream fout("./log.txt", std::ios::app);

        fout << seq_num << " " << vol << '\n';
    }
    assert(new_vol >= 0);
    if (new_vol == 0) {
        this->seller_queue->delete_element(pt);
    }else {
        t.set_order_vol(new_vol);
        this->seller_queue->update_element(pt, t);
    }
}

void order_book_t::display_buyer(std::string filename){
    // std::cout << "display_buyer_list" << std::endl;
    this->buyer_queue->display_list(filename);
}

void order_book_t::display_seller(std::string filename){
    // std::cout << "display_seller_list" << std::endl;
    this->seller_queue->display_list(filename);
}



order_book_t::order_book_t() {

    this->buyer_queue = new Skiplist<price_time, order_t>(32, -1);
    this->seller_queue = new Skiplist<price_time, order_t>(32, 1);
}

std::string order_book_t::serialize(time_stamp_t cur_time) const {
    
    // 先转成bson格式，在调用to_json转成字符串
    std::vector<std::pair<price_time, order_t>> sklist = buyer_queue->dump_list();

    auto builder = bsoncxx::builder::stream::document{};
    
    auto before_buyer = builder
    << "timestamp" << std::to_string(cur_time)
    << "buyer_queue" << bsoncxx::builder::stream::open_array;

    for (auto &[k, v]: sklist) 
        before_buyer = before_buyer << bsoncxx::builder::stream::open_document 
                                    << "key"
                                    << k.price.get_price_num_in_int() 
                                    << "value" 
                                    << v.get_order_vol()
                                    << bsoncxx::builder::stream::close_document;

    sklist = seller_queue->dump_list();
    
    auto before_seller = before_buyer << bsoncxx::builder::stream::close_array
    << "seller_queue" << bsoncxx::builder::stream::open_array;

    for (auto &[k, v]: sklist) 
        before_seller = before_seller << bsoncxx::builder::stream::open_document 
                                      << "key"
                                      << k.price.get_price_num_in_int() 
                                      << "value" 
                                      << v.get_order_vol()
                                      << bsoncxx::builder::stream::close_document;

    bsoncxx::document::value doc_value = before_seller << bsoncxx::builder::stream::close_array
    << bsoncxx::builder::stream::finalize;  
    return bsoncxx::to_json(doc_value);
}

bsoncxx::document::value order_book_t::deserialize(std::string data) {
    return bsoncxx::from_json(data);
}

order_book_t::~order_book_t() {
    delete buyer_queue;
    delete seller_queue;
}

void calculator::do_calculation(VectorSerialization<order_t>& orders, VectorSerialization<trade_t>& trades) {
    auto it_orders = orders.begin();
    auto it_trades = trades.begin();
    int counter = 0;
    int tot = 0;
    int storage_interval = 10000; //多少次计算存一次

    it_orders = orders.begin();
    it_trades = trades.begin();

    CRedisPublisher publisher;
    std::string channel_name = "test-channel";

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

    order_t cur_order;
    trade_t cur_trade;
    // 按照时间先后处理
    while (it_orders != orders.end() && it_trades != trades.end())
    {
        cur_order = *it_orders;
        cur_trade = *it_trades;

        // 交易先达成
        if (cur_order.get_nano_second() > cur_trade.get_nano_second()) {
            handle_trade(cur_trade);
            it_trades ++;
        }else {
            handle_order(cur_order);
            it_orders ++;
        }

        if (++counter > storage_interval) {
            counter = 0;
            tot++;
            // order_book->display_buyer("tmp.txt");
            // order_book->display_seller("tmp.txt");
            publisher.publish(channel_name, this->order_book->serialize(cur_time));
            // std::cerr << order_book->serialize(cur_time);
        }
    }

    while (it_orders != orders.end())
    {
        cur_order = *it_orders;
        handle_order(cur_order);
        it_orders ++;
        if (++counter > storage_interval) {
            counter = 0;
            tot++;
            publisher.publish(channel_name, this->order_book->serialize(cur_time));
        }
    }

    while (it_trades != trades.end()) {
        cur_trade = *it_trades;
        handle_trade(cur_trade);
        it_trades ++;
        if (++counter > storage_interval) {
            counter = 0;
            tot++;
            publisher.publish(channel_name, this->order_book->serialize(cur_time));
        }
    }

    // std::cerr << '\n' << this->order_book->serialize(cur_time) << '\n';
    publisher.publish(channel_name, this->order_book->serialize(cur_time));
    
    sleep(1);
    publisher.disconnect();
    publisher.uninit();
    
}

void calculator::handle_order(order_t order) {
    order_side_t side = order.get_side();
    cur_time = order.get_nano_second();

    if (side == bid_flag){ 
        this->order_book->insert_buyer_order(order);
    }else if (side == ask_flag){
        this->order_book->insert_seller_order(order);
    }else  assert(false);
}

// 这里有一个问题：
// 我们后来把价格相同的order放在skiplist的一个node里面，但是撤单中只提供了seqnum一个信息，
// 但是seqnum在我们存的时候丢失掉了，暂时有两个解决方法：
// 1. 在计算之前，把每一笔撤单的价格补上（根据seqnum在order里面搜索）
// 2. 改变order_t的结构，将int seqnum 改成unordered_set，这样不会丢失seqnum信息
// 暂时先采用第二种方法
void calculator::handle_trade(trade_t trade) {
    trade_flag_t trade_flag = trade.get_trade_flag();
    int ask = trade.get_ask_seq_num();
    int bid = trade.get_bid_seq_num();
    cur_time = trade.get_nano_second();

    // 这里有个问题，无法对每一个seqnum对应的vol检测
    if (trade_flag == chargeback_flag) {
        // 如果是买，则卖的seqnum就是0
        if (ask == 0) {
            this->order_book->del_target_buyer(bid, trade.get_trade_vol());
        }else if (bid == 0) {
            this->order_book->del_target_seller(ask, trade.get_trade_vol());
        }else assert(false);

    // 这里有点问题，成交是按照trade的价格成交，但是要按照seqnum删除，因为委托和成交价格可能不一样
    }else if (trade_flag == deal_flag) {
        this->order_book->del_target_buyer(bid, trade.get_trade_vol());
        this->order_book->del_target_seller(ask, trade.get_trade_vol());
    }

}

