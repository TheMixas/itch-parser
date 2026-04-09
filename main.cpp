#include <cstdint>
#include <chrono>
#include <cstring>
#include <iostream>
#include <array>
#include "MappedFile.h"

uint16_t read_be16(const uint8_t *p) {
    return (p[0] << 8) | p[1];
}

uint32_t read_be32(const uint8_t *p) {
    return (p[0] << 24) | (p[1] << 16) | (p[2] << 8) | p[3];
}

uint64_t read_be64(const uint8_t *p) {
    return (uint64_t)p[0] << 56 | (uint64_t)p[1] << 48 | (uint64_t)p[2] << 40 | (uint64_t)p[3] << 32
         | (uint64_t)p[4] << 24 | (uint64_t)p[5] << 16 | (uint64_t)p[6] <<  8 | (uint64_t)p[7];
}

size_t message_length(char type) {
    static const auto table = []() {
        std::array<size_t, 256> t{};
        t['S'] = 12; t['R'] = 39; t['H'] = 25; t['Y'] = 20;
        t['L'] = 26; t['V'] = 35; t['W'] = 12; t['K'] = 28;
        t['J'] = 35; t['h'] = 21; t['A'] = 36; t['F'] = 40;
        t['E'] = 31; t['C'] = 36; t['X'] = 23; t['D'] = 19;
        t['U'] = 35; t['P'] = 44; t['Q'] = 40; t['B'] = 19;
        t['I'] = 50; t['N'] = 20;
        return t;
    }();

    size_t len = table[(unsigned char)type];
    if (len == 0) [[unlikely]]
        throw std::runtime_error(std::string("Unknown ITCH message type: ") + type);
    return len;
}

#pragma pack(push, 1)

struct Header {
    char     msg_type;
    uint16_t stock_locate;
    uint16_t tracking_number;
    uint16_t timestamp_high;
    uint32_t timestamp_low;
};
static_assert(sizeof(Header) == 11);

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

#pragma pack(pop)

void parse_itch_file(const MappedFile& f) {
    uint64_t counts[256] = {};
    const uint8_t* ptr = f.data;
    const uint8_t* end = f.data + f.size;

    while (ptr + 2 <= end) {
        uint16_t msg_len = read_be16(ptr);
        ptr += 2;

        if (ptr + msg_len > end) break;

        char type = static_cast<char>(*ptr);

        switch (type) {
            case 'S': { reinterpret_cast<const Header*>(ptr);                    counts['S']++; break; }
            case 'R': { reinterpret_cast<const Header*>(ptr);                    counts['R']++; break; }
            case 'H': { reinterpret_cast<const StockTradingAction*>(ptr);        counts['H']++; break; }
            case 'Y': { reinterpret_cast<const RegSHORestriction*>(ptr);         counts['Y']++; break; }
            case 'L': { reinterpret_cast<const MarketParticipantPosition*>(ptr); counts['L']++; break; }
            case 'V': { reinterpret_cast<const MWCBDeclineLevel*>(ptr);          counts['V']++; break; }
            case 'W': { reinterpret_cast<const MWCBStatus*>(ptr);                counts['W']++; break; }
            case 'K': { reinterpret_cast<const QuotingPeriodUpdate*>(ptr);       counts['K']++; break; }
            case 'J': { reinterpret_cast<const LULDAuctionCollar*>(ptr);         counts['J']++; break; }
            case 'h': { reinterpret_cast<const OperationalHalt*>(ptr);           counts['h']++; break; }
            case 'A': { reinterpret_cast<const AddOrder*>(ptr);                  counts['A']++; break; }
            case 'F': { reinterpret_cast<const AddOrderMPID*>(ptr);              counts['F']++; break; }
            case 'E': { reinterpret_cast<const OrderExecuted*>(ptr);             counts['E']++; break; }
            case 'C': { reinterpret_cast<const OrderExecutedPrice*>(ptr);        counts['C']++; break; }
            case 'X': { reinterpret_cast<const OrderCancel*>(ptr);               counts['X']++; break; }
            case 'D': { reinterpret_cast<const OrderDelete*>(ptr);               counts['D']++; break; }
            case 'U': { reinterpret_cast<const OrderReplace*>(ptr);              counts['U']++; break; }
            case 'P': { reinterpret_cast<const Header*>(ptr);                    counts['P']++; break; }
            case 'Q': { reinterpret_cast<const Header*>(ptr);                    counts['Q']++; break; }
            case 'B': { reinterpret_cast<const Header*>(ptr);                    counts['B']++; break; }
            case 'I': { reinterpret_cast<const Header*>(ptr);                    counts['I']++; break; }
            case 'N': { reinterpret_cast<const Header*>(ptr);                    counts['N']++; break; }
            default:  {                                                                          break; }
        }

        ptr += msg_len;
    }

    std::cout << "SystemEvent:              " << counts['S'] << "\n";
    std::cout << "StockDirectory:           " << counts['R'] << "\n";
    std::cout << "StockTradingAction:       " << counts['H'] << "\n";
    std::cout << "RegSHORestriction:        " << counts['Y'] << "\n";
    std::cout << "MarketParticipantPos:     " << counts['L'] << "\n";
    std::cout << "MWCBDeclineLevel:         " << counts['V'] << "\n";
    std::cout << "MWCBStatus:               " << counts['W'] << "\n";
    std::cout << "QuotingPeriodUpdate:      " << counts['K'] << "\n";
    std::cout << "LULDAuctionCollar:        " << counts['J'] << "\n";
    std::cout << "OperationalHalt:          " << counts['h'] << "\n";
    std::cout << "AddOrder:                 " << counts['A'] << "\n";
    std::cout << "AddOrderMPID:             " << counts['F'] << "\n";
    std::cout << "OrderExecuted:            " << counts['E'] << "\n";
    std::cout << "OrderExecutedPrice:       " << counts['C'] << "\n";
    std::cout << "OrderCancel:              " << counts['X'] << "\n";
    std::cout << "OrderDelete:              " << counts['D'] << "\n";
    std::cout << "OrderReplace:             " << counts['U'] << "\n";
    std::cout << "TradeNonCross:            " << counts['P'] << "\n";
    std::cout << "CrossTrade:               " << counts['Q'] << "\n";
    std::cout << "BrokenTrade:              " << counts['B'] << "\n";
    std::cout << "NOII:                     " << counts['I'] << "\n";
    std::cout << "RPII:                     " << counts['N'] << "\n";

    uint64_t total = 0;
    for (auto c : counts) total += c;
    std::cout << "Total:                    " << total << "\n";
}

int main() {
    const std::string path = "data/12302019.NASDAQ_ITCH50";

    MappedFile file(path);

    auto start = std::chrono::high_resolution_clock::now();
    parse_itch_file(file);
    auto stop = std::chrono::high_resolution_clock::now();

    double seconds = std::chrono::duration<double>(stop - start).count();
    uint64_t total = 0;

    std::cout << "Elapsed:    " << seconds << "s\n";
    std::cout << "Throughput: " << (268744780.0 / seconds / 1e6) << "M msg/sec\n";
}
