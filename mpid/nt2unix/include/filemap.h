#ifndef FILEMAP_HEADER
#define FILEMAP_HEADER

#define SYSVSEGMENT 0
#define FILESEGMENT 1

#include <vector>

// We manage a list (a vector) of created file mappings.
struct FileMapping {
  LPVOID *BaseAddresses;		// base addresse of mappings
  DWORD dwNumberOfBytesToMap;	// mapping size
  HANDLE hFileMappingObject;	// file handle
  char FileName[MAX_PATH]; 	// file name
  DWORD refcnt;			// number of references to the mapping
  DWORD type;
  BOOL Closed;
  
};
//static vector<FileMapping> FileMappings;
extern std::vector<FileMapping> FileMappings;

#ifndef MAP_FAILED
#define MAP_FAILED -1
#endif 

#ifdef sparc
#define PATH_PREFIX "/global/tmp"
#else
#define PATH_PREFIX 0
#endif
#endif //FILEMAP_HEADER
