#ifndef LIB_PPM_H
#define LIB_PPM_H

struct ppm_header
{

    char sign[2];
    unsigned int width;
    unsigned int height;
    char *data;

};

void ppm_parse(struct ppm_header *header, void *buffer);

#endif

