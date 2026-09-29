# LSB Image Steganography in C

A command-line tool written in C that hides secret files (such as `.txt`, `.c`, `.sh`, or `.html`) inside 24-bit uncompressed BMP images using Least Significant Bit (LSB) steganography. It also extracts and restores the hidden file back to its original format using a passphrase.

---

## How It Works

Every pixel in a 24-bit BMP image is made up of 3 bytes (Red, Green, Blue). Because modifying the least significant bit of a color byte makes an invisible difference to the human eye, 8 image bytes can safely store 1 byte of hidden data without changing how the picture looks.

### Data Layout in the Stego Image

When encoding, the program skips the first 54 bytes (the standard BMP header) and embeds information in the following order:

1. **Magic String Size** (32 bits)
2. **Magic String / Passphrase** (8 bits per character)
3. **Secret File Extension Size** (32 bits)
4. **Secret File Extension** (8 bits per character, e.g., `.txt`)
5. **Secret File Size** (32 bits)
6. **Secret File Contents** (8 bits per byte)
7. **Remaining Image Data** (copied directly without changes)

During decoding, the tool reads the header, checks the embedded magic passphrase, reconstructs the original file extension and size, and writes the hidden contents into a restored output file.

---

## Features

- **Passphrase Protection:** The secret file can only be extracted if the user enters the exact matching passphrase used during encoding.
- **Image Capacity Check:** Calculates whether the carrier BMP image has enough bytes to hold the entire secret payload before touching any data.
- **Header Protection:** Keeps the 54-byte BMP header completely intact so the output image remains a valid, viewable image.
- **Multiple File Types Supported:** Works with `.txt`, `.c`, `.sh`, and `.html` secret files.
- **Chunked File Copying:** Large image remnants are copied in 4 KB chunks to keep memory usage low and prevent stack overflow.

---

## File Structure

```text
├── main.c        # Command-line parsing and workflow routing
├── encode.c      # Functions for capacity check, bit manipulation, and encoding
├── encode.h      # Encode structures, macros, and prototypes
├── decode.c      # Functions for decoding bits, verifying passphrase, and file extraction
├── decode.h      # Decode function prototypes
├── types.h       # Status enums and custom type aliases
├── common.h      # Shared macro constants (max passphrase length)
└── README.md     # Documentation
```

---

## How to Build and Run

### Requirements
- GCC compiler
- Linux, macOS, or WSL (Windows Subsystem for Linux)
- A 24-bit uncompressed `.bmp` image file

### 1. Compile
Compile all source files together using `gcc`:

```bash
gcc main.c encode.c decode.c -o stego
```

---

### 2. Encoding (Hiding a File)

**Syntax:**
```bash
./stego -e <source_image.bmp> <secret_file> [output_image.bmp]
```

* If you do not specify an output image name, it defaults to `stego_img.bmp`.

**Example:**
```bash
./stego -e beautiful.bmp secret.txt stego_output.bmp
```

You will be prompted to enter a magic passphrase:
```text
Enter Magic String (Passphrase) to encode: mysecretkey
INFO: ## Encoding Procedure Started ##
INFO: Carrier image capacity: 1536000 bytes
INFO: Secret file size: 24 bytes
INFO: Done. Found OK
INFO: Copying Image Header
INFO: DONE
INFO: Encoding Magic String Size
INFO: DONE
...
INFO: ## Encoding Done Successfully ##
```

---

### 3. Decoding (Extracting the File)

**Syntax:**
```bash
./stego -d <stego_image.bmp> [output_filename]
```

* If you do not provide an output filename, it defaults to `decoded_output`. The program will automatically append the correct extension (e.g., `.txt`).

**Example:**
```bash
./stego -d stego_output.bmp recovered
```

You will be prompted to enter the passphrase:
```text
Enter Magic String (Passphrase) to decode: mysecretkey
INFO: Decoding Magic String Size
DONE
INFO: Decoding Magic String Signature
DONE
File extension found: .txt
Saving output to: recovered.txt
INFO: Decoding File Size
DONE
INFO: Decoding File Data
DONE
INFO: ## Decoding Done Successfully ##
```

If the passphrase does not match, the program aborts immediately without creating a corrupted file.

---

## Author

- **P. Nithyanand**
- GitHub: [@NithyanandPuralasetti](https://github.com/NithyanandPuralasetti)
