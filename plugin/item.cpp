#include "item.h"
#include <string>
#include <sstream>

#define _CRT_SECURE_NO_WARNINGS

Item::Item( const char * name_)
{
    strcpy(name, name_);
}

Segment::Segment( const char *name_, float tx_, float ty_, float tz_,
                        float rx_, float ry_, float rz_, float rw_ )
    : Item(name_)
{
        tx = tx_;
        ty = ty_;
        tz = tz_;
        rx = rx_;
        ry = ry_;
        rz = rz_;
        rw = rw_;
}

Marker::Marker( const char *name_, float tx_, float ty_, float tz_ )
    : Item(name_)
{
        tx = tx_;
        ty = ty_;
        tz = tz_;
}


size_t serializeItems( std::vector<Item*> &items,  char *buffer, size_t buflen)
{
    char *ptr = buffer;

    *(ptr++) = 42; 
    *(ptr++) = (unsigned char) items.size(); 

    int ret;

    for( size_t i = 0; i < items.size(); i++)
    {

        Item *item = items[i];
        Segment *segment = dynamic_cast<Segment*>( item );
        Marker  *marker  = dynamic_cast<Marker*> ( item );

        if( segment != NULL)
        {
            unsigned char namelen = strlen( segment->name );

            *(ptr++) = 43;
            *(ptr++) = namelen;
            unsigned char *datalen = (unsigned char*)(ptr++);
            *datalen = 0;

            memcpy( ptr, segment->name, namelen);
            ptr += namelen;

            float *eachFloat = &segment->tx;

            for( int i =0 ; i < 7; i++)
            {
                ret = sprintf( ptr, "%f",  *(eachFloat++) );
                ptr += ret + 1;
                *datalen += ret + 1;
            };
        }
        if( marker != NULL)
        {
            unsigned char namelen = strlen( marker->name );

            *(ptr++) = 44;
            *(ptr++) = namelen;
            unsigned char *datalen = (unsigned char*)(ptr++);
            *datalen = 0;

            memcpy( ptr, marker->name, namelen);
            ptr += namelen;

            float *eachFloat = &marker->tx;

            for( int i =0 ; i < 3; i++)
            {
                ret = sprintf( ptr, "%f",  *(eachFloat++) );
                ptr += ret + 1;
                *datalen += ret + 1;
            };
        }

    }

    return ptr-buffer;
}



size_t parseItems(const char *buffer, size_t len, std::vector<Item*> *items )
{
    if( len < 3) return 0;

    // The first character should be 42, the next should be the number if entries.
    if(buffer[0] != 42)
    {
        fprintf(stderr,"Invalid packet header:  %02X (%zu) - expected %02X", buffer[0], len, 42);
        return 0;
    }
    unsigned char count = buffer[1];

    if(count == 0) return 0;

    const char *ptr = buffer + 2;

    items->clear();

    for (size_t i = 0; i < count && (ptr - buffer) < len; i++)
    {

        char id = *(ptr++);
        if (id != 43 && id != 44)
        {
            fprintf(stderr, "Invalid packet entry %02X, expected %02x or %02x\n", id, 43, 44);
            return 0;
        }

        unsigned char namelen = *(ptr++);
        unsigned char datalen = *(ptr++);
        const char*   name = ptr;  ptr += namelen;
        const char*   data = ptr;  ptr += datalen;

        if (namelen > 30 )
        {
           fprintf(stderr, "Name length overflow: %.*s\n", name, namelen);
           continue;
        }
            

        if (ptr - buffer > len)
        {
            fprintf(stderr, "Parser buffer overrun\n");
            return 0;
        }

        if( id == 43 )
        {
            Segment *segment = new Segment();

            // Name
            memcpy(segment->name, name, namelen);
            segment->name[namelen] = 0;

            float   *fptr = &segment->tx;
            size_t   dptr = 0;

            for (int j = 0; j < 7 && ptr - buffer <= len; j++)
            {
                fptr[j] = atof(data+dptr);
                dptr += strlen(data+dptr)+1;
            }

            items->push_back(segment);

        }

        if( id == 44 )
        {
            Marker *marker = new Marker();

            // Name
            memcpy(marker->name, name, namelen);
            marker->name[namelen] = 0;

            float      *fptr = &marker->tx;
            size_t      dptr = 0;

            for (int j = 0; j < 3 && ptr - buffer < len; j++)
            {
                fptr[j] = atof(data + dptr);
                dptr += strlen(data + dptr) + 1;
            }

            items->push_back(marker);

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
        char id = *(ptr++);

        if( id != 43 && id != 44)
        {
            fprintf(stderr, "Invalid Packet entry at byte: %zu\n", ptr-buffer);
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

