/*
 File: ContFramePool.C
 
 Author:
 Date  : 
 
 */

/*--------------------------------------------------------------------------*/
/* 
 POSSIBLE IMPLEMENTATION
 -----------------------

 The class SimpleFramePool in file "simple_frame_pool.H/C" describes an
 incomplete vanilla implementation of a frame pool that allocates 
 *single* frames at a time. Because it does allocate one frame at a time, 
 it does not guarantee that a sequence of frames is allocated contiguously.
 This can cause problems.
 
 The class ContFramePool has the ability to allocate either single frames,
 or sequences of contiguous frames. This affects how we manage the
 free frames. In SimpleFramePool it is sufficient to maintain the free 
 frames.
 In ContFramePool we need to maintain free *sequences* of frames.
 
 This can be done in many ways, ranging from extensions to bitmaps to 
 free-lists of frames etc.
 
 IMPLEMENTATION:
 
 One simple way to manage sequences of free frames is to add a minor
 extension to the bitmap idea of SimpleFramePool: Instead of maintaining
 whether a frame is FREE or ALLOCATED, which requires one bit per frame, 
 we maintain whether the frame is FREE, or ALLOCATED, or HEAD-OF-SEQUENCE.
 The meaning of FREE is the same as in SimpleFramePool. 
 If a frame is marked as HEAD-OF-SEQUENCE, this means that it is allocated
 and that it is the first such frame in a sequence of frames. Allocated
 frames that are not first in a sequence are marked as ALLOCATED.
 
 NOTE: If we use this scheme to allocate only single frames, then all 
 frames are marked as either FREE or HEAD-OF-SEQUENCE.
 
 NOTE: In SimpleFramePool we needed only one bit to store the state of 
 each frame. Now we need two bits. In a first implementation you can choose
 to use one char per frame. This will allow you to check for a given status
 without having to do bit manipulations. Once you get this to work, 
 revisit the implementation and change it to using two bits. You will get 
 an efficiency penalty if you use one char (i.e., 8 bits) per frame when
 two bits do the trick.
 
 DETAILED IMPLEMENTATION:
 
 How can we use the HEAD-OF-SEQUENCE state to implement a contiguous
 allocator? Let's look a the individual functions:
 
 Constructor: Initialize all frames to FREE, except for any frames that you 
 need for the management of the frame pool, if any.
 
 get_frames(_n_frames): Traverse the "bitmap" of states and look for a 
 sequence of at least _n_frames entries that are FREE. If you find one, 
 mark the first one as HEAD-OF-SEQUENCE and the remaining _n_frames-1 as
 ALLOCATED.

 release_frames(_first_frame_no): Check whether the first frame is marked as
 HEAD-OF-SEQUENCE. If not, something went wrong. If it is, mark it as FREE.
 Traverse the subsequent frames until you reach one that is FREE or 
 HEAD-OF-SEQUENCE. Until then, mark the frames that you traverse as FREE.
 
 mark_inaccessible(_base_frame_no, _n_frames): This is no different than
 get_frames, without having to search for the free sequence. You tell the
 allocator exactly which frame to mark as HEAD-OF-SEQUENCE and how many
 frames after that to mark as ALLOCATED.
 
 needed_info_frames(_n_frames): This depends on how many bits you need 
 to store the state of each frame. If you use a char to represent the state
 of a frame, then you need one info frame for each FRAME_SIZE frames.
 
 A WORD ABOUT RELEASE_FRAMES():
 
 When we releae a frame, we only know its frame number. At the time
 of a frame's release, we don't know necessarily which pool it came
 from. Therefore, the function "release_frame" is static, i.e., 
 not associated with a particular frame pool.
 
 This problem is related to the lack of a so-called "placement delete" in
 C++. For a discussion of this see Stroustrup's FAQ:
 http://www.stroustrup.com/bs_faq2.html#placement-delete
 
 */
/*--------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "cont_frame_pool.H"
#include "console.H"
#include "utils.H"
#include "assert.H"

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

ContFramePool* ContFramePool::head;
ContFramePool* ContFramePool::tail;

/*--------------------------------------------------------------------------*/
/* METHODS FOR CLASS   C o n t F r a m e P o o l */
/*--------------------------------------------------------------------------*/
ContFramePool::FrameState ContFramePool::get_state(unsigned long _frame_no) {
	    unsigned int bitmap_index = _frame_no / 4;
	        unsigned char mask = 0x03 << 2*(_frame_no % 4);

		    if ((bitmap[bitmap_index] & mask) == mask) 
			return FrameState::Used;
		    else if ((bitmap[bitmap_index] & mask) == 0x00)
			return FrameState::Free;
		    else
			return FrameState::HoS;
}

void ContFramePool::set_state(unsigned long _frame_no, FrameState _state) {
    unsigned int bitmap_index = _frame_no / 4;
    unsigned char mask = 0x03 << 2*(_frame_no % 4);
    unsigned char mask_1 = 0x01 << 2*(_frame_no % 4);

    switch(_state) {
      case FrameState::Used:
      bitmap[bitmap_index] |= mask;
      break;
      case FrameState::Free:
      bitmap[bitmap_index] &= ~mask;
      break;
      case FrameState::HoS:
      bitmap[bitmap_index] &= ~mask;
      bitmap[bitmap_index] |= mask_1; 	
      break;
    }
}


ContFramePool::ContFramePool(unsigned long _base_frame_no,
                             unsigned long _n_frames,
                             unsigned long _info_frame_no)
{
    base_frame_no = _base_frame_no;
    n_frames = _n_frames;
    info_frame_no = _info_frame_no;
    nFreeFrames = _n_frames;

    // Bitmap must fit in a single frame!
    // 2 bits for storing state, there are three states  
    assert(_n_frames <= FRAME_SIZE * 4);


    // If _info_frame_no is zero then we keep management info in the first
    //frame, else we use the provided frame to keep management info
    //states: 
    //00->FREE
    //11->ALLOCATED
    //01->HEAD-OF-SEQUENCE
    if(info_frame_no == 0) {
        bitmap = (unsigned char *) (base_frame_no * FRAME_SIZE);
    } else {
        bitmap = (unsigned char *) (info_frame_no * FRAME_SIZE);
    }

    // Everything ok. Proceed to mark all frame as free.
    for(int fno = 0; fno < n_frames; fno++) {
	    set_state(fno, FrameState::Free);
    }

    // Mark the first frame as if it is being used
    if(info_frame_no == 0) {
        set_state(0, FrameState::Used);
        nFreeFrames--;
    }
	
    if (ContFramePool::head == NULL) {
	ContFramePool::head = this;
    	ContFramePool::tail = this;
    } else {
    ContFramePool::tail->next = this;
    ContFramePool::tail = this;
    }
    ContFramePool::tail->next = NULL;
   	    
    
    //Console::puts("Frame Pool initialized\n");

}

unsigned long ContFramePool::get_frames(unsigned int _n_frames)
{   int count, loc;
    unsigned char mask, mask1;
    count = 0;	    
    // Any frames left to allocate?
    if(_n_frames > nFreeFrames)
  	  Console::puts("get_frames Failed! number of free frames less than required \n");	    
    else if (_n_frames>0) {
	//go over all bitmap entries one by one
	for(int fno=0;fno<n_frames;fno++) {
		if(get_state(fno) == FrameState::Free) {
			if(count == 0) {
				loc = fno;
			}		
			count++;
		}
		else {
			count = 0;
		}
                if (count >= _n_frames)
                        break;


	}//for	

    }//else
    
    if ((count == _n_frames) && (_n_frames>0)) {
	nFreeFrames = nFreeFrames - _n_frames;
	set_state(loc, FrameState::HoS);
    	for(int fno=loc+1;fno<loc+_n_frames;fno++)
		set_state(fno, FrameState::Used);	
    	return base_frame_no + loc;
	}
    else if (_n_frames == 0) {
	    Console::puts("get_frames Failed! n_frames should be greater than 0 \n");
	    return 0;
    } else {
	Console::puts("\n");
	Console::puts("get_frames Failed! unable to find contagious frames of the required size\n");
	return 0;
	}
}//func


void ContFramePool::mark_inaccessible(unsigned long _base_frame_no,
                                      unsigned long _n_frames)
{	int f_start;	

	if ((_base_frame_no < base_frame_no) || (_base_frame_no + _n_frames  > n_frames + base_frame_no))
	       Console::puts("Failed! Range out of index\n");
	else {
		f_start = _base_frame_no - base_frame_no;
		set_state(f_start, FrameState::HoS);nFreeFrames--;
		for(int fno=f_start+1;fno<(f_start+_n_frames);fno++) {
                        set_state(fno, FrameState::Used);
			nFreeFrames--;
                        }
		//Console::puts("Done! marked inaccessible\n");
	}
}

void ContFramePool::release_frames(unsigned long _first_frame_no)
{	ContFramePool* current_pool;
	int found = 0;
	//find the frame pool this frame no belongs to 
	
	for(current_pool = ContFramePool::head; current_pool->next != NULL; current_pool = current_pool->next) {
		if((current_pool->base_frame_no <= _first_frame_no) && ((current_pool->base_frame_no + current_pool->n_frames-1) >=  _first_frame_no)) {
			found = 1;
			break;
		}	
	}
	if (found == 0) {
	current_pool = ContFramePool::tail;       	
	if((current_pool->base_frame_no <= _first_frame_no) && ((current_pool->base_frame_no + current_pool->n_frames-1) >=  _first_frame_no)) {
		found = 1;
	}
	}

	if (found != 1 ) {
		Console::puts("release_frames Failed! first frame not found, fno = ");Console::puti(_first_frame_no);Console::puts("\n");
	} else if (current_pool->get_state(_first_frame_no - current_pool->base_frame_no) != FrameState::HoS)
		Console::puts("release_frames Failed! first frame not head of sequence \n");
	else {	
		current_pool->set_state(_first_frame_no - current_pool->base_frame_no, FrameState::Free);current_pool->nFreeFrames++;
		for(int fno=(_first_frame_no - current_pool->base_frame_no + 1);fno < current_pool->n_frames;fno++) {
			if (current_pool->get_state(fno) == FrameState::Used) {
				current_pool->set_state(fno, FrameState::Free);
				current_pool->nFreeFrames++;
			}
			else if(current_pool->get_state(fno) == FrameState::Free || current_pool->get_state(fno) == FrameState::HoS) 
				break;
		}
		//Console::puts("frames released \n");		
	}//else

}//func



unsigned long ContFramePool::needed_info_frames(unsigned long _n_frames)

{	//for each frame we need two bits, assuming the FRAME_SIZE in bytes 	 
	return ((_n_frames*2)/(8*FRAME_SIZE)) + (((_n_frames*2)%(8*FRAME_SIZE)>0)?1:0);
}
