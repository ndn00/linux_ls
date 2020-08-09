#include <dirent.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

bool i_flag = false;
bool l_flag = false;
bool R_flag = false;
int f_idx = 1;

void handle_inputs(int argc, char **argv) {
  for (int idx = 1; idx < argc; ++idx) {
    if (argv[idx][0] == '-') {  // handle flags
      for (int idx2 = 1; idx2 < strlen(argv[idx]); ++idx2) {
        if (argv[idx][idx2] == 'i') {
          i_flag = true;
        }
        if (argv[idx][idx2] == 'l') {
          l_flag = true;
        }
        if (argv[idx][idx2] == 'R') {
          R_flag = true;
        }
      }
    } else {  // mark begining index of file/directory
      return;
    }
  }
}

int main(int argc, char **argv) {}