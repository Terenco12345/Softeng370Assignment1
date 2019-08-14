/*
    The Merge Sort to use for Operating Systems Assignment 1 2019
    written by Robert Sheehan

    Modified by: put your name here
    UPI: tqu593

    By submitting a program you are claiming that you and only you have made
    adjustments and additions to this code.
 */

#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h>
#include <sys/resource.h>
#include <stdbool.h>
#include <pthread.h>

#define SIZE    2

struct block {
	int size;
	int *first;
};

// void print_block_data(struct block *blk) {
//     printf("size: %d address: %p\n", blk->size, blk->first);
// }

/* Combine the two halves back together. */
void merge(struct block *left, struct block *right) {
	int combined[left->size + right->size];
	int dest = 0, l = 0, r = 0;
	while (l < left->size && r < right->size) {
		if (left->first[l] < right->first[r])
			combined[dest++] = left->first[l++];
		else
			combined[dest++] = right->first[r++];
	}
	while (l < left->size)
		combined[dest++] = left->first[l++];
	while (r < right->size)
		combined[dest++] = right->first[r++];
	memmove(left->first, combined, (left->size + right->size) * sizeof(int));
}

/* Merge sort the data. */
void merge_sort(struct block *my_data) {
	// print_block_data(my_data);
	if (my_data->size > 1) {
		struct block left_block;
		struct block right_block;
		left_block.size = my_data->size / 2;
		left_block.first = my_data->first;
		right_block.size = left_block.size + (my_data->size % 2);
		right_block.first = my_data->first + left_block.size;
		merge_sort(&left_block);
		merge_sort(&right_block);
		merge(&left_block, &right_block);
	}
}

/* Check to see if the data is sorted. */
bool is_sorted(int data[], int size) {
	bool sorted = true;
	for (int i = 0; i < size - 1; i++) {
		if (data[i] > data[i + 1])
			sorted = false;
	}
	return sorted;
}

/* Merge sort algorithm, but the input and function match the types necessary to
 * be passed into a pthread. */
void *merge_sort_threaded(void *data) {
	struct block *my_data = data;

	if (my_data->size > 1) {
		struct block left_block;
		struct block right_block;
		left_block.size = my_data->size / 2;
		left_block.first = my_data->first;
		right_block.size = left_block.size + (my_data->size % 2);
		right_block.first = my_data->first + left_block.size;
		merge_sort(&left_block);
		merge_sort(&right_block);
		merge(&left_block, &right_block);
	}
}

void print_stack_rlimit(){
	struct rlimit rlimit;
	getrlimit(RLIMIT_STACK, &rlimit);
	printf("%lld", (long long int)rlimit.rlim_cur);
}

/* Step 3 */
void *too_many_threads_merge_sort(void *data){
	struct block *my_data = data;

	if (my_data->size > 1) {
		struct block left_block;
		struct block right_block;
		left_block.size = my_data->size / 2;
		left_block.first = my_data->first;
		right_block.size = left_block.size + (my_data->size % 2);
		right_block.first = my_data->first + left_block.size;

		// Thread for the left block
		pthread_t thread1_id;
		pthread_attr_t thread1_attr;
		pthread_attr_init(&thread1_attr);
		pthread_attr_setstacksize(&thread1_attr, 10000000);
		pthread_create(&thread1_id, &thread1_attr, too_many_threads_merge_sort, (void *)&left_block);

		// Thread for the right block
		pthread_t thread2_id;
		pthread_attr_t thread2_attr;
		pthread_attr_init(&thread2_attr);
		pthread_attr_setstacksize(&thread2_attr, 10000000);
		pthread_create(&thread2_id, &thread2_attr, too_many_threads_merge_sort, (void *)&right_block);

		// Wait for threads to join before merging
		pthread_join(thread1_id, NULL);
		pthread_join(thread2_id, NULL);

		merge(&left_block, &right_block);
	}
}

int main(int argc, char *argv[]) {
	// Print data of the original process stack limit
	printf("The stack limit before was: ");
	print_stack_rlimit();
	printf("\n");

	// Set the stack limit to be about 10x what the default is.
	struct rlimit rlimit;
	getrlimit(RLIMIT_STACK, &rlimit);
	rlimit.rlim_cur= 90000000;
	setrlimit(RLIMIT_STACK, &rlimit);

	// Print data of the process stack limit after it has been changed.
	printf("The stack limit after is now: ");
	print_stack_rlimit();
	printf("\n");

	// Creation of the block of a specified size (default 2).
	long size;

	if (argc < 2) {
		size = SIZE;
	} else {
		size = atol(argv[1]);
	}
	struct block start_block;
	int data[size];
	start_block.size = size;
	start_block.first = data;

	// Populate the block with random integers.
	for (int i = 0; i < size; i++) {
		data[i] = rand();
	}

	// Sort the block.
	printf("starting---\n");
	too_many_threads_merge_sort(&start_block);
	printf("---ending\n");

	// Check if the block is sorted properly.
	printf(is_sorted(data, size) ? "sorted\n" : "not sorted\n");
	exit(EXIT_SUCCESS);
}
