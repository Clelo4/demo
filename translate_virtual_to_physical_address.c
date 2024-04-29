/**
 * 参考：https://shanetully.com/2014/12/translating-virtual-addresses-to-physcial-addresses-in-user-space/
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <errno.h>
#include <fcntl.h>
#include <stdint.h>

#define ORIG_STR "Hello, World!"
#define NEW_STR "Hello, Linux!"

// The page frame shifted left by PAGE_SHIFT will give us the physcial address of the frame
// Note that this number is architecture dependent. For me on x86_64 with 4096 page sizes,
// it is defined as 12. If you're running something different, check the kernel source
// for what it is defined as.
#ifdef __x86_64__
#define PAGE_SHIFT 12
#define PAGEMAP_LENGTH 8
#else
#define PAGE_SHIFT
#define PAGEMAP_LENGTH
#endif

void *createBuffer(void);
uint64_t getPhysicalPageNumber(void *addr);

int main(void)
{
    const int PAGE_SIZE = getpagesize();

    // 创建一个固定在物理内存的虚拟内存缓存
    void *buffer = createBuffer();
    printf("Virutal memeory address: 0x%llx\n", (uint64_t)buffer);

    // 获取虚拟内存buffer的物理内存表编号PPN（Physical Page Number）
    uint64_t ppn = getPhysicalPageNumber(buffer);
    printf("PPN (Physical Page Number): 0x%llx\n", ppn);

    // 获取虚拟内存buffer的页内偏移量（Virtual Page Offer）
    uint64_t vpo = (uint64_t)buffer % PAGE_SIZE;

    // 计算虚拟内存buffer的全局物理内存偏移量
    uint64_t memOffset = (ppn << PAGE_SHIFT) + vpo;
    printf("Buffer's physical memory offset: 0x%llx\n", memOffset);

    // 用root用户打开全局物理内存
    int memFd = open("/dev/mem", O_RDWR);
    if (memFd == -1)
    {
        perror("Error opening /dev/mem: ");
        exit(1);
    }

    // 设置memFd的offset
    off_t pos = lseek(memFd, memOffset, SEEK_SET);
    if (pos == -1)
    {
        perror("Failed to seek /dev/mem: ");
        exit(1);
    }

    // 修改物理内存的数据，验证虚拟内存的数据是否同步修改
    if (write(memFd, NEW_STR, strlen(NEW_STR)) == -1)
    {
        perror("Failed to write /dev/mem: ");
        exit(1);
    }

    printf("Buffer: %s\n", (char*)buffer);

    free(buffer);
    close(memFd);

    return 0;
}

void *createBuffer(void)
{
    const size_t buffSize = strlen(ORIG_STR) + 1;
    void *buffer = malloc(buffSize);

    if (buffer == NULL)
    {
        perror("Failed to allocate memory for buffer\n");
        exit(1);
    }

    if (mlock(buffer, buffSize) == -1)
    {
        perror("Failed to lock (unlock) physical pages in memory\n");
        exit(1);
    }

    strncpy(buffer, ORIG_STR, strlen(ORIG_STR));

    return buffer;
}

uint64_t getPhysicalPageNumber(void *addr)
{
    // Open the pagemap file for the current process
    int pageMapFd = open("/proc/self/pagemap", O_RDONLY);
    if (pageMapFd == -1) {
        perror("Failed to open /proc/self/pagemap: ");
        exit(1);
    }

    // 获取addr在页表中的页表项PTE的偏移量
    uint64_t offset = (uint64_t)addr / getpagesize() * PAGEMAP_LENGTH;

    if (lseek(pageMapFd, offset, SEEK_SET) == -1) {
        perror("Failed to seek /proc/self/pagemap: ");
        exit(1);
    }

    // 读取物理页号PPN（或page frame number）
    // 详情参考：https://www.kernel.org/doc/Documentation/vm/pagemap.txt
    uint64_t pageFrameNumber = 0;
    if (read(pageMapFd, (void*)&pageFrameNumber, PAGEMAP_LENGTH) == -1) {
        perror("Failed to read /proc/self/pagemap: ");
        exit(1);
    }

    // 取前54位
    pageFrameNumber &= 0x7FFFFFFFFFFFFF;

    close(pageMapFd);

    return pageFrameNumber;
}