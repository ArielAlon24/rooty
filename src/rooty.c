#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Ariel Alon");
MODULE_DESCRIPTION("Rooty");
MODULE_VERSION("0.01");

static int __init rooty_init(void) {
    printk(KERN_INFO "Hello, world!\n");
    return 0;
}

static void __exit rooty_exit(void) { printk(KERN_INFO "Goodbye, world!\n"); }

module_init(rooty_init);
module_exit(rooty_exit);
