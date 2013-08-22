/*
 * =============================================================================
 *       Filename:  common.h
 *    Description:  
 *        Version:  1.0
 *        Created:  2013-08-21 21:24:19
 *         Author:  newk (newk), newkedison@gmail.com
 *        Company:  
 *          Copyright (c) 2013, newk
 * =============================================================================
 */

#ifndef COMMON_H
#define COMMON_H

#include <unistd.h>
#include <ctime>

const int MAX_KEY_COUNT = 0x300;
const size_t RECORD_KEY = 128;  // only record key if key-code less than this

struct Record
{
  time_t t;
  unsigned int record[RECORD_KEY];
};

#endif  // COMMON_H

// vim: fdm=marker

