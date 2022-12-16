/*
     File        : file.C

     Author      : Riccardo Bettati
     Modified    : 2021/11/28

     Description : Implementation of simple File class, with support for
                   sequential read/write operations.
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
#include "file.H"

/*--------------------------------------------------------------------------*/
/* CONSTRUCTOR/DESTRUCTOR */
/*--------------------------------------------------------------------------*/

File::File(FileSystem *_fs, int _id) {
    Console::puts("Opening file.\n");
         fd = _id;
         file_system = _fs;
         position = 0;
         block_no = file_system->get_block_no(fd);
         file_system->disk->read(block_no, (unsigned char *)block_cache);
}

File::~File() {
    Console::puts("Closing file.\n");
    /* Make sure that you write any cached data to disk. */
    /* Also make sure that the inode in the inode list is updated. */
    file_system->disk->write(block_no, (unsigned char *)block_cache);
}

/*--------------------------------------------------------------------------*/
/* FILE FUNCTIONS */
/*--------------------------------------------------------------------------*/

int File::Read(unsigned int _n, char *_buf) {
    Console::puts("reading from file\n");
         int read = 0;
         int bytes_to_read = _n;
         if (bytes_to_read>512)
                Console::puts("Error: bytes to read greater than 512\n");


         while (!EoF() && (bytes_to_read > 0)) {
         _buf[read] = block_cache[position];
         bytes_to_read--;
         read++;
         position++;
         if (position>=512)
                position = 0;
         }

         return read;

}

int File::Write(unsigned int _n, const char *_buf) {
    Console::puts("writing to file\n");
         int write = 0;
         int bytes_to_write = _n;


         while (bytes_to_write > 0 ) {
         block_cache[position] = _buf[write];
         write++;
         position++;
         bytes_to_write--;
         }
         return write;
}

void File::Reset() {
    Console::puts("resetting file\n");

    position = 0;
}

bool File::EoF() {
    Console::puts("checking for EoF\n");

    if (position >= 512) {
        return true;
    }
    return false;
}
