#define HEADER_SIZE  68
#define PACKET_SIZE  65536
#define PAYLOAD_SIZE PACKET_SIZE - HEADER_SIZE

/*
 * Initialize the trace, give it a folder destination
 */
void init_trace(char *folder);

/*
 * Trace entry, used to say "I entered a span"
 *
 * @param address used to identify the span
 */
int trace_entry(unsigned long address);

/*
 * Trace exit, used to say "I finished a span"
 *
 * @param address used to identify the span (unused in the trace to save space)
 */
int trace_exit(unsigned long address);

/*
 * Flush, must be called at the end to write the last elements to disk.
 */
void flush();