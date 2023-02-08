// for glorious rpi-picos!

#include "ctf.h"
#include "hardware/rtc.h"
#include "hardware/timer.h"
#include "hardware/uart.h"
#include "metadata.h"
#include "pico/multicore.h"
#include "pico/stdlib.h"
#include "pico/time.h"
#include "pico/types.h"
#include "pico/util/queue.h"
#include <stdio.h>
#include <string.h>

#define UART_ID uart1

unsigned long start_time = 0;
unsigned long end_time = 0;
char trace_folder[256];

int pos = HEADER_SIZE;
unsigned char packet[PACKET_SIZE];

int add_event(unsigned char *packet, int position, int entry,
              unsigned long time, long address, unsigned char tid,
              unsigned char pid);

int add_event_2(int type, long address) {

  unsigned long time = get_absolute_time() * 1000; // it's in us, we want ns
  pos = add_event(packet, pos, type, time, address, 0,
                  0); // get TID and PID if we're doing that
  return pos;
}

int trace_entry(unsigned long address) { return add_event_2(0, address); }

int trace_exit(unsigned long address) { return add_event_2(1, 0); }

queue_t trace_queue;

typedef struct {
  char name[16];
  unsigned char *packet;
  int size;
} queue_entry_t;

void core1_entry() {
  uart_init(UART_ID, 115200);
  uart_set_translate_crlf(UART_ID, 0);
  while (1) {
    queue_entry_t entry;
    queue_remove_blocking(&trace_queue, &entry);
    // write to serial here
    uart_write_blocking(UART_ID, entry.name, 16);
    
    
    int size = entry.size;
    for (int i = 0; i < 4; i++) {
      uart_putc(UART_ID, size & 0xff);
      size = size >> 8;
    }
    uart_write_blocking(UART_ID, entry.packet, entry.size);
  }
}

void init_trace(char *folder) {
  stdio_init_all();
  stdio_usb_init();
  queue_entry_t entry;
  queue_init(&trace_queue, sizeof(queue_entry_t), 2);
  strcpy(entry.name, "metadata");
  entry.packet = metadata;
  entry.size = metadata_len;
  queue_add_blocking(&trace_queue, &entry);
}

void write_packet(char *folder, unsigned char *packet, int size, long lost,
                  long cpu) {
  queue_entry_t entry;
  strcpy(entry.name, "stream");
  entry.size = size;
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
  unsigned char tx_packet[PACKET_SIZE];
  memcpy(tx_packet, packet, PACKET_SIZE);
  queue_add_blocking(&trace_queue, &entry);
}

int add_event(unsigned char *packet, int position, int entry,
              unsigned long time, long address, unsigned char tid,
              unsigned char pid) {
  if (position > PACKET_SIZE * 0.99) { // make sure not to overrun
    write_packet(trace_folder, packet, position, 0,
                 0); // always on CPU zero for a pico pi! Also only one thread!
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

void flush() {
  write_packet(trace_folder, packet, pos, 0, 0);
  // join here
}
