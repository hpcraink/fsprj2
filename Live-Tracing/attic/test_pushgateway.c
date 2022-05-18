#include<stdio.h>
#include<stdlib.h>

int main() {
	printf("Hello World\n");
	int status = system("echo 'some_metric 5' | curl --data-binary @- http://localhost:9091/metrics/job/some_job");
	return 0;
}
