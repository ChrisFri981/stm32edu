/*
 * Autor:  J. Kerdels 
 * Lizenz: CC BY 3.0
*/

// Definition der standard Integer-Typen
#include <stdint.h>

#include "discovery_ex.h"

// u.a. Definition der Hardwareregister des STM32F4
#include "libfoo/stm32f4xx.h"

/* Hier das Beispiel ausw�hlen, welches laufen soll: */
//#define LED_AND_BUTTON
//#define LED_AND_TIMER
//#define TIMER_IRQ
//#define PWM_LED
//#define DMA_LED

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------



/* In diesem einfachen Beispiel werden die 4 LEDs des discovery boards
eingeschaltet, wenn der User-Button des boards gedr�ckt wird.*/
void led_and_button_example(void)
{
#ifdef LED_AND_BUTTON
    
    /* endlose Hauptschleife 
       (die "1" wird als "wahr" bzw. "true" interpretiert) */
    while(1) {
        
        /* Der User-Button des discovery bords ist am Pin 0 des Port A ange-
           schlossen. Seinen aktuellen Zustand (gedr�ckt = 1, ungedr�ckt = 0)
           k�nnen wir aus dem "Input Data Register" (IDR) des Ports A ablesen
           (Seite 150 in [1]). Im Gegensatz zur bisherigen Vorgehensweise 
           definieren wir an dieser Stelle das Hardware-Register nicht explizit
           als Pointer-Variable, sondern verwenden bequemerweise die korrespon-
           dierende Definition in der Datei "stm32f4xx.h" (s. obiges #include).
           Da das Register GPIOA->IDR uns den Status aller 16 IO-Pins des 
           Ports A anzeigt, m�ssen wir den Pin, der uns interessiert (Pin 0)
           mit einer sogenannten Bitmaske (hier: 0x00000001) isolieren: */
        
        if ((GPIOA->IDR & 0x00000001) == 1) {
            
        /* Man beachte hierbei die Klammerung des &-Terms. Diese Klammerung 
           ist wichtig, da der Vergleichsoperator "==" st�rker bindet als der
           Bitweise-Und-Operator "&". Diese st�rkere Bindung w�rde ohne 
           Klammerung dazu f�hren, dass erst der Vergleich 0x00000001 == 1 
           und erst dann die &-Verkn�pfung mit dem Register erfolgen w�rde.
           Noch viel Schlimmer: Der Code w�rde in diesem Fall sogar mit 
           fehlender Klammerung funktionieren! Der Vergleich 0x00000001 == 1
           resultiert in den Wahrheitswert "true", der sogleich auf den Wert
           1 abgebildet w�rde. Die &-Verkn�pfung von 1 mit dem Register ergibt
           genau dann eins, wenn der Taster gedr�ckt ist. Diese 1 w�rde 
           wiederum vom if-Konstrukt als Wahheitswert "true" interpretiert! 
           Das ganze bricht nat�rlich bereits dann in sich zusammen, wenn z.B.
           der Taster nicht an Pin 0, sondern an Pin 1 angeschlossen w�re.
            
           Die Ansteuerung der einzelnen Pins eines GPIO-Ports kann beim STM32
           auf zwei verschiedene Arten erfolgen. Zum einen kann der logische
           Zustand eines IO-Ports �ber das zugeh�rige "output data register" 
           (ODR) gesetzt werden (Seite 150 in [1]). Die unteren 16-Bit dieses 
           Registers (die oberen Bits sind reserviert und d�rfen nicht ver-
           �ndert werden) korrespondieren hierbei mit den einzelnen Pins des 
           Ports. M�chte man also beispielsweise Pin 12 von Port D auf 1 
           setzen, so w�rde man dies mit dem Aufruf

           GPIOD->ODR |= 0x00001000;

           machen. Um Pin 12 auf 0 zu setzen, muss eine Bitweise-Und-Operation
           verwendet werden. Die passende Bit-Maske hat hierbei an allen 
           Stellen au�er an Stelle 12 eine 1. Daraus folgt der Aufruf:
           
           GPIOD->ODR &= 0xFFFFEFFF;
           
           H�ufig findet in diesem Zusammenhang auch die Bitweise-Negation "~"
           Verwendung, um keine "von Hand" invertierte Bitmaske verwenden zu 
           m�ssen:
           
           GPIOD->ODR &= ~(0x00001000);
           
           Im Folgenden soll jedoch die zweite M�glichkeit f�r die Ansteuerung
           der IO-Pins eines GPIO-Ports verwendet werden. Der STM32 hat hierf�r
           das sogenannte "bit set/reset register" (BSRR) (Seite 150/151 in 
           [1]). Die unteren (low) 16-Bit und die oberen (high) 16-Bit dieses 
           32-Bit-Registers werden �blicherweise getrennt unter den Bezeich-
           nungen BSRRL (low) und BSRRH (high) angesprochen. Hierbei repr�sen-
           tieren die Bits beider Register nicht den logischen Zustand des 
           korrespondierenden IO-Pins, sondern sind als "Befehlsschnittstelle"
           zu verstehen. Schreibt man in eines der Bits eine "1", so ent-
           spricht dies dem Kommando an den Mikrocontroller, den korrespon-
           dierenden IO-Pin auf logisch 1 im Falle des BSRRL-Registers oder 
           auf logisch 0 im Falle des BSRRH-Registers zu setzen. In beiden 
           F�llen wird also eine "1", die man bequem mit einer Bitweisen-Oder-
           Operation schreiben kann, verwendet. Die Wahl des Registers (BSRRL 
           oder BSRRH) bestimmt, ob der IO-Pin auf 0 oder 1 geschaltet wird.
           
           F�r unser einfaches LED-Beispiel ergibt sich damit, dass wir im
           Falle des gedr�ckten Tasters die IO-Pins 12 bis 15 des Ports D
           auf 1 setzen wollen. Wir verwenden also das BSRRL-Register: */
            GPIOD->BSRRL |= 0xF000;
        } else {
        /* Ist der Taster nicht gedr�ckt, wollen wir die IO-Pins 12 bis 15 des
           Ports D auf 0 setzen. Wir verwenden also das BSRRH-Register: */
            GPIOD->BSRRH |= 0xF000;
            
        }
        
    } // end while (1)
    
#endif
}



//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------


/* Dieses Beispiel zeigt, wie man die Grundkonfiguration eines Timers
durchf�hrt und wie man den Timer auf sehr einfach Weise nutzen kann. */
void led_and_timer_example(void)
{
#ifdef LED_AND_TIMER
    
    /* Timer-Module geh�ren zu den vielseitigsten und damit wichtigsten
       Komponenten eines jeden Mikrocontrollers. Im Kern handelt es sich
       hierbei um Z�hler, die den Wert eines speziellen Registers je nach 
       Ausf�hrung herauf- oder herunterz�hlen. Der "Z�hlvorgang" kann dabei
       auf verschiedene Weise ausgel�st werden, z.B. durch einen Systemtakt
       oder ein Ereignis an einem IO-Pin. Zu diesen Kerneigenschaften 
       gesellen sich h�ufig Ereweiterungen, die es u.a. erm�glichen, PWM-
       Signale zu generieren oder Signale von Quadraturencodern zu 
       dekodieren.
       Der STM32F4 besitzt insgesamt 14 derartige Timer-Module. Die 
       Timer-Module werden mit TIM1 bis TIM14 bezeichnet. Trotz dieser 
       einheitlichen Bezeichnung unterscheiden sich die einzelnen Module
       jedoch in ihren F�higkeiten und Eigenschaften. Einige Timer haben
       16-Bit breite Register, einige haben 32-Bit breite Register, etc...
       Dar�ber hinaus sind ein Teil der Timer am schnellen Peripheriebus
       APB2 angebunden w�hrend ein anderer Teil am langsameren APB1-Bus h�ngt.
       Eine �bersicht hier�ber bietet Abbildung 5 in [2] auf Seite 17. Welche
       Eigenschaften die einzelnen Timer im Detail besitzen, kann ab Seite 
       293 in [1] nachgelesen werden.
       
       F�r das folgende Beispiel soll Timer-Modul 3 (TIM3) verwendet werden.
       Wie bereits die GPIO-Ports m�ssen auch die Timer Module �ber die reset
       and clock control (RCC) aktiviert und mit Takt versehen werden. Aus 
       Abbildung 5 in [2] (S.17) l�sst sich entnehmen, dass TIM3 am APB1-Bus
       h�ngt. Hierzu korrespondierend findet sich auf Seite 113 in [1] die
       Beschreibung des RCC_APB1ENR Konfigurationsregisters. Setzen wir hier
       Bit 1 auf "1", so wird TIM3 aktiviert und mit Takt versorgt: */
       
    RCC->APB1ENR |= 0x00000002;
    
    /* Ab Seite 392 in [1] werden die Konfigurationsregister der Timer Module
       TIM2 bis TIM5 beschrieben. F�r dieses Beispiel soll Timer 3 als 
       einfacher "up-counter" konfiguriert werden, der vom Wert 0 auf einen
       einstellbaren Maximalwert hochz�hlt und dann wieder bei 0 beginnt. Da
       dies die Standardkonfiguration (s. "reset value" S.392) des Timers ist,
       m�ssten wir eigentlich das Kontrollregister CR1 nicht anfassen. Zur
       Vollst�ndigkeit halber, wollen wir die Bits in diesem Register dennoch
       explizit setzen. Bit 0 des CR1-Registers startet den Z�hler. Dieses Bit
       wird am Ende der Konfiguration gesetzt. Das n�chste Bit von 
       unmittelbarem Interesse ist Bit 4. Es gibt die Z�hlrichtung (0=rauf 
       oder 1=runter) des Z�hlers an. Die n�chsten beiden Bits (5 und 6) geben 
       schlie�lich den "Z�hlmodus" an. Neben einfachem Rauf- und Runterz�hlen
       beherrschen die Z�hler auch Varianten wie abwechselndes Rauf- und 
       Runterz�hlen. Die weiteren Bits sollen an dieser Stelle erstmal nicht 
       n�her besprochen werden. Im Moment reicht es, diese Bits auf 0 zu 
       setzen. Die obersten 6 Bit des CR1-Registers sind als "Reserved" 
       angegeben und sollten unver�ndert bleiben. Um unseren Z�hler zu 
       konfigurieren setzen wir also einfach die unteren 10-Bit des CR1-
       Registers auf 0: */
    
    TIM3->CR1 &= 0xFC00;
    
    /* Als n�chstes muss festgelegt werden, wie schnell der Timer z�hlen soll.
       Man k�nnte zun�chst annehmen, dass Timer 3 direkt mit dem Bustakt des 
       APB1-Busses - an dem der Timer angebunden ist - versorgt wird. Ein 
       Blick auf Abbildung 9 in [1] (S.85) zeigt jedoch, dass den Timern noch
       ein "Taktverdopplungsmodul" vorgeschaltet ist (In der Mitte der 
       Abbildung rechts). Diese Taktverdopplung ist dann aktiv, wenn der 
       Prescaler des jeweiligen APB-Busses gr��er 1 ist. In unserem Fall wird
       der Timer also nicht mit dem 42 MHz Takt des APB1-Busses versorgt, 
       sondern mit einem doppelt so schnellen 84 MHz Takt. Bei dieser Taktrate
       vollzieht der Z�hler 84 Millionen Schritte pro Sekunde. Da es sich beim
       Timer 3 um ein 16-Bit breites Z�hlregister handelt, dauern die maximal
       m�glichen 65536 Schritte f�r einen Z�hlerdurchlauf bei dieser Geschwin-
       digkeit gerade einmal 0.78 Millisekunden! Damit man den Z�hler auch 
       langsamer laufen lassen kann gibt es die M�glichkeit den Z�hlertakt 
       durch einen Prescaler zu veringern. Dieser Prescaler wird im 16-Bit
       Konfigurationsregister PSC eingestellt (S.406 in [1]). F�r unser Bei-
       spiel wollen wir die LED im Sekundentakt (halbe Sekunde an, halbe 
       Sekunde aus) blinken lassen. Daher w�hlen wir einen Prescaler von 
       8400: */ 
       
    TIM3->PSC = 8400;
       
    /* Damit durchl�uft der Z�hler einen Bereich von 0 bis 10000 in etwa 
       1 Sekunde. Als n�chstes m�ssen wir die Obergrenze des Z�hlers 
       festlegen. Dies geschieht �ber das 16-Bit Konfigurationsregister 
       ARR (S.406 in [1]). Da wir nachher einen Sekundentakt haben wollen, 
       setzen wir die Obergrenze des Z�hlers auf 10000: */
       
    TIM3->ARR = 10000;
    
    /* Wenn der Z�hler nun den Wert 10000 erreicht, wird der Z�hler auf 0 
       zur�ckgestellt und das Heraufz�hlen beginnt von vorne. Bevor wir nun 
       den Z�hler starten k�nnen, muss �ber das Bit 0 des EGR Registers (event
       generation register, S.399 in [1]) ein manuelles Update-Event ausgel�st
       werden, damit der Z�hler unsere Konfiguration �bernimmt: */
       
    TIM3->EGR |= 0x0001;
    
    /* Zu guter Letzt muss der Timer �ber das Bit 0 des CR1 Registers (S.392 
       in [1]) gestartet werden: */
       
    TIM3->CR1 |= 0x0001;
    
    /* Bevor wir uns um die Hauptschleife des Programms k�mmern, wollen wir
       kurz unsere 4 LEDs in eine h�bsche Startkonfiguration bringen, in der
       zwei gegen�berliegende LEDs an und die anderen aus sind: */
    
    GPIOD->BSRRL |= 0xA000; // Pin 13 und 15 ein
    GPIOD->BSRRH |= 0x5000; // Pin 12 und 14 aus
    
    /* In der folgenden Hauptschleife verwenden wir das sogenannte 
       �berlaufereignis des Z�hlers, um unsere Hauptschleife im 1 Sekunden-
       Rythmus zu durchlaufen. Das �berlaufereignis tritt ein, wenn der Z�hler
       den im Register ARR eingestellten Wert erreicht und erneut bei 0 zu
       z�hlen beginnt. Tritt ein �berlauf ein, wird das Bit 0 des Timer Status
       Registers SR von der Hardware auf 1 gesetzt. Im Anschluss muss das Bit
       durch die Software wieder auf 0 gesetzt werden. Wir beginnen die 
       Hauptschleife mit der �berpr�fung dieses Ereignisses, damit auch schon 
       der erste Durchlauf 1 Sekunde lang ist. */
    
    while (1) {
        // Warten auf �berlaufereignis
        while ((TIM3->SR & 0x0001) == 0);
        // Resetten von Bit 0
        TIM3->SR &= 0xFFFE;
        // von hier an bis zum Ende der While-Schleife haben wir 1 Sekunde Zeit
        
        // Als erstes wollen wir die LEDs "toggeln", d.h. die LEDs, die an 
        // sind, sollen ausgehen, und die LEDs, die aus sind, sollen angehen.
        // Dies l�sst sich elegant �ber eine XOR-Operation auf dem ODR-Register 
        // der GPIO-Pins erledigen:
        
        GPIOD->ODR ^= 0x0000F000;
        
        // nun wollen wir eine halbe Sekunde warten und dann die LEDs erneut 
        // toggeln. �ber das Register CNT (S.406 in [1]) k�nnen wir auf den
        // aktuellen Wert unseres Z�hlers zugreifen. Aus unserer Konfiguration
        // wissen wir, dass der Z�hler einen Wert von 5000 erreichen muss, 
        // damit eine halbe Sekunde vergangen ist...
        
        while (TIM3->CNT < 5000);
        
        // und nun das erneute Toggeln... 
        
        GPIOD->ODR ^= 0x0000F000;
        
        // Am Ende unserer Schleife m�ssen wir nicht etwa auf einen Z�hlerwert 
        // von 10000 warten, da wir ja am Anfang der Schleife bereits auf das
        // �berlaufevent warten. Es w�re auch nicht klug, genau auf den 
        // h�chsten Z�hlerwert zu warten, da wir den genauen Wert "10000" u.U
        // verpassen. M�chte man dennoch am Ende der Schleife warten, so sollte
        // der H�chstwert des Z�hlers einen ausreichenden Abstand zu dem 
        // jeweils �berpr�ften Wert haben. 
    }
    
    
#endif
}






//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

/* In diesem Beispiel soll die Verwendung eines Interrupts erl�utert 
   werden. */
void timer_irq_example(void)
{
#ifdef TIMER_IRQ
    /* Im vorhergehenden Beispiel wurde das Blinken der LEDs in der 
       Hauptschleife des Programms gesteuert. Dies ist nicht "per se" ein 
       schlechter Ansatz. Wenn die �bergeordnete Aufgabe, die durch den 
       Mikrocontroller bew�ltigt werden soll, einen festen Rythmus verlangt, 
       also z.B. in einem festen Takt von 1Hz ablaufen soll, dann kann man 
       sein Mikrocontroller-Programm durchaus auf diese Weise strukturieren.
       Gerade bei einfachen Aufgaben sollte man es vermeiden, seinen Code zu
       "over-engineeren".
       In vielen F�llen ist es jedoch eher aufw�ndig, die zu l�sende Aufgabe
       in einen festen Rythmus zu pressen. An dieser Stelle lohnt sich die
       Verwendung von sogenannten Interrupts. Ein Interrupt ist die Unter-
       brechung des normalen Programmflusses aufgrund eines bestimmten 
       Ereignisses. Tritt das Ereignis ein, wird eine spezielle Stelle im
       Programm, die sogenannte Interrupt-Routine, aufgerufen und abgearbei-
       tet. Am Ende der Interrupt-Routine wird der Programmcode an der Stelle
       fortgesetzt, an der er zuvor unterbrochen wurde.
       Diese abstrakte Beschreibung l�sst sich gut anhand eines Timer-
       Interrupts illustrieren. Wie der Beschreibung des "Upcounting mode" des
       Timer Moduls 3 auf Seite 363 in [1] zu entnehmen ist, kann das 
       Timer Modul ein Interrupt-Ereignis namens "Update event" beim �ber-
       schreiten des im ARR-Register eingestellten Maximalwertes (s. vorheriges
       Beispiel) generieren. Damit dies geschieht, muss die Generierung von 
       Update-Events im "DMA/Interrupt enable register" (DIER) aktiviert werden
       (S.396 in [1]). Tritt nun ein Update-Event ein (der Z�hler �berschreitet
       den eingestellten Maximalwert), so setzt das Timer-Modul das Bit 0 des 
       Timer "status registers" (SR) auf 1 (S.397 in [1]). An dieser Stelle ist
       der "Job" des Timer-Moduls erst einmal erledigt. Damit aus diesem "Flag"
       im Status-Register (SR) auch tats�chlich eine Unterbrechnung des 
       Programmcodes resultiert, betritt noch ein weiter Spieler das Feld: der 
       NVIC (nested vectored interrupt controller). Dieses Hardware-Modul des
       Mikrocontrollers beobachtet kontinuierlich die Statusregister 
       zahlreicher Hardwarekomponenten. Detektiert der NVIC in einem Status-
       register eine 1 und ist der entsprechende Interrupt auch im NVIC 
       aktiviert, so unterbricht der NVIC den Programmfluss und bringt die
       korrespondierende Interruptroutine zur Ausf�hrung. Auf Seite 195ff in 
       [1] sind in Tabelle 30 die verschiedenen Hardwarekomponenten aufgef�hrt,
       die der NVIC beobachtet. Auf Seite 197 findet sich unter Position 29
       auch Timer 3, dessen Interrupt wir in diesem Beispiel nutzen wollen.
       Damit der NVIC auch tats�chlich die Interruptroutine des Timer 3 
       aufruft, m�ssen wir die Interruptbehandlung f�r Timer 3 im NVIC 
       aktivieren. Dies geschieht �ber das ISER-Register des NVIC. Da Timer 3
       laut Tabelle 30 der 29. Interrupt ist, setzen wir im ISER-Register das
       29. Bit auf 1: */
        
    NVIC->ISER[0] |= 0x20000000;
    
    /* Da es insgesamt 81 Interruptquellen gibt, besteht das ISER-Register aus
       3 x 32-Bit. M�chte man also z.B. den 37. Interrupt (USART1) aktivieren,
       so m�sste man auf die 2. 32-Bit zugreifen und dort Bit 5 auf 1 setzen.
       Damit ist der NVIC auch schon ausreichend konfiguriert, und wir k�nnen
       uns der Konfiguration des Timers widmen. Die Grundkonfiguration 
       entspricht (fast) 1-zu-1 der Konfiguration aus dem vorherigen Beispiel.
       Einziger Unterschied: Wir setzen nun die Obergrenze des Timers nicht auf
       1 Sekunde, sondern auf 500ms (ARR = 5000), da wir die LEDs im halb-
       sekundentakt toggeln wollen und der Interrupt nur bei einem �berlauf 
       ausgel�st wird: */
    
    RCC->APB1ENR |= 0x00000002;
    TIM3->CR1    &= 0xFC00;
    TIM3->PSC     = 8400;
    TIM3->ARR     = 5000;

    /* Neu ist nun, dass wir auch auf Seiten des Timers den Interrupt 
       aktivieren m�ssen. Dies geschieht, wie bereits erw�hnt, im 
       DIER-Register: */

    TIM3->DIER = 0x0001;
    
    /* Wie zuvor m�ssen wir eine 1 in das EGR-Register schreiben, um ein 
       manuelles Update der Konfigurationswerte zu erwirken. Im Anschluss
       starten wir den Timer indem wir Bit 0 des CR1 Registers auf 1 setzen.*/
    TIM3->EGR |= 0x0001;
    TIM3->CR1 |= 0x0001;
    
    /* Bevor wir uns weiter unten der Interruptroutine widmen, werden noch
       kurz die LEDs initialisiert. Zwei LEDs werden eingeschaltet und zwei
       LEDs werden ausgeschaltet: */
    
    GPIOD->BSRRL |= 0xA000; // Pin 13 und 15 ein
    GPIOD->BSRRH |= 0x5000; // Pin 12 und 14 aus
    
    while (1) {
    
    /* Dank der Verwendung eines Interrupts ist unsere Hauptschleife nun 
       vollkommen leer. ;-) Die Arbeit findet nun in der unten folgenden 
       Interruptroutine TIM3_IRQHandler(void) statt. */
    
    }
   
#endif
}

// bugfix - Erkl�rung folgt!
volatile int temp;

#ifdef TIMER_IRQ
void TIM3_IRQHandler(void) 
{   
    /* Die Interruptroutine wird immer dann aufgerufen, wenn der Z�hler den
       Maximalwert (ARR-Register!) �berschreitet. In diesem Fall schreibt das
       Timer-Modul eine 1 in das Status-Register (SR) des Timers. Der NVIC 
       bekommt dies mit und f�hrt die Interruptroutine TIM3_IRQHandler(void) 
       aus, in der wir uns gerade befinden. Aber Moment! Woher kennt der NVIC
       �berhaupt diese Methode? Aufschluss gibt erneut Tabelle 30 auf Seite 
       197 in [1]. In der rechten Spalte steht eine Adresse. An dieser Adresse
       erwartet der NVIC einen Funktionszeiger, der auf diese Methode 
       TIM3_IRQHandler(void) zeigt. Wie es dazu kommt, dass dies auch der Fall
       ist, kann man sich in der Datei startup_stm32f4xx.s anschauen. Dort 
       findet sich ab Zeile 125 die Definition einer "prototypischen" Vektor-
       tabelle (als Vektortabelle wird die Tabelle der Funktionspointer 
       bezeichnet, die der NVIC f�r seine Arbeit braucht...), die die 
       Bezeichnungen aller Interruptroutinen enth�lt. Weiter unten (Zeile 
       240ff) in startup_stm32f4xx.s werden diese Methodenbezeichner als "weak"
       deklariert. Dies bedeutet, dass sobald in irgendeiner C-Datei eine
       Methode den gleichen Namen hat, diese Methode den "weak"-Eintrag 
       �berschreibt! Exisitert keine Methode mit diesem Namen, so wird ein 
       Default-Handler aufgerufen. Durch diesen Mechanismus muss man also 
       einfach nur eine Methode mit passendem Namen definieren und schon kann
       diese vom NVIC aufgerufen werden. Schnieke!
       
       In unserem Fall wollen wir die LEDs toggeln. Zu diesem Zweck verwenden
       wir diesmal das ODR-Register und XORen die Bits der LEDs einfach mit 1.
       Dadurch toggeln die Ausg�nge bei jedem Interruptaufruf elegant hin und 
       her: */
    
    GPIOD->ODR ^= 0x0000F000;
    
    /* Um dem NVIC zu signalisieren, dass der Interrupt behandelt wurde, muss
       nun h�ndisch das passende Bit im Status-Register SR auf 0 zur�ckgesetzt
       werden. Geschieht dies nicht, so w�rde der NVIC die Interruptroutine
       sofort erneut aufrufen: */
    
    TIM3->SR &= 0xFFFE;
    
    /* Eigentlich w�re unsere Arbeit damit getan. Aber leider gibt es bei 
       diesem Minimalbeispiel ein kleines Problem. Da unser Programm effektiv
       nix tut, verl�uft der Kontextwechsel (so nennt man den �bergang zwischen
       Hauptprogramm und Interruptroutine und zur�ck) zu schnell. Es kommt zu
       einer sogenannten Race-Condition: Die 0, die wir ins Status-Register SR
       geschrieben haben, ist dort "noch nicht angekommen". Dies bedeutet, dass
       f�r den NVIC immer noch eine 1 im SR steht und er folgerichtig den
       Interrupt erneut aufruft. Um nun sicherzustellen, dass das SR auch 
       wirklich up-to-date ist, lesen wir das Register in eine externe, 
       volatile Variable "temp". Auf diese Weise wird die Race-Condition 
       zuverl�ssig unterbunden: */
       
    temp = TIM3->SR; 
    
    /* Kleine Bemerkung am Rande: M�chte man Daten zwischen Interruptroutine
       und Hauptprogramm �ber gemeinsame Variablen austauschen, so sollten
       diese Variablen immer als "volatile" gekennzeichnet werden. Ansonsten
       k�nnten es passieren, dass der Compiler die gemeinsame Variable in einem
       Register cached. Der Inhalt dieses Registers w�rde dann beim 
       Kontextwechsel verlorengehen. */
}
    
#endif





//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

/* In diesem Beispiel soll die Verwendung eines Timers zur PWM-Generierung
erl�utert werden. */
void pwm_led_example(void)
{
#ifdef PWM_LED
    
    /* In diesem Beispiel soll unser LED-Geblinke ein wenig aufgepeppt werden.
       Die LEDs sollen nicht einfach nur ein- oder ausgeschaltet werden, 
       sondern weich ein- und ausblenden. Dies soll mithilfe einer PWM-
       Ansteuerung erfolgen. PWM steht f�r "pulse width modulation" und 
       bedeutet, dass die LEDs mit einem Rechtecksignal angesteuert werden,
       bei dem das Verh�ltnis von High zu Low Pegel eingestellt werden kann.
       Dieses Verh�ltnis wird h�ufig auch als "duty cycle" oder als 
       "Tastverh�ltnis" bezeichnet. Je gr��er der Anteil des High-Pegels ist,
       desto heller leuchtet die LED.
      
       Grundvoraussetzung daf�r, dass wir die LEDs mit einem PWM-Signal 
       ansteuern k�nnen, ist ihr Anschluss an den sogenannten "output compare"
       Ausg�ngen eines oder mehrerer Timer, mit denen wir das PWM-Signal 
       erzeugen k�nnen. In Tabelle 7 auf Seite 57 in [2] ist nachzulesen, dass
       unsere LEDs (PD12 bis PD15) praktischerweise an den 4 output compare 
       Ausg�ngen des Timers 4 h�ngen! Bevor es an die Konfiguration geht, sei
       in K�rze erkl�rt, wie mit einem Timer die 4 PWM-Signale f�r die LEDs
       erzeugt werden. Im einfachsten Fall wird der Timer als "normaler" Z�hler
       konfiguriert, der kontinuierlich von 0 bis zu einem eingestellten
       Maximalwert hochz�hlt und anschlie�end wieder bei 0 beginnt. Zus�tzlich
       besitzt der Z�hler f�r jeden output compare Ausgang ein zugeh�riges 
       output compare Register. In dieses Register schreibt man nun einen Wert,
       der irgendwo zwischen 0 und dem eingestellten Maximalwert liegt. Solange
       der Wert des Z�hlers kleiner ist, als der Wert im output compare 
       Register, hat der output compare Ausgang den Wert 1. Ist der Wert des
       Z�hlers gr��er als der Wert im output compare Register, so hat der 
       zugeh�rige output compare Ausgang den Wert 0. Auf diese Weise kann �ber
       die output compare Register das Tastverh�ltnis des am output compare
       Ausgang entstehenden Rechecksignals eingestellt werden. Die Frequenz
       dieses Rechtecksignals ist durch die Geschwindigkeit des Z�hlers 
       gegeben. Ein voller Durchlauf des Z�hlers entspricht hierbei einer 
       Periode des Rechtecksignals. Die Z�hler des STM32 bieten noch zahlreiche
       weitere Varianten, wie PWM-Signale erzeugt werden k�nnen. Die hier be-
       schriebene Form geh�rt zu den einfachsten Arten und ist f�r die An-
       steuerung der LEDs in diesem Beispiel voll und ganz ausreichend.
       
       Damit die output compare Ausg�nge des Timers 4 an den Pins PD12 bis
       PD15 zur Verf�gung stehen, m�ssen Pins PD12 bis PD15 als "alternate
       function" konfiguriert werden. Ein Blick auf Tabelle 14 in [1] (S.138)
       ergibt auf die Schnelle, welche Einstellungen wir vornehmen m�ssen:
       Wir wollen die Pins als Alternate Function (AF), Push Pull und mit
       50 MHz konfigurieren. Die Geschwindigkeit ist hierbei nicht ganz so
       wichtig. 25MHz w�rden es sicherlich auch tun...
       
       Anhand von Tabelle 14 ergibt sich damit erstmal folgende 
       Konfiguration der IO-Pins: */
    
    GPIOD->MODER &= 0x00FFFFFF;
    GPIOD->MODER |= 0xAA000000;
    
    GPIOD->OTYPER &= 0xFFFF0FFF;
    
    GPIOD->OSPEEDR &= 0x00FFFFFF;
    GPIOD->OSPEEDR |= 0xAA000000;
    
    GPIOD->PUPDR &= 0x00FFFFFF;

    /* Nun m�ssen wir noch ausw�hlen, welche "Alternate Function" wir verwenden
       wollen. Aus Tabelle 7 in [2] (S.57) k�nnen wir entnehmen, dass die 
       output compare Ausg�nge des Timer 4 als Alternate Function 2 deklariert
       sind. �ber das AFR-Register des GPIO-Ports D k�nnen wir diese Auswahl 
       f�r unsere LED-Pins PD12 bis PD15 treffen. Das Register AFR besteht aus
       zwei 32-Bit Registern, von denen wir das zweite 32-Bit Register 
       ben�tigen. �rgerlicherweise wird in [1] das Register auf den Seiten 152
       und 153 als AFRL und AFRH beschrieben, in der Library-Header Datei 
       stm32f4xx.h jedoch als Array AFR[2] definiert. Gem�� der Beschreibung 
       auf Seite 153 f�r AFRH konfigurieren wir also AFR[1]: */
       
    GPIOD->AFR[1] &= 0x0000FFFF;
    GPIOD->AFR[1] |= 0x22220000;
    
    /* Damit sind die GPIO-Pins vorbereitet und wir k�nnen uns der 
       Konfiguration des Timers widmen. Wie zuvor muss auch Timer 4 erstmal 
       beim RCC aktiviert und mit Takt versorgt werden: */

    RCC->APB1ENR |= 0x00000004;
    
    /* Im Anschluss daran konfigurieren wir Timer 4 erstmal genauso wie Timer 3
       zuvor - als einfachen Up-Counting Z�hler: */
       
    TIM4->CR1 &= 0xFC00;
    
    /* Den Prescaler w�hlen wir jedoch deutlich geringer! Mit einem Wert von 42
       hoppelt unser Z�hler nun mit flinken 2MHz (84MHz / 42) daher: */
       
    TIM4->PSC = 42;
    
    /* Den Maximalwert setzen wir auf 1000. Damit erreichen wir 2000 volle 
       Z�hlerdurchl�ufe pro Sekunde und erzeugen damit eine PWM-Frequenz von
       2KHz: */
       
    TIM4->ARR = 1000;
    
    /* Als n�chstes stellen wir den passenden Output-Compare-Modus mithilfe der
       Register CCMR1 und CCMR2 (S.400ff in [1]) f�r alle 4 Output Compare 
       Kan�le des Timer 4 ein: */
       
    TIM4->CCMR1 &= 0x0000;
    TIM4->CCMR1 |= 0x6868;

    TIM4->CCMR2 &= 0x0000;
    TIM4->CCMR2 |= 0x6868;
    
    /* �ber das CCER-Register (S.404 in [1]) aktivieren wir schlie�lich den 
       Output-Compare-Modus f�r alle 4 OC-Kan�le des Timers: */
       
    TIM4->CCER &= 0x4444;
    TIM4->CCER |= 0x1111;

    /* Der Vollst�ndigkeit halber initialisieren wir auch die 4 Output-Compare-
       Register. Der kleine Wert von 5 f�hrt dazu, das die PWM Signale 
       �berwiegend den Wert 0 haben und die LEDs damit nur ganz schwach 
       leuchten: */
       
    TIM4->CCR1 = 5;
    TIM4->CCR2 = 5;
    TIM4->CCR3 = 5;
    TIM4->CCR4 = 5;
    
    /* Wie auch zuvor bei Timer 3 m�ssen wir mittels des EGR-Registers ein
       manuelles Update-Event ausl�sen, damit die Konfiguration des Timers 
       �bernommen wird. Im Anschluss daran kann Timer 4 �ber das CR1-Register
       eingeschaltet werden: */
       
    TIM4->EGR |= 1;
    TIM4->CR1 |= 1;

    /* Damit haben wir unsere LEDs dimmbar gemacht! �ber die Register CCR1 bis
       CCR2 k�nnen wir nun die Helligkeit der LEDs mit Werten zwischen 0 und 
       1000 einstellen. Jetzt k�mmern wir uns um den Effekt eines weichen Ein-
       und Ausblendens der LEDs. Hierf�r m�ssen wir die CCR-Register zun�chst
       langsam erh�hen und anschlie�end wieder langsam erniedrigen. Wir 
       brauchen also einen weiteren Timer, der diese Arbeit f�r uns erledigt.
       Was liegt n�her, als den uns schon bekannten Timer 3 daf�r zu verwenden!
       Also fluchs den Interrupt beim NVIC angeknipst (Timer 3 war Nummer 29)
       und in der �blichen Konfiguration als einfacher Up-Counter 
       konfiguriert: */
       
    NVIC->ISER[0] |= 0x20000000;  // Interrupt von Timer 3 beim NVIC aktivieren

    RCC->APB1ENR |= 0x00000002;   // Timer 3 mit Takt versorgen
    TIM3->CR1    &= 0xFC00;       // Einfacher Upcoutner
    TIM3->PSC     = 8400;         // Prescaler von 8400 -> 10000 Ticks /Sekunde
    TIM3->ARR     = 10000 / 16;   // Maximalwert von 10000/16 -> s.u.
    TIM3->DIER    = 0x0001;       // "�berlauf"-Interrupt aktivieren
    TIM3->EGR    |= 1;            // "manuelles Update"
    TIM3->CR1    |= 1;            // Timer 3 anschalten
    
    /* Warum wir als Maximalwert 10000 Sechzehntel verwenden, wird weiter unten
       deutlich werden... */

    while (1) {
        
    /* Die Hauptschleife bleibt mal wieder leer, da wir auch hier Timer 3 mit 
       Interrupt verwenden ...*/
        
    }
    
#endif
}

#ifdef PWM_LED

/* Hier kommt also nun unser weiches Blinken! Die Werte, die wir nach und nach
   in die CCR-Register schreiben wollen, definieren wir zun�chst als konstantes
   Array. Dar�ber hinaus definieren wir uns noch zwei volatile Indizes, mit 
   denen wir dieses Array sp�ter indiziern wollen. Wie verwenden zwei Indizes,
   da wir wie bisher die LEDs paarweise abwechselnd blinken lassen wollen. Dies
   erreichen wir durch einen Versatz in der Initialisierung der Indizes: 
   idx1 wird mit 0 initialisiert, d.h. die zugeh�rigen LEDs starten mit einem 
   CCR-Wert von compareValues[idx1] = 5 und sind entsprechend dunkel. Das 
   andere Paar LEDs startet mit einem CCR-Wert von compareValues[idx2] = 995,
   da idx2 mit 8 initialisiert wird. Das zweite Paar LEDs ist also zu Beginn
   hell. */

static const uint16_t compareValues[16] = {
    5,15,30,60,150,500,750,995,995,750,500,150,60,30,15,5
};

volatile uint8_t idx1 = 0;
volatile uint8_t idx2 = 8;

void TIM3_IRQHandler(void) 
{   
    /* In der Interruptroutine werden die CCR-Register mit einem neuen 
       Wert aus unserem compareValues-Array gem�� der Indizes idx1 und idx2
       versorgt. */
    
    TIM4->CCR1 = compareValues[idx1]; // PD12
    TIM4->CCR2 = compareValues[idx2]; // PD13
    TIM4->CCR3 = compareValues[idx1]; // PD14
    TIM4->CCR4 = compareValues[idx2]; // PD15
    
    /* Im Anschluss daran werden die beiden Indizes um 1 erh�ht. Sollten die
       Indizes einen Wert von 16 erreichen, sorgt die VerUNDung mit 15 daf�r,
       dass aus der 16 eine 0 wird und wir wieder vorne in unserem Array 
       compareValues beginnen. Die VerUNDung mit 15 entspricht also einer 
       Modulo-Operation mit dem Wert 16. Sie ist nur deutlich effizienter.
       Dieser "Modulo-Trick" funktioniert immer dann, wenn man Modulo einer
       Zweierpotenz rechnen will. Dies ist u.a. ein Grund daf�r, warum man
       gerne Arrayl�ngen als Zweierpotenz w�hlt: */

    idx1 = (idx1 + 1) & 15;
    idx2 = (idx2 + 1) & 15;

    /* An dieser Stelle wird nun auch klar, warum wir einen Maximalwert (ARR)
       von 10000/16 gew�hlt haben. Da das Array 16 Werte enth�lt, laufen wir
       mit diesem Maximalwert genau 1 mal pro Sekunde durch das Array. Die LEDs
       blinken also weiterhin im 1 Sekunden-Takt. */


    /* Wie zuvor m�ssen wir auch hier das Flag im Status-Register des Timers
       am Ende der Interruptroutine auf 0 zur�cksetzen. */
    TIM3->SR &= 0xFFFE;
    
    /* Und auch hier ist wieder unser "Bugfix" am Start... */
    temp = TIM3->SR; 
    
}
    
#endif


//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

/* Das folgende Beispiel demonstriert, wie man mithilfe des DMA controllers
   g�nzlich ohne Unterbrechung der Hauptprogrammschleife die LEDs weich ein-
   und ausblenden lassen kann.*/
void dma_pwm_led_example(void)
{
#ifdef DMA_LED
    /* Der sogenannte "direct memory access" (DMA) erlaubt den Transfer von
       Daten zwischen Teilen des Speichers, vom Speicher zur Peripherie und
       von der Peripherie zum Speicher, ohne dass der Prozessorkern daran
       beteiligt werden muss. Zudem kann der Transfer von bestimmten 
       Hardwareereignissen ausgel�st werden. Diesen Transfer �bernimmt der
       sogenannte "DMA controller". Der STM32 F4 hat dieser zwei. Sie werden
       ab Seite 160 in [1] ausf�hrlich beschrieben.
       F�r dieses Beispiel wollen wir den DMA controller auf folgende Weise
       verwenden: Aus einem Array im Speicher, dass der DMA controller 
       zyklisch auslesen soll, laden wir nach und nach die passenden compare
       Werte in die CCR-Register des Timer 4, der die PWM-Signale f�r unsere
       LEDs erzeugt. Die �bertragung der einzelnen Daten soll hierbei vom
       Timer 3 ausgel�st werden, so dass wir das gleiche "Blinkergebnis" 
       erzielen wie beim vorherigen Beispiel - nur eben ohne die regelm��ige
       Unterbrechung der Hauptschleife!
    
       Bevor wir zum Kern des Beispiels, der Konfiguration des DMA-Controllers,
       kommen, m�ssen wir Timer 4 wie gehabt konfigurieren. Als erstes stellen
       wir die Pins PD12 bis PD15 wieder auf die Alternate Function 2 ein: */
    
    GPIOD->MODER &= 0x00FFFFFF;
    GPIOD->MODER |= 0xAA000000;
    
    GPIOD->OTYPER &= 0xFFFF0FFF;
    
    GPIOD->OSPEEDR &= 0x00FFFFFF;
    GPIOD->OSPEEDR |= 0xAA000000;
    
    GPIOD->PUPDR &= 0x00FFFFFF;

    GPIOD->AFR[1] &= 0x0000FFFF;
    GPIOD->AFR[1] |= 0x22220000;
    
    /* Als n�chstes konfigurieren wir wie gehabt den Timer 4 als PWM-Generator
       f�r unsere 4 LEDs: */
    
    RCC->APB1ENR |= 0x00000004; // Timer mit Takt versorgen

    TIM4->CR1 &= 0xFC00; // Konfiguration als normaler Up-Counter
    
    TIM4->PSC = 42;   // Prescaler und Maximalwert so einstellen, dass wir 
    TIM4->ARR = 1000; // eine PWM-Frequenz von 2KHz erhalten
    
    TIM4->CCMR1 &= 0x0000; // output compare f�r alle 4 outputs konfigurieren
    TIM4->CCMR1 |= 0x6868;
    TIM4->CCMR2 &= 0x0000;
    TIM4->CCMR2 |= 0x6868;
       
    TIM4->CCER &= 0x4444;
    TIM4->CCER |= 0x1111;

    // Compare register schonmal mit einem Wert initialisieren...
    // -- im Grunde nicht wirklich notwendig --
    TIM4->CCR1 = 5;
    TIM4->CCR2 = 5;
    TIM4->CCR3 = 5;
    TIM4->CCR4 = 5;
    
    TIM4->EGR |= 1; // manuelles Update Event ausl�sen
    TIM4->CR1 |= 1; // Timer aktivieren

    
    /* Man sieht also, das die Timer-Konfiguration 1-zu-1 der Konfiguration 
       aus dem vorhergehenden Beispiel entspricht. Der Timer wei� also im 
       Grunde nichts davon, dass er gleich vom DMA controller traktiert wird...
       
       Als n�chstes definieren wir ein Array, dass eine sch�ne Abfolge von
       Compare-Werten enth�lt. Es f�llt auf, dass im Vergleich zum vorher-
       gehenden Beispiel das Array nunmehr 24 anstatt 16 Eintr�ge enth�lt
       und die letzten 8 Eintr�ge einer halben "Blinkperiode" entsprechen.
       Dieser etwas seltsame Aufbau des Arrays wird im gleich noch genauer
       erl�utert... */

    static const uint16_t compareValues[24] = {
          5,  15,  30,  60, 150, 500, 750, 995,
        995, 750, 500, 150,  60,  30,  15,   5,
          5,  15,  30,  60, 150, 500, 750, 995
    };
    
    /* An dieser Stelle folgt endlich die Konfiguration des DMA-Controllers.
       Als allererstes m�ssen wir den DMA-Controller, wie jede andere 
       Hardwarekomponente auch, im RCC aktivieren und mit einem Taktsignal
       versorgen: */
    
    RCC->AHB1ENR |= 0x00200000; // DMA1 mit Takt versorgen
    
    /* Jeder DMA controller des STM32 besitzt acht sogenannte "Streams", die
       unabh�ngig voneinander DMA Operationen ausf�hren k�nnen (s.Abb.19 auf 
       S.162 in [1]). Dar�ber hinaus besitzt jeder Stream 8 Kan�le, die �ber
       einen Einstrag (CHSEL) im Stream configuration register (CR) ausgew�hlt
       werden k�nnen (s.Abb.21 auf Seite 164 in [1]). Dem ersten Anschein nach
       klingt das nach einer gro�en und flexiblen Konfigurierbarkeit des DMA-
       controllers. Betrachtet man jedoch Tabelle 20 auf Seite 164 in [1], so
       erkennt man, dass jede Stream/Kanal-Kombination nur eine sehr begrenzte
       Menge an Hardwarekomponenten als Ausl�ser f�r einen DMA-Transfer 
       zul�sst. In diesem Beispiel ben�tigen wir 4 DMA Ausl�ser, mit denen wir
       die Daten des Arrays "compareValues" in die 4 Compare-Register des 
       Timer 4 schieben k�nnen. Praktischerweise k�nnen die 4 Compare Register
       des Timer 3 (!) derartige DMA-Transfers ausl�sen. Die 4 Compare-"Kan�le"
       des Timer 3 finden sich in Tabelle 20 in [1] (S.164) als TIM3_CH1 bis 
       TIM3_CH4. Laut dieser Tabelle m�ssen wir also die Streams 2, 4, 5 und 7
       des DMA1 controllers jeweils mit Kanal 5 konfigurieren.

       Auf Seite 179 in [1] wird Schritt f�r Schritt erkl�rt, in welcher 
       Reihenfolge ein Stream des DMA controller konfiguriert werden muss.
       In einem ersten Schritt muss der Stream abgeschaltet werden. Dies
       geschieht durch ein Schreiben von 0 an Bit 0 des Stream configuration 
       registers (CR, S.185 in [1]): */
       
    DMA1_Stream2->CR &= 0xFFFFFFFE;
    
    /* Auch in diesem Fall stimmen leider die Registerbezeichnungen in [1] und
       die Registerbezeichnungen in der library Datei stm32f4xx.h nicht 
       �berein.
       
       Da das Abschalten eines Streams nicht zwingend sofort erfolgt, sondern
       ein m�glicherweise noch stattfindender Transfer zuerst beendet wird, 
       muss auf die Deaktivierung des Streams gewartet werden, indem man das
       Bit 0 des CR-Registers beobachtet: */
    
    while ((DMA1_Stream2->CR & 0x00000001) == 1);
    
    /* Das gleiche Spiel machen wir nun auch f�r die anderen drei Streams, die
       wir konfigurieren wollen: */
    
    DMA1_Stream4->CR &= 0xFFFFFFFE;
    while ((DMA1_Stream4->CR & 0x00000001) == 1);

    DMA1_Stream5->CR &= 0xFFFFFFFE;
    while ((DMA1_Stream5->CR & 0x00000001) == 1);

    DMA1_Stream7->CR &= 0xFFFFFFFE;
    while ((DMA1_Stream7->CR & 0x00000001) == 1);
    
    /* Als n�chstes l�schen wir die Status-Eintr�ge der 4 Streams in den 
       Status-Registern LISR und HISR (S.181/182): */
    
    DMA1->LISR &= 0xFF82FFFF;
    DMA1->HISR &= 0xF0FFF082;
    
    /* Interessanterweise sind diese Register in [1] als "read only" beschrieben,
       obwohl die Anleitung auf S.179 etwas anderes verlauten l�sst.
       
       Die Konfiguration des DMA-controllers geht nun mit dem Einstellen der 
       Peripherie-Adresse weiter. Je nach Modus des DMA kann diese Adresse als
       Quell- oder als Zieladresse vom DMA-controller interpretiert werden. In
       unserem Fall wollen Daten an die Peripherie schicken. Es handelt sich
       also um die Zieladressen unserer DMA-Operation. Die Adressen der 
       Peripherie-Register, in die wir schreiben wollen, werden im "peripheral
       address register" (PAR, S.188 in [1]) des jeweiligen Streams 
       abgelegt: */
    
    DMA1_Stream2->PAR = (uint32_t)(&(TIM4->CCR1));
    DMA1_Stream4->PAR = (uint32_t)(&(TIM4->CCR2));
    DMA1_Stream5->PAR = (uint32_t)(&(TIM4->CCR3));
    DMA1_Stream7->PAR = (uint32_t)(&(TIM4->CCR4));
    
    /* Wie bereits erw�hnt handelt es sich bei den Zielregistern um die Output-
       Compare-Register CCR1 bis CCR4 des Timers 4, der die PWM-Signale der 
       LEDs generiert. Wir erhalten die Adresse der Register durch die 
       Verwendung des Adressoperators "&". Der Cast nach uint32_t ist n�tig, 
       da die PAR-Register als einfache 32Bit-Werte definiert sind.
       
       Im n�chsten Schritt konfigurieren wir die Quelladressen der Daten, die
       in die Register CCR1 bis CCR4 wandern sollen. Dies geschieht �ber die
       "memory 0 address register" (M0AR, S.188 in [1]) des jeweiligen 
       Streams: */

    DMA1_Stream2->M0AR = (uint32_t)(compareValues);
    DMA1_Stream4->M0AR = (uint32_t)(compareValues + 8);
    DMA1_Stream5->M0AR = (uint32_t)(compareValues);
    DMA1_Stream7->M0AR = (uint32_t)(compareValues + 8);
    
    /* An dieser Stelle kann man schon ahnen, warum das compareValues-Array 24
       anstelle von 16 Eintr�gen hat. Wir stellen in den M0AR-Registern n�mlich
       f�r zwei Compare-Register die Quelladressen um eine halbe Periode des
       16-wertigen Blinksignals versetzt nach vorne ein. Damit erreichen wir
       sp�ter ein paarweise abwechselndes Blinken der LEDs.
       
       Das Bild vervollst�ndigt sich mit der n�chsten Einstellung: Im "number
       of data register" (NDTR, S.188 in [1]) der jeweiligen Streams stellen
       wir ein, wie viele Datenelemente an der jeweiligen Speicheradresse zu
       erwarten sind. Hier setzen wir die Anzahl der Elemente auf 16 (!) und 
       nicht etwa auf 24: */
    
    DMA1_Stream2->NDTR = 16;
    DMA1_Stream4->NDTR = 16;
    DMA1_Stream5->NDTR = 16;
    DMA1_Stream7->NDTR = 16;
    
    /* F�r ein LED-Paar nutzen wir also das compareValues-Array von 0 bis 15 
       und f�r das andere LED-Paar nutzen wir das Array von 8 bis 23. Wie 
       genau und wann die einzelnen Werte in die CCR-Register geschrieben 
       werden, dass wird im n�chsten Konfigurationsschritt klar. 
       
       Der Rest der Konfiguration des DMA-Controllers findet �ber die 
       configuration register (CR, S.185 in [1]) der einzelnen Streams statt.
       An dieser Stelle lohnt sich ein ausf�hrliches Studium der 
       Registerbeschreibung in [1], da eine sehr gro�e Anzahl an Optionen in
       diesem Register eingestellt werden (m�ssen). 
       
       Fangen wir also an: Mit den Bits 25 bis 27 w�hlen wir DMA-Kanal 5 f�r
       jeden der Streams aus (s. Tabelle 20 in [1] S. 164). Sowohl den "memory
       burst transfer" (MBURST, Bits 23 bis 24) als auch den "peripheral burst 
       transfer" (PBURST, Bits 21 bis 22) stellen wir auf "single transfer", da
       wir pro DMA-Transfer immer nur ein Datenelement �bertragen wollen. Als
       n�chstes stellen wir das "priority level" (PL, Bits 16 und 17) auf 
       "Low", da das Blinken der LEDs ja nu nicht wirklich wichtig ist und 
       keine andere (DMA-)Operation behindern sollte. In Bits 11 bis 14 stellen
       wir schlie�lich die Gr��e (PSIZE und MSIZE) unserer Datenelemente in der
       Peripherie und im Speicher ein. In beiden F�llen sind dies 16-Bit.
       
       Achtung! Hier kommt ein wesentlicher Teil der Konfiguration: In Bit 10 
       (MINC) stellen wir ein, dass der DMA-Controller die Speicheradresse im
       M0AR-Register nach jedem DMA-Transfer um MSIZE (s.o.) erh�hen soll! Wir
       wandern also mit jedem DMA-Transfer eine Stelle weiter in unserem 
       compareValues-Array. Im Bit 9 (PINC) stellen wir ein, dass diese 
       Erh�hung der Speicheradresse nach jedem DMA-Transfer nicht f�r das 
       PAR-Register geleten soll. Wir wollen ja immer in das entsprechende 
       CCR-Register des Timers schreiben!
       
       Was passiert aber nun, wenn wir an das Ende unseres compareValues-Array
       gelangen? Dies stellen wir in Bit 8 (CIRC) ein. Dort aktivieren wir den
       sogenannten "circular mode". Nachdem NDTR (s.o.) DMA-Transfers 
       durchgef�hrt wurden, wird die urspr�ngliche Speicheradresse im M0AR-
       Register wieder hergestellt! Wir laufen also kontinuierlich 16 Schritte
       in unserem compareValues-Array ab und beginnen dann wieder an der 
       jeweiligen Startadresse (compareValues[0] bzw. compareValues[8]). Mit 
       den Bits 6 und 7 (DIR) stellen wir schlie�lich die Richtung des 
       DMA-Transfers ein. Wir w�hlen in diesem Fall "Memory-to-peripheral" aus.
       
       Zusammengefasst ergibt dieses Bitgefrickel nun die folgende 
       Konfiguration der 4 Streams: */
    
    DMA1_Stream2->CR = 0x0A002D40;
    DMA1_Stream4->CR = 0x0A002D40;
    DMA1_Stream5->CR = 0x0A002D40;
    DMA1_Stream7->CR = 0x0A002D40;

    /* Endlich - m�chte man sagen - haben wir die Konfiguration beendet und
       k�nnen die DMA-Streams aktivieren. Dies geschieht �ber das Schreiben 
       einer 1 in die Bits 0 der einzelnen Stream configuration register:*/

    DMA1_Stream2->CR |= 0x00000001;
    DMA1_Stream4->CR |= 0x00000001;
    DMA1_Stream5->CR |= 0x00000001;
    DMA1_Stream7->CR |= 0x00000001;

    /* So, jetzt m�ssen wir die DMA-Transfers der einzelnen Streams nur noch
       in regelm��igen Abst�nden ansto�en. Zu diesem Zweck verwenden wir 
       Timer 3 und seine 4 Output-Compare Einheiten. Wir konfigurieren Timer
       3 jedoch so, dass er nur "compared" aber nicht "outputted" ;-)
       
       Zun�chst einmal wird der Timer wie gehabt als einfacher Up-Counter 
       eingestellt, der 16 mal pro Sekunde einen ganzen Z�hler-Durchlauf
       vollzieht: */

    RCC->APB1ENR |= 0x00000002;   // Timer 3 mit Takt versorgen
    TIM3->CR1    &= 0xFC00;       // Einfacher Upcoutner
    TIM3->PSC     = 8400;         // Prescaler von 8400 -> 10000 Ticks /Sekunde
    TIM3->ARR     = 10000 / 16;   // Maximalwert von 10000/16 -> s.u.

    /* An dieser Stelle konfigurieren wir die 4 Output-Compare-Einheiten des
       Timers so, dass er keine Outputs generiert: */
       
    TIM3->CCMR1 = 0x0000;
    TIM3->CCMR2 = 0x0000;
    TIM3->CCER  = 0x0000;

    /* In den vier Compare-Registern des Timers platzieren wir h�bsch mittig
       zwischen 0 und dem Maximalwert in ARR einen Vergleichswert von 10000/32,
       so dass bei jedem Z�hlerdurchlauf ein Compare-Ereignis ausgel�st 
       wird: */
       
    TIM3->CCR1 = 10000 / 32;
    TIM3->CCR2 = 10000 / 32;
    TIM3->CCR3 = 10000 / 32;
    TIM3->CCR4 = 10000 / 32;
    
    /* Im bereits bekannten DMA/Interrupt enable register (DIER) stellen wir
       nun ein, dass der Timer einen DMA-Request zu jedem Compare-Event der
       4 Compare-Einheiten generieren soll. Auf diese Weise sto�en wir nun 
       16 mal pro Sekunde einen DMA-Transfer vom Speicher in die CCR-Register
       des Timer 4 (!) an: */
       
    TIM3->DIER    = 0x1E00;       // CC-DMAs aktivieren
    
    /* Wie immer m�ssen wir noch ein manuelles Update mittels des EGR-Registers
       generieren und schlie�lich Timer 3 �ber das CR1-Register aktivieren: */
       
    TIM3->EGR    |= 1;            // "manuelles Update"
    TIM3->CR1    |= 1;            // Timer 3 anschalten
    
    while (1) {
        
    /* Nun bleibt die Hauptschleife mal wieder leer und sie wird in ihrer Leere
       noch nicht einmal von einem Interrupt unterbrochen. :) */
        
    }
    
#endif
}






