#include <linux/module.h> /* Needed by all linux kernel driver modules */
#include <linux/kernel.h> /* Needed for KERN_INFO */
#include <linux/cdev.h> /* provides cdev struct, how the kernel represents char devices internally */

// just sorta added:
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/irq.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <linux/interrupt.h>
#include <asm/io.h>

#define MAJOR_NUM 35
#define DEVICE_NAME "PL-int"

u32 *jpg_addr;

static int irq; // the virtual irq number that will be requested from open firmware
// static test_hw_map *module_register;
static struct cdev timer_intr_cdev;

/* IRQ HANDLER */
static irqreturn_t timer_intr_irq_handler(int irq, void *dev_id) {
//	printk(KERN_ERR "printk err timer_intr - in the irq handler!\n");
//	pr_err("jpgsaver - in the irq handler!\n");
	return IRQ_HANDLED;
}

/* FILE SYSTEM OPERATIONS */
static int timer_intr_open(struct inode *inode, struct file *file) {
	printk(KERN_INFO "timer_intr_open: successful\n");
	return 0;
}

struct file_operations timer_intr_fops = {
	.open = timer_intr_open,
};

/* LOADING/UNLOADING DRIVER FUNCTIONS */
/* init function is run when driver is added to kernel */
static int __init jpgsaver(void) {
	struct irq_data *data;
	struct device_node *np = NULL;
	int result, hw_irq;
	struct resource resource;

	unsigned long *virt_addr;
	unsigned int startAddr;

	// check how to get output
	printk(KERN_INFO "jpgsaver init\n");

	np = of_find_node_by_name(NULL, "PL-int");
	if (!np) {
		printk(KERN_ERR "PL-int: can't find compatible node in this kernel build");
		return -ENODEV;
	} else {
		result = of_address_to_resource(np, 0, &resource);
		if (result < 0) {
			return result;
		}
		printk(KERN_INFO "PL-int: reg. size=%d Bytes\n", (u32)resource.end - (u32)resource.start);
		startAddr = (unsigned int)resource.start;

		// get a virtual irq number from device resource struct
		irq = of_irq_to_resource(np, 0, &resource);
		if (irq == NO_IRQ) {
			printk(KERN_ERR "PL-int: of_irq_to_resource failed...\n");
			of_node_put(np);
			return -ENODEV;
		}
		printk(KERN_INFO "PL-int: virq=%d\n", irq);
		// check the hw irq is correct
		data = irq_get_irq_data(irq);
		hw_irq = WARN_ON(!data)?0:data->hwirq;
		printk(KERN_INFO "PL-int: hw_irq=%d\n", hw_irq);

		if(register_chrdev(MAJOR_NUM,"PL-int",&timer_intr_fops))
		{
			printk(KERN_ERR "PL-int: cannot register /dev/PL-int\n");
			return -ENODEV;
		}
		// map the physical address of the driver into the virtual address space
		virt_addr = of_iomap(np, 0);
		printk(KERN_INFO "jpgsaver at 0x%08X mapped to 0x%08X\n", startAddr, (u32)virt_addr);

		//jpg_addr = (u32 *)virt_addr;
		//printk(KERN_INFO "data at 0x%08X is 0x%08X\n", (u32)virt_addr,*jpg_addr);
		
		// install the irq handler
		result = request_irq(irq, timer_intr_irq_handler, IRQF_DISABLED, "PL-int", NULL);
		if (result < 0) {
			printk(KERN_ERR "unable to request IRQ%d : %d\n", irq, result);
			of_node_put(np);
			return -ENODEV;
		}
	}

	printk(KERN_INFO "jpgsaver with interrupt module inserted successfully\n");

	return 0;
}

static void __exit timer_intr_exit(void)
{
	unregister_chrdev(MAJOR_NUM,"PL-int");
	printk(KERN_INFO "Goodbye - jpgsaver\n");
}

module_init(jpgsaver);
// subsys_initcall(jpgsaver);
module_exit(timer_intr_exit);

MODULE_AUTHOR("David VandeBunte");
MODULE_LICENSE("GPL");
