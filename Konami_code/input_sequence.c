#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/input.h>
#include <linux/notifier.h>
#include <linux/keyboard.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/unistd.h>
#include <linux/syscalls.h>
#include <linux/cred.h>
#include <linux/sched.h>
#include <linux/workqueue.h>

#define SEQUENCE_LENGTH 10
#define SEQUENCE_FILE "/tmp/sekwencja_wykonana"
#define SCRIPT_FILE "/tmp/launch_firefox.sh"

static int key_sequence[SEQUENCE_LENGTH] = {
    62979, 62979, 62976, 62976, 62977, 62978,
    62977, 62978, 64354, 64353
};
static int current_index = 0;

static struct workqueue_struct *wq;
static struct work_struct open_url_work;

static void create_script_file(void)
{
    struct file *file;
    const char *script_content = "#!/bin/bash\n/usr/bin/firefox https://www.youtube.com/shorts/CKrmjUIriTc\n";

    printk(KERN_INFO "Creating script file to launch Firefox\n");

    file = filp_open(SCRIPT_FILE, O_CREAT | O_WRONLY | O_TRUNC, 0755);
    if (IS_ERR(file)) {
        printk(KERN_ERR "Failed to create script file: %ld\n", PTR_ERR(file));
        return;
    }

    kernel_write(file, script_content, strlen(script_content), &file->f_pos);
    filp_close(file, NULL);

    printk(KERN_INFO "Script file created successfully at %s\n", SCRIPT_FILE);
}

static void open_url_work_func(struct work_struct *work)
{
    printk(KERN_INFO "Launching script to open Firefox with YouTube\n");

    create_script_file();

    char *argv[] = {"/bin/bash", SCRIPT_FILE, NULL};
    int ret = call_usermodehelper(argv[0], argv, NULL, UMH_WAIT_EXEC);

    if (ret) {
        printk(KERN_ERR "Failed to launch script with error code: %d\n", ret);
    }
}

static int input_sequence_handler(struct notifier_block *nb, unsigned long action, void *data)
{
    struct keyboard_notifier_param *param = data;

    if (!param || action != KBD_KEYSYM) {
        return NOTIFY_OK;
    }

    if (param->down) {
        printk(KERN_INFO "Key code: %d pressed\n", param->value);

        if (param->value == key_sequence[current_index]) {
            current_index++;

            if (current_index == SEQUENCE_LENGTH) {
                printk(KERN_INFO "Correct sequence entered, creating signal file...\n");

                struct file *file = filp_open(SEQUENCE_FILE, O_CREAT | O_WRONLY, 0644);
                if (IS_ERR(file)) {
                    printk(KERN_ERR "Failed to create sequence file\n");
                } else {
                    const char *message = "sequence_complete";
                    kernel_write(file, message, strlen(message), &file->f_pos);
                    filp_close(file, NULL);
                }

                INIT_WORK(&open_url_work, open_url_work_func);
                queue_work(wq, &open_url_work);

                current_index = 0;
            }
        } else {
            current_index = 0;
            printk(KERN_INFO "Incorrect key code, resetting sequence\n");
        }
    }

    return NOTIFY_OK;
}

static struct notifier_block input_notifier = {
    .notifier_call = input_sequence_handler,
};

static int __init input_sequence_init(void)
{
    int ret;

    printk(KERN_INFO "Input sequence module loaded\n");

    wq = create_singlethread_workqueue("input_sequence_wq");
    if (!wq) {
        printk(KERN_ERR "Failed to create workqueue\n");
        return -ENOMEM;
    }

    ret = register_keyboard_notifier(&input_notifier);
    if (ret) {
        printk(KERN_ERR "Failed to register keyboard notifier\n");
        return ret;
    }

    return 0;
}

static void __exit input_sequence_exit(void)
{
    unregister_keyboard_notifier(&input_notifier);

    if (wq) {
        destroy_workqueue(wq);
    }

    printk(KERN_INFO "Input sequence module unloaded\n");
}

module_init(input_sequence_init);
module_exit(input_sequence_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("KP & AW");
MODULE_DESCRIPTION("A kernel module to detect key sequence, create a file and launch user-space program");
