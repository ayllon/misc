#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

size_t encode(long ints[], size_t nints,
              unsigned char *buffer, size_t bufsize)
{
  size_t        i, bufpos;
  int64_t       val;
  unsigned char byte;

  memset(buffer, 0, sizeof(buffer));

  bufpos = 0;
  for (i = 0; i < nints && bufpos < bufsize; ++i) {
    val = ints[i];
    do {
      byte = val & 0x7F;
      val >>= 7;
      if (val)
        byte |= 0x80;
      buffer[bufpos++] = byte;
    } while (val && bufpos < bufsize);
  }

  if (bufpos == bufsize)
    return 0;

  return bufpos;
}

size_t decode(const unsigned char *data, size_t nbytes,
              long *out, size_t outsize)
{
  size_t        i, outp;
  long          val;
  unsigned char byte, shift;

  outp = 0;
  for (i = 0; i < nbytes && outp < outsize; ) {
    val   = 0;
    shift = 0;
    do {
      byte = data[i++];
      val += ((byte & 0x7F) << shift);
      if (!(byte & 0x80))
        break;
      shift += 7;
    } while (i < nbytes);
    out[outp++] = val; 
  }

  return outp;
}

static uint8_t base64_digit(uint8_t value)
{
  if      (value < 26)  return 'A' + value;
  else if (value < 52)  return 'a' + value - 26;
  else if (value < 62)  return '0' + value - 52;
  else if (value == 62) return '+';
  else if (value == 63) return '/';
  else                  abort(); 
}

size_t base64_encode(const unsigned char *in, size_t inbytes, unsigned char *out, size_t outsize)
{
  size_t ii, oi, padding;
 
  for (ii = 0, oi = 0; ii < inbytes && oi < outsize; ) {
    uint8_t byte1 = ii < inbytes ? in[ii++] : 0;
    uint8_t byte2 = ii < inbytes ? in[ii++] : 0;
    uint8_t byte3 = ii < inbytes ? in[ii++] : 0;

    uint32_t triplet = (byte1 << 16) | (byte2 << 8) | (byte3);

    if (oi < outsize) out[oi++] = base64_digit((triplet >> 18) & 0x3F);
    if (oi < outsize) out[oi++] = base64_digit((triplet >> 12) & 0x3F);
    if (oi < outsize) out[oi++] = base64_digit((triplet >>  6) & 0x3F);
    if (oi < outsize) out[oi++] = base64_digit((triplet) & 0x3F);
  }

  if (oi >= outsize) return 0;

  padding = (3 - (inbytes % 3)) % 3;

  for (ii = 0; ii < padding; ++ii) {
    out[oi - 1 - ii] = '=';
  }

  if (oi < outsize) out[oi++] = '\0';
  return oi; 
}

static uint8_t base64_value(uint8_t digit)
{
  errno = 0;
  if      (digit >= 'A' && digit <= 'Z') return digit - 'A';
  else if (digit >= 'a' && digit <= 'z') return digit - 'a' + 26;
  else if (digit >= '0' && digit <= '9') return digit - '0' + 52;
  else if (digit == '+')                 return 62;
  else if (digit == '/')                 return 63;
  else {
    errno = EINVAL;
    return 0;
  }
}

size_t base64_decode(const unsigned char *in, unsigned char *out, size_t outsize)
{
  size_t ii, oi;
  
  for (ii = 0, oi = 0; in[ii] != '\0' && oi < outsize; ) {
    uint8_t  digit;
    uint32_t triplet;

    triplet = 0;
    do {
      digit   = in[ii++];
      if (digit == '=') break;
      triplet = (triplet << 6) | base64_value(digit);
    } while (in[ii] != '\0' && ii % 4);

    if (digit == '=' && in[ii] == '=') {
       out[oi++] = (triplet >> 4) & 0xFF;
       break;
    }
    else if (digit == '=') {
      out[oi++] = (triplet >> 10) & 0xFF;
      out[oi++] = (triplet >>  2) & 0xFF;
      break;
    }
    else {
      out[oi++] = (triplet >> 16) & 0xFF;
      out[oi++] = (triplet >>  8) & 0xFF;
      out[oi++] = (triplet      ) & 0xFF;
    }
  }

  return oi;
}


#define ARRAY_LEN 10

int main(int argc, char *argv[])
{
  long          array[ARRAY_LEN];
  long          validate[ARRAY_LEN];
  unsigned char buffer[512], base64[1024], buffer2[512];
  size_t        i, n, n2;

  if (argc < 2) {
    printf("Using time as seed.\n");
    srandom(time(NULL));
  }
  else {
    int seed = atoi(argv[1]);
    printf("Using %d as seed.\n", seed);
    srandom(seed);
  }

  for (i = 0; i < ARRAY_LEN; ++i)
    array[i] = random();

  n = encode(array, ARRAY_LEN,
             buffer, sizeof(buffer));

  printf("Space used: %lu (original %lu)\n", n, sizeof(array));

  /* Validate */
  n2 = decode(buffer, n, validate, ARRAY_LEN);
  if (n2 != ARRAY_LEN) {
    printf("Expected %u elements, got %lu\n", ARRAY_LEN, n2);
    return 1;
  }

  for (i = 0; i < n2; ++i) {
    if (array[i] != validate[i]) {
      printf("Position %lu: expected %ld, got %ld\n", i, array[i], validate[i]);
      return 1;
    }
  }
  printf("Encode/Decode OK\n");

  /* Base 64 */
  n2 = base64_encode(buffer, n, base64, sizeof(base64)); 
  if (n2 > 0) {
    base64[n2] = '\0';
    printf("Base 64 encoding (%ld characters): %s\n", n2, base64);
  }
  else {
    printf("Buffer size too small\n");
    return 1;
  }

  /* Digits */
  for (i = 0; i < 64; ++i) {
    assert(base64_value(base64_digit(i)) == i);
  }

  /* Decode */
  n2 = base64_decode(base64, buffer2, sizeof(buffer2));
  if (n2 != n) {
    printf("Expected %d bytes, got %d\n", n, n2);
    return 1;
  }

  for (i = 0; i < n2; ++i) {
    if (buffer2[i] != buffer[i]) {
      printf("Position %lu: expected %u, got %u\n", i, buffer[i], buffer2[i]);
      return 1;
    }
  }

  printf("Base64 encode/decode OK\n");

  return 0;
}

