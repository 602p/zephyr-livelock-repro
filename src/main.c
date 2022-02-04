
// Uncomment to enable "mitigation" that turns these livelocks into crashes.
// #define ENABLE_MITIGATION

// Uncomment to cause main thread to join on user thread instead of polling,
//   changes boundary conditions of failure
// #define MODE_JOIN

#include <zephyr.h>
#include <app_memory/app_memdomain.h>
#include <logging/log_ctrl.h>
#include <logging/log.h>

LOG_MODULE_REGISTER(app, LOG_LEVEL_DBG);

K_THREAD_STACK_DEFINE(user_task_stack, 1024);

void k_sys_fatal_error_handler(unsigned int reason, const z_arch_esf_t *esf) {
	LOG_PANIC();
	LOG_ERR("k_sys_fatal_error_handler invoked.");


	uint8_t flags = k_current_get()->base.thread_state;
	bool aborting = flags & _THREAD_ABORTING;
	bool dead = flags & _THREAD_DEAD;
	char* message;

	if      (aborting && dead) message = "aborting & dead";
	else if (aborting)         message = "aborting";
	else if (dead)             message = "dead";
	else                       message = "normal";

	LOG_INF("Thread is %s", message);

#ifdef ENABLE_MITIGATION
	if (aborting || dead) {
		LOG_ERR("Livelock detected. Halt.");
		k_fatal_halt(reason);
	}
#endif

	if (k_current_get()->base.user_options & K_USER) {
		LOG_WRN("Userspace thread; aborting it.");
		return;
	} else {
		LOG_ERR("Kernel thread. Halt.");
		k_fatal_halt(reason);
	}
}

void stack_overflow(int stride) {
	char arr[stride];
	LOG_DBG("Overflowing, &arr=%p", &arr);
	stack_overflow(stride);
}

void my_entry_point(void* p1, void* unused2, void* unused3) {
	k_thread_name_set(NULL, "user_thread");

	int time = 1;
	while(1) {
		time = time + 1;
		LOG_DBG("Spinning in user thread...");
		k_msleep(1000);
		if (time > 2) {
			LOG_INF("Stackoverflowing...");
			stack_overflow((int)p1);
		}
	}
}

struct k_thread user_task_thread;

void main() {
	// Strides to try overflowing by.
	char to_try[] = {
		     // MODE_JOIN		!MODE_JOIN
		1,   // Works			Works
		10,  // Works			Works
		50,  // Livelocks		Works
		100, //      			Double faults, recovers  
		120  // 				Livelocks
	};

	for (int i = 0; i < sizeof(to_try); i++) {
		int stride = to_try[i];
		LOG_INF("Starting task, stride=%i...", stride);

		k_thread_create(
			&user_task_thread,
			user_task_stack,
			K_THREAD_STACK_SIZEOF(user_task_stack),
			my_entry_point,
			(void*)stride, NULL, NULL,
			-1,
			K_USER,
			K_NO_WAIT
		);

		// Wait for user task to terminate
#ifdef MODE_JOIN
		k_thread_join(&user_task_thread, K_FOREVER);
#else
		while(user_task_thread.base.thread_state != _THREAD_DEAD) {
			LOG_DBG("Spinning in main thread...");
			k_msleep(1000);
		}
#endif
	}
}