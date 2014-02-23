/**
 * @file kernel/exec.c
 *
 * !!!
 *
 * Right now this file just contains the code to run the first process. This
 * code is very close to what exec actually does.
 *
 * !!!
 *
 * @author David Matlack
 */
#include <kernel/exec.h>
#include <kernel/loader.h>
#include <kernel/proc.h>
#include <kernel/stack.h>

#include <fs/vfs.h>

#include <assert.h>
#include <list.h>

extern struct vm_space *postboot_vm_space;
struct proc_struct init_proc;

#include <arch/x86/cpu.h> // iret_to_userspace

/**
 * @brief Load and initialize the first process that will run.
 */
void run_first_proc(char *execpath, int argc, char **argv) {
  struct vfs_file *file;
  struct thread_struct *thread;
  int ret;

  ret = proc_fork(NULL, &init_proc);
  ASSERT_EQUALS(ret, 0);

  thread = list_head(&init_proc.threads);

  file = vfs_file_get(execpath);
  ASSERT(NULL != file);

  init_proc.exec = exec_file_get(file);
  ASSERT(NULL != init_proc.exec);

  init_proc.space = postboot_vm_space;

  /*
   * Load the executable into memory
   */
  ret = load(init_proc.exec, init_proc.space);
  ASSERT_EQUALS(ret, 0);

  /*
   * Create a runtime stack for the process.
   */
  ret = create_user_stack(thread, argc, argv);
  ASSERT_EQUALS(ret, 0);

  iret_to_userspace(thread->kstack_hi, (size_t) thread->proc->space->object,
                    thread->proc->exec->entry, thread->ustack_entry);
}
