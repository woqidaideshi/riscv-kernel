/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2018 ARM Limited
 */
#ifndef __ASM_VDSO_GETTIMEOFDAY_H
#define __ASM_VDSO_GETTIMEOFDAY_H

#ifndef __ASSEMBLY__

#include <asm/alternative.h>
#include <asm/barrier.h>
#include <asm/unistd.h>
#include <asm/sysreg.h>

#define VDSO_HAS_CLOCK_GETRES		1

static __always_inline
int gettimeofday_fallback(struct __kernel_old_timeval *_tv,
			  struct timezone *_tz)
{
	register struct timezone *tz asm("x1") = _tz;
	register struct __kernel_old_timeval *tv asm("x0") = _tv;
	register long ret asm ("x0");
	register long nr asm("x8") = __NR_gettimeofday;

	asm volatile(
	"       svc #0\n"
	: "=r" (ret)
	: "r" (tv), "r" (tz), "r" (nr)
	: "memory");

	return ret;
}

static __always_inline
long clock_gettime_fallback(clockid_t _clkid, struct __kernel_timespec *_ts)
{
	register struct __kernel_timespec *ts asm("x1") = _ts;
	register clockid_t clkid asm("x0") = _clkid;
	register long ret asm ("x0");
	register long nr asm("x8") = __NR_clock_gettime;

	asm volatile(
	"       svc #0\n"
	: "=r" (ret)
	: "r" (clkid), "r" (ts), "r" (nr)
	: "memory");

	return ret;
}

static __always_inline
int clock_getres_fallback(clockid_t _clkid, struct __kernel_timespec *_ts)
{
	register struct __kernel_timespec *ts asm("x1") = _ts;
	register clockid_t clkid asm("x0") = _clkid;
	register long ret asm ("x0");
	register long nr asm("x8") = __NR_clock_getres;

	asm volatile(
	"       svc #0\n"
	: "=r" (ret)
	: "r" (clkid), "r" (ts), "r" (nr)
	: "memory");

	return ret;
}

static __always_inline u64 __arch_get_hw_counter(s32 clock_mode,
						 const struct vdso_data *vd)
{
	u64 res;

	/*
	 * Core checks for mode already, so this raced against a concurrent
	 * update. Return something. Core will do another round and then
	 * see the mode change and fallback to the syscall.
	 */
	if (clock_mode == VDSO_CLOCKMODE_NONE)
		return 0;

	/*
	 * If FEAT_ECV is available, use the self-synchronizing counter.
	 * Otherwise the isb is required to prevent that the counter value
	 * is speculated.
	*/
	asm volatile(
	ALTERNATIVE("isb\n"
		    "mrs %0, cntvct_el0",
		    "nop\n"
		    __mrs_s("%0", SYS_CNTVCTSS_EL0),
		    ARM64_HAS_ECV)
	: "=r" (res)
	:
	: "memory");

#ifdef CONFIG_ARM_ARCH_TIMER_WORKAROUND_IN_USERSPACE
	if (vd->vdso_fix) {
		u64 new;
		int retries = 50;

		asm volatile(
		ALTERNATIVE("mrs %0, cntvct_el0",
			    __mrs_s("%0", SYS_CNTVCTSS_EL0),
			    ARM64_HAS_ECV)
		: "=r" (new)
		:
		: "memory");
		while (unlikely((new - res) >> vd->vdso_shift) && retries) {
			asm volatile(
			ALTERNATIVE("mrs %0, cntvct_el0",
				    __mrs_s("%0", SYS_CNTVCTSS_EL0),
				    ARM64_HAS_ECV)
			: "=r" (res)
			:
			: "memory");

			asm volatile(
			ALTERNATIVE("mrs %0, cntvct_el0",
				    __mrs_s("%0", SYS_CNTVCTSS_EL0),
				    ARM64_HAS_ECV)
			: "=r" (new)
			:
			: "memory");
			retries--;
		}
	}
#endif
	arch_counter_enforce_ordering(res);

	return res;
}

static __always_inline
const struct vdso_data *__arch_get_vdso_data(void)
{
	return _vdso_data;
}

#ifdef CONFIG_TIME_NS
static __always_inline
const struct vdso_data *__arch_get_timens_vdso_data(const struct vdso_data *vd)
{
	return _timens_data;
}
#endif

#endif /* !__ASSEMBLY__ */

#endif /* __ASM_VDSO_GETTIMEOFDAY_H */
