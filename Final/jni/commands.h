/*
 * commands.h
 *
 *  Created on: 30/08/2011
 *      Author: winston
 */

#ifndef COMMANDS_H_
#define COMMANDS_H_

#include "TooN/TooN.h"

using namespace TooN;

class Commands
{
public:
  void clear();
  void get_command(float *tf);

  union
  {
    struct{
      float transform[6];
     // float ball_pos[3];
    //  float ball_rotation[3]; //3 for rotation axis
     // float angle; //1 for angle
//      char mask[800*480/8];
    //  char *mask;
    };
//    char ch[16*4 + 7*4 + 800*480/8];
    char ch[6*4];
 //   char *maskch;
  } packet;
};

#endif /* COMMANDS_H_ */
