#define P9P_TLERROR                     6
#define P9P_RLERROR                     7
#define P9P_TLSTATFS                    8
#define P9P_RLSTATFS                    9
#define P9P_TLOPEN                      12
#define P9P_RLOPEN                      13
#define P9P_TLCREATE                    14
#define P9P_RLCREATE                    15
#define P9P_TSYMLINK                    16
#define P9P_RSYMLINK                    17
#define P9P_TMKNOD                      18
#define P9P_RMKNOD                      19
#define P9P_TRENAME                     20
#define P9P_RRENAME                     21
#define P9P_TREADLINK                   22
#define P9P_RREADLINK                   23
#define P9P_TGETATTR                    24
#define P9P_RGETATTR                    25
#define P9P_TSETATTR                    26
#define P9P_RSETATTR                    27
#define P9P_TXATTRWALK                  30
#define P9P_RXATTRWALK                  31
#define P9P_TXATTRCREATE                32
#define P9P_RXATTRCREATE                33
#define P9P_TREADDIR                    40
#define P9P_RREADDIR                    41
#define P9P_TFSYNC                      50
#define P9P_RFSYNC                      51
#define P9P_TLOCK                       52
#define P9P_RLOCK                       53
#define P9P_TGETLOCK                    54
#define P9P_RGETLOCK                    55
#define P9P_TLINK                       70
#define P9P_RLINK                       71
#define P9P_TMKDIR                      72
#define P9P_RMKDIR                      73
#define P9P_TRENAMEAT                   74
#define P9P_RRENAMEAT                   75
#define P9P_TUNLINKAT                   76
#define P9P_RUNLINKAT                   77
#define P9P_TVERSION                    100
#define P9P_RVERSION                    101
#define P9P_TAUTH                       102
#define P9P_RAUTH                       103
#define P9P_TATTACH                     104
#define P9P_RATTACH                     105
#define P9P_TERROR                      106
#define P9P_RERROR                      107
#define P9P_TFLUSH                      108
#define P9P_RFLUSH                      109
#define P9P_TWALK                       110
#define P9P_RWALK                       111
#define P9P_TOPEN                       112
#define P9P_ROPEN                       113
#define P9P_TCREATE                     114
#define P9P_RCREATE                     115
#define P9P_TREAD                       116
#define P9P_RREAD                       117
#define P9P_TWRITE                      118
#define P9P_RWRITE                      119
#define P9P_TCLUNK                      120
#define P9P_RCLUNK                      121
#define P9P_TREMOVE                     122
#define P9P_RREMOVE                     123
#define P9P_TSTAT                       124
#define P9P_RSTAT                       125
#define P9P_TWSTAT                      126
#define P9P_RWSTAT                      127
#define P9P_OFFSET_SIZE                 0
#define P9P_OFFSET_TYPE                 4
#define P9P_OFFSET_TAG                  5
#define P9P_OFFSET_DATA                 7
#define P9P_VERSION_UNKNOWN             0
#define P9P_VERSION_9P2000              1
#define P9P_VERSION_9P2000U             2
#define P9P_VERSION_9P2000L             3
#define P9P_VERSION_9P2000F             4

struct p9p_header
{

    unsigned char size[4];
    unsigned char type[1];
    unsigned char tag[2];

};

unsigned char p9p_read1(void *buffer, unsigned int o);
unsigned short p9p_read2(void *buffer, unsigned int o);
unsigned int p9p_read4(void *buffer, unsigned int o);
void *p9p_readbuffer(void *buffer, unsigned int o);
unsigned short p9p_readstringlength(void *buffer, unsigned int o);
char *p9p_readstringdata(void *buffer, unsigned int o);
unsigned int p9p_write1(void *buffer, unsigned int o, unsigned char v);
unsigned int p9p_write2(void *buffer, unsigned int o, unsigned short v);
unsigned int p9p_write4(void *buffer, unsigned int o, unsigned int v);
unsigned int p9p_write8(void *buffer, unsigned int o, unsigned int vl, unsigned int vh);
unsigned int p9p_writebuffer(void *buffer, unsigned int o, unsigned int count, void *v);
unsigned int p9p_writestring(void *buffer, unsigned int o, char *v);
unsigned int p9p_mk(void *buffer, unsigned int size, unsigned char type, unsigned short tag);
unsigned int p9p_mkrlerror(void *buffer, unsigned short tag, int ecode);
unsigned int p9p_mkrerror(void *buffer, unsigned short tag, char *ename);
unsigned int p9p_mktattach(void *buffer, unsigned short tag, unsigned int fid, unsigned int afid, char *uname, char *aname);
unsigned int p9p_mktattach_u(void *buffer, unsigned short tag, unsigned int fid, unsigned int afid, char *uname, char *aname, unsigned int nuname);
unsigned int p9p_mkrattach(void *buffer, unsigned short tag, char qid[13]);
unsigned int p9p_mktauth(void *buffer, unsigned short tag, unsigned int afid, char *uname, char *aname);
unsigned int p9p_mkrauth(void *buffer, unsigned short tag, char aqid[13]);
unsigned int p9p_mktauth_u(void *buffer, unsigned short tag, unsigned int afid, char *uname, char *aname, unsigned int nuname);
unsigned int p9p_mktclunk(void *buffer, unsigned short tag, unsigned int fid);
unsigned int p9p_mkrclunk(void *buffer, unsigned short tag);
unsigned int p9p_mktflush(void *buffer, unsigned short tag, unsigned short oldtag);
unsigned int p9p_mkrflush(void *buffer, unsigned short tag);
unsigned int p9p_mktversion(void *buffer, unsigned short tag, unsigned int msize, char *version);
unsigned int p9p_mkrversion(void *buffer, unsigned short tag, unsigned int msize, char *version);
unsigned int p9p_mktwalk(void *buffer, unsigned short tag, unsigned int fid, unsigned int newfid, unsigned short nwname, char **wname);
unsigned int p9p_mkrwalk(void *buffer, unsigned short tag, unsigned short nwqid, char *wqid[13]);
unsigned int p9p_mktread(void *buffer, unsigned short tag, unsigned int fid, unsigned int offsetl, unsigned int offseth, unsigned int count);
unsigned int p9p_mkrread(void *buffer, unsigned short tag, unsigned int count, void *data);
unsigned int p9p_mktwrite(void *buffer, unsigned short tag, unsigned int fid, unsigned int offsetl, unsigned int offseth, unsigned int count, void *data);
unsigned int p9p_mkrwrite(void *buffer, unsigned short tag, unsigned int count);
unsigned int p9p_mktopen(void *buffer, unsigned short tag, unsigned int fid, unsigned char mode);
unsigned int p9p_mkropen(void *buffer, unsigned short tag, char qid[13], unsigned int iounit);
unsigned int p9p_mktcreate(void *buffer, unsigned short tag, unsigned int fid, char *name, unsigned int perm, unsigned char mode);
unsigned int p9p_mkrcreate(void *buffer, unsigned short tag, char qid[13], unsigned int iounit);
unsigned int p9p_mktremove(void *buffer, unsigned short tag, unsigned int fid);
unsigned int p9p_mkrremove(void *buffer, unsigned short tag);
unsigned int p9p_mktstat(void *buffer, unsigned short tag, unsigned int fid);
unsigned int p9p_mkrstat(void *buffer, unsigned short tag, void *stat);
unsigned int p9p_mktwstat(void *buffer, unsigned short tag, unsigned int fid, void *stat);
unsigned int p9p_mkrwstat(void *buffer, unsigned short tag);
unsigned int p9p_mktgetattr(void *buffer, unsigned short tag, unsigned int fid, char mask[8]);
unsigned int p9p_mkrgetattr(void *buffer, unsigned short tag, char valid[8], char qid[13], unsigned int mode, unsigned int uid, unsigned int gid, char nlink[8], char rdev[8], char size[8], char blksize[8], char blocks[8], char atimesec[8], char atimensec[8], char mtimesec[8], char mtimensec[8], char ctimesec[8], char ctimensec[8], char btimesec[8], char btimensec[8], char gen[8], char dataversion[8]);
