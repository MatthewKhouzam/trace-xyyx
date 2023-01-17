#include "ctf.h"
#include "metadata.h"
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <threads.h>
#include <time.h>
#include <unistd.h>

unsigned long start_time = 0;
unsigned long end_time = 0;
char trace_folder[256];

_Thread_local int pos = HEADER_SIZE;
_Thread_local unsigned char packet[PACKET_SIZE];

_Thread_local struct timespec now;

int add_event(unsigned char *packet, int position, int entry,
              unsigned long time, long address, unsigned char tid,
              unsigned char pid);

int add_event_2(int type, long address) {
  timespec_get(&now, TIME_UTC);
  unsigned long time = now.tv_sec;
  time *= 1e9;
  time += now.tv_nsec;
  pos = add_event(packet, pos, type, time, address, 0, 0); // get TID and PID if we're doing that
  return pos;
}

int trace_entry(unsigned long address) { return add_event_2(0, address); }

int trace_exit(unsigned long address) { return add_event_2(1, 0); }

void init_trace(char *folder) {
  struct stat st = {0};
  strcpy(trace_folder, folder);
  if (stat(folder, &st) == -1) {
    mkdir(folder, 0700);
  }

  char file_name[200];
  strcpy(file_name, "./");
  strcat(file_name, folder);
  strcat(file_name, "/metadata");
  FILE *fp = fopen(file_name, "w");
  fputs(metadata, fp); // warning because xxd makes an unsigned char[] instead of char[]
  fclose(fp);
  strcpy(file_name, "./");
  strcat(file_name, folder);
  strcat(file_name, "/stream"); // Need to add ThreadID if we're doing that
  remove(file_name);
}

void write_packet(char *folder, unsigned char *packet, int size, long lost,
                  long cpu) {
  FILE *stream_file;
  char file_name[200];
  strcpy(file_name, "./");
  strcat(file_name, folder);
  strcat(file_name, "/stream"); // Need to add ThreadID if we're doing that
  stream_file = fopen(file_name, "aw");
  // Magic number
  int position = 0;
  packet[position++] = 0xc1;
  packet[position++] = 0x1f;
  packet[position++] = 0xfc;
  packet[position++] = 0xc1;
  // UUID
  for (int i = 0; i < 16; i++) {
    packet[position++] = 0xaa;
  }
  // stream ID
  packet[position++] = 0;
  packet[position++] = 0;
  packet[position++] = 0;
  packet[position++] = 0;
  // Start time
  for (int i = 0; i < 8; i++) {
    packet[position++] = start_time & 0xff;
    start_time = start_time >> 8;
  }
  // End time
  for (int i = 0; i < 8; i++) {
    packet[position++] = end_time & 0xff;
    end_time = end_time >> 8;
  }
  int content = size * 8;
  int payload = PACKET_SIZE * 8;
  // content
  for (int i = 0; i < 8; i++) {
    packet[position++] = content & 0xff;
    content = content >> 8;
  }
  // size
  for (int i = 0; i < 8; i++) {
    packet[position++] = payload & 0xff;
    payload = payload >> 8;
  }
  // lost events and CPU
  for (int i = 0; i < 12; i++) {
    packet[position++] = 0;
  }
  if (!fwrite(packet, 1, PACKET_SIZE, stream_file)) {
    printf("Error on write packet");
  }
  fclose(stream_file);
}

int add_event(unsigned char *packet, int position, int entry,
              unsigned long time, long address, unsigned char tid,
              unsigned char pid) {
  if (position > 65000) {
    write_packet(trace_folder, packet, position, 0, 0);
    memset(packet, 0, PAYLOAD_SIZE);
    position = HEADER_SIZE;
  }
  if (start_time == 0) {
    start_time = time;
  }
  if (time > end_time) {
    end_time = time;
  }
  packet[position++] = entry != 0;
  for (int i = 0; i < 8; i++) {
    packet[position++] = 0xff & time;
    time = time >> 8;
  }
  if (entry == 0) {
    for (int i = 0; i < 8; i++) {
      packet[position++] = 0xff & address;
      address = address >> 8;
    }
  }
  for (int i = 0; i < 1; i++) {
    packet[position++] = 0xff & tid;
    tid = tid >> 8;
  }
  for (int i = 0; i < 1; i++) {
    packet[position++] = 0xff & pid;
    pid = pid >> 8;
  }
  return position;
}

void flush() { write_packet(trace_folder, packet, pos, 0, 0); }