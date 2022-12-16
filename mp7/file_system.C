/*
     File        : file_system.C

     Author      : Riccardo Bettati
     Modified    : 2021/11/28

     Description : Implementation of simple File System class.
                   Has support for numerical file identifiers.
 */

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

    /* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "assert.H"
#include "console.H"
#include "file_system.H"

/*--------------------------------------------------------------------------*/
/* CLASS Inode */
/*--------------------------------------------------------------------------*/

/* You may need to add a few functions, for example to help read and store 
   inodes from and to disk. */

/*--------------------------------------------------------------------------*/
/* CLASS FileSystem */
/*--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*/
/* CONSTRUCTOR */
/*--------------------------------------------------------------------------*/

FileSystem::FileSystem() {
    Console::puts("In file system constructor.\n");
    FileSystem::disk = NULL;
    FileSystem::inodes = new Inode[MAX_INODES];
    FileSystem::free_blocks = (unsigned char*)(512);
    FileSystem::size = 0;
    for(int i=0;i<MAX_INODES;i++) {
	inodes[i].id = -1;
    }
    for(int i=0;i<512;i++) {
	free_blocks[i] = 0;
    }
	free_blocks[0] = 1;
	free_blocks[1] = 1;
}

FileSystem::~FileSystem() {
    Console::puts("unmounting file system\n");
    /* Make sure that the inode list and the free list are saved. */
    save_FL(disk, (unsigned char*)free_blocks);
    save_IL(disk, inodes);

}


/*--------------------------------------------------------------------------*/
/* FILE SYSTEM FUNCTIONS */
/*--------------------------------------------------------------------------*/


bool FileSystem::Mount(SimpleDisk * _disk) {
    Console::puts("mounting file system from disk\n");

    /* Here you read the inode list and the free list into memory */
     if (disk == NULL) {
        disk = _disk;
	load_FL(disk);
        load_IL(disk);
	return true;
     }
     Console::puts("already a disk associated with this file system\n");
     return false;
}

bool FileSystem::Format(SimpleDisk * _disk, unsigned int _size) { // static!
    Console::puts("formatting disk\n");
    /* Here you populate the disk with an initialized (probably empty) inode list
       and a free list. Make sure that blocks used for the inodes and for the free list
       are marked as used, otherwise they may get overwritten. */
        unsigned int no_blocks = (_size/SimpleDisk::BLOCK_SIZE);
	unsigned char FB[512];
	Inode *_inodes;
	_inodes = new Inode[MAX_INODES];

        for(int i = 0;i<no_blocks;i++) {
                FB[i] = 0;
                _inodes[i].id = -1;
        }
	FB[0] = 1;
        FB[1] = 1;
        save_FL(_disk, (unsigned char*)FB);
        save_IL(_disk, _inodes);

    return true;
}

Inode * FileSystem::LookupFile(int _file_id) {
    Console::puts("looking up file with id = "); Console::puti(_file_id); Console::puts("\n");
    /* Here you go through the inode list to find the file. */
    Inode *inode_temp;

    for (int i=0; i<MAX_INODES; i++) {
        if(_file_id == inodes[i].id) {
		inode_temp = &inodes[i];
                return inode_temp;
        }
    }
    return NULL;

}

bool FileSystem::CreateFile(int _file_id) {
    Console::puts("creating file with id:"); Console::puti(_file_id); Console::puts("\n");
    /* Here you check if the file exists already. If so, throw an error.
       Then get yourself a free inode and initialize all the data needed for the
       new file. After this function there will be a new file on disk. */
    int blockno = -1;
    for (int i=0; i<MAX_INODES; i++) {
        if(_file_id == inodes[i].id) {
                Console::puts("Error: file already exists\n");
                return false;
        }
    }

    for (int i=2; i<512; i++) {

        if(free_blocks[i]==0) {
                blockno = i;
                break;
        }
    }
    if(blockno == -1) {
            Console::puts("empty block not found\n");
    }
    for (int i=0; i<MAX_INODES; i++) {
        if(inodes[i].id == -1) {
                inodes[i].id = _file_id;
                inodes[i].block_no = blockno;
		free_blocks[blockno] = 1;
                break;
        }
    }

    return true;

}

bool FileSystem::DeleteFile(int _file_id) {
    Console::puts("deleting file with id:"); Console::puti(_file_id); Console::puts("\n");
    /* First, check if the file exists. If not, throw an error. 
       Then free all blocks that belong to the file and delete/invalidate 
       (depending on your implementation of the inode list) the inode. */
    int blockno = -1;
    int found = -1;
    for (int i=0; i<MAX_INODES; i++) {
        if(_file_id == inodes[i].id) {
                found  = 1;
        }
    }
    if (found == -1) {
        Console::puts("Error: file dies not exist\n");
        return false;
    }

    for (int i=0; i<MAX_INODES; i++) {
        if(inodes[i].id == _file_id) {
                inodes[i].id = -1;
		int bn = inodes[i].block_no;
		free_blocks[bn] = 0;
                break;
        }
    }

    return true;
}

void FileSystem::load_FL(SimpleDisk * _disk) {
	unsigned char buf[512];
	_disk->read(1, buf);
	memcpy(free_blocks, buf, 512);
}

void FileSystem::load_IL(SimpleDisk * _disk) {
        unsigned char buf[512];
        _disk->read(0, buf);
        memcpy(inodes, buf, 512);
}


void FileSystem::save_FL(SimpleDisk * _disk, unsigned char *_buf) {
        _disk->write(1, _buf);
}

void FileSystem::save_IL(SimpleDisk * _disk, Inode *_inode) {
        unsigned char buf[512];
	memcpy(buf, _inode, 512);
        _disk->write(0, buf);
}

int FileSystem::get_block_no(long _file_id) {
    for (int i=0; i<MAX_INODES; i++) {
        if(inodes[i].id == _file_id) {
                return inodes[i].id;
        }
    }
        Console::puts("Error: file not found\n");
        return -1;
}

