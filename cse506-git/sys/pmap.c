
#include <sys/pmap.h>
#include <sys/memlayout.h>
#include <sys/ahci.h>
#include <sys/mmu.h>
#include <sys/print.h>
#include <defs.h>
#include <string.h>
#include <sys/mmu.h>
#include <error.h>


uintptr_t end;

void check_page_free_list(bool);
void check_page_alloc(void);

//#define BOOT_PAGE_TABLE_START 0xf0008000
//#define BOOT_PAGE_TABLE_END   0xf000e000

// These variables are set by i386_detect_memory()
size_t npages;          // Amount of physical memory (in pages)
static size_t npages_basemem;   // Amount of base memory (in pages)

// These variables are set in mem_init()
pml4e_t *boot_pml4e;        // Kernel's initial page directory
physaddr_t boot_cr3;        // Physical address of boot time page directory

struct Page *pages;     // Physical page state array
static struct Page *page_free_list; // Free list of physical pages

uint64_t *pte;
// Set up a four-level page table:
//    boot_pml4e is its linear (virtual) address of the root
//
// This function only sets up the kernel part of the address space
// (ie. addresses >= UTOP).  The user part of the address space
// will be setup later.
//
// From UTOP to ULIM, the user is allowed to read but not write.
// Above ULIM the user cannot read or write.
uint64_t bar5;

void mm_init(uint32_t* modulep, void* physbase, void* physfree)
{

    pml4e_t* pml4e;

    static size_t npages_extmem;   // Amount of upper memory (in pages)
    end=KERNBASE+(uintptr_t)physfree;

    struct smap_t {
        uint64_t base, length;
        uint32_t type;
    }__attribute__((packed)) *smap;
    while(modulep[0] != 0x9001) modulep += modulep[1]+2;
    for(smap = (struct smap_t*)(modulep+2); smap < (struct smap_t*)((char*)modulep+modulep[1]+2*4); ++smap) {
        if (smap->type == 1 /* memory */ && smap->length != 0) {

            if(smap->base==0) {
                npages_basemem=smap->length/PGSIZE;
                //print("\nLower Memory Pages = %d\n",npages_basemem);
            }
            else {
                npages_extmem=((smap->base+smap->length) - (uintptr_t)physfree)/PGSIZE;
              //  print("\nHigher Memory Pages = %d\n",npages_extmem);
            }

       //     print("Available Physical Memory [%x-%x]\n", smap->base, smap->base + smap->length);
        }
    }

    npages=npages_basemem+npages_extmem;
  //  print("\nAvailable Physical Pages [%d]\n", npages);		

    pml4e = boot_alloc(PGSIZE);

    memset(pml4e, 0, PGSIZE);
    boot_pml4e = pml4e;
    boot_cr3 = PADDR(pml4e);

    /*
    //moves page_directory (which is a pointer) into the cr3 register.
    asm volatile("mov %0, %%cr3":: "b"(boot_cr3));

    //reads cr0, switches the "paging enable" bit, and writes it back.
    unsigned int cr0;
    asm volatile("mov %%cr0, %0": "=b"(cr0));
    cr0 |= 0x80000000;
    asm volatile("mov %0, %%cr0":: "b"(cr0));
     */


    //////////////////////////////////////////////////////////////////////
    // Allocate an array of npage 'struct Page's and store it in 'pages'.
    // The kernel uses this array to keep track of physical pages: for
    // each physical page, there is a corresponding struct Page in this
    // array.  'npage' is the number of physical pages in memory.
    // User-level programs will get read-only access to the array as well.

    // Your code goes here:

    //cprintf("check1\n");  

    pages = (struct Page*)boot_alloc(npages*sizeof(struct Page));


    //memset(pages,0,npages*sizeof(struct Page));


    // Now that we've allocated the initial kernel data structures, we set
    // up the list of free physical pages. Once we've done so, all further
    // memory management will go through the page_* functions. In
    // particular, we can now map memory using boot_map_segment or page_insert

    page_init(); 

    //    check_page_free_list(1);
    //  check_page_alloc();
    //check_page();

    //////////////////////////////////////////////////////////////////////

    // Now we set up virtual memory 

    //////////////////////////////////////////////////////////////////////

    // Map 'pages' read-only by the user at linear address UPAGES
    // Permissions:
    //    - the new image at UPAGES -- kernel R, user R
    //      (ie. perm = PTE_U | PTE_P)
    //    - pages itself -- kernel RW, user NONE

    // Your code goes here:

    //    boot_map_segment(pml4e,UPAGES, PTSIZE, PADDR(pages), PTE_U|PTE_P);

    //////////////////////////////////////////////////////////////////////

    // Use the physical memory that 'bootstack' refers to as the kernel
    // stack.  The kernel stack grows down from virtual address KSTACKTOP.
    // We consider the entire range from [KSTACKTOP-PTSIZE, KSTACKTOP) 
    // to be the kernel stack, but break this into two pieces:
    //     * [KSTACKTOP-KSTKSIZE, KSTACKTOP) -- backed by physical memory
    //     * [KSTACKTOP-PTSIZE, KSTACKTOP-KSTKSIZE) -- not backed; so if
    //       the kernel overflows its stack, it will fault rather than
    //       overwrite memory.  Known as a "guard page".
    //     Permissions: kernel RW, user NONE

    // Your code goes here:

    //  boot_map_segment(pml4e,KSTACKTOP - KSTKSIZE,KSTKSIZE,(physaddr_t)physbase,PTE_W|PTE_P);

    //////////////////////////////////////////////////////////////////////

    // Map all of physical memory at KERNBASE. 
    // Ie.  the VA range [KERNBASE, 2^32) should map to
    //      the PA range [0, 2^32 - KERNBASE)
    // We might not have 2^32 - KERNBASE bytes of physical memory, but
    // we just set up the mapping anyway.
    // Permissions: kernel RW, user NONE
    // Your code goes here: 

    //    boot_map_segment( pml4e,MEMBASE,256*1024*1024, (uintptr_t)0x0,PTE_W|PTE_P);
    boot_map_segment(pml4e,KERNBASE+(uintptr_t)physbase, 0x7ffe000, (uintptr_t)physbase,PTE_W|PTE_P|PTE_U);
    boot_map_segment( pml4e,KERNBASE+(uintptr_t)0xb8000, 4096*3, (uintptr_t)0xb8000,PTE_W|PTE_P|PTE_U);
    bar5 = checkAllBuses();
    boot_map_segment( pml4e, 0xFFFFFFFF00000000+(uintptr_t)bar5, 4096, (uintptr_t)bar5,PTE_W|PTE_P|PTE_U);
    //  boot_map_segment( pml4e,USTACK, PGSIZE, (uintptr_t)0x1000,PTE_W|PTE_P|PTE_U);
    //boot_map_segment( pml4e,KERNBASE+(uintptr_t)physfree, (npages+1)*4096,(uintptr_t)physfree,PTE_W|PTE_P);
    //   boot_map_segment( pml4e,KERNBASE+(uintptr_t)physfree, 0x7ffe000,(uintptr_t)physfree,PTE_W|PTE_P);

    // check_boot_pml4e(boot_pml4e);

    //pdpe_t *pdpe = KADDR(PTE_ADDR(pml4e[0]));
    //    pde_t *pgdir = KADDR(PTE_ADDR(pdpe[3]));

    //lcr3(boot_cr3);

    //moves page_directory (which is a pointer) into the cr3 register.
    asm volatile("mov %0, %%cr3":: "b"(boot_cr3));
    change_video_pointer();


    //struct Page *p=page_alloc(0);

    //print("\n%x-%x\n",p,PADDR(p));


    /*    int a=2;
          print("%d",a);

          char c='a';
          print("%c",c);

          char *str="ass";
          print("%s",str);
          print("%p",&c);
     */
    /*
    //reads cr0, switches the "paging enable" bit, and writes it back.
    unsigned int cr0;
    asm volatile("mov %%cr0, %0": "=b"(cr0));
    cr0 |= 0x80000000;
    asm volatile("mov %0, %%cr0":: "b"(cr0));
     */

    //    check_page_free_list(0)

    /*  print("hello");*/

}


// This simple physical memory allocator is used only while JOS is setting
// up its virtual memory system.  page_alloc() is the real allocator.
//
// If n>0, allocates enough pages of contiguous physical memory to hold 'n'
// bytes.  Doesn't initialize the memory.  Returns a kernel virtual address.
//
// If n==0, returns the address of the next free page without allocating
// anything.
//
// If we're out of memory, boot_alloc should panic.
// This function may ONLY be used during initialization,
// before the page_free_list list has been set up.

    void *
boot_alloc(uint64_t n)
{
    static char *nextfree;  // virtual address of next byte of free memory
    char *result = 0;
    // Initialize nextfree if this is the first time.
    // 'end' is a magic symbol automatically generated by the linker,
    // which points to the end of the kernel's bss segment:
    // the first virtual address that the linker did *not* assign
    // to any kernel code or global variables.

    // Allocate a chunk large enough to hold 'n' bytes, then update
    // nextfree.  Make sure nextfree is kept aligned
    // to a multiple of PGSIZE.
    //
    // LAB 2: Your code here.

    if (!nextfree) {
        nextfree = ROUNDUP((char *) end, PGSIZE);
        result=nextfree;
        nextfree+=n;
    }
    else
    {
        if(n==0) {
            nextfree = ROUNDUP((char *) nextfree, PGSIZE);          /*Fix 1*/
            PADDR(nextfree);
            result=nextfree;
        }
        else
            if(n>0)
            {
                nextfree = ROUNDUP((char *) nextfree, PGSIZE);
                result=nextfree;
                nextfree+=n;
                PADDR(nextfree);
            }
    }
    return result;

}


// --------------------------------------------------------------
// Tracking of physical pages.
// The 'pages' array has one 'struct Page' entry per physical page.
// Pages are reference counted, and free pages are kept on a linked list.

// --------------------------------------------------------------
//
// Initialize page structure and memory free list.
// After this is done, NEVER use boot_alloc again.  ONLY use the page
// allocator functions below to allocate and deallocate physical
// memory via the page_free_list.

// --------------------------------------------------------------
// Tracking of physical pages.
// The 'pages' array has one 'struct Page' entry per physical page.
// Pages are reference counted, and free pages are kept on a linked list.

// --------------------------------------------------------------
// Initialize page structure and memory free list.
// After this is done, NEVER use boot_alloc again.  ONLY use the page
// allocator functions below to allocate and deallocate physical
// memory via the page_free_list.

    void
page_init(void)
{

    // LAB 4:
    // Change your code to mark the physical page at MPENTRY_PADDR
    // as in use

    // The example code here marks all physical pages as free.
    // However this is not truly the case.  What memory is free?
    //  1) Mark physical page 0 as in use.
    //     This way we preserve the real-mode IDT and BIOS structures
    //     in case we ever need them.  (Currently we don't, but...)
    //  2) The rest of base memory, [PGSIZE, npages_basemem * PGSIZE)
    //     is free.
    //  3) Then comes the IO hole [IOPHYSMEM, EXTPHYSMEM), which must
    //     never be allocated.
    //  4) Then extended memory [EXTPHYSMEM, ...).
    //     Some of it is in use, some is free. Where is the kernel
    //     in physical memory?  Which pages are already in use for
    //     page tables and other data structures?
    //
    // Change the code to reflect this.
    // NB: DO NOT actually touch the physical memory corresponding to
    // free pages!
    // NB: Remember to mark the memory used for initial boot page table i.e (va>=BOOT_PAGE_TABLE_START && va < BOOT_PAGE_TABLE_END) as in-use (not free)


    size_t i;
    physaddr_t pa;
    for (i = 0; i < npages; i++) {
        if (i==0) {
            pages[i].pp_ref = 1;
            pages[i].pp_link = NULL;
        } else if (i<npages_basemem) {
            pages[i].pp_ref = 0;
            pages[i].pp_link = page_free_list;
            page_free_list = &pages[i];
        } else if ((i<=(EXTPHYSMEM / PGSIZE))
                || (i<(((uint64_t)boot_alloc(0) - KERNBASE)>>PGSHIFT))){
            pages[i].pp_ref++;
            pages[i].pp_link = NULL;
        } else {
            pages[i].pp_ref = 0;
            pages[i].pp_link = page_free_list;
            page_free_list = &pages[i];
        }

        pa = page2pa(&pages[i]);

        if ((pa == 0 || pa == IOPHYSMEM)
                && (pages[i].pp_ref==0))
            print("page error: i %d\n", i);
    } 

}

//
// Allocates a physical page.  If (alloc_flags & ALLOC_ZERO), fills the entire
// returned physical page with '\0' bytes.  Does NOT increment the reference
// count of the page - the caller must do these if necessary (either explicitly
// or via page_insert).
//
// Be sure to set the pp_link field of the allocated page to NULL
//
// Returns NULL if out of free memory.
//
// Hint: use page2kva and memset

    struct Page *
page_alloc(int alloc_flags)
{

    struct Page * pp = NULL;

    if (!page_free_list)
        return NULL;

    pp = page_free_list;

    page_free_list = page_free_list->pp_link;

    if (alloc_flags & ALLOC_ZERO)
        memset(page2kva(pp), 0, PGSIZE);
    return pp;

}


//
// Return a page to the free list.
// (This function should only be called when pp->pp_ref reaches 0.)
//
    void
page_free(struct Page *pp)
{
    assert(pp->pp_ref==0);
    pp->pp_link = page_free_list;
    page_free_list = pp;
}



//
// Decrement the reference count on a page,
// freeing it if there are no more refs.
//
    void
page_decref(struct Page* pp)
{
    if (--pp->pp_ref == 0)
        page_free(pp);
}


// Given a pml4 pointer, pml4e_walk returns a pointer
// to the page table entry (PTE) for linear address 'va'
// This requires walking the 4-level page table structure
//
// The relevant page directory pointer page(PDPE) might not exist yet.
// If this is true and create == false, then pml4e_walk returns NULL.
// Otherwise, pml4e_walk allocates a new PDPE page with page_alloc.
//       -If the allocation fails , pml4e_walk returns NULL.
//       -Otherwise, the new page's reference count is incremented,
// the page is cleared,
// and it calls the pdpe_walk() to with the given relevant pdpe_t pointer
// The pdpe_walk takes the page directory pointer and fetches returns the page table entry (PTE)
// If the pdpe_walk returns NULL 
//       -the page allocated for pdpe pointer (if newly allocated) should be freed.
// Hint 1: you can turn a Page * into the physical address of the
// page it refers to with page2pa() from kern/pmap.h.
//
// Hint 2: the x86 MMU checks permission bits in both the page directory
// and the page table, so it's safe to leave permissions in the page
// more permissive than strictly necessary.
//
// Hint 3: look at inc/mmu.h for useful macros that mainipulate page
// table, page directory,page directory pointer and pml4 entries.
//



    pte_t *
pml4e_walk(pml4e_t *pml4e, const void *va, int create)
{
    pdpe_t        *pdpe_ptr;
    pte_t         *pte_ptr;

    struct Page   *pp=NULL;

    if (!pml4e[PML4(va)] & PTE_P) {
        switch (create) {
            case 0 :
#if DEBUG
                print("PWALK : |PLM4 No Entry Created| : [index] %d\n",PML4(va));
#endif

                return NULL;
            case 1 :
                if((pp = page_alloc(ALLOC_ZERO))!=NULL) {
#if DEBUG                          
                    print("PWALK : |PLM4 Entry Created| : [index] %d\n",PML4(va));
#endif

                    *(pml4e+PML4(va)) = (pml4e_t)(page2pa(pp)) | PTE_P | PTE_U | PTE_W;
                    // *(pml4e+PML4(va)) = (pml4e_t)(page2pa(pp)) | PTE_P  | PTE_W;
                    pp->pp_ref++;

                }
                else {
#if DEBUG
                    print("PWALK : |PML4E Index Allocation Failed|\n");
#endif

                    return NULL;
                }

                break;

            default :
                break;

        }
    }

    //      cprintf (" DBG :: PML4E :%x\n",pml4e);
    //      cprintf (" DBG :: PML4E[PML4(va)] :%x\n",pml4e[PML4(va)]);

    pdpe_ptr = KADDR(PTE_ADDR(pml4e[PML4(va)]));

#if DEBUG
    print("PWALK : pml4[%d]=%x->%x\n",PML4(va),pml4e[PML4(va)],pdpe_ptr);
#endif

    if((pte_ptr = pdpe_walk(pdpe_ptr, va, create))!=NULL)
        return pte_ptr;
    else {
        if(pp!=NULL) {
            pml4e[PML4(va)] = 0;
            page_decref(pp);
        }
        return NULL;
    }

}


// Given a pdpe i.e page directory pointer pdpe_walk returns the pointer to page table entry
// The programming logic in this function is similar to pml4e_walk.
// It calls the pgdir_walk which returns the page_table entry pointer.
// Hints are the same as in pml4e_walk

pte_t *
pdpe_walk(pdpe_t *pdpe,const void *va,int create){

    pde_t         *pde_ptr;
    pte_t         *pte_ptr;

    struct Page   *pp = NULL;

    if ((!pdpe[PDPE(va)]) & PTE_P ){
        switch (create) {
            case 0 :
#if DEBUG
                print("PWALK : |PDPE No Entry Created| : [index] %d\n",PDPE(va));
#endif
                return NULL;
            case 1 :
                if((pp = page_alloc(ALLOC_ZERO))!=NULL) {
#if DEBUG
                    print("PWALK : |PDPE Entry Created| : [index] %d\n",PDPE(va));
#endif

                    *(pdpe+PDPE(va)) = (pdpe_t)page2pa(pp) | PTE_P | PTE_U | PTE_W;
                    //  *(pdpe+PDPE(va)) = (pdpe_t)page2pa(pp) | PTE_P | PTE_W;
                    pp->pp_ref++;
                }

                else {
#if DEBUG
                    print("PWALK : |PDPE Index Allocation Failed|\n");
#endif

                    return NULL;
                }

                break;
            default :
                break;

        }

    }

    //cprintf("J_PWALK :pdpe at %x  pdpe[%d]=%x\n",pdpe, PDPE(va),pdpe[PDPE(va)]);

    pde_ptr = KADDR(PTE_ADDR(pdpe[PDPE(va)]));

#if DEBUG

    print("PWALK : pdpe[%d]=%x->%x\n",PDPE(va),pdpe[PDPE(va)],pde_ptr);

#endif

    if((pte_ptr = pgdir_walk(pde_ptr, va, create))!=NULL)
        return pte_ptr;
    else {
        if(pp != NULL) {
            pdpe[PDPE(va)] = 0;
            page_decref(pp);
        }
        return NULL;
    }
}

// Given 'pgdir', a pointer to a page directory, pgdir_walk returns
// a pointer to the page table entry (PTE). 
// The programming logic and the hints are the same as pml4e_walk
// and pdpe_walk.

    pte_t *
pgdir_walk(pde_t *pgdir, const void *va, int create)
{
    pte_t  *pte_ptr;
    struct Page   *pp;

    if (!pgdir[PDX(va)] & PTE_P) {
        switch (create) {
            case 0 :
#if DEBUG
                print("PWALK : |PDE No Entry Created| : [index] %d\n",PDX(va));
#endif
                return NULL;
            case 1 :

                if((pp = page_alloc(ALLOC_ZERO))!=NULL) {
                    //cprintf("J_PWALK : |PDE Entry Created| : [index] %d\n",PDX(va));
                    pgdir[PDX(va)] = (pde_t)page2pa(pp) | PTE_P | PTE_W | PTE_U;
                    //  pgdir[PDX(va)] = (pde_t)page2pa(pp) | PTE_P | PTE_W ;

                    if(PDX(va) >= PDX(KERNBASE))
                        pgdir[PDX(va)] = (pde_t)page2pa(pp) | PTE_P | PTE_W | PTE_U; /* Fix : during Lab 3 for Ex 5*/
                    //   pgdir[PDX(va)] = (pde_t)page2pa(pp) | PTE_P | PTE_W; /* Fix : during Lab 3 for Ex 5*/

                    pp->pp_ref++;

                }
                else {
#if DEBUG
                    print("PWALK : |PDE Index Allocation Failed|\n");
#endif
                    return NULL;
                }
                break;
        }

    }

    //      cprintf("J_PWALK : pdpe[%d]  pde[%d]=%x\n",PDPE(va), PDX(va),pgdir[PDX(va)]);

    pte_ptr = KADDR(PTE_ADDR(*(pgdir+PDX(va))));

#if DEBUG
    print("PWALK : pde[%d]=%x->%x\n",PDX(va),pgdir[PDX(va)],pte_ptr);
#endif

    pte_ptr = &pte_ptr[PTX(va)];

    return   pte_ptr;

}





//
// Map [va, va+size) of virtual address space to physical [pa, pa+size)
// in the page table rooted at pml4e.  Size is a multiple of PGSIZE.
// Use permission bits perm|PTE_P for the entries.
//
// This function is only intended to set up the ``static'' mappings
// above UTOP. As such, it should *not* change the pp_ref field on the
// mapped pages.
//
// Hint: the TA solution uses pml4e_walk

    void
boot_map_segment(pml4e_t *pml4e, uintptr_t la, size_t size, physaddr_t pa, int perm)
{
    // Fill this function in
    uint32_t i  =0;
    pte_t   *pte_ptr;

    uintptr_t va = ROUNDUP(la,PGSIZE);

    for (i = 0; i < ROUNDUP(size, PGSIZE); i+=PGSIZE) {             /*Lab4 Fix */
        pte_ptr = pml4e_walk(pml4e, (char*)(va+i), 1);

        if(pte_ptr!=NULL) {
            *pte_ptr = pa+i;
            *pte_ptr = *pte_ptr | (perm | PTE_P | PTE_U | PTE_W) ;
            //  *pte_ptr = *pte_ptr | (perm | PTE_P) ;
        }

        else
            print(" Null Boot map segment\n");

#if DEBUG
        print("P_INSERT : : PDP :%d [va] %x -> [PTE Entry]:%x\n",PDX(la), la+i,PTE_ADDR(*pte_ptr));
#endif     
    }
}


/*
   bump allocator for user process   
 */

uint64_t *kmalloc(uint64_t size)
{
    //uint32_t temp_size = size;
    uint64_t temp_size = size;
    uint64_t pages_required = 0;

    struct Page* pp = NULL;

    //uint64_t *start_address = NULL;
    int i = 0;

    // Calculating the no of pages required corresponding to the size
    if (temp_size < PGSIZE)
        pages_required = 1;
    else {
        pages_required = temp_size/PGSIZE;
        temp_size -= (pages_required*PGSIZE);

        if (temp_size >0) 
            pages_required++;
    }   

    // Getting the pages allocated  
    if (!page_free_list)
        return NULL;

    pp = page_free_list;
    uint64_t *start_address =(uint64_t *)page2kva(pp); 

    for(i = 0; i< pages_required; i++) {
        pp->pp_ref++;
        memset(page2kva((void*)pp), 0, PGSIZE);
        pp = pp->pp_link;
    }   

    page_free_list = pp; 
    //    print("Starting address for %d pages is %x ", pages_required, start_address);

    return start_address;
}


uint64_t * kmalloc_user (pml4e_t *pml4e,uint64_t size) {
    static uint64_t *bumpPtr=(uint64_t *)0xFFFFF0F080700000;     // start vm from here 
    uint64_t *ret;
    ret = bumpPtr; 
    uint64_t no_of_blocks;
    uint64_t *pte=NULL;
    uint64_t i;

    struct Page *pp=NULL;

    no_of_blocks = size/PGSIZE;

    if((size - (size/PGSIZE)*PGSIZE) != 0)
        no_of_blocks ++ ;

    //  printf("1: %p\n",bumpPtr);
    //  printf("blocks %d\n",no_of_blocks);

    while(no_of_blocks) {
        no_of_blocks--;
        for (i = 0; i < ROUNDUP(size, PGSIZE);  i+=PGSIZE) {             /*Lab4 Fix */
            pte = pml4e_walk(pml4e, (char*)bumpPtr, 1); 

            pp = page_alloc(ALLOC_ZERO);
            pp->pp_ref++;

            *pte = ((uint64_t)page2pa(pp)) | PTE_P | PTE_W | PTE_U;
            //print("%x-%x",pte,*pte);
        }                                      
        bumpPtr += 512; 
    }   

    return ret;

}


//
// Map the physical page 'pp' at virtual address 'va'.
// The permissions (the low 12 bits) of the page table entry
// should be set to 'perm|PTE_P'.
//
// Requirements
// - If there is already a page mapped at 'va', it should be page_remove()d.
// - If necessary, on demand, a page table should be allocated and inserted
// into 'pml4e through pdpe through pgdir'.
// - pp->pp_ref should be incremented if the insertion succeeds.
// - The TLB must be invalidated if a page was formerly present at 'va'.
//
// Corner-case hint: Make sure to consider what happens when the same
// pp is re-inserted at the same virtual address in the same pgdir.
// However, try not to distinguish this case in your code, as this
// frequently leads to subtle bugs; there's an elegant way to handle
// everything in one code path.
//
// RETURNS:
// 0 on success
// -E_NO_MEM, if page table couldn't be allocated
//
// Hint: The TA solution is implemented using pml4e_walk, page_remove,
// and page2pa.
//


    int
page_insert(pml4e_t *pml4e, struct Page *pp, void *va, int perm)
{
    pte_t * pte = pml4e_walk(pml4e, (void*)va, 1);
    if (pte == NULL) return -E_NO_MEM;

    pp->pp_ref++;
    if(*pte & PTE_P)
        page_remove(pml4e, va);
    *pte = ((uint64_t)page2pa(pp)) | perm | PTE_P | PTE_U | PTE_W;

    return 0;
}


//
// Return the page mapped at virtual address 'va'.
// If pte_store is not zero, then we store in it the address
// of the pte for this page. This is used by page_remove and
// can be used to verify page permissions for syscall arguments,
// but should not be used by most callers.
//
// Return NULL if there is no page mapped at va.
//
// Hint: the TA solution uses pml4e_walk and pa2page.
//

    struct Page *
page_lookup(pml4e_t *pml4e, void *va, pte_t **pte_store)
{
    pte_t * pte = pml4e_walk(pml4e, (void*)va, 0);
    if (pte == NULL) {
        *pte_store = NULL;
        return NULL;
    }

    if (*pte != 0) {
        if (pte_store != NULL)
            *pte_store = pte;
        return pa2page((physaddr_t)(PTE_ADDR(*pte)));
    }

    return NULL;
}



//
//INVLPG is an instruction available since the 486 that invalidates a single page
//table entry in the TLB. Intel notes that this instruction may be implemented
//differently on future processes, but that this alternate behavior must be explicitly enabled. INVLPG modifies no flags.

//Inline asm in GCC (from Linux kernel source):

static inline void __native_flush_tlb_single(uint64_t addr)
{
    asm volatile("invlpg (%0)" ::"r" (addr) : "memory");

}


//
// Invalidate a TLB entry, but only if the page tables being
// edited are the ones currently in use by the processor.
//
    void
tlb_invalidate(pml4e_t *pml4e, void *va)
{
    //        assert(pml4e != NULL);
    // Flush the entry only if we're modifying the current address space.
    //        assert(pml4e!=NULL);
    //        if (!curenv || curenv->env_pml4e == pml4e)
    __native_flush_tlb_single((uint64_t)va);
}


//
// Unmaps the physical page at virtual address 'va'.
// If there is no physical page at that address, silently does nothing.
//
// Details:
// - The ref count on the physical page should decrement.
// - The physical page should be freed if the refcount reaches 0.
// - The pg table entry corresponding to 'va' should be set to 0.
// (if such a PTE exists)
// - The TLB must be invalidated if you remove an entry from
// the page table.
//
// Hint: The TA solution is implemented using page_lookup,
//         tlb_invalidate, and page_decref.
//
    void
page_remove(pml4e_t *pml4e, void *va)
{
    pte_t *pte;
    struct Page *pp = page_lookup(pml4e, va, &pte);
    if (pp) {
        if (pte) {
            *pte = 0;
            tlb_invalidate(pml4e, va);
        }
        page_decref(pp);
    }
}
uint64_t kmalloc_ahci(uint64_t size)
{
        uint32_t temp_size = size;
        int pages_required = 0;
        struct Page* pp = NULL;
        uint64_t start_address;
        int i = 0;
        // Calculating the no of pages required corresponding to the size
        if (temp_size < PGSIZE)
                pages_required = 1;
        else
                {
                pages_required = temp_size/PGSIZE;
                temp_size -= (pages_required*PGSIZE);
                if (temp_size >0)
                        pages_required++;
                }
        // Getting the pages allocated  
        if (!page_free_list)
                return 0;

        pp = page_free_list;
        start_address =(uint64_t) pp;

        for(i = 0; i< pages_required; i++)
        {
                pp->pp_ref++;
                memset(page2kva((void*)pp), 0, PGSIZE);
                pp = pp->pp_link;
        }
        page_free_list = pp;
//        print("Starting address for %d pages is %x ", pages_required, start_address);
        return start_address;
}

