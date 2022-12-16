/*
 File: vm_pool.C
 
 Author:
 Date  :
 
 */

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "vm_pool.H"
#include "console.H"
#include "utils.H"
#include "assert.H"
#include "simple_keyboard.H"

/*--------------------------------------------------------------------------*/
/* DATA STRUCTURES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* CONSTANTS */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* FORWARDS */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* METHODS FOR CLASS   V M P o o l */
/*--------------------------------------------------------------------------*/

VMPool::VMPool(unsigned long  _base_address,
               unsigned long  _size,
               ContFramePool *_frame_pool,
               PageTable     *_page_table) {

	   base_addr = _base_address;
	   size = _size;
	   frame_pool = _frame_pool;
	   page_table = _page_table;
	   VMPool_list_next = NULL;     
	   region_count= 0;
   
	   page_table->register_pool(this);
   	   region_list = (struct region *)(base_addr);

    	   Console::puts("Constructed VMPool object.\n");
}

unsigned long VMPool::allocate(unsigned long _size) {
	
	    if (size == 0){
	        Console::puts("size should be greater than zero");
	        return 0;
	    }
	
	    //allocate memory in pages
	    unsigned long frames = (_size / (Machine::PAGE_SIZE)) + ((_size % (Machine::PAGE_SIZE))>0?1:0 ) ;
	
	    if (region_count == 0) { // first frame is used to store the list of allocator maintaining this region
	        region_list[region_count].base_address  = base_addr + Machine::PAGE_SIZE ;
	        region_list[region_count].size = frames*(Machine::PAGE_SIZE) ;
	        region_count++;
	    } else {
	        region_list[region_count].base_address = region_list[region_count - 1].base_address + region_list[region_count - 1].size ;
	    	region_list[region_count].size = frames*(Machine::PAGE_SIZE);
	    	region_count++;
            }
            Console::puts("Allocated region of memory.\n");
   	    return region_list[region_count-1].base_address;	    
}

void VMPool::release(unsigned long _start_address) {
     int found = -1;
     unsigned int page_count;
     unsigned long temp_addr;     
     for (int i = 0; i < region_count; i++) {
        if (region_list[i].base_address == _start_address) {
            found = i;
            break;
        }
    }
     
    if(found == -1 ) {
	   Console::puts("region of memory not found.\n"); 
    } else {
    
    page_count = ( (region_list[found].size) / (Machine::PAGE_SIZE) ) ;
   
    temp_addr = _start_address;
    
    for (int i = 0 ; i < page_count ;i++) {

        page_table->free_page(temp_addr);
        temp_addr += Machine::PAGE_SIZE;
    

    }
    // left shift the elements after that
    for (int i = found; i < region_count - 1; i++) {
        region_list[i] = region_list[i+1];
    }
    region_count--;
	}
    //flush TLB
    page_table->load();	
    
    Console::puts("Released region of memory.\n");
    
}

bool VMPool::is_legitimate(unsigned long _address) {
    if ((_address <(base_addr + size) ) && (_address >= base_addr))
            return true;
	 
    return false;	
}

