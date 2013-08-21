/*
 * =============================================================================
 *       Filename:  kbd_hook.cpp
 *    Description:  
 *        Version:  1.0
 *        Created:  2013-08-21 10:23:06
 *         Author:  newk (newk), newkedison@gmail.com
 *        Company:  
 *          Copyright (c) 2013, newk
 * =============================================================================
 */

// Idea from:http://stackoverflow.com/a/2554421/1032255

#include "keytable.h"
#include <cassert>
#include <ctime>
#include <ctype.h>
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <linux/input.h>
#include <linux/input.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#define KEY_RELEASE 0
#define KEY_PRESS 1
#define KEY_KEEPING_PRESSED 2


const int MAX_KEY_COUNT = 0x300;
const size_t RECORD_KEY = 128;  // only record key if key-code less than this
const char* key_names[MAX_KEY_COUNT];
int uid = 0;
int gid = 0;

unsigned int records[RECORD_KEY];

void init_uid_gid()
{  // {{{
  char* sudo_uid = getenv("SUDO_UID");
  if (sudo_uid)
  {
    uid = std::stoi(sudo_uid);
    gid = std::stoi(getenv("SUDO_GID"));
  }
  else
  {
    uid = getuid();
    gid = getgid();
  }
}  // }}}

void init_key_names()
{  // {{{
  for (auto& m: key_map)
  {
    assert(m.first >= 0 && m.first < MAX_KEY_COUNT);
    key_names[m.first] = m.second;
  }
}  // }}}

void save_record(time_t now, const struct tm* timeinfo)
{  // {{{
  unsigned int sum = 0;
  for (size_t i = 0; i < RECORD_KEY; ++i)
  {
    sum += records[i];
  }
  // First element may be 0 all the time, so use it to save sum of all value
  records[0] = sum;
  setegid(gid);
  seteuid(uid);
  const char* home = getenv("HOME");
  char dir[1000];
  snprintf(dir, 1000, "%s/log", home);
  mkdir(dir, 0755);
  char file_name[1000];
  snprintf(file_name, 1000, "%s/kbd_%04d-%02d-%02d.log", dir,
      timeinfo->tm_year + 1900, timeinfo->tm_mon + 1, timeinfo->tm_mday);
  std::ofstream f(file_name, std::ifstream::binary | std::ios::app);
  if (f)
  {
    f.write(reinterpret_cast<char*>(&now), sizeof(now));
    f.write(reinterpret_cast<char*>(records), sizeof(records));
    f.close();
  }
  seteuid(0);
  setegid(0);
}  // }}}

void process_key_event(const struct input_event& event)
{  // {{{
  if (event.type != EV_KEY)
    return;
  static int last_minute = -1;
  time_t now = time(NULL);
  struct tm* timeinfo = localtime(&now);
  int minute = timeinfo->tm_yday * 1440 + timeinfo->tm_hour * 24
    + timeinfo->tm_min;
  if (minute != last_minute)
  {
    if (last_minute != -1)
      save_record(now, timeinfo);
    memset(records, 0, sizeof(records));
    last_minute = minute;
  }
  if (event.value == KEY_PRESS)
  {
    if (event.code < RECORD_KEY)
    {
      ++ records[event.code];
    }
  }
}  // }}}

int init_fds(int argc, const char* argv[], int fds[])
{  // {{{
  int max_fd = -1;
  for (int i = 1; i < argc; ++i)
  {
    fds[i - 1] = -1;
    int fd = open(argv[i], O_RDONLY);
    if (fd < 0)
    {
      char msg[300];
      snprintf(msg, sizeof(msg), "Cannot open %s.\n", argv[i]);
      perror(msg);
    }
    else
    {
      fds[i - 1] = fd;
      if (fd > max_fd)
      {
        max_fd = fd;
      }
    }
  }
  return max_fd;
}  // }}}

void read_event(int fd)
{  // {{{
  if (fd < 0)
    return;
  struct input_event events[64];
  size_t read_count = read(fd, events, sizeof(events));
  if (read_count < sizeof(struct input_event))
  {
    perror("Cannot read enough data from input device");
  }
  for (size_t i = 0; i < (read_count / sizeof(struct input_event)); ++i)
  {
    if (EV_KEY == events[i].type)
    {
      process_key_event(events[i]);
    }
  }

}  // }}}

int main(int argc, const char* argv[])
{  // {{{
  if (argc < 2)
  {
    fprintf(stderr, "usage: %s input-device [input-device ...] "
        "(/dev/input/eventX)\n", argv[0]);
    return 1;
  }
  memset(records, 0, sizeof(records));
  init_key_names();
  init_uid_gid();
  int fds[argc - 1];
  int max_fd = init_fds(argc, argv, fds);
  if (max_fd < 0)
  {
    perror("Couldn't open all input device, do you forget to run as root?");
    return 1;
  }
  ++ max_fd;

  fd_set fds_read;
  for(;;)
  {
    struct timeval tv = {1, 0};
    FD_ZERO(&fds_read);
    for (size_t i = 0; i < sizeof(fds) / sizeof(fds[0]); ++ i)
    {
      if (fds[i] != -1)
        FD_SET(fds[i], &fds_read);
    }
    if (select(max_fd, &fds_read, NULL, NULL, &tv) > 0)
    {
      for (size_t i = 0; i < sizeof(fds) / sizeof(fds[0]); ++ i)
      {
        if (fds[i] != -1)
        {
          if (FD_ISSET(fds[i], &fds_read))
          {
            read_event(fds[i]);
          }
        }
      }
    }
  }
  return 0;
}  // }}}

// vim:fdm=marker
