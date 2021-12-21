#ifndef _FS_DIGGER_H
#define _FS_DIGGER_H	1

#include <stdint.h>
#ifdef __cplusplus
	extern "C" {
	class fsDigger;
	class fsDigger_specimen;
#else
	typedef struct fsDigger fsDigger;
	typedef struct fsDigger_specimen fsDigger_specimen;
#endif
	//	typedef unsigned long uLong;
	int dd_main(int argc, char **argv);
	ssize_t fsDigger_write(int fd, char const* buf, size_t size);

	/*
		***The macro 'STDOUT_FILENO' seems to point to the terminal only.
		***Need to adjust code to not close 'STDOUT_FILENO' to prevent application hangs.
	Creating a tmp file to 'static char const *output_file' not an option:
		-Unnecessary code execution happens.
		-There may be other unexpected side effects.
	Adding code logic to 'static bool close_stdout_required' not an option:
		-There could be possible interrupts.
	Modifying 'cleanup()' not an option:
		-The function will handle normal and unusual program termination.
	Consider modifying name of 'STDOUT_FILENO':
		-Occurs only during normal termination.
	Consider modifying 'finish_up()':
		-Occurs only during normal termination.
	*/
	int DD_STDOUT_FILENO;
	char const *output_file;
	char const *input_file;
	char* ibuf;
	char* obuf;
	uintmax_t r_full;
	uintmax_t w_full;
	uintmax_t w_bytes;
	uintmax_t w_partial;
	uintmax_t seek_records;
	uintmax_t seek_bytes;
	void fsDigger_finish_up(void);
	// void finish_up(void);
	void cleanup_in(void);
	void cleanup_out(void);
	void print_stats(void);
	void process_signals(void);
#ifdef __cplusplus
	}
#endif
#endif
