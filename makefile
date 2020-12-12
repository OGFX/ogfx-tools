prefix ?= /usr/local
cxx_extra_flags ?= -O3 -march=native  -Wall -Werror -Wno-conversion

.PHONY: clean all install

all: ogfx_lv2ls ogfx_jack_switch ogfx_jack_midi_tool ogfx_jack_list_ports

install: all
	mkdir -p ${prefix}/bin
	install ogfx_lv2ls ${prefix}/bin/
	install ogfx_jack_switch ${prefix}/bin/
	install ogfx_jack_midi_tool ${prefix}/bin/
	install ogfx_jack_list_ports ${prefix}/bin/

clean: 
	rm -f ogfx_lv2ls ogfx_jack_swith ogfx_jack_midi_json_dump ogfx_jack_list_ports ogfx_jack_midi_tool

ogfx_lv2ls: ogfx_lv2ls.cc
	$(CXX) ${cxx_extra_flags} -o ogfx_lv2ls ogfx_lv2ls.cc `pkg-config lilv-0 --cflags --libs`

ogfx_jack_switch: ogfx_jack_switch.cc
	$(CXX) ${cxx_extra_flags} -o ogfx_jack_switch ogfx_jack_switch.cc `pkg-config jack --cflags --libs` -lboost_program_options

ogfx_jack_midi_tool: ogfx_jack_midi_tool.cc
	$(CXX) ${cxx_extra_flags} -o ogfx_jack_midi_tool ogfx_jack_midi_tool.cc `pkg-config jack --cflags --libs` -lboost_program_options 

ogfx_jack_list_ports: ogfx_jack_list_ports.cc
	$(CXX) ${cxx_extra_flags} -o ogfx_jack_list_ports ogfx_jack_list_ports.cc `pkg-config jack --cflags --libs` -lboost_program_options
