#include <cstdint>
#include <chrono>
#include <cstring>
#include <iostream>
#include <array>
#include "MappedFile.h"

uint16_t read_be16(const uint8_t *p) {
    // combine p[0] and p[1] into a uint16_t, big-endian
    // hint: the first byte needs to be shifted left by 8 bits
    uint16_t result = (p[0] << 8) | p[1];
    return result;
}

uint32_t read_be32(const uint8_t *p) {
    // combine p[0] and p[1] into a uint16_t, big-endian
    // hint: the first byte needs to be shifted left by 8 bits
    uint32_t result = (p[0] << 24) | (p[1] << 16) | (p[2] << 8) | (p[3]);
    return result;
}

uint64_t read_be64(const uint8_t *p) {
    // combine p[0] and p[1] into a uint16_t, big-endian
    // hint: the first byte needs to be shifted left by 8 bits
    uint64_t result = (uint64_t) p[0] << 56 | (uint64_t) p[1] << 48 | (uint64_t) p[2] << 40 | (uint64_t) p[3] << 32 | (
                          uint64_t) p[4] << 24 | (uint64_t) p[5] << 16 | (uint64_t) p[6] << 8 | (uint64_t) (p[7]);
    return result;
}
size_t message_length(char type) {
    static const auto table = []() {
        std::array<size_t, 256> t{};
        t['S'] = 12;  // SystemEvent
        t['R'] = 39;  // StockDirectory
        t['H'] = 25;  // StockTradingAction
        t['Y'] = 20;  // RegSHORestriction
        t['L'] = 26;  // MarketParticipantPosition
        t['V'] = 35;  // MWCBDeclineLevel
        t['W'] = 12;  // MWCBStatus
        t['K'] = 28;  // QuotingPeriodUpdate
        t['J'] = 35;  // LULDAuctionCollar
        t['h'] = 21;  // OperationalHalt
        t['A'] = 36;  // AddOrder
        t['F'] = 40;  // AddOrderMPID
        t['E'] = 31;  // OrderExecuted
        t['C'] = 36;  // OrderExecutedPrice
        t['X'] = 23;  // OrderCancel
        t['D'] = 19;  // OrderDelete
        t['U'] = 35;  // OrderReplace
        t['P'] = 44;  // TradeNonCross
        t['Q'] = 40;  // CrossTrade
        t['B'] = 19;  // BrokenTrade
        t['I'] = 50;  // NOII
        t['N'] = 20;  // RPII
        return t;
    }();

    size_t len = table[(unsigned char)type];
    if (len == 0) [[unlikely]] {
        // In production: log and skip using the SoupBinTCP frame length instead
        // Never derive skip distance from this table for unknown types
        throw std::runtime_error(
            std::string("Unknown ITCH message type: ") + type
        );
    }
    return len;
}
#pragma pack(push, 1)
struct Header {
    char msg_type; // offset 0,  1 byte
    uint16_t stock_locate; // offset 1,  2 bytes
    uint16_t tracking_number; // offset 3   2 bytes
    uint16_t timestamp_high; // offset 5 2 bytes
    uint32_t timestamp_low; // offset 7 4 bytes
};

static_assert(sizeof(Header) == 11);

struct StockTradingAction : Header {
    char stock[8]; // offset 11 8 bytes
    char trading_state; // offset 19 1 bytes
    char reserved; // offset 20 1 bytes
    char reason[4]; //offset 21 4 bytes
};

static_assert(sizeof(StockTradingAction) == 25);

struct RegSHORestriction : Header {
    char stock[8]; // offset 11 8 bytes
    char reg_sho_action; // offset 19 1 bytes
};

static_assert(sizeof(RegSHORestriction) == 20);

struct MarketParticipantPosition : Header {
    char mpid[4]; // offset 11 4 bytes
    char stock[8]; // offset 15, 8 bytes
    char primary_market_maker; //offset 23, 1 bytes
    char market_maker_mode; // offset 24, 1 byte
    char market_participant_state; //offset 25, 1 byte
};

static_assert(sizeof(MarketParticipantPosition) == 26);

struct
MWCBDeclineLevel : Header {
    uint64_t level1; // offset 11 8 bytes (PRICE8)
    uint64_t level2; // offset 19 8 bytes (PRICE8)
    uint64_t level3; // offset 27 8 bytes (PRICE8)
};

static_assert(sizeof(MWCBDeclineLevel) == 35);

struct MWCBStatus : Header {
    uint8_t breached_level; // offset 11 1 bytes
};

static_assert(sizeof(MWCBStatus) == 12);

struct QuotingPeriodUpdate : Header {
    char stock[8]; // offset 11 8 bytes
    uint32_t ipq_quotation_release_time; // offset 19 4 bytes
    char ipq_quotation_release_qualifier; //offset 23, 1byte
    uint32_t ipo_price; //offset 24, 4 byte PRICE(4)
};

static_assert(sizeof(QuotingPeriodUpdate) == 28);

struct LULDAuctionCollar : Header {
    char stock[8]; // offset 11 8 bytes
    uint32_t auction_collar_reference_price; //19, 4 bytes (PRICE4)
    uint32_t upper_auction_collar_price; //23, 4 bytes (PRICE4)
    uint32_t lower_auction_collar_price; //27, 4 bytes (PRICE4)
    uint32_t auction_collar_extension; //31, 4 bytes
    };

static_assert(sizeof(LULDAuctionCollar) == 35);

struct OperationalHalt : Header {
    char stock[8]; // offset 11 8 bytes
    char market_code; //offset 19, 1 byte
    char operation_halt_action; // offset 20, 1 byte
};
static_assert(sizeof(OperationalHalt) == 21);

struct AddOrder : Header {
    uint64_t order_ref_num; // offset 11 8 bytes
    char side; // offset 19 1 byte
    uint32_t shares; // offset 20 4 bytes
    char stock[8]; // offset 24 8 bytes
    uint32_t price; // offset 32 4 bytes
};
static_assert(sizeof(AddOrder) == 36);

struct AddOrderMPID : AddOrder {
    char attribution[4];
};
static_assert(sizeof(AddOrderMPID) == 40);

struct OrderExecuted : Header {
    uint64_t order_ref_num; // offset 11 8 bytes
    uint32_t executed_shares; // offset 19 4
    uint64_t match_number; // offset 23 8 bytes
};
static_assert(sizeof(OrderExecuted) == 31);

struct OrderExecutedPrice : OrderExecuted {
    char printable;
    uint32_t execution_price; // price4
};
static_assert(sizeof(OrderExecutedPrice) == 36);

struct OrderCancel : Header {
    uint64_t order_ref_num; // offset 11 8 bytes
    uint32_t cancelled_shares; // offset 19 4
};
static_assert(sizeof(OrderCancel) == 23);

struct OrderDelete : Header {
    uint64_t order_ref_num; // offset 11 8 bytes
};
static_assert(sizeof(OrderDelete) == 19);

struct OrderReplace : Header {
    uint64_t order_ref_num; // offset 11 8 bytes
    uint64_t new_order_ref_num; // offset 19 8
    uint32_t shares; // offset 27, 4
    uint32_t price; // offset 31,4
};
uint64_t counts[256] = {};
void par_itch_file(const MappedFile& f) {
    const uint8_t* ptr = f.data;
    const uint8_t* end = f.data + f.size;

    while (ptr + 2 <= end) {
        uint16_t msg_len = read_be16(ptr);
        ptr += 2;

        if (ptr + msg_len > end) break;

        char type = static_cast<char>(*ptr);

        switch (type) {
            case 'S': { auto* m = reinterpret_cast<const Header*>(ptr);                  counts['S']++; break; }
            case 'R': { auto* m = reinterpret_cast<const Header*>(ptr);                  counts['R']++; break; }
            case 'H': { auto* m = reinterpret_cast<const StockTradingAction*>(ptr);      counts['H']++; break; }
            case 'Y': { auto* m = reinterpret_cast<const RegSHORestriction*>(ptr);       counts['Y']++; break; }
            case 'L': { auto* m = reinterpret_cast<const MarketParticipantPosition*>(ptr); counts['L']++; break; }
            case 'V': { auto* m = reinterpret_cast<const MWCBDeclineLevel*>(ptr);        counts['V']++; break; }
            case 'W': { auto* m = reinterpret_cast<const MWCBStatus*>(ptr);              counts['W']++; break; }
            case 'K': { auto* m = reinterpret_cast<const QuotingPeriodUpdate*>(ptr);     counts['K']++; break; }
            case 'J': { auto* m = reinterpret_cast<const LULDAuctionCollar*>(ptr);       counts['J']++; break; }
            case 'h': { auto* m = reinterpret_cast<const OperationalHalt*>(ptr);         counts['h']++; break; }
            case 'A': { auto* m = reinterpret_cast<const AddOrder*>(ptr);                counts['A']++; break; }
            case 'F': { auto* m = reinterpret_cast<const AddOrderMPID*>(ptr);            counts['F']++; break; }
            case 'E': { auto* m = reinterpret_cast<const OrderExecuted*>(ptr);           counts['E']++; break; }
            case 'C': { auto* m = reinterpret_cast<const OrderExecutedPrice*>(ptr);      counts['C']++; break; }
            case 'X': { auto* m = reinterpret_cast<const OrderCancel*>(ptr);             counts['X']++; break; }
            case 'D': { auto* m = reinterpret_cast<const OrderDelete*>(ptr);             counts['D']++; break; }
            case 'U': { auto* m = reinterpret_cast<const OrderReplace*>(ptr);            counts['U']++; break; }
            case 'P': { auto* m = reinterpret_cast<const Header*>(ptr);                  counts['P']++; break; }
            case 'Q': { auto* m = reinterpret_cast<const Header*>(ptr);                  counts['Q']++; break; }
            case 'B': { auto* m = reinterpret_cast<const Header*>(ptr);                  counts['B']++; break; }
            case 'I': { auto* m = reinterpret_cast<const Header*>(ptr);                  counts['I']++; break; }
            case 'N': { auto* m = reinterpret_cast<const Header*>(ptr);                  counts['N']++; break; }
            default:  { break; }
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
static_assert(sizeof(OrderReplace) == 35);
#pragma pack(pop)
int main() {
    auto start = std::chrono::high_resolution_clock::now();
    std::string NASDAQ_filepath = "/home/themi/CLionProjects/itch-parser/12302019.NASDAQ_ITCH50";
    MappedFile file(NASDAQ_filepath);
    par_itch_file(file);
    auto end = std::chrono::high_resolution_clock::now();
    double seconds = std::chrono::duration<double>(end - start).count();
    uint64_t total = 0;
    for (auto c : counts) total += c;
    std::cout << "Total messages: " << total << "\n";
    std::cout << "Elapsed: " << seconds << "s\n";
    std::cout << "Throughput: " << (total / seconds / 1e6) << "M msg/sec\n";
    //std::cout << "Throughput: " << (total / seconds / 1e6) << "M msg/sec\n";
}
// int main() {
//     std::cout << "Hello, World!" << std::endl;
//     std::uint8_t add_order_buffer[36];
//
//     // offset 0, length 1 — Message Type
//     add_order_buffer[0] = 0x41;
//
//     // offset 1, length 2 — Stock Locate = 5
//     add_order_buffer[1] = 0x00;
//     add_order_buffer[2] = 0x05;
//
//     // offset 3, length 2 — Tracking Number = 0
//     add_order_buffer[3] = 0x00;
//     add_order_buffer[4] = 0x00;
//
//     // offset 5, length 6 — Timestamp = 0
//     add_order_buffer[5] = 0x00;
//     add_order_buffer[6] = 0x00;
//     add_order_buffer[7] = 0x00;
//     add_order_buffer[8] = 0x00;
//     add_order_buffer[9] = 0x00;
//     add_order_buffer[10] = 0x00;
//
//     // offset 11, length 8 -  Order Reference Number
//     add_order_buffer[11] = 0x00; // 50
//     add_order_buffer[12] = 0x00;
//     add_order_buffer[13] = 0x00;
//     add_order_buffer[14] = 0x00;
//     add_order_buffer[15] = 0x00;
//     add_order_buffer[16] = 0x00;
//     add_order_buffer[17] = 0x00;
//     add_order_buffer[18] = 0x32;
//
//     //offset 19, length 1 Buy/Sell Indicator
//     add_order_buffer[19] = 0x42; // 'b' buy
//
//     //offset 20, length 4 - Shares
//     add_order_buffer[20] = 0x00;
//     add_order_buffer[21] = 0x00;
//     add_order_buffer[22] = 0x00;
//     add_order_buffer[23] = 0x32;
//
//     //offset 24, length 8 - stock symbol // G    O    O    G    L
//     add_order_buffer[24] = 0x47; //G
//     add_order_buffer[25] = 0x4F; //O
//     add_order_buffer[26] = 0x4F; //O
//     add_order_buffer[27] = 0x47; //G
//     add_order_buffer[28] = 0x4C; //L
//     add_order_buffer[29] = 0x20; //(SPACE)
//     add_order_buffer[30] = 0x20; //(SPACE)
//     add_order_buffer[31] = 0x20; //(SPACE)
//
//     //offset 32, length 4 - price as integer (PRICE4) $250.75 * 10000 = 2,507,500
//     add_order_buffer[32] = 0x00;
//     add_order_buffer[33] = 0x26;
//     add_order_buffer[34] = 0x42;
//     add_order_buffer[35] = 0xEC;
//
//     std::uint8_t order_delete_buffer[19];
//     order_delete_buffer[0] = 0x44; //order delete message
//
//     order_delete_buffer[1] = 0x00; //locate code
//     order_delete_buffer[2] = 0x04;
//
//     order_delete_buffer[3] = 0x00; //tracking num
//     order_delete_buffer[4] = 0x01;
//
//     order_delete_buffer[5] = 0x00; //timestamp
//     order_delete_buffer[6] = 0x00;
//     order_delete_buffer[7] = 0x00;
//     order_delete_buffer[8] = 0x00;
//     order_delete_buffer[9] = 0x00;
//     order_delete_buffer[10] = 0x02;
//
//sdasdi     order_delete_buffer[11] = 0x00; // order ref num 1
//     order_delete_buffer[12] = 0x00;
//     order_delete_buffer[13] = 0x00;
//     order_delete_buffer[14] = 0x00;
//     order_delete_buffer[15] = 0x00;
//     order_delete_buffer[16] = 0x00;
//     order_delete_buffer[17] = 0x00;
//     order_delete_buffer[18] = 0x01;
//
//
//     auto *msg = reinterpret_cast<const AddOrder *>(add_order_buffer);
//
//     std::cout << "type:    " << msg->msg_type << std::endl;
//     std::cout << "locate:  " << read_be16((const uint8_t *) &msg->stock_locate) << std::endl;
//     std::cout << "ref:     " << read_be64((const uint8_t *) &msg->order_ref_num) << std::endl;
//     std::cout << "side:    " << msg->side << std::endl;
//     std::cout << "shares:  " << read_be32((const uint8_t *) &msg->shares) << std::endl;
//     std::cout << "stock symbol: " << std::string{msg->stock, 8} << std::endl;
//     std::cout << "price: " << read_be32((const uint8_t *) &msg->price) << std::endl;
//
//     auto *delete_order_msg = reinterpret_cast<const OrderDelete *>(order_delete_buffer);
//
//     std::cout << " --  Reading Order Delete -- " << std::endl;
//     std::cout << "type:    " << delete_order_msg->msg_type << std::endl;
//     std::cout << "locate:     " << read_be16((const uint8_t *) &delete_order_msg->stock_locate) << std::endl;
//     std::cout << "tracking num:     " << read_be16((const uint8_t *) &delete_order_msg->tracking_number) << std::endl;
//     std::cout << "timestamp: " << read_be16((const uint8_t *) &delete_order_msg->timestamp_high) + read_be32(
//         (const uint8_t *) &delete_order_msg->timestamp_low) << std::endl;
//     std::cout << "order ref num: " << read_be64((const uint8_t *) &delete_order_msg->order_ref_num) << std::endl;
//
//     uint8_t stream[55];
//     std::memcpy(stream, add_order_buffer, 36);
//     std::memcpy(stream + 36, delete_order_msg, 19);
//
//     const uint8_t *ptr = stream;
//     const uint8_t *end = stream + 55;
//
//     // based on type, figure out the message length
//     // cast to the right struct, print the fields
//     // advance ptr by the message length
//     while (ptr < end) {
//         char type = (char) *ptr;
//         switch (type) {
//             case 'A': {
//                 auto add_order_message = reinterpret_cast<const AddOrder *>(ptr);
//                 std::cout << "Received add order wit order ref num:" << read_be64(
//                     (const uint8_t *) &add_order_message->order_ref_num) << std::endl;
//                 ptr += sizeof(AddOrder);
//                 break;
//             }
//             case 'D': {
//                 auto delete_order_message = reinterpret_cast<const OrderDelete *>(ptr);
//                 std::cout << "Received delete order with order ref num: " << read_be64(
//                     (const uint8_t *) &delete_order_message->order_ref_num) << std::endl;
//                 ptr += sizeof(OrderDelete);
//                 break;
//             }
//             default: {
//                 std::cout << "Unknown type: " << type << std::endl;
//                 ptr += 1;
//                 break;
//             }
//         }
//     }
//     return 0;
// }

