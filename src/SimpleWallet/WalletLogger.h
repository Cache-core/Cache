// Copyright (c) 2020 - The Cache Developers
//
// Distributed under the GNU Lesser General Public License v3.0.
// Please read Cache/License.md

#pragma once

#include <system_error>

#include <Logging/LoggerRef.h>

using namespace Logging;

namespace CryptoNote
{
  class WalletLogger
  {
    public:
      void multiple_node_or_port(LoggerRef& logger);
      void parse_failed_node(LoggerRef& logger, std::string d);
      void success_node_connect(LoggerRef& logger, std::string d);

      void wallet_menu_prompt();

      void miltiple_generate_and_file(LoggerRef& logger);
      void filename_exists(LoggerRef& logger, std::string w);
      void bad_password(LoggerRef& logger);
      void failed_to_create_wallet(LoggerRef& logger);
      void failed_to_create_wallet(LoggerRef& logger, std::exception e);
      void failed_to_write_wallet(LoggerRef& logger, std::string w);
      void failed_to_load_wallet(LoggerRef& logger, std::exception e);
      void failed_to_create_wallet(LoggerRef& logger, std::error_code e);
      void failed_to_save_wallet(LoggerRef& logger, std::exception e);

      void node_initialize_failed(LoggerRef& logger, std::error_code e);

      void successfully_opened_wallet(LoggerRef& logger, std::string w);
      void successfully_loaded_wallet(LoggerRef& logger, std::string w);
      void successfully_saved_wallet(LoggerRef& logger, std::string w);

      void bad_private_key_for_seed(LoggerRef& logger);
      void invalid_english_word(LoggerRef& logger,std::string eW);
      void invalid_mnemonic(LoggerRef& logger);
      void invalid_mnemonic_size(LoggerRef& logger);
  };
}