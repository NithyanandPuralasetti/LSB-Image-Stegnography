#include "decode.h"

/* Rebuild one byte or int from LSB bits */
long int decode_data_from_lsb(char *buffer, int size)
{
    unsigned long res = 0;
    for (int i = 0; i < size; i++)
    {
        res = res | (((unsigned char)buffer[i] & 1) << (size - i - 1));
    }
    return (long int)res;
}

/* Main decode function */
Status do_decoding(EncodeInfo *encInfo)
{
    // Skip the 54-byte BMP header
    fseek(encInfo->fptr_src_image, 54, SEEK_SET);

    char *buffer = (char *)malloc(32);
    if (buffer == NULL)
        return e_failure;

    // 1. Read magic string size
    printf("INFO: Decoding Magic String Size\n");
    if (fread(buffer, 1, 32, encInfo->fptr_src_image) != 32)
    {
        free(buffer);
        fclose(encInfo->fptr_src_image);
        return e_failure;
    }
    long embedded_magic_len = decode_data_from_lsb(buffer, 32);

    if (embedded_magic_len <= 0 || embedded_magic_len > 100)
    {
        printf("Error: Not an encoded file or corrupted data.\n");
        free(buffer);
        fclose(encInfo->fptr_src_image);
        return e_failure;
    }
    printf("DONE\n");

    // 2. Read magic string characters
    printf("INFO: Decoding Magic String Signature\n");
    char *decoded_magic = (char *)malloc(embedded_magic_len + 1);
    if (decoded_magic == NULL)
    {
        free(buffer);
        fclose(encInfo->fptr_src_image);
        return e_failure;
    }

    for (int i = 0; i < embedded_magic_len; i++)
    {
        if (fread(buffer, 1, 8, encInfo->fptr_src_image) != 8)
        {
            free(decoded_magic);
            free(buffer);
            fclose(encInfo->fptr_src_image);
            return e_failure;
        }
        decoded_magic[i] = (char)decode_data_from_lsb(buffer, 8);
    }
    decoded_magic[embedded_magic_len] = '\0';

    // Check if entered passphrase matches
    if (strcmp(decoded_magic, encInfo->magic_pass) != 0)
    {
        printf("Error: Wrong passphrase.\n");
        free(decoded_magic);
        free(buffer);
        fclose(encInfo->fptr_src_image);
        return e_failure;
    }
    printf("DONE\n");
    free(decoded_magic);

    // 3. Read extension size
    printf("INFO: Decoding Output File Extenstion\n");
    if (fread(buffer, 1, 32, encInfo->fptr_src_image) != 32)
    {
        free(buffer);
        fclose(encInfo->fptr_src_image);
        return e_failure;
    }
    printf("DONE\n");
    long extn_size = decode_data_from_lsb(buffer, 32);

    if (extn_size <= 0 || extn_size > 10)
    {
        printf("ERROR: Invalid file extension size found in carrier.\n");
        free(buffer);
        fclose(encInfo->fptr_src_image);
        return e_failure;
    }
    // 4. Read extension string
    printf("INFO: Decoding extension string\n");
    char *extn = (char *)malloc(extn_size + 1);
    if (extn == NULL)
    {
        free(buffer);
        fclose(encInfo->fptr_src_image);
        return e_failure;
    }
    for (int i = 0; i < extn_size; i++)
    {
        if (fread(buffer, 1, 8, encInfo->fptr_src_image) != 8)
        {
            free(extn);
            free(buffer);
            fclose(encInfo->fptr_src_image);
            return e_failure;
        }
        extn[i] = (char)decode_data_from_lsb(buffer, 8);
    }
    extn[extn_size] = '\0';
    printf("File extension found: %s\n", extn);
    printf("DONE\n");

    // 5. Build output filename with extension
    char out_filename[256];
    char *dot = strrchr(encInfo->secret_fname, '.');
    if (dot != NULL)
    {
        int base_len = dot - encInfo->secret_fname;
        strncpy(out_filename, encInfo->secret_fname, base_len);
        out_filename[base_len] = '\0';
        strcat(out_filename, extn);
    }
    else
    {
        snprintf(out_filename, sizeof(out_filename), "%s%s", encInfo->secret_fname, extn);
    }
    free(extn);

    // Open output file
    encInfo->fptr_secret = fopen(out_filename, "wb");
    if (encInfo->fptr_secret == NULL)
    {
        perror("fopen");
        free(buffer);
        fclose(encInfo->fptr_src_image);
        return e_failure;
    }
    printf("Saving output to: %s\n", out_filename);

    // 6. Read secret file size
    printf("INFO: Decoding  File Size\n");
    if (fread(buffer, 1, 32, encInfo->fptr_src_image) != 32)
    {
        free(buffer);
        fclose(encInfo->fptr_secret);
        fclose(encInfo->fptr_src_image);
        return e_failure;
    }
    long file_size = decode_data_from_lsb(buffer, 32);
    printf("DONE\n");
    // 7. Read and write secret file data
    printf("INFO: Decoding  File Data\n");
    for (long i = 0; i < file_size; i++)
    {
        if (fread(buffer, 1, 8, encInfo->fptr_src_image) != 8)
        {
            free(buffer);
            fclose(encInfo->fptr_secret);
            fclose(encInfo->fptr_src_image);
            return e_failure;
        }
        char ch = (char)decode_data_from_lsb(buffer, 8);
        fputc(ch, encInfo->fptr_secret);
    }
    printf("DONE\n");
    // Clean up
    free(buffer);
    fclose(encInfo->fptr_secret);
    encInfo->fptr_secret = NULL;
    fclose(encInfo->fptr_src_image);
    encInfo->fptr_src_image = NULL;

    return e_success;
}