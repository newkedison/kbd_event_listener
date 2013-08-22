kbd_event_listener: main.cpp keytable.h
	g++ -g -o $@ -std=c++11 main.cpp

change_mode: kbd_event_listener
	chown root:root kbd_event_listener
	chmod u+s kbd_event_listener

parse_log: parse_log.cpp
	g++ -g -o $@ -std=c++11 parse_log.cpp

