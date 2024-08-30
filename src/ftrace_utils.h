#include <linux/ftrace.h>
#include <linux/kprobes.h>
#include <linux/linkage.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/uaccess.h>

#include "logger.h"

#define SYSCALL_NAME(name) "__arm64_sys_" name

#define SYSCALL_HOOK(_name, _function, _original)             \
    {                                                         \
        .name = SYSCALL_NAME(_name), .function = (_function), \
        .original = (_original),                              \
    }

#define HOOK(_name, _function, _original) \
    { .name = (_name), .function = (_function), .original = (_original), }

#define USE_FENTRY_OFFSET 0
#if !USE_FENTRY_OFFSET
#pragma GCC optimize("-fno-optimize-sibling-calls")
#endif

struct ftrace_hook {
    const char *name;
    void *function;
    void *original;

    unsigned long address;
    struct ftrace_ops ops;
};

int fh_install_hook(struct ftrace_hook *hook);
void fh_remove_hook(struct ftrace_hook *hook);
int fh_install_hooks(struct ftrace_hook *hooks, size_t count);
void fh_remove_hooks(struct ftrace_hook *hooks, size_t count);

static int fh_resolve_hook_address(struct ftrace_hook *hook) {
    struct kprobe kp = {.symbol_name = hook->name};

    int ret = register_kprobe(&kp);
    if (ret < 0) {
        WARNING("rooty: register_kprobe failed for %s: %d\n", hook->name, ret);
        return ret;
    }

    hook->address = (unsigned long)kp.addr;
    unregister_kprobe(&kp);

    if (!hook->address) {
        WARNING("unresolved symbol: %s\n", hook->name);
        return -ENOENT;
    }

#if USE_FENTRY_OFFSET
    *((unsigned long *)hook->original) = hook->address + MCOUNT_INSN_SIZE;
#else
    *((unsigned long *)hook->original) = hook->address;
#endif
    return 0;
}

static void fh_ftrace_thunk(unsigned long ip, unsigned long parent_ip,
                            struct ftrace_ops *ops, struct ftrace_regs *fregs) {
    struct ftrace_hook *hook = container_of(ops, struct ftrace_hook, ops);

#if USE_FENTRY_OFFSET
    fregs->pc = (unsigned long)hook->function;
#else
    if (!within_module(parent_ip, THIS_MODULE))
        fregs->pc = (unsigned long)hook->function;
#endif
}

int fh_install_hook(struct ftrace_hook *hook) {
    int err;

    err = fh_resolve_hook_address(hook);
    if (err) return err;

    hook->ops.func = fh_ftrace_thunk;
    hook->ops.flags = FTRACE_OPS_FL_SAVE_REGS_IF_SUPPORTED |
                      FTRACE_OPS_FL_IPMODIFY | FTRACE_OPS_FL_RECURSION;

    err = ftrace_set_filter_ip(&hook->ops, hook->address, 0, 0);

    if (err) {
        WARNING("ftrace_set_filter_ip() failed: %d\n", err);
        return err;
    }

    err = register_ftrace_function(&hook->ops);
    if (err) {
        WARNING("register_ftrace_function() failed: %d\n", err);
        return err;
    }

    INFO("Installed hook for syscall '%s'", hook->name);

    return 0;
}

void fh_remove_hook(struct ftrace_hook *hook) {
    int err;

    err = unregister_ftrace_function(&hook->ops);
    if (err) {
        WARNING("unregister_ftrace_function() failed: %d\n", err);
    }

    err = ftrace_set_filter_ip(&hook->ops, hook->address, 1, 0);
    if (err) {
        WARNING("ftrace_set_filter_ip() failed: %d\n", err);
    }
}

int fh_install_hooks(struct ftrace_hook *hooks, size_t count) {
    int err;
    size_t i;

    for (i = 0; i < count; i++) {
        err = fh_install_hook(&hooks[i]);
        if (err) goto error;
    }
    return 0;

error:
    while (i != 0) {
        fh_remove_hook(&hooks[--i]);
    }
    return err;
}

void fh_remove_hooks(struct ftrace_hook *hooks, size_t count) {
    size_t i;

    for (i = 0; i < count; i++) fh_remove_hook(&hooks[i]);
}
