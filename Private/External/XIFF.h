/** ===========================================
 (C) Mahrouss Logic
    ===========================================*/

#pragma once

/** ---------------------------------------------------

    * THIS FILE CONTAINS CODE FOR THE eXtended Information File Format.
    * XIFF is used to make setup programs/audio/video files.

------------------------------------------------------- */

/***
 * @brief Generic XIFF header
 * Used by XIFF based containers.
 */

struct __attribute__((packed)) XiffHeader {
  unsigned char mag[4];      // XIFF string (includes \0)
  unsigned int size;         // overall size of header (XiffHeader) in bytes
  unsigned int format_type;  // format type. generic
  unsigned char specific_mag[4];      // The sub header magic
  unsigned int specific_size;         // length of the format data
  unsigned int specific_format_type;  // format type. generic
};

/* video file */
#define kXIFFVideo "XVFF"

/* audio file */
#define kXIFFAudio "XAFF"

/* install buddy program. */
#define kXIFFInstaller "XnFF"

/* generic container */
#define kXIFFGeneric "XIFF"

/* binary with metadata. */
#define kXIFFFlatBinary "XEFF"
