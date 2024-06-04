#include <dlfcn.h>
#include <assert.h>
#include <stddef.h>

int main(void) {
	void *ret = dlsym(RTLD_DEFAULT, "stat");
	assert(NULL != ret);
	ret = dlsym(RTLD_DEFAULT, "__xstat");
	assert(NULL != ret);
}
