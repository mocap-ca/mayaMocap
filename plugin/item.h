#ifndef ITEM_H
#define ITEM_H


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <vector>
// Item struct for passing data from thread

class Item
{
public:
    Item() {}

    Item( const char *name_, float tx_, float ty_, float tz_,
                        float rx_, float ry_, float rz_, float rw_ )
    {
        strcpy(name, name_);
        tx = tx_;
        ty = ty_;
        tz = tz_;
        rx = rx_;
        ry = ry_;
        rz = rz_;
        rw = rw_;
    }
    char  name[20];
    float tx;
    float ty;
    float tz;
    float rx;
    float ry;
    float rz;
    float rw;
};


    


size_t serializeItems( std::vector<Item> &items,  char *buffer, size_t buflen)
{
    char *ptr = buffer;

    *(ptr++) = 42; 
    *(ptr++) = (unsigned char) items.size(); 

    int ret;

    for( size_t i = 0; i < items.size(); i++)
    {
        Item &item = items[i];
        unsigned char namelen = strlen( item.name );

        *(ptr++) = 43;
        *(ptr++) = namelen;
        unsigned char *datalen = (unsigned char*)(ptr++);
        *datalen = 0;

        memcpy( ptr, item.name, namelen);
        ptr += namelen;

        float *eachFloat = &item.tx;

        for( int i =0 ; i < 7; i++)
        {
            ret = sprintf( ptr, "%f",  *(eachFloat++) );
            ptr += ret + 1;
            *datalen += ret + 1;
        }
    }

    return ptr-buffer;
}
        
    

void dumpData( const char *buffer) 
{
    if(buffer[0] != 42 )
    {
        fprintf(stderr, "Invalid packet header\n");
        return;
    }

    unsigned char count = buffer[1];

    printf("Items: %d\n", count);

    const char *ptr = buffer + 2;

    for (unsigned char i = 0; i < count; i++)
    {

        if( *(ptr++) != 43 )
        {
            fprintf(stderr, "Invalid Packet entry at byte: %d\n", ptr-buffer);
            return;
        }

        unsigned char namelen = *(ptr++);
        unsigned char datalen = *(ptr++);

        const char* name = ptr;  ptr+= namelen;
        const char* data = ptr;  ptr+= datalen;

        printf("name   (%d)  %.*s\n", namelen, namelen, name );
        printf("floats (%d)\n", datalen );
        printf("  %s\n", data);

        for(unsigned char n = 0; n < datalen; n++)
        {
            if(data[n] == 0)
            {
                n++;
                if(n < datalen) printf("  %s\n", data+n);
            }
        }
    }
}


    



size_t parseItems(const char *buffer, size_t len, std::vector<Item> &items )
{
    if( len < 3) return 0;

    // The first character should be 42, the next should be the number if entries.
    if(buffer[0] != 42)
    {
        fprintf(stderr,"Invalid packet header:  %02X (%d)", buffer[0], len );
        return 0;
    }
    unsigned char count = buffer[1];

    if(count == 0) return 0;

    const char *ptr = buffer + 2;

    items.clear();

    for(size_t i = 0; i < count && (ptr-buffer) < len; i++)
    {

        if( *(ptr++) != 43 )
        {
            fprintf(stderr, "Invalid packet entry\n");
            return 0;
        }

        Item newItem;

        unsigned char namelen = *(ptr++);
        unsigned char datalen = *(ptr++);
        const char*   name    = ptr;  ptr += namelen;
        const char*   data    = ptr;  ptr += datalen;

        if(ptr-buffer > len)
        {
            fprintf(stderr, "Parser buffer overrun\n");
            return 0;
        }
         
        float* fdata = & (newItem.tx);

        // Name
        memcpy(newItem.name, name, namelen);
        newItem.name[ namelen ] = 0;

        fdata[0] = atof( data );

        int fn=1;

        for( int n=0; fn < 7 && n < datalen; n++ )
        {
            if( data[n] == 0)
            {
                n++;
                if(n < datalen) fdata[ fn++ ] = atof( data+n );
            }
        }
                
        items.push_back(newItem);
    }

    return ptr-buffer;
}


#endif // ITEM_H
