/*
 * Loader Implementation
 *
 * 2022, Operating Systems
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "exec_parser.h"
#include <stdbool.h>

#define SIGSEGV_ERROR 139

static so_exec_t *exec;
static int exec_decriptor;

ssize_t mm_read(int fd, void *buf, size_t count)
{
	ssize_t bytes_read =read(fd, buf,count);
	return bytes_read;
}

void copy_into(so_seg_t *segment,size_t  offset, void *adresa)
{
	ssize_t pageSize = getpagesize();
	char *buffer = malloc(sizeof(char)*pageSize);
	lseek(exec_decriptor, segment->offset + offset, SEEK_SET);
	if (offset + pageSize <= segment->file_size)
	{
		mm_read(exec_decriptor, buffer, pageSize);
		memcpy(adresa, buffer, pageSize);
	}
	else if (offset <= segment->file_size)
	{
		mm_read(exec_decriptor, buffer, segment->file_size - offset);
		memset(buffer + segment->file_size - offset, 0, offset + pageSize - segment->file_size);
		memcpy(adresa, buffer, pageSize);
	}
	else if (offset > segment->file_size)
		memset(adresa, 0, pageSize);
	free(buffer);
}

so_seg_t *find_segment_of(void *addr)
{
	long diff;
	for (int i = 0; i < exec->segments_no; i++)
	{
		diff = (char *)addr - (char *)exec->segments[i].vaddr;
		if (diff <= exec->segments[i].mem_size && diff >= 0)
			return &(exec->segments[i]);
	}
	return NULL;
}

static void segv_handler(int sig, siginfo_t *si, void *unused)
{
	size_t pagesize = getpagesize();
	so_seg_t *segment = find_segment_of(si->si_addr);
	size_t segment_offset = (char *)si->si_addr - (char *)segment->vaddr;
	size_t page_offset = segment_offset % pagesize; //
	segment_offset -= page_offset;

	if (!segment|(si->si_code==SEGV_ACCERR))
	{
		exit(SIGSEGV_ERROR);
	}
	void *adresa = mmap((void *)segment->vaddr + segment_offset, pagesize, PERM_R | PERM_W, MAP_FIXED | MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	copy_into(segment, segment_offset, adresa);
	mprotect(adresa, pagesize, segment->perm);
}

int so_init_loader(void)
{
	struct sigaction sig;
	memset(&sig, 0, sizeof(sig));
	sig.sa_flags = SA_SIGINFO;
	sigemptyset(&sig.sa_mask);
	sig.sa_sigaction = segv_handler;
	sigaction(SIGSEGV, &sig, NULL);

	return -1;
}

int so_execute(char *path, char *argv[])
{
	exec_decriptor = open(path, O_RDONLY); 
	exec = so_parse_exec(path);
	if (!exec)
		return -1;

	so_start_exec(exec, argv);

	return -1;
}
