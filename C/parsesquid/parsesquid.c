#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <time.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#define CACHE_FILE	"/tmp/.parsesquid_cache"
#define LOG_FILE	"/var/log/squid/access.log"
#define TIME_INTERVAL 25000

#define NEW		0
#define UPDATE	1

struct {
	int time;
	long offset;
} s_cache;

void help(const char* name){
	printf("\nИспользование:\n%s [Ключи]\nВыдает информацию о трафике, полученном извне и из кеша прокси\n", name);
	printf("\t-h\t--help\t\t\tЭто сообщение\n");
	printf("\t-f\t--from ДАТА\t\tУказать дату, с которой необходимо начать подсчет\n");
	printf("\t-t\t--to ДАТА\t\tУказать дату, на которой необходимо закончить подсчет\n");
	printf("\nФормат даты: Д/М/Г-ч:м, Д/М/Г, ч:м, Д/М, М\n\n");
	exit(0);
}

int get_date(char *line){
	time_t date;
	struct tm time_, time_now;
	time(&date);
	time_now = *localtime(&date);
	time_.tm_sec = 0;
	if(sscanf(line, "%d/%d/%d-%d:%d", &time_.tm_mday, &time_.tm_mon, &time_.tm_year, 
		&time_.tm_hour, &time_.tm_min) == 5){time_.tm_mon -= 1;}
	else if(!strchr(line, ':') && sscanf(line, "%d/%d/%d", &time_.tm_mday, &time_.tm_mon, &time_.tm_year) == 3){
		date = -1; time_.tm_mon -= 1;}
	else if(!strchr(line, ':') && sscanf(line, "%d/%d", &time_.tm_mday, &time_.tm_mon) == 2){
		date = -1; time_.tm_mon -= 1; time_.tm_year = time_now.tm_year;}
	else if(sscanf(line, "%d:%d", &time_.tm_hour, &time_.tm_min) == 2){
		time_.tm_year = time_now.tm_year; time_.tm_mon = time_now.tm_mon;
		time_.tm_mday = time_now.tm_mday;}
	else if(!strchr(line, ':') && !strchr(line, '/') && !strchr(line, '.') && !strchr(line, '-')
			&& sscanf(line, "%d", &time_.tm_mon) == 1){
		date = -1; time_.tm_mon -= 1; time_.tm_year = time_now.tm_year;
		time_.tm_mday = 1;}
	else{
		printf("\nНеверный формат времени!\n");
		printf("Форматы: D/M/Y-hh:mm, D/M/Y, hh:mm, D/M, M\n");
		exit(1);
	}
	if(date == -1){
		time_.tm_hour = 0;
		time_.tm_min = 0;		
	}
	if(time_.tm_mon > 11 || time_.tm_mon < 0){
		printf("\nМесяц вне диапазона 1..12\n");
		exit(2);
	}
	if(time_.tm_mday > 31 || time_.tm_mday < 1){
		printf("\nЧисло месяца вне диапазона 1..31, %d\n", time_.tm_mday);
		exit(3);
	}
	if(time_.tm_year > 1900) time_.tm_year -= 1900;
	else if(time_.tm_year > -1 && time_.tm_year < 100) time_.tm_year += 100;
	else if(time_.tm_year < 0 || time_.tm_year > 200){
		printf("\nНеверный формат года %d\n", time_.tm_year);
		exit(4);
	}
	if(time_.tm_hour > 23 || time_.tm_hour < 0){
		printf("\nВремя вне диапазона 0..23 часа\n");
		exit(5);
	}
	if(time_.tm_min > 59 || time_.tm_min < 0){
		printf("\nВремя вне диапазона 0..59 минут\n");
		exit(6);
	}
	date = mktime(&time_);
#ifdef DEBUG
	printf("date: %d\n", date);
#endif
	return (int)date;
}

void makecache(unsigned char flag){
	int cache = open(CACHE_FILE, O_CREAT|O_RDWR, 00644);
	long offset = 0;
	int tmp = 0;
	if(cache < 0){
		printf("\nНе могу создать кеш-файл\n");
		exit(7);
	}
	if(flag == UPDATE){
		if((tmp = lseek(cache, -sizeof(s_cache), SEEK_END)) > 0){
			if(read(cache, &s_cache, sizeof(s_cache)) > 0){
				offset = s_cache.offset;
			}
		}
	}
	size_t len = 0;
	char *string = NULL;
	struct stat filestat;
	FILE *log = fopen(LOG_FILE, "r");
	if(!log){
		printf("\nНе могу открыть " LOG_FILE " \n");
		exit(8);
	}
	printf("\nСоздаю кеш\n");
	if(stat(LOG_FILE, &filestat) != 0){
		printf("\nОшибка, " LOG_FILE ": не могу сделать stat\n");
		exit(10);
	}
	if(fseek(log, offset, SEEK_SET) != 0){
		printf("\nВнимание: " LOG_FILE " устарел, обновляю кеш полностью\n");
		offset = 0;
	}
	s_cache.offset = offset;
	if(getline(&string, &len, log) < 1){
		printf("\nОшибка: " LOG_FILE "пуст\n");
		exit(9);
	}
	long dataportion = filestat.st_size / 100;
	int indpos = 1;
	int frac = 0;
	if(offset > 0) frac = atoi(string) / TIME_INTERVAL;
	do{
		
		if( ( tmp = ((s_cache.time = atoi(string)) / TIME_INTERVAL)) != frac ){
			write(cache, &s_cache, sizeof(s_cache));
#ifdef DEBUG
			printf("очередная строка, время: %d, смещение: %d\n", s_cache.time, s_cache.offset);
#endif
			frac = tmp;
		}
		s_cache.offset = ftell(log);
		if( (tmp = s_cache.offset / dataportion) > indpos ){
			if( (tmp % 10) == 0) printf(" %d%% ", tmp);
			else printf(".");
			indpos = tmp;
			fflush(stdout);
		}
	} while(getline(&string, &len, log) > 0);
	printf("\nГотово!\n");
	free(string);
	close(cache);
	fclose(log);
}

int count_bytes(from_offset, to_offset, from_date, to_date){
	long dataportion = (to_offset - from_offset) / 100;
	long long bytes_from_outside = 0, bytes_from_cache = 0, curpos;
	char* string = NULL, *ptr;
	size_t len = 0;
	int time, indpos = 0, tmp, bytes;
	FILE *log = fopen(LOG_FILE, "r");
	fseek(log, from_offset, SEEK_SET);
	while(getline(&string, &len, log) > 0){
		time = atoi(string);
		curpos = ftell(log) - from_offset;
		if( (tmp = curpos / dataportion) > indpos){
			if( (tmp % 10) == 0) printf(" %d%% ", tmp);
			else printf(".");
			indpos = tmp;
			fflush(stdout);		
		}
		if(time < from_date) continue;
		else if(time > to_date) break;
		sscanf(string, "%*s %*s %*s %*s %d",  &bytes);
		if(strstr(string, "NONE")) bytes_from_cache += bytes;
		else bytes_from_outside += bytes;
	}
	printf("\nПолучено информации\n\t\tиз мира: %lld байт (%.2f МБ);\n\t\tиз кэша: %lld байт (%.2f МБ)\n",
		bytes_from_outside, (double)bytes_from_outside/1024./1024., bytes_from_cache,
			(double)bytes_from_cache/1024./1024.);
	free(string);
	fclose(log);
	return(time);
}

int main(int argc, char** argv){
	int from_date = 0, to_date = INT_MAX, last_cache_time, last_log_time;
	struct stat filestat;
	char* const short_options = "hf:t:u";
	struct option long_options[] = {
		{ "help",	0,	NULL,	'h'},
		{ "from",	1,	NULL,	'f'},
		{ "to",		1,	NULL,	't'},
		{ "update",	0,	NULL,	'u'},
		{ NULL,		0,	NULL,	0  }
	};
	int next_option;
	do{
		next_option = getopt_long(argc, argv, short_options, long_options, NULL);
		switch(next_option){
			case 'h': help(argv[0]);
			break;
			case 'f': from_date = get_date((char*)optarg);
			break;
			case 't': to_date = get_date((char*)optarg);
			break;
			case 'u': makecache(UPDATE); exit(0);
			break;
		}
	}while(next_option != -1);
	if(stat(CACHE_FILE, &filestat) != 0) makecache(NEW);
	else if(filestat.st_size == 0) makecache(NEW);
	int cache = open(CACHE_FILE, O_RDONLY);
	size_t bytes_read;
	unsigned char find_from = 1;
	long from_offset = 0, to_offset = 0;
	bytes_read = read(cache, &s_cache, sizeof(s_cache));
	while(bytes_read == sizeof(s_cache)){
#ifdef DEBUG
			printf("\nfrom=%d\n",s_cache.offset);
#endif
		if( (last_cache_time = s_cache.time) >= from_date && find_from){
			find_from = 0;
			from_offset = s_cache.offset;
		}
		else if(s_cache.time >= to_date)
			to_offset = s_cache.offset;
		bytes_read = read(cache, &s_cache, sizeof(s_cache));
	}
	close(cache);
	if(find_from){
		printf("\nНачальная дата поиска старше последней записи логов\n");
		exit(11);
	}
	if(to_offset == 0){
		stat(LOG_FILE, &filestat);
		to_offset = filestat.st_size;
	}
	if(to_offset <= from_offset){
		printf("\nНеверно выбраны начало и конец диапазона дат,\n"
			"\tлибо сведения за указанный период отсутствуют\n");
			exit(12);
	}
	printf("\nПровожу подсчет (от %d до %d)\n", from_offset, to_offset);
	printf("\n====================================================================\n");
	last_log_time = count_bytes(from_offset, to_offset, from_date, to_date);
	printf("\n====================================================================\n");
	if(last_log_time - last_cache_time > TIME_INTERVAL){
		printf("\nЗаписи файла кеша устарели. Обновляю...\n");
		makecache(UPDATE);
//		count_bytes(from_offset, to_offset, from_date, to_date);
	}
	exit(0);
}
