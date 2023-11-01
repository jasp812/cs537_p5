/* Define mmap flags */
#define MAP_PRIVATE 0x0001
#define MAP_SHARED 0x0002
#define MAP_ANONYMOUS 0x0004
#define MAP_ANON MAP_ANONYMOUS
#define MAP_FIXED 0x0008
#define MAP_GROWSUP 0x0010
#define MAP_FAIL (void*)-1
#define MAX_MAPS 32

/* Protections on memory mapping */
#define PROT_READ 0x1
#define PROT_WRITE 0x2

struct map_mem {
    void* addr;
    size_t length;
    struct file* f;
    off_t offset;
    int flags;
    int fd;
    int prot;
};
