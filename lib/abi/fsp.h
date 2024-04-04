unsigned int fsp_list(unsigned int target, unsigned int id, struct record records[8]);
unsigned int fsp_list2(unsigned int descriptor, unsigned int id, struct record records[8]);
void fsp_listresponse(unsigned int source, unsigned int session, unsigned int nrecords, struct record *records);
unsigned int fsp_read(unsigned int target, unsigned int id, unsigned int count, unsigned int offset, unsigned int ocount, void *obuffer);
unsigned int fsp_read2(unsigned int descriptor, unsigned int id, unsigned int count, unsigned int offset, unsigned int ocount, void *obuffer);
void fsp_readresponse(unsigned int source, unsigned int session, unsigned int count, void *buffer);
unsigned int fsp_walk(unsigned int target, unsigned int id, char *path);
unsigned int fsp_walk2(unsigned int descriptor, unsigned int id, char *path);
void fsp_walkresponse(unsigned int source, unsigned int session, unsigned int id);
