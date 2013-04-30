#define _GNU_SOURCE
#include <sys/types.h>	// opendir, closedir
#include <sys/stat.h>	// stat
#include <dirent.h>		// opendir, closedir
#include <string.h>		// strdup
#include <unistd.h>		// getcwd, chdir
#include <stdio.h>		// perror
#include <stdlib.h>		// exit, calloc
#include <errno.h>		// errno

char *app_prefix = "app_id1_";
size_t app_p_len = 8;

// process directory path, oldWD - old working directory
void proc_dir(char *path, char *oldWD){
	DIR *dir;
	struct dirent *de;
	struct stat statbuf;
	char msg[1025];
	char curname[PATH_MAX];
	char dirname[9];
	off_t size;
	if(chdir(path)){
		perror("Can't change directory");
		return;
	}
	if(!(dir = opendir(path))){
		snprintf(msg, 1024, "Couldn't open dir %s", path);
		perror(msg);
		return;
	}
	while((de = readdir(dir)) != NULL){
		if(!strcmp(de->d_name, ".") || !strcmp(de->d_name, "..")) continue;
		if(stat(de->d_name, &statbuf)){
			snprintf(msg, 1024, "Can't STAT %s", de->d_name);
			perror(msg);
			continue;
		}
		size = statbuf.st_size;
		if(S_ISREG(statbuf.st_mode)){
			if(strncmp(de->d_name, app_prefix, app_p_len)) continue; // not our file
			strncpy(dirname, de->d_name + app_p_len, 8);
			if(mkdir(dirname, 0755) && errno != EEXIST){\
				perror("mkdir"); exit(1);
			}
			snprintf(curname, PATH_MAX-1, "%s/%s", dirname, de->d_name);
			printf("Move %s to %s\n", de->d_name, curname);
			if(link(de->d_name, curname)){
				perror("link"); exit(1);
			}
			if(unlink(de->d_name)){
				perror("unlink"); exit(1);
			}
		}
	}
	if(oldWD) chdir(oldWD);
	closedir(dir);
}

int main(int argc, char **argv){
	DIR *dir;
	char *path = NULL, *wd = NULL;
	wd = get_current_dir_name();
	if(argc == 1) path = strdup(wd);
	else{
		path = strdup(argv[1]);
		if(argc == 3){
			app_prefix = argv[2];
			app_p_len = strlen(app_prefix);
		}
	}
	if(!(dir = opendir(path))){
		perror("Couldn't open dir, try to open CWD");
		if(!(dir = opendir(wd))){
			perror("Couldn't open CWD, die");
			exit(1);
		}else{
			free(path);
			path = strdup(wd);
		}
	}
	closedir(dir);
	proc_dir(path, wd);
	free(wd); free(path);
	return 0;
}
