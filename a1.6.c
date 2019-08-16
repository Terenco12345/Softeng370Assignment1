/*
    The Merge Sort to use for Operating Systems Assignment 1 2019
    written by Robert Sheehan

    Modified by: Terence Qu
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

/* Merge sort the data. Original algorithm supplied in the assignment. */
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

void print_stack_rlimit(){
	struct rlimit rlimit;
	getrlimit(RLIMIT_STACK, &rlimit);
	printf("%lld", (long long int)rlimit.rlim_cur);
}

/* Step 6 */
void forked_merge_sort(struct block *data){
	struct block left_block;
	struct block right_block;

	left_block.size = data->size / 2;
	left_block.first = data->first;

	right_block.size = left_block.size + (data->size % 2);
	right_block.first = data->first + left_block.size;

	// Initialization
	pid_t l_child_id = -1;
	pid_t r_child_id = -1;
	int fdl[2];
	int fdr[2];
	int pipe_status_left = pipe(fdl);
	int pipe_status_right = pipe(fdr);

	// Process for left block
	l_child_id = fork();
	if(l_child_id < 0){
		// Failure to create process!
		printf("Failure to create left block process.");
		exit(1);
	} else if(l_child_id == 0){
		// This is the left child process.
		close(fdl[0]);
		merge_sort(&left_block);
		write(fdl[1], left_block.first, left_block.size*sizeof(int));
		// Write to pipe.
		exit(0);
	} else {
		// This is the parent process. Start another process for the right block.
		r_child_id = fork();
		if(r_child_id < 0){
			// Failure to create process!
			printf("Failure to create right block process.");
			exit(1);
		} else if(r_child_id == 0){
			// This is the right child process.
			close(fdr[0]);
			merge_sort(&right_block);
			// Write to pipe.
			write(fdr[1], right_block.first, left_block.size*sizeof(int));
			exit(0);
		} else {
			// This is the parent process.
			// Read from pipe.
			close(fdl[1]);
			close(fdr[1]);
			read(fdl[0], left_block.first,left_block.size*sizeof(int));
			read(fdr[0], right_block.first,right_block.size*sizeof(int));
			// Merge.
			merge(&left_block, &right_block);
		}
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
	rlimit.rlim_cur= 900000000;
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
	forked_merge_sort(&start_block);
	printf("---ending\n");

	// Check if the block is sorted properly.
	printf(is_sorted(data, size) ? "sorted\n" : "not sorted\n");
	exit(EXIT_SUCCESS);
}
