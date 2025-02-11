// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright(c) 2022 Huawei Technologies Co., Ltd
 */

#include <linux/acpi.h>
#include <linux/of.h>
#include <linux/init.h>
#include <linux/kvm_host.h>
#include "hisi_virt.h"

static enum hisi_cpu_type cpu_type = UNKNOWN_HI_TYPE;

static bool dvmbm_enabled;

static const char * const hisi_cpu_type_str[] = {
	"Hisi1612",
	"Hisi1616",
	"Hisi1620",
	"HIP09",
	"Unknown"
};

/* ACPI Hisi oem table id str */
static const char * const oem_str[] = {
	"HIP06",	/* Hisi 1612 */
	"HIP07",	/* Hisi 1616 */
	"HIP08",	/* Hisi 1620 */
	"HIP09"		/* HIP09 */
};

/*
 * Probe Hisi CPU type form ACPI.
 */
static enum hisi_cpu_type acpi_get_hisi_cpu_type(void)
{
	struct acpi_table_header *table;
	acpi_status status;
	int i, str_size = ARRAY_SIZE(oem_str);

	/* Get oem table id from ACPI table header */
	status = acpi_get_table(ACPI_SIG_DSDT, 0, &table);
	if (ACPI_FAILURE(status)) {
		pr_warn("Failed to get ACPI table: %s\n",
			acpi_format_exception(status));
		return UNKNOWN_HI_TYPE;
	}

	for (i = 0; i < str_size; ++i) {
		if (!strncmp(oem_str[i], table->oem_table_id, 5))
			return i;
	}

	return UNKNOWN_HI_TYPE;
}

/* of Hisi cpu model str */
static const char * const of_model_str[] = {
	"Hi1612",
	"Hi1616"
};

/*
 * Probe Hisi CPU type from DT.
 */
static enum hisi_cpu_type of_get_hisi_cpu_type(void)
{
	const char *model;
	int ret, i, str_size = ARRAY_SIZE(of_model_str);

	/*
	 * Note: There may not be a "model" node in FDT, which
	 * is provided by the vendor. In this case, we are not
	 * able to get CPU type information through this way.
	 */
	ret = of_property_read_string(of_root, "model", &model);
	if (ret < 0) {
		pr_warn("Failed to get Hisi cpu model by OF.\n");
		return UNKNOWN_HI_TYPE;
	}

	for (i = 0; i < str_size; ++i) {
		if (strstr(model, of_model_str[i]))
			return i;
	}

	return UNKNOWN_HI_TYPE;
}

void probe_hisi_cpu_type(void)
{
	if (!acpi_disabled)
		cpu_type = acpi_get_hisi_cpu_type();
	else
		cpu_type = of_get_hisi_cpu_type();

	kvm_info("detected: Hisi CPU type '%s'\n", hisi_cpu_type_str[cpu_type]);
}

/*
 * We have the fantastic HHA ncsnp capability on Kunpeng 920,
 * with which hypervisor doesn't need to perform a lot of cache
 * maintenance like before (in case the guest has non-cacheable
 * Stage-1 mappings).
 */
#define NCSNP_MMIO_BASE	0x20107E238
bool hisi_ncsnp_supported(void)
{
	void __iomem *base;
	unsigned int high;
	bool supported = false;

	if (cpu_type != HI_1620)
		return supported;

	base = ioremap(NCSNP_MMIO_BASE, 4);
	if (!base) {
		pr_warn("Unable to map MMIO region when probing ncsnp!\n");
		return supported;
	}

	high = readl_relaxed(base) >> 28;
	iounmap(base);
	if (high != 0x1)
		supported = true;

	return supported;
}

static int __init early_dvmbm_enable(char *buf)
{
	return strtobool(buf, &dvmbm_enabled);
}
early_param("kvm-arm.dvmbm_enabled", early_dvmbm_enable);

static void hardware_enable_dvmbm(void *data)
{
	u64 val;

	val  = read_sysreg_s(SYS_LSUDVM_CTRL_EL2);
	val |= LSUDVM_CTLR_EL2_MASK;
	write_sysreg_s(val, SYS_LSUDVM_CTRL_EL2);
}

static void hardware_disable_dvmbm(void *data)
{
	u64 val;

	val  = read_sysreg_s(SYS_LSUDVM_CTRL_EL2);
	val &= ~LSUDVM_CTLR_EL2_MASK;
	write_sysreg_s(val, SYS_LSUDVM_CTRL_EL2);
}

bool hisi_dvmbm_supported(void)
{
	if (cpu_type != HI_IP09)
		return false;

	/* Determine whether DVMBM is supported by the hardware */
	if (!(read_sysreg(aidr_el1) & AIDR_EL1_DVMBM_MASK))
		return false;

	/* User provided kernel command-line parameter */
	if (!dvmbm_enabled || !is_kernel_in_hyp_mode()) {
		on_each_cpu(hardware_disable_dvmbm, NULL, 1);
		return false;
	}

	/*
	 * Enable TLBI Broadcast optimization by setting
	 * LSUDVM_CTRL_EL2's bit[0].
	 */
	on_each_cpu(hardware_enable_dvmbm, NULL, 1);
	return true;
}

int kvm_sched_affinity_vcpu_init(struct kvm_vcpu *vcpu)
{
	if (!kvm_dvmbm_support)
		return 0;

	if (!zalloc_cpumask_var(&vcpu->arch.sched_cpus, GFP_ATOMIC) ||
	    !zalloc_cpumask_var(&vcpu->arch.pre_sched_cpus, GFP_ATOMIC))
		return -ENOMEM;

	return 0;
}

void kvm_sched_affinity_vcpu_destroy(struct kvm_vcpu *vcpu)
{
	if (!kvm_dvmbm_support)
		return;

	free_cpumask_var(vcpu->arch.sched_cpus);
	free_cpumask_var(vcpu->arch.pre_sched_cpus);
}

static void __kvm_write_lsudvmbm(struct kvm *kvm)
{
	write_sysreg_s(kvm->arch.tlbi_dvmbm, SYS_LSUDVMBM_EL2);
}

static void kvm_write_lsudvmbm(struct kvm *kvm)
{
	spin_lock(&kvm->arch.sched_lock);
	__kvm_write_lsudvmbm(kvm);
	spin_unlock(&kvm->arch.sched_lock);
}

static int kvm_dvmbm_get_dies_info(struct kvm *kvm, u64 *vm_aff3s, int size)
{
	int num = 0, cpu;

	for_each_cpu(cpu, kvm->arch.sched_cpus) {
		bool found = false;
		u64 aff3;
		int i;

		if (num >= size)
			break;

		aff3 = MPIDR_AFFINITY_LEVEL(cpu_logical_map(cpu), 3);
		for (i = 0; i < num; i++) {
			if (vm_aff3s[i] == aff3) {
				found = true;
				break;
			}
		}

		if (!found)
			vm_aff3s[num++] = aff3;
	}

	return num;
}

static u32 socket_num, die_num;

static u32 kvm_get_socket_num(void)
{
	int socket_id[MAX_PG_CFG_SOCKETS], cpu;
	u32 num = 0;

	for_each_cpu(cpu, cpu_possible_mask) {
		bool found = false;
		u64 aff3, socket;
		int i;

		aff3 = MPIDR_AFFINITY_LEVEL(cpu_logical_map(cpu), 3);
		/* aff3[7:3]: socket ID */
		socket = (aff3 & SOCKET_ID_MASK) >> SOCKET_ID_SHIFT;
		for (i = 0; i < num; i++) {
			if (socket_id[i] == socket) {
				found = true;
				break;
			}
		}
		if (!found)
			socket_id[num++] = socket;
	}
	return num;
}

static u32 kvm_get_die_num(void)
{
	int die_id[MAX_DIES_PER_SOCKET], cpu;
	u32 num = 0;

	for_each_cpu(cpu, cpu_possible_mask) {
		bool found = false;
		u64 aff3, die;
		int i;

		aff3 = MPIDR_AFFINITY_LEVEL(cpu_logical_map(cpu), 3);
		/* aff3[2:0]: die ID */
		die = aff3 & DIE_ID_MASK;
		for (i = 0; i < num; i++) {
			if (die_id[i] == die) {
				found = true;
				break;
			}
		}
		if (!found)
			die_id[num++] = die;
	}
	return num;
}

static u32 g_die_pg[MAX_PG_CFG_SOCKETS * MAX_DIES_PER_SOCKET][MAX_CLUSTERS_PER_DIE];

static void kvm_get_die_pg(unsigned long pg_cfg, int socket_id, int die_id)
{
	u32 pg_num = 0, i, j;
	u32 pg_flag[MAX_CLUSTERS_PER_DIE];
	u32 die_tmp = socket_id * die_num + die_id;

	for (i = 0; i < MAX_CLUSTERS_PER_DIE; i++) {
		if (test_bit(i, &pg_cfg))
			pg_num++;
		g_die_pg[die_tmp][i] = i;
		pg_flag[i] = 0;
	}

	for (i = 0; i < MAX_CLUSTERS_PER_DIE - pg_num; i++) {
		if (test_bit(i, &pg_cfg)) {
			for (j = 0; j < pg_num; j++) {
				u32 cluster_bak = MAX_CLUSTERS_PER_DIE - pg_num + j;

				if (!test_bit(cluster_bak, &pg_cfg) &&
				    !pg_flag[cluster_bak]) {
					pg_flag[cluster_bak] = 1;
					g_die_pg[die_tmp][i] = cluster_bak;
					g_die_pg[die_tmp][cluster_bak] = i;
					break;
				}
			}
		}
	}
}

static void kvm_update_vm_lsudvmbm(struct kvm *kvm)
{
	u64 mpidr, aff3, aff2, aff1, phy_aff2;
	u64 vm_aff3s[DVMBM_MAX_DIES];
	u64 val;
	int cpu, nr_dies;
	u32 socket_id, die_id;

	nr_dies = kvm_dvmbm_get_dies_info(kvm, vm_aff3s, DVMBM_MAX_DIES);
	if (nr_dies > 2) {
		val = DVMBM_RANGE_ALL_DIES << DVMBM_RANGE_SHIFT;
		goto out_update;
	}

	if (nr_dies == 1) {
		val = DVMBM_RANGE_ONE_DIE << DVMBM_RANGE_SHIFT	|
		      vm_aff3s[0] << DVMBM_DIE1_SHIFT;

		/* fulfill bits [52:0] */
		for_each_cpu(cpu, kvm->arch.sched_cpus) {
			mpidr = cpu_logical_map(cpu);
			aff3 = MPIDR_AFFINITY_LEVEL(mpidr, 3);
			aff2 = MPIDR_AFFINITY_LEVEL(mpidr, 2);
			aff1 = MPIDR_AFFINITY_LEVEL(mpidr, 1);
			socket_id = (aff3 & SOCKET_ID_MASK) >> SOCKET_ID_SHIFT;
			die_id = (aff3 & DIE_ID_MASK) >> DIE_ID_SHIFT;
			if (die_id == TOTEM_B_ID)
				die_id = 0;
			else
				die_id = 1;

			phy_aff2 = g_die_pg[socket_id * die_num + die_id][aff2];
			val |= 1ULL << (phy_aff2 * 4 + aff1);
		}

		goto out_update;
	}

	/* nr_dies == 2 */
	val = DVMBM_RANGE_TWO_DIES << DVMBM_RANGE_SHIFT	|
	      DVMBM_GRAN_CLUSTER << DVMBM_GRAN_SHIFT	|
	      vm_aff3s[0] << DVMBM_DIE1_SHIFT		|
	      vm_aff3s[1] << DVMBM_DIE2_SHIFT;

	/* and fulfill bits [43:0] */
	for_each_cpu(cpu, kvm->arch.sched_cpus) {
		mpidr = cpu_logical_map(cpu);
		aff3 = MPIDR_AFFINITY_LEVEL(mpidr, 3);
		aff2 = MPIDR_AFFINITY_LEVEL(mpidr, 2);
		socket_id = (aff3 & SOCKET_ID_MASK) >> SOCKET_ID_SHIFT;
		die_id = (aff3 & DIE_ID_MASK) >> DIE_ID_SHIFT;
		if (die_id == TOTEM_B_ID)
			die_id = 0;
		else
			die_id = 1;

		if (aff3 == vm_aff3s[0]) {
			phy_aff2 = g_die_pg[socket_id * die_num + die_id][aff2];
			val |= 1ULL << (phy_aff2 + DVMBM_DIE1_CLUSTER_SHIFT);
		} else {
			phy_aff2 = g_die_pg[socket_id * die_num + die_id][aff2];
			val |= 1ULL << (phy_aff2 + DVMBM_DIE2_CLUSTER_SHIFT);
		}
	}

out_update:
	kvm->arch.tlbi_dvmbm = val;
}

void kvm_tlbi_dvmbm_vcpu_load(struct kvm_vcpu *vcpu)
{
	struct kvm *kvm = vcpu->kvm;
	struct kvm_vcpu *tmp;
	cpumask_t mask;
	unsigned long i;

	/* Don't bother on old hardware */
	if (!kvm_dvmbm_support)
		return;

	cpumask_copy(vcpu->arch.sched_cpus, current->cpus_ptr);

	if (likely(cpumask_equal(vcpu->arch.sched_cpus,
				 vcpu->arch.pre_sched_cpus))) {
		kvm_write_lsudvmbm(kvm);
		return;
	}

	/* Re-calculate sched_cpus for this VM */
	spin_lock(&kvm->arch.sched_lock);

	cpumask_clear(&mask);
	kvm_for_each_vcpu(i, tmp, kvm) {
		/*
		 * We may get the stale sched_cpus if another thread
		 * is concurrently changing its affinity. It'll
		 * eventually go through vcpu_load() and we rely on
		 * the last sched_lock holder to make things correct.
		 */
		cpumask_or(&mask, &mask, tmp->arch.sched_cpus);
	}

	if (cpumask_equal(kvm->arch.sched_cpus, &mask))
		goto out_unlock;

	cpumask_copy(kvm->arch.sched_cpus, &mask);

	kvm_flush_remote_tlbs(kvm);

	/*
	 * Re-calculate LSUDVMBM_EL2 for this VM and kick all vcpus
	 * out to reload the LSUDVMBM configuration.
	 */
	kvm_update_vm_lsudvmbm(kvm);
	kvm_make_all_cpus_request(kvm, KVM_REQ_RELOAD_TLBI_DVMBM);

out_unlock:
	__kvm_write_lsudvmbm(kvm);
	spin_unlock(&kvm->arch.sched_lock);
}

void kvm_tlbi_dvmbm_vcpu_put(struct kvm_vcpu *vcpu)
{
	if (!kvm_dvmbm_support)
		return;

	cpumask_copy(vcpu->arch.pre_sched_cpus, vcpu->arch.sched_cpus);
}

void kvm_get_pg_cfg(void)
{
	void __iomem *mn_base;
	u32 i, j;
	u32 pg_cfgs[MAX_PG_CFG_SOCKETS * MAX_DIES_PER_SOCKET];
	u64 mn_phy_base;
	u32 val;

	socket_num = kvm_get_socket_num();
	die_num = kvm_get_die_num();

	for (i = 0; i < socket_num; i++) {
		for (j = 0; j < die_num; j++) {

			/*
			 * totem B means the first CPU DIE within a SOCKET,
			 * totem A means the second one.
			 */
			mn_phy_base = (j == 0) ? TB_MN_BASE : TA_MN_BASE;
			mn_phy_base += CHIP_ADDR_OFFSET(i);
			mn_phy_base += MN_ECO0_OFFSET;

			mn_base = ioremap(mn_phy_base, 4);
			if (!mn_base) {
				kvm_info("MN base addr ioremap failed\n");
				return;
			}
			val = readl_relaxed(mn_base);
			pg_cfgs[j + i * die_num] = val & 0xff;
			kvm_get_die_pg(pg_cfgs[j + i * die_num], i, j);
			iounmap(mn_base);
		}
	}
}

int kvm_sched_affinity_vm_init(struct kvm *kvm)
{
	if (!kvm_dvmbm_support)
		return 0;

	spin_lock_init(&kvm->arch.sched_lock);
	if (!zalloc_cpumask_var(&kvm->arch.sched_cpus, GFP_ATOMIC))
		return -ENOMEM;

	return 0;
}

void kvm_sched_affinity_vm_destroy(struct kvm *kvm)
{
	if (!kvm_dvmbm_support)
		return;

	free_cpumask_var(kvm->arch.sched_cpus);
}

void kvm_hisi_reload_lsudvmbm(struct kvm *kvm)
{
	if (WARN_ON_ONCE(!kvm_dvmbm_support))
		return;

	preempt_disable();
	kvm_write_lsudvmbm(kvm);
	preempt_enable();
}
