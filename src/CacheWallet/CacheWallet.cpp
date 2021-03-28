// Copyright (c) 2018-2020 - The TurtleCoin Developers
// Copyright (c) 2020 - The Cache Developers
//
// Distributed under the GNU Lesser General Public License v3.0.
// Please read Cache/License.md

#include <CacheWallet/CacheWallet.h>

int main(int argc, char **argv)
{
    /* On ctrl+c the program seems to throw "simplewallet.exe has stopped
       working" when calling exit(0)... I'm not sure why, this is a bit of
       a hack, it disables that */
    #ifdef _WIN32
    SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX);
    #endif

    Config config = parseArguments(argc, argv);

    /* User requested --help or --version */
    if (config.exit)
    {
        return 0;
    }

    /* Only log to file so we don't have crap filling up the terminal */
    Logging::LoggerManager logManager;
    /* Should we maybe change this to INFO? On the one hand, DEBUGGING really
       spams the file, on the other hand, it exposes a lot of information
       that might help more devs debug a users issues. */
    logManager.setMaxLevel(Logging::DEBUGGING);

    Logging::FileLogger fileLogger;
    fileLogger.init("simplewallet.log");
    logManager.addLogger(fileLogger);

    Logging::LoggerRef logger(logManager, "simplewallet");

    /* Currency contains our coin parameters, such as decimal places, supply */
    CryptoNote::Currency currency 
        = CryptoNote::CurrencyBuilder(logManager).currency();

    System::Dispatcher localDispatcher;
    System::Dispatcher *dispatcher = &localDispatcher;

    /* Our connection to cache-daemon */
    std::unique_ptr<CryptoNote::INode> node(
        new CryptoNote::NodeRpcProxy(config.host, config.port));

    std::promise<std::error_code> errorPromise;
    std::future<std::error_code> error = errorPromise.get_future();
    auto callback = [&errorPromise](std::error_code e) 
                    {errorPromise.set_value(e); };

    node->init(callback);

    std::future<void> initNode = std::async(std::launch::async, [&] {
            if (error.get())
            {
                throw std::runtime_error("Failed to initialize node!");
            }
    });

    std::future_status status = initNode.wait_for(std::chrono::seconds(20));

    /* Connection took to long to remote node, let program continue regardless
       as they could perform functions like export_keys without being
       connected */
    if (status != std::future_status::ready)
    {
        if (config.host != "127.0.0.1")
        {
            std::cout << WarningMsg("Unable to connect to remote node, "
                                    "connection timed out.")
                      << std::endl
                      << WarningMsg("Confirm the remote node is functioning, "
                                    "or try a different remote node.")
                      << std::endl << std::endl;
        }
        else
        {
            std::cout << WarningMsg("Unable to connect to node, "
                                    "connection timed out.")
                      << std::endl << std::endl;
        }
    }

    /* Create the wallet instance */
    CryptoNote::WalletGreen wallet(*dispatcher, currency, *node, 
                                   logger.getLogger());

    /* Run the interactive wallet interface */
    run(wallet, *node, config);
}

void run(CryptoNote::WalletGreen &wallet, CryptoNote::INode &node,
         Config &config)
{
    auto maybeWalletInfo = Nothing<std::shared_ptr<WalletInfo>>();
    Action action;

    do
    {
        std::cout << InformationMsg("Cache v"
                                  + std::string(PROJECT_VERSION)
                                  + " Wallet Beta") << std::endl;

        /* Open/import/generate the wallet */
        action = getAction(config);
        maybeWalletInfo = handleAction(wallet, action, config);

    /* Didn't manage to get the wallet info, returning to selection screen */
    } while (!maybeWalletInfo.isJust);

    auto walletInfo = maybeWalletInfo.x;

    bool alreadyShuttingDown = false;

    /* This will call shutdown when ctrl+c is hit. This is a lambda function,
       & means capture all variables by reference */
    Tools::SignalHandler::install([&] {
        /* If we're already shutting down let control flow continue as normal */
        if (shutdown(walletInfo->wallet, node, alreadyShuttingDown))
        {
            exit(0);
        }
    });

    while (node.getLastKnownBlockHeight() == 0)
    {
        std::cout << RedMsg("It looks like cache-daemon isn't open!")
                  << std::endl << std::endl
                  << RedMsg("Ensure cache-daemon is open and has finished "
                            "initializing.")
                  << std::endl
                  << RedMsg("If it's still not working, try restarting "
                            "cache-daemon. The daemon sometimes gets stuck.") 
                  << std::endl
                  << RedMsg("Alternatively, perhaps cache-daemon can't "
                            "communicate with any peers.")
                  << std::endl << std::endl
                  << RedMsg("The wallet can't function until it can "
                            "communicate with the network.")
                  << std::endl << std::endl;

        bool proceed = false;

        while (true)
        {
            std::cout << "[" << InformationMsg("T") << "]ry again, "
                      << "[" << InformationMsg("E") << "]xit, or "
                      << "[" << InformationMsg("C") << "]ontinue anyway?: ";

            std::string answer;
            std::getline(std::cin, answer);

            char c = std::tolower(answer[0]);

            /* Lets people spam enter in the transaction screen */
            if (c == 't' || c == '\0')
            {
                break;
            }
            else if (c == 'e' || c == std::ifstream::traits_type::eof())
            {
                shutdown(walletInfo->wallet, node, alreadyShuttingDown);
                return;
            }
            else if (c == 'c')
            {
                proceed = true;
                break;
            }
            else
            {
                std::cout << WarningMsg("Bad input: ") << InformationMsg(answer)
                          << WarningMsg(" - please enter either T, E, or C.")
                          << std::endl;
            }
        }

        if (proceed)
        {
            break;
        }

        std::cout << std::endl;
    }

    /* Scan the chain for new transactions. In the case of an imported 
       wallet, we need to scan the whole chain to find any transactions. 
       If we opened the wallet however, we just need to scan from when we 
       last had it open. If we are generating a wallet, there is no need
       to check for transactions as there is no way the wallet can have
       received any money yet. */
    if (action != Generate)
    {
        findNewTransactions(node, walletInfo);
    }
    else
    {
        std::cout << InformationMsg("Your wallet is syncing with the "
                                    "network in the background.")
                  << std::endl
                  << InformationMsg("Until this is completed new "
                                    "transactions might not show up.")
                  << std::endl
                  << InformationMsg("Use bc_height to check the progress.")
                  << std::endl << std::endl;
    }

    welcomeMsg();

    inputLoop(walletInfo, node);
    shutdown(walletInfo->wallet, node, alreadyShuttingDown);
}

Maybe<std::shared_ptr<WalletInfo>> handleAction(CryptoNote::WalletGreen &wallet,
                                                Action action, Config &config)
{
    if (action == Generate)
    {
        return Just<std::shared_ptr<WalletInfo>>(generateWallet(wallet));
    }
    else if (action == Open)
    {
        return openWallet(wallet, config);
    }
    else if (action == Import)
    {
        return Just<std::shared_ptr<WalletInfo>>(importWallet(wallet));
    }
    else if (action == SeedImport)
    {
        return mnemonicImportWallet(wallet);
    }
    else
    {
        throw("Unimplemented action!");
    }
}

std::shared_ptr<WalletInfo> importWallet(CryptoNote::WalletGreen &wallet)
{
    Crypto::SecretKey privateSpendKey = getPrivateKey("Private Spend Key: ");
    Crypto::SecretKey privateViewKey = getPrivateKey("Private View Key: ");
    return importFromKeys(wallet, privateSpendKey, privateViewKey);
}

std::shared_ptr<WalletInfo> mnemonicImportWallet(CryptoNote::WalletGreen
                                                 &wallet)
{
    std::string mnemonicPhrase;

    Crypto::SecretKey privateSpendKey;
    Crypto::SecretKey privateViewKey;

    do
    {
        std::cout << "Mnemonic Phrase (25 words): ";
        std::getline(std::cin, mnemonicPhrase);
        boost::algorithm::trim(mnemonicPhrase);
    }
    while (!isValidMnemonic(mnemonicPhrase, privateSpendKey));

    CryptoNote::AccountBase::generateViewFromSpend(privateSpendKey, 
                                                   privateViewKey);

    return importFromKeys(wallet, privateSpendKey, privateViewKey);
}

std::shared_ptr<WalletInfo> importFromKeys(CryptoNote::WalletGreen &wallet,
                                           Crypto::SecretKey privateSpendKey, 
                                           Crypto::SecretKey privateViewKey)
{
    std::string walletFileName = getNewWalletFileName();
    std::string walletPass = getWalletPassword(true);

    std::cout << std::endl << "Making initial contact with cache-daemon."
              << std::endl << "Please note, this sometimes can take a long "
              << "time." << std::endl << "Please wait..." << std::endl
              << std::endl;

    wallet.initializeWithViewKey(walletFileName, walletPass, privateViewKey);

    std::string walletAddress = wallet.createAddress(privateSpendKey);

    std::cout << YellowMsg("\nYour wallet " + walletAddress 
                         + " has been successfully imported!")
              << std::endl << std::endl;

    return std::make_shared<WalletInfo>(walletFileName, walletPass, 
                                        walletAddress, wallet);
}

std::shared_ptr<WalletInfo> generateWallet(CryptoNote::WalletGreen &wallet)
{
    std::string walletFileName = getNewWalletFileName();
    std::string walletPass = getWalletPassword(true);

    std::cout << std::endl << "Making initial contact with cache-daemon."
              << std::endl << "Please note, this sometimes can take a long "
              << "time." << std::endl << "Please wait..." << std::endl
              << std::endl;

    CryptoNote::KeyPair spendKey;
    Crypto::SecretKey privateViewKey;

    Crypto::generate_keys(spendKey.publicKey, spendKey.secretKey);
    CryptoNote::AccountBase::generateViewFromSpend(spendKey.secretKey,
                                                   privateViewKey);

    wallet.initializeWithViewKey(walletFileName, walletPass, privateViewKey);

    std::string walletAddress = wallet.createAddress(spendKey.secretKey);

    promptSaveKeys(wallet);

    std::cout << RedMsg("If you lose these your wallet cannot be recreated!")
              << std::endl << std::endl;

    return std::make_shared<WalletInfo>(walletFileName, walletPass,
                                        walletAddress, wallet);
}

Maybe<std::shared_ptr<WalletInfo>> openWallet(CryptoNote::WalletGreen &wallet,
                                              Config &config)
{
    std::string walletFileName = getExistingWalletFileName(config);

    bool initial = true;

    while (true)
    {
        std::string walletPass;

        /* Only use the command line pass once, otherwise we will infinite
           loop if it is incorrect */
        if (initial && config.passGiven)
        {
            walletPass = config.walletPass;
        }
        else
        {
            walletPass = getWalletPassword(false);
        }

        initial = false;

        std::cout << std::endl << "Making initial contact with cache-daemon."
                  << std::endl << "Please note, this sometimes can take a "
                  << "long time." << std::endl << "Please wait..."
                  << std::endl << std::endl;

        try
        {
            wallet.load(walletFileName, walletPass);

            std::string walletAddress = wallet.getAddress(0);

            std::cout << std::endl
                      << YellowMsg("Your wallet " + walletAddress
                                 + " has been successfully opened!\n\n");

                return Just<std::shared_ptr<WalletInfo>>
                           (std::make_shared<WalletInfo>(walletFileName,
                                                         walletPass, 
                                                         walletAddress,
                                                         wallet));
        }
        catch (const std::system_error& e)
        {
            std::string walletGreenBadPwdMsg = 
                "Restored view public key doesn't correspond to secret key: "
                "The password is wrong";

            std::string walletGreenBadPwdMsg2 =
                "Restored spend public key doesn't correspond to secret key: "
                "The password is wrong";

            std::string walletLegacyBadPwdMsg =
                ": The password is wrong";

            std::string alreadyOpenMsg =
                "MemoryMappedFile::open: The process cannot access the file "
                "because it is being used by another process.";

            std::string notAWalletMsg =
                "Unsupported wallet version: Wrong version";

            std::string errorMsg = e.what();
                
            /* There are two different error messages depending upon if we're
               opening a walletgreen or a walletlegacy wallet */
            if (errorMsg == walletGreenBadPwdMsg || 
                errorMsg == walletGreenBadPwdMsg2 ||
                errorMsg == walletLegacyBadPwdMsg)
            {
                std::cout << RedMsg("Incorrect password! Try again.")
                          << std::endl;
            }
            /* The message actually has a \r\n on the end but i'd prefer to
               keep just the raw string in the source so check the it starts
               with instead */
            else if (boost::starts_with(errorMsg, alreadyOpenMsg))
            {
                std::cout << WarningMsg("Could not open wallet! It is already "
                                        "open in another process.")
                          << std::endl
                          << WarningMsg("Check with a task manager that you "
                                        "don't have simplewallet open twice.")
                          << std::endl
                          << WarningMsg("Also check you don't have another "
                                        "wallet program open, such as a GUI "
                                        "wallet or walletd.")
                          << std::endl << std::endl;

                std::cout << "Returning to selection screen..." << std::endl
                          << std::endl;

                return Nothing<std::shared_ptr<WalletInfo>>();
            }
            else if (errorMsg == notAWalletMsg)
            {
                std::cout << WarningMsg("Could not open wallet file! It "
                                        "doesn't appear to be a valid wallet!")
                          << std::endl
                          << WarningMsg("Ensure you are opening a wallet "
                                        "file, and the file has not gotten "
                                        "corrupted.")
                          << std::endl
                          << WarningMsg("Try reimporting via keys, and always "
                                        "close simplewallet with the exit "
                                        "command to prevent corruption.")
                          << std::endl << std::endl;
                std::cout << "Returning to selection screen..." << std::endl
                          << std::endl;

                return Nothing<std::shared_ptr<WalletInfo>>();
            }
            else
            {
                std::cout << "Unexpected error: " << errorMsg << std::endl;
                std::cout << "Please report this error message and what "
                          << "you did to cause it." << std::endl << std::endl;

                std::cout << "Returning to selection screen..." << std::endl
                          << std::endl;

                return Nothing<std::shared_ptr<WalletInfo>>();
            }
        }
    }
}

Crypto::SecretKey getPrivateKey(std::string msg)
{
    size_t privateKeyLen = 64;
    size_t size;

    std::string privateKeyString;
    Crypto::Hash privateKeyHash;
    Crypto::SecretKey privateKey;
    Crypto::PublicKey publicKey;

    while (true)
    {
        std::cout << msg;

        std::getline(std::cin, privateKeyString);
        boost::algorithm::trim(privateKeyString);

        if (privateKeyString.length() != privateKeyLen)
        {
            std::cout << RedMsg("Invalid private key, should be 64 "
                                "characters! Try again.") << std::endl;
            continue;
        }
        else if (!Common::fromHex(privateKeyString, &privateKeyHash, 
                  sizeof(privateKeyHash), size)
                 || size != sizeof(privateKeyHash))
        {
            std::cout << RedMsg("Invalid private key, failed to parse! "
                                "Ensure you entered it correctly.")
                      << std::endl;
            continue;
        }

        privateKey = *(struct Crypto::SecretKey *) &privateKeyHash;

        /* Just used for verification purposes before we pass it to
           walletgreen */
        if (!Crypto::secret_key_to_public_key(privateKey, publicKey))
        {
            std::cout << "Invalid private key, failed to parse! Ensure "
                         "you entered it correctly." << std::endl;
            continue;
        }

        return privateKey;
    }
}

std::string getExistingWalletFileName(Config &config)
{
    bool initial = true;
    std::string walletName;

    while (true)
    {
        /* Only use wallet file once in case it is incorrect */
        if (config.walletGiven && initial)
        {
            walletName = config.walletFile;
        }
        else
        {
            std::cout << "What is the name of the wallet you want to open?: ";
            std::getline(std::cin, walletName);
        }

        initial = false;

        std::string walletFileName = walletName + ".wallet";

        if (walletName == "")
        {
            std::cout << WarningMsg("Wallet name can't be blank! Try again.")
                      << std::endl;
        }
        /* Allow people to enter wallet name with or without file extension */
        else if (boost::filesystem::exists(walletName))
        {
            return walletName;
        }
        else if (boost::filesystem::exists(walletFileName))
        {
            return walletFileName;
        }
        else
        {
            std::cout << WarningMsg("A wallet with the filename " 
                                  + walletFileName + " doesn't exist!")
                      << std::endl
                      << "Ensure you entered your wallet name correctly."
                      << std::endl;
        }
    }
}

std::string getNewWalletFileName()
{
    std::string walletName;

    while (true)
    {
        std::cout << "What would you like to call your new wallet?: ";
        std::getline(std::cin, walletName);

        std::string walletFileName = walletName + ".wallet";

        if (boost::filesystem::exists(walletFileName))
        {
            std::cout << RedMsg("A wallet with the filename " + walletFileName
                              + " already exists!") << std::endl
                      << "Try another name." << std::endl;
        }
        else if (walletName == "")
        {
            std::cout << RedMsg("Wallet name can't be blank! Try again.")
                      << std::endl;
        }
        else
        {
            return walletFileName;
        }
    }
}

std::string getWalletPassword(bool verifyPwd)
{
    Tools::PasswordContainer pwdContainer;
    pwdContainer.read_password(verifyPwd);
    return pwdContainer.password();
}

Action getAction(Config &config)
{
    if (config.walletGiven || config.passGiven)
    {
        return Open;
    }

    while (true)
    {
        std::cout << std::endl << "Welcome, please choose an option below:"
                  << std::endl << std::endl << "\t[" << YellowMsg("G")
                  << "] - Generate a new wallet address" << std::endl 
                  << "\t[" << YellowMsg("O") << "] - Open a wallet already "
                  << "on your system" << std::endl << "\t[" << YellowMsg("S")
                  << "] - Regenerate your wallet using a seed phrase of words"
                  << std::endl << "\t[" << YellowMsg("I") << "] - Import "
                  << "your wallet using a View Key and Spend Key"
                  << std::endl << std::endl
                  << "or, press CTRL_C to exit: ";

        std::string answer;
        std::getline(std::cin, answer);

        char c = answer[0];
        c = std::tolower(c);

        if (c == 'o')
        {
            return Open;
        }
        else if (c == 'g')
        {
            return Generate;
        }
        else if (c == 'i')
        {
            return Import;
        }
        else if (c == 's')
        {
            return SeedImport;
        }
        else
        {
            std::cout << "Unknown command: " << RedMsg(answer) << std::endl;
        }
    }
}

bool isValidMnemonic(std::string &mnemonic_phrase, 
                     Crypto::SecretKey &private_spend_key)
{

  /* Uncommenting these will allow importing of different languages, exporting
     in different languages however has not been added, as it will require
     changing the export_keys command to take an argument to specify what
     language the seed should be exported in. For now, multilanguage support
     has been disabled as there are a couple of issues - we can't print out
     what words aren't present in the dictionary if we don't know what
     dictionary they are using, and it's a lot more friendly to work that
     out automatically rather than asking, and secondly, it is possible that
     dictionaries of other words can overlap enough to allow an esperanto
     seed for example to be imported as an english seed */

    /*
    static std::string languages[] = {"English", "Nederlands", "Français", 
                                      "Português", "Italiano", "Deutsch", 
                                      "русский язык", "简体中文 (中国)", 
                                      "Esperanto", "Lojban"};

    static const int num_of_languages = 10;
    */

    static std::string languages[] = {"English"};

    static const int num_of_languages = 1;

    static const int mnemonic_phrase_length = 25;

    std::vector<std::string> words;

    words = boost::split(words, mnemonic_phrase, ::isspace);

    if (words.size() != mnemonic_phrase_length)
    {
        std::cout << RedMsg("Invalid mnemonic phrase! Seed phrase is not 25 "
                            "words! Please try again.") << std::endl;

        logIncorrectMnemonicWords(words);
        return false;
    }

    /* Check every language for our phrase so the user doesn't have to specify
    it, this shouldn't be an issue as long as one language doesn't have enough
    of another languages words, might need some testing */
    for (int i = 0; i < num_of_languages; i++)
    {
        if (crypto::ElectrumWords::words_to_bytes(mnemonic_phrase, 
                                                  private_spend_key,
                                                  languages[i]))
        {
            return true;
        }
    }

    /* The issue with this is if we try and automagically determine what
    language the seed phrase is in, then we can't log words which aren't in the
    dictionary, we will have to take an argument to know what language they
    are in, but this is less user friendly. */
    std::cout << RedMsg("Invalid mnemonic phrase!") << std::endl;
    logIncorrectMnemonicWords(words);

    return false;
}

void logIncorrectMnemonicWords(std::vector<std::string> words)
{
    Language::Base *language 
        = Language::Singleton<Language::English>::instance();

    const std::vector<std::string> &dictionary = language->get_word_list();

    for (auto i : words)
    {
        if (std::find(dictionary.begin(), dictionary.end(), i) 
                   == dictionary.end())
        {
            std::cout << YellowMsg(i) << RedMsg(" is not in the english word "
                                                "list!") << std::endl;
        }
    }
}

void promptSaveKeys(CryptoNote::WalletGreen &wallet)
{
    std::cout << "Welcome to your new wallet, here is your payment address:"
              << std::endl << YellowMsg(wallet.getAddress(0))
              << std::endl << std::endl 
              << "Please copy your secret keys and mnemonic seed and store "
              << "them in a secure location: " << std::endl;

    printPrivateKeys(wallet);

    std::cout << std::endl;
}

void exportKeys(std::shared_ptr<WalletInfo> &walletInfo)
{
    confirmPassword(walletInfo->walletPass);
    printPrivateKeys(walletInfo->wallet);
}

void printPrivateKeys(CryptoNote::WalletGreen &wallet)
{
    Crypto::SecretKey privateSpendKey = wallet.getAddressSpendKey(0).secretKey;
    Crypto::SecretKey privateViewKey = wallet.getViewKey().secretKey;

    Crypto::SecretKey derivedPrivateViewKey;

    CryptoNote::AccountBase::generateViewFromSpend(privateSpendKey,
                                                   derivedPrivateViewKey);

    bool deterministicPrivateKeys = derivedPrivateViewKey == privateViewKey;

    std::cout << GreenMsg("Private spend key: " 
                        + Common::podToHex(privateSpendKey)) << std::endl
              << GreenMsg("Private view key: " 
                        + Common::podToHex(privateViewKey)) << std::endl;

    if (deterministicPrivateKeys)
    {
        std::string mnemonicSeed;

        crypto::ElectrumWords::bytes_to_words(privateSpendKey, 
                                              mnemonicSeed,
                                              "English");

        std::cout << GreenMsg("Mnemonic seed: " + mnemonicSeed) << std::endl;
    }
}

void welcomeMsg()
{
    std::cout << "Use the " << YellowMsg("help") << " command to see the list "
              << "of available commands." << std::endl << "Use "
              << YellowMsg("exit") << " when closing to ensure your wallet "
              << "file doesn't get corrupted." << std::endl << std::endl;
}

std::string getInputAndDoWorkWhileIdle(std::shared_ptr<WalletInfo> &walletInfo)
{
    auto lastUpdated = std::chrono::system_clock::now();

    std::future<std::string> inputGetter = std::async(std::launch::async, [] {
            std::string command;
            std::getline(std::cin, command);
            return command;
    });
    while (true)
    {
        /* Check if the user has inputted something yet (Wait for zero seconds
           to instantly return) */
        std::future_status status = inputGetter.wait_for(std::chrono::seconds(0));
        /* User has inputted, get what they inputted and return it */
        if (status == std::future_status::ready)
        {
            return inputGetter.get();
        }

        auto currentTime = std::chrono::system_clock::now();

        /* Otherwise check if we need to update the wallet cache */
        if ((currentTime - lastUpdated) > std::chrono::seconds(5))
        {
            lastUpdated = currentTime;
            checkForNewTransactions(walletInfo);
        }

        /* Sleep for enough for it to not be noticeable when the user enters
           something, but enough that we're not starving the CPU */
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}

void inputLoop(std::shared_ptr<WalletInfo> &walletInfo, CryptoNote::INode &node)
{
    while (true)
    {
        std::cout << getPrompt(walletInfo);

        std::string command = getInputAndDoWorkWhileIdle(walletInfo);

        /* Split into args to support legacy transfer command, for example
           transfer 5 cxchexyz... 100, sends 100 $CXCHE to cxchexyz... with a mixin
           of 5 */
        std::vector<std::string> words;
        words = boost::split(words, command, ::isspace);

        if (command == "")
        {
            // no-op
        }
        else if (command == "export_keys")
        {
            exportKeys(walletInfo);
        }
        else if (command == "help")
        {
            help();
        }
        else if (command == "balance")
        {
            balance(node, walletInfo->wallet);
        }
        else if (command == "address")
        {
            std::cout << GreenMsg(walletInfo->walletAddress) << std::endl;
        }
        else if (command == "incoming_transfers")
        {
            listTransfers(true, false, walletInfo->wallet);
        }
        else if (command == "outgoing_transfers")
        {
            listTransfers(false, true, walletInfo->wallet);
        }
        else if (command == "list_transfers")
        {
            listTransfers(true, true, walletInfo->wallet);
        }
        else if (command == "transfer")
        {
            transfer(walletInfo);
        }
        else if (words[0] == "transfer")
        {
            /* remove the first item from words - this is the "transfer"
               command, leaving us just the transfer arguments. */
            words.erase(words.begin());
            transfer(walletInfo, words);
        }
        else if (command == "exit")
        {
            return;
        }
        else if (command == "save")
        {
            std::cout << InformationMsg("Saving.") << std::endl;
            walletInfo->wallet.save();
            std::cout << InformationMsg("Saved.") << std::endl;
        }
        else if (command == "bc_height")
        {
            blockchainHeight(node, walletInfo->wallet);
        }
        else if (command == "quick_optimize")
        {
            quickOptimize(walletInfo->wallet);
        }
        else if (command == "full_optimize")
        {
            fullOptimize(walletInfo->wallet);
        }
        else if (command == "reset")
        {
            reset(node, walletInfo);
        }
        else
        {
            std::cout << "Unknown command: " << RedMsg(command) 
                      << ", use " << YellowMsg("help") << " command to list "
                      << "all possible commands." << std::endl;
        }
    }
}

void help()
{
    std::cout << "Available commands:" << std::endl
              << GreenMsg("help", 25)
              << "List this help message" << std::endl
              << GreenMsg("quick_optimize", 25)
              << "Quickly optimize your wallet to send large amounts"
              << std::endl
              << GreenMsg("full_optimize", 25)
              << "Fully optimize your wallet to send large amounts"
              << std::endl
              << GreenMsg("reset", 25)
              << "Discard cached data and recheck for transactions" << std::endl
              << GreenMsg("bc_height", 25)
              << "Show the blockchain height" << std::endl
              << GreenMsg("balance", 25)
              << "Display how much $CXCHE you have" << std::endl
              << GreenMsg("export_keys", 25)
              << "Export your private keys" << std::endl
              << GreenMsg("address", 25)
              << "Displays your payment address" << std::endl
              << GreenMsg("incoming_transfers", 25)
              << "Show incoming transfers" << std::endl
              << GreenMsg("outgoing_transfers", 25)
              << "Show outgoing transfers" << std::endl
              << GreenMsg("list_transfers", 25)
              << "Show all transfers" << std::endl
              << GreenMsg("transfer", 25)
              << "Send $CXCHE to someone" << std::endl
              << SuccessMsg("save", 25)
              << "Save your wallet state" << std::endl
              << GreenMsg("exit", 25)
              << "Exit and save your wallet" << std::endl;
}

void balance(CryptoNote::INode &node, CryptoNote::WalletGreen &wallet)
{
    uint64_t unconfirmedBalance = wallet.getPendingBalance();
    uint64_t confirmedBalance = wallet.getActualBalance();
    uint64_t totalBalance = unconfirmedBalance + confirmedBalance;

    uint32_t localHeight = node.getLastLocalBlockHeight();
    uint32_t remoteHeight = node.getLastKnownBlockHeight();
    uint32_t walletHeight = wallet.getBlockCount();

    if (localHeight < remoteHeight)
    {
        std::cout << std::endl
                  << "Your daemon is not fully synced with the network!"
                  << std::endl << "Your balance may be incorrect until you "
                  << "are fully synced!" << std::endl;
    }
    /* Small buffer because wallet height doesn't update instantly like node
       height does */
    else if (walletHeight + 1000 < remoteHeight)
    {
        std::cout << std::endl
                  << "The blockchain is still being scanned for incoming "
                  << "payments, and so your balance may be incorrect until "
                  << "this is complete!" << std::endl;
    }

    std::cout << "Available balance: "
              << GreenMsg(formatAmount(confirmedBalance)) << std::endl
              << "Locked (unconfirmed) balance: "
              << RedMsg(formatAmount(unconfirmedBalance))
              << std::endl << "Total balance: "
              << YellowMsg(formatAmount(totalBalance)) << std::endl;
}

void blockchainHeight(CryptoNote::INode &node, CryptoNote::WalletGreen &wallet)
{
    uint32_t localHeight = node.getLastLocalBlockHeight();
    uint32_t remoteHeight = node.getLastKnownBlockHeight();
    uint32_t walletHeight = wallet.getBlockCount();

    /* This is the height that the wallet has been scanned to. The blockchain
       can be fully updated, but we have to walk the chain to find our
       transactions, and this number indicates that progress. */
    std::cout << "Wallet blockchain height: ";

    /* Small buffer because wallet height doesn't update instantly like node
       height does */
    if (walletHeight + 1000 > remoteHeight)
    {
        std::cout << GreenMsg(std::to_string(walletHeight));
    }
    else
    {
        std::cout << RedMsg(std::to_string(walletHeight));
    }

    std::cout << std::endl << "Local blockchain height: ";

    if (localHeight == remoteHeight)
    {
        std::cout << GreenMsg(std::to_string(localHeight));
    }
    else
    {
        std::cout << RedMsg(std::to_string(localHeight));
    }

    std::cout << std::endl << "Network blockchain height: "
              << GreenMsg(std::to_string(remoteHeight)) << std::endl;

    if (localHeight == 0 && remoteHeight == 0)
    {
        std::cout << "Uh oh, it looks like you don't have cache-daemon open!"
                  << std::endl;
    }
    else if (localHeight == remoteHeight)
    {
        std::cout << "Yay! You are synced!" << std::endl;
    }
    else
    {
        std::cout << "Be patient, you are still syncing with the network!"
                  << std::endl;
    }
}

bool shutdown(CryptoNote::WalletGreen &wallet, CryptoNote::INode &node, bool &alreadyShuttingDown)
{
    if (alreadyShuttingDown)
    {
        std::cout << "Patience, we're already shutting down!" 
                  << std::endl;
        return false;
    }
    else
    {
        alreadyShuttingDown = true;
        std::cout << YellowMsg("Saving wallet and shutting down, please "
                               "wait...") << std::endl;
    }

    bool finishedShutdown = false;

    boost::thread timelyShutdown([&finishedShutdown]
    {
        auto startTime = std::chrono::system_clock::now();
        /* Has shutdown finished? */
        while (!finishedShutdown)
        {
            auto currentTime = std::chrono::system_clock::now();

            /* If not, wait for a max of 20 seconds then force exit. */
            if ((currentTime - startTime) > std::chrono::seconds(20))
            {
                std::cout << WarningMsg("Wallet took too long to save! "
                                        "Force closing.") << std::endl
                          << "Bye." << std::endl;
                exit(0);
            }

            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    });

    wallet.save();
    wallet.shutdown();
    node.shutdown();

    finishedShutdown = true;

    /* Wait for shutdown watcher to finish */
    timelyShutdown.join();

    std::cout << "Bye." << std::endl;
    return true;
}

void printOutgoingTransfer(CryptoNote::WalletTransaction t)
{
    std::cout << WarningMsg("Outgoing transfer: " + Common::podToHex(t.hash))
              << std::endl
              << WarningMsg("Spent: " + formatAmount(-t.totalAmount - t.fee))
              << std::endl
              << WarningMsg("Fee: " + formatAmount(t.fee))
              << std::endl
              << WarningMsg("Total Spent: " + formatAmount(-t.totalAmount))
              << std::endl << std::endl;
}

void printIncomingTransfer(CryptoNote::WalletTransaction t)
{
    std::cout << SuccessMsg("Incoming transfer: " + Common::podToHex(t.hash))
              << std::endl
              << SuccessMsg("Amount: " + formatAmount(t.totalAmount))
              << std::endl << std::endl;
}

void listTransfers(bool incoming, bool outgoing, 
                   CryptoNote::WalletGreen &wallet)
{
    size_t numTransactions = wallet.getTransactionCount();
    int64_t totalSpent = 0;
    int64_t totalReceived = 0;

    for (size_t i = 0; i < numTransactions; i++)
    {
        CryptoNote::WalletTransaction t = wallet.getTransaction(i);

        if (t.totalAmount < 0 && outgoing)
        {
            printOutgoingTransfer(t);
            totalSpent += -t.totalAmount;
        }
        else if (t.totalAmount > 0 && incoming)
        {
            printIncomingTransfer(t);
            totalReceived += t.totalAmount;
        }
    }

    if (incoming)
    {
        std::cout << GreenMsg("Total received: " + formatAmount(totalReceived))
                  << std::endl;
    }

    if (outgoing)
    {
        std::cout << RedMsg("Total spent: " + formatAmount(totalSpent))
                  << std::endl;
    }
}

void checkForNewTransactions(std::shared_ptr<WalletInfo> &walletInfo)
{
    walletInfo->wallet.updateInternalCache();
    size_t newTransactionCount = walletInfo->wallet.getTransactionCount();

    if (newTransactionCount != walletInfo->knownTransactionCount)
    {
        for (size_t i = walletInfo->knownTransactionCount; 
                    i < newTransactionCount; i++)
        {
            CryptoNote::WalletTransaction t 
                = walletInfo->wallet.getTransaction(i);

            /* Don't print outgoing or fusion transfers */
            if (t.totalAmount > 0)
            {
                std::cout << std::endl
                          << InformationMsg("New transaction found!")
                          << std::endl
                          << SuccessMsg("Incoming transfer: " 
                                      + Common::podToHex(t.hash))
                          << std::endl
                          << SuccessMsg("Amount: "
                                      + formatAmount(t.totalAmount))
                          << std::endl
                          << getPrompt(walletInfo)
                          << std::flush;
            }
        }
        walletInfo->knownTransactionCount = newTransactionCount;
    }
}

void reset(CryptoNote::INode &node, std::shared_ptr<WalletInfo> &walletInfo)
{
    std::cout << YellowMsg("Resetting wallet...") << std::endl;

    /* Wallet is now unitialized. You must reinit with load, initWithKeys,
       or whatever. This function wipes the cache, then saves the wallet. */
    walletInfo->wallet.clearCacheAndShutdown();

    /* Now, we reopen the wallet. It now has no cached tx's, and balance */
    walletInfo->wallet.load(walletInfo->walletFileName,
                            walletInfo->walletPass);

    /* Now we rescan the chain to re-discover our balance and transactions */
    findNewTransactions(node, walletInfo);
}

void findNewTransactions(CryptoNote::INode &node, 
                         std::shared_ptr<WalletInfo> &walletInfo)
{
    uint32_t localHeight = node.getLastLocalBlockHeight();
    uint32_t walletHeight = walletInfo->wallet.getBlockCount();
    uint32_t remoteHeight = node.getLastKnownBlockHeight();
    size_t transactionCount = walletInfo->wallet.getTransactionCount();

    int stuckCounter = 0;

    if (localHeight != remoteHeight)
    {
        std::cout << "Your cache-daemon isn't fully synced yet!" << std::endl
                  << "Until you are fully synced, you won't be able to send "
                  << "transactions, and your balance may be missing or "
                  << "incorrect!" << std::endl << std::endl;
    }

    /* If we open a legacy wallet then it will load the transactions but not
       have the walletHeight == transaction height. Lets just throw away the
       transactions and rescan. */
    if (walletHeight == 1 && transactionCount != 0)
    {
        std::cout << "Upgrading your wallet from an older version of the "
                  << "software..." << std::endl << "Unfortunately, we have "
                  << "to rescan the chain to find your transactions."
                  << std::endl;
        transactionCount = 0;
        walletInfo->wallet.clearCaches(true, false);
    }

    if (walletHeight == 1)
    {
        std::cout << "Scanning through the blockchain to find transactions "
                     "that belong to you." << std::endl << "Please wait, this "
                     "will take some time." << std::endl << std::endl;
    }
    else
    {
        std::cout << "Scanning through the blockchain to find any new "
                     "transactions you received whilst your wallet wasn't "
                     "open." << std::endl << "Please wait, this may take some "
                     "time." << std::endl << std::endl;
    }

    while (walletHeight < localHeight)
    {
        int counter = 1;

        /* This MUST be called on the main thread! */
        walletInfo->wallet.updateInternalCache();

        localHeight = node.getLastLocalBlockHeight();
        remoteHeight = node.getLastKnownBlockHeight();
        std::cout << SuccessMsg(std::to_string(walletHeight))
                  << " of " << InformationMsg(std::to_string(localHeight))
                  << std::endl;

        uint32_t tmpWalletHeight = walletInfo->wallet.getBlockCount();

        int waitSeconds = 1;
        /* Save periodically so if someone closes before completion they don't
           lose all their progress */
        if (counter % 60 == 0)
        {
            walletInfo->wallet.save();
        }

        if (tmpWalletHeight == walletHeight)
        {
            stuckCounter++;
            waitSeconds = 3;

            if (stuckCounter > 20)
            {
                std::string warning =
                    "Syncing may be stuck. Try restarting cache-daemon.\n"
                    "If this persists, visit "
                    "https://discord.gg/smaGAbP for support.";
                std::cout << WarningMsg(warning) << std::endl;
            }
            else if (stuckCounter > 19)
            {
                /*
                   Calling save has the side-effect of starting
                   and stopping blockchainSynchronizer, which seems
                   to sometimes force the sync to resume properly.
                   So we'll try this before warning the user.
                */
                std::cout << InformationMsg("Saving wallet.") << std::endl;
                walletInfo->wallet.save();
                waitSeconds = 5;
            }
        }
        else
        {
            stuckCounter = 0;
            walletHeight = tmpWalletHeight;
            size_t tmpTransactionCount = walletInfo->wallet.getTransactionCount();
            if (tmpTransactionCount != transactionCount)
            {
                for (size_t i = transactionCount; i < tmpTransactionCount; i++)
                {
                    CryptoNote::WalletTransaction t
                        = walletInfo->wallet.getTransaction(i);

                    /* Don't print out fusion transactions */
                    if (t.totalAmount != 0)
                    {
                        std::cout << std::endl
                                  << InformationMsg("New transaction found!")
                                  << std::endl << std::endl;

                        if (t.totalAmount < 0)
                        {
                            printOutgoingTransfer(t);
                        }
                        else
                        {
                            printIncomingTransfer(t);
                        }
                    }
                }
                transactionCount = tmpTransactionCount;
            }
        }
        counter++;

        std::this_thread::sleep_for(std::chrono::seconds(waitSeconds));
    }

    std::cout << GreenMsg("Finished scanning blockchain!") << std::endl
              << std::endl;

    /* In case the user force closes, we don't want them to have to rescan
       the whole chain. */
    walletInfo->wallet.save();

    walletInfo->knownTransactionCount = transactionCount;
}

ColouredMsg getPrompt(std::shared_ptr<WalletInfo> &walletInfo)
{
    const int promptLength = 20;
    const std::string extension = ".wallet";

    std::string walletName = walletInfo->walletFileName;

    /* Filename ends in .wallet, remove extension */
    if (std::equal(extension.rbegin(), extension.rend(), 
                   walletInfo->walletFileName.rbegin()))
    {
        size_t extPos = walletInfo->walletFileName.find_last_of('.');

        walletName = walletInfo->walletFileName.substr(0, extPos);
    }

    std::string shortName = walletName.substr(0, promptLength);
    return InformationMsg("[cxche " + shortName + "]: ");
}