#include "include/getbasic.h"
#include "include/calculate.h"
#include <mutex>
#include "./sub-pub/redis_subscriber.h"
#include <ctime>

// cd src and make and in kernel run ./csob
// there is a breakpoint in getbasic.cpp to show tjhe documents
GetBasic getbasic();
std::mutex mtx_g;
void callback1(const char *channel_name,
                     const char *message, int len) {
    mtx_g.lock();
    printf("    Recieve message:\n    channel name: %s\n    len: %d\n",
            channel_name, len);
    if (message == NULL){
        mtx_g.unlock();
         return ;
    }
    std::string msg = message;
    if (msg.substr(0, 7) == "getdata") {
        rapidcsv::Document doc(getbasic.path + "/" + getbasic.fileNames[getbasic.cur_id], rapidcsv::LabelParams(-1,-1));
        getbasic.parse(doc, std::stoi(msg.substr(7, 1)));
        getbasic.cur_id++;
    }
    mtx_g.unlock();

}
int main(int argc, char* argv[])
{
    
    if (argc != 2) {
        std::cerr << "usage: " << argv[0] << " dirname\n";
        return 0;
    }
    std::string dir_name = argv[1];
    getbasic.scan_dir(dir_name);
    
    clock_t start, end;

    start = clock();
    CRedisSubscriber subscriber;
    CRedisSubscriber::NotifyMessageFn fn =
        bind(callback1, std::placeholders::_1,
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
    subscriber.subscribe("producer");

    rapidcsv::Document doc(getbasic.path + "/" + getbasic.fileNames[getbasic.cur_id], rapidcsv::LabelParams(-1,-1));
    getbasic.parse(doc, 0);
    getbasic.cur_id++;

    rapidcsv::Document doc1(getbasic.path + "/" + getbasic.fileNames[getbasic.cur_id], rapidcsv::LabelParams(-1,-1));
    getbasic.parse(doc1, 1);
    getbasic.cur_id++;

    rapidcsv::Document doc2(getbasic.path + "/" + getbasic.fileNames[getbasic.cur_id], rapidcsv::LabelParams(-1,-1));
    getbasic.parse(doc2, 2);
    getbasic.cur_id++;

    rapidcsv::Document doc3(getbasic.path + "/" + getbasic.fileNames[getbasic.cur_id], rapidcsv::LabelParams(-1,-1));
    getbasic.parse(doc3, 3);
    getbasic.cur_id++;

    while (getbasic.cur_id < getbasic.fileNames.size()) {
        sleep(1);
    }
    end = clock();
    std::cout << "load: " << (double)(end-start)/CLOCKS_PER_SEC << "s" << std::endl;
    subscriber.disconnect();
    subscriber.uninit();
    
    return 0;
}