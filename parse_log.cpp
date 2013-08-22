/*
 * =============================================================================
 *       Filename:  parse_log.cpp
 *    Description:  
 *        Version:  1.0
 *        Created:  2013-08-21 18:10:32
 *         Author:  newk (newk), newkedison@gmail.com
 *        Company:  
 *          Copyright (c) 2013, newk
 * =============================================================================
 */

#include "common.h"
#include "keytable.h"
#include <cstdio>
#include <cstdlib>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <dirent.h>
#include <cstring>
#include <fstream>
#include <vector>
#include <algorithm>
#include <map>
#include <iostream>

Record total;
Record day_total;

void add_to_record(Record& dst, const Record& other)
{  // {{{
  dst.t = other.t;
  for (size_t i = 0; i < sizeof(other.record) / sizeof(other.record[0]); ++ i)
  {
    dst.record[i] += other.record[i];
  }
}  // }}}

std::vector<int> find_top(const Record& record, size_t count)
{  // {{{
  std::vector<int> ret;
  size_t n = sizeof(record.record) / sizeof(record.record[0]);
  if (count < 1 || count > n)
  {
    std::cout << "count = " << count << " is invalid" << std::endl;
    return ret;
  }
  for (size_t i = 0; i < count; ++i)
  {
    int found = -1;
    for (size_t j = 1; j < n; ++j)
    {
      if (std::find(ret.begin(), ret.end(), j) == ret.end()
          && (found < 0 || record.record[j] > record.record[found]))
      {
        found = j;
      }
    }
    if (found >= 0)
    {
      ret.push_back(found);
    }
  }
  return ret;
}  // }}}

void process_log_file(const char* file_name)
{  // {{{
  printf("Processing %s\n", file_name);
  Record record;
  std::ifstream ifs(file_name, std::ifstream::binary);
  if (!ifs)
  {
    fprintf(stderr, "open %s fail\n", file_name);
    return;
  }
  ifs.seekg(0, ifs.end);
  size_t file_length = ifs.tellg();
  if (file_length < sizeof(Record) || file_length % sizeof(Record) != 0)
  {
    fprintf(stderr, "length of %s is %d, it must be a multiple of %d\n",
        file_name, file_length, sizeof(Record));
    ifs.close();
    return;
  }
  ifs.seekg(0, ifs.beg);
  memset(&day_total, 0, sizeof(day_total));
  for (size_t i = 0; i < file_length / sizeof(Record); ++i)
  {
    ifs.read(reinterpret_cast<char*>(&record), sizeof(Record));
    if (!ifs.good())
    {
      fprintf(stderr, "Error occur when reading %s\n", file_name);
      ifs.close();
      return;
    }
    add_to_record(day_total, record);
  }
  printf("Daily total: %d\n", day_total.record[0]);
  auto top = find_top(day_total, 10);
  if (top.empty())
  {
    printf("cannot find top press\n");
  }
  else
  {
    for (auto i: top)
    {
      printf("%20s:%8d\n", key_map.find(i)->second, day_total.record[i]);
    }
  }
}  // }}}

bool process_log_dir(const char* dir_name)
{  // {{{
  DIR* dir = opendir(dir_name);
  if (!dir)
  {
    fprintf(stderr, "cannot open %s\n", dir_name);
    return false;
  }
  struct dirent* ent;
  char full_file_name[1000];
  while ((ent = readdir(dir)) != NULL)
  {
    if (strlen(ent->d_name) > 0 && ent->d_name[0] != '.')
    {
      snprintf(full_file_name, 1000, "%s/%s", dir_name, ent->d_name);
      process_log_file(full_file_name);
      add_to_record(total, day_total);
    }
  }
  closedir(dir);
  std::cout << "============SUMMARY==============" << std::endl;
  std::cout << "Total press: " << total.record[0] << std::endl;
  auto top = find_top(total, 10);
  if (!top.empty())
  {
    std::cout << "TOP 10:" << std::endl;
    for (auto i: top)
    {
      printf("%20s:%8d\n", key_map.find(i)->second, total.record[i]);
    }
  }
  return true;
}  // }}}

int main(int argc, char* argv[])
{  // {{{
  if (argc < 2)
  {
    fprintf(stderr, "Usage: %s dir-name (where the log file store)\n", argv[0]);
    return EXIT_FAILURE;
  }
  struct stat st;
  if (stat(argv[1], &st) == -1)
  {
    fprintf(stderr, "%s not exist or cannot be accessed.\n", argv[1]);
    return EXIT_FAILURE;
  }
  if (!S_ISDIR(st.st_mode))
  {
    fprintf(stderr, "%s is not a directory.\n", argv[1]);
    return EXIT_FAILURE;
  }
  memset(&total, 0, sizeof(total));
  memset(&day_total, 0, sizeof(day_total));
  if (!process_log_dir(argv[1]))
  {
    fprintf(stderr, "Process directory %s fail\n", argv[1]);
    return EXIT_FAILURE;
  }
}  // }}}

// vim: fdm=marker

