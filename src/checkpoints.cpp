// Copyright (c) 2009-2012 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <boost/assign/list_of.hpp> // for 'map_list_of()'
#include <boost/foreach.hpp>

#include "checkpoints.h"

#include "main.h"
#include "uint256.h"

namespace Checkpoints
{
    typedef std::map<int, uint256> MapCheckpoints;

    // How many times we expect transactions after the last checkpoint to
    // be slower. This number is a compromise, as it can't be accurate for
    // every system. When reindexing from a fast disk with a slow CPU, it
    // can be up to 20, while when downloading from a slow network with a
    // fast multicore CPU, it won't be much higher than 1.
    static const double fSigcheckVerificationFactor = 5.0;

    struct CCheckpointData {
        const MapCheckpoints *mapCheckpoints;
        int64 nTimeLastCheckpoint;
        int64 nTransactionsLastCheckpoint;
        double fTransactionsPerDay;
    };

    // What makes a good checkpoint block?
    // + Is surrounded by blocks with reasonable timestamps
    //   (no blocks before with a timestamp after, none after with
    //    timestamp before)
    // + Contains no strange transactions
    static MapCheckpoints mapCheckpoints =
        boost::assign::map_list_of
        (  0, uint256("0xd0e9d5a9f2dc4aeb64e8055a4a6afdfc7d374593a24fb64f094c0d770ad5c5cf"))
        (  1, uint256("0x5273d7f2a81787bc722105f71d4fcb32d99c3e4f6677552f7e6a84dc469c5b8c"))
        (  2, uint256("0x52dab489733b6990a1db2f4ca2743ec06c1262678ef6d1344778782d44661c8f"))
        (  3, uint256("0xd0622431e3637f41250d110eae8d890a5f9e382ab7018b3151d368a459214d80"))
        (  4, uint256("0x11f1442e7961c51b97a47814559a4b2cdc4c58c09d4a9032836fb33488a48c57"))
        (  5, uint256("0xf3e030ac0a36df48694b2537e6d1f3ead6e69b0ffc398ae2674d525ec44b6454"))
        (  6, uint256("0xe2efc5a993d8703b077ea707be32f2c62da092e70b786269260ef5a17e2ae301"))
        (  7, uint256("0x1b776f0d33c6c2f7ae12132ea1fcfe2d85e4189a2ba8f48476312928ceca2eb7"))
        (  8, uint256("0xb36295b3ad7c30ecd008d0dde947a4c2f03e0e80d157b01451274a012fd294c9"))
        (  9, uint256("0xa8245043038250ecc583bd05996b41c380ae85bf917ac5a8e69f8759b9c9c8f9"))
        (  10, uint256("0x3263db32aaa9062e7cf4feb038eb4aa4a937ffdf8335f5563244040f4a45f56b"))
        ;
    static const CCheckpointData data = {
        &mapCheckpoints,
        1518601662,	// * UNIX timestamp of last checkpoint block. Unix Epoch timestamp "1518601662" == "2018-02-14 09:47:42 GMT" converted on https://www.epochconverter.com/
        11,    		// * total number of transactions between genesis and last checkpoint
                    //   (the tx=... number in the SetBestChain debug.log lines)
        1000     	// * estimated number of transactions per day after checkpoint
    };

    static MapCheckpoints mapCheckpointsTestnet = 
        boost::assign::map_list_of
        (   0, uint256("0x0c264d7455c96216a85d6fba090efa64cc2b5915bd34f3983c3560089be22fbbf"))
        (   1, uint256("0xd06021e22b610bd73ff04083ba77a6299c99d3acd7a840aefa07690170c12692"))
        (   2, uint256("0x5c66012cd9aebb8cc9fba420ac14daddfd42fcf4a16405921095459bd2919485"))
        (   3, uint256("0x753e2bc3dddf25d5a914fa08fbf77ee485d54e186de84f6e5967382ae16b3cae"))
        (   4, uint256("0x4fbea3eb7b02e8367b8e24cf9fc7d929b2064e715d5aa93d1ab71e093bf616f8"))
        (   5, uint256("0x72ef263ae93e0465f4f7ba38a9f1061c26a4274c07bd62d8ea323a22b199aa72"))
        (   6, uint256("0xcfe042208d7f7b41c4e6f71e95867e2331f04568fb64f4c926bf5e3ad83ffa20"))
        (   7, uint256("0xee22385cd6344faa44de852f10aab618b0f83b506b41fd69746eee5b7e1d76f3"))
        (   8, uint256("0xbe3d70d8189e7b106af750006ec78e23ed683bfdcf150b6c0813d1c51be21ad3"))
        (   9, uint256("0xd5340b7b6b5d3c45be51c43c211aefee6b8221059d01201255ff5804cc36dcc3"))
        (   10, uint256("0x7a52b843cd1b4bd51161934ca9c59be7fd6ef5bab9a2e285a27346e9340b8c46"))
        (   11, uint256("0xb4afe864b5dcb3183010f84379a7df94019a62f0d7d67e7a5a8307c420643aff"))
        (   12, uint256("0xb810a5e795e9363e3dbce3668a78d55154de27a2cee064c80f9b8d046bfd6ed3"))
        (   13, uint256("0xec2b3ef2a7f2148b76e2421d7c890deb9120156cb59d257ba3e5902c0053c85e"))
        (   14, uint256("0xe8dfbaf48f34a26785aaa75c2990c98b266514d8f085ea2a7d9b9f5f522369bf"))
        ;
    static const CCheckpointData dataTestnet = {
        &mapCheckpointsTestnet,
        1518607132,	// UNIX Epoch timestamp "1518607132" == "2018-02-14 11:18:52 GMT" converted on https://www.epochconverter.com/
        15,
        1000
    };

    const CCheckpointData &Checkpoints() {
        if (fTestNet)
            return dataTestnet;
        else
            return data;
    }

    bool CheckBlock(int nHeight, const uint256& hash)
    {
        if (fTestNet) return true; // Testnet has no checkpoints
        if (!GetBoolArg("-checkpoints", true))
            return true;

        const MapCheckpoints& checkpoints = *Checkpoints().mapCheckpoints;

        MapCheckpoints::const_iterator i = checkpoints.find(nHeight);
        if (i == checkpoints.end()) return true;
        return hash == i->second;
    }

    // Guess how far we are in the verification process at the given block index
    double GuessVerificationProgress(CBlockIndex *pindex) {
        if (pindex==NULL)
            return 0.0;

        int64 nNow = time(NULL);

        double fWorkBefore = 0.0; // Amount of work done before pindex
        double fWorkAfter = 0.0;  // Amount of work left after pindex (estimated)
        // Work is defined as: 1.0 per transaction before the last checkoint, and
        // fSigcheckVerificationFactor per transaction after.

        const CCheckpointData &data = Checkpoints();

        if (pindex->nChainTx <= data.nTransactionsLastCheckpoint) {
            double nCheapBefore = pindex->nChainTx;
            double nCheapAfter = data.nTransactionsLastCheckpoint - pindex->nChainTx;
            double nExpensiveAfter = (nNow - data.nTimeLastCheckpoint)/86400.0*data.fTransactionsPerDay;
            fWorkBefore = nCheapBefore;
            fWorkAfter = nCheapAfter + nExpensiveAfter*fSigcheckVerificationFactor;
        } else {
            double nCheapBefore = data.nTransactionsLastCheckpoint;
            double nExpensiveBefore = pindex->nChainTx - data.nTransactionsLastCheckpoint;
            double nExpensiveAfter = (nNow - pindex->nTime)/86400.0*data.fTransactionsPerDay;
            fWorkBefore = nCheapBefore + nExpensiveBefore*fSigcheckVerificationFactor;
            fWorkAfter = nExpensiveAfter*fSigcheckVerificationFactor;
        }

        return fWorkBefore / (fWorkBefore + fWorkAfter);
    }

    int GetTotalBlocksEstimate()
    {
        if (fTestNet) return 0; // Testnet has no checkpoints
        if (!GetBoolArg("-checkpoints", true))
            return 0;

        const MapCheckpoints& checkpoints = *Checkpoints().mapCheckpoints;

        return checkpoints.rbegin()->first;
    }

    CBlockIndex* GetLastCheckpoint(const std::map<uint256, CBlockIndex*>& mapBlockIndex)
    {
        if (fTestNet) return NULL; // Testnet has no checkpoints
        if (!GetBoolArg("-checkpoints", true))
            return NULL;

        const MapCheckpoints& checkpoints = *Checkpoints().mapCheckpoints;

        BOOST_REVERSE_FOREACH(const MapCheckpoints::value_type& i, checkpoints)
        {
            const uint256& hash = i.second;
            std::map<uint256, CBlockIndex*>::const_iterator t = mapBlockIndex.find(hash);
            if (t != mapBlockIndex.end())
                return t->second;
        }
        return NULL;
    }
}
