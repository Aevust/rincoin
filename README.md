# Rincoin Core

![Version](https://img.shields.io/badge/version-1.0.6-blue.svg)
![License](https://img.shields.io/badge/license-MIT-green.svg)

Rincoin is a decentralized digital currency derived from the Litecoin codebase (itself a long-established Bitcoin derivative) that introduces a new Proof-of-Work hashing algorithm called **RinHash**. RinHash is a hybrid PoW algorithm combining BLAKE3, Argon2d, and SHA3-256, designed to provide robust security while enabling broad, accessible participation during the network's formative phase. This README provides an overview of Rincoin's specifications, the RinHash algorithm, and network parameters.

## 🛡️ Core Architecture & Network Sovereignty

Rincoin Core has been engineered and reviewed with the goal of mathematical and network independence from its upstream codebases. Recent milestones include:

- **P2P Network Sovereignty (confirmed in both Sim and Core):** Legacy network identifiers have been replaced with Rincoin-native values across two distinct layers. (1) **Message-start magic bytes** `0x52 0x49 0x4E 0x43` ("RINC"), defined in `src/chainparams.cpp`. (2) **Internal IPv6 prefix** `INTERNAL_IN_IPV6_PREFIX = FD 2D DD 82 F5 C8` — `0xFD + sha256("rincoin")[0:5]`, replacing the upstream `FD 6B 88 C0 87 24` (`sha256("bitcoin")[0:5]`), confirmed in `src/netaddress.h` with inline comment. This prefix is used when serializing non-IP peers (Tor/I2P/CJDNS) in ADDR messages. `CNetAddr::SetInternal()` in `netaddress.cpp` is a separate mechanism that hashes individual DNS seed names per-peer for internal tracking — already Rincoin-native by using Rincoin's own seeds. Note: minor residual upstream naming artifacts remain in `netaddress.h` (`#ifndef BITCOIN_NETADDRESS_H`, `bitcoin-config.h` include, copyright header) but have no functional impact on network sovereignty.
- **Customized Consensus & Emission (Scenario II):** Rincoin implements a multi-phase emission schedule. It begins with 210,000-block intervals (~145 days) and dilates to multi-million-block epochs after height 840,000 to slow subsidy decay. It culminates in a perpetual terminal reward (0.6 RIN), which is intended to support a long-term security budget. Consensus rules, including the custom base58 address prefix (prefix `60`), are validated by the test suite.
- **Continuous Integration:** The active validation and utility test suites report a 100% PASS state. Legacy upstream benchmarks that depend on obsolete upstream block data have been decoupled as not applicable to Rincoin, keeping the CI pipeline stable for ongoing development.

## 📊 Key Specifications

| Feature | Specification |
| :--- | :--- |
| **Coin Name / Ticker** | Rincoin (**RIN**) |
| **Consensus Mechanism**| Proof-of-Work (PoW) – **RinHash** algorithm |
| **Block Target Time** | 1 minute (60 seconds per block) |
| **Initial Block Reward**| 50 RIN |
| **Emission Schedule** | Custom multi-phase (Initial: 210k blocks, Dilated: up to 2.1M blocks, Terminal: 0.6 RIN) |
| **Difficulty Retarget**| 1,980-block intervals until block 30,000; DGW (per-block) thereafter |
| **Proof-of-Work Hash** | 256-bit output |
| **Address Format** | Base58 addresses start with **R** (version byte `60`) |
| **Network Ports** | P2P: `9555`, RPC: `9556` |
| **Network Magic** | `0x52` `0x49` `0x4E` `0x43` ("RINC") |

> **Note:** All network parameters (difficulty retarget, magic bytes, address prefixes) are defined in `src/chainparams.cpp`, which is the single source of truth. Values documented here are for reference only.

### 📉 Emission Schedule (Customized Halving: Scenario II)

Rincoin implements a piecewise emission schedule intended to support long-term network sustainability and slow subsidy decay.

| Phase | Block Height | Reward (RIN) | Duration (Blocks) |
| :--- | :--- | :--- | :--- |
| **Phase 0** | 0 - 209,999 | 50 | 210,000 |
| **Phase 1** | 210,000 - 419,999 | 25 | 210,000 |
| **Phase 2** | 420,000 - 629,999 | 12.5 | 210,000 |
| **Phase 3** | 630,000 - 839,999 | 6.25 | 210,000 |
| **Phase 4** | 840,000 - 2,099,999 | 4 | 1,260,000 |
| **Phase 5** | 2,100,000 - 4,199,999 | 2 | 2,100,000 |
| **Phase 6** | 4,200,000 - 6,299,999 | 1 | 2,100,000 |
| **Terminal**| 6,300,000+ | 0.6 | Perpetual |

The Phase 4 transition at block 840,000 is a hard fork. See the technical roadmap and the corresponding Rincoin Improvement Proposal for the activation plan and required peer/version negotiation.

**Privacy layer (MWEB):** MWEB (MimbleWimble Extension Blocks) is implemented in the codebase. Per project policy, mainnet activation is intentionally deferred until the SQLite descriptor-wallet migration completes. The target state (validated in Rincoin-Sim) is `NEVER_ACTIVE` on mainnet. **Pending action for v1.1.0:** the current Core `chainparams.cpp` still carries Litecoin-inherited height-based MWEB activation values (`nStartHeight = 2,217,600`) and must be updated to `NEVER_ACTIVE` before the next mainnet release. Testnet activates MWEB at block 840 for validation purposes.

## ⚙️ Proof-of-Work Algorithm: RinHash

RinHash is a custom proof-of-work algorithm using:

1. **BLAKE3**: Fast initial hashing
2. **Argon2d**: Memory-hard step that broadens hardware accessibility and supports fair early distribution
3. **SHA3-256**: Final standard cryptographic hash

A valid block satisfies:
`SHA3-256( Argon2d( BLAKE3(block_header) )) < Target`

This design provides fast verification, a memory-hard core that broadens early mining participation, and seamless compatibility with existing 256-bit PoW frameworks.

## 🌐 Network and Usage

- **Magic bytes:** see `src/chainparams.cpp` (documented in the table above for reference)
- **Ports:** `9555` (P2P), `9556` (RPC)
- **Mining:** Accessible to general-purpose hardware (CPU/GPU)
- **Wallet:** Full-node wallet with RIN units

## 🧭 Network Evolution & Mining Policy

Rincoin's Proof-of-Work parameters are treated as a phase-dependent engineering matter, not a permanent ideological commitment. The current RinHash configuration prioritizes broad, accessible participation and fair distribution while the network is establishing its hashrate base and economic weight.

As the network matures, the security model is expected to rest increasingly on the emission schedule, the accumulated economic weight of the chain, and a sustained long-term security budget. Rincoin makes **no permanent guarantee about supported or excluded mining hardware classes**. Any future change to consensus-relevant PoW parameters is an engineering decision subject to the project's formal improvement-proposal process, simulation-based verification, and maintainer approval, with advance notice to node operators and mining participants.

## ⏭️ Planned: v1.1.0

The following changes have been validated in Rincoin-Sim and are planned for the next Core release. All consensus changes will be verified at 1/1000 scale in Rincoin-Sim before mainnet deployment, and will be published as formal Rincoin Improvement Proposals (RIPs).

### A — MWEB Mainnet Sealing (Critical chainparams fix)

`src/chainparams.cpp` currently carries Litecoin-inherited MWEB activation heights (`nStartHeight = 2,217,600`). This must be replaced with `NEVER_ACTIVE` to match documented policy and Rincoin-Sim's validated state. Failure to apply this fix before mainnet block 2,217,600 would allow MWEB to activate under the BDB-locked wallet path, directly contradicting the BDB→SQLite migration risk mitigation.

### B — Customized Halving Hard Fork (Block 840,000)

Three consensus changes activate simultaneously at block 840,000:

| Change | Detail |
| :--- | :--- |
| **CH dilation** (RIP-0002) | Block reward: 6.25 → 4.00 RIN/block (Phase 4 of emission schedule) |
| **nVersion RIN3 marker** | Block version field carries RIN3 identifier for replay-attack protection |
| **Minimum peer version 70018** | Nodes below protocol version 70018 are rejected at the hard fork height |

Implementation: `consensus.nRinHashForkHeight = 840000` in `chainparams.cpp`, with corresponding logic in `validation.cpp` (`GetBlockSubsidy()`) and `net_processing.cpp`. Pre-fork coordination requires at minimum 30 days advance notice to seed operators and mining pools, and monitoring from block 800,000.

### C — Known Inconsistencies (Pending fixes)

| Item | Current value | Correct value | Impact |
| :--- | :--- | :--- | :--- |
| `nMinerConfirmationWindow` (mainnet) | `8064` (inherited from Bitcoin: 2016×4) | `7920` (Rincoin: 1980×4) | BIP9 soft fork activation window |
| Naming artifacts in `netaddress.h` | `BITCOIN_NETADDRESS_H`, `bitcoin-config.h`, Bitcoin copyright | Rincoin equivalents | No functional impact; completeness of sovereignty |

## 🛠️ Building Rincoin

For detailed instructions on building release binaries for Linux and Windows, see [doc/build-release.md](doc/build-release.md).

**Quick start for building from source:**
- [Linux/Unix Build Notes](doc/build-unix.md)
- [Windows Build Notes](doc/build-windows.md)

## 💻 Developer Notes

- See `src/chainparams.cpp` for network configuration (ports, magic bytes, difficulty algorithm, address prefixes). This file is authoritative.
- See `src/netaddress.h` for `INTERNAL_IN_IPV6_PREFIX` (`FD 2D DD 82 F5 C8`, confirmed `sha256("rincoin")`-derived). Minor naming artifacts (`BITCOIN_NETADDRESS_H`, `bitcoin-config.h`) remain but are functionally inert.
- See `src/primitives/block.cpp` for the `GetPoWHash()` RinHash implementation.
- See `src/validation.cpp` for `GetBlockSubsidy()` (CH emission logic).
- Pending fixes and upcoming features are tracked in the **Planned: v1.1.0** section above.

## 💬 Community

Join the official Rincoin community to stay updated, get support, and discuss development:

[![Discord Banner 2](https://discord.com/api/guilds/1354664874176680017/widget.png?style=banner2)](https://discord.gg/H4Du5YuqFa)
