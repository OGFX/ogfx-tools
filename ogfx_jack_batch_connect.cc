#include <jack/jack.h>
#include <jack/ringbuffer.h>
#include <jack/midiport.h>

#include <boost/program_options.hpp>
#include <string>
#include <assert.h>
#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include <nlohmann/json.hpp>
/*
    A small program to list available jack ports in
    an easy readable JSON format
*/
int main(int argc, char *argv[]) {
    std::string name;
    std::string connections;
    bool disconnect;

    namespace po = boost::program_options;
    po::options_description desc("Allowed options");
    desc.add_options()
        ("help,h",
         "get some help")
        ("name,n",
         po::value<std::string>(&name)->default_value("jack_batch_connect"),
         "the jack client name")
        ("connections,c",
         po::value<std::string>(&connections)->default_value("[]"),
         "a json array of 2-element string arrays naming ports that should get connected")
        ("disconnect,d",
         po::value<bool>(&disconnect)->default_value(false),
         "disconnect instead of connect")
         ;

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if (vm.count("help")) {
        std::cout << desc << std::endl;
        return EXIT_SUCCESS;
    }

    nlohmann::json j = nlohmann::json::parse(connections);

    jack_status_t jack_status;
    jack_client_t *jack_client = jack_client_open(name.c_str(), JackNullOption, &jack_status);

    if (NULL == jack_client) {
        std::cerr << "Failed to create jack client. Exiting..." << std::endl;
        return EXIT_FAILURE;
    }

    for (auto it = j.begin(); it != j.end(); ++it) {
        std::string from = it->at(0).get<std::string>();
        std::string to = it->at(1).get<std::string>();
        std::cout << "Connecting: " << from << " --> " << to << std::endl;
        int ret = jack_connect(jack_client, from.c_str(), to.c_str());
        if (ret != 0) {
            std::cerr << "Failed to connect ports: " << from << " -> "  << to << " (ret: " << ret << ")" << std::endl;
        }
    }

    jack_client_close(jack_client);
}
