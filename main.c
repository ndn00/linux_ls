#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#include <dirent.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

typedef struct dirent dirent_t;
typedef struct stat stat_t;

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

void get_stat(char *pathname, stat_t *pbuf) {
  if (stat(pathname, pbuf) != 0) {
    // handle error;
  }
}
void get_lstat(char *pathname, stat_t *pbuf) {
  if (lstat(pathname, pbuf) != 0) {
    // handle error;
  }
}
void get_dirname(char *pathname, char *dirname) {
  char *ptr = strrchr(pathname, '/');
  if (ptr == NULL) {
    strcpy(dirname, pathname);
  } else if (*(ptr + 1) == '\0') {
    // TODO
  } else {
    ptr++;
    strncpy(dirname, ptr, strlen(pathname) - (int)(ptr - pathname));
  }
}
void get_path(char *pathname, char *dirname, char *path_buf) {
  strcpy(path_buf, pathname);
  strcat(path_buf, "/");
  strcat(path_buf, dirname);
}

// pathname must not have "/" at the end
void printDir(char *pathname, char *dirname) {
  // char dname_buf[1000];
  char path_buf[1000];
  stat_t buf;

  dirent_t **nameList;
  int entries;
  entries = scandir(pathname, &nameList, NULL, alphasort);

  // get_dirname(pathname, dname_buf); // current dir name
  printf("%s:\n", dirname);

  for (int idx = 0; idx < entries; ++idx) {
    if (strcmp(nameList[idx]->d_name, ".") != 0 &&
        strcmp(nameList[idx]->d_name, "..") != 0 &&
        strcmp(nameList[idx]->d_name, dirname) != 0) {
      get_path(pathname, nameList[idx]->d_name, path_buf);
      get_stat(path_buf, &buf);
      printEntry(buf, nameList[idx]->d_name);
    }
  }
  for (int idx = 0; idx < entries; ++idx) {
    if (strcmp(nameList[idx]->d_name, ".") != 0 &&
        strcmp(nameList[idx]->d_name, "..") != 0 &&
        strcmp(nameList[idx]->d_name, dirname) != 0) {
      get_path(pathname, nameList[idx]->d_name, path_buf);
      get_stat(path_buf, &buf);
      if (R_flag && S_ISDIR(buf.st_mode)) {
        printDir(path_buf, nameList[idx]->d_name);
      }
    }
    free(nameList[idx]);
  }
  free(nameList);
}

void handle_input(char *pathname) {
  stat_t buf;
  char dirname_buf[1000];
  get_dirname(pathname, dirname_buf);
  get_stat(pathname, &buf);
  if (S_ISDIR(buf.st_mode)) {
    printDir(pathname, dirname_buf);
  }
  printEntry(buf, dirname_buf);
}

int main(int argc, char **argv) {
  handle_flags(argc, argv);
  for (; f_idx < argc; ++f_idx) {
    handle_input(argv[f_idx]);
  }

  return 0;
}