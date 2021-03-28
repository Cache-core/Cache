// Copyright (c) 2020 - The Cache Developers
//
// Distributed under the GNU Lesser General Public License v3.0.
// Please read Cache/License.md

#pragma once

#include <cstddef>
#include <initializer_list>

namespace CryptoNote {
	struct CheckpointData {
    uint32_t height;
    const char *blockId;
	};

#ifdef __GNUC__
  __attribute__((unused))
#endif

  /**
   * @param CheckpointData Uses data to help sync with the network quicker and to avoid split-chains.
   */

      const std::initializer_list<CheckpointData> CHECKPOINTS = {
            {0,  "f204990ad47bf5cfa8ead24383ebfc99c099bfff446f272cffbc169ab3de1e0d"},
          {15191,  "983ccab3bc1dbd67d2f7caef25571e91f2ab1f3f7fbfb9437033c2c01e1440a1"},
          {17257,  "4956c42d234d6baf7d0230af5fd34375ca25dd539d9e812e004b63a28364a62d"},
          {20103,  "7f0a160f28003e5e051b61a74e56ffec403d1b847bf907744807d87c9fe35a45"},
          {20988,  "4fbbd656dbddc7460e9de8888b3f67686b863783ca70258af758f5fc447484f7"},
          {25205,  "ad0d6b9b3e84340bdbec63099ff047bced8d9971b48ec7d790cf833490b7f2e2"},
          {25939,  "33ab22874e4599045e116d816a37ec2770322125c3d41da5f423c001b09357ce"},
          {26734,  "2924ed0e12b28125deeed4c1d3193717231fed8d025b46ed963e87ef66c5e793"},
          {26735,  "1a76f0d0f733c3a916343f1f68c1d1571ddb3aeeacbe3abb90ac8c4c96a5e8bf"},
          {27500,  "b473484d96f7d3d6755c68570468cea99666a951d408e7b00db2be17c86dde4c"},
          {30000,  "b12aadc6a418fbae5c603327ba5fdff62f0db35411cae3a068506542d44043ac"},
          {32500,  "eaec1c62a16e7a18f0cc5cc3ce558012449374b21f002a3006bca805b6ed8d21"},
          {35000,  "3fd8289b9f31bb461e8a40dacda69b5424aa486c556ddc561ec26d3d4c8ca02d"},
          {37500,  "670d975251a5721854cf9a7edcc3cd6b7177a786fedadc08202dd1023fff3d79"},
          {40000,  "f7f65cc42a0d885b7a045c3026f894a3b86ecc783147d75196cf293d9c995560"},
          {42500,  "8472d77cfbffc5c4a1559d43e7fd7ab4cea1487357080fcde4e013edb9809b65"},
          {45000,  "75ce15ae067f03ddd999ece24cfce87f2603c06b6efd3d780c8ea281ecc2f7e7"},
          {47500,  "640dc48803c0b51aeff5dd9c03d3dd05076d31448a051ba8ee2b4b341835dd2c"},
          {50000,  "c4424df1757396302def1b0272f0bcef7b5ee6ace65d34d676bf1f42c9231e61"},
          {63913,  "06950adec793fb4337dcc98b3b31439662574ee70cc60a35bce964ae2fb2690d"}
  };
} // namespace CryptoNote
