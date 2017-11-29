/*
 * Command.cpp
 *
 * based on: SQ_Command.cpp
 *  Created on: Mar 29, 2014
 *      Author: Kees Bakker
 */


#include <stdint.h>
#include <Arduino.h>
#include "Command.h"

int Command::findCommand(const Command args[], uint8_t nr, const char *line)
{
  for (uint8_t i = 0; i < nr; ++i) {
    const Command *a = &args[i];
    if (strncasecmp(line, a->cmd_prefix, strlen(a->cmd_prefix)) == 0) {
      return i;
    }
  }
  return -1;
}

/*
 * Execute a command from the commandline
 *
 * Return true if it was a valid command
 */
bool Command::execCommand(const Command args[], uint8_t nr, const char * line)
{
  int ix = findCommand(args, nr, line);
  if (ix < 0) {
    return false;
  }
  const Command *s = &args[ix];
  if (s->exec_func) {
    s->exec_func(s, line + strlen(s->cmd_prefix));
  }
  return true;
}

void Command::set_string(const Command *s, const char *line)
{
  char *ptr = (char *)s->value;
  if (ptr && s->param_size > 0) {
    memset(ptr, 0, s->param_size);

    // Strip off leading white space
    while (*line == ' ' || *line == '\t') {
      ++line;
    }

    size_t len = strlen(line);
    if (len > s->param_size - 1) {
      len = s->param_size - 1;
    }

    // Strip off trailing white space
    while (len > 0 && (line[len - 1] == ' ' || line[len - 1] == '\t')) {
      --len;
    }

    if (len > 0) {
      strncpy(ptr, line, len);
    }
  }
}

void Command::set_uint8(const Command *s, const char *line)
{
  uint8_t *ptr = (uint8_t *)s->value;
  if (ptr) {
    *ptr = strtoul(line, NULL, 0);
  }
}

void Command::set_int16(const Command *s, const char *line)
{
  int16_t *ptr = (int16_t *)s->value;
  if (ptr) {
    *ptr = strtol(line, NULL, 0);
  }
}

void Command::set_uint16(const Command *s, const char *line)
{
  uint16_t *ptr = (uint16_t *)s->value;
  if (ptr) {
    *ptr = strtoul(line, NULL, 0);
  }
}

void Command::set_uint32(const Command *s, const char *line)
{
  uint32_t *ptr = (uint32_t *)s->value;
  if (ptr) {
    *ptr = strtoul(line, NULL, 0);
  }
}

void Command::show_name(Stream * stream) const
{
  // reinterpret_cast<const __FlashStringHelper *>(ptr)
  stream->print("  ");
  stream->print(name);
  stream->print(" (");
  stream->print(cmd_prefix);
  stream->print("): ");
}

void Command::show_string(const Command *s, Stream * stream)
{
  char *ptr = (char *)s->value;
  s->show_name(stream);
  if (ptr) {
    stream->println(ptr);
  } else {
    stream->println();
  }
}

void Command::show_uint8(const Command *s, Stream * stream)
{
  uint8_t *ptr = (uint8_t *)s->value;
  if (ptr) {
    s->show_name(stream);
    stream->println(*ptr);
  }
}

void Command::show_int16(const Command *s, Stream * stream)
{
  int16_t *ptr = (int16_t *)s->value;
  if (ptr) {
    s->show_name(stream);
    stream->println(*ptr);
  }
}

void Command::show_uint16(const Command *s, Stream * stream)
{
  uint16_t *ptr = (uint16_t *)s->value;
  if (ptr) {
    s->show_name(stream);
    stream->println(*ptr);
  }
}

void Command::show_uint32(const Command *s, Stream * stream)
{
  uint32_t *ptr = (uint32_t *)s->value;
  if (ptr) {
    s->show_name(stream);
    stream->println(*ptr);
  }
}
