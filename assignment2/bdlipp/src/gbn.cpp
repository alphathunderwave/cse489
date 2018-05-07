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
#include <vector>
#include <cstdlib>
#include <string.h>
#include <cstring>
#include <stdio.h>

int window_size;
int base;
int next_seqnum;
int expected_seqnum;
char empty[20] = "";
float interrupttime;
std::vector<pkt> buffer;

int make_checksum(struct pkt packet)
{
  int checksum = 0;
  checksum += packet.seqnum;
  checksum += packet.acknum;

  for (int i = 0; i < sizeof(packet.payload); i++)
  {
    checksum += packet.payload[i];
  }

  return checksum;
}

struct pkt make_packet(int seq, int ack, struct msg message)
{
  struct pkt packet;
  packet.seqnum = seq;
  packet.acknum = ack;
  strcpy(packet.payload,message.data);
  packet.checksum = make_checksum(packet);

  return packet;
}


/* called from layer 5, passed the data to be sent to other side */
void A_output(struct msg message)
{
  printf("A_output: \n");
  printf("     recieved message: %.20s\n",message.data );

  if(next_seqnum < base + window_size)
  {
    struct pkt* out_packet = (struct pkt*) malloc(sizeof(struct pkt));
    out_packet->seqnum = next_seqnum;
    out_packet->acknum = 0;
    strcpy(out_packet->payload, message.data);
    out_packet->checksum = make_checksum(*out_packet);
    buffer.push_back(*out_packet);
    tolayer3(0, buffer[next_seqnum-1]);
    if(base = next_seqnum)
    {
      printf("     buffer is empty, sending message\n");
      starttimer(0, interrupttime);
    }
    next_seqnum++;
  } else {
    printf("     adding to buffer\n");
    struct pkt* out_packet = (struct pkt*) malloc(sizeof(struct pkt));
    out_packet->seqnum = next_seqnum;
    out_packet->acknum = 0;
    strcpy(out_packet->payload, message.data);
    out_packet->checksum = make_checksum(*out_packet);
    buffer.push_back(*out_packet);
  }
}

/* called from layer 3, when a packet arrives for layer 4 */
void A_input(struct pkt packet)
{
  printf("A_input: \n");

  if(make_checksum(packet) == packet.checksum && base)
  {
    base = packet.acknum+1;
    if(base == next_seqnum)
    {
      stoptimer(0);
    } else {
      starttimer(0, interrupttime);
    }
  }
}

/* called when A's timer goes off */
void A_timerinterrupt()
{
  printf("A_timerinterrupt: \n");

  starttimer(0, interrupttime);
  for(int i = base; i < next_seqnum; i++)
  {
    tolayer3(0, buffer[i-1]);
  }
}

/* the following routine will be called once (only) before any other */
/* entity A routines are called. You can use it to do any initialization */
void A_init()
{
  printf("A_init: \n");

  next_seqnum = 1;
  base = 1;
  window_size = getwinsize();
  interrupttime = 20.0;
}

/* Note that with simplex transfer from a-to-B, there is no B_output() */

/* called from layer 3, when a packet arrives for layer 4 at B*/
void B_input(struct pkt packet)
{
  printf("B_input: \n");

  if(make_checksum(packet) == packet.checksum && expected_seqnum == packet.seqnum)
  {
    printf("          OUTPUT: %.20s\n", packet.payload);
    printf("          BUFFER: ");
    for (size_t i = 0; i < buffer.size(); i++) {
      printf("%c", buffer[(i)].payload[0] );
    }
    printf("\n");

    tolayer5(1, packet.payload);
    struct pkt* ACKpkt = (struct pkt*) malloc(sizeof(struct pkt));
    ACKpkt->seqnum = expected_seqnum;
    ACKpkt->acknum = expected_seqnum;
    strcpy(ACKpkt->payload, empty);
    ACKpkt->checksum = make_checksum(*ACKpkt);
    tolayer3(1, *ACKpkt);
    expected_seqnum++;

  }
  else
  {
    struct pkt* ACKpkt = (struct pkt*) malloc(sizeof(struct pkt));
    ACKpkt->seqnum = expected_seqnum;
    ACKpkt->acknum = expected_seqnum;
    strcpy(ACKpkt->payload, empty);
    ACKpkt->checksum = make_checksum(*ACKpkt);
    tolayer3(1, *ACKpkt);
  }
}

/* the following rouytine will be called once (only) before any other */
/* entity B routines are called. You can use it to do any initialization */
void B_init()
{
  printf("B_init: \n");

    expected_seqnum = 1;
}
