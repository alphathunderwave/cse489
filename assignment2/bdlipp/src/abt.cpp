#include "../include/simulator.h"

/* ******************************************************************
 ALTERNATING BIT AND GO-BACK-N NETWORK EMULATOR: VERSION 1.1  J.F.Kurose

   This code should be used for PA2, unidirectional data transfer
   protocols (from A to B). Network properties:
   - one way network delay averages five time units (longer if there
     are other messages in the channel for GBN), but can be larger
   - packets can be corrupted (either the header or the data portion)
     or lost, according to user-defined probabilities
   - packets will be delivered in the order in which they were sent
     (although some can be lost).
**********************************************************************/

/********* STUDENTS WRITE THE NEXT SEVEN ROUTINES *********/
#include <string.h>
#include <stdio.h>
#define MESSAGESIZE 20
#define BUFFERSIZE 300
#define INTERRUPTTIME 30

int a_seqnum;
int b_seqnum;

int buffer_head;
struct msg buffer[BUFFERSIZE];
struct msg empty_msg;

struct pkt a_packet;

int make_checksum(int seq, int ack, char *message)
{
  int checksum = seq+ack;
  for (size_t i = 0; i < MESSAGESIZE; i++) {
    checksum+=message[i];
  }
  return checksum;
}
struct pkt make_packet(int seq, int ack, struct msg message)
{
  struct pkt packet;
  packet.seqnum = seq;
  packet.acknum = ack;
  packet.checksum = make_checksum(seq,ack,message.data);
  strcpy(packet.payload,message.data);
  return packet;
}

int is_valid(struct pkt packet)
{
  if (packet.checksum == make_checksum(packet.seqnum,packet.acknum,packet.payload)) {
    printf("    valid packet\n");
    return 0;}
  else return 1;
}

int flip_seq(int seq)
{
  if(seq==0) return 1;
  else return 0;
}

void buffer_add(struct msg message)
{
  printf("          adding mess to buffer: %c\n", message.data[0]);
  for(int i = 0; i < BUFFERSIZE; i++)
  {
    if(buffer[(buffer_head + i)%BUFFERSIZE].data[0] == '\0')
    {
      buffer[(buffer_head + i)%BUFFERSIZE] = message;
      break;
    }
  }
}

struct msg pop_buffer()
{
  printf("         popping buffer_head: %c\n", buffer[buffer_head%BUFFERSIZE].data[0]);
  struct msg message = buffer[buffer_head];
  buffer[buffer_head].data[0] = '\0';
  buffer_head = (1 + buffer_head)%BUFFERSIZE;
  return message;
}

struct msg* view_buffer()
{
  if(buffer[buffer_head].data[0] == '\0') return &empty_msg;
  else return &(buffer[buffer_head]);
}

void send_message(struct msg message)
{
  struct pkt packet = make_packet(a_seqnum,a_seqnum,message);
  a_packet = packet;
  starttimer(0,INTERRUPTTIME);
  tolayer3(0,packet);
}

/* called from layer 5, passed the data to be sent to other side */
void A_output(struct msg message)
{
  printf("A_output:\n");
  printf("     recieved message: %.20s\n",message.data );
  if(view_buffer()==&empty_msg)
  {
    printf("     buffer is empty, sending message\n");
    buffer_add(message);
    send_message(message);
  }
  else
  {
    printf("     adding to buffer\n");
    buffer_add(message);
  }
}

/* called from layer 3, when a packet arrives for layer 4 */
void A_input(struct pkt packet)
{
  printf("A_input:\n");
  if(packet.acknum != a_seqnum || is_valid(packet)){
    printf("     not valid\n");
  }
  else
  {
    a_seqnum = flip_seq(a_seqnum);

    pop_buffer();
    stoptimer(0);
    if(view_buffer() != &empty_msg) send_message(*view_buffer());
  }
}

/* called when A's timer goes off */
void A_timerinterrupt()
{
  printf("A_timerinterrupt:\n" );
  tolayer3(0,a_packet);
  starttimer(0,INTERRUPTTIME);

}

/* the following routine will be called once (only) before any other */
/* entity A routines are called. You can use it to do any initialization */
void A_init()
{
  printf("A_init:\n");
  a_seqnum = 0;
  buffer_head = 0;
}

/* Note that with simplex transfer from a-to-B, there is no B_output() */

/* called from layer 3, when a packet arrives for layer 4 at B*/
void B_input(struct pkt packet)
{
  printf("B_input:\n" );
  int seq;
  int ack;
  if(packet.seqnum != b_seqnum || is_valid(packet))
  {
    seq = flip_seq (b_seqnum);
    ack = seq;
  }
  else
  {
    seq = b_seqnum;
    ack = seq;
    b_seqnum = flip_seq(b_seqnum);
    printf("          OUTPUT: %.20s\n", packet.payload);
    printf("          BUFFER: ");
    for (size_t i = 0; i < BUFFERSIZE; i++) {
      printf("%c", buffer[(i)].data[0] );
    }
    printf("\n");
    tolayer5(1,packet.payload);
  }
  struct msg empty;
  for (size_t i = 0; i < MESSAGESIZE; i++) {
    empty.data[i] = ' ';
  }
  struct pkt ACKpkt = make_packet(seq,ack,empty);
  tolayer3(1,ACKpkt);
}

/* the following rouytine will be called once (only) before any other */
/* entity B routines are called. You can use it to do any initialization */
void B_init()
{
  printf("B_init:\n");
  b_seqnum =0;
}
