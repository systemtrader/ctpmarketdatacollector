﻿#include "ctpmdspi.h"

#include "utils/logger.h"

void CtpMdSpi::setOnFrontConnected(std::function<void()>&& fun) {
    std::lock_guard<utils::spinlock> guard(lock_);
    on_connected_fun_ = fun;
}

void CtpMdSpi::setOnFrontDisConnected(std::function<void(int32)>&& fun) {
    std::lock_guard<utils::spinlock> guard(lock_);
    on_disconnected_fun_ = fun;
}

void CtpMdSpi::setOnLoginFun(std::function<void(CThostFtdcRspUserLoginField*, CThostFtdcRspInfoField*)> fun) {
    std::lock_guard<utils::spinlock> guard(lock_);
    on_login_fun_ = fun;
}

void CtpMdSpi::setOnErrorFun(std::function<void(CThostFtdcRspInfoField*)> fun) {
    std::lock_guard<utils::spinlock> guard(lock_);
    on_error_fun_ = fun;
}

void CtpMdSpi::setOnSubFun(std::function<void(CThostFtdcSpecificInstrumentField*, CThostFtdcRspInfoField*)> fun) {
    std::lock_guard<utils::spinlock> guard(lock_);
    on_sub_fun_ = fun;
}

void CtpMdSpi::setOnUnSubFun(std::function<void(CThostFtdcSpecificInstrumentField*, CThostFtdcRspInfoField*)> fun) {
    std::lock_guard<utils::spinlock> guard(lock_);
    on_unsub_fun_ = fun;
}

void CtpMdSpi::setOnDataFun(std::function<void(CThostFtdcDepthMarketDataField*)> fun) {
    std::lock_guard<utils::spinlock> guard(lock_);
    on_data_fun_ = fun;
}

void CtpMdSpi::setOnQuoteFun(std::function<void(CThostFtdcForQuoteRspField*)> fun) {
    std::lock_guard<utils::spinlock> guard(lock_);
    on_quote_fun_ = fun;
}

void CtpMdSpi::OnFrontConnected() {
    ILOG("Ctp connected to front.");
    std::lock_guard<utils::spinlock> guard(lock_);
    if (on_connected_fun_) {
        std::invoke(on_connected_fun_);
    }
}

void CtpMdSpi::OnFrontDisconnected(int32 nReason) {
    ILOG("Ctp disconnect from front! Reason:{}", nReason);
    std::lock_guard<utils::spinlock> guard(lock_);
    if (on_disconnected_fun_) {
        std::invoke(on_disconnected_fun_, nReason);
    }
}

void CtpMdSpi::OnHeartBeatWarning(int32 nTimeLapse) {
    WLOG("Ctp {}s no heartbeat!", nTimeLapse);
}

void CtpMdSpi::OnRspUserLogin(CThostFtdcRspUserLoginField* pRspUserLogin,
                              CThostFtdcRspInfoField*      pRspInfo,
                              int32                        nRequestID,
                              bool                         bIsLast) {
    if (pRspInfo && pRspInfo->ErrorID != 0) {
        ELOG("Ctp Login failed! RequestID:{},IsLast:{},ErrorID:{},ErrorMsg:{}",
             nRequestID,
             bIsLast,
             pRspInfo->ErrorID,
             pRspInfo->ErrorMsg);
    } else if (pRspUserLogin && bIsLast) {
        ILOG("Ctp Login success! RequestID:{},IsLast:{},ErrorId:{}", nRequestID, bIsLast, pRspInfo->ErrorID);
    }

    std::lock_guard<utils::spinlock> guard(lock_);
    if (on_login_fun_) {
        std::invoke(on_login_fun_, pRspUserLogin, pRspInfo);
    }
}

void CtpMdSpi::OnRspUserLogout(CThostFtdcUserLogoutField* pUserLogout,
                               CThostFtdcRspInfoField*    pRspInfo,
                               int32                      nRequestID,
                               bool                       bIsLast) {
    if (pRspInfo && pRspInfo->ErrorID != 0) {
        ELOG("Ctp Logout failed! RequestID:{},IsLast:{},ErrorID:{},ErrorMsg:{}",
             nRequestID,
             bIsLast,
             pRspInfo->ErrorID,
             pRspInfo->ErrorMsg);
    } else if (pUserLogout && bIsLast) {
        ILOG("Ctp Logout success! RequestID:{},IsLast:{}", nRequestID, bIsLast);
    }
}

void CtpMdSpi::OnRspError(CThostFtdcRspInfoField* pRspInfo, int32 nRequestID, bool bIsLast) {
    ELOG("Ctp error happpend RequestID:{},IsLast:{},ErrorID:{},ErrorMsg:{}",
         nRequestID,
         bIsLast,
         pRspInfo->ErrorID,
         pRspInfo->ErrorMsg);
    std::lock_guard<utils::spinlock> guard(lock_);
    if (on_error_fun_) {
        std::invoke(on_error_fun_, pRspInfo);
    }
}

void CtpMdSpi::OnRspSubMarketData(CThostFtdcSpecificInstrumentField* pSpecificInstrument,
                                  CThostFtdcRspInfoField*            pRspInfo,
                                  int32                              nRequestID,
                                  bool                               bIsLast) {
    if (pRspInfo && pRspInfo->ErrorID != 0) {
        ELOG("Ctp SubMarketData failed! RequestID:{},IsLast:{},ErrorID:{},ErrorMsg:{}",
             nRequestID,
             bIsLast,
             pRspInfo->ErrorID,
             pRspInfo->ErrorMsg);
    } else if (pSpecificInstrument) {
        ILOG("Ctp SubMarketData success! RequestID:{},IsLast:{},InstrumentID:{},ErrodId:{}",
             nRequestID,
             bIsLast,
             pSpecificInstrument->InstrumentID,
             pRspInfo->ErrorID);
    }
    std::lock_guard<utils::spinlock> guard(lock_);
    if (on_sub_fun_) {
        std::invoke(on_sub_fun_, pSpecificInstrument, pRspInfo);
    }
}

void CtpMdSpi::OnRspUnSubMarketData(CThostFtdcSpecificInstrumentField* pSpecificInstrument,
                                    CThostFtdcRspInfoField*            pRspInfo,
                                    int32                              nRequestID,
                                    bool                               bIsLast) {
    if (pRspInfo && pRspInfo->ErrorID != 0) {
        ELOG("Ctp UnSubMarketData failed! RequestID:{},IsLast:{},ErrorID:{},ErrorMsg:{}",
             nRequestID,
             bIsLast,
             pRspInfo->ErrorID,
             pRspInfo->ErrorMsg);
    } else if (pSpecificInstrument) {
        ILOG("Ctp UnSubMarketData success! RequestID:{},IsLast:{},InstrumentID:{}",
             nRequestID,
             bIsLast,
             pSpecificInstrument->InstrumentID);
    }
    std::lock_guard<utils::spinlock> guard(lock_);
    if (on_unsub_fun_) {
        std::invoke(on_unsub_fun_, pSpecificInstrument, pRspInfo);
    }
}

void CtpMdSpi::OnRspSubForQuoteRsp(CThostFtdcSpecificInstrumentField* pSpecificInstrument,
                                   CThostFtdcRspInfoField*            pRspInfo,
                                   int32                              nRequestID,
                                   bool                               bIsLast) {
    if (pRspInfo && pRspInfo->ErrorID != 0) {
        ELOG("Ctp SubForQuote failed! RequestID:{},IsLast:{},ErrorID:{},ErrorMsg:{}",
             nRequestID,
             bIsLast,
             pRspInfo->ErrorID,
             pRspInfo->ErrorMsg);
    } else if (pSpecificInstrument) {
        ILOG("Ctp SubForQuote success! RequestID:{},IsLast:{},InstrumentID:{}",
             nRequestID,
             bIsLast,
             pSpecificInstrument->InstrumentID);
    }
}

void CtpMdSpi::OnRspUnSubForQuoteRsp(CThostFtdcSpecificInstrumentField* pSpecificInstrument,
                                     CThostFtdcRspInfoField*            pRspInfo,
                                     int32                              nRequestID,
                                     bool                               bIsLast) {
    if (pRspInfo && pRspInfo->ErrorID != 0) {
        ELOG("Ctp UnSubForQuote failed! RequestID:{},IsLast:{},ErrorID:{},ErrorMsg:{}",
             nRequestID,
             bIsLast,
             pRspInfo->ErrorID,
             pRspInfo->ErrorMsg);
    } else if (pSpecificInstrument) {
        ILOG("Ctp UnSubForQuote success! RequestID:{},IsLast:{},InstrumentID:{}",
             nRequestID,
             bIsLast,
             pSpecificInstrument->InstrumentID);
    }
}

void CtpMdSpi::OnRtnDepthMarketData(CThostFtdcDepthMarketDataField* pDepthMarketData) {
    // ELOG("Ctp receive MarketData.");
    if (pDepthMarketData) {
        DLOG("Ctp receive MarketData.");
    }
    // clang-format off
    DLOG(
        "TradingDay:{},InstrumentID;{},ExchangeID:{},ExchangeInstID:{},LastPrice:{},PreSettlementPrice:{},PreClosePrice:{},PreOpenInterest:{},OpenPrice:{},\
            HighestPrice:{},LowestPrice:{},Volume:{},Turnover:{},OpenInterest:{},ClosePrice:{},SettlementPrice:{},UpperLimitPrice:{},LowerLimitPrice:{},PreDelta:{},CurrDelta:{},\
            UpdateTime:{},UpdateMillisec:{},BidPrice1:{},BidVolume1:{},AskPrice1:{},AskVolume1:{},BidPrice2:{},BidVolume2:{},AskPrice2:{},AskVolume2:{},AveragePrice:{},\
            ActionDay:{}\
             ",
        pDepthMarketData->TradingDay,pDepthMarketData->InstrumentID,pDepthMarketData->ExchangeID,pDepthMarketData->ExchangeInstID, pDepthMarketData->LastPrice,pDepthMarketData->PreSettlementPrice,
        pDepthMarketData->PreClosePrice,pDepthMarketData->PreOpenInterest,pDepthMarketData->OpenPrice,pDepthMarketData->HighestPrice, pDepthMarketData->LowestPrice,pDepthMarketData->Volume,
        pDepthMarketData->Turnover,pDepthMarketData->OpenInterest,pDepthMarketData->ClosePrice,pDepthMarketData->SettlementPrice, pDepthMarketData->UpperLimitPrice,pDepthMarketData->LowerLimitPrice,
        pDepthMarketData->PreDelta,pDepthMarketData->CurrDelta,pDepthMarketData->UpdateTime,pDepthMarketData->UpdateMillisec,pDepthMarketData->BidPrice1, pDepthMarketData->BidVolume1,pDepthMarketData->AskPrice1,
        pDepthMarketData->AskVolume1,pDepthMarketData->BidPrice2,pDepthMarketData->BidVolume2,pDepthMarketData->AskPrice2, pDepthMarketData->AskVolume2,pDepthMarketData->AveragePrice,
        pDepthMarketData->ActionDay   );
    // clang-format on
    std::lock_guard<utils::spinlock> guard(lock_);
    if (on_data_fun_) {
        //Not tradeTime:vol is zero,maybe use time judge but if exist situation like :
        //{
        //"_id" : ObjectId("5b6e39885e6d8a0814002459"),
        //"id" : "TF1812",
        //"actionDate" : "2018-08-11",
        //"actionTime" : "09:19:00",
        //"exchange" : "",
        //"high" : 98.03,
        //"close" : 98.03,
        //"open" : 98.03,
        //"low" : 98.03,
        //"volume" : 0,
        //"marketVol" : 0,
        //"highestPrice" : 1.79769313486232e+308,
        //"lowestPrice" : 1.79769313486232e+308,
        //"openPrice" : 1.79769313486232e+308,
        //"preSettlementPrice" : 98.055,
        //"preClosePrice" : 98.03,
        //"Turnover" : 0.0,
        //"PreOpenInterest" : 10933.0,
        //"OpenInterest" : 10933.0,
        //"UpperLimitPrice" : 99.23,
        //"LowerLimitPrice" : 96.88,
        //"mdTradingDay" : "20180813",
        //"mdUpdateTime" : "18:11:16",
        //"recordTime"
        //: ISODate("2018-08-11T09:19:00.000Z")
        //}
        if (pDepthMarketData->Volume != 0) {
        
            std::invoke(on_data_fun_, pDepthMarketData);
        }
    }
}

void CtpMdSpi::OnRtnForQuoteRsp(CThostFtdcForQuoteRspField* pForQuoteRsp) {
    if (pForQuoteRsp) {
        DLOG("Ctp receive QuoteRsp.");
    }
    std::lock_guard<utils::spinlock> guard(lock_);
    if (on_quote_fun_) {
        std::invoke(on_quote_fun_, pForQuoteRsp);
    }
}

void CtpMdSpi::clearCallback() {
    std::lock_guard<utils::spinlock> guard(lock_);
    on_connected_fun_    = {};
    on_disconnected_fun_ = {};
    on_login_fun_        = {};
    on_error_fun_        = {};
    on_sub_fun_          = {};
    on_unsub_fun_        = {};
    on_data_fun_         = {};
    on_quote_fun_        = {};
}
