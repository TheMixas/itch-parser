# itch-parser

Nasdaq ITCH 5.0 binary protocol parser. Parses a full trading day (268M messages) at **170M msg/sec** warm, **58M msg/sec** cold.

Zero-copy `mmap` ingestion, SoupBinTCP framing, all 22 message types parsed and dispatched.

---

## What it does

Reads Nasdaq's binary ITCH 5.0 market data format directly from disk using `mmap` — no intermediate copies, no buffering. Walks the SoupBinTCP frame stream, dispatches each message type via a jump table, and accumulates per-type counts.

The parser is the foundation for [order-book](https://github.com/TheMixas/order-book) — a limit order book that consumes ITCH messages to reconstruct the full Nasdaq book in real time.

---

## Build

```bash
make release    # build optimised binary
make debug      # build debug binary
make run        # run release binary
make clean      # remove build directories
```

Requires: `cmake >= 3.28`, `gcc >= 13`.

---

## Data

Download Nasdaq ITCH 5.0 sample files from:
`https://emi.nasdaq.com/ITCH/Nasdaq%20ITCH/`

```bash
mkdir data
wget -P data https://emi.nasdaq.com/ITCH/Nasdaq%20ITCH/12302019.NASDAQ_ITCH50.gz
gunzip data/12302019.NASDAQ_ITCH50.gz
```

# itch-parser

Nasdaq ITCH 5.0 binary protocol parser. Parses a full trading day (268M messages) at **170M msg/sec** warm, **58M msg/sec** cold.

Zero-copy `mmap` ingestion, SoupBinTCP framing, all 22 message types parsed and dispatched.

---

## What it does

Reads Nasdaq's binary ITCH 5.0 market data format directly from disk using `mmap` — no intermediate copies, no buffering. Walks the SoupBinTCP frame stream, dispatches each message type via a jump table, and accumulates per-type counts.

The parser is the foundation for [order-book](https://github.com/TheMixas/order-book) — a limit order book that consumes ITCH messages to reconstruct the full Nasdaq book in real time.

---

## Benchmarks

Tested on Dec 30 2019 Nasdaq ITCH 5.0 sample (5.5 GB, 268,744,780 messages).  
Environment: WSL2, GCC 13.3.0, `-O3 -march=native`.

| Run | Throughput | Notes |
|-----|-----------|-------|
| Cold | 58M msg/sec | First run, OS faulting pages from disk |
| Warm | 170M msg/sec | Page cache hot, bottleneck is RAM bandwidth |

The ~3x difference between cold and warm is entirely OS page cache — the parse loop itself is memory-bandwidth bound, not compute bound. `-O3 -march=native` yields <1% improvement over `-O2` because there is no compute to optimise: the CPU spends >95% of its time waiting for RAM reads.

---

## Message counts — Dec 30 2019

| Type | Message | Count |
|------|---------|-------|
| `A` | AddOrder | 117,145,568 |
| `D` | OrderDelete | 114,360,997 |
| `U` | OrderReplace | 21,639,067 |
| `L` | MarketParticipantPosition | 215,161 |
| `I` | NOII | 4,024,315 |
| `E` | OrderExecuted | 5,722,824 |
| `P` | TradeNonCross | 1,218,602 |
| `X` | OrderCancel | 2,787,676 |
| `F` | AddOrderMPID | 1,485,888 |
| `C` | OrderExecutedPrice | 99,917 |
| `R` | StockDirectory | 8,906 |
| `H` | StockTradingAction | 8,966 |
| `Y` | RegSHORestriction | 9,013 |
| `Q` | CrossTrade | 17,836 |
| `V` | MWCBDeclineLevel | 1 |
| `S` | SystemEvent | 6 |
| **Total** | | **268,744,780** |

Only ~5% of added orders result in an execution. The rest are cancelled or replaced — characteristic of HFT market-making activity.

---

## Design notes

**Zero-copy mmap** — the file is mapped directly into virtual address space. The OS page cache is the buffer. No `read()` calls, no memcpy, no intermediate allocation.

**SoupBinTCP framing** — each message is preceded by a 2-byte big-endian length prefix. The parser trusts the wire length for advancing the pointer; the internal message-length table is used only as a sanity check, never for control flow. This means the parser survives protocol extensions without breaking.

**`#pragma pack(1)` structs** — all message types are packed structs that overlay directly onto the raw bytes. Casting `const uint8_t*` to a struct pointer is zero-overhead — no deserialisation step.

**Big-endian reads** — Nasdaq ITCH is big-endian. All multi-byte fields are read with explicit byte-shift functions (`read_be16`, `read_be32`, `read_be64`) rather than relying on `ntohl` or platform endianness.

---

## Data

Download Nasdaq ITCH 5.0 sample files from:  
`https://emi.nasdaq.com/ITCH/Nasdaq%20ITCH/`

```bash
wget https://emi.nasdaq.com/ITCH/Nasdaq%20ITCH/12302019.NASDAQ_ITCH50.gz
gunzip 12302019.NASDAQ_ITCH50.gz
```

---

## Build

```bash
cmake -B cmake-build-release -DCMAKE_BUILD_TYPE=Release
cmake --build cmake-build-release
./cmake-build-release/itch_parser
```
Place the decompressed file at `data/12302019.NASDAQ_ITCH50`. The `data/` directory is gitignored.

---

## Benchmarks

Tested on Dec 30 2019 Nasdaq ITCH 5.0 sample (5.5 GB, 268,744,780 messages).  
Environment: WSL2, GCC 13.3.0, `-O3 -march=native`.

| Run | Throughput | Notes |
|-----|-----------|-------|
| Cold | 58M msg/sec | First run, OS faulting pages from disk |
| Warm | 170M msg/sec | Page cache hot, bottleneck is RAM bandwidth |

The ~3x difference between cold and warm is entirely OS page cache — the parse loop itself is memory-bandwidth bound, not compute bound. `-O3 -march=native` yields <1% improvement over `-O2` because there is no compute to optimise: the CPU spends >95% of its time waiting for RAM reads.

---

## Message counts — Dec 30 2019

| Type | Message | Count |
|------|---------|-------|
| `A` | AddOrder | 117,145,568 |
| `D` | OrderDelete | 114,360,997 |
| `U` | OrderReplace | 21,639,067 |
| `L` | MarketParticipantPosition | 215,161 |
| `I` | NOII | 4,024,315 |
| `E` | OrderExecuted | 5,722,824 |
| `P` | TradeNonCross | 1,218,602 |
| `X` | OrderCancel | 2,787,676 |
| `F` | AddOrderMPID | 1,485,888 |
| `C` | OrderExecutedPrice | 99,917 |
| `R` | StockDirectory | 8,906 |
| `H` | StockTradingAction | 8,966 |
| `Y` | RegSHORestriction | 9,013 |
| `Q` | CrossTrade | 17,836 |
| `V` | MWCBDeclineLevel | 1 |
| `S` | SystemEvent | 6 |
| **Total** | | **268,744,780** |

Only ~5% of added orders result in an execution. The rest are cancelled or replaced — characteristic of HFT market-making activity.

---

## Design notes

**Zero-copy mmap** — the file is mapped directly into virtual address space. The OS page cache is the buffer. No `read()` calls, no memcpy, no intermediate allocation.

**SoupBinTCP framing** — each message is preceded by a 2-byte big-endian length prefix. The parser trusts the wire length for advancing the pointer; the internal message-length table is used only as a sanity check, never for control flow. This means the parser survives protocol extensions without breaking.

**`#pragma pack(1)` structs** — all message types are packed structs that overlay directly onto the raw bytes. Casting `const uint8_t*` to a struct pointer is zero-overhead — no deserialisation step.

**Big-endian reads** — Nasdaq ITCH is big-endian. All multi-byte fields are read with explicit byte-shift functions (`read_be16`, `read_be32`, `read_be64`) rather than relying on `ntohl` or platform endianness.
