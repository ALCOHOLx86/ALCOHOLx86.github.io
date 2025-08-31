#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

int main(int argc, char** argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage BEER <exe_path>\n");
        return 1;
    }
    const char* path = argv[1];

    int fd = open(path, O_RDONLY);
    if (fd < 0) { perror("open"); return 1; }

    struct stat st;
    if (fstat(fd, &st) < 0) { perror("fstat"); close(fd); return 1; }
    size_t size = st.st_size;

    void* mem = mmap(NULL, size, PROT_READ | PROT_WRITE,
                     MAP_PRIVATE |  MAP_ANON, -1, 0);
    if(mem == MAP_FAILED) { perror("mmap"); close(fd); return 1; }

    if (read(fd, mem, size) != size) { perror("read"); munmap(mem, size); close(fd); return 1; }
    close(fd);

    if (mprotect(mem, size, PROT_READ | PROT_EXEC) != 0) {
        perror("mprotect"); munmap(mem, size); return 1;
    }

    printf("loaded %s into memory at %p. Attempting to execute...\n", path, mem);

    void (*entry)() = mem;
    entry();

    munmap(mem, size);
    return 0;
}