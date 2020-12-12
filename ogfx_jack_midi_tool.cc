#include <jack/jack.h>
#include <jack/ringbuffer.h>
#include <jack/midiport.h>

#include <boost/program_options.hpp>
#include <string>
#include <assert.h>
#include <iostream>
#include <cstdlib>
#include <unistd.h>

jack_port_t *in0;
jack_port_t *out0;

jack_ringbuffer_t *in_ringbuffer;
jack_ringbuffer_t *out_ringbuffer;

const size_t message_size = 3;

size_t maximum_number_of_messages = 0;

extern "C" {
  int process(jack_nframes_t nframes, void *arg) {
    void *in0_buffer = jack_port_get_buffer(in0, nframes);
    void *out0_buffer = jack_port_get_buffer(out0, nframes);

    jack_midi_clear_buffer(out0_buffer);

    size_t time = 0;
    while (jack_ringbuffer_read_space(out_ringbuffer) >= message_size) {
        // std::cout << "writing stuff.." << std::endl;
        jack_midi_data_t *out_midi_data = jack_midi_event_reserve(out0_buffer, time, message_size);
        char data[message_size];
        jack_ringbuffer_read(out_ringbuffer, data, message_size);

        out_midi_data[0] = data[0];
        out_midi_data[1] = data[1];
        out_midi_data[2] = data[2];
    
        jack_midi_event_write(out0_buffer, time, out_midi_data, message_size);

        ++time;
    }

    jack_nframes_t number_of_events = jack_midi_get_event_count(in0_buffer);

    for (jack_nframes_t event_index = 0; event_index < number_of_events; ++event_index) {
      jack_midi_event_t event;

      // std::cout << "event\n";
      jack_midi_event_get(&event, in0_buffer, event_index);

      if (event.size == 3) { 
      	jack_ringbuffer_write(in_ringbuffer, (const char*)event.buffer, message_size);
      }
    }
    
    return 0;
  }
}

int main(int argc, char *argv[]) {
  std::string name;
  
  namespace po = boost::program_options;
  po::options_description desc("Allowed options");
  desc.add_options()
    ("help,h",
     "get some help")
    ("name,n",
     po::value<std::string>(&name)->default_value("ogfx_jack_midi_tool"),
     "the jack client name")
    ("maximum-messages,m",
     po::value<size_t>(&maximum_number_of_messages)->default_value(50),
     "the maximum number of messages to receive on stdout at once");

  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, desc), vm);
  po::notify(vm);

  if (vm.count("help")) {
    std::cout << desc << std::endl;
    return EXIT_SUCCESS;
  }

  in_ringbuffer = jack_ringbuffer_create(2 << 16);
  out_ringbuffer = jack_ringbuffer_create(2 << 16);
  
  jack_status_t jack_status;
  jack_client_t *jack_client = jack_client_open(name.c_str(), JackNullOption, &jack_status);

  if (NULL == jack_client) {
    std::cerr << "ogfx_jack_midi_tool: Failed to create jack client. Exiting..." << std::endl;
    return EXIT_FAILURE;
  }

  in0 = jack_port_register(jack_client, "in0", JACK_DEFAULT_MIDI_TYPE, JackPortIsInput, 0);
  out0 = jack_port_register(jack_client, "out0", JACK_DEFAULT_MIDI_TYPE, JackPortIsOutput, 0);

  if (0 != jack_set_process_callback(jack_client, process, 0)) {
    std::cerr << "ogfx_jack_midi_tool: Failed to set process callback. Exiting..." << std::endl;
    return EXIT_FAILURE;
  }

  if (0 != jack_activate(jack_client)) {
    std::cerr << "ogfx_jack_midi_tool: Failed to activate. Exiting..." << std::endl;
    return EXIT_FAILURE;
  }

  while(true) {
    size_t available;

    if ((available = jack_ringbuffer_read_space(in_ringbuffer)) >= message_size) {
      char data[message_size];
      jack_ringbuffer_read(in_ringbuffer, data, message_size);

      std::cout << (int)((uint8_t)data[0]) << " " << (int)((uint8_t)data[1]) << " " << (int)((uint8_t)data[2]) << " " << std::endl;
    } else {
      std::cout << "" << std::endl;
    }

    std::string input;
    std::getline(std::cin, input);

    if (input == "quit") {
        break;
    }

    if (input.length() != 0) {
        std::stringstream str(input);
        std::vector<uint8_t> data;

        while(str.good()) {
            int n;
            str >> n;
            data.push_back((uint8_t)n);
        }

        if (data.size() % message_size != 0) {
           continue; 
        }

        size_t current_index = 0;
        while (  (available = jack_ringbuffer_write_space(out_ringbuffer)) >= message_size 
                 && current_index < data.size()) {
            // std::cout << "writing " << (int)data[current_index] << " " << (int)data[current_index + 1] << " "  << (int)data[current_index + 2] << std::endl;
            jack_ringbuffer_write(out_ringbuffer, (const char*)(&data[current_index]), message_size);
            current_index += message_size;
        }
    }

  }
  
  jack_client_close(jack_client);
}
