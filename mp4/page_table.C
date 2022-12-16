#include "assert.H"
#include "exceptions.H"
#include "console.H"
#include "paging_low.H"
#include "page_table.H"

PageTable * PageTable::current_page_table = NULL;
unsigned int PageTable::paging_enabled = 0;
ContFramePool * PageTable::kernel_mem_pool = NULL;
ContFramePool * PageTable::process_mem_pool = NULL;
unsigned long PageTable::shared_size = 0;
VMPool * PageTable::VMPool_listhead = NULL;
void PageTable::init_paging(ContFramePool * _kernel_mem_pool,
                            ContFramePool * _process_mem_pool,
                            const unsigned long _shared_size)
{

   PageTable::kernel_mem_pool = _kernel_mem_pool;
   PageTable::process_mem_pool = _process_mem_pool;
   PageTable::shared_size = _shared_size;
   Console::puts("Initialized Paging System\n");
}

PageTable::PageTable()
{  unsigned long addr=0;
   unsigned int no_frames;	
   //changed to process mem pools
   page_directory = (unsigned long*)(process_mem_pool->get_frames(1)*PAGE_SIZE);
   unsigned long *page_table = (unsigned long*)(process_mem_pool->get_frames(1)*PAGE_SIZE);

   no_frames = PageTable::shared_size/PAGE_SIZE;
   //Console::puti(no_frames);Console::puts("\n"); 
   //fill page table
   for(int i=0;i<no_frames;i++) {
	   page_table[i] = addr|3;// setting R/W and present bit
	   addr+=PAGE_SIZE;
   }
   //fill page directory
   page_directory[0] = (unsigned long)page_table|3;
   page_directory[1023] = (unsigned long)page_directory|3;//recursive page-table lookup
   for(int i=1;i<1023;i++) {
	   page_directory[i] = 0|2;// setting R/W bit, supervisor level

   }
   Console::puts("Constructed Page Table object\n");
}


void PageTable::load()
{
   current_page_table = this;
   Console::puts("Loaded page table\n");
   //added this here so that this function can also be used for flushing TLB
   write_cr3((unsigned long)current_page_table->page_directory);
}

void PageTable::enable_paging()
{
   write_cr3((unsigned long)current_page_table->page_directory);
   paging_enabled = 1;
   write_cr0(read_cr0()|0x80000000);
   Console::puts("Enabled paging\n");
}

void PageTable::handle_fault(REGS * _r)
{
   unsigned long err_code = _r->err_code;
   unsigned long addr = read_cr2();	
   unsigned long *page_dir = (unsigned long *) 0xfffff000;
   unsigned long page_dir_address = addr>>22;
   unsigned long page_tab_address = (addr>>12) & 0x000003ff;
   unsigned long *npage_table;
   unsigned int is_legim = 0;
   VMPool *ptr = PageTable::VMPool_listhead;
   for(;ptr!=NULL;ptr=ptr->VMPool_list_next)
      {
        if(ptr->is_legitimate(addr) == true)

         {
           is_legim = 1;
           break;
         }
      }
	
   if(is_legim == 0 && ptr!= NULL)
    {
      Console::puts("address is invalid \n");
      assert(false);    
    }

   if((err_code & 0x01) == 0) {
	//present bit is zero
	//------PD----|-----PT-----|--PAGE OFFSET--|
	//0000 0000 00 00 0000 0000  0000 0000 0000
	   if(page_dir[page_dir_address] & 0x01 == 1) {
	           //present in page directory, so fault in page table
		   npage_table = (unsigned long*)((page_dir_address<<12) | 0xffc00000 );
		   npage_table[page_tab_address] = PageTable::process_mem_pool->get_frames(1)*PAGE_SIZE | 3;// setting R/W and present bit
	   } else {

   		   //not in page directory
		page_dir[page_dir_address] = PageTable::process_mem_pool->get_frames(1)*PAGE_SIZE | 3;// setting R/W and present bit 
		npage_table = (unsigned long*)((page_dir_address<<12) |  0xffc00000 );

		//fill page table
		for(int i=0;i<1024;i++)
			npage_table[i] = 4; //setting page as user page

		npage_table[page_tab_address] = process_mem_pool->get_frames(1)*PAGE_SIZE | 3;// setting R/W and present bit
	   }
   }
  Console::puts("handled page fault\n");
}

void PageTable::register_pool(VMPool * _vm_pool)
{
 //list of pools is a linked list
 
  if( PageTable::VMPool_listhead == NULL ) 
 {
        PageTable::VMPool_listhead= _vm_pool;
    }
    else 
 {
  VMPool *ptr = PageTable::VMPool_listhead;
        for(;ptr->VMPool_list_next!=NULL;ptr= ptr->VMPool_list_next);
        ptr->VMPool_list_next= _vm_pool;
    }
    
 Console::puts("VM pool registered\n");  
}

void PageTable::free_page(unsigned long _page_no) 
{

    /*From the address, getting the page number and hence the frame number to release.*/
    unsigned long PDE = _page_no >> 22; 
    unsigned long PTE = (_page_no >> 12) & 0x000003ff ;

    unsigned long *page_table_entry= (unsigned long *) ( (0xffc00000) | (PDE << 12) );
    unsigned long frame_no = (page_table_entry[PTE])/ Machine::PAGE_SIZE;
	
    process_mem_pool->release_frames(frame_no);
    page_table_entry[PTE] = 2; // R/W, INVALID
    Console::puts("removed page\n");
	
    //flushing TLB
    load();
}
