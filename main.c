#define _GNU_SOURCE
#define _DEFAULT_SOURCE
#include <dirent.h>
#include <errno.h>
#include <grp.h>
#include <pwd.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#define SIZE_OF_DATE_AND_TIME 16

typedef struct dirent dirent_t;
typedef struct stat stat_t;

bool i_flag = false;
bool l_flag = false;
bool R_flag = false;
int f_idx = 1;
bool multiple_inputs = false;
bool first = true;
int level = 0;

void getAndPrintGroup(const gid_t grpNum) {
  struct group *grp = getgrgid(grpNum);

  if (grp) {
    printf("%s\t", grp->gr_name);
  } else {
    printf("No group name for %u found\n", grpNum);
  }
}

void getAndPrintUserName(const uid_t uid) {
  struct passwd *pw = getpwuid(uid);

  if (pw) {
    printf("%s\t", pw->pw_name);
  } else {
    perror("Hmm not found???");
    printf("No name found for %u\n", uid);
  }
}

void getAndPrintFileMode(const mode_t mode) {
  // Regular mode (1 bit)
  if (S_ISREG(mode)) printf("-");
  // Directory mode (1 bit)
  else if (S_ISDIR(mode))
    printf("d");
  // Symbolic link mode (1 bit)
  else if (S_ISLNK(mode))
    printf("l");

  // Owner permission (3 bits)
  printf(mode & S_IRUSR ? "r" : "-");  // Read
  printf(mode & S_IWUSR ? "w" : "-");  // Write
  printf(mode & S_IXUSR ? "x" : "-");  // Execute

  // Group permission (3 bits)
  printf(mode & S_IRGRP ? "r" : "-");  // Read
  printf(mode & S_IWGRP ? "w" : "-");  // Write
  printf(mode & S_IXGRP ? "x" : "-");  // Execute

  // Other permission (3 bits)
  printf(mode & S_IROTH ? "r" : "-");  // Read
  printf(mode & S_IWOTH ? "w" : "-");  // Write
  printf(mode & S_IXOTH ? "x" : "-");  // Execute

  printf("\t");
}

void getAndPrintTime(time_t time) {
  char dateAndTime[SIZE_OF_DATE_AND_TIME];
  strftime(dateAndTime, sizeof(dateAndTime), "%b %e %H:%M", localtime(&time));
  printf("%s\t", dateAndTime);
}

void printEntry(stat_t stats, char *dirname, char *pathname) {
  ino_t index = stats.st_ino;
  mode_t mode = stats.st_mode;
  nlink_t hardLinksNum = stats.st_nlink;
  uid_t uid = stats.st_uid;
  gid_t groupNum = stats.st_gid;
  off_t size = stats.st_size;
  time_t time = stats.st_mtime;
  if (i_flag) printf("%ld\t", index);
  if (l_flag) {
    getAndPrintFileMode(mode);
    printf("%ld\t", hardLinksNum);
    getAndPrintUserName(uid);
    getAndPrintGroup(groupNum);
    printf("%ld\t", size);
    getAndPrintTime(time);
  }
  printf("%s", dirname);
  if (l_flag && S_ISLNK(stats.st_mode)) {
    char dest_buf[1000];
    ssize_t len = readlink(pathname, dest_buf, sizeof(dest_buf) - 1);

    if (len != -1) {
      dest_buf[len] = '\0';
    } else {
      /* handle error condition */
    }
    printf(" -> %s", dest_buf);
  }
  printf("\n");
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
      if (argc - f_idx > 1) {
        multiple_inputs = true;
      }
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
    strncpy(dirname, ptr, strlen(pathname) + 1 + (int)(ptr - pathname));
  }
}
void get_path(char *pathname, char *dirname, char *path_buf) {
  strcpy(path_buf, pathname);
  strcat(path_buf, "/");
  strcat(path_buf, dirname);
}

// pathname must not have "/" at the end
void printDir(char *pathname) {
  // char dname_buf[1000];
  char path_buf[1000];
  stat_t buf;

  dirent_t **nameList;
  int entries;
  entries = scandir(pathname, &nameList, NULL, alphasort);

  // get_dirname(pathname, dname_buf); // current dir name
  if (level || multiple_inputs || R_flag) {
    if (!first) {
      printf("\n");
    } else {
      first = true;
    }
    printf("%s:\n", pathname);
  }
  level++;

  for (int idx = 0; idx < entries; ++idx) {
    if (strcmp(nameList[idx]->d_name, ".") != 0 &&
        strcmp(nameList[idx]->d_name, "..") != 0) {
      get_path(pathname, nameList[idx]->d_name, path_buf);
      get_lstat(path_buf, &buf);
      printEntry(buf, nameList[idx]->d_name, path_buf);
    }
  }
  for (int idx = 0; idx < entries; ++idx) {
    if (strcmp(nameList[idx]->d_name, ".") != 0 &&
        strcmp(nameList[idx]->d_name, "..") != 0) {
      get_path(pathname, nameList[idx]->d_name, path_buf);
      get_lstat(path_buf, &buf);
      if (R_flag && S_ISDIR(buf.st_mode)) {
        printDir(path_buf);
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
    printDir(pathname);
  } else
    printEntry(buf, dirname_buf, pathname);
  first = false;
}

void swap(int *a, int *b) {
  int temp = *b;
  *b = *a;
  *a = temp;
}

void sort_input(char **argv, int arr_size, int *arr) {
  for (int i = 0; i < arr_size - 1; ++i) 
    for (int j = 0; j < arr_size - i - 1; ++j) 
      if (strcmp(argv[arr[j]], argv[arr[j + 1]]) > 0) 
        swap(&arr[j], &arr[j + 1]);
}

int main(int argc, char **argv) {
  handle_flags(argc, argv);
  stat_t buf;
  int cnt_dir = 0, cnt_entry = 0;
  for(; f_idx < argc; ++f_idx) {
    get_lstat(argv[f_idx], &buf);
    if (S_ISDIR(buf.st_mode)) ++cnt_dir;
    else if (S_ISREG(buf.st_mode)) ++cnt_entry;
    else if (S_ISLNK(buf.st_mode)) l_flag ? ++cnt_entry : ++cnt_dir;
  }
  int *dir = calloc(cnt_dir, sizeof(int));
  int *entry = calloc(cnt_entry, sizeof(int));
  cnt_dir = 0;
  cnt_entry = 0;
  for (int j = 0; j < argc; ++j) {
      get_lstat(argv[j], &buf);
      if (S_ISDIR(buf.st_mode) || (S_ISLNK(buf.st_mode) && !l_flag)) dir[cnt_dir++] = j;
      else entry[cnt_entry++] = j; 
  }

  return 0;
}