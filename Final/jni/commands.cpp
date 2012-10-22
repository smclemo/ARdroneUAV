/*
 * commands.cpp
 *
 *  Created on: 30/08/2011
 *      Author: winston
 */

#include "commands.h"

#include <string.h>

void Commands::clear()
{
  memset(packet.ch, 0.0, 6*4);
}

void Commands::get_command(float *tf)
{
  memcpy(packet.ch, tf, 6*4);
}
