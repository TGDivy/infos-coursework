/*
 * TAR File-system Driver
 * SKELETON IMPLEMENTATION -- TO BE FILLED IN FOR TASK (4)
 */

/*
 * STUDENT NUMBER: s1885517
 */
#include "tarfs.h"
#include <infos/kernel/log.h>

using namespace infos::fs;
using namespace infos::drivers;
using namespace infos::drivers::block;
using namespace infos::kernel;
using namespace infos::util;
using namespace tarfs;

/**
 * TAR files contain header data encoded as octal values in ASCII.  This function
 * converts this terrible representation into a real unsigned integer.
 *
 * You DO NOT need to modify this function.
 *
 * @param data The (null-terminated) ASCII data containing an octal number.
 * @return Returns an unsigned integer number, corresponding to the input data.
 */
static inline unsigned int octal2ui(const char *data)
{
	// Current working value.
	unsigned int value = 0;

	// Length of the input data.
	int len = strlen(data);

	// Starting at i = 1, with a factor of one.
	int i = 1, factor = 1;
	while (i < len) {
		// Extract the current character we're working on (backwards from the end).
		char ch = data[len - i];

		// Add the value of the character, multipled by the factor, to
		// the working value.
		value += factor * (ch - '0');
		
		// Increment the factor by multiplying it by eight.
		factor *= 8;
		
		// Increment the current character position.
		i++;
	}

	// Return the current working value.
	return value;
}

// The structure that represents the header block present in
// TAR files.  A header block occurs before every file, this
// this structure must EXACTLY match the layout as described
// in the TAR file format description.
namespace tarfs {
	struct posix_header {
	 	char name[100];               /*   0 */
		char mode[8];                 /* 100 */
		char uid[8];                  /* 108 */
		char gid[8];                  /* 116 */
		char size[12];                /* 124 */
		char mtime[12];               /* 136 */
		char chksum[8];               /* 148 */
		char typeflag;                /* 156 */
		char linkname[100];           /* 157 */
		char magic[6];                /* 257 */
		char version[2];              /* 263 */
		char uname[32];               /* 265 */
		char gname[32];               /* 297 */
		char devmajor[8];             /* 329 */
		char devminor[8];             /* 337 */
		char prefix[155];             /* 345 */
                                /* 500 */
	} __packed;
}

/**
 * Reads the contents of the file into the buffer, from the specified file offset.
 * @param buffer The buffer to read the data into.
 * @param size The size of the buffer, and hence the number of bytes to read.
 * @param off The offset within the file.
 * @return Returns the number of bytes read into the buffer.
 */
int TarFSFile::pread(void* buffer, size_t size, off_t off)
{
	// buffer is a pointer to the buffer that should receive the data.
	// size is the amount of data to read from the file.
	// off is the zero-based offset within the file to start reading from.

	// Nothing present to read.	
	if (off >= this->size()) return 0;

	infos::drivers::block::BlockDevice& block_device = _owner.block_device();

	// Read the entire file in file_buffer.
	int block_size = block_device.block_size();
	int file_size = this->size(); // in bytes.
	int blocks_to_read = ((file_size-1) / block_size) + 1;
	uint8_t *file_buffer = new uint8_t[block_size * blocks_to_read];

	block_device.read_blocks(file_buffer, _file_start_block, blocks_to_read);

	
	//Find out how many bytes we can read in.
	int bytes_that_can_be_read;
	if(file_size-off<size)
		bytes_that_can_be_read = file_size-off;
	else
		bytes_that_can_be_read = size;

	//Read the requested information from the file into the buffer.
	memcpy(buffer, file_buffer+off, bytes_that_can_be_read);
	delete file_buffer;

	return bytes_that_can_be_read;
}

/**
 * Reads all the file headers in the TAR file, and builds an in-memory
 * representation.
 * @return Returns the root TarFSNode that corresponds to the TAR file structure.
 */
TarFSNode* TarFS::build_tree()
{
	// Create the root node.
	TarFSNode *root = new TarFSNode(NULL, "", *this);
	// You must read the TAR file, and build a tree of TarFSNodes that represents each file present in the archive.
	
	size_t nr_blocks = block_device().block_count();
	size_t block_size = block_device().block_size();
	syslog.messagef(LogLevel::DEBUG, "Block Device nr-blocks=%lu", nr_blocks);

	// Reading header.
	struct posix_header *hdr = (struct posix_header *) new char[block_size];
	uint8_t *fileheader_buffer = new uint8_t[block_size];

	// off = offset the position of the current header
	size_t off = 0;
	while (off< nr_blocks){
		syslog.messagef(LogLevel::DEBUG, "offset is %d",  off);

		block_device().read_blocks(hdr, off, 1);
		block_device().read_blocks(fileheader_buffer, off, 1);

		if(is_zero_block(fileheader_buffer)){ //if a zero block skip.
			off+=1;
			continue;
		}

		// Find the parent and create the child.
		List<String> sname = ((String)(hdr->name)).split('/', true);		
		String name;
		TarFSNode *parent = root;
		while(sname.count()>1){
			name = sname.pop();
			parent =  parent->get_child(name);
		}
		name = sname.pop(); // Name of the new child.
		syslog.messagef(LogLevel::DEBUG, "name is %s", name.c_str());

		// Create the child and set it's properties.
		TarFSNode *child = new TarFSNode(parent, name, *this);
		child->set_block_offset(off);
		size_t filesize = octal2ui(hdr->size); 
		child->size(filesize);
		// Add child to parent.
		parent->add_child(name, child);

		// calculate next block position.
		off += filesize==0 ? 1 : ((filesize-1)/block_size) +2;
	}
	syslog.messagef(LogLevel::DEBUG, "Loop COMPLETE");
	
	return root;
}

/**
 * Returns the size of this TarFS File
 */
unsigned int TarFSFile::size() const
{
	syslog.messagef(LogLevel::DEBUG, "Side %d", octal2ui(_hdr->size));
	return octal2ui(_hdr->size);
}

/* --- YOU DO NOT NEED TO CHANGE ANYTHING BELOW THIS LINE --- */

/**
 * Mounts a TARFS filesystem, by pre-building the file system tree in memory.
 * @return Returns the root node of the TARFS filesystem.
 */
PFSNode *TarFS::mount()
{
	// If the root node has not been generated, then build it.
	if (_root_node == NULL) {
		_root_node = build_tree();
	}

	// Return the root node.
	return _root_node;
}

/**
 * Constructs a TarFS File object, given the owning file system and the block
 */
TarFSFile::TarFSFile(TarFS& owner, unsigned int file_header_block)
: _hdr(NULL),
_owner(owner),
_file_start_block(file_header_block),
_cur_pos(0)
{
	// Allocate storage for the header.
	_hdr = (struct posix_header *) new char[_owner.block_device().block_size()];
	
	// Read the header block into the header structure.
	_owner.block_device().read_blocks(_hdr, _file_start_block, 1);
	
	// Increment the starting block for file data.
	_file_start_block++;
}

TarFSFile::~TarFSFile()
{
	// Delete the header structure that was allocated in the constructor.
	delete _hdr;
}

/**
 * Releases any resources associated with this file.
 */
void TarFSFile::close()
{
	// Nothing to release.
}

/**
 * Reads the contents of the file into the buffer, from the current file offset.
 * The current file offset is advanced by the number of bytes read.
 * @param buffer The buffer to read the data into.
 * @param size The size of the buffer, and hence the number of bytes to read.
 * @return Returns the number of bytes read into the buffer.
 */
int TarFSFile::read(void* buffer, size_t size)
{
	// Read can be seen as a special case of pread, that uses an internal
	// current position indicator, so just delegate actual processing to
	// pread, and update internal state accordingly.

	// Perform the read from the current file position.
	int rc = pread(buffer, size, _cur_pos);

	// Increment the current file position by the number of bytes that was read.
	// The number of bytes actually read may be less than 'size', so it's important
	// we only advance the current position by the actual number of bytes read.
	_cur_pos += rc;

	// Return the number of bytes read.
	return rc;
}

/**
 * Moves the current file pointer, based on the input arguments.
 * @param offset The offset to move the file pointer either 'to' or 'by', depending
 * on the value of type.
 * @param type The type of movement to make.  An absolute movement moves the
 * current file pointer directly to 'offset'.  A relative movement increments
 * the file pointer by 'offset' amount.
 */
void TarFSFile::seek(off_t offset, SeekType type)
{
	// If this is an absolute seek, then set the current file position
	// to the given offset (subject to the file size).  There should
	// probably be a way to return an error if the offset was out of bounds.
	if (type == File::SeekAbsolute) {
		_cur_pos = offset;
	} else if (type == File::SeekRelative) {
		_cur_pos += offset;
	}
	if (_cur_pos >= size()) {
		_cur_pos = size() - 1;
	}
}

TarFSNode::TarFSNode(TarFSNode *parent, const String& name, TarFS& owner) : PFSNode(parent, owner), _name(name), _size(0), _has_block_offset(false), _block_offset(0)
{
}

TarFSNode::~TarFSNode()
{
}

/**
 * Opens this node for file operations.
 * @return 
 */
File* TarFSNode::open()
{
	// This is only a file if it has been associated with a block offset.
	if (!_has_block_offset) {
		return NULL;
	}

	// Create a new file object, with a header from this node's block offset.
	return new TarFSFile((TarFS&) owner(), _block_offset);
}

/**
 * Opens this node for directory operations.
 * @return 
 */
Directory* TarFSNode::opendir()
{
	return new TarFSDirectory(*this);
}

/**
 * Attempts to retrieve a child node of the given name.
 * @param name
 * @return 
 */
TarFSNode* TarFSNode::get_child(const String& name)
{
	TarFSNode *child;

	// Try to find the given child node in the children map, and return
	// NULL if it wasn't found.
	if (!_children.try_get_value(name.get_hash(), child)) {
		return NULL;
	}

	return child;
}

/**
 * Creates a subdirectory in this node.  This is a read-only file-system,
 * and so this routine does not need to be implemented.
 * @param name
 * @return 
 */
PFSNode* TarFSNode::mkdir(const String& name)
{
	// DO NOT IMPLEMENT
	return NULL;
}

/**
 * A helper routine that updates this node with the offset of the block
 * that contains the header of the file that this node represents.
 * @param offset The block offset that corresponds to this node.
 */
void TarFSNode::set_block_offset(unsigned int offset)
{
	_has_block_offset = true;
	_block_offset = offset;
}

/**
 * A helper routine that adds a child node to the internal children
 * map of this node.
 * @param name The name of the child node.
 * @param child The actual child node.
 */
void TarFSNode::add_child(const String& name, TarFSNode *child)
{
	_children.add(name.get_hash(), child);
}

TarFSDirectory::TarFSDirectory(TarFSNode& node) : _entries(NULL), _nr_entries(0), _cur_entry(0)
{
	_nr_entries = node.children().count();
	_entries = new DirectoryEntry[_nr_entries];

	int i = 0;
	for (const auto& child : node.children()) {
		_entries[i].name = child.value->name();
		_entries[i++].size = child.value->size();
	}
}

TarFSDirectory::~TarFSDirectory()
{
	delete _entries;
}

bool TarFSDirectory::read_entry(infos::fs::DirectoryEntry& entry)
{
	if (_cur_entry < _nr_entries) {
		entry = _entries[_cur_entry++];
		return true;
	} else {
		return false;
	}
}

void TarFSDirectory::close()
{

}

static Filesystem *tarfs_create(VirtualFilesystem& vfs, Device *dev)
{
	if (!dev->device_class().is(BlockDevice::BlockDeviceClass)) return NULL;
	return new TarFS((BlockDevice &) * dev);
}

RegisterFilesystem(tarfs, tarfs_create);
