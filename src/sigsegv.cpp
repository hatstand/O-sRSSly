/**
 * This source file is used to print out a stack-trace when your program
 * segfaults.  It is relatively reliable and spot-on accurate.
 *
 * This code is in the public domain.  Use it as you see fit, some credit
 * would be appreciated, but is not a prerequisite for usage.  Feedback
 * on it's use would encourage further development and maintenance.
 *
 * Author:  Jaco Kroon <jaco@kroon.co.za>
 *
 * Copyright (C) 2005 - 2008 Jaco Kroon
 */
#ifndef _GNU_SOURCE
# define _GNU_SOURCE
#endif

#include <memory.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <ucontext.h>
#include <dlfcn.h>
#include <execinfo.h>
#ifndef NO_CPP_DEMANGLE
#include <cxxabi.h>
#endif
#include <inttypes.h>

#if defined(REG_RIP)
# define SIGSEGV_STACK_IA64
# define REGFORMAT "%016lx"
#elif defined(REG_EIP)
# define SIGSEGV_STACK_X86
# define REGFORMAT "%08x"
#else
# define SIGSEGV_STACK_GENERIC
# define REGFORMAT "%x"
#endif

using namespace __cxxabiv1;

static void signal_segv(int signum, siginfo_t* info, void*ptr) {
    static const char *si_codes[3] = {"", "SEGV_MAPERR", "SEGV_ACCERR"};

    ucontext_t *ucontext = (ucontext_t*)ptr;

    fprintf(stderr, "Segmentation Fault!\n");
    fprintf(stderr, "info.si_signo = %d\n", signum);
    fprintf(stderr, "info.si_errno = %d\n", info->si_errno);
    fprintf(stderr, "info.si_code  = %d (%s)\n", info->si_code, si_codes[info->si_code]);
    fprintf(stderr, "info.si_addr  = %p\n", info->si_addr);
    for(int i = 0; i < NGREG; i++)
        fprintf(stderr, "reg[%02d]       = 0x" REGFORMAT "\n", i, ucontext->uc_mcontext.gregs[i]);

	void* buffer[100];
	int n = backtrace(buffer, 100);
	char** symbols = backtrace_symbols(buffer, n);
    fprintf(stderr, "Stack trace:\n");
	for (int i = 0; i < n; ++i) {
#ifndef NO_CPP_DEMANGLE
		char* func = strchr(symbols[i], '(');
		if (func) {
			char* func_end = strchr(func, '+');
			ptrdiff_t func_length = func_end - func - 1;
			char* function_string = (char*)malloc(func_length + 1);
			// One past the '('
			char* old_ptr = func + 1;
			for (int j = 0; j < func_length; ++j) {
				function_string[j] = old_ptr[j];
			}
			function_string[func_length] = '\0';
			
			int status;
			char *tmp = __cxa_demangle(function_string, NULL, 0, &status);

			if(status == 0 && tmp) {
				*func = '\0';
				char* address = strchr(func+1, '[');
				fprintf(stderr, "%d: %s %s %s\n", i, symbols[i], tmp, address);
			}

			free(function_string);
		} else {
			fprintf(stderr, "%d: %s\n", i, symbols[i]);
		}
#else
		fprintf(stderr, "%d: %s\n", i, symbols[i]);
#endif
	}

	free(symbols);

    fprintf(stderr, "End of stack trace\n");
    exit (-1);
}

int setup_sigsegv() {
    struct sigaction action;
    memset(&action, 0, sizeof(action));
    action.sa_sigaction = signal_segv;
    action.sa_flags = SA_SIGINFO;
    if(sigaction(SIGSEGV, &action, NULL) < 0) {
        perror("sigaction");
        return 0;
    }

    return 1;
}

#ifndef SIGSEGV_NO_AUTO_INIT
static void __attribute((constructor)) init(void) {
    setup_sigsegv();
}
#endif
