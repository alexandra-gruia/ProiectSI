#include "stm32f4xx.h"
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>

#include "Types.h"
#include "Drivers.h"

void delayMs(int);
UART_MSG_T msg = {.data = {0}, .id = 0, .len = 0};
U8 CyclicMessage_U8 = 0;
U8 TimerCountDown_U16 = SECONDS_TO_COUNT_U8;
U8 PrintInCycleMode_U8 = 0;

/* Lucrarea 2 de completat */
void PeriphInit(void)
{

	__disable_irq();
	// Configure PB[7..4] as output
	//RCC->AHB1ENR |= 2;			 /* Enable GPIOB clock */
	//GPIOB->MODER &= 0; /* Reset GPIOB PB[7..4]  */
//	GPIOB->MODER |= 0x00005500;	 /* Set GPIOB PB[7..4]  as ouput */

	// Configure PC[11..8] as input
	//RCC->AHB1ENR |= 0;			 /* Enable GPIOC clock */
	//GPIOC->MODER &= 0; /* Reset GPIOC PC[11..8]  for input mode */

	// Configure PB[15..12] port as input and enables pull-ups
	//GPIOB->MODER &= 0; /* Reset GPIOB PB[15..12]  */
	//GPIOB->PUPDR |= 0;	 /* Enable pull-ups on GPIOB PB[15..12]  */
 
	USART2_init();

	__enable_irq();
}

/* TODO: change the 0's to the correct value */
const U8 Hex7Segment[16] = {
	0,
	0xF9,
	0xA4,
	0xB0,
	0x99,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0};

/* end lucrarea 2*/

U8 ReceiveMessage(void)
{
	U8 error = 0;
	char msg_str[MAX_PACKET_STR_SIZE];
	const char *pos = msg_str;

	memset(&msg, 0, sizeof(UART_MSG_T));

	fgets(msg_str, MAX_PACKET_STR_SIZE, stdin);

	// printf("Unformated string: %s, size %d\n", msg_str, strlen(msg_str));

	if (strlen(msg_str) < 3)
	{
		msg.id = 0xFF;
		return 0;
	}

	if (sscanf(pos, "%2hhx", &msg.id) == 0)
	{
		msg.id = 0xFF;
		return 0;
	}

	if (strlen(msg_str) > 5)
	{
		pos += 3;

		for (int i = 1; i < strlen(msg_str) / 3; i++)
		{
			sscanf(pos, "%2hhx", &msg.data[i - 1]);
			pos += 3;
			msg.len++;
		}
	}
	return 1;
}

void PrintReceivedMessage(const UART_MSG_T msg)
{
	printf("the received packet is: \n");
	printf("ID: %02x \n", msg.id);
	printf("Size in Bytes: %d\n", msg.len);
	printf("Data:");
	for (int i = 0; i < msg.len; i++)
		printf("%02X ", msg.data[i]);
	printf("\r\n");
}

void ProcessMessage(const UART_MSG_T msg)
{
	U32 p;
	switch (msg.id)
	{
	case SW_VERSION:
		printf("Current program: Lucrarea 2 SI\n");
		break;

	case PORT_INPUT:
		if (msg.len < 2)
		{
			printf("bad-len\n");
		}
		else if (msg.data[0] != 0x42 && msg.data[0] != 0x43)
		{
			printf("bad-port\n");
		}
		else if (((msg.data[0] == 0x42) && (msg.data[1] > 15 || msg.data[1] < 12)) ||
				((msg.data[0] == 0x43) && (msg.data[1] > 11 || msg.data[1] < 8)))
		{
			printf("bad-pin\n");
		}
		else
		{
			U32 i = 1;
			p = msg.data[0] != 0x42 ? GPIOC->IDR : GPIOB->IDR;
			i <<= msg.data[1];
			if (CyclicMessage_U8 == 0 || PrintInCycleMode_U8 == 1)
			{
				printf("%d \n", (p & i) >> msg.data[1]);
				PrintInCycleMode_U8 = 0;
			}

			if (CyclicMessage_U8 == 0 && msg.len >= 3)
			{
				if (msg.data[2] > 0)
				{
					CyclicMessage_U8 = 1;
					InitPeriodicTimer(PERIODIC_TIMER_VALUE_500US_U16);
				}
			}
		}

		break;
	case PORT_OUTPUT:
		if (msg.len < 3)
		{
			printf("bad-len\n");
		}
		else if (msg.data[0] != 0x42)
		{
			printf("bad-port\n");
		}
		else if ((msg.data[1] > 7) || (msg.data[1] < 4))
		{
			printf("bad-pin\n");
		}
		else
		{
			U32 i = 1;
			i <<= msg.data[1];
			GPIOB->BSRR = msg.data[2] ? (GPIOB->BSRR | i) : (GPIOB->BSRR | (i << 16));
			printf("%02X\n", (GPIOB->ODR & 0x000000F0) >> 4);
		}
		break;
	case PORT_HEX:
		if (msg.len < 2)
		{
			printf("bad-len\n");
		}
		else if (msg.data[0] > 3)
		{
			printf("bad-display\n");
		}
		else if (msg.data[1] > 15)
		{
			printf("bad-number\n");
		}
		else
		{
			SPI1_init();
			SPI1_write(Hex7Segment[msg.data[1]]); /* write pattern to the seven segments */
			SPI1_write(1 << msg.data[0]);		  /* select digit */
		}
		break;
	default:

		printf("Invalid ID received \n");
		break;
	}
}

int readInteger() {
    char buffer[256];
    int number;

    // Citirea unei linii de la intrare
    fgets(buffer, sizeof(buffer), stdin);

    // Convertirea liniei într-un numar întreg
    number = atoi(buffer);

    return number;
}

int main() {
    U8 status = 0;
    PeriphInit();

    Board board;
    Move move;
    char currentPlayer = 'X';
    char winner = ' ';

    // Initialize the game board
    initBoard(&board);
         
    printf("BINE ATI VENIT LA JOCUL X & 0!\n");
    printf("\n");
    printf("Acestea sunt regulile jocului:\n");
    printf("-Necesita doi jucatori, X respectiv 0, care marcheaza pe rand cate o casuta dintr-un tabel cu 3 linii si 3 coloane.\n");
    printf("-Jucatorul care reuseste primul sa marcheze 3 casute adiacente pe orizontala, verticala sau diagonala castiga jocul.\n");
    printf("\n");
    printf("Tabla de joc este urmatoarea:\n");
		printBoard(&board);
  
   // Bucla principala a jocului
 while (1) {
    printf("Este randul jucatorului %c.\n", currentPlayer);

    int position;
    if (currentPlayer == 'X') {
        // Jucatorul X introduce pozi?ia dorita
        printf("Introduceti pozitia dorita (1-9): ");
        position = readInteger();
    } else {
        // Nucleo alege o pozi?ie aleatoare valida
        do {
            position = rand() % 9 + 1; // Genereaza o pozi?ie între 1 ?i 9
        } while (board.cells[(position - 1) / 3][(position - 1) % 3] != EMPTY_CELL);
    }

    // Verifica daca pozi?ia introdusa este valida
    if (position < 1 || position > 9) {
        printf("Pozitie invalida! Incercati din nou.\n");
        continue;
    }

    // Realizeaza mutarea doar daca pozi?ia este libera
    int row = (position - 1) / 3;
    int col = (position - 1) % 3;
    if (board.cells[row][col] != EMPTY_CELL) {
        printf("Pozitie ocupata! Incercati din nou.\n");
        continue;
    }
    
    // Realizeaza mutarea
    board.cells[row][col] = currentPlayer;

    // Verifica daca s-a realizat o victorie sau daca jocul s-a terminat într-o remiza
    winner = checkWin(&board);
    if (winner != ' ') {
        break; // Jocul s-a terminat
    }

    // Schimba jucatorul curent doar daca mutarea a fost valida ?i jocul nu s-a terminat
    currentPlayer = (currentPlayer == 'X') ? 'O' : 'X';

    // Afi?eaza tabla de joc actualizata
    printf("Tabla de joc actualizata:\n");
    printBoard(&board);
}

// Afiseaza rezultatul jocului
printBoard(&board);
printf("Jocul s-a incheiat.\n");
if (winner == 'T') {
    printf("Egalitate!\n");
} else {
    printf("Jucatorul %c a castigat!\n", winner);
}

    return 0;

}
        

