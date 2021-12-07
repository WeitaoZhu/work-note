#include <pthread.h>
#include "errors.h"

void *thread_routine(void *arg) {
	return arg;
}

int main(void) {
	pthread_t thread_id;
	void *thread_result;
	int status;

	status = pthread_create(&thread_id, NULL, thread_routine, NULL);
	if (status != 0)
		err_abort(status, "Create thread");

	status = pthread_join(thread_id, &thread_result);
	if (status != 0)
		err_abort(status, "Join thread");

	if (thread_result == NULL)
		return 0;
	else
		return 1;
}