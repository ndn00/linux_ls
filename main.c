#define _GNU_SOURCE
#include <dirent.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/stat.h>
#include <grp.h>
#include <pwd.h>
#include <time.h>

#define SIZE_OF_DATE_AND_TIME 16

bool i_flag = false;
bool l_flag = false;
bool R_flag = false;
int f_idx = 1;

void getAndPrintGroup(const gid_t grpNum) {
  struct group *grp = getgrgid(grpNum);

  if (grp) {
    printf("%s ", grp->gr_name);
  } else {
    printf("No group name for %u found\n", grpNum);
  }
}

void getAndPrintUserName(const uid_t uid) {
  struct passwd *pw = getpwuid(uid);

  if (pw) {
      printf("%s ", pw->pw_name);
  } else {
      perror("Hmm not found???");
      printf("No name found for %u\n", uid);
  }
}

void getAndPrintFileMode(const mode_t mode) {
  // Regular mode (1 bit)
  if (S_ISREG(mode)) printf("-");
  // Directory mode (1 bit)
  else if (S_ISDIR(mode)) printf("d");
  // Symbolic link mode (1 bit)
  else if (S_ISLNK(mode)) printf("l");
  
  // Owner permission (3 bits)
  printf(mode & S_IRUSR ? "r" : "-"); // Read
  printf(mode & S_IWUSR ? "w" : "-"); // Write
  printf(mode & S_IXUSR ? "x" : "-"); // Execute

  // Group permission (3 bits)
  printf(mode & S_IRGRP ? "r" : "-"); // Read
  printf(mode & S_IWGRP ? "w" : "-"); // Write
  printf(mode & S_IXGRP ? "x" : "-"); // Execute

  // Other permission (3 bits)
  printf(mode & S_IROTH ? "r" : "-"); // Read
  printf(mode & S_IWOTH ? "w" : "-"); // Write
  printf(mode & S_IXOTH ? "x" : "-"); // Execute

  printf(" ");
}

void getAndPrintTime(time_t time) {
  char dateAndTime[SIZE_OF_DATE_AND_TIME];
  strftime(dateAndTime, sizeof(dateAndTime), "%b %e %H:%M", localtime(&time));
  printf("%s ", dateAndTime);
}

void printEntry(struct stat stats, char *fileName) {
  ino_t index = stats.st_ino;
  mode_t mode = stats.st_mode;
  nlink_t hardLinksNum = stats.st_nlink;
  uid_t uid = stats.st_uid;
  gid_t groupNum = stats.st_gid;
  off_t size = stats.st_size;
  time_t time = stats.st_mtime;
//  if (i_flag) 
    printf("%ld ", index);
//  if (l_flag) {
    getAndPrintFileMode(mode);
    printf("%ld ", hardLinksNum);
    getAndPrintUserName(uid);
    getAndPrintGroup(groupNum);
    printf("%ld ", size);
    getAndPrintTime(time);
//  }
  printf("%s\n", fileName);
}

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

int main(int argc, char **argv) {
  struct stat buffer; 
  char* fileName = "main.c";
  stat(fileName, &buffer);
  printEntry(buffer, fileName);
  return 0;
}