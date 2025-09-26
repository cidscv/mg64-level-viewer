#include <stdio.h>
#include <stdlib.h>
#include <unitypes.h>

// Course data table start offset = 0xE473F0
long STARTING_OFFSET = 0xE473F0;

// Length of each type of data in table
int VTX_LENGTH = 0x231;
int DTX_LENGTH = 0x800;

int generate_vtx(FILE *rom, int level_index)
{

    long vtx_entry = STARTING_OFFSET + (level_index * 7 * 8);

    long vtx_entry_offset = STARTING_OFFSET + (level_index * 7 * 8);

    // Read the length and offset from the pointer table
    fseek(rom, vtx_entry_offset, SEEK_SET);

    unsigned char length_bytes[4], offset_bytes[4];
    fread(length_bytes, 1, 4, rom);
    fread(offset_bytes, 1, 4, rom);

    // Convert from big-endian to get actual values
    uint32_t vtx_length = (length_bytes[0] << 24) | (length_bytes[1] << 16) |
                          (length_bytes[2] << 8) | length_bytes[3];
    uint32_t vtx_relative_offset = (offset_bytes[0] << 24) | (offset_bytes[1] << 16) |
                                   (offset_bytes[2] << 8) | offset_bytes[3];

    // Calculate the absolute offset where VTX data actually is
    uint32_t vtx_absolute_offset = STARTING_OFFSET + vtx_relative_offset;

    printf("VTX Length: 0x%x, Absolute Offset: 0x%x\n", vtx_length, vtx_absolute_offset);

    // Now seek to where the VTX data actually is
    fseek(rom, vtx_absolute_offset, SEEK_SET);

    // Allocate buffer using the actual length from the table
    unsigned char *vtx_buffer = malloc(vtx_length);
    size_t bytes_read = fread(vtx_buffer, 1, vtx_length, rom);

    // Write to file
    FILE *vtx_file = fopen("course_data/hole1.vtx", "wb");
    fwrite(vtx_buffer, 1, bytes_read, vtx_file);
    fclose(vtx_file);
    free(vtx_buffer);

    return 0;
}

int generate_dtx(FILE *rom)
{
    long dtx_start = STARTING_OFFSET + VTX_LENGTH;

    printf("%lX\n", dtx_start);

    // Find to starting offset and read in data
    fseek(rom, dtx_start, SEEK_SET);
    unsigned char *dtx_buffer = malloc(DTX_LENGTH);
    size_t bytes_read = fread(dtx_buffer, 1, DTX_LENGTH, rom);

    // Write to .vtx file
    FILE *dtx_file = fopen("course_data/hole1.dtx", "wb");
    fwrite(dtx_buffer, 1, bytes_read, dtx_file);
    fclose(dtx_file);

    free(dtx_buffer);

    return 0;
}

int main(int argc, char **argv)
{

    // Check command was correctly inputted
    if (argc != 2)
    {
        printf("Usage: %s <rom.z64>\n", argv[0]);
        return 1;
    }

    char *rom_path = argv[1];

    // Open our MG64 rom
    FILE *rom;
    rom = fopen(rom_path, "rb");
    if (!rom)
    {
        printf("Error: unable to open file '%s'\n", rom_path);
        return 1;
    }
    else
    {
        printf("Reading file %s\n", rom_path);
    }

    int level_index = 0;
    generate_vtx(rom, level_index);
    generate_dtx(rom);

    fclose(rom);

    return 0;
}