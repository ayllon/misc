#include <stdio.h>
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
  if (value < 26)       return 'A' + value;
  else if (value < 52)  return 'a' + value - 26;
  else if (value < 62)  return '0' + value - 52;
  else if (value == 62) return '+';
  else if (value == 63) return '/';
  else                  abort(); 
}


int base64_encode(const unsigned char *in, size_t inbytes, unsigned char *out, size_t outsize)
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

  out[oi++] = '\0';
  return oi; 
}

#define ARRAY_LEN 10

int main(int argc, char *argv[])
{
  long          array[ARRAY_LEN];
  long          validate[ARRAY_LEN];
  unsigned char buffer[512], base64[1024];
  size_t        i, n, n2;

  srandom(time(NULL));
  for (i = 0; i < ARRAY_LEN; ++i)
    array[i] = random();

  n = encode(array, ARRAY_LEN,
             buffer, sizeof(buffer));

  for (i = 0; i < n; ++i) {
    if (i % 8 == 0)
      printf("%02lu => %02lu\t", i, i + 7);
    
    printf("0x%02X ", buffer[i]);
    
    if (i % 8 == 7)
      printf("\n");
  }
  printf("\n");
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
  printf("All OK :)\n");

  /* Base 64 */
  n2 = base64_encode(buffer, n, base64, sizeof(base64)); 
  base64[n2] = '\0';
  printf("Base 64 encoding (%ld characters): %s\n", n2, base64);

  return 0;
}

