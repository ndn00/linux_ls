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
#define MAX(X, Y) (((X) > (Y)) ? (X) : (Y))

typedef struct dirent dirent_t;
typedef struct stat stat_t;

bool i_flag = false;
bool l_flag = false;
bool R_flag = false;
int f_idx = 1;
bool multiple_inputs = false;
bool first = true;
int level = 0;
typedef struct {
  int ino_len;
  int hlink_len;
  int uname_len;
  int gname_len;
  int size_len;
  bool hasSpcPath;
} PrintProfile;

bool hasSpcChr(char *pathname) {
  bool spc_flag = false;
  char spcchr[7] = {' ', '!', '$', '^', '&', '(', ')'};
  for (int idx = 0; idx < 7; ++idx) {
    if (strchr(pathname, spcchr[idx]) != NULL) {
      spc_flag = true;
    }
  }
  return spc_flag;
}

int get_intlen(int i) {
  int count = 0;
  while (i != 0) {
    i /= 10;
    ++count;
  }
  return count;
}
void printPath(char *pathname, bool beginSpc) {
  bool spc_flag = hasSpcChr(pathname);
  if (spc_flag) {
    printf("\'");
  } else if (beginSpc) {
    printf(" ");
  }
  printf("%s", pathname);
  if (spc_flag) {
    printf("\'");
  }
}

char *get_gname(const gid_t gid) {
  struct group *grp = getgrgid(gid);

  if (grp) {
    return grp->gr_name;
  } else {
    return "!not found!";
  }
}
char *get_uname(const uid_t uid) {
  struct passwd *pw = getpwuid(uid);

  if (pw) {
    return pw->pw_name;
  } else {
    return "!not found!";
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

  printf(" ");
}

void getAndPrintTime(time_t time) {
  char dateAndTime[SIZE_OF_DATE_AND_TIME];
  strftime(dateAndTime, sizeof(dateAndTime), "%b %e %H:%M", localtime(&time));
  printf("%s ", dateAndTime);
}

void printEntry(stat_t *stats, char *dirname, char *pathname,
                PrintProfile *ppp) {
  if (i_flag) printf("%*ld ", ppp->ino_len, stats->st_ino);
  if (l_flag) {
    getAndPrintFileMode(stats->st_mode);
    printf("%*ld ", ppp->hlink_len, stats->st_nlink);
    printf("%*s ", ppp->uname_len, get_uname(stats->st_uid));
    printf("%*s ", ppp->gname_len, get_gname(stats->st_gid));
    printf("%*ld ", ppp->size_len, stats->st_size);
    getAndPrintTime(stats->st_mtime);
  }
  // printf("%s", dirname);
  printPath(dirname, ppp->hasSpcPath);

  // handle special char
  if (l_flag && S_ISLNK(stats->st_mode)) {
    char dest_buf[1000];
    ssize_t len = readlink(pathname, dest_buf, sizeof(dest_buf) - 1);

    if (len != -1) {
      dest_buf[len] = '\0';
    } else {
      /* handle error condition */
    }
    printf(" -> ");
    printPath(dest_buf, false);
  }
  printf("\n");
}

void handle_flags(int argc, char **argv) {
  for (f_idx = 1; f_idx < argc; ++f_idx) {
    if (argv[f_idx][0] == '-' && strlen(argv[f_idx]) > 1) {  // handle flags
      for (int idx2 = 1; idx2 < strlen(argv[f_idx]); ++idx2) {
        if (argv[f_idx][idx2] == 'i') {
          i_flag = true;
        } else if (argv[f_idx][idx2] == 'l') {
          l_flag = true;
        } else if (argv[f_idx][idx2] == 'R') {
          R_flag = true;
        } else {
          printf("myls: invalid option -- \'%c\'\n", argv[f_idx][idx2]);
          printf("Try other options!\n");
          exit(-1);
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

bool get_stat(char *pathname, stat_t *pbuf) {
  if (stat(pathname, pbuf) != 0) {
    // handle error;
    //   printf("myls: cannot access \'%s\': No such file or directory\n",
    //   pathname);
    pbuf = NULL;
    return false;
  }
  return true;
}
bool get_lstat(char *pathname, stat_t *pbuf) {
  if (lstat(pathname, pbuf) != 0) {
    // handle error;
    //  printf("myls: cannot access \'%s\': No such file or directory\n",
    //  pathname);
    pbuf = NULL;
    return false;
  }
  return true;
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

  PrintProfile pp;
  memset(&pp, 0, sizeof(pp));

  // init pp
  for (int idx = 0; idx < entries; ++idx) {
    if (strcmp(nameList[idx]->d_name, ".") != 0 &&
        strcmp(nameList[idx]->d_name, "..") != 0 &&
        nameList[idx]->d_name[0] != '.') {
      get_path(pathname, nameList[idx]->d_name, path_buf);
      get_lstat(path_buf, &buf);
      pp.ino_len = MAX(pp.ino_len, get_intlen(buf.st_ino));
      pp.hlink_len = MAX(pp.hlink_len, get_intlen(buf.st_nlink));
      pp.size_len = MAX(pp.size_len, get_intlen(buf.st_size));
      pp.uname_len = MAX(pp.uname_len, strlen(get_uname(buf.st_uid)));
      pp.gname_len = MAX(pp.gname_len, strlen(get_gname(buf.st_gid)));
      // printf("%d %d\n", )
      pp.hasSpcPath = (l_flag || i_flag) &&
                      (pp.hasSpcPath || hasSpcChr(nameList[idx]->d_name));
    }
  }

  // get_dirname(pathname, dname_buf); // current dir name
  if (level || multiple_inputs || R_flag) {
    if (!first) {
      printf("\n");
    } else {
      first = false;
    }
    // printf("\n");

    // printf("%s:\n", pathname);
    printPath(pathname, false);
    printf(":\n");
  }
  level++;

  // print entries
  for (int idx = 0; idx < entries; ++idx) {
    if (strcmp(nameList[idx]->d_name, ".") != 0 &&
        strcmp(nameList[idx]->d_name, "..") != 0 &&
        nameList[idx]->d_name[0] != '.') {
      get_path(pathname, nameList[idx]->d_name, path_buf);
      get_lstat(path_buf, &buf);
      printEntry(&buf, nameList[idx]->d_name, path_buf, &pp);
    }
  }

  // depth-first & cleanup
  for (int idx = 0; idx < entries; ++idx) {
    if (strcmp(nameList[idx]->d_name, ".") != 0 &&
        strcmp(nameList[idx]->d_name, "..") != 0 &&
        nameList[idx]->d_name[0] != '.') {
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

// void handle_input(char *pathname) {
//   stat_t buf;
//   char dirname_buf[1000];
//   get_dirname(pathname, dirname_buf);
//   get_stat(pathname, &buf);
//   if (S_ISDIR(buf.st_mode)) {
//     printDir(pathname);
//   } else{
//     PrintProfile pp;
//     memset(&pp, 0, sizeof(pp));
//     printEntry(&buf, dirname_buf, pathname, &pp);
//   }
//   first = false;
// }

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
  for (int idx = f_idx; idx < argc; ++idx) {
    if (get_lstat(argv[idx], &buf)) {
      if (S_ISDIR(buf.st_mode))
        ++cnt_dir;
      else if (S_ISREG(buf.st_mode))
        ++cnt_entry;
      else if (S_ISLNK(buf.st_mode))
        (!l_flag && get_stat(argv[idx], &buf) && S_ISDIR(buf.st_mode))
            ? ++cnt_dir
            : ++cnt_entry;
    } else
      printf("myls: cannot access \'%s\': No such file or directory\n",
             argv[idx]);
  }

  int *dir = calloc(cnt_dir, sizeof(int));
  int *entry = calloc(cnt_entry, sizeof(int));
  cnt_dir = 0, cnt_entry = 0;
  for (int idx = f_idx; idx < argc; ++idx) {
    if (get_lstat(argv[idx], &buf)) {
      if (S_ISDIR(buf.st_mode))
        dir[cnt_dir++] = idx;
      else if (S_ISREG(buf.st_mode))
        entry[cnt_entry++] = idx;
      else if (S_ISLNK(buf.st_mode))
        (!l_flag && get_stat(argv[idx], &buf) && S_ISDIR(buf.st_mode))
            ? (dir[cnt_dir++] = idx)
            : (entry[cnt_entry++] = idx);
    }
  }
  // for (int k = f_idx, cnt_dir = 0, cnt_entry = 0; k < argc; ++k) {
  //   get_lstat(argv[k], &buf);
  //   if (!l_flag && get_stat(argv[k], &buf) && S_ISDIR(buf.st_mode))
  //     dir[cnt_dir++] = k;
  //   else
  //     entry[cnt_entry++] = k;
  // }
  sort_input(argv, cnt_dir, dir);
  sort_input(argv, cnt_entry, entry);

  PrintProfile pp;
  memset(&pp, 0, sizeof(pp));

  // handle default case
  if (!cnt_entry && !cnt_dir && f_idx >= argc) {
    if (get_stat(".", &buf)) printDir(".");
  }

  for (int idx = 0; idx < cnt_entry; ++idx) {
    get_lstat(argv[entry[idx]], &buf);
    pp.ino_len = MAX(pp.ino_len, get_intlen(buf.st_ino));
    pp.hlink_len = MAX(pp.hlink_len, get_intlen(buf.st_nlink));
    pp.size_len = MAX(pp.size_len, get_intlen(buf.st_size));
    pp.uname_len = MAX(pp.uname_len, strlen(get_uname(buf.st_uid)));
    pp.gname_len = MAX(pp.gname_len, strlen(get_gname(buf.st_gid)));
    pp.hasSpcPath =
        (l_flag || i_flag) && (pp.hasSpcPath || hasSpcChr(argv[entry[idx]]));
  }

  for (int idx = 0; idx < cnt_entry; ++idx) {
    get_lstat(argv[entry[idx]], &buf);

    printEntry(&buf, argv[entry[idx]], argv[entry[idx]], &pp);
    first = false;
  }
  free(entry);
  for (int idx = 0; idx < cnt_dir; ++idx) {
    if (get_stat(argv[dir[idx]], &buf)) printDir(argv[dir[idx]]);
  }
  free(dir);

  return 0;
}