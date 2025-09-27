#include <stdio.h>
#include <stdlib.h>
#include <unitypes.h>

// Course data table start offset = 0xE473F0
long STARTING_OFFSET = 0xE473F0;

void locate_data_in_table(FILE *rom, long entry_offset, char *output)
{
    // Read the length and offset from the pointer table
    fseek(rom, entry_offset, SEEK_SET);

    unsigned char length_bytes[4], offset_bytes[4];
    fread(length_bytes, 1, 4, rom);
    fread(offset_bytes, 1, 4, rom);

    // Convert from big-endian to get actual values
    uint32_t length = (length_bytes[0] << 24) | (length_bytes[1] << 16) |
                      (length_bytes[2] << 8) | length_bytes[3];
    uint32_t relative_offset = (offset_bytes[0] << 24) | (offset_bytes[1] << 16) |
                               (offset_bytes[2] << 8) | offset_bytes[3];

    // Calculate the absolute offset where data actually is
    uint32_t absolute_offset = STARTING_OFFSET + relative_offset;

    printf("Length: 0x%x, Absolute Offset: 0x%x\n", length, absolute_offset);

    // Now seek to where the data actually is
    fseek(rom, absolute_offset, SEEK_SET);

    // Allocate buffer using the actual length from the table
    unsigned char *buffer = malloc(length);
    size_t bytes_read = fread(buffer, 1, length, rom);

    // Write to file
    FILE *vtx_file = fopen(output, "wb");
    fwrite(buffer, 1, bytes_read, vtx_file);
    fclose(vtx_file);
    free(buffer);
}

int generate_vtx(FILE *rom, int level_index)
{
    // Generate our VTX encoded data
    long vtx_entry_offset = STARTING_OFFSET + (level_index * 7 * 8);

    char output_path[25];
    sprintf(output_path, "course_data/hole%d.vtx", level_index);

    locate_data_in_table(rom, vtx_entry_offset, output_path);

    return 0;
}

int generate_dtx(FILE *rom, int level_index)
{
    // Genereta our DTX encoded data
    long dtx_entry_offset = STARTING_OFFSET + ((level_index * 7 + 1) * 8);

    char output_path[25];
    sprintf(output_path, "course_data/hole%d.dtx", level_index);

    locate_data_in_table(rom, dtx_entry_offset, output_path);

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
    generate_dtx(rom, level_index);

    fclose(rom);

    return 0;
}