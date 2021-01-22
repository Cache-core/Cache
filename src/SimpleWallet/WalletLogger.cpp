// Copyright (c) 2020 - The Cache Developers
//
// Distributed under the GNU Lesser General Public License v3.0.
// Please read Cache/License.md

//#pragma once

#include <string>
#include <iostream>
#include <system_error>

#include "WalletLogger.h"

#include "Common/ColouredMsg.h"

using namespace Logging;

namespace CryptoNote
{
  void WalletLogger::multiple_node_or_port(LoggerRef& logger)
  {
    logger(Logging::ERROR, Logging::BRIGHT_RED) << "You can't use multiple Node Hosts or Ports.";
  }
  
  void WalletLogger::parse_failed_node(LoggerRef& logger, std::string d)
  {
    logger(Logging::ERROR, Logging::BRIGHT_RED) << "Failed to parse Node Host: " << d;
  }
  
  void WalletLogger::success_node_connect(LoggerRef& logger, std::string d)
  {
    logger(Logging::INFO, Logging::BRIGHT_GREEN) << "Successfully connect to Remote Node: " << d;
  }

  void WalletLogger::wallet_menu_prompt()
  {
    std::cout << "" << std::endl
      << "Welcome " << BrightMagentaMsg("Cacher") << ", please choose an option below:" << std::endl 
      << std::endl << BrightMagentaMsg("\t[G]") << " - Generate a new wallet."
      << std::endl << BrightMagentaMsg("\t[O]") << " - Open a wallet already on your system"
      << std::endl << BrightMagentaMsg("\t[S]") << " - Import your wallet using a Seed phrase of words"
      << std::endl << BrightMagentaMsg("\t[I]") << " - Import your wallet using a View Key and Spend Key"
      << std::endl << std::endl << "or, press CTRL_C to exit: "
      << std::flush;
  }

  void WalletLogger::miltiple_generate_and_file(LoggerRef& logger)
  {
    logger(Logging::ERROR, Logging::BRIGHT_RED) << "You can't use \"generate-new-wallet\" and \"wallet-file\" together.";
  }
  
  void WalletLogger::filename_exists(LoggerRef& logger, std::string w)
  {
    logger(Logging::ERROR, Logging::BRIGHT_RED) << "Filename already exists: " << w;
  }

  void WalletLogger::bad_password(LoggerRef& logger)
  {
    logger(Logging::ERROR, Logging::BRIGHT_RED) << "Failed to read password.";
  }

  void WalletLogger::failed_to_create_wallet(LoggerRef& logger)
  {
    logger(Logging::ERROR, Logging::BRIGHT_RED) << "Failed to create wallet.";
  }

  void WalletLogger::failed_to_create_wallet(LoggerRef& logger, std::exception e)
  {
    logger(Logging::ERROR, Logging::BRIGHT_RED) << "Failed to create wallet: " << e.what();
  }
  

  void WalletLogger::failed_to_write_wallet(LoggerRef& logger, std::string w)
  {
    logger(Logging::ERROR, Logging::BRIGHT_RED) << "Failed to write to wallet: " << w;
  }

  void WalletLogger::failed_to_load_wallet(LoggerRef& logger, std::exception e)
  {
    logger(Logging::ERROR, Logging::BRIGHT_RED) << "Failed to load wallet:" << e.what();
  }
  
  void WalletLogger::failed_to_create_wallet(LoggerRef& logger, std::error_code e)
  {
    logger(Logging::ERROR, Logging::BRIGHT_RED) << "Failed to create wallet:" << e.message();
  }

  void WalletLogger::failed_to_save_wallet(LoggerRef& logger, std::exception e)
  {
    logger(Logging::ERROR, Logging::BRIGHT_RED) << "Failed to save wallet:" << e.what();
  }
  
  
  void WalletLogger::node_initialize_failed(LoggerRef& logger, std::error_code e)
  {
    logger(Logging::ERROR, Logging::BRIGHT_RED) << "Node RPC has failed to initialize:" << e.message();
  }
  
  void WalletLogger::successfully_opened_wallet(LoggerRef& logger, std::string w)
  {
    logger(Logging::INFO, Logging::BRIGHT_GREEN) << "Successfully opened wallet: " << w;
  }

  void WalletLogger::successfully_loaded_wallet(LoggerRef& logger, std::string w)
  {
    logger(Logging::INFO, Logging::BRIGHT_GREEN) << "Successfully loaded wallet: " << w;
  }

  void WalletLogger::successfully_saved_wallet(LoggerRef& logger, std::string w)
  {
    logger(Logging::INFO, Logging::BRIGHT_GREEN) << "Successfully saved wallet: " << w;
  }

  void WalletLogger::bad_private_key_for_seed(LoggerRef& logger)
  {
    logger(Logging::ERROR, Logging::BRIGHT_RED) << "Failed to a generate a Mnemonic Seed from the wallets Private Keys.";
  }

  void WalletLogger::invalid_english_word(LoggerRef& logger, std::string eW)
  {
    logger(Logging::ERROR, Logging::BRIGHT_RED) << "\"" << eW << "\" does not seem to be a valid English word.";
  }

  void WalletLogger::invalid_mnemonic(LoggerRef& logger)
  {
    logger(Logging::ERROR, Logging::BRIGHT_RED) << "Invalid mnemonic phrase.";
  }

  void WalletLogger::invalid_mnemonic_size(LoggerRef& logger)
  {
    logger(Logging::ERROR, Logging::BRIGHT_RED) << "Invalid mnemonic phrase. Seed phrase is not 25 words! Please try again.";
  }


}