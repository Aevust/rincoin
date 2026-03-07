// genesis_miner.cpp - Rincoin Testnet Genesis Block Miner
//
// Standalone utility to deterministically "mine" the testnet genesis block by brute-forcing the nonce.
//
// Requirements:
// - Reuse existing block, PoW, and consensus code (no reimplementation)
// - Copy testnet genesis parameters verbatim from chainparams.cpp (see below)
// - Output values for direct copy-paste into chainparams.cpp
//
// Usage: Build with the rest of the project, run from command line.
//
// Copyright (c) 2026 The Rincoin Developers

#include <chainparams.h>
#include <consensus/merkle.h>
#include <consensus/params.h>
#include <hash.h>
#include <primitives/block.h>
#include <primitives/transaction.h>
#include <script/script.h>
#include <uint256.h>
#include <util/strencodings.h>
#include <pow.h> // for CheckProofOfWork
#include <iostream>
#include <memory>
#include <vector>
#include <cassert>


// -------------------
// Copied from chainparams.cpp (regtest genesis block parameters)
// -------------------
static const char* pszTimestamp = "RinCoin Genesis Block - RinHash RegTest1";
static const CScript genesisOutputScript = CScript() 
    << ParseHex("04b1c2d3e4f5a6b7c8d9eaf1b2c3d4e5f6a7b8c9dae1f2b3c4d5e6f7a8b9c0d1e2f3a4b5c6d7e8f9a0b1c2d3e4f5a6b7c8d9eaf1b2c3d4e5f6a7b8c9dae1f2b3c4d5e6f7a8b9c0d1e2f3a4b5c6d7e8f9a0b1")
    << OP_CHECKSIG;
static const uint32_t nTime = 1743059120;
static const uint32_t nBits = 0x207fffff;
static const int32_t nVersion = 1;
static const CAmount genesisReward = 50 * COIN;
// -------------------

// Helper: Construct regtest genesis block exactly as in CreateRegTestGenesisBlock
static CBlock CreateGenesisBlock(uint32_t nTime, uint32_t nNonce) {
    CMutableTransaction txNew;
    txNew.nVersion = 1;
    txNew.vin.resize(1);
    txNew.vout.resize(1);
    txNew.vin[0].scriptSig = CScript() << 486604799 << CScriptNum(4)
        << std::vector<unsigned char>((const unsigned char*)pszTimestamp, (const unsigned char*)pszTimestamp + strlen(pszTimestamp));
    txNew.vout[0].nValue = genesisReward;
    txNew.vout[0].scriptPubKey = genesisOutputScript;

    CBlock genesis;
    genesis.nTime    = nTime;
    genesis.nBits    = nBits;
    genesis.nNonce   = nNonce;
    genesis.nVersion = nVersion;
    genesis.vtx.push_back(MakeTransactionRef(std::move(txNew)));
    genesis.hashPrevBlock.SetNull();
    genesis.hashMerkleRoot = BlockMerkleRoot(genesis);
    return genesis;
}

int main() {
    // Use regtest consensus params for PoW check
    Consensus::Params consensusParams;
    consensusParams.powLimit = uint256S("7fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
    consensusParams.fPowNoRetargeting = true;
    consensusParams.fPowAllowMinDifficultyBlocks = true;

    // Construct block with fixed fields, search for valid nonce
    uint32_t nNonce = 0;
    CBlock genesis;
    uint256 hash;
    bool found = false;

    // Compute Merkle root once (deterministic)
    genesis = CreateGenesisBlock(nTime, nNonce);
    const uint256 merkleRoot = genesis.hashMerkleRoot;

    // -------------------
    // Nonce search loop
    // -------------------
    for (nNonce = 0; nNonce < UINT32_MAX; ++nNonce) {
        genesis = CreateGenesisBlock(nTime, nNonce);
        hash = genesis.GetPoWHash();
        if (CheckProofOfWork(hash, nBits, consensusParams)) {
            found = true;
            break;
        }
        if (nNonce % 1000000 == 0) {
            std::cout << "." << std::flush; // Progress indicator
        }
    }
    std::cout << std::endl;

    if (!found) {
        std::cerr << "No valid nonce found in range." << std::endl;
        return 1;
    }

    // Output for direct copy-paste into chainparams.cpp
    std::cout << "// --- Regtest Genesis Block ---" << std::endl;
    std::cout << "nNonce = " << nNonce << ";" << std::endl;
    std::cout << "hashGenesisBlock = uint256S(\"0x" << hash.GetHex() << "\");" << std::endl;
    std::cout << "hashMerkleRoot = uint256S(\"0x" << merkleRoot.GetHex() << "\");" << std::endl;
    std::cout << "nBits = 0x" << std::hex << nBits << ";" << std::endl;
    std::cout << "nTime = " << std::dec << nTime << ";" << std::endl;
    return 0;
}
