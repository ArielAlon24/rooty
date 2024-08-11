#ifndef LOGGER_H
#define LOGGER_H

#include <linux/kernel.h>
#include <linux/module.h>

#define KLOG_INFO KERN_INFO
#define KLOG_DEBUG KERN_DEBUG
#define KLOG_WARNING KERN_WARNING

#define INFO(fmt, ...) printk(KLOG_INFO "Rooty: [%s:%d] " fmt, __FILE__, __LINE__, ##__VA_ARGS__)
#define DEBUG(fmt, ...) printk(KLOG_DEBUG "Rooty: [%s:%d] " fmt, __FILE__, __LINE__, ##__VA_ARGS__)
#define WARNING(fmt, ...)                                                                          \
    printk(KLOG_WARNING "Rooty: [%s:%d] " fmt, __FILE__, __LINE__, ##__VA_ARGS__)

#endif
