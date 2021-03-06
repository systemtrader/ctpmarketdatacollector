﻿#include "datastore/mongostore.h"

#include <bsoncxx/builder/basic/document.hpp>
#include <bsoncxx/builder/basic/kvp.hpp>
#include <bsoncxx/types.hpp>

#include "utils/logger.h"

mongocxx::instance MongoStore::instance_ = {};
using namespace std::chrono_literals;
MongoStore::~MongoStore() {
    is_running_.store(false, std::memory_order_release);
    if (inter_thread_.joinable()) {
        inter_thread_.join();
    }
}

int32 MongoStore::init(const MongoConfig& mongo_config) {
    try {
        config_ = mongo_config;
        uri_    = {mongo_config.address};
        client_ = {uri_};
        db_     = client_.database(mongo_config.db);
    } catch (const std::exception& e) {
        ELOG("MongoDb init failed! {}", e.what());
        return -1;
    }

    return 0;
}

int32 MongoStore::start() {
    is_running_.store(true, std::memory_order_release);
    inter_thread_ = std::thread(&MongoStore::loop, this);
    return 0;
}

int32 MongoStore::stop() {
    is_running_.store(false, std::memory_order_release);
    if (inter_thread_.joinable()) {
        inter_thread_.join();
    }
    while (!buffer_.empty()) {
        process();
    }
    return 0;
}

MongoStore::DataBuffer& MongoStore::getBuffer() {
    return buffer_;
}

void MongoStore::loop() {
    while (is_running_.load(std::memory_order_relaxed)) {
        process();
        std::this_thread::sleep_for(5s);
    }
}

void MongoStore::process() {

    auto count = buffer_.read_available();

    DLOG("MongoDb insert!count:{} ", count);
    if (count == 0) {
        return;
    }
    MarketData data;
    while (buffer_.pop(data)) {
        DLOG("MongoDb pop data");
        try {
            if (data.volume != 0) {
                using bsoncxx::builder::basic::kvp;
                bsoncxx::builder::basic::document builder{};
                builder.append(kvp("id", data.instrument_id));
                builder.append(kvp("actionDate", data.action_day));
                builder.append(kvp("actionTime", data.action_time));
                builder.append(kvp("exchange", data.exchange_id));
                builder.append(kvp("high", data.high));
                builder.append(kvp("close", (data.close)));
                builder.append(kvp("open", (data.open)));
                builder.append(kvp("low", (data.low)));
                builder.append(kvp("volume", (data.volume)));
                builder.append(kvp("marketVol", (data.marketVol)));
                builder.append(kvp("highestPrice", (data.highest_price)));
                builder.append(kvp("lowestPrice", (data.lowest_price)));
                builder.append(kvp("openPrice", (data.open_price)));
                builder.append(kvp("preSettlementPrice", (data.PreSettlementPrice)));
                builder.append(kvp("preClosePrice", (data.PreClosePrice)));
                builder.append(kvp("Turnover", (data.Turnover)));
                builder.append(kvp("PreOpenInterest", (data.PreOpenInterest)));
                builder.append(kvp("OpenInterest", (data.OpenInterest)));
                builder.append(kvp("UpperLimitPrice", (data.UpperLimitPrice)));
                builder.append(kvp("LowerLimitPrice", (data.LowerLimitPrice)));
                builder.append(kvp("mdTradingDay", data.md_trading_day));
                builder.append(kvp("mdUpdateTime", data.md_update_time));
                builder.append(kvp("recordTime", bsoncxx::types::b_date(data.last_record_time)));
                db_[data.destination_id].insert_one(builder.view());
                ILOG("MongoDb record one data ok!Inst:{},updateTime:{}", data.instrument_id, data.md_update_time);
            }else{
                ILOG(
                    "MongoDb ignore one "
                    "data!Inst:{},actionDate:{},actionTime{},volume:{},marketVol:{},Turnover{},mdTradingDay:{},"
                    "mdUpdateTime;{}",
                    data.instrument_id,
                    data.action_day,
                    data.action_time,
                    data.volume,
                    data.marketVol,
                    data.Turnover,
                    data.md_trading_day,
                    data.md_update_time);
            
            }



        } catch (const std::exception& e) {
            ELOG("MongoDb insert failed! {}", e.what());
        }
    }
}
