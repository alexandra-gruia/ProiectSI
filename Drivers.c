#include "Drivers.h"
#include "stm32f4xx.h"

/* DRIVER C*/

#include "Drivers.h"
#define BOARD_SIZE 3

/* TIM 2 as a general timer for cyclic messages */
void InitPeriodicTimer(U16 timeInMs)
{
  /* setup TIM2 */
  RCC->APB1ENR |= 1;        /* enable TIM2 clock */
  TIM2->PSC = 16000 - 1;    /* divided by 16000  to generate an up timer at 1kHz or 1ms*/
  TIM2->ARR = timeInMs - 1; /* divided by TimeInMs to generate the diseired timeout */
  TIM2->CR1 = 1;            /* enable counter */

  TIM2->DIER |= 1;           /* enable UIE */
  NVIC_EnableIRQ(TIM2_IRQn); /* enable interrupt in NVIC */

  /* setup main board LED to togle each 500 ms for periodic messages */
  RCC->AHB1ENR |= 1; /* enable GPIOA clock */

  GPIOA->MODER &= ~0x00000C00; /* clear the led */
  GPIOA->MODER |= 0x00000400;  /* set the mod as output */
}

void StopPeriodicTimer(void)
{
  RCC->APB1ENR &= ~1;         /* disable TIM2 clock */
  TIM2->CR1 = 0;              /* disable counter */
  TIM2->DIER &= ~1;           /* disable UIE */
  NVIC_DisableIRQ(TIM2_IRQn); /* disable interrupt in NVIC */
}

/* configure SPI1 and the associated GPIO pins */
void SPI1_init(void)
{
  RCC->AHB1ENR |= 1;      /* enable GPIOA clock */
  RCC->APB2ENR |= 0x1000; /* enable SPI1 clock */

  /* PORTA 5, 7 for SPI1 MOSI and SCLK */
  GPIOA->MODER &= ~0x0000CC00;  /* clear pin mode */
  GPIOA->MODER |= 0x00008800;   /* set pin alternate mode */
  GPIOA->AFR[0] &= ~0xF0F00000; /* clear alt mode */
  GPIOA->AFR[0] |= 0x50500000;  /* set alt mode SPI1 */

  /* PA9 as GPIO output for SPI slave select */
  GPIOA->MODER &= ~0x000C0000; /* clear pin mode */
  GPIOA->MODER |= 0x00040000;  /* set pin output mode */

  SPI1->CR1 = 0x31F;
  SPI1->CR2 = 0;
  SPI1->CR1 |= 0x40; /* enable SPI1 module */
}

/* This function enables slave select, writes one byte to SPI1, */
/* wait for transmit complete and deassert slave select. */
void SPI1_write(U8 data)
{
  while (!(SPI1->SR & 2))
  {
  }                         /* wait until Transfer buffer Empty */
  GPIOA->BSRR = 0x02000000; /* assert slave select */
  SPI1->DR = data;          /* write data */
  while (SPI1->SR & 0x80)
  {
  }                         /* wait for transmission done */
  GPIOA->BSRR = 0x00000200; /* deassert slave select */
}

void USART2_init (void) {
    RCC->AHB1ENR |= 1;          /* Enable GPIOA clock */
    RCC->APB1ENR |= 0x20000;    /* Enable USART2 clock */

    /* Configure PA2 and PA3 for USART2_TX and USART2_RX */
    GPIOA->AFR[0] &= ~0xFF00;
    GPIOA->AFR[0] |=  0x7700;   /* alt7 for USART2 */
    GPIOA->MODER  &= ~0x00F0;
    GPIOA->MODER  |=  0x00A0;   /* enable alternate function for PA2 and PA3 */

    USART2->BRR = 0x008B;       /* 115200 baud @ 16 MHz */
    USART2->CR1 = 0x000C;       /* enable Tx/Rx, 8-bit data */
    USART2->CR2 = 0x0000;       /* 1 stop bit */
    USART2->CR3 = 0x0000;       /* no flow control */
    USART2->CR1 |= 0x2000;      /* enable USART2*/
}


/* Write a character to USART2 */
int USART2_write (int ch) {
    while (!(USART2->SR & 0x0080)) {}   // wait until Tx buffer empty
    USART2->DR = (ch & 0xFF);
		return ch;
}

/* Read a character from USART2 */
int USART2_read(void) {
    while (!(USART2->SR & 0x0020)) {}   // wait until char arrives
    return USART2->DR;
}


/* The code below is the interface to the C standard I/O library.
 * All the I/O are directed to the console, which is UART2.
 */
//struct __FILE { int handle; };
FILE __stdin  = {0};
FILE __stdout = {1};
FILE __stderr = {2};

/* Called by C library console/file input
 * This function echoes the character received.
 * If the character is '\r', it is substituted by '\n'.
 */
int fgetc(FILE *f) {
    int c;

    c = USART2_read();      /* read the character from console */

    if (c == '\r') {        /* if '\r', after it is echoed, a '\n' is appended*/
        USART2_write(c);    /* echo */
        //c = '\n';
    }

    USART2_write(c);        /* echo */

    return c;
}

/* Called by C library console/file output */
int fputc(int c, FILE *f) {
    return USART2_write(c);  /* write the character to console */
}


// Initialize the game board
void initBoard(Board *board) {
    for (int i = 0; i < BOARD_SIZE; ++i) {
        for (int j = 0; j < BOARD_SIZE; ++j) {
            board->cells[i][j] = ' ';
        }
    }
}

// Print the game board
void printBoard(const Board *board) {
    for (int i = 0; i < BOARD_SIZE; ++i) {
        for (int j = 0; j < BOARD_SIZE; ++j) {
            printf(" %c ", board->cells[i][j]);
            if (j < BOARD_SIZE - 1) {
                printf("|");
            }
        }
        printf("\n");
        if (i < BOARD_SIZE - 1) {
            printf("---|---|---\n");
        }
    }
}

// Make a move on the game board
int makeMove(Board *board, const Move *move) {
    int row = move->row;
    int col = move->col;
    
    // Check if the cell is empty
    if (board->cells[row][col] == ' ') {
        board->cells[row][col] = move->player;
        return 1; // Move successful
    } else {
        return 0; // Move unsuccessful
    }
}

// Check for a win or tie
char checkWin(const Board *board) {
    // Check rows for a win
    for (int i = 0; i < BOARD_SIZE; ++i) {
        if (board->cells[i][0] == board->cells[i][1] && 
            board->cells[i][0] == board->cells[i][2] &&
            board->cells[i][0] != ' ') {
            return board->cells[i][0]; // Return winning player
        }
    }

    // Check columns for a win
    for (int j = 0; j < BOARD_SIZE; ++j) {
        if (board->cells[0][j] == board->cells[1][j] && 
            board->cells[0][j] == board->cells[2][j] &&
            board->cells[0][j] != ' ') {
            return board->cells[0][j]; // Return winning player
        }
    }

    // Check diagonals for a win
    if ((board->cells[0][0] == board->cells[1][1] && 
         board->cells[0][0] == board->cells[2][2] &&
         board->cells[0][0] != ' ') ||
        (board->cells[0][2] == board->cells[1][1] &&
         board->cells[0][2] == board->cells[2][0] &&
         board->cells[0][2] != ' ')) {
        return board->cells[1][1]; // Return winning player
    }

    // Check for a tie
    int emptyCount = 0;
    for (int i = 0; i < BOARD_SIZE; ++i) {
        for (int j = 0; j < BOARD_SIZE; ++j) {
            if (board->cells[i][j] == ' ') {
                emptyCount++;
            }
        }
    }

    if (emptyCount == 0) {
        return 'T'; // Tie
    }

    return ' '; // Game still ongoing
}