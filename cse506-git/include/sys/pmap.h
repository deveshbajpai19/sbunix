
#ifndef PMAP_H
#define PMAP_H


#include <sys/memlayout.h>
#include <defs.h>
#include <sys/print.h>
#include <assert.h>

struct Env;

extern char bootstacktop[], bootstack[];

extern struct Page *pages;
extern size_t npages;

extern pml4e_t *boot_pml4e;
extern uint64_t bar5;
#define PAGE_TABLE_ALIGNLENT 0x1000

/* mask out bits under page size */
#define ALIGN_DOWN(x)   (x & ~(PAGE_TABLE_ALIGNLENT-1))




/* This macro takes a kernel virtual address -- an address that points above
 * KERNBASE, where the machine's maximum 256MB of physical memory is mapped --
 * and returns the corresponding physical address.  It panics if you pass it a
 * non-kernel virtual address.
 */
#define PADDR(kva)						\
    ({								\
     physaddr_t __m_kva = (physaddr_t) (kva);		\
     if (__m_kva < KERNBASE)					\
     print("PADDR called with invalid kva %08lx", __m_kva);\
     __m_kva - KERNBASE;					\
     })

/* This macro takes a physical address and returns the corresponding kernel
 * virtual address.  It panics if you pass an invalid physical address. */
#define KADDR(pa)						\
    ({								\
     physaddr_t __m_pa = (pa);				\
     uint32_t __m_ppn = PPN(__m_pa);\
     if (__m_ppn >= npages)					\
     print("KADDR called with invalid pa %08lx", __m_pa);\
     (void*) ((uint64_t)(__m_pa + KERNBASE));				\
     })


enum {
    // For page_alloc, zero the returned physical page.
    ALLOC_ZERO = 1<<0,
};


void mm_init(uint32_t* , void* , void*);
void *boot_alloc(uint64_t n);
void boot_map_segment(pml4e_t *pml4e, uintptr_t la, size_t size, physaddr_t pa, int perm);
void	page_init(void);
struct Page * page_alloc(int alloc_flags);
void	page_free(struct Page *pp);
int	page_insert(pml4e_t *pml4e, struct Page *pp, void *va, int perm);
void	page_remove(pml4e_t *pml4e, void *va);
struct Page *page_lookup(pml4e_t *pml4e, void *va, pte_t **pte_store);
void	page_decref(struct Page *pp);

void	tlb_invalidate(pml4e_t *pml4e, void *va);

void *	mmio_map_region(physaddr_t pa, size_t size);

int	user_mem_check(struct Env *env, const void *va, size_t len, int perm);
void	user_mem_assert(struct Env *env, const void *va, size_t len, int perm);




uint64_t * vmmngr_bump_alloc_process (uint64_t **bumpPtr,uint64_t size);
uint64_t * vmmngr_bump_alloc (uint64_t size);
uint64_t *kmalloc(uint64_t size);
uint64_t *kmalloc_user(pml4e_t *pml4e,uint64_t size);




    static inline ppn_t
page2ppn(struct Page *pp)
{
    return pp - pages;
}

    static inline physaddr_t
page2pa(struct Page *pp)
{
    return page2ppn(pp) << PGSHIFT;

}

    static inline struct Page*
pa2page(physaddr_t pa)
{
    if (PPN(pa) >= npages)
        print("pa2page called with invalid pa");
    print("\npage 2 pa %x", &pages[PPN(pa)]);
    return &pages[PPN(pa)];
}

    static inline void*
page2kva(struct Page *pp)
{
    return KADDR(page2pa(pp));
}

pte_t *pgdir_walk(pde_t *pgdir, const void *va, int create);

pte_t *pml4e_walk(pml4e_t *pml4e, const void *va, int create);

pde_t *pdpe_walk(pdpe_t *pdpe,const void *va,int create);

uint64_t kmalloc_ahci(uint64_t size);
#endif /* PMAP_H */
