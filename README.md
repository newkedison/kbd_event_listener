Linux keyboard event listener [![Build Status](https://travis-ci.org/newkedison/kbd_event_listener.png?branch=master)](https://travis-ci.org/newkedison/kbd_event_listener)
=============================

This program can listen to events of keyboard and then save them every minute to a log file.

Basic method was learn form [StackOverflow](http://stackoverflow.com/a/2554421/1032255)

### How to build?

There was a simple [Makefile](Makefile) here, so just `make` it, but keep in mind that make sure your gcc support c++11(which means you need gcc-4.8)

### How to use?

This program can listen to multiple events, these event devices were stay in /dev/input directory in most case. 

There are usually more than 10 eventX in /dev/input, you need to find which event is the keyboard event yourself

A simple method is to view the symbol links in /dev/input/by-id, something with `kbd` in its name maybe the exact event

**Note**: If you have more than one keyboard(eg. a laptop keyboard and a USB keyboard), you need to find all event devices for them. In my case, /dev/input/event4 is for my laptop keyboard and the /dev/input/event5 is for my USB keyboard

After find the event device, run command like below

    $ sudo ./kbd_event_listener /dev/input/event5

if you have multiple keyboard, just list them, seperate with space

    $ sudo ./kbd_event_listener /dev/input/event4 /dev/input/event5

Just remember to repace the device of your own

### Where is the log file?

I have hard code the log file path in the programe, they will be put in the `$HOME/log/` directory, with file name like `kbd_2013-08-22.log`

If you don't like this path, you can modify the source code yourself

### How to read log files?

In order to limit file size, the log file is a binary file, you cannot read it directly

I have write a simple parser to read the log file, please refer to the [parse_log.cpp](parse_log.cpp) for more detail

Run `make parse_log` and build the program `parse_log`, and then

    $ ./parse_log ~/log

will parse all log files in ~/log

If you want to parse log file yourself, the format of log file is simple

> There are hundreds of "record" in the log file, every "record" is a struct called "Record" in [common.h](common.h), so, just read them and do what you want. 

Note every recode is summary of one minute, first 4 byte is a `time_t` variable indicate the time, and the following 128 `int`s is how many times the key of this code is pressed.

### Run as root

Since only root can access the event devices, this program must run as root, there are two methods.

1. Run with `sudo`

2. Change owner of the executable to root and set a `s` bit for it

If you prefer to use method 2, you can simply run `sudo make change_mode`

**Note**: if you want to copy a 'setuid executable file' to other directory, do not forget to copy with `cp -a`.

With either method, the log file created by this programe will make the owner and group to be your real account, not the root, read the `init_uid_gid()` and `save_to_file()` function in [main.cpp](main.cpp) for more detail.


[![Bitdeli Badge](https://d2weczhvl823v0.cloudfront.net/newkedison/kbd_event_listener/trend.png)](https://bitdeli.com/free "Bitdeli Badge")

