v1.1.0 Release Notes
====================

Rincoin Core version 1.1.0 is now available from:

  <https://github.com/Rin-coin/rincoin/releases>

> **Release candidate.** These notes are published with `v1.1.0-rc2`.
> The candidate is offered for testing and is not the final release.
> Between this candidate and the final tag `v1.1.0`, the version string
> and the documentation change; should a further code change prove
> necessary, it will be published as another candidate. What changed
> since `v1.1.0-rc1` is listed in its own section below.

This is a MAJOR release under RIP-0001. It carries RIN3 transaction-
version enforcement, which activates unconditionally at block 840,000,
together with the P2P signalling that lets upgraded nodes find each
other before then, and it restates the Customized Halving schedule
that v1.0.6 already shipped so that mainnet and the 1/1000-scale test
networks share one rule. Every consensus change in this release was
exercised in Rincoin-Sim at 1/1000 scale before being shipped here,
and each change ported from Sim records the Sim commit in the Core
commit that carries it.

**Every node must be running v1.1.0 before block 840,000.** A node on
v1.0.6 or earlier does not enforce RIN3: from that height it accepts
blocks that v1.1.0 rejects, and can end up following a different
chain.

Please report bugs using the issue tracker at GitHub:

  <https://github.com/Rin-coin/rincoin/issues>

To receive release notifications, watch the repository on GitHub or
join the community Discord linked from the README.

How to Upgrade
==============

If you are running an older version, shut it down. Wait until it has
completely shut down (which might take a few minutes in some cases),
then unpack the release archive and copy over `rincoind`,
`rincoin-cli`, `rincoin-tx` and `rincoin-wallet` (and `rincoin-qt`
where the archive includes it). No chain data or wallet migration is
required.

After upgrading, confirm the version and the RIN3 service bit:

```
$ rincoind -version | head -1
Rincoin Core version v1.1.0-rc2

$ rincoin-cli getnetworkinfo | grep -A8 localservicesnames
```

`RIN3` must appear in `localservicesnames` and `protocolversion` must
read `70018`.

Compatibility
=============

Rincoin Core v1.1.0 is built and tested on Ubuntu 24.04 (x86_64,
g++ 13.3.0). The Linux x86_64 binaries are the release artifacts this
project builds natively and verifies. Windows x64 and ARM64 builds,
where published, are produced by the
`depends` cross-compilation path and are marked experimental until the
verification checklist in `doc/release-process.md` section 10-6 has been
completed for them.

Changes since v1.1.0-rc1
========================

Commit hashes refer to <https://github.com/Rin-coin/rincoin>.

The first candidate funded transactions with the caller's legacy
version through three wallet RPCs, and from the fork height the node
rejects such transactions. This candidate carries the fix and two
functional tests for it, ported from Rincoin-Sim v1.1.0.2, and corrects
two statements in these notes. Under `src/` the difference between the
two candidates is `src/wallet/wallet.cpp` alone; consensus, P2P and
mempool policy are unchanged.

- `fundrawtransaction`, `walletcreatefundedpsbt` and `send` now carry
  the RIN3 transaction version from one block below the fork height,
  as the wallet's own transactions already did. See "Funded
  transactions carry the RIN3 version" under Wallet. (5709a60b8;
  merged as 433759964 via rincoin-core/rincoin#3)
- `wallet_rin3_fund_version.py` (7 subtests) and
  `wallet_rin3_boundary_rescue.py` (9 subtests) are added and
  registered in `test_runner.py`. (1634eb6d2, 106f0b73e)
- The version is 1.1.0rc2. (126d21d36)
- Two corrections to the rc1 notes. `src/validation.cpp` was listed as
  byte-identical to the Rincoin-Sim reference tree; one line differs
  (see "Relation to Rincoin-Sim"). The operator checklist said the
  bundled wallet always emitted the RIN3 version; for the three RPCs
  above, rc1 did not.

Notable changes
===============

Consensus
---------

### RIN3 transaction-version enforcement at block 840,000 (RIP-0009)

From block 840,000, standard transactions must carry
`nVersion = 0x52494E33` ("RIN3"). Enforcement is dual-layer: at mempool
entry in `PreChecks` and at block connection in `ContextualCheckBlock`,
with the reason string `bad-tx-rinhash-version`. Coinbase, HogEx and
MWEB-only transactions are exempt. Transactions on the pre-fork chain
are unaffected. (37dd72f00, 51ad19bd9)

The block template refuses legacy-version transactions from the fork
height, and RIN3-version transactions are standard for relay.
(ba6d1415b)

A peer that relays a legacy-version transaction after the fork is not
penalised: the rejection is classified as a recent consensus change
rather than a consensus failure, so nodes that have not upgraded and
still relay such transactions are not disconnected for it. (9426ca366)

`feature_rin3_enforcement.py` exercises all of this end-to-end and
passes its 10 subtests on this tree. (75f7c0746)

### Customized Halving schedule restated as interval scaling (RIP-0002, RIP-0010)

The Scenario II schedule shipped in v1.0.6 as branches on literal
block heights. `GetBlockSubsidy()` now derives every phase boundary
from `nSubsidyHalvingInterval` alone (4x, 10x, 20x, 30x), so mainnet at
interval 210,000 and testnet/regtest at interval 210 run one rule with
proportional timing. Mainnet values are unchanged: 6.25 RIN through
block 839,999, 4.00 RIN from block 840,000, then 2, 1 and 0.6 RIN at
the later phase boundaries. The fork height is
`consensus.nRinHashForkHeight = 840000`. (ebfcaff4e, bf91d99b4)

Measured on this tree: `getblockstats` reports a subsidy of
625,000,000 satoshi at height 839 and 400,000,000 at height 840 on
regtest (1/1000 scale).

### Chain parameter self-checks

Each chain parameter constructor now asserts that the Customized
Halving fork height equals four times the halving interval, and
`CMainParams` additionally asserts that the derived supply cap equals
`MAX_MONEY`. `SetupServerArgs` constructs all networks' parameters at
startup to fill in the help text, so every `rincoind` start checks
every parameter set. Both assertions were verified by breaking them:
a fork height off by one and a cap off by one satoshi each abort
`rincoind` with exit status 134, naming the assertion; reverted, it
starts normally. (c4594f4e3, 9f53e04aa)

### Supply accounting, Stage A (RIP-0002)

`GetTotalSubsidy()` expresses the emission schedule's integral in
closed form over the phase boundaries, and `GetSupplyCap()` derives the
168,000,000 RIN cap from the halving interval. Unit tests pin the
closed form against a brute-force sum of `GetBlockSubsidy()`, including
the height at which cumulative issuance reaches the cap (234,587,500).
(1f388261d, c2bafc878, 9577511dc, bb3a074ff; merged as 70ed956ba via
rincoin-core/rincoin#2)

This is accounting only. `GetBlockSubsidy()` is unchanged, no
consensus rule reads the new constant or calls the new functions, and
emission is not altered. Enforcement of the cap is Stage B and
requires a separate RIP.

### MWEB and Taproot sealed on mainnet (RIP-0004, RIP-0011)

Mainnet MWEB and Taproot deployments are set to `NEVER_ACTIVE`,
replacing the Litecoin-inherited height-based values. MWEB is deferred
pending the SQLite descriptor-wallet migration; Taproot is deferred
because its inherited activation heights are not aligned to Rincoin's
BIP9 window. Testnet and regtest are unaffected: MWEB signals from
block 840 on testnet (`nStartHeight = 840`, `nTimeoutHeight = 1050`,
BIP8) and Taproot is `ALWAYS_ACTIVE` on both. (cd1fae169)

### BIP9 window corrected on mainnet

`nMinerConfirmationWindow` is 7,920 (1,980 x 4) and
`nRuleChangeActivationThreshold` is 5,940 (75%), replacing the
upstream 8,064 / 6,048. The sealing above and this change land in the
same commit, so no live deployment observes the new window
retroactively. (cd1fae169)

### `-vbparams` parser fix (Litecoin #1095)

The parser zero-initialises `nStartHeight` / `nTimeoutHeight`, which
were previously uninitialised on the legacy three-argument form. This
is the Rincoin-side application of the fix reported upstream as
litecoin-project/litecoin#1095. (37dd72f00)

### HogEx with no inputs (RIP-0004 section 4)

The first HogEx transaction of an MWEB block has no inputs when the
activation block carries no peg-ins. Context-free validation in
`consensus/tx_check.cpp` rejected it as `bad-txns-vin-empty`; HogEx is
now exempt from that check, and its inputs are validated contextually
while MWEB is active. This is a deliberate fork-local divergence from
upstream Litecoin, which retains the rejection. RIP-0004 section 4.4
records the audited consequences of the exemption on a mainnet where
MWEB is sealed, and condition S-7 schedules its retirement, bundled
with any MWEB re-activation, as a consensus tightening. (e68cf8fe5)

P2P and network
---------------

### NODE_RIN3 service bit and protocol version 70018 (RIP-0009)

Nodes advertise `NODE_RIN3` (service bit 25) and `PROTOCOL_VERSION`
70018. `MIN_PEER_PROTO_VERSION` is unchanged, so RIN3 support is a
capability rather than a version floor: peers that do not signal it
are still connected and served. (fae242216, cac01210e)

Automatic outbound connections prefer peers that advertise the bit
(`GetDesirableServiceFlags`), which gives every upgraded node a
connected relay subgraph for RIN3 transactions, which legacy nodes drop
as non-standard. Inbound connections from legacy peers are accepted,
and manual connections (`-addnode`, `-connect`) are exempt from the
preference. This mirrors the outbound preference Bitcoin applied to
`NODE_WITNESS` at the SegWit rollout and is what preserves soft-fork
topology up to the activation height. (6f7b056ba)

`GetDesirableServiceFlags` now includes `NODE_RIN3`, so a v1.1.0 node
queries its DNS seeds with the filter prefix `x3000009`
(`NODE_NETWORK | NODE_WITNESS | NODE_MWEB | NODE_RIN3`). The project's
seeder was given that whitelist entry on 2026-08-28 (Aevust/seeder#2);
before it, the default whitelist rejected the filter and such queries
returned empty. Anyone running a DNS seeder for this network needs the
same entry. Until nodes advertising RIN3 appear, the filtered query
answers with nothing, which is expected rather than a failure. The
seeds are `seed.rincoin.org` and `seed.rincoin.net`. (b0966f7fe)

Measured on this tree: `localservices` reports `0000000003800449`
with `RIN3` among the names, and the version message on the wire
carries 70018 and the same service mask. `p2p_rin3_services.py`
passes its 3 subtests. (7445cf336)

### Fixed seed list

The fixed seed list (`src/chainparamsseeds.h`) is regenerated from
`contrib/seeds/nodes_main.txt` and `nodes_test.txt`, which now name
the two nodes this project operates. The previous list was
hand-written: its input files still held Litecoin's node lists, and
the testnet entries encoded port 19539 where `nDefaultPort` is 19555.
Entries that operated outside the project, and one that no longer answers
on any port, were removed. Fixed seeds are the fallback used when DNS
seeds are unavailable. (b8f2b0597)

Wallet
------

### Taproot wallet guard (RIP-0011)

While the Taproot deployment is sealed, `CWallet::CreateTransaction`
refuses to create outputs paying to witness version 1 or later.
Without the guard such outputs would be anyone-can-spend at the
consensus level on mainnet, and the hazard is reachable from
`sendtoaddress`. The refusal surfaces as RPC error -6. MWEB recipients
are exempt. `feature_taproot_wallet_guard.py` covers both branches of
the guard and passes its 3 subtests on this tree. (56843e34d,
80d5d831a)

### RIN3 version emitted one block early

The wallet switches to the RIN3 transaction version when the tip is
one block below the fork height, so a transaction created at the
boundary is not left with a legacy version when the block that
includes it lands on or after 840,000. (7e95083bd)

### Funded transactions carry the RIN3 version

On v1.1.0-rc1 the caller's default `nVersion` of 2 survived funding
through `fundrawtransaction`, `walletcreatefundedpsbt` and `send` at
any height, so from block 840,000 the node rejects the result with
`bad-tx-rinhash-version`; `send` returned a txid for a transaction the
node had not accepted into its mempool (measured on the published rc1
binaries, 2026-09-25). `CWallet::FundTransaction()` copied the output
amounts and the inputs of the transaction `CreateTransaction()` built,
but not its version. It now copies the version as well when the
caller's transaction has `CTransaction::CURRENT_VERSION`, so these
RPCs switch to the RIN3 marker one block below the fork height, where
`txassembler.cpp` switches the wallet's own transactions. Any other
version the caller set is kept, so the workaround published on the rc1
release page, setting the marker before funding, keeps working; a
caller that sets 2 itself cannot be told apart from one that left the
default and gets the wallet's version too. Ported from Rincoin-Sim
v1.1.0.2, released 2026-10-05. (5709a60b8)

Two functional tests cover it. `wallet_rin3_fund_version.py` pins the
version each of the three RPCs gives on both sides of the switch.
`wallet_rin3_boundary_rescue.py` follows a legacy-version transaction
broadcast before the fork height that did not confirm: a mempool that
holds it keeps it and, by default, refuses its replacement and its
re-creation; a restart drops it, after which both are accepted and
mined. See Known issues. (1634eb6d2, 106f0b73e)

### `-walletdir` with a trailing separator

Since Boost 1.78, `fs::canonical()` no longer strips a trailing path
separator, so a user-supplied trailing slash in `-walletdir` was
retained in the stored path. It is now removed after
canonicalisation, mirroring Bitcoin Core PR 24104. Wallet operation
was unaffected; the walletinit unit tests that check the stored path
pass again. (07e6bef0a)

Logging
-------

### Header-sync timing moved behind `-debug=bench`

Two per-batch header-sync timing lines (`HEADERSYNC-PERF`) were
written unconditionally and accounted for the large majority of log
volume on a synced v1.0.6 node (197 of the last 200 lines on one
production node, about 8 MB per day). They now use the `bench`
category, which this tree already uses for comparable timing output
elsewhere. Output is unchanged with `-debug=bench`; without it the
lines are suppressed. Operators who relied on these lines should add
`-debug=bench`. (2515fc659)

Tests
-----

### Functional test framework synchronised with the chain

The functional test framework now matches `chainparams.cpp`: message
magic bytes on all three networks, Rincoin address encodings and
`RIN` fee-rate units in place of inherited Litecoin values, the
`indexes/` directory preserved in the cached datadir (tests that use
the 199-block cache previously crashed in setup because
`-blockfilterindex` defaults to on in this tree), and the `rin_` test
prefix accepted by `test_runner.py`. (065898039 through 646715055)

Five tests written for Rincoin are registered:
`feature_rin3_enforcement.py` (10 subtests),
`feature_taproot_wallet_guard.py` (3), `p2p_rin3_services.py` (3),
`wallet_rin3_fund_version.py` (7) and `wallet_rin3_boundary_rescue.py`
(9). The Qt RPC console test uses the Rincoin mainnet Merkle root.
(75f7c0746, 80d5d831a, 7445cf336, e09a447fb, 1634eb6d2, 106f0b73e)

### Unit tests

`validation_tests` gains a full regtest-scale sweep pinning
`GetTotalSubsidy()` against `GetBlockSubsidy()` for every height in
[0, 240000] and mainnet-scale boundary checks at S_fix and the cap
height. The unit suite now runs 500 cases. (bb3a074ff)

Build and release
-----------------

- The test, bench and Qt executables are named `test_rincoin`,
  `bench_rincoin` and `rincoin-qt`. (1488c8729, 11dc913a9,
  6a8534447)
- `contrib/release/` provides the multi-platform build Makefile and
  the signing script; `doc/release-process.md` describes the release
  procedure around them. (e2b753cc7, 6e361434d, fd201f357,
  929c130f2, adb43eb54, b6cfc0a34)
- The version is 1.1.0rc2 for this candidate (`_CLIENT_VERSION_RC = 2`
  in `configure.ac`; the first candidate carried 1). `contrib/release/`
  derives artifact names and the git tag from `configure.ac`:
  distributed files carry `1.1.0rc2` and the tag carries `v1.1.0-rc2`,
  following the convention of the inherited release candidates in this
  history. (c92864c64, 2bdf74061, 126d21d36)
- `chainparams.cpp` includes `<stdexcept>` explicitly. (30d4da632)

Documentation and policy
------------------------

- `SECURITY.md` describes how to report vulnerabilities and the GPG
  policy for releases. The release signing key is no longer bundled in
  the tree and is published as `SECURITY.md` describes. (7096d0a57,
  456c6a29d, dbb667d16, e80a700ea, cb9bde370, 524138cfb)
- The README describes the release contents, the emission schedule and
  the test-suite state as measured. (d27d8074e, 0f70caace, de22a42fc)

Verification
============

All measurements below were taken on the tree this candidate is built
from (126d21d36), on Ubuntu 24.04 (x86_64), with every commit verified
against the release signing key, except the mined testnet chain, which
was measured on v1.1.0-rc1 (a1b12dc8c) and is marked as such; between
the two candidates the only change under `src/` is
`src/wallet/wallet.cpp`.

### Unit tests

```
$ ./src/test/test_rincoin
Running 500 test cases...
*** No errors detected

$ make check
exit status 0: the unit suite run per source file (121 files), the
util tests, libsecp256k1 2/2, univalue 3/3
```

Of the 500 cases, `script_assets_test` reports a warning and skips its
body because `DIR_UNIT_TEST_DATA` is unset, as it does upstream; the
other 499 pass without one.

### Functional tests

```
$ test/functional/test_runner.py --exclude feature_loadblock.py
  211 scripts: 116 passed, 53 failed, 42 skipped     (runtime 963 s)
```

The 53 failures are the same set as at v1.1.0-rc1, compared as sets
rather than counts, and the two scripts added in this candidate pass.
Every one of the 53 also fails in the Rincoin-Sim baseline of
2026-09-03, which has one further failure of its own
(`feature_config_args.py`, which passes here). None is caused by the
changes in this release: 52 were present in the first baseline this
tree could produce, taken as soon as the framework repairs let the
suite run, and the 53rd, `p2p_addr_relay.py`, was intermittent when
rc1 was measured and failed in every run taken for this candidate, in
the suite and in three runs on its own. The set has been re-measured
after every group of changes since. They fall into these classes:

- Address vectors and message-signing prefixes in older tests still
  carry Litecoin values (`rltc1`, WIF and P2SH bytes, `Litecoin Signed
  Message:`); the tests fail on the encoding, not on the node.
- Test-framework helpers derive block identity from SHA256d, where
  this chain uses RinHash for `GetHash()` and `GetPoWHash()`; tests
  that wait on a block by hash time out.
- A small number depend on deployment state (`time-too-new`, buried
  deployments, MWEB) or on the test environment.
- `p2p_addr_relay.py`: for its rate-limit checks the test asks the node
  for an outbound connection to a mock peer that advertises
  `NODE_NETWORK | NODE_WITNESS | NODE_MWEB`; the node expects
  `NODE_RIN3` of outbound peers as well and closes the connection. A
  framework default, to be corrected in Sim first; intermittent when
  rc1 was measured, in every run for this candidate.

Each class is tracked for a 1.1.x maintenance release.

`feature_loadblock.py` is excluded because `contrib/linearize`
identifies blocks by SHA256d and never matches on a RinHash chain, so
the test hangs rather than fails.

### Individual runs

| Test | Result |
| ---- | ------ |
| `feature_rin3_enforcement.py` | 10 subtests PASS |
| `feature_taproot_wallet_guard.py` | 3 subtests PASS |
| `p2p_rin3_services.py` | 3 subtests PASS |
| `wallet_rin3_fund_version.py` | 7 subtests PASS |
| `wallet_rin3_boundary_rescue.py` | 9 subtests PASS |
| `validation_tests` | 4 cases PASS (sweep included) |

### Fork boundary script

`scripts/sim-rin3-boundary.sh` from the Rincoin-Sim v1.1.0.2 tree
(sha256 `f19b4d06`...), run with `EXPECT_FUND_MARKER=1`, drives two
nodes across the regtest fork height and checks the transaction
version on both sides, the subsidy at the boundary, relay of legacy
transactions and the three funding RPCs. Against this tree's binaries:
PASS 24, FAIL 0, INCONCLUSIVE 0. Against the published v1.1.0-rc1
binaries: PASS 21, FAIL 3, the three being the funding checks this
candidate fixes. The script is not part of this tree.

### Consensus behaviour on regtest

```
$ rincoin-cli -regtest getblockstats 839 | grep subsidy
  "subsidy": 625000000,
$ rincoin-cli -regtest getblockstats 840 | grep subsidy
  "subsidy": 400000000,
$ rincoin-cli -regtest getblockstats 841 | grep subsidy
  "subsidy": 400000000,
```

### Consensus behaviour on a mined testnet chain

The same rules were exercised on v1.1.0-rc1 (a1b12dc8c) on a testnet
chain mined from genesis, where the difficulty adjustment is live
rather than pinned at the regtest minimum; the consensus code is
unchanged in this candidate. The chain is private: it was mined solely
by this node with `generatetoaddress` and had no peers at any point.

Every Customized Halving phase boundary, in satoshi:

| boundary | subsidy before | subsidy at and after |
| ---- | ---- | ---- |
| 210 | 5000000000 | 2500000000 |
| 420 | 2500000000 | 1250000000 |
| 630 | 1250000000 | 625000000 |
| **840** | **625000000** | **400000000** |

The MWEB deployment, as observed while the chain was being mined.
`getblockchaininfo` reports the state that applies to the next block,
so a tip of 839 already reports `started` with `since` 840:

| tip | bip8 status | since |
| ---- | ---- | ---- |
| 18 | defined | 0 |
| 839 | started | 840 |
| 1049 | locked_in | 1050 |
| 1260 | active | 1260 |

RIN3 enforcement, both directions, at a tip well above the testnet
fork height of 840:

- A wallet transaction was created with `sendtoaddress` and mined. Its
  version is `0x52494e33`.
- A transaction built with `createrawtransaction`, which emits version
  2, was signed and submitted with `sendrawtransaction`. The node
  answered with error -26 `bad-tx-rinhash-version`, and the
  transaction is absent from the mempool afterwards.

A record of this run, listing the commit, the digests of the binaries
that produced it, the chain's genesis hash and peer count, and every
value above, is published with the release together with the script
that produces it.

### Relation to Rincoin-Sim

The reference tree is Rincoin-Sim v1.1.0.2 at commit
`7d07316439428c29ec4ace6c56754b2ec932584d`, the tree that release was
verified on. The following files are byte-identical to it:
`src/protocol.h`, `src/protocol.cpp`, `src/init.cpp`, `src/amount.h`,
`src/validation.h`, `src/test/validation_tests.cpp`,
`src/wallet/wallet.cpp`, and every file under `test/`.
`src/validation.cpp` differs in one line: the header-sync timing line
is behind `-debug=bench` here (2515fc659) and unconditional in Sim.
The rc1 notes listed that file as byte-identical; it was not, for the
same line. `src/chainparams.cpp` differs in comments, whitespace and
the order of two assignments only; the simulator's chain guard is in
its `src/bitcoind.cpp`. Each Core commit that ports a Sim change
records the Sim commit hash in its message. The Sim v1.1.0.2 release
is at <https://github.com/Aevust/rincoin-sim/releases/tag/v1.1.0.2>;
the archived build and test evidence of Sim v1.1.0 is at
<https://doi.org/10.5281/zenodo.21805345>.

Known issues
============

- The 53 functional test failures listed above. None affects node
  operation; they are test-framework defects inherited from upstream
  and are tracked for 1.1.x.
- `p2p_node_network_limited.py` passes the service-mask check added
  in this release but still fails later at `wait_for_block`, for the
  SHA256d reason above.
- The man pages under `doc/man/` are the inherited Litecoin 0.21.4
  pages and do not describe this release. They are regenerated from
  the tagged binaries as part of the final release build.
- `build_msvc/` is the inherited Bitcoin 0.21.3 tree and is not a
  supported build path for this release. Windows binaries, where
  published, come from the `depends` cross-compilation path.
- MWEB and Taproot remain `NEVER_ACTIVE` on mainnet; see RIP-0004 and
  RIP-0011 for the conditions under which either is revisited.
- The mempool refuses replacements by default (`-mempoolreplacement`,
  inherited from Litecoin). A legacy-version transaction that a node
  accepted before the fork height blocks any re-creation of it on that
  node until the transaction leaves the mempool; a restart is the
  measured way out, and `bumpfee` is not one
  (`wallet_rin3_boundary_rescue.py`).
- The funding defect fixed in this candidate was present in
  v1.1.0-rc1, measured on its published binaries. The rc1 release page
  carries the workaround for nodes still on rc1.

Node operator checklist
=======================

1. Upgrade every node to v1.1.0 before block 840,000.
2. Confirm `RIN3` in `localservicesnames` and `protocolversion` 70018.
3. If you operate a DNS seeder, give it the whitelist entry for the
   filter v1.1.0 sends, `x3000009` (Aevust/seeder#2); a seeder without
   it answers such queries with nothing.
4. If you relied on `HEADERSYNC-PERF` log lines, add `-debug=bench`.
5. Mining pools and exchanges: transactions you create from block
   840,000 must carry `nVersion = 0x52494E33`. The bundled wallet does
   this from this candidate on; rc1 did not for `fundrawtransaction`,
   `walletcreatefundedpsbt` and `send` (see "Changes since
   v1.1.0-rc1"). External transaction builders must be updated.

Credits
=======

Thanks to everyone who directly contributed to this release:

- Aevust
- ysmreg

And to the developers of Litecoin Core and Bitcoin Core, on whose work
this tree is built.
