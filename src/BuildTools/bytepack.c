#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char formated_string[1024] = {0};
char c_file_path[200] = {0};
char h_file_path[200] = {0};
char header_guard_text[200] = {0};

void get_file_name(const char *filepath, char *out, size_t out_buf_len) {
  size_t start = 0;
  size_t length = strlen(filepath);
  for (intptr_t i = length - 1; i >= 0; i--) {
    if (filepath[i] == '/'
#ifdef _WIN32
        || filepath[i] == '\\'
#endif
    ) {
      start = i + 1;
      break;
    }
  }

  // memcpy(out, &filepath[start], length - start);
  // out[length] = '\0';
  snprintf(out, out_buf_len, "%s", &filepath[start]);
}

void name2cident(char *name, char *out) {
  char *c = name;
  while (*c != '\0') {
    *out = *c;
    if ((*c < '0' || *c > '9') && (*c < 'a' || *c > 'z') && (*c < 'A' || *c > 'Z'))
      *out = '_';
    c++;
    out++;
  }
}

int read_file(const char *filepath, unsigned char **blob, size_t *bytes_read) {
  FILE *input = fopen(filepath, "rb");

  if (!input) {
    fprintf(stderr, "bytepack: Couldnt open file %s to reading.\n", filepath);
    return 1;
  }

  if (fseek(input, 0, SEEK_END)) {
    fprintf(stderr, "bytepack: fseek failed on file %s\n", filepath);
    *bytes_read = 0;
    *blob = NULL;
    return 0;
  }
  *bytes_read = ftell(input);
  fseek(input, 0, SEEK_SET);

  *blob = calloc(1, *bytes_read + 1);
  fread(*blob, *bytes_read, 1, input);
  fclose(input);

  fprintf(stderr, "bytepacker: %ld bytes read from %s\n", *bytes_read, filepath);
  return 0;
}

int main(int argc, char **argv) {
  if (argc < 3) {
    fprintf(stderr, "bytepack: Not enough arguments\n");
    return 1;
  }

  snprintf(c_file_path, 200, "%s.c", argv[1]);
  FILE *cfile = fopen(c_file_path, "wb");
  snprintf(h_file_path, 200, "%s.h", argv[1]);
  FILE *hfile = fopen(h_file_path, "wb");

  if (!cfile) {
    fprintf(stderr, "bytepack: Cannot open file %s for writing\n", c_file_path);
    return 1;
  }

  if (!hfile) {
    fprintf(stderr, "bytepack: Cannot open file %s for writing\n", h_file_path);
    return 1;
  }

  get_file_name(h_file_path, formated_string, 1024);
  fprintf(cfile, "#include \"%s\"\n\n", formated_string);
  name2cident(formated_string, formated_string);
  fprintf(hfile,
          "#ifndef %s\n"
          "#define %s\n"
          "#include <stdlib.h>\n\n",
          formated_string, formated_string);

  unsigned char *blob;
  size_t bytes_read;
  for (int i = 2; i < argc; i++) {
    if (read_file(argv[i], &blob, &bytes_read)) {
      fprintf(stderr, "bytepack: skipped file %s\n", argv[i]);
      continue;
    }

    get_file_name(argv[i], formated_string, 1024);
    name2cident(formated_string, formated_string);
    fprintf(hfile,
            "extern const size_t %s_size;\n"
            "extern const unsigned char %s_data[%ld];\n\n",
            formated_string, formated_string, bytes_read + 1);

    fprintf(cfile,
            "const size_t %s_size = %ld;\n"
            "const unsigned char %s_data[%ld] = {",
            formated_string, bytes_read, formated_string, bytes_read + 1);

    unsigned char *end = &blob[bytes_read]; //since last byte is null terminator (in case its c style string)
    for (unsigned char *uc = blob; uc <= end; uc++) {
      fprintf(cfile, "0x%0.2x, ", *uc);
    }
    fprintf(cfile, "};\n\n");

    free(blob);
  }

  fprintf(hfile, "#endif");
  fclose(hfile);
  fclose(cfile);

  fprintf(stderr, "bytepack: \x1b[32msuccesfully wrote files \x1b[34m%s\x1b[32m and \x1b[34m%s\x1b[0m\n", c_file_path, h_file_path);

  return 0;
}
