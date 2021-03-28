// Copyright (c) 2011-2017 The Cryptonote Developers
// Copyright (c) 2017-2018 The Circle Foundation & Conceal Devs
// Copyright (c) 2018-2019 Conceal Network & Conceal Devs
// Copyright (c) 2020 - The Cache Developers
//
// Distributed under the GNU Lesser General Public License v3.0.
// Please read Cache/License.md

#pragma once

#include <cstddef>
#include <cstdint>
#include <limits>
#include <initializer_list>

#include "Common/Types.h"

namespace CryptoNote {
  namespace parameters {
    const uint64_t MAX_BLOCK_NUMBER = 500000000;
    const size_t   MAX_BLOCK_BLOB_SIZE = 500000000;
    const size_t   MAX_TX_SIZE = 1000000000;

    /* cxche address prefix - 43810262 */
    const uint64_t PUBLIC_ADDRESS_BASE58_PREFIX = 0x29c7dd6;

    /* 20 minutes */
    const size_t   MINED_COINS_UNLOCK_WINDOW = 10;

    const size_t   DECIMAL_POINT = 5;
    const uint64_t COIN = UINT64_C(100000);
    const uint64_t MONEY_SUPPLY = UINT64_C(30000000000000);

    const uint64_t MINIMUM_FEE = UINT64_C(100);
    const uint64_t MINIMUM_FEE_BANKING = UINT64_C(100);
    const uint64_t DEFAULT_DUST_THRESHOLD = UINT64_C(10);

	         /* Fee adjustment V1 */
    const uint64_t MINIMUM_FEE_V1                                = UINT64_C(10);

    const uint64_t MINIMUM_FEE_V1_HEIGHT                         = 1337;

	          /* Fee adjustment V2 */
    const uint64_t MINIMUM_FEE_V2                                = UINT64_C(1);

    const uint64_t MINIMUM_FEE_V2_HEIGHT                         = 1337;

    const Mixin    MINIMUM_MIXIN = 0;
    const Mixin    DEFAULT_MIXIN = 5;

    const uint64_t MULTIPLIER_FACTOR = 100;
    const Height   END_MULTIPLIER_BLOCK = 12750;

    const BlockOrTimestamp DIFFICULTY_TARGET = 120;
    const size_t   DIFFICULTY_WINDOW = 120;

    /* Works with LWMA3 */
    const uint64_t BLOCK_FUTURE_TIME_LIMIT = 360;
    const size_t   BLOCKCHAIN_TIMESTAMP_CHECK_WINDOW = 11;
    const size_t   DIFFICULTY_BLOCKS_COUNT = DIFFICULTY_WINDOW + 1;

    const Height   EXPECTED_NUMBER_OF_BLOCKS_PER_DAY = 24 * 60 * 60 / DIFFICULTY_TARGET;

    const Amount   DEPOSIT_MIN_AMOUNT = 1 * COIN;

    const Height   DEPOSIT_MIN_TERM = 21900;
    const Height   DEPOSIT_MAX_TERM = 1 * 12 * 21900;

    const uint64_t DEPOSIT_MIN_TOTAL_RATE_FACTOR = 0;
    const uint64_t DEPOSIT_MAX_TOTAL_RATE = 4;

    const Height   UPGRADE_HEIGHT = 1;
    const Height   UPGRADE_HEIGHT_V2 = 2;

    const unsigned UPGRADE_VOTING_THRESHOLD = 90;
    const size_t   UPGRADE_VOTING_WINDOW = EXPECTED_NUMBER_OF_BLOCKS_PER_DAY;
    const size_t   UPGRADE_WINDOW = EXPECTED_NUMBER_OF_BLOCKS_PER_DAY;

    const Amount   POINT = UINT64_C(100);
    const size_t   OPTIMIZE_SIZE = 100;

    const size_t   REWARD_BLOCKS_WINDOW = 100;
    const size_t   COINBASE_BLOB_RESERVED_SIZE = 600;
    const size_t   BLOCK_GRANTED_FULL_REWARD_ZONE = 100000;

    const size_t   MAX_BLOCK_SIZE_INITIAL = BLOCK_GRANTED_FULL_REWARD_ZONE * 10;
    const uint64_t MAX_BLOCK_SIZE_GROWTH_SPEED_NUMERATOR = 100 * 1024;
    const uint64_t MAX_BLOCK_SIZE_GROWTH_SPEED_DENOMINATOR = 365 * 24 * 60 * 60 / DIFFICULTY_TARGET;
    const size_t   MAX_TX_SIZE_LIMIT = BLOCK_GRANTED_FULL_REWARD_ZONE - COINBASE_BLOB_RESERVED_SIZE;

    const uint64_t LOCKED_TX_ALLOWED_DELTA_BLOCKS = 1;
    const uint64_t LOCKED_TX_ALLOWED_DELTA_SECONDS = DIFFICULTY_TARGET * LOCKED_TX_ALLOWED_DELTA_BLOCKS;

    const uint64_t MEMPOOL_TX_LIVETIME = (60 * 60 * 12);
    const uint64_t MEMPOOL_TX_FROM_ALT_BLOCK_LIVETIME = (60 * 60 * 12);
    const uint64_t NUMBER_OF_PERIODS_TO_FORGET_TX_DELETED_FROM_POOL = 7;

    const size_t   FUSION_TX_MAX_SIZE = BLOCK_GRANTED_FULL_REWARD_ZONE * 30 / 100;
    const size_t   FUSION_TX_MIN_INPUT_COUNT = 12;
    const size_t   FUSION_TX_MIN_IN_OUT_COUNT_RATIO = 4;

    static_assert(DEPOSIT_MIN_TERM > 0, "Bad DEPOSIT_MIN_TERM");
    static_assert(DEPOSIT_MIN_TERM <= DEPOSIT_MAX_TERM, "Bad DEPOSIT_MAX_TERM");
    static_assert(DEPOSIT_MIN_TERM* DEPOSIT_MAX_TOTAL_RATE > DEPOSIT_MIN_TOTAL_RATE_FACTOR, "Bad DEPOSIT_MIN_TOTAL_RATE_FACTOR or DEPOSIT_MAX_TOTAL_RATE");
    static_assert(0 < UPGRADE_VOTING_THRESHOLD && UPGRADE_VOTING_THRESHOLD <= 100, "Bad UPGRADE_VOTING_THRESHOLD");
    static_assert(UPGRADE_VOTING_WINDOW > 1, "Bad UPGRADE_VOTING_WINDOW");
  } // namespace parameters

  const Amount    BLOCK_REWARD = (UINT64_C(3) * parameters::COIN);
  const Amount    STATIC_BLOCK_REWARD = (UINT64_C(5) * parameters::COIN);
  const BlockOrTimestamp REWARD_INCREASE_INTERVAL = (parameters::EXPECTED_NUMBER_OF_BLOCKS_PER_DAY * 90);

  const Amount    FOUNDATION_TRUST = (UINT64_C(15000000) * parameters::COIN);

  const char      PROJECT_NAME[] = "Cache";
  const char      DATA_DIR[] = "cache-blockchain-data";

  const char      GENESIS_COINBASE_TX_HEX[] = "010a01ff0001a0c21e029b2e4c0281c0b02e7c53291a94d1d0cbff8883f8024f5142ee494ffbbd08807121015149ab15af1e07d68f42a3de7876aa48c948809445f2eb08b2cb5f1f5006cd58";
  const uint32_t  GENESIS_NONCE = 10000;
  const Timestamp GENESIS_TIMESTAMP = 1605434400;

  const Port      P2P_DEFAULT_PORT = 38999;
  const Port      RPC_DEFAULT_PORT = 39000;

  const Version   P2P_CURRENT_VERSION = 2;
  const Version   P2P_MINIMUM_VERSION = 1;

  const Version   TRANSACTION_VERSION_1 = 1;
  const Version   TRANSACTION_VERSION_2 = 2;

  const Version   BLOCK_MAJOR_VERSION_1 = 1;
  const Version   BLOCK_MAJOR_VERSION_2 = 2;

  const Version   BLOCK_MINOR_VERSION_0 = 0;
  const Version   BLOCK_MINOR_VERSION_1 = 1;

  const size_t    BLOCKS_IDS_SYNCHRONIZING_DEFAULT_COUNT = 10000;
  const size_t    BLOCKS_SYNCHRONIZING_DEFAULT_COUNT = 128;
  const size_t    COMMAND_RPC_GET_BLOCKS_FAST_MAX_COUNT = 1000;

  const size_t    P2P_CONNECTION_MAX_WRITE_BUFFER_SIZE = 64 * 1024 * 1024;
  const size_t    P2P_DEFAULT_ANCHOR_CONNECTIONS_COUNT = 2;
  const uint32_t  P2P_DEFAULT_CONNECTION_TIMEOUT = 5000;
  const uint32_t  P2P_DEFAULT_CONNECTIONS_COUNT = 8;
  const uint32_t  P2P_DEFAULT_HANDSHAKE_INTERVAL = 60;
  const size_t    P2P_DEFAULT_HANDSHAKE_INVOKE_TIMEOUT = 5000;
  const uint64_t  P2P_DEFAULT_INVOKE_TIMEOUT = 60 * 2 * 1000;
  const uint32_t  P2P_DEFAULT_PACKET_MAX_SIZE = 50000000;
  const uint32_t  P2P_DEFAULT_PEERS_IN_HANDSHAKE = 250;
  const uint32_t  P2P_DEFAULT_PING_CONNECTION_TIMEOUT = 2000;
  const size_t    P2P_DEFAULT_WHITELIST_CONNECTIONS_PERCENT = 70;
  const uint32_t  P2P_FAILED_ADDR_FORGET_SECONDS = (60 * 60);
  const uint32_t  P2P_IDLE_CONNECTION_KILL_INTERVAL = (5 * 60);
  const uint32_t  P2P_IP_BLOCKTIME = (60 * 60 * 24);
  const uint32_t  P2P_IP_FAILS_BEFORE_BLOCK = 10;
  const size_t    P2P_LOCAL_GRAY_PEERLIST_LIMIT = 5000;
  const size_t    P2P_LOCAL_WHITE_PEERLIST_LIMIT = 1000;
  const char      P2P_STAT_TRUSTED_PUB_KEY[] = "540D4327ABA4604ED19FBBAF98656C0D39FEAD65F0BA629B6BD08A5A6833C45C";
  const uint8_t  P2P_UPGRADE_WINDOW = 2;
} // namespace CryptoNote

#define ALLOW_DEBUG_COMMANDS
