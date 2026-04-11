#pragma once

#include <cstdint>

#pragma pack(push, 1)

struct Header {
    char     msg_type;
    uint16_t stock_locate;
    uint16_t tracking_number;
    uint16_t timestamp_high;
    uint32_t timestamp_low;
};
static_assert(sizeof(Header) == 11);

struct SystemEvent : Header {
    char event_code;
};
static_assert(sizeof(SystemEvent) == 12);

struct StockDirectory : Header {
    char     stock[8];
    char     market_category;
    char     financial_status_indicator;
    uint32_t round_lot_size;
    char     round_lots_only;
    char     issue_classification;
    char     issue_sub_type[2];
    char     authenticity;
    char     short_sale_threshold_indicator;
    char     ipo_flag;
    char     luld_reference_price_tier;
    char     etp_flag;
    uint32_t etp_leverage_factor;
    char     inverse_indicator;
};
static_assert(sizeof(StockDirectory) == 39);

struct StockTradingAction : Header {
    char stock[8];
    char trading_state;
    char reserved;
    char reason[4];
};
static_assert(sizeof(StockTradingAction) == 25);

struct RegSHORestriction : Header {
    char stock[8];
    char reg_sho_action;
};
static_assert(sizeof(RegSHORestriction) == 20);

struct MarketParticipantPosition : Header {
    char mpid[4];
    char stock[8];
    char primary_market_maker;
    char market_maker_mode;
    char market_participant_state;
};
static_assert(sizeof(MarketParticipantPosition) == 26);

struct MWCBDeclineLevel : Header {
    uint64_t level1;
    uint64_t level2;
    uint64_t level3;
};
static_assert(sizeof(MWCBDeclineLevel) == 35);

struct MWCBStatus : Header {
    uint8_t breached_level;
};
static_assert(sizeof(MWCBStatus) == 12);

struct QuotingPeriodUpdate : Header {
    char     stock[8];
    uint32_t ipq_quotation_release_time;
    char     ipq_quotation_release_qualifier;
    uint32_t ipo_price;
};
static_assert(sizeof(QuotingPeriodUpdate) == 28);

struct LULDAuctionCollar : Header {
    char     stock[8];
    uint32_t auction_collar_reference_price;
    uint32_t upper_auction_collar_price;
    uint32_t lower_auction_collar_price;
    uint32_t auction_collar_extension;
};
static_assert(sizeof(LULDAuctionCollar) == 35);

struct OperationalHalt : Header {
    char stock[8];
    char market_code;
    char operation_halt_action;
};
static_assert(sizeof(OperationalHalt) == 21);

struct AddOrder : Header {
    uint64_t order_ref_num;
    char     side;
    uint32_t shares;
    char     stock[8];
    uint32_t price;
};
static_assert(sizeof(AddOrder) == 36);

struct AddOrderMPID : AddOrder {
    char attribution[4];
};
static_assert(sizeof(AddOrderMPID) == 40);

struct OrderExecuted : Header {
    uint64_t order_ref_num;
    uint32_t executed_shares;
    uint64_t match_number;
};
static_assert(sizeof(OrderExecuted) == 31);

struct OrderExecutedPrice : OrderExecuted {
    char     printable;
    uint32_t execution_price;
};
static_assert(sizeof(OrderExecutedPrice) == 36);

struct OrderCancel : Header {
    uint64_t order_ref_num;
    uint32_t cancelled_shares;
};
static_assert(sizeof(OrderCancel) == 23);

struct OrderDelete : Header {
    uint64_t order_ref_num;
};
static_assert(sizeof(OrderDelete) == 19);

struct OrderReplace : Header {
    uint64_t order_ref_num;
    uint64_t new_order_ref_num;
    uint32_t shares;
    uint32_t price;
};
static_assert(sizeof(OrderReplace) == 35);

// TODO: TradeNonCross ('P') — 44 bytes
// TODO: CrossTrade    ('Q') — 40 bytes
// TODO: BrokenTrade   ('B') — 19 bytes
// TODO: NOII          ('I') — 50 bytes
// TODO: RPII          ('N') — 20 bytes

#pragma pack(pop)
