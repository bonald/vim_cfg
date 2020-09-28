#include <stdio.h>
#include <stddef.h>
#include <signal.h>
#include <sys/poll.h>
#include <sys/mman.h>
#include <sys/user.h>
#define __FRAME_OFFSETS
#include <asm/ptrace.h>
#include <asm/types.h>

#ifdef __i386__
#define __SYSCALL_I386(nr, sym, qual) [nr] = 1,
static char syscalls[] = {
#include <asm/syscalls_32.h>
};
#else
#define __SYSCALL_64(nr, sym, qual) [nr] = 1,
static char syscalls[] = {
#include <asm/syscalls_64.h>
};
#endif

#define DEFINE(sym, val) \
	asm volatile("\n->" #sym " %0 " #val : : "i" (val))

#define DEFINE_LONGS(sym, val) \
	asm volatile("\n->" #sym " %0 " #val : : "i" (val/sizeof(unsigned long)))

void foo(void)
{
#ifdef __i386__
	DEFINE_LONGS(HOST_FP_SIZE, sizeof(struct user_fpregs_struct));
	DEFINE_LONGS(HOST_FPX_SIZE, sizeof(struct user_fpxregs_struct));

	DEFINE(HOST_IP, EIP);
	DEFINE(HOST_SP, UESP);
	DEFINE(HOST_EFLAGS, EFL);
	DEFINE(HOST_AX, EAX);
	DEFINE(HOST_BX, EBX);
	DEFINE(HOST_CX, ECX);
	DEFINE(HOST_DX, EDX);
	DEFINE(HOST_SI, ESI);
	DEFINE(HOST_DI, EDI);
	DEFINE(HOST_BP, EBP);
	DEFINE(HOST_CS, CS);
	DEFINE(HOST_SS, SS);
	DEFINE(HOST_DS, DS);
	DEFINE(HOST_FS, FS);
	DEFINE(HOST_ES, ES);
	DEFINE(HOST_GS, GS);
	DEFINE(HOST_ORIG_AX, ORIG_EAX);
#else
/* modified by chenc for tm 64 um compile error */
struct _header {
	__u64				xfeatures;
	__u64				reserved1[2];
	__u64				reserved2[5];
};
struct _ymmh_state {
	/* 16x YMM registers, 16 bytes each: */
	__u32				ymmh_space[64];
};
struct _xstate {
	struct _fpstate			fpstate;
	struct _header			xstate_hdr;
	struct _ymmh_state		ymmh;
	/* New processor state extensions go here: */
};
/* modified by chenc end*/
	DEFINE(HOST_FP_SIZE, sizeof(struct _xstate) / sizeof(unsigned long));
	DEFINE_LONGS(HOST_BX, RBX);
	DEFINE_LONGS(HOST_CX, RCX);
	DEFINE_LONGS(HOST_DI, RDI);
	DEFINE_LONGS(HOST_SI, RSI);
	DEFINE_LONGS(HOST_DX, RDX);
	DEFINE_LONGS(HOST_BP, RBP);
	DEFINE_LONGS(HOST_AX, RAX);
	DEFINE_LONGS(HOST_R8, R8);
	DEFINE_LONGS(HOST_R9, R9);
	DEFINE_LONGS(HOST_R10, R10);
	DEFINE_LONGS(HOST_R11, R11);
	DEFINE_LONGS(HOST_R12, R12);
	DEFINE_LONGS(HOST_R13, R13);
	DEFINE_LONGS(HOST_R14, R14);
	DEFINE_LONGS(HOST_R15, R15);
	DEFINE_LONGS(HOST_ORIG_AX, ORIG_RAX);
	DEFINE_LONGS(HOST_CS, CS);
	DEFINE_LONGS(HOST_SS, SS);
	DEFINE_LONGS(HOST_EFLAGS, EFLAGS);
#if 0
	DEFINE_LONGS(HOST_FS, FS);
	DEFINE_LONGS(HOST_GS, GS);
	DEFINE_LONGS(HOST_DS, DS);
	DEFINE_LONGS(HOST_ES, ES);
#endif

	DEFINE_LONGS(HOST_IP, RIP);
	DEFINE_LONGS(HOST_SP, RSP);
#endif

	DEFINE(UM_FRAME_SIZE, sizeof(struct user_regs_struct));
	DEFINE(UM_POLLIN, POLLIN);
	DEFINE(UM_POLLPRI, POLLPRI);
	DEFINE(UM_POLLOUT, POLLOUT);

	DEFINE(UM_PROT_READ, PROT_READ);
	DEFINE(UM_PROT_WRITE, PROT_WRITE);
	DEFINE(UM_PROT_EXEC, PROT_EXEC);

	DEFINE(__NR_syscall_max, sizeof(syscalls) - 1);
	DEFINE(NR_syscalls, sizeof(syscalls));
}
