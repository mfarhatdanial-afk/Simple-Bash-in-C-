// 1. Read
// using loop, and get string from user.
// 2. Parsing
// 3. Execute

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#define READ_bufsize 1024
#define TOKEN_bufsize 64
#define DELIM " \t\n\a\r"

void lsh_loop(void);
char *user_Read(void);
char **user_Parse(char *line);
int user_launch(char **args);
int user_execute(char **args);

int user_cd(char **args);
int user_help(char **args);
int user_exit(char **args);

int main() {

  system("clear");

  lsh_loop();

  return EXIT_SUCCESS;
  return 0;
}

void lsh_loop(void) {

  char *line;
  char **args;
  int status;

  do {
    printf("> ");
    line = user_Read();
    args = user_Parse(line);
    status = user_execute(args);
    free(line);
    free(args);
  } while (status);
}
char *user_Read(void) {

  int size = READ_bufsize;
  int slot = 0;
  int get_Input;
  char *buffer = malloc(sizeof(char *) * size);

  if (!buffer) {
    fprintf(stderr, "lsh : allocation error\n");
    exit(EXIT_FAILURE);
  }
  while (1) {

    get_Input = getchar(); // to get user input

    // if user want to insert input. We dont know how many user insert the input
    if (get_Input == EOF) {
      exit(EXIT_SUCCESS);
    } else if (get_Input == '\n') {
      buffer[slot] = '\0';
      return buffer;
    } else {
      buffer[slot] = get_Input;
    }

    slot++;

    if (slot >= size) {
      size += READ_bufsize;
      buffer = realloc(buffer, size);
      if (!buffer) {
        fprintf(stderr, "lsh : allocation error\n");
        exit(EXIT_FAILURE);
      }
    }
  }
}
char **user_Parse(char *line) {

  int size = TOKEN_bufsize;
  char **tokens = malloc(sizeof(char *) * size);
  char *token;
  int slot = 0;

  if (!tokens) {
    fprintf(stderr, "lsh : allocation error\n");
    exit(EXIT_FAILURE);
  }
  token = strtok(line, DELIM);

  while (token != NULL) {
    tokens[slot] = token;
    slot++;

    if (slot >= size) {
      size += TOKEN_bufsize;
      tokens = realloc(tokens, size * sizeof(char *));
    }
    token = strtok(NULL, DELIM);
  }

  tokens[slot] = NULL;
  return tokens;
}

int user_launch(char **args) {

  pid_t pid, wpid;
  int status;

  pid = fork();
  if (pid == 0) {
    if (execvp(args[0], args) == -1) {
      perror("lsh");
    }
    exit(EXIT_FAILURE);
  } else if (pid < 0) {
    perror("lsh");
  } else {

    do {
      wpid = waitpid(pid, &status, WUNTRACED);

    } while (!WIFEXITED(status) && !WIFSIGNALED(status));
  }

  return 1;
}

char *builtin_str[] = {"cd", "help", "exit"};

int (*builtin_func[])(char **) = {
    &user_cd,
    &user_help,
    &user_exit,
};

int bultins() { return sizeof(builtin_str) / sizeof(char *); }

int user_cd(char **args) {

  if (args[1] == NULL) {
    fprintf(stderr, "lsh : expected arguments tp \"cd\"\n");
  } else {
    if (chdir(args[1]) != 0) {
      perror("lsh");
    }
  }

  return 1;
}

int user_help(char **args) {

  printf("Farhat Danial's LSH\n");
  printf("Type program names and hit enter\n");
  printf("The following are built in :\n");

  for (int i = 0; i < bultins(); i++) {
    printf("  %s\n", builtin_str[i]);
  }

  printf("Use the man command for manual pages\n");
  return 1;
}
int user_exit(char **args) { return 0; }

int user_execute(char **args) {
  if (args[0] == NULL) {
    return 1;
  }

  for (int i = 0; i < bultins(); i++) {
    if (strcmp(args[0], builtin_str[i]) == 0) {
      return (*builtin_func[i])(args);
    }
  }

  return user_launch(args);
}
