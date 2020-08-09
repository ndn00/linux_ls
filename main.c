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

void handle_flags(int argc, char **argv) {
  for (f_idx = 1; f_idx < argc; ++f_idx) {
    if (argv[f_idx][0] == '-') {  // handle flags
      for (int idx2 = 1; idx2 < strlen(argv[f_idx]); ++idx2) {
        if (argv[f_idx][idx2] == 'i') {
          i_flag = true;
        }
        if (argv[f_idx][idx2] == 'l') {
          l_flag = true;
        }
        if (argv[f_idx][idx2] == 'R') {
          R_flag = true;
        }
      }
    } else {  // f_idx mark begining index of files/directories
      return;
    }
  }
}

int main(int argc, char **argv) {}