#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <time.h>
#include <pthread.h>
#include <stdlib.h>

int msleep (long msec);

// Used for socket-operation error codes.
int lock = 0;
int ErrorCodes = 0;
int IoErrorCount = 0;
int Last11 = 0;
int R1Sign = 0;
int R2Sign = 0;
int R3Sign = 0;
typedef enum
{ false, true } bool;

char DefaultHostname[] = "localhost";
char *Hostname = DefaultHostname;
int ServerSocket = -1;
int Portnum = 19697;

// Matches image widget filenames to channel 010 CCCCC and DDDDD fields.
const int SevenSegmentFilenames[32] =
  { 0, 0xff, 0xff, 1, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff,
  4, 0xff, 0xff, 0xff, 7, 0xff, 0, 0xff, 0xff, 0xff, 2, 0xff, 3, 6, 8, 5, 9
};

typedef enum RegisterDigit
{
  MD1Digit,
  MD2Digit,
  VD1Digit,
  VD2Digit,
  ND1Digit,
  ND2Digit,
//3 registers with 5 digits each

//R1 --Where memory reads show up
  R1D1Digit,
  R1D2Digit,
  R1D3Digit,
  R1D4Digit,
  R1D5Digit,
//R2
  R2D1Digit,
  R2D2Digit,
  R2D3Digit,
  R2D4Digit,
  R2D5Digit,
//R3 --Where memory read inputs show up
  R3D1Digit,
  R3D2Digit,
  R3D3Digit,
  R3D4Digit,
  R3D5Digit
} RegisterDigit;

#define DSKY_AGC_WARN 000001
#define DSKY_TEMP     000010
#define DSKY_KEY_REL  000020
#define DSKY_VN_FLASH 000040
#define DSKY_OPER_ERR 000100
#define DSKY_RESTART  000200
#define DSKY_STBY     000400
#define DSKY_EL_OFF   001000
#define PORT 19698
#define SOCKET_ERROR -1
#define SOCKET_BROKEN (errno == EPIPE)
#define PULSE_INTERVAL 80

//--------------------------------------------------------------------------------
// This function can take an i/o channel number and a 15-bit value for it, and
// constructs a 4-byte packet suitable for transmission to yaAGC via a socket.
// Space for the packet must have been allocated by the calling program.  
// Refer to the Virtual AGC Technical Manual, "I/O Specifics" subheading of the 
// "Developer Details" chapter.  Briefly, the 4 bytes are:
//      00pppppp 01pppddd 10dddddd 11dddddd
// where ppppppppp is the 9-bit channel number and ddddddddddddddd is the 15-bit
// value.  Finally, it transmits the packet. Returns 0 on success.
//
// ... Later, the 9-bit "Channel" is actually the u-bit plus an 8-bit channel
// number, but the function works the same.

int
FormIoPacket (int Channel, int Value, unsigned char *Packet)
{
  if (Channel < 0 || Channel > 0x1ff)
    return (1);
  if (Value < 0 || Value > 0x7fff)
    return (1);
  if (Packet == NULL)
    return (1);
  Packet[0] = Channel >> 3;
  Packet[1] = 0x40 | ((Channel << 3) & 0x38) | ((Value >> 12) & 0x07);
  Packet[2] = 0x80 | ((Value >> 6) & 0x3F);
  Packet[3] = 0xc0 | (Value & 0x3F);
  return (0);
}


//--------------------------------------------------------------------------------
// A nice little function to output a keycode (except PRO) to yaAGC.

int makeConnection ();

void
OutputKeycode (int Keycode)
{
  unsigned char Packet[4] = { 0 };
  int j = 0;

  //simulate human typing speed
  msleep (500);

  if (ServerSocket != -1)
    {
      FormIoPacket (015, Keycode, Packet);
      j = send (ServerSocket, (const char *) Packet, 4, MSG_NOSIGNAL);
      if (j == SOCKET_ERROR && SOCKET_BROKEN)
	{
	  close (ServerSocket);
	  ServerSocket = -1;
	}
    }
}

//--------------------------------------------------------------------------------
// This function is the opposite of FormIoPacket:  A 4-byte packet representing
// yaAGC channel i/o can be converted to an integer channel-number and value.
// Returns 0 on success.

int
ParseIoPacket (unsigned char *Packet, int *Channel, int *Value, int *uBit)
{
  // Pick the channel number and value from the packet.
  if (0x00 != (0xc0 & Packet[0]))
    return (1);
  if (0x40 != (0xc0 & Packet[1]))
    return (1);
  if (0x80 != (0xc0 & Packet[2]))
    return (1);
  if (0xc0 != (0xc0 & Packet[3]))
    return (1);
  *Channel = ((Packet[0] & 0x1F) << 3) | ((Packet[1] >> 3) & 7);
  *Value =
    ((Packet[1] << 12) & 0x7000) | ((Packet[2] << 6) & 0x0FC0) | (Packet[3] &
								  0x003F);
  *uBit = (0x20 & Packet[0]);
  return (0);
}

// Initialize the socket system.  Return 0 on success.
static int SocketSystemInitialized = 0;

int
InitializeSocketSystem (void)
{
  if (SocketSystemInitialized)
    return (0);
  SocketSystemInitialized = 1;
  return (0);
}

// Set an existing socket to be non-blocking.
void
UnblockSocket (int SocketNum)
{
  fcntl (SocketNum, F_SETFL, O_NONBLOCK);
}

//----------------------------------------------------------------------
// Function for creating a socket.  Copied from
// http://world.std.com/~jimf/papers/sockets/sockets.html, and then
// modified somewhat for my own purposes.  The parameters:
//
//      portnum         The port on which we're going to listen.
//      MaxClients      Max number of queued connections.  (5 is good.)
//
// Returns -1 on error, or the new socket number (>=0) if successful.

#define MAXHOSTNAME 256
int
EstablishSocket (unsigned short portnum, int MaxClients)
{
  char myname[MAXHOSTNAME + 1];
  int s, i;
  struct sockaddr_in sa;
  struct hostent *hp;

  InitializeSocketSystem ();

  memset (&sa, 0, sizeof (struct sockaddr_in));	/* clear our address */
  gethostname (myname, MAXHOSTNAME);	/* who are we? */
  hp = gethostbyname (myname);	/* get our address info */
  if (hp == NULL)		/* we don't exist !? */
    {
      char s[32];
      switch (h_errno)
	{
	case HOST_NOT_FOUND:
	  strcpy (s, "Host not found");
	  break;
	case NO_ADDRESS:
	  strcpy (s, "No address");
	  break;
	  //case NO_DATA: strcpy(s, "No data"); break;
	case NO_RECOVERY:
	  strcpy (s, "No recovery");
	  break;
	case TRY_AGAIN:
	  strcpy (s, "Try again");
	  break;
	default:
	  sprintf (s, "Error %d", h_errno);
	  break;
	}
      fprintf (stderr, "gethostbyname (\"%s\" %d) reports %s\n", myname,
	       portnum, s);
      ErrorCodes = 0x101;
      return (-1);
    }
  sa.sin_family = hp->h_addrtype;	/* this is our host address */
  sa.sin_port = htons (portnum);	/* this is our port number */
  if ((s = socket (AF_INET, SOCK_STREAM, 0)) < 0)	/* create socket */
    {
      ErrorCodes = 0x102;
      return (-1);
    }

  // Make sure to clean up after any previous disconnects of the
  // port.  Otherwise there would be a timeout until we could
  // reuse the port.
  i = 1;
  setsockopt (s, SOL_SOCKET, SO_REUSEADDR, (const char *) &i, sizeof (int));

  if (bind (s, (struct sockaddr *) &sa, sizeof (struct sockaddr_in)) < 0)
    {
      close (s);
      ErrorCodes = 0x103;
      return (-1);		/* bind address to socket */
    }
  listen (s, MaxClients);	/* max # of queued connects */
  // Don't want to wait when there's no incoming data.
  UnblockSocket (s);
  return (s);
}

//----------------------------------------------------------------------
// Client connection to server via socket.
// http://world.std.com/~jimf/papers/sockets/sockets.html.
// The hostname is the name of the server, either resolvable by DNS,
// or else a dotted IP number.  (The latter fails on Win32.)
// The portnum is the port-number on which the server listens.

int
CallSocket (char *hostname, unsigned short portnum)
{
  struct sockaddr_in sa;
  struct hostent *hp;
  //int a;
  int s;

  InitializeSocketSystem ();

  if ((hp = gethostbyname (hostname)) == NULL)
    {
      /* do we know the host's */
      //errno= ECONNREFUSED; /* address? */
      ErrorCodes = 0x301;
      return (-1);		/* no */
    }

  memset (&sa, 0, sizeof (sa));
  memcpy ((char *) &sa.sin_addr, hp->h_addr, hp->h_length);	/* set address */
  sa.sin_family = hp->h_addrtype;
  sa.sin_port = htons ((u_short) portnum);
  if ((s = socket (hp->h_addrtype, SOCK_STREAM, 0)) < 0)	/* get socket */
    {
      ErrorCodes = 0x302;
      return (-1);
    }
  if (connect (s, (struct sockaddr *) &sa, sizeof (sa)) < 0)
    {
      /* connect */
      close (s);
      ErrorCodes = 0x303;
      return (-1);
    }
  UnblockSocket (s);
  return (s);
}

#define BUF_SIZE 33
char buffer[BUF_SIZE] = { 0 };

char *
int2bin (int a, char *buffer, int buf_size)
{
  buffer += (buf_size - 1);

  for (int i = 31; i >= 0; i--)
    {
      *buffer-- = (a & 1) + '0';

      a >>= 1;
    }

  return buffer;
}

int
GetLeftDigit(int Value )
{
      int i = (Value >> 5) & 0x1F;
      return SevenSegmentFilenames[i];
}

int
GetRightDigit(int Value)
{
      int i = Value & 0x1F;
      return SevenSegmentFilenames[i];
}

char VERB_DIGITS[2];
char NOUN_DIGITS[2];
char REGISTER1_DIGITS[5];
char REGISTER2_DIGITS[5];
char REGISTER3_DIGITS[5];

void DisplayNounVerb(char *nounverbdigits)
{
     printf("%o %o\n", 
              nounverbdigits[0],
              nounverbdigits[1]);
}

void DisplayRegister(char *registerdigits)
{
     printf("%o %o %o %o %o\n", 
              registerdigits[0],
              registerdigits[1],
              registerdigits[2], 
              registerdigits[3],
              registerdigits[4]);
}

void
ActOnIncomingIO (unsigned char *Packet)
{
  int Channel = 0;
  int Value = 0;
  int uBit = 0;

  if (Packet != NULL)
    {
      // Check to see if the message has a yaAGC signature.  If not,
      // ignore it.  The yaAGC signature is 00 01 10 11 in the
      // 2 most-significant bits of the packet's bytes.  We are
      // guaranteed that the first byte is signed 00, so we don't
      // need to check it.
      if (0x40 != (Packet[1] & 0xc0) || 0x80 != (Packet[2] & 0xc0)
	  || 0xc0 != (Packet[3] & 0xc0))
	return;

      ParseIoPacket (Packet, &Channel, &Value, &uBit);
      //printf("Just read the packet Value is 0x%08X\n", Value);

    }
  else
    {
      printf ("NULL packet\n");
    }

  // Now take care of everything that's left.  Only a few channels are of interest to the
  // DSKY as far as input is concerned.
  if (Channel == 010)
    {
      // 7-segment display management.
      char *Sign = NULL;
      int Left = 0, Right = 0;
      int RSign = 0;
      switch (Value & 0x7800)
	{
	case 0x5800:		// AAAA=11D 
	  printf ("LMD1D, RMD2D\n");
	  Left = MD1Digit;
	  Right = MD2Digit;
	  break;
	case 0x5000:		// AAAA=10D 
	  printf ("LVERB D1D, RVERB D2D\n");
	  Left = VD1Digit;
          VERB_DIGITS[0] = GetLeftDigit(Value);
	  Right = VD2Digit;
          VERB_DIGITS[1] = GetRightDigit(Value);
	  break;
	case 0x4800:		// AAAA=9 
	  printf ("LNOUN D1D, RNOUN D2D\n");
	  Left = ND1Digit;
          NOUN_DIGITS[0] = GetLeftDigit(Value);
	  Right = ND2Digit;
          NOUN_DIGITS[1] = GetRightDigit(Value);
	  break;
	case 0x4000:		// AAAA=8 //register one
	  printf ("RR1D1\n");
	  Right = R1D1Digit;
          REGISTER1_DIGITS[0]= GetRightDigit(Value);
	  break;

	case 0x3800:		// AAAA=7
	  //Sign = MainWindow->R1PlusMinus; //register one sign
	  printf ("LR1D2, RR1D3\n");

	  if (0 != (Value & 0x0400))
	    R1Sign |= 2;
	  else
	    R1Sign &= ~2;

	  RSign = R1Sign;
	  Left = R1D2Digit;
          REGISTER1_DIGITS[1] = GetLeftDigit(Value);
	  Right = R1D3Digit;
          REGISTER1_DIGITS[2] = GetRightDigit(Value);

	  break;
  
        //Fixed memory read results appear in Register 1
	case 0x3000:		// AAAA=6
	  printf ("LR1D4, RR1D5\n");
	  //Sign = MainWindow->R1PlusMinus;
	  if (0 != (Value & 0x0400))
	    R1Sign |= 1;
	  else
	    R1Sign &= ~1;
	  RSign = R1Sign;
	  Left = R1D4Digit;
          REGISTER1_DIGITS[3] = GetLeftDigit(Value);
	  Right = R1D5Digit;
          REGISTER1_DIGITS[4] = GetRightDigit(Value);
	  break;

	case 0x2800:		// AAAA=5
	  printf ("LR2D1, RR2D2\n");
	  //Sign = MainWindow->R2PlusMinus;
	  if (0 != (Value & 0x0400))
	    R2Sign |= 2;
	  else
	    R2Sign &= ~2;
	  RSign = R2Sign;
	  Left = R2D1Digit;
          REGISTER2_DIGITS[0] = GetLeftDigit(Value);
	  Right = R2D2Digit;
          REGISTER2_DIGITS[1] = GetRightDigit(Value);
	  break;

	case 0x2000:		// AAAA=4
	  printf ("LR2D3, RR2D4\n");
	  //Sign = MainWindow->R2PlusMinus;
	  if (0 != (Value & 0x0400))
	    R2Sign |= 1;
	  else
	    R2Sign &= ~1;
	  RSign = R2Sign;
	  Left = R2D3Digit;
          REGISTER2_DIGITS[2] = GetLeftDigit(Value);
	  Right = R2D4Digit;
          REGISTER2_DIGITS[3] = GetRightDigit(Value);
	  break;

	case 0x1800:		// AAAA=3
	  printf ("LR2D5, RR3D1\n");
	  Left = R2D5Digit;
          REGISTER2_DIGITS[4] = GetLeftDigit(Value);
	  Right = R3D1Digit;
          REGISTER3_DIGITS[0] = GetRightDigit(Value);
	  break;

	case 0x1000:		// AAAA=2
	  printf ("LR3D2, RR3D3\n");
	  //Sign = MainWindow->R3PlusMinus;
	  if (0 != (Value & 0x0400))
	    R3Sign |= 2;
	  else
	    R3Sign &= ~2;
	  RSign = R3Sign;
	  Left = R3D2Digit;
          REGISTER3_DIGITS[1] = GetLeftDigit(Value);
	  Right = R3D3Digit;
          REGISTER3_DIGITS[2] = GetRightDigit(Value);
	  break;

	case 0x0800:		// AAAA=1
	  printf ("LR3D4, RR3D5\n");
	  //Sign = MainWindow->R3PlusMinus;
	  if (0 != (Value & 0x0400))
	    R3Sign |= 1;
	  else
	    R3Sign &= ~1;
	  RSign = R3Sign;
	  Left = R3D4Digit;
          REGISTER3_DIGITS[3] = GetLeftDigit(Value);
	  Right = R3D5Digit;
          REGISTER3_DIGITS[4] = GetRightDigit(Value);
	  break;

	default:
	  goto Error;

	}
        DisplayNounVerb(NOUN_DIGITS);
        DisplayNounVerb(VERB_DIGITS);
        DisplayRegister(REGISTER1_DIGITS);
        DisplayRegister(REGISTER2_DIGITS);
        DisplayRegister(REGISTER3_DIGITS);
    }

  else if (Channel == 011)
    {
      // Here are appropriate Luminary 131 actions for various discrete
      // annunciations.
      if ((Value & 2) != (Last11 & 2))
	{
	  //if (0 == (Value & 2))
	  //printf("CompActyOff\n");
	  //else
	  //printf("CompActyOn\n");
	}

      Last11 = Value;
    }
  else if (Channel == 0163)
    {
      // Handle Verb/Noun flashing via the fake V/N flash channel 163
      if (Value & DSKY_VN_FLASH)
	{
	  //printf("VERB NOUN FLASH ON\n");
	}
      else
	{
	  //printf("VERB NOUN FLASH OFF\n");
	}
    }

  return;

Error:IoErrorCount++;
}

int
makeConnection ()
{
  unsigned char Packet[4];
  int PacketSize = 0;
  int i = 0;
  unsigned char c = 0;


  // Try to connect to the server (yaAGC) if not already connected.
  if (ServerSocket == -1)
    {
      ServerSocket = CallSocket (Hostname, Portnum);
      if (ServerSocket != -1)
	printf ("Connected to Apollo Guidance Computer.\n");
    }
  if (ServerSocket != -1)
    {
      for (;;)
	{
	  i = recv (ServerSocket, (char *) &c, 1, MSG_NOSIGNAL);
	  if (i == -1)
	    {
	      if (errno == EAGAIN || errno == 0 || errno == 9)
		i = 0;
	      else
		{
		  printf ("Server connection error %d\n", errno);
		  close (ServerSocket);
		  ServerSocket = -1;
		  break;
		}
	    }
	  if (i == 0)
	    break;
	  // This (newer) code will accept any packet signature of the form
	  // 00 XX XX XX.
	  if (0 == (0xc0 & c))
	    PacketSize = 0;
	  if (PacketSize != 0 || (0xc0 & c) == 0)
	    {
	      Packet[PacketSize++] = c;
	      if (PacketSize >= 4)
		{
		  ActOnIncomingIO (Packet);
		  PacketSize = 0;
		}
	    }
	}
    }
  return 0;
}


int
msleep (long msec)
{
  struct timespec ts;
  int res;

  if (msec < 0)
    {
      errno = EINVAL;
      return -1;
    }

  ts.tv_sec = msec / 1000;
  ts.tv_nsec = (msec % 1000) * 1000000;

  do
    {
      res = nanosleep (&ts, &ts);
    }
  while (res && errno == EINTR);

  return res;
}

void
OutputReset ()
{
  OutputKeycode (18);
}

void
OutputVerb ()
{
  OutputKeycode (17);
}

void
OutputNoun ()
{
  OutputKeycode (31);
}

void
OutputZero ()
{
  OutputKeycode (16);
}

void
OutputEnter ()
{
  OutputKeycode (28);
}

void
OutputOnethroughNine (int number)
{
  OutputKeycode (number);
}

void
OutputKeyRel ()
{
  OutputKeycode (25);
}

void
QueryMemoryTest ()
{

  OutputVerb ();
  OutputOnethroughNine (2);
  OutputOnethroughNine (7);
  OutputNoun ();
  OutputZero ();
  OutputOnethroughNine (2);
  OutputEnter ();
  OutputOnethroughNine (5);
  OutputOnethroughNine (7);
  OutputOnethroughNine (3);
  OutputOnethroughNine (5);
  OutputOnethroughNine (5);
  OutputEnter ();


}


void
QueryFixedMemory (int memval)
{
  int i = 0;

  OutputVerb ();
  OutputOnethroughNine (2);
  OutputOnethroughNine (7);
  OutputNoun ();
  OutputZero ();
  OutputOnethroughNine (2);
  OutputEnter ();

  //simulate typing in the digits   

  for (i = 5; i > 0; i--)
    {
      OutputOnethroughNine (memval % 10);
      memval = memval /= 10;
    }

  OutputEnter ();
}

void
SelfTest ()
{
  OutputReset ();
  OutputEnter ();
  OutputVerb ();
  OutputOnethroughNine (3);
  OutputOnethroughNine (5);
  OutputEnter ();
}

void *
doit (void *id)
{
  do
    {
      makeConnection ();
      msleep (PULSE_INTERVAL);
    }
  while (1);

}


int
main (int argc, char *argv[])
{
  int done = 1;
  pthread_t CheckAGC;
  int memtoread = 0;
  char memtoread_str[6] = { 0 };

  int t = pthread_create (&CheckAGC, NULL, doit, (void *) 1);

  if (argc < 2)
    {
      printf ("Requires octal 5 digit memory location to read\n");
      return 0;
    }

  strncpy (memtoread_str, argv[1], 6);
  printf ("%s\n", memtoread_str);
  memtoread = atoi (memtoread_str);

  printf ("Will read memory location %d\n", memtoread);
      OutputReset ();
      OutputKeyRel ();

  sleep(3);

  do
    {

      if (done)
	{
	  QueryFixedMemory (memtoread);
	  done = 0;
	}
      sleep (1);
      //OutputReset ();
      //OutputKeyRel ();

    }
  while (1);
  return 0;
}
