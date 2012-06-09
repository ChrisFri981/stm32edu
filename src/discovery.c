/*
 * Autor:  J. Kerdels 
 * Lizenz: CC BY 3.0
*/


// Definition der standard Integer-Typen
#include <stdint.h>

#include "discovery.h"



/* Die Methode discovery_init() initialisiert zun�chst nur die
4 LEDs und den User-Button des discovery boards. */
void discovery_basic_init(void)
{
/* Das STM32F4 discovery board besitzt vier frei schaltbare LEDs, die an den
Pins PD12, PD13, PD14 und PD15 angeschlossen sind. Zus�tzlich gibt es noch 
einen Taster der mit Pin PA0 verbunden ist. Abbildung 16 in [3] (S.36) gibt
Aufschluss dar�ber, wie die LEDs und der Taster auf dem discovery board 
implementiert sind. Die LEDs haben jeweils einen Vorwiderstand und sind mit
der Masse verbunden. Dies bedeutet, dass wir die entsprechenden IO Pins als
Ausg�nge im sogenannten "Push-Pull" Modus (s.S.144 in [1]) betreiben m�ssen.
Der Taster (B1) schlie�t zwischen VDD und Masse �ber einen externen Pull-Down-
Widerstand. Zus�tzlich ist ein RC-Glied vorgesehen, dass den Taster entprellen
k�nnte. Der hierf�r vorgesehene Kondensator C38 ist standardm��ig jedoch nicht
eingel�tet. F�r den Taster muss der Pin PA0 als Eingang ohne eigenen Pull-Up- 
oder Pull-Down-Widerstand (s.S.143 in [1]) konfiguriert werden.

Eine kurze Erl�uterung vorweg: Im Folgenden wird immer wieder von "Pins" aber
auch von "Ports" die Rede sein. Mit dem Begriff "Pin" wird jeweils ein 
einzelner, physischer Ein- bzw. Ausgang bezeichnet, der im Regelfall auch
einen korrespondierenden Hardwareanschluss am Geh�use des Mikrocontrollers 
hat. Der Begriff "Port" bezieht sich auf eine logische Gruppierung mehrerer
Pins. Im Falle des STM32 werden in der Regel 16 Pins zu einem Port zusammen-
gefasst. Wenn also zuvor die Rede vom Pin A0 war, so bezeichnet dies den Pin 0
des Port A. Die Pins PD12 bis PD15 sind die Pins 12 bis 15 des Port D. Der 
STM32 besitzt die Ports A bis I, wobei der Port I nur 12 und nicht 16 Pins
beherbergt. Insgesamt besitzt der STM32 also 8 x 16 + 11 = 139 IO-Pins! 
Abbildung 5 in [2] (S.17) illustriert, dass die GPIO Ports an den Kern �ber
den AHB1-Bus mit maximal 168 MHz angebunden sind. Der AHB1-Bus ist Teil der
Multi-AHB-Busmatrix. Diese wird in Abbildung 6 in [2] (S.20) im Detail dar-
gestellt.

Bevor wir die Pins 12 bis 15 von Port D und Pin 0 von Port A nutzen k�nnen, 
m�ssen die beiden Ports erst einmal aktiviert werden. Dies geschieht �ber das
"RCC AHB1 peripheral clock register" (RCC_AHB1ENR), das auf Seite 110 in [1]
beschrieben wird. Wie auch in der rcc_init() Methode ben�tigen wir also zu-
n�chst die Basisadresse der RCC-Register (S.50 in [1]): */

static const uint32_t RCC_BASE = 0x40023800;

/* Ausgehend von dieser Adresse kann nun das RCC_AHB1ENR-Register mit einem 
"address offset" von 0x30 definiert werden: */

volatile uint32_t *RCC_AHB1ENR = (uint32_t*)(RCC_BASE + 0x30);

/* Port A wird �ber Bit 0 und Port D wird �ber Bit 3 aktiviert. Aktiviert
bedeutet hierbei, dass die beiden Ports mit einem Taktsignal versorgt werden.
Im Vergleich zu anderen Mikrocontrollern, z.B. der ATMega-Serie von Atmel, mag
es ungew�hnlich erscheinen, einen IO-Port erst aktivieren zu m�ssen. Durch die
selektive Taktung der einzelnen Komponenten ist es jedoch m�glich, den Mikro-
controller besonders energiesparend zu betreiben. Der Beschreibung des 
RCC_AHB1ENR-Register kann man entnehmen, dass man z.B. auch die DMA-Controller
explizit "anschalten" muss.

TIPP: Wenn einmal eine Komponente partout nicht laufen will, sollte man unbe-
      dingt pr�fen, ob die Komponente �berhaupt ein Taktsignal bekommt!

Zur�ck zu den LEDs und dem Taster. Wir aktivieren also den Takt f�r Port A und
Port D: */

// Bit 0 und 3 auf 1 setzen, um Port A und D mit Taktsignal zu versorgen
*RCC_AHB1ENR |= 0x00000009;


/* Die logische Zusammenfassung der IO-Pins in einzelne Ports spiegelt sich 
vor allem in der Ansteuerung der IO-Pins �ber die zugeh�rigen GPIO-Register
wider. Zu jedem Port (A bis I) gibt es einen ganzen Satz an Konfigurationsre-
gistern. Wie zuvor ben�tigen wir auch in diesem Fall zun�chst die Basis-
adressen des Registersatzes f�r Port A und des Registersatzes f�r Port B: */

static const uint32_t GPIOA_BASE = 0x40020000;
static const uint32_t GPIOD_BASE = 0x40020C00;

/* Im Grunde k�nnte man nun wie bisher die einzelnen Register mit ihrem je-
weiligen "Address Offset" bzgl. der Basisadresse des jeweiligen Ports als
Zeigervariablen definieren. Dies ist erscheint jedoch relativ "unelegant", da
ja jeder Port den gleichen Satz an Registern besitzt, nur halt jeweils an 
einer anderen Basisadresse. Es ist daher eine gute Gelegenheit, eine etwas
elegantere Ansteuerung der Konfigurationsregister vorzustellen. Anstelle der
Definition jedes einzelnen Hardwareregisters, kann man die Register, die sich
hintereinander an einer Basisadresse wiederfinden, auch als einen zusammenge-
setzen Datentyp beschreiben. F�r die GPIO-Register, die in [1] ab Seite 148
beschrieben sind, ergibt sich damit der folgende zusammengesetzte Datentyp: */

typedef struct
{
    volatile uint32_t MODER;    /* GPIO port mode register,
                                    Address offset: 0x00 */
    volatile uint32_t OTYPER;   /* GPIO port output type register,
                                    Address offset: 0x04 */
    volatile uint32_t OSPEEDR;  /* GPIO port output speed register,
                                    Address offset: 0x08 */
    volatile uint32_t PUPDR;    /* GPIO port pull-up/pull-down register,
                                    Address offset: 0x0C */
    volatile uint32_t IDR;      /* GPIO port input data register,
                                    Address offset: 0x10 */
    volatile uint32_t ODR;      /* GPIO port output data register,
                                    Address offset: 0x14 */
    volatile uint16_t BSRRL;    /* GPIO port bit set/reset low register,
                                    Address offset: 0x18 */
    volatile uint16_t BSRRH;    /* GPIO port bit set/reset high register,
                                    Address offset: 0x1A */
    volatile uint32_t LCKR;     /* GPIO port configuration lock register,
                                    Address offset: 0x1C */
    volatile uint32_t AFR[2];   /* GPIO alternate function registers,
                                    Address offset: 0x24-0x28 */
} GPIO_TDef;

/* Durch die Verwendung der in <stdint.h> definierten Standard-Integertypen
kann die Bitbreite jedes Eintrags im Struct GPIO_TDef exakt festgelegt 
werden. Es ist hierbei zwingend notwendig, dass die in der Dokumentation auf
Seite 148ff [1] angegebenen Bitbreiten und "Address Offsets" der Register 
eingehalten werden. Dar�ber hinaus m�ssen auch die einzelnen Eintr�ge des 
Struct als "volatile" gekennzeichnet werden.

Mit dem auf diese Weise definierten Struct GPIO_TDef k�nnen nun auf bequeme
Weise zwei Zeigervariablen auf die Konfigurationsregister der Ports A und D
erzeugt werden: */

volatile GPIO_TDef *GPIOA = (GPIO_TDef*)(GPIOA_BASE);
volatile GPIO_TDef *GPIOD = (GPIO_TDef*)(GPIOD_BASE);

/* Der Zugriff auf die einzelnen Kontrollregister kann nun �ber den 
Dereferenzierungsoperator "->" erfolgen. Der hier beschriebene Zugriff auf die
Konfigurationsregister der GPIO-Ports wird im Rahmen der normalerweise ver-
wendeten Library-Header stm32f4xx.h durchg�ngig f�r alle Arten von Hardware-
registern verwendet. Dementsprechend w�rde man z.B. die obige Aktivierung des
Taktes der Ports A und D �ber den Ausdruck

RCC->AHB1ENR |= 0x00000009;

vollziehen.

Nachdem wir uns nun einen "eleganten" Zugriff auf die Kontrollregister der 
GPIO-Ports A und D gezimmert haben, k�nnen wir endlich unsere Pins 
konfigurieren! Hierf�r empfiehlt sich der Blick auf Tabelle 14 in [1] (S.137).
In der rechten Spalte "I/O configuration" sucht man sich die gew�nschte 
IO-Konfiguration aus, z.B. "GP output PP" f�r unsere LEDs, und verwendet dann
die Werte in den �brigen Spalten der entsprechenden Zeile f�r die 
Konfiguration der einzelnen GPIO-Register. Das (i) in den Bezeichnungen der
Spalten bezieht sich im �brigen auf den jeweiligen IO-Pin. Im Falle unserer
LEDs am Port D m�ssen wir z.B. MODER(12), MODER(13), MODER(14) und MODER(15)
auf "01" setzen. Wo sich MODER(12) usw. im 32-Bit Register MODER befinden, 
kann der Registerbeschreibung auf Seite 148 [1] entnommen werden. Es sind die
Bits 24:25, 26:27, 28:29 und 30:31. Sucht man sich f�r die anderen Tabellen-
eintr�ge die entsprechenden Informationen auf den Seiten 148ff zusammen, so 
kommt die folgende Konfiguration der LED-Pins PD12 bis PD15 zustande: */

// Bits 24 bis 31 im MODER-Register "freir�umen"
GPIOD->MODER &= 0x00FFFFFF;
// setze Bits 24 bis 31 im MODER-Register auf 0101 0101
GPIOD->MODER |= 0x55000000;

// setze Bits 12 bis 15 im OTYPER-Register auf 0
GPIOD->OTYPER &= 0xFFFF0FFF;

// Bits 24 bis 31 im OSPEEDR-Register "freir�umen"
GPIOD->OSPEEDR &= 0x00FFFFFF;
// setze Bits 24 bis 31 im OSPEEDR-Register auf 0101 0101 (25MHz)
GPIOD->OSPEEDR |= 0x55000000;

// Setze Bits 24 bis 31 im PUPDR-Register auf 0000 0000
GPIOD->PUPDR &= 0x00FFFFFF;

/* Auf �hnliche Weise wie wir soeben die PINs f�r unsere LEDs konfiguriert 
haben, k�nnen wir uns nun dem Input Pin f�r den Taster zuwenden. In Tabelle 14
([1], S.138) suchen wir uns den Eintrag "Input Floating" in der rechten Spalte
der Tabelle. In drei der �brigen Spalten finden sich "x" Markierungen. Dies 
bedeutet, dass hier die konkreten Registerwerte keine Bedeutung haben. H�ufig
wird dies auch als "don't care" bezeichnet. Wir m�ssen f�r unseren Input-Pin
also nur das MODER-Register und das PUPDR-Register einstellen: */

// Bits 0 und 1 im MODER-Register auf 00 setzen
GPIOA->MODER &= 0xFFFFFFFC;

// Bits 0 und 1 im PUPDR-Register auf 00 setzen
GPIOA->PUPDR &= 0xFFFFFFFC;

/* Die Werte, die an den einzelnen IO-Pins eines Ports anliegen, stehen im 
IDR-Register. Sie werden in jedem Takt von der Hardwareseite aus aktualisiert.
�ber einen Zugriff auf das ODR-Register k�nnen die IO-Pins eines Ports - wenn
dieser als Ausgang konfiguriert ist - gesetzt werden. Setzt man ein Bit auf 0,
so wird auch der korrespondierende Pin logisch 0. Setzt man das Bit auf 1, so 
wird auch der korrespondierende Pin logisch 1. Soweit unterscheidet sich der 
STM32 nicht von anderen Mikrocontrollern. Er hat aber noch zwei zus�tzliche
Register: BSRRL und BSRRH. Dies sind Set- und Reset-Register f�r die IO-Pins
des jeweiligen Ports. Schreibt man eine 1 in ein Bit des BSRRL-Registers, so
wird der zugeh�rige Pin logisch 1, Schreibt man eine 1 in ein Bit des BSRRH-
Registers, so wird der zugeh�rige Pin logisch 0. Setzen wir also die LED-Pins
initial auf 0:*/

// Bits 12 bis 15 des BSRRH-Registers auf 1 
//   -> (IO Pins 12 bis 15 auf logisch 0)
GPIOD->BSRRH |= 0xF000;

/* Die Konfiguration der einfachen Komponenten (LEDs und Taster) des discovery 
boards ist damit abgeschlossen. */


}





/* Die Methode discovery_acc_init() f�hrt die Grundkonfiguration des
Beschleunigungssensors auf dem discovery board durch. */
void discovery_acc_init(void)
{
/* Das STM32F4 discovery board besitzt einen 3-Achsen Beschleunigungssensor
   (LIS302DL), der �ber das SPI-Protokoll angesprochen wird. Physikalisch ist
   der Sensor mit den GPIO-Pins PA5/SPI_SCK, PA6/SPI1_MISO, PA7/SPI1_MOSI 
   verbunden. Zus�tzlich sind an PE0 und PE1 zwei Interruptleitungen des
   LIS302DL angeschlossen. Hier�ber kann der Beschleunigungssensor z.B. 
   mitteilen, ob er sich gerade im freien Fall befindet. Derartige Messungen
   werden beispielsweise in Notebooks eingesetzt, um bei einem Sturz des
   Notebooks noch schnell die Lesek�pfe der Festplatten in eine sichere 
   Position zu bringen. Als letztes ist noch Pin PE2 zu erw�hnen. Mit diesem
   Pin w�hlt man aus, ob der LIS302DL im SPI- oder im I2C-Modus kommunizieren
   soll. F�r den SPI-Modus muss PE2 auf 0 stehen. 
   PA5 bis PA7 m�ssen also als Alternate Function konfiguriert werden, PE0
   und PE1 als Input und PE2 als output. Vor allem m�ssen GPIO A und E 
   zun�chst einmal mit Takt versorgt werden (RCC_AHB1ENR, S.110 in [1]): */
   
static const uint32_t RCC_BASE = 0x40023800;

volatile uint32_t *RCC_AHB1ENR = (uint32_t*)(RCC_BASE + 0x30);

*RCC_AHB1ENR |= 0x00000011;

/* Als n�chstes wollen wir die GPIOs A und E konfigurieren. Also flux die
passenden Pointer gezimmert: */

static const uint32_t GPIOA_BASE = 0x40020000;
static const uint32_t GPIOE_BASE = 0x40021000;

typedef struct
{
    volatile uint32_t MODER;    /* GPIO port mode register,
                                    Address offset: 0x00 */
    volatile uint32_t OTYPER;   /* GPIO port output type register,
                                    Address offset: 0x04 */
    volatile uint32_t OSPEEDR;  /* GPIO port output speed register,
                                    Address offset: 0x08 */
    volatile uint32_t PUPDR;    /* GPIO port pull-up/pull-down register,
                                    Address offset: 0x0C */
    volatile uint32_t IDR;      /* GPIO port input data register,
                                    Address offset: 0x10 */
    volatile uint32_t ODR;      /* GPIO port output data register,
                                    Address offset: 0x14 */
    volatile uint16_t BSRRL;    /* GPIO port bit set/reset low register,
                                    Address offset: 0x18 */
    volatile uint16_t BSRRH;    /* GPIO port bit set/reset high register,
                                    Address offset: 0x1A */
    volatile uint32_t LCKR;     /* GPIO port configuration lock register,
                                    Address offset: 0x1C */
    volatile uint32_t AFR[2];   /* GPIO alternate function registers,
                                    Address offset: 0x24-0x28 */
} GPIO_TDef;

volatile GPIO_TDef *GPIOA = (GPIO_TDef*)(GPIOA_BASE);
volatile GPIO_TDef *GPIOE = (GPIO_TDef*)(GPIOE_BASE);

/* Wir konfigurieren nun PA5 bis PA7 als Alternate Function 5 (S.56 in [2]):
Die Taktleitung SPI1_SCK an PA5 wird als Output-Push-Pull mit 2MHz 
konfiguriert (s.Tabelle 14 auf S.138 in [1]): */

//MODER = 10, OTYPER = 0, OSPEEDR = 00, PUPDR = 00

// Bits 10 und 11, S.148 in [1]:
GPIOA->MODER &= 0xFFFFF3FF;
GPIOA->MODER |= 0x00000800;

// Bit 5, S.148 in [1]:
GPIOA->OTYPER &= 0xFFFFFFDF;

// Bits 10 und 11, S.149 in [1]:
GPIOA->OSPEEDR &= 0xFFFFF3FF;

// Bits 10 und 11, S.149 in [1]:
GPIOA->PUPDR &= 0xFFFFF3FF;

// Alternate Function 5, Bits 20 bis 23, S.152 in [1]:
GPIOA->AFR[0] &= 0xFF0FFFFF;
GPIOA->AFR[0] |= 0x00500000;

/* Der SPI1 Master In Slave Out (MISO) an PA6 wird als Input Open-Drain mit 
2MHz konfiguriert (s.Tabelle 14 auf S.138 in [1]): */

//MODER = 10, OTYPER = 1, OSPEEDR = 00, PUPDR = 00

// Bits 12 und 13, S.148 in [1]:
GPIOA->MODER &= 0xFFFFCFFF;
GPIOA->MODER |= 0x00002000;

// Bit 6, S.148 in [1]:
GPIOA->OTYPER |= 0x00000040;

// Bits 12 und 13, S.149 in [1]:
GPIOA->OSPEEDR &= 0xFFFFCFFF;

// Bits 12 und 13, S.149 in [1]:
GPIOA->PUPDR &= 0xFFFFCFFF;

// Alternate Function 5, Bits 24 bis 27, S.152 in [1]:
GPIOA->AFR[0] &= 0xF0FFFFFF;
GPIOA->AFR[0] |= 0x05000000;

/* Der SPI1 Master Out Slave In (MOSI) an PA7 wird wieder als Output-Push-Pull
mit 2MHz konfiguriert (s.Tabelle 14 auf S.138 in [1]): */

//MODER = 10, OTYPER = 0, OSPEEDR = 00, PUPDR = 00

// Bits 14 und 15, S.148 in [1]:
GPIOA->MODER &= 0xFFFF3FFF;
GPIOA->MODER |= 0x00008000;

// Bit 7, S.148 in [1]:
GPIOA->OTYPER &= 0xFFFFFF7F;

// Bits 14 und 15, S.149 in [1]:
GPIOA->OSPEEDR &= 0xFFFF3FFF;

// Bits 14 und 15, S.149 in [1]:
GPIOA->PUPDR &= 0xFFFF3FFF;

// Alternate Function 5, Bits 28 bis 31, S.152 in [1]:
GPIOA->AFR[0] &= 0x0FFFFFFF;
GPIOA->AFR[0] |= 0x50000000;


/* Nachdem nun die SPI-Pins konfiguriert sind, m�ssen wir PE2 noch als
Output-Push-Pull konfigurieren und auf 0 setzen, damit SPI als Protokoll auf
dem Beschleunigungssensor aktiviert wird: */

//MODER = 01, OTYPER = 0, OSPEEDR = 00, PUPDR = 00

// Bits 4 und 5, S.148 in [1]:
GPIOE->MODER &= 0xFFFFFFCF;
GPIOE->MODER |= 0x00000010;

// Bit 2, S.148 in [1]:
GPIOE->OTYPER &= 0xFFFFFFFB;

// Bits 4 und 5, S.149 in [1]:
GPIOE->OSPEEDR &= 0xFFFFFFCF;

// Bits 4 und 5, S.149 in [1]:
GPIOE->PUPDR &= 0xFFFFFFCF;

// Pin PE2 auf 0 setzen
GPIOE->BSRRH |= 0x0004;




}


