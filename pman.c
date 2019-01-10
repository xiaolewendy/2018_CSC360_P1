/*CSC360 A01 Pman
 *Student: Xiaole (Wendy) Tan
 *Vnumber: V00868734*/

/*Includes*/
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

/*Define*/
#define MAXREAD 200
#define MAXLINESIZE 100
/*Process type:
  node which keep track of process info: id, name, state*/
typedef struct p process;
struct p{
  char *name;
  pid_t pid;
  int state; /*0=pause(stop), 1=running*/
  process *next;
};

/*List to track running processes*/
process *p_list = NULL;


/* ---------     LIST     ---------- */
/*Process Node: constructing new node*/
process *new_p(char *name, pid_t pid){
   process *temp = (process *)malloc(sizeof(process));
   temp->name = strdup(name);
   temp->pid = pid;
   temp->state = 1;
   temp->next = NULL;
   return temp;
}

/*Process list: to record background processes*/
process *add_to_list(process *list, process *new_p){
  new_p->next = list;
  return new_p;
}


/* ---------     HELPER FUNCTIONS     ---------- */
/*Read in process name and arguments*/
void build_args(char** args){
  size_t getline_size = MAXLINESIZE;
  char *read = (char *)malloc(sizeof(char));
  getline(&read, &getline_size, stdin);
  /*A null terminated array of char* for EXECVP*/
  int i = 0;
  char *tok = strtok(read, " ");
  while(tok != NULL){
    args[i] = tok;
    i++;
    tok = strtok(NULL, " ");
  }
  args[i] = NULL;
  /*printf("in build: %s\n", *args);*/
}


/*check if pid valid*/
int check_pid(pid_t pid){
  if (p_list == NULL){
    return 0;
  }else{
    process *pt = p_list;
    while(pt != NULL){
      if(pt->pid == pid){
        return 1;
      }
      pt = pt->next;
    }
    return 0;
  }
}

/*Remove process(pid) from list*/
void remove_process(pid_t pid){
  process *front_list_pt = p_list;
  /*List contain single process OR remove first*/
  if(front_list_pt->next == NULL || front_list_pt->pid == pid){
    p_list = front_list_pt->next;
    free(front_list_pt);
    return;
  }
  process *list_pt = front_list_pt->next;
  /*More than one process, find process and delete*/
  while(list_pt != NULL){
    if(list_pt->pid == pid){
      front_list_pt->next = list_pt->next;
      free(list_pt);
      return;
    }
    front_list_pt = list_pt;
    list_pt = list_pt->next;
  }
}


/*Change state of process with given number: 1=> RUNNING, 0=>PAUSE*/
void change_state(pid_t pid, int state){
  process *list_pt = p_list;
  while(list_pt != NULL){
    if(list_pt->pid == pid){
      list_pt->state = state;
      return;
    }
    list_pt = list_pt->next;
  }
}


/*UPDATE process: process terminate => remove from list AND report to user
                  process pause     => change status to 0
                  process running   => change status to 1*/
void updat_process(){
  pid_t pid;
  /*pointer to terminating process info*/
  int p_status;
  while(1){
    pid = waitpid(-1, &p_status, WNOHANG | WCONTINUED | WUNTRACED);
    /*if pid returned, check status of child process*/
    if(pid > 0){
      if(WIFSIGNALED(p_status)){
        printf("PMan:> Process %d killed\n", pid);
        remove_process(pid);
      }
      if(WIFEXITED(p_status)){
        printf("PMan:> Proces %d exited\n", pid);
        remove_process(pid);
      }
      if(WIFSTOPPED(p_status)){
        printf("PMan:> Process %d stopped\n", pid);
        change_state(pid, 0);
      }
      if(WIFCONTINUED(p_status)){
        printf("PMan:> Process %d started\n", pid);
        change_state(pid, 1);
      }
    }else{
      break;
    }
  }
}


/*pstat helper: read and print proc stat*/
void read_stat(pid_t pid){
  /*create proc path*/
  char procstat_path[MAXLINESIZE];
  sprintf(procstat_path, "/proc/%d/stat",pid);
  /*Open proc file: read only*/
  FILE *stat;
  stat = fopen(procstat_path, "r");
  if(stat != NULL){
    /*read then tokenize*/
    char readstat[MAXREAD];
    fgets(readstat, MAXREAD, stat);
    int i = 0;
    char* p;
    char *tok = strtok(readstat, " ");
    float tck = (float)(sysconf(_SC_CLK_TCK));
    while(tok != NULL){
      switch(i) {
        /*comm*/
        case 1:
          printf("        comm: %s\n", tok);
          break;
        /*state*/
        case 2:
          printf("        state: %s\n", tok);
          break;
        /*utime*/
        case 13:
          /*do cal*/
          printf("        utime: %f\n", atof(tok)/tck);
          break;
        /*stime*/
        case 14:
          /*do cal*/
          printf("        stime: %f\n", atof(tok)/tck);
          break;
        /*rss*/
        case 23:
          printf("        rss: %s\n", tok);
        break;
        /*else: do nothing*/
        default:
          break;
      }
      i++;
      tok = strtok(NULL, " ");
    }
  fclose(stat);
  }else{
    printf("PMan:> Failed to open %d stat file\n", pid);
  }
}


/*pstat helper: read and print proc status*/
void read_status(pid){
  /*create status proc path*/
  char procstatus_path[MAXLINESIZE];
  sprintf(procstatus_path, "/proc/%d/status",pid);
  /*Open proc file: read only*/
  FILE *status;
  status = fopen(procstatus_path, "r");
  if(status != NULL){
  	char readstatus[MAXREAD];
  	int i = 0;
  	fgets(readstatus, MAXREAD, status);
  	while(i < 43){
    	/*voluntary_ctxt_switches*/
		  if( i == 40){
	  		printf("        %s", readstatus);
		  }
      /*nonvoluntary_ctxt_switches*/
		  if(i == 41){
	  		printf("        %s", readstatus);
		  }
		  fgets(readstatus, MAXREAD, status);
		  i++;
    }
  fclose(status);
  }else{
    printf("PMan:> Failed to open %d status file\n", pid);
  }
}

/* ---------     COMMAND METHODS     ---------- */
/*bg p_name: run program in the background*/
void bg(char *p_name){
  /*read in process name and arguments*/
  char* args [50];
  build_args(args);
  p_name = args[0];
  /*ERROR handling: if process name not entered*/
  if(strcmp(p_name, "\n") == 0){
    printf("PMan:> Error: Process name not found\n");
    return;
  }
  /*create child process*/
  pid_t pid = fork();
  /*child process*/
  if(pid == 0){
    if(execvp(p_name, args) < 0){
      printf("PMan:> Failed to execute %s\n", p_name);
      exit(-1);
    }
  /*parent process*/
  }else if(pid > 0){
    printf("PMan:> Background process: %d started\n", pid);
    /*Add process to list*/
    p_list = add_to_list(p_list, new_p(p_name, pid));
    sleep(1);
  /*fork failed*/
  }else{
    perror("PMan:> Fork failed");
    exit(1);
  }
}


/*Display list of all program currently executing in the background*/
void bglist(){
  updat_process();
  printf("PMan:> \n");
  int job_count = 0;
  /*to find process absolute path*/
  char path_temp[MAXLINESIZE];
  char *abs_path;
  /*iterate through list of process AND print running process*/
  process *list_pt = p_list;
  while(list_pt != NULL){
    if(list_pt->state == 1){
      abs_path = realpath(list_pt->name, path_temp);
      printf("       %d: %s\n", list_pt->pid, abs_path);
      job_count++;
    }
    list_pt = list_pt->next;
  }
  printf("       Total background jobs: %d\n", job_count);
}


/*Send TERM signal to terminate job*/
void bgkill(pid_t pid){
  if(check_pid(pid)){
    int fail = kill(pid, SIGTERM);
    sleep(1);
    if(fail){
      printf("PMan:> Fail to kill process %d.\n", pid);
    }
  }else{
    printf("PMan:> Error: Process %d does not exist.\n", pid);
  }
  return;
}

/*send STOP signal to stop job temporarily*/
void bgstop(pid_t pid){
  if(check_pid(pid)){
    int fail = kill(pid, SIGSTOP);
    if(fail){
      printf("PMan:> Fail to stop process %d.\n", pid);
    }
  }else{
    printf("PMan:> Error: Process %d does not exist.\n", pid);
  }
  updat_process();
  return;
}

/*send CONT signal to restart job*/
void bgstart(pid_t pid){
  if(check_pid(pid)){
    int fail = kill(pid, SIGCONT);
    if(fail){
      printf("PMan:> Fail to restart process %d.\n", pid);
    }
  }else{
    printf("PMan:> Error: Process %d does not exist.\n", pid);
  }
  return;
}

void pstat(pid_t pid){
  if(check_pid(pid)){
    printf("PMan:> \n");
    read_stat(pid);
    read_status(pid);
  }else{
    printf("PMan:> Error: Process %d does not exist.\n", pid);
  }
}

int main(int argc, char *argv[]){

  /*promt for user input*/
  char *command = (char *)malloc(sizeof(char));
  char *p_name = (char *)malloc(sizeof(char));
  /*Use to exit PMan program*/
  int end = 1;

  /*continue prompting user*/
  while(end == 1){
    updat_process();
    printf("PMan:> ");

    /*Commands*/
    scanf("%s", command);
    /*get PID from command line*/
    char scan[10];
    pid_t pid;
    //printf("command is %s\n", command);
    if(strncmp(command, "bg", 10)==0){
      bg(p_name);
    }else if(strncmp(command, "bglist", 10)==0){
      bglist();
    }else if(strncmp(command, "bgkill", 10)==0){
      scanf("%s", scan);
      pid = atoi(scan);
      bgkill(pid);
    }else if(strncmp(command, "bgstop", 10)==0){
      scanf("%s", scan);
      pid = atoi(scan);
      bgstop(pid);
    }else if(strncmp(command, "bgstart", 10)==0){
      scanf("%s", scan);
      pid = atoi(scan);
      bgstart(pid);
    }else if(strncmp(command, "pstat", 10)==0){
      scanf("%s", scan);
      pid = atoi(scan);
      pstat(pid);
      /*END PMAN: if user enter end command*/
    }else if(strncmp(command, "end", 10)==0){
      end = 0;
      /*command not found*/
    }else{
      printf("PMan:> %s: command not found\n", command);
    }
  }

  printf("PMan:> Exiting PMan\n");
  free(command);
  free(p_name);
}
