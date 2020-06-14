#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>

#define MAX_COMMAND_LENGTH 256
#define RES_EXECUTE_ERROR 1
#define RES_NORMAL 0
#define RES_BAD_COMMAND 1
#define RES_BAD_COMMAND_PIPE 2
#define RES_BAD_COMMAND_REDIRECT 3

char* format(char* command);
void mysys(char* command);
int execute(int argc);
char* mypwd(void);
void cdhome(char* path);
void mycd(int argc);
int execRedirect(int st, int ed);
int execPipe(int st, int ed);

char currentDir[256];						// current directory path
char hostname[50];
char *argv_[MAX_COMMAND_LENGTH];     // command

int main(int argc, char* argv[])
{
    char cmd[MAX_COMMAND_LENGTH];
    
    gethostname(hostname, 50);
	strcpy(currentDir, mypwd());

    while(1) {
        printf("[\e[1;31m%s@%s\e[0m %s]>", getenv("LOGNAME"), hostname, currentDir);   
        fgets(cmd, sizeof(cmd), stdin);                                              
        cmd[strlen(cmd) - 1] = '\0';                                               
        
        mysys(cmd);
    }

    return 0;
}

char* format(char* command) {
	int len = strlen(command), i;
	char newcmd[MAX_COMMAND_LENGTH];
	char* np = newcmd;

	*np++ = command[0];
	for (i = 1; i < len; ++i) {
		if (command[i] == '>' || command[i] == '<' || command[i] == '|') {
			if (command[i - 1] != ' ') {
				*np++ = ' ';
			}
			*np++ = command[i];
			if (i + 1 < len && command[i + 1] != ' ') {
				*np++ = ' ';
			}
			continue;
		}
		*np++ = command[i];
	}
	*np = '\0';
	np = newcmd;
	return np;
}
void mysys(char *command) {
    char newcmd[MAX_COMMAND_LENGTH];
    char *token, *np = newcmd;
    int argc = 0;

	strcpy(newcmd, format(command));                   // format command string before execute
    token = strtok_r(newcmd, " ", &np);
	while (token != NULL) {
        argv_[argc++] = token;
        token = strtok_r(NULL, " ", &np);
    }
    argv_[argc] = NULL;

    if (strcmp(argv_[0], "exit") == 0) {
        exit(EXIT_SUCCESS);
    } else if (strcmp(argv_[0], "cd") == 0) {
        mycd(argc);
		strcpy(currentDir, mypwd());                  // update pwd
    } else {
        int res = execute(argc);
		switch (res) {
            case RES_EXECUTE_ERROR: exit(EXIT_FAILURE);
            case RES_BAD_COMMAND_PIPE: printf("'|' at bad position\n"); break;
            case RES_BAD_COMMAND_REDIRECT: printf("'<' or '>' at bad position\n"); break;
            default: break;
        }
    }
}
int execute(int argc) {
    pid_t pid = fork();
    if (pid == 0) {
        int res = execPipe(0, argc);
        exit(res);
    } else {
        int res;
        waitpid(pid, &res, 0);
        return WEXITSTATUS(res);
    }
}
int execPipe(int st, int ed) {
	if (st >= ed) return RES_NORMAL;

	int index = -1, i;                         // find first '|'
	for (i = st; i < ed; ++i) {
		if (strcmp(argv_[i], "|") == 0) {
			index = i;
			break;
		}
	}

	if (index == -1) {                         // '|' not found
		return execRedirect(st, ed);
	} else if (index == ed - 1) {
		return RES_BAD_COMMAND_PIPE;
	}
	
	int fd[2];
	if (pipe(fd) == -1) {
		perror("pipe()");
		return RES_EXECUTE_ERROR;
	}
	
	int res;
	pid_t pid = fork();
	if (pid == -1) {
		perror("fork()");
        return RES_EXECUTE_ERROR;
	} else if (pid == 0) {					// child
		close(fd[0]);
		dup2(fd[1], 1);
		close(fd[1]);
		
		res = execRedirect(st, index);		// pipeIn
		exit(res);
	} else {								// parent
		int stat;
		waitpid(pid, &stat, 0);
		int res_ = WEXITSTATUS(stat);

        if (index < ed - 1) {
			close(fd[1]);
			dup2(fd[0], 0);					// read from pipe
			close(fd[0]);
			res = execPipe(index + 1, ed);	// pipeOut
		}
	}
	return res;
}
int execRedirect(int st, int ed) {
	int inCount = 0, outCount = 0, i, right = ed;
	char* inPath = NULL, *outPath = NULL;

	for (i = st; i < ed; ++i) {
		if (strcmp(argv_[i], "<") == 0) {
			++inCount;
			if (i == ed - 1) return RES_BAD_COMMAND_REDIRECT;              // bad position
			else {
				inPath = argv_[i + 1];
			}
			if (right == ed) right = i;
		} else if (strcmp(argv_[i], ">") == 0) {
			++outCount;
			if (i == ed - 1) return RES_BAD_COMMAND_REDIRECT;              // bad position
			else {
				outPath = argv_[i + 1];
			}
			if (right == ed) right = i;
		}
	}
	if (inCount > 1 || outCount > 1) return RES_BAD_COMMAND_REDIRECT;
	
	int res = RES_NORMAL;
	pid_t pid = fork();
	if (pid == -1) {
		perror("fork()");
		res = RES_EXECUTE_ERROR;
	} else if (pid == 0) {                     // child
        if (inCount == 1) {
            int infd = open(inPath, O_RDONLY);
            if (infd == -1) {
                perror("open()");
                return RES_EXECUTE_ERROR;
            }
            dup2(infd, 0);
            close(infd);
        }
        if (outCount == 1) {
            int outfd = open(outPath, O_CREAT | O_RDWR, 0666);
            if (outfd == -1) {
                perror("open()");
                return RES_EXECUTE_ERROR;
            }
            dup2(outfd, 1);
            close(outfd);
        }
        char* realCommand[MAX_COMMAND_LENGTH];
        for (i = st; i < right; ++i) {
            realCommand[i] = argv_[i];
        }
        realCommand[i] = NULL;
        execvp(realCommand[st], realCommand + st);
        exit(errno);
	} else {                                   // parent
        int stat;
        waitpid(pid, &stat, 0);
        int error = WEXITSTATUS(stat);
        if (error != 0) {
            printf("Process exit error: %s\n", strerror(error));
        }
    }
    return res;
}

char* mypwd(void) {
    char buf[256];					// only for printing command hint
    char* dir = NULL;
    dir = getcwd(buf, sizeof(buf));
	if (dir == NULL) {
        perror("getcwd()");
        exit(EXIT_FAILURE);
    } else {
        return dir;
    }
}
void cdhome(char* path) {
    char homepath[] = "/home/";
    char username[50];
    strcpy(username, getenv("LOGNAME"));
    strcpy(path, homepath);
    strcat(path, username);
}
void mycd(int argc) {
    char path[1024];
    if (argc == 1) {
        cdhome(path);
    } else if (argc == 2) {
        if (strcmp(argv_[1], "~") == 0) {
            cdhome(path);
        } else {
            strcpy(path, argv_[1]);
        }
    }
    
    if (chdir(path) != 0) {
        perror("chdir()");
    }
}
