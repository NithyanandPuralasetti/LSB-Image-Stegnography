#include "encode.h"
#include "decode.h"

int main(int argc, char *argv[])
{
    EncodeInfo encInfo;

    // Validate command-line options
    if (read_and_validate_encode_args(argv, &encInfo, argc) == e_failure)
    {
        return 1;
    }

    // Open required carrier and secret files
    if (open_files(&encInfo, argv[1]) == e_failure)
    {
        printf("ERROR: File opening failed\n");
        return 1;
    }

    // Perform Encoding
    if (strcmp(argv[1], "-e") == 0)
    {
        printf("Enter Magic String (Passphrase) to encode: ");
        if (scanf("%31s", encInfo.magic_pass) != 1)
        {
            printf("ERROR: Failed to read passphrase\n");
            return 1;
        }
        encInfo.magic_len = strlen(encInfo.magic_pass);

        if (do_encoding(&encInfo) == e_failure)
        {
            printf("ERROR: Encoding failed\n");
            return 1;
        }
        printf("INFO: ## Encoding Done Successfully ##\n");
    }
    // Perform Decoding
    else if (strcmp(argv[1], "-d") == 0)
    {
        printf("Enter Magic String (Passphrase) to decode: ");
        if (scanf("%31s", encInfo.magic_pass) != 1)
        {
            printf("ERROR: Failed to read passphrase\n");
            return 1;
        }
        encInfo.magic_len = strlen(encInfo.magic_pass);

        if (do_decoding(&encInfo) == e_failure)
        {
            printf("ERROR: Decoding failed\n");
            return 1;
        }
        printf("INFO: ## Decoding Done Successfully ##\n");
    }

    return 0;
}