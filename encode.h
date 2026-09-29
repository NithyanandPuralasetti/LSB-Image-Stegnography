#ifndef ENCODE_H
#define ENCODE_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "types.h"
#include "common.h"

/* Buffer and string size definitions */
#define MAX_SECRET_BUF_SIZE 1
#define MAX_IMAGE_BUF_SIZE (MAX_SECRET_BUF_SIZE * 8)
#define MAX_FILE_SUFFIX 10

/* Structure to store information required for steganography */
typedef struct _EncodeInfo
{
    /* Source / Carrier Image info */
    char *src_image_fname;
    FILE *fptr_src_image;
    uint image_capacity;
    char image_data[MAX_IMAGE_BUF_SIZE];

    /* Dynamic Magic String / Passphrase */
    char magic_pass[MAX_MAGIC_LEN];
    long magic_len;

    /* Secret File Info */
    char *secret_fname;
    FILE *fptr_secret;
    long extn_size;
    char extn_secret_file[MAX_FILE_SUFFIX];
    char secret_data[MAX_SECRET_BUF_SIZE];
    long size_secret_file;

    /* Stego / Encrypted Image Info */
    char *stego_image_fname;
    FILE *fptr_stego_image;

} EncodeInfo;

/* Function Prototypes */

/* Read and validate command-line arguments */
Status read_and_validate_encode_args(char *argv[], EncodeInfo *encInfo, int argc);

/* Master controller function for encoding */
Status do_encoding(EncodeInfo *encInfo);

/* Open file pointers for input and output files */
Status open_files(EncodeInfo *encInfo, char *str);

/* Check if image capacity is sufficient to hold secret data */
Status check_capacity(EncodeInfo *encInfo);

/* Get image capacity (width * height * 3) from BMP header */
uint get_image_size_for_bmp(FILE *fptr_image);

/* Get secret file size */
uint get_file_size(FILE *fptr);

/* Copy 54-byte BMP header from source to destination */
Status copy_bmp_header(FILE *fptr_src_image, FILE *fptr_dest_image);

/* Encode the Magic String into image LSBs */
Status encode_magic_string(const char *magic_string, EncodeInfo *encInfo);

/* Encode integer values (extn_size, file_size) as 32 LSB bits */
Status encode_secret_file_size(long int size, EncodeInfo *encInfo);

/* Encode secret file extension characters */
Status encode_secret_file_extn(const char *file_extn, EncodeInfo *encInfo);

/* Encode secret file payload data into carrier image */
Status encode_data_to_image(char *data, int size, FILE *fptr_src_image, FILE *fptr_stego_image);

/* Encode 1 byte of secret data into 8 carrier image bytes */
Status encode_byte_tolsb(char data, char *image_buffer);

/* Copy remaining unencoded image bytes safely in chunks */
Status copy_remaining_img_data(long size, FILE *fptr_src, FILE *fptr_dest);

/* Utility to view BMP header bytes in hex */
void print_bmp_header(FILE *fptr);

#endif