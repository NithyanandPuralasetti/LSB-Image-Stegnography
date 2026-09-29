#include "encode.h"

/* Read width and height from BMP header (offset 18) and calculate capacity */
uint get_image_size_for_bmp(FILE *fptr_image)
{
    uint width, height;
    fseek(fptr_image, 18, SEEK_SET);

    fread(&width, sizeof(uint), 1, fptr_image);
    fread(&height, sizeof(uint), 1, fptr_image);
    rewind(fptr_image);

    return width * height * 3;
}

/* Open files for reading and writing based on operation */
Status open_files(EncodeInfo *encInfo, char *str)
{
    printf("INFO: Opening required files\n");
    // Open source carrier image
    encInfo->fptr_src_image = fopen(encInfo->src_image_fname, "rb");
    if (encInfo->fptr_src_image == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->src_image_fname);
        return e_failure;
    }
    printf("INFO: Opened %s\n", encInfo->src_image_fname);
    // If encoding, open secret file to read and stego image to write
    if (strcmp(str, "-e") == 0)
    {
        encInfo->fptr_secret = fopen(encInfo->secret_fname, "rb");
        if (encInfo->fptr_secret == NULL)
        {
            perror("fopen");
            fprintf(stderr, "ERROR: Unable to open secret file %s\n", encInfo->secret_fname);
            return e_failure;
        }
        printf("INFO: Opened %s\n",encInfo->secret_fname);
        encInfo->fptr_stego_image = fopen(encInfo->stego_image_fname, "wb+");
        if (encInfo->fptr_stego_image == NULL)
        {
            perror("fopen");
            fprintf(stderr, "ERROR: Unable to create stego image %s\n", encInfo->stego_image_fname);
            return e_failure;
        }
        printf("INFO: Opened %s\n",encInfo->stego_image_fname);
    }
    printf("INFO: DONE\n");
    return e_success;
}

/* Find size of a file in bytes */
uint get_file_size(FILE *fptr)
{
    printf("INFO: Checking for secret.txt size\n");
    uint file_size;
    fseek(fptr, 0, SEEK_END);
    file_size = (uint)ftell(fptr);
    rewind(fptr);
    if(file_size)
    {
        printf("INFO: DONE NOT EMPTY\n");
    }
    return file_size;   
}

/* Validate command line arguments for encode and decode */
Status read_and_validate_encode_args(char *argv[], EncodeInfo *encInfo, int argc)
{
    int len;
    if (argc < 3)
    {
        printf("ENCODE : ./a.out -e <.bmp_file> <secret_file> [output file]\n");
        printf("DECODE : ./a.out -d <.bmp_file> [output file]\n");
        return e_failure;
    }

    if (strcmp(argv[1], "-e") == 0)
    {
        if (argc < 4)
        {
            printf("ENCODE : ./a.out -e <.bmp_file> <secret_file> [output file]\n");
            return e_failure;
        }

        // Check if carrier file ends in .bmp
        len = strlen(argv[2]);
        if (len < 5 || strcmp(argv[2] + len - 4, ".bmp") != 0)
        {
            printf("ERROR: Provide a valid carrier .bmp file\n");
            return e_failure;
        }

        // Validate secret file extension
        char *extn = strrchr(argv[3], '.');
        if (extn == NULL || (strcmp(extn, ".txt") != 0 &&
                             strcmp(extn, ".c") != 0 &&
                             strcmp(extn, ".sh") != 0 &&
                             strcmp(extn, ".html") != 0))
        {
            printf("ERROR: Secret file must have extension: .txt, .c, .sh, or .html\n");
            return e_failure;
        }

        strcpy(encInfo->extn_secret_file, extn);
        encInfo->extn_size = strlen(extn);

        printf("INFO: Operation: ENCODING\n");
        if (argc < 5)
        {
            printf("INFO: Output File not mentioned. Creating 'steged_img.bmp' as default\n");
            encInfo->stego_image_fname = "stego_img.bmp";
        }
        else
        {
            encInfo->stego_image_fname = argv[4];
        }

        encInfo->src_image_fname = argv[2];
        encInfo->secret_fname = argv[3];
        printf("INFO: Done. Opened all required files\n");
        return e_success;
    }
    else if (strcmp(argv[1], "-d") == 0)
    {
        len = strlen(argv[2]);
        if (len < 5 || strcmp(argv[2] + len - 4, ".bmp") != 0)
        {
            printf("ERROR: Provide a valid stego .bmp file\n");
            return e_failure;
        }

        printf("INFO: Operation: DECODING\n");
        if (argc < 4)
        {
            printf("INFO: Output File not mentioned. Creating decoded_output as default'\n");
            encInfo->secret_fname = "decoded_output";
        }
        else
        {
            encInfo->secret_fname = argv[3];
        }

        encInfo->src_image_fname = argv[2];
        printf("INFO: Done. Opened all required files\n");
        return e_success;
    }
    else
    {
        printf("ERROR: Unsupported operation '%s'. Use -e or -d\n", argv[1]);
        return e_failure;
    }
}

/* Check if the carrier image has enough bytes to hold all secret data */
Status check_capacity(EncodeInfo *encInfo)
{
    printf("INFO: Checking for %s capacity to handle %s\n",encInfo->src_image_fname,encInfo->secret_fname);
    // Total bits = (magic_len size + magic string + extn size + extn + file size + file data) * 8
    long total_required_bits = 8 * (sizeof(int) + 
                                    encInfo->magic_len + 
                                    sizeof(int) + 
                                    encInfo->extn_size + 
                                    sizeof(int) + 
                                    encInfo->size_secret_file);

    if (encInfo->image_capacity < (uint)total_required_bits)
    {
        return e_failure;
    }
    printf("INFO: Done. Found OK\n");
    return e_success;
}

/* Copy original 54-byte BMP header to destination image unchanged */
Status copy_bmp_header(FILE *fptr_src_image, FILE *fptr_dest_image)
{
    unsigned char buffer[54];
    rewind(fptr_src_image);
    rewind(fptr_dest_image);

    if (fread(buffer, 1, 54, fptr_src_image) != 54)
    {
        return e_failure;
    }
    if (fwrite(buffer, 1, 54, fptr_dest_image) != 54)
    {
        return e_failure;
    }
    return e_success;
}

/* Encode 8 bits of a byte into LSB of 8 image bytes */
Status encode_byte_tolsb(char data, char *image_buffer)
{
    for (int i = 7; i >= 0; i--)
    {
        image_buffer[7 - i] = ((data >> i) & 1) | (image_buffer[7 - i] & ~1);
    }
    return e_success;
}

/* Encode magic string characters into image bytes */
Status encode_magic_string(const char *magic_string, EncodeInfo *encInfo)
{
    int i = 0;
    while (magic_string[i] != '\0')
    {
        if (fread(encInfo->image_data, 1, 8, encInfo->fptr_src_image) != 8)
            return e_failure;

        encode_byte_tolsb(magic_string[i], encInfo->image_data);

        if (fwrite(encInfo->image_data, 1, 8, encInfo->fptr_stego_image) != 8)
            return e_failure;

        i++;
    }
    return e_success;
}

/* Encode extension string characters into image bytes */
Status encode_secret_file_extn(const char *file_extn, EncodeInfo *encInfo)
{
    int i = 0;
    while (file_extn[i] != '\0')
    {
        if (fread(encInfo->image_data, 1, 8, encInfo->fptr_src_image) != 8)
            return e_failure;

        encode_byte_tolsb(file_extn[i], encInfo->image_data);

        if (fwrite(encInfo->image_data, 1, 8, encInfo->fptr_stego_image) != 8)
            return e_failure;

        i++;
    }
    return e_success;
}

/* Encode a 32-bit integer into 32 image bytes */
Status encode_secret_file_size(long int size, EncodeInfo *encInfo)
{
    char buffer[32];
    if (fread(buffer, 1, 32, encInfo->fptr_src_image) != 32)
        return e_failure;

    for (int i = 31; i >= 0; i--)
    {
        buffer[31 - i] = ((size >> i) & 1) | (buffer[31 - i] & ~1);
    }

    if (fwrite(buffer, 1, 32, encInfo->fptr_stego_image) != 32)
        return e_failure;

    return e_success;
}

/* Encode secret file data into image bytes */
Status encode_data_to_image(char *data, int size, FILE *fptr_src_image, FILE *fptr_stego_image)
{
    char buffer[8];
    for (int i = 0; i < size; i++)
    {
        if (fread(buffer, 1, 8, fptr_src_image) != 8)
            return e_failure;

        encode_byte_tolsb(data[i], buffer);

        if (fwrite(buffer, 1, 8, fptr_stego_image) != 8)
            return e_failure;
    }
    return e_success;
}

/* Copy leftover image bytes in chunks to prevent stack overflow */
Status copy_remaining_img_data(long size, FILE *fptr_src, FILE *fptr_dest)
{
    char buffer[4096];
    size_t chunk;

    while (size > 0)
    {
        chunk = (size > 4096) ? 4096 : size;
        if (fread(buffer, 1, chunk, fptr_src) != chunk)
            return e_failure;

        if (fwrite(buffer, 1, chunk, fptr_dest) != chunk)
            return e_failure;

        size -= chunk;
    }
    return e_success;
}

/* Run the full encoding workflow */
Status do_encoding(EncodeInfo *encInfo)
{
    printf("INFO: ## Encoding Procedure Started ##\n");
    encInfo->image_capacity = get_image_size_for_bmp(encInfo->fptr_src_image);
    printf("INFO: Checking for secret.txt size\n");
    encInfo->size_secret_file = get_file_size(encInfo->fptr_secret);
    printf("INFO: Carrier image capacity: %u bytes\n", encInfo->image_capacity);
    printf("INFO: Secret file size: %ld bytes\n", encInfo->size_secret_file);

    // Check if image is big enough
    if (check_capacity(encInfo) == e_failure)
    {
        printf("ERROR: Image capacity is insufficient to store secret data\n");
        return e_failure;
    }

    // Copy BMP header (first 54 bytes)
    printf("INFO: Copying Image Header\n");
    if (copy_bmp_header(encInfo->fptr_src_image, encInfo->fptr_stego_image) == e_failure)
    {
        printf("ERROR: Failed to copy BMP header\n");
        return e_failure;
    }
    printf("INFO: DONE\n");

    // 1. Encode magic string length (32-bit int)
    printf("INFO: Encoding Magic String Size\n");
    if (encode_secret_file_size(encInfo->magic_len, encInfo) == e_failure)
        return e_failure;
    printf("INFO: DONE\n");
    // 2. Encode magic string characters
    printf("INFO: Encoding Magic String Signature\n");
    if (encode_magic_string(encInfo->magic_pass, encInfo) == e_failure)
        return e_failure;
    printf("INFO: DONE\n");

    // 3. Encode secret file extension size (32-bit int)
    printf("INFO: Encoding secret.txt File Extenstion size\n");
    if (encode_secret_file_size(encInfo->extn_size, encInfo) == e_failure)
        return e_failure;
    printf("INFO: DONE\n");

    // 4. Encode secret file extension characters
    printf("INFO: Encoding secret.txt File Extenstion\n");
    if (encode_secret_file_extn(encInfo->extn_secret_file, encInfo) == e_failure)
        return e_failure;
    printf("INFO: DONE\n");
    // 5. Encode secret file size (32-bit int)
    printf("INFO: Encoding secret.txt File Size\n");
    if (encode_secret_file_size(encInfo->size_secret_file, encInfo) == e_failure)
        return e_failure;
    printf("INFO: DONE\n");
    // 6. Read secret file data and encode into image
    printf("INFO: Encoding secret.txt File Data\n");
    char *secret_buffer = (char *)malloc(encInfo->size_secret_file);
    if (secret_buffer == NULL)
        return e_failure;

    if (fread(secret_buffer, 1, encInfo->size_secret_file, encInfo->fptr_secret) != (size_t)encInfo->size_secret_file)
    {
        free(secret_buffer);
        return e_failure;
    }
    if (encode_data_to_image(secret_buffer, encInfo->size_secret_file, encInfo->fptr_src_image, encInfo->fptr_stego_image) == e_failure)
    {
        free(secret_buffer);
        return e_failure;
    }
    printf("INFO: DONE\n");
    free(secret_buffer);

    // 7. Copy remaining unencoded image bytes
    long encoded_carrier_bytes = 8 * (sizeof(int) +
                                      encInfo->magic_len + 
                                      sizeof(int) + 
                                      encInfo->extn_size + 
                                      sizeof(int) + 
                                      encInfo->size_secret_file);
    long remaining_bytes = encInfo->image_capacity - encoded_carrier_bytes;
    printf("INFO: Copying Left Over Data\n");
    if (copy_remaining_img_data(remaining_bytes, encInfo->fptr_src_image, encInfo->fptr_stego_image) == e_failure)
    {
        printf("ERROR: Failed to copy remaining image bytes\n");
        return e_failure;
    }
    printf("INFO: DONE\n");
    // Close files
    fclose(encInfo->fptr_secret);
    encInfo->fptr_secret = NULL;
    fclose(encInfo->fptr_src_image);
    encInfo->fptr_src_image = NULL;
    fclose(encInfo->fptr_stego_image);
    encInfo->fptr_stego_image = NULL;
    return e_success;
}

/* Helper to print first 54 bytes of BMP header in hex */
void print_bmp_header(FILE *fptr)
{
    unsigned char header[54];
    rewind(fptr);
    size_t bytes_read = fread(header, 1, 54, fptr);
    if (bytes_read < 54)
    {
        fprintf(stderr, "Error: File smaller than 54-byte BMP header\n");
        return;
    }
    for (int i = 0; i < 54; i++)
    {
        printf("%02X ", header[i]);
        if ((i + 1) % 16 == 0)
            printf("\n");
    }
    printf("\n");
}