#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

short VTX_X_PATTERN[] = {
    0x0000,
    0x0200,
    0x0400,
    0x0600,
    0x0800,
    0x0A00,
    0x0C00,
    0x0E00,
    0x1000,
    0x0100,
    0x0300,
    0x0500,
    0x0700,
    0x0900,
    0x0B00,
    0x0D00,
    0x0F00};

typedef struct
{
    short x, y, z;
} COORDS;

// Get our implict X value based on pattern
short get_default_vtxX(int index)
{
    int column = index % 17;
    return VTX_X_PATTERN[column];
}

// Get our implicit Z value based on pattern
short get_default_vtxZ(int index)
{
    int row = index / 17;
    return row * 256;
}

// For each coord data, we need to determine the shift amount based on the value we get from each of the 8 bits in the command byte
short read_coord_data(unsigned char *buffer, int *position, unsigned char shift_value, bool xz_or_y)
{
    // Amount to shift left by
    // Index of list matches to 0-3 which are the different shift values. Differs for x and z vs y
    int shift_xzamt[] = {0, 8, 5, 6};
    int shift_yamt[] = {0, 0, 1, 2};

    unsigned short value;

    if (shift_value == 0x00)
    {
        // No shift, read value as short
        value = (buffer[*position] << 8) | buffer[*position + 1];
        *position += 2;
    }
    else
    {
        // Need to shift and store as a byte
        value = buffer[*position];
        *position += 1;

        if (xz_or_y == true)
        {
            value = value << shift_xzamt[shift_value];
        }
        else
        {
            value = value << shift_yamt[shift_value];
        }
    }

    return value;
}

// Our actual decoding function
int decode_vtx(unsigned char *buffer, int file_size)
{

    int vertex_index = 0;
    int position = 0;

    while (position < file_size && vertex_index < 561)
    {
        unsigned char command = buffer[position++];

        if (command == 0x09)
        {
            printf("Encountered terminator. Stopping read!\n");
            break;
        }

        int shift_x = command & 0x03;        // bits 1-0
        int shift_y = (command & 0x0C) >> 2; // bits 3-2
        int shift_z = (command & 0x30) >> 4; // bits 5-4

        COORDS coords;

        int implicitX = command & 0x40; // bit 6
        if (implicitX != 0)
        {
            coords.x = get_default_vtxX(vertex_index);
        }
        else
        {
            coords.x = read_coord_data(buffer, &position, shift_x, true);
        }

        coords.y = read_coord_data(buffer, &position, shift_y, false);

        int implicitZ = command & 0x80; // bit 7
        if (implicitZ != 0)
        {
            coords.z = get_default_vtxZ(vertex_index);
        }
        else
        {
            coords.z = read_coord_data(buffer, &position, shift_z, true);
        }
        printf("Vertex %d: X=0x%04x, Y=0x%04x, Z=0x%04x\n",
               vertex_index, coords.x, coords.y, coords.z);

        vertex_index++;
    }

    return 0;
}

// Read in our .vtx file and decode vertex coordinates
int read_vtx_file(FILE *vtx_file)
{
    fseek(vtx_file, 0, SEEK_END);
    long file_size = ftell(vtx_file);
    fseek(vtx_file, 0, SEEK_SET);

    unsigned char *vtx_file_buffer = malloc(file_size);
    fread(vtx_file_buffer, 1, file_size, vtx_file);
    fclose(vtx_file);

    decode_vtx(vtx_file_buffer, file_size);

    free(vtx_file_buffer);

    return 0;
}

int main(int argc, char **argv)
{
    FILE *vtx_file = fopen("course_data/hole1.vtx", "rb");
    read_vtx_file(vtx_file);
    return 0;
}