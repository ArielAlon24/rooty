#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>

#include "ftrace_utils.h"
#include "logger.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Ariel Alon");
MODULE_DESCRIPTION("Rooty");
MODULE_VERSION("0.01");

static asmlinkage long (*orig_mkdirat)(const struct ftrace_regs *);
asmlinkage int hook_mkdirat(const struct ftrace_regs *regs);

asmlinkage int hook_mkdirat(const struct ftrace_regs *regs) {
    // int dfd = regs->regs[0];                    // x0: directory file descriptor
    char __user *pathname = (char *)regs->regs[1]; // x1: pathname
    char dir_name[NAME_MAX] = {0};

    long error = strncpy_from_user(dir_name, pathname, NAME_MAX);

    if (error > 0) {
        INFO("Trying to create directory with name: %s\n", dir_name);
    }

    int result = orig_mkdirat(regs);
    return result;
}

static struct ftrace_hook hooks[] = {
    HOOK("mkdirat", hook_mkdirat, &orig_mkdirat),
};

static int __init rooty_init(void) {

    int err;
    err = fh_install_hooks(hooks, ARRAY_SIZE(hooks));
    if (err)
        return err;

    INFO("rooty: loaded\n");
    return 0;
}

static void __exit rooty_exit(void) {
    fh_remove_hooks(hooks, ARRAY_SIZE(hooks));
    INFO("rooty: unloaded\n");
}

module_init(rooty_init);
module_exit(rooty_exit);
