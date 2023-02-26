#ifndef GETBASIC_H
#define GETBASIC_H


#include "RapidCSV.h"
#include "basic.h"
#include "utils.h"

#include <vector>
#include <unordered_map>


#define BaseFid 1541
#define TradeFid 1538
#define OrderFid 1539


#define Base_Datatype_Col 0
#define Base_FirstFid_Col 1
#define Base_ScrType_Col 2
#define Base_SrcId_Col 3
#define Base_ActionDay_Col 4
#define Base_InstrumentId_Col 5
#define Base_UpdateTime_Col 6
#define Base_UpdateMillisec_Col 7
#define Base_ExchangeNum_Col 8
#define Base_ProductType_COl 9
#define Base_NanoSecond_Col 10
#define Base_SecondFid_Col 11
#define Base_TradingDay_Col 12
#define Base_MutiVolume_Col 13
#define Base_StreamId_Col 14
#define Base_PriceTick_Col 15
#define Base_Highest_Col 16
#define Base_Lowest_Col 17
#define Base_Open_Col 18
#define Base_Close_Col 19
#define Base_UpperLimit_Col 20
#define Base_LowerLimit_Col 21
#define Base_Settlement_Col 22
#define Base_CurrDelta_Col 23
#define Base_PreCloseCol 24
#define Base_PreSettlement_Col 25
#define Base_PreOpenInterest_Col 26
#define Base_PreIOPV_Col 27
#define Base_AuctionPrice_Col 28
#define Base_AuctionQty_Col 29


#define Trade_Datatype_Col 0
#define Trade_FirstFid_Col 1
#define Trade_ScrType_Col 2
#define Trade_SrcId_Col 3
#define Trade_ActionDay_Col 4
#define Trade_InstrumentId_Col 5
#define Trade_UpdateTime_Col 6
#define Trade_UpdateMillisec_Col 7
#define Trade_ExchangeNum_Col 8
#define Trade_ProductType_COl 9
#define Trade_NanoSecond_Col 10
#define Trade_SecondFid_Col 11
#define Trade_Side_Col 12
#define Trade_TradeFlag_Col 13
#define Trade_Price_Col 14
#define Trade_Vol_Col 15
#define Trade_SrcExch_Col 16
#define Trade_MsgType_Col 17
#define Trade_Channel_Col 18
#define Trade_StreamId_Col 19
#define Trade_SeqNum_Col 20
#define Trade_BidSeqNum_Col 21
#define Trade_AskSeqNum_Col 22
#define Trade_Money_Col 23
#define Trade_BizIndex_Col 24


#define Order_Datatype_Col 0
#define Order_FirstFid_Col 1
#define Order_ScrType_Col 2
#define Order_SrcId_Col 3
#define Order_ActionDay_Col 4
#define Order_InstrumentId_Col 5
#define Order_UpdateTime_Col 6
#define Order_UpdateMillisec_Col 7
#define Order_ExchangeNum_Col 8
#define Order_ProductType_COl 9
#define Order_NanoSecond_Col 10
#define Order_SecondFid_Col 11
#define Order_Side_Col 12
#define Order_OrderType_Col 13
#define Order_Price_Col 14
#define Order_Vol_Col 15
#define Order_SrcExch_Col 16
#define Order_MsgType_Col 17
#define Order_Channel_Col 18
#define Order_StreamId_Col 19
#define Order_SeqNum_Col 20
#define Order_BizIndex_Col 21

template<class T>
class Serialization : public T {
public:
 
    void serialization(std::ostringstream& ostream) {
        boost::archive::text_oarchive oa(ostream);
        oa << *this;
    }
 
    void unserialization(std::istringstream& istream) {
        boost::archive::text_iarchive ia(istream);
        ia >> *this;
    }
private:  
    friend class boost::serialization::access;  
 
    template<class Archive>  
    void serialize(Archive& ar, const unsigned int version) {  
        ar & boost::serialization::base_object<T>(*this);  
    }
};


template<class T>
class VectorSerialization : public Serialization<std::vector<T> > {
};


class GetBasic
{
private:

    VectorSerialization<trade_t> trade;
    VectorSerialization<order_t> order;


    //根据InstrumentID储存入二维数组中
    // std::vector<VectorSerialization<trade_t>> Trades;
    // std::vector<VectorSerialization<order_t>> Orders;
//    std::vector<std::vector<base_t>> Bases;

    //将第row行数据构造一个trade并加入trades
    void get_Trade(int row, rapidcsv::Document& doc);


    //将第row行数据构造一个order并加入orders
    void get_Order(int row, rapidcsv::Document& doc);

    //将第row行数据构造成一个base并加入bases
//    void get_Base(int row, rapidcsv::Document& doc);


public:
    std::vector<std::string> fileNames;
    int cur_id;
    std::string path;

    GetBasic();  

    //根据SrcId将数据分流至get_Trade()或者get_Order()
    void parse(rapidcsv::Document& doc, const std::string& channel_name);

    void publish(rapidcsv::Document, int);

    void scan_dir(const std::string&);
};


#endif // GETBASIC_H
